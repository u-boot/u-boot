/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Based on original work by
 *      Roel Loeffen, (C) Copyright 2006 Prodrive B.V.
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

/************************************************************************
 * p3mx.h - configuration for Prodrive P3M750 & P3M7448 boards
 *
 * The defines:
 * CONFIG_P3M750 or
 * CONFIG_P3M7448
 * are written into include/config.h by the "make xxx_config" command
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_P3Mx			/* used for both board versions	*/

#if defined (CONFIG_P3M750)
#define CONFIG_750FX			/* 750GL/GX/FX			*/
#define CFG_BOARD_NAME		"P3M750"
#define CFG_BUS_HZ		100000000
#define CFG_BUS_CLK		CFG_BUS_HZ
#define CFG_TCLK		100000000
#elif defined (CONFIG_P3M7448)
#define CONFIG_74xx
#define CFG_BOARD_NAME		"P3M7448"
#define CFG_BUS_HZ		133333333
#define CFG_BUS_CLK		CFG_BUS_HZ
#define CFG_TCLK		133333333
#endif
#define CFG_GT_DUAL_CPU			/* also for JTAG even with one cpu */

/* which initialization functions to call for this board */
#define CFG_BOARD_ASM_INIT	1
#define CONFIG_BOARD_EARLY_INIT_F 1     /* Call board_early_init_f	*/
#define CONFIG_BOARD_EARLY_INIT_R 1     /* Call board_early_init_f	*/
#define CONFIG_MISC_INIT_R      1	/* Call misc_init_r()		*/
#define CONFIG_ADD_RAM_INFO	1	/* Print additional info	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE		0x00000000
#ifdef CONFIG_P3M750
#define CFG_SDRAM1_BASE		0x10000000	/* each 256 MByte	*/
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#if defined (CONFIG_P3M750)
#define CFG_FLASH_BASE		0xff800000	/* start of flash banks	*/
#define CFG_BOOT_SIZE		_8M		/* boot flash		*/
#elif defined (CONFIG_P3M7448)
#define CFG_FLASH_BASE		0xff000000	/* start of flash banks	*/
#define CFG_BOOT_SIZE		_16M		/* boot flash		*/
#endif
#define CFG_BOOT_SPACE		CFG_FLASH_BASE	/* BOOT_CS0 flash 0    */
#define CFG_MONITOR_BASE	0xfff00000
#define CFG_RESET_ADDRESS	0xfff00100
#define CFG_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */
#define CFG_MISC_REGION_BASE	0xf0000000

#define CFG_DFL_GT_REGS		0xf1000000	/* boot time GT_REGS */
#define CFG_GT_REGS		0xf1000000	/* GT Registers are mapped here */
#define CFG_INT_SRAM_BASE	0x42000000	/* GT offers 256k internal SRAM */

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
 /*
 * When locking data in cache you should point the CFG_INIT_RAM_ADDRESS
 * To an unused memory region. The stack will remain in cache until RAM
 * is initialized
*/
#undef	CFG_INIT_RAM_LOCK
#define CFG_INIT_RAM_ADDR	0x42000000
#define CFG_INIT_RAM_END	0x1000
#define CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for init data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)


/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_MPSC			/* MV64460 Serial		*/
#define CONFIG_MPSC_PORT	0
#define CONFIG_BAUDRATE		115200	/* console baudrate		*/
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
/* Change the default ethernet port, use this define (options: 0, 1, 2) */
#define CFG_ETH_PORT		ETH_0
#define CONFIG_NET_MULTI
#define MV_ETH_DEVS		2
#define CONFIG_PHY_RESET        1	/* reset phy upon startup         */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI			/* The flash is CFI compatible		*/
#define CFG_FLASH_CFI_DRIVER		/* Use common CFI driver		*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/
#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#if defined (CONFIG_P3M750)
#define CFG_ENV_SECT_SIZE	0x20000 	/* one sector (1 device)*/
#elif defined (CONFIG_P3M7448)
#define CFG_ENV_SECT_SIZE	0x40000 	/* two sectors (2 devices parallel */
#endif
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_MV64460_ECC

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CFG_I2C_SPEED		100000		/* I2C speed default	*/

/* I2C RTC */
#define CONFIG_RTC_M41T11	1
#define CFG_I2C_RTC_ADDR	0x68
#define CFG_M41T11_BASE_YEAR	1900	/* play along with linux	*/

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#undef CONFIG_PCI			/* include pci support		*/
#ifdef CONFIG_PCI
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW		/* show devices on bus		*/
#endif /* CONFIG_PCI */

