/*
 * ti_armv7_omap.h
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * The various ARMv7 SoCs from TI all share a number of IP blocks when
 * implementing a given feature. This is meant to isolate the features
 * that are based on OMAP architecture.
 */
#ifndef __CONFIG_TI_ARMV7_OMAP_H__
#define __CONFIG_TI_ARMV7_OMAP_H__

/* Common defines for all OMAP architecture based SoCs */
#define CONFIG_OMAP

/* I2C IP block */
#define CONFIG_SYS_OMAP24_I2C_SPEED	100000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_SYS_I2C_OMAP24XX

/* SPI IP Block */
#define CONFIG_OMAP3_SPI

/* GPIO block */
#define CONFIG_OMAP_GPIO

/*
 * GPMC NAND block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#ifdef CONFIG_NAND
#define CONFIG_NAND_OMAP_GPMC
#ifndef CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_NAND_BASE		0x8000000
#endif
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_CMD_NAND
#endif

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_TI_ARMV7_OMAP_H__ */
