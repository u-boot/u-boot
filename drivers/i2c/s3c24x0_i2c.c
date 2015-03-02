/*
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same I2C controller inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#if (defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pinmux.h>
#else
#include <asm/arch/s3c24x0_cpu.h>
#endif
#include <asm/io.h>
#include <i2c.h>
#include "s3c24x0_i2c.h"

#define	I2C_WRITE	0
#define I2C_READ	1

#define I2C_OK		0
#define I2C_NOK		1
#define I2C_NACK	2
#define I2C_NOK_LA	3	/* Lost arbitration */
#define I2C_NOK_TOUT	4	/* time out */

/* HSI2C specific register description */

/* I2C_CTL Register bits */
#define HSI2C_FUNC_MODE_I2C		(1u << 0)
#define HSI2C_MASTER			(1u << 3)
#define HSI2C_RXCHON			(1u << 6)	/* Write/Send */
#define HSI2C_TXCHON			(1u << 7)	/* Read/Receive */
#define HSI2C_SW_RST			(1u << 31)

/* I2C_FIFO_CTL Register bits */
#define HSI2C_RXFIFO_EN			(1u << 0)
#define HSI2C_TXFIFO_EN			(1u << 1)
#define HSI2C_TXFIFO_TRIGGER_LEVEL	(0x20 << 16)
#define HSI2C_RXFIFO_TRIGGER_LEVEL	(0x20 << 4)

/* I2C_TRAILING_CTL Register bits */
#define HSI2C_TRAILING_COUNT		(0xff)

/* I2C_INT_EN Register bits */
#define HSI2C_TX_UNDERRUN_EN		(1u << 2)
#define HSI2C_TX_OVERRUN_EN		(1u << 3)
#define HSI2C_RX_UNDERRUN_EN		(1u << 4)
#define HSI2C_RX_OVERRUN_EN		(1u << 5)
#define HSI2C_INT_TRAILING_EN		(1u << 6)
#define HSI2C_INT_I2C_EN		(1u << 9)

#define HSI2C_INT_ERROR_MASK	(HSI2C_TX_UNDERRUN_EN |\
				 HSI2C_TX_OVERRUN_EN  |\
				 HSI2C_RX_UNDERRUN_EN |\
				 HSI2C_RX_OVERRUN_EN  |\
				 HSI2C_INT_TRAILING_EN)

/* I2C_CONF Register bits */
#define HSI2C_AUTO_MODE			(1u << 31)
#define HSI2C_10BIT_ADDR_MODE		(1u << 30)
#define HSI2C_HS_MODE			(1u << 29)

/* I2C_AUTO_CONF Register bits */
#define HSI2C_READ_WRITE		(1u << 16)
#define HSI2C_STOP_AFTER_TRANS		(1u << 17)
#define HSI2C_MASTER_RUN		(1u << 31)

/* I2C_TIMEOUT Register bits */
#define HSI2C_TIMEOUT_EN		(1u << 31)

/* I2C_TRANS_STATUS register bits */
#define HSI2C_MASTER_BUSY		(1u << 17)
#define HSI2C_SLAVE_BUSY		(1u << 16)
#define HSI2C_TIMEOUT_AUTO		(1u << 4)
#define HSI2C_NO_DEV			(1u << 3)
#define HSI2C_NO_DEV_ACK		(1u << 2)
#define HSI2C_TRANS_ABORT		(1u << 1)
#define HSI2C_TRANS_SUCCESS		(1u << 0)
#define HSI2C_TRANS_ERROR_MASK	(HSI2C_TIMEOUT_AUTO |\
				 HSI2C_NO_DEV | HSI2C_NO_DEV_ACK |\
				 HSI2C_TRANS_ABORT)
#define HSI2C_TRANS_FINISHED_MASK (HSI2C_TRANS_ERROR_MASK | HSI2C_TRANS_SUCCESS)


/* I2C_FIFO_STAT Register bits */
#define HSI2C_RX_FIFO_EMPTY		(1u << 24)
#define HSI2C_RX_FIFO_FULL		(1u << 23)
#define HSI2C_TX_FIFO_EMPTY		(1u << 8)
#define HSI2C_TX_FIFO_FULL		(1u << 7)
#define HSI2C_RX_FIFO_LEVEL(x)		(((x) >> 16) & 0x7f)
#define HSI2C_TX_FIFO_LEVEL(x)		((x) & 0x7f)

#define HSI2C_SLV_ADDR_MAS(x)		((x & 0x3ff) << 10)

/* S3C I2C Controller bits */
#define I2CSTAT_BSY	0x20	/* Busy bit */
#define I2CSTAT_NACK	0x01	/* Nack bit */
#define I2CCON_ACKGEN	0x80	/* Acknowledge generation */
#define I2CCON_IRPND	0x10	/* Interrupt pending bit */
#define I2C_MODE_MT	0xC0	/* Master Transmit Mode */
#define I2C_MODE_MR	0x80	/* Master Receive Mode */
#define I2C_START_STOP	0x20	/* START / STOP */
#define I2C_TXRX_ENA	0x10	/* I2C Tx/Rx enable */

#define I2C_TIMEOUT_MS 10		/* 10 ms */

#define	HSI2C_TIMEOUT_US 10000 /* 10 ms, finer granularity */


/* To support VCMA9 boards and other who dont define max_i2c_num */
#ifndef CONFIG_MAX_I2C_NUM
#define CONFIG_MAX_I2C_NUM 1
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * For SPL boot some boards need i2c before SDRAM is initialised so force
 * variables to live in SRAM
 */
