/*
 * (C) Copyright 2007-2008
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com.
 * Based on the sequoia configuration file.
 *
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/************************************************************************
 * PMC440.h - configuration for esd PMC440 boards
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_440EPX		1	/* Specific PPC440EPx   */
#define CONFIG_440		1	/* ... PPC440 family    */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF90000
#endif

#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_CLK_FREQ	33333400

#if 0 /* temporary disabled because OS/9 does not like dcache on startup */
#define CONFIG_4xx_DCACHE		/* enable dcache        */
#endif

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f */
#define CONFIG_MISC_INIT_F	1
#define CONFIG_MISC_INIT_R	1	/* Call misc_init_r     */
#define CONFIG_BOARD_TYPES	1	/* support board types  */
/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MONITOR_LEN		(~(CONFIG_SYS_TEXT_BASE) + 1)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserve 256 kB for malloc()  */

#define CONFIG_PRAM		0	/* use pram variable to overwrite */

#define CONFIG_SYS_BOOT_BASE_ADDR	0xf0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000	/* _must_ be 0          */
#define CONFIG_SYS_FLASH_BASE		0xfc000000	/* start of FLASH       */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_ADDR		0xd0000000	/* NAND Flash           */
#define CONFIG_SYS_OCM_BASE		0xe0010000	/* ocm                  */
#define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_OCM_BASE
#define CONFIG_SYS_PCI_BASE		0xe0000000	/* Internal PCI regs    */
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory    */
#define CONFIG_SYS_PCI_MEMBASE1	CONFIG_SYS_PCI_MEMBASE  + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE2	CONFIG_SYS_PCI_MEMBASE1 + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE3	CONFIG_SYS_PCI_MEMBASE2 + 0x10000000
#define CONFIG_SYS_PCI_MEMSIZE		0x80000000	/* 2GB! */

#define CONFIG_SYS_USB2D0_BASE		0xe0000100
#define CONFIG_SYS_USB_DEVICE		0xe0000000
#define CONFIG_SYS_USB_HOST		0xe0000400
#define CONFIG_SYS_FPGA_BASE0		0xef000000	/* 32 bit */
#define CONFIG_SYS_FPGA_BASE1		0xef100000	/* 16 bit */
#define CONFIG_SYS_RESET_BASE		0xef200000

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache	*/
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_BASE	/* OCM                  */
#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#undef CONFIG_SYS_EXT_SERIAL_CLOCK
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use FLASH for environment vars */

/*-----------------------------------------------------------------------
 * RTC
 *----------------------------------------------------------------------*/
#define CONFIG_RTC_RX8025

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI		/* The flash is CFI compatible  */
#define CONFIG_FLASH_CFI_DRIVER	/* Use common CFI driver        */

#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks           */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip    */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)      */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)      */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)     */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* use hardware flash protection        */

#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash        */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000	/* size of one complete sector          */
#define CONFIG_ENV_ADDR		((-CONFIG_SYS_MONITOR_LEN)-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector     */

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif

#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_I2C_ENV_EEPROM_BUS	0
#define CONFIG_ENV_OFFSET		0	/* environment starts at the beginning of the EEPROM */
#define CONFIG_ENV_SIZE		0x1000	/* 4096 bytes may be used for env vars */
#endif

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_DDR_DATA_EYE	/* use DDR2 optimization        */
#define CONFIG_SYS_MEM_TOP_HIDE	(4 << 10) /* don't use last 4kbytes */
						  /* 440EPx errata CHIP 11 */

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F
#define CONFIG_SYS_I2C_PPC4XX_CH1
#define CONFIG_SYS_I2C_PPC4XX_SPEED_1		400000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_1		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x01

#define CONFIG_SYS_EEPROM_WREN			1
#define CONFIG_SYS_I2C_BOOT_EEPROM_ADDR	0x52

