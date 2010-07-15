/*
 * CPUAT91 by (C) Copyright 2006 Eric Benard
 * eric@eukrea.com
 *
 * Configuration settings for the CPUAT91 board.
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

#define CONFIG_AT91_LEGACY

#ifdef CONFIG_CPUAT91_RAM
#define CONFIG_SKIP_LOWLEVEL_INIT	1
#define CONFIG_SKIP_RELOCATE_UBOOT	1
#define CONFIG_CPUAT91			1
#else
#define CONFIG_BOOTDELAY		1
#endif

#define AT91C_MAIN_CLOCK		179712000
#define AT91C_MASTER_CLOCK		59904000

#define AT91_SLOW_CLOCK			32768

#define CONFIG_ARM920T			1
#define CONFIG_AT91RM9200		1

#undef CONFIG_USE_IRQ
#define USE_920T_MMU			1

#define CONFIG_CMDLINE_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_USE_MAIN_OSCILLATOR	1
/* flash */
#define CONFIG_SYS_MC_PUIA_VAL	0x00000000
#define CONFIG_SYS_MC_PUP_VAL	0x00000000
#define CONFIG_SYS_MC_PUER_VAL	0x00000000
#define CONFIG_SYS_MC_ASR_VAL	0x00000000
#define CONFIG_SYS_MC_AASR_VAL	0x00000000
#define CONFIG_SYS_EBI_CFGR_VAL	0x00000000
#define CONFIG_SYS_SMC_CSR0_VAL	0x00003284 /* 16bit, 2 TDF, 4 WS */

/* clocks */
#define CONFIG_SYS_PLLAR_VAL	0x20263E04 /* 179.712000 MHz for PCK */
#define CONFIG_SYS_PLLBR_VAL	0x10483E0E /* 48.054857 MHz for USB */
#define CONFIG_SYS_MCKR_VAL	0x00000202 /* PCK/3 = MCK Master Clock */

/* sdram */
#define CONFIG_SYS_PIOC_ASR_VAL	0xFFFF0000 /* Configure PIOC as D16/D31 */
#define CONFIG_SYS_PIOC_BSR_VAL	0x00000000
#define CONFIG_SYS_PIOC_PDR_VAL	0xFFFF0000
#define CONFIG_SYS_EBI_CSA_VAL	0x00000002 /* CS1=SDRAM */
#define CONFIG_SYS_SDRC_CR_VAL	0x2188C155 /* set up the SDRAM */
#define CONFIG_SYS_SDRAM	0x20000000 /* address of the SDRAM */
#define CONFIG_SYS_SDRAM1	0x20000080 /* address of the SDRAM */
#define CONFIG_SYS_SDRAM_VAL	0x00000000 /* value written to SDRAM */
#define CONFIG_SYS_SDRC_MR_VAL	0x00000002 /* Precharge All */
#define CONFIG_SYS_SDRC_MR_VAL1	0x00000004 /* refresh */
#define CONFIG_SYS_SDRC_MR_VAL2	0x00000003 /* Load Mode Register */
#define CONFIG_SYS_SDRC_MR_VAL3	0x00000000 /* Normal Mode */
#define CONFIG_SYS_SDRC_TR_VAL	0x000002E0 /* Write refresh rate */
#endif	/* CONFIG_SKIP_LOWLEVEL_INIT */

/* define one of these to choose the DBGU, USART0 or USART1 as console */
#define CONFIG_AT91RM9200_USART		1
#define CONFIG_DBGU			1
#undef CONFIG_USART0
#undef CONFIG_USART1

#define CONFIG_HARD_I2C			1

#if defined(CONFIG_HARD_I2C)
#define	CONFIG_SYS_I2C_SPEED			50000
#define CONFIG_SYS_I2C_SLAVE			0
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	1
#define	CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10
#endif

#define CONFIG_BOOTP_BOOTFILESIZE	1
#define CONFIG_BOOTP_BOOTPATH		1
#define CONFIG_BOOTP_GATEWAY		1
#define CONFIG_BOOTP_HOSTNAME		1

#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP			1
#define CONFIG_CMD_PING			1
#define CONFIG_CMD_MII			1
#define CONFIG_CMD_CACHE		1
#undef CONFIG_CMD_USB
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NFS

