/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _S3C24X0_I2C_H
#define _S3C24X0_I2C_H

struct s3c24x0_i2c {
	u32	iiccon;
	u32	iicstat;
	u32	iicadd;
	u32	iicds;
	u32	iiclc;
};

struct s3c24x0_i2c_bus {
	int node;	/* device tree node */
	int bus_num;	/* i2c bus number */
	struct s3c24x0_i2c *regs;
	int id;
};
#endif /* _S3C24X0_I2C_H */
