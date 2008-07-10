/*
 * Copyright (C) 2007,2008
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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
#include <environment.h>

extern void malloc_bin_reloc (void);
extern int cpu_init(void);
extern int board_init(void);
extern int dram_init(void);
extern int watchdog_init(void);
extern int timer_init(void);

const char version_string[] = U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")";

unsigned long monitor_flash_len = CFG_MONITOR_LEN;

static unsigned long mem_malloc_start;
static unsigned long mem_malloc_end;
static unsigned long mem_malloc_brk;

static void mem_malloc_init (void)
{

	mem_malloc_start = (TEXT_BASE - CFG_GBL_DATA_SIZE - CFG_MALLOC_LEN);
	mem_malloc_end = (mem_malloc_start + CFG_MALLOC_LEN - 16);
	mem_malloc_brk = mem_malloc_start;
	memset ((void *) mem_malloc_start, 0,
		(mem_malloc_end - mem_malloc_start));
}

void *sbrk (ptrdiff_t increment)
{
	unsigned long old = mem_malloc_brk;
	unsigned long new = old + increment;

	if ((new < mem_malloc_start) ||
	    (new > mem_malloc_end)) {
		return NULL;
	}

	mem_malloc_brk = new;
	return (void *) old;
}

static int sh_flash_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_flashsize = flash_init();
	printf("FLASH: %dMB\n", gd->bd->bi_flashsize / (1024*1024));

	return 0;
}

#if defined(CONFIG_CMD_NAND)
#include <nand.h>
static int sh_nand_init(void)
{
	printf("NAND: ");
	nand_init();	/* go init the NAND */
	return 0;
}
#endif /* CONFIG_CMD_NAND */

#if defined(CONFIG_CMD_IDE)
#include <ide.h>
static int sh_marubun_init(void)
{
	puts ("IDE:   ");
	ide_init();
	return 0;
}
#endif /* (CONFIG_CMD_IDE) */

#if defined(CONFIG_PCI)
static int sh_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_PCI */

static int sh_mem_env_init(void)
{
	mem_malloc_init();
	malloc_bin_reloc();
	env_relocate();
	jumptable_init();
	return 0;
}

#if defined(CONFIG_CMD_NET)
static int sh_net_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	char *s, *e;
	int i;

	gd->bd->bi_ip_addr = getenv_IPaddr("ipaddr");
	s = getenv("ethaddr");
	for (i = 0; i < 6; ++i) {
		gd->bd->bi_enetaddr[i] = s ? simple_strtoul(s, &e, 16) : 0;
		if (s) s = (*e) ? e + 1 : e;
	}

	return 0;
}
#endif

typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] =
{
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
	interrupt_init,		/* set up exceptions */
	env_init,		/* event init */
	serial_init,		/* SCIF init */
	watchdog_init,		/* watchdog init */
	console_init_f,
	display_options,
	checkcpu,
	checkboard,		/* Check support board */
	dram_init,		/* SDRAM init */
	timer_init,		/* SuperH Timer (TCNT0 only) init */
	sh_flash_init,		/* Flash memory(NOR) init*/
	sh_mem_env_init,
#if defined(CONFIG_CMD_NAND)
	sh_nand_init,		/* Flash memory (NAND) init */
#endif
#if defined(CONFIG_PCI)
	sh_pci_init,		/* PCI Init */
#endif
	devices_init,
	console_init_r,
	interrupt_init,
#ifdef BOARD_LATE_INIT
	board_late_init,
#endif
#if defined(CONFIG_CMD_NET)
	sh_net_init,		/* SH specific eth init */
#endif
	NULL			/* Terminate this list */
};

void sh_generic_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	int i;
	char *s;

	memset (gd, 0, CFG_GBL_DATA_SIZE);

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	gd->bd = (bd_t *) (gd + 1);	/* At end of global data */
	gd->baudrate = CONFIG_BAUDRATE;

	gd->cpu_clk = CONFIG_SYS_CLK_FREQ;

	bd = gd->bd;
	bd->bi_memstart	= CFG_SDRAM_BASE;
	bd->bi_memsize = CFG_SDRAM_SIZE;
	bd->bi_flashstart = CFG_FLASH_BASE;
#if defined(CFG_SRAM_BASE) && defined(CFG_SRAM_SIZE)
	bd->bi_sramstart= CFG_SRAM_BASE;
	bd->bi_sramsize	= CFG_SRAM_SIZE;
#endif
	bd->bi_baudrate	= CONFIG_BAUDRATE;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr , i++) {
		if ((*init_fnc_ptr) () != 0) {
			hang();
		}
	}

#if defined(CONFIG_CMD_NET)
	puts ("Net:   ");
	eth_initialize(gd->bd);

	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif /* CONFIG_CMD_NET */

	while (1) {
		main_loop();
	}
}

/***********************************************************************/

void hang (void)
{
	puts ("Board ERROR\n");
	for (;;);
}
