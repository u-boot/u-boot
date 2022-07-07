/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Atmel Corporation
 *
 * Configuation settings for the AT91SAM9X5EK board.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* 12 MHz crystal */

/* general purpose I/O */

/*
 * define CONFIG_USB_EHCI_HCD to enable USB Hi-Speed (aka 2.0)
 * NB: in this case, USB 1.1 devices won't be recognized.
 */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x08000000	/* 128 megs */

/* DataFlash */

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8		1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PD4
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PD5
#endif

/* SPL */

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20c73f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#endif
