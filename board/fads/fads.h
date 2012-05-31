/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Derived from FADS860T definitions by Magnus Damm, Helmut Buchsbaum,
 * and Dan Malek
 *
 * Modified by, Yuli Barcohen, Arabella Software Ltd., yuli@arabellasw.com
 *
 * This header file contains values common to all FADS family boards.
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

/****************************************************************************
 * Flash Memory Map as used by U-Boot:
 *
 *                          Start Address    Length
 * +-----------------------+ 0xFE00_0000     Start of Flash -----------------
 * |                       | 0xFE00_0100     Reset Vector
 * +                       + 0xFE0?_????
 * | U-Boot code           |
 * |                       |
 * +-----------------------+ 0xFE04_0000 (sector border)
 * |                       |
 * |                       |
 * | U-Boot environment    |
 * |                       |                                 ^
 * |                       |                                 | U-Boot
 * +=======================+ 0xFE08_0000 (sector border)    -----------------
 * | Available             |                                 | Applications
 * | ...                   |                                 v
 *
 *****************************************************************************/

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_ENV_OVERWRITE

#define CONFIG_NFSBOOTCOMMAND							\
    "dhcp;"									\
    "setenv bootargs root=/dev/nfs rw nfsroot=$rootpath "			\
    "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:eth0:off;"		\
    "bootm"

#define CONFIG_BOOTCOMMAND							\
    "setenv bootargs root=/dev/mtdblock2 rw mtdparts=phys:1280K(ROM)ro,-(root) "\
    "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:eth0:off;"		\
    "bootm fe080000"

#undef CONFIG_BOOTARGS

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#if !defined(CONFIG_MPC885ADS)
#define CONFIG_BZIP2	 /* include support for bzip2 compressed images */
#endif

/*
 * New MPC86xADS and MPC885ADS provide two Ethernet connectivity options:
 * 10Mbit/s on SCC and 100Mbit/s on FEC. FADS provides SCC Ethernet on
 * motherboard and FEC Ethernet on daughterboard. All new PQ1 chips have
 * got FEC so FEC is the default.
 */
#ifndef CONFIG_ADS
#undef	CONFIG_SCC1_ENET		/* Disable SCC1 ethernet */
#define	CONFIG_FEC_ENET			/* Use FEC ethernet  */
#else					/* Old ADS has not got FEC option */
#define	CONFIG_SCC1_ENET		/* Use SCC1 ethernet */
#undef	CONFIG_FEC_ENET			/* No FEC ethernet  */
#endif /* !CONFIG_ADS */

#if defined(CONFIG_SCC1_ENET) && defined(CONFIG_FEC_ENET)
#error Both CONFIG_SCC1_ENET and CONFIG_FEC_ENET configured
#endif

#ifdef CONFIG_FEC_ENET
#define CONFIG_SYS_DISCOVER_PHY
#define CONFIG_MII_INIT		1
#endif


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


#if !defined(FADS_COMMANDS_ALREADY_DEFINED)
/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCMCIA
#define CONFIG_CMD_PING

#endif


/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_PROMPT		"=>"		/* Monitor Command Prompt	*/
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define	CONFIG_SYS_LONGHELP				/* #undef to save memory	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16) /* Print Buffer Size	*/
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define	CONFIG_SYS_HZ		        1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#if defined(CONFIG_MPC86xADS) || defined(CONFIG_MPC885ADS) /* New ADS or Duet */
#define	CONFIG_SYS_SDRAM_SIZE		0x00800000	/* 8 Mbyte */
/*
 * 2048	SDRAM rows
 * 1000	factor s -> ms
 * 64	PTP (pre-divider from MPTPR) from SDRAM example configuration
 * 4	Number of refresh cycles per period
 * 64	Refresh cycle in ms per number of rows
 */
#define CONFIG_SYS_PTA_PER_CLK		((2048 * 64 * 1000) / (4 * 64))
#elif defined(CONFIG_FADS)				/* Old/new FADS */
#define	CONFIG_SYS_SDRAM_SIZE		0x00400000		/* 4 Mbyte */
#else							/* Old ADS */
#define	CONFIG_SYS_SDRAM_SIZE		0x00000000		/* No SDRAM */
#endif

#define CONFIG_SYS_MEMTEST_START	0x0100000	/* memtest works on	*/
#if (CONFIG_SYS_SDRAM_SIZE)
#define CONFIG_SYS_MEMTEST_END		CONFIG_SYS_SDRAM_SIZE	/* 1 ... SDRAM_SIZE	*/
#else
#define CONFIG_SYS_MEMTEST_END		0x0400000	/* 1 ... 4 MB in DRAM	*/
#endif /* CONFIG_SYS_SDRAM_SIZE */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 KB for monitor	*/

