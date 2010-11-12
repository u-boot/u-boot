/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifndef __ASSEMBLY__
#include <galileo/core.h>
#endif

#include "../board/evb64260/local.h"

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_P3G4		1	/* this is a P3G4  board	*/
#define CONFIG_SYS_GT_6426x        GT_64260 /* with a 64260 system controller */

#define	CONFIG_SYS_TEXT_BASE	0xfff00000

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115200	*/

#undef	CONFIG_ECC			/* enable ECC support */
/* #define CONFIG_EVB64260_750CX  1 */      /* Support the EVB-64260-750CX Board */

/* which initialization functions to call for this board */
#define CONFIG_MISC_INIT_R	1
#define CONFIG_BOARD_EARLY_INIT_F 1

#define CONFIG_SYS_BOARD_NAME		"P3G4"

#undef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/*
 * The following defines let you select what serial you want to use
 * for your console driver.
 *
 * to use the MPSC, #define CONFIG_MPSC.  If you have wired up another
 * mpsc channel, change CONFIG_MPSC_PORT to the desired value.
 */
#define	CONFIG_MPSC
#define CONFIG_MPSC_PORT	0

#define CONFIG_NET_MULTI        /* attempt all available adapters */

/* define this if you want to enable GT MAC filtering */
#define CONFIG_GT_USE_MAC_HASH_TABLE

#undef CONFIG_ETHER_PORT_MII	/* use RMII */

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=p3g4\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
	        "bootm\0"						\
	"rootpath=/opt/eldk/ppc_74xx\0"					\
	"bootfile=/tftpboot/p3g4/uImage\0"				\
	"kernel_addr=ff000000\0"					\
	"ramdisk_addr=ff010000\0"					\
	"load=tftp 100000 /tftpboot/p3g4/u-boot.bin\0"			\
	"update=protect off fff00000 fff3ffff;era fff00000 fff3ffff;"	\
		"cp.b 100000 fff00000 ${filesize};"			\
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_LOADS_ECHO	0	/* echo off for serial download	*/
#define	CONFIG_SYS_LOADS_BAUD_CHANGE		/* allow baudrate changes	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#undef	CONFIG_ALTIVEC                  /* undef to disable             */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PCI
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_UNIVERSE
#define CONFIG_CMD_BSP


/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00C00000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x00300000	/* default load address	*/

#define	CONFIG_SYS_HZ			1000		/* decr freq: 1ms ticks	*/
#define CONFIG_SYS_BUS_CLK		133000000	/* 133 MHz		*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }


/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define	CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_RAM_LOCK


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xff000000
#define CONFIG_SYS_RESET_ADDRESS	0xfff00100
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */

/* areas to map different things with the GT in physical space */
#define CONFIG_SYS_DRAM_BANKS		1
#define CONFIG_SYS_DFL_GT_REGS		0x14000000	/* boot time GT_REGS */

/* What to put in the bats. */
#define CONFIG_SYS_MISC_REGION_BASE	0xf0000000

/* Peripheral Device section */
#define CONFIG_SYS_GT_REGS		0xf8000000
#define CONFIG_SYS_DEV_BASE		0xff000000

#define CONFIG_SYS_DEV0_SPACE		CONFIG_SYS_DEV_BASE
#define CONFIG_SYS_DEV1_SPACE		(CONFIG_SYS_DEV0_SPACE + CONFIG_SYS_DEV0_SIZE)
#define CONFIG_SYS_DEV2_SPACE		(CONFIG_SYS_DEV1_SPACE + CONFIG_SYS_DEV1_SIZE)
#define CONFIG_SYS_DEV3_SPACE		(CONFIG_SYS_DEV2_SPACE + CONFIG_SYS_DEV2_SIZE)

#define CONFIG_SYS_DEV0_SIZE		 _8M /* Flash bank */
#define CONFIG_SYS_DEV1_SIZE		 0   /* unused */
#define CONFIG_SYS_DEV2_SIZE		 0   /* unused */
#define CONFIG_SYS_DEV3_SIZE		 0   /* unused */

