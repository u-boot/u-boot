/*
 * (C) Copyright 2014 - 2015 Xilinx
 *
 * Xilinx ZynqMP Quad-SPI(QSPI) controller driver (master mode only)
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <memalign.h>
#include <ubi_uboot.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk.h>
#include "../mtd/spi/sf_internal.h"
#include <clk.h>

DECLARE_GLOBAL_DATA_PTR;

#define ZYNQMP_QSPI_GFIFO_STRT_MODE_MASK	(1 << 29)
#define ZYNQMP_QSPI_CONFIG_MODE_EN_MASK	(3 << 30)
#define ZYNQMP_QSPI_CONFIG_DMA_MODE	(2 << 30)
#define ZYNQMP_QSPI_CONFIG_CPHA_MASK	(1 << 2)
#define ZYNQMP_QSPI_CONFIG_CPOL_MASK	(1 << 1)

/* QSPI MIO's count for different connection topologies */
#define ZYNQMP_QSPI_MIO_NUM_QSPI0		6
#define ZYNQMP_QSPI_MIO_NUM_QSPI1		5
#define ZYNQMP_QSPI_MIO_NUM_QSPI1_CS	1

/*
 * QSPI Interrupt Registers bit Masks
 *
 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
 * bit definitions.
 */
#define ZYNQMP_QSPI_IXR_TXNFULL_MASK	0x00000004 /* QSPI TX FIFO Overflow */
#define ZYNQMP_QSPI_IXR_TXFULL_MASK	0x00000008 /* QSPI TX FIFO is full */
#define ZYNQMP_QSPI_IXR_RXNEMTY_MASK	0x00000010 /* QSPI RX FIFO Not Empty */
#define ZYNQMP_QSPI_IXR_GFEMTY_MASK	0x00000080 /* QSPI Generic FIFO Empty */
#define ZYNQMP_QSPI_IXR_ALL_MASK	(ZYNQMP_QSPI_IXR_TXNFULL_MASK | \
					ZYNQMP_QSPI_IXR_RXNEMTY_MASK)

/*
 * QSPI Enable Register bit Masks
 *
 * This register is used to enable or disable the QSPI controller
 */
#define ZYNQMP_QSPI_ENABLE_ENABLE_MASK	0x00000001 /* QSPI Enable Bit Mask */

#define ZYNQMP_QSPI_GFIFO_LOW_BUS		(1 << 14)
#define ZYNQMP_QSPI_GFIFO_CS_LOWER	(1 << 12)
#define ZYNQMP_QSPI_GFIFO_UP_BUS		(1 << 15)
#define ZYNQMP_QSPI_GFIFO_CS_UPPER	(1 << 13)
#define ZYNQMP_QSPI_SPI_MODE_QSPI		(3 << 10)
#define ZYNQMP_QSPI_SPI_MODE_SPI		(1 << 10)
#define ZYNQMP_QSPI_SPI_MODE_DUAL_SPI		(2 << 10)
#define ZYNQMP_QSPI_IMD_DATA_CS_ASSERT	5
#define ZYNQMP_QSPI_IMD_DATA_CS_DEASSERT	5
#define ZYNQMP_QSPI_GFIFO_TX		(1 << 16)
#define ZYNQMP_QSPI_GFIFO_RX		(1 << 17)
#define ZYNQMP_QSPI_GFIFO_STRIPE_MASK	(1 << 18)
#define ZYNQMP_QSPI_GFIFO_IMD_MASK	0xFF
#define ZYNQMP_QSPI_GFIFO_EXP_MASK	(1 << 9)
#define ZYNQMP_QSPI_GFIFO_DATA_XFR_MASK	(1 << 8)
#define ZYNQMP_QSPI_STRT_GEN_FIFO		(1 << 28)
#define ZYNQMP_QSPI_GEN_FIFO_STRT_MOD	(1 << 29)
#define ZYNQMP_QSPI_GFIFO_WP_HOLD		(1 << 19)
#define ZYNQMP_QSPI_BAUD_DIV_MASK	(7 << 3)
#define ZYNQMP_QSPI_DFLT_BAUD_RATE_DIV	(1 << 3)
#define ZYNQMP_QSPI_GFIFO_ALL_INT_MASK	0xFBE
#define ZYNQMP_QSPI_DMA_DST_I_STS_DONE	(1 << 1)
#define ZYNQMP_QSPI_DMA_DST_I_STS_MASK	0xFE
#define MODEBITS	0x6

