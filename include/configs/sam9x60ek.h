/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the SAM9X60EK board.
 *
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Sandeep Sheriker M <sandeep.sheriker@microchip.com>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define CONFIG_SYS_AT91_MAIN_CLOCK	24000000	/* 24 MHz crystal */

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_USART_BASE   ATMEL_BASE_DBGU
#define CONFIG_USART_ID     0 /* ignored in arm */

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY            /* required until (g)pio is fixed */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * define CONFIG_USB_EHCI_HCD to enable USB Hi-Speed (aka 2.0)
 * NB: in this case, USB 1.1 devices won't be recognized.
 */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000	/* 256 megs */

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_MASK_ALE	BIT(21)
#define CONFIG_SYS_NAND_MASK_CLE	BIT(22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PD4
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PD5
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif

/* PMECC & PMERRLOC */
#define CONFIG_ATMEL_NAND_HWECC
#define CONFIG_ATMEL_NAND_HW_PMECC
#define CONFIG_PMECC_CAP		8
#define CONFIG_PMECC_SECTOR_SIZE	512

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#ifdef CONFIG_SD_BOOT
/* bootstrap + u-boot + env + linux in sd card */
#define CONFIG_BOOTCOMMAND  \
			"fatload mmc 0:1 0x21000000 at91-sam9x60ek.dtb;" \
			"fatload mmc 0:1 0x22000000 zImage;" \
			"bootz 0x22000000 - 0x21000000"

#elif defined(CONFIG_NAND_BOOT)
/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_BOOTCOMMAND	"nand read " \
				"0x22000000 0x200000 0x600000; " \
				"nand read 0x21000000 0x180000 0x20000; " \
				"bootz 0x22000000 - 0x21000000"

#elif defined(CONFIG_QSPI_BOOT)
/* bootstrap + u-boot + env + linux in SPI NOR flash */
#define CONFIG_BOOTCOMMAND	"sf probe 0; "					\
				"sf read 0x21000000 0x180000 0x80000; "		\
				"sf read 0x22000000 0x200000 0x600000; "	\
				"bootz 0x22000000 - 0x21000000"
#endif

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024 + 0x1000)

#endif
