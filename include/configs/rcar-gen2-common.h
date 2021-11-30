/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/rcar-gen2-common.h
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 */

#ifndef __RCAR_GEN2_COMMON_H
#define __RCAR_GEN2_COMMON_H

#include <asm/arch/rmobile.h>

#ifdef CONFIG_SPL
#define CONFIG_SPL_TARGET	"spl/u-boot-spl.srec"
#endif

#ifndef CONFIG_PINCTRL_PFC
#define CONFIG_SH_GPIO_PFC
#endif

/* console */
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_BAUDRATE_TABLE	{ 38400, 115200 }

#define CONFIG_SYS_SDRAM_BASE		(RCAR_GEN2_SDRAM_BASE)
#define CONFIG_SYS_SDRAM_SIZE		(RCAR_GEN2_UBOOT_SDRAM_SIZE)

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)

/* ENV setting */

/* Common ENV setting */

/* SF MTD */
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_SPI
#undef CONFIG_DM_SPI_FLASH
#endif

/* Timer */
#define CONFIG_TMU_TIMER
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(TMU_BASE + 0xc)	/* TCNT0 */
#define CONFIG_SYS_TIMER_RATE		(CONFIG_SYS_CLK_FREQ / 8)

#endif	/* __RCAR_GEN2_COMMON_H */
