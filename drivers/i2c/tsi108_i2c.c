/*
 * (C) Copyright 2004 Tundra Semiconductor Corp.
 * Author: Alex Bounine
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>

#include <tsi108.h>

#if defined(CONFIG_CMD_I2C)

#define I2C_DELAY	100000
#undef  DEBUG_I2C

#ifdef DEBUG_I2C
#define DPRINT(x) printf (x)
#else
#define DPRINT(x)
#endif

/* All functions assume that Tsi108 I2C block is the only master on the bus */
/* I2C read helper function */

void i2c_init(int speed, int slaveaddr)
{
	/*
	 * The TSI108 has a fixed I2C clock rate and doesn't support slave
	 * operation.  This function only exists as a stub to fit into the
	 * U-Boot I2C API.
	 */
}

static int i2c_read_byte (
		uint i2c_chan,	/* I2C channel number: 0 - main, 1 - SDC SPD */
		uchar chip_addr,/* I2C device address on the bus */
		uint byte_addr,	/* Byte address within I2C device */
		uchar * buffer	/* pointer to data buffer */
		)
{
	u32 temp;
	u32 to_count = I2C_DELAY;
	u32 op_status = TSI108_I2C_TIMEOUT_ERR;
	u32 chan_offset = TSI108_I2C_OFFSET;

	DPRINT (("I2C read_byte() %d 0x%02x 0x%02x\n",
		i2c_chan, chip_addr, byte_addr));

	if (0 != i2c_chan)
		chan_offset = TSI108_I2C_SDRAM_OFFSET;

	/* Check if I2C operation is in progress */
	temp = *(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + chan_offset + I2C_CNTRL2);

	if (0 == (temp & (I2C_CNTRL2_RD_STATUS | I2C_CNTRL2_WR_STATUS |
			  I2C_CNTRL2_START))) {
		/* Set device address and operation (read = 0) */
		temp = (byte_addr << 16) | ((chip_addr & 0x07) << 8) |
		    ((chip_addr >> 3) & 0x0F);
		*(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + chan_offset + I2C_CNTRL1) =
		    temp;

		/* Issue the read command
		 * (at this moment all other parameters are 0
		 * (size = 1 byte, lane = 0)
		 */

		*(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + chan_offset + I2C_CNTRL2) =
		    (I2C_CNTRL2_START);

		/* Wait until operation completed */
		do {
			/* Read I2C operation status */
			temp = *(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + chan_offset + I2C_CNTRL2);

			if (0 == (temp & (I2C_CNTRL2_RD_STATUS | I2C_CNTRL2_START))) {
				if (0 == (temp &
				     (I2C_CNTRL2_I2C_CFGERR |
				      I2C_CNTRL2_I2C_TO_ERR))
				    ) {
					op_status = TSI108_I2C_SUCCESS;

					temp = *(u32 *) (CONFIG_SYS_TSI108_CSR_BASE +
							 chan_offset +
							 I2C_RD_DATA);

					*buffer = (u8) (temp & 0xFF);
				} else {
					/* report HW error */
					op_status = TSI108_I2C_IF_ERROR;

					DPRINT (("I2C HW error reported: 0x%02x\n", temp));
				}

				break;
			}
		} while (to_count--);
	} else {
		op_status = TSI108_I2C_IF_BUSY;

		DPRINT (("I2C Transaction start failed: 0x%02x\n", temp));
	}

	DPRINT (("I2C read_byte() status: 0x%02x\n", op_status));
	return op_status;
}

/*
 * I2C Read interface as defined in "include/i2c.h" :
 *   chip_addr: I2C chip address, range 0..127
 *                  (to read from SPD channel EEPROM use (0xD0 ... 0xD7)
 *              NOTE: The bit 7 in the chip_addr serves as a channel select.
 *              This hack is for enabling "i2c sdram" command on Tsi108 boards
 *              without changes to common code. Used for I2C reads only.
 *   byte_addr: Memory or register address within the chip
 *   alen:      Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:    Pointer to destination buffer for data to be read
 *   len:       How many bytes to read
 *
 *   Returns: 0 on success, not 0 on failure
 */

