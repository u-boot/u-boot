/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 *
 * Based on work by:
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 */

#ifndef _CONFIG_DREAMPLUG_H
#define _CONFIG_DREAMPLUG_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SHEEVA_88SV131	1	/* CPU Core subversion */
#define CONFIG_MACH_TYPE	MACH_TYPE_DREAMPLUG

#include "mv-plug-common.h"

/*
 *  Environment variables configurations
 */

/*
 * max 4k env size is enough, but in case of nand
 * it has to be rounded to sector size
 */

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND		"setenv ethact ethernet-controller@72000; " \
	"${x_bootcmd_ethernet}; setenv ethact ethernet-controller@76000; " \
	"${x_bootcmd_ethernet}; ${x_bootcmd_usb}; ${x_bootcmd_kernel}; "\
	"setenv bootargs ${x_bootargs} ${x_bootargs_root}; "	\
	"bootm 0x6400000;"

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"x_bootcmd_ethernet=ping 192.168.2.1\0"	\
	"x_bootcmd_usb=usb start\0"	\
	"x_bootcmd_kernel=fatload usb 0 0x6400000 uImage\0" \
	"x_bootargs=console=ttyS0,115200\0"	\
	"x_bootargs_root=root=/dev/sda2 rootdelay=10\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 1}	/* enable both ports */
#define CONFIG_PHY_BASE_ADR	0
#endif /* CONFIG_CMD_NET */

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_LBA48
#endif /* CONFIG_SATA */

#endif /* _CONFIG_DREAMPLUG_H */
