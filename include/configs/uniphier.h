/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* U-Boot - Common settings for UniPhier Family */

#ifndef __CONFIG_UNIPHIER_COMMON_H__
#define __CONFIG_UNIPHIER_COMMON_H__

#define CONFIG_I2C_EEPROM
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS  10

#define CONFIG_SMC911X

/* dummy: referenced by examples/standalone/smc911x_eeprom.c */
#define CONFIG_SMC911X_BASE	0
#define CONFIG_SMC911X_32_BIT

/*-----------------------------------------------------------------------
 * MMU and Cache Setting
 *----------------------------------------------------------------------*/

/* Comment out the following to enable L1 cache */
/* #define CONFIG_SYS_ICACHE_OFF */
/* #define CONFIG_SYS_DCACHE_OFF */

#define CONFIG_SYS_CACHELINE_SIZE	32

/* Comment out the following to disable L2 cache */
#define CONFIG_UNIPHIER_L2CACHE_ON

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_MISC_INIT_F
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

#define CONFIG_TIMESTAMP

/* FLASH related */
#define CONFIG_MTD_DEVICE

/*
 * uncomment the following to disable FLASH related code.
 */
/* #define CONFIG_SYS_NO_FLASH */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI

#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_MONITOR_BASE		0
#define CONFIG_SYS_MONITOR_LEN		0x00080000	/* 512KB */
#define CONFIG_SYS_FLASH_BASE		0

/*
 * flash_toggle does not work for out supoort card.
 * We need to use flash_status_poll.
 */
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL

#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT 1

/* serial console configuration */
#define CONFIG_BAUDRATE			115200

#if !defined(CONFIG_SPL_BUILD) && !defined(CONFIG_ARM64)
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
#endif

#define CONFIG_SYS_LONGHELP		/* undef to save memory */

#define CONFIG_CMDLINE_EDITING		/* add command line history	*/
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_CONS_INDEX		1

/* #define CONFIG_ENV_IS_NOWHERE */
/* #define CONFIG_ENV_IS_IN_NAND */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET			0x80000
#define CONFIG_ENV_SIZE				0x2000
/* #define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE) */

#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		1

#ifdef CONFIG_ARM64
#define CPU_RELEASE_ADDR			0x80000000
#define COUNTER_FREQUENCY			50000000
#define CONFIG_GICV3
#define GICD_BASE				0x5fe00000
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
#define GICR_BASE				0x5fe40000
#elif defined(CONFIG_ARCH_UNIPHIER_LD20)
#define GICR_BASE				0x5fe80000
#endif
#else
/* Time clock 1MHz */
#define CONFIG_SYS_TIMER_RATE			1000000
#endif


#define CONFIG_SYS_MAX_NAND_DEVICE			1
#define CONFIG_SYS_NAND_MAX_CHIPS			2
#define CONFIG_SYS_NAND_ONFI_DETECTION

#define CONFIG_NAND_DENALI_ECC_SIZE			1024

#ifdef CONFIG_ARCH_UNIPHIER_SLD3
#define CONFIG_SYS_NAND_REGS_BASE			0xf8100000
#define CONFIG_SYS_NAND_DATA_BASE			0xf8000000
#else
#define CONFIG_SYS_NAND_REGS_BASE			0x68100000
#define CONFIG_SYS_NAND_DATA_BASE			0x68000000
#endif

#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_NAND_DATA_BASE + 0x10)

#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_BAD_BLOCK_POS			0

/* USB */
#define CONFIG_USB_MAX_CONTROLLER_COUNT		2
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS	4
#define CONFIG_FAT_WRITE
#define CONFIG_DOS_PARTITION

/* SD/MMC */
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_GENERIC_MMC

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x01000000)

/*
 * Network Configuration
 */
#define CONFIG_SERVERIP			192.168.11.1
#define CONFIG_IPADDR			192.168.11.10
#define CONFIG_GATEWAYIP		192.168.11.1
#define CONFIG_NETMASK			255.255.255.0

#define CONFIG_LOADADDR			0x84000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING		/* add command line history	*/

#define CONFIG_BOOTCOMMAND		"run $bootmode"

#define CONFIG_ROOTPATH			"/nfs/root/path"
#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs $bootargs root=/dev/nfs rw "			\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off;" \
		"run __nfsboot"

#ifdef CONFIG_FIT
#define CONFIG_BOOTFILE			"fitImage"
#define LINUXBOOT_ENV_SETTINGS \
	"fit_addr=0x00100000\0" \
	"fit_addr_r=0x84100000\0" \
	"fit_size=0x00f00000\0" \
	"norboot=setexpr fit_addr $nor_base + $fit_addr &&" \
		"bootm $fit_addr\0" \
	"nandboot=nand read $fit_addr_r $fit_addr $fit_size &&" \
		"bootm $fit_addr_r\0" \
	"tftpboot=tftpboot $fit_addr_r $bootfile &&" \
		"bootm $fit_addr_r\0" \
	"__nfsboot=run tftpboot\0"
