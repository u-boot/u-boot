/*
 * Configuration for Xilinx ZynqMP
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * Based on Configuration for Versatile Express
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __XILINX_ZYNQMP_H
#define __XILINX_ZYNQMP_H

#define CONFIG_REMAKE_ELF

/* #define CONFIG_ARMV8_SWITCH_TO_EL1 */

#define CONFIG_SYS_NO_FLASH

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0xF9010000
#define GICC_BASE	0xF9020000

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_SCRATCH	0xfffc0000

#ifndef CONFIG_NR_DRAM_BANKS
# define CONFIG_NR_DRAM_BANKS		2
#endif
#define CONFIG_SYS_MEMTEST_START	0
#define CONFIG_SYS_MEMTEST_END		1000

/* Have release address at the end of 256MB for now */
#define CPU_RELEASE_ADDR	0xFFFFFF0

/* Cache Definitions */
#define CONFIG_SYS_CACHELINE_SIZE	64

#if !defined(CONFIG_IDENT_STRING)
# define CONFIG_IDENT_STRING		" Xilinx ZynqMP"
#endif

#define CONFIG_SYS_INIT_SP_ADDR		0xfffffffc

/* Generic Timer Definitions - setup in EL3. Setup by ATF for other cases */
#if !defined(COUNTER_FREQUENCY)
# define COUNTER_FREQUENCY		100000000
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 0x2000000)

/* Serial setup */
#define CONFIG_ARM_DCC
#define CONFIG_CPU_ARMV8
#define CONFIG_ZYNQ_SERIAL

#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/* Command line configuration */
#define CONFIG_CMD_ENV
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#ifndef CONFIG_SPL_BUILD
# define CONFIG_ISO_PARTITION
#endif
#define CONFIG_MP

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MAY_FAIL
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_SUBNETMASK

/* Diff from config_distro_defaults.h */
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_ENV_VARS_UBOOT_CONFIG
#define CONFIG_AUTO_COMPLETE

/* PXE */
#define CONFIG_CMD_PXE
#define CONFIG_MENU

#ifdef CONFIG_ZYNQMP_QSPI
# define CONFIG_SPI_GENERIC
# define CONFIG_SF_DEFAULT_SPEED	30000000
# define CONFIG_SF_DUAL_FLASH
# define CONFIG_CMD_SF_TEST
#endif

#if defined(CONFIG_ZYNQ_SDHCI)
# define CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_SUPPORT_EMMC_BOOT
# define CONFIG_SDHCI
# ifndef CONFIG_ZYNQ_SDHCI_MAX_FREQ
#  define CONFIG_ZYNQ_SDHCI_MAX_FREQ	200000000
# endif
#endif

#if defined(CONFIG_ZYNQ_SDHCI) || defined(CONFIG_ZYNQMP_USB)
# define CONFIG_FAT_WRITE
#endif

#ifdef CONFIG_NAND_ARASAN
# define CONFIG_CMD_NAND_LOCK_UNLOCK
# define CONFIG_SYS_MAX_NAND_DEVICE	1
# define CONFIG_SYS_NAND_SELF_INIT
# define CONFIG_SYS_NAND_ONFI_DETECTION
# define CONFIG_MTD_DEVICE
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		0x8000000

#if defined(CONFIG_ZYNQMP_USB)
#define CONFIG_USB_MAX_CONTROLLER_COUNT         1
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS      2
#define CONFIG_USB_STORAGE
#define CONFIG_USB_XHCI_ZYNQMP

#define CONFIG_SYS_DFU_DATA_BUF_SIZE	0x1800000
#define DFU_DEFAULT_POLL_TIMEOUT	300
#define CONFIG_USB_FUNCTION_DFU
#define CONFIG_DFU_RAM
#define CONFIG_USB_CABLE_CHECK
#define CONFIG_CMD_THOR_DOWNLOAD
#define CONFIG_USB_FUNCTION_THOR
#define CONFIG_THOR_RESET_OFF
#define DFU_ALT_INFO_RAM \
	"dfu_ram_info=" \
	"setenv dfu_alt_info " \
	"Image ram $kernel_addr $kernel_size\\\\;" \
	"system.dtb ram $fdt_addr $fdt_size\0" \
	"dfu_ram=run dfu_ram_info && dfu 0 ram 0\0" \
	"thor_ram=run dfu_ram_info && thordown 0 ram 0\0"

