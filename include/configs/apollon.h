/*
 * (C) Copyright 2005-2008
 * Samsung Electronics,
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Configuration settings for the 2420 Samsung Apollon board.
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
#define CONFIG_ARM1136		1 /* This is an arm1136 CPU core */
#define CONFIG_OMAP		1 /* in a TI OMAP core */
#define CONFIG_OMAP2420		1 /* which is in a 2420 */
#define CONFIG_OMAP2420_APOLLON	1
#define CONFIG_APOLLON		1
#define CONFIG_APOLLON_PLUS	1 /* If you have apollon plus 1.x */

/* Clock config to target*/
#define PRCM_CONFIG_I		1
/* #define PRCM_CONFIG_II	1 */

/* Boot method */
/* uncomment if you use NOR boot */
/* #define CONFIG_SYS_NOR_BOOT		1 */

/* uncomment if you use NOR on CS3 */
/* #define CONFIG_SYS_USE_NOR		1 */

#ifdef CONFIG_SYS_NOR_BOOT
#undef CONFIG_SYS_USE_NOR
#define CONFIG_SYS_USE_NOR		1
#endif

/* uncommnet if you want to use UBI */
#define CONFIG_SYS_USE_UBI

#include <asm/arch/omap2420.h>	/* get chip and board defs */

#define	V_SCLK	12000000

/* input clock of PLL */
/* the OMAP2420 H4 has 12MHz, 13MHz, or 19.2Mhz crystal input */
#define	CONFIG_SYS_CLK_FREQ	V_SCLK

#undef	CONFIG_USE_IRQ	/* no support for IRQs */
#define	CONFIG_MISC_INIT_R

#define	CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs */
#define	CONFIG_SETUP_MEMORY_TAGS	1
#define	CONFIG_INITRD_TAG	1
#define	CONFIG_REVISION_TAG	1

/*
 * Size of malloc() pool
 */
#define	CONFIG_ENV_SIZE SZ_128K	/* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE_FLEX SZ_256K
#define	CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + SZ_1M)
/* bytes reserved for initial data */

/*
 * Hardware drivers
 */

/*
 * SMC91c96 Etherent
 */
#define CONFIG_NET_MULTI
#define	CONFIG_LAN91C96
#define	CONFIG_LAN91C96_BASE	(APOLLON_CS1_BASE+0x300)
#define	CONFIG_LAN91C96_EXT_PHY

/*
 * NS16550 Configuration
 */
#define	V_NS16550_CLK	(48000000)	/* 48MHz (APLL96/2) */

#define	CONFIG_SYS_NS16550
#define	CONFIG_SYS_NS16550_SERIAL
#define	CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define	CONFIG_SYS_NS16550_CLK	V_NS16550_CLK	/* 3MHz (1.5MHz*2) */
#define	CONFIG_SYS_NS16550_COM1	OMAP2420_UART1

/*
 * select serial console configuration
 */
#define	CONFIG_SERIAL1	1	/* UART1 on H4 */

/* allow to overwrite serial and ethaddr */
#define	CONFIG_ENV_OVERWRITE
#define	CONFIG_CONS_INDEX	1
#define	CONFIG_BAUDRATE		115200
#define	CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include	<config_cmd_default.h>

#define	CONFIG_CMD_DHCP
#define	CONFIG_CMD_DIAG
#define	CONFIG_CMD_ONENAND

#ifdef CONFIG_SYS_USE_UBI
#define	CONFIG_CMD_JFFS2
#define	CONFIG_CMD_UBI
#define	CONFIG_RBTREE
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#endif

#undef	CONFIG_CMD_SOURCE

#ifndef	CONFIG_SYS_USE_NOR
# undef	CONFIG_CMD_FLASH
# undef	CONFIG_CMD_IMLS
#endif

#define	CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT

#define	CONFIG_BOOTDELAY	1

#define	CONFIG_NETMASK	255.255.255.0
#define	CONFIG_IPADDR	192.168.116.25
#define	CONFIG_SERVERIP	192.168.116.1
#define	CONFIG_BOOTFILE	"uImage"
#define	CONFIG_ETHADDR	00:0E:99:00:24:20

#ifdef CONFIG_APOLLON_PLUS
#define CONFIG_SYS_MEM	"mem=64M"
#else
#define CONFIG_SYS_MEM	"mem=128"
#endif

#ifdef CONFIG_SYS_USE_UBI
#define CONFIG_SYS_UBI "ubi.mtd=4"
#else
#define CONFIG_SYS_UBI ""
#endif

