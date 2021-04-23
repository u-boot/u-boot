/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Timesys Corporation
 * Copyright (C) 2015 General Electric Company
 * Copyright (C) 2014 Advantech
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the GE MX6Q Bx50v3 boards.
 */

#ifndef __GE_BX50V3_CONFIG_H
#define __GE_BX50V3_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#define CONFIG_BOARD_NAME	"General Electric Bx50v3"

#include "mx6_common.h"
#include <linux/sizes.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

/* SATA Configs */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#endif

/* USB Configs */
#ifdef CONFIG_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#define CONFIG_USBD_HS
#define CONFIG_USB_GADGET_MASS_STORAGE
#endif

/* Serial Flash */

#define CONFIG_LOADADDR	0x12000000

#ifdef CONFIG_CMD_NFS
#define NETWORKBOOT \
        "setnetworkboot=" \
                "setenv ipaddr 172.16.2.10; setenv serverip 172.16.2.20; " \
                "setenv gatewayip 172.16.2.20; setenv nfsserver 172.16.2.20; " \
                "setenv netmask 255.255.255.0; setenv ethaddr ca:fe:de:ca:f0:11; " \
                "setenv bootargs root=/dev/nfs nfsroot=${nfsserver}:/srv/nfs/,v3,tcp rw rootwait" \
                "setenv bootargs $bootargs ip=${ipaddr}:${nfsserver}:${gatewayip}:${netmask}::eth0:off " \
                "setenv bootargs $bootargs cma=128M bootcause=${bootcause} ${videoargs} " \
                "setenv bootargs $bootargs systemd.mask=helix-network-defaults.service " \
                "setenv bootargs $bootargs watchdog.handle_boot_enabled=1\0" \
        "networkboot=" \
                "run setnetworkboot; " \
                "nfs ${loadaddr} /srv/nfs/fitImage; " \
                "bootm ${loadaddr}\0" \

#define CONFIG_NETWORKBOOTCOMMAND \
	"run networkboot; " \

#else
#define NETWORKBOOT \

#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	NETWORKBOOT \
	"image=/boot/fitImage\0" \
	"dev=mmc\0" \
	"devnum=2\0" \
	"rootdev=mmcblk0p\0" \
	"quiet=quiet loglevel=0\0" \
	"setargs=setenv bootargs root=/dev/${rootdev}${partnum} " \
		"ro rootwait cma=128M " \
		"bootcause=${bootcause} " \
		"${quiet} " \
		"${videoargs}" "\0" \
	"doquiet=" \
		"if ext2load ${dev} ${devnum}:5 0x7000A000 /boot/console; " \
			"then setenv quiet; fi\0" \
	"hasfirstboot=" \
		"test -e ${dev} ${devnum}:${partnum} /boot/bootcause/firstboot\0" \
	"swappartitions=" \
		"setexpr partnum 3 - ${partnum}\0" \
	"failbootcmd=" \
		"cls; " \
		"setcurs 5 4; " \
		"lcdputs \"Monitor failed to start. " \
		"Try again, or contact GE Service for support.\"; " \
		"bootcount reset; " \
		"while true; do sleep 1; done; \0" \
	"altbootcmd=" \
		"run doquiet; " \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run hasfirstboot || setenv partnum 0; " \
		"if test ${partnum} != 0; then " \
			"run swappartitions loadimage doboot; " \
		"fi; " \
		"run failbootcmd\0" \
	"loadimage=" \
		"ext2load ${dev} ${devnum}:${partnum} ${loadaddr} ${image}\0" \
	"doboot=" \
		"echo Booting from ${dev}:${devnum}:${partnum} ...; " \
		"run setargs; " \
		"bootm ${loadaddr}\0" \
	"tryboot=" \
		"setenv partnum 1; run hasfirstboot || setenv partnum 2; " \
		"run loadimage || run swappartitions && run loadimage || " \
			"setenv partnum 0 && echo MISSING IMAGE;" \
		"run doboot; " \
		"run failbootcmd\0" \

#define CONFIG_MMCBOOTCOMMAND \
	"run doquiet; " \
	"run tryboot; " \

#ifdef CONFIG_CMD_NFS
#define CONFIG_BOOTCOMMAND CONFIG_NETWORKBOOTCOMMAND
#else
#define CONFIG_BOOTCOMMAND CONFIG_MMCBOOTCOMMAND
#endif


/* Miscellaneous configurable options */

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_BOOTMAPSZ (256 << 20)     /* 256M */

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */

#define CONFIG_SYS_FSL_USDHC_NUM	3

/* Framebuffer */
#define CONFIG_HIDE_LOGO_VERSION
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP

#define CONFIG_IMX6_PWM_PER_CLK	66000000

#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(7, 12)
#define CONFIG_PCIE_IMX_POWER_GPIO	IMX_GPIO_NR(1, 5)

#endif	/* __GE_BX50V3_CONFIG_H */
