/*
 * Copyright (C) 2004-2006 Atmel Corporation
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
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <devices.h>
#include <version.h>
#include <net.h>

#include <asm/initcalls.h>
#include <asm/sections.h>

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

DECLARE_GLOBAL_DATA_PTR;

const char version_string[] =
	U_BOOT_VERSION " (" __DATE__ " - " __TIME__ ") " CONFIG_IDENT_STRING;

unsigned long monitor_flash_len;

/*
 * Begin and end of memory area for malloc(), and current "brk"
 */
static unsigned long mem_malloc_start = 0;
static unsigned long mem_malloc_end = 0;
static unsigned long mem_malloc_brk = 0;

/* The malloc area is wherever the board wants it to be */
static void mem_malloc_init(void)
{
	mem_malloc_start = CFG_MALLOC_START;
	mem_malloc_end = CFG_MALLOC_END;
	mem_malloc_brk = mem_malloc_start;

	printf("malloc: Using memory from 0x%08lx to 0x%08lx\n",
	       mem_malloc_start, mem_malloc_end);

	memset ((void *)mem_malloc_start, 0,
		mem_malloc_end - mem_malloc_start);
}

void *sbrk(ptrdiff_t increment)
{
	unsigned long old = mem_malloc_brk;
	unsigned long new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end))
		return NULL;

	mem_malloc_brk = new;
	return ((void *)old);
}

static int init_baudrate(void)
{
	char tmp[64];
	int i;

	i = getenv_r("baudrate", tmp, sizeof(tmp));
	if (i > 0) {
		gd->baudrate = simple_strtoul(tmp, NULL, 10);
	} else {
		gd->baudrate = CONFIG_BAUDRATE;
	}
	return 0;
}


static int display_banner (void)
{
	printf ("\n\n%s\n\n", version_string);
	printf ("U-Boot code: %p -> %p  data: %p -> %p\n",
		_text, _etext, _data, _end);
	return 0;
}

void hang(void)
{
	for (;;) ;
}

static int display_dram_config (void)
{
	int i;

	puts ("DRAM Configuration:\n");

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}

	return 0;
}

static void display_flash_config (void)
{
	puts ("Flash: ");
	print_size(gd->bd->bi_flashsize, " ");
	printf("at address 0x%08lx\n", gd->bd->bi_flashstart);
}

void start_u_boot (void)
{
	gd_t gd_data;

	/* Initialize the global data pointer */
	memset(&gd_data, 0, sizeof(gd_data));
	gd = &gd_data;

	monitor_flash_len = _edata - _text;

	/* Perform initialization sequence */
	cpu_init();
	timer_init();
	env_init();
	init_baudrate();
	serial_init();
	console_init_f();
	display_banner();

	board_init_memories();
	mem_malloc_init();

	gd->bd = malloc(sizeof(bd_t));
	memset(gd->bd, 0, sizeof(bd_t));
	gd->bd->bi_baudrate = gd->baudrate;
	gd->bd->bi_dram[0].start = CFG_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->sdram_size;

	board_init_info();
	flash_init();

	if (gd->bd->bi_flashsize)
		display_flash_config();
	if (gd->bd->bi_dram[0].size)
		display_dram_config();

	gd->bd->bi_boot_params = malloc(CFG_BOOTPARAMS_LEN);
	if (!gd->bd->bi_boot_params)
		puts("WARNING: Cannot allocate space for boot parameters\n");

	/* initialize environment */
	env_relocate();

	devices_init();
	jumptable_init();
	console_init_r();

	for (;;) {
		main_loop();
	}
}
