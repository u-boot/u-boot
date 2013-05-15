/*
 * (C) Copyright 2011 - 2013 Xilinx
 *
 * Xilinx PS Quad-SPI (QSPI) controller driver (master mode only)
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <ubi_uboot.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

/* QSPI Transmit Data Register */
#define XQSPIPS_TXD_00_00_OFFSET	0x1C /* Transmit 4-byte inst, WO */
#define XQSPIPS_TXD_00_01_OFFSET	0x80 /* Transmit 1-byte inst, WO */
#define XQSPIPS_TXD_00_10_OFFSET	0x84 /* Transmit 2-byte inst, WO */
#define XQSPIPS_TXD_00_11_OFFSET	0x88 /* Transmit 3-byte inst, WO */

/*
 * QSPI Configuration Register bit Masks
 *
 * This register contains various control bits that effect the operation
 * of the QSPI controller
 */
#define XQSPIPS_CONFIG_MANSRT_MASK	0x00010000 /* Manual TX Start */
#define XQSPIPS_CONFIG_CPHA_MASK	0x00000004 /* Clock Phase Control */
#define XQSPIPS_CONFIG_CPOL_MASK	0x00000002 /* Clock Polarity Control */
#define XQSPIPS_CONFIG_SSCTRL_MASK	0x00003C00 /* Slave Select Mask */

/*
 * QSPI Interrupt Registers bit Masks
 *
 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
 * bit definitions.
 */
#define XQSPIPS_IXR_TXNFULL_MASK	0x00000004 /* QSPI TX FIFO Overflow */
#define XQSPIPS_IXR_TXFULL_MASK		0x00000008 /* QSPI TX FIFO is full */
#define XQSPIPS_IXR_RXNEMTY_MASK	0x00000010 /* QSPI RX FIFO Not Empty */
#define XQSPIPS_IXR_ALL_MASK		(XQSPIPS_IXR_TXNFULL_MASK | \
					XQSPIPS_IXR_RXNEMTY_MASK)

/*
 * QSPI Enable Register bit Masks
 *
 * This register is used to enable or disable the QSPI controller
 */
#define XQSPIPS_ENABLE_ENABLE_MASK	0x00000001 /* QSPI Enable Bit Mask */

/*
 * QSPI Linear Configuration Register
 *
 * It is named Linear Configuration but it controls other modes when not in
 * linear mode also.
 */
#define XQSPIPS_LCFG_TWO_MEM_MASK	0x40000000 /* QSPI Enable Bit Mask */
#define XQSPIPS_LCFG_SEP_BUS_MASK	0x20000000 /* QSPI Enable Bit Mask */
#define XQSPIPS_LCFG_U_PAGE		0x10000000 /* QSPI Upper memory set */

#define XQSPIPS_LCFG_DUMMY_SHIFT	8

#define XQSPIPS_FAST_READ_QOUT_CODE	0x6B	/* read instruction code */

/*
 * The modebits configurable by the driver to make the SPI support different
 * data formats
 */
#define MODEBITS			(SPI_CPOL | SPI_CPHA)

/* Definitions for the status of queue */
#define XQSPIPS_QUEUE_STOPPED		0
#define XQSPIPS_QUEUE_RUNNING		1

/* QSPI MIO's count for different connection topologies */
#define XQSPIPS_MIO_NUM_QSPI0		6
#define XQSPIPS_MIO_NUM_QSPI1		5
#define XQSPIPS_MIO_NUM_QSPI1_CS	1

/* QSPI connections topology */
enum xqspips_con_topology {
	MODE_UNKNOWN = -1,
	MODE_SINGLE,
	MODE_DUAL_STACKED,
	MODE_DUAL_PARALLEL,
};