#define CONFIG_SYS_16BIT_BOOT_PAR	0xc01b5e7c
#define CONFIG_SYS_DEV0_PAR		CONFIG_SYS_16BIT_BOOT_PAR

#if 0 /* Wrong?? NTL */
#define CONFIG_SYS_MPP_CONTROL_0	0x53541717	/* InitAct EOT[4] DBurst TCEn[1] */
						/* DMAAck[1:0] GNT0[1:0] */
#else
#define CONFIG_SYS_MPP_CONTROL_0	0x53547777	/* InitAct EOT[4] DBurst TCEn[1] */
						/* REQ0[1:0] GNT0[1:0] */
#endif
#define CONFIG_SYS_MPP_CONTROL_1	0x44009911	/* TCEn[4] TCTcnt[4] GPP[13:12] */
						/* DMAReq[4] DMAAck[4] WDNMI WDE */
#if 0 /* Wrong?? NTL */
#define CONFIG_SYS_MPP_CONTROL_2	0x40091818	/* TCTcnt[0] GPP[22:21] BClkIn */
						/* DMAAck[1:0] GNT1[1:0] */
#else
#define CONFIG_SYS_MPP_CONTROL_2	0x40098888	/* TCTcnt[0] */
						/* GPP[22] (RS232IntB or PCI1Int) */
						/* GPP[21] (RS323IntA) */
						/* BClkIn */
						/* REQ1[1:0] GNT1[1:0] */
#endif

#if 0 /* Wrong?? NTL */
# define CONFIG_SYS_MPP_CONTROL_3	0x00090066	/* GPP[31:29] BClkOut0 */
						/* GPP[27:26] Int[1:0] */
#else
# define CONFIG_SYS_MPP_CONTROL_3	0x22090066      /* MREQ MGNT */
						/* GPP[29]    (PCI1Int) */
						/* BClkOut0 */
						/* GPP[27]    (PCI0Int) */
						/* GPP[26]    (RtcInt or PCI1Int) */
						/* CPUInt[25:24] */
#endif

#define CONFIG_SYS_SERIAL_PORT_MUX	0x00001102	/* 11=MPSC1/MPSC0 02=ETH 0 and 2 RMII */

#if 0 /* Wrong?? - NTL */
# define CONFIG_SYS_GPP_LEVEL_CONTROL	0x000002c6
#else
# define CONFIG_SYS_GPP_LEVEL_CONTROL	0x2c600000	/* 0010 1100 0110 0000 */
						/* gpp[29] */
						/* gpp[27:26] */
						/* gpp[22:21] */

# define CONFIG_SYS_SDRAM_CONFIG	0xd8e18200	/* 0x448 */
				/* idmas use buffer 1,1
				   comm use buffer 0
				   pci use buffer 1,1
				   cpu use buffer 0
				   normal load (see also ifdef HVL)
				   standard SDRAM (see also ifdef REG)
				   non staggered refresh */
				/* 31:26  25 23  20 19 18 16 */
				/* 110110 00 111 0  0  00 1 */
				/* refresh_count=0x200
				   phisical interleaving disable
				   virtual interleaving enable */
				/* 15 14 13:0 */
				/* 1  0  0x200 */
#endif

#if 0
#define CONFIG_SYS_DUART_IO		CONFIG_SYS_DEV2_SPACE
#define CONFIG_SYS_DUART_CHAN		1		/* channel to use for console */
#endif
#undef CONFIG_SYS_INIT_CHAN1
#undef CONFIG_SYS_INIT_CHAN2
#if 0
#define SRAM_BASE		CONFIG_SYS_DEV0_SPACE
#define SRAM_SIZE		0x00100000		/* 1 MB of sram */
#endif


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */

#define PCI_HOST_ADAPTER 0              /* configure ar pci adapter     */
#define PCI_HOST_FORCE  1               /* configure as pci host        */
#define PCI_HOST_AUTO   2               /* detected via arbiter enable  */

#define CONFIG_PCI                      /* include pci support          */
#define CONFIG_PCI_HOST PCI_HOST_FORCE  /* select pci host function     */
#define CONFIG_PCI_PNP                  /* do pci plug-and-play         */

