/*
 * (C) Copyright 2011 - 2013 Xilinx
 *
 * Xilinx Zynq Quad-SPI(QSPI) controller driver (master mode only)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <dm.h>
#include <ubi_uboot.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clk.h>
#include "../mtd/spi/sf_internal.h"

DECLARE_GLOBAL_DATA_PTR;

/* QSPI Transmit Data Register */
#define ZYNQ_QSPI_TXD_00_00_OFFSET	0x1C /* Transmit 4-byte inst, WO */
#define ZYNQ_QSPI_TXD_00_01_OFFSET	0x80 /* Transmit 1-byte inst, WO */
#define ZYNQ_QSPI_TXD_00_10_OFFSET	0x84 /* Transmit 2-byte inst, WO */
#define ZYNQ_QSPI_TXD_00_11_OFFSET	0x88 /* Transmit 3-byte inst, WO */

/*
 * QSPI Configuration Register bit Masks
 *
 * This register contains various control bits that effect the operation
 * of the QSPI controller
 */
#define ZYNQ_QSPI_CONFIG_IFMODE_MASK	(1 << 31)  /* Flash intrface mode*/
#define ZYNQ_QSPI_CONFIG_MSA_MASK	(1 << 15)  /* Manual start enb */
#define ZYNQ_QSPI_CONFIG_MCS_MASK	(1 << 14)  /* Manual chip select */
#define ZYNQ_QSPI_CONFIG_PCS_MASK	(1 << 10)  /* Peri chip select */
#define ZYNQ_QSPI_CONFIG_FW_MASK	(0x3 << 6) /* FIFO width */
#define ZYNQ_QSPI_CONFIG_BAUD_DIV_MASK	(0x7 << 3) /* Baud rate div */
#define ZYNQ_QSPI_CONFIG_MSTREN_MASK	(1 << 0)   /* Mode select */
#define ZYNQ_QSPI_CONFIG_MANSRT_MASK	0x00010000 /* Manual TX Start */
#define ZYNQ_QSPI_CONFIG_CPHA_MASK	0x00000004 /* Clock Phase Control */
#define ZYNQ_QSPI_CONFIG_CPOL_MASK	0x00000002 /* Clock Polarity Control */
#define ZYNQ_QSPI_CONFIG_SSCTRL_MASK	0x00003C00 /* Slave Select Mask */
/*
 * QSPI Interrupt Registers bit Masks
 *
 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
 * bit definitions.
 */
#define ZYNQ_QSPI_IXR_TXNFULL_MASK	0x00000004 /* QSPI TX FIFO Overflow */
#define ZYNQ_QSPI_IXR_TXFULL_MASK	0x00000008 /* QSPI TX FIFO is full */
#define ZYNQ_QSPI_IXR_RXNEMTY_MASK	0x00000010 /* QSPI RX FIFO Not Empty */
#define ZYNQ_QSPI_IXR_ALL_MASK		(ZYNQ_QSPI_IXR_TXNFULL_MASK | \
					ZYNQ_QSPI_IXR_RXNEMTY_MASK)

/*
 * QSPI Enable Register bit Masks
 *
 * This register is used to enable or disable the QSPI controller
 */
#define ZYNQ_QSPI_ENABLE_ENABLE_MASK	0x00000001 /* QSPI Enable Bit Mask */

/*
 * QSPI Linear Configuration Register
 *
 * It is named Linear Configuration but it controls other modes when not in
 * linear mode also.
 */
#define ZYNQ_QSPI_LCFG_TWO_MEM_MASK	0x40000000 /* QSPI Enable Bit Mask */
#define ZYNQ_QSPI_LCFG_SEP_BUS_MASK	0x20000000 /* QSPI Enable Bit Mask */
#define ZYNQ_QSPI_LCFG_U_PAGE		0x10000000 /* QSPI Upper memory set */

#define ZYNQ_QSPI_LCFG_DUMMY_SHIFT	8

