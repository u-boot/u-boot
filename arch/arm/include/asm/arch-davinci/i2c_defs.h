/*
 * (C) Copyright 2004-2014
 * Texas Instruments, <www.ti.com>
 *
 * Some changes copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _I2C_DEFS_H_
#define _I2C_DEFS_H_

#ifndef CONFIG_SOC_DA8XX
#define I2C_BASE		0x01c21000
#else
#define I2C_BASE		0x01c22000
#endif

#endif