#define QUAD_OUT_READ_CMD		0x6B
#define QUAD_PAGE_PROGRAM_CMD		0x32
#define DUAL_OUTPUT_FASTRD_CMD		0x3B

#define ZYNQMP_QSPI_GFIFO_SELECT		(1 << 0)

#define ZYNQMP_QSPI_FIFO_THRESHOLD 1

#define SPI_XFER_ON_BOTH	0
#define SPI_XFER_ON_LOWER	1
#define SPI_XFER_ON_UPPER	2

#define ZYNQMP_QSPI_DMA_ALIGN	0x4
#define ZYNQMP_QSPI_MAX_BAUD_RATE_VAL	7
#define ZYNQMP_QSPI_DFLT_BAUD_RATE_VAL	2

#define ZYNQMP_QSPI_TIMEOUT	100000000

/* QSPI register offsets */
struct zynqmp_qspi_regs {
	u32 confr;	/* 0x00 */
	u32 isr;	/* 0x04 */
	u32 ier;	/* 0x08 */
	u32 idisr;	/* 0x0C */
	u32 imaskr;	/* 0x10 */
	u32 enbr;	/* 0x14 */
	u32 dr;		/* 0x18 */
	u32 txd0r;	/* 0x1C */
	u32 drxr;	/* 0x20 */
	u32 sicr;	/* 0x24 */
	u32 txftr;	/* 0x28 */
	u32 rxftr;	/* 0x2C */
	u32 gpior;	/* 0x30 */
	u32 reserved0;	/* 0x34 */
	u32 lpbkdly;	/* 0x38 */
	u32 reserved1;	/* 0x3C */
	u32 genfifo;	/* 0x40 */
	u32 gqspisel;	/* 0x44 */
	u32 reserved2;	/* 0x48 */
	u32 gqfifoctrl;	/* 0x4C */
	u32 gqfthr;	/* 0x50 */
	u32 gqpollcfg;	/* 0x54 */
	u32 gqpollto;	/* 0x58 */
	u32 gqxfersts;	/* 0x5C */
	u32 gqfifosnap;	/* 0x60 */
	u32 gqrxcpy;	/* 0x64 */
	u32 reserved3[36];	/* 0x68 */
	u32 gqspidlyadj;	/* 0xF8 */
};

struct zynqmp_qspi_dma_regs {
	u32 dmadst;	/* 0x00 */
	u32 dmasize;	/* 0x04 */
	u32 dmasts;	/* 0x08 */
	u32 dmactrl;	/* 0x0C */
	u32 reserved0;	/* 0x10 */
	u32 dmaisr;	/* 0x14 */
	u32 dmaier;	/* 0x18 */
	u32 dmaidr;	/* 0x1C */
	u32 dmaimr;	/* 0x20 */
	u32 dmactrl2;	/* 0x24 */
	u32 dmadstmsb;	/* 0x28 */
};

struct zynqmp_qspi_platdata {
	struct zynqmp_qspi_regs *regs;
	struct zynqmp_qspi_dma_regs *dma_regs;
	u32 frequency;
	u32 speed_hz;
	unsigned int is_dual;
	unsigned int tx_rx_mode;
};

struct zynqmp_qspi_priv {
	struct zynqmp_qspi_regs *regs;
	struct zynqmp_qspi_dma_regs *dma_regs;
	u8 mode;
	const void *tx_buf;
	void *rx_buf;
	unsigned len;
	int bytes_to_transfer;
	int bytes_to_receive;
	unsigned int is_inst;
	unsigned int is_dual;
	unsigned int u_page;
	unsigned int bus;
	unsigned int stripe;
	unsigned cs_change:1;
	unsigned int dummy_bytes;
	unsigned int tx_rx_mode;
};

static u8 last_cmd;