#if defined(CONFIG_HARD_I2C)
#define CONFIG_CMD_EEPROM		1
#define CONFIG_CMD_I2C			1
#endif

#define CONFIG_NR_DRAM_BANKS			1
#define PHYS_SDRAM				0x20000000
#define PHYS_SDRAM_SIZE				0x02000000

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			\
	(CONFIG_SYS_MEMTEST_START + PHYS_SDRAM_SIZE - 512 * 1024)

#define CONFIG_NET_MULTI		1
#define CONFIG_DRIVER_AT91EMAC		1
#define CONFIG_SYS_RX_ETH_BUFFER	8
#define CONFIG_RMII			1
#define CONFIG_MII			1
#define CONFIG_DRIVER_AT91EMAC_PHYADDR	1
#define CONFIG_NET_RETRY_COUNT			20
#define CONFIG_KS8721_PHY			1

#define CONFIG_SYS_FLASH_CFI			1
#define CONFIG_FLASH_CFI_DRIVER			1
#define CONFIG_SYS_FLASH_EMPTY_INFO		1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_FLASH_PROTECTION		1
#define PHYS_FLASH_1				0x10000000
#define CONFIG_SYS_FLASH_BASE			PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT		128

#if defined(CONFIG_CMD_USB)
#define CONFIG_USB_OHCI_NEW			1
#define CONFIG_USB_STORAGE			1
#define CONFIG_DOS_PARTITION			1
#define CONFIG_AT91C_PQFP_UHPBU			1
#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		AT91_USB_HOST_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91rm9200"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#endif

#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_ADDR			(PHYS_FLASH_1 + 0x20000)
#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_SECT_SIZE		0x20000

#define CONFIG_SYS_LOAD_ADDR		0x21000000

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 57600, 38400, 19200, 9600 }

#define CONFIG_SYS_PROMPT		"CPUAT91=> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_PBSIZE		\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_CMDLINE_EDITING		1
#define CONFIG_SYS_LONGHELP		1

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_HZ_CLOCK		(AT91C_MASTER_CLOCK / 2)

#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_STACKSIZE		(32 * 1024)

#if defined(CONFIG_USE_IRQ)
#error CONFIG_USE_IRQ not supported
#endif

#define CONFIG_DEVICE_NULLDEV	 	1
#define CONFIG_SILENT_CONSOLE		1

#define CONFIG_AUTOBOOT_KEYED		1
#define CONFIG_AUTOBOOT_PROMPT		\
	"Press SPACE to abort autoboot\n"
#define CONFIG_AUTOBOOT_STOP_STR	" "
#define CONFIG_AUTOBOOT_DELAY_STR	"d"

#define CONFIG_VERSION_VARIABLE		1

#define MTDIDS_DEFAULT			"nor0=physmap-flash.0"
#define MTDPARTS_DEFAULT		\
	"mtdparts=physmap-flash.0:"	\
		"128k(u-boot)ro,"	\
		"128k(u-boot-env),"	\
		"1408k(kernel),"	\
		"-(rootfs)"

#define CONFIG_BOOTARGS 		\
	"root=/dev/mtdblock3 rootfstype=jffs2 console=ttyS0,115200"

#define CONFIG_BOOTCOMMAND		"run flashboot"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"mtdid=" MTDIDS_DEFAULT "\0"					\
	"mtdparts=" MTDPARTS_DEFAULT "\0"				\
	"flub=tftp 21000000 cpuat91/u-boot.bin; protect off 10000000 "	\
		"1001FFFF; erase 10000000 1001FFFF; cp.b 21000000 "	\
		"10000000 ${filesize}\0"				\
	"flui=tftp 21000000 cpuat91/uImage; protect off 10040000 "	\
		"1019ffff; erase 10040000 1019ffff; cp.b 21000000 "	\
		"10040000 ${filesize}\0"				\
	"flrfs=tftp 21000000 cpuat91/rootfs.jffs2; protect off "	\
		"101a0000 10ffffff; erase 101a0000 10ffffff; cp.b "	\
		"21000000 101A0000 ${filesize}\0"			\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"		\
	"flashboot=run ramargs;bootm 10040000\0"			\
	"netboot=run ramargs;tftpboot 21000000 cpuat91/uImage;"		\
		"bootm 21000000\0"
#endif	/* __CONFIG_H */
