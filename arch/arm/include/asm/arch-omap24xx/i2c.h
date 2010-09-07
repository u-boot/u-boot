/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
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
 */
#ifndef _OMAP24XX_I2C_H_
#define _OMAP24XX_I2C_H_

#define I2C_BASE1		0x48070000
#define I2C_BASE2               0x48072000 /* nothing hooked up on h4 */

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
	unsigned short res4p1;
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

#define I2C_BUS_MAX	2

#endif