/* PCI MEMORY MAP section */
#define CFG_PCI0_MEM_BASE	0x80000000
#define CFG_PCI0_MEM_SIZE	_128M
#define CFG_PCI1_MEM_BASE	0x88000000
#define CFG_PCI1_MEM_SIZE	_128M

#define CFG_PCI0_0_MEM_SPACE	(CFG_PCI0_MEM_BASE)
#define CFG_PCI1_0_MEM_SPACE	(CFG_PCI1_MEM_BASE)

/* PCI I/O MAP section */
#define CFG_PCI0_IO_BASE	0xfa000000
#define CFG_PCI0_IO_SIZE	_16M
#define CFG_PCI1_IO_BASE	0xfb000000
#define CFG_PCI1_IO_SIZE	_16M

#define CFG_PCI0_IO_SPACE	(CFG_PCI0_IO_BASE)
#define CFG_PCI0_IO_SPACE_PCI	0x00000000
#define CFG_PCI1_IO_SPACE	(CFG_PCI1_IO_BASE)
#define CFG_PCI1_IO_SPACE_PCI	0x00000000

#define CFG_ISA_IO_BASE_ADDRESS (CFG_PCI0_IO_BASE)
#define CFG_PCI_IDSEL 0x30

#undef	CONFIG_BOOTARGS
#define	CONFIG_EXTRA_ENV_SETTINGS_COMMON				\
	"netdev=eth0\0"							\
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
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"u-boot=p3mx/u-boot/u-boot.bin\0"				\
	"load=tftp 100000 ${u-boot}\0"					\
	"update=protect off fff00000 fff3ffff;era fff00000 fff3ffff;"	\
		"cp.b 100000 fff00000 40000;"			        \
		"setenv filesize;saveenv\0"				\
	"upd=run load;run update\0"					\
	"serverip=11.0.0.152\0"

#if defined (CONFIG_P3M750)
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_EXTRA_ENV_SETTINGS_COMMON				\
	"hostname=p3m750\0"						\
	"bootfile=/tftpboot/p3mx/vxWorks.st\0"				\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc180000\0"					\
	"vxfile=p3m750/vxWorks\0"					\
	"vxuser=ddg\0"							\
	"vxpass=ddg\0"							\
	"vxtarget=target\0"						\
	"vxflags=0x8\0"							\
	"vxargs=setenv bootargs mgi(0,0)host:${vxfile} h=${serverip} "	\
		"e=${ipaddr} u=${vxuser} pw=${vxpass} tn=${vxtarget} "	\
		"f=${vxflags}\0"
#elif defined (CONFIG_P3M7448)
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_EXTRA_ENV_SETTINGS_COMMON				\
	"hostname=p3m7448\0"
#endif

#if defined (CONFIG_P3M750)
#define CONFIG_BOOTCOMMAND	"tftp;run vxargs;bootvx"
#elif defined (CONFIG_P3M7448)
#define CONFIG_BOOTCOMMAND	" "
#endif

#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds */
#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | \
				 CONFIG_BOOTP_BOOTFILESIZE)

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_PCI
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_SDRAM


/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "

#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	        16	/* max number of command args	*/
#define CFG_BARGSIZE	        CFG_CBSIZE /* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000 /* memtest works on	        */
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x08000000	/* default load address */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * Marvell MV64460 config settings
 *----------------------------------------------------------------------*/
/* Reset values for Port behavior (8bit/ 32bit, etc.) only corrected device width */
#if defined (CONFIG_P3M750)
#define CFG_BOOT_PAR		0x8FDFF87F	/* 16 bit flash, disable burst*/
#elif defined (CONFIG_P3M7448)
#define CFG_BOOT_PAR		0x8FEFFFFF	/* 32 bit flash, burst enabled */
#endif