/* Definitions of the flash commands - Flash opcodes in ascending order */
#define XQSPIPS_FLASH_OPCODE_WRSR	0x01	/* Write status register */
#define XQSPIPS_FLASH_OPCODE_PP		0x02	/* Page program */
#define XQSPIPS_FLASH_OPCODE_NORM_READ	0x03	/* Normal read data bytes */
#define XQSPIPS_FLASH_OPCODE_WRDS	0x04	/* Write disable */
#define XQSPIPS_FLASH_OPCODE_RDSR1	0x05	/* Read status register 1 */
#define XQSPIPS_FLASH_OPCODE_WREN	0x06	/* Write enable */
#define XQSPIPS_FLASH_OPCODE_FAST_READ	0x0B	/* Fast read data bytes */
#define XQSPIPS_FLASH_OPCODE_BRRD	0x16	/* Bank address reg read */
#define XQSPIPS_FLASH_OPCODE_BRWR	0x17	/* Bank address reg write */
#define XQSPIPS_FLASH_OPCODE_BE_4K	0x20	/* Erase 4KiB block */
#define XQSPIPS_FLASH_OPCODE_RDSR2	0x35	/* Read status register 2 */
#define XQSPIPS_FLASH_OPCODE_DUAL_READ	0x3B	/* Dual read data bytes */
#define XQSPIPS_FLASH_OPCODE_BE_32K	0x52	/* Erase 32KiB block */
#define XQSPIPS_FLASH_OPCODE_QUAD_READ	0x6B	/* Quad read data bytes */
#define XQSPIPS_FLASH_OPCODE_ERASE_SUS	0x75	/* Erase suspend */
#define XQSPIPS_FLASH_OPCODE_ERASE_RES	0x7A	/* Erase resume */
#define XQSPIPS_FLASH_OPCODE_RDID	0x9F	/* Read JEDEC ID */
#define XQSPIPS_FLASH_OPCODE_WREAR	0xC5	/* Extended address reg write */
#define XQSPIPS_FLASH_OPCODE_RDEAR	0xC8	/* Extended address reg read */
#define XQSPIPS_FLASH_OPCODE_BE		0xC7	/* Erase whole flash block */
#define XQSPIPS_FLASH_OPCODE_SE		0xD8	/* Sector erase (usually 64KB)*/

/* Few mtd flash functions */
extern int spi_flash_cmd(struct spi_slave *spi, u8 cmd,
		void *response, size_t len);
