/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 SECO USA
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the SECO Tiago board.
 */
#ifndef __TIAGO_CONFIG_H
#define __TIAGO_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include <linux/stringify.h>
#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>

#include "imx6_spl.h"
#undef CONFIG_SPL_PAD_TO
#define CONFIG_SPL_PAD_TO 0

#define PHYS_SDRAM_SIZE	SZ_1G

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

#define CONFIG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#ifdef CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC1_BASE_ADDR
#endif

#if IS_ENABLED(CONFIG_CHAIN_OF_TRUST)
#define ALTBOOTCMD \
	"altbootcmd=echo Entering fastboot...; " \
		"run fastboot_aliases; " \
		"fastboot 0; " \
		"run bootcmd;\0"
#else
#define ALTBOOTCMD \
	"altbootcmd=echo Entering fastboot...; " \
		"run fastboot_aliases; " \
		"fastboot 0\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"image=vmlinuz\0" \
	"altimage=vmlinuz.old\0" \
	"console=ttymxc0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"ip_dyn=yes\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=boot\0" \
	"mmcrootpart=root\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=PARTLABEL=${mmcrootpart} ro rootwait " \
		"seco.board_rev=${board_rev} " \
		"seco.u_boot_ver=${ver}\0"  \
	"loadimage=load mmc ${mmcdev}#${mmcpart} ${loadaddr} ${mmcimage}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm;\0" \
	"fastboot_aliases=setenv fastboot_partition_alias_user ${mmcdev}.0:0; " \
		"setenv fastboot_partition_alias_boot0 ${mmcdev}.1:0; " \
		"setenv fastboot_partition_alias_boot1 ${mmcdev}.2:0;\0" \
	ALTBOOTCMD \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp rw\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"bootm;\0" \


#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; " \
	"if mmc rescan; then " \
		"setenv mmcimage ${image}; " \
		"if run loadimage; then " \
			"run mmcboot; " \
		"else " \
			"setenv mmcimage ${altimage}; " \
			"if run loadimage; then " \
				"run mmcboot; " \
			"fi; " \
		"fi; " \
	"fi; " \
	"run altbootcmd"

/* Miscellaneous configurable options */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_MMCROOT			"/dev/mmcblk1p1"  /* USDHC2 */

#define CONFIG_IMX_THERMAL

#define CONFIG_IOMUX_LPSR

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS   0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#endif

#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_ENET_DEV		1
#if (CONFIG_FEC_ENET_DEV == 0)
#define CONFIG_ETHPRIME			"eth0"
#elif (CONFIG_FEC_ENET_DEV == 1)
#define CONFIG_ETHPRIME			"eth1"
#endif
#endif

#endif
