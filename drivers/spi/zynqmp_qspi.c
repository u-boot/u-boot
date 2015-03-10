/*
 * (C) Copyright 2014 - 2015 Xilinx
 *
 * Xilinx ZynqMP Quad-SPI(QSPI) controller driver (master mode only)
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <ubi_uboot.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk.h>
#include "../mtd/spi/sf_internal.h"

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
#define ZYNQMP_QSPI_DFLT_BAUD_RATE_DIV	(1 << 3)
#define ZYNQMP_QSPI_GFIFO_ALL_INT_MASK	0xFBE
#define ZYNQMP_QSPI_DMA_DST_I_STS_DONE	(1 << 1)
#define ZYNQMP_QSPI_DMA_DST_I_STS_MASK	0xFE
#define MODEBITS	0x6

#define QUAD_OUT_READ_CMD		0x6B
#define QUAD_PAGE_PROGRAM_CMD		0x32

#define ZYNQMP_QSPI_GFIFO_SELECT		(1 << 0)

#define ZYNQMP_QSPI_FIFO_THRESHOLD 1

#define SPI_XFER_ON_BOTH	0
#define SPI_XFER_ON_LOWER	1
#define SPI_XFER_ON_UPPER	2

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

#define zynqmp_qspi_base				\
		 ((struct zynqmp_qspi_regs *)(ZYNQMP_QSPI_BASEADDR + 0x100))
#define zynqmp_qspi_dma					\
		 ((struct zynqmp_qspi_dma_regs *)(ZYNQMP_QSPI_BASEADDR + 0x800))

struct zynqmp_qspi {
	u32 input_clk_hz;
	u32 speed_hz;
	const void *txbuf;
	void *rxbuf;
	int bytes_to_transfer;
	int bytes_to_receive;
	unsigned int is_inst;
	unsigned int is_dual;
	unsigned int u_page;
	unsigned int bus;
	unsigned int stripe;
};

struct spi_device {
	struct zynqmp_qspi master;
	u32 max_speed_hz;
	u8 chip_select;
	u8 mode;
};

struct spi_transfer {
	const void *tx_buf;
	void *rx_buf;
	unsigned len;
	unsigned cs_change:1;
	u16 delay_usecs;
	u32 speed_hz;
};

struct zynqmp_qspi_slave {
	struct spi_slave slave;
	struct spi_device qspi;
};
#define to_zynqmp_qspi_slave(s) container_of(s, struct zynqmp_qspi_slave, slave)

static u8 last_cmd;

static void zynqmp_qspi_init_hw(int is_dual, unsigned int cs)
{
	u32 config_reg;

	writel(ZYNQMP_QSPI_GFIFO_SELECT, &zynqmp_qspi_base->gqspisel);
	writel(ZYNQMP_QSPI_GFIFO_ALL_INT_MASK, &zynqmp_qspi_base->idisr);
	writel(ZYNQMP_QSPI_FIFO_THRESHOLD, &zynqmp_qspi_base->txftr);
	writel(ZYNQMP_QSPI_FIFO_THRESHOLD, &zynqmp_qspi_base->rxftr);
	writel(ZYNQMP_QSPI_GFIFO_ALL_INT_MASK, &zynqmp_qspi_base->isr);

	config_reg = readl(&zynqmp_qspi_base->confr);
	config_reg &= ~(ZYNQMP_QSPI_GFIFO_STRT_MODE_MASK |
			ZYNQMP_QSPI_CONFIG_MODE_EN_MASK);
	config_reg |= ZYNQMP_QSPI_CONFIG_DMA_MODE |
		      ZYNQMP_QSPI_GFIFO_WP_HOLD |
		      ZYNQMP_QSPI_DFLT_BAUD_RATE_DIV;
	writel(config_reg, &zynqmp_qspi_base->confr);

	writel(ZYNQMP_QSPI_ENABLE_ENABLE_MASK, &zynqmp_qspi_base->enbr);
}

static u32 zynqmp_qspi_bus_select(struct spi_device *qspi)
{
	u32 gqspi_fifo_reg = 0;

	if (qspi->master.is_dual == SF_DUAL_PARALLEL_FLASH) {
		if (qspi->master.bus == SPI_XFER_ON_BOTH)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_UP_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER;
		else if (qspi->master.bus == SPI_XFER_ON_LOWER)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER;
		else if (qspi->master.bus == SPI_XFER_ON_UPPER)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_UP_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER;
		else
			printf("Wrong Bus selection:0x%x\n", qspi->master.bus);
	} else {
		if (qspi->master.u_page)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_UPPER;
		else
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS |
					 ZYNQMP_QSPI_GFIFO_CS_LOWER;
	}
	return gqspi_fifo_reg;
}

static void zynqmp_qspi_chipselect(struct spi_device *qspi, int is_on)
{
	u32 gqspi_fifo_reg = 0;

	if (is_on) {
		gqspi_fifo_reg = zynqmp_qspi_bus_select(qspi);
		gqspi_fifo_reg |= ZYNQMP_QSPI_SPI_MODE_SPI |
				  ZYNQMP_QSPI_IMD_DATA_CS_ASSERT;
	} else {
		if (qspi->master.is_dual == SF_DUAL_PARALLEL_FLASH)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_UP_BUS |
					 ZYNQMP_QSPI_GFIFO_LOW_BUS;
		else if (qspi->master.u_page)
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_UP_BUS;
		else
			gqspi_fifo_reg = ZYNQMP_QSPI_GFIFO_LOW_BUS;
		gqspi_fifo_reg |= ZYNQMP_QSPI_IMD_DATA_CS_DEASSERT;
	}

	debug("GFIFO_CMD_CS: 0x%x\n", gqspi_fifo_reg);

	writel(gqspi_fifo_reg, &zynqmp_qspi_base->genfifo);
}

static int zynqmp_qspi_setup_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	u32 config_reg;

	if (qspi->mode & ~MODEBITS) {
		printf("%s: Unsupported mode bits %x\n",
		       __func__, qspi->mode & ~MODEBITS);
		return -1;
	}

	config_reg = readl(&zynqmp_qspi_base->confr);

	/* Set the QSPI clock phase and clock polarity */
	config_reg &= (~ZYNQMP_QSPI_CONFIG_CPHA_MASK) &
				(~ZYNQMP_QSPI_CONFIG_CPOL_MASK);
	if (qspi->mode & SPI_CPHA)
		config_reg |= ZYNQMP_QSPI_CONFIG_CPHA_MASK;
	if (qspi->mode & SPI_CPOL)
		config_reg |= ZYNQMP_QSPI_CONFIG_CPOL_MASK;

	return 0;
}

