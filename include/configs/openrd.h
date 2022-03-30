/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Net Insight <www.netinsight.net>
 * Written-by: Simon Kagstrom <simon.kagstrom@netinsight.net>
 *
 * Based on sheevaplug.h:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_OPENRD_H
#define _CONFIG_OPENRD_H

#include "mv-common.h"

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

#define CONFIG_EXTRA_ENV_SETTINGS	"x_bootargs=console=ttyS0,115200 " \
	CONFIG_MTDPARTS_DEFAULT " rw ubi.mtd=2,2048\0" \
	"x_bootcmd_kernel=nand read 0x6400000 0x100000 0x300000\0"	\
	"x_bootcmd_usb=usb start\0"					\
	"x_bootargs_root=root=ubi0:rootfs rootfstype=ubifs\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
# ifdef CONFIG_BOARD_IS_OPENRD_BASE
#  define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
# else
#  define CONFIG_MVGBE_PORTS	{1, 1}	/* enable both ports */
# endif
# ifdef CONFIG_BOARD_IS_OPENRD_ULTIMATE
#  define CONFIG_PHY_BASE_ADR	0x0
#  define PHY_NO		"88E1121"
# else
#  define CONFIG_PHY_BASE_ADR	0x8
#  define PHY_NO		"88E1116"
# endif
#endif /* CONFIG_CMD_NET */

/*
 * SATA Driver configuration
 */

#endif /* _CONFIG_OPENRD_BASE_H */
