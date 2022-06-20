/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Allied Telesis <www.alliedtelesis.co.nz>
 */

#ifndef _CONFIG_SBX81LIFKW_H
#define _CONFIG_SBX81LIFKW_H

/* additions for new ARM relocation support */
#define CONFIG_SYS_SDRAM_BASE	0x00000000

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#define CONFIG_SYS_NS16550_COM1		KW_UART0_BASE

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define MTDPARTS_DEFAULT "mtdparts=spi0.0:768K(boot)ro,256K(boot-env),14M(user),1M(errlog)"
#define MTDPARTS_MTDOOPS "errlog"

/*
 *  Environment variables configurations
 */

/*
 * U-Boot bootcode configuration
 */

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for monitor */

#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Mem map for Linux*/

/* size in bytes reserved for initial data */

#include <asm/arch/config.h>
/* There is no PHY directly connected so don't ask it for link status */

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable a single port */
#define CONFIG_PHY_BASE_ADR	0x01
#endif /* CONFIG_CMD_NET */

#endif /* _CONFIG_SBX81LIFKW_H */
