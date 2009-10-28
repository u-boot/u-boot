/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2006
 * Eric Schumann, Phytec Messatechnik GmbH
 *
 * (C) Copyright 2009
 * Jon Smirl <jonsmirl@gmail.com>
 *
 * (C) Copyright 2009
 * Eric Millbrandt, DEKA Research and Development Corporation
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

#define CONFIG_BOARDINFO	 "galaxy5200"

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200		1	/* (more precisely an MPC5200 CPU) */
#define CONFIG_SYS_MPC5XXX_CLKIN 33333333	/* ... running at 33.333333MHz */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	4	/* console is on PSC4 -> */
					/* define gps port conf. */
					/* register later on to  */
					/* enable UART function! */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_PING
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_USB
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_FAT

#define	CONFIG_TIMESTAMP	1	/* Print image info with timestamp */

#if (TEXT_BASE == 0xFE000000)		/* Boot low */
#define CONFIG_SYS_LOWBOOT 1
#endif
/* RAMBOOT will be defined automatically in memory section */

#define MTDIDS_DEFAULT		"nor0=physmap-flash.0"
#define MTDPARTS_DEFAULT	"mtdparts=physmap-flash.0:256k(ubootl)," \
	"1792k(kernel),13312k(jffs2),256k(uboot)ro,256k(oftree),-(space)"

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	10	/* autoboot after 10 seconds */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* allow stopping of boot process */
					/* even with bootdelay=0 */
#define CONFIG_BOOT_RETRY_TIME 120	/* Reset if no command is entered  */
#define CONFIG_RESET_TO_RETRY

#define CONFIG_PREBOOT	"echo;"	\
	"echo Welcome to U-Boot;"\
	"echo"

#define CONFIG_BOOTCOMMAND     "go ff300004 0; go ff300004 2 2;" \
	"bootm ff040000 ff900000 fffc0000"
#define CONFIG_BOOTARGS        "console=ttyPSC0,115200"
#define CONFIG_EXTRA_ENV_SETTINGS "epson=yes\0"

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK	/* define for 133MHz speed */
#define CONFIG_SYS_XLB_PIPELINING	1

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C 1		/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE 2		/* Select I2C module #1 or #2 */
#define CONFIG_SYS_I2C_SPEED 100000	/* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE 0x7F
#define CONFIG_SYS_I2C_INIT_MPC5XXX	/* Reset devices on i2c bus */

/*
 * EEPROM CAT24WC32 configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x52	/* 1010100x */
#define CONFIG_SYS_I2C_FACT_ADDR	0x52	/* EEPROM CAT24WC32 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2	/* Bytes of address */
#define CONFIG_SYS_EEPROM_SIZE		4096
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 15

/*
 * RTC configuration
 */
#define RTC
#define CONFIG_RTC_DS3231		1
#define CONFIG_SYS_I2C_RTC_ADDR		0x68

/*
 * Flash configuration
 */

#define CONFIG_SYS_FLASH_BASE		0xfe000000
/*
 * The flash size is autoconfigured, but cpu/mpc5xxx/cpu_init.c needs this
 * variable defined
 */
#define CONFIG_SYS_FLASH_SIZE		0x02000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

#define CONFIG_SYS_FLASH_CFI 1		/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER	1	/* Use the common driver */
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_MAX_FLASH_SECT 259	/* max num of sects on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS 1	/* max num of flash banks */
					/* (= chip selects) */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

/*
 * Use hardware protection. This seems required, as the BDI uses hardware
 * protection. Without this, U-Boot can't work with this sectors as its
 * protection is software only by default.
 */
#define CONFIG_SYS_FLASH_PROTECTION	1

/*
 * Environment settings
 */

#define CONFIG_ENV_IS_IN_EEPROM	1
#define CONFIG_ENV_OFFSET	0x00	/* environment starts at the */
					/* beginning of the EEPROM */