static int zynqmp_qspi_ofdata_to_platdata(struct udevice *bus)
{
	struct zynqmp_qspi_platdata *plat = bus->platdata;
	int is_dual;
	u32 mode = 0;
	int offset;
	u32 value;
	int ret;
	struct clk clk;
	unsigned long clock;

	debug("%s\n", __func__);

	plat->regs = (struct zynqmp_qspi_regs *)(devfdt_get_addr(bus) + 0x100);
	plat->dma_regs = (struct zynqmp_qspi_dma_regs *)(devfdt_get_addr(bus) +
							 0x800);

	ret = clk_get_by_index(bus, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return clock;
	}
	debug("%s: CLK %ld\n", __func__, clock);

	ret = clk_enable(&clk);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	is_dual = fdtdec_get_int(gd->fdt_blob, dev_of_offset(bus), "is-dual", -1);
	if (is_dual < 0)
		plat->is_dual = SF_SINGLE_FLASH;
	else if (is_dual == 1)
		plat->is_dual = SF_DUAL_PARALLEL_FLASH;
	else
		if (fdtdec_get_int(gd->fdt_blob, dev_of_offset(bus),
				   "is-stacked", -1) < 0)
			plat->is_dual = SF_SINGLE_FLASH;
		else
			plat->is_dual = SF_DUAL_STACKED_FLASH;

	offset = fdt_first_subnode(gd->fdt_blob, dev_of_offset(bus));

	value = fdtdec_get_uint(gd->fdt_blob, offset, "spi-rx-bus-width", 1);
	switch (value) {
	case 1:
		break;
	case 2:
		mode |= SPI_RX_DUAL;
		break;
	case 4:
		mode |= SPI_RX_QUAD;
		break;
	default:
		printf("Invalid spi-rx-bus-width %d\n", value);
		break;
	}

	value = dev_read_u32_default(bus, "spi-tx-bus-width", 1);
	switch (value) {
	case 1:
		break;
	case 2:
		mode |= SPI_TX_DUAL;
		break;
	case 4:
		mode |= SPI_TX_QUAD;
		break;
	default:
		printf("Invalid spi-tx-bus-width %d\n", value);
		break;
	}

	plat->tx_rx_mode = mode;

	plat->frequency = clock;
	plat->speed_hz = plat->frequency;

	return 0;
}

static void zynqmp_qspi_init_hw(struct zynqmp_qspi_priv *priv)
{
	u32 config_reg;
	struct zynqmp_qspi_regs *regs = priv->regs;

	writel(ZYNQMP_QSPI_GFIFO_SELECT, &regs->gqspisel);
	writel(ZYNQMP_QSPI_GFIFO_ALL_INT_MASK, &regs->idisr);
	writel(ZYNQMP_QSPI_FIFO_THRESHOLD, &regs->txftr);
	writel(ZYNQMP_QSPI_FIFO_THRESHOLD, &regs->rxftr);
	writel(ZYNQMP_QSPI_GFIFO_ALL_INT_MASK, &regs->isr);

	config_reg = readl(&regs->confr);
	config_reg &= ~(ZYNQMP_QSPI_GFIFO_STRT_MODE_MASK |
			ZYNQMP_QSPI_CONFIG_MODE_EN_MASK);
	config_reg |= ZYNQMP_QSPI_CONFIG_DMA_MODE |
		      ZYNQMP_QSPI_GFIFO_WP_HOLD |
		      ZYNQMP_QSPI_DFLT_BAUD_RATE_DIV;
	writel(config_reg, &regs->confr);

	writel(ZYNQMP_QSPI_ENABLE_ENABLE_MASK, &regs->enbr);
}

static u32 zynqmp_qspi_bus_select(struct zynqmp_qspi_priv *priv)
{
	u32 gqspi_fifo_reg = 0;

	if (priv->is_dual == SF_DUAL_PARALLEL_FLASH) {
		if (priv->bus == SPI_XFER_ON_BOTH)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_UP_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER;
		else if (priv->bus == SPI_XFER_ON_LOWER)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER;
		else if (priv->bus == SPI_XFER_ON_UPPER)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_UP_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER;
		else
			debug("Wrong Bus selection:0x%x\n", priv->bus);
	} else {
		if (priv->u_page)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER;
		else
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER;
	}
	return gqspi_fifo_reg;
}

