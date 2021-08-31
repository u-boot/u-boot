/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common part of configuration settings for the AT91 SAMA5 board.
 *
 * Copyright (C) 2015 Atmel Corporation
 *		      Josh Wu <josh.wu@atmel.com>
 */

#ifndef __AT91_SAMA5_COMMON_H
#define __AT91_SAMA5_COMMON_H

#include <linux/kconfig.h>

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

#ifdef CONFIG_SD_BOOT
#define CONFIG_BOOTCOMMAND	"if test ! -n ${dtb_name}; then "	\
				    "setenv dtb_name at91-${board_name}.dtb; " \
				"fi; "					\
				"fatload mmc 0:1 0x21000000 ${dtb_name}; " \
				"fatload mmc 0:1 0x22000000 zImage; "	\
				"bootz 0x22000000 - 0x21000000"

#else

#ifdef CONFIG_NAND_BOOT
/* u-boot env in nand flash */
#define CONFIG_BOOTCOMMAND		"nand read 0x21000000 0x180000 0x80000;"	\
					"nand read 0x22000000 0x200000 0x600000;"	\
					"bootz 0x22000000 - 0x21000000"
#elif CONFIG_SPI_BOOT
/* u-boot env in serial flash, by default is bus 0 and cs 0 */
#define CONFIG_BOOTCOMMAND		"sf probe 0; "				\
					"sf read 0x21000000 0x60000 0xc000; "	\
					"sf read 0x22000000 0x6c000 0x394000; "	\
					"bootz 0x22000000 - 0x21000000"
#elif CONFIG_QSPI_BOOT
#define CONFIG_BOOTCOMMAND		"sf probe 0; "					\
					"sf read 0x21000000 0x180000 0x80000; "		\
					"sf read 0x22000000 0x200000 0x600000; "	\
					"bootz 0x22000000 - 0x21000000"
#endif

#endif

#endif