static int zynqmp_qspi_fill_tx_fifo(u32 *buf, u32 size)
{
	u32 data;
	u32 timeout = 10000000;

	debug("TxFIFO: 0x%x, size: 0x%x\n", readl(&zynqmp_qspi_base->isr),
	      size);

	while (size && timeout) {
		if (readl(&zynqmp_qspi_base->isr) &
			ZYNQMP_QSPI_IXR_TXNFULL_MASK) {
			if (size >= 4) {
				writel(*buf, &zynqmp_qspi_base->txd0r);
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
				writel(data, &zynqmp_qspi_base->txd0r);
				size = 0;
			}
		} else {
			timeout--;
		}
	}
	if (!timeout) {
		printf("zynqmp_qspi_fill_tx_fifo: Timeout\n");
		return -1;
	}

	return 0;
}

static void zynqmp_qspi_genfifo_cmd(struct spi_device *qspi,
				    struct spi_transfer *transfer)
{
	u8 command = 1;
	u32 gen_fifo_cmd;
	u32 bytecount = 0;

	while (transfer->len) {
		gen_fifo_cmd = zynqmp_qspi_bus_select(qspi);
		gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_TX;

		if (command) {
			command = 0;
			last_cmd = *(u8 *)transfer->tx_buf;
		}

		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_SPI;
		gen_fifo_cmd |= *(u8 *)transfer->tx_buf;
		bytecount++;
		transfer->len--;
		transfer->tx_buf = (u8 *)transfer->tx_buf + 1;

		debug("GFIFO_CMD_Cmd = 0x%x\n", gen_fifo_cmd);

		writel(gen_fifo_cmd, &zynqmp_qspi_base->genfifo);
	}
}

