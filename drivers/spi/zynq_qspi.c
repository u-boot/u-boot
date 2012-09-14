/*
 * (C) Copyright 2011 Xilinx
 *
 * Xilinx PSS Quad-SPI (QSPI) controller driver (master mode only)
 * based on Xilinx PSS SPI Driver (xspips.c)
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

#ifdef LINUX_ONLY_NOT_UBOOT
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/xilinx_devices.h>
#else

#include "xbasic_types.h"
#include <common.h>
#include <malloc.h>

#include <linux/mtd/compat.h>
#include <ubi_uboot.h>
#include <spi.h>

#include "zynq_qspi.h"

#define DEBUG
#define DEBUG_REG
#undef DEBUG
#undef DEBUG_REG

#endif

/****** stubs to make this Linux driver build in this environment **/

#define spin_lock_irqsave(__X__, flags) \
		flags = 0;
#define spin_unlock_irqrestore(__X__, flags) \
		flags |= 0;
#define MODULE_ALIAS(__X__)

#define dev_dbg(dev, format, arg...) \
        printf(format , ## arg)

#define dev_err(dev, format, arg...) \
        printf(format , ## arg)

#define wait_for_completion(__X__) \
	{ \
		u32 data; \
\
		do { \
			data = xqspips_irq_poll(xqspi); \
		} while (data == 0); \
	}

#define __devinit
#define __devinitdata
#define __devexit
#define __devexitdata
#define __force

extern void XIo_Out32(u32 OutAddress, u32 Value);
extern u32  XIo_In32(u32 InAddress);

#define spi_master_get_devdata(__X__)  (&(__X__))
#define INIT_COMPLETION(__X__)

/**
 * enum irqreturn
 * @IRQ_NONE            interrupt was not from this device
 * @IRQ_HANDLED         interrupt was handled by this device
 * @IRQ_WAKE_THREAD     handler requests to wake the handler thread
 */
enum irqreturn {
        IRQ_NONE,
        IRQ_HANDLED,
        IRQ_WAKE_THREAD,
};
typedef enum irqreturn irqreturn_t;

/*******************************************************************/

#undef HACK_WRITE_NO_DELAY

/*
 * Name of this driver
 */
#define DRIVER_NAME			"Xilinx_PSS_QSPI"

/*
 * Register offset definitions
 */
#define XQSPIPSS_CONFIG_OFFSET		0x00 /* Configuration  Register, RW */
#define XQSPIPSS_STATUS_OFFSET 		0x04 /* Interrupt Status Register, RO */
#define XQSPIPSS_IEN_OFFSET		0x08 /* Interrupt Enable Register, WO */
#define XQSPIPSS_IDIS_OFFSET		0x0C /* Interrupt Disable Reg, WO */
#define XQSPIPSS_IMASK_OFFSET		0x10 /* Interrupt Enabled Mask Reg,RO */
#define XQSPIPSS_ENABLE_OFFSET		0x14 /* Enable/Disable Register, RW */
#define XQSPIPSS_DELAY_OFFSET 		0x18 /* Delay Register, RW */
#define XQSPIPSS_TXD_00_00_OFFSET	0x1C /* Transmit 4-byte inst, WO */
#define XQSPIPSS_TXD_00_01_OFFSET	0x80 /* Transmit 1-byte inst, WO */
#define XQSPIPSS_TXD_00_10_OFFSET	0x84 /* Transmit 2-byte inst, WO */
#define XQSPIPSS_TXD_00_11_OFFSET	0x88 /* Transmit 3-byte inst, WO */
#define XQSPIPSS_RXD_OFFSET		0x20 /* Data Receive Register, RO */
#define XQSPIPSS_SIC_OFFSET		0x24 /* Slave Idle Count Register, RW */
#define XQSPIPSS_TX_THRESH_OFFSET	0x28 /* TX FIFO Watermark Reg, RW */
#define XQSPIPSS_RX_THRESH_OFFSET	0x2C /* RX FIFO Watermark Reg, RW */
#define XQSPIPSS_GPIO_OFFSET		0x30 /* GPIO Register, RW */
#define XQSPIPSS_LINEAR_CFG_OFFSET	0xA0 /* Linear Adapter Config Ref, RW */
#define XQSPIPSS_MOD_ID_OFFSET		0xFC /* Module ID Register, RO */

/*
 * QSPI Configuration Register bit Masks
 *
 * This register contains various control bits that effect the operation
 * of the QSPI controller
 */
#define XQSPIPSS_CONFIG_MANSRT_MASK	0x00010000 /* Manual TX Start */
#define XQSPIPSS_CONFIG_CPHA_MASK	0x00000004 /* Clock Phase Control */
#define XQSPIPSS_CONFIG_CPOL_MASK	0x00000002 /* Clock Polarity Control */
#define XQSPIPSS_CONFIG_SSCTRL_MASK	0x00003C00 /* Slave Select Mask */

/*
 * QSPI Interrupt Registers bit Masks
 *
 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
 * bit definitions.
 */
#define XQSPIPSS_IXR_TXNFULL_MASK	0x00000004 /* QSPI TX FIFO Overflow */
#define XQSPIPSS_IXR_TXFULL_MASK	0x00000008 /* QSPI TX FIFO is full */
#define XQSPIPSS_IXR_RXNEMTY_MASK	0x00000010 /* QSPI RX FIFO Not Empty */
#define XQSPIPSS_IXR_ALL_MASK		(XQSPIPSS_IXR_TXNFULL_MASK | \
					 XQSPIPSS_IXR_RXNEMTY_MASK)

/*
 * QSPI Enable Register bit Masks
 *
 * This register is used to enable or disable the QSPI controller
 */
#define XQSPIPSS_ENABLE_ENABLE_MASK	0x00000001 /* QSPI Enable Bit Mask */

/*
 * The modebits configurable by the driver to make the SPI support different
 * data formats
 */
#define MODEBITS			(SPI_CPOL | SPI_CPHA)

/*
 * Definitions for the status of queue
 */
#define XQSPIPSS_QUEUE_STOPPED		0
#define XQSPIPSS_QUEUE_RUNNING		1

/*
 * Definitions of the flash commands
 */
/* Flash opcodes in ascending order */
#define	XQSPIPSS_FLASH_OPCODE_WRSR	0x01	/* Write status register */
#define	XQSPIPSS_FLASH_OPCODE_PP	0x02	/* Page program */
#define	XQSPIPSS_FLASH_OPCODE_NORM_READ	0x03	/* Normal read data bytes */
#define	XQSPIPSS_FLASH_OPCODE_WRDS	0x04	/* Write disable */
#define	XQSPIPSS_FLASH_OPCODE_RDSR1	0x05	/* Read status register 1 */
#define	XQSPIPSS_FLASH_OPCODE_WREN	0x06	/* Write enable */
#define	XQSPIPSS_FLASH_OPCODE_FAST_READ	0x0B	/* Fast read data bytes */
#define	XQSPIPSS_FLASH_OPCODE_BE_4K	0x20	/* Erase 4KiB block */
#define	XQSPIPSS_FLASH_OPCODE_RDSR2	0x35	/* Read status register 2 */
#define	XQSPIPSS_FLASH_OPCODE_DUAL_READ	0x3B	/* Dual read data bytes */
#define	XQSPIPSS_FLASH_OPCODE_BE_32K	0x52	/* Erase 32KiB block */
#define	XQSPIPSS_FLASH_OPCODE_QUAD_READ	0x6B	/* Quad read data bytes */
#define	XQSPIPSS_FLASH_OPCODE_ERASE_SUS	0x75	/* Erase suspend */
#define	XQSPIPSS_FLASH_OPCODE_ERASE_RES	0x7A	/* Erase resume */
#define	XQSPIPSS_FLASH_OPCODE_RDID	0x9F	/* Read JEDEC ID */
#define	XQSPIPSS_FLASH_OPCODE_BE	0xC7	/* Erase whole flash block */
#define	XQSPIPSS_FLASH_OPCODE_SE	0xD8	/* Sector erase (usually 64KB)*/

/*
 * Macros for the QSPI controller read/write
 */
#ifdef LINUX_ONLY_NOT_UBOOT
#define xqspips_read(addr)		__raw_readl(addr)
#define xqspips_write(addr, val)	__raw_writel((val), (addr))
#else
static inline
u32 xqspips_read(void *addr)
{					
	u32 val;

	val =  XIo_In32((unsigned)addr);
#ifdef DEBUG_REG
	printf("xqspips_read:  addr: 0x%08x = 0x%08x\n",
		addr, val);
#endif
	return val;
}
static inline
void xqspips_write(void *addr, u32 val)
{
#ifdef DEBUG_REG
	printf("xqspips_write: addr: 0x%08x = 0x%08x\n",
		addr, val);
#endif
	XIo_Out32((unsigned)addr, val);
}
#endif

#ifdef LINUX_ONLY_NOT_UBOOT

/**
 * struct xqspips - Defines qspi driver instance
 * @workqueue:		Queue of all the transfers
 * @work:		Information about current transfer
 * @queue:		Head of the queue
 * @queue_state:	Queue status
 * @regs:		Virtual address of the QSPI controller registers
 * @input_clk_hz:	Input clock frequency of the QSPI controller in Hz
 * @irq:		IRQ number
 * @speed_hz:		Current QSPI bus clock speed in Hz
 * @trans_queue_lock:	Lock used for accessing transfer queue
 * @config_reg_lock:	Lock used for accessing configuration register
 * @txbuf: 		Pointer	to the TX buffer
 * @rxbuf:		Pointer to the RX buffer
 * @bytes_to_transfer:	Number of bytes left to transfer
 * @bytes_to_receive:	Number of bytes left to receive
 * @dev_busy:		Device busy flag
 * @done:		Transfer complete status
 * @curr_inst:		Current executing instruction format
 * @inst_response:	Responce to the instruction or data
 * @is_inst:		Flag to indicate the first message in a Transfer request
 **/
struct xqspips {
	struct workqueue_struct *workqueue;
	struct work_struct work;
	struct list_head queue;

	u8 queue_state;
	void __iomem *regs;
	u32 input_clk_hz;
	u32 irq;
	u32 speed_hz;
	spinlock_t trans_queue_lock;
	spinlock_t config_reg_lock;
	const void *txbuf;
	void *rxbuf;
	int bytes_to_transfer;
	int bytes_to_receive;
	u8 dev_busy;
	struct completion done;
	struct xqspips_inst_format *curr_inst;
	u8 inst_response;
	bool is_inst;
};

#endif

/**
 * struct xqspips_inst_format - Defines qspi flash instruction format
 * @opcode:		Operational code of instruction
 * @inst_size:		Size of the instruction including address bytes
 * @offset:		Register address where instruction has to be written
 **/
struct xqspips_inst_format {
	u8 opcode;
	u8 inst_size;
	u8 offset;
};

/*
 * List of all the QSPI instructions and its format
 */
static struct xqspips_inst_format __devinitdata flash_inst[] = {
	{ XQSPIPSS_FLASH_OPCODE_WREN, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_WRDS, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_RDSR1, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_RDSR2, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_WRSR, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_PP, 4, XQSPIPSS_TXD_00_00_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_SE, 4, XQSPIPSS_TXD_00_00_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_BE_32K, 4, XQSPIPSS_TXD_00_00_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_BE_4K, 4, XQSPIPSS_TXD_00_00_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_BE, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_ERASE_SUS, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_ERASE_RES, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_RDID, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_NORM_READ, 4, XQSPIPSS_TXD_00_00_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_FAST_READ, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_DUAL_READ, 1, XQSPIPSS_TXD_00_01_OFFSET },
	{ XQSPIPSS_FLASH_OPCODE_QUAD_READ, 1, XQSPIPSS_TXD_00_01_OFFSET },
	/* Add all the instructions supported by the flash device */
};

/**
 * xqspips_init_hw - Initialize the hardware
 * @regs_base:		Base address of QSPI controller
 * @is_dual:		Indicates whether dual memories are used
 *
 * The default settings of the QSPI controller's configurable parameters on
 * reset are
 *	- Master mode
 * 	- Baud rate divisor is set to 2
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
 **/
void xqspips_init_hw(void __iomem *regs_base, unsigned int is_dual)
{
	u32 config_reg;

	xqspips_write(regs_base + XQSPIPSS_ENABLE_OFFSET,
		~XQSPIPSS_ENABLE_ENABLE_MASK);
	xqspips_write(regs_base + XQSPIPSS_IDIS_OFFSET, 0x7F);

	/* Disable linear mode as the boot loader may have used it */
	xqspips_write(regs_base + XQSPIPSS_LINEAR_CFG_OFFSET, 0);

	/* Clear the RX FIFO */
	while (xqspips_read(regs_base + XQSPIPSS_STATUS_OFFSET) &
			XQSPIPSS_IXR_RXNEMTY_MASK)
		xqspips_read(regs_base + XQSPIPSS_RXD_OFFSET);

	xqspips_write(regs_base + XQSPIPSS_STATUS_OFFSET , 0x7F);
	config_reg = xqspips_read(regs_base + XQSPIPSS_CONFIG_OFFSET);
	config_reg &= 0xFBFFFFFF; /* Set little endian mode of TX FIFO */
	config_reg |= 0x8000FCC1;
	xqspips_write(regs_base + XQSPIPSS_CONFIG_OFFSET, config_reg);

	if (is_dual == 1)
		/* Enable two memories on seperate buses */
		xqspips_write(regs_base + XQSPIPSS_LINEAR_CFG_OFFSET,
				0x6400016B);

	xqspips_write(regs_base + XQSPIPSS_ENABLE_OFFSET,
			XQSPIPSS_ENABLE_ENABLE_MASK);
}

/**
 * xqspips_copy_read_data - Copy data to RX buffer
 * @xqspi:	Pointer to the xqspips structure
 * @data:	The 32 bit variable where data is stored
 * @size:	Number of bytes to be copied from data to RX buffer
 **/
static void xqspips_copy_read_data(struct xqspips *xqspi, u32 data, u8 size)
{
	u8 byte3;

#ifdef DEBUG
	printf("%s data 0x%04x rxbuf addr: 0x%08x size %d\n",
		__FUNCTION__, data, (unsigned)(xqspi->rxbuf), size);
#endif

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
#ifdef LINUX_ONLY_NOT_UBOOT
			(*(u32 *)xqspi->rxbuf) = data;
#else
			/* Can not assume word aligned buffer */
			memcpy(xqspi->rxbuf, &data,  size);
#endif
			xqspi->rxbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	}
	xqspi->bytes_to_receive -= size;
	if (xqspi->bytes_to_receive < 0) {
		xqspi->bytes_to_receive = 0;
	}
}

