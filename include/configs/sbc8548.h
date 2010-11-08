/*
 * Copyright 2007,2009 Wind River Systems <www.windriver.com>
 * Copyright 2007 Embedded Specialties, Inc.
 * Copyright 2004, 2007 Freescale Semiconductor.
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
 * sbc8548 board configuration file
 * Please refer to doc/README.sbc8548 for more info.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Top level Makefile configuration choices
 */
#ifdef CONFIG_MK_PCI
#define CONFIG_PCI
#define CONFIG_PCI1
#endif

#ifdef CONFIG_MK_66
#define CONFIG_SYS_CLK_DIV 1
#endif

#ifdef CONFIG_MK_33
#define CONFIG_SYS_CLK_DIV 2
#endif

#ifdef CONFIG_MK_PCIE
#define CONFIG_PCIE1
#endif

/*
 * High Level Configuration Options
 */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8548		1	/* MPC8548 specific */
#define CONFIG_SBC8548		1	/* SBC8548 board specific */

#undef CONFIG_RIO

#ifdef CONFIG_PCI
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT    1	/* enable 64-bit PCI resources */
#endif
#ifdef CONFIG_PCIE1
#define CONFIG_FSL_PCIE_RESET   1	/* need PCIe reset errata */
#endif

#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

/*
 * Below assumes that CCB:SYSCLK remains unchanged at 6:1 via SW2:[1-4]
 */
#ifndef CONFIG_SYS_CLK_DIV
#define CONFIG_SYS_CLK_DIV	1	/* 2, if 33MHz PCI card installed */
#endif
#define CONFIG_SYS_CLK_FREQ	(66000000 / CONFIG_SYS_CLK_DIV)

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CONFIG_SYS_DRAM_TEST			/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CONFIG_SYS_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CONFIG_SYS_CCSRBAR_PHYS	CONFIG_SYS_CCSRBAR	/* physical addr of CCSRBAR */
#define CONFIG_SYS_IMMR		CONFIG_SYS_CCSRBAR	/* PQII uses CONFIG_SYS_IMMR */

/* DDR Setup */
#define CONFIG_FSL_DDR2
#undef CONFIG_FSL_DDR_INTERACTIVE
#undef CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#undef CONFIG_DDR_SPD
#undef CONFIG_DDR_ECC			/* only for ECC DDR module */

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x51	/* CTLR 0 DIMM 0 */

/*
 * Make sure required options are set
 */
#ifndef CONFIG_SPD_EEPROM
	#define CONFIG_SYS_SDRAM_SIZE	256		/* DDR is 256MB */
