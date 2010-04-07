/*
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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

/*
 * config for XPedite1000 from XES Inc.
 * Ported from EBONY config by Travis B. Sawyer <tsawyer@sandburst.com>
 * (C) Copyright 2003 Sandburst Corporation
 * board/config_EBONY.h - configuration for AMCC 440GP Ref (Ebony)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_XPEDITE1000	1
#define CONFIG_SYS_BOARD_NAME	"XPedite1000"
#define CONFIG_4xx		1		/* ... PPC4xx family */
#define CONFIG_440		1
#define CONFIG_440GX		1		/* 440 GX */
#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_pre_init	*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* external freq to pll */

/*
 * DDR config
 */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for setup */
#define SPD_EEPROM_ADDRESS	{0x54}	/* SPD i2c spd addresses */
#define CONFIG_VERY_BIG_RAM	1

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xff000000	/* start of FLASH */
#define CONFIG_SYS_MONITOR_BASE		TEXT_BASE	/* start of monitor */
#define CONFIG_SYS_PCI_MEMBASE		0x80000000	/* mapped pci memory */
#define CONFIG_SYS_PERIPHERAL_BASE	0xe0000000	/* internal peripherals */
#define CONFIG_SYS_ISRAM_BASE		0xc0000000	/* internal SRAM */
#define CONFIG_SYS_PCI_BASE		0xd0000000	/* internal PCI regs */
#define CONFIG_SYS_NVRAM_BASE_ADDR	(CONFIG_SYS_PERIPHERAL_BASE + 0x08000000)
#define CONFIG_SYS_GPIO_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x00000700)

/*
 * Diagnostics
 */
#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x0400000
#define CONFIG_SYS_MEMTEST_END		0x0C00000

/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_RTC	| \
				 CONFIG_SYS_POST_I2C)

/*
 * LED support
 */
#define USR_LED0	0x00000080
#define USR_LED1	0x00000100
#define USR_LED2	0x00000200
#define USR_LED3	0x00000400

#ifndef __ASSEMBLY__
extern unsigned long in32(unsigned int);
extern void out32(unsigned int, unsigned long);

#define LED0_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED0))
#define LED1_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED1))
#define LED2_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED2))
#define LED3_ON() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) & ~USR_LED3))

#define LED0_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED0))
#define LED1_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED1))
#define LED2_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED2))
#define LED3_OFF() out32(CONFIG_SYS_GPIO_BASE, (in32(CONFIG_SYS_GPIO_BASE) | USR_LED3))
#endif

/*
 * Use internal SRAM for initial stack
 */
#define CONFIG_SYS_TEMP_STACK_OCM	1
#define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_ISRAM_BASE
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_ISRAM_BASE	/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_END		0x2000	/* End of used area in RAM */
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* num bytes initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_POST_WORD_ADDR	(CONFIG_SYS_GBL_DATA_OFFSET - 0x4)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_POST_WORD_ADDR

#define CONFIG_SYS_MONITOR_LEN	(512 * 1024)	/* Reserve 512 KB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(1024 * 1024)	/* Reserved for malloc */

/*
 * Serial Port
 */
#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400}
#define CONFIG_BAUDRATE			115200
#define CONFIG_LOADS_ECHO		1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * Use the HUSH parser
 */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/*
 * NOR flash configuration
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	3
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE, 0xf0000000, 0xf4000000 }
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* sectors per device */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_FLASH_QUIET_TEST		/* MirrorBit flashes are optional */
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

/*
 * I2C
 */
#define CONFIG_HARD_I2C			1	/* I2C with hardware support */
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7f
#define CONFIG_I2C_MULTI_BUS

/* I2C EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/* I2C RTC: STMicro M41T00 */
#define CONFIG_RTC_M41T11		1
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
#define CONFIG_SYS_M41T11_BASE_YEAR	2000

/*
 * PCI
 */
/* General PCI */
#define CONFIG_PCI				/* include pci support */
#define CONFIG_PCI_PNP				/* do pci plug-and-play */
#define CONFIG_PCI_SCAN_SHOW			/* show pci devices on startup */
#define CONFIG_SYS_PCI_TARGBASE	0x80000000	/* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT		/* let board init pci target */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1014	/* IBM */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */
#define CONFIG_SYS_PCI_FORCE_PCI_CONV		/* Force PCI Conventional Mode */

/*
 * Networking options
 */
