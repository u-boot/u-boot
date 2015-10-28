/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2013 Xilinx, Inc.
 *
 * Common configuration options for all Zynq boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_COMMON_H
#define __CONFIG_ZYNQ_COMMON_H

/* CPU clock */
#ifndef CONFIG_CPU_FREQ_HZ
# define CONFIG_CPU_FREQ_HZ	800000000
#endif

/* Cache options */
#define CONFIG_CMD_CACHE
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_L2CACHE_OFF
#ifndef CONFIG_SYS_L2CACHE_OFF
# define CONFIG_SYS_L2_PL310
# define CONFIG_SYS_PL310_BASE		0xf8f02000
#endif

#define ZYNQ_SCUTIMER_BASEADDR		0xF8F00600
#define CONFIG_SYS_TIMERBASE		ZYNQ_SCUTIMER_BASEADDR
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(CONFIG_SYS_TIMERBASE + 0x4)

/* Serial drivers */
#define CONFIG_BAUDRATE		115200
/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CONFIG_ZYNQ_GPIO
#define CONFIG_CMD_GPIO

/* DCC driver */
#if defined(CONFIG_ZYNQ_DCC)
# define CONFIG_ARM_DCC
#else
# if defined(CONFIG_ZYNQ_SERIAL_UART0) || defined(CONFIG_ZYNQ_SERIAL_UART1)
#  define CONFIG_ZYNQ_SERIAL
# endif
#endif

#define CONFIG_ZYNQ_GPIO
#define CONFIG_CMD_GPIO

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM0) || defined(CONFIG_ZYNQ_GEM1)
# define CONFIG_ZYNQ_GEM
# define CONFIG_MII
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_PHYLIB
# define CONFIG_PHY_MARVELL
# define CONFIG_SYS_ENET
# define CONFIG_BOOTP_SERVERIP
# define CONFIG_BOOTP_BOOTPATH
# define CONFIG_BOOTP_GATEWAY
# define CONFIG_BOOTP_HOSTNAME
# define CONFIG_BOOTP_MAY_FAIL
# if !defined(CONFIG_ZYNQ_GEM_EMIO0)
#  define CONFIG_ZYNQ_GEM_EMIO0	0
# endif
# if !defined(CONFIG_ZYNQ_GEM_EMIO1)
#  define CONFIG_ZYNQ_GEM_EMIO1	0
# endif
#endif

/* SPI */
#ifdef CONFIG_ZYNQ_SPI
# define CONFIG_SPI_FLASH_SST
#endif

#if defined(CONFIG_ZYNQ_SPI) || defined(CONFIG_ZYNQ_QSPI)
# define CONFIG_CMD_SPI
# define CONFIG_CMD_SF
#endif

/* NOR */
#ifndef CONFIG_SYS_NO_FLASH
# define CONFIG_SYS_FLASH_BASE		0xE2000000
# define CONFIG_SYS_FLASH_SIZE		(16 * 1024 * 1024)
# define CONFIG_SYS_MAX_FLASH_BANKS	1
# define CONFIG_SYS_MAX_FLASH_SECT	512
# define CONFIG_SYS_FLASH_ERASE_TOUT	1000
# define CONFIG_SYS_FLASH_WRITE_TOUT	5000
# define CONFIG_FLASH_SHOW_PROGRESS	10
# define CONFIG_SYS_FLASH_CFI
# undef CONFIG_SYS_FLASH_EMPTY_INFO
# define CONFIG_FLASH_CFI_DRIVER
# undef CONFIG_SYS_FLASH_PROTECTION
# define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
# define CONFIG_ZYNQ_M29EW_WB_HACK
#endif

/* MMC */
#if defined(CONFIG_ZYNQ_SDHCI0) || defined(CONFIG_ZYNQ_SDHCI1)
# define CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_SDHCI
# define CONFIG_ZYNQ_SDHCI
# define CONFIG_CMD_MMC
# define CONFIG_ZYNQ_SDHCI_MAX_FREQ	52000000
#endif

#ifdef CONFIG_ZYNQ_USB
# define CONFIG_USB_EHCI
# define CONFIG_CMD_USB
# define CONFIG_USB_STORAGE
# define CONFIG_USB_EHCI_ZYNQ
# define CONFIG_USB_ULPI_VIEWPORT
# define CONFIG_USB_ULPI
# define CONFIG_EHCI_IS_TDI
# define CONFIG_USB_MAX_CONTROLLER_COUNT	2

