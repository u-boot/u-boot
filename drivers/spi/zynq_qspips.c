/*
 * (C) Copyright 2011 Xilinx
 *
 * Xilinx PS Quad-SPI (QSPI) controller driver (master mode only)
 * based on Xilinx PS SPI Driver (xspips.c)
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <malloc.h>
#include <ubi_uboot.h>
#include <spi.h>
#include <asm/arch/hardware.h>

#include "zynq_qspips.h"

/* Register offset definitions */
#define XQSPIPS_CONFIG_OFFSET		0x00 /* Configuration  Register, RW */
#define XQSPIPS_STATUS_OFFSET		0x04 /* Interrupt Status Register, RO */
#define XQSPIPS_IEN_OFFSET		0x08 /* Interrupt Enable Register, WO */
#define XQSPIPS_IDIS_OFFSET		0x0C /* Interrupt Disable Reg, WO */
#define XQSPIPS_IMASK_OFFSET		0x10 /* Interrupt Enabled Mask Reg,RO */
#define XQSPIPS_ENABLE_OFFSET		0x14 /* Enable/Disable Register, RW */
#define XQSPIPS_DELAY_OFFSET		0x18 /* Delay Register, RW */
#define XQSPIPS_TXD_00_00_OFFSET	0x1C /* Transmit 4-byte inst, WO */
#define XQSPIPS_TXD_00_01_OFFSET	0x80 /* Transmit 1-byte inst, WO */
#define XQSPIPS_TXD_00_10_OFFSET	0x84 /* Transmit 2-byte inst, WO */
#define XQSPIPS_TXD_00_11_OFFSET	0x88 /* Transmit 3-byte inst, WO */
#define XQSPIPS_RXD_OFFSET		0x20 /* Data Receive Register, RO */
#define XQSPIPS_SIC_OFFSET		0x24 /* Slave Idle Count Register, RW */
#define XQSPIPS_TX_THRESH_OFFSET	0x28 /* TX FIFO Watermark Reg, RW */
#define XQSPIPS_RX_THRESH_OFFSET	0x2C /* RX FIFO Watermark Reg, RW */
#define XQSPIPS_GPIO_OFFSET		0x30 /* GPIO Register, RW */
#define XQSPIPS_LINEAR_CFG_OFFSET	0xA0 /* Linear Adapter Config Ref, RW */
#define XQSPIPS_MOD_ID_OFFSET		0xFC /* Module ID Register, RO */

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

/* Definitions of the flash commands - Flash opcodes in ascending order */
#define	XQSPIPS_FLASH_OPCODE_WRSR	0x01	/* Write status register */
#define	XQSPIPS_FLASH_OPCODE_PP		0x02	/* Page program */
#define	XQSPIPS_FLASH_OPCODE_NORM_READ	0x03	/* Normal read data bytes */
#define	XQSPIPS_FLASH_OPCODE_WRDS	0x04	/* Write disable */
#define	XQSPIPS_FLASH_OPCODE_RDSR1	0x05	/* Read status register 1 */
#define	XQSPIPS_FLASH_OPCODE_WREN	0x06	/* Write enable */
#define	XQSPIPS_FLASH_OPCODE_FAST_READ	0x0B	/* Fast read data bytes */
#define	XQSPIPS_FLASH_OPCODE_BRRD	0x16	/* Bank address reg read */
#define	XQSPIPS_FLASH_OPCODE_BRWR	0x17	/* Bank address reg write */
#define	XQSPIPS_FLASH_OPCODE_BE_4K	0x20	/* Erase 4KiB block */
#define	XQSPIPS_FLASH_OPCODE_RDSR2	0x35	/* Read status register 2 */
#define	XQSPIPS_FLASH_OPCODE_DUAL_READ	0x3B	/* Dual read data bytes */
#define	XQSPIPS_FLASH_OPCODE_BE_32K	0x52	/* Erase 32KiB block */
#define	XQSPIPS_FLASH_OPCODE_QUAD_READ	0x6B	/* Quad read data bytes */
#define	XQSPIPS_FLASH_OPCODE_ERASE_SUS	0x75	/* Erase suspend */
#define	XQSPIPS_FLASH_OPCODE_ERASE_RES	0x7A	/* Erase resume */
#define	XQSPIPS_FLASH_OPCODE_RDID	0x9F	/* Read JEDEC ID */
#define	XQSPIPS_FLASH_OPCODE_BE		0xC7	/* Erase whole flash block */
#define	XQSPIPS_FLASH_OPCODE_SE		0xD8	/* Sector erase (usually 64KB)*/

