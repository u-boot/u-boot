/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated.
 * Aneesh V       <aneesh@ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 *
 * TI OMAP4 common configuration settings
 */

#ifndef __CONFIG_TI_OMAP4_COMMON_H
#define __CONFIG_TI_OMAP4_COMMON_H

#ifndef CONFIG_SYS_L2CACHE_OFF
#define CFG_SYS_PL310_BASE	0x48242000
#endif

/* Get CPU defs */
#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/* Use General purpose timer 1 */
#define CFG_SYS_TIMERBASE		GPT2_BASE

#include <configs/ti_armv7_omap.h>

/*
 * Hardware drivers
 */
#define CFG_SYS_NS16550_CLK		48000000
#if !CONFIG_IS_ENABLED(DM_SERIAL)
#define CFG_SYS_NS16550_COM3		UART3_BASE
#endif

/*
 * Environment setup
 */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \

#include <config_distro_bootcmd.h>
#include <env/ti/mmc.h>

#define CFG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	DEFAULT_FIT_TI_ARGS \
	"console=ttyO2,115200n8\0" \
	"fdtfile=undefined\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	"loaduimage=load mmc ${mmcdev} ${loadaddr} uImage\0" \
	"uimageboot=echo Booting from mmc${mmcdev} ...; " \
		"run args_mmc; " \
		"bootm ${loadaddr}\0" \
	BOOTENV

#endif /* __CONFIG_TI_OMAP4_COMMON_H */