int i2c_read (uchar chip_addr, uint byte_addr, int alen,
		uchar * buffer, int len)
{
	u32 op_status = TSI108_I2C_PARAM_ERR;
	u32 i2c_if = 0;

	/* Hack to support second (SPD) I2C controller (SPD EEPROM read only).*/
	if (0xD0 == (chip_addr & ~0x07)) {
		i2c_if = 1;
		chip_addr &= 0x7F;
	}
	/* Check for valid I2C address */
	if (chip_addr <= 0x7F && (byte_addr + len) <= (0x01 << (alen * 8))) {
		while (len--) {
			op_status = i2c_read_byte(i2c_if, chip_addr, byte_addr++, buffer++);

			if (TSI108_I2C_SUCCESS != op_status) {
				DPRINT (("I2C read_byte() failed: 0x%02x (%d left)\n", op_status, len));

				break;
			}
		}
	}

	DPRINT (("I2C read() status: 0x%02x\n", op_status));
	return op_status;
}

/* I2C write helper function */

static int i2c_write_byte (uchar chip_addr,/* I2C device address on the bus */
			  uint byte_addr, /* Byte address within I2C device */
			  uchar * buffer  /*  pointer to data buffer */
			  )
{
	u32 temp;
	u32 to_count = I2C_DELAY;
	u32 op_status = TSI108_I2C_TIMEOUT_ERR;

	/* Check if I2C operation is in progress */
	temp = *(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_I2C_OFFSET + I2C_CNTRL2);

	if (0 == (temp & (I2C_CNTRL2_RD_STATUS | I2C_CNTRL2_WR_STATUS | I2C_CNTRL2_START))) {
		/* Place data into the I2C Tx Register */
		*(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_I2C_OFFSET +
			  I2C_TX_DATA) = (u32) * buffer;

		/* Set device address and operation  */
		temp =
		    I2C_CNTRL1_I2CWRITE | (byte_addr << 16) |
		    ((chip_addr & 0x07) << 8) | ((chip_addr >> 3) & 0x0F);
		*(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_I2C_OFFSET +
			  I2C_CNTRL1) = temp;

		/* Issue the write command (at this moment all other parameters
		 * are 0 (size = 1 byte, lane = 0)
		 */

		*(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_I2C_OFFSET +
			  I2C_CNTRL2) = (I2C_CNTRL2_START);

		op_status = TSI108_I2C_TIMEOUT_ERR;

		/* Wait until operation completed */
		do {
			/* Read I2C operation status */
			temp = *(u32 *) (CONFIG_SYS_TSI108_CSR_BASE + TSI108_I2C_OFFSET + I2C_CNTRL2);

			if (0 == (temp & (I2C_CNTRL2_WR_STATUS | I2C_CNTRL2_START))) {
				if (0 == (temp &
				     (I2C_CNTRL2_I2C_CFGERR |
				      I2C_CNTRL2_I2C_TO_ERR))) {
					op_status = TSI108_I2C_SUCCESS;
				} else {
					/* report detected HW error */
					op_status = TSI108_I2C_IF_ERROR;

					DPRINT (("I2C HW error reported: 0x%02x\n", temp));
				}

				break;
			}

		} while (to_count--);
	} else {
		op_status = TSI108_I2C_IF_BUSY;

		DPRINT (("I2C Transaction start failed: 0x%02x\n", temp));
	}

	return op_status;
}

/*
 * I2C Write interface as defined in "include/i2c.h" :
 *   chip_addr: I2C chip address, range 0..127
 *   byte_addr: Memory or register address within the chip
 *   alen:      Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:    Pointer to data to be written
 *   len:       How many bytes to write
 *
 *   Returns: 0 on success, not 0 on failure
 */

int i2c_write (uchar chip_addr, uint byte_addr, int alen, uchar * buffer,
	      int len)
{
	u32 op_status = TSI108_I2C_PARAM_ERR;

	/* Check for valid I2C address */
	if (chip_addr <= 0x7F && (byte_addr + len) <= (0x01 << (alen * 8))) {
		while (len--) {
			op_status =
			    i2c_write_byte (chip_addr, byte_addr++, buffer++);

			if (TSI108_I2C_SUCCESS != op_status) {
				DPRINT (("I2C write_byte() failed: 0x%02x (%d left)\n", op_status, len));

				break;
			}
		}
	}

	return op_status;
}

/*
 * I2C interface function as defined in "include/i2c.h".
 * Probe the given I2C chip address by reading single byte from offset 0.
 * Returns 0 if a chip responded, not 0 on failure.
 */

int i2c_probe (uchar chip)
{
	u32 tmp;

	/*
	 * Try to read the first location of the chip.
	 * The Tsi108 HW doesn't support sending just the chip address
	 * and checkong for an <ACK> back.
	 */
	return i2c_read (chip, 0, 1, (uchar *)&tmp, 1);
}

#endif