# define CONFIG_CI_UDC           /* ChipIdea CI13xxx UDC */
# define CONFIG_USB_GADGET
# define CONFIG_USB_GADGET_DUALSPEED
# define CONFIG_USBDOWNLOAD_GADGET
# define CONFIG_SYS_DFU_DATA_BUF_SIZE	0x600000
# define DFU_DEFAULT_POLL_TIMEOUT	300
# define CONFIG_DFU_FUNCTION
# define CONFIG_DFU_RAM
# define CONFIG_USB_GADGET_VBUS_DRAW	2
# define CONFIG_G_DNL_VENDOR_NUM	0x03FD
# define CONFIG_G_DNL_PRODUCT_NUM	0x0300
# define CONFIG_G_DNL_MANUFACTURER	"Xilinx"
# define CONFIG_USB_GADGET
# define CONFIG_USB_CABLE_CHECK
# define CONFIG_CMD_DFU
# define CONFIG_CMD_THOR_DOWNLOAD
# define CONFIG_THOR_FUNCTION
# define CONFIG_THOR_RESET_OFF
# define DFU_ALT_INFO_RAM \
	"dfu_ram_info=" \
	"set dfu_alt_info " \
	"${kernel_image} ram 0x3000000 0x500000\\\\;" \
	"${devicetree_image} ram 0x2A00000 0x20000\\\\;" \
	"${ramdisk_image} ram 0x2000000 0x600000\0" \
	"dfu_ram=run dfu_ram_info && dfu 0 ram 0\0" \
	"thor_ram=run dfu_ram_info && thordown 0 ram 0\0"

# if defined(CONFIG_ZYNQ_SDHCI0) || defined(CONFIG_ZYNQ_SDHCI1)
#  define CONFIG_DFU_MMC
#  define DFU_ALT_INFO_MMC \
	"dfu_mmc_info=" \
	"set dfu_alt_info " \
	"${kernel_image} fat 0 1\\\\;" \
	"${devicetree_image} fat 0 1\\\\;" \
	"${ramdisk_image} fat 0 1\0" \
	"dfu_mmc=run dfu_mmc_info && dfu 0 mmc 0\0" \
	"thor_mmc=run dfu_mmc_info && thordown 0 mmc 0\0"

#  define DFU_ALT_INFO	\
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_MMC
# else
#  define DFU_ALT_INFO	\
	DFU_ALT_INFO_RAM
# endif
#endif

#if !defined(DFU_ALT_INFO)
# define DFU_ALT_INFO
#endif

#if defined(CONFIG_ZYNQ_SDHCI) || defined(CONFIG_ZYNQ_USB)
# define CONFIG_SUPPORT_VFAT
# define CONFIG_CMD_FAT
# define CONFIG_CMD_EXT2
# define CONFIG_FAT_WRITE
# define CONFIG_DOS_PARTITION
# define CONFIG_CMD_EXT4
# define CONFIG_CMD_EXT4_WRITE
# define CONFIG_CMD_FS_GENERIC
#endif

/* QSPI */
#ifdef CONFIG_ZYNQ_QSPI
# define CONFIG_SF_DEFAULT_SPEED	30000000
# define CONFIG_SPI_FLASH_SPANSION
# define CONFIG_SPI_FLASH_STMICRO
# define CONFIG_SPI_FLASH_WINBOND
# define CONFIG_SPI_FLASH_ISSI
# define CONFIG_SPI_FLASH_MACRONIX
# define CONFIG_SF_DUAL_FLASH
#endif

/* NAND */
#ifdef CONFIG_NAND_ZYNQ
# define CONFIG_CMD_NAND
# define CONFIG_CMD_NAND_LOCK_UNLOCK
# define CONFIG_SYS_MAX_NAND_DEVICE 1
# define CONFIG_SYS_NAND_SELF_INIT
# define CONFIG_SYS_NAND_ONFI_DETECTION
# define CONFIG_MTD_DEVICE
#endif

#if defined(CONFIG_ZYNQ_I2C0) || defined(CONFIG_ZYNQ_I2C1)
#define CONFIG_SYS_I2C_ZYNQ
#endif