static void zynqmp_qspi_fill_gen_fifo(struct zynqmp_qspi_priv *priv,
				      u32 gqspi_fifo_reg)
{
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 reg;

	do {
		reg = readl(&regs->isr);
	} while (!(reg & ZYNQMP_QSPI_IXR_GFEMTY_MASK));

	writel(gqspi_fifo_reg, &regs->genfifo);
}

static void zynqmp_qspi_chipselect(struct zynqmp_qspi_priv *priv, int is_on)
{
	u32 gqspi_fifo_reg = 0;

	if (is_on) {
		gqspi_fifo_reg = zynqmp_qspi_bus_select(priv);
		gqspi_fifo_reg |= ZYNQMP_QSPI_SPI_MODE_SPI |
				  ZYNQMP_QSPI_IMD_DATA_CS_ASSERT;
	} else {
		if (priv->is_dual == SF_DUAL_PARALLEL_FLASH)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_UP_BUS |
					 ZYNQMP_QSPI_GFIFO_LOW_BUS;
		else if (priv->u_page)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_UP_BUS;
		else
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS;
		gqspi_fifo_reg |= ZYNQMP_QSPI_IMD_DATA_CS_DEASSERT;
	}

	debug("GFIFO_CMD_CS: 0x%x\n", gqspi_fifo_reg);

	zynqmp_qspi_fill_gen_fifo(priv, gqspi_fifo_reg);
}

#define GQSPI_BAUD_DIV_SHIFT		2
#define GQSPI_LPBK_DLY_ADJ_LPBK_SHIFT	5
#define GQSPI_LPBK_DLY_ADJ_DLY_1	0x2
#define GQSPI_LPBK_DLY_ADJ_DLY_1_SHIFT	3
#define GQSPI_LPBK_DLY_ADJ_DLY_0	0x3
#define GQSPI_USE_DATA_DLY		0x1
#define GQSPI_USE_DATA_DLY_SHIFT	31
#define GQSPI_DATA_DLY_ADJ_VALUE	0x2
#define GQSPI_DATA_DLY_ADJ_SHIFT	28
#define TAP_DLY_BYPASS_LQSPI_RX_VALUE	0x1
#define TAP_DLY_BYPASS_LQSPI_RX_SHIFT	2
#define GQSPI_DATA_DLY_ADJ_OFST		0x000001F8
#define IOU_TAPDLY_BYPASS_OFST		0xFF180390
#define GQSPI_LPBK_DLY_ADJ_USE_LPBK_MASK	0x00000020
#define GQSPI_FREQ_40MHZ		40000000
#define GQSPI_FREQ_100MHZ		100000000
#define GQSPI_FREQ_150MHZ		150000000
#define IOU_TAPDLY_BYPASS_MASK		0x7

void zynqmp_qspi_set_tapdelay(struct udevice *bus, u32 baudrateval)
{
	struct zynqmp_qspi_platdata *plat = bus->platdata;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 tapdlybypass = 0, lpbkdlyadj = 0, datadlyadj = 0, clk_rate;
	u32 reqhz = 0;

	clk_rate = plat->frequency;
	reqhz = (clk_rate / (GQSPI_BAUD_DIV_SHIFT << baudrateval));

	debug("%s, req_hz:%d, clk_rate:%d, baudrateval:%d\n",
	      __func__, reqhz, clk_rate, baudrateval);

	if (reqhz < GQSPI_FREQ_40MHZ) {
		zynqmp_mmio_read(IOU_TAPDLY_BYPASS_OFST, &tapdlybypass);
		tapdlybypass |= (TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
				TAP_DLY_BYPASS_LQSPI_RX_SHIFT);
	} else if (reqhz < GQSPI_FREQ_100MHZ) {
		zynqmp_mmio_read(IOU_TAPDLY_BYPASS_OFST, &tapdlybypass);
		tapdlybypass |= (TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
				TAP_DLY_BYPASS_LQSPI_RX_SHIFT);
		lpbkdlyadj = readl(&regs->lpbkdly);
		lpbkdlyadj |= (GQSPI_LPBK_DLY_ADJ_USE_LPBK_MASK);
		datadlyadj = readl(&regs->gqspidlyadj);
		datadlyadj |= ((GQSPI_USE_DATA_DLY << GQSPI_USE_DATA_DLY_SHIFT)
				| (GQSPI_DATA_DLY_ADJ_VALUE <<
					GQSPI_DATA_DLY_ADJ_SHIFT));
	} else if (reqhz < GQSPI_FREQ_150MHZ) {
		lpbkdlyadj = readl(&regs->lpbkdly);
		lpbkdlyadj |= ((GQSPI_LPBK_DLY_ADJ_USE_LPBK_MASK) |
				GQSPI_LPBK_DLY_ADJ_DLY_0);
	}

	zynqmp_mmio_write(IOU_TAPDLY_BYPASS_OFST, IOU_TAPDLY_BYPASS_MASK,
			  tapdlybypass);
	writel(lpbkdlyadj, &regs->lpbkdly);
	writel(datadlyadj, &regs->gqspidlyadj);
}