#ifdef CONFIG_BZIP2
#define	CONFIG_SYS_MALLOC_LEN		(2500 << 10)	/* Reserve ~2.5 MB for malloc()	*/
#else
#define	CONFIG_SYS_MALLOC_LEN		(384 << 10)	/* Reserve 384 kB for malloc()	*/
#endif /* CONFIG_BZIP2 */

/*-----------------------------------------------------------------------
 * Flash organization
 */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_FLASH_SIZE		((uint)(8 * 1024 * 1024))	/* max 8Mbyte	*/

#define CONFIG_SYS_MAX_FLASH_BANKS	4	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	8	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	0x40000	/* see README - env sector total size	*/
#define CONFIG_ENV_OFFSET		CONFIG_ENV_SECT_SIZE
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment		*/
#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

#define	CONFIG_SYS_DIRECT_FLASH_TFTP

#if defined(CONFIG_CMD_JFFS2)

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=fads0,nor1=fads-1,nor2=fads-2,nor3=fads-3"
#define MTDPARTS_DEFAULT	"mtdparts=fads-0:-@1m(user1),fads-1:-(user2),fads-2:-(user3),fads-3:-(user4)"
*/

#define CONFIG_SYS_JFFS2_SORT_FRAGMENTS
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/

/*-----------------------------------------------------------------------
 * I2C configuration
 */
#if defined(CONFIG_CMD_I2C)
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address defaults */
#define CONFIG_SYS_I2C_SLAVE		0x7F
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	SCCR_TBS

/*-----------------------------------------------------------------------
 * DER - Debug Enable Register
 *-----------------------------------------------------------------------
 * Set to zero to prevent the processor from entering debug mode
 */
#define CONFIG_SYS_DER		 0

/* Because of the way the 860 starts up and assigns CS0 the entire
 * address space, we have to set the memory controller differently.
 * Normally, you write the option register first, and then enable the
 * chip select by writing the base register.  For CS0, you must write
 * the base register first, followed by the option register.
 */

/*
 * Init Memory Controller:
 *
 * BR0/OR0 (Flash)
 * BR1/OR1 (BCSR)
 */
/* the other CS:s are determined by looking at parameters in BCSRx */

#define BCSR_ADDR		((uint) 0xFF080000)

#define CONFIG_SYS_PRELIM_OR_AM	0xFF800000	/* OR addr mask */

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 0 */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_3_CLK | OR_TRLX)

#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)   /* 8 Mbyte until detected */
#define CONFIG_SYS_BR0_PRELIM	((CONFIG_SYS_FLASH_BASE & BR_BA_MSK) | BR_V )

/* BCSRx - Board Control and Status Registers */
#define CONFIG_SYS_OR1_PRELIM	0xFFFF8110		/* 64Kbyte address space */
#define CONFIG_SYS_BR1_PRELIM	((BCSR_ADDR) | BR_V)

/* values according to the manual */

#define	BCSR0			((uint) (BCSR_ADDR + 0x00))
#define	BCSR1			((uint) (BCSR_ADDR + 0x04))
#define	BCSR2			((uint) (BCSR_ADDR + 0x08))
#define	BCSR3			((uint) (BCSR_ADDR + 0x0c))
#define	BCSR4			((uint) (BCSR_ADDR + 0x10))

/*
 * (F)ADS bitvalues by Helmut Buchsbaum
 *
 * See User's Manual for a proper
 * description of the following structures
 */

#define BCSR0_ERB       ((uint)0x80000000)
#define BCSR0_IP        ((uint)0x40000000)
#define BCSR0_BDIS      ((uint)0x10000000)
#define BCSR0_BPS_MASK  ((uint)0x0C000000)
#define BCSR0_ISB_MASK  ((uint)0x01800000)
#define BCSR0_DBGC_MASK ((uint)0x00600000)
#define BCSR0_DBPC_MASK ((uint)0x00180000)
#define BCSR0_EBDF_MASK ((uint)0x00060000)

