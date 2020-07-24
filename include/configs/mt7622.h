/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7629 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef __MT7622_H
#define __MT7622_H

#include <linux/sizes.h>

#define CONFIG_SYS_MAXARGS		8
#define CONFIG_SYS_BOOTM_LEN		SZ_64M
#define CONFIG_SYS_CBSIZE		SZ_1K
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +	\
					sizeof(CONFIG_SYS_PROMPT) + 16)
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		SZ_4M
#define CONFIG_SYS_NONCACHED_MEMORY	SZ_1M

/* Uboot definition */
#define CONFIG_SYS_UBOOT_BASE                   CONFIG_SYS_TEXT_BASE

/* SPL -> Uboot */
#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + SZ_2M - \
					 GENERATED_GBL_DATA_SIZE)
/* UBoot -> Kernel */
#define CONFIG_LOADADDR		        0x4007ff28
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x40000000

/* Ethernet */
#define CONFIG_IPADDR			192.168.1.1
#define CONFIG_SERVERIP			192.168.1.3

#endif
