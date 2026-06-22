/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2025 Renesas Electronics Corporation
 */

#ifndef __RCAR_GEN5_COMMON_H
#define __RCAR_GEN5_COMMON_H

#include <asm/arch/renesas.h>

/* Console */
#define CFG_SYS_BAUDRATE_TABLE		{ 38400, 115200, 921600, 1843200, 3250000 }
#define CFG_HSCIF

/* Memory */
#define DRAM_RSV_SIZE			0x20600000
#ifdef CONFIG_RCAR_64_RSIP
#define CFG_SYS_SDRAM_BASE		0xb8400000
#else
#define CFG_SYS_SDRAM_BASE		(0x40000000 + DRAM_RSV_SIZE)
#endif
#define CFG_SYS_SDRAM_SIZE		(0x80000000u - DRAM_RSV_SIZE)
#define CFG_MAX_MEM_MAPPED		(0x80000000u - DRAM_RSV_SIZE)

/* Timer */
#if defined(CONFIG_RCAR_64_RSIP)
#define CFG_SYS_TIMER_COUNTER		(TMU_BASE + 0xc)	/* TCNT0 */
#define CFG_SYS_TIMER_RATE		(133333333 / 4)

/* Environment setting */
#define CFG_EXTRA_ENV_SETTINGS						\
	"rsip_ipl_params_base=0x8c100000\0"				\
	"rsip_ipl_params_optee=0x8c100088\0"				\
	"rsip_ipl_params_uboot=0x8c100030\0"				\
	"rsip_ipl_optee_ep=0x8c400000\0"				\
	"rsip_ipl_tfa_ep=0x8c200000\0"					\
	"rsip_ipl_uboot_ep=0x8e300000\0"				\
	"rsip_ipl_params_write="					\
		"base ${rsip_ipl_params_base} ; "			\
		"mw 0x00 0 0x9e ; "	/* Clear the area */		\
		"mw 0x00 0x00300103 ; " /* type, version, size */	\
		"mw 0x20 0x${rsip_ipl_params_uboot} ; " /* U-Boot descriptor */ \
		""							\
		"base ${rsip_ipl_params_uboot} ; "			\
		"mw 0x00 0x00580101 ; " /* type, version, size */	\
		"mw 0x04 0x00000001 ; " /* attr */			\
		"mw 0x08 ${rsip_ipl_uboot_ep} ; " /* U-Boot entry point */ \
		"mw 0x10 0x000003c5 ; " /* SPSR */			\
		""							\
		"base ${rsip_ipl_params_optee} ; "			\
		"mw 0x00 0x00580201 ; " /* type, version, size */	\
		"mw 0x04 0x00000008 ; " /* attr */			\
		"mw 0x08 ${rsip_ipl_optee_ep} ; " /* OPTEE-OS entry point */ \
		"mw 0x10 0x000003c5 ; " /* SPSR */			\
		""							\
		"base 0\0"						\
	"rsip_ipl_boot_ca0=" /* Start TFA BL31, OPTEE-OS, U-Boot on Cortex-A720AE core 0 */ \
		"scsi scan && " /* Scan for UFS devices */		\
		"rproc init && " /* Start remoteproc */			\
		"rproc load 0 0x344c0000 0x60000 && " /* Load SCP from HF */ \
		"rproc start 0 && " /* Start SCP */			\
		"scsi read ${rsip_ipl_uboot_ep} 0x7200 0x100 && " /* Load U-Boot from UFS */ \
		"scsi read ${rsip_ipl_optee_ep} 0x5200 0x200 && " /* Load OPTEE-OS from UFS */ \
		"scsi read ${rsip_ipl_tfa_ep} 0x5000 0x40 && " /* Load TFA BL31 from UFS */ \
		"run rsip_ipl_params_write && " /* Write entry point descriptors */ \
		"rproc load 13 ${rsip_ipl_tfa_ep} 4 && " /* Set up Cortex-A720AE Core 0 */ \
		"rproc start 13\0"	/* Start Cortex-A720AE Core 0 */

#else
/* Environment setting */
#define CFG_EXTRA_ENV_SETTINGS		\
	"bootm_size=0x10000000\0"
#endif

#endif	/* __RCAR_GEN5_COMMON_H */
