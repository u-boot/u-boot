/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014-2022 Tony Dinh <mibodhi@gmail.com>
 *
 * Based on
 * Copyright (C) 2012
 * David Purdy <david.c.purdy@gmail.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_POGO_V4_H
#define _CONFIG_POGO_V4_H

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * Default environment variables
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"dtb_file=/boot/dts/" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"bootargs_console=console=ttyS0,115200\0" \
	"bootcmd_usb=usb start; load usb 0:1 0x00800000 /boot/uImage; " \
	"load usb 0:1 0x01100000 /boot/uInitrd; " \
	"load usb 0:1 0x2c00000 $dtb_file\0"

/*
 * Ethernet Driver configuration
 */
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR	0

#endif /* _CONFIG_POGO_V4_H */
