/*
 * (C) Copyright 2011 - 2013 Xilinx
 *
 * Xilinx Zynq Quad-SPI(QSPI) controller driver (master mode only)
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

/* Definitions of the flash commands - Flash opcodes in ascending order */
#define ZYNQ_QSPI_FLASH_OPCODE_WRSR	0x01	/* Write status register */
#define ZYNQ_QSPI_FLASH_OPCODE_PP	0x02	/* Page program */
#define ZYNQ_QSPI_FLASH_OPCODE_NR	0x03	/* Normal read data bytes */
#define ZYNQ_QSPI_FLASH_OPCODE_WRDS	0x04	/* Write disable */
#define ZYNQ_QSPI_FLASH_OPCODE_RDSR1	0x05	/* Read status register 1 */
#define ZYNQ_QSPI_FLASH_OPCODE_WREN	0x06	/* Write enable */
#define ZYNQ_QSPI_FLASH_OPCODE_FR	0x0B	/* Fast read data bytes */
#define ZYNQ_QSPI_FLASH_OPCODE_BRRD	0x16	/* Bank address reg read */
#define ZYNQ_QSPI_FLASH_OPCODE_BRWR	0x17	/* Bank address reg write */
#define ZYNQ_QSPI_FLASH_OPCODE_BE_4K	0x20	/* Erase 4KiB block */
#define ZYNQ_QSPI_FLASH_OPCODE_QPP	0x32	/* Quad Page Program */
#define ZYNQ_QSPI_FLASH_OPCODE_RDSR2	0x35	/* Read status register 2 */
#define ZYNQ_QSPI_FLASH_OPCODE_DR	0x3B	/* Dual read data bytes */
#define ZYNQ_QSPI_FLASH_OPCODE_BE_32K	0x52	/* Erase 32KiB block */
#define ZYNQ_QSPI_FLASH_OPCODE_QR	0x6B	/* Quad read data bytes */
#define ZYNQ_QSPI_FLASH_OPCODE_ES	0x75	/* Erase suspend */
#define ZYNQ_QSPI_FLASH_OPCODE_ER	0x7A	/* Erase resume */
#define ZYNQ_QSPI_FLASH_OPCODE_RDID	0x9F	/* Read JEDEC ID */
#define ZYNQ_QSPI_FLASH_OPCODE_DIOR	0xBB	/* Dual IO high perf read */
#define ZYNQ_QSPI_FLASH_OPCODE_WREAR	0xC5	/* Extended address reg write */
#define ZYNQ_QSPI_FLASH_OPCODE_RDEAR	0xC8	/* Extended address reg read */
#define ZYNQ_QSPI_FLASH_OPCODE_BE	0xC7	/* Erase whole flash block */
#define ZYNQ_QSPI_FLASH_OPCODE_SE	0xD8	/* Sector erase (usually 64KB)*/

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

#define zynq_qspi_base ((struct zynq_qspi_regs *)ZYNQ_QSPI_BASEADDR)

struct zynq_qspi {
	u32 input_clk_hz;
	u32 speed_hz;
	const void *txbuf;
	void *rxbuf;
	int bytes_to_transfer;
	int bytes_to_receive;
	struct zynq_qspi_inst_format *curr_inst;
	u8 inst_response;
	unsigned int is_inst;
	unsigned int is_dual;
	unsigned int u_page;
};

