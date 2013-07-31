/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Hacked for the DB64360 board by Ingo.Assmus@keymile.com
 * extra improvments by Brain Waite
 */
#include <common.h>
#include <mpc8xx.h>
#include <malloc.h>
#include <i2c.h>
#include "../include/mv_gen_reg.h"
#include "../include/core.h"

#define MAX_I2C_RETRYS	    10
#define I2C_DELAY	    1000	/* Should be at least the # of MHz of Tclk */
#undef	DEBUG_I2C
/*#define DEBUG_I2C*/

#ifdef DEBUG_I2C
#define DP(x) x
#else
#define DP(x)
#endif

/* Assuming that there is only one master on the bus (us) */

void i2c_init (int speed, int slaveaddr)
{
	unsigned int n, m, freq, margin, power;
	unsigned int actualN = 0, actualM = 0;
	unsigned int control, status;
	unsigned int minMargin = 0xffffffff;
	unsigned int tclk = CONFIG_SYS_TCLK;
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

	DP (puts ("udelay...\n"));

	udelay (I2C_DELAY);

	DP (puts ("set baudrate\n"));

	GT_REG_WRITE (I2C_STATUS_BAUDE_RATE, (actualM << 3) | actualN);
	GT_REG_WRITE (I2C_CONTROL, (0x1 << 2) | (0x1 << 6));

	udelay (I2C_DELAY * 10);

	DP (puts ("read control, baudrate\n"));

	GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
	GT_REG_READ (I2C_CONTROL, &control);
}

static uchar i2c_start (void)
{				/* DB64360 checked -> ok */
	unsigned int control, status;
	int count = 0;

	DP (puts ("i2c_start\n"));

	/* Set the start bit */

/* gtI2cGenerateStartBit() */

	GT_REG_READ (I2C_CONTROL, &control);
	control |= (0x1 << 5);	/* generate the I2C_START_BIT */
	GT_REG_WRITE (I2C_CONTROL, control);

	GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);

	count = 0;
	while ((status & 0xff) != 0x08) {
		udelay (I2C_DELAY);
		if (count > 20) {
			GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
			return (status);
		}
		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
	}

	return (0);
}