#define ZYNQ_QSPI_FR_QOUT_CODE	0x6B	/* read instruction code */
#define ZYNQ_QSPI_FR_DUALIO_CODE	0xBB

/*
 * The modebits configurable by the driver to make the SPI support different
 * data formats
 */
#define MODEBITS			(SPI_CPOL | SPI_CPHA)

/* Definitions for the status of queue */
#define ZYNQ_QSPI_QUEUE_STOPPED		0
#define ZYNQ_QSPI_QUEUE_RUNNING		1
#define ZYNQ_QSPI_RXFIFO_THRESHOLD	32
#define ZYNQ_QSPI_FIFO_DEPTH		63

/* QSPI MIO's count for different connection topologies */
#define ZYNQ_QSPI_MIO_NUM_QSPI0		6
#define ZYNQ_QSPI_MIO_NUM_QSPI1		5
#define ZYNQ_QSPI_MIO_NUM_QSPI1_CS	1

/* QSPI MIO's count for different connection topologies */
#define ZYNQ_QSPI_MIO_NUM_QSPI0_DIO	4
#define ZYNQ_QSPI_MIO_NUM_QSPI1_DIO	3
#define ZYNQ_QSPI_MIO_NUM_QSPI1_CS_DIO	1

#define ZYNQ_QSPI_MAX_BAUD_RATE		0x7
#define ZYNQ_QSPI_DEFAULT_BAUD_RATE	0x2

/* QSPI register offsets */
struct zynq_qspi_regs {
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
	u32 reserved0[19];
	u32 txd1r;	/* 0x80 */
	u32 txd2r;	/* 0x84 */
	u32 txd3r;	/* 0x88 */
	u32 reserved1[5];
	u32 lcr;	/* 0xA0 */
	u32 reserved2[22];
	u32 midr;	/* 0xFC */
};


struct zynq_qspi_platdata {
	struct zynq_qspi_regs *regs;
	u32 frequency;          /* input frequency */
	u32 speed_hz;
	u32 is_dual;
	u32 tx_rx_mode;
};

struct zynq_qspi_priv {
	struct zynq_qspi_regs *regs;
	u8 mode;
	u32 freq;
	const void *txbuf;
	void *rxbuf;
	unsigned len;
	int bytes_to_transfer;
	int bytes_to_receive;
	unsigned int is_inst;
	unsigned int is_dual;
        unsigned int is_dio;
        unsigned int u_page;
	unsigned cs_change:1;
};

static int zynq_qspi_ofdata_to_platdata(struct udevice *bus)
{
	struct zynq_qspi_platdata *plat = bus->platdata;
	int is_dual;
	u32 mode = 0;
	int offset;
	u32 value;

	debug("%s\n", __func__);
	plat->regs = (struct zynq_qspi_regs *)ZYNQ_QSPI_BASEADDR;

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

	value = fdtdec_get_uint(gd->fdt_blob, offset, "spi-tx-bus-width", 1);
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

	plat->frequency = 166666666;
	plat->speed_hz = plat->frequency / 2;

	return 0;
}

/*
 * zynq_qspi_init_hw - Initialize the hardware
 * @is_dual:		Indicates whether dual memories are used
 * @cs:			Indicates which chip select is used in dual stacked
 *
 * The default settings of the QSPI controller's configurable parameters on
 * reset are
 *	- Master mode
 *	- Baud rate divisor is set to 2
 *	- Threshold value for TX FIFO not full interrupt is set to 1
 *	- Flash memory interface mode enabled
 *	- Size of the word to be transferred as 8 bit
 * This function performs the following actions
 *	- Disable and clear all the interrupts
 *	- Enable manual slave select
 *	- Enable auto start
 *	- Deselect all the chip select lines
 *	- Set the size of the word to be transferred as 32 bit
 *	- Set the little endian mode of TX FIFO and
 *	- Enable the QSPI controller
 */
