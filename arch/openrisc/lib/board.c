/*
 * (C) Copyright 2011
 * Julius Baxter, julius@opencores.org
 *
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <console.h>
#include <stdio_dev.h>
#include <watchdog.h>
#include <malloc.h>
#include <mmc.h>
#include <net.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#ifdef CONFIG_CMD_NAND
#include <nand.h>	/* cannot even include nand.h if it isnt configured */
#endif

#include <timestamp.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */

extern int cache_init(void);

/*
 * Initialization sequence
 */
static int (* const init_sequence[])(void) = {
	cache_init,
	timer_init,		/* initialize timer */
	env_init,
	serial_init,
	console_init_f,
	display_options,
	checkcpu,
	checkboard,
};


/***********************************************************************/
void board_init(void)
{
	bd_t *bd;
	int i;

	gd = (gd_t *)CONFIG_SYS_GBL_DATA_ADDR;

	memset((void *)gd, 0, GENERATED_GBL_DATA_SIZE);

	gd->bd = (bd_t *)(gd+1);	/* At end of global data */
	gd->baudrate = CONFIG_BAUDRATE;
	gd->cpu_clk = CONFIG_SYS_CLK_FREQ;

	bd = gd->bd;
	bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
#ifndef CONFIG_SYS_NO_FLASH
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
#endif
#if	defined(CONFIG_SYS_SRAM_BASE) && defined(CONFIG_SYS_SRAM_SIZE)
	bd->bi_sramstart = CONFIG_SYS_SRAM_BASE;
	bd->bi_sramsize = CONFIG_SYS_SRAM_SIZE;
#endif

	for (i = 0; i < ARRAY_SIZE(init_sequence); i++) {
		WATCHDOG_RESET();
		if (init_sequence[i]())
			hang();
	}

	WATCHDOG_RESET();

	/* The Malloc area is immediately below the monitor copy in RAM */
	mem_malloc_init(CONFIG_SYS_MALLOC_BASE, CONFIG_SYS_MALLOC_LEN);

#ifndef CONFIG_SYS_NO_FLASH
	WATCHDOG_RESET();
	bd->bi_flashsize = flash_init();
#endif

#ifdef CONFIG_CMD_NAND
	puts("NAND:  ");
	nand_init();
#endif

#ifdef CONFIG_GENERIC_MMC
	puts("MMC:   ");
	mmc_initialize(bd);
#endif

	WATCHDOG_RESET();
	env_relocate();

	WATCHDOG_RESET();
	stdio_init();
	jumptable_init();
	console_init_r();

	WATCHDOG_RESET();
	interrupt_init();

#if defined(CONFIG_BOARD_LATE_INIT)
	board_late_init();
#endif

#if defined(CONFIG_CMD_NET)
	puts("NET:   ");
	eth_initialize();
#endif

	/* main_loop */
	for (;;) {
		WATCHDOG_RESET();
		main_loop();
	}
}