#ifdef CONFIG_SYS_I2C
static struct s3c24x0_i2c_bus i2c_bus[CONFIG_MAX_I2C_NUM]
			__attribute__((section(".data")));
#endif

enum exynos_i2c_type {
	EXYNOS_I2C_STD,
	EXYNOS_I2C_HS,
};

#ifdef CONFIG_SYS_I2C
/**
 * Get a pointer to the given bus index
 *
 * @bus_idx: Bus index to look up
 * @return pointer to bus, or NULL if invalid or not available
 */
static struct s3c24x0_i2c_bus *get_bus(unsigned int bus_idx)
{
	if (bus_idx < ARRAY_SIZE(i2c_bus)) {
		struct s3c24x0_i2c_bus *bus;

		bus = &i2c_bus[bus_idx];
		if (bus->active)
			return bus;
	}

	debug("Undefined bus: %d\n", bus_idx);
	return NULL;
}
#endif

#if !(defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
static int GetI2CSDA(void)
{
	struct s3c24x0_gpio *gpio = s3c24x0_get_base_gpio();

#ifdef CONFIG_S3C2410
	return (readl(&gpio->gpedat) & 0x8000) >> 15;
#endif
#ifdef CONFIG_S3C2400
	return (readl(&gpio->pgdat) & 0x0020) >> 5;
#endif
}

static void SetI2CSCL(int x)
{
	struct s3c24x0_gpio *gpio = s3c24x0_get_base_gpio();

#ifdef CONFIG_S3C2410
	writel((readl(&gpio->gpedat) & ~0x4000) |
					(x & 1) << 14, &gpio->gpedat);
#endif
#ifdef CONFIG_S3C2400
	writel((readl(&gpio->pgdat) & ~0x0040) | (x & 1) << 6, &gpio->pgdat);
#endif
}
#endif

/*
 * Wait til the byte transfer is completed.
 *
 * @param i2c- pointer to the appropriate i2c register bank.
 * @return I2C_OK, if transmission was ACKED
 *         I2C_NACK, if transmission was NACKED
 *         I2C_NOK_TIMEOUT, if transaction did not complete in I2C_TIMEOUT_MS
 */

static int WaitForXfer(struct s3c24x0_i2c *i2c)
{
	ulong start_time = get_timer(0);

	do {
		if (readl(&i2c->iiccon) & I2CCON_IRPND)
			return (readl(&i2c->iicstat) & I2CSTAT_NACK) ?
				I2C_NACK : I2C_OK;
	} while (get_timer(start_time) < I2C_TIMEOUT_MS);

	return I2C_NOK_TOUT;
}

/*
 * Wait for transfer completion.
 *
 * This function reads the interrupt status register waiting for the INT_I2C
 * bit to be set, which indicates copletion of a transaction.
 *
 * @param i2c: pointer to the appropriate register bank
 *
 * @return: I2C_OK in case of successful completion, I2C_NOK_TIMEOUT in case
 *          the status bits do not get set in time, or an approrpiate error
 *          value in case of transfer errors.
 */
static int hsi2c_wait_for_trx(struct exynos5_hsi2c *i2c)
{
	int i = HSI2C_TIMEOUT_US;

	while (i-- > 0) {
		u32 int_status = readl(&i2c->usi_int_stat);

		if (int_status & HSI2C_INT_I2C_EN) {
			u32 trans_status = readl(&i2c->usi_trans_status);

			/* Deassert pending interrupt. */
			writel(int_status, &i2c->usi_int_stat);

			if (trans_status & HSI2C_NO_DEV_ACK) {
				debug("%s: no ACK from device\n", __func__);
				return I2C_NACK;
			}
			if (trans_status & HSI2C_NO_DEV) {
				debug("%s: no device\n", __func__);
				return I2C_NOK;
			}
			if (trans_status & HSI2C_TRANS_ABORT) {
				debug("%s: arbitration lost\n", __func__);
				return I2C_NOK_LA;
			}
			if (trans_status & HSI2C_TIMEOUT_AUTO) {
				debug("%s: device timed out\n", __func__);
				return I2C_NOK_TOUT;
			}
			return I2C_OK;
		}
		udelay(1);
	}
	debug("%s: transaction timeout!\n", __func__);
	return I2C_NOK_TOUT;
}

static void ReadWriteByte(struct s3c24x0_i2c *i2c)
{
	writel(readl(&i2c->iiccon) & ~I2CCON_IRPND, &i2c->iiccon);
}

#ifdef CONFIG_SYS_I2C
static struct s3c24x0_i2c *get_base_i2c(int bus)
{
#ifdef CONFIG_EXYNOS4
	struct s3c24x0_i2c *i2c = (struct s3c24x0_i2c *)(samsung_get_base_i2c()
							+ (EXYNOS4_I2C_SPACING
							* bus));
	return i2c;
#elif defined CONFIG_EXYNOS5
	struct s3c24x0_i2c *i2c = (struct s3c24x0_i2c *)(samsung_get_base_i2c()
							+ (EXYNOS5_I2C_SPACING
							* bus));
	return i2c;
#else
	return s3c24x0_get_base_i2c();
#endif
}
#endif

static void i2c_ch_init(struct s3c24x0_i2c *i2c, int speed, int slaveadd)
{
	ulong freq, pres = 16, div;
#if (defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
	freq = get_i2c_clk();
#else
	freq = get_PCLK();
#endif
	/* calculate prescaler and divisor values */
	if ((freq / pres / (16 + 1)) > speed)
		/* set prescaler to 512 */
		pres = 512;

	div = 0;
	while ((freq / pres / (div + 1)) > speed)
		div++;

	/* set prescaler, divisor according to freq, also set ACKGEN, IRQ */
	writel((div & 0x0F) | 0xA0 | ((pres == 512) ? 0x40 : 0), &i2c->iiccon);

	/* init to SLAVE REVEIVE and set slaveaddr */
	writel(0, &i2c->iicstat);
	writel(slaveadd, &i2c->iicadd);
	/* program Master Transmit (and implicit STOP) */
	writel(I2C_MODE_MT | I2C_TXRX_ENA, &i2c->iicstat);
}

