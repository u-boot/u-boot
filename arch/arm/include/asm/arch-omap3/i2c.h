/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _OMAP3_I2C_H_
#define _OMAP3_I2C_H_

#define I2C_BUS_MAX	3
#define I2C_DEFAULT_BASE	I2C_BASE1

struct i2c {
	unsigned short rev;	/* 0x00 */
	unsigned short res1;
	unsigned short ie;	/* 0x04 */
	unsigned short res2;
	unsigned short stat;	/* 0x08 */
	unsigned short res3;
	unsigned short iv;	/* 0x0C */
	unsigned short res4;
	unsigned short syss;	/* 0x10 */
	unsigned short res4a;
	unsigned short buf;	/* 0x14 */
	unsigned short res5;
	unsigned short cnt;	/* 0x18 */
	unsigned short res6;
	unsigned short data;	/* 0x1C */
	unsigned short res7;
	unsigned short sysc;	/* 0x20 */
	unsigned short res8;
	unsigned short con;	/* 0x24 */
	unsigned short res9;
	unsigned short oa;	/* 0x28 */
	unsigned short res10;
	unsigned short sa;	/* 0x2C */
	unsigned short res11;
	unsigned short psc;	/* 0x30 */
	unsigned short res12;
	unsigned short scll;	/* 0x34 */
	unsigned short res13;
	unsigned short sclh;	/* 0x38 */
	unsigned short res14;
	unsigned short systest;	/* 0x3c */
	unsigned short res15;
};

#endif /* _OMAP3_I2C_H_ */