static int zynqmp_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct zynqmp_qspi_platdata *plat = bus->platdata;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	uint32_t confr;
	u8 baud_rate_val = 0;

	debug("%s\n", __func__);
	if (speed > plat->frequency)
		speed = plat->frequency;

	/* Set the clock frequency */
	confr = readl(&regs->confr);
	if (speed == 0) {
		/* Set baudrate x8, if the freq is 0 */
		baud_rate_val = ZYNQMP_QSPI_DFLT_BAUD_RATE_VAL;
	} else if (plat->speed_hz != speed) {
		while ((baud_rate_val < 8) &&
		       ((plat->frequency /
		       (2 << baud_rate_val)) > speed))
			baud_rate_val++;

		if (baud_rate_val > ZYNQMP_QSPI_MAX_BAUD_RATE_VAL)
			baud_rate_val = ZYNQMP_QSPI_DFLT_BAUD_RATE_VAL;

		plat->speed_hz = plat->frequency / (2 << baud_rate_val);
	}
	confr &= ~ZYNQMP_QSPI_BAUD_DIV_MASK;
	confr |= (baud_rate_val << 3);
	writel(confr, &regs->confr);

	zynqmp_qspi_set_tapdelay(bus, baud_rate_val);
	debug("regs=%p, speed=%d\n", priv->regs, plat->speed_hz);

	return 0;
}

static int zynqmp_qspi_child_pre_probe(struct udevice *bus)
{
	struct spi_slave *slave = dev_get_parent_priv(bus);
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus->parent);
	struct zynqmp_qspi_platdata *plat = dev_get_platdata(bus->parent);

	slave->option = priv->is_dual;
	slave->mode = plat->tx_rx_mode;
	slave->bytemode = SPI_4BYTE_MODE;

	return 0;
}

static int zynqmp_qspi_probe(struct udevice *bus)
{
	struct zynqmp_qspi_platdata *plat = dev_get_platdata(bus);
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);

	debug("zynqmp_qspi_probe:  bus:%p, priv:%p \n", bus, priv);

	priv->regs = plat->regs;
	priv->dma_regs = plat->dma_regs;
	priv->is_dual = plat->is_dual;
	priv->tx_rx_mode = plat->tx_rx_mode;

	if (priv->is_dual == -1) {
		debug("%s: No QSPI device detected based on MIO settings\n",
		      __func__);
		return -1;
	}

	/* init the zynq spi hw */
	zynqmp_qspi_init_hw(priv);

	return 0;
}

static int zynqmp_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	uint32_t confr;

	debug("%s\n", __func__);
	/* Set the SPI Clock phase and polarities */
	confr = readl(&regs->confr);
	confr &= ~(ZYNQMP_QSPI_CONFIG_CPHA_MASK |
		   ZYNQMP_QSPI_CONFIG_CPOL_MASK);

	if (priv->mode & SPI_CPHA)
		confr |= ZYNQMP_QSPI_CONFIG_CPHA_MASK;
	if (priv->mode & SPI_CPOL)
		confr |= ZYNQMP_QSPI_CONFIG_CPOL_MASK;

	//writel(confr, &regs->confr);
	priv->mode = mode;

	debug("regs=%p, mode=%d\n", priv->regs, priv->mode);

	return 0;
}


