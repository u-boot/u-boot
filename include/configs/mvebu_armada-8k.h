/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_MVEBU_ARMADA_8K_H
#define _CONFIG_MVEBU_ARMADA_8K_H

#undef CONFIG_IDENT_STRING
#include "uboot_version.h"
#include "version.h"

#define CONFIG_ENV_WRITE_DEFAULT_IF_CRC_BAD
#define CONFIG_POPULATE_SERIAL_NUMBER

#define CONFIG_DEFAULT_CONSOLE		"console=ttyS0,115200 "\
					"earlycon=uart8250,mmio32,0xf0512000"

#include <configs/mvebu_armada-common.h>

#undef CONFIG_ENV_OFFSET
#undef CONFIG_ETHPRIME
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_ENV_OFFSET			0x00200000
#define CONFIG_ETHPRIME				"eth1"
#define CONFIG_FIT				1
#define CONFIG_SHA1				1
#define CONFIG_MD5				1
#define CONFIG_SYS_BOOTMAPSZ			(64 << 20)
#define CONFIG_SYS_BOOTM_LEN			(64 << 20)
#define CONFIG_SYS_NO_REPEATABLE
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_SYS_EEPROM
#define CONFIG_SYS_EEPROM_LOAD_ENV_MAC
#define CONFIG_SYS_EEPROM_MAX_NUM_ETH_PORTS	3
#define CONFIG_SYS_EEPROM_USE_COMMON_I2C_IO
#define CONFIG_SYS_I2C_EEPROM_BUS		0
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x56
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_OFFSET		0
#define CONFIG_SYS_EEPROM_MAX_SIZE		256

#ifdef CONFIG_VERSION_VARIABLE
	#define VER_VARIABLE "ver=" U_BOOT_VERSION " (" U_BOOT_DATE " - " U_BOOT_TIME ")" CONFIG_IDENT_STRING
#else
	#define VER_VARIABLE "\0"
#endif

#define PLATFORM_VARIABLE	\
	"consoledev=ttyS0\0"		\
	"baudrate=115200\0"		\
	"onie_start=0x00210000\0"	\
	"onie_sz.b=0x00df0000\0"	\
	"autoload=no\0"					\
	"stdin=serial\0stdout=serial\0stderr=serial\0"	\
	"onie_version=" UBOOT_ONIE_VERSION "\0"		\
	"onie_vendor_id=259\0"				\
	"onie_platform=arm64-accton_as4224-r0\0"	\
	"onie_machine=accton_as4224\0"			\
	"platform=accton_as4224\0"			\
	"onie_machine_rev=0\0"				\
	"dhcp_vendor-class-identifier=arm64-accton_as4224-r0\0"		\
	"dhcp_user-class=arm64-accton_as4224-r0_uboot\0"		\
	"onie_build_date=" UBOOT_BUILD_DATE "\0"			\
	"onie_uboot_version=" U_BOOT_VERSION_STRING "\0"		\
	"ver=" U_BOOT_VERSION_STRING "\0" 				\
	"onie_rescue=setenv onie_boot_reason rescue && boot\0"		\
	"onie_update=setenv onie_boot_reason update && boot\0"		\
	"onie_uninstall=setenv onie_boot_reason uninstall && boot\0"	\
	"onie_initargs=setenv bootargs quiet console=$consoledev,$baudrate maxcpus=4 pci=pcie_bus_safe cpuidle.off=1\0" \
	"onie_platformargs=setenv bootargs $bootargs serial_num=${serial#} "	\
	"eth_addr=$eth2addr "							\
	"$onie_bootargs $onie_debugargs \0"					\
	"onie_args=run onie_initargs onie_platformargs\0"			\
	"nos_bootcmd=echo\0"							\
	"onie_bootcmd=echo Loading Open Network Install Environment ...; "	\
	  "echo Platform: $onie_platform ; "					\
	  "echo Version : $onie_version ; "					\
	  "sf probe && sf read $loadaddr $onie_start ${onie_sz.b} && "		\
	  "run onie_args && bootm ${loadaddr}\0"				\
	"check_boot_reason=if test -n $onie_boot_reason; then "			\
	  "setenv onie_bootargs boot_reason=$onie_boot_reason; "		\
	  "run onie_bootcmd; "							\
	"fi;\0"

#define CONFIG_EXTRA_ENV_SETTINGS	"extra_params=pci=pcie_bus_safe\0" \
					"loadaddr=0x20000000\0"		\
					"fdt_high=0xffffffffffffffff\0"	\
					"hostname=marvell\0"		\
					"ramdisk_addr_r=0x8000000\0"	\
					"ramfs_name=-\0"		\
					"cpuidle=cpuidle.off=1\0"	\
					"fdt_name=fdt.dtb\0"		\
					"netdev=eth1\0"			\
					"image_name=Image\0"		\
					"get_ramfs=if test \"${ramfs_name}\"" \
						" != \"-\"; then setenv " \
						"ramdisk_addr_r 0x8000000; " \
						"tftpboot $ramdisk_addr_r " \
						"$ramfs_name; else setenv " \
						"ramdisk_addr_r -;fi\0"	\
					"get_images=tftpboot $kernel_addr_r " \
						"$image_name; tftpboot " \
						"$fdt_addr_r $fdt_name; " \
						"run get_ramfs\0"	\
					"console=" CONFIG_DEFAULT_CONSOLE "\0"\
					"root=root=/dev/nfs rw\0"	\
					"set_bootargs=setenv bootargs $console"\
						" $root ip=$ipaddr:$serverip:" \
						"$gatewayip:$netmask:$hostname"\
						":$netdev:none nfsroot="\
						"$serverip:$rootpath,tcp,v3 " \
						"$extra_params " \
						"$cpuidle\0" \
					"usb_pgood_delay=5000\0" \
					PLATFORM_VARIABLE \
					VER_VARIABLE

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

#define CONFIG_BOARD_EARLY_INIT_R

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_NAND_ONFI_DETECTION

#define CONFIG_USB_MAX_CONTROLLER_COUNT (3 + 3)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

/* RTC configuration */
#ifdef CONFIG_MARVELL_RTC
#define ERRATA_FE_3124064
#endif

#endif /* _CONFIG_MVEBU_ARMADA_8K_H */
