/*
 * Basic I2C functions
 *
 * Copyright (c) 2003 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author: Jian Zhang jzhang@ti.com, Texas Instruments
 *
 * Copyright (c) 2003 Wolfgang Denk, wd@denx.de
 * Rewritten to fit into the current U-Boot framework
 *
 */

#include <common.h>

static void wait_for_bb (void);
static u16 wait_for_pin (void);

void i2c_init (int speed, int slaveadd)
{
	u16 scl;

	if (inw (I2C_CON) & I2C_CON_EN) {
		outw (0, I2C_CON);
		udelay (5000);
	}

	/* 12MHz I2C module clock */
	outw (0, I2C_PSC);
	outw (I2C_CON_EN, I2C_CON);
	outw (0, I2C_SYSTEST);
	/* have to enable intrrupts or OMAP i2c module doesn't work */
	outw (I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
	      I2C_IE_NACK_IE | I2C_IE_AL_IE, I2C_IE);
	scl = (12000000 / 2) / speed - 6;
	outw (scl, I2C_SCLL);
	outw (scl, I2C_SCLH);
	/* own address */
	outw (slaveadd, I2C_OA);
	outw (0, I2C_CNT);
	udelay (1000);
}

static int i2c_read_byte (u8 devaddr, u8 regoffset, u8 * value)
{
	int i2c_error = 0;
	u16 status;

	/* wait until bus not busy */
	wait_for_bb ();

	/* one byte only */
	outw (1, I2C_CNT);
	/* set slave address */
	outw (devaddr, I2C_SA);
	/* no stop bit needed here */
	outw (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX, I2C_CON);

	status = wait_for_pin ();

	if (status & I2C_STAT_XRDY) {
		/* Important: have to use byte access */
		*(volatile u8 *) (I2C_DATA) = regoffset;
		udelay (20000);
		if (inw (I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}

	if (!i2c_error) {
		/* free bus, otherwise we can't use a combined transction */
		outw (0, I2C_CON);
		while (inw (I2C_STAT) || (inw (I2C_CON) & I2C_CON_MST)) {
			udelay (10000);
			/* Have to clear pending interrupt to clear I2C_STAT */
			inw (I2C_IV);
		}

		wait_for_bb ();
		/* set slave address */
		outw (devaddr, I2C_SA);
		/* read one byte from slave */
		outw (1, I2C_CNT);
		/* need stop bit here */
		outw (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP,
		      I2C_CON);

		status = wait_for_pin ();
		if (status & I2C_STAT_RRDY) {
			*value = inw (I2C_DATA);
			udelay (20000);
		} else {
			i2c_error = 1;
		}

		if (!i2c_error) {
			outw (I2C_CON_EN, I2C_CON);
			while (inw (I2C_STAT)
			       || (inw (I2C_CON) & I2C_CON_MST)) {
				udelay (10000);
				inw (I2C_IV);
			}
		}
	}

	return i2c_error;
}

static int i2c_write_byte (u8 devaddr, u8 regoffset, u8 value)
{
	int i2c_error = 0;
	u16 status;

	/* wait until bus not busy */
	wait_for_bb ();

	/* two bytes */
	outw (2, I2C_CNT);
	/* set slave address */
	outw (devaddr, I2C_SA);
	/* stop bit needed here */
	outw (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
	      I2C_CON_STP, I2C_CON);

	/* wait until state change */
	status = wait_for_pin ();

	if (status & I2C_STAT_XRDY) {
		/* send out two bytes */
		outw ((value << 8) + regoffset, I2C_DATA);
		/* must have enough delay to allow BB bit to go low */
		udelay (30000);
		if (inw (I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}

	if (!i2c_error) {
		outw (I2C_CON_EN, I2C_CON);
		while (inw (I2C_STAT) || (inw (I2C_CON) & I2C_CON_MST)) {
			udelay (1000);
			/* have to read to clear intrrupt */
			inw (I2C_IV);
		}
	}

	return i2c_error;
}

int i2c_probe (uchar chip)
{
	int res = 1;

	if (chip == inw (I2C_OA)) {
		return res;
	}

	/* wait until bus not busy */
	wait_for_bb ();

	/* try to read one byte */
	outw (1, I2C_CNT);
	/* set slave address */
	outw (chip, I2C_SA);
	/* stop bit needed here */
	outw (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP, I2C_CON);
	/* enough delay for the NACK bit set */
	udelay (2000);
	if (!(inw (I2C_STAT) & I2C_STAT_NACK)) {
		res = 0;
	} else {
		outw (inw (I2C_CON) | I2C_CON_STP, I2C_CON);
		udelay (20);
		wait_for_bb ();
	}

	return res;
}

int i2c_read (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	int i;

	if (alen > 1) {
		printf ("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf ("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_read_byte (chip, addr + i, &buffer[i])) {
			printf ("I2C read: I/O error\n");
			i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	int i;

	if (alen > 1) {
		printf ("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf ("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_write_byte (chip, addr + i, buffer[i])) {
			printf ("I2C read: I/O error\n");
			i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

static void wait_for_bb (void)
{
	int timeout = 10;

	while ((inw (I2C_STAT) & I2C_STAT_BB) && timeout--) {
		inw (I2C_IV);
		udelay (1000);
	}

	if (timeout <= 0) {
		printf ("timed out in wait_for_bb: I2C_STAT=%x\n",
			inw (I2C_STAT));
	}
}

static u16 wait_for_pin (void)
{
	u16 status, iv;
	int timeout = 10;

	do {
		udelay (1000);
		status = inw (I2C_STAT);
		iv = inw (I2C_IV);
	} while (!iv &&
		 !(status &
		   (I2C_STAT_ROVR | I2C_STAT_XUDF | I2C_STAT_XRDY |
		    I2C_STAT_RRDY | I2C_STAT_ARDY | I2C_STAT_NACK |
		    I2C_STAT_AL)) && timeout--);

	if (timeout <= 0) {
		printf ("timed out in wait_for_pin: I2C_STAT=%x\n",
			inw (I2C_STAT));
	}

	return status;
}