/*
 * MPP[0]	Serial Port 0 TxD	TxD	OUT	Connected to P14 (buffered)
 * MPP[1]	Serial Port 0 RxD	RxD	IN	Connected to P14 (buffered)
 * MPP[2]	NC
 * MPP[3]	Serial Port 1 TxD	TxD	OUT	Connected to P14 (buffered)
 * MPP[4]	PCI Monarch#		GPIO	IN	Connected to P12
 * MPP[5]	Serial Port 1 RxD	RxD	IN	Connected to P14 (buffered)
 * MPP[6]	PMC Carrier Interrupt 0	Int	IN	Connected to P14
 * MPP[7]	PMC Carrier Interrupt 1	Int	IN	Connected to P14
 * MPP[8]	Reserved				Do not use
 * MPP[9]	Reserved				Do not use
 * MPP[10]	Reserved				Do not use
 * MPP[11]	Reserved				Do not use
 * MPP[12]	Phy 0 Interrupt		Int	IN
 * MPP[13]	Phy 1 Interrupt		Int	IN
 * MPP[14]	NC
 * MPP[15]	NC
 * MPP[16]	PCI Interrupt C		Int	IN	Connected to P11
 * MPP[17]	PCI Interrupt D		Int	IN	Connected to P11
 * MPP[18]	Watchdog NMI#		GPIO	IN	Connected to MPP[24]
 * MPP[19]	Watchdog Expired#	WDE	OUT	Connected to rst logic
 * MPP[20]	Watchdog Status		WD_STS	IN	Read back of rst by watchdog
 * MPP[21]	NC
 * MPP[22]	GP LED Green		GPIO	OUT
 * MPP[23]	GP LED Red		GPIO	OUT
 * MPP[24]	Watchdog NMI#		Int	OUT
 * MPP[25]	NC
 * MPP[26]	NC
 * MPP[27]	PCI Interrupt A		Int	IN	Connected to P11
 * MPP[28]	NC
 * MPP[29]	PCI Interrupt B		Int	IN	Connected to P11
 * MPP[30]	Module reset		GPIO	OUT	Board reset
 * MPP[31]	PCI EReady		GPIO	IN	Connected to P12
 */
#define CFG_MPP_CONTROL_0	0x00303022
#define CFG_MPP_CONTROL_1	0x00000000
#define CFG_MPP_CONTROL_2	0x00004000
#define CFG_MPP_CONTROL_3	0x00000004
#define CFG_GPP_LEVEL_CONTROL	0x280730D0

/*----------------------------------------------------------------------
 * Initial BAT mappings
 */

/* NOTES:
 * 1) GUARDED and WRITE_THRU not allowed in IBATS
 * 2) CACHEINHIBIT and WRITETHROUGH not allowed together in same BAT
 */
/* SDRAM */
#define CFG_IBAT0L (CFG_SDRAM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT0U (CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT0L (CFG_SDRAM_BASE | BATL_PP_RW | BATL_GUARDEDSTORAGE | BATL_CACHEINHIBIT)
#define CFG_DBAT0U CFG_IBAT0U

/* init ram */
#define CFG_IBAT1L  (CFG_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CFG_IBAT1U  (CFG_INIT_RAM_ADDR | BATU_BL_256K | BATU_VS | BATU_VP)
#define CFG_DBAT1L  CFG_IBAT1L
#define CFG_DBAT1U  CFG_IBAT1U

/* PCI0, PCI1 in one BAT */
#define CFG_IBAT2L BATL_NO_ACCESS
#define CFG_IBAT2U CFG_DBAT2U
#define CFG_DBAT2L (CFG_PCI0_MEM_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CFG_DBAT2U (CFG_PCI0_MEM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* GT regs, bootrom, all the devices, PCI I/O */
#define CFG_IBAT3L (CFG_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW)
#define CFG_IBAT3U (CFG_MISC_REGION_BASE | BATU_VS | BATU_VP | BATU_BL_256M)
#define CFG_DBAT3L (CFG_MISC_REGION_BASE | BATL_CACHEINHIBIT | BATL_PP_RW | BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U CFG_IBAT3U

#define CFG_IBAT4L (CFG_SDRAM1_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT4U (CFG_SDRAM1_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT4L (CFG_SDRAM1_BASE | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT4U CFG_IBAT4U

/* set rest out of range for Linux !!!!!!!!!!! */

/* IBAT5 and DBAT5 */
#define CFG_IBAT5L (0x20000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT5U (0x20000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT5L (0x20000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT5U CFG_IBAT5U

/* IBAT6 and DBAT6 */
#define CFG_IBAT6L (0x20000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT6U (0x20000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT6L (0x20000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT6U CFG_IBAT6U

/* IBAT7 and DBAT7 */
#define CFG_IBAT7L (0x20000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT7U (0x20000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT7L (0x20000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT7U CFG_IBAT7U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8<<20) /* Initial Memory map for Linux */
#define CFG_VXWORKS_MAC_PTR	0x42010000 /* use some memory in SRAM that's not used!!! */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For all MPC74xx CPUs		 */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * L2CR setup -- make sure this is right for your board!
 * look in include/mpc74xx.h for the defines used here
 */
#define CFG_L2

#if defined (CONFIG_750CX) || defined (CONFIG_750FX)
#define L2_INIT 0
#else
#define L2_INIT		(L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
			L2CR_L2OH_5 | L2CR_L2CTL | L2CR_L2WT)
#endif

#define L2_ENABLE	(L2_INIT | L2CR_L2E)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot		    */

#endif	/* __CONFIG_H */