/* I2C */
#if defined(CONFIG_SYS_I2C_ZYNQ)
# define CONFIG_CMD_I2C
# define CONFIG_SYS_I2C
# define CONFIG_SYS_I2C_ZYNQ_SPEED		100000
# define CONFIG_SYS_I2C_ZYNQ_SLAVE		0
#endif

/* EEPROM */
#ifdef CONFIG_ZYNQ_EEPROM
# define CONFIG_CMD_EEPROM
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
# define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
# define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
# define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
# define CONFIG_SYS_EEPROM_SIZE			1024 /* Bytes */
# define CONFIG_SYS_I2C_MUX_ADDR		0x74
# define CONFIG_SYS_I2C_MUX_EEPROM_SEL		0x4
#endif

/* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE			(128 << 10)

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Environment */
#ifndef CONFIG_ENV_IS_NOWHERE
# ifndef CONFIG_SYS_NO_FLASH
/* Environment in NOR flash */
#  define CONFIG_ENV_IS_IN_FLASH
# elif defined(CONFIG_ZYNQ_QSPI)
/* Environment in Serial Flash */
#  define CONFIG_ENV_IS_IN_SPI_FLASH
# elif defined(CONFIG_NAND_ZYNQ)
/* Environment in NAND flash */
#  define CONFIG_ENV_IS_IN_NAND
# elif defined(CONFIG_SYS_NO_FLASH)
#  define CONFIG_ENV_IS_NOWHERE
# endif

# define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE
# ifndef CONFIG_ENV_OFFSET
#  define CONFIG_ENV_OFFSET		0xE0000
# endif
#endif

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_image=uImage\0"	\
	"kernel_load_address=0x2080000\0" \
	"ramdisk_image=uramdisk.image.gz\0"	\
	"ramdisk_load_address=0x4000000\0"	\
	"devicetree_image=devicetree.dtb\0"	\
	"devicetree_load_address=0x2000000\0"	\
	"bitstream_image=system.bit.bin\0"	\
	"boot_image=BOOT.bin\0"	\
	"loadbit_addr=0x100000\0"	\
	"loadbootenv_addr=0x2000000\0" \
	"kernel_size=0x500000\0"	\
	"devicetree_size=0x20000\0"	\
	"ramdisk_size=0x5E0000\0"	\
	"boot_size=0xF00000\0"	\
	"fdt_high=0x20000000\0"	\
	"initrd_high=0x20000000\0"	\
	"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc 0 ${loadbootenv_addr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from SD ...; " \
		"env import -t ${loadbootenv_addr} $filesize\0" \
	"sd_uEnvtxt_existence_test=test -e mmc 0 /uEnv.txt\0" \
	"preboot=if test $modeboot = sdboot && env run sd_uEnvtxt_existence_test; " \
			"then if env run loadbootenv; " \
				"then env run importbootenv; " \
			"fi; " \
		"fi; \0" \
	"mmc_loadbit=echo Loading bitstream from SD/MMC/eMMC to RAM.. && " \
		"mmcinfo && " \
		"load mmc 0 ${loadbit_addr} ${bitstream_image} && " \
		"fpga load 0 ${loadbit_addr} ${filesize}\0" \
	"norboot=echo Copying Linux from NOR flash to RAM... && " \
		"cp.b 0xE2100000 ${kernel_load_address} ${kernel_size} && " \
		"cp.b 0xE2600000 ${devicetree_load_address} ${devicetree_size} && " \
		"echo Copying ramdisk... && " \
		"cp.b 0xE2620000 ${ramdisk_load_address} ${ramdisk_size} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"qspiboot=echo Copying Linux from QSPI flash to RAM... && " \
		"sf probe 0 0 0 && " \
		"sf read ${kernel_load_address} 0x100000 ${kernel_size} && " \
		"sf read ${devicetree_load_address} 0x600000 ${devicetree_size} && " \
		"echo Copying ramdisk... && " \
		"sf read ${ramdisk_load_address} 0x620000 ${ramdisk_size} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"uenvboot=" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv}; " \
			"run importbootenv; " \
		"fi; " \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...; " \
			"run uenvcmd; " \
		"fi\0" \
	"sdboot=if mmcinfo; then " \
			"run uenvboot; " \
			"echo Copying Linux from SD to RAM... && " \
			"load mmc 0 ${kernel_load_address} ${kernel_image} && " \
			"load mmc 0 ${devicetree_load_address} ${devicetree_image} && " \
			"load mmc 0 ${ramdisk_load_address} ${ramdisk_image} && " \
			"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}; " \
		"fi\0" \
	"usbboot=if usb start; then " \
			"run uenvboot; " \
			"echo Copying Linux from USB to RAM... && " \
			"load usb 0 ${kernel_load_address} ${kernel_image} && " \
			"load usb 0 ${devicetree_load_address} ${devicetree_image} && " \
			"load usb 0 ${ramdisk_load_address} ${ramdisk_image} && " \
			"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}; " \
		"fi\0" \
	"nandboot=echo Copying Linux from NAND flash to RAM... && " \
		"nand read ${kernel_load_address} 0x100000 ${kernel_size} && " \
		"nand read ${devicetree_load_address} 0x600000 ${devicetree_size} && " \
		"echo Copying ramdisk... && " \
		"nand read ${ramdisk_load_address} 0x620000 ${ramdisk_size} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"jtagboot=echo TFTPing Linux to RAM... && " \
		"tftpboot ${kernel_load_address} ${kernel_image} && " \
		"tftpboot ${devicetree_load_address} ${devicetree_image} && " \
		"tftpboot ${ramdisk_load_address} ${ramdisk_image} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_norboot=echo Copying Image from NOR flash to RAM... && " \
		"cp.b 0xE2100000 0x100000 ${boot_size} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_nandboot=echo Copying Image from NAND flash to RAM... && " \
		"nand read 0x100000 0x0 ${boot_size} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_qspiboot=echo Copying Image from QSPI flash to RAM... && " \
		"sf probe 0 0 0 && " \
		"sf read 0x100000 0x0 ${boot_size} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_sdboot=echo Copying Image from SD to RAM... && " \
		"load mmc 0 0x100000 ${boot_image} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_jtagboot=echo TFTPing Image to RAM... && " \
		"tftpboot 0x100000 ${boot_image} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
		DFU_ALT_INFO

