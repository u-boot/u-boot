/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_CONFIGS_H
#define __ARCH_CONFIGS_H

#include <asm/iproc-common/configs.h>

/* uArchitecture specifics */

/* Serial Info */
/* Post pad 3 bytes after each reg addr */
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_MEM32

#define CONFIG_SYS_NS16550_CLK		100000000
#define CONFIG_SYS_NS16550_CLK_DIV	54
#define CONFIG_SERIAL_MULTI
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		0x18023000

/* Ethernet */
#define CONFIG_BCM_SF2_ETH
#define CONFIG_BCM_SF2_ETH_GMAC

#define CONFIG_PHYLIB
#define CONFIG_PHY_BROADCOM
#define CONFIG_PHY_RESET_DELAY 10000 /* PHY reset delay in us*/

#define CONFIG_CMD_PING
#define CONFIG_CMD_MII

#endif /* __ARCH_CONFIGS_H */
