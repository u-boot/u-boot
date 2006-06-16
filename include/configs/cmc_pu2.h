/*
 * 2004-2005 Gary Jennejohn <garyj@denx.de>
 *
 * Configuration settings for the CMC PU2 board.
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

/* ARM asynchronous clock */
#define AT91C_MAIN_CLOCK	179712000	/* from 18.432 MHz crystal (18432000 / 4 * 39) */
#define AT91C_MASTER_CLOCK	(AT91C_MAIN_CLOCK/3)	/* peripheral clock */

#define AT91_SLOW_CLOCK		32768	/* slow clock */

#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define CONFIG_AT91RM9200	1	/* It's an Atmel AT91RM9200 SoC	*/
#define CONFIG_CMC_PU2		1	/* on an CMC_PU2 Board		*/
#undef  CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/
#define USE_920T_MMU		1

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#define CFG_USE_MAIN_OSCILLATOR		1
/* flash */
#define MC_PUIA_VAL	0x00000000
#define MC_PUP_VAL	0x00000000
#define MC_PUER_VAL	0x00000000
#define MC_ASR_VAL	0x00000000
#define MC_AASR_VAL	0x00000000
#define EBI_CFGR_VAL	0x00000000
#define SMC2_CSR_VAL	0x100032ad /* 16bit, 2 TDF, 4 WS */

/* clocks */
#define PLLAR_VAL	0x2026BE04 /* 179,712 MHz for PCK */
#define PLLBR_VAL	0x10483E0E /* 48.054857 MHz (divider by 2 for USB) */
#define MCKR_VAL	0x00000202 /* PCK/3 = MCK Master Clock = 69.120MHz from PLLA */

/* sdram */
#define PIOC_ASR_VAL	0xFFFF0000 /* Configure PIOC as peripheral (D16/D31) */
#define PIOC_BSR_VAL	0x00000000
#define PIOC_PDR_VAL	0xFFFF0000
#define EBI_CSA_VAL	0x00000002 /* CS1=SDRAM */
#define SDRC_CR_VAL	0x3399c1d4 /* set up the SDRAM */
#define SDRAM		0x20000000 /* address of the SDRAM */
#define SDRAM1		0x20000080 /* address of the SDRAM */
#define SDRAM_VAL	0x00000000 /* value written to SDRAM */
#define SDRC_MR_VAL	0x00000002 /* Precharge All */
#define SDRC_MR_VAL1	0x00000004 /* refresh */
#define SDRC_MR_VAL2	0x00000003 /* Load Mode Register */
#define SDRC_MR_VAL3	0x00000000 /* Normal Mode */
#define SDRC_TR_VAL	0x000002E0 /* Write refresh rate */
#endif	/* CONFIG_SKIP_LOWLEVEL_INIT */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_BAUDRATE		9600

/*
 * Hardware drivers
 */

/* define one of these to choose the DBGU, USART0  or USART1 as console */
#undef CONFIG_DBGU
#define CONFIG_USART0
#undef CONFIG_USART1

#undef	CONFIG_HWFLOW			/* don't include RTS/CTS flow control support	*/

#undef	CONFIG_MODEM_SUPPORT		/* disable modem initialization stuff */

#define CONFIG_HARD_I2C

#ifdef CONFIG_HARD_I2C
#define CFG_I2C_SPEED		0	/* not used */
#define CFG_I2C_SLAVE		0	/* not used */
#define CONFIG_RTC_RS5C372A		/* RICOH I2C RTC */
#define CFG_I2C_RTC_ADDR	0x32
#define CFG_I2C_EEPROM_ADDR	0x50
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_I2C_EEPROM_ADDR_OVERFLOW
#endif
/* still about 20 kB free with this defined */
#define CFG_LONGHELP

#define CONFIG_BOOTDELAY      1

#ifdef CONFIG_HARD_I2C
#define CONFIG_COMMANDS		\
		       ((CONFIG_CMD_DFL	| \
			CFG_CMD_DATE	| \
			CFG_CMD_DHCP 	| \
			CFG_CMD_EEPROM	| \
			CFG_CMD_I2C	| \
			CFG_CMD_NFS	| \
			CFG_CMD_SNTP  ) & \
		      ~(CFG_CMD_FPGA | CFG_CMD_MISC) )
#else
#define CONFIG_COMMANDS		\
		       ((CONFIG_CMD_DFL	| \
			CFG_CMD_DHCP 	| \
			CFG_CMD_NFS	| \
			CFG_CMD_SNTP  ) & \
		      ~(CFG_CMD_FPGA | CFG_CMD_MISC) )
#define CONFIG_TIMESTAMP
#endif
#define CFG_LONGHELP

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define AT91_SMART_MEDIA_ALE	(1 << 22)	/* our ALE is AD22 */
#define AT91_SMART_MEDIA_CLE	(1 << 21)	/* our CLE is AD21 */

#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM		0x20000000
#define PHYS_SDRAM_SIZE		0x1000000 	/* 16 megs */