/**
 * xqspips_copy_write_data - Copy data from TX buffer
 * @xqspi:	Pointer to the xqspips structure
 * @data:	Pointer to the 32 bit variable where data is to be copied
 * @size:	Number of bytes to be copied from TX buffer to data
 **/
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
#ifdef LINUX_ONLY_NOT_UBOOT
			*data = *((u32 *)xqspi->txbuf);
#else
			/* Can not assume word aligned buffer */
			memcpy(data, xqspi->txbuf,  size);
#endif
			xqspi->txbuf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	} else
		*data = 0;

#ifdef DEBUG
	printf("%s data 0x%08x txbuf addr: 0x%08x size %d\n",
		__FUNCTION__, *data, (u32)xqspi->txbuf, size);
#endif

	xqspi->bytes_to_transfer -= size;
	if (xqspi->bytes_to_transfer < 0) {
		xqspi->bytes_to_transfer = 0;
	}
}

/**
 * xqspips_chipselect - Select or deselect the chip select line
 * @qspi:	Pointer to the spi_device structure
 * @is_on:	Select(1) or deselect (0) the chip select line
 **/
static void xqspips_chipselect(struct spi_device *qspi, int is_on)
{
	struct xqspips *xqspi = spi_master_get_devdata(qspi->master);
	u32 config_reg;
	unsigned long flags;

#ifdef DEBUG
	printf("xqspips_chipselect: is_on: %d\n", is_on);
#endif

	spin_lock_irqsave(&xqspi->config_reg_lock, flags);

	config_reg = xqspips_read(xqspi->regs + XQSPIPSS_CONFIG_OFFSET);

	if (is_on) {
		/* Select the slave */
		config_reg &= ~XQSPIPSS_CONFIG_SSCTRL_MASK;
		config_reg |= (((~(0x0001 << qspi->chip_select)) << 10) &
				XQSPIPSS_CONFIG_SSCTRL_MASK);
	} else
		/* Deselect the slave */
		config_reg |= XQSPIPSS_CONFIG_SSCTRL_MASK;

	xqspips_write(xqspi->regs + XQSPIPSS_CONFIG_OFFSET, config_reg);

	spin_unlock_irqrestore(&xqspi->config_reg_lock, flags);
}

