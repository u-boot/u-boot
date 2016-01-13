/*
 *  Common configuration options for ifm camera boards
 *
 * (C) Copyright 2005
 * Sebastien Cazaux, ifm electronic gmbh
 *
 * (C) Copyright 2012
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __O2D_CONFIG_H
#define __O2D_CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_MPC5200
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* running at 33.000000MHz */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
/* log base 2 of the above value */
#define CONFIG_SYS_CACHELINE_SHIFT	5
#endif

/*
#define CONFIG_POST	(CONFIG_SYS_POST_MEMORY | \
			 CONFIG_SYS_POST_I2C)
*/

#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define MPC5XXX_SRAM_POST_SIZE	(MPC5XXX_SRAM_SIZE - 4)
#endif

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	5	/* console is on PSC5 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#undef CONFIG_PCI
#define CONFIG_PCI_PNP		1

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

#define CONFIG_TIMESTAMP	/* Print image info with timestamp */

#define CONFIG_SYS_ALT_MEMTEST	/* Much more complex memory test */

/*
 * Supported commands
 */
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#endif
#ifdef CONFIG_POST
#define CONFIG_CMD_DIAG
#endif

#if (CONFIG_SYS_TEXT_BASE == 0xFC000000) || (CONFIG_SYS_TEXT_BASE == 0xFF000000)
/* Boot low with 16 or 32 MB Flash */
#define CONFIG_SYS_LOWBOOT	1
#elif (CONFIG_SYS_TEXT_BASE != 0x00100000)
#error "CONFIG_SYS_TEXT_BASE value is invalid"
#endif

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"run master"

#undef	CONFIG_BOOTARGS

#if !defined(CONFIG_CONSOLE_DEV)
#define CONFIG_CONSOLE_DEV	"ttyPSC1"
#endif

/*
 * Default environment for booting old and new kernel versions
 */
#define CONFIG_IFM_DEFAULT_ENV_OLD					\
	"flash_self_old=run ramargs addip addmem;"			\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs_old=run nfsargs addip addmem;"			\
		"bootm ${kernel_addr}\0"				\
	"net_nfs_old=tftp ${kernel_addr_r} ${bootfile};"		\
		"run nfsargs addip addmem;"				\
		"bootm ${kernel_addr_r}\0"

#define CONFIG_IFM_DEFAULT_ENV_NEW					\
	"fdt_addr_r=900000\0"						\
	"fdt_file="CONFIG_BOARD_NAME"/"CONFIG_BOARD_NAME".dtb\0"	\
	"flash_self=run ramargs addip addtty addmisc;"			\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"flash_nfs=run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\

#define	CONFIG_IFM_DEFAULT_ENV_SETTINGS					\
	"IOpin=0x64\0"							\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addmem=setenv bootargs ${bootargs} ${memlimit}\0"		\
	"addmisc=sete bootargs ${bootargs} ${miscargs}\0"		\
	"addtty=sete bootargs ${bootargs} console="			\
		CONFIG_CONSOLE_DEV ",${baudrate}\0"			\
	"bootfile="CONFIG_BOARD_NAME"/uImage_"CONFIG_BOARD_NAME"_act\0"	\
	"kernel_addr_r=600000\0"					\
	"initrd_high=0x03e00000\0"					\
	"memlimit=mem="CONFIG_BOARD_MEM_LIMIT"M\0"			\
	"memtest=mtest 0x00100000 "__stringify(CONFIG_SYS_MEMTEST_END)" 0 1\0" \
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"linuxname="CONFIG_BOARD_NAME"/uImage_"CONFIG_BOARD_NAME"_act\0"\
	"progLinux=tftp 200000 ${linuxname};erase ${linbot} ${lintop};" \
		"cp.b ${fileaddr} ${linbot} ${filesize}\0"		\
	"ramname="CONFIG_BOARD_NAME"/uRamdisk_"CONFIG_BOARD_NAME"_act\0"\
	"progRam=tftp 200000 ${ramname};erase ${rambot} ${ramtop};"	\
		"cp.b ${fileaddr} ${rambot} ${filesize}\0"		\
	"jffname="CONFIG_BOARD_NAME"/uJFFS2_"CONFIG_BOARD_NAME"_act\0"	\
	"progJff=tftp 200000 ${jffname};erase ${jffbot} ${jfftop};"	\
		"cp.b ${fileaddr} ${jffbot} ${filesize}\0"		\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"uboname=" CONFIG_BOARD_NAME					\
		"/u-boot.bin_" CONFIG_BOARD_NAME "_act\0"		\
	"progubo=tftp 200000 ${uboname};"				\
		"protect off ${ubobot} ${ubotop};"			\
		"erase ${ubobot} ${ubotop};"				\
		"cp.b ${fileaddr} ${ubobot} ${filesize}\0"		\
	"unlock=yes\0"							\
	"post=echo !!! "CONFIG_BOARD_NAME" POWER ON SELF TEST !!!;"	\
		"setenv bootdelay 1;"					\
		"crc32 "__stringify(CONFIG_SYS_TEXT_BASE)" "		\
			BOARD_POST_CRC32_END";"				\
		"setenv bootcmd "CONFIG_BOARD_BOOTCMD";saveenv;reset\0"