/* Macros for the QSPI controller read/write */
#define xqspips_write(off, val)		__raw_writel(val, off)
#define xqspips_read(off)		__raw_readl(off)

struct zynq_spi_slave {
	struct spi_slave  slave;
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
	/* Add all the instructions supported by the flash device */
};

/*
 * xqspips_init_hw - Initialize the hardware
 * @regs_base:		Base address of QSPI controller
 * @is_dual:		Indicates whether dual memories are used
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
void xqspips_init_hw(void __iomem *regs_base, unsigned int is_dual)
{
	u32 config_reg;

	xqspips_write(regs_base + XQSPIPS_ENABLE_OFFSET,
		~XQSPIPS_ENABLE_ENABLE_MASK);
	xqspips_write(regs_base + XQSPIPS_IDIS_OFFSET, 0x7F);

	/* Disable linear mode as the boot loader may have used it */
	xqspips_write(regs_base + XQSPIPS_LINEAR_CFG_OFFSET, 0);

	/* Clear the RX FIFO */
	while (xqspips_read(regs_base + XQSPIPS_STATUS_OFFSET) &
			XQSPIPS_IXR_RXNEMTY_MASK)
		xqspips_read(regs_base + XQSPIPS_RXD_OFFSET);

	xqspips_write(regs_base + XQSPIPS_STATUS_OFFSET , 0x7F);
	config_reg = xqspips_read(regs_base + XQSPIPS_CONFIG_OFFSET);
	config_reg &= 0xFBFFFFFF; /* Set little endian mode of TX FIFO */
	config_reg |= 0x8000FCC1;
	xqspips_write(regs_base + XQSPIPS_CONFIG_OFFSET, config_reg);

	if (is_dual == 1)
		/* Enable two memories on seperate buses */
		xqspips_write(regs_base + XQSPIPS_LINEAR_CFG_OFFSET,
			(XQSPIPS_LCFG_TWO_MEM_MASK |
			 XQSPIPS_LCFG_SEP_BUS_MASK |
			 (1 << XQSPIPS_LCFG_DUMMY_SHIFT) |
			 XQSPIPS_FAST_READ_QOUT_CODE));

	xqspips_write(regs_base + XQSPIPS_ENABLE_OFFSET,
			XQSPIPS_ENABLE_ENABLE_MASK);
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

	debug("xqspips_copy_read_data: data 0x%04x rxbuf addr: 0x%08x"
		" size %d\n", data, (unsigned)(xqspi->rxbuf), size);

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
			memcpy(xqspi->rxbuf, &data,  size);
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
			memcpy(data, xqspi->txbuf,  size);
			xqspi->txbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	} else
		*data = 0;

	debug("xqspips_copy_write_data: data 0x%08x txbuf addr: 0x%08x"
		" size %d\n", *data, (u32)xqspi->txbuf, size);

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
	struct xqspips *xqspi = &qspi->master;
	u32 config_reg;

	debug("xqspips_chipselect: is_on: %d\n", is_on);

	config_reg = xqspips_read(xqspi->regs + XQSPIPS_CONFIG_OFFSET);

	if (is_on) {
		/* Select the slave */
		config_reg &= ~XQSPIPS_CONFIG_SSCTRL_MASK;
		config_reg |= (((~(0x0001 << qspi->chip_select)) << 10) &
				XQSPIPS_CONFIG_SSCTRL_MASK);
	} else
		/* Deselect the slave */
		config_reg |= XQSPIPS_CONFIG_SSCTRL_MASK;

	xqspips_write(xqspi->regs + XQSPIPS_CONFIG_OFFSET, config_reg);
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
int xqspips_setup_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	struct xqspips *xqspi = &qspi->master;
	u8 bits_per_word;
	u32 config_reg;
	u32 req_hz;
	u32 baud_rate_val = 0;

	debug("xqspips_setup_transfer: qspi: 0x%08x transfer: 0x%08x\n",
		(u32)qspi, (u32)transfer);

	bits_per_word = (transfer) ?
			transfer->bits_per_word : qspi->bits_per_word;
	req_hz = (transfer) ? transfer->speed_hz : qspi->max_speed_hz;

	if (qspi->mode & ~MODEBITS) {
		printf("%s, unsupported mode bits %x\n",
			__func__, qspi->mode & ~MODEBITS);
		return -1;
	}

	if (bits_per_word != 32)
		bits_per_word = 32;

	config_reg = xqspips_read(xqspi->regs + XQSPIPS_CONFIG_OFFSET);

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
		while ((baud_rate_val < 8)  &&
			(xqspi->input_clk_hz / (2 << baud_rate_val)) > req_hz) {
				baud_rate_val++;
		}
		config_reg &= 0xFFFFFFC7;
		config_reg |= (baud_rate_val << 3);
		xqspi->speed_hz = req_hz;
	}

	xqspips_write(xqspi->regs + XQSPIPS_CONFIG_OFFSET, config_reg);

	debug("xqspips_setup_transfer: mode %d, %u bits/w, %u clock speed\n",
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

	while ((!(xqspips_read(xqspi->regs + XQSPIPS_STATUS_OFFSET) &
			XQSPIPS_IXR_TXFULL_MASK)) &&
			(xqspi->bytes_to_transfer > 0)) {
		if (xqspi->bytes_to_transfer < 4) {
			/* Write TXD1, TXD2, TXD3 only if TxFIFO is empty. */
			if (!(xqspips_read(xqspi->regs+XQSPIPS_STATUS_OFFSET)
					& XQSPIPS_IXR_TXNFULL_MASK) &&
					!xqspi->rxbuf)
				return;
			len = xqspi->bytes_to_transfer;
			xqspips_copy_write_data(xqspi, &data, len);
			offset = (xqspi->rxbuf) ? offsets[0] : offsets[len];
			xqspips_write(xqspi->regs + offset, data);
		} else {
			xqspips_copy_write_data(xqspi, &data, 4);
			xqspips_write(xqspi->regs + XQSPIPS_TXD_00_00_OFFSET,
					data);
		}
	}
}

