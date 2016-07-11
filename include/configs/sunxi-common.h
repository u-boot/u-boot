/*
 * (C) Copyright 2012-2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Configuration settings for the Allwinner sunxi series of boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_COMMON_CONFIG_H
#define _SUNXI_COMMON_CONFIG_H

#include <asm/arch/cpu.h>
#include <linux/stringify.h>

#ifdef CONFIG_OLD_SUNXI_KERNEL_COMPAT
/*
 * The U-Boot workarounds bugs in the outdated buggy sunxi-3.4 kernels at the
 * expense of restricting some features, so the regular machine id values can
 * be used.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	0
#else
/*
 * A compatibility guard to prevent loading outdated buggy sunxi-3.4 kernels.
 * Only sunxi-3.4 kernels with appropriate fixes applied are able to pass
 * beyond the machine id check.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	1
#endif

/*
 * High Level Configuration Options
 */
#define CONFIG_SUNXI		/* sunxi family */
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_THUMB_BUILD	/* Thumbs mode to save space in SPL */
#endif

/* Serial & console */
#define CONFIG_SYS_NS16550_SERIAL
/* ns16550 reg in the low bits of cpu reg */
#define CONFIG_SYS_NS16550_CLK		24000000
#ifndef CONFIG_DM_SERIAL
# define CONFIG_SYS_NS16550_REG_SIZE	-4
# define CONFIG_SYS_NS16550_COM1		SUNXI_UART0_BASE
# define CONFIG_SYS_NS16550_COM2		SUNXI_UART1_BASE
# define CONFIG_SYS_NS16550_COM3		SUNXI_UART2_BASE
# define CONFIG_SYS_NS16550_COM4		SUNXI_UART3_BASE
# define CONFIG_SYS_NS16550_COM5		SUNXI_R_UART_BASE
#endif

/* CPU */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_SYS_CACHELINE_SIZE	64
#define CONFIG_TIMER_CLK_FREQ		24000000

/*
 * The DRAM Base differs between some models. We cannot use macros for the
 * CONFIG_FOO defines which contain the DRAM base address since they end
 * up unexpanded in include/autoconf.mk .
 *
 * So we have to have this #ifdef #else #endif block for these.
 */
#ifdef CONFIG_MACH_SUN9I
#define SDRAM_OFFSET(x) 0x2##x
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* default load address */
#define CONFIG_SYS_TEXT_BASE		0x2a000000
#define CONFIG_PRE_CON_BUF_ADDR		0x2f000000
/* Note SPL_STACK_R_ADDR is set through Kconfig, we include it here 
 * since it needs to fit in with the other values. By also #defining it
 * we get warnings if the Kconfig value mismatches. */
#define CONFIG_SPL_STACK_R_ADDR		0x2fe00000
#define CONFIG_SPL_BSS_START_ADDR	0x2ff80000
#else
#define SDRAM_OFFSET(x) 0x4##x
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_LOAD_ADDR		0x42000000 /* default load address */
#define CONFIG_SYS_TEXT_BASE		0x4a000000
#define CONFIG_PRE_CON_BUF_ADDR		0x4f000000
/* Note SPL_STACK_R_ADDR is set through Kconfig, we include it here 
 * since it needs to fit in with the other values. By also #defining it
 * we get warnings if the Kconfig value mismatches. */
#define CONFIG_SPL_STACK_R_ADDR		0x4fe00000
#define CONFIG_SPL_BSS_START_ADDR	0x4ff80000
#endif

#define CONFIG_SPL_BSS_MAX_SIZE		0x00080000 /* 512 KiB */

#if defined(CONFIG_MACH_SUN9I) || defined(CONFIG_MACH_SUN50I)
/*
 * The A80's A1 sram starts at 0x00010000 rather then at 0x00000000 and is
 * slightly bigger. Note that it is possible to map the first 32 KiB of the
 * A1 at 0x00000000 like with older SoCs by writing 0x16aa0001 to the
 * undocumented 0x008000e0 SYS_CTRL register. Where the 16aa is a key and
 * the 1 actually activates the mapping of the first 32 KiB to 0x00000000.
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x10000
#define CONFIG_SYS_INIT_RAM_SIZE	0x08000	/* FIXME: 40 KiB ? */
#else
#define CONFIG_SYS_INIT_RAM_ADDR	0x0
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000	/* 32 KiB */
#endif

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_0			CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_0_SIZE		0x80000000 /* 2 GiB */

