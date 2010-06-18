/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * (C) Copyright 2009
 * Frederik Kriewitz <frederik@kriewitz.eu>
 *
 * Configuration settings for the DevKit8000 board.
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

/* High Level Configuration Options */
#define CONFIG_ARMV7		1	/* This is an ARM V7 CPU core */
#define CONFIG_OMAP		1	/* in a TI OMAP core */
#define CONFIG_OMAP34XX		1	/* which is a 34XX */
#define CONFIG_OMAP3430		1	/* which is in a 3430 */
#define CONFIG_OMAP3_DEVKIT8000	1	/* working with DevKit8000 */

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap3.h>

/* Display CPU and Board information */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

/* Clock Defines */
#define V_OSCK				26000000	/* Clock output from T2 */
#define V_SCLK				(V_OSCK >> 1)

#undef CONFIG_USE_IRQ			/* no support for IRQs */
#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

/* Size of malloc() pool */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
						/* Sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10))
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* bytes reserved for */
						/* initial data */

/* Hardware drivers */

/* DDR - I use Micron DDR */
#define CONFIG_OMAP3_MICRON_DDR		1

/* DM9000 */
#define CONFIG_NET_MULTI		1
#define CONFIG_NET_RETRY_COUNT		20
#define	CONFIG_DRIVER_DM9000		1
#define	CONFIG_DM9000_BASE		0x2c000000
#define	DM9000_IO			CONFIG_DM9000_BASE
#define	DM9000_DATA			(CONFIG_DM9000_BASE + 0x400)
#define	CONFIG_DM9000_USE_16BIT		1
#define CONFIG_DM9000_NO_SROM		1
#undef	CONFIG_DM9000_DEBUG

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		48000000 /* 48MHz (APLL96/2) */

/* select serial console configuration */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/* MMC */
#define CONFIG_MMC			1
#define CONFIG_OMAP3_MMC		1
#define CONFIG_DOS_PARTITION		1

/* I2C */
#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
#define CONFIG_DRIVER_OMAP34XX_I2C	1

/* TWL4030 */
#define CONFIG_TWL4030_POWER		1
#define CONFIG_TWL4030_LED		1

/* Board NAND Info */
#define CONFIG_SYS_NO_FLASH		/* no NOR flash */
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:" \
						"512k(x-loader)," \
						"1920k(u-boot)," \
						"128k(u-boot-env)," \
						"4m(kernel)," \
						"-(fs)"

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

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP			/* DHCP support			*/
#define CONFIG_CMD_EXT2			/* EXT2 Support			*/
#define CONFIG_CMD_FAT			/* FAT support			*/
#define CONFIG_CMD_I2C			/* I2C serial bus support	*/
#define CONFIG_CMD_JFFS2		/* JFFS2 Support		*/
#define CONFIG_CMD_MMC			/* MMC support			*/
#define CONFIG_CMD_MTDPARTS		/* Enable MTD parts commands	*/
#define CONFIG_CMD_NAND			/* NAND support			*/
#define CONFIG_CMD_NAND_LOCK_UNLOCK	/* nand (un)lock commands	*/

#undef CONFIG_CMD_FPGA			/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI			/* iminfo			*/

/* BOOTP/DHCP options */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_NISDOMAIN
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_NTPSERVER
#define CONFIG_BOOTP_TIMEOFFSET
#undef CONFIG_BOOTP_VENDOREX

/* Environment information */
#define CONFIG_ENV_OVERWRITE /* allow to overwrite serial and ethaddr */

#define CONFIG_BOOTDELAY		3

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyS2,115200n8\0" \
	"vram=12M\0" \
	"dvimode=1024x768MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"nfsopts=hard,tcp,rsize=65536,wsize=65536\0" \
	"kernelopts=rw\0" \
	"commonargs=" \
		"setenv bootargs console=${console} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay}\0" \
	"mmcargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk0p2 " \
		"${kernelopts}\0" \
	"nandargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=/dev/mtdblock4 " \
		"rootfstype=jffs2 " \
		"${kernelopts}\0" \
	"netargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off " \
		"${kernelopts} " \
		"dnsip1=${dnsip} " \
		"dnsip2=${dnsip2}\0" \
	"loadbootscript=fatload mmc 0 ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc 0 ${loadaddr} uImage\0" \
	"eraseenv=nand unlock 0x260000 0x20000; nand erase 0x260000 0x20000\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \
	"netboot=echo Booting from network ...; " \
		"dhcp ${loadaddr}; " \
		"run netargs; " \
		"bootm ${loadaddr}\0" \
	"autoboot=if mmc init 0; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"if run loaduimage; then " \
					"run mmcboot; " \
				"else run nandboot; " \
				"fi; " \
			"fi; " \
		"else run nandboot; fi\0"


#define CONFIG_BOOTCOMMAND "run autoboot"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_AUTO_COMPLETE		1
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"OMAP3 DevKit8000 # "
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		128	/* max number of command args */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0 + 0x07000000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					0x01000000) /* 16MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0 + 0x02000000)

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2 /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/* The stack sizes are set up in start.S using the settings below */
#define CONFIG_STACKSIZE	(128 << 10)	/* regular stack 128 KiB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ		(4 << 10)	/* IRQ stack 4 KiB */
#define CONFIG_STACKSIZE_FIQ		(4 << 10)	/* FIQ stack 4 KiB */
#endif

/*  Physical Memory Map  */
#define CONFIG_NR_DRAM_BANKS		2 /* CS1 may or may not be populated */
#define PHYS_SDRAM_1			OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE		(128 << 20)	/* at least 128 MiB */
#define PHYS_SDRAM_2			OMAP34XX_SDRC_CS1

/* SDRAM Bank Allocation method */
#define SDRC_R_B_C			1

/* NAND and environment organization  */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_ENV_IS_IN_NAND		1
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_ENV_OFFSET		boot_flash_off

#ifndef __ASSEMBLY__
extern unsigned int boot_flash_base;
extern volatile unsigned int boot_flash_env_addr;
extern unsigned int boot_flash_off;
extern unsigned int boot_flash_sec;
extern unsigned int boot_flash_type;
#endif

#endif /* __CONFIG_H */
