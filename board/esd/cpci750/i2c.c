/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Hacked for the DB64360 board by Ingo.Assmus@keymile.com
 * extra improvments by Brain Waite
 * for cpci750 by reinhard.arlt@esd-electronics.com
 */
#include <common.h>
#include <mpc8xx.h>
#include <malloc.h>
#include "../../Marvell/include/mv_gen_reg.h"
#include "../../Marvell/include/core.h"

#define I2C_DELAY	    100
#undef	DEBUG_I2C

#ifdef DEBUG_I2C
#define DP(x) x
#else
#define DP(x)
#endif

/* Assuming that there is only one master on the bus (us) */

static void i2c_init (int speed, int slaveaddr)
{
	unsigned int n, m, freq, margin, power;
	unsigned int actualN = 0, actualM = 0;
	unsigned int minMargin = 0xffffffff;
	unsigned int tclk = CFG_TCLK;
	unsigned int i2cFreq = speed;	/* 100000 max. Fast mode not supported */

	DP (puts ("i2c_init\n"));
/* gtI2cMasterInit */
	for (n = 0; n < 8; n++) {
		for (m = 0; m < 16; m++) {
			power = 2 << n;	/* power = 2^(n+1) */
			freq = tclk / (10 * (m + 1) * power);
			if (i2cFreq > freq)
				margin = i2cFreq - freq;
			else
				margin = freq - i2cFreq;
			if (margin < minMargin) {
				minMargin = margin;
				actualN = n;
				actualM = m;
			}
		}
	}

	DP (puts ("setup i2c bus\n"));

	/* Setup bus */
	/* gtI2cReset */
	GT_REG_WRITE (I2C_SOFT_RESET, 0);
	asm(" sync");
	GT_REG_WRITE (I2C_CONTROL, 0);
	asm(" sync");

	DP (puts ("set baudrate\n"));

	GT_REG_WRITE (I2C_STATUS_BAUDE_RATE, (actualM << 3) | actualN);
	asm(" sync");

	DP (puts ("udelay...\n"));

	udelay (I2C_DELAY);

	GT_REG_WRITE (I2C_CONTROL, (0x1 << 2) | (0x1 << 6));
	asm(" sync");
}


static uchar i2c_select_device (uchar dev_addr, uchar read, int ten_bit)
{
	unsigned int status, data, bits = 7;
	unsigned int control;
	int count = 0;

	DP (puts ("i2c_select_device\n"));

	/* Output slave address */

	if (ten_bit) {
		bits = 10;
	}

	GT_REG_READ (I2C_CONTROL, &control);
	control |=  (0x1 << 2);
	GT_REG_WRITE (I2C_CONTROL, control);
	asm(" sync");

	GT_REG_READ (I2C_CONTROL, &control);
	control |= (0x1 << 5);	/* generate the I2C_START_BIT */
	GT_REG_WRITE (I2C_CONTROL, control);
	asm(" sync");
	RESET_REG_BITS (I2C_CONTROL, (0x01 << 3));
	asm(" sync");

	GT_REG_READ (I2C_CONTROL, &status);
	while ((status & 0x08) != 0x08) {
		GT_REG_READ (I2C_CONTROL, &status);
		}


	count = 0;

	GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
	while (((status & 0xff) != 0x08) && ((status & 0xff) != 0x10)){
		if (count > 200) {
#ifdef DEBUG_I2C
			printf ("Failed to set startbit: 0x%02x\n", status);
#endif
			GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
			asm(" sync");
			return (status);
		}
		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
	}

	DP (puts ("i2c_select_device:write addr byte\n"));

	/* assert the address */

	data = (dev_addr << 1);
	/* set the read bit */
	data |= read;
	GT_REG_WRITE (I2C_DATA, data);
	asm(" sync");
	RESET_REG_BITS (I2C_CONTROL, BIT3);
	asm(" sync");

	GT_REG_READ (I2C_CONTROL, &status);
	while ((status & 0x08) != 0x08) {
		GT_REG_READ (I2C_CONTROL, &status);
		}

	GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
	count = 0;
	while (((status & 0xff) != 0x40) && ((status & 0xff) != 0x18)) {
		if (count > 200) {
#ifdef DEBUG_I2C
			printf ("Failed to write address: 0x%02x\n", status);
#endif
			GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
			return (status);
		}
		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		asm(" sync");
		count++;
	}

	if (bits == 10) {
		printf ("10 bit I2C addressing not yet implemented\n");
		return (0xff);
	}

	return (0);
}

