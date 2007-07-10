/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#define MV_VERSION	"v0.2.0"

/* LED0 = Power , LED1 = Error , LED2-5 = error code, LED6-7=00 -->PPCBoot error */
#define ERR_NONE			0
#define ERR_ENV 			1
#define ERR_BOOTM_BADMAGIC 	2
#define ERR_BOOTM_BADCRC   	3
#define ERR_BOOTM_GUNZIP   	4
#define ERR_BOOTP_TIMEOUT	5
#define ERR_DHCP			6
#define ERR_TFTP			7
#define ERR_NOLAN 			8
#define ERR_LANDRV 			9

#define CONFIG_BOARD_TYPES	1
#define MVBLUE_BOARD_BOX	1
#define MVBLUE_BOARD_LYNX	2

#if 0
#define ERR_LED(code)	do { if (code) \
								*(volatile char *)(0xff000003) = ( 3 | (code<<4) ) & 0xf3; \
							 else \
								*(volatile char *)(0xff000003) = ( 1 ); \
						} while(0)
#else
#define ERR_LED(code)
#endif

#undef DEBUG

#define CONFIG_MPC824X		1
#define CONFIG_MPC8245		1
#define CONFIG_MVBLUE		1

#define CONFIG_CLOCKS_IN_MHZ	1

#define CONFIG_BOARD_TYPES    1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_BOOTDELAY		3
#define CONFIG_BOOT_RETRY_TIME	-1

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT		"autoboot in %d seconds (stop with 's')...\n"
#define CONFIG_AUTOBOOT_STOP_STR	"s"
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_RESET_TO_RETRY		60


/*
 * Command line configuration.
 */

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#define CONFIG_CMD_RUN


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_NISDOMAIN
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_NTPSERVER
#define CONFIG_BOOTP_TIMEOFFSET


/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		/* undef to save memory		*/
#define CFG_PROMPT	"=> "	/* Monitor Command Prompt	*/
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/

#define CFG_PBSIZE		(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_MAXARGS		16			/* Max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_LOAD_ADDR	0x00100000	/* Default load address			*/

#define CONFIG_BOOTCOMMAND  	"run nfsboot"
#define CONFIG_BOOTARGS			"root=/dev/mtdblock5 ro rootfstype=jffs2"

#define CONFIG_NFSBOOTCOMMAND  	"bootp; run nfsargs addcons;bootm"

#define CONFIG_EXTRA_ENV_SETTINGS			\
	"console_nr=0\0"				\
    "dhcp_client_id=mvBOX-XP\0"				\
    "dhcp_vendor-class-identifier=mvBOX\0"		\
    "adminboot=setenv bootargs root=/dev/mtdblock5 rw rootfstype=jffs2;run addcons;bootm ffc00000\0"	\
    "flashboot=setenv bootargs root=/dev/mtdblock5 ro rootfstype=jffs2;run addcons;bootm ffc00000\0"	\
    "safeboot=setenv bootargs root=/dev/mtdblock2 rw rootfstype=cramfs;run addcons;bootm ffc00000\0"	\
    "hdboot=setenv bootargs root=/dev/hda1;run addcons;bootm ffc00000\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
			"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off\0"	\
	"addcons=setenv bootargs ${bootargs} console=ttyS${console_nr},${baudrate}N8\0" \
    "mv_version=" MV_VERSION "\0"	\
	"bootretry=30\0"

#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */

#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW

#define CONFIG_NET_MULTI
#define CONFIG_NET_RETRY_COUNT 5

#define CONFIG_TULIP
#define CONFIG_TULIP_FIX_DAVICOM	1
#define CONFIG_ETHADDR      		b6:b4:45:eb:fb:c0

#define CONFIG_HW_WATCHDOG

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE	    0x00000000

#define CFG_FLASH_BASE      0xFFF00000
#define CFG_MONITOR_BASE    TEXT_BASE

#define CFG_RESET_ADDRESS   0xFFF00100
#define CFG_EUMB_ADDR	    0xFC000000

#define CFG_MONITOR_LEN     0x00100000
#define CFG_MALLOC_LEN      (512 << 10) /* Reserve some kB for malloc()  */

#define CFG_MEMTEST_START   0x00100000	/* memtest works on		*/
#define CFG_MEMTEST_END	    0x00800000	/* 1M ... 8M in DRAM		*/

