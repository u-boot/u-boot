/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU) */
#define CONFIG_MPC5200_DDR	1	/* (with DDR-SDRAM) */
#define CONFIG_MUNICES		1	/* ... on MUNICes board */
#define CFG_MPC5XXX_CLKIN	33333333 /* ... running at 33.333333MHz */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot	     */
#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */
#define CONFIG_BOOTDELAY	5   /* autoboot after 5 seconds */
#undef	CONFIG_BOOTARGS

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run net_nfs\" to load Kernel over TFTP and to mount root filesystem over NFS;" \
	"echo"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=$(serverip):$(rootpath)\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs $(bootargs) "				\
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"	\
		":$(hostname):$(netdev):off panic=5\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm $(kernel_addr)\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm $(kernel_addr) $(ramdisk_addr)\0"		\
	"net_nfs=tftp 200000 $(bootfile);run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"bootfile=/tftpboot/munices/u-boot.bin\0"			\
	"update=tftpboot 200000 ${bootfile};protect off fff00000 fff3ffff;" \
	"erase fff00000 fff3ffff; cp.b 200000 FFF00000 ${filesize}\0"	\
	""
#define CONFIG_BOOTCOMMAND	"run net_nfs"

/*
 * IPB Bus clocking configuration.
 */
#define  CFG_IPBSPEED_133		/* define for 133MHz speed */
#if defined(CFG_IPBSPEED_133)
/*
 * PCI Bus clocking configuration
 *
 * Actually a PCI Clock of 66 MHz is only set (in cpu_init.c) if
 * CFG_IPBSPEED_133 is defined. This is because a PCI Clock of 66 MHz yet hasn't
 * been tested with a IPB Bus Clock of 66 MHz.
 */
#define CFG_PCISPEED_66		/* define for 66MHz speed */
#else
#undef CFG_PCISPEED_66			/* for 33MHz speed */
#endif

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000 /* MBAR hast to be switched by other bootloader or debugger config  */

#define CFG_DEFAULT_MBAR	0x80000000
#define CFG_SDRAM_BASE		0x00000000
/* Use SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE	/* End of used area in DPRAM */
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Flash configuration
 */
#define CFG_FLASH_BASE		0xFF000000
#define CFG_FLASH_CFI		1	/* Flash is CFI conformant */
#define CFG_FLASH_CFI_DRIVER	1	/* Use the common driver */
//#define CFG_FLASH_BANKS_LIST	{ CFG_BOOTCS_START }
#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }
#define CFG_FLASH_EMPTY_INFO
#define CFG_FLASH_SIZE		0x01000000 /* 16 MByte */
#define CFG_MAX_FLASH_SECT	128	/* max num of sects on one chip */
#define CFG_MAX_FLASH_BANKS	1	 /* max num of flash banks (= chip selects) */
#define CFG_FLASH_USE_BUFFER_WRITE	/* not supported yet for AMD */

/*
 * Chip selects configuration
 */
/* Boot Chipselect */
#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00047800

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x40000
#define CFG_ENV_ADDR		(TEXT_BASE + CFG_ENV_OFFSET)
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_SIZE		0x4000
#define CFG_ENV_OFFSET_REDUND   (CFG_ENV_OFFSET + CFG_ENV_SECT_SIZE)
#define CFG_ENV_ADDR_REDUND	(TEXT_BASE + CFG_ENV_OFFSET_REDUND)
#define CFG_ENV_SIZE_REDUND     (CFG_ENV_SIZE)
#define CONFIG_ENV_OVERWRITE	1

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_PHY_ADDR		0x01
#define CONFIG_MII		1

/*
 * GPIO configuration
 */
#define CFG_GPS_PORT_CONFIG	0x00058044 /* PSC1=UART, PSC2=UART ; Ether=100MBit with MD
						no PCI */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x200000	/* default load address */
#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_DISPLAY_BOARDINFO 1
#define CONFIG_CMDLINE_EDITING  1

/*
 * Various low-level settings
 */
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333
#define CFG_RESET_ADDRESS	0xff000000

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_SOC                  "soc5200@f0000000"
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

#endif /* __CONFIG_H */
