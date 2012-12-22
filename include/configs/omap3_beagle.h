/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Configuration settings for the TI OMAP3530 Beagle board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP		1	/* in a TI OMAP core */
#define CONFIG_OMAP34XX		1	/* which is a 34XX */
#define CONFIG_OMAP3_BEAGLE	1	/* working with BEAGLE */
#define CONFIG_OMAP_GPIO

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap3.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_OF_LIBFDT		1

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
						/* Sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10))

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3	/* UART3 on Beagle Rev 2 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_OMAP_HSMMC		1
#define CONFIG_DOS_PARTITION		1

/* Status LED */
#define CONFIG_STATUS_LED		1
#define CONFIG_BOARD_SPECIFIC_LED	1
#define STATUS_LED_BIT			0x01
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT1			0x02
#define STATUS_LED_STATE1		STATUS_LED_ON
#define STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BOOT			STATUS_LED_BIT
#define STATUS_LED_GREEN		STATUS_LED_BIT1

/* Enable Multi Bus support for I2C */
#define CONFIG_I2C_MULTI_BUS		1

/* Probe all devices */
#define CONFIG_SYS_I2C_NOPROBES		{{0x0, 0x0}}

/* USB */
#define CONFIG_MUSB_GADGET
#define CONFIG_USB_MUSB_OMAP2PLUS
#define CONFIG_MUSB_PIO_ONLY
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_TWL4030_USB		1
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETHER_RNDIS

/* USB EHCI */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI

#define CONFIG_USB_EHCI_OMAP
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	147

#define CONFIG_USB_ULPI
#define CONFIG_USB_ULPI_VIEWPORT_OMAP

#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS 3
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_USB_ETHER_ASIX


/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_JFFS2	/* JFFS2 Support		*/
#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:512k(x-loader),"\
					"1920k(u-boot),128k(u-boot-env),"\
					"4m(kernel),-(fs)"

#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_USB_STORAGE	/* USB storage support		*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_LED		/* LED support			*/
#define CONFIG_CMD_NET      /* bootp, tftpboot, rarpboot    */
#define CONFIG_CMD_NFS      /* NFS support          */
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SETEXPR	/* Evaluate expressions		*/
#define CONFIG_CMD_GPIO     /* Enable gpio command */

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_SYS_NO_FLASH
#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
#define CONFIG_I2C_MULTI_BUS		1
#define CONFIG_DRIVER_OMAP34XX_I2C	1
#define CONFIG_VIDEO_OMAP3	/* DSS Support			*/

/*
 * TWL4030
 */
#define CONFIG_TWL4030_POWER		1
#define CONFIG_TWL4030_LED		1

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_QUIET_TEST	1
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */
#define GPMC_NAND_ECC_LP_x16_LAYOUT	1

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */
#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* size of jffs2 */
							/* partition */

/* Environment information */
#define CONFIG_BOOTDELAY		3

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80200000\0" \
	"rdaddr=0x81000000\0" \
	"usbtty=cdc_acm\0" \
	"bootfile=uImage.beagle\0" \
	"console=ttyO2,115200n8\0" \
	"mpurate=auto\0" \
	"buddy=none\0" \
	"optargs=\0" \
	"camera=none\0" \
	"vram=12M\0" \
	"dvimode=640x480MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"nandroot=ubi0:rootfs ubi.mtd=4\0" \
	"nandrootfstype=ubifs\0" \
	"ramroot=/dev/ram0 rw ramdisk_size=65536 initrd=0x81000000,64M\0" \
	"ramrootfstype=ext2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"mpurate=${mpurate} " \
		"buddy=${buddy} "\
		"camera=${camera} "\
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"mpurate=${mpurate} " \
		"buddy=${buddy} "\
		"camera=${camera} "\
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
	"ramargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"mpurate=${mpurate} " \
		"buddy=${buddy} "\
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${ramroot} " \
		"rootfstype=${ramrootfstype}\0" \
	"loadramdisk=fatload mmc ${mmcdev} ${rdaddr} ramdisk.gz\0" \
	"loaduimagefat=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"loaduimage=ext2load mmc ${mmcdev}:2 ${loadaddr} /boot/uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \
	"ramboot=echo Booting from ramdisk ...; " \
		"run ramargs; " \
		"bootm ${loadaddr}\0" \
	"userbutton=if gpio input 173; then run userbutton_xm; " \
		"else run userbutton_nonxm; fi;\0" \
	"userbutton_xm=gpio input 4;\0" \
	"userbutton_nonxm=gpio input 7;\0"
/* "run userbutton" will return 1 (false) if is pressed and 0 (false) if not */
#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run userbutton; then " \
			"setenv bootenv uEnv.txt;" \
		"else " \
			"setenv bootenv user.txt;" \
		"fi;" \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loaduimage; then " \
			"run mmcboot;" \
		"fi;" \
	"fi;" \
	"run nandboot;" \

#define CONFIG_AUTO_COMPLETE		1
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"OMAP3 beagleboard.org # "
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		32	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_ALT_MEMTEST		1
#define CONFIG_SYS_MEMTEST_START	(0x82000000)		/* memtest */
								/* defaults */
#define CONFIG_SYS_MEMTEST_END		(0x87FFFFFF) 		/* 128MB */
#define CONFIG_SYS_MEMTEST_SCRATCH	(0x81000000)	/* dummy address */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)	/* default */
							/* load address */

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M
#define PISMO1_ONEN_SIZE		GPMC_SIZE_128M

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#if defined(CONFIG_CMD_NAND)
#define CONFIG_SYS_FLASH_BASE		PISMO1_NAND_BASE
#endif

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

#define CONFIG_ENV_IS_IN_NAND		1
#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			SMNAND_ENV_OFFSET

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

#define CONFIG_OMAP3_SPI

#define CONFIG_SYS_CACHELINE_SIZE	64

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_TEXT_BASE		0x40200800
#define CONFIG_SPL_MAX_SIZE		(54 * 1024)	/* 8 KB for stack */
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_OMAP3_ID_NAND
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"

/* NAND boot config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

#endif /* __CONFIG_H */
