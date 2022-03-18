/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Embest/Timll DevKit3250 board configuration file
 *
 * Copyright (C) 2011-2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef __CONFIG_DEVKIT3250_H__
#define __CONFIG_DEVKIT3250_H__

/* SoC and board defines */
#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * Memory configurations
 */
#define CONFIG_SYS_SDRAM_BASE		EMC_DYCS0_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_64M

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_4K \
					 - GENERATED_GBL_DATA_SIZE)

/*
 * DMA
 */

/*
 * GPIO
 */

/*
 * NOR Flash
 */
#define CONFIG_SYS_MAX_FLASH_SECT	71
#define CONFIG_SYS_FLASH_BASE		EMC_CS0_BASE
#define CONFIG_SYS_FLASH_SIZE		SZ_4M

/*
 * NAND controller
 */
#define CONFIG_SYS_NAND_BASE		SLC_NAND_BASE
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }

/*
 * NAND chip timings
 */
#define CONFIG_LPC32XX_NAND_SLC_WDR_CLKS	14
#define CONFIG_LPC32XX_NAND_SLC_WWIDTH		66666666
#define CONFIG_LPC32XX_NAND_SLC_WHOLD		200000000
#define CONFIG_LPC32XX_NAND_SLC_WSETUP		50000000
#define CONFIG_LPC32XX_NAND_SLC_RDR_CLKS	14
#define CONFIG_LPC32XX_NAND_SLC_RWIDTH		66666666
#define CONFIG_LPC32XX_NAND_SLC_RHOLD		200000000
#define CONFIG_LPC32XX_NAND_SLC_RSETUP		50000000

/*
 * USB
 */
#define CONFIG_USB_OHCI_LPC32XX
#define CONFIG_USB_ISP1301_I2C_ADDR		0x2d

/*
 * U-Boot General Configurations
 */
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * Pass open firmware flat tree
 */

/*
 * Environment
 */

#define CONFIG_EXTRA_ENV_SETTINGS		\
	"autoload=no\0"				\
	"ethaddr=00:01:90:00:C0:81\0"		\
	"dtbaddr=0x81000000\0"			\
	"nfsroot=/opt/projects/images/vladimir/oe/devkit3250/rootfs\0"	\
	"tftpdir=vladimir/oe/devkit3250\0"	\
	"userargs=oops=panic\0"

/*
 * U-Boot Commands
 */

/*
 * SPL specific defines
 */
/* SPL will be executed at offset 0 */

/* SPL will use SRAM as stack */
#define CONFIG_SPL_STACK		0x0000FFF8

/* Use the framework and generic lib */

/* SPL will use serial */

/* SPL loads an image from NAND */
#define CONFIG_SPL_NAND_RAW_ONLY

#define CONFIG_SPL_NAND_SOFTECC

#define CONFIG_SPL_MAX_SIZE		0x20000
#define CONFIG_SPL_PAD_TO		CONFIG_SPL_MAX_SIZE

/* U-Boot will be 0x60000 bytes, loaded and run at CONFIG_SYS_TEXT_BASE */
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x60000

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE

/* See common/spl/spl.c  spl_set_header_raw_uboot() */
#define CONFIG_SYS_MONITOR_LEN		CONFIG_SYS_NAND_U_BOOT_SIZE

/*
 * Include SoC specific configuration
 */
#include <asm/arch/config.h>

#endif  /* __CONFIG_DEVKIT3250_H__*/
