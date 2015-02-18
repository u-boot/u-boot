/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#include <asm/arch/hardware.h>

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
#include <mmc.h>
#endif
DECLARE_GLOBAL_DATA_PTR;

unsigned long monitor_flash_len;

__weak void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;
}

/* Weak aliases for optional board functions */
static int __do_nothing(void)
{
	return 0;
}
int board_postclk_init(void) __attribute__((weak, alias("__do_nothing")));
int board_early_init_r(void) __attribute__((weak, alias("__do_nothing")));

static int init_baudrate(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

static int display_banner (void)
{
	printf ("\n\n%s\n\n", version_string);
	printf ("U-Boot code: %08lx -> %08lx  data: %08lx -> %08lx\n",
		(unsigned long)_text, (unsigned long)_etext,
		(unsigned long)_data, (unsigned long)(&__bss_end));
	return 0;
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

	/* Initialize the global data pointer */
	memset(&gd_data, 0, sizeof(gd_data));
	gd = &gd_data;

	/* Perform initialization sequence */
	board_early_init_f();
	arch_cpu_init();
	board_postclk_init();
	env_init();
	init_baudrate();
	serial_init();
	console_init_f();
	display_banner();
	dram_init();

	/* If we have no SDRAM, we can't go on */
	if (gd->ram_size <= 0)
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
	addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size;
	monitor_len = (char *)(&__bss_end) - _text;

	/*
	 * Reserve memory for u-boot code, data and bss.
	 * Round down to next 4 kB limit.
	 */
	addr -= monitor_len;
	addr &= ~(4096UL - 1);
	monitor_addr = addr;

	/* Reserve memory for malloc() */
	addr -= CONFIG_SYS_MALLOC_LEN;

#ifdef CONFIG_LCD
#ifdef CONFIG_FB_ADDR
	printf("LCD: Frame buffer allocated at preset 0x%08x\n",
	       CONFIG_FB_ADDR);
	gd->fb_base = CONFIG_FB_ADDR;
#else
	addr = lcd_setmem(addr);
	printf("LCD: Frame buffer allocated at 0x%08lx\n", addr);
	gd->fb_base = addr;
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
	gd->start_addr_sp = addr;
	*(--new_sp) = 0;
	*(--new_sp) = 0;

	dram_init_banksize();

	memcpy(new_gd, gd, sizeof(gd_t));

	relocate_code((unsigned long)new_sp, new_gd, monitor_addr);
}

void board_init_r(gd_t *new_gd, ulong dest_addr)
{
#ifndef CONFIG_ENV_IS_NOWHERE
	extern char * env_name_spec;
#endif
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
	fixup_cmdtable(ll_entry_start(cmd_tbl_t, cmd),
			ll_entry_count(cmd_tbl_t, cmd));
#endif /* defined(CONFIG_NEEDS_MANUAL_RELOC) */

	/* there are some other pointer constants we must deal with */
#ifndef CONFIG_ENV_IS_NOWHERE
	env_name_spec += gd->reloc_off;
#endif

	timer_init();

	/* The malloc area is right below the monitor image in RAM */
	mem_malloc_init(CONFIG_SYS_MONITOR_BASE + gd->reloc_off -
			CONFIG_SYS_MALLOC_LEN, CONFIG_SYS_MALLOC_LEN);

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

	stdio_init();
	jumptable_init();
	console_init_r();

	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
	puts("Net:   ");
	eth_initialize(gd->bd);
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
	mmc_initialize(gd->bd);
#endif
	for (;;) {
		main_loop();
	}
}