#endif

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * FLASH on the Local Bus
 * Two banks, one 8MB the other 64MB, using the CFI driver.
 * Boot from BR0/OR0 bank at 0xff80_0000
 * Alternate BR6/OR6 bank at 0xfb80_0000
 *
 * BR0:
 *    Base address 0 = 0xff80_0000 = BR0[0:16] = 1111 1111 1000 0000 0
 *    Port Size = 8 bits = BRx[19:20] = 01
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0000 1000 0000 0001 = ff800801    BR0
 *
 * BR6:
 *    Base address 6 = 0xfb80_0000 = BR6[0:16] = 1111 1011 1000 0000 0
 *    Port Size = 32 bits = BRx[19:20] = 11
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1011 1000 0000 0001 1000 0000 0001 = fb801801    BR6
 *
 * OR0:
 *    Addr Mask = 8M = OR1[0:16] = 1111 1111 1000 0000 0
 *    XAM = OR0[17:18] = 11
 *    CSNT = OR0[20] = 1
 *    ACS = half cycle delay = OR0[21:22] = 11
 *    SCY = 6 = OR0[24:27] = 0110
 *    TRLX = use relaxed timing = OR0[29] = 1
 *    EAD = use external address latch delay = OR0[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0110 1110 0110 0101 = ff806e65    OR0
 *
 * OR6:
 *    Addr Mask = 64M = OR6[0:16] = 1111 1000 0000 0000 0
 *    XAM = OR6[17:18] = 11
 *    CSNT = OR6[20] = 1
 *    ACS = half cycle delay = OR6[21:22] = 11
 *    SCY = 6 = OR6[24:27] = 0110
 *    TRLX = use relaxed timing = OR6[29] = 1
 *    EAD = use external address latch delay = OR6[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1000 0000 0000 0110 1110 0110 0101 = f8006e65    OR6
 */

#define CONFIG_SYS_BOOT_BLOCK		0xff800000	/* start of 8MB Flash */
#define CONFIG_SYS_ALT_FLASH		0xfb800000	/* 64MB "user" flash */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_BOOT_BLOCK	/* start of FLASH 16M */

#define CONFIG_SYS_BR0_PRELIM		0xff800801
#define CONFIG_SYS_BR6_PRELIM		0xfb801801

#define	CONFIG_SYS_OR0_PRELIM		0xff806e65
#define	CONFIG_SYS_OR6_PRELIM		0xf8006e65

#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE, \
					 CONFIG_SYS_ALT_FLASH}
#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256		/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO

/* CS5 = Local bus peripherals controlled by the EPLD */

#define CONFIG_SYS_BR5_PRELIM		0xf8000801
#define CONFIG_SYS_OR5_PRELIM		0xff006e65
#define CONFIG_SYS_EPLD_BASE		0xf8000000
#define CONFIG_SYS_LED_DISP_BASE	0xf8000000
#define CONFIG_SYS_USER_SWITCHES_BASE	0xf8100000
#define CONFIG_SYS_BD_REV		0xf8300000
#define CONFIG_SYS_EEPROM_BASE		0xf8b00000

/*
 * SDRAM on the Local Bus (CS3 and CS4)
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#define CONFIG_SYS_LBC_SDRAM_SIZE	128		/* LBC SDRAM is 128MB */

/*
 * Base Register 3 and Option Register 3 configure the 1st 1/2 SDRAM.
 * The SDRAM base address, CONFIG_SYS_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR3, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f0001861
 *
 */

#define CONFIG_SYS_BR3_PRELIM		0xf0001861

/*
 * The SDRAM size in MB, of 1/2 CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR3, need:
 *    64MB mask for AM, OR3[0:7] = 1111 1100
 *		   XAM, OR3[17:18] = 11
 *    10 columns OR3[19-21] = 011
 *    12 rows   OR3[23-25] = 011
 *    EAD set for extra time OR[31] = 0
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1100 0000 0000 0110 1100 1100 0000 = fc006cc0
 */

#define CONFIG_SYS_OR3_PRELIM		0xfc006cc0

/*
 * Base Register 4 and Option Register 4 configure the 2nd 1/2 SDRAM.
 * The base address, (SDRAM_BASE + 1/2*SIZE), is 0xf4000000.
 *
 * For BR4, need:
 *    Base address of 0xf4000000 = BR[0:16] = 1111 0100 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f4001861
 *
 */

#define CONFIG_SYS_BR4_PRELIM		0xf4001861

/*
 * The SDRAM size in MB, of 1/2 CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR4, need:
 *    64MB mask for AM, OR3[0:7] = 1111 1100
 *		   XAM, OR3[17:18] = 11
 *    10 columns OR3[19-21] = 011
 *    12 rows   OR3[23-25] = 011
 *    EAD set for extra time OR[31] = 0
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1100 0000 0000 0110 1100 1100 0000 = fc006cc0
 */

#define CONFIG_SYS_OR4_PRELIM		0xfc006cc0

#define CONFIG_SYS_LBC_LCRR		0x00000002    /* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR		0x00000000    /* LB config reg */
#define CONFIG_SYS_LBC_LSRT		0x20000000  /* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR		0x00000000  /* LB refresh timer prescal*/

/*
 * Common settings for all Local Bus SDRAM commands.
 * At run time, either BSMA1516 (for CPU 1.1)
 *                  or BSMA1617 (for CPU 1.0) (old)
 * is OR'ed in too.
 */
#define CONFIG_SYS_LBC_LSDMR_COMMON	( LSDMR_RFCR16		\
				| LSDMR_PRETOACT7	\
				| LSDMR_ACTTORW7	\
				| LSDMR_BL8		\
				| LSDMR_WRC4		\
				| LSDMR_CL3		\
				| LSDMR_RFEN		\
				)

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_END	0x4000		/* End of used area in RAM */