#define CONFIG_BOOTARGS "root=/dev/nfs rw " CONFIG_SYS_MEM \
	" console=ttyS0,115200n8" \
	" ip=192.168.116.25:192.168.116.1:192.168.116.1:255.255.255.0:" \
	"apollon:eth0:off nfsroot=/tftpboot/nfsroot profile=2 " \
	CONFIG_SYS_UBI

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"Image=tftp 0x80008000 Image; go 0x80008000\0"			\
	"zImage=tftp 0x80180000 zImage; go 0x80180000\0"		\
	"uImage=tftp 0x80180000 uImage; bootm 0x80180000\0"		\
	"uboot=tftp 0x80008000 u-boot.bin; go 0x80008000\0"		\
	"xloader=tftp 0x80180000 x-load.bin; "				\
	" cp.w 0x80180000 0x00000400 0x1000; go 0x00000400\0"		\
	"syncmode50=mw.w 0x1e442 0xc0c4; mw 0x6800a060 0xe30d1201\0"	\
	"syncmode=mw.w 0x1e442 0xe0f4; mw 0x6800a060 0xe30d1201\0"	\
	"norboot=cp32 0x18040000 0x80008000 0x200000; go 0x80008000\0"	\
	"oneboot=onenand read 0x80008000 0x40000 0x200000; go 0x80008000\0" \
	"onesyncboot=run syncmode oneboot\0"				\
	"updateb=tftp 0x80180000 u-boot-onenand.bin; "			\
	" onenand erase 0x0 0x20000; onenand write 0x80180000 0x0 0x20000\0" \
	"ubi=setenv bootargs ${bootargs} ubi.mtd=4 ${mtdparts}; run uImage\0" \
	"bootcmd=run uboot\0"

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP	/* undef to save memory */
#define	CONFIG_SYS_PROMPT	"Apollon # "
#define	CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define	CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define	CONFIG_SYS_MAXARGS	16	/* max number of command args */
/* Boot Argument Buffer Size */
#define	CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
/* memtest works on */
#define	CONFIG_SYS_MEMTEST_START	(OMAP2420_SDRC_CS0)
#define	CONFIG_SYS_MEMTEST_END		(OMAP2420_SDRC_CS0+SZ_31M)

/* default load address */
#define	CONFIG_SYS_LOAD_ADDR	(OMAP2420_SDRC_CS0)

/* The 2420 has 12 GP timers, they can be driven by the SysClk (12/13/19.2)
 * or by 32KHz clk, or from external sig. This rate is divided by a local
 * divisor.
 */
#define	CONFIG_SYS_TIMERBASE	OMAP2420_GPT2
#define	CONFIG_SYS_PTV		7	/* 2^(PTV+1) */
#define	CONFIG_SYS_HZ		((CONFIG_SYS_CLK_FREQ)/(2 << CONFIG_SYS_PTV))

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define	CONFIG_STACKSIZE SZ_128K	/* regular stack */
#ifdef	CONFIG_USE_IRQ
# define	CONFIG_STACKSIZE_IRQ SZ_4K	/* IRQ stack */
# define	CONFIG_STACKSIZE_FIQ SZ_4K	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define	CONFIG_NR_DRAM_BANKS	1	/* CS1 may or may not be populated */
#define	PHYS_SDRAM_1		OMAP2420_SDRC_CS0
#define	PHYS_SDRAM_1_SIZE	SZ_128M
#define	PHYS_SDRAM_2		OMAP2420_SDRC_CS1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#ifdef	CONFIG_SYS_USE_NOR
/* OneNAND boot, NOR has CS3, But NOR has CS0 when NOR boot */
# define	CONFIG_SYS_FLASH_BASE		0x18000000
# define	CONFIG_SYS_MAX_FLASH_BANKS	1
# define	CONFIG_SYS_MAX_FLASH_SECT	1024
/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
/* Flash memory is CFI compliant */
# define	CONFIG_SYS_FLASH_CFI	1
# define	CONFIG_FLASH_CFI_DRIVER	1	/* Use drivers/cfi_flash.c */
/* Use buffered writes (~10x faster) */
/* #define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1 */
/* Use h/w sector protection*/
# define	CONFIG_SYS_FLASH_PROTECTION	1

#else	/* !CONFIG_SYS_USE_NOR */
# define	CONFIG_SYS_NO_FLASH	1
#endif	/* CONFIG_SYS_USE_NOR */

/* OneNAND boot, OneNAND has CS0, NOR boot ONeNAND has CS2 */
#define	CONFIG_SYS_ONENAND_BASE	0x00000000
#define CONFIG_SYS_MONITOR_LEN		SZ_256K	/* U-Boot image size */
#define	CONFIG_ENV_IS_IN_ONENAND	1
#define CONFIG_ENV_ADDR		0x00020000
#define CONFIG_ENV_ADDR_FLEX	0x00040000

#ifdef CONFIG_SYS_USE_UBI
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"onenand0=onenand"
#define MTDPARTS_DEFAULT	"mtdparts=onenand:128k(bootloader),"	\
					"128k(params),"			\
					"2m(kernel),"			\
					"16m(rootfs),"			\
					"32m(fs),"			\
					"-(ubifs)"
#endif

#endif /* __CONFIG_H */
