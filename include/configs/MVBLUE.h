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
#define ERR_NONE		0
#define ERR_ENV			1
#define ERR_BOOTM_BADMAGIC	2
#define ERR_BOOTM_BADCRC	3
#define ERR_BOOTM_GUNZIP	4
#define ERR_BOOTP_TIMEOUT	5
#define ERR_DHCP		6
#define ERR_TFTP		7
#define ERR_NOLAN		8
#define ERR_LANDRV		9

#define CONFIG_BOARD_TYPES	1
#define MVBLUE_BOARD_BOX	1
#define MVBLUE_BOARD_LYNX	2

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000
#define CONFIG_SYS_LDSCRIPT	"board/mvblue/u-boot.lds"

#if 0
#define ERR_LED(code)	do { if (code) \
		*(volatile char *)(0xff000003) = ( 3 | (code<<4) ) & 0xf3; \
	else \
		*(volatile char *)(0xff000003) = ( 1 ); \
} while(0)
#else
#define ERR_LED(code)
#endif

#define CONFIG_MPC824X		1
#define CONFIG_MPC8245		1
#define CONFIG_MVBLUE		1

#define CONFIG_CLOCKS_IN_MHZ	1

#define CONFIG_BOARD_TYPES	1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOT_RETRY_TIME	-1

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT		\
	"autoboot in %d seconds (stop with 's')...\n", bootdelay
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
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
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
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/

#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* Max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR	0x00100000	/* Default load address			*/

#define CONFIG_BOOTCOMMAND	"run nfsboot"
#define CONFIG_BOOTARGS			"root=/dev/mtdblock5 ro rootfstype=jffs2"

#define CONFIG_NFSBOOTCOMMAND	"bootp; run nfsargs addcons;bootm"

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

#define CONFIG_NET_RETRY_COUNT		5

#define CONFIG_TULIP
#define CONFIG_TULIP_FIX_DAVICOM	1
#define CONFIG_ETHADDR			b6:b4:45:eb:fb:c0

#define CONFIG_HW_WATCHDOG

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE	    0x00000000

#define CONFIG_SYS_FLASH_BASE      0xFFF00000
#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_RESET_ADDRESS   0xFFF00100
#define CONFIG_SYS_EUMB_ADDR	    0xFC000000

#define CONFIG_SYS_MONITOR_LEN     0x00100000
#define CONFIG_SYS_MALLOC_LEN      (512 << 10) /* Reserve some kB for malloc()  */

#define CONFIG_SYS_MEMTEST_START   0x00100000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END	    0x00800000	/* 1M ... 8M in DRAM		*/

/* Maximum amount of RAM.  */
#define CONFIG_SYS_MAX_RAM_SIZE    0x10000000	/* 0 .. 256MB of (S)DRAM */


#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
#undef CONFIG_SYS_RAMBOOT
#else
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_ISA_IO      0xFE000000

/*
 * serial configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_SYS_NS16550_REG_SIZE    1

#define CONFIG_SYS_NS16550_CLK     get_bus_freq(0)

#define CONFIG_SYS_NS16550_COM1    (CONFIG_SYS_EUMB_ADDR + 0x4500)
#define CONFIG_SYS_NS16550_COM2    (CONFIG_SYS_EUMB_ADDR + 0x4600)

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_RAM_ADDR     0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE      0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET   (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 */

#define CONFIG_SYS_CLK_FREQ  33000000
#define CONFIG_SYS_HZ			 10000

/* Bit-field values for MCCR1.  */
#define CONFIG_SYS_ROMNAL      7
#define CONFIG_SYS_ROMFAL      11

/* Bit-field values for MCCR2.  */
#define CONFIG_SYS_TSWAIT      0x5
#define CONFIG_SYS_REFINT      430

/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.  */
#define CONFIG_SYS_BSTOPRE     121

/* Bit-field values for MCCR3.  */
#define CONFIG_SYS_REFREC      8

/* Bit-field values for MCCR4.  */
#define CONFIG_SYS_PRETOACT    3
#define CONFIG_SYS_ACTTOPRE    5
#define CONFIG_SYS_ACTORW      3
#define CONFIG_SYS_SDMODE_CAS_LAT  3
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER 1
#define CONFIG_SYS_EXTROM      1
#define CONFIG_SYS_REGDIMM     0
#define CONFIG_SYS_DBUS_SIZE2  1
#define CONFIG_SYS_SDMODE_WRAP 0

#define CONFIG_SYS_PGMAX       0x32
#define CONFIG_SYS_SDRAM_DSCD  0x20

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
 */

#define CONFIG_SYS_BANK0_START	    0x00000000
#define CONFIG_SYS_BANK0_END	    (CONFIG_SYS_MAX_RAM_SIZE - 1)
#define CONFIG_SYS_BANK0_ENABLE    1
#define CONFIG_SYS_BANK1_START     0x3ff00000
#define CONFIG_SYS_BANK1_END       0x3fffffff
#define CONFIG_SYS_BANK1_ENABLE    0
#define CONFIG_SYS_BANK2_START     0x3ff00000
#define CONFIG_SYS_BANK2_END       0x3fffffff
#define CONFIG_SYS_BANK2_ENABLE    0
#define CONFIG_SYS_BANK3_START     0x3ff00000
#define CONFIG_SYS_BANK3_END       0x3fffffff
#define CONFIG_SYS_BANK3_ENABLE    0
#define CONFIG_SYS_BANK4_START     0x3ff00000
#define CONFIG_SYS_BANK4_END       0x3fffffff
#define CONFIG_SYS_BANK4_ENABLE    0
#define CONFIG_SYS_BANK5_START     0x3ff00000
#define CONFIG_SYS_BANK5_END       0x3fffffff
#define CONFIG_SYS_BANK5_ENABLE    0
#define CONFIG_SYS_BANK6_START     0x3ff00000
#define CONFIG_SYS_BANK6_END       0x3fffffff
#define CONFIG_SYS_BANK6_ENABLE    0
#define CONFIG_SYS_BANK7_START     0x3ff00000
#define CONFIG_SYS_BANK7_END       0x3fffffff
#define CONFIG_SYS_BANK7_ENABLE    0

#define CONFIG_SYS_ODCR	    0xff

#define CONFIG_SYS_IBAT0L  (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U  (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT1L  (CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U  (CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT2L  (0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U  (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT3L  (0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U  (0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT0L  CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U  CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT1L  CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U  CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT2L  CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U  CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT3L  CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U  CONFIG_SYS_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	    (8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#undef  CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS		1	/* Max number of flash banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT		63	/* Max number of sectors per flash	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	12000
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000


#define CONFIG_ENV_IS_IN_FLASH

#define CONFIG_ENV_OFFSET		0x00010000
#define CONFIG_ENV_SIZE		0x00010000
#define CONFIG_ENV_SECT_SIZE	0x00010000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif
#endif	/* __CONFIG_H */
