/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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

/*
 * This file was taken from ARM and changed to remove things we don't
 * need. This is most of it, so have tried to avoid being over-zealous!
 * For example, we want to have an emulation of the 'DRAM' used by
 * U-Boot.
 *
 * has been talk upstream of unifying the architectures w.r.t board.c,
 * so the less change here the better.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <serial.h>

#include <os.h>

DECLARE_GLOBAL_DATA_PTR;

static gd_t gd_mem;

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

static int display_banner(void)
{
	display_options();

	return 0;
}

/**
 * Configure and report on the DRAM configuration, which in our case is
 * fairly simple.
 */
static int display_dram_config(void)
{
	ulong size = 0;
	int i;

	debug("RAM Configuration:\n");

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
#ifdef DEBUG
		printf("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size(gd->bd->bi_dram[i].size, "\n");
#endif
		size += gd->bd->bi_dram[i].size;
	}
	puts("DRAM:  ");
	print_size(size, "\n");
	return 0;
}

/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

void __dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size =  gd->ram_size;
}

void dram_init_banksize(void)
	__attribute__((weak, alias("__dram_init_banksize")));

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
	timer_init,		/* initialize timer */
	env_init,		/* initialize environment */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	sandbox_early_getopt_check,	/* process command line flags (err/help) */
	display_banner,		/* say that we are here */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
	dram_init,		/* configure available RAM banks */
	NULL,
};

void board_init_f(ulong bootflag)
{
	init_fnc_t **init_fnc_ptr;
	uchar *mem;
	unsigned long addr_sp, addr, size;

	gd = &gd_mem;
	assert(gd);

	memset((void *)gd, 0, sizeof(gd_t));

#if defined(CONFIG_OF_EMBED)
	/* Get a pointer to the FDT */
	gd->fdt_blob = _binary_dt_dtb_start;
#elif defined(CONFIG_OF_SEPARATE)
	/* FDT is at end of image */
	gd->fdt_blob = (void *)(_end_ofs + _TEXT_BASE);
#endif

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0)
			hang();
	}

	size = CONFIG_SYS_SDRAM_SIZE;
	mem = os_malloc(CONFIG_SYS_SDRAM_SIZE);

	assert(mem);
	gd->ram_buf = mem;
	addr = (ulong)(mem + size);

	/*
	 * reserve memory for malloc() arena
	 */
	addr_sp = addr - TOTAL_MALLOC_LEN;
	debug("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, addr_sp);
	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr_sp -= sizeof(bd_t);
	gd->bd = (bd_t *) addr_sp;
	debug("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof(bd_t), addr_sp);

	/* Ram ist board specific, so move it to board code ... */
	dram_init_banksize();
	display_dram_config();	/* and display it */

	/* We don't relocate, so just run the post-relocation code */
	board_init_r(NULL, 0);

	/* NOTREACHED - no way out of command loop except booting */
}

/************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 *
 ************************************************************************
 */

void board_init_r(gd_t *id, ulong dest_addr)
{

	if (id)
		gd = id;

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif

#ifdef CONFIG_POST
	post_output_backlog();
#endif

	/* The Malloc area is at the top of simulated DRAM */
	mem_malloc_init((ulong)gd->ram_buf + gd->ram_size - TOTAL_MALLOC_LEN,
			TOTAL_MALLOC_LEN);

	/* initialize environment */
	env_relocate();

	stdio_init();	/* get the devices list going. */

	jumptable_init();

	console_init_r();	/* fully init console as a device */

#if defined(CONFIG_DISPLAY_BOARDINFO_LATE)
	checkboard();
#endif

#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

	 /* set up exceptions */
	interrupt_init();
	/* enable exceptions */
	enable_interrupts();

#ifdef CONFIG_BOARD_LATE_INIT
	board_late_init();
#endif

#ifdef CONFIG_POST
	post_run(NULL, POST_RAM | post_bootmode_get(0));
#endif

	sandbox_main_loop_init();

	/*
	 * For now, run the main loop. Later we might let this be done
	 * in the main program.
	 */
	while (1)
		main_loop();

	/* NOTREACHED - no way out of command loop except booting */
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}