static u32 zynqmp_qspi_calc_exp(struct spi_transfer *transfer,
				u32 *gen_fifo_cmd)
{
	u32 expval = 8;
	u32 len;

	while (1) {
		if (transfer->len > 255) {
			if (transfer->len & (1 << expval)) {
				*gen_fifo_cmd &= ~ZYNQMP_QSPI_GFIFO_IMD_MASK;
				*gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_EXP_MASK;
				*gen_fifo_cmd |= expval;
				transfer->len -= (1 << expval);
				return expval;
			}
			expval++;
		} else {
			*gen_fifo_cmd &= ~(ZYNQMP_QSPI_GFIFO_IMD_MASK |
					  ZYNQMP_QSPI_GFIFO_EXP_MASK);
			*gen_fifo_cmd |= (u8)transfer->len;
			len = (u8)transfer->len;
			transfer->len  = 0;
			return len;
		}
	}
}

static int zynqmp_qspi_genfifo_fill_tx(struct spi_device *qspi,
					struct spi_transfer *transfer)
{
	u32 gen_fifo_cmd;
	u32 len;
	int ret = 0;

	gen_fifo_cmd = zynqmp_qspi_bus_select(qspi);
	gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_TX |
			ZYNQMP_QSPI_GFIFO_DATA_XFR_MASK;

	if (qspi->master.stripe)
		gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_STRIPE_MASK;

	if (last_cmd == QUAD_PAGE_PROGRAM_CMD)
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_QSPI;
	else
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_SPI;

	while (transfer->len) {
		len = zynqmp_qspi_calc_exp(transfer, &gen_fifo_cmd);
		writel(gen_fifo_cmd, &zynqmp_qspi_base->genfifo);

		debug("GFIFO_CMD_TX:0x%x\n", gen_fifo_cmd);

		if (gen_fifo_cmd & ZYNQMP_QSPI_GFIFO_EXP_MASK)
			ret = zynqmp_qspi_fill_tx_fifo((u32 *)transfer->tx_buf,
						       1 << len);
		else
			ret = zynqmp_qspi_fill_tx_fifo((u32 *)transfer->tx_buf,
						       len);

		if (ret)
			return ret;
	}
	return ret;
}

static int zynqmp_qspi_genfifo_fill_rx(struct spi_device *qspi,
					struct spi_transfer *transfer)
{
	u32 gen_fifo_cmd;
	u32 *buf;
	u32 addr;
	u32 size, len;
	u32 timeout = 10000000;
	u32 actuallen = transfer->len;

	gen_fifo_cmd = zynqmp_qspi_bus_select(qspi);
	gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_RX |
			ZYNQMP_QSPI_GFIFO_DATA_XFR_MASK;

	if (last_cmd == QUAD_OUT_READ_CMD)
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_QSPI;
	else
		gen_fifo_cmd |= ZYNQMP_QSPI_SPI_MODE_SPI;

	if (qspi->master.stripe)
		gen_fifo_cmd |= ZYNQMP_QSPI_GFIFO_STRIPE_MASK;

	if (!((u32)transfer->rx_buf & 0x3) && !(actuallen % 4)) {
		buf = (u32 *)transfer->rx_buf;
	} else {
		ALLOC_CACHE_ALIGN_BUFFER(u8, tmp, roundup(transfer->len, 4));
		buf = (u32 *)tmp;
	}
	writel((u32)buf, &zynqmp_qspi_dma->dmadst);
	writel(roundup(transfer->len, 4), &zynqmp_qspi_dma->dmasize);
	writel(ZYNQMP_QSPI_DMA_DST_I_STS_MASK, &zynqmp_qspi_dma->dmaier);
	addr = (u32)buf;
	size = roundup(transfer->len, ARCH_DMA_MINALIGN);
	flush_dcache_range(addr, addr+size);

	while (transfer->len) {
		len = zynqmp_qspi_calc_exp(transfer, &gen_fifo_cmd);
		if (!(gen_fifo_cmd & ZYNQMP_QSPI_GFIFO_EXP_MASK) &&
		    (len % 4)) {
			gen_fifo_cmd &= ~(0xFF);
			gen_fifo_cmd |= (len/4 + 1) * 4;
		}
		writel(gen_fifo_cmd, &zynqmp_qspi_base->genfifo);

		debug("GFIFO_CMD_RX:0x%x\n", gen_fifo_cmd);
	}

