/*
 * TI QSPI driver
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/omap.h>
#include <malloc.h>
#include <spi.h>
#include <asm/gpio.h>
#include <asm/omap_gpio.h>

/* ti qpsi register bit masks */
#define QSPI_TIMEOUT                    2000000
#define QSPI_FCLK                       192000000
/* clock control */
#define QSPI_CLK_EN                     (1 << 31)
#define QSPI_CLK_DIV_MAX                0xffff
/* command */
#define QSPI_EN_CS(n)                   (n << 28)
#define QSPI_WLEN(n)                    ((n-1) << 19)
#define QSPI_3_PIN                      (1 << 18)
#define QSPI_RD_SNGL                    (1 << 16)
#define QSPI_WR_SNGL                    (2 << 16)
#define QSPI_INVAL                      (4 << 16)
#define QSPI_RD_QUAD                    (7 << 16)
/* device control */
#define QSPI_DD(m, n)                   (m << (3 + n*8))
#define QSPI_CKPHA(n)                   (1 << (2 + n*8))
#define QSPI_CSPOL(n)                   (1 << (1 + n*8))
#define QSPI_CKPOL(n)                   (1 << (n*8))
/* status */
#define QSPI_WC                         (1 << 1)
#define QSPI_BUSY                       (1 << 0)
#define QSPI_WC_BUSY                    (QSPI_WC | QSPI_BUSY)
#define QSPI_XFER_DONE                  QSPI_WC
#define MM_SWITCH                       0x01
#define MEM_CS                          0x100
#define MEM_CS_UNSELECT                 0xfffff0ff
#define MMAP_START_ADDR_DRA		0x5c000000
#define MMAP_START_ADDR_AM43x		0x30000000
#define CORE_CTRL_IO                    0x4a002558

#define QSPI_CMD_READ                   (0x3 << 0)
#define QSPI_CMD_READ_QUAD              (0x6b << 0)
#define QSPI_CMD_READ_FAST              (0x0b << 0)
#define QSPI_SETUP0_NUM_A_BYTES         (0x2 << 8)
#define QSPI_SETUP0_NUM_D_BYTES_NO_BITS (0x0 << 10)
#define QSPI_SETUP0_NUM_D_BYTES_8_BITS  (0x1 << 10)
#define QSPI_SETUP0_READ_NORMAL         (0x0 << 12)
#define QSPI_SETUP0_READ_QUAD           (0x3 << 12)
#define QSPI_CMD_WRITE                  (0x2 << 16)
#define QSPI_NUM_DUMMY_BITS             (0x0 << 24)

/* ti qspi register set */
struct ti_qspi_regs {
	u32 pid;
	u32 pad0[3];
	u32 sysconfig;
	u32 pad1[3];
	u32 int_stat_raw;
	u32 int_stat_en;
	u32 int_en_set;
	u32 int_en_ctlr;
	u32 intc_eoi;
	u32 pad2[3];
	u32 clk_ctrl;
	u32 dc;
	u32 cmd;
	u32 status;
	u32 data;
	u32 setup0;
	u32 setup1;
	u32 setup2;
	u32 setup3;
	u32 memswitch;
	u32 data1;
	u32 data2;
	u32 data3;
};

/* ti qspi slave */
struct ti_qspi_slave {
	struct spi_slave slave;
	struct ti_qspi_regs *base;
	unsigned int mode;
	u32 cmd;
	u32 dc;
};

static inline struct ti_qspi_slave *to_ti_qspi_slave(struct spi_slave *slave)
{
	return container_of(slave, struct ti_qspi_slave, slave);
}

static void ti_spi_setup_spi_register(struct ti_qspi_slave *qslave)
{
	struct spi_slave *slave = &qslave->slave;
	u32 memval = 0;

#ifdef CONFIG_DRA7XX
	slave->memory_map = (void *)MMAP_START_ADDR_DRA;
#else
	slave->memory_map = (void *)MMAP_START_ADDR_AM43x;
	slave->op_mode_rx = 8;
#endif

	memval |= QSPI_CMD_READ | QSPI_SETUP0_NUM_A_BYTES |
			QSPI_SETUP0_NUM_D_BYTES_NO_BITS |
			QSPI_SETUP0_READ_NORMAL | QSPI_CMD_WRITE |
			QSPI_NUM_DUMMY_BITS;

	writel(memval, &qslave->base->setup0);
}

static void ti_spi_set_speed(struct spi_slave *slave, uint hz)
{
	struct ti_qspi_slave *qslave = to_ti_qspi_slave(slave);
	uint clk_div;

	debug("ti_spi_set_speed: hz: %d, clock divider %d\n", hz, clk_div);

	if (!hz)
		clk_div = 0;
	else
		clk_div = (QSPI_FCLK / hz) - 1;

	/* disable SCLK */
	writel(readl(&qslave->base->clk_ctrl) & ~QSPI_CLK_EN,
	       &qslave->base->clk_ctrl);

	/* assign clk_div values */
	if (clk_div < 0)
		clk_div = 0;
	else if (clk_div > QSPI_CLK_DIV_MAX)
		clk_div = QSPI_CLK_DIV_MAX;

	/* enable SCLK */
	writel(QSPI_CLK_EN | clk_div, &qslave->base->clk_ctrl);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* CS handled in xfer */
	return;
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct ti_qspi_slave *qslave = to_ti_qspi_slave(slave);

	debug("spi_cs_deactivate: 0x%08x\n", (u32)slave);

	writel(qslave->cmd | QSPI_INVAL, &qslave->base->cmd);
}