/**
 * xqspips_setup_transfer - Configure QSPI controller for specified transfer
 * @qspi:	Pointer to the spi_device structure
 * @transfer:	Pointer to the spi_transfer structure which provides information
 *		about next transfer setup parameters
 *
 * Sets the operational mode of QSPI controller for the next QSPI transfer and
 * sets the requested clock frequency.
 *
 * returns:	0 on success and -EINVAL on invalid input parameter
 *
 * Note: If the requested frequency is not an exact match with what can be
 * obtained using the prescalar value, the driver sets the clock frequency which
 * is lower than the requested frequency (maximum lower) for the transfer. If
 * the requested frequency is higher or lower than that is supported by the QSPI
 * controller the driver will set the highest or lowest frequency supported by
 * controller.
 **/
int xqspips_setup_transfer(struct spi_device *qspi,
		struct spi_transfer *transfer)
{
	struct xqspips *xqspi = spi_master_get_devdata(qspi->master);
	u8 bits_per_word;
	u32 config_reg;
	u32 req_hz;
	u32 baud_rate_val = 0;
	unsigned long flags;

#ifdef DEBUG
	printf("xqspips_setup_transfer: qspi: 0x%08x transfer: 0x%08x\n",
		(u32)qspi, (u32)transfer);
#endif
	bits_per_word = (transfer) ?
			transfer->bits_per_word : qspi->bits_per_word;
	req_hz = (transfer) ? transfer->speed_hz : qspi->max_speed_hz;