static int hsi2c_get_clk_details(struct s3c24x0_i2c_bus *i2c_bus)
{
	struct exynos5_hsi2c *hsregs = i2c_bus->hsregs;
	ulong clkin;
	unsigned int op_clk = i2c_bus->clock_frequency;
	unsigned int i = 0, utemp0 = 0, utemp1 = 0;
	unsigned int t_ftl_cycle;

#if (defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
	clkin = get_i2c_clk();
#else
	clkin = get_PCLK();
#endif
	/* FPCLK / FI2C =
	 * (CLK_DIV + 1) * (TSCLK_L + TSCLK_H + 2) + 8 + 2 * FLT_CYCLE
	 * uTemp0 = (CLK_DIV + 1) * (TSCLK_L + TSCLK_H + 2)
	 * uTemp1 = (TSCLK_L + TSCLK_H + 2)
	 * uTemp2 = TSCLK_L + TSCLK_H
	 */
	t_ftl_cycle = (readl(&hsregs->usi_conf) >> 16) & 0x7;
	utemp0 = (clkin / op_clk) - 8 - 2 * t_ftl_cycle;

	/* CLK_DIV max is 256 */
	for (i = 0; i < 256; i++) {
		utemp1 = utemp0 / (i + 1);
		if ((utemp1 < 512) && (utemp1 > 4)) {
			i2c_bus->clk_cycle = utemp1 - 2;
			i2c_bus->clk_div = i;
			return 0;
		}
	}
	return -EINVAL;
}

static void hsi2c_ch_init(struct s3c24x0_i2c_bus *i2c_bus)
{
	struct exynos5_hsi2c *hsregs = i2c_bus->hsregs;
	unsigned int t_sr_release;
	unsigned int n_clkdiv;
	unsigned int t_start_su, t_start_hd;
	unsigned int t_stop_su;
	unsigned int t_data_su, t_data_hd;
	unsigned int t_scl_l, t_scl_h;
	u32 i2c_timing_s1;
	u32 i2c_timing_s2;
	u32 i2c_timing_s3;
	u32 i2c_timing_sla;

	n_clkdiv = i2c_bus->clk_div;
	t_scl_l = i2c_bus->clk_cycle / 2;
	t_scl_h = i2c_bus->clk_cycle / 2;
	t_start_su = t_scl_l;
	t_start_hd = t_scl_l;
	t_stop_su = t_scl_l;
	t_data_su = t_scl_l / 2;
	t_data_hd = t_scl_l / 2;
	t_sr_release = i2c_bus->clk_cycle;

	i2c_timing_s1 = t_start_su << 24 | t_start_hd << 16 | t_stop_su << 8;
	i2c_timing_s2 = t_data_su << 24 | t_scl_l << 8 | t_scl_h << 0;
	i2c_timing_s3 = n_clkdiv << 16 | t_sr_release << 0;
	i2c_timing_sla = t_data_hd << 0;

	writel(HSI2C_TRAILING_COUNT, &hsregs->usi_trailing_ctl);

	/* Clear to enable Timeout */
	clrsetbits_le32(&hsregs->usi_timeout, HSI2C_TIMEOUT_EN, 0);

	/* set AUTO mode */
	writel(readl(&hsregs->usi_conf) | HSI2C_AUTO_MODE, &hsregs->usi_conf);

	/* Enable completion conditions' reporting. */
	writel(HSI2C_INT_I2C_EN, &hsregs->usi_int_en);

	/* Enable FIFOs */
	writel(HSI2C_RXFIFO_EN | HSI2C_TXFIFO_EN, &hsregs->usi_fifo_ctl);

	/* Currently operating in Fast speed mode. */
	writel(i2c_timing_s1, &hsregs->usi_timing_fs1);
	writel(i2c_timing_s2, &hsregs->usi_timing_fs2);
	writel(i2c_timing_s3, &hsregs->usi_timing_fs3);
	writel(i2c_timing_sla, &hsregs->usi_timing_sla);
}

/* SW reset for the high speed bus */
static void exynos5_i2c_reset(struct s3c24x0_i2c_bus *i2c_bus)
{
	struct exynos5_hsi2c *i2c = i2c_bus->hsregs;
	u32 i2c_ctl;

	/* Set and clear the bit for reset */
	i2c_ctl = readl(&i2c->usi_ctl);
	i2c_ctl |= HSI2C_SW_RST;
	writel(i2c_ctl, &i2c->usi_ctl);

	i2c_ctl = readl(&i2c->usi_ctl);
	i2c_ctl &= ~HSI2C_SW_RST;
	writel(i2c_ctl, &i2c->usi_ctl);

	/* Initialize the configure registers */
	hsi2c_ch_init(i2c_bus);
}