#ifdef CONFIG_AHCI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SUNXI_AHCI
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#define CONFIG_CMD_SCSI
#endif

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_SERIAL_TAG

#ifdef CONFIG_NAND_SUNXI
#define CONFIG_SPL_NAND_SUPPORT 1
#endif

/* mmc config */
#ifdef CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SUNXI
#define CONFIG_MMC_SUNXI_SLOT		0
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0	/* first detected MMC controller */
#endif

/* 64MB of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (64 << 20))

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	1024	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* standalone support */
#define CONFIG_STANDALONE_LOAD_ADDR	CONFIG_SYS_LOAD_ADDR

/* baudrate */
#define CONFIG_BAUDRATE			115200

/* The stack sizes are set up in start.S using the settings below */
#define CONFIG_STACKSIZE		(256 << 10)	/* 256 KiB */

/* FLASH and environment organization */

#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_MONITOR_LEN		(768 << 10)	/* 768 KiB */
#define CONFIG_IDENT_STRING		" Allwinner Technology"
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_ENV_OFFSET		(544 << 10) /* (8 + 24 + 512) KiB */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */

#define CONFIG_FAT_WRITE	/* enable write access */

#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT

#define CONFIG_SPL_BOARD_LOAD_IMAGE

#if defined(CONFIG_MACH_SUN9I)
#define CONFIG_SPL_TEXT_BASE		0x10020		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x5fe0		/* ? KiB on sun9i */
#elif defined(CONFIG_MACH_SUN50I)
#define CONFIG_SPL_TEXT_BASE		0x10020		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x7fe0		/* 32 KiB on sun50i */
#else
#define CONFIG_SPL_TEXT_BASE		0x20		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x5fe0		/* 24KB on sun4i/sun7i */
#endif

#define CONFIG_SPL_LIBDISK_SUPPORT

#ifdef CONFIG_MMC
#define CONFIG_SPL_MMC_SUPPORT
#endif

#ifndef CONFIG_ARM64
#define CONFIG_SPL_LDSCRIPT "arch/arm/cpu/armv7/sunxi/u-boot-spl.lds"
#endif

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	80	/* 40KiB */
#define CONFIG_SPL_PAD_TO		32768		/* decimal for 'dd' */

#if defined(CONFIG_MACH_SUN9I) || defined(CONFIG_MACH_SUN50I)
/* FIXME: 40 KiB instead of 32 KiB ? */
#define LOW_LEVEL_SRAM_STACK		0x00018000
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK
#else
/* end of 32 KiB in sram */
#define LOW_LEVEL_SRAM_STACK		0x00008000 /* End of sram */
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK
#endif

/* I2C */
#if defined CONFIG_AXP152_POWER || defined CONFIG_AXP209_POWER || \
    defined CONFIG_SY8106A_POWER
#define CONFIG_SPL_I2C_SUPPORT
#endif

#if defined CONFIG_I2C0_ENABLE || defined CONFIG_I2C1_ENABLE || \
    defined CONFIG_I2C2_ENABLE || defined CONFIG_I2C3_ENABLE || \
    defined CONFIG_I2C4_ENABLE || defined CONFIG_R_I2C_ENABLE
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MVTWSI
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_SYS_I2C_SLAVE		0x7f
#endif

#if defined CONFIG_VIDEO_LCD_PANEL_I2C && !(defined CONFIG_SPL_BUILD)
#define CONFIG_SYS_I2C_SOFT
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x00
/* We use pin names in Kconfig and sunxi_name_to_gpio() */
#define CONFIG_SOFT_I2C_GPIO_SDA	soft_i2c_gpio_sda
#define CONFIG_SOFT_I2C_GPIO_SCL	soft_i2c_gpio_scl
#ifndef __ASSEMBLY__
extern int soft_i2c_gpio_sda;
extern int soft_i2c_gpio_scl;
#endif
#define CONFIG_VIDEO_LCD_I2C_BUS	0 /* The lcd panel soft i2c is bus 0 */
#define CONFIG_SYS_SPD_BUS_NUM		1 /* And the axp209 i2c bus is bus 1 */
#else
#define CONFIG_SYS_SPD_BUS_NUM		0 /* The axp209 i2c bus is bus 0 */
#define CONFIG_VIDEO_LCD_I2C_BUS	-1 /* NA, but necessary to compile */
#endif

