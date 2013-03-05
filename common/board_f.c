/*
 * Copyright (c) 2011 The Chromium OS Authors.
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

#include <common.h>
#include <linux/compiler.h>
#include <version.h>
#include <environment.h>
#include <fdtdec.h>
#include <initcall.h>
#include <logbuff.h>
#include <post.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <linux/compiler.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it if needed.
 */
#ifdef XTRN_DECLARE_GLOBAL_DATA_PTR
#undef	XTRN_DECLARE_GLOBAL_DATA_PTR
#define XTRN_DECLARE_GLOBAL_DATA_PTR	/* empty = allocate here */
DECLARE_GLOBAL_DATA_PTR = (gd_t *) (CONFIG_SYS_INIT_GD_ADDR);
#else
DECLARE_GLOBAL_DATA_PTR;
#endif

/*
 * sjg: IMO this code should be
 * refactored to a single function, something like:
 *
 * void led_set_state(enum led_colour_t colour, int on);
 */
/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
inline void __coloured_LED_init(void) {}
void coloured_LED_init(void)
	__attribute__((weak, alias("__coloured_LED_init")));
inline void __red_led_on(void) {}
void red_led_on(void) __attribute__((weak, alias("__red_led_on")));
inline void __red_led_off(void) {}
void red_led_off(void) __attribute__((weak, alias("__red_led_off")));
inline void __green_led_on(void) {}
void green_led_on(void) __attribute__((weak, alias("__green_led_on")));
inline void __green_led_off(void) {}
void green_led_off(void) __attribute__((weak, alias("__green_led_off")));
inline void __yellow_led_on(void) {}
void yellow_led_on(void) __attribute__((weak, alias("__yellow_led_on")));
inline void __yellow_led_off(void) {}
void yellow_led_off(void) __attribute__((weak, alias("__yellow_led_off")));
inline void __blue_led_on(void) {}
void blue_led_on(void) __attribute__((weak, alias("__blue_led_on")));
inline void __blue_led_off(void) {}
void blue_led_off(void) __attribute__((weak, alias("__blue_led_off")));

/*
 * Why is gd allocated a register? Prior to reloc it might be better to
 * just pass it around to each function in this file?
 *
 * After reloc one could argue that it is hardly used and doesn't need
 * to be in a register. Or if it is it should perhaps hold pointers to all
 * global data for all modules, so that post-reloc we can avoid the massive
 * literal pool we get on ARM. Or perhaps just encourage each module to use
 * a structure...
 */

/*
 * Could the CONFIG_SPL_BUILD infection become a flag in gd?
 */