struct spi_device {
	struct zynq_qspi master;
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

struct zynq_qspi_slave {
	struct spi_slave slave;
	struct spi_device qspi;
};
#define to_zynq_qspi_slave(s) container_of(s, struct zynq_qspi_slave, slave)

/*
 * struct zynq_qspi_inst_format - Defines qspi flash instruction format
 * @opcode:		Operational code of instruction
 * @inst_size:		Size of the instruction including address bytes
 * @offset:		Register address where instruction has to be written
 */
struct zynq_qspi_inst_format {
	u8 opcode;
	u8 inst_size;
	u8 offset;
};

/* List of all the QSPI instructions and its format */
static struct zynq_qspi_inst_format flash_inst[] = {
	{ ZYNQ_QSPI_FLASH_OPCODE_WREN, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_WRDS, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_RDSR1, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_RDSR2, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_WRSR, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_PP, 4, ZYNQ_QSPI_TXD_00_00_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_SE, 4, ZYNQ_QSPI_TXD_00_00_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_BE_32K, 4, ZYNQ_QSPI_TXD_00_00_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_BE_4K, 4, ZYNQ_QSPI_TXD_00_00_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_BE, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_ES, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_ER, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_RDID, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_NR, 4, ZYNQ_QSPI_TXD_00_00_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_FR, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_DR, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_QR, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_BRWR, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_BRRD, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_WREAR, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_RDEAR, 1, ZYNQ_QSPI_TXD_00_01_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_QPP, 4, ZYNQ_QSPI_TXD_00_00_OFFSET },
	{ ZYNQ_QSPI_FLASH_OPCODE_DIOR, 4, ZYNQ_QSPI_TXD_00_00_OFFSET },
	/* Add all the instructions supported by the flash device */
};

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
 *	- Enable manual start
 *	- Deselect all the chip select lines
 *	- Set the size of the word to be transferred as 32 bit
 *	- Set the little endian mode of TX FIFO and
 *	- Enable the QSPI controller
 */
static void zynq_qspi_init_hw(int is_dual, unsigned int cs)
{
	u32 config_reg;

	writel(~ZYNQ_QSPI_ENABLE_ENABLE_MASK, &zynq_qspi_base->enbr);
	writel(0x7F, &zynq_qspi_base->idisr);

	/* Disable linear mode as the boot loader may have used it */
	writel(0x0, &zynq_qspi_base->lcr);

	/* Clear the TX and RX threshold reg */
	writel(0x1, &zynq_qspi_base->txftr);
	writel(ZYNQ_QSPI_RXFIFO_THRESHOLD, &zynq_qspi_base->rxftr);

	/* Clear the RX FIFO */
	while (readl(&zynq_qspi_base->isr) & ZYNQ_QSPI_IXR_RXNEMTY_MASK)
		readl(&zynq_qspi_base->drxr);

	writel(0x7F, &zynq_qspi_base->isr);
	config_reg = readl(&zynq_qspi_base->confr);
	config_reg |= ZYNQ_QSPI_CONFIG_IFMODE_MASK |
		ZYNQ_QSPI_CONFIG_MCS_MASK | ZYNQ_QSPI_CONFIG_PCS_MASK |
		ZYNQ_QSPI_CONFIG_FW_MASK | ZYNQ_QSPI_CONFIG_MSTREN_MASK;
	if (is_dual == SF_DUAL_STACKED_FLASH)
		config_reg |= 0x10;
	writel(config_reg, &zynq_qspi_base->confr);

	if (is_dual == SF_DUAL_PARALLEL_FLASH)
		/* Enable two memories on seperate buses */
		writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
			ZYNQ_QSPI_LCFG_SEP_BUS_MASK |
			(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
			ZYNQ_QSPI_FR_QOUT_CODE),
			&zynq_qspi_base->lcr);
	else if (is_dual == SF_DUAL_STACKED_FLASH)
		/* Configure two memories on shared bus by enabling lower mem */
		writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
			(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
			ZYNQ_QSPI_FR_QOUT_CODE),
			&zynq_qspi_base->lcr);

	writel(ZYNQ_QSPI_ENABLE_ENABLE_MASK, &zynq_qspi_base->enbr);
}

/*
 * zynq_qspi_copy_read_data - Copy data to RX buffer
 * @zqspi:	Pointer to the zynq_qspi structure
 * @data:	The 32 bit variable where data is stored
 * @size:	Number of bytes to be copied from data to RX buffer
 */