static void zynq_qspi_init_hw(struct zynq_qspi_priv *priv)
{
	u32 config_reg;
	struct zynq_qspi_regs *regs = priv->regs;

	writel(~ZYNQ_QSPI_ENABLE_ENABLE_MASK, &regs->enbr);
	writel(0x7F, &regs->idisr);

	/* Disable linear mode as the boot loader may have used it */
	writel(0x0, &regs->lcr);

	/* Clear the TX and RX threshold reg */
	writel(0x1, &regs->txftr);
	writel(ZYNQ_QSPI_RXFIFO_THRESHOLD, &regs->rxftr);

	/* Clear the RX FIFO */
	while (readl(&regs->isr) & ZYNQ_QSPI_IXR_RXNEMTY_MASK)
		readl(&regs->drxr);

	debug("%s is_dual:0x%x, is_dio:0x%x\n", __func__, priv->is_dual, priv->is_dio);

	writel(0x7F, &regs->isr);
	config_reg = readl(&regs->confr);
	config_reg &= ~ZYNQ_QSPI_CONFIG_MSA_MASK;
	config_reg |= ZYNQ_QSPI_CONFIG_IFMODE_MASK |
		ZYNQ_QSPI_CONFIG_MCS_MASK | ZYNQ_QSPI_CONFIG_PCS_MASK |
		ZYNQ_QSPI_CONFIG_FW_MASK | ZYNQ_QSPI_CONFIG_MSTREN_MASK;
	if (priv->is_dual == SF_DUAL_STACKED_FLASH)
		config_reg |= 0x10;
	writel(config_reg, &regs->confr);

	if (priv->is_dual == SF_DUAL_PARALLEL_FLASH) {
		if (priv->is_dio == SF_DUALIO_FLASH)
			/* Enable two memories on seperate buses */
			writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
				ZYNQ_QSPI_LCFG_SEP_BUS_MASK |
				(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
				ZYNQ_QSPI_FR_DUALIO_CODE),
				&regs->lcr);
		else
			/* Enable two memories on seperate buses */
			writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
				ZYNQ_QSPI_LCFG_SEP_BUS_MASK |
				(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
				ZYNQ_QSPI_FR_QOUT_CODE),
				&regs->lcr);
	} else if (priv->is_dual == SF_DUAL_STACKED_FLASH) {
		if (priv->is_dio == SF_DUALIO_FLASH)
			/* Configure two memories on shared bus
			 * by enabling lower mem */
			writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
				(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
				ZYNQ_QSPI_FR_DUALIO_CODE),
				&regs->lcr);
		else
			/* Configure two memories on shared bus
			 * by enabling lower mem */
			writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
				(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
				ZYNQ_QSPI_FR_QOUT_CODE),
				&regs->lcr);
	}
	writel(ZYNQ_QSPI_ENABLE_ENABLE_MASK, &regs->enbr);
}

static int zynq_qspi_child_pre_probe(struct udevice *bus)
{
	struct spi_slave *slave = dev_get_parent_priv(bus);
	struct zynq_qspi_priv *priv = dev_get_priv(bus->parent);
	struct zynq_qspi_platdata *plat = dev_get_platdata(bus->parent);

	slave->option = priv->is_dual;
	slave->dio = priv->is_dio;
	slave->mode = plat->tx_rx_mode;

	return 0;
}

static int zynq_qspi_probe(struct udevice *bus)
{
	struct zynq_qspi_platdata *plat = dev_get_platdata(bus);
	struct zynq_qspi_priv *priv = dev_get_priv(bus);

	debug("zynq_qspi_probe:  bus:%p, priv:%p \n", bus, priv);

	priv->regs = plat->regs;
	priv->is_dual = plat->is_dual;

	if (priv->is_dual == -1) {
		debug("%s: No QSPI device detected based on MIO settings\n",
		      __func__);
		return -1;
	}

	/* init the zynq spi hw */
	zynq_qspi_init_hw(priv);

	return 0;
}