extern int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/* QSPI register offsets */
struct xqspips_regs {
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

#define xqspips_base ((struct xqspips_regs *)ZYNQ_QSPI_BASEADDR)

struct xqspips {
	u32 input_clk_hz;
	u32 speed_hz;
	const void *txbuf;
	void *rxbuf;
	int bytes_to_transfer;
	int bytes_to_receive;
	struct xqspips_inst_format *curr_inst;
	u8 inst_response;
	unsigned int is_inst;
	unsigned int is_dual;
	unsigned int u_page;
};

struct spi_device {
	struct xqspips master;
	u32 max_speed_hz;
	u8 chip_select;
	u8 mode;
	u8 bits_per_word;
};

struct spi_transfer {
	const void *tx_buf;
	void *rx_buf;
	unsigned len;
	unsigned cs_change:1;
	u8 bits_per_word;
	u16 delay_usecs;
	u32 speed_hz;
};

struct zynq_spi_slave {
	struct spi_slave slave;
	struct spi_device qspi;
};
#define to_zynq_spi_slave(s) container_of(s, struct zynq_spi_slave, slave)

/*
 * struct xqspips_inst_format - Defines qspi flash instruction format
 * @opcode:		Operational code of instruction
 * @inst_size:		Size of the instruction including address bytes
 * @offset:		Register address where instruction has to be written
 */
struct xqspips_inst_format {
	u8 opcode;
	u8 inst_size;
	u8 offset;
};

/* List of all the QSPI instructions and its format */
static struct xqspips_inst_format flash_inst[] = {
	{ XQSPIPS_FLASH_OPCODE_WREN, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_WRDS, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_RDSR1, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_RDSR2, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_WRSR, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_PP, 4, XQSPIPS_TXD_00_00_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_SE, 4, XQSPIPS_TXD_00_00_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_BE_32K, 4, XQSPIPS_TXD_00_00_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_BE_4K, 4, XQSPIPS_TXD_00_00_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_BE, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_ERASE_SUS, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_ERASE_RES, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_RDID, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_NORM_READ, 4, XQSPIPS_TXD_00_00_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_FAST_READ, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_DUAL_READ, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_QUAD_READ, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_BRWR, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_BRRD, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_WREAR, 1, XQSPIPS_TXD_00_01_OFFSET },
	{ XQSPIPS_FLASH_OPCODE_RDEAR, 1, XQSPIPS_TXD_00_01_OFFSET },
	/* Add all the instructions supported by the flash device */
};

/*
 * xqspips_init_hw - Initialize the hardware
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
 *	- Enable manual start
 *	- Deselect all the chip select lines
 *	- Set the size of the word to be transferred as 32 bit
 *	- Set the little endian mode of TX FIFO and
 *	- Enable the QSPI controller
 */
static void xqspips_init_hw(int is_dual, unsigned int cs)
{
	u32 config_reg;

	writel(~XQSPIPS_ENABLE_ENABLE_MASK, &xqspips_base->enbr);
	writel(0x7F, &xqspips_base->idisr);

	/* Disable linear mode as the boot loader may have used it */
	writel(0x0, &xqspips_base->lcr);

	/* Clear the RX FIFO */
	while (readl(&xqspips_base->isr) & XQSPIPS_IXR_RXNEMTY_MASK)
		readl(&xqspips_base->drxr);

	writel(0x7F, &xqspips_base->isr);
	config_reg = readl(&xqspips_base->confr);
	config_reg &= 0xFBFFFFFF; /* Set little endian mode of TX FIFO */
	config_reg |= 0x8000FCC1;
	writel(config_reg, &xqspips_base->confr);

	if (is_dual == MODE_DUAL_PARALLEL)
		/* Enable two memories on seperate buses */
		writel((XQSPIPS_LCFG_TWO_MEM_MASK |
			XQSPIPS_LCFG_SEP_BUS_MASK |
			(1 << XQSPIPS_LCFG_DUMMY_SHIFT) |
			XQSPIPS_FAST_READ_QOUT_CODE),
			&xqspips_base->lcr);
	else if (is_dual == MODE_DUAL_STACKED)
		/* Configure two memories on shared bus by enabling lower mem */
		writel((XQSPIPS_LCFG_TWO_MEM_MASK |
			(1 << XQSPIPS_LCFG_DUMMY_SHIFT) |
			XQSPIPS_FAST_READ_QOUT_CODE),
			&xqspips_base->lcr);

	writel(XQSPIPS_ENABLE_ENABLE_MASK, &xqspips_base->enbr);
}

/*
 * xqspips_copy_read_data - Copy data to RX buffer
 * @xqspi:	Pointer to the xqspips structure
 * @data:	The 32 bit variable where data is stored
 * @size:	Number of bytes to be copied from data to RX buffer
 */
static void xqspips_copy_read_data(struct xqspips *xqspi, u32 data, u8 size)
{
	u8 byte3;

	debug("%s: data 0x%04x rxbuf addr: 0x%08x size %d\n", __func__ ,
	      data, (unsigned)(xqspi->rxbuf), size);

	if (xqspi->rxbuf) {
		switch (size) {
		case 1:
			*((u8 *)xqspi->rxbuf) = data;
			xqspi->rxbuf += 1;
			break;
		case 2:
			*((u16 *)xqspi->rxbuf) = data;
			xqspi->rxbuf += 2;
			break;
		case 3:
			*((u16 *)xqspi->rxbuf) = data;
			xqspi->rxbuf += 2;
			byte3 = (u8)(data >> 16);
			*((u8 *)xqspi->rxbuf) = byte3;
			xqspi->rxbuf += 1;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(xqspi->rxbuf, &data, size);
			xqspi->rxbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	}
	xqspi->bytes_to_receive -= size;
	if (xqspi->bytes_to_receive < 0)
		xqspi->bytes_to_receive = 0;
}

/*
 * xqspips_copy_write_data - Copy data from TX buffer
 * @xqspi:	Pointer to the xqspips structure
 * @data:	Pointer to the 32 bit variable where data is to be copied
 * @size:	Number of bytes to be copied from TX buffer to data
 */
static void xqspips_copy_write_data(struct xqspips *xqspi, u32 *data, u8 size)
{
	if (xqspi->txbuf) {
		switch (size) {
		case 1:
			*data = *((u8 *)xqspi->txbuf);
			xqspi->txbuf += 1;
			*data |= 0xFFFFFF00;
			break;
		case 2:
			*data = *((u16 *)xqspi->txbuf);
			xqspi->txbuf += 2;
			*data |= 0xFFFF0000;
			break;
		case 3:
			*data = *((u16 *)xqspi->txbuf);
			xqspi->txbuf += 2;
			*data |= (*((u8 *)xqspi->txbuf) << 16);
			xqspi->txbuf += 1;
			*data |= 0xFF000000;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(data, xqspi->txbuf, size);
			xqspi->txbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	} else {
		*data = 0;
	}

	debug("%s: data 0x%08x txbuf addr: 0x%08x size %d\n", __func__,
	      *data, (u32)xqspi->txbuf, size);

	xqspi->bytes_to_transfer -= size;
	if (xqspi->bytes_to_transfer < 0)
		xqspi->bytes_to_transfer = 0;
}

/*
 * xqspips_chipselect - Select or deselect the chip select line
 * @qspi:	Pointer to the spi_device structure
 * @is_on:	Select(1) or deselect (0) the chip select line
 */
static void xqspips_chipselect(struct spi_device *qspi, int is_on)
{
	u32 config_reg;

	debug("%s: is_on: %d\n", __func__, is_on);

	config_reg = readl(&xqspips_base->confr);

	if (is_on) {
		/* Select the slave */
		config_reg &= ~XQSPIPS_CONFIG_SSCTRL_MASK;
		config_reg |= (((~(0x0001 << qspi->chip_select)) << 10) &
				XQSPIPS_CONFIG_SSCTRL_MASK);
	} else
		/* Deselect the slave */
		config_reg |= XQSPIPS_CONFIG_SSCTRL_MASK;

	writel(config_reg, &xqspips_base->confr);
}

/*
 * xqspips_setup_transfer - Configure QSPI controller for specified transfer
 * @qspi:	Pointer to the spi_device structure
 * @transfer:	Pointer to the spi_transfer structure which provides information
 *		about next transfer setup parameters
 *
 * Sets the operational mode of QSPI controller for the next QSPI transfer and
 * sets the requested clock frequency.
 *
 * returns:	0 on success and -1 on invalid input parameter
 *
 * Note: If the requested frequency is not an exact match with what can be
 * obtained using the prescalar value, the driver sets the clock frequency which
 * is lower than the requested frequency (maximum lower) for the transfer. If
 * the requested frequency is higher or lower than that is supported by the QSPI
 * controller the driver will set the highest or lowest frequency supported by
 * controller.
 */
static int xqspips_setup_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	struct xqspips *xqspi = &qspi->master;
	u8 bits_per_word;
	u32 config_reg;
	u32 req_hz;
	u32 baud_rate_val = 0;

	debug("%s: qspi: 0x%08x transfer: 0x%08x\n", __func__,
	      (u32)qspi, (u32)transfer);

	bits_per_word = (transfer) ?
			transfer->bits_per_word : qspi->bits_per_word;
	req_hz = (transfer) ? transfer->speed_hz : qspi->max_speed_hz;

	if (qspi->mode & ~MODEBITS) {
		printf("%s: Unsupported mode bits %x\n",
		       __func__, qspi->mode & ~MODEBITS);
		return -1;
	}

	if (bits_per_word != 32)
		bits_per_word = 32;

	config_reg = readl(&xqspips_base->confr);

	/* Set the QSPI clock phase and clock polarity */
	config_reg &= (~XQSPIPS_CONFIG_CPHA_MASK) &
				(~XQSPIPS_CONFIG_CPOL_MASK);
	if (qspi->mode & SPI_CPHA)
		config_reg |= XQSPIPS_CONFIG_CPHA_MASK;
	if (qspi->mode & SPI_CPOL)
		config_reg |= XQSPIPS_CONFIG_CPOL_MASK;

	/* Set the clock frequency */
	if (xqspi->speed_hz != req_hz) {
		baud_rate_val = 0;
		while ((baud_rate_val < 8) &&
			(xqspi->input_clk_hz / (2 << baud_rate_val)) > req_hz) {
				baud_rate_val++;
		}
		config_reg &= 0xFFFFFFC7;
		config_reg |= (baud_rate_val << 3);
		xqspi->speed_hz = req_hz;
	}

	writel(config_reg, &xqspips_base->confr);

	debug("%s: mode %d, %u bits/w, %u clock speed\n", __func__,
	      qspi->mode & MODEBITS, qspi->bits_per_word, xqspi->speed_hz);

	return 0;
}

/*
 * xqspips_fill_tx_fifo - Fills the TX FIFO with as many bytes as possible
 * @xqspi:	Pointer to the xqspips structure
 */
static void xqspips_fill_tx_fifo(struct xqspips *xqspi)
{
	u32 data = 0;
	unsigned len, offset;
	static const unsigned offsets[4] = {
		XQSPIPS_TXD_00_00_OFFSET, XQSPIPS_TXD_00_01_OFFSET,
		XQSPIPS_TXD_00_10_OFFSET, XQSPIPS_TXD_00_11_OFFSET };

	while ((!(readl(&xqspips_base->isr) &
			XQSPIPS_IXR_TXFULL_MASK)) &&
			(xqspi->bytes_to_transfer > 0)) {
		if (xqspi->bytes_to_transfer < 4) {
			/* Write TXD1, TXD2, TXD3 only if TxFIFO is empty. */
			if (!(readl(&xqspips_base->isr)
					& XQSPIPS_IXR_TXNFULL_MASK) &&
					!xqspi->rxbuf)
				return;
			len = xqspi->bytes_to_transfer;
			xqspips_copy_write_data(xqspi, &data, len);
			offset = (xqspi->rxbuf) ? offsets[0] : offsets[len];
			writel(data, &xqspips_base->confr + (offset / 4));
		} else {
			xqspips_copy_write_data(xqspi, &data, 4);
			writel(data, &xqspips_base->txd0r);
		}
	}
}

/*
 * xqspips_irq_poll - Interrupt service routine of the QSPI controller
 * @xqspi:	Pointer to the xqspips structure
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
static int xqspips_irq_poll(struct xqspips *xqspi)
{
	int max_loop;
	u32 intr_status;

	debug("%s: xqspi: 0x%08x\n", __func__, (u32)xqspi);

	/* Poll until any of the interrupt status bits are set */
	max_loop = 0;
	do {
		intr_status = readl(&xqspips_base->isr);
		max_loop++;
	} while ((intr_status == 0) && (max_loop < 100000));

	if (intr_status == 0) {
		printf("%s: Timeout\n", __func__);
		return 0;
	}

	writel(intr_status, &xqspips_base->isr);

	/* Disable all interrupts */
	writel(XQSPIPS_IXR_ALL_MASK, &xqspips_base->idisr);
	if ((intr_status & XQSPIPS_IXR_TXNFULL_MASK) ||
	    (intr_status & XQSPIPS_IXR_RXNEMTY_MASK)) {
		/*
		 * This bit is set when Tx FIFO has < THRESHOLD entries. We have
		 * the THRESHOLD value set to 1, so this bit indicates Tx FIFO
		 * is empty
		 */
		u32 config_reg;

		/* Read out the data from the RX FIFO */
		while (readl(&xqspips_base->isr) &
				XQSPIPS_IXR_RXNEMTY_MASK) {
			u32 data;

			data = readl(&xqspips_base->drxr);

			if ((xqspi->inst_response) &&
			    (!((xqspi->curr_inst->opcode ==
				XQSPIPS_FLASH_OPCODE_RDSR1) ||
			       (xqspi->curr_inst->opcode ==
				XQSPIPS_FLASH_OPCODE_RDSR2)))) {
				xqspi->inst_response = 0;
				xqspips_copy_read_data(xqspi, data,
						xqspi->curr_inst->inst_size);
			} else if (xqspi->bytes_to_receive < 4) {
				xqspips_copy_read_data(xqspi, data,
						       xqspi->bytes_to_receive);
			} else {
				xqspips_copy_read_data(xqspi, data, 4);
			}
		}

		if (xqspi->bytes_to_transfer) {
			/* There is more data to send */
			xqspips_fill_tx_fifo(xqspi);

			writel(XQSPIPS_IXR_ALL_MASK, &xqspips_base->ier);

			config_reg = readl(&xqspips_base->confr);

			config_reg |= XQSPIPS_CONFIG_MANSRT_MASK;
			writel(config_reg, &xqspips_base->confr);
		} else {
			/*
			 * If transfer and receive is completed then only send
			 * complete signal
			 */
			if (!xqspi->bytes_to_receive) {
				/* return operation complete */
				writel(XQSPIPS_IXR_ALL_MASK,
				       &xqspips_base->idisr);
				return 1;
			}
		}
	}

	return 0;
}

/*
 * xqspips_start_transfer - Initiates the QSPI transfer
 * @qspi:	Pointer to the spi_device structure
 * @transfer:	Pointer to the spi_transfer structure which provide information
 *		about next transfer parameters
 *
 * This function fills the TX FIFO, starts the QSPI transfer, and waits for the
 * transfer to be completed.
 *
 * returns:	Number of bytes transferred in the last transfer
 */
static int xqspips_start_transfer(struct spi_device *qspi,
			struct spi_transfer *transfer)
{
	struct xqspips *xqspi = &qspi->master;
	static u8 current_u_page;
	u32 config_reg;
	u32 data = 0;
	u8 instruction = 0;
	u8 index;