static int init_baud_rate(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

static int display_text_info(void)
{
	ulong bss_start, bss_end;

	bss_start = _bss_start_ofs + _TEXT_BASE;
	bss_end = _bss_end_ofs + _TEXT_BASE;
	debug("U-Boot code: %08X -> %08lX  BSS: -> %08lX\n",
	      CONFIG_SYS_TEXT_BASE, bss_start, bss_end);

#ifdef CONFIG_MODEM_SUPPORT
	debug("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return 0;
}

static int announce_dram_init(void)
{
	puts("DRAM:  ");
	return 0;
}

static int show_dram_config(void)
{
	ulong size;

#ifdef CONFIG_NR_DRAM_BANKS
	int i;

	debug("\nRAM Configuration:\n");
	for (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
		debug("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
#ifdef DEBUG
		print_size(gd->bd->bi_dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");
#else
	size = gd->ram_size;
#endif

	print_size(size, "\n");

	return 0;
}

void __dram_init_banksize(void)
{
#if defined(CONFIG_NR_DRAM_BANKS) && defined(CONFIG_SYS_SDRAM_BASE)
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();
#endif
}

void dram_init_banksize(void)
	__attribute__((weak, alias("__dram_init_banksize")));

static int zero_global_data(void)
{
	memset((void *)gd, '\0', sizeof(gd_t));

	return 0;
}

static int setup_mon_len(void)
{
	gd->mon_len = _bss_end_ofs;
	return 0;
}

__weak int arch_cpu_init(void)
{
	return 0;
}

static int setup_fdt(void)
{
#ifdef CONFIG_OF_EMBED
	/* Get a pointer to the FDT */
	gd->fdt_blob = _binary_dt_dtb_start;
#elif defined CONFIG_OF_SEPARATE
	/* FDT is at end of image */
	gd->fdt_blob = (void *)(_end_ofs + CONFIG_SYS_TEXT_BASE);
#endif
	/* Allow the early environment to override the fdt address */
	gd->fdt_blob = (void *)getenv_ulong("fdtcontroladdr", 16,
						(uintptr_t)gd->fdt_blob);
	return 0;
}

/* Get the top of usable RAM */
__weak ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_top;
}

static int setup_dest_addr(void)
{
	debug("Monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug("Ram size: %08lX\n", (ulong)gd->ram_size);
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	/*
	 * Subtract specified amount of memory to hide so that it won't
	 * get "touched" at all by U-Boot. By fixing up gd->ram_size
	 * the Linux kernel should now get passed the now "corrected"
	 * memory size and won't touch it either. This should work
	 * for arch/ppc and arch/powerpc. Only Linux board ports in
	 * arch/powerpc with bootwrapper support, that recalculate the
	 * memory size from the SDRAM controller setup will have to
	 * get fixed.
	 */
	gd->ram_size -= CONFIG_SYS_MEM_TOP_HIDE;
#endif
#ifdef CONFIG_SYS_SDRAM_BASE
	gd->ram_top = CONFIG_SYS_SDRAM_BASE;
#endif
	gd->ram_top = board_get_usable_ram_top(gd->mon_len);
	gd->dest_addr = gd->ram_top;
	debug("Ram top: %08lX\n", (ulong)gd->ram_top);
	gd->dest_addr_sp = gd->dest_addr;
	return 0;
}

#if defined(CONFIG_LOGBUFFER) && !defined(CONFIG_ALT_LB_ADDR)
static int reserve_logbuffer(void)
{
	/* reserve kernel log buffer */
	gd->dest_addr -= LOGBUFF_RESERVE;
	debug("Reserving %dk for kernel logbuffer at %08lx\n", LOGBUFF_LEN,
		gd->dest_addr);
	return 0;
}
#endif

#ifdef CONFIG_PRAM
/* reserve protected RAM */
static int reserve_pram(void)
{
	ulong reg;

	reg = getenv_ulong("pram", 10, CONFIG_PRAM);
	gd->dest_addr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->dest_addr);
	return 0;
}
#endif /* CONFIG_PRAM */

/* Round memory pointer down to next 4 kB limit */
static int reserve_round_4k(void)
{
	gd->dest_addr &= ~(4096 - 1);
	return 0;
}

#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF)) && \
		defined(CONFIG_ARM)
static int reserve_mmu(void)
{
	/* reserve TLB table */
	gd->arch.tlb_size = 4096 * 4;
	gd->dest_addr -= gd->arch.tlb_size;

	/* round down to next 64 kB limit */
	gd->dest_addr &= ~(0x10000 - 1);

	gd->arch.tlb_addr = gd->dest_addr;
	debug("TLB table from %08lx to %08lx\n", gd->arch.tlb_addr,
	      gd->arch.tlb_addr + gd->arch.tlb_size);
	return 0;
}
#endif

#ifdef CONFIG_LCD
static int reserve_lcd(void)
{
#ifdef CONFIG_FB_ADDR
	gd->fb_base = CONFIG_FB_ADDR;
#else
	/* reserve memory for LCD display (always full pages) */
	gd->dest_addr = lcd_setmem(gd->dest_addr);
	gd->fb_base = gd->dest_addr;
#endif /* CONFIG_FB_ADDR */
	return 0;
}
#endif /* CONFIG_LCD */

static int reserve_uboot(void)
{
	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 4 kB limit
	 */
	gd->dest_addr -= gd->mon_len;
	gd->dest_addr &= ~(4096 - 1);

	debug("Reserving %ldk for U-Boot at: %08lx\n", gd->mon_len >> 10,
	      gd->dest_addr);
	return 0;
}

#ifndef CONFIG_SPL_BUILD
/* reserve memory for malloc() area */
static int reserve_malloc(void)
{
	gd->dest_addr_sp = gd->dest_addr - TOTAL_MALLOC_LEN;
	debug("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, gd->dest_addr_sp);
	return 0;
}

/* (permanently) allocate a Board Info struct */
static int reserve_board(void)
{
	gd->dest_addr_sp -= sizeof(bd_t);
	gd->bd = (bd_t *)gd->dest_addr_sp;
	memset(gd->bd, '\0', sizeof(bd_t));
	debug("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof(bd_t), gd->dest_addr_sp);
	return 0;
}
#endif

static int setup_machine(void)
{
#ifdef CONFIG_MACH_TYPE
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE; /* board id for Linux */
#endif
	return 0;
}

static int reserve_global_data(void)
{
	gd->dest_addr_sp -= sizeof(gd_t);
	gd->new_gd = (gd_t *)gd->dest_addr_sp;
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
			sizeof(gd_t), gd->dest_addr_sp);
	return 0;
}

static int reserve_fdt(void)
{
	/*
	 * If the device tree is sitting immediate above our image then we
	 * must relocate it. If it is embedded in the data section, then it
	 * will be relocated with other data.
	 */
	if (gd->fdt_blob) {
		gd->fdt_size = ALIGN(fdt_totalsize(gd->fdt_blob) + 0x1000, 32);

		gd->dest_addr_sp -= gd->fdt_size;
		gd->new_fdt = (void *)gd->dest_addr_sp;
		debug("Reserving %lu Bytes for FDT at: %p\n",
		      gd->fdt_size, gd->new_fdt);
	}

	return 0;
}

static int reserve_stacks(void)
{
#ifdef CONFIG_SPL_BUILD
# ifdef CONFIG_ARM
	gd->dest_addr_sp -= 128;	/* leave 32 words for abort-stack */
	gd->irq_sp = gd->dest_addr_sp;
# endif
#else

	/* setup stack pointer for exceptions */
	gd->dest_addr_sp -= 16;
	gd->dest_addr_sp &= ~0xf;
	gd->irq_sp = gd->dest_addr_sp;

	/*
	 * Handle architecture-specific things here
	 * TODO(sjg@chromium.org): Perhaps create arch_reserve_stack()
	 * to handle this and put in arch/xxx/lib/stack.c
	 */
# ifdef CONFIG_ARM
#  ifdef CONFIG_USE_IRQ
	gd->dest_addr_sp -= (CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ);
	debug("Reserving %zu Bytes for IRQ stack at: %08lx\n",
		CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ, gd->dest_addr_sp);

	/* 8-byte alignment for ARM ABI compliance */
	gd->dest_addr_sp &= ~0x07;
#  endif
	/* leave 3 words for abort-stack, plus 1 for alignment */
	gd->dest_addr_sp -= 16;
# endif /* Architecture specific code */

	return 0;
#endif
}

static int display_new_sp(void)
{
	debug("New Stack Pointer is: %08lx\n", gd->dest_addr_sp);

	return 0;
}

#ifdef CONFIG_POST
static int init_post(void)
{
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));

	return 0;
}
#endif

static int setup_baud_rate(void)
{
	/* Ick, can we get rid of this line? */
	gd->bd->bi_baudrate = gd->baudrate;

	return 0;
}

static int setup_dram_config(void)
{
	/* Ram is board specific, so move it to board code ... */
	dram_init_banksize();

	return 0;
}

static int reloc_fdt(void)
{
	if (gd->new_fdt) {
		memcpy(gd->new_fdt, gd->fdt_blob, gd->fdt_size);
		gd->fdt_blob = gd->new_fdt;
	}

	return 0;
}

static int setup_reloc(void)
{
	gd->relocaddr = gd->dest_addr;
	gd->start_addr_sp = gd->dest_addr_sp;
	gd->reloc_off = gd->dest_addr - CONFIG_SYS_TEXT_BASE;
	memcpy(gd->new_gd, (char *)gd, sizeof(gd_t));

	debug("Relocation Offset is: %08lx\n", gd->reloc_off);
	debug("Relocating to %08lx, new gd at %p, sp at %08lx\n",
	      gd->dest_addr, gd->new_gd, gd->dest_addr_sp);

	return 0;
}

/* ARM calls relocate_code from its crt0.S */
#if !defined(CONFIG_ARM)

static int jump_to_copy(void)
{
	relocate_code(gd->dest_addr_sp, gd->new_gd, gd->dest_addr);

	return 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
static int mark_bootstage(void)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	return 0;
}

static init_fnc_t init_sequence_f[] = {
	setup_fdt,
	setup_mon_len,
	arch_cpu_init,		/* basic arch cpu dependent setup */
	mark_bootstage,
#ifdef CONFIG_OF_CONTROL
	fdtdec_check_fdt,
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
	timer_init,		/* initialize timer */
#ifdef CONFIG_BOARD_POSTCLK_INIT
	board_postclk_init,
#endif
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	env_init,		/* initialize environment */
	init_baud_rate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_options,	/* say that we are here */
	display_text_info,	/* show debugging info if required */
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
	announce_dram_init,
	/* TODO: unify all these dram functions? */
#ifdef CONFIG_ARM
	dram_init,		/* configure available RAM banks */
#endif
#ifdef CONFIG_POST
	init_post,
#endif
	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - area that won't get touched by U-Boot and Linux (optional)
	 *  - kernel log buffer
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	 */
	setup_dest_addr,
#if defined(CONFIG_LOGBUFFER) && !defined(CONFIG_ALT_LB_ADDR)
	reserve_logbuffer,
#endif
#ifdef CONFIG_PRAM
	reserve_pram,
#endif
	reserve_round_4k,
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF)) && \
		defined(CONFIG_ARM)
	reserve_mmu,
#endif
#ifdef CONFIG_LCD
	reserve_lcd,
#endif
	reserve_uboot,
#ifndef CONFIG_SPL_BUILD
	reserve_malloc,
	reserve_board,
#endif
	setup_machine,
	reserve_global_data,
	reserve_fdt,
	reserve_stacks,
	setup_dram_config,
	show_dram_config,
	setup_baud_rate,
	display_new_sp,
	reloc_fdt,
	setup_reloc,
#ifndef CONFIG_ARM
	jump_to_copy,
#endif
	NULL,
};

void board_init_f(ulong boot_flags)
{
	gd_t data;

	gd = &data;

	gd->flags = boot_flags;

	if (initcall_run_list(init_sequence_f))
		hang();

#ifndef CONFIG_ARM
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