static int zynqmp_qspi_fill_tx_fifo(struct zynqmp_qspi_priv *priv, u32 size)
{
	u32 data;
	u32 timeout = ZYNQMP_QSPI_TIMEOUT;
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 *buf = (u32 *)priv->tx_buf;
	u32 len = size;

	debug("TxFIFO: 0x%x, size: 0x%x\n", readl(&regs->isr),
	      size);

	while (size && timeout) {
		if (readl(&regs->isr) &
			ZYNQMP_QSPI_IXR_TXNFULL_MASK) {
			if (size >= 4) {
				writel(*buf, &regs->txd0r);
				buf++;
				size -= 4;
			} else {
				switch (size) {
				case 1:
					data = *((u8 *)buf);
					buf += 1;
					data |= 0xFFFFFF00;
					break;
				case 2:
					data = *((u16 *)buf);
					buf += 2;
					data |= 0xFFFF0000;
					break;
				case 3:
					data = *((u16 *)buf);
					buf += 2;
					data |= (*((u8 *)buf) << 16);
					buf += 1;
					data |= 0xFF000000;
					break;
				}
				writel(data, &regs->txd0r);
				size = 0;
			}
		} else {
			udelay(1);
			timeout--;
		}
	}
	if (!timeout) {
		printf("zynqmp_qspi_fill_tx_fifo: Timeout\n");
		return -1;
	}

	priv->tx_buf += len;
	return 0;
}

static void zynqmp_qspi_genfifo_cmd(struct zynqmp_qspi_priv *priv)
{
	u8 command = 1;
	u32 gen_fifo_cmd;
	u32 bytecount = 0;

	if (priv->dummy_bytes)
		priv->len -= priv->dummy_bytes;

	while (priv->len) {
		gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
		gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_TX;

		if (command) {
			command = 0;
			last_cmd = *(u8 *)priv->tx_buf;
		}

		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_SPI;
		gen_fifo_cmd |= *(u8 *)priv->tx_buf;
		bytecount++;
		priv->len--;
		priv->tx_buf = (u8 *)priv->tx_buf + 1;

		debug("GFIFO_CMD_Cmd = 0x%x\n", gen_fifo_cmd);

		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);
	}

	if (priv->dummy_bytes) {
		gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
		gen_fifo_cmd &= ~(ZYNQMP_QSPI_GFIFO_TX | ZYNQMP_QSPI_GFIFO_RX);
		if (priv->tx_rx_mode & SPI_RX_QUAD)
			gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_QSPI;
		else if (priv->tx_rx_mode & SPI_RX_DUAL)
			gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_DUAL_SPI;
		else
			gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_SPI;
		gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_DATA_XFR_MASK;
		gen_fifo_cmd |= (priv->dummy_bytes * 8);
		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);
	}
}

static u32 zynqmp_qspi_calc_exp(struct zynqmp_qspi_priv *priv,
				u32 *gen_fifo_cmd)
{
	u32 expval = 8;
	u32 len;

	while (1) {
		if (priv->len > 255) {
			if (priv->len & (1 << expval)) {
				*gen_fifo_cmd &= ~ZYNQMP_QSPI_GFIFO_IMD_MASK;
				*gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_EXP_MASK;
				*gen_fifo_cmd |= expval;
				priv->len -= (1 << expval);
				return expval;
			}
			expval++;
		} else {
			*gen_fifo_cmd &= ~(ZYNQMP_QSPI_GFIFO_IMD_MASK |
					  ZYNQMP_QSPI_GFIFO_EXP_MASK);
			*gen_fifo_cmd |= (u8)priv->len;
			len = (u8)priv->len;
			priv->len  = 0;
			return len;
		}
	}
}

static int zynqmp_qspi_genfifo_fill_tx(struct zynqmp_qspi_priv *priv)
{
	u32 gen_fifo_cmd;
	u32 len;
	int ret = 0;

	gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
	gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_TX |
			ZYNQMP_QSPI_GFIFO_DATA_XFR_MASK;

	if (priv->stripe)
		gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_STRIPE_MASK;

	if (last_cmd == QUAD_PAGE_PROGRAM_CMD)
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_QSPI;
	else
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_SPI;

	while (priv->len) {
		len = zynqmp_qspi_calc_exp(priv, &gen_fifo_cmd);
		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);

		debug("GFIFO_CMD_TX:0x%x\n", gen_fifo_cmd);

		if (gen_fifo_cmd & ZYNQMP_QSPI_GFIFO_EXP_MASK)
			ret = zynqmp_qspi_fill_tx_fifo(priv,
						       1 << len);
		else
			ret = zynqmp_qspi_fill_tx_fifo(priv,
						       len);

		if (ret)
			return ret;
	}
	return ret;
}