# define DFU_ALT_INFO  \
		DFU_ALT_INFO_RAM
#endif

#if !defined(DFU_ALT_INFO)
# define DFU_ALT_INFO
#endif

/* Initial environment variables */
#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr=0x200000\0" \
	"initrd_addr=0xa00000\0" \
	"initrd_size=0x2000000\0" \
	"fdt_addr=4000000\0" \
	"fdt_high=0x10000000\0" \
	"loadbootenv_addr=0x100000\0" \
	"sdbootdev=0\0"\
	"kernel_offset=0x180000\0" \
	"fdt_offset=0x100000\0" \
	"kernel_size=0x1e00000\0" \
	"fdt_size=0x80000\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc $sdbootdev:$partid ${loadbootenv_addr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from SD ...; " \
		"env import -t ${loadbootenv_addr} $filesize\0" \
	"sd_uEnvtxt_existence_test=test -e mmc $sdbootdev:$partid /uEnv.txt\0" \
	"sata_root=if test $scsidevs -gt 0; then setenv bootargs $bootargs root=/dev/sda rw rootfstype=ext4; fi\0" \
	"sataboot=load scsi 0 80000 boot/Image && load scsi 0 $fdt_addr boot/system.dtb && booti 80000 - $fdt_addr\0" \
	"veloce=fdt addr f000000 && fdt resize" \
		"fdt set /amba/misc_clk clock-frequency <48000> && "\
		"fdt set /timer clock-frequency <240000> && " \
		"fdt set /amba/i2c_clk clock-frequency <240000> && " \
		"booti 80000 - f000000\0" \
	"netboot=tftpboot 10000000 image.ub && bootm\0" \
	"qspiboot=sf probe 0 0 0 && sf read $fdt_addr $fdt_offset $fdt_size && " \
		  "sf read $kernel_addr $kernel_offset $kernel_size && " \
		  "booti $kernel_addr - $fdt_addr\0" \
	"uenvboot=" \
		"if run sd_uEnvtxt_existence_test; then " \
			"run loadbootenv; " \
			"echo Loaded environment from ${bootenv}; " \
			"run importbootenv; " \
		"fi\0" \
	"sdboot=mmc dev $sdbootdev && mmcinfo && run uenvboot; " \
		"load mmc $sdbootdev:$partid $fdt_addr system.dtb && " \
		"load mmc $sdbootdev:$partid $kernel_addr Image && " \
		"booti $kernel_addr - $fdt_addr\0" \
	"nandboot=nand info && nand read $fdt_addr $fdt_offset $fdt_size && " \
		  "nand read $kernel_addr $kernel_offset $kernel_size && " \
		  "booti $kernel_addr - $fdt_addr\0" \
	"xen=tftpb $fdt_addr system.dtb && fdt addr $fdt_addr && fdt resize && " \
		"tftpb 0x80000 Image && " \
		"fdt set /chosen/dom0 reg <0x80000 0x$filesize> && "\
		"tftpb 6000000 xen.ub && bootm 6000000 - $fdt_addr\0" \
	"jtagboot=tftpboot 80000 Image && tftpboot $fdt_addr system.dtb && " \
		 "tftpboot 6000000 rootfs.cpio.ub && booti 80000 6000000 $fdt_addr\0" \
	"nosmp=setenv bootargs $bootargs maxcpus=1\0" \
	"nfsroot=setenv bootargs $bootargs root=/dev/nfs nfsroot=$serverip:/mnt/sata,tcp ip=$ipaddr:$serverip:$serverip:255.255.255.0:zynqmp:eth0:off rw\0" \
	"sdroot=setenv bootargs $bootargs root=/dev/mmcblk0p2 rw rootwait\0" \
	"sdroot1=setenv bootargs $bootargs root=/dev/mmcblk1p2 rw rootwait\0" \
	"android=setenv bootargs $bootargs init=/init androidboot.selinux=disabled androidboot.hardware=$board\0" \
	"android_debug=run android && setenv bootargs $bootargs video=DP-1:1024x768@60 drm.debug=0xf\0" \
	"usbhostboot=usb start && load usb 0 $fdt_addr system.dtb && " \
		     "load usb 0 $kernel_addr Image && " \
		     "booti $kernel_addr - $fdt_addr\0" \
	DFU_ALT_INFO