#ifdef CONFIG_SYS_I2C
static void s3c24x0_i2c_init(struct i2c_adapter *adap, int speed, int slaveadd)
{
	struct s3c24x0_i2c *i2c;
	struct s3c24x0_i2c_bus *bus;
#if !(defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
	struct s3c24x0_gpio *gpio = s3c24x0_get_base_gpio();
#endif
	ulong start_time = get_timer(0);

	i2c = get_base_i2c(adap->hwadapnr);
	bus = &i2c_bus[adap->hwadapnr];
	if (!bus)
		return;

	/*
	 * In case the previous transfer is still going, wait to give it a
	 * chance to finish.
	 */
	while (readl(&i2c->iicstat) & I2CSTAT_BSY) {
		if (get_timer(start_time) > I2C_TIMEOUT_MS) {
			printf("%s: I2C bus busy for %p\n", __func__,
			       &i2c->iicstat);
			return;
		}
	}

#if !(defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
	int i;

	if ((readl(&i2c->iicstat) & I2CSTAT_BSY) || GetI2CSDA() == 0) {
#ifdef CONFIG_S3C2410
		ulong old_gpecon = readl(&gpio->gpecon);
#endif
#ifdef CONFIG_S3C2400
		ulong old_gpecon = readl(&gpio->pgcon);
#endif
		/* bus still busy probably by (most) previously interrupted
		   transfer */

#ifdef CONFIG_S3C2410
		/* set I2CSDA and I2CSCL (GPE15, GPE14) to GPIO */
		writel((readl(&gpio->gpecon) & ~0xF0000000) | 0x10000000,
		       &gpio->gpecon);
#endif
#ifdef CONFIG_S3C2400
		/* set I2CSDA and I2CSCL (PG5, PG6) to GPIO */
		writel((readl(&gpio->pgcon) & ~0x00003c00) | 0x00001000,
		       &gpio->pgcon);
#endif

		/* toggle I2CSCL until bus idle */
		SetI2CSCL(0);
		udelay(1000);
		i = 10;
		while ((i > 0) && (GetI2CSDA() != 1)) {
			SetI2CSCL(1);
			udelay(1000);
			SetI2CSCL(0);
			udelay(1000);
			i--;
		}
		SetI2CSCL(1);
		udelay(1000);

		/* restore pin functions */
#ifdef CONFIG_S3C2410
		writel(old_gpecon, &gpio->gpecon);
#endif
#ifdef CONFIG_S3C2400
		writel(old_gpecon, &gpio->pgcon);
#endif
	}
#endif /* #if !(defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5) */

	i2c_ch_init(i2c, speed, slaveadd);

	bus->active = true;
	bus->regs = i2c;
}
#endif /* CONFIG_SYS_I2C */

/*
 * Poll the appropriate bit of the fifo status register until the interface is
 * ready to process the next byte or timeout expires.
 *
 * In addition to the FIFO status register this function also polls the
 * interrupt status register to be able to detect unexpected transaction
 * completion.
 *
 * When FIFO is ready to process the next byte, this function returns I2C_OK.
 * If in course of polling the INT_I2C assertion is detected, the function
 * returns I2C_NOK. If timeout happens before any of the above conditions is
 * met - the function returns I2C_NOK_TOUT;

 * @param i2c: pointer to the appropriate i2c register bank.
 * @param rx_transfer: set to True if the receive transaction is in progress.
 * @return: as described above.
 */
static unsigned hsi2c_poll_fifo(struct exynos5_hsi2c *i2c, bool rx_transfer)
{
	u32 fifo_bit = rx_transfer ? HSI2C_RX_FIFO_EMPTY : HSI2C_TX_FIFO_FULL;
	int i = HSI2C_TIMEOUT_US;

	while (readl(&i2c->usi_fifo_stat) & fifo_bit) {
		if (readl(&i2c->usi_int_stat) & HSI2C_INT_I2C_EN) {
			/*
			 * There is a chance that assertion of
			 * HSI2C_INT_I2C_EN and deassertion of
			 * HSI2C_RX_FIFO_EMPTY happen simultaneously. Let's
			 * give FIFO status priority and check it one more
			 * time before reporting interrupt. The interrupt will
			 * be reported next time this function is called.
			 */
			if (rx_transfer &&
			    !(readl(&i2c->usi_fifo_stat) & fifo_bit))
				break;
			return I2C_NOK;
		}
		if (!i--) {
			debug("%s: FIFO polling timeout!\n", __func__);
			return I2C_NOK_TOUT;
		}
		udelay(1);
	}
	return I2C_OK;
}

/*
 * Preapre hsi2c transaction, either read or write.
 *
 * Set up transfer as described in section 27.5.1.2 'I2C Channel Auto Mode' of
 * the 5420 UM.
 *
 * @param i2c: pointer to the appropriate i2c register bank.
 * @param chip: slave address on the i2c bus (with read/write bit exlcuded)
 * @param len: number of bytes expected to be sent or received
 * @param rx_transfer: set to true for receive transactions
 * @param: issue_stop: set to true if i2c stop condition should be generated
 *         after this transaction.
 * @return: I2C_NOK_TOUT in case the bus remained busy for HSI2C_TIMEOUT_US,
 *          I2C_OK otherwise.
 */
static int hsi2c_prepare_transaction(struct exynos5_hsi2c *i2c,
				     u8 chip,
				     u16 len,
				     bool rx_transfer,
				     bool issue_stop)
{
	u32 conf;

	conf = len | HSI2C_MASTER_RUN;

	if (issue_stop)
		conf |= HSI2C_STOP_AFTER_TRANS;

	/* Clear to enable Timeout */
	writel(readl(&i2c->usi_timeout) & ~HSI2C_TIMEOUT_EN, &i2c->usi_timeout);

	/* Set slave address */
	writel(HSI2C_SLV_ADDR_MAS(chip), &i2c->i2c_addr);

	if (rx_transfer) {
		/* i2c master, read transaction */
		writel((HSI2C_RXCHON | HSI2C_FUNC_MODE_I2C | HSI2C_MASTER),
		       &i2c->usi_ctl);

		/* read up to len bytes, stop after transaction is finished */
		writel(conf | HSI2C_READ_WRITE, &i2c->usi_auto_conf);
	} else {
		/* i2c master, write transaction */
		writel((HSI2C_TXCHON | HSI2C_FUNC_MODE_I2C | HSI2C_MASTER),
		       &i2c->usi_ctl);

		/* write up to len bytes, stop after transaction is finished */
		writel(conf, &i2c->usi_auto_conf);
	}

	/* Reset all pending interrupt status bits we care about, if any */
	writel(HSI2C_INT_I2C_EN, &i2c->usi_int_stat);

	return I2C_OK;
}