	debug("%s: qspi: 0x%08x transfer: 0x%08x len: %d\n", __func__,
	      (u32)qspi, (u32)transfer, transfer->len);

	xqspi->txbuf = transfer->tx_buf;
	xqspi->rxbuf = transfer->rx_buf;
	xqspi->bytes_to_transfer = transfer->len;
	xqspi->bytes_to_receive = transfer->len;

	if (xqspi->txbuf)
		instruction = *(u8 *)xqspi->txbuf;

	if (instruction && xqspi->is_inst) {
		for (index = 0; index < ARRAY_SIZE(flash_inst); index++)
			if (instruction == flash_inst[index].opcode)
				break;

		/*
		 * Instruction might have already been transmitted. This is a
		 * 'data only' transfer
		 */
		if (index == ARRAY_SIZE(flash_inst))
			goto xfer_data;

		xqspi->curr_inst = &flash_inst[index];
		xqspi->inst_response = 1;

		if ((xqspi->is_dual == MODE_DUAL_STACKED) &&
				(current_u_page != xqspi->u_page)) {
			if (xqspi->u_page) {
				/* Configure two memories on shared bus
				 * by enabling upper mem
				 */
				writel((XQSPIPS_LCFG_TWO_MEM_MASK |
					XQSPIPS_LCFG_U_PAGE |
					(1 << XQSPIPS_LCFG_DUMMY_SHIFT) |
					XQSPIPS_FAST_READ_QOUT_CODE),
					&xqspips_base->lcr);
			} else {
				/* Configure two memories on shared bus
				 * by enabling lower mem
				 */
				writel((XQSPIPS_LCFG_TWO_MEM_MASK |
					(1 << XQSPIPS_LCFG_DUMMY_SHIFT) |
					XQSPIPS_FAST_READ_QOUT_CODE),
					&xqspips_base->lcr);
			}

			current_u_page = xqspi->u_page;
		}

		/* Get the instruction */
		data = 0;
		xqspips_copy_write_data(xqspi, &data,
					xqspi->curr_inst->inst_size);

		/*
		 * Write the instruction to LSB of the FIFO. The core is
		 * designed such that it is not necessary to check whether the
		 * write FIFO is full before writing. However, write would be
		 * delayed if the user tries to write when write FIFO is full
		 */
		writel(data, &xqspips_base->confr +
				(xqspi->curr_inst->offset / 4));

		/*
		 * Read status register and Read ID instructions don't require
		 * to ignore the extra bytes in response of instruction as
		 * response contains the value
		 */
		if ((instruction == XQSPIPS_FLASH_OPCODE_RDSR1) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_RDSR2) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_RDID) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_BRRD) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_RDEAR)) {
			if (xqspi->bytes_to_transfer < 4)
				xqspi->bytes_to_transfer = 0;
			else
				xqspi->bytes_to_transfer -= 3;
		}
	}

