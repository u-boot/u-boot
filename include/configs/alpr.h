/*
 * (C) Copyright 2006-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_ALPR		1	    /* Board is ebony		*/
#define CONFIG_440GX		1	    /* Specifc GX support	*/
#define CONFIG_440		1	    /* ... PPC440 family	*/
#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_pre_init	*/
#define CONFIG_LAST_STAGE_INIT	1	    /* call last_stage_init()	*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000

#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/
#define CONFIG_4xx_DCACHE		/* Enable i- and d-cache	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_SDRAM_BASE		0x00000000	/* _must_ be 0			*/
#define CONFIG_SYS_FLASH_BASE		0xffe00000	/* start of FLASH		*/
#define CONFIG_SYS_MONITOR_BASE	0xfffc0000	/* start of monitor		*/
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory		*/
#define	CONFIG_SYS_PCI_MEMSIZE		0x40000000	/* size of mapped pci memory	*/
#define CONFIG_SYS_ISRAM_BASE		0xc0000000	/* internal SRAM		*/
#define CONFIG_SYS_PCI_BASE		0xd0000000	/* internal PCI regs		*/
#define CONFIG_SYS_PCI_MEMBASE1	CONFIG_SYS_PCI_MEMBASE  + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE2	CONFIG_SYS_PCI_MEMBASE1 + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE3	CONFIG_SYS_PCI_MEMBASE2 + 0x10000000


#define CONFIG_SYS_FPGA_BASE	    (CONFIG_SYS_PERIPHERAL_BASE + 0x08300000)
#define CONFIG_SYS_NVRAM_BASE_ADDR (CONFIG_SYS_PERIPHERAL_BASE + 0x08000000)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_TEMP_STACK_OCM  1
#define CONFIG_SYS_OCM_DATA_ADDR   CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_ADDR   CONFIG_SYS_ISRAM_BASE  /* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_SIZE    0x2000	    /* Size of used area in RAM	*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)

#define CONFIG_SYS_MONITOR_LEN	    (256 * 1024)    /* Reserve 256 kB for Mon	*/
#define CONFIG_SYS_MALLOC_LEN	    (128 * 1024)    /* Reserve 128 kB for malloc*/

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	2	/* Use UART1			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef	CONFIG_SYS_EXT_SERIAL_CLOCK
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI		1	/* The flash is CFI compatible		*/
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use common CFI driver		*/
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash	*/

#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

#define CONFIG_ENV_SECT_SIZE	0x10000	/* size of one complete sector		*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#undef CONFIG_SPD_EEPROM		/* Don't use SPD EEPROM for setup	*/
#define CONFIG_SDRAM_BANK0	1	/* init onboard DDR SDRAM bank 0	*/
#undef CONFIG_SDRAM_ECC			/* enable ECC support			*/
#define CONFIG_SYS_SDRAM_TABLE	{ \
		{(256 << 20), 13, 0x000C4001}, /* 256MB mode 3, 13x10(4)*/ \
		{(64 << 20),  12, 0x00082001}} /* 64MB mode 2, 12x9(4)	*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		100000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F
#define CONFIG_SYS_I2C_NOPROBES	{ {0, 0x69} }	/* Don't probe these addrs */

/*-----------------------------------------------------------------------
 * I2C EEPROM (PCF8594C)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x54	/* EEPROM PCF8594C		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3	/* The Philips PCF8594C has	*/
					/* 8 byte page write mode using */
					/* last 3 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	40   /* and takes up to 40 msec */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run kernelx\" to boot the system;"			\
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth3\0"							\
	"hostname=alpr\0"						\
	"fdt_file=alpr/alpr.dtb\0"					\
	"fdt_addr=400000\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath} ${init}\0"		\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS1,${baudrate} " \
		"mem=193M\0"						\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
		"bootm\0"						\
	"net_nfs_fdt=tftp 200000 ${bootfile};"				\
		"tftp ${fdt_addr} ${fdt_file};"				\
		"run nfsargs addip addtty;"				\
		"bootm 200000 - ${fdt_addr}\0"				\
	"rootpath=/opt/projects/alpr/nfs_root\0"			\
	"bootfile=/alpr/uImage\0"					\
	"kernel_addr=fff00000\0"					\
	"ramdisk_addr=fff10000\0"					\
	"load=tftp 100000 /alpr/u-boot/u-boot.bin\0"			\
	"update=protect off fffc0000 ffffffff;era fffc0000 ffffffff;"	\
		"cp.b 100000 fffc0000 40000;"			        \
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
	"ethprime=ppc_4xx_eth3\0"					\
	"ethact=ppc_4xx_eth3\0"						\
	"autoload=no\0"							\
	"ipconfig=dhcp;setenv serverip 11.0.0.152\0"			\
	"load_fpga=fpga load 0 ffe00000 10dd9a\0"			\
	"mtdargs=setenv bootargs root=/dev/mtdblock6 rw "		\
		"rootfstype=jffs2 init=/sbin/init\0"			\
	"kernel1_mtd=nand read 200000 0 200000;run mtdargs addip addtty"\
		";bootm 200000\0"					\
	"kernel2_mtd=nand read 200000 200000 200000;run mtdargs addip "	\
		"addtty;bootm 200000\0"					\
	"kernel1=setenv actkernel 'kernel1';run load_fpga "		\
		"kernel1_mtd\0"						\
	"kernel2=setenv actkernel 'kernel2';run load_fpga "		\
		"kernel2_mtd\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run kernel2"