#define CONFIG_SYS_INIT_L2_ADDR	0xf8f80000	/* relocate boot L2SRAM */

#define CONFIG_SYS_GBL_DATA_SIZE	128		/* num bytes initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * For soldered on flash, (128kB/sector) we use 2 sectors for u-boot and
 * one for env+bootpg (TEXT_BASE=0xfffa_0000, 384kB total).  For SODIMM
 * flash (512kB/sector) we use 1 sector for u-boot, and one for env+bootpg
 * (TEXT_BASE=0xfff0_0000, 1MB total).  This dynamically sets the right
 * thing for MONITOR_LEN in both cases.
 */
#define CONFIG_SYS_MONITOR_LEN		(~TEXT_BASE + 1)
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(400000000 / CONFIG_SYS_CLK_DIV)

#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x3000

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCI_VIRT		0x80000000	/* 1G PCI TLB */
#define CONFIG_SYS_PCI_PHYS		0x80000000	/* 1G PCI TLB */

#define CONFIG_SYS_PCI1_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCI1_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0x80000000
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_VIRT		0xe2000000
#define CONFIG_SYS_PCI1_IO_BUS		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xe2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00800000	/* 8M */

#ifdef CONFIG_PCIE1
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xe2800000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe2800000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000	/* 8M */
#endif

#ifdef CONFIG_RIO
/*
 * RapidIO MMU
 */
#define CONFIG_SYS_RIO_MEM_BASE	0xC0000000
#define CONFIG_SYS_RIO_MEM_SIZE	0x20000000	/* 512M */
#endif

#if defined(CONFIG_PCI)

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif	/* CONFIG_PCI */


#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC1"
#undef CONFIG_MPC85XX_FEC

#define TSEC1_PHY_ADDR		0x19
#define TSEC2_PHY_ADDR		0x1a

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0

#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: eTSEC[0-3] */
#define CONFIG_ETHPRIME		"eTSEC0"
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x2000
#if TEXT_BASE == 0xfff00000	/* Boot from 64MB SODIMM */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x80000)
#define CONFIG_ENV_SECT_SIZE	0x80000	/* 512K(one sector) for env */
#elif TEXT_BASE == 0xfffa0000	/* Boot from 8MB soldered flash */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x40000)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
#else
#warning undefined environment size/location.
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_CMDLINE_EDITING			/* undef to save memory */
#define CONFIG_AUTO_COMPLETE			/* add autocompletion support */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

/* The mac addresses for all ethernet interface */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR	 02:E0:0C:00:00:FD
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR	 02:E0:0C:00:01:FD
#endif

#define CONFIG_IPADDR	 192.168.0.55

#define CONFIG_HOSTNAME	 sbc8548
#define CONFIG_ROOTPATH	 /opt/eldk/ppc_85xx
#define CONFIG_BOOTFILE	 /uImage
#define CONFIG_UBOOTPATH /u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP	 192.168.0.2
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_NETMASK	 255.255.255.0

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 10	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs*/

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS				\
 "netdev=eth0\0"						\
 "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
 "tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
	"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
	"protect on " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
 "consoledev=ttyS0\0"				\
 "ramdiskaddr=2000000\0"			\
 "ramdiskfile=uRamdisk\0"			\
 "fdtaddr=c00000\0"				\
 "fdtfile=sbc8548.dtb\0"

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr - $fdtaddr"


#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND	CONFIG_RAMBOOTCOMMAND

#endif	/* __CONFIG_H */
