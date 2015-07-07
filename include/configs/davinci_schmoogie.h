/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*=======*/
/* Board */
/*=======*/
#define SCHMOOGIE
#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_SYS_USE_NAND
#define MACH_TYPE_SCHMOOGIE 1255
#define CONFIG_MACH_TYPE MACH_TYPE_SCHMOOGIE

/*===================*/
/* SoC Configuration */
/*===================*/
#define CONFIG_SYS_TIMERBASE		0x01c21400	/* use timer 0 */
#define CONFIG_SYS_HZ_CLOCK		27000000	/* Timer Input clock freq */
#define CONFIG_SOC_DM644X
/*=============*/
/* Memory Info */
/*=============*/
#define CONFIG_SYS_MALLOC_LEN		(0x10000 + 256*1024)	/* malloc() len */
#define CONFIG_SYS_MEMTEST_START	0x80000000	/* memtest start address */
#define CONFIG_SYS_MEMTEST_END		0x81000000	/* 16MB RAM test */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x80000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* DDR size 128MB */
#define DDR_4BANKS				/* 4-bank DDR2 (128MB) */
/*====================*/
/* Serial Driver info */
/*====================*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4	/* NS16550 register size, byteorder */
#define CONFIG_SYS_NS16550_COM1	0x01c20000	/* Base address of UART0 */
#define CONFIG_SYS_NS16550_CLK	CONFIG_SYS_HZ_CLOCK	/* Input clock to NS16550 */
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */
/*===================*/
/* I2C Configuration */
/*===================*/
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DAVINCI
#define CONFIG_SYS_DAVINCI_I2C_SPEED 80000 /* 100Kbps won't work, silicon bug */
#define CONFIG_SYS_DAVINCI_I2C_SLAVE 10    /* Bogus, master-only in U-Boot */
/*==================================*/
/* Network & Ethernet Configuration */
/*==================================*/
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_MII
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#define CONFIG_OVERWRITE_ETHADDR_ONCE
/*=====================*/
/* Flash & Environment */
/*=====================*/
#undef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_SYS_NO_FLASH
#define CONFIG_NAND_DAVINCI
#define CONFIG_SYS_NAND_CS		2
#define CONFIG_ENV_IS_IN_NAND		/* U-Boot env in NAND Flash  */
#define CONFIG_ENV_SECT_SIZE	2048	/* Env sector Size */
#define CONFIG_ENV_SIZE		(128 << 10)	/* 128 KiB */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is loaded by a bootloader */
#define CONFIG_SYS_NAND_BASE		0x02000000
#define CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1	/* Max number of NAND devices */
#define CONFIG_ENV_OFFSET		0x0	/* Block 0--not used by bootcode */
/*=====================*/
/* Board related stuff */
/*=====================*/
#define CONFIG_RTC_DS1307		/* RTC chip on SCHMOOGIE */
#define CONFIG_SYS_I2C_RTC_ADDR	0x6f	/* RTC chip I2C address */
#define CONFIG_UID_DS28CM00		/* Unique ID on SCHMOOGIE */
#define CONFIG_SYS_UID_ADDR		0x50	/* UID chip I2C address */
/*==============================*/
/* U-Boot general configuration */
/*==============================*/
#define CONFIG_MISC_INIT_R
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTFILE		"uImage"	/* Boot file name */
#define CONFIG_SYS_PROMPT		"U-Boot > "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print buffer sz */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_LOAD_ADDR		0x80700000	/* default Linux kernel load address */
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE		/* Won't work with hush so far, may be later */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC
/*===================*/
/* Linux Information */
/*===================*/
#define LINUX_BOOT_PARAM_ADDR	0x80000100
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS		"mem=56M console=ttyS0,115200n8 root=/dev/hda1 rw noinitrd ip=dhcp"
#define CONFIG_BOOTCOMMAND	"setenv setboot setenv bootargs \\$(bootargs) video=dm64xxfb:output=\\$(videostd);run setboot"
/*=================*/
/* U-Boot commands */
/*=================*/
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NAND
#undef CONFIG_CMD_EEPROM

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#define CONFIG_MAX_RAM_BANK_SIZE	(256 << 20)	/* 256 MB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

#endif /* __CONFIG_H */