xfer_data:
	/*
	 * In case of Fast, Dual and Quad reads, transmit the instruction first.
	 * Address and dummy byte should be transmitted after instruction
	 * is transmitted
	 */
	if (((xqspi->is_inst == 0) && (xqspi->bytes_to_transfer)) ||
	    ((xqspi->bytes_to_transfer) &&
	     (instruction != XQSPIPS_FLASH_OPCODE_FAST_READ) &&
	     (instruction != XQSPIPS_FLASH_OPCODE_DUAL_READ) &&
	     (instruction != XQSPIPS_FLASH_OPCODE_QUAD_READ)))
		xqspips_fill_tx_fifo(xqspi);

	writel(XQSPIPS_IXR_ALL_MASK, &xqspips_base->ier);
	/* Start the transfer by enabling manual start bit */
	config_reg = readl(&xqspips_base->confr) | XQSPIPS_CONFIG_MANSRT_MASK;
	writel(config_reg, &xqspips_base->confr);

	/* wait for completion */
	do {
		data = xqspips_irq_poll(xqspi);
	} while (data == 0);

	return (transfer->len) - (xqspi->bytes_to_transfer);
}

static int xqspips_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	struct xqspips *xqspi = &qspi->master;
	unsigned cs_change = 1;
	int status = 0;

	debug("%s\n", __func__);

	while (1) {
		if (transfer->bits_per_word || transfer->speed_hz) {
			status = xqspips_setup_transfer(qspi, transfer);
			if (status < 0)
				break;
		}

		/* Select the chip if required */
		if (cs_change)
			xqspips_chipselect(qspi, 1);

		cs_change = transfer->cs_change;

		if (!transfer->tx_buf && !transfer->rx_buf && transfer->len) {
			status = -1;
			break;
		}

		/* Request the transfer */
		if (transfer->len) {
			status = xqspips_start_transfer(qspi, transfer);
			xqspi->is_inst = 0;
		}

		if (status != transfer->len) {
			if (status > 0)
				status = -EMSGSIZE;
			break;
		}
		status = 0;

		if (transfer->delay_usecs)
			udelay(transfer->delay_usecs);

		if (cs_change)
			/* Deselect the chip */
			xqspips_chipselect(qspi, 0);

		break;
	}

	xqspips_setup_transfer(qspi, NULL);

	return 0;
}

