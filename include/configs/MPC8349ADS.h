/*
 * Copyright 2004 Freescale Semiconductor.
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

/*
 * mpc8349ads board configuration file
 *
 * Please refer to doc/README.mpc83xxads for more info.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef DEBUG

#define CONFIG_MII

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */
#define CONFIG_MPC83XX		1	/* MPC83XX family */
#define CONFIG_MPC8349		1	/* MPC8349 specific */
#define CONFIG_MPC8349ADS	1	/* MPC8349ADS board specific */

/* FIXME: Real PCI support will come in a follow-up update. */
#undef CONFIG_PCI

#define CONFIG_TSEC_ENET 		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/

#undef CONFIG_DDR_ECC			/* only for ECC DDR module */

#define PCI_66M
#ifdef PCI_66M
#define CONFIG_83XX_CLKIN	66000000	/* in Hz */
#else
#define CONFIG_83XX_CLKIN	33000000	/* in Hz */
#endif

#ifndef CONFIG_SYS_CLK_FREQ
#ifdef PCI_66M
#define CONFIG_SYS_CLK_FREQ	66000000
#else
#define CONFIG_SYS_CLK_FREQ	33000000
#endif
#endif

#define CONFIG_BOARD_EARLY_INIT_F	/* call board_pre_init */

#define CFG_IMMRBAR		0xE0000000

#undef CFG_DRAM_TEST                   /* memory test, takes time */
#define CFG_MEMTEST_START       0x00000000      /* memtest region */
#define CFG_MEMTEST_END         0x00100000

/*
 * DDR Setup
 */

#define CFG_DDR_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE CFG_DDR_BASE
#undef  CONFIG_DDR_2T_TIMING
#define CFG_DDR_SDRAM_BASE CFG_DDR_BASE

#if defined(CONFIG_SPD_EEPROM)
	/*
	 * Determine DDR configuration from I2C interface.
	 */
	#define SPD_EEPROM_ADDRESS	0x51		/* DDR DIMM */
#else
	/*
	 * Manually set up DDR parameters
	 */
	#define CFG_DDR_SIZE	    256		/* Mb */
	#define CFG_DDR_CONFIG	(CSCONFIG_EN | CSCONFIG_ROW_BIT_13 | CSCONFIG_COL_BIT_9)
	#define CFG_DDR_TIMING_1	0x37344321
	#define CFG_DDR_TIMING_2	0x00000800  /* P9-45,may need tuning */
	#define CFG_DDR_CONTROL 	0xc2000000  /* unbuffered,no DYN_PWR */
	#define CFG_DDR_MODE    	0x00000062  /* DLL,normal,seq,4/2.5 */
	#define CFG_DDR_INTERVAL	0x05200100  /* autocharge,no open page */
#endif

/*
 * SDRAM on the Local Bus
 */
#define CFG_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#define CFG_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

/*
 * FLASH on the Local Bus
 */
#define CFG_FLASH_CFI			/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER			/* use the CFI driver */
#define CFG_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CFG_FLASH_SIZE		8		/* FLASH size in MB */
/* #define CFG_FLASH_USE_BUFFER_WRITE */

#define CFG_BR0_PRELIM	(CFG_FLASH_BASE |	/* Flash Base address */ \
			(2 << BR_PS_SHIFT) |	/* 32 bit port size */	 \
			BR_V)			/* valid */
#define CFG_OR0_PRELIM		0xff806ff7	/* 16Mb Flash size*/
#define CFG_LBLAWBAR0_PRELIM CFG_FLASH_BASE	/* Window base at flash base */
#define CFG_LBLAWAR0_PRELIM  0x80000016		/* 16Mb window size */

#define CFG_MAX_FLASH_BANKS	1		/* number of banks */
#define CFG_MAX_FLASH_SECT	64		/* sectors per device */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CFG_MID_FLASH_JUMP      0x7F000000
#define CFG_MONITOR_BASE    	TEXT_BASE	/* start of monitor */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef  CFG_RAMBOOT
#endif

/*
 * BCSR register on local bus 32KB, 8-bit wide for ADS config reg
 */
