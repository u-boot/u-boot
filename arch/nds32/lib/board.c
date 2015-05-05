/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SYS_I2C)
#include <i2c.h>
#endif

ulong monitor_flash_len;

/*
 * Init Utilities
 */

#if !defined(CONFIG_BAUDRATE)
#define CONFIG_BAUDRATE 38400
#endif
static int init_baudrate(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config(void)
{
	int i;

#ifdef DEBUG
	puts("RAM Configuration:\n");

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		printf("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size(gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		size += gd->bd->bi_dram[i].size;

	puts("DRAM:  ");
	print_size(size, "\n");
#endif

	return 0;
}

#ifndef CONFIG_SYS_NO_FLASH
static void display_flash_config(ulong size)
{
	puts("Flash: ");
	print_size(size, "\n");
}
#endif /* CONFIG_SYS_NO_FLASH */

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
#include <pci.h>
static int nds32_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI */

#if defined(CONFIG_PMU) || defined(CONFIG_PCU)
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
static int pmu_init(void)
{
#if defined(CONFIG_FTPMU010_POWER)
#ifdef __NDS32_N1213_43U1H__	/* AG101: internal definition in toolchain */
	ftpmu010_sdram_clk_disable(CONFIG_SYS_FTPMU010_PDLLCR0_HCLKOUTDIS);
	ftpmu010_mfpsr_select_dev(FTPMU010_MFPSR_AC97CLKSEL);
	ftpmu010_sdramhtc_set(CONFIG_SYS_FTPMU010_SDRAMHTC);
#endif	/* __NDS32_N1213_43U1H__ */
#endif
	return 0;
}
#endif
#endif

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
typedef int (init_fnc_t)(void);

void __dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;
}
void dram_init_banksize(void)
	__attribute__((weak, alias("__dram_init_banksize")));

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
#if defined(CONFIG_PMU) || defined(CONFIG_PCU)
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	pmu_init,
#endif
#endif
	board_init,		/* basic board dependent setup */
#if defined(CONFIG_USE_IRQ)
	interrupt_init,		/* set up exceptions */
#endif
	timer_init,		/* initialize timer */
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SYS_I2C)
	init_func_i2c,
#endif
	dram_init,		/* configure available RAM banks */
	display_dram_config,
	NULL,
};

void board_init_f(ulong bootflag)
{
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	gd_t *id;
	ulong addr, addr_sp;

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) ((CONFIG_SYS_INIT_SP_ADDR) & ~0x07);

	memset((void *)gd, 0, GENERATED_GBL_DATA_SIZE);

	gd->mon_len = (unsigned int)(&__bss_end) - (unsigned int)(&_start);

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0)
			hang();
	}

	debug("monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug("ramsize: %08lX\n", gd->ram_size);

	addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size;

	/* round down to next 4 kB limit */
	addr &= ~(4096 - 1);
	debug("Top of RAM usable for U-Boot at: %08lx\n", addr);

#ifdef CONFIG_LCD
#ifdef CONFIG_FB_ADDR
	gd->fb_base = CONFIG_FB_ADDR;
#else
	/* reserve memory for LCD display (always full pages) */
	addr = lcd_setmem(addr);
	gd->fb_base = addr;
#endif /* CONFIG_FB_ADDR */
#endif /* CONFIG_LCD */

	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 4 kB limit
	 */
	addr -= gd->mon_len;
	addr &= ~(4096 - 1);

	debug("Reserving %ldk for U-Boot at: %08lx\n", gd->mon_len >> 10, addr);

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
	addr_sp -= GENERATED_BD_INFO_SIZE;
	bd = (bd_t *) addr_sp;
	gd->bd = bd;
	memset((void *)bd, 0, GENERATED_BD_INFO_SIZE);
	debug("Reserving %zu Bytes for Board Info at: %08lx\n",
			GENERATED_BD_INFO_SIZE, addr_sp);

	addr_sp -= GENERATED_GBL_DATA_SIZE;
	id = (gd_t *) addr_sp;
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
			GENERATED_GBL_DATA_SIZE, addr_sp);

	/* setup stackpointer for exeptions */
	gd->irq_sp = addr_sp;