/*
 * xqspips_check_is_dual_flash - checking for dual or single qspi
 *
 * This function will check the type of the flash whether it supports
 * single or dual qspi based on the MIO configuration done by FSBL.
 *
 * User needs to correctly configure the MIO's based on the
 * number of qspi flashes present on the board.
 *
 * function will return -1, if there is no MIO configuration for
 * qspi flash.
 */
static int xqspips_check_is_dual_flash(void)
{
	int is_dual = MODE_UNKNOWN;
	int lower_mio = 0, upper_mio = 0, upper_mio_cs1 = 0;

	lower_mio = zynq_slcr_get_mio_pin_status("qspi0");
	if (lower_mio == XQSPIPS_MIO_NUM_QSPI0)
		is_dual = MODE_SINGLE;

	upper_mio_cs1 = zynq_slcr_get_mio_pin_status("qspi1_cs");
	if ((lower_mio == XQSPIPS_MIO_NUM_QSPI0) &&
	    (upper_mio_cs1 == XQSPIPS_MIO_NUM_QSPI1_CS))
		is_dual = MODE_DUAL_STACKED;

	upper_mio = zynq_slcr_get_mio_pin_status("qspi1");
	if ((lower_mio == XQSPIPS_MIO_NUM_QSPI0) &&
	    (upper_mio_cs1 == XQSPIPS_MIO_NUM_QSPI1_CS) &&
	    (upper_mio == XQSPIPS_MIO_NUM_QSPI1))
		is_dual = MODE_DUAL_PARALLEL;

	return is_dual;
}

