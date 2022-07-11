/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Sricharan R	  <r.sricharan@ti.com>
 *
 * Configuration settings for the TI EVM5430 board.
 * See ti_omap5_common.h for omap5 common settings.
 */

#ifndef __CONFIG_OMAP5_EVM_H
#define __CONFIG_OMAP5_EVM_H

#include <environment/ti/dfu.h>

#ifndef CONFIG_SPL_BUILD
/* Define the default GPT table for eMMC */
#define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	"name=rootfs,start=2MiB,size=-,uuid=${uuid_gpt_rootfs}"
#endif

#define DFUARGS \
	"dfu_bufsiz=0x10000\0" \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_RAM

#include <configs/ti_omap5_common.h>

#define CONFIG_SYS_NS16550_COM3		UART3_BASE

/* MMC ENV related defines */

/* Enhance our eMMC support / experience. */
#define CONFIG_HSMMC2_8BIT

/* Required support for the TCA642X GPIO we have on the uEVM */
#define CONFIG_TCA642X
#define CONFIG_SYS_I2C_TCA642X_BUS_NUM 4
#define CONFIG_SYS_I2C_TCA642X_ADDR 0x22

/* Enabled commands */

/* USB Networking options */

#define CONSOLEDEV		"ttyS2"

#endif /* __CONFIG_OMAP5_EVM_H */