/* Maximum amount of RAM.  */
#define CFG_MAX_RAM_SIZE    0x10000000	/* 0 .. 256MB of (S)DRAM */


#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
#undef CFG_RAMBOOT
#else
#define CFG_RAMBOOT
#endif

#define CFG_ISA_IO      0xFE000000

/*
 * serial configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL

#define CFG_NS16550_REG_SIZE    1

#define CFG_NS16550_CLK     get_bus_freq(0)

#define CFG_NS16550_COM1    (CFG_EUMB_ADDR + 0x4500)
#define CFG_NS16550_COM2    (CFG_EUMB_ADDR + 0x4600)

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CFG_INIT_RAM_ADDR     0x40000000
#define CFG_INIT_RAM_END      0x1000
#define CFG_GBL_DATA_SIZE     128
#define CFG_GBL_DATA_OFFSET   (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 */

#define CONFIG_SYS_CLK_FREQ  33000000
#define CFG_HZ		     	 10000

/* Bit-field values for MCCR1.  */
#define CFG_ROMNAL      7
#define CFG_ROMFAL      11

/* Bit-field values for MCCR2.  */
#define CFG_TSWAIT      0x5
#define CFG_REFINT      430

/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.  */
#define CFG_BSTOPRE     121

/* Bit-field values for MCCR3.  */
#define CFG_REFREC      8

/* Bit-field values for MCCR4.  */
#define CFG_PRETOACT    3
#define CFG_ACTTOPRE    5
#define CFG_ACTORW      3
#define CFG_SDMODE_CAS_LAT  3
#define CFG_REGISTERD_TYPE_BUFFER 1
#define CFG_EXTROM      1
#define CFG_REGDIMM     0
#define CFG_DBUS_SIZE2  1
#define CFG_SDMODE_WRAP 0

#define CFG_PGMAX       0x32
#define CFG_SDRAM_DSCD  0x20

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
 */

#define CFG_BANK0_START	    0x00000000
#define CFG_BANK0_END	    (CFG_MAX_RAM_SIZE - 1)
#define CFG_BANK0_ENABLE    1
#define CFG_BANK1_START     0x3ff00000
#define CFG_BANK1_END       0x3fffffff
#define CFG_BANK1_ENABLE    0
#define CFG_BANK2_START     0x3ff00000
#define CFG_BANK2_END       0x3fffffff
#define CFG_BANK2_ENABLE    0
#define CFG_BANK3_START     0x3ff00000
#define CFG_BANK3_END       0x3fffffff
#define CFG_BANK3_ENABLE    0
#define CFG_BANK4_START     0x3ff00000
#define CFG_BANK4_END       0x3fffffff
#define CFG_BANK4_ENABLE    0
#define CFG_BANK5_START     0x3ff00000
#define CFG_BANK5_END       0x3fffffff
#define CFG_BANK5_ENABLE    0
#define CFG_BANK6_START     0x3ff00000
#define CFG_BANK6_END       0x3fffffff
#define CFG_BANK6_ENABLE    0
#define CFG_BANK7_START     0x3ff00000
#define CFG_BANK7_END       0x3fffffff
#define CFG_BANK7_ENABLE    0

#define CFG_ODCR	    0xff

#define CFG_IBAT0L  (CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U  (CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT1L  (CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U  (CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

#define CFG_IBAT2L  (0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT2U  (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT3L  (0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT3U  (0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_DBAT0L  CFG_IBAT0L
#define CFG_DBAT0U  CFG_IBAT0U
#define CFG_DBAT1L  CFG_IBAT1L
#define CFG_DBAT1U  CFG_IBAT1U
#define CFG_DBAT2L  CFG_IBAT2L
#define CFG_DBAT2U  CFG_IBAT2U
#define CFG_DBAT3L  CFG_IBAT3L
#define CFG_DBAT3U  CFG_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	    (8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#undef  CFG_FLASH_PROTECTION
#define CFG_MAX_FLASH_BANKS		1	/* Max number of flash banks		*/
#define CFG_MAX_FLASH_SECT		63	/* Max number of sectors per flash	*/

#define CFG_FLASH_ERASE_TOUT	12000
#define CFG_FLASH_WRITE_TOUT	1000


#define CFG_ENV_IS_IN_FLASH

#define CFG_ENV_OFFSET		0x00010000
#define CFG_ENV_SIZE		0x00010000
#define CFG_ENV_SECT_SIZE	0x00010000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#endif	/* __CONFIG_H */