/*
 * xqspips_write_quad_bit - Write 1 to QUAD bit on flash
 *
 * This function will write a 1 to quad bit in flash
 * using QSPI controller and supports only spansion flash.
 *
 * @regs_base: base address of QSPI controller
 */
static void xqspips_write_quad_bit(void __iomem *regs_base)
{
	u32 config_reg, intr_status;

	/* enable the QSPI controller */
	writel(XQSPIPS_ENABLE_ENABLE_MASK, &xqspips_base->enbr);

	/* Write QUAD bit with 3-byte instruction */
	writel(0x20001, &xqspips_base->txd3r);

	/* Enable manual start command */
	config_reg = readl(&xqspips_base->confr) | XQSPIPS_CONFIG_MANSRT_MASK;
	writel(config_reg, &xqspips_base->confr);

	/* Wait for the transfer to finish by polling Tx fifo status */
	do {
		intr_status = readl(&xqspips_base->isr);
	} while ((intr_status & 0x04) == 0);

	/* Read data receive register */
	config_reg = readl(&xqspips_base->drxr);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* 1 bus with 2 chipselect */
	return bus == 0 && cs < 2;
}

void spi_cs_activate(struct spi_slave *slave)
{
	debug("%s: slave 0x%08x\n", __func__, (unsigned)slave);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	debug("%s: slave 0x%08x\n", __func__, (unsigned)slave);
}

void spi_init()
{
	debug("%s\n", __func__);
}

/*
 * spi_enable_quad_bit - Enable the QUAD bit for SPI flash
 *
 * This function will enable the quad bit in flash using
 * the QSPI controller. Supports only spansion.
 *
 * @spi : SPI slave structure
 */