#define BCSR1_FLASH_EN           ((uint)0x80000000)
#define BCSR1_DRAM_EN            ((uint)0x40000000)
#define BCSR1_ETHEN              ((uint)0x20000000)
#define BCSR1_IRDEN              ((uint)0x10000000)
#define BCSR1_FLASH_CFG_EN       ((uint)0x08000000)
#define BCSR1_CNT_REG_EN_PROTECT ((uint)0x04000000)
#define BCSR1_BCSR_EN            ((uint)0x02000000)
#define BCSR1_RS232EN_1          ((uint)0x01000000)
#define BCSR1_PCCEN              ((uint)0x00800000)
#define BCSR1_PCCVCC0            ((uint)0x00400000)
#define BCSR1_PCCVPP_MASK        ((uint)0x00300000)
#define BCSR1_DRAM_HALF_WORD     ((uint)0x00080000)
#define BCSR1_RS232EN_2          ((uint)0x00040000)
#define BCSR1_SDRAM_EN           ((uint)0x00020000)
#define BCSR1_PCCVCC1            ((uint)0x00010000)

#define BCSR1_PCCVCCON		 BCSR1_PCCVCC0

#define BCSR2_FLASH_PD_MASK      ((uint)0xF0000000)
#define BCSR2_FLASH_PD_SHIFT     28
#define BCSR2_DRAM_PD_MASK       ((uint)0x07800000)
#define BCSR2_DRAM_PD_SHIFT      23
#define BCSR2_EXTTOLI_MASK       ((uint)0x00780000)
#define BCSR2_DBREVNR_MASK       ((uint)0x00030000)

#define BCSR3_DBID_MASK          ((ushort)0x3800)
#define BCSR3_CNT_REG_EN_PROTECT ((ushort)0x0400)
#define BCSR3_BREVNR0            ((ushort)0x0080)
#define BCSR3_FLASH_PD_MASK      ((ushort)0x0070)
#define BCSR3_BREVN1             ((ushort)0x0008)
#define BCSR3_BREVN2_MASK        ((ushort)0x0003)

#define BCSR4_ETHLOOP            ((uint)0x80000000)
#define BCSR4_TFPLDL             ((uint)0x40000000)
#define BCSR4_TPSQEL             ((uint)0x20000000)
#define BCSR4_SIGNAL_LAMP        ((uint)0x10000000)
#if defined(CONFIG_MPC823)
#define BCSR4_USB_EN             ((uint)0x08000000)
#define BCSR4_USB_SPEED          ((uint)0x04000000)
#define BCSR4_VCCO               ((uint)0x02000000)
#define BCSR4_VIDEO_ON           ((uint)0x00800000)
#define BCSR4_VDO_EKT_CLK_EN     ((uint)0x00400000)
#define BCSR4_VIDEO_RST          ((uint)0x00200000)
#define BCSR4_MODEM_EN           ((uint)0x00100000)
#define BCSR4_DATA_VOICE         ((uint)0x00080000)
#elif defined(CONFIG_MPC850)
#define BCSR4_DATA_VOICE         ((uint)0x00080000)
#elif defined(CONFIG_MPC860SAR)
#define BCSR4_UTOPIA_EN          ((uint)0x08000000)
#else /* MPC860T and other chips with FEC */
#define BCSR4_FETH_EN            ((uint)0x08000000)
#define BCSR4_FETHCFG0           ((uint)0x04000000)
#define BCSR4_FETHFDE            ((uint)0x02000000)
#define BCSR4_FETHCFG1           ((uint)0x00400000)
#define BCSR4_FETHRST            ((uint)0x00200000)
#endif

/* BSCR5 exists on MPC86xADS and MPC885ADS only */

#define CONFIG_SYS_PHYDEV_ADDR		(BCSR_ADDR + 0x20000)

#define BCSR5			(CONFIG_SYS_PHYDEV_ADDR + 0x300)

#define BCSR5_MII2_EN		0x40
#define BCSR5_MII2_RST		0x20
#define BCSR5_T1_RST		0x10
#define BCSR5_ATM155_RST	0x08
#define BCSR5_ATM25_RST		0x04
#define BCSR5_MII1_EN		0x02
#define BCSR5_MII1_RST		0x01

/* We don't use the 8259.
*/
#define NR_8259_INTS	0

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_PCMCIA_MEM_ADDR	(0xE0000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR	(0xE4000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_IO_ADDR	(0xEC000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_MAC_PARTITION    1
#define CONFIG_DOS_PARTITION    1
#define CONFIG_ISO_PARTITION	1

#undef	CONFIG_ATAPI
#if 0	/* does not make sense when CONFIG_CMD_IDE is not enabled, too */
#define CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card Adapter */
#endif
#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#undef	CONFIG_IDE_LED			/* LED	 for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 2 IDE busses	*/
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_PCMCIA_MEM_ADDR
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)
/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(2 * CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)
/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0000

#define CONFIG_DISK_SPINUP_TIME 1000000
/* #undef CONFIG_DISK_SPINUP_TIME */	/* usin  Compact Flash */