static int zynq_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct zynq_qspi_platdata *plat = bus->platdata;
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;
	uint32_t confr;
	u8 baud_rate_val = 0;

	debug("%s\n", __func__);
	if (speed > plat->frequency)
		speed = plat->frequency;

	/* Set the clock frequency */
	confr = readl(&regs->confr);
	if (speed == 0) {
		/* Set baudrate x8, if the freq is 0 */
		baud_rate_val = 0x2;
	} else if (plat->speed_hz != speed) {
		while ((baud_rate_val < 8) &&
		       ((plat->frequency /
		       (2 << baud_rate_val)) > speed))
			baud_rate_val++;

		if (baud_rate_val > ZYNQ_QSPI_MAX_BAUD_RATE)
			baud_rate_val = ZYNQ_QSPI_DEFAULT_BAUD_RATE;

		plat->speed_hz = speed / (2 << baud_rate_val);
	}
	confr &= ~ZYNQ_QSPI_CONFIG_BAUD_DIV_MASK;
	confr |= (baud_rate_val << 3);

	writel(confr, &regs->confr);
	priv->freq = speed;

	debug("zynq_spi_set_speed: regs=%p, mode=%d\n", priv->regs, priv->freq);

	return 0;
}

static int zynq_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;
	uint32_t confr;

	debug("%s\n", __func__);
	/* Set the SPI Clock phase and polarities */
	confr = readl(&regs->confr);
	confr &= ~(ZYNQ_QSPI_CONFIG_CPHA_MASK | ZYNQ_QSPI_CONFIG_CPOL_MASK);

	if (priv->mode & SPI_CPHA)
		confr |= ZYNQ_QSPI_CONFIG_CPHA_MASK;
	if (priv->mode & SPI_CPOL)
		confr |= ZYNQ_QSPI_CONFIG_CPOL_MASK;

	writel(confr, &regs->confr);
	priv->mode = mode;

	debug("zynq_spi_set_mode: regs=%p, mode=%d\n", priv->regs, priv->mode);

	return 0;
}

/*
 * zynq_qspi_copy_read_data - Copy data to RX buffer
 * @zqspi:	Pointer to the zynq_qspi structure
 * @data:	The 32 bit variable where data is stored
 * @size:	Number of bytes to be copied from data to RX buffer
 */