/* PCI MEMORY MAP section */
#define CONFIG_SYS_PCI0_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI0_MEM_SIZE	_128M
#define CONFIG_SYS_PCI0_0_MEM_SPACE	(CONFIG_SYS_PCI0_MEM_BASE)

#define CONFIG_SYS_PCI1_MEM_BASE	0x88000000
#define CONFIG_SYS_PCI1_MEM_SIZE	_128M
#define CONFIG_SYS_PCI1_0_MEM_SPACE	(CONFIG_SYS_PCI1_MEM_BASE)

/* PCI I/O MAP section */
#define CONFIG_SYS_PCI0_IO_BASE	0xfa000000
#define CONFIG_SYS_PCI0_IO_SIZE	_16M
#define CONFIG_SYS_PCI0_IO_SPACE	(CONFIG_SYS_PCI0_IO_BASE)
#define CONFIG_SYS_PCI0_IO_SPACE_PCI	0x00000000

#define CONFIG_SYS_PCI1_IO_BASE	0xfb000000
#define CONFIG_SYS_PCI1_IO_SIZE	_16M
#define CONFIG_SYS_PCI1_IO_SPACE	(CONFIG_SYS_PCI1_IO_BASE)
#define CONFIG_SYS_PCI1_IO_SPACE_PCI	0x00000000

/*----------------------------------------------------------------------
 * Initial BAT mappings
 */

/* NOTES:
 * 1) GUARDED and WRITE_THRU not allowed in IBATS
 * 2) CACHEINHIBIT and WRITETHROUGH not allowed together in same BAT
 */

/* SDRAM */
#define CONFIG_SYS_IBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT0U (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT0U CONFIG_SYS_IBAT0U

/* init ram */
#define CONFIG_SYS_IBAT1L  (CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U  (CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L  CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U  CONFIG_SYS_IBAT1U

/* PCI0, PCI1 in one BAT */
#define CONFIG_SYS_IBAT2L BATL_NO_ACCESS
#define CONFIG_SYS_IBAT2U CONFIG_SYS_DBAT2U
#define CONFIG_SYS_DBAT2L (CONFIG_SYS_PCI0_MEM_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U (CONFIG_SYS_PCI0_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* GT regs, bootrom, all the devices, PCI I/O */
#define CONFIG_SYS_IBAT3L (CONFIG_SYS_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW)
#define CONFIG_SYS_IBAT3U (CONFIG_SYS_MISC_REGION_BASE | BATU_VS | BATU_VP | BATU_BL_256M)
#define CONFIG_SYS_DBAT3L (CONFIG_SYS_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U CONFIG_SYS_IBAT3U

/* I2C speed and slave address (for compatability) defaults */
#define CONFIG_SYS_I2C_SPEED	400000
#define CONFIG_SYS_I2C_SLAVE	0x7F

/* I2C addresses for the two DIMM SPD chips */
#ifndef CONFIG_EVB64260_750CX
#define DIMM0_I2C_ADDR	0x56
#define DIMM1_I2C_ADDR	0x54
#else /* CONFIG_EVB64260_750CX - only has 1 DIMM */
#define DIMM0_I2C_ADDR  0x54
#define DIMM1_I2C_ADDR	0x54
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8<<20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	67	/* max number of sectors on one chip */

#define CONFIG_SYS_EXTRA_FLASH_DEVICE	BOOT_DEVICE
#define CONFIG_SYS_EXTRA_FLASH_WIDTH	2	/* 16 bit */
#define CONFIG_SYS_BOOT_FLASH_WIDTH	2	/* 16 bit */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */
#define CONFIG_SYS_FLASH_CFI		1

#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_SIZE		0x1000	/* Total Size of Environment Sector */
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_ADDR		0xFFFE0000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32	/* For all MPC74xx CPUs		 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * L2CR setup -- make sure this is right for your board!
 * look in include/74xx_7xx.h for the defines used here
 */

#define CONFIG_SYS_L2

#define L2_INIT		(L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
			 L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)

#define L2_ENABLE	(L2_INIT | L2CR_L2E)

#define CONFIG_SYS_BOARD_ASM_INIT      1


#endif	/* __CONFIG_H */
