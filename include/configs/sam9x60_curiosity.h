/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SAM9X60 CURIOSITY board.
 *
 * Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Durai Manickam KR <durai.manickamkr@microchip.com>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define CONFIG_SYS_AT91_MAIN_CLOCK	24000000	/* 24 MHz crystal */

#define CONFIG_USART_BASE   ATMEL_BASE_DBGU
#define CONFIG_USART_ID     0 /* ignored in arm */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x8000000	/* 128 MB */

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR         0x218000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 + CONFIG_SYS_MALLOC_F_LEN - \
	 GENERATED_GBL_DATA_SIZE)
#endif

#endif
