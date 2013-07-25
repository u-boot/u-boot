/*
 * (C) Copyright 2009
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU		*/
#define CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU)	*/
#define CONFIG_INKA4X0		1	/* INKA4x0 board			*/

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFFE00000	boot low
 * 0x00100000	boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFE00000	/* Standard: boot low */
#endif
#define CONFIG_SYS_LDSCRIPT	"arch/powerpc/cpu/mpc5xxx/u-boot-customlayout.lds"

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz		*/

#define CONFIG_MISC_INIT_F	1	/* Use misc_init_f()			*/

#define CONFIG_HIGH_BATS	1	/* High BATs supported			*/

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1	*/
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps	*/
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CONFIG_SYS_XLB_PIPELINING	1

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION


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

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB

#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

#if (CONFIG_SYS_TEXT_BASE == 0xFFE00000)		/* Boot low */
#   define CONFIG_SYS_LOWBOOT		1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	1	/* autoboot after 1 second */

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_ETHADDR		00:a0:a4:03:00:00
#define	CONFIG_OVERWRITE_ETHADDR_ONCE

#define	CONFIG_IPADDR		192.168.100.2
#define	CONFIG_SERVERIP		192.168.100.1
#define	CONFIG_NETMASK		255.255.255.0
#define HOSTNAME		inka4x0
#define CONFIG_BOOTFILE		"/tftpboot/inka4x0/uImage"
#define	CONFIG_ROOTPATH		"/opt/eldk/ppc_6xx"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=ttyS0,${baudrate}\0"				\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp 200000 ${bootfile};"				\
		"run nfsargs addip addcons;bootm\0"			\
	"enable_disp=mw.l 100000 04000000 1;"				\
		"cp.l 100000 f0000b20 1;"				\
		"cp.l 100000 f0000b28 1\0"				\
	"ideargs=setenv bootargs root=/dev/hda1 rw\0"			\
	"ide_boot=ext2load ide 0:1 200000 uImage;"			\
		"run ideargs addip addcons enable_disp;bootm\0"		\
	"brightness=255\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run ide_boot"

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1
#define CONFIG_SYS_FLASH_BASE		0xffe00000
#define CONFIG_SYS_FLASH_SIZE		0x00200000
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x4000)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x2000
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/*
 * SDRAM controller configuration
 */
#undef CONFIG_SDR_MT48LC16M16A2
#undef CONFIG_DDR_MT46V16M16
#undef CONFIG_DDR_MT46V32M16
#undef CONFIG_DDR_HYB25D512160BF
#define CONFIG_DDR_K4H511638C

/* Use ON-Chip SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM

/* preserve space for the post_word at end of on-chip SRAM */
#define MPC5XXX_SRAM_POST_SIZE (MPC5XXX_SRAM_SIZE - 4)

#ifdef CONFIG_POST
#define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_POST_SIZE
#else
#define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_SIZE
#endif

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
/*
 * Define CONFIG_MPC5xxx_FEC_MII10 to force FEC at 10Mb
 */
/* #define CONFIG_MPC5xxx_FEC_MII10 */
#define CONFIG_PHY_ADDR		0x00
#define CONFIG_MII

/*
 * GPIO configuration
 *
 * use CS1 as gpio_wkup_6 output
 *	Bit 0 (mask: 0x80000000): 0
 * use ALT CAN position: Bits 2-3 (mask: 0x30000000):
 *	00 -> No Alternatives, I2C1 is used for onboard EEPROM
 *	01 -> CAN1 on I2C1, CAN2 on Tmr0/1 do not use on TQM5200 with onboard
 *	      EEPROM
 * use PSC1 as UART: Bits 28-31 (mask: 0x00000007): 0100
 * use PSC2 as UART: Bits 24-27 (mask: 0x00000070): 0100
 * use PSC3 as UART: Bits 20-23 (mask: 0x00000700): 0100
 * use PSC6 as UART: Bits  9-11 (mask: 0x00700000): 0101
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x01501444

/*
 * RTC configuration
 */
#define CONFIG_RTC_RTC4543 	1	/* use external RTC */

/*
 * Software (bit-bang) three wire serial configuration
 *
 * Note that we need the ifdefs because otherwise compilation of
 * mkimage.c fails.
 */