void spi_init(void)
{
	/* nothing to do */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct ti_qspi_slave *qslave;

#ifdef CONFIG_AM43XX
	gpio_request(CONFIG_QSPI_SEL_GPIO, "qspi_gpio");
	gpio_direction_output(CONFIG_QSPI_SEL_GPIO, 1);
#endif

	qslave = spi_alloc_slave(struct ti_qspi_slave, bus, cs);
	if (!qslave) {
		printf("SPI_error: Fail to allocate ti_qspi_slave\n");
		return NULL;
	}

	qslave->base = (struct ti_qspi_regs *)QSPI_BASE;
	qslave->mode = mode;

	ti_spi_set_speed(&qslave->slave, max_hz);

#ifdef CONFIG_TI_SPI_MMAP
	ti_spi_setup_spi_register(qslave);
#endif

	return &qslave->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct ti_qspi_slave *qslave = to_ti_qspi_slave(slave);
	free(qslave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct ti_qspi_slave *qslave = to_ti_qspi_slave(slave);

	debug("spi_claim_bus: bus:%i cs:%i\n", slave->bus, slave->cs);

	qslave->dc = 0;
	if (qslave->mode & SPI_CPHA)
		qslave->dc |= QSPI_CKPHA(slave->cs);
	if (qslave->mode & SPI_CPOL)
		qslave->dc |= QSPI_CKPOL(slave->cs);
	if (qslave->mode & SPI_CS_HIGH)
		qslave->dc |= QSPI_CSPOL(slave->cs);

	writel(qslave->dc, &qslave->base->dc);
	writel(0, &qslave->base->cmd);
	writel(0, &qslave->base->data);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct ti_qspi_slave *qslave = to_ti_qspi_slave(slave);

	debug("spi_release_bus: bus:%i cs:%i\n", slave->bus, slave->cs);

	writel(0, &qslave->base->dc);
	writel(0, &qslave->base->cmd);
	writel(0, &qslave->base->data);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct ti_qspi_slave *qslave = to_ti_qspi_slave(slave);
	uint words = bitlen >> 3; /* fixed 8-bit word length */
	const uchar *txp = dout;
	uchar *rxp = din;
	uint status;
	int timeout;

#ifdef CONFIG_DRA7XX
	int val;
#endif

	debug("spi_xfer: bus:%i cs:%i bitlen:%i words:%i flags:%lx\n",
	      slave->bus, slave->cs, bitlen, words, flags);

	/* Setup mmap flags */
	if (flags & SPI_XFER_MMAP) {
		writel(MM_SWITCH, &qslave->base->memswitch);
#ifdef CONFIG_DRA7XX
		val = readl(CORE_CTRL_IO);
		val |= MEM_CS;
		writel(val, CORE_CTRL_IO);
#endif
		return 0;
	} else if (flags & SPI_XFER_MMAP_END) {
		writel(~MM_SWITCH, &qslave->base->memswitch);
#ifdef CONFIG_DRA7XX
		val = readl(CORE_CTRL_IO);
		val &= MEM_CS_UNSELECT;
		writel(val, CORE_CTRL_IO);
#endif
		return 0;
	}

	if (bitlen == 0)
		return -1;

	if (bitlen % 8) {
		debug("spi_xfer: Non byte aligned SPI transfer\n");
		return -1;
	}

	/* Setup command reg */
	qslave->cmd = 0;
	qslave->cmd |= QSPI_WLEN(8);
	qslave->cmd |= QSPI_EN_CS(slave->cs);
	if (flags & SPI_3WIRE)
		qslave->cmd |= QSPI_3_PIN;
	qslave->cmd |= 0xfff;

/* FIXME: This delay is required for successfull
 * completion of read/write/erase. Once its root
 * caused, it will be remove from the driver.
 */
#ifdef CONFIG_AM43XX
	udelay(100);
#endif
	while (words--) {
		if (txp) {
			debug("tx cmd %08x dc %08x data %02x\n",
			      qslave->cmd | QSPI_WR_SNGL, qslave->dc, *txp);
			writel(*txp++, &qslave->base->data);
			writel(qslave->cmd | QSPI_WR_SNGL,
			       &qslave->base->cmd);
			status = readl(&qslave->base->status);
			timeout = QSPI_TIMEOUT;
			while ((status & QSPI_WC_BUSY) != QSPI_XFER_DONE) {
				if (--timeout < 0) {
					printf("spi_xfer: TX timeout!\n");
					return -1;
				}
				status = readl(&qslave->base->status);
			}
			debug("tx done, status %08x\n", status);
		}
		if (rxp) {
			qslave->cmd |= QSPI_RD_SNGL;
			debug("rx cmd %08x dc %08x\n",
			      qslave->cmd, qslave->dc);
			#ifdef CONFIG_DRA7XX
				udelay(500);
			#endif
			writel(qslave->cmd, &qslave->base->cmd);
			status = readl(&qslave->base->status);
			timeout = QSPI_TIMEOUT;
			while ((status & QSPI_WC_BUSY) != QSPI_XFER_DONE) {
				if (--timeout < 0) {
					printf("spi_xfer: RX timeout!\n");
					return -1;
				}
				status = readl(&qslave->base->status);
			}
			*rxp++ = readl(&qslave->base->data);
			debug("rx done, status %08x, read %02x\n",
			      status, *(rxp-1));
		}
	}

	/* Terminate frame */
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
