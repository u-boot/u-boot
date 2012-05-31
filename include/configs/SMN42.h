/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Configuation settings for the SMN42 board from Siemens.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * If we are developing, we might want to start u-boot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#undef CONFIG_SKIP_LOWLEVEL_INIT

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM7		1	/* This is a ARM7 CPU	*/
#define CONFIG_ARM_THUMB	1	/* this is an ARM720TDMI */
#define CONFIG_LPC2292
#undef	CONFIG_ARM7_REVD		/* disable ARM720 REV.D Workarounds */

#undef CONFIG_USE_IRQ			/* don't need them anymore */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */

/*
 * select serial console configuration
 */
#define CONFIG_LPC2292_SERIAL
#define CONFIG_SERIAL1		1	/* we use Serial line 1 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/* enable I2C and select the hardware/software driver */
#undef	CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/
/* this would be 0xAE if E0, E1 and E2 were pulled high */
#define CONFIG_SYS_I2C_SLAVE		0xA0
#define CONFIG_SYS_I2C_EEPROM_ADDR	(0xA0 >> 1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2 /* 16 bit address */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 6 /* 64 bytes per write */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 20
/* not used but required by devices.c */
#define CONFIG_SYS_I2C_SPEED 10000

#ifdef CONFIG_SOFT_I2C
/*
 * Software (bit-bang) I2C driver configuration
 */
#define SCL		0x00000004		/* P0.2 */
#define SDA		0x00000008		/* P0.3 */

#define	I2C_READ	((GET32(IO0PIN) & SDA) ? 1 : 0)
#define	I2C_SDA(x)	{ if (x) PUT32(IO0SET, SDA); else PUT32(IO0CLR, SDA); }
#define	I2C_SCL(x)	{ if (x) PUT32(IO0SET, SCL); else PUT32(IO0CLR, SCL); }
#define	I2C_DELAY	{ udelay(100); }
#define	I2C_ACTIVE	{ unsigned int i2ctmp; \
					  i2ctmp = GET32(IO0DIR); \
					  i2ctmp |= SDA; \
					  PUT32(IO0DIR, i2ctmp); }
#define	I2C_TRISTATE	{ unsigned int i2ctmp; \
					      i2ctmp = GET32(IO0DIR); \
					      i2ctmp &= ~SDA; \
						  PUT32(IO0DIR, i2ctmp); }
#endif /* CONFIG_SOFT_I2C */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NET
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_PING


#define CONFIG_DOS_PARTITION

#define CONFIG_BOOTDELAY	5

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"SMN42 # " /* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x81800000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x83000000	/* 24 MB in SRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x81000000	/* default load address	*/
						/* for uClinux img is here*/

#define CONFIG_SYS_SYS_CLK_FREQ	58982400	/* Hz */
#define	CONFIG_SYS_HZ			2048		/* decrementer freq in Hz */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of SRAM */
#define PHYS_SDRAM_1		0x81000000 /* SRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB SRAM */

/* This is the external flash */
#define PHYS_FLASH_1		0x80000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE		0x01000000 /* 16 MB */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/*
 * The first entry in CONFIG_SYS_FLASH_BANKS_LIST is a dummy, but it must be present.
 */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ 0, PHYS_FLASH_1 }
#define CONFIG_SYS_FLASH_ADDR0			0x555
#define CONFIG_SYS_FLASH_ADDR1			0x2AA
#define CONFIG_SYS_FLASH_ERASE_TOUT	16384	/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	5	/* Timeout for Flash Write (in ms) */

#define CONFIG_SYS_MAX_FLASH_SECT	128  /* max number of sectors on one chip    */

#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/

#define	CONFIG_ENV_IS_IN_FLASH	1
/* The Environment Sector is in the CPU-internal flash */
#define CONFIG_SYS_FLASH_BASE		0
#define CONFIG_ENV_OFFSET		0x3C000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x2000 /* Total Size of Environment Sector	*/

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_MMC			1
/* we use this ethernet chip */
#define CONFIG_ENC28J60_LPC2292

#endif	/* __CONFIG_H */