static void zynq_qspi_copy_read_data(struct zynq_qspi_priv *priv, u32 data, u8 size)
{
	u8 byte3;

	debug("%s: data 0x%04x rxbuf addr: 0x%08x size %d\n", __func__ ,
	      data, (unsigned)(priv->rxbuf), size);

	if (priv->rxbuf) {
		switch (size) {
		case 1:
			*((u8 *)priv->rxbuf) = data;
			priv->rxbuf += 1;
			break;
		case 2:
			*((u8 *)priv->rxbuf) = data;
			priv->rxbuf += 1;
			*((u8 *)priv->rxbuf) = (u8)(data >> 8);
			priv->rxbuf += 1;
			break;
		case 3:
			*((u8 *)priv->rxbuf) = data;
			priv->rxbuf += 1;
			*((u8 *)priv->rxbuf) = (u8)(data >> 8);
			priv->rxbuf += 1;
			byte3 = (u8)(data >> 16);
			*((u8 *)priv->rxbuf) = byte3;
			priv->rxbuf += 1;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(priv->rxbuf, &data, size);
			priv->rxbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	}
	priv->bytes_to_receive -= size;
	if (priv->bytes_to_receive < 0)
		priv->bytes_to_receive = 0;
}

/*
 * zynq_qspi_copy_write_data - Copy data from TX buffer
 * @zqspi:	Pointer to the zynq_qspi structure
 * @data:	Pointer to the 32 bit variable where data is to be copied
 * @size:	Number of bytes to be copied from TX buffer to data
 */
static void zynq_qspi_copy_write_data(struct  zynq_qspi_priv *priv,
		u32 *data, u8 size)
{
	if (priv->txbuf) {
		switch (size) {
		case 1:
			*data = *((u8 *)priv->txbuf);
			priv->txbuf += 1;
			*data |= 0xFFFFFF00;
			break;
		case 2:
			*data = *((u8 *)priv->txbuf);
			priv->txbuf += 1;
			*data |= (*((u8 *)priv->txbuf) << 8);
			priv->txbuf += 1;
			*data |= 0xFFFF0000;
			break;
		case 3:
			*data = *((u8 *)priv->txbuf);
			priv->txbuf += 1;
			*data |= (*((u8 *)priv->txbuf) << 8);
			priv->txbuf += 1;
			*data |= (*((u8 *)priv->txbuf) << 16);
			priv->txbuf += 1;
			*data |= 0xFF000000;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(data, priv->txbuf, size);
			priv->txbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	} else {
		*data = 0;
	}

	debug("%s: data 0x%08x txbuf addr: 0x%08x size %d\n", __func__,
	      *data, (u32)priv->txbuf, size);

	priv->bytes_to_transfer -= size;
	if (priv->bytes_to_transfer < 0)
		priv->bytes_to_transfer = 0;
}

/*
 * zynq_qspi_chipselect - Select or deselect the chip select line
 * @qspi:	Pointer to the spi_device structure
 * @is_on:	Select(1) or deselect (0) the chip select line
 */
static void zynq_qspi_chipselect(struct  zynq_qspi_priv *priv, int is_on)
{
	u32 config_reg;
	struct zynq_qspi_regs *regs = priv->regs;

	debug("%s: is_on: %d\n", __func__, is_on);

	config_reg = readl(&regs->confr);

	if (is_on) {
		/* Select the slave */
		config_reg &= ~ZYNQ_QSPI_CONFIG_SSCTRL_MASK;
		config_reg |= (((~(0x0001 << 0)) << 10) &
				ZYNQ_QSPI_CONFIG_SSCTRL_MASK);
	} else
		/* Deselect the slave */
		config_reg |= ZYNQ_QSPI_CONFIG_SSCTRL_MASK;

	writel(config_reg, &regs->confr);
}

/*
 * zynq_qspi_fill_tx_fifo - Fills the TX FIFO with as many bytes as possible
 * @zqspi:	Pointer to the zynq_qspi structure
 */
static void zynq_qspi_fill_tx_fifo(struct zynq_qspi_priv *priv, u32 size)
{
	u32 data = 0;
	u32 fifocount = 0;
	unsigned len, offset;
	struct zynq_qspi_regs *regs = priv->regs;
	static const unsigned offsets[4] = {
		ZYNQ_QSPI_TXD_00_00_OFFSET, ZYNQ_QSPI_TXD_00_01_OFFSET,
		ZYNQ_QSPI_TXD_00_10_OFFSET, ZYNQ_QSPI_TXD_00_11_OFFSET };

	while ((fifocount < size) &&
			(priv->bytes_to_transfer > 0)) {
		if (priv->bytes_to_transfer >= 4) {
			if (priv->txbuf) {
				memcpy(&data, priv->txbuf, 4);
				priv->txbuf += 4;
			} else {
				data = 0;
			}
			writel(data, &regs->txd0r);
			priv->bytes_to_transfer -= 4;
			fifocount++;
		} else {
			/* Write TXD1, TXD2, TXD3 only if TxFIFO is empty. */
			if (!(readl(&regs->isr)
					& ZYNQ_QSPI_IXR_TXNFULL_MASK) &&
					!priv->rxbuf)
				return;
			len = priv->bytes_to_transfer;
			zynq_qspi_copy_write_data(priv, &data, len);
			offset = (priv->rxbuf) ? offsets[0] : offsets[len];
			writel(data, &regs->confr + (offset / 4));
		}
	}
}

/*
 * zynq_qspi_irq_poll - Interrupt service routine of the QSPI controller
 * @zqspi:	Pointer to the zynq_qspi structure
 *
 * This function handles TX empty and Mode Fault interrupts only.
 * On TX empty interrupt this function reads the received data from RX FIFO and
 * fills the TX FIFO if there is any data remaining to be transferred.
 * On Mode Fault interrupt this function indicates that transfer is completed,
 * the SPI subsystem will identify the error as the remaining bytes to be
 * transferred is non-zero.
 *
 * returns:	0 for poll timeout
 *		1 transfer operation complete
 */
static int zynq_qspi_irq_poll(struct zynq_qspi_priv *priv)
{
	int max_loop;
	u32 intr_status;
	u32 rxindex = 0;
	u32 rxcount;
	struct zynq_qspi_regs *regs = priv->regs;

	debug("%s: zqspi: 0x%08x\n", __func__, (u32)priv);

	/* Poll until any of the interrupt status bits are set */
	max_loop = 0;
	do {
		intr_status = readl(&regs->isr);
		max_loop++;
	} while ((intr_status == 0) && (max_loop < 100000));

	if (intr_status == 0) {
		debug("%s: Timeout\n", __func__);
		return 0;
	}

	writel(intr_status, &regs->isr);

	/* Disable all interrupts */
	writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->idisr);
	if ((intr_status & ZYNQ_QSPI_IXR_TXNFULL_MASK) ||
	    (intr_status & ZYNQ_QSPI_IXR_RXNEMTY_MASK)) {
		/*
		 * This bit is set when Tx FIFO has < THRESHOLD entries. We have
		 * the THRESHOLD value set to 1, so this bit indicates Tx FIFO
		 * is empty
		 */
		rxcount = priv->bytes_to_receive - priv->bytes_to_transfer;
		rxcount = (rxcount % 4) ? ((rxcount/4)+1) : (rxcount/4);
		while ((rxindex < rxcount) &&
				(rxindex < ZYNQ_QSPI_RXFIFO_THRESHOLD)) {
			/* Read out the data from the RX FIFO */
			u32 data;
			data = readl(&regs->drxr);

			if (priv->bytes_to_receive >= 4) {
				if (priv->rxbuf) {
					memcpy(priv->rxbuf, &data, 4);
					priv->rxbuf += 4;
				}
				priv->bytes_to_receive -= 4;
			} else {
				zynq_qspi_copy_read_data(priv, data,
					priv->bytes_to_receive);
			}
			rxindex++;
		}

		if (priv->bytes_to_transfer) {
			/* There is more data to send */
			zynq_qspi_fill_tx_fifo(priv,
					       ZYNQ_QSPI_RXFIFO_THRESHOLD);

			writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->ier);
		} else {
			/*
			 * If transfer and receive is completed then only send
			 * complete signal
			 */
			if (!priv->bytes_to_receive) {
				/* return operation complete */
				writel(ZYNQ_QSPI_IXR_ALL_MASK,
				       &regs->idisr);
				return 1;
			}
		}
	}

	return 0;
}