#define CONFIG_BOOTDELAY	2	/* autoboot after 5 seconds	*/

#define CONFIG_BAUDRATE		115200

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0x02	/* dummy setting, no EMAC0 used	*/
#define CONFIG_PHY1_ADDR	0x03	/* dummy setting, no EMAC1 used	*/
#define CONFIG_PHY2_ADDR	0x01	/* PHY address for EMAC2	*/
#define CONFIG_PHY3_ADDR	0x02	/* PHY address for EMAC3	*/
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_M88E1111_PHY	1	/* needed for PHY specific setup*/
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_SYS_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/


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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FPGA
#define CONFIG_CMD_FPGA_LOADMK
#define CONFIG_CMD_I2C
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#define CONFIG_CMD_PCI

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_ALT_MEMTEST		1	/* Enable more extensive memtest*/
#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC	1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE	1	/* include version env variable */

#define CONFIG_SYS_4xx_RESET_TYPE	0x2	/* use chip reset on this board	*/

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup	*/
#define CONFIG_SYS_PCI_TARGBASE    0x80000000	/* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */
#define CONFIG_PCI_BOOTDELAY	1       /* enable pci bootdelay variable*/

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT		/* let board init pci target    */
#define CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */

/*-----------------------------------------------------------------------
 * FPGA stuff
 *-----------------------------------------------------------------------*/
#define CONFIG_FPGA
#define CONFIG_FPGA_ALTERA
#define CONFIG_FPGA_CYCLON2
#define CONFIG_SYS_FPGA_CHECK_CTRLC
#define CONFIG_SYS_FPGA_PROG_FEEDBACK
#define CONFIG_FPGA_COUNT       1		/* Ich habe 2 ... aber in
					Reihe geschaltet -> sollte gehen,
					aufpassen mit Datasize ist jetzt
					halt doppelt so gross ... Seite 306
					ist das mit den multiple Device in PS
					Mode erklaert ...*/

/* FPGA program pin configuration */
#define CONFIG_SYS_GPIO_CLK		18	/* FPGA clk pin (cpu output)		*/
#define CONFIG_SYS_GPIO_DATA		19	/* FPGA data pin (cpu output)		*/
#define CONFIG_SYS_GPIO_STATUS		20	/* FPGA status pin (cpu input)		*/
#define CONFIG_SYS_GPIO_CONFIG		21	/* FPGA CONFIG pin (cpu output)		*/
#define CONFIG_SYS_GPIO_CON_DON	22	/* FPGA CONFIG_DONE pin (cpu input)	*/

#define CONFIG_SYS_GPIO_SEL_DPR	14	/* cpu output */
#define CONFIG_SYS_GPIO_SEL_AVR	15	/* cpu output */
#define CONFIG_SYS_GPIO_PROG_EN	23	/* cpu output */

/*-----------------------------------------------------------------------
 * Definitions for GPIO setup
 *-----------------------------------------------------------------------*/
#define CONFIG_SYS_GPIO_SHUTDOWN	(0x80000000 >> 6)
#define CONFIG_SYS_GPIO_SSD_EMPTY	(0x80000000 >> 9)
#define CONFIG_SYS_GPIO_EREADY		(0x80000000 >> 26)
#define CONFIG_SYS_GPIO_REV0		(0x80000000 >> 14)
#define CONFIG_SYS_GPIO_REV1		(0x80000000 >> 15)

/*-----------------------------------------------------------------------
 * NAND-FLASH stuff
 *-----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_NAND_DEVICE	4
#define CONFIG_SYS_NAND_BASE		0xF0000000	/* NAND FLASH Base Address	*/
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE + 0, CONFIG_SYS_NAND_BASE + 2,	\
				  CONFIG_SYS_NAND_BASE + 4, CONFIG_SYS_NAND_BASE + 6 }
#define CONFIG_SYS_NAND_QUIET_TEST	1	/* don't warn upon unknown NAND flash	*/
#define CONFIG_SYS_NAND_MAX_OOBFREE	2
#define CONFIG_SYS_NAND_MAX_ECCPOS	56

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH		CONFIG_SYS_FLASH_BASE

/* Memory Bank 0 (Flash Bank 0, NOR-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB0AP		0x92015480
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH | 0x3A000) /* BS=2MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (NAND-FLASH) initialization					*/
#define CONFIG_SYS_EBC_PB1AP		0x01840380	/* TWT=3			*/
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_NAND_BASE | 0x18000) /* BS=1MB,BU=R/W,BW=8bit */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#endif	/* __CONFIG_H */