#define CFG_BCSR             0xF8000000
#define CFG_LBLAWBAR1_PRELIM CFG_BCSR	/* Access window base at BCSR base */
#define CFG_LBLAWAR1_PRELIM  0x8000000E		/* Access window size 32K */
#define CFG_BR1_PRELIM	  (CFG_BCSR|0x00000801)	/* Port-size=8bit, MSEL=GPCM */
#define CFG_OR1_PRELIM		0xFFFFE8f0	/* length 32K */

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_LOCK 	1
#define CFG_INIT_RAM_ADDR	0xe4010000   /* Initial RAM address */
#define CFG_INIT_RAM_END    	0x1000	     /* End of used area in RAM*/

#define CFG_GBL_DATA_SIZE  	0x100     /* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN	    	(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN	    	(128 * 1024) /* Reserved for malloc */

/*
 * Local Bus LCRR and LBCR regs
 *    LCRR:  DLL bypass, Clock divider is 4
 * External Local Bus rate is
 *    CLKIN * HRCWL_CSB_TO_CLKIN / HRCWL_LCL_BUS_TO_SCB_CLK / LCRR_CLKDIV
 */
#define CFG_LCRR	(LCRR_DBYP | LCRR_CLKDIV_4)
#define CFG_LBC_LBCR	0x00000000

#define CFG_LB_SDRAM	/* if board has SRDAM on local bus */

#ifdef CFG_LB_SDRAM
/*local bus BR2, OR2 definition for SDRAM if soldered on the ADS board*/
/*
 * Base Register 2 and Option Register 2 configure SDRAM.
 * The SDRAM base address, CFG_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR2, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f0001861
 *
 * FIXME: CFG_LBC_SDRAM_BASE should be masked and OR'ed into
 * FIXME: the top 17 bits of BR2.
 */

#define CFG_BR2_PRELIM		0xf0001861 /*Port-size=32bit, MSEL=SDRAM*/
#define CFG_LBLAWBAR2_PRELIM	0xF0000000
#define CFG_LBLAWAR2_PRELIM	0x80000019 /*64M*/

/*
 * The SDRAM size in MB, CFG_LBC_SDRAM_SIZE, is 64.
 *
 * For OR2, need:
 *    64MB mask for AM, OR2[0:7] = 1111 1100
 *		   XAM, OR2[17:18] = 11
 *    9 columns OR2[19-21] = 010
 *    13 rows   OR2[23-25] = 100
 *    EAD set for extra time OR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1100 0000 0000 0110 1001 0000 0001 = fc006901
 */

#define CFG_OR2_PRELIM	0xfc006901

#define CFG_LBC_LSRT	0x32000000    /* LB sdram refresh timer, about 6us */
#define CFG_LBC_MRTPR	0x20000000    /* LB refresh timer prescal, 266MHz/32*/

/*
 * LSDMR masks
 */
#define CFG_LBC_LSDMR_RFEN	(1 << (31 -  1))
#define CFG_LBC_LSDMR_BSMA1516	(3 << (31 - 10))
#define CFG_LBC_LSDMR_BSMA1617	(4 << (31 - 10))
#define CFG_LBC_LSDMR_RFCR5	(3 << (31 - 16))
#define CFG_LBC_LSDMR_RFCR8	(5 << (31 - 16))
#define CFG_LBC_LSDMR_RFCR16	(7 << (31 - 16))
#define CFG_LBC_LSDMR_PRETOACT3	(3 << (31 - 19))
#define CFG_LBC_LSDMR_PRETOACT6	(5 << (31 - 19))
#define CFG_LBC_LSDMR_PRETOACT7	(7 << (31 - 19))
#define CFG_LBC_LSDMR_ACTTORW3	(3 << (31 - 22))
#define CFG_LBC_LSDMR_ACTTORW7	(7 << (31 - 22))
#define CFG_LBC_LSDMR_ACTTORW6	(6 << (31 - 22))
#define CFG_LBC_LSDMR_BL8	(1 << (31 - 23))
#define CFG_LBC_LSDMR_WRC2	(2 << (31 - 27))
#define CFG_LBC_LSDMR_WRC3	(3 << (31 - 27))
#define CFG_LBC_LSDMR_WRC4	(0 << (31 - 27))
#define CFG_LBC_LSDMR_BUFCMD	(1 << (31 - 29))
#define CFG_LBC_LSDMR_CL3	(3 << (31 - 31))