/*
 * standard dtt sensor configuration - bottom bit will determine local or
 * remote sensor of the TMP401
 */
#define CONFIG_DTT_SENSORS		{ 0, 1 }

/*
 * The PMC440 uses a TI TMP401 temperature sensor. This part
 * is basically compatible to the ADM1021 that is supported
 * by U-Boot.
 *
 * - i2c addr 0x4c
 * - conversion rate 0x02 = 0.25 conversions/second
 * - ALERT ouput disabled
 * - local temp sensor enabled, min set to 0 deg, max set to 70 deg
 * - remote temp sensor enabled, min set to 0 deg, max set to 70 deg
 */
#define CONFIG_DTT_ADM1021
#define CONFIG_SYS_DTT_ADM1021		{ { 0x4c, 0x02, 0, 1, 70, 0, 1, 70, 0} }

#define CONFIG_PREBOOT		"echo Add \\\"run fpga\\\" and "	\
				"\\\"painit\\\" to preboot command"

#undef	CONFIG_BOOTARGS

/* Setup some board specific values for the default environment variables */
#define CONFIG_HOSTNAME		pmc440
#define CONFIG_SYS_BOOTFILE	"bootfile=/tftpboot/pmc440/uImage\0"
#define CONFIG_SYS_ROOTPATH	"rootpath=/opt/eldk/ppc_4xxFP\0"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_SYS_BOOTFILE						\
	CONFIG_SYS_ROOTPATH						\
	"fdt_file=/tftpboot/pmc440/pmc440.dtb\0"			\
	"netdev=eth0\0"							\
	"ethrotate=no\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
	"nfsroot=${serverip}:${rootpath}\0"				\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"addmisc=setenv bootargs ${bootargs} mem=${mem}\0"		\
	"nandargs=setenv bootargs root=/dev/mtdblock6 rootfstype=jffs2 rw\0" \
	"nand_boot_fdt=run nandargs addip addtty addmisc;"		\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"net_nfs_fdt=tftp ${kernel_addr_r} ${bootfile};"		\
		"tftp  ${fdt_addr_r} ${fdt_file};"			\
		"run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"kernel_addr=ffc00000\0"					\
	"kernel_addr_r=200000\0"					\
	"fpga_addr=fff00000\0"						\
	"fdt_addr=fff80000\0"						\
	"fdt_addr_r=800000\0"						\
	"fpga=fpga loadb 0 ${fpga_addr}\0"				\
	"load=tftp 200000 /tftpboot/pmc440/u-boot.bin\0"		\
	"update=protect off fff90000 ffffffff;era fff90000 ffffffff;"	\
		"cp.b 200000 fff90000 70000\0"				\
	""

#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds     */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download  */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change        */

#define CONFIG_PPC4xx_EMAC
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management           */
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics  */

#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0
#define CONFIG_SYS_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	1
#define CONFIG_RESET_PHY_R	1

/* USB */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CONFIG_SYS_OHCI_BE_CONTROLLER

#define CONFIG_SYS_USB_OHCI_BOARD_INIT 1
#define CONFIG_SYS_USB_OHCI_CPU_INIT	1
#define CONFIG_SYS_USB_OHCI_REGS_BASE	CONFIG_SYS_USB_HOST
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"ppc440"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PCI
#define CONFIG_CMD_REGINFO

/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_MEMORY |	\
				 CONFIG_SYS_POST_CPU    |	\
				 CONFIG_SYS_POST_UART   |	\
				 CONFIG_SYS_POST_I2C    |	\
				 CONFIG_SYS_POST_CACHE  |	\
				 CONFIG_SYS_POST_FPU    |	\
				 CONFIG_SYS_POST_ETHER  |	\
				 CONFIG_SYS_POST_SPR)

#define CONFIG_LOGBUFFER
#define CONFIG_SYS_POST_CACHE_ADDR	0x7fff0000	/* free virtual address     */