#define CFG_MEMTEST_START	PHYS_SDRAM
#define CFG_MEMTEST_END		CFG_MEMTEST_START + PHYS_SDRAM_SIZE - 262144

#define CONFIG_DRIVER_ETHER
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_AT91C_USE_RMII

#define CFG_SPI_WRITE_TOUT		(5*CFG_HZ)
#define CFG_MAX_DATAFLASH_BANKS		2
#define CFG_MAX_DATAFLASH_PAGES		16384
#define CFG_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* Logical adress for CS0 */
#define CFG_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* Logical adress for CS3 */

#define PHYS_FLASH_1			0x10000000
#define PHYS_FLASH_SIZE			0x800000  /* 8 megs main flash */
#define CFG_FLASH_BASE			PHYS_FLASH_1
#define CFG_MONITOR_BASE		CFG_FLASH_BASE
#define CFG_MAX_FLASH_BANKS		1
#define CFG_MAX_FLASH_SECT		256
#define CFG_FLASH_ERASE_TOUT		(11 * CFG_HZ)	/* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT		( 2 * CFG_HZ)	/* Timeout for Flash Write */

#define CFG_ENV_IS_IN_FLASH		1
#define CFG_ENV_OFFSET			0x20000		/* after u-boot.bin */
#define CFG_ENV_SECT_SIZE		(64 << 10)	/* sectors are 64 kB */
#define CFG_ENV_SIZE			(16 << 10)	/* Use only 16 kB */

#define CFG_LOAD_ADDR		0x21000000  /* default load address */

#define CFG_BAUDRATE_TABLE	{ 115200, 57600, 38400, 19200, 9600 }

#define CFG_PROMPT		"=> "		/* Monitor Command Prompt */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size */
#define CFG_MAXARGS		32		/* max number of command args */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */

#ifndef __ASSEMBLY__
/*-----------------------------------------------------------------------
 * Board specific extension for bd_info
 *
 * This structure is embedded in the global bd_info (bd_t) structure
 * and can be used by the board specific code (eg board/...)
 */

struct bd_info_ext {
	/* helper variable for board environment handling
	 *
	 * env_crc_valid == 0	 =>   uninitialised
	 * env_crc_valid  > 0	 =>   environment crc in flash is valid
	 * env_crc_valid  < 0	 =>   environment crc in flash is invalid
	 */
	int env_crc_valid;
};
#endif	/* __ASSEMBLY__ */

#define CFG_HZ 1000
#define CFG_HZ_CLOCK (AT91C_MASTER_CLOCK/2)	/* AT91C_TC0_CMR is implicitly set to */
						/* AT91C_TC_TIMER_DIV1_CLOCK */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"net_nfs=tftp ${loadaddr} ${bootfile};run nfsargs addip addcons " \
		"addmtd;bootm\0" \
	"nfsargs=setenv bootargs root=/dev/nfs rw " \
		"nfsroot=${serverip}:${rootpath}\0" \
	"net_cramfs=tftp ${loadaddr} ${bootfile}; run flashargs addip " \
		"addcons addmtd; bootm\0" \
	"flash_cramfs=run flashargs addip addcons addmtd; bootm 10030000\0" \
	"flashargs=setenv bootargs root=/dev/mtdblock3 ro\0" \
	"addip=setenv bootargs ${bootargs} ethaddr=${ethaddr} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:" \
		"${hostname}::off\0" \
	"addcons=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"addmtd=setenv bootargs ${bootargs} mtdparts=cmc_pu2:128k(uboot)ro," \
		"64k(environment),768k(linux),4096k(root),-\0" \
	"load=tftp ${loadaddr} ${loadfile}\0" \
	"update=protect off 10000000 1001ffff;erase 10000000 1001ffff; " \
		"cp.b ${loadaddr} 10000000 ${filesize};" \
		"protect on 10000000 1001ffff\0" \
	"updatel=era 10030000 100effff;tftp ${loadaddr} ${bootfile}; " \
		"cp.b ${loadaddr} 10030000 ${filesize}\0" \
	"updatec=era 100f0000 104effff;tftp ${loadaddr} ${cramfsimage}; " \
		"cp.b ${loadaddr} 100f0000 ${filesize}\0" \
	"updatej=era 104f0000 107fffff;tftp ${loadaddr} ${jffsimage}; " \
		"cp.b ${loadaddr} 104f0000 ${filesize}\0" \
	"cramfsimage=cramfs_cmc-pu2.img\0" \
	"jffsimage=jffs2_cmc-pu2.img\0" \
	"loadfile=u-boot_cmc-pu2.bin\0" \
	"bootfile=uImage_cmc-pu2\0" \
	"loadaddr=0x20800000\0" \
	"hostname=CMC-TC-PU2\0" \
	"bootcmd=run dhcp_start;run flash_cramfs\0" \
	"autoload=n\0" \
	"dhcp_start=echo no DHCP\0" \
	"ipaddr=192.168.0.190\0"
#endif	/* __CONFIG_H */
