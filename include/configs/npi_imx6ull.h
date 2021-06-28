/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (c) 2021 Linumiz
 * Author: Navin Sankar Velliangiri <navin@linumiz.com>
 */

#ifndef _NPI_IMX6ULL_H
#define _NPI_IMX6ULL_H

#include <linux/sizes.h>
#include "mx6_common.h"

/* SPL options */
#include "imx6_spl.h"

#define CONFIG_SYS_FSL_USDHC_NUM	1

/* Size of malloc() poll */
#define CONFIG_SYS_MALLOC_LEN		SZ_2M

/* Console configs */
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

#define CONFIG_NETMASK			255.255.255.0

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR	\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment settings */
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		0
#elif defined(CONFIG_ENV_IS_IN_NAND)
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#endif

/* NAND */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	1

#ifdef CONFIG_CMD_NET
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR		0x1
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"eth0"
#endif

#define CONFIG_IMX_THERMAL

#define CONFIG_FEC_ENET_DEV		1

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200n8\0" \
	"image=zImage\0" \
	"fdtfile=imx6ull-seeed-npi-dev-board.dtb\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"fdt_addr_r=0x82000000\0" \
	"kernel_addr_r=0x81000000\0" \
	"pxefile_addr_r=0x87100000\0" \
	"ramdisk_addr_r=0x82100000\0" \
	"scriptaddr=0x87000000\0" \
	"root=/dev/mmcblk0p2 rootwait\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(UBIFS, ubifs, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#endif /* _NPI_IMX6ULL_H */
