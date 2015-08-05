/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC5200		1	/* This is an MPC5200 CPU */
#define CONFIG_MPC5200_DDR	1	/* (with DDR-SDRAM) */
#define CONFIG_MUNICES		1	/* ... on MUNICes board */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF00000
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN	33333333 /* ... running at 33.333333MHz */
#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/*
 * Command line configuration.
 */
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

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
#define  CONFIG_SYS_IPBSPEED_133		/* define for 133MHz speed */
#if defined(CONFIG_SYS_IPBSPEED_133)
/*
 * PCI Bus clocking configuration
 *
 * Actually a PCI Clock of 66 MHz is only set (in cpu_init.c) if
 * CONFIG_SYS_IPBSPEED_133 is defined. This is because a PCI Clock of 66 MHz yet hasn't
 * been tested with a IPB Bus Clock of 66 MHz.
 */
#define CONFIG_SYS_PCISPEED_66		/* define for 66MHz speed */
#else
#undef CONFIG_SYS_PCISPEED_66			/* for 33MHz speed */
#endif

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000 /* MBAR hast to be switched by other bootloader or debugger config  */

#define CONFIG_SYS_DEFAULT_MBAR	0x80000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_SIZE	/* Size of used area in DPRAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		0xFF000000
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_SIZE		0x01000000 /* 16 MByte */
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	 /* max num of flash banks (= chip selects) */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* not supported yet for AMD */

/*
 * Chip selects configuration
 */
/* Boot Chipselect */
#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x00047800

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_TEXT_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_SIZE		0x4000
#define CONFIG_ENV_OFFSET_REDUND   (CONFIG_ENV_OFFSET + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_TEXT_BASE + CONFIG_ENV_OFFSET_REDUND)
#define CONFIG_ENV_SIZE_REDUND     (CONFIG_ENV_SIZE)
#define CONFIG_ENV_OVERWRITE	1

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR		0x01
#define CONFIG_MII		1

/*
 * GPIO configuration
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x00058044 /* PSC1=UART, PSC2=UART ; Ether=100MBit with MD
						no PCI */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x200000	/* default load address */

#define CONFIG_DISPLAY_BOARDINFO 1
#define CONFIG_CMDLINE_EDITING  1

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333333
#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_SOC                  "soc5200@f0000000"
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

#endif /* __CONFIG_H */