#define CONFIG_ENV_SIZE		CONFIG_SYS_EEPROM_SIZE

#define CONFIG_ENV_OVERWRITE	1

/*
 * SDRAM configuration
 */
#define SDRAM_DDR	1
#define SDRAM_MODE	0x018D0000
#define SDRAM_EMODE	0x40090000
#define SDRAM_CONTROL	0x71500F00
#define SDRAM_CONFIG1	0x73711930
#define SDRAM_CONFIG2	0x47770000

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR	0xF0000000	/* MBAR has to be switched by other */
					/* bootloader or debugger config */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR		0x80000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM

/* End of used area in SPRAM */
#define CONFIG_SYS_INIT_RAM_END		MPC5XXX_SRAM_SIZE

/* Size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
						CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#	define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN (192 << 10)	/* Reserve 192 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN (128 << 10)	/* Reserve 128 kB for malloc() */
#define CONFIG_SYS_BOOTMAPSZ (8 << 20)	/* Initial Memory map for Linux */

/* Chip Select configuration for NAND flash */
#define CONFIG_SYS_CS1_START		0x20000000
#define CONFIG_SYS_CS1_SIZE		0x90000
#define CONFIG_SYS_CS1_CFG		0x00025b00

/* Chip Select configuration for Epson S1D13513 */
#define CONFIG_SYS_CS3_START		0x10000000
#define CONFIG_SYS_CS3_SIZE		0x400000
#define CONFIG_SYS_CS3_CFG		0xffff3d10

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC		1
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR			0x01
#define CONFIG_NO_AUTOLOAD		1

/*
 * GPIO configuration
 *
 * GPS port configuration
 *
 * [29:31] = 01x
 * AC97 on PSC1
 * PSC1_0 -> AC97 SDATA out
 * PSC1_1 -> AC97 SDTA in
 * PSC1_2 -> AC97 SYNC out
 * PSC1_3 -> AC97 bitclock out
 * PSC1_4 -> AC97 reset out
 *
 * [28] = Reserved
 *
 * [25:27] = 110
 * SPI on PSC2
 * PSC2_0 -> MOSI
 * PSC2_1 -> MISO
 * PSC2_2 -> n/a
 * PSC2_3 -> CLK
 * PSC2_4 -> SS
 *
 * [24] = Reserved
 *
 * [20:23] = 0001
 * USB on PSC3
 * PSC3_0 -> USB_OE OE out
 * PSC3_1 -> USB_TXN Tx- out
 * PSC3_2 -> USB_TXP Tx+ out
 * PSC3_3 -> USB_TXD
 * PSC3_4 -> USB_RXP Rx+ in
 * PSC3_5 -> USB_RXN Rx- in
 * PSC3_6 -> USB_PWR PortPower out
 * PSC3_7 -> USB_SPEED speed out
 * PSC3_8 -> USB_SUSPEND suspend
 * PSC3_9 -> USB_OVRCURNT overcurrent in
 *
 * [18:19] = 10
 * Two UARTs
 *
 * [17] = 0
 * USB differential mode
 *
 * [16] = 1
 * PCI disabled
 *
 * [12:15] = 0101
 * Ethernet 100Mbit with MD
 * ETH_0 -> ETH Txen
 * ETH_1 -> ETH TxD0
 * ETH_2 -> ETH TxD1
 * ETH_3 -> ETH TxD2
 * ETH_4 -> ETH TxD3
 * ETH_5 -> ETH Txerr
 * ETH_6 -> ETH MDC
 * ETH_7 -> ETH MDIO
 * ETH_8 -> ETH RxDv
 * ETH_9 -> ETH RxCLK
 * ETH_10 -> ETH Collision
 * ETH_11 -> ETH TxD
 * ETH_12 -> ETH RxD0
 * ETH_13 -> ETH RxD1
 * ETH_14 -> ETH RxD2
 * ETH_15 -> ETH RxD3
 * ETH_16 -> ETH Rxerr
 * ETH_17 -> ETH CRS
 *
 * [9:11] = 111
 * SPI on PSC6
 * PSC6_0 -> MISO
 * PSC6_1 -> SS#
 * PSC6_2 -> MOSI
 * PSC6_3 -> CLK
 *
 * [8] = 0
 * IrDA/USB 48MHz clock generated internally
 *
 * [6:7] = 01
 * ATA chip selects on csb_4/5
 * CSB_4 -> ATA_CS0 out
 * CSB_5 -> ATA_CS1 out
 *
 * [5] = 1
 * PSC3_4 is used as CS6
 *
 * [4] = 1
 * PSC3_5 is used as CS7
 *
 * [2:3] = 00
 * No Alternatives
 *
 * [1] = 0
 * gpio_wkup_7 is GPIO
 *
 * [0] = 0
 * gpio_wkup_6 is GPIO
 *
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x0d75a162

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_PROMPT "uboot> "		/* Monitor Command Prompt */