/* PMU */
#if defined CONFIG_AXP152_POWER || defined CONFIG_AXP209_POWER || \
    defined CONFIG_AXP221_POWER || defined CONFIG_AXP818_POWER || \
    defined CONFIG_SY8106A_POWER
#define CONFIG_SPL_POWER_SUPPORT
#endif

#ifndef CONFIG_CONS_INDEX
#define CONFIG_CONS_INDEX              1       /* UART0 */
#endif

#ifdef CONFIG_REQUIRE_SERIAL_CONSOLE
#if CONFIG_CONS_INDEX == 1
#ifdef CONFIG_MACH_SUN9I
#define OF_STDOUT_PATH		"/soc/serial@07000000:115200"
#else
#define OF_STDOUT_PATH		"/soc@01c00000/serial@01c28000:115200"
#endif
#elif CONFIG_CONS_INDEX == 2 && defined(CONFIG_MACH_SUN5I)
#define OF_STDOUT_PATH		"/soc@01c00000/serial@01c28400:115200"
#elif CONFIG_CONS_INDEX == 3 && defined(CONFIG_MACH_SUN8I)
#define OF_STDOUT_PATH		"/soc@01c00000/serial@01c28800:115200"
#elif CONFIG_CONS_INDEX == 5 && defined(CONFIG_MACH_SUN8I)
#define OF_STDOUT_PATH		"/soc@01c00000/serial@01f02800:115200"
#else
#error Unsupported console port nr. Please fix stdout-path in sunxi-common.h.
#endif
#endif /* ifdef CONFIG_REQUIRE_SERIAL_CONSOLE */

/* GPIO */
#define CONFIG_SUNXI_GPIO
#define CONFIG_SPL_GPIO_SUPPORT

#ifdef CONFIG_VIDEO
/*
 * The amount of RAM to keep free at the top of RAM when relocating u-boot,
 * to use as framebuffer. This must be a multiple of 4096.
 */
#define CONFIG_SUNXI_MAX_FB_SIZE (16 << 20)

/* Do we want to initialize a simple FB? */
#define CONFIG_VIDEO_DT_SIMPLEFB

#define CONFIG_VIDEO_SUNXI

#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_STD_TIMINGS
#define CONFIG_I2C_EDID
#define VIDEO_LINE_LEN (pGD->plnSizeX)

/* allow both serial and cfb console. */
#define CONFIG_CONSOLE_MUX
/* stop x86 thinking in cfbconsole from trying to init a pc keyboard */
#define CONFIG_VGA_AS_SINGLE_DEVICE

#endif /* CONFIG_VIDEO */

/* Ethernet support */
#ifdef CONFIG_SUNXI_EMAC
#define CONFIG_PHY_ADDR		1
#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_PHYLIB
#endif

#ifdef CONFIG_SUNXI_GMAC
#define CONFIG_PHY_GIGE			/* GMAC can use gigabit PHY	*/
#define CONFIG_PHY_ADDR		1
#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_PHY_REALTEK
#endif

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_OHCI_SUNXI
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS 1
#endif

#ifdef CONFIG_USB_MUSB_SUNXI
#define CONFIG_USB_MUSB_PIO_ONLY
#endif

#ifdef CONFIG_USB_MUSB_GADGET
#define CONFIG_USB_FUNCTION_DFU
#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#endif

#ifdef CONFIG_USB_FUNCTION_DFU
#define CONFIG_DFU_RAM
#endif

#ifdef CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_CMD_FASTBOOT
#define CONFIG_FASTBOOT_BUF_ADDR	CONFIG_SYS_LOAD_ADDR
#define CONFIG_FASTBOOT_BUF_SIZE	0x2000000
#define CONFIG_ANDROID_BOOT_IMAGE

#define CONFIG_FASTBOOT_FLASH

#ifdef CONFIG_MMC
#define CONFIG_FASTBOOT_FLASH_MMC_DEV	0
#define CONFIG_EFI_PARTITION
#endif
#endif

#ifdef CONFIG_USB_FUNCTION_MASS_STORAGE
#endif

#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_CONSOLE_MUX
#define CONFIG_PREBOOT
#define CONFIG_SYS_STDIO_DEREGISTER
#define CONFIG_SYS_USB_EVENT_POLL_VIA_INT_QUEUE
#endif