#define CFG_LBC_LSDMR_OP_NORMAL	(0 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ARFRSH	(1 << (31 - 4))
#define CFG_LBC_LSDMR_OP_SRFRSH	(2 << (31 - 4))
#define CFG_LBC_LSDMR_OP_MRW	(3 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PRECH	(4 << (31 - 4))
#define CFG_LBC_LSDMR_OP_PCHALL	(5 << (31 - 4))
#define CFG_LBC_LSDMR_OP_ACTBNK	(6 << (31 - 4))
#define CFG_LBC_LSDMR_OP_RWINV	(7 << (31 - 4))

#define CFG_LBC_LSDMR_COMMON    ( CFG_LBC_LSDMR_RFEN            \
				| CFG_LBC_LSDMR_BSMA1516	\
				| CFG_LBC_LSDMR_RFCR8		\
				| CFG_LBC_LSDMR_PRETOACT6	\
				| CFG_LBC_LSDMR_ACTTORW3	\
				| CFG_LBC_LSDMR_BL8		\
				| CFG_LBC_LSDMR_WRC3		\
				| CFG_LBC_LSDMR_CL3		\
				)

/*
 * SDRAM Controller configuration sequence.
 */
#define CFG_LBC_LSDMR_1		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_PCHALL)
#define CFG_LBC_LSDMR_2		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_ARFRSH)
#define CFG_LBC_LSDMR_3		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_ARFRSH)
#define CFG_LBC_LSDMR_4		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_MRW)
#define CFG_LBC_LSDMR_5		( CFG_LBC_LSDMR_COMMON \
				| CFG_LBC_LSDMR_OP_NORMAL)
#endif

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX     1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE    1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1        (CFG_IMMRBAR+0x4500)
#define CFG_NS16550_COM2        (CFG_IMMRBAR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* I2C */
#define  CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES        {0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET      0x3000
#define CFG_I2C2_OFFSET      0x3100

/* TSEC */
#define CFG_TSEC1_OFFSET 0x24000
#define CFG_TSEC1 (CFG_IMMRBAR+CFG_TSEC1_OFFSET)
#define CFG_TSEC2_OFFSET 0x25000
#define CFG_TSEC2 (CFG_IMMRBAR+CFG_TSEC2_OFFSET)

/* IO Configuration */
#define CFG_IO_CONF (\
	IO_CONF_UART |\
	IO_CONF_TSEC1 |\
	IO_CONF_IRQ0 |\
	IO_CONF_IRQ1 |\
	IO_CONF_IRQ2 |\
	IO_CONF_IRQ3 |\
	IO_CONF_IRQ4 |\
	IO_CONF_IRQ5 |\
	IO_CONF_IRQ6 |\
	IO_CONF_IRQ7 )

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xe2000000
#define CFG_PCI1_IO_SIZE	0x1000000	/* 16M */

#define CFG_PCI2_MEM_BASE	0xA0000000
#define CFG_PCI2_MEM_PHYS	CFG_PCI2_MEM_BASE
#define CFG_PCI2_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI2_IO_BASE	0x00000000
#define CFG_PCI2_IO_PHYS	0xe3000000
#define CFG_PCI2_IO_SIZE	0x1000000	/* 16M */
#if defined(CONFIG_PCI)

#define PCI_ALL_PCI1
#if defined(PCI_64BIT)
#undef PCI_ALL_PCI1
#undef PCI_TWO_PCI1
#undef PCI_ONE_PCI1
#endif

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP	               	/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
	#define PCI_ENET0_IOADDR	0xFIXME
	#define PCI_ENET0_MEMADDR	0xFIXME
	#define PCI_IDSEL_NUMBER	0x0c 	/* slot0->3(IDSEL)=12->15 */
#endif

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CFG_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)
#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI 	1
#endif

#define CONFIG_GMII		1	/* MII PHY management */
#define CONFIG_MPC83XX_TSEC1	1
#define CONFIG_MPC83XX_TSEC1_NAME	"TSEC0"
#define CONFIG_MPC83XX_TSEC2	1
#define CONFIG_MPC83XX_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"TSEC0"