#else
#ifdef CONFIG_ARM64
#define CONFIG_CMD_BOOTI
#define CONFIG_BOOTFILE			"Image"
#define LINUXBOOT_CMD			"booti"
#define KERNEL_ADDR_R			"kernel_addr_r=0x80080000\0"
#define KERNEL_SIZE			"kernel_size=0x00c00000\0"
#define RAMDISK_ADDR			"ramdisk_addr=0x00e00000\0"
#else
#define CONFIG_BOOTFILE			"zImage"
#define LINUXBOOT_CMD			"bootz"
#define KERNEL_ADDR_R			"kernel_addr_r=0x80208000\0"
#define KERNEL_SIZE			"kernel_size=0x00800000\0"
#define RAMDISK_ADDR			"ramdisk_addr=0x00a00000\0"
#endif
#define LINUXBOOT_ENV_SETTINGS \
	"fdt_addr=0x00100000\0" \
	"fdt_addr_r=0x84100000\0" \
	"fdt_size=0x00008000\0" \
	"kernel_addr=0x00200000\0" \
	KERNEL_ADDR_R \
	KERNEL_SIZE \
	RAMDISK_ADDR \
	"ramdisk_addr_r=0x84a00000\0" \
	"ramdisk_size=0x00600000\0" \
	"ramdisk_file=rootfs.cpio.uboot\0" \
	"boot_common=setexpr bootm_low $kernel_addr_r '&' fe000000 &&" \
		LINUXBOOT_CMD " $kernel_addr_r $ramdisk_addr_r $fdt_addr_r\0" \
	"norboot=setexpr kernel_addr $nor_base + $kernel_addr &&" \
		"setexpr kernel_size $kernel_size / 4 &&" \
		"cp $kernel_addr $kernel_addr_r $kernel_size &&" \
		"setexpr ramdisk_addr_r $nor_base + $ramdisk_addr &&" \
		"setexpr fdt_addr_r $nor_base + $fdt_addr &&" \
		"run boot_common\0" \
	"nandboot=nand read $kernel_addr_r $kernel_addr $kernel_size &&" \
		"nand read $ramdisk_addr_r $ramdisk_addr $ramdisk_size &&" \
		"nand read $fdt_addr_r $fdt_addr $fdt_size &&" \
		"run boot_common\0" \
	"tftpboot=tftpboot $kernel_addr_r $bootfile &&" \
		"tftpboot $ramdisk_addr_r $ramdisk_file &&" \
		"tftpboot $fdt_addr_r $fdt_file &&" \
		"run boot_common\0" \
	"__nfsboot=tftpboot $kernel_addr_r $bootfile &&" \
		"tftpboot $fdt_addr_r $fdt_file &&" \
		"tftpboot $fdt_addr_r $fdt_file &&" \
		"setenv ramdisk_addr_r - &&" \
		"run boot_common\0"
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"netdev=eth0\0"						\
	"verify=n\0"						\
	"nor_base=0x42000000\0"					\
	"sramupdate=setexpr tmp_addr $nor_base + 0x50000 &&"	\
		"tftpboot $tmp_addr u-boot-spl.bin &&"		\
		"setexpr tmp_addr $nor_base + 0x60000 &&"	\
		"tftpboot $tmp_addr u-boot.bin\0"		\
	"emmcupdate=mmcsetn &&"					\
		"mmc partconf $mmc_first_dev 0 1 1 &&"		\
		"tftpboot u-boot-spl.bin &&"			\
		"mmc write $loadaddr 0 80 &&"			\
		"tftpboot u-boot.bin &&"			\
		"mmc write $loadaddr 80 780\0"			\
	"nandupdate=nand erase 0 0x00100000 &&"			\
		"tftpboot u-boot-spl.bin &&"			\
		"nand write $loadaddr 0 0x00010000 &&"		\
		"tftpboot u-boot.bin &&"			\
		"nand write $loadaddr 0x00010000 0x000f0000\0"	\
	LINUXBOOT_ENV_SETTINGS

#define CONFIG_SYS_BOOTMAPSZ			0x20000000

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_NR_DRAM_BANKS		2
/* for LD20; the last 64 byte is used for dynamic DDR PHY training */
#define CONFIG_SYS_MEM_TOP_HIDE		64

#if defined(CONFIG_ARM64)
#define CONFIG_SPL_TEXT_BASE		0x30000000
#elif defined(CONFIG_ARCH_UNIPHIER_SLD3) || \
	defined(CONFIG_ARCH_UNIPHIER_LD4) || \
	defined(CONFIG_ARCH_UNIPHIER_SLD8)
#define CONFIG_SPL_TEXT_BASE		0x00040000
#else
#define CONFIG_SPL_TEXT_BASE		0x00100000
#endif

#if defined(CONFIG_ARCH_UNIPHIER_LD11)
#define CONFIG_SPL_STACK		(0x30014c00)
#elif defined(CONFIG_ARCH_UNIPHIER_LD20)
#define CONFIG_SPL_STACK		(0x3001c000)
#else
#define CONFIG_SPL_STACK		(0x00100000)
#endif
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE)

#define CONFIG_PANIC_HANG

#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_NOR_SUPPORT
#ifdef CONFIG_ARM64
#define CONFIG_SPL_BOARD_LOAD_IMAGE
#else
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#endif

#define CONFIG_SPL_LIBCOMMON_SUPPORT	/* for mem_malloc_init */
#define CONFIG_SPL_LIBGENERIC_SUPPORT

#define CONFIG_SPL_BOARD_INIT

#define CONFIG_SYS_NAND_U_BOOT_OFFS		0x10000

/* subtract sizeof(struct image_header) */
#define CONFIG_SYS_UBOOT_BASE			(0x60000 - 0x40)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x80

#define CONFIG_SPL_TARGET			"u-boot-with-spl.bin"
#define CONFIG_SPL_MAX_FOOTPRINT		0x10000
#define CONFIG_SPL_MAX_SIZE			0x10000
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
#define CONFIG_SPL_BSS_START_ADDR		0x30012000
#elif defined(CONFIG_ARCH_UNIPHIER_LD20)
#define CONFIG_SPL_BSS_START_ADDR		0x30016000
#endif
#define CONFIG_SPL_BSS_MAX_SIZE			0x2000

#endif /* __CONFIG_UNIPHIER_COMMON_H__ */