#if !defined CONFIG_ENV_IS_IN_MMC && \
    !defined CONFIG_ENV_IS_IN_NAND && \
    !defined CONFIG_ENV_IS_IN_FAT && \
    !defined CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_NOWHERE
#endif

#define CONFIG_MISC_INIT_R
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#ifndef CONFIG_SPL_BUILD
#include <config_distro_defaults.h>

/* Enable pre-console buffer to get complete log on the VGA console */
#define CONFIG_PRE_CONSOLE_BUFFER
#define CONFIG_PRE_CON_BUF_SZ		4096 /* Aprox 2 80*25 screens */

/*
 * 160M RAM (256M minimum minus 64MB heap + 32MB for u-boot, stack, fb, etc.
 * 32M uncompressed kernel, 16M compressed kernel, 1M fdt,
 * 1M script, 1M pxe and the ramdisk at the end.
 */

#define KERNEL_ADDR_R  __stringify(SDRAM_OFFSET(2000000))
#define FDT_ADDR_R     __stringify(SDRAM_OFFSET(3000000))
#define SCRIPT_ADDR_R  __stringify(SDRAM_OFFSET(3100000))
#define PXEFILE_ADDR_R __stringify(SDRAM_OFFSET(3200000))
#define RAMDISK_ADDR_R __stringify(SDRAM_OFFSET(3300000))

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0xa000000\0" \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0"

#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram " KERNEL_ADDR_R " 0x1000000;" \
	"fdt ram " FDT_ADDR_R " 0x100000;" \
	"ramdisk ram " RAMDISK_ADDR_R " 0x4000000\0"

#ifdef CONFIG_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#if CONFIG_MMC_SUNXI_SLOT_EXTRA != -1
#define BOOT_TARGET_DEVICES_MMC_EXTRA(func) func(MMC, mmc, 1)
#else
#define BOOT_TARGET_DEVICES_MMC_EXTRA(func)
#endif
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#define BOOT_TARGET_DEVICES_MMC_EXTRA(func)
#endif

#ifdef CONFIG_AHCI
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

/* FEL boot support, auto-execute boot.scr if a script address was provided */
#define BOOTENV_DEV_FEL(devtypeu, devtypel, instance) \
	"bootcmd_fel=" \
		"if test -n ${fel_booted} && test -n ${fel_scriptaddr}; then " \
			"echo '(FEL boot)'; " \
			"source ${fel_scriptaddr}; " \
		"fi\0"
#define BOOTENV_DEV_NAME_FEL(devtypeu, devtypel, instance) \
	"fel "

#define BOOT_TARGET_DEVICES(func) \
	func(FEL, fel, na) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_MMC_EXTRA(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#ifdef CONFIG_OLD_SUNXI_KERNEL_COMPAT
#define BOOTCMD_SUNXI_COMPAT \
	"bootcmd_sunxi_compat=" \
		"setenv root /dev/mmcblk0p3 rootwait; " \
		"if ext2load mmc 0 0x44000000 uEnv.txt; then " \
			"echo Loaded environment from uEnv.txt; " \
			"env import -t 0x44000000 ${filesize}; " \
		"fi; " \
		"setenv bootargs console=${console} root=${root} ${extraargs}; " \
		"ext2load mmc 0 0x43000000 script.bin && " \
		"ext2load mmc 0 0x48000000 uImage && " \
		"bootm 0x48000000\0"
#else
#define BOOTCMD_SUNXI_COMPAT
#endif

#include <config_distro_bootcmd.h>

#ifdef CONFIG_USB_KEYBOARD
#define CONSOLE_STDIN_SETTINGS \
	"preboot=usb start\0" \
	"stdin=serial,usbkbd\0"
#else
#define CONSOLE_STDIN_SETTINGS \
	"stdin=serial\0"
#endif

#ifdef CONFIG_VIDEO
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial,vga\0" \
	"stderr=serial,vga\0"
#else
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial\0" \
	"stderr=serial\0"
#endif

#define CONSOLE_ENV_SETTINGS \
	CONSOLE_STDIN_SETTINGS \
	CONSOLE_STDOUT_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONSOLE_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	DFU_ALT_INFO_RAM \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"console=ttyS0,115200\0" \
	BOOTCMD_SUNXI_COMPAT \
	BOOTENV

#else /* ifndef CONFIG_SPL_BUILD */
#define CONFIG_EXTRA_ENV_SETTINGS
#endif

#endif /* _SUNXI_COMMON_CONFIG_H */
