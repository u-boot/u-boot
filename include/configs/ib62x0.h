/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011-2012
 * Gerald Kerma <dreagle@doukki.net>
 * Luka Perkov <luka@openwrt.org>
 */

#ifndef _CONFIG_IB62x0_H
#define _CONFIG_IB62x0_H

#include "mv-common.h"

/*
 * Environment variables configuration
 */

/*
 * Default environment variables
 */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0"				\
	"mtdids=nand0=orion_nand\0"					\
	"mtdparts="CONFIG_MTDPARTS_DEFAULT			\
	"kernel=/boot/zImage\0"						\
	"fdt=/boot/ib62x0.dtb\0"					\
	"bootargs_root=ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw\0"

/*
 * Ethernet driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR	0
#endif /* CONFIG_CMD_NET */

/*
 * SATA driver configuration
 */
#ifdef CONFIG_IDE
#define __io
#endif /* CONFIG_IDE */

#endif /* _CONFIG_IB62x0_H */