static uchar i2c_select_device (uchar dev_addr, uchar read, int ten_bit)
{
	unsigned int status, data, bits = 7;
	int count = 0;

	DP (puts ("i2c_select_device\n"));

	/* Output slave address */

	if (ten_bit) {
		bits = 10;
	}

	data = (dev_addr << 1);
	/* set the read bit */
	data |= read;
	GT_REG_WRITE (I2C_DATA, data);
	/* assert the address */
	RESET_REG_BITS (I2C_CONTROL, BIT3);

	udelay (I2C_DELAY);

	GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
	count = 0;
	while (((status & 0xff) != 0x40) && ((status & 0xff) != 0x18)) {
		udelay (I2C_DELAY);
		if (count > 20) {
			GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
			return (status);
		}
		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
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

	unsigned int data, status = 0;
	int count = 0;

	DP (puts ("i2c_get_data\n"));

	while (len) {

		/* Get and return the data */

		RESET_REG_BITS (I2C_CONTROL, (0x1 << 3));

		udelay (I2C_DELAY * 5);

		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
		while ((status & 0xff) != 0x50) {
			udelay (I2C_DELAY);
			if (count > 2) {
				GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
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
	while ((status & 0xff) != 0x58) {
		udelay (I2C_DELAY);
		if (count > 200) {
			GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
			return (status);
		}
		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
	}
	GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/* stop */

	return (0);
}

static uchar i2c_write_data (unsigned int *data, int len)
{
	unsigned int status;
	int count = 0;
	unsigned int temp;
	unsigned int *temp_ptr = data;

	DP (puts ("i2c_write_data\n"));

	while (len) {
		temp = (unsigned int) (*temp_ptr);
		GT_REG_WRITE (I2C_DATA, temp);
		RESET_REG_BITS (I2C_CONTROL, (0x1 << 3));

		udelay (I2C_DELAY);

		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
		while ((status & 0xff) != 0x28) {
			udelay (I2C_DELAY);
			if (count > 20) {
				GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
				return (status);
			}
			GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
			count++;
		}
		len--;
		temp_ptr++;
	}
/* 11-14-2002 Paul Marchese */
/* Can't have the write issuing a stop command */
/* it's wrong to have a stop bit in read stream or write stream */
/* since we don't know if it's really the end of the command */
/* or whether we have just send the device address + offset */
/* we will push issuing the stop command off to the original */
/* calling function */
	/* set the interrupt bit in the control register */
	GT_REG_WRITE (I2C_CONTROL, (0x1 << 3));
	udelay (I2C_DELAY * 10);
	return (0);
}

/* 11-14-2002 Paul Marchese */
/* created this function to get the i2c_write() */
/* function working properly. */
/* function to write bytes out on the i2c bus */
/* this is identical to the function i2c_write_data() */
/* except that it requires a buffer that is an */
/* unsigned character array.  You can't use */
/* i2c_write_data() to send an array of unsigned characters */
/* since the byte of interest ends up on the wrong end of the bus */
/* aah, the joys of big endian versus little endian! */
/* */
/* returns 0 = success */
/*         anything other than zero is failure */
static uchar i2c_write_byte (unsigned char *data, int len)
{
	unsigned int status;
	int count = 0;
	unsigned int temp;
	unsigned char *temp_ptr = data;

	DP (puts ("i2c_write_byte\n"));

	while (len) {
		/* Set and assert the data */
		temp = *temp_ptr;
		GT_REG_WRITE (I2C_DATA, temp);
		RESET_REG_BITS (I2C_CONTROL, (0x1 << 3));

		udelay (I2C_DELAY);

		GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
		count++;
		while ((status & 0xff) != 0x28) {
			udelay (I2C_DELAY);
			if (count > 20) {
				GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));	/*stop */
				return (status);
			}
			GT_REG_READ (I2C_STATUS_BAUDE_RATE, &status);
			count++;
		}
		len--;
		temp_ptr++;
	}
/* Can't have the write issuing a stop command */
/* it's wrong to have a stop bit in read stream or write stream */
/* since we don't know if it's really the end of the command */
/* or whether we have just send the device address + offset */
/* we will push issuing the stop command off to the original */
/* calling function */
/*	GT_REG_WRITE(I2C_CONTROL, (0x1 << 3) | (0x1 << 4));
	GT_REG_WRITE(I2C_CONTROL, (0x1 << 4)); */
	/* set the interrupt bit in the control register */
	GT_REG_WRITE (I2C_CONTROL, (0x1 << 3));
	udelay (I2C_DELAY * 10);

	return (0);
}

static uchar
i2c_set_dev_offset (uchar dev_addr, unsigned int offset, int ten_bit,
		    int alen)
{
	uchar status;
	unsigned int table[2];

/* initialize the table of address offset bytes */
/* utilized for 2 byte address offsets */
/* NOTE: the order is high byte first! */
	table[1] = offset & 0xff;	/* low byte */
	table[0] = offset / 0x100;	/* high byte */

	DP (puts ("i2c_set_dev_offset\n"));

	status = i2c_select_device (dev_addr, 0, ten_bit);
	if (status) {
#ifdef DEBUG_I2C
		printf ("Failed to select device setting offset: 0x%02x\n",
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

int
i2c_read (uchar dev_addr, unsigned int offset, int alen, uchar * data,
	  int len)
{
	uchar status = 0;
	unsigned int i2cFreq = CONFIG_SYS_I2C_SPEED;

	DP (puts ("i2c_read\n"));

	/* set the i2c frequency */
	i2c_init (i2cFreq, CONFIG_SYS_I2C_SLAVE);

	status = i2c_start ();

	if (status) {
#ifdef DEBUG_I2C
		printf ("Transaction start failed: 0x%02x\n", status);
#endif
		return status;
	}

	status = i2c_set_dev_offset (dev_addr, offset, 0, alen);	/* send the slave address + offset */
	if (status) {
#ifdef DEBUG_I2C
		printf ("Failed to set slave address & offset: 0x%02x\n",
			status);
#endif
		return status;
	}

	/* set the i2c frequency again */
	i2c_init (i2cFreq, CONFIG_SYS_I2C_SLAVE);

	status = i2c_start ();
	if (status) {
#ifdef DEBUG_I2C
		printf ("Transaction restart failed: 0x%02x\n", status);
#endif
		return status;
	}

	status = i2c_select_device (dev_addr, 1, 0);	/* send the slave address */
	if (status) {
#ifdef DEBUG_I2C
		printf ("Address not acknowledged: 0x%02x\n", status);
#endif
		return status;
	}

	status = i2c_get_data (data, len);
	if (status) {
#ifdef DEBUG_I2C
		printf ("Data not received: 0x%02x\n", status);
#endif
		return status;
	}

	return 0;
}

/* 11-14-2002 Paul Marchese */
/* Function to set the I2C stop bit */
void i2c_stop (void)
{
	GT_REG_WRITE (I2C_CONTROL, (0x1 << 4));
}

/* 11-14-2002 Paul Marchese */
/* I2C write function */
/* dev_addr = device address */
/* offset = address offset */
/* alen = length in bytes of the address offset */
/* data = pointer to buffer to read data into */
/* len = # of bytes to read */
/* */
/* returns 0 = succesful */
/*         anything but zero is failure */
int
i2c_write (uchar dev_addr, unsigned int offset, int alen, uchar * data,
	   int len)
{
	uchar status = 0;
	unsigned int i2cFreq = CONFIG_SYS_I2C_SPEED;

	DP (puts ("i2c_write\n"));

	/* set the i2c frequency */
	i2c_init (i2cFreq, CONFIG_SYS_I2C_SLAVE);

	status = i2c_start ();	/* send a start bit */

	if (status) {
#ifdef DEBUG_I2C
		printf ("Transaction start failed: 0x%02x\n", status);
#endif
		return status;
	}

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

/* 11-14-2002 Paul Marchese */
/* function to determine if an I2C device is present */
/* chip = device address of chip to check for */
/* */
/* returns 0 = sucessful, the device exists */
/*         anything other than zero is failure, no device */
int i2c_probe (uchar chip)
{

	/* We are just looking for an <ACK> back. */
	/* To see if the device/chip is there */

#ifdef DEBUG_I2C
	unsigned int i2c_status;
#endif
	uchar status = 0;
	unsigned int i2cFreq = CONFIG_SYS_I2C_SPEED;

	DP (puts ("i2c_probe\n"));

	/* set the i2c frequency */
	i2c_init (i2cFreq, CONFIG_SYS_I2C_SLAVE);

	status = i2c_start ();	/* send a start bit */

	if (status) {
#ifdef DEBUG_I2C
		printf ("Transaction start failed: 0x%02x\n", status);
#endif
		return (int) status;
	}

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