/* Default environment */
#define CONFIG_IPADDR	10.10.70.102
#define CONFIG_SERVERIP	10.10.70.101

/* default boot is according to the bootmode switch settings */
#if defined(CONFIG_CMD_ZYNQ_RSA)
#define CONFIG_BOOTCOMMAND		"run rsa_$modeboot"
#else
#define CONFIG_BOOTCOMMAND		"run $modeboot"
#endif
#define CONFIG_BOOTDELAY		3 /* -1 to Disable autoboot */
#define CONFIG_SYS_LOAD_ADDR		0 /* default? */

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT		"zynq-uboot> "
#define CONFIG_SYS_HUSH_PARSER

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SYS_LONGHELP
#define CONFIG_CLOCKS
#define CONFIG_CMD_CLK
#define CONFIG_SYS_MAXARGS		32 /* max number of command args */
#define CONFIG_SYS_CBSIZE		2048 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* Physical Memory map */
#if defined(CONFIG_CSE_QSPI) || defined(CONFIG_CSE_NOR)
# define CONFIG_SYS_TEXT_BASE		0xFFFC4800
#elif defined(CONFIG_CSE_NAND)
# define CONFIG_SYS_TEXT_BASE		0x00100000
#elif defined(CONFIG_ZYNQ_OCM)
# define CONFIG_SYS_TEXT_BASE		0xFFFC0000
#else
# define CONFIG_SYS_TEXT_BASE		0x4000000
#endif

#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x1000)

#define CONFIG_SYS_MALLOC_LEN		0xC00000

#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)

/* Enable the PL to be downloaded */
#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_ZYNQPL
#define CONFIG_CMD_FPGA_LOADMK
#define CONFIG_CMD_FPGA_LOADP
#define CONFIG_CMD_FPGA_LOADBP
#define CONFIG_CMD_FPGA_LOADFS

/* Open Firmware flat tree */
#define CONFIG_OF_LIBFDT

/* FIT support */
#define CONFIG_IMAGE_FORMAT_LEGACY /* enable also legacy image format */

/* FDT support */
#define CONFIG_DISPLAY_BOARDINFO_LATE

/* Extend size of kernel image for uncompression */
#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