	if (qspi->mode & ~MODEBITS) {
		dev_err(&qspi->dev, "%s, unsupported mode bits %x\n",
			__func__, qspi->mode & ~MODEBITS);
		return -EINVAL;
	}

	if (bits_per_word != 32) {
		bits_per_word = 32;
	}

	spin_lock_irqsave(&xqspi->config_reg_lock, flags);

	config_reg = xqspips_read(xqspi->regs + XQSPIPSS_CONFIG_OFFSET);

	/* Set the QSPI clock phase and clock polarity */
	config_reg &= (~XQSPIPSS_CONFIG_CPHA_MASK) &
				(~XQSPIPSS_CONFIG_CPOL_MASK);
	if (qspi->mode & SPI_CPHA)
		config_reg |= XQSPIPSS_CONFIG_CPHA_MASK;
	if (qspi->mode & SPI_CPOL)
		config_reg |= XQSPIPSS_CONFIG_CPOL_MASK;

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

	xqspips_write(xqspi->regs + XQSPIPSS_CONFIG_OFFSET, config_reg);

	spin_unlock_irqrestore(&xqspi->config_reg_lock, flags);

#ifdef DEBUG
	dev_dbg(&qspi->dev, "%s, mode %d, %u bits/w, %u clock speed\n",
		__func__, qspi->mode & MODEBITS, qspi->bits_per_word,
		xqspi->speed_hz);
#endif

	return 0;
}

#ifdef LINUX_ONLY_NOT_UBOOT
/**
 * xqspips_setup - Configure the QSPI controller
 * @qspi:	Pointer to the spi_device structure
 *
 * Sets the operational mode of QSPI controller for the next QSPI transfer, baud
 * rate and divisor value to setup the requested qspi clock.
 *
 * returns:	0 on success and error value on failure
 **/
static int xqspips_setup(struct spi_device *qspi)
{

	if (qspi->mode & SPI_LSB_FIRST)
		return -EINVAL;

	if (!qspi->max_speed_hz)
		return -EINVAL;

	if (!qspi->bits_per_word)
		qspi->bits_per_word = 32;

	return xqspips_setup_transfer(qspi, NULL);
}
#endif

/**
 * xqspips_fill_tx_fifo - Fills the TX FIFO with as many bytes as possible
 * @xqspi:	Pointer to the xqspips structure
 **/
static void xqspips_fill_tx_fifo(struct xqspips *xqspi)
{
	u32 data = 0;

	while ((!(xqspips_read(xqspi->regs + XQSPIPSS_STATUS_OFFSET) &
		XQSPIPSS_IXR_TXFULL_MASK)) && (xqspi->bytes_to_transfer > 0)) {
		if (xqspi->bytes_to_transfer < 4) {
			xqspips_copy_write_data(xqspi, &data,
				xqspi->bytes_to_transfer);
		} else {
			xqspips_copy_write_data(xqspi, &data, 4);
		}

		xqspips_write(xqspi->regs + XQSPIPSS_TXD_00_00_OFFSET, data);
	}
}

/**
 * xqspips_irq - Interrupt service routine of the QSPI controller
 * @irq:	IRQ number
 * @dev_id:	Pointer to the xqspi structure
 *
 * This function handles TX empty and Mode Fault interrupts only.
 * On TX empty interrupt this function reads the received data from RX FIFO and
 * fills the TX FIFO if there is any data remaining to be transferred.
 * On Mode Fault interrupt this function indicates that transfer is completed,
 * the SPI subsystem will identify the error as the remaining bytes to be
 * transferred is non-zero.
 *
 * returns:	IRQ_HANDLED always
 **/