static void zynq_qspi_copy_read_data(struct zynq_qspi *zqspi, u32 data, u8 size)
{
	u8 byte3;

	debug("%s: data 0x%04x rxbuf addr: 0x%08x size %d\n", __func__ ,
	      data, (unsigned)(zqspi->rxbuf), size);

	if (zqspi->rxbuf) {
		switch (size) {
		case 1:
			*((u8 *)zqspi->rxbuf) = data;
			zqspi->rxbuf += 1;
			break;
		case 2:
			*((u16 *)zqspi->rxbuf) = data;
			zqspi->rxbuf += 2;
			break;
		case 3:
			*((u16 *)zqspi->rxbuf) = data;
			zqspi->rxbuf += 2;
			byte3 = (u8)(data >> 16);
			*((u8 *)zqspi->rxbuf) = byte3;
			zqspi->rxbuf += 1;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(zqspi->rxbuf, &data, size);
			zqspi->rxbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	}
	zqspi->bytes_to_receive -= size;
	if (zqspi->bytes_to_receive < 0)
		zqspi->bytes_to_receive = 0;
}

/*
 * zynq_qspi_copy_write_data - Copy data from TX buffer
 * @zqspi:	Pointer to the zynq_qspi structure
 * @data:	Pointer to the 32 bit variable where data is to be copied
 * @size:	Number of bytes to be copied from TX buffer to data
 */
static void zynq_qspi_copy_write_data(struct zynq_qspi *zqspi,
		u32 *data, u8 size)
{
	if (zqspi->txbuf) {
		switch (size) {
		case 1:
			*data = *((u8 *)zqspi->txbuf);
			zqspi->txbuf += 1;
			*data |= 0xFFFFFF00;
			break;
		case 2:
			*data = *((u16 *)zqspi->txbuf);
			zqspi->txbuf += 2;
			*data |= 0xFFFF0000;
			break;
		case 3:
			*data = *((u16 *)zqspi->txbuf);
			zqspi->txbuf += 2;
			*data |= (*((u8 *)zqspi->txbuf) << 16);
			zqspi->txbuf += 1;
			*data |= 0xFF000000;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(data, zqspi->txbuf, size);
			zqspi->txbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	} else {
		*data = 0;
	}

	debug("%s: data 0x%08x txbuf addr: 0x%08x size %d\n", __func__,
	      *data, (u32)zqspi->txbuf, size);

	zqspi->bytes_to_transfer -= size;
	if (zqspi->bytes_to_transfer < 0)
		zqspi->bytes_to_transfer = 0;
}

/*
 * zynq_qspi_chipselect - Select or deselect the chip select line
 * @qspi:	Pointer to the spi_device structure
 * @is_on:	Select(1) or deselect (0) the chip select line
 */
static void zynq_qspi_chipselect(struct spi_device *qspi, int is_on)
{
	u32 config_reg;

	debug("%s: is_on: %d\n", __func__, is_on);

	config_reg = readl(&zynq_qspi_base->confr);

	if (is_on) {
		/* Select the slave */
		config_reg &= ~ZYNQ_QSPI_CONFIG_SSCTRL_MASK;
		config_reg |= (((~(0x0001 << qspi->chip_select)) << 10) &
				ZYNQ_QSPI_CONFIG_SSCTRL_MASK);
	} else
		/* Deselect the slave */
		config_reg |= ZYNQ_QSPI_CONFIG_SSCTRL_MASK;

	writel(config_reg, &zynq_qspi_base->confr);
}

/*
 * zynq_qspi_setup_transfer - Configure QSPI controller for specified transfer
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
static int zynq_qspi_setup_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	struct zynq_qspi *zqspi = &qspi->master;
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

	config_reg = readl(&zynq_qspi_base->confr);

	/* Set the QSPI clock phase and clock polarity */
	config_reg &= (~ZYNQ_QSPI_CONFIG_CPHA_MASK) &
				(~ZYNQ_QSPI_CONFIG_CPOL_MASK);
	if (qspi->mode & SPI_CPHA)
		config_reg |= ZYNQ_QSPI_CONFIG_CPHA_MASK;
	if (qspi->mode & SPI_CPOL)
		config_reg |= ZYNQ_QSPI_CONFIG_CPOL_MASK;

	/* Set the clock frequency */
	if (zqspi->speed_hz != req_hz) {
		baud_rate_val = 0;
		while ((baud_rate_val < 7) &&
			(zqspi->input_clk_hz / (2 << baud_rate_val)) > req_hz) {
				baud_rate_val++;
		}
		config_reg &= 0xFFFFFFC7;
		config_reg |= (baud_rate_val << 3);
		zqspi->speed_hz = req_hz;
	}

	writel(config_reg, &zynq_qspi_base->confr);

	debug("%s: mode %d, %u bits/w, %u clock speed\n", __func__,
	      qspi->mode & MODEBITS, qspi->bits_per_word, zqspi->speed_hz);

	return 0;
}

/*
 * zynq_qspi_fill_tx_fifo - Fills the TX FIFO with as many bytes as possible
 * @zqspi:	Pointer to the zynq_qspi structure
 */
static void zynq_qspi_fill_tx_fifo(struct zynq_qspi *zqspi, u32 size)
{
	u32 data = 0;
	u32 fifocount = 0;
	unsigned len, offset;
	static const unsigned offsets[4] = {
		ZYNQ_QSPI_TXD_00_00_OFFSET, ZYNQ_QSPI_TXD_00_01_OFFSET,
		ZYNQ_QSPI_TXD_00_10_OFFSET, ZYNQ_QSPI_TXD_00_11_OFFSET };

	while ((fifocount < size) &&
			(zqspi->bytes_to_transfer > 0)) {
		if (zqspi->bytes_to_transfer >= 4) {
			if (zqspi->txbuf) {
				memcpy(&data, zqspi->txbuf, 4);
				zqspi->txbuf += 4;
			} else {
				data = 0;
			}
			writel(data, &zynq_qspi_base->txd0r);
			zqspi->bytes_to_transfer -= 4;
			fifocount++;
		} else {
			/* Write TXD1, TXD2, TXD3 only if TxFIFO is empty. */
			if (!(readl(&zynq_qspi_base->isr)
					& ZYNQ_QSPI_IXR_TXNFULL_MASK) &&
					!zqspi->rxbuf)
				return;
			len = zqspi->bytes_to_transfer;
			zynq_qspi_copy_write_data(zqspi, &data, len);
			offset = (zqspi->rxbuf) ? offsets[0] : offsets[len];
			writel(data, &zynq_qspi_base->confr + (offset / 4));
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
static int zynq_qspi_irq_poll(struct zynq_qspi *zqspi)
{
	int max_loop;
	u32 intr_status;
	u32 rxindex = 0;
	u32 rxcount;

	debug("%s: zqspi: 0x%08x\n", __func__, (u32)zqspi);

	/* Poll until any of the interrupt status bits are set */
	max_loop = 0;
	do {
		intr_status = readl(&zynq_qspi_base->isr);
		max_loop++;
	} while ((intr_status == 0) && (max_loop < 100000));

	if (intr_status == 0) {
		printf("%s: Timeout\n", __func__);
		return 0;
	}

	writel(intr_status, &zynq_qspi_base->isr);

	/* Disable all interrupts */
	writel(ZYNQ_QSPI_IXR_ALL_MASK, &zynq_qspi_base->idisr);
	if ((intr_status & ZYNQ_QSPI_IXR_TXNFULL_MASK) ||
	    (intr_status & ZYNQ_QSPI_IXR_RXNEMTY_MASK)) {
		/*
		 * This bit is set when Tx FIFO has < THRESHOLD entries. We have
		 * the THRESHOLD value set to 1, so this bit indicates Tx FIFO
		 * is empty
		 */
		rxcount = zqspi->bytes_to_receive - zqspi->bytes_to_transfer;
		rxcount = (rxcount % 4) ? ((rxcount/4)+1) : (rxcount/4);
		while ((rxindex < rxcount) &&
				(rxindex < ZYNQ_QSPI_RXFIFO_THRESHOLD)) {
			/* Read out the data from the RX FIFO */
				u32 data;

				data = readl(&zynq_qspi_base->drxr);

				if ((zqspi->inst_response) &&
				    (!((zqspi->curr_inst->opcode ==
				    ZYNQ_QSPI_FLASH_OPCODE_RDSR1) ||
				    (zqspi->curr_inst->opcode ==
				    ZYNQ_QSPI_FLASH_OPCODE_RDSR2)))) {
					zqspi->inst_response = 0;
					zynq_qspi_copy_read_data(zqspi, data,
						zqspi->curr_inst->inst_size);
				} else if (zqspi->bytes_to_receive < 4) {
					zynq_qspi_copy_read_data(zqspi, data,
						zqspi->bytes_to_receive);
				} else {
				if (zqspi->rxbuf) {
					memcpy(zqspi->rxbuf, &data, 4);
					zqspi->rxbuf += 4;
				}
				zqspi->bytes_to_receive -= 4;
			}
			rxindex++;
		}

		if (zqspi->bytes_to_transfer) {
			/* There is more data to send */
			zynq_qspi_fill_tx_fifo(zqspi,
					       ZYNQ_QSPI_RXFIFO_THRESHOLD);

			writel(ZYNQ_QSPI_IXR_ALL_MASK, &zynq_qspi_base->ier);
		} else {
			/*
			 * If transfer and receive is completed then only send
			 * complete signal
			 */
			if (!zqspi->bytes_to_receive) {
				/* return operation complete */
				writel(ZYNQ_QSPI_IXR_ALL_MASK,
				       &zynq_qspi_base->idisr);
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
static int zynq_qspi_start_transfer(struct spi_device *qspi,
			struct spi_transfer *transfer)
{
	struct zynq_qspi *zqspi = &qspi->master;
	static u8 current_u_page;
	u32 data = 0;
	u8 instruction = 0;
	u8 index;

	debug("%s: qspi: 0x%08x transfer: 0x%08x len: %d\n", __func__,
	      (u32)qspi, (u32)transfer, transfer->len);

	zqspi->txbuf = transfer->tx_buf;
	zqspi->rxbuf = transfer->rx_buf;
	zqspi->bytes_to_transfer = transfer->len;
	zqspi->bytes_to_receive = transfer->len;

	if (zqspi->txbuf)
		instruction = *(u8 *)zqspi->txbuf;

	if (instruction && zqspi->is_inst) {
		for (index = 0; index < ARRAY_SIZE(flash_inst); index++)
			if (instruction == flash_inst[index].opcode)
				break;

		/*
		 * Instruction might have already been transmitted. This is a
		 * 'data only' transfer
		 */
		if (index == ARRAY_SIZE(flash_inst))
			goto xfer_data;

		zqspi->curr_inst = &flash_inst[index];
		zqspi->inst_response = 1;

		if ((zqspi->is_dual == SF_DUAL_STACKED_FLASH) &&
				(current_u_page != zqspi->u_page)) {
			if (zqspi->u_page) {
				/* Configure two memories on shared bus
				 * by enabling upper mem
				 */
				writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
					ZYNQ_QSPI_LCFG_U_PAGE |
					(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
					ZYNQ_QSPI_FR_QOUT_CODE),
					&zynq_qspi_base->lcr);
			} else {
				/* Configure two memories on shared bus
				 * by enabling lower mem
				 */
				writel((ZYNQ_QSPI_LCFG_TWO_MEM_MASK |
					(1 << ZYNQ_QSPI_LCFG_DUMMY_SHIFT) |
					ZYNQ_QSPI_FR_QOUT_CODE),
					&zynq_qspi_base->lcr);
			}

			current_u_page = zqspi->u_page;
		}

		/* Get the instruction */
		data = 0;
		zynq_qspi_copy_write_data(zqspi, &data,
					zqspi->curr_inst->inst_size);

		/*
		 * Write the instruction to LSB of the FIFO. The core is
		 * designed such that it is not necessary to check whether the
		 * write FIFO is full before writing. However, write would be
		 * delayed if the user tries to write when write FIFO is full
		 */
		writel(data, &zynq_qspi_base->confr +
				(zqspi->curr_inst->offset / 4));

		/*
		 * Read status register and Read ID instructions don't require
		 * to ignore the extra bytes in response of instruction as
		 * response contains the value
		 */
		if ((instruction == ZYNQ_QSPI_FLASH_OPCODE_RDSR1) ||
		    (instruction == ZYNQ_QSPI_FLASH_OPCODE_RDSR2) ||
		    (instruction == ZYNQ_QSPI_FLASH_OPCODE_RDID) ||
		    (instruction == ZYNQ_QSPI_FLASH_OPCODE_BRRD) ||
		    (instruction == ZYNQ_QSPI_FLASH_OPCODE_RDEAR)) {
			if (zqspi->bytes_to_transfer < 4)
				zqspi->bytes_to_transfer = 0;
			else
				zqspi->bytes_to_transfer -= 3;
		}
	}

xfer_data:
	/*
	 * In case of Fast, Dual and Quad reads, transmit the instruction first.
	 * Address and dummy byte should be transmitted after instruction
	 * is transmitted
	 */
	if (((zqspi->is_inst == 0) && (zqspi->bytes_to_transfer)) ||
	    ((zqspi->bytes_to_transfer) &&
	     (instruction != ZYNQ_QSPI_FLASH_OPCODE_FR) &&
	     (instruction != ZYNQ_QSPI_FLASH_OPCODE_DR) &&
	     (instruction != ZYNQ_QSPI_FLASH_OPCODE_QR) &&
	     (instruction != ZYNQ_QSPI_FLASH_OPCODE_DIOR)))
		zynq_qspi_fill_tx_fifo(zqspi, ZYNQ_QSPI_FIFO_DEPTH);

	writel(ZYNQ_QSPI_IXR_ALL_MASK, &zynq_qspi_base->ier);
	/* Start the transfer by enabling manual start bit */

	/* wait for completion */
	do {
		data = zynq_qspi_irq_poll(zqspi);
	} while (data == 0);

	return (transfer->len) - (zqspi->bytes_to_transfer);
}

static int zynq_qspi_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	struct zynq_qspi *zqspi = &qspi->master;
	unsigned cs_change = 1;
	int status = 0;

	debug("%s\n", __func__);

	while (1) {
		if (transfer->bits_per_word || transfer->speed_hz) {
			status = zynq_qspi_setup_transfer(qspi, transfer);
			if (status < 0)
				break;
		}

		/* Select the chip if required */
		if (cs_change)
			zynq_qspi_chipselect(qspi, 1);

		cs_change = transfer->cs_change;

		if (!transfer->tx_buf && !transfer->rx_buf && transfer->len) {
			status = -1;
			break;
		}

		/* Request the transfer */
		if (transfer->len) {
			status = zynq_qspi_start_transfer(qspi, transfer);
			zqspi->is_inst = 0;
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
			zynq_qspi_chipselect(qspi, 0);

		break;
	}

	zynq_qspi_setup_transfer(qspi, NULL);

	return 0;
}

/*
 * zynq_qspi_check_is_dual_flash - checking for dual or single qspi
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
static int zynq_qspi_check_is_dual_flash(void)
{
	int is_dual = -1;
	int lower_mio = 0, upper_mio = 0, upper_mio_cs1 = 0;

	lower_mio = zynq_slcr_get_mio_pin_status("qspi0");
	if (lower_mio == ZYNQ_QSPI_MIO_NUM_QSPI0)
		is_dual = SF_SINGLE_FLASH;

	upper_mio_cs1 = zynq_slcr_get_mio_pin_status("qspi1_cs");
	if ((lower_mio == ZYNQ_QSPI_MIO_NUM_QSPI0) &&
	    (upper_mio_cs1 == ZYNQ_QSPI_MIO_NUM_QSPI1_CS))
		is_dual = SF_DUAL_STACKED_FLASH;

	upper_mio = zynq_slcr_get_mio_pin_status("qspi1");
	if ((lower_mio == ZYNQ_QSPI_MIO_NUM_QSPI0) &&
	    (upper_mio_cs1 == ZYNQ_QSPI_MIO_NUM_QSPI1_CS) &&
	    (upper_mio == ZYNQ_QSPI_MIO_NUM_QSPI1))
		is_dual = SF_DUAL_PARALLEL_FLASH;

	return is_dual;
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

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	int is_dual;
	unsigned long lqspi_frequency;
	struct zynq_qspi_slave *qspi;

	debug("%s: bus: %d cs: %d max_hz: %d mode: %d\n",
	      __func__, bus, cs, max_hz, mode);

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	is_dual = zynq_qspi_check_is_dual_flash();

	if (is_dual == -1) {
		printf("%s: No QSPI device detected based on MIO settings\n",
		       __func__);
		return NULL;
	}

	zynq_qspi_init_hw(is_dual, cs);

	qspi = spi_alloc_slave(struct zynq_qspi_slave, bus, cs);
	if (!qspi) {
		printf("%s: Fail to allocate zynq_qspi_slave\n", __func__);
		return NULL;
	}

	lqspi_frequency = zynq_clk_get_rate(lqspi_clk);
	if (!lqspi_frequency) {
		debug("Defaulting to 200000000 Hz qspi clk");
		qspi->qspi.master.input_clk_hz = 200000000;
	} else {
		qspi->qspi.master.input_clk_hz = lqspi_frequency;
		debug("Qspi clk frequency set to %ld Hz\n", lqspi_frequency);
	}

	qspi->slave.option = is_dual;
	qspi->slave.op_mode_rx = SPI_OPM_RX_QOF;
	qspi->slave.op_mode_tx = SPI_OPM_TX_QPP;
	qspi->qspi.master.speed_hz = qspi->qspi.master.input_clk_hz / 2;
	qspi->qspi.max_speed_hz = (max_hz < qspi->qspi.master.speed_hz) ?
								max_hz : qspi->qspi.master.speed_hz;
	qspi->qspi.master.is_dual = is_dual;
	qspi->qspi.mode = mode;
	qspi->qspi.chip_select = 0;
	qspi->qspi.bits_per_word = 32;
	zynq_qspi_setup_transfer(&qspi->qspi, NULL);

	return &qspi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct zynq_qspi_slave *qspi;

	debug("%s: slave: 0x%08x\n", __func__, (u32)slave);

	qspi = to_zynq_qspi_slave(slave);
	free(qspi);
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
	struct zynq_qspi_slave *qspi;
	struct spi_transfer transfer;

	debug("%s: slave: 0x%08x bitlen: %d dout: 0x%08x ", __func__,
	      (u32)slave, bitlen, (u32)dout);
	debug("din: 0x%08x flags: 0x%lx\n", (u32)din, flags);

	qspi = (struct zynq_qspi_slave *)slave;
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

	transfer.delay_usecs = 0;
	transfer.bits_per_word = 32;
	transfer.speed_hz = qspi->qspi.max_speed_hz;

	zynq_qspi_transfer(&qspi->qspi, &transfer);

	return 0;
}