#define CONFIG_CMDLINE_EDITING 1		/* add command line history */

#define CONFIG_SYS_CACHELINE_SIZE 32		/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT 5		/* log base 2 of the above value */
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE 1024			/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE 512			/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
						/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS 32			/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START 0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END 0x00f00000	/* 1 ... 15 MB in DRAM */

#define CONFIG_SYS_LOAD_ADDR 0x400000		/* default load address */
#define CONFIG_SYS_HZ 1000			/* decrementer freq: 1 ms ticks */

#define CONFIG_DISPLAY_BOARDINFO 1

#define CONFIG_SYS_HUSH_PARSER 1
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "

#define CONFIG_CRC32_VERIFY  1

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | \
				 CONFIG_BOOTP_DNS | \
				 CONFIG_BOOTP_DNS2 | \
				 CONFIG_BOOTP_SEND_HOSTNAME )

#define CONFIG_VERSION_VARIABLE 1

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

/* no burst access on the LPB */
#define CONFIG_SYS_CS_BURST		0x00000000
/* one deadcycle for the 33MHz statemachine */
#define CONFIG_SYS_CS_DEADCYCLE		0x33333331

#define CONFIG_SYS_BOOTCS_CFG		0x0002d900
#define CONFIG_SYS_BOOTCS_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*
 * USB settings
 */
#define CONFIG_USB_CLOCK		0x0001bbbb
/* USB is on PSC3 */
#define CONFIG_PSC3_USB
#define CONFIG_USB_CONFIG		0x00000100
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/*
 * IDE/ATA stuff Supports IDE harddisk
 */
#undef  CONFIG_IDE_8xx_PCCARD	/* Use IDE with PC Card Adapter */
#undef	CONFIG_IDE_8xx_DIRECT	/* Direct IDE not supported */
#undef	CONFIG_IDE_LED		/* LED for ide not supported */

#define	CONFIG_IDE_RESET 1	/* reset for ide supported */
#define CONFIG_IDE_PREINIT
#define CONFIG_SYS_IDE_MAXBUS 1	/* max. 1 IDE bus */
#define CONFIG_SYS_IDE_MAXDEVICE 2	/* max. 2 drives per IDE bus */
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA
/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)
/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)
/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)
/* Interval between registers */
#define CONFIG_SYS_ATA_STRIDE		4
#define CONFIG_ATAPI			1

/* we enable IDE and FAT support, so we also need partition support */
#define CONFIG_DOS_PARTITION 1

/*
 * Open Firmware flat tree
 */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1

#define OF_CPU				"PowerPC,5200@0"
#define OF_TBCLK			CONFIG_SYS_MPC5XXX_CLKIN
#define OF_SOC				"soc5200@f0000000"
#define OF_STDOUT_PATH			"/soc5200@f0000000/serial@2600"

#endif /* __CONFIG_H */