#ifdef LINUX_ONLY_NOT_UBOOT
static irqreturn_t xqspips_irq(int irq, void *dev_id)
{
	struct xqspips *xqspi = dev_id;
#else
static int xqspips_irq_poll(struct xqspips *xqspi)
{
	int max_loop;
#endif
	u32 intr_status;

#ifdef DEBUG
	printf("xqspips_irq_poll: xqspi: 0x%08x\n",
		(u32)xqspi);
#endif

#ifdef LINUX_ONLY_NOT_UBOOT
	intr_status = xqspips_read(xqspi->regs + XQSPIPSS_STATUS_OFFSET);
#else
	/* u-boot: Poll until any of the interrupt status bits are set */
	max_loop = 0;
	do {
		intr_status = xqspips_read(xqspi->regs +
				XQSPIPSS_STATUS_OFFSET);
		max_loop ++;
	} while ((intr_status == 0) && (max_loop < 100000));
	if (intr_status == 0) {
		printf("xqspips_irq_poll: timeout\n");
		return 0;
	}
#endif

	xqspips_write(xqspi->regs + XQSPIPSS_STATUS_OFFSET , intr_status);
#ifndef LINUX_ONLY_NOT_UBOOT
	/* u-boot: Disable all interrupts */
	xqspips_write(xqspi->regs + XQSPIPSS_IDIS_OFFSET,
			XQSPIPSS_IXR_ALL_MASK);
#endif
	if ((intr_status & XQSPIPSS_IXR_TXNFULL_MASK) ||
		   (intr_status & XQSPIPSS_IXR_RXNEMTY_MASK)) {
		/* This bit is set when Tx FIFO has < THRESHOLD entries. We have
		   the THRESHOLD value set to 1, so this bit indicates Tx FIFO
		   is empty */
		u32 config_reg;

		/* Read out the data from the RX FIFO */
		while (xqspips_read(xqspi->regs + XQSPIPSS_STATUS_OFFSET) &
			XQSPIPSS_IXR_RXNEMTY_MASK) {
			u32 data;

			data = xqspips_read(xqspi->regs + XQSPIPSS_RXD_OFFSET);

			if ((xqspi->inst_response) &&
				(!((xqspi->curr_inst->opcode ==
					XQSPIPSS_FLASH_OPCODE_RDSR1) ||
				(xqspi->curr_inst->opcode ==
					XQSPIPSS_FLASH_OPCODE_RDSR2)))) {
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

			xqspips_write(xqspi->regs + XQSPIPSS_IEN_OFFSET,
					XQSPIPSS_IXR_ALL_MASK);

			spin_lock(&xqspi->config_reg_lock);
			config_reg = xqspips_read(xqspi->regs +
						XQSPIPSS_CONFIG_OFFSET);

			config_reg |= XQSPIPSS_CONFIG_MANSRT_MASK;
			xqspips_write(xqspi->regs + XQSPIPSS_CONFIG_OFFSET,
				config_reg);
			spin_unlock(&xqspi->config_reg_lock);
		} else {
			/* If transfer and receive is completed then only send
			 * complete signal */
			if (!xqspi->bytes_to_receive) {
#ifdef LINUX_ONLY_NOT_UBOOT
				complete(&xqspi->done);
#else
				/* u-boot: return "operation complete" */
				xqspips_write(xqspi->regs +
					XQSPIPSS_IDIS_OFFSET,
					XQSPIPSS_IXR_ALL_MASK);
				return 1;
#endif
			}
		}
	}

#ifdef LINUX_ONLY_NOT_UBOOT
	return IRQ_HANDLED;
#else
	/* u-boot: Transfer not complete */
	return 0;
#endif
}

/**
 * xqspips_start_transfer - Initiates the QSPI transfer
 * @qspi:	Pointer to the spi_device structure
 * @transfer:	Pointer to the spi_transfer structure which provide information
 *		about next transfer parameters
 *
 * This function fills the TX FIFO, starts the QSPI transfer, and waits for the
 * transfer to be completed.
 *
 * returns:	Number of bytes transferred in the last transfer
 **/
static int xqspips_start_transfer(struct spi_device *qspi,
			struct spi_transfer *transfer)
{
	struct xqspips *xqspi = spi_master_get_devdata(qspi->master);
	u32 config_reg;
	unsigned long flags;
	u32 data = 0;
	u8 instruction = 0;
	u8 index;
#ifdef HACK_WRITE_NO_DELAY
	static bool no_delay = 0;
#endif

#ifdef DEBUG
	printf("%s: qspi: 0x%08x transfer: 0x%08x len: %d\n",
		__func__, (u32)qspi, (u32)transfer, transfer->len);
#endif

	xqspi->txbuf = transfer->tx_buf;
	xqspi->rxbuf = transfer->rx_buf;
	xqspi->bytes_to_transfer = transfer->len;
	xqspi->bytes_to_receive = transfer->len;

#ifdef HACK_WRITE_NO_DELAY
	if (no_delay) {
		/* Indicates Page programm command + address is already in Tx
		 * FIFO. We need to receive extra 4 bytes for command + address
		 */
		xqspi->bytes_to_receive += 4;
		no_delay = 0;
	}
#endif

	if (xqspi->txbuf)
		instruction = *(u8 *)xqspi->txbuf;

	if (instruction && xqspi->is_inst) {
		for (index = 0 ; index < ARRAY_SIZE(flash_inst); index++)
			if (instruction == flash_inst[index].opcode)
				break;

		/* Instruction might have already been transmitted. This is a
		 * 'data only' transfer */
		if (index == ARRAY_SIZE(flash_inst))
			goto xfer_data;

		xqspi->curr_inst = &flash_inst[index];
		xqspi->inst_response = 1;

		/* In case of dual memories, convert 25 bit address to 24 bit
		 * address before transmitting to the 2 memories
		 */
		if ((xqspi->is_dual == 1) &&
		    ((instruction == XQSPIPSS_FLASH_OPCODE_PP) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_SE) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_BE_32K) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_BE_4K) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_BE) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_NORM_READ) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_FAST_READ) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_DUAL_READ) ||
		    (instruction == XQSPIPSS_FLASH_OPCODE_QUAD_READ))) {

			u8 *ptr = (u8*) (xqspi->txbuf);
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

		/* Write the instruction to LSB of the FIFO. The core is
		 * designed such that it is not necessary to check whether the
		 * write FIFO is full before writing. However, write would be
		 * delayed if the user tries to write when write FIFO is full
		 */
		xqspips_write(xqspi->regs + xqspi->curr_inst->offset, data);

#ifdef HACK_WRITE_NO_DELAY
		if (xqspi->curr_inst->opcode == XQSPIPSS_FLASH_OPCODE_PP) {
			/* Write instruction + address to the Tx FIFO, but do
			 * not start transmission yet. Wait for the next
			 * spi_message with data, and start transmission after
			 * data is filled into the FIFO
			 */
			no_delay = 1;
			return (transfer->len);
		}
#endif

		/* Read status register and Read ID instructions don't require
		 * to ignore the extra bytes in response of instruction as
		 * response contains the value */
		if ((instruction == XQSPIPSS_FLASH_OPCODE_RDSR1) ||
			(instruction == XQSPIPSS_FLASH_OPCODE_RDSR2) ||
			(instruction == XQSPIPSS_FLASH_OPCODE_RDID)) {
			if (xqspi->bytes_to_transfer < 4)
				xqspi->bytes_to_transfer = 0;
			else
				xqspi->bytes_to_transfer -= 3;
		}
	}