	while (timeout) {
		if (readl(&zynqmp_qspi_dma->dmaisr) &
		    ZYNQMP_QSPI_DMA_DST_I_STS_DONE) {
			writel(ZYNQMP_QSPI_DMA_DST_I_STS_DONE,
			       &zynqmp_qspi_dma->dmaisr);
			break;
		}
		timeout--;
	}

	debug("buf:0x%lx, txbuf:0x%lx, *buf:0x%x len: 0x%x\n",
	      (unsigned long)buf, (unsigned long)transfer->rx_buf, *buf,
	      actuallen);
	if (!timeout) {
		printf("DMA Timeout:0x%x\n", readl(&zynqmp_qspi_dma->dmaisr));
		return -1;
	}

	if (buf != transfer->rx_buf)
		memcpy(transfer->rx_buf, buf, actuallen);

	return 0;
}

static int zynqmp_qspi_start_transfer(struct spi_device *qspi,
			struct spi_transfer *transfer)
{
	int ret = 0;

	if (qspi->master.is_inst) {
		if (transfer->tx_buf)
			zynqmp_qspi_genfifo_cmd(qspi, transfer);
		else
			ret = -1;
	} else {
		if (transfer->tx_buf)
			ret = zynqmp_qspi_genfifo_fill_tx(qspi, transfer);
		else if (transfer->rx_buf)
			ret = zynqmp_qspi_genfifo_fill_rx(qspi, transfer);
		else
			ret = -1;
	}
	return ret;
}

static int zynqmp_qspi_check_is_dual_flash(void)
{
	int is_dual = -1;
	int lower_mio = 0, upper_mio = 0, upper_mio_cs1 = 0;

	lower_mio = zynq_slcr_get_mio_pin_status("qspi0");
	if (lower_mio == ZYNQMP_QSPI_MIO_NUM_QSPI0)
		is_dual = SF_SINGLE_FLASH;

	upper_mio_cs1 = zynq_slcr_get_mio_pin_status("qspi1_cs");
	if ((lower_mio == ZYNQMP_QSPI_MIO_NUM_QSPI0) &&
	    (upper_mio_cs1 == ZYNQMP_QSPI_MIO_NUM_QSPI1_CS))
		is_dual = SF_DUAL_STACKED_FLASH;

	upper_mio = zynq_slcr_get_mio_pin_status("qspi1");
	if ((lower_mio == ZYNQMP_QSPI_MIO_NUM_QSPI0) &&
	    (upper_mio_cs1 == ZYNQMP_QSPI_MIO_NUM_QSPI1_CS) &&
	    (upper_mio == ZYNQMP_QSPI_MIO_NUM_QSPI1))
		is_dual = SF_DUAL_PARALLEL_FLASH;

	return is_dual;
}

static int zynqmp_qspi_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	struct zynqmp_qspi *zqspi = &qspi->master;
	static unsigned cs_change = 1;
	int status = 0;

	debug("%s\n", __func__);

	while (1) {
		if (transfer->speed_hz) {
			status = zynqmp_qspi_setup_transfer(qspi, transfer);
			if (status < 0)
				break;
		}

		/* Select the chip if required */
		if (cs_change)
			zynqmp_qspi_chipselect(qspi, 1);

		cs_change = transfer->cs_change;

		if (!transfer->tx_buf && !transfer->rx_buf && transfer->len) {
			status = -1;
			break;
		}

		/* Request the transfer */
		if (transfer->len) {
			status = zynqmp_qspi_start_transfer(qspi, transfer);
			zqspi->is_inst = 0;
			if (status < 0)
				break;
		}

		if (transfer->delay_usecs)
			udelay(transfer->delay_usecs);

		if (cs_change)
			/* Deselect the chip */
			zynqmp_qspi_chipselect(qspi, 0);
		break;
	}

	zynqmp_qspi_setup_transfer(qspi, NULL);

	return status;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* 1 bus with 2 chipselect */
	return bus == 0 && cs < 2;
}