#ifdef CONFIG_USE_IRQ
	addr_sp -= (CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ);
	debug("Reserving %zu Bytes for IRQ stack at: %08lx\n",
		CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ, addr_sp);
#endif
	/* leave 3 words for abort-stack    */
	addr_sp -= 12;

	/* 8-byte alignment for ABI compliance */
	addr_sp &= ~0x07;
	debug("New Stack Pointer is: %08lx\n", addr_sp);

	/* Ram isn't board specific, so move it to board code ... */
	dram_init_banksize();
	display_dram_config();	/* and display it */

	gd->relocaddr = addr;
	gd->start_addr_sp = addr_sp;

	gd->reloc_off = addr - _TEXT_BASE;

	debug("relocation Offset is: %08lx\n", gd->reloc_off);
	memcpy(id, (void *)gd, GENERATED_GBL_DATA_SIZE);

	relocate_code(addr_sp, id, addr);

	/* NOTREACHED - relocate_code() does not return */
}

/*
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 */
void board_init_r(gd_t *id, ulong dest_addr)
{
	bd_t *bd;
	ulong malloc_start;

	gd = id;
	bd = gd->bd;

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	monitor_flash_len = (ulong)&_end - (ulong)&_start;
	debug("monitor flash len: %08lX\n", monitor_flash_len);

	board_init();	/* Setup chipselects */

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	/*
	 * We have to relocate the command table manually
	 */
	fixup_cmdtable(ll_entry_start(cmd_tbl_t, cmd),
			ll_entry_count(cmd_tbl_t, cmd));
#endif /* defined(CONFIG_NEEDS_MANUAL_RELOC) */

	serial_initialize();

	debug("Now running in RAM - U-Boot at: %08lx\n", dest_addr);

	/* The Malloc area is immediately below the monitor copy in DRAM */
	malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_init(malloc_start, TOTAL_MALLOC_LEN);

#ifndef CONFIG_SYS_NO_FLASH
	/* configure available FLASH banks */
	gd->bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
	gd->bd->bi_flashsize = flash_init();
	gd->bd->bi_flashoffset = CONFIG_SYS_FLASH_BASE + gd->bd->bi_flashsize;

	if (gd->bd->bi_flashsize)
			display_flash_config(gd->bd->bi_flashsize);
#endif /* CONFIG_SYS_NO_FLASH */

#if defined(CONFIG_CMD_NAND)
	puts("NAND:  ");
	nand_init();		/* go init the NAND */
#endif

#if defined(CONFIG_CMD_IDE)
	puts("IDE:   ");
	ide_init();
#endif

#ifdef CONFIG_GENERIC_MMC
	puts("MMC:   ");
	mmc_initialize(gd->bd);
#endif

#if defined(CONFIG_SYS_I2C_ADAPTERS)
	i2c_reloc_fixup();
#endif

	/* initialize environment */
	env_relocate();

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
	puts("PCI:   ");
	nds32_pci_init();
#endif

	stdio_init();	/* get the devices list going. */

	jumptable_init();

#if defined(CONFIG_API)
	/* Initialize API */
	api_init();
#endif

	console_init_r();	/* fully init console as a device */

#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

#if defined(CONFIG_USE_IRQ)
	/* set up exceptions */
	interrupt_init();
	/* enable exceptions */
	enable_interrupts();
#endif

	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);

#ifdef CONFIG_BOARD_LATE_INIT
	board_late_init();
#endif

#if defined(CONFIG_CMD_NET)
	puts("Net:   ");

	eth_initialize();
#if defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;)
		main_loop();

	/* NOTREACHED - no way out of command loop except booting */
}