#define CONFIG_BOOTCOMMAND	"run post"

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

#if defined(CONFIG_SYS_IPBCLK_EQUALS_XLBCLK)
/*
 * PCI Bus clocking configuration
 *
 * Actually a PCI Clock of 66 MHz is only set (in cpu_init.c) if
 * CONFIG_SYS_IPBCLK_EQUALS_XLBCLK is defined. This is because a PCI Clock
 * of 66 MHz yet hasn't been tested with a IPB Bus Clock of 66 MHz.
 */
#define CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2	/* define for 66MHz speed */
#endif

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C			1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE		1	/* Select I2C module #1 or #2 */
#define CONFIG_SYS_I2C_SPEED		100000	/* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration:
 *
 * O2DNT board is equiped with Ramtron FRAM device FM24CL16
 * 16 Kib Ferroelectric Nonvolatile serial RAM memory
 * organized as 2048 x 8 bits and addressable as eight I2C devices
 * 0x50 ... 0x57 each 256 bytes in size
 *
 */
#define CONFIG_SYS_I2C_FRAM
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
/*
 * There is no write delay with FRAM, write operations are performed at bus
 * speed. Thus, no status polling or write delay is needed.
 */

/*
 * Flash configuration
 */
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define CONFIG_FLASH_16BIT
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_CFI_AMD_RESET
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Erase Timeout (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Write Timeout (in ms) */
/* Timeout for Flash Clear Lock Bits (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	10000
/* "Real" (hardware) sectors protection */
#define CONFIG_SYS_FLASH_PROTECTION

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x20000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00040000)

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE	0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#ifdef CONFIG_POST
/* preserve space for the post_word at end of on-chip SRAM */
#define CONFIG_SYS_INIT_RAM_END		MPC5XXX_SRAM_POST_SIZE
#else
/* End of used area in DPRAM */
#define CONFIG_SYS_INIT_RAM_END		MPC5XXX_SRAM_SIZE
#endif

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 << 10) /* 192 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10) /* 128 kB for malloc() */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)   /* Initial map for Linux */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT		1
#endif

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR			0x00
#define CONFIG_RESET_PHY_R

/*
 * GPIO configuration
 */
#define CONFIG_SYS_GPIO_DATADIR		0x00000064 /* PSC1_2, PSC2_1,2 output */
#define CONFIG_SYS_GPIO_OPENDRAIN	0x00000000 /* No open drain */
#define CONFIG_SYS_GPIO_DATAVALUE	0x00000000 /* PSC1_1 to 1, rest to 0 */
#define CONFIG_SYS_GPIO_ENABLE		0x00000064 /* PSC1_2, PSC2_1,2 enable */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_HUSH_PARSER

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS		16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* default load address */
#define CONFIG_SYS_LOAD_ADDR		0x100000

/* decrementer freq: 1 ms ticks */

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_BOOTCS_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

#define CONFIG_BOARD_EARLY_INIT_R

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE		0x33333333

/*
 * DT support
 */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)

#endif /* __O2D_CONFIG_H */