xfer_data:
	INIT_COMPLETION(xqspi->done);

	/* In case of Fast, Dual and Quad reads, transmit the instruction first.
	 * Address and dummy byte should be transmitted after instruction
	 * is transmitted */
	if (((xqspi->is_inst == 0) && (xqspi->bytes_to_transfer)) ||
	     ((xqspi->bytes_to_transfer) &&
	      (instruction != XQSPIPSS_FLASH_OPCODE_FAST_READ) &&
	      (instruction != XQSPIPSS_FLASH_OPCODE_DUAL_READ) &&
	      (instruction != XQSPIPSS_FLASH_OPCODE_QUAD_READ)))
		xqspips_fill_tx_fifo(xqspi);
	xqspips_write(xqspi->regs + XQSPIPSS_IEN_OFFSET,
			XQSPIPSS_IXR_ALL_MASK);
	/* Start the transfer by enabling manual start bit */
	spin_lock_irqsave(&xqspi->config_reg_lock, flags);
	config_reg = xqspips_read(xqspi->regs +
			XQSPIPSS_CONFIG_OFFSET) | XQSPIPSS_CONFIG_MANSRT_MASK;
	xqspips_write(xqspi->regs + XQSPIPSS_CONFIG_OFFSET, config_reg);
	spin_unlock_irqrestore(&xqspi->config_reg_lock, flags);

	wait_for_completion(&xqspi->done);

	return (transfer->len) - (xqspi->bytes_to_transfer);
}

#ifdef LINUX_ONLY_NOT_UBOOT
/**
 * xqspips_work_queue - Get the request from queue to perform transfers
 * @work:	Pointer to the work_struct structure
 **/
static void xqspips_work_queue(struct work_struct *work)
#else
int
xqspips_transfer(struct spi_device *qspi, struct spi_transfer *transfer)
#endif
{
#ifdef LINUX_ONLY_NOT_UBOOT
	struct xqspips *xqspi = container_of(work, struct xqspips, work);
#else
	struct xqspips *xqspi = spi_master_get_devdata(qspi->master);
#endif
	unsigned long flags;

	spin_lock_irqsave(&xqspi->trans_queue_lock, flags);
	xqspi->dev_busy = 1;

#ifdef DEBUG
	printf("xqspips_transfer\n");
#endif

#ifdef LINUX_ONLY_NOT_UBOOT
	/* Check if list is empty or queue is stoped */
	if (list_empty(&xqspi->queue) ||
		xqspi->queue_state == XQSPIPSS_QUEUE_STOPPED) {
		xqspi->dev_busy = 0;
		spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);
		return;
	}
#endif

#ifdef LINUX_ONLY_NOT_UBOOT
	/* Keep requesting transfer till list is empty */
	while (!list_empty(&xqspi->queue)) {
		struct spi_message *msg;
		struct spi_device *qspi;
		struct spi_transfer *transfer = NULL;
		unsigned cs_change = 1;
		int status = 0;

		msg = container_of(xqspi->queue.next, struct spi_message,
					queue);
		list_del_init(&msg->queue);
		spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);
		qspi = msg->spi;
		xqspi->is_inst = 1;

		list_for_each_entry(transfer, &msg->transfers, transfer_list) {
#else
	{
		unsigned cs_change = 1;
		int status = 0;

		while (1) {
#endif
			if (transfer->bits_per_word || transfer->speed_hz) {
				status =
					xqspips_setup_transfer(qspi, transfer);
				if (status < 0)
					break;
			}

			/* Select the chip if required */
			if (cs_change)
				xqspips_chipselect(qspi, 1);

			cs_change = transfer->cs_change;

			if (!transfer->tx_buf && !transfer->rx_buf &&
				transfer->len) {
				status = -EINVAL;
				break;
			}

			/* Request the transfer */
			if (transfer->len) {
				status =
					xqspips_start_transfer(qspi, transfer);
				xqspi->is_inst = 0;
			}

			if (status != transfer->len) {
				if (status > 0)
					status = -EMSGSIZE;
				break;
			}
#ifdef LINUX_ONLY_NOT_UBOOT
			msg->actual_length += status;
#endif
			status = 0;

			if (transfer->delay_usecs)
				udelay(transfer->delay_usecs);

			if (cs_change)
				/* Deselect the chip */
				xqspips_chipselect(qspi, 0);

#ifdef LINUX_ONLY_NOT_UBOOT
			if (transfer->transfer_list.next == &msg->transfers)
				break;
		}

		msg->status = status;
		msg->complete(msg->context);
#else
			break;
		}
#endif

		xqspips_setup_transfer(qspi, NULL);

#ifdef LINUX_ONLY_NOT_UBOOT
		if (!(status == 0 && cs_change))
			xqspips_chipselect(qspi, 0);
#endif

		spin_lock_irqsave(&xqspi->trans_queue_lock, flags);
	}

	xqspi->dev_busy = 0;
	spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);

