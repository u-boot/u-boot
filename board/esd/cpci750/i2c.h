/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Hacked for the DB64360 board by Ingo.Assmus@keymile.com
 */

#ifndef __I2C_H__
#define __I2C_H__

/* function declarations */
uchar i2c_read(uchar, unsigned int, int, uchar*, int);

#endif
