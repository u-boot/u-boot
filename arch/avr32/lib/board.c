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
#include <stdio_dev.h>
#include <version.h>
#include <net.h>

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#include <asm/sections.h>
#include <asm/arch/mmu.h>

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

DECLARE_GLOBAL_DATA_PTR;

unsigned long monitor_flash_len;

/* Weak aliases for optional board functions */
static int __do_nothing(void)
{
	return 0;
}
int board_postclk_init(void) __attribute__((weak, alias("__do_nothing")));
int board_early_init_r(void) __attribute__((weak, alias("__do_nothing")));

#ifdef CONFIG_SYS_DMA_ALLOC_LEN
#include <asm/arch/cacheflush.h>
#include <asm/io.h>

static unsigned long dma_alloc_start;
static unsigned long dma_alloc_end;
static unsigned long dma_alloc_brk;

static void dma_alloc_init(void)
{
	unsigned long monitor_addr;

	monitor_addr = CONFIG_SYS_MONITOR_BASE + gd->reloc_off;
	dma_alloc_end = monitor_addr - CONFIG_SYS_MALLOC_LEN;
	dma_alloc_start = dma_alloc_end - CONFIG_SYS_DMA_ALLOC_LEN;
	dma_alloc_brk = dma_alloc_start;

	printf("DMA: Using memory from 0x%08lx to 0x%08lx\n",
	       dma_alloc_start, dma_alloc_end);

	dcache_invalidate_range(cached(dma_alloc_start),
				dma_alloc_end - dma_alloc_start);
}

void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	unsigned long paddr = dma_alloc_brk;

	if (dma_alloc_brk + len > dma_alloc_end)
		return NULL;

	dma_alloc_brk = ((paddr + len + CONFIG_SYS_DCACHE_LINESZ - 1)
			 & ~(CONFIG_SYS_DCACHE_LINESZ - 1));

	*handle = paddr;
	return uncached(paddr);
}
#else
static inline void dma_alloc_init(void)
{

}
#endif