void spi_cs_activate(struct spi_slave *slave)
{
	debug("%s: slave 0x%08lx\n", __func__, (unsigned long)slave);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	debug("%s: slave 0x%08lx\n", __func__, (unsigned long)slave);
}

void spi_init(void)
{
	debug("%s\n", __func__);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	int is_dual = SF_SINGLE_FLASH;
	struct zynqmp_qspi_slave *qspi;

	debug("%s: bus: %d cs: %d max_hz: %d mode: %d\n",
	      __func__, bus, cs, max_hz, mode);

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	is_dual = zynqmp_qspi_check_is_dual_flash();

	if (is_dual == -1) {
		printf("%s: No QSPI device detected based on MIO settings\n",
		       __func__);
		return NULL;
	}

	zynqmp_qspi_init_hw(is_dual, cs);

	qspi = spi_alloc_slave(struct zynqmp_qspi_slave, bus, cs);
	if (!qspi) {
		printf("%s: Fail to allocate zynqmp_qspi_slave\n", __func__);
		return NULL;
	}

	debug("Defaulting to 200000000 Hz qspi clk");
	qspi->qspi.master.input_clk_hz = 200000000;

	qspi->slave.option = is_dual;
	qspi->slave.op_mode_rx = SPI_OPM_RX_QOF;
	qspi->slave.op_mode_tx = SPI_OPM_TX_QPP;
	qspi->qspi.master.speed_hz = qspi->qspi.master.input_clk_hz / 2;
	qspi->qspi.max_speed_hz = (max_hz < qspi->qspi.master.speed_hz) ?
				  max_hz : qspi->qspi.master.speed_hz;
	qspi->qspi.master.is_dual = is_dual;
	qspi->qspi.mode = mode;
	qspi->qspi.chip_select = 0;
	zynqmp_qspi_setup_transfer(&qspi->qspi, NULL);

	return &qspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct zynqmp_qspi_slave *qspi;

	debug("%s: slave: 0x%08lx\n", __func__, (unsigned long)slave);

	qspi = to_zynqmp_qspi_slave(slave);
	free(qspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	debug("%s: slave: 0x%08lx\n", __func__, (unsigned long)slave);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	debug("%s: slave: 0x%08lx\n", __func__, (unsigned long)slave);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct zynqmp_qspi_slave *qspi;
	struct spi_transfer transfer;

	debug("%s: slave: 0x%08lx bitlen: %d dout: 0x%08lx ", __func__,
	      (unsigned long)slave, bitlen, (unsigned long)dout);
	debug("din: 0x%08lx flags: 0x%lx\n", (unsigned long)din, flags);

	qspi = (struct zynqmp_qspi_slave *)slave;
	transfer.tx_buf = dout;
	transfer.rx_buf = din;
	transfer.len = bitlen / 8;

	/*
	 * Festering sore.
	 * Assume that the beginning of a transfer with bits to
	 * transmit must contain a device command.
	 */
	if (dout && flags & SPI_XFER_BEGIN)
		qspi->qspi.master.is_inst = 1;
	else
		qspi->qspi.master.is_inst = 0;

	if (flags & SPI_XFER_END)
		transfer.cs_change = 1;
	else
		transfer.cs_change = 0;

	if (flags & SPI_XFER_U_PAGE)
		qspi->qspi.master.u_page = 1;
	else
		qspi->qspi.master.u_page = 0;

	qspi->qspi.master.stripe = 0;
	qspi->qspi.master.bus = 0;
	if (qspi->slave.option == SF_DUAL_PARALLEL_FLASH) {
		qspi->qspi.master.is_dual = SF_DUAL_PARALLEL_FLASH;
		if (flags & SPI_XFER_MASK)
			qspi->qspi.master.bus = (flags & SPI_XFER_MASK) >> 8;
		if (flags & SPI_XFER_STRIPE)
			qspi->qspi.master.stripe = 1;
	}

	transfer.delay_usecs = 0;
	transfer.speed_hz = qspi->qspi.max_speed_hz;

	zynqmp_qspi_transfer(&qspi->qspi, &transfer);

	return 0;
}