static int zynqmp_qspi_start_dma(struct zynqmp_qspi_priv *priv,
				 u32 gen_fifo_cmd, u32 *buf)
{
	u32 addr;
	u32 size, len;
	u32 timeout = ZYNQMP_QSPI_TIMEOUT;
	u32 actuallen = priv->len;
	struct zynqmp_qspi_dma_regs *dma_regs = priv->dma_regs;

	writel((unsigned long)buf, &dma_regs->dmadst);
	writel(roundup(priv->len, 4), &dma_regs->dmasize);
	writel(ZYNQMP_QSPI_DMA_DST_I_STS_MASK, &dma_regs->dmaier);
	addr = (unsigned long)buf;
	size = roundup(priv->len, ARCH_DMA_MINALIGN);
	flush_dcache_range(addr, addr+size);

	while (priv->len) {
		len = zynqmp_qspi_calc_exp(priv, &gen_fifo_cmd);
		if (!(gen_fifo_cmd & ZYNQMP_QSPI_GFIFO_EXP_MASK) &&
		    (len % 4)) {
			gen_fifo_cmd &= ~(0xFF);
			gen_fifo_cmd |= (len/4 + 1) * 4;
		}
		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);

		debug("GFIFO_CMD_RX:0x%x\n", gen_fifo_cmd);
	}

	while (timeout) {
		if (readl(&dma_regs->dmaisr) &
		    ZYNQMP_QSPI_DMA_DST_I_STS_DONE) {
			writel(ZYNQMP_QSPI_DMA_DST_I_STS_DONE,
			       &dma_regs->dmaisr);
			break;
		}
		udelay(1);
		timeout--;
	}

	debug("buf:0x%lx, rxbuf:0x%lx, *buf:0x%x len: 0x%x\n",
	      (unsigned long)buf, (unsigned long)priv->rx_buf, *buf,
	      actuallen);
	if (!timeout) {
		printf("DMA Timeout:0x%x\n", readl(&dma_regs->dmaisr));
		return -1;
	}

	if (buf != priv->rx_buf)
		memcpy(priv->rx_buf, buf, actuallen);

	return 0;
}

static int zynqmp_qspi_genfifo_fill_rx(struct zynqmp_qspi_priv *priv)
{
	u32 gen_fifo_cmd;
	u32 *buf;
	u32 actuallen = priv->len;

	gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
	gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_RX |
			ZYNQMP_QSPI_GFIFO_DATA_XFR_MASK;

	if (last_cmd == QUAD_OUT_READ_CMD)
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_QSPI;
	else if (last_cmd == DUAL_OUTPUT_FASTRD_CMD)
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_DUAL_SPI;
	else
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_SPI;

	if (priv->stripe)
		gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_STRIPE_MASK;

	/*
	 * Check if receive buffer is aligned to 4 byte and length
	 * is multiples of four byte as we are using dma to receive.
	 */
	if (!((unsigned long)priv->rx_buf & (ZYNQMP_QSPI_DMA_ALIGN - 1)) &&
	    !(actuallen % ZYNQMP_QSPI_DMA_ALIGN)) {
		buf = (u32 *)priv->rx_buf;
		return zynqmp_qspi_start_dma(priv, gen_fifo_cmd, buf);
	}

	ALLOC_CACHE_ALIGN_BUFFER(u8, tmp, roundup(priv->len,
						  ZYNQMP_QSPI_DMA_ALIGN));
	buf = (u32 *)tmp;
	return zynqmp_qspi_start_dma(priv, gen_fifo_cmd, buf);
}