static int init_baudrate(void)
{
	char tmp[64];
	int i;

	i = getenv_f("baudrate", tmp, sizeof(tmp));
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
	printf ("U-Boot code: %08lx -> %08lx  data: %08lx -> %08lx\n",
		(unsigned long)_text, (unsigned long)_etext,
		(unsigned long)_data, (unsigned long)__bss_end__);
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

void board_init_f(ulong board_type)
{
	gd_t gd_data;
	gd_t *new_gd;
	bd_t *bd;
	unsigned long *new_sp;
	unsigned long monitor_len;
	unsigned long monitor_addr;
	unsigned long addr;
	long sdram_size;

	/* Initialize the global data pointer */
	memset(&gd_data, 0, sizeof(gd_data));
	gd = &gd_data;

	/* Perform initialization sequence */
	board_early_init_f();
	cpu_init();
	board_postclk_init();
	env_init();
	init_baudrate();
	serial_init();
	console_init_f();
	display_banner();
	sdram_size = initdram(board_type);

	/* If we have no SDRAM, we can't go on */
	if (sdram_size <= 0)
		panic("No working SDRAM available\n");

	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - u-boot image
	 *  - heap for malloc()
	 *  - board info struct
	 *  - global data struct
	 *  - stack
	 */
	addr = CONFIG_SYS_SDRAM_BASE + sdram_size;
	monitor_len = __bss_end__ - _text;

	/*
	 * Reserve memory for u-boot code, data and bss.
	 * Round down to next 4 kB limit.
	 */
	addr -= monitor_len;
	addr &= ~(4096UL - 1);
	monitor_addr = addr;

	/* Reserve memory for malloc() */
	addr -= CONFIG_SYS_MALLOC_LEN;

#ifdef CONFIG_SYS_DMA_ALLOC_LEN
	/* Reserve DMA memory (must be cache aligned) */
	addr &= ~(CONFIG_SYS_DCACHE_LINESZ - 1);
	addr -= CONFIG_SYS_DMA_ALLOC_LEN;
#endif

#ifdef CONFIG_LCD
#ifdef CONFIG_FB_ADDR
	printf("LCD: Frame buffer allocated at preset 0x%08x\n",
	       CONFIG_FB_ADDR);
	gd->fb_base = (void *)CONFIG_FB_ADDR;
#else
	addr = lcd_setmem(addr);
	printf("LCD: Frame buffer allocated at 0x%08lx\n", addr);
	gd->fb_base = (void *)addr;
#endif /* CONFIG_FB_ADDR */
#endif /* CONFIG_LCD */

	/* Allocate a Board Info struct on a word boundary */
	addr -= sizeof(bd_t);
	addr &= ~3UL;
	gd->bd = bd = (bd_t *)addr;

	/* Allocate a new global data copy on a 8-byte boundary. */
	addr -= sizeof(gd_t);
	addr &= ~7UL;
	new_gd = (gd_t *)addr;

	/* And finally, a new, bigger stack. */
	new_sp = (unsigned long *)addr;
	gd->stack_end = addr;
	*(--new_sp) = 0;
	*(--new_sp) = 0;

	/*
	 * Initialize the board information struct with the
	 * information we have.
	 */
	bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	bd->bi_dram[0].size = sdram_size;
	bd->bi_baudrate = gd->baudrate;

	memcpy(new_gd, gd, sizeof(gd_t));

	relocate_code((unsigned long)new_sp, new_gd, monitor_addr);
}

void board_init_r(gd_t *new_gd, ulong dest_addr)
{
	extern void malloc_bin_reloc (void);
#ifndef CONFIG_ENV_IS_NOWHERE
	extern char * env_name_spec;
#endif
	char *s;
	bd_t *bd;

	gd = new_gd;
	bd = gd->bd;

	gd->flags |= GD_FLG_RELOC;
	gd->reloc_off = dest_addr - CONFIG_SYS_MONITOR_BASE;

	/* Enable the MMU so that we can keep u-boot simple */
	mmu_init_r(dest_addr);

	board_early_init_r();

	monitor_flash_len = _edata - _text;

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	/*
	 * We have to relocate the command table manually
	 */
	fixup_cmdtable(&__u_boot_cmd_start,
		(ulong)(&__u_boot_cmd_end - &__u_boot_cmd_start));
#endif /* defined(CONFIG_NEEDS_MANUAL_RELOC) */

	/* there are some other pointer constants we must deal with */
#ifndef CONFIG_ENV_IS_NOWHERE
	env_name_spec += gd->reloc_off;
#endif

	timer_init();

	/* The malloc area is right below the monitor image in RAM */
	mem_malloc_init(CONFIG_SYS_MONITOR_BASE + gd->reloc_off -
			CONFIG_SYS_MALLOC_LEN, CONFIG_SYS_MALLOC_LEN);
	malloc_bin_reloc();
	dma_alloc_init();

	enable_interrupts();

	bd->bi_flashstart = 0;
	bd->bi_flashsize = 0;
	bd->bi_flashoffset = 0;

#ifndef CONFIG_SYS_NO_FLASH
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
	bd->bi_flashsize = flash_init();
	bd->bi_flashoffset = (unsigned long)_edata - (unsigned long)_text;

	if (bd->bi_flashsize)
		display_flash_config();
#endif

	if (bd->bi_dram[0].size)
		display_dram_config();

	gd->bd->bi_boot_params = malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	if (!gd->bd->bi_boot_params)
		puts("WARNING: Cannot allocate space for boot parameters\n");

	/* initialize environment */
	env_relocate();

	bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	stdio_init();
	jumptable_init();
	console_init_r();

	s = getenv("loadaddr");
	if (s)
		load_addr = simple_strtoul(s, NULL, 16);

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
	s = getenv("bootfile");
	if (s)
		copy_filename(BootFile, s, sizeof(BootFile));
#if defined(CONFIG_NET_MULTI)
	puts("Net:   ");
#endif
	eth_initialize(gd->bd);
#endif

	for (;;) {
		main_loop();
	}
}
