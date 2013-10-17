/*
 * Faraday I2C Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <i2c.h>

#include "fti2c010.h"

#ifndef CONFIG_HARD_I2C
#error "fti2c010: CONFIG_HARD_I2C is not defined"
#endif

#ifndef CONFIG_SYS_I2C_SPEED
#define CONFIG_SYS_I2C_SPEED    50000
#endif

#ifndef CONFIG_FTI2C010_FREQ
#define CONFIG_FTI2C010_FREQ    clk_get_rate("I2C")
#endif

/* command timeout */
#define CFG_CMD_TIMEOUT         10 /* ms */

/* 7-bit chip address + 1-bit read/write */
#define I2C_RD(chip)            ((((chip) << 1) & 0xff) | 1)
#define I2C_WR(chip)            (((chip) << 1) & 0xff)

struct fti2c010_chip {
	void __iomem *regs;
	uint bus;
	uint speed;
};

static struct fti2c010_chip chip_list[] = {
	{
		.bus  = 0,
		.regs = (void __iomem *)CONFIG_FTI2C010_BASE,
	},
#ifdef CONFIG_I2C_MULTI_BUS
# ifdef CONFIG_FTI2C010_BASE1
	{
		.bus  = 1,
		.regs = (void __iomem *)CONFIG_FTI2C010_BASE1,
	},
# endif
# ifdef CONFIG_FTI2C010_BASE2
	{
		.bus  = 2,
		.regs = (void __iomem *)CONFIG_FTI2C010_BASE2,
	},
# endif
# ifdef CONFIG_FTI2C010_BASE3
	{
		.bus  = 3,
		.regs = (void __iomem *)CONFIG_FTI2C010_BASE3,
	},
# endif
#endif  /* #ifdef CONFIG_I2C_MULTI_BUS */
};

static struct fti2c010_chip *curr = chip_list;

static int fti2c010_wait(uint32_t mask)
{
	int ret = -1;
	uint32_t stat, ts;
	struct fti2c010_regs *regs = curr->regs;

	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		stat = readl(&regs->sr);
		if ((stat & mask) == mask) {
			ret = 0;
			break;
		}
	}

	return ret;
}

/*
 * u-boot I2C API
 */

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
void i2c_init(int speed, int slaveaddr)
{
	if (speed || !curr->speed)
		i2c_set_bus_speed(speed);

	/* if slave mode disabled */
	if (!slaveaddr)
		return;

	/*
	 * TODO:
	 * Implement slave mode, but is it really necessary?
	 */
}

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uchar chip)
{
	int ret;
	struct fti2c010_regs *regs = curr->regs;

	i2c_init(0, 0);

	/* 1. Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_WR(chip), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(SR_DT);
	if (ret)
		return ret;

	/* 2. Select device register */
	writel(0, &regs->dr);
	writel(CR_ENABLE | CR_TBEN, &regs->cr);
	ret = fti2c010_wait(SR_DT);

	return ret;
}

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int ret, pos;
	uchar paddr[4];
	struct fti2c010_regs *regs = curr->regs;

	i2c_init(0, 0);

	paddr[0] = (addr >> 0)  & 0xFF;
	paddr[1] = (addr >> 8)  & 0xFF;
	paddr[2] = (addr >> 16) & 0xFF;
	paddr[3] = (addr >> 24) & 0xFF;

	/*
	 * Phase A. Set register address
	 */

	/* A.1 Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_WR(chip), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(SR_DT);
	if (ret)
		return ret;

	/* A.2 Select device register */
	for (pos = 0; pos < alen; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		writel(paddr[pos], &regs->dr);
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(SR_DT);
		if (ret)
			return ret;
	}

	/*
	 * Phase B. Get register data
	 */

	/* B.1 Select slave device (7bits Address + 1bit R/W) */
	writel(I2C_RD(chip), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(SR_DT);
	if (ret)
		return ret;

	/* B.2 Get register data */
	for (pos = 0; pos < len; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;
		uint32_t stat = SR_DR;

		if (pos == len - 1) {
			ctrl |= CR_NAK | CR_STOP;
			stat |= SR_ACK;
		}
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(stat);
		if (ret)
			break;
		buf[pos] = (uchar)(readl(&regs->dr) & 0xFF);
	}

	return ret;
}

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int ret, pos;
	uchar paddr[4];
	struct fti2c010_regs *regs = curr->regs;

	i2c_init(0, 0);

	paddr[0] = (addr >> 0)  & 0xFF;
	paddr[1] = (addr >> 8)  & 0xFF;
	paddr[2] = (addr >> 16) & 0xFF;
	paddr[3] = (addr >> 24) & 0xFF;

	/*
	 * Phase A. Set register address
	 *
	 * A.1 Select slave device (7bits Address + 1bit R/W)
	 */
	writel(I2C_WR(chip), &regs->dr);
	writel(CR_ENABLE | CR_TBEN | CR_START, &regs->cr);
	ret = fti2c010_wait(SR_DT);
	if (ret)
		return ret;

	/* A.2 Select device register */
	for (pos = 0; pos < alen; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		writel(paddr[pos], &regs->dr);
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(SR_DT);
		if (ret)
			return ret;
	}

	/*
	 * Phase B. Set register data
	 */
	for (pos = 0; pos < len; ++pos) {
		uint32_t ctrl = CR_ENABLE | CR_TBEN;

		if (pos == len - 1)
			ctrl |= CR_STOP;
		writel(buf[pos], &regs->dr);
		writel(ctrl, &regs->cr);
		ret = fti2c010_wait(SR_DT);
		if (ret)
			break;
	}

	return ret;
}

/*
 * Functions for setting the current I2C bus and its speed
 */
#ifdef CONFIG_I2C_MULTI_BUS

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 *    bus - bus index, zero based
 *
 *    Returns: 0 on success, not 0 on failure
 */
int i2c_set_bus_num(uint bus)
{
	if (bus >= ARRAY_SIZE(chip_list))
		return -1;
	curr = chip_list + bus;
	i2c_init(0, 0);
	return 0;
}

/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */

uint i2c_get_bus_num(void)
{
	return curr->bus;
}

#endif    /* #ifdef CONFIG_I2C_MULTI_BUS */

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 *    speed - bus speed in Hz
 *
 *    Returns: 0 on success, not 0 on failure
 */
int i2c_set_bus_speed(uint speed)
{
	struct fti2c010_regs *regs = curr->regs;
	uint clk = CONFIG_FTI2C010_FREQ;
	uint gsr = 0, tsr = 32;
	uint spd, div;

	if (!speed)
		speed = CONFIG_SYS_I2C_SPEED;

	for (div = 0; div < 0x3ffff; ++div) {
		/* SCLout = PCLK/(2*(COUNT + 2) + GSR) */
		spd = clk / (2 * (div + 2) + gsr);
		if (spd <= speed)
			break;
	}

	if (curr->speed == spd)
		return 0;

	writel(CR_I2CRST, &regs->cr);
	mdelay(100);
	if (readl(&regs->cr) & CR_I2CRST) {
		printf("fti2c010: reset timeout\n");
		return -1;
	}

	curr->speed = spd;

	writel(TGSR_GSR(gsr) | TGSR_TSR(tsr), &regs->tgsr);
	writel(CDR_DIV(div), &regs->cdr);

	return 0;
}

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

uint i2c_get_bus_speed(void)
{
	return curr->speed;
}