/*
 * zynq_qspi_start_transfer - Initiates the QSPI transfer
 * @qspi:	Pointer to the spi_device structure
 * @transfer:	Pointer to the spi_transfer structure which provide information
 *		about next transfer parameters
 *
 * This function fills the TX FIFO, starts the QSPI transfer, and waits for the
 * transfer to be completed.
 *
 * returns:	Number of bytes transferred in the last transfer
 */
static int zynq_qspi_start_transfer(struct zynq_qspi_priv *priv)
{
	static u8 current_u_page;
	u32 data = 0;
	struct zynq_qspi_regs *regs = priv->regs;

	debug("%s: qspi: 0x%08x transfer: 0x%08x len: %d\n", __func__,
	      (u32)priv, (u32)priv, priv->len);

	priv->bytes_to_transfer = priv->len;
	priv->bytes_to_receive = priv->len;

	if (priv->is_inst && (priv->is_dual == SF_DUAL_STACKED_FLASH) &&
	    (current_u_page != priv->u_page)) {
		if (priv->u_page) {
			if (priv->is_dio == SF_DUALIO_FLASH)
				writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
					ZYNQ_QSPI_LCFG_U_PAGE |
					(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
					ZYNQ_QSPI_FR_DUALIO_CODE),
					&regs->lcr);
			else
				/* Configure two memories on shared bus
				 * by enabling upper mem
				 */
				writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
					ZYNQ_QSPI_LCFG_U_PAGE |
					(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
					ZYNQ_QSPI_FR_QOUT_CODE),
					&regs->lcr);
		} else {
			if (priv->is_dio == SF_DUALIO_FLASH)
				writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
					(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
					ZYNQ_QSPI_FR_DUALIO_CODE),
					&regs->lcr);
			else
				/* Configure two memories on shared bus
				 * by enabling lower mem
				 */
				writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
					(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
					ZYNQ_QSPI_FR_QOUT_CODE),
					&regs->lcr);
		}
		current_u_page = priv->u_page;
	}

	if (priv->len < 4)
		zynq_qspi_fill_tx_fifo(priv, priv->len);
	else
		zynq_qspi_fill_tx_fifo(priv, ZYNQ_QSPI_FIFO_DEPTH);

	writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->ier);
	/* Start the transfer by enabling manual start bit */

	/* wait for completion */
	do {
		data = zynq_qspi_irq_poll(priv);
	} while (data == 0);

	return (priv->len) - (priv->bytes_to_transfer);
}