/*
 * Wait while i2c bus is settling down (mostly stop gets completed).
 */
static int hsi2c_wait_while_busy(struct exynos5_hsi2c *i2c)
{
	int i = HSI2C_TIMEOUT_US;

	while (readl(&i2c->usi_trans_status) & HSI2C_MASTER_BUSY) {
		if (!i--) {
			debug("%s: bus busy\n", __func__);
			return I2C_NOK_TOUT;
		}
		udelay(1);
	}
	return I2C_OK;
}

static int hsi2c_write(struct exynos5_hsi2c *i2c,
		       unsigned char chip,
		       unsigned char addr[],
		       unsigned char alen,
		       unsigned char data[],
		       unsigned short len,
		       bool issue_stop)
{
	int i, rv = 0;

	if (!(len + alen)) {
		/* Writes of zero length not supported in auto mode. */
		debug("%s: zero length writes not supported\n", __func__);
		return I2C_NOK;
	}

	rv = hsi2c_prepare_transaction
		(i2c, chip, len + alen, false, issue_stop);
	if (rv != I2C_OK)
		return rv;

	/* Move address, if any, and the data, if any, into the FIFO. */
	for (i = 0; i < alen; i++) {
		rv = hsi2c_poll_fifo(i2c, false);
		if (rv != I2C_OK) {
			debug("%s: address write failed\n", __func__);
			goto write_error;
		}
		writel(addr[i], &i2c->usi_txdata);
	}

	for (i = 0; i < len; i++) {
		rv = hsi2c_poll_fifo(i2c, false);
		if (rv != I2C_OK) {
			debug("%s: data write failed\n", __func__);
			goto write_error;
		}
		writel(data[i], &i2c->usi_txdata);
	}

	rv = hsi2c_wait_for_trx(i2c);

 write_error:
	if (issue_stop) {
		int tmp_ret = hsi2c_wait_while_busy(i2c);
		if (rv == I2C_OK)
			rv = tmp_ret;
	}

	writel(HSI2C_FUNC_MODE_I2C, &i2c->usi_ctl); /* done */
	return rv;
}

static int hsi2c_read(struct exynos5_hsi2c *i2c,
		      unsigned char chip,
		      unsigned char addr[],
		      unsigned char alen,
		      unsigned char data[],
		      unsigned short len)
{
	int i, rv, tmp_ret;
	bool drop_data = false;

	if (!len) {
		/* Reads of zero length not supported in auto mode. */
		debug("%s: zero length read adjusted\n", __func__);
		drop_data = true;
		len = 1;
	}

	if (alen) {
		/* Internal register adress needs to be written first. */
		rv = hsi2c_write(i2c, chip, addr, alen, NULL, 0, false);
		if (rv != I2C_OK)
			return rv;
	}

	rv = hsi2c_prepare_transaction(i2c, chip, len, true, true);

	if (rv != I2C_OK)
		return rv;

	for (i = 0; i < len; i++) {
		rv = hsi2c_poll_fifo(i2c, true);
		if (rv != I2C_OK)
			goto read_err;
		if (drop_data)
			continue;
		data[i] = readl(&i2c->usi_rxdata);
	}

	rv = hsi2c_wait_for_trx(i2c);

 read_err:
	tmp_ret = hsi2c_wait_while_busy(i2c);
	if (rv == I2C_OK)
		rv = tmp_ret;

	writel(HSI2C_FUNC_MODE_I2C, &i2c->usi_ctl); /* done */
	return rv;
}

#ifdef CONFIG_SYS_I2C
static unsigned int s3c24x0_i2c_set_bus_speed(struct i2c_adapter *adap,
					      unsigned int speed)
#else
static int s3c24x0_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
#endif
{
	struct s3c24x0_i2c_bus *i2c_bus;

#ifdef CONFIG_SYS_I2C
	i2c_bus = get_bus(adap->hwadapnr);
	if (!i2c_bus)
		return -EFAULT;
#else
	i2c_bus = dev_get_priv(dev);
#endif
	i2c_bus->clock_frequency = speed;

	if (i2c_bus->is_highspeed) {
		if (hsi2c_get_clk_details(i2c_bus))
			return -EFAULT;
		hsi2c_ch_init(i2c_bus);
	} else {
		i2c_ch_init(i2c_bus->regs, i2c_bus->clock_frequency,
			    CONFIG_SYS_I2C_S3C24X0_SLAVE);
	}

	return 0;
}

/*
 * cmd_type is 0 for write, 1 for read.
 *
 * addr_len can take any value from 0-255, it is only limited
 * by the char, we could make it larger if needed. If it is
 * 0 we skip the address write cycle.
 */