void spi_enable_quad_bit(struct spi_slave *spi)
{
	int ret;
	u8 idcode[5];
	u8 rdid_cmd = 0x9f;	/* RDID */
	u8 rcr_data = 0;
	u8 rcr_cmd = 0x35;	/* RCR */
	u8 rdsr_cmd = 0x05;	/* RDSR */
	u8 wren_cmd = 0x06;	/* WREN */
	int count = 0;

	ret = spi_flash_cmd(spi, rdid_cmd, &idcode, sizeof(idcode));
	if (ret) {
		debug("%s: Failed read RDID\n", __func__);
		return;
	}

	if ((idcode[0] == 0x01) || (idcode[0] == 0xef)) {
		/* Read config register */
		ret = spi_flash_cmd_read(spi, &rcr_cmd, sizeof(rcr_cmd),
					&rcr_data, sizeof(rcr_data));
		if (ret) {
			debug("%s: Failed read RCR\n", __func__);
			return;
		}

		if (rcr_data & 0x2) {
			debug("%s: QUAD bit is already set\n", __func__);
		} else {
			debug("%s: QUAD bit needs to be set\n", __func__);

			/* Write enable */
			ret = spi_flash_cmd(spi, wren_cmd, NULL, 0);
			if (ret) {
				debug("%s: Failed write WREN\n", __func__);
				return;
			}

			/* Write QUAD bit */
			xqspips_write_quad_bit((void *)ZYNQ_QSPI_BASEADDR);

			/* Read RDSR */
			count = 0;
			do {
				ret = spi_flash_cmd_read(spi, &rdsr_cmd,
						sizeof(rdsr_cmd), &rcr_data,
						sizeof(rcr_data));
			} while ((ret == 0) && (rcr_data != 0) &&
				 (count++ < 1000));

			/* Read config register */
			ret = spi_flash_cmd_read(spi, &rcr_cmd, sizeof(rcr_cmd),
						&rcr_data, sizeof(rcr_data));
			if (!(rcr_data & 0x2)) {
				printf("%s: Fail to set QUAD enable bit 0x%x\n",
				       __func__, rcr_data);
				return;
			} else
				debug("%s: QUAD enable bit is set 0x%x\n",
				      __func__, rcr_data);
		}
	} else
		debug("%s: QUAD bit not enabled for 0x%x SPI flash\n",
		      __func__, idcode[0]);

	return;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	int is_dual;
	struct zynq_spi_slave *pspi;

	debug("%s: bus: %d cs: %d max_hz: %d mode: %d\n",
	      __func__, bus, cs, max_hz, mode);

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	is_dual = xqspips_check_is_dual_flash();

	if (is_dual == MODE_UNKNOWN) {
		printf("%s: No QSPI device detected based on MIO settings\n",
		       __func__);
		return NULL;
	}

	xqspips_init_hw(is_dual, cs);

	pspi = spi_alloc_slave(struct zynq_spi_slave, bus, cs);
	if (!pspi) {
		printf("%s: Fail to allocate zynq_spi_slave\n", __func__);
		return NULL;
	}

	pspi->slave.is_dual = is_dual;
	pspi->qspi.master.input_clk_hz = 100000000;
	pspi->qspi.master.speed_hz = pspi->qspi.master.input_clk_hz / 2;
	pspi->qspi.max_speed_hz = pspi->qspi.master.speed_hz;
	pspi->qspi.master.is_dual = is_dual;
	pspi->qspi.mode = mode;
	pspi->qspi.chip_select = 0;
	pspi->qspi.bits_per_word = 32;
	xqspips_setup_transfer(&pspi->qspi, NULL);

	spi_enable_quad_bit(&pspi->slave);

	return &pspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct zynq_spi_slave *pspi;

	debug("%s: slave: 0x%08x\n", __func__, (u32)slave);

	pspi = to_zynq_spi_slave(slave);
	free(pspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	debug("%s: slave: 0x%08x\n", __func__, (u32)slave);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	debug("%s: slave: 0x%08x\n", __func__, (u32)slave);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct zynq_spi_slave *pspi;
	struct spi_transfer transfer;

	debug("%s: slave: 0x%08x bitlen: %d dout: 0x%08x ", __func__,
	      (u32)slave, bitlen, (u32)dout);
	debug("din: 0x%08x flags: 0x%lx\n", (u32)din, flags);

	pspi = (struct zynq_spi_slave *)slave;
	transfer.tx_buf = dout;
	transfer.rx_buf = din;
	transfer.len = bitlen / 8;

	/*
	 * Festering sore.
	 * Assume that the beginning of a transfer with bits to
	 * transmit must contain a device command.
	 */
	if (dout && flags & SPI_XFER_BEGIN)
		pspi->qspi.master.is_inst = 1;
	else
		pspi->qspi.master.is_inst = 0;

	if (flags & SPI_XFER_END)
		transfer.cs_change = 1;
	else
		transfer.cs_change = 0;

	if (flags & SPI_FLASH_U_PAGE)
		pspi->qspi.master.u_page = 1;
	else
		pspi->qspi.master.u_page = 0;

	transfer.delay_usecs = 0;
	transfer.bits_per_word = 32;
	transfer.speed_hz = pspi->qspi.max_speed_hz;

	xqspips_transfer(&pspi->qspi, &transfer);

	return 0;
}
