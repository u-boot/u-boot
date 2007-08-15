/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include <version.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

const char version_string[] = U_BOOT_VERSION " (" __DATE__ " - " __TIME__ ")";

#ifdef CFG_GPIO_0
extern int gpio_init (void);
#endif
#ifdef CFG_INTC_0
extern int interrupts_init (void);
#endif
#if defined(CONFIG_CMD_NET)
extern int eth_init (bd_t * bis);
extern int getenv_IPaddr (char *);
#endif

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;

/*
 * The Malloc area is immediately below the monitor copy in DRAM
 * aka CFG_MONITOR_BASE - Note there is no need for reloc_off
 * as our monitory code is run from SDRAM
 */
static void mem_malloc_init (void)
{
	mem_malloc_end = (CFG_MALLOC_BASE + CFG_MALLOC_LEN);
	mem_malloc_start = CFG_MALLOC_BASE;
	mem_malloc_brk = mem_malloc_start;
	memset ((void *)mem_malloc_start, 0, mem_malloc_end - mem_malloc_start);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;
	return ((void *)old);
}

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
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	env_init,
	serial_init,
#ifdef CFG_GPIO_0
	gpio_init,
#endif
#ifdef CFG_INTC_0
	interrupts_init,
#endif
	NULL,
};

void board_init (void)
{
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	gd = (gd_t *) CFG_GBL_DATA_OFFSET;
#if defined(CONFIG_CMD_FLASH)
	ulong flash_size = 0;
#endif
	asm ("nop");	/* FIXME gd is not initialize - wait */
	memset ((void *)gd, 0, CFG_GBL_DATA_SIZE);
	gd->bd = (bd_t *) (gd + 1);	/* At end of global data */
	gd->baudrate = CONFIG_BAUDRATE;
	bd = gd->bd;
	bd->bi_baudrate = CONFIG_BAUDRATE;
	bd->bi_memstart = CFG_SDRAM_BASE;
	bd->bi_memsize = CFG_SDRAM_SIZE;

	/* Initialise malloc() area */
	mem_malloc_init ();

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		WATCHDOG_RESET ();
		if ((*init_fnc_ptr) () != 0) {
			hang ();
		}
	}

#if defined(CONFIG_CMD_FLASH)
	bd->bi_flashstart = CFG_FLASH_BASE;
	if (0 < (flash_size = flash_init ())) {
		bd->bi_flashsize = flash_size;
		bd->bi_flashoffset = CFG_FLASH_BASE + flash_size;
	} else {
		puts ("Flash init FAILED");
		bd->bi_flashstart = 0;
		bd->bi_flashsize = 0;
		bd->bi_flashoffset = 0;
	}
#endif

#if defined(CONFIG_CMD_NET)
	char *s, *e;
	int i;
	/* board MAC address */
	s = getenv ("ethaddr");
	for (i = 0; i < 6; ++i) {
		bd->bi_enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr ("ipaddr");
	eth_init (bd);
#endif

	/* relocate environment function pointers etc. */
	env_relocate ();

	/* main_loop */
	for (;;) {
		WATCHDOG_RESET ();
		main_loop ();
	}
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;) ;
}
