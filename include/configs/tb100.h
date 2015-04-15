/*
 * Copyright (C) 2011-2014 Pierrick Hascoet, Abilis Systems
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_TB100_H_
#define _CONFIG_TB100_H_

#include <linux/sizes.h>

/*
 *  CPU configuration
 */
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_CLK_FREQ

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_128M

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		SZ_128K
#define CONFIG_SYS_BOOTM_LEN		SZ_32M
#define CONFIG_SYS_LOAD_ADDR		0x82000000

#define CONFIG_SYS_NO_FLASH

/*
 * UART configuration
 */
#define CONFIG_DW_SERIAL
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		166666666
#define CONFIG_BAUDRATE			115200

/*
 * Ethernet PHY configuration
 */
#define CONFIG_PHYLIB
#define CONFIG_PHY_GIGE

/*
 * Even though the board houses Realtek RTL8211E PHY
 * corresponding PHY driver (drivers/net/phy/realtek.c) behaves unexpectedly.
 * In particular "parse_status" reports link is down.
 *
 * Until Realtek PHY driver is fixed fall back to generic PHY driver
 * which implements all required functionality and behaves much more stable.
 *
 * #define CONFIG_PHY_REALTEK
 *
 */

/*
 * Ethernet configuration
 */
#define CONFIG_DESIGNWARE_ETH
#define ETH0_BASE_ADDRESS		0xFE100000
#define ETH1_BASE_ADDRESS		0xFE110000

/*
 * Command line configuration
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_PING

#define CONFIG_OF_LIBFDT

#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS		16

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			SZ_2K
#define CONFIG_ENV_OFFSET		0

/*
 * Environment configuration
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTARGS			"console=ttyS0,115200n8"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"[tb100]:~# "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

#endif /* _CONFIG_TB100_H_ */
