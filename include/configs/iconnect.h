/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Tony Dinh <mibodhi@gmail.com>
 * (C) Copyright 2009-2012
 * Wojciech Dubowik <wojciech.dubowik@neratec.com>
 * Luka Perkov <luka@openwrt.org>
 */

#ifndef _CONFIG_ICONNECT_H
#define _CONFIG_ICONNECT_H

#include "mv-common.h"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0"	\
	"mtdids=nand0=orion_nand\0"		\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT	\
	"kernel=/boot/uImage\0"			\
	"bootargs_root=noinitrd ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs\0"

/*
 * Ethernet driver configuration
 *
 * This board has PCIe Wifi card, so allow Ethernet to be disabled
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR	11
#endif /* CONFIG_CMD_NET */

#endif /* _CONFIG_ICONNECT_H */