#ifndef LINUX_ONLY_NOT_UBOOT
	return 0;
#endif
}

#ifdef LINUX_ONLY_NOT_UBOOT
/**
 * xqspips_transfer - Add a new transfer request at the tail of work queue
 * @qspi:	Pointer to the spi_device structure
 * @message:	Pointer to the spi_transfer structure which provides information
 *		about next transfer parameters
 *
 * returns:	0 on success, -EINVAL on invalid input parameter and
 *		-ESHUTDOWN if queue is stopped by module unload function
 **/
static int
xqspips_transfer(struct spi_device *qspi, struct spi_message *message)
{
	struct xqspips *xqspi = spi_master_get_devdata(qspi->master);
	struct spi_transfer *transfer;
	unsigned long flags;

	if (xqspi->queue_state == XQSPIPSS_QUEUE_STOPPED)
		return -ESHUTDOWN;

	message->actual_length = 0;
	message->status = -EINPROGRESS;

	/* Check each transfer's parameters */
	list_for_each_entry(transfer, &message->transfers, transfer_list) {
		u8 bits_per_word =
			transfer->bits_per_word ? : qspi->bits_per_word;

		bits_per_word = bits_per_word ? : 32;
		if (!transfer->tx_buf && !transfer->rx_buf && transfer->len)
			return -EINVAL;
		/* QSPI controller supports only 32 bit transfers whereas higher
		 * layer drivers request 8 bit transfers. Re-visit at a later
		 * time */
		/* if (bits_per_word != 32)
			return -EINVAL; */
	}

	spin_lock_irqsave(&xqspi->trans_queue_lock, flags);
	list_add_tail(&message->queue, &xqspi->queue);
	if (!xqspi->dev_busy)
		queue_work(xqspi->workqueue, &xqspi->work);
	spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);

	return 0;
}

/**
 * xqspips_start_queue - Starts the queue of the QSPI driver
 * @xqspi:	Pointer to the xqspips structure
 *
 * returns:	0 on success and -EBUSY if queue is already running or device is
 *		busy
 **/
static inline int xqspips_start_queue(struct xqspips *xqspi)
{
	unsigned long flags;

	spin_lock_irqsave(&xqspi->trans_queue_lock, flags);

	if (xqspi->queue_state == XQSPIPSS_QUEUE_RUNNING || xqspi->dev_busy) {
		spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);
		return -EBUSY;
	}

	xqspi->queue_state = XQSPIPSS_QUEUE_RUNNING;
	spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);

	return 0;
}

/**
 * xqspips_stop_queue - Stops the queue of the QSPI driver
 * @xqspi:	Pointer to the xqspips structure
 *
 * This function waits till queue is empty and then stops the queue.
 * Maximum time out is set to 5 seconds.
 *
 * returns:	0 on success and -EBUSY if queue is not empty or device is busy
 **/
static inline int xqspips_stop_queue(struct xqspips *xqspi)
{
	unsigned long flags;
	unsigned limit = 500;
	int ret = 0;

	if (xqspi->queue_state != XQSPIPSS_QUEUE_RUNNING)
		return ret;

	spin_lock_irqsave(&xqspi->trans_queue_lock, flags);

	while ((!list_empty(&xqspi->queue) || xqspi->dev_busy) && limit--) {
		spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);
		msleep(10);
		spin_lock_irqsave(&xqspi->trans_queue_lock, flags);
	}

	if (!list_empty(&xqspi->queue) || xqspi->dev_busy)
		ret = -EBUSY;

	if (ret == 0)
		xqspi->queue_state = XQSPIPSS_QUEUE_STOPPED;

	spin_unlock_irqrestore(&xqspi->trans_queue_lock, flags);

	return ret;
}

/**
 * xqspips_destroy_queue - Destroys the queue of the QSPI driver
 * @xqspi:	Pointer to the xqspips structure
 *
 * returns:	0 on success and error value on failure
 **/
static inline int xqspips_destroy_queue(struct xqspips *xqspi)
{
	int ret;

	ret = xqspips_stop_queue(xqspi);
	if (ret != 0)
		return ret;

	destroy_workqueue(xqspi->workqueue);

	return 0;
}

/**
 * xqspips_probe - Probe method for the QSPI driver
 * @dev:	Pointer to the platform_device structure
 *
 * This function initializes the driver data structures and the hardware.
 *
 * returns:	0 on success and error value on failure
 **/