static int i2c_transfer(struct s3c24x0_i2c *i2c,
			unsigned char cmd_type,
			unsigned char chip,
			unsigned char addr[],
			unsigned char addr_len,
			unsigned char data[],
			unsigned short data_len)
{
	int i = 0, result;
	ulong start_time = get_timer(0);

	if (data == 0 || data_len == 0) {
		/*Don't support data transfer of no length or to address 0 */
		debug("i2c_transfer: bad call\n");
		return I2C_NOK;
	}

	while (readl(&i2c->iicstat) & I2CSTAT_BSY) {
		if (get_timer(start_time) > I2C_TIMEOUT_MS)
			return I2C_NOK_TOUT;
	}

	writel(readl(&i2c->iiccon) | I2CCON_ACKGEN, &i2c->iiccon);

	/* Get the slave chip address going */
	writel(chip, &i2c->iicds);
	if ((cmd_type == I2C_WRITE) || (addr && addr_len))
		writel(I2C_MODE_MT | I2C_TXRX_ENA | I2C_START_STOP,
		       &i2c->iicstat);
	else
		writel(I2C_MODE_MR | I2C_TXRX_ENA | I2C_START_STOP,
		       &i2c->iicstat);

	/* Wait for chip address to transmit. */
	result = WaitForXfer(i2c);
	if (result != I2C_OK)
		goto bailout;

	/* If register address needs to be transmitted - do it now. */
	if (addr && addr_len) {
		while ((i < addr_len) && (result == I2C_OK)) {
			writel(addr[i++], &i2c->iicds);
			ReadWriteByte(i2c);
			result = WaitForXfer(i2c);
		}
		i = 0;
		if (result != I2C_OK)
			goto bailout;
	}

	switch (cmd_type) {
	case I2C_WRITE:
		while ((i < data_len) && (result == I2C_OK)) {
			writel(data[i++], &i2c->iicds);
			ReadWriteByte(i2c);
			result = WaitForXfer(i2c);
		}
		break;

	case I2C_READ:
		if (addr && addr_len) {
			/*
			 * Register address has been sent, now send slave chip
			 * address again to start the actual read transaction.
			 */
			writel(chip, &i2c->iicds);

			/* Generate a re-START. */
			writel(I2C_MODE_MR | I2C_TXRX_ENA | I2C_START_STOP,
				&i2c->iicstat);
			ReadWriteByte(i2c);
			result = WaitForXfer(i2c);

			if (result != I2C_OK)
				goto bailout;
		}

		while ((i < data_len) && (result == I2C_OK)) {
			/* disable ACK for final READ */
			if (i == data_len - 1)
				writel(readl(&i2c->iiccon)
				       & ~I2CCON_ACKGEN,
				       &i2c->iiccon);
			ReadWriteByte(i2c);
			result = WaitForXfer(i2c);
			data[i++] = readl(&i2c->iicds);
		}
		if (result == I2C_NACK)
			result = I2C_OK; /* Normal terminated read. */
		break;

	default:
		debug("i2c_transfer: bad call\n");
		result = I2C_NOK;
		break;
	}

bailout:
	/* Send STOP. */
	writel(I2C_MODE_MR | I2C_TXRX_ENA, &i2c->iicstat);
	ReadWriteByte(i2c);

	return result;
}

#ifdef CONFIG_SYS_I2C
static int s3c24x0_i2c_probe(struct i2c_adapter *adap, uchar chip)
#else
static int s3c24x0_i2c_probe(struct udevice *dev, uint chip, uint chip_flags)
#endif
{
	struct s3c24x0_i2c_bus *i2c_bus;
	uchar buf[1];
	int ret;

#ifdef CONFIG_SYS_I2C
	i2c_bus = get_bus(adap->hwadapnr);
	if (!i2c_bus)
		return -EFAULT;
#else
	i2c_bus = dev_get_priv(dev);
#endif
	buf[0] = 0;

	/*
	 * What is needed is to send the chip address and verify that the
	 * address was <ACK>ed (i.e. there was a chip at that address which
	 * drove the data line low).
	 */
	if (i2c_bus->is_highspeed) {
		ret = hsi2c_read(i2c_bus->hsregs,
				chip, 0, 0, buf, 1);
	} else {
		ret = i2c_transfer(i2c_bus->regs,
				I2C_READ, chip << 1, 0, 0, buf, 1);
	}

	return ret != I2C_OK;
}

#ifdef CONFIG_SYS_I2C
static int s3c24x0_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
			    int alen, uchar *buffer, int len)
{
	struct s3c24x0_i2c_bus *i2c_bus;
	uchar xaddr[4];
	int ret;

	i2c_bus = get_bus(adap->hwadapnr);
	if (!i2c_bus)
		return -EFAULT;

	if (alen > 4) {
		debug("I2C read: addr len %d not supported\n", alen);
		return -EADDRNOTAVAIL;
	}

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;
	}

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	if (alen > 0)
		chip |= ((addr >> (alen * 8)) &
			 CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif
	if (i2c_bus->is_highspeed)
		ret = hsi2c_read(i2c_bus->hsregs, chip, &xaddr[4 - alen],
				 alen, buffer, len);
	else
		ret = i2c_transfer(i2c_bus->regs, I2C_READ, chip << 1,
				&xaddr[4 - alen], alen, buffer, len);

	if (ret) {
		if (i2c_bus->is_highspeed)
			exynos5_i2c_reset(i2c_bus);
		debug("I2c read failed %d\n", ret);
		return -EIO;
	}
	return 0;
}

static int s3c24x0_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
			 int alen, uchar *buffer, int len)
{
	struct s3c24x0_i2c_bus *i2c_bus;
	uchar xaddr[4];
	int ret;

	i2c_bus = get_bus(adap->hwadapnr);
	if (!i2c_bus)
		return -EFAULT;

	if (alen > 4) {
		debug("I2C write: addr len %d not supported\n", alen);
		return -EINVAL;
	}

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;
	}