/*
 * xqspips_irq - Interrupt service routine of the QSPI controller
 * @xqspi:      Pointer to the xqspips structure
 *
 * This function handles TX empty and Mode Fault interrupts only.
 * On TX empty interrupt this function reads the received data from RX FIFO and
 * fills the TX FIFO if there is any data remaining to be transferred.
 * On Mode Fault interrupt this function indicates that transfer is completed,
 * the SPI subsystem will identify the error as the remaining bytes to be
 * transferred is non-zero.
 *
 * returns:	IRQ_HANDLED always
 */
static int xqspips_irq_poll(struct xqspips *xqspi)
{
	int max_loop;
	u32 intr_status;

	debug("xqspips_irq_poll: xqspi: 0x%08x\n", (u32)xqspi);

	/* Poll until any of the interrupt status bits are set */
	max_loop = 0;
	do {
		intr_status = xqspips_read(xqspi->regs +
				XQSPIPS_STATUS_OFFSET);
		max_loop++;
	} while ((intr_status == 0) && (max_loop < 100000));

	if (intr_status == 0) {
		printf("xqspips_irq_poll: timeout\n");
		return 0;
	}

	xqspips_write(xqspi->regs + XQSPIPS_STATUS_OFFSET , intr_status);

	/* Disable all interrupts */
	xqspips_write(xqspi->regs + XQSPIPS_IDIS_OFFSET,
			XQSPIPS_IXR_ALL_MASK);
	if ((intr_status & XQSPIPS_IXR_TXNFULL_MASK) ||
			(intr_status & XQSPIPS_IXR_RXNEMTY_MASK)) {

		/*
		 * This bit is set when Tx FIFO has < THRESHOLD entries. We have
		 * the THRESHOLD value set to 1, so this bit indicates Tx FIFO
		 * is empty
		 */
		u32 config_reg;

		/* Read out the data from the RX FIFO */
		while (xqspips_read(xqspi->regs + XQSPIPS_STATUS_OFFSET) &
				XQSPIPS_IXR_RXNEMTY_MASK) {
			u32 data;

			data = xqspips_read(xqspi->regs + XQSPIPS_RXD_OFFSET);

			if ((xqspi->inst_response) &&
					(!((xqspi->curr_inst->opcode ==
					XQSPIPS_FLASH_OPCODE_RDSR1) ||
					(xqspi->curr_inst->opcode ==
					XQSPIPS_FLASH_OPCODE_RDSR2)))) {
				xqspi->inst_response = 0;
				xqspips_copy_read_data(xqspi, data,
					xqspi->curr_inst->inst_size);
			} else if (xqspi->bytes_to_receive < 4)
				xqspips_copy_read_data(xqspi, data,
					xqspi->bytes_to_receive);
			else
				xqspips_copy_read_data(xqspi, data, 4);
		}

		if (xqspi->bytes_to_transfer) {
			/* There is more data to send */
			xqspips_fill_tx_fifo(xqspi);

			xqspips_write(xqspi->regs + XQSPIPS_IEN_OFFSET,
					XQSPIPS_IXR_ALL_MASK);

			config_reg = xqspips_read(xqspi->regs +
						XQSPIPS_CONFIG_OFFSET);

			config_reg |= XQSPIPS_CONFIG_MANSRT_MASK;
			xqspips_write(xqspi->regs + XQSPIPS_CONFIG_OFFSET,
					config_reg);
		} else {
			/*
			 * If transfer and receive is completed then only send
			 * complete signal
			 */
			if (!xqspi->bytes_to_receive) {
				/* return operation complete */
				xqspips_write(xqspi->regs + XQSPIPS_IDIS_OFFSET,
						XQSPIPS_IXR_ALL_MASK);
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
	u32 config_reg;
	u32 data = 0;
	u8 instruction = 0;
	u8 index;

	debug("xqspips_start_transfer: qspi: 0x%08x transfer: 0x%08x len: %d\n",
		(u32)qspi, (u32)transfer, transfer->len);

	xqspi->txbuf = transfer->tx_buf;
	xqspi->rxbuf = transfer->rx_buf;
	xqspi->bytes_to_transfer = transfer->len;
	xqspi->bytes_to_receive = transfer->len;

	if (xqspi->txbuf)
		instruction = *(u8 *)xqspi->txbuf;

	if (instruction && xqspi->is_inst) {
		for (index = 0 ; index < ARRAY_SIZE(flash_inst); index++)
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

		/*
		 * In case of dual memories, convert 25 bit address to 24 bit
		 * address before transmitting to the 2 memories
		 */
		if ((xqspi->is_dual == 1) &&
		    ((instruction == XQSPIPS_FLASH_OPCODE_PP) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_SE) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_BE_32K) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_BE_4K) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_BE) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_NORM_READ) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_FAST_READ) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_DUAL_READ) ||
		    (instruction == XQSPIPS_FLASH_OPCODE_QUAD_READ))) {

			u8 *ptr = (u8 *) (xqspi->txbuf);
			data = ((u32) ptr[1] << 24) | ((u32) ptr[2] << 16) |
				((u32) ptr[3] << 8) | ((u32) ptr[4]);
			data = data/2;
			ptr[1] = (u8) (data >> 16);
			ptr[2] = (u8) (data >> 8);
			ptr[3] = (u8) (data);
			xqspi->bytes_to_transfer -= 1;
			xqspi->bytes_to_receive -= 1;
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
		xqspips_write(xqspi->regs + xqspi->curr_inst->offset, data);

		/*
		 * Read status register and Read ID instructions don't require
		 * to ignore the extra bytes in response of instruction as
		 * response contains the value
		 */
		if ((instruction == XQSPIPS_FLASH_OPCODE_RDSR1) ||
				(instruction == XQSPIPS_FLASH_OPCODE_RDSR2) ||
				(instruction == XQSPIPS_FLASH_OPCODE_RDID) ||
				(instruction == XQSPIPS_FLASH_OPCODE_BRRD)) {
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

	xqspips_write(xqspi->regs + XQSPIPS_IEN_OFFSET,
			XQSPIPS_IXR_ALL_MASK);
	/* Start the transfer by enabling manual start bit */
	config_reg = xqspips_read(xqspi->regs +
			XQSPIPS_CONFIG_OFFSET) | XQSPIPS_CONFIG_MANSRT_MASK;
	xqspips_write(xqspi->regs + XQSPIPS_CONFIG_OFFSET, config_reg);

	/* wait for completion */
	do {
		data = xqspips_irq_poll(xqspi);
	} while (data == 0);

	return (transfer->len) - (xqspi->bytes_to_transfer);
}

int xqspips_transfer(struct spi_device *qspi, struct spi_transfer *transfer)
{
	struct xqspips *xqspi = &qspi->master;
	unsigned cs_change = 1;
	int status = 0;

	debug("xqspips_transfer\n");

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
 *
 * @regs_base:	base address of SLCR
 */

int xqspips_check_is_dual_flash(void __iomem *regs_base)
{
	int is_dual = -1, lower_mio = 0, upper_mio = 0, val;
	u16 mask = 3, type = 2;
	u32 mio_pin_index;
	void *mio_base;

	mio_base = regs_base + 0x700;

	/* checking single QSPI MIO's */
	for (mio_pin_index = 2; mio_pin_index < 7; mio_pin_index++) {
		val = xqspips_read(mio_base + 4 * mio_pin_index);
		if ((val & mask) == type)
			lower_mio++;
	}

	/* checking dual QSPI MIO's */
	for (mio_pin_index = 8; mio_pin_index < 14; mio_pin_index++) {
		val = xqspips_read(mio_base + 4 * mio_pin_index);
		if ((val & mask) == type)
			upper_mio++;
	}

	if ((lower_mio == 5) && (upper_mio == 6))
		is_dual = 1;
	else if (lower_mio == 5)
		is_dual = 0;

	return is_dual;
}

/*
 * xqspips_write_quad_bit - Write 1 to QUAD bit on flash
 *
 * This function will write a 1 to quad bit in flash
 * using QSPI controller and supports only spansion flash.
 *
 * @regs_base:  base address of QSPI controller
 */
void xqspips_write_quad_bit(void __iomem *regs_base)
{
	u32 config_reg, intr_status;

	/* enable the QSPI controller */
	xqspips_write(regs_base + XQSPIPS_ENABLE_OFFSET,
			XQSPIPS_ENABLE_ENABLE_MASK);

	/* Write QUAD bit with 3-byte instruction */
	xqspips_write(regs_base + XQSPIPS_TXD_00_11_OFFSET, 0x20001);

	/* Enable manual start command */
	config_reg = xqspips_read(regs_base +
		XQSPIPS_CONFIG_OFFSET) | XQSPIPS_CONFIG_MANSRT_MASK;
	xqspips_write(regs_base + XQSPIPS_CONFIG_OFFSET, config_reg);

	/* Wait for the transfer to finish by polling Tx fifo status */
	do {
		intr_status = xqspips_read(regs_base +
			XQSPIPS_STATUS_OFFSET);
	} while ((intr_status & 0x04) == 0);

	/* Read data receive register */
	config_reg = xqspips_read(regs_base + XQSPIPS_RXD_OFFSET);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	debug("spi_cs_is_valid: bus: %d cs: %d\n",
		bus, cs);
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	debug("spi_cs_activate: slave 0x%08x\n", (unsigned)slave);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	debug("spi_cs_deactivate: slave 0x%08x\n", (unsigned)slave);
}

void spi_init()
{
	debug("spi_init\n");
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

	ret = spi_flash_cmd(spi, rdid_cmd, &idcode, sizeof(idcode));
	if (ret) {
		debug("SF error: Failed read RDID\n");
		return;
	}

	if ((idcode[0] == 0x01) || (idcode[0] == 0xef)) {
		/* Read config register */
		ret = spi_flash_cmd_read(spi, &rcr_cmd, sizeof(rcr_cmd),
					&rcr_data, sizeof(rcr_data));
		if (ret) {
			debug("SF error: Failed read RCR\n");
			return;
		}

		if (rcr_data & 0x2)
			debug("QUAD bit is already set..\n");
		else {
			debug("QUAD bit needs to be set ..\n");

			/* Write enable */
			ret = spi_flash_cmd(spi, wren_cmd, NULL, 0);
			if (ret) {
				debug("SF error: Failed write WREN\n");
				return;
			}

			/* Write QUAD bit */
			xqspips_write_quad_bit((void *)XPSS_QSPI_BASEADDR);

			/* Read RDSR */
			do {
				ret = spi_flash_cmd_read(spi, &rdsr_cmd,
						sizeof(rdsr_cmd), &rcr_data,
						sizeof(rcr_data));
			} while ((ret == 0) && (rcr_data != 0));

			/* Read config register */
			ret = spi_flash_cmd_read(spi, &rcr_cmd, sizeof(rcr_cmd),
						&rcr_data, sizeof(rcr_data));
			if (!(rcr_data & 0x2)) {
				printf("SF error: Fail to set QUAD enable bit"
					" 0x%x\n", rcr_data);
				return;
			} else
				debug("SF: QUAD enable bit is set 0x%x\n",
						rcr_data);
		}
	} else
		debug("SF: QUAD bit not enabled for 0x%x SPI flash\n",
					idcode[0]);

	return;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	int is_dual;
	struct zynq_spi_slave *pspi;

	debug("spi_setup_slave: bus: %d cs: %d max_hz: %d mode: %d\n",
		bus, cs, max_hz, mode);

	is_dual = xqspips_check_is_dual_flash((void *)XPSS_SYS_CTRL_BASEADDR);

	if (is_dual == -1) {
		printf("SPI error: No QSPI device detected based"
				" on MIO settings\n");
		return NULL;
	}

	xqspips_init_hw((void *)XPSS_QSPI_BASEADDR, is_dual);

	pspi = malloc(sizeof(struct zynq_spi_slave));
	if (!pspi) {
		printf("SPI error: fail to allocate zynq_spi_slave\n");
		return NULL;
	}

	pspi->slave.bus = bus;
	pspi->slave.cs = cs;
	pspi->slave.is_dual = is_dual;
	pspi->qspi.master.input_clk_hz = 100000000;
	pspi->qspi.master.speed_hz = pspi->qspi.master.input_clk_hz / 2;
	pspi->qspi.max_speed_hz = pspi->qspi.master.speed_hz;
	pspi->qspi.master.regs = (void *)XPSS_QSPI_BASEADDR;
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

	debug("spi_free_slave: slave: 0x%08x\n", (u32)slave);

	pspi = to_zynq_spi_slave(slave);
	free(pspi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	debug("spi_claim_bus: slave: 0x%08x\n", (u32)slave);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	debug("spi_release_bus: slave: 0x%08x\n", (u32)slave);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct zynq_spi_slave *pspi;
	struct spi_transfer transfer;

	debug("spi_xfer: slave: 0x%08x bitlen: %d dout: 0x%08x din:"
		" 0x%08x flags: 0x%lx\n",
		(u32)slave, bitlen, (u32)dout, (u32)din, flags);

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

	transfer.delay_usecs = 0;
	transfer.bits_per_word = 32;
	transfer.speed_hz = pspi->qspi.max_speed_hz;

	xqspips_transfer(&pspi->qspi, &transfer);

	return 0;
}
