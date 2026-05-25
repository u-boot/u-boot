/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2018 NXP
 * Copyright 2019 Siemens AG
 */

#ifndef __IMX8X_CAPRICORN_H
#define __IMX8X_CAPRICORN_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

/* SPL config */
#ifdef CONFIG_XPL_BUILD
#define CFG_MALLOC_F_ADDR		0x00120000

#endif /* CONFIG_XPL_BUILD */

/* ENET1 connects to base board and MUX with ESAI */
#define CFG_FEC_ENET_DEV		1
#define CFG_FEC_MXC_PHYADDR		0x0

/* EEPROM */
#define  EEPROM_I2C_BUS		0 /* I2C0 */
#define  EEPROM_I2C_ADDR	0x50
/* PCA9552 */
#define  PCA9552_1_I2C_BUS	1 /* I2C1 */
#define  PCA9552_1_I2C_ADDR	0x60

/* AHAB */
#ifdef CONFIG_AHAB_BOOT
#define AHAB_ENV "sec_boot=yes\0"
#else
#define AHAB_ENV "sec_boot=no\0"
#endif

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS \
	AHAB_ENV

/* Default location for tftp and bootm */

/* On CCP board, USDHC1 is for eMMC */

#define CFG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x880000000
/* Set default values to the smallest DDR we have in capricorn modules
 * Use it in case the system controller would return an error
 */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1 GB */
#define PHYS_SDRAM_2_SIZE		0x00000000	/* 0 GB */

#define BOOTAUX_RESERVED_MEM_BASE	0x88000000
#define BOOTAUX_RESERVED_MEM_SIZE	SZ_128M /* Reserve from second 128MB */

#endif /* __IMX8X_CAPRICORN_H */
