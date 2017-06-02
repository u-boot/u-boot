/*
 *  Copyright (C) 2015-2017 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __CONFIG_SOCFGPA_ARRIA10_H__
#define __CONFIG_SOCFGPA_ARRIA10_H__

#include <asm/arch/base_addr_a10.h>

#define CONFIG_HW_WATCHDOG

/* Booting Linux */
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/*
 * U-Boot general configurations
 */
/* Cache options */
#define CONFIG_SYS_DCACHE_OFF

/* Memory configurations  */
#define PHYS_SDRAM_1_SIZE		0x40000000

/* Ethernet on SoC (EMAC) */
#if defined(CONFIG_CMD_NET)
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9031
#endif

/*
 * U-Boot environment configurations
 */
#define CONFIG_ENV_IS_IN_MMC

/*
 * arguments passed to the bootz command. The value of
 * CONFIG_BOOTARGS goes into the environment value "bootargs".
 * Do note the value will overide also the chosen node in FDT blob.
 */
#define CONFIG_BOOTARGS "console=ttyS0," __stringify(CONFIG_BAUDRATE)

/*
 * Serial / UART configurations
 */
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_BAUDRATE_TABLE {4800, 9600, 19200, 38400, 57600, 115200}

/*
 * L4 OSC1 Timer 0
 */
/* reload value when timer count to zero */
#define TIMER_LOAD_VAL			0xFFFFFFFF

/*
 * Flash configurations
 */
#define CONFIG_SYS_MAX_FLASH_BANKS     1

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOCFGPA_ARRIA10_H__ */