static uchar i2c_get_data (uchar * return_data, int len)
{

	unsigned int data, status;
	int count = 0;

	DP (puts ("i2c_get_data\n"));

	while (len) {

		RESET_REG_BITS (I2C_CONTROL, BIT3);
		asm(" sync");

		/* Get and return the data */

		GT_REG_READ (I2C_CONTROL, &status);
		while ((status & 0x08) != 0x08) {
			GT_REG_READ (I2C_CONTROL, &status);
			}

		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
		while ((status & 0xff) != 0x50) {
			if (count > 20) {
#ifdef DEBUG_I2C
				printf ("Failed to get data len status: 0x%02x\n", status);
#endif
				GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
				asm(" sync");
				return 0;
			}
			GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
			count++;
		}
		GT_REG_READ (I2C_DATA, &data);
		len--;
		*return_data = (uchar) data;
		return_data++;

	}
	RESET_REG_BITS (I2C_CONTROL, BIT2 | BIT3);
	asm(" sync");
	count = 0;

	GT_REG_READ (I2C_CONTROL, &status);
	while ((status & 0x08) != 0x08) {
		GT_REG_READ (I2C_CONTROL, &status);
		}

	while ((status & 0xff) != 0x58) {
		if (count > 2000) {
			GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
			return (status);
		}
		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
	}
	GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/* stop */
	asm(" sync");
	RESET_REG_BITS (I2C_CONTROL, (0x1 << 3));
	asm(" sync");

	return (0);
}


static uchar i2c_write_data (unsigned int *data, int len)
{
	unsigned int status;
	int count;
	unsigned int temp;
	unsigned int *temp_ptr = data;

	DP (puts ("i2c_write_data\n"));

	while (len) {
		count = 0;
		temp = (unsigned int) (*temp_ptr);
		GT_REG_WRITE (I2C_DATA, temp);
		asm(" sync");
		RESET_REG_BITS (I2C_CONTROL, (0x1 << 3));
		asm(" sync");

		GT_REG_READ (I2C_CONTROL, &status);
		while ((status & 0x08) != 0x08) {
			GT_REG_READ (I2C_CONTROL, &status);
			}

		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
		while ((status & 0xff) != 0x28) {
			if (count > 200) {
				GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
				asm(" sync");
				return (status);
			}
			GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
			count++;
		}
		len--;
		temp_ptr++;
	}
	return (0);
}


static uchar i2c_write_byte (unsigned char *data, int len)
{
	unsigned int status;
	int count;
	unsigned int temp;
	unsigned char *temp_ptr = data;

	DP (puts ("i2c_write_byte\n"));

	while (len) {
		count = 0;
		/* Set and assert the data */
		temp = *temp_ptr;
		GT_REG_WRITE (I2C_DATA, temp);
		asm(" sync");
		RESET_REG_BITS (I2C_CONTROL, (0x1 << 3));
		asm(" sync");


		GT_REG_READ (I2C_CONTROL, &status);
		while ((status & 0x08) != 0x08) {
			GT_REG_READ (I2C_CONTROL, &status);
			}

		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
		while ((status & 0xff) != 0x28) {
			if (count > 200) {
				GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
				asm(" sync");
				return (status);
			}
			GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
			count++;
		}
		len--;
		temp_ptr++;
	}
	return (0);
}