#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	if (alen > 0)
		chip |= ((addr >> (alen * 8)) &
			 CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif
	if (i2c_bus->is_highspeed)
		ret = hsi2c_write(i2c_bus->hsregs, chip, &xaddr[4 - alen],
				  alen, buffer, len, true);
	else
		ret = i2c_transfer(i2c_bus->regs, I2C_WRITE, chip << 1,
				&xaddr[4 - alen], alen, buffer, len);

	if (ret != 0) {
		if (i2c_bus->is_highspeed)
			exynos5_i2c_reset(i2c_bus);
		return 1;
	} else {
		return 0;
	}
}

#ifdef CONFIG_OF_CONTROL
static void process_nodes(const void *blob, int node_list[], int count,
			 int is_highspeed)
{
	struct s3c24x0_i2c_bus *bus;
	int i, flags;

	for (i = 0; i < count; i++) {
		int node = node_list[i];

		if (node <= 0)
			continue;

		bus = &i2c_bus[i];
		bus->active = true;
		bus->is_highspeed = is_highspeed;

		if (is_highspeed) {
			flags = PINMUX_FLAG_HS_MODE;
			bus->hsregs = (struct exynos5_hsi2c *)
					fdtdec_get_addr(blob, node, "reg");
		} else {
			flags = 0;
			bus->regs = (struct s3c24x0_i2c *)
					fdtdec_get_addr(blob, node, "reg");
		}

		bus->id = pinmux_decode_periph_id(blob, node);
		bus->clock_frequency = fdtdec_get_int(blob, node,
						"clock-frequency",
						CONFIG_SYS_I2C_S3C24X0_SPEED);
		bus->node = node;
		bus->bus_num = i;
		exynos_pinmux_config(PERIPH_ID_I2C0 + bus->id, flags);

		/* Mark position as used */
		node_list[i] = -1;
	}
}

void board_i2c_init(const void *blob)
{
	int node_list[CONFIG_MAX_I2C_NUM];
	int count;

	/* First get the normal i2c ports */
	count = fdtdec_find_aliases_for_id(blob, "i2c",
		COMPAT_SAMSUNG_S3C2440_I2C, node_list,
		CONFIG_MAX_I2C_NUM);
	process_nodes(blob, node_list, count, 0);

	/* Now look for high speed i2c ports */
	count = fdtdec_find_aliases_for_id(blob, "i2c",
		COMPAT_SAMSUNG_EXYNOS5_I2C, node_list,
		CONFIG_MAX_I2C_NUM);
	process_nodes(blob, node_list, count, 1);
}

int i2c_get_bus_num_fdt(int node)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(i2c_bus); i++) {
		if (node == i2c_bus[i].node)
			return i;
	}

	debug("%s: Can't find any matched I2C bus\n", __func__);
	return -EINVAL;
}

int i2c_reset_port_fdt(const void *blob, int node)
{
	struct s3c24x0_i2c_bus *i2c_bus;
	int bus;

	bus = i2c_get_bus_num_fdt(node);
	if (bus < 0) {
		debug("could not get bus for node %d\n", node);
		return bus;
	}

	i2c_bus = get_bus(bus);
	if (!i2c_bus) {
		debug("get_bus() failed for node %d\n", node);
		return -EFAULT;
	}

	if (i2c_bus->is_highspeed) {
		if (hsi2c_get_clk_details(i2c_bus))
			return -EINVAL;
		hsi2c_ch_init(i2c_bus);
	} else {
		i2c_ch_init(i2c_bus->regs, i2c_bus->clock_frequency,
			    CONFIG_SYS_I2C_S3C24X0_SLAVE);
	}

	return 0;
}
#endif /* CONFIG_OF_CONTROL */

#ifdef CONFIG_EXYNOS5
static void exynos_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
	/* This will override the speed selected in the fdt for that port */
	debug("i2c_init(speed=%u, slaveaddr=0x%x)\n", speed, slaveaddr);
	if (i2c_set_bus_speed(speed))
		error("i2c_init: failed to init bus for speed = %d", speed);
}
#endif /* CONFIG_EXYNOS5 */

/*
 * Register s3c24x0 i2c adapters
 */
