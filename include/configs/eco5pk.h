/*
 * Copyright (C) 2012 8D Technologies inc.
 * Based on mt_ventoux.h, original banner below:
 *
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright (C) 2009 TechNexion Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tam3517-common.h"

#undef CONFIG_USB_EHCI
#undef CONFIG_USB_EHCI_OMAP
#undef CONFIG_USB_OMAP3

/* Our console port is port3 */
#undef CONFIG_CONS_INDEX
#undef CONFIG_SYS_NS16550_COM1
#undef CONFIG_SERIAL1

#define CONFIG_CONS_INDEX	3
#define CONFIG_SYS_NS16550_COM3	OMAP34XX_UART3
#define CONFIG_SERIAL3

#define CONFIG_MACH_TYPE	MACH_TYPE_ECO5_PK

#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_AUTO_COMPLETE

/*
 * Set its own mtdparts, different from common
 */
#undef MTDIDS_DEFAULT
#undef MTDPARTS_DEFAULT
#define MTDIDS_DEFAULT		"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT	"mtdparts=omap2-nand.0:512k(xloader-nand)," \
				"1024k(uboot-nand),256k(params-nand)," \
				"5120k(kernel),-(ubifs)"

/*
 * The arithmetic in tam3517.h is wrong for us and the kernel gets overwritten.
 */
#undef CONFIG_ENV_OFFSET_REDUND
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
						CONFIG_SYS_ENV_SECT_SIZE)

#define	CONFIG_EXTRA_ENV_SETTINGS	CONFIG_TAM3517_SETTINGS \
	"install_kernel=if dhcp $bootfile; then nand erase kernel;" \
				"nand write $fileaddr kernel; fi\0" \
	"mtdparts="MTDPARTS_DEFAULT"\0" \
	"serverip=192.168.142.60\0"

#endif /* __CONFIG_H */