static int zynqmp_qspi_start_transfer(struct zynqmp_qspi_priv *priv)
{
	int ret = 0;

	if (priv->is_inst) {
		if (priv->tx_buf)
			zynqmp_qspi_genfifo_cmd(priv);
		else
			ret = -1;
	} else {
		if (priv->tx_buf)
			ret = zynqmp_qspi_genfifo_fill_tx(priv);
		else if (priv->rx_buf)
			ret = zynqmp_qspi_genfifo_fill_rx(priv);
		else
			ret = -1;
	}
	return ret;
}

static int zynqmp_qspi_transfer(struct zynqmp_qspi_priv *priv)
{
	static unsigned cs_change = 1;
	int status = 0;

	debug("%s\n", __func__);

	while (1) {
		/* Select the chip if required */
		if (cs_change)
			zynqmp_qspi_chipselect(priv, 1);

		cs_change = priv->cs_change;

		if (!priv->tx_buf && !priv->rx_buf && priv->len) {
			status = -1;
			break;
		}

		/* Request the transfer */
		if (priv->len) {
			status = zynqmp_qspi_start_transfer(priv);
			priv->is_inst = 0;
			if (status < 0)
				break;
		}

		if (cs_change)
			/* Deselect the chip */
			zynqmp_qspi_chipselect(priv, 0);
		break;
	}

	return status;
}

static int zynqmp_qspi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;

	debug("%s\n", __func__);
	writel(ZYNQMP_QSPI_ENABLE_ENABLE_MASK, &regs->enbr);

	return 0;
}

static int zynqmp_qspi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;

	debug("%s\n", __func__);
	writel(~ZYNQMP_QSPI_ENABLE_ENABLE_MASK, &regs->enbr);

	return 0;
}

int zynqmp_qspi_xfer(struct udevice *dev, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct spi_slave *slave = dev_get_parent_priv(dev);

	debug("%s: priv: 0x%08lx bitlen: %d dout: 0x%08lx ", __func__,
	      (unsigned long)priv, bitlen, (unsigned long)dout);
	debug("din: 0x%08lx flags: 0x%lx\n", (unsigned long)din, flags);

	priv->tx_buf = dout;
	priv->rx_buf = din;
	priv->len = bitlen / 8;

	/*
	 * Festering sore.
	 * Assume that the beginning of a transfer with bits to
	 * transmit must contain a device command.
	 */
	if (dout && flags & SPI_XFER_BEGIN)
		priv->is_inst = 1;
	else
		priv->is_inst = 0;

	if (flags & SPI_XFER_END)
		priv->cs_change = 1;
	else
		priv->cs_change = 0;

	if (flags & SPI_XFER_U_PAGE)
		priv->u_page = 1;
	else
		priv->u_page = 0;

	priv->stripe = 0;
	priv->bus = 0;

	if (priv->is_dual == SF_DUAL_PARALLEL_FLASH) {
		if (flags & SPI_XFER_MASK)
			priv->bus = (flags & SPI_XFER_MASK) >> 8;
		if (flags & SPI_XFER_STRIPE)
			priv->stripe = 1;
	}

	priv->dummy_bytes = slave->dummy_bytes;
	zynqmp_qspi_transfer(priv);

	return 0;
}

static const struct dm_spi_ops zynqmp_qspi_ops = {
	.claim_bus      = zynqmp_qspi_claim_bus,
	.release_bus    = zynqmp_qspi_release_bus,
	.xfer           = zynqmp_qspi_xfer,
	.set_speed      = zynqmp_qspi_set_speed,
	.set_mode       = zynqmp_qspi_set_mode,
};

static const struct udevice_id zynqmp_qspi_ids[] = {
	{ .compatible = "xlnx,zynqmp-qspi-1.0" },
	{ }
};

U_BOOT_DRIVER(zynqmp_qspi) = {
	.name   = "zynqmp_qspi",
	.id     = UCLASS_SPI,
	.of_match = zynqmp_qspi_ids,
	.ops    = &zynqmp_qspi_ops,
	.ofdata_to_platdata = zynqmp_qspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct zynqmp_qspi_platdata),
	.priv_auto_alloc_size = sizeof(struct zynqmp_qspi_priv),
	.probe  = zynqmp_qspi_probe,
	.child_pre_probe = zynqmp_qspi_child_pre_probe,
};