static uchar
i2c_set_dev_offset (uchar dev_addr, unsigned int offset, int ten_bit,
		    int alen)
{
	uchar status;
	unsigned int table[2];

	table[1] = (offset     ) & 0x0ff;	/* low byte */
	table[0] = (offset >> 8) & 0x0ff;	/* high byte */

	DP (puts ("i2c_set_dev_offset\n"));

	status = i2c_select_device (dev_addr, 0, ten_bit);
	if (status) {
#ifdef DEBUG_I2C
22		printf ("Failed to select device setting offset: 0x%02x\n",
			status);
#endif
		return status;
	}
/* check the address offset length */
	if (alen == 0)
		/* no address offset */
		return (0);
	else if (alen == 1) {
		/* 1 byte address offset */
		status = i2c_write_data (&offset, 1);
		if (status) {
#ifdef DEBUG_I2C
			printf ("Failed to write data: 0x%02x\n", status);
#endif
			return status;
		}
	} else if (alen == 2) {
		/* 2 bytes address offset */
		status = i2c_write_data (table, 2);
		if (status) {
#ifdef DEBUG_I2C
			printf ("Failed to write data: 0x%02x\n", status);
#endif
			return status;
		}
	} else {
		/* address offset unknown or not supported */
		printf ("Address length offset %d is not supported\n", alen);
		return 1;
	}
	return 0;		/* sucessful completion */
}

uchar
i2c_read (uchar dev_addr, unsigned int offset, int alen, uchar * data,
	  int len)
{
	uchar status = 0;
	unsigned int i2cFreq = CFG_I2C_SPEED;

	DP (puts ("i2c_read\n"));

	i2c_init (i2cFreq, 0);	/* set the i2c frequency */

	status = i2c_set_dev_offset (dev_addr, offset, 0, alen);	/* send the slave address + offset */
	if (status) {
#ifdef DEBUG_I2C
		printf ("Failed to set slave address & offset: 0x%02x\n",
			status);
#endif
		return status;
	}

	status = i2c_select_device (dev_addr, 1, 0);
	if (status) {
#ifdef DEBUG_I2C
		printf ("Failed to select device for data read: 0x%02x\n",
			status);
#endif
		return status;
	}

	status = i2c_get_data (data, len);
	if (status) {
#ifdef DEBUG_I2C
		printf ("Data not read: 0x%02x\n", status);
#endif
		return status;
	}

	return 0;
}


void i2c_stop (void)
{
	GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));
	asm(" sync");
}


uchar
i2c_write (uchar dev_addr, unsigned int offset, int alen, uchar * data,
	   int len)
{
	uchar status = 0;
	unsigned int i2cFreq = CFG_I2C_SPEED;

	DP (puts ("i2c_write\n"));

	i2c_init (i2cFreq, 0);	/* set the i2c frequency */

	status = i2c_set_dev_offset (dev_addr, offset, 0, alen);	/* send the slave address + offset */
	if (status) {
#ifdef DEBUG_I2C
		printf ("Failed to set slave address & offset: 0x%02x\n",
			status);
#endif
		return status;
		}


	status = i2c_write_byte (data, len);	/* write the data */
	if (status) {
#ifdef DEBUG_I2C
		printf ("Data not written: 0x%02x\n", status);
#endif
		return status;
		}
	/* issue a stop bit */
	i2c_stop ();
	return 0;
}


int i2c_probe (uchar chip)
{

#ifdef DEBUG_I2C
	unsigned int i2c_status;
#endif
	uchar status = 0;
	unsigned int i2cFreq = CFG_I2C_SPEED;

	DP (puts ("i2c_probe\n"));

	i2c_init (i2cFreq, 0);	/* set the i2c frequency */

	status = i2c_set_dev_offset (chip, 0, 0, 0);	/* send the slave address + no offset */
	if (status) {
#ifdef DEBUG_I2C
		printf ("Failed to set slave address: 0x%02x\n", status);
#endif
		return (int) status;
	}
#ifdef DEBUG_I2C
	GT_REG_READ (I2C_STATUS_BAUDE_RATE, &i2c_status);
	printf ("address %#x returned %#x\n", chip, i2c_status);
#endif
	/* issue a stop bit */
	i2c_stop ();
	return 0;		/* successful completion */
}