static int __devinit xqspips_probe(struct platform_device *dev)
{
	int ret = 0;
	struct spi_master *master;
	struct xqspips *xqspi;
	struct resource *r;
	struct xspi_platform_data *platform_info;

	master = spi_alloc_master(&dev->dev, sizeof(struct xqspips));
	if (master == NULL)
		return -ENOMEM;

	xqspi = spi_master_get_devdata(master);
	platform_set_drvdata(dev, master);

	platform_info = dev->dev.platform_data;
	if (platform_info == NULL) {
		ret = -ENODEV;
		dev_err(&dev->dev, "platform data not available\n");
		goto put_master;
	}

	r = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		ret = -ENODEV;
		dev_err(&dev->dev, "platform_get_resource failed\n");
		goto put_master;
	}

	if (!request_mem_region(r->start,
			r->end - r->start + 1, dev->name)) {
		ret = -ENXIO;
		dev_err(&dev->dev, "request_mem_region failed\n");
		goto put_master;
	}

	xqspi->regs = ioremap(r->start, r->end - r->start + 1);
	if (xqspi->regs == NULL) {
		ret = -ENOMEM;
		dev_err(&dev->dev, "ioremap failed\n");
		goto release_mem;
	}

	xqspi->irq = platform_get_irq(dev, 0);
	if (xqspi->irq < 0) {
		ret = -ENXIO;
		dev_err(&dev->dev, "irq resource not found\n");
		goto unmap_io;
	}

	ret = request_irq(xqspi->irq, xqspips_irq, 0, dev->name, xqspi);
	if (ret != 0) {
		ret = -ENXIO;
		dev_err(&dev->dev, "request_irq failed\n");
		goto unmap_io;
	}

	/* QSPI controller initializations */
	xqspips_init_hw(xqspi->regs);

	init_completion(&xqspi->done);
	master->bus_num = platform_info->bus_num;
	master->num_chipselect = platform_info->num_chipselect;
	master->setup = xqspips_setup;
	master->transfer = xqspips_transfer;
	xqspi->input_clk_hz = platform_info->speed_hz;
	xqspi->speed_hz = platform_info->speed_hz / 2;
	xqspi->dev_busy = 0;

	INIT_LIST_HEAD(&xqspi->queue);
	spin_lock_init(&xqspi->trans_queue_lock);
	spin_lock_init(&xqspi->config_reg_lock);

	xqspi->queue_state = XQSPIPSS_QUEUE_STOPPED;
	xqspi->dev_busy = 0;

	INIT_WORK(&xqspi->work, xqspips_work_queue);
	xqspi->workqueue =
		create_singlethread_workqueue(dev_name(&master->dev));
	if (!xqspi->workqueue) {
		ret = -ENOMEM;
		dev_err(&dev->dev, "problem initializing queue\n");
		goto free_irq;
	}

	ret = xqspips_start_queue(xqspi);
	if (ret != 0) {
		dev_err(&dev->dev, "problem starting queue\n");
		goto remove_queue;
	}

	ret = spi_register_master(master);
	if (ret) {
		dev_err(&dev->dev, "spi_register_master failed\n");
		goto remove_queue;
	}

	dev_info(&dev->dev, "at 0x%08X mapped to 0x%08X, irq=%d\n", r->start,
		 (u32 __force)xqspi->regs, xqspi->irq);

	return ret;

remove_queue:
	(void)xqspips_destroy_queue(xqspi);
free_irq:
	free_irq(xqspi->irq, xqspi);
unmap_io:
	iounmap(xqspi->regs);
release_mem:
	release_mem_region(r->start, r->end - r->start + 1);
put_master:
	platform_set_drvdata(dev, NULL);
	spi_master_put(master);
	return ret;
}

/**
 * xqspips_remove - Remove method for the QSPI driver
 * @dev:	Pointer to the platform_device structure
 *
 * This function is called if a device is physically removed from the system or
 * if the driver module is being unloaded. It frees all resources allocated to
 * the device.
 *
 * returns:	0 on success and error value on failure
 **/
static int __devexit xqspips_remove(struct platform_device *dev)
{
	struct spi_master *master = platform_get_drvdata(dev);
	struct xqspips *xqspi = spi_master_get_devdata(master);
	struct resource *r;
	int ret = 0;

	r = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		dev_err(&dev->dev, "platform_get_resource failed\n");
		return -ENODEV;
	}

	ret = xqspips_destroy_queue(xqspi);
	if (ret != 0)
		return ret;

	xqspips_write(xqspi->regs + XQSPIPSS_ENABLE_OFFSET,
			~XQSPIPSS_ENABLE_ENABLE_MASK);

	free_irq(xqspi->irq, xqspi);
	iounmap(xqspi->regs);
	release_mem_region(r->start, r->end - r->start + 1);

	spi_unregister_master(master);
	spi_master_put(master);

	/* Prevent double remove */
	platform_set_drvdata(dev, NULL);

	dev_dbg(&dev->dev, "remove succeeded\n");
	return 0;
}

/* Work with hotplug and coldplug */
MODULE_ALIAS("platform:" DRIVER_NAME);

/*
 * xqspips_driver - This structure defines the QSPI platform driver
 */
static struct platform_driver xqspips_driver = {
	.probe	= xqspips_probe,
	.remove	= __devexit_p(xqspips_remove),
	.suspend = NULL,
	.resume = NULL,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * xqspips_init - QSPI driver module initialization function
 *
 * returns:	0 on success and error value on failure
 **/
static int __init xqspips_init(void)
{
	return platform_driver_register(&xqspips_driver);
}

module_init(xqspips_init);

/**
 * xqspips_exit - QSPI driver module exit function
 **/
static void __exit xqspips_exit(void)
{
	platform_driver_unregister(&xqspips_driver);
}

module_exit(xqspips_exit);

MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION("Xilinx PSS QSPI driver");
MODULE_LICENSE("GPL");

#endif

/**
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
	u32 mio_base, mio_pin_index;

#ifdef CONFIG_EP107
#ifdef CONFIG_XILINX_PSS_QSPI_USE_DUAL_FLASH
	is_dual = 1;
#else
	is_dual = 0;
#endif
	return is_dual;
#endif

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

/**
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
	xqspips_write(regs_base + XQSPIPSS_ENABLE_OFFSET,
			XQSPIPSS_ENABLE_ENABLE_MASK);

	/* Write QUAD bit with 3-byte instruction */
	xqspips_write(regs_base + XQSPIPSS_TXD_00_11_OFFSET, 0x20001);

	/* Enable manual start command */
	config_reg = xqspips_read(regs_base +
		XQSPIPSS_CONFIG_OFFSET) | XQSPIPSS_CONFIG_MANSRT_MASK;
	xqspips_write(regs_base + XQSPIPSS_CONFIG_OFFSET, config_reg);

	/* Wait for the transfer to finish by polling Tx fifo status */
	do {
		intr_status = xqspips_read(regs_base +
			XQSPIPSS_STATUS_OFFSET);
	} while ((intr_status & 0x04) == 0);

	/* Read data receive register */
	config_reg = xqspips_read(regs_base + XQSPIPSS_RXD_OFFSET);
}