#if defined(CONFIG_EXYNOS5420)
U_BOOT_I2C_ADAP_COMPLETE(i2c00, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 0)
U_BOOT_I2C_ADAP_COMPLETE(i2c01, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 1)
U_BOOT_I2C_ADAP_COMPLETE(i2c02, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 2)
U_BOOT_I2C_ADAP_COMPLETE(i2c03, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 3)
U_BOOT_I2C_ADAP_COMPLETE(i2c04, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 4)
U_BOOT_I2C_ADAP_COMPLETE(i2c05, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 5)
U_BOOT_I2C_ADAP_COMPLETE(i2c06, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 6)
U_BOOT_I2C_ADAP_COMPLETE(i2c07, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 7)
U_BOOT_I2C_ADAP_COMPLETE(i2c08, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 8)
U_BOOT_I2C_ADAP_COMPLETE(i2c09, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 9)
U_BOOT_I2C_ADAP_COMPLETE(i2c10, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 10)
#elif defined(CONFIG_EXYNOS5250)
U_BOOT_I2C_ADAP_COMPLETE(i2c00, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 0)
U_BOOT_I2C_ADAP_COMPLETE(i2c01, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 1)
U_BOOT_I2C_ADAP_COMPLETE(i2c02, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 2)
U_BOOT_I2C_ADAP_COMPLETE(i2c03, exynos_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 3)
U_BOOT_I2C_ADAP_COMPLETE(i2c04, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 4)
U_BOOT_I2C_ADAP_COMPLETE(i2c05, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 5)
U_BOOT_I2C_ADAP_COMPLETE(i2c06, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 6)
U_BOOT_I2C_ADAP_COMPLETE(i2c07, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 7)
U_BOOT_I2C_ADAP_COMPLETE(i2c08, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 8)
U_BOOT_I2C_ADAP_COMPLETE(i2c09, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 9)
U_BOOT_I2C_ADAP_COMPLETE(s3c10, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 10)
#elif defined(CONFIG_EXYNOS4)
U_BOOT_I2C_ADAP_COMPLETE(i2c00, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 0)
U_BOOT_I2C_ADAP_COMPLETE(i2c01, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 1)
U_BOOT_I2C_ADAP_COMPLETE(i2c02, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 2)
U_BOOT_I2C_ADAP_COMPLETE(i2c03, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 3)
U_BOOT_I2C_ADAP_COMPLETE(i2c04, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 4)
U_BOOT_I2C_ADAP_COMPLETE(i2c05, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 5)
U_BOOT_I2C_ADAP_COMPLETE(i2c06, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 6)
U_BOOT_I2C_ADAP_COMPLETE(i2c07, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 7)
U_BOOT_I2C_ADAP_COMPLETE(i2c08, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 8)
#else
U_BOOT_I2C_ADAP_COMPLETE(s3c0, s3c24x0_i2c_init, s3c24x0_i2c_probe,
			s3c24x0_i2c_read, s3c24x0_i2c_write,
			s3c24x0_i2c_set_bus_speed,
			CONFIG_SYS_I2C_S3C24X0_SPEED,
			CONFIG_SYS_I2C_S3C24X0_SLAVE, 0)
#endif
#endif /* CONFIG_SYS_I2C */

#ifdef CONFIG_DM_I2C
static int i2c_write_data(struct s3c24x0_i2c_bus *i2c_bus, uchar chip,
			  uchar *buffer, int len, bool end_with_repeated_start)
{
	int ret;

	if (i2c_bus->is_highspeed) {
		ret = hsi2c_write(i2c_bus->hsregs, chip, 0, 0,
				  buffer, len, true);
		if (ret)
			exynos5_i2c_reset(i2c_bus);
	} else {
		ret = i2c_transfer(i2c_bus->regs, I2C_WRITE,
				   chip << 1, 0, 0, buffer, len);
	}

	return ret != I2C_OK;
}

static int i2c_read_data(struct s3c24x0_i2c_bus  *i2c_bus, uchar chip,
			 uchar *buffer, int len)
{
	int ret;

	if (i2c_bus->is_highspeed) {
		ret = hsi2c_read(i2c_bus->hsregs, chip, 0, 0, buffer, len);
		if (ret)
			exynos5_i2c_reset(i2c_bus);
	} else {
		ret = i2c_transfer(i2c_bus->regs, I2C_READ,
				   chip << 1, 0, 0, buffer, len);
	}

	return ret != I2C_OK;
}

static int s3c24x0_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
			    int nmsgs)
{
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	int ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		bool next_is_read = nmsgs > 1 && (msg[1].flags & I2C_M_RD);

		if (msg->flags & I2C_M_RD) {
			ret = i2c_read_data(i2c_bus, msg->addr, msg->buf,
					    msg->len);
		} else {
			ret = i2c_write_data(i2c_bus, msg->addr, msg->buf,
					     msg->len, next_is_read);
		}
		if (ret)
			return -EREMOTEIO;
	}

	return 0;
}

static int s3c_i2c_ofdata_to_platdata(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	int node, flags;

	i2c_bus->is_highspeed = dev->of_id->data;
	node = dev->of_offset;

	if (i2c_bus->is_highspeed) {
		flags = PINMUX_FLAG_HS_MODE;
		i2c_bus->hsregs = (struct exynos5_hsi2c *)
				fdtdec_get_addr(blob, node, "reg");
	} else {
		flags = 0;
		i2c_bus->regs = (struct s3c24x0_i2c *)
				fdtdec_get_addr(blob, node, "reg");
	}

	i2c_bus->id = pinmux_decode_periph_id(blob, node);

	i2c_bus->clock_frequency = fdtdec_get_int(blob, node,
						"clock-frequency",
						CONFIG_SYS_I2C_S3C24X0_SPEED);
	i2c_bus->node = node;
	i2c_bus->bus_num = dev->seq;

	exynos_pinmux_config(i2c_bus->id, flags);

	i2c_bus->active = true;

	return 0;
}

static const struct dm_i2c_ops s3c_i2c_ops = {
	.xfer		= s3c24x0_i2c_xfer,
	.probe_chip	= s3c24x0_i2c_probe,
	.set_bus_speed	= s3c24x0_i2c_set_bus_speed,
};

static const struct udevice_id s3c_i2c_ids[] = {
	{ .compatible = "samsung,s3c2440-i2c", .data = EXYNOS_I2C_STD },
	{ .compatible = "samsung,exynos5-hsi2c", .data = EXYNOS_I2C_HS },
	{ }
};

U_BOOT_DRIVER(i2c_s3c) = {
	.name	= "i2c_s3c",
	.id	= UCLASS_I2C,
	.of_match = s3c_i2c_ids,
	.ofdata_to_platdata = s3c_i2c_ofdata_to_platdata,
	.per_child_auto_alloc_size = sizeof(struct dm_i2c_chip),
	.priv_auto_alloc_size = sizeof(struct s3c24x0_i2c_bus),
	.ops	= &s3c_i2c_ops,
};
#endif /* CONFIG_DM_I2C */