#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#ifndef CFG_RAMBOOT
	#define CFG_ENV_IS_IN_FLASH	1
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x40000)
	#define CFG_ENV_SECT_SIZE	0x40000	/* 256K(one sector) for env */
	#define CFG_ENV_SIZE		0x2000
#else
	#define CFG_NO_FLASH		1	/* Flash is not usable now */
	#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
	#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
	#define CFG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

#if defined(CFG_RAMBOOT)
#if defined(CONFIG_PCI)
#define  CONFIG_COMMANDS	((CONFIG_CMD_DFL	\
				 | CFG_CMD_PING		\
				 | CFG_CMD_PCI		\
				 | CFG_CMD_I2C)		\
				&			\
				 ~(CFG_CMD_ENV		\
				  | CFG_CMD_LOADS))
#else
#define  CONFIG_COMMANDS	((CONFIG_CMD_DFL	\
				 | CFG_CMD_PING		\
				 | CFG_CMD_I2C)		\
				&			\
				 ~(CFG_CMD_ENV		\
				  | CFG_CMD_LOADS))
#endif
#else
#if defined(CONFIG_PCI)
#define  CONFIG_COMMANDS	(CONFIG_CMD_DFL		\
				| CFG_CMD_PCI		\
				| CFG_CMD_PING		\
				| CFG_CMD_I2C)
#else
#define  CONFIG_COMMANDS	(CONFIG_CMD_DFL		\
				| CFG_CMD_PING		\
				| CFG_CMD_I2C       \
				| CFG_CMD_MII       \
				)
#endif
#endif

#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	*/
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
	#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
	#define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/* Cache Configuration */
#define CFG_DCACHE_SIZE		32768
#define CFG_CACHELINE_SIZE	32
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/*log base 2 of the above value*/
#endif

#define CFG_RCWH_PCIHOST 0x80000000 /* PCIHOST  */

#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_4X1 |\
	HRCWL_VCO_1X2 |\
	HRCWL_CORE_TO_CSB_2X1)

#if defined(PCI_64BIT)
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_64_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_DISABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII )
#else
#define CFG_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_32_BIT_PCI |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCI2_ARBITER_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_TSEC1M_IN_GMII |\
	HRCWH_TSEC2M_IN_GMII )
#endif

#define CFG_HID0_INIT 0x000000000

#define CFG_HID0_FINAL CFG_HID0_INIT

/* #define CFG_HID0_FINAL		(\
	HID0_ENABLE_INSTRUCTION_CACHE |\
	HID0_ENABLE_M_BIT |\
	HID0_ENABLE_ADDRESS_BROADCAST ) */

#define CFG_HID2 0x000000000

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02	/* Software reboot */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_ETHADDR   00:04:9f:11:22:33
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR  00:E0:0C:00:7D:01
#endif

#define CONFIG_IPADDR    192.168.1.253

#define CONFIG_HOSTNAME	 unknown
#define CONFIG_ROOTPATH	 /nfsroot
#define CONFIG_BOOTFILE	 your.uImage

#define CONFIG_SERVERIP  192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_NETMASK   255.255.255.0

#define CONFIG_LOADADDR  200000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY 6	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE	 115200


#define	CONFIG_EXTRA_ENV_SETTINGS			\
	"netdev=eth0\0"					\
	"consoledev=ttyS0\0"				\
	"ramdiskaddr=400000\0"				\
	"ramdiskfile=ramfs.83xx\0"

#define CONFIG_NFSBOOTCOMMAND				\
	"setenv bootargs root=/dev/nfs rw "		\
	"nfsroot=$serverip:$rootpath "			\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftp $loadaddr $bootfile;"			\
	"bootm $loadaddr"

#define CONFIG_RAMBOOTCOMMAND				\
	"setenv bootargs root=/dev/ram rw "		\
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftp $ramdiskaddr $ramdiskfile;"		\
	"tftp $loadaddr $bootfile;"			\
	"bootm $loadaddr $ramdiskaddr"

#define CONFIG_BOOTCOMMAND CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