#define CONFIG_SYS_CONSOLE_IS_IN_ENV	/* Otherwise it catches logbuffer as output */

#define CONFIG_SUPPORT_VFAT

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_LONGHELP			/* undef to save memory         */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size      */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size      */
#endif
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on          */
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM       */

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address      */
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history     */
#define CONFIG_MX_CYCLIC	1	/* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI		/* include pci support          */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP		/* do (not) pci plug-and-play   */
#define CONFIG_SYS_PCI_CACHE_LINE_SIZE	0	/* to avoid problems with PNP   */
#define CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup  */
#define CONFIG_SYS_PCI_TARGBASE	0x80000000 /* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT
#define CONFIG_SYS_PCI_MASTER_INIT
#define CONFIG_SYS_PCI_BOARD_FIXUP_IRQ

#define CONFIG_PCI_BOOTDELAY 0

/* PCI identification */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x12FE	/* PCI Vendor ID: esd gmbh      */
#define CONFIG_SYS_PCI_SUBSYS_ID_NONMONARCH 0x0441	/* PCI Device ID: Non-Monarch */
#define CONFIG_SYS_PCI_SUBSYS_ID_MONARCH 0x0440	/* PCI Device ID: Monarch */
/* for weak __pci_target_init() */
#define CONFIG_SYS_PCI_SUBSYS_ID	CONFIG_SYS_PCI_SUBSYS_ID_MONARCH
#define CONFIG_SYS_PCI_CLASSCODE_NONMONARCH	PCI_CLASS_PROCESSOR_POWERPC
#define CONFIG_SYS_PCI_CLASSCODE_MONARCH	PCI_CLASS_BRIDGE_HOST

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FPGA stuff
 *----------------------------------------------------------------------*/
#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_SPARTAN2
#define CONFIG_FPGA_SPARTAN3

#define CONFIG_FPGA_COUNT	2
/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/*
 * On Sequoia CS0 and CS3 are switched when configuring for NAND booting
 */
#define CONFIG_SYS_NAND_CS		2	/* NAND chip connected to CSx   */

/* Memory Bank 0 (NOR-FLASH) initialization */
#define CONFIG_SYS_EBC_PB0AP		0x03017200
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH_BASE | 0xda000)

/* Memory Bank 2 (NAND-FLASH) initialization */
#define CONFIG_SYS_EBC_PB2AP		0x018003c0
#define CONFIG_SYS_EBC_PB2CR		(CONFIG_SYS_NAND_ADDR | 0x1c000)

/* Memory Bank 1 (RESET) initialization */
#define CONFIG_SYS_EBC_PB1AP		0x7f817200 /* 0x03017200 */
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_RESET_BASE | 0x1c000)

/* Memory Bank 4 (FPGA / 32Bit) initialization */
#define CONFIG_SYS_EBC_PB4AP		0x03840f40	/* BME=0,TWT=7,CSN=1,TH=7,RE=1,SOR=0,BEM=1 */
#define CONFIG_SYS_EBC_PB4CR		(CONFIG_SYS_FPGA_BASE0 | 0x1c000)	/* BS=1M,BU=R/W,BW=32bit */

/* Memory Bank 5 (FPGA / 16Bit) initialization */
#define CONFIG_SYS_EBC_PB5AP		0x03840f40	/* BME=0,TWT=3,CSN=1,TH=0,RE=1,SOR=0,BEM=1 */
#define CONFIG_SYS_EBC_PB5CR		(CONFIG_SYS_FPGA_BASE1 | 0x1a000)	/* BS=1M,BU=R/W,BW=16bit */

/*-----------------------------------------------------------------------
 * NAND FLASH
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_NAND_ADDR + CONFIG_SYS_NAND_CS)
#define CONFIG_SYS_NAND_SELECT_DEVICE	1 /* nand driver supports mutipl. chips */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

#define CONFIG_API		1

#endif /* __CONFIG_H */