#define CONFIG_PPC4xx_EMAC
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_NET_MULTI	1
#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_PHY_RESET	1	/* reset phy upon startup */
#define CONFIG_SYS_RX_ETH_BUFFER 32	/* Number of ethernet rx buffers & descriptors */
#define CONFIG_ETHPRIME		"ppc_4xx_eth2"
#define CONFIG_PHY_ADDR		4	/* PHY address phy0 not populated */
#define CONFIG_PHY2_ADDR	4	/* PHY address phy2 */
#define CONFIG_HAS_ETH2		1	/* add support for "eth2addr" */
#define CONFIG_PHY3_ADDR	8	/* PHY address phy3 */
#define CONFIG_HAS_ETH3		1	/* add support for "eth3addr" */

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command configuration
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SNTP

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */
#define CONFIG_CMDLINE_EDITING	1		/* Command-line editing */
#define CONFIG_BOOTDELAY	3		/* -1 disables auto-boot */
#define CONFIG_PANIC_HANG			/* do not reset board on panic */
#define CONFIG_PREBOOT				/* enable preboot variable */
#define CONFIG_FIT		1
#define CONFIG_FIT_VERBOSE	1
#define CONFIG_INTEGRITY			/* support booting INTEGRITY OS */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_into (bd_t) */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

/*
 * Environment Configuration
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128k (one sector) for env */
#define CONFIG_ENV_SIZE		0x8000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - (256 * 1024))

/*
 * Flash memory map:
 * fff80000 - ffffffff	U-Boot (512 KB)
 * fff40000 - fff7ffff	U-Boot Environment (256 KB)
 * fff00000 - fff3ffff	FDT (256KB)
 * ffc00000 - ffefffff	OS image (3MB)
 * ff000000 - ffbfffff	OS Use/Filesystem (12MB)
 */

#define CONFIG_UBOOT_ENV_ADDR	MK_STR(TEXT_BASE)
#define CONFIG_FDT_ENV_ADDR	MK_STR(0xfff00000)
#define CONFIG_OS_ENV_ADDR	MK_STR(0xffc00000)

#define CONFIG_PROG_UBOOT						\
	"$download_cmd $loadaddr $ubootfile; "				\
	"if test $? -eq 0; then "					\
		"protect off "CONFIG_UBOOT_ENV_ADDR" +80000; "		\
		"erase "CONFIG_UBOOT_ENV_ADDR" +80000; "		\
		"cp.w $loadaddr "CONFIG_UBOOT_ENV_ADDR" 40000; "	\
		"protect on "CONFIG_UBOOT_ENV_ADDR" +80000; "		\
		"cmp.b $loadaddr "CONFIG_UBOOT_ENV_ADDR" 80000; "	\
		"if test $? -ne 0; then "				\
			"echo PROGRAM FAILED; "				\
		"else; "						\
			"echo PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_BOOT_OS_NET						\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"if test -n $fdtaddr; then "				\
			"$download_cmd $fdtaddr $fdtfile; "		\
			"if test $? -eq 0; then "			\
				"bootm $osaddr - $fdtaddr; "		\
			"else; "					\
				"echo FDT DOWNLOAD FAILED; "		\
			"fi; "						\
		"else; "						\
			"bootm $osaddr; "				\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_OS							\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_OS_ENV_ADDR" +$filesize; "		\
		"cp.b $osaddr "CONFIG_OS_ENV_ADDR" $filesize; "		\
		"cmp.b $osaddr "CONFIG_OS_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo OS PROGRAM FAILED; "			\
		"else; "						\
			"echo OS PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_FDT							\
	"$download_cmd $fdtaddr $fdtfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_FDT_ENV_ADDR" +$filesize;"		\
		"cp.b $fdtaddr "CONFIG_FDT_ENV_ADDR" $filesize; "	\
		"cmp.b $fdtaddr "CONFIG_FDT_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo FDT PROGRAM FAILED; "			\
		"else; "						\
			"echo FDT PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo FDT DOWNLOAD FAILED; "				\
	"fi;"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"autoload=yes\0"						\
	"download_cmd=tftp\0"						\
	"console_args=console=ttyS0,115200\0"				\
	"root_args=root=/dev/nfs rw\0"					\
	"misc_args=ip=on\0"						\
	"set_bootargs=setenv bootargs ${console_args} ${root_args} ${misc_args}\0" \
	"bootfile=/home/user/file\0"					\
	"osfile=/home/user/uImage-XPedite1000\0"			\
	"fdtfile=/home/user/xpedite1000.dtb\0"				\
	"ubootfile=/home/user/u-boot.bin\0"				\
	"fdtaddr=c00000\0"						\
	"osaddr=0x1000000\0"						\
	"loadaddr=0x1000000\0"						\
	"prog_uboot="CONFIG_PROG_UBOOT"\0"				\
	"prog_os="CONFIG_PROG_OS"\0"					\
	"prog_fdt="CONFIG_PROG_FDT"\0"					\
	"bootcmd_net=run set_bootargs; "CONFIG_BOOT_OS_NET"\0"		\
	"bootcmd_flash=run set_bootargs; "				\
		"bootm "CONFIG_OS_ENV_ADDR" - "CONFIG_FDT_ENV_ADDR"\0"	\
	"bootcmd=run bootcmd_flash\0"
#endif	/* __CONFIG_H */