#define CONFIG_SOFT_TWS		1

#ifdef TWS_IMPLEMENTATION
#include <mpc5xxx.h>
#include <asm/io.h>

#define TWS_CE		MPC5XXX_GPIO_WKUP_PSC1_4 /* GPIO_WKUP_0 */
#define TWS_WR		MPC5XXX_GPIO_WKUP_PSC2_4 /* GPIO_WKUP_1 */
#define TWS_DATA	MPC5XXX_GPIO_SINT_PSC3_4 /* GPIO_SINT_0 */
#define TWS_CLK		MPC5XXX_GPIO_SINT_PSC3_5 /* GPIO_SINT_1 */

static inline void tws_ce(unsigned bit)
{
	struct mpc5xxx_wu_gpio *wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;
	if (bit)
		setbits_8(&wu_gpio->dvo, TWS_CE);
	else
		clrbits_8(&wu_gpio->dvo, TWS_CE);
}

static inline void tws_wr(unsigned bit)
{
	struct mpc5xxx_wu_gpio *wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;
	if (bit)
		setbits_8(&wu_gpio->dvo, TWS_WR);
	else
		clrbits_8(&wu_gpio->dvo, TWS_WR);
}

static inline void tws_clk(unsigned bit)
{
	struct mpc5xxx_gpio *gpio =
		(struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	if (bit)
		setbits_8(&gpio->sint_dvo, TWS_CLK);
	else
		clrbits_8(&gpio->sint_dvo, TWS_CLK);
}

static inline void tws_data(unsigned bit)
{
	struct mpc5xxx_gpio *gpio =
		(struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	if (bit)
		setbits_8(&gpio->sint_dvo, TWS_DATA);
	else
		clrbits_8(&gpio->sint_dvo, TWS_DATA);
}

static inline unsigned tws_data_read(void)
{
	struct mpc5xxx_gpio *gpio =
			(struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	return !!(in_8(&gpio->sint_ival) & TWS_DATA);
}

static inline void tws_data_config_output(unsigned output)
{
	struct mpc5xxx_gpio *gpio =
		(struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	if (output)
		setbits_8(&gpio->sint_ddr, TWS_DATA);
	else
		clrbits_8(&gpio->sint_ddr, TWS_DATA);
}
#endif /* TWS_IMPLEMENTATION */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/* Enable an alternate, more extensive memory test */
#define CONFIG_SYS_ALT_MEMTEST

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Enable loopw command.
 */
#define CONFIG_LOOPW

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x00087800 /* for pci_clk  = 66 MHz */
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

/* 32Mbit SRAM @0x30000000 */
#define CONFIG_SYS_CS1_START		0x30000000
#define CONFIG_SYS_CS1_SIZE		0x00400000
#define CONFIG_SYS_CS1_CFG		0x31800 /* for pci_clk = 33 MHz */

/* 2 quad UART @0x80000000 (MBAR is relocated to 0xF0000000) */
#define CONFIG_SYS_CS2_START		0x80000000
#define CONFIG_SYS_CS2_SIZE		0x0001000
#define CONFIG_SYS_CS2_CFG		0x21800  /* for pci_clk = 33 MHz */

/* GPIO in @0x30400000 */
#define CONFIG_SYS_CS3_START		0x30400000
#define CONFIG_SYS_CS3_SIZE		0x00100000
#define CONFIG_SYS_CS3_CFG		0x31800 /* for pci_clk = 33 MHz */

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333333

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_OHCI
#define CONFIG_USB_CLOCK	0x00015555
#define CONFIG_USB_CONFIG	0x00001000
#define CONFIG_USB_STORAGE

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA
#define CONFIG_SYS_ATA_DATA_OFFSET	0x0060	/* Offset for data I/O		*/
#define CONFIG_SYS_ATA_REG_OFFSET (CONFIG_SYS_ATA_DATA_OFFSET) /* Offset for normal register accesses */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x005C	/* Offset for alternate registers */
#define CONFIG_SYS_ATA_STRIDE          4	/* Interval between registers	*/

#define CONFIG_ATAPI            1

#define CONFIG_SYS_BRIGHTNESS          0xFF	/* LCD Default Brightness (255 = off) */

#endif /* __CONFIG_H */