static int zynq_qspi_transfer(struct zynq_qspi_priv *priv)
{
	unsigned cs_change = 1;
	int status = 0;

	debug("%s\n", __func__);

	while (1) {

		/* Select the chip if required */
		if (cs_change)
			zynq_qspi_chipselect(priv, 1);

		cs_change = priv->cs_change;

		if (!priv->txbuf && !priv->rxbuf && priv->len) {
			status = -1;
			break;
		}

		/* Request the transfer */
		if (priv->len) {
			status = zynq_qspi_start_transfer(priv);
			priv->is_inst = 0;
		}

		if (status != priv->len) {
			if (status > 0)
				status = -EMSGSIZE;
			debug("zynq_qspi_transfer:%d len:%d\n", status, priv->len);
			break;
		}
		status = 0;

		if (cs_change)
			/* Deselect the chip */
			zynq_qspi_chipselect(priv, 0);

		break;
	}

	return 0;
}

static int zynq_qspi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;

	debug("%s\n", __func__);
	writel(ZYNQ_QSPI_ENABLE_ENABLE_MASK, &regs->enbr);

	return 0;
}

static int zynq_qspi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;

	debug("%s\n", __func__);
	writel(~ZYNQ_QSPI_ENABLE_ENABLE_MASK, &regs->enbr);

	return 0;
}

static int zynq_qspi_xfer(struct udevice *dev, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct zynq_qspi_priv *priv = dev_get_priv(bus);

	debug("%s", __func__);
	debug("%s: slave: 0x%08x bitlen: %d dout: 0x%08x ", __func__,
	      (u32)priv, bitlen, (u32)dout);
	debug("din: 0x%08x flags: 0x%lx\n", (u32)din, flags);

	priv->txbuf = dout;
	priv->rxbuf = din;
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

	zynq_qspi_transfer(priv);

	return 0;
}

static const struct dm_spi_ops zynq_qspi_ops = {
	.claim_bus      = zynq_qspi_claim_bus,
	.release_bus    = zynq_qspi_release_bus,
	.xfer           = zynq_qspi_xfer,
	.set_speed      = zynq_qspi_set_speed,
	.set_mode       = zynq_qspi_set_mode,
};

static const struct udevice_id zynq_qspi_ids[] = {
	{ .compatible = "xlnx,zynq-qspi-1.0" },
	{ }
};

U_BOOT_DRIVER(zynq_qspi) = {
	.name   = "zynq_qspi",
	.id     = UCLASS_SPI,
	.of_match = zynq_qspi_ids,
	.ops    = &zynq_qspi_ops,
	.ofdata_to_platdata = zynq_qspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct zynq_qspi_platdata),
	.priv_auto_alloc_size = sizeof(struct zynq_qspi_priv),
	.probe  = zynq_qspi_probe,
	.child_pre_probe = zynq_qspi_child_pre_probe,
};