/* Boot FreeBSD/vxWorks from an ELF image */
#if defined(CONFIG_ZYNQ_BOOT_FREEBSD)
# define CONFIG_API
# define CONFIG_CMD_ELF
# define CONFIG_SYS_MMC_MAX_DEVICE	1
#endif

#define CONFIG_SYS_LDSCRIPT  "arch/arm/mach-zynq/u-boot.lds"

/* Commands */
#ifdef CONFIG_SYS_ENET
# define CONFIG_CMD_PING
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_MII
# define CONFIG_CMD_TFTPPUT
#else
# undef CONFIG_CMD_NET
# undef CONFIG_CMD_NFS
#endif

#if defined(CONFIG_CMD_ZYNQ_RSA)
# ifndef CONFIG_RSA
#  define CONFIG_RSA
# endif
#define CONFIG_SHA256
#define CONFIG_CMD_ZYNQ_AES
#endif

#define CONFIG_CMD_BOOTZ
#undef CONFIG_BOOTM_NETBSD

#define CONFIG_SYS_HZ			1000

/* For development/debugging */
#ifdef DEBUG
# define CONFIG_CMD_REGINFO
# define CONFIG_PANIC_HANG
#endif

/* SPL part */
#define CONFIG_CMD_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_BOARD_INIT

#define CONFIG_SPL_LDSCRIPT	"arch/arm/mach-zynq/u-boot-spl.lds"

/* FPGA support */
#define CONFIG_SPL_FPGA_SUPPORT
#define CONFIG_SPL_FPGA_LOAD_ADDR      0x1000000
/* #define CONFIG_SPL_FPGA_BIT */
#ifdef CONFIG_SPL_FPGA_BIT
# define CONFIG_SPL_FPGA_LOAD_ARGS_NAME "download.bit"
#else
# define CONFIG_SPL_FPGA_LOAD_ARGS_NAME "fpga.bin"
#endif

/* MMC support */
#ifdef CONFIG_ZYNQ_SDHCI0
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS      0x200 /* 256 KB */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION     1
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME     "u-boot.img"
#endif

/* Disable dcache for SPL just for sure */
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_DCACHE_OFF
#undef CONFIG_FPGA
#endif

/* Address in RAM where the parameters must be copied by SPL. */
#define CONFIG_SYS_SPL_ARGS_ADDR	0x10000000
#define CONFIG_SYS_SPI_ARGS_OFFS	0 /* FIXME */
#define CONFIG_SYS_SPI_ARGS_SIZE	0 /* FIXME */

#define CONFIG_SPL_FS_LOAD_ARGS_NAME		"system.dtb"
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME		"uImage"

/* Not using MMC raw mode - just for compilation purpose */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0

/* qspi mode is working fine */
#if 0
#ifdef CONFIG_ZYNQ_QSPI
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x100000
#endif
#endif

#ifdef DEBUG
#define CONFIG_SPL_RAM_DEVICE
#define CONFIG_SPL_NET_SUPPORT
#define CONFIG_SPL_ETH_SUPPORT
#define CONFIG_SPL_ENV_SUPPORT
#define CONFIG_SPL_ETH_DEVICE "Gem.e000b000"
#endif

/* for booting directly linux */
#define CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_SPI_KERNEL_OFFS	0 /* FIXME */

/* SP location before relocation, must use scratch RAM */
#define CONFIG_SPL_TEXT_BASE	0x0

/* 3 * 64kB blocks of OCM - one is on the top because of bootrom */
#define CONFIG_SPL_MAX_FOOTPRINT	0x30000
#define CONFIG_SPL_MAX_SIZE	0x30000

/* The highest 64k OCM address */
#define OCM_HIGH_ADDR	0xffff0000

/* Just define any reasonable size */
#define CONFIG_SPL_STACK_SIZE	0x1000

/* SPL stack position - and stack goes down */
#define CONFIG_SPL_STACK	(OCM_HIGH_ADDR + CONFIG_SPL_STACK_SIZE)

/* On the top of OCM space */
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_STACK + \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x1000

/* BSS setup */
#define CONFIG_SPL_BSS_START_ADDR	0x100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x100000

#define CONFIG_SYS_UBOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_GENERIC_BOARD

#endif /* __CONFIG_ZYNQ_COMMON_H */