#endif

#define CONFIG_PREBOOT		"run setup"
#define CONFIG_BOOTCOMMAND	"run $modeboot"

#define CONFIG_BOARD_LATE_INIT

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_MAXARGS		64

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM)
# define CONFIG_NET_MULTI
# define CONFIG_MII
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_PHY_MARVELL
# define CONFIG_PHY_NATSEMI
# define CONFIG_PHY_TI
# define CONFIG_PHY_GIGE
# define CONFIG_PHY_VITESSE
# define CONFIG_PHY_REALTEK
# define PHY_ANEG_TIMEOUT       20000
#endif

/* I2C */
#if defined(CONFIG_SYS_I2C_ZYNQ)
# define CONFIG_SYS_I2C
# define CONFIG_SYS_I2C_ZYNQ_SPEED		100000
# define CONFIG_SYS_I2C_ZYNQ_SLAVE		0
#endif

/* EEPROM */
#ifdef CONFIG_ZYNQMP_EEPROM
# define CONFIG_CMD_EEPROM
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
# define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
# define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
# define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
# define CONFIG_SYS_EEPROM_SIZE			(64 * 1024)
#endif

#ifdef CONFIG_SATA_CEVA
#define CONFIG_AHCI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	2
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#define CONFIG_SCSI
#endif

#define CONFIG_ARM_SMC

#define CONFIG_FPGA_ZYNQMPPL
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA

#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

#define CONFIG_CMD_BOOTI
#define CONFIG_CMD_UNZIP

#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_CLOCKS

#define ENV_MEM_LAYOUT_SETTINGS \
	"fdt_high=10000000\0" \
	"initrd_high=10000000\0" \
	"fdt_addr_r=0x40000000\0" \
	"pxefile_addr_r=0x10000000\0" \
	"kernel_addr_r=0x18000000\0" \
	"scriptaddr=0x02000000\0" \
	"ramdisk_addr_r=0x02100000\0" \

#if defined(CONFIG_ZYNQ_SDHCI)
# define BOOT_TARGET_DEVICES_MMC(func)	func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
# define BOOT_TARGET_DEVICES_MMC(func)
#endif

#if defined(CONFIG_SATA_CEVA)
# define BOOT_TARGET_DEVICES_SCSI(func)	func(SCSI, scsi, 0)
#else
# define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#if defined(CONFIG_ZYNQMP_USB)
# define BOOT_TARGET_DEVICES_USB(func)	func(USB, usb, 0) func(USB, usb, 1)
#else
# define BOOT_TARGET_DEVICES_USB(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Initial environment variables */
#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV \
	DFU_ALT_INFO
#endif

#define CONFIG_SPL_TEXT_BASE		0xfffc0000
#define CONFIG_SPL_MAX_SIZE		0x20000

/* Just random location in OCM */
#define CONFIG_SPL_BSS_START_ADDR	0x1000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x2000000

#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_RAM_DEVICE

#define CONFIG_SPL_OS_BOOT
/* u-boot is like dtb */
#define CONFIG_SPL_FS_LOAD_ARGS_NAME	"u-boot.bin"
#define CONFIG_SYS_SPL_ARGS_ADDR	0x8000000

/* ATF is my kernel image */
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME	"atf.ub"

/* FIT load address for RAM boot */
#define CONFIG_SPL_LOAD_FIT_ADDRESS	0x10000000

/* MMC support */
#ifdef CONFIG_ZYNQ_SDHCI
# define CONFIG_SPL_MMC_SUPPORT
# define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
# define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0 /* unused */
# define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0 /* unused */
# define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0 /* unused */
# define CONFIG_SPL_LIBDISK_SUPPORT
# define CONFIG_SPL_FAT_SUPPORT
# define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"
#endif

#endif /* __XILINX_ZYNQMP_H */
