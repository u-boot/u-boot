/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/compiler.h>
#include <version.h>
#include <environment.h>
#include <fdtdec.h>
#include <fs.h>
#if defined(CONFIG_CMD_IDE)
#include <ide.h>
#endif
#include <i2c.h>
#include <initcall.h>
#include <logbuff.h>

/* TODO: Can we move these into arch/ headers? */
#ifdef CONFIG_8xx
#include <mpc8xx.h>
#endif
#ifdef CONFIG_5xx
#include <mpc5xx.h>
#endif
#ifdef CONFIG_MPC5xxx
#include <mpc5xxx.h>
#endif

#include <os.h>
#include <post.h>
#include <spi.h>
#include <trace.h>
#include <watchdog.h>
#include <asm/errno.h>
#include <asm/io.h>
#ifdef CONFIG_MP
#include <asm/mp.h>
#endif
#include <asm/sections.h>
#ifdef CONFIG_X86
#include <asm/init_helpers.h>
#include <asm/relocate.h>
#endif
#ifdef CONFIG_SANDBOX
#include <asm/state.h>
#endif
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

#if defined(CONFIG_WATCHDOG)
static int init_func_watchdog_init(void)
{
	puts("       Watchdog enabled\n");
	WATCHDOG_RESET();

	return 0;
}

int init_func_watchdog_reset(void)
{
	WATCHDOG_RESET();

	return 0;
}
#endif /* CONFIG_WATCHDOG */

void __board_add_ram_info(int use_default)
{
	/* please define platform specific board_add_ram_info() */
}

void board_add_ram_info(int)
	__attribute__ ((weak, alias("__board_add_ram_info")));

static int init_baud_rate(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

static int display_text_info(void)
{
#ifndef CONFIG_SANDBOX
	ulong bss_start, bss_end;

	bss_start = (ulong)&__bss_start;
	bss_end = (ulong)&__bss_end;

	debug("U-Boot code: %08X -> %08lX  BSS: -> %08lX\n",
	      CONFIG_SYS_TEXT_BASE, bss_start, bss_end);
#endif

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

#ifdef CONFIG_PPC
static int init_func_ram(void)
{
#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#else
	int board_type = 0;	/* use dummy arg */
#endif

	gd->ram_size = initdram(board_type);

	if (gd->ram_size > 0)
		return 0;

	puts("*** failed ***\n");
	return 1;
}
#endif

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

	print_size(size, "");
	board_add_ram_info(0);
	putc('\n');

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

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SYS_I2C)
static int init_func_i2c(void)
{
	puts("I2C:   ");
#ifdef CONFIG_SYS_I2C
	i2c_init_all();
#else
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
	puts("ready\n");
	return 0;
}
#endif

#if defined(CONFIG_HARD_SPI)
static int init_func_spi(void)
{
	puts("SPI:   ");
	spi_init();
	puts("ready\n");
	return 0;
}
#endif

__maybe_unused
static int zero_global_data(void)
{
	memset((void *)gd, '\0', sizeof(gd_t));

	return 0;
}

static int setup_mon_len(void)
{
#ifdef __ARM__
	gd->mon_len = (ulong)&__bss_end - (ulong)_start;
#elif defined(CONFIG_SANDBOX)
	gd->mon_len = (ulong)&_end - (ulong)_init;
#else
	/* TODO: use (ulong)&__bss_end - (ulong)&__text_start; ? */
	gd->mon_len = (ulong)&__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	return 0;
}

__weak int arch_cpu_init(void)
{
	return 0;
}

#ifdef CONFIG_OF_HOSTFILE

#define CHECK(x)		err = (x); if (err) goto failed;

/* Create an empty device tree blob */
static int make_empty_fdt(void *fdt)
{
	int err;

	CHECK(fdt_create(fdt, 256));
	CHECK(fdt_finish_reservemap(fdt));
	CHECK(fdt_begin_node(fdt, ""));
	CHECK(fdt_end_node(fdt));
	CHECK(fdt_finish(fdt));

	return 0;
failed:
	printf("Unable to create empty FDT: %s\n", fdt_strerror(err));
	return -EACCES;
}

static int read_fdt_from_file(void)
{
	struct sandbox_state *state = state_get_current();
	void *blob;
	int size;
	int err;

	blob = map_sysmem(CONFIG_SYS_FDT_LOAD_ADDR, 0);
	if (!state->fdt_fname) {
		err = make_empty_fdt(blob);
		if (!err)
			goto done;
		return err;
	}
	err = fs_set_blk_dev("host", NULL, FS_TYPE_SANDBOX);
	if (err)
		return err;
	size = fs_read(state->fdt_fname, CONFIG_SYS_FDT_LOAD_ADDR, 0, 0);
	if (size < 0)
		return -EIO;

done:
	gd->fdt_blob = blob;

	return 0;
}
#endif

#ifdef CONFIG_SANDBOX
static int setup_ram_buf(void)
{
	struct sandbox_state *state = state_get_current();

	gd->arch.ram_buf = state->ram_buf;
	gd->ram_size = state->ram_size;

	return 0;
}
#endif

static int setup_fdt(void)
{
#ifdef CONFIG_OF_EMBED
	/* Get a pointer to the FDT */
	gd->fdt_blob = __dtb_dt_begin;
#elif defined CONFIG_OF_SEPARATE
	/* FDT is at end of image */
	gd->fdt_blob = (ulong *)&_end;
#elif defined(CONFIG_OF_HOSTFILE)
	if (read_fdt_from_file()) {
		puts("Failed to read control FDT\n");
		return -1;
	}
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
	gd->ram_top += get_effective_memsize();
	gd->ram_top = board_get_usable_ram_top(gd->mon_len);
	gd->relocaddr = gd->ram_top;
	debug("Ram top: %08lX\n", (ulong)gd->ram_top);
#if defined(CONFIG_MP) && (defined(CONFIG_MPC86xx) || defined(CONFIG_E500))
	/*
	 * We need to make sure the location we intend to put secondary core
	 * boot code is reserved and not used by any part of u-boot
	 */
	if (gd->relocaddr > determine_mp_bootpg(NULL)) {
		gd->relocaddr = determine_mp_bootpg(NULL);
		debug("Reserving MP boot page to %08lx\n", gd->relocaddr);
	}
#endif
	return 0;
}

#if defined(CONFIG_LOGBUFFER) && !defined(CONFIG_ALT_LB_ADDR)
static int reserve_logbuffer(void)
{
	/* reserve kernel log buffer */
	gd->relocaddr -= LOGBUFF_RESERVE;
	debug("Reserving %dk for kernel logbuffer at %08lx\n", LOGBUFF_LEN,
		gd->relocaddr);
	return 0;
}
#endif

#ifdef CONFIG_PRAM
/* reserve protected RAM */
static int reserve_pram(void)
{
	ulong reg;

	reg = getenv_ulong("pram", 10, CONFIG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	return 0;
}
#endif /* CONFIG_PRAM */

/* Round memory pointer down to next 4 kB limit */
static int reserve_round_4k(void)
{
	gd->relocaddr &= ~(4096 - 1);
	return 0;
}

#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF)) && \
		defined(CONFIG_ARM)
static int reserve_mmu(void)
{
	/* reserve TLB table */
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->relocaddr -= gd->arch.tlb_size;

	/* round down to next 64 kB limit */
	gd->relocaddr &= ~(0x10000 - 1);

	gd->arch.tlb_addr = gd->relocaddr;
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
	gd->relocaddr = lcd_setmem(gd->relocaddr);
	gd->fb_base = gd->relocaddr;
#endif /* CONFIG_FB_ADDR */
	return 0;
}
#endif /* CONFIG_LCD */

static int reserve_trace(void)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %dk for trace data at: %08lx\n",
	      CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	return 0;
}

#if defined(CONFIG_VIDEO) && (!defined(CONFIG_PPC) || defined(CONFIG_8xx)) \
		&& !defined(CONFIG_ARM) && !defined(CONFIG_X86)
static int reserve_video(void)
{
	/* reserve memory for video display (always full pages) */
	gd->relocaddr = video_setmem(gd->relocaddr);
	gd->fb_base = gd->relocaddr;

	return 0;
}
#endif

static int reserve_uboot(void)
{
	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 4 kB limit
	 */
	gd->relocaddr -= gd->mon_len;
	gd->relocaddr &= ~(4096 - 1);
#ifdef CONFIG_E500
	/* round down to next 64 kB limit so that IVPR stays aligned */
	gd->relocaddr &= ~(65536 - 1);
#endif

	debug("Reserving %ldk for U-Boot at: %08lx\n", gd->mon_len >> 10,
	      gd->relocaddr);

	gd->start_addr_sp = gd->relocaddr;

	return 0;
}

#ifndef CONFIG_SPL_BUILD
/* reserve memory for malloc() area */
static int reserve_malloc(void)
{
	gd->start_addr_sp = gd->start_addr_sp - TOTAL_MALLOC_LEN;
	debug("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
	return 0;
}

/* (permanently) allocate a Board Info struct */
static int reserve_board(void)
{
	gd->start_addr_sp -= sizeof(bd_t);
	gd->bd = (bd_t *)map_sysmem(gd->start_addr_sp, sizeof(bd_t));
	memset(gd->bd, '\0', sizeof(bd_t));
	debug("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof(bd_t), gd->start_addr_sp);
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
	gd->start_addr_sp -= sizeof(gd_t);
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, sizeof(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
			sizeof(gd_t), gd->start_addr_sp);
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

		gd->start_addr_sp -= gd->fdt_size;
		gd->new_fdt = map_sysmem(gd->start_addr_sp, gd->fdt_size);
		debug("Reserving %lu Bytes for FDT at: %08lx\n",
		      gd->fdt_size, gd->start_addr_sp);
	}

	return 0;
}

static int reserve_stacks(void)
{
#ifdef CONFIG_SPL_BUILD
# ifdef CONFIG_ARM
	gd->start_addr_sp -= 128;	/* leave 32 words for abort-stack */
	gd->irq_sp = gd->start_addr_sp;
# endif
#else
# ifdef CONFIG_PPC
	ulong *s;
# endif

	/* setup stack pointer for exceptions */
	gd->start_addr_sp -= 16;
	gd->start_addr_sp &= ~0xf;
	gd->irq_sp = gd->start_addr_sp;

	/*
	 * Handle architecture-specific things here
	 * TODO(sjg@chromium.org): Perhaps create arch_reserve_stack()
	 * to handle this and put in arch/xxx/lib/stack.c
	 */
# if defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
#  ifdef CONFIG_USE_IRQ
	gd->start_addr_sp -= (CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ);
	debug("Reserving %zu Bytes for IRQ stack at: %08lx\n",
		CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ, gd->start_addr_sp);

	/* 8-byte alignment for ARM ABI compliance */
	gd->start_addr_sp &= ~0x07;
#  endif
	/* leave 3 words for abort-stack, plus 1 for alignment */
	gd->start_addr_sp -= 16;
# elif defined(CONFIG_PPC)
	/* Clear initial stack frame */
	s = (ulong *) gd->start_addr_sp;
	*s = 0; /* Terminate back chain */
	*++s = 0; /* NULL return address */
# endif /* Architecture specific code */

	return 0;
#endif
}

static int display_new_sp(void)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	return 0;
}

#ifdef CONFIG_PPC
static int setup_board_part1(void)
{
	bd_t *bd = gd->bd;

	/*
	 * Save local variables to board info struct
	 */

	bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;	/* start of memory */
	bd->bi_memsize = gd->ram_size;			/* size in bytes */

#ifdef CONFIG_SYS_SRAM_BASE
	bd->bi_sramstart = CONFIG_SYS_SRAM_BASE;	/* start of SRAM */
	bd->bi_sramsize = CONFIG_SYS_SRAM_SIZE;		/* size  of SRAM */
#endif

#if defined(CONFIG_8xx) || defined(CONFIG_8260) || defined(CONFIG_5xx) || \
		defined(CONFIG_E500) || defined(CONFIG_MPC86xx)
	bd->bi_immr_base = CONFIG_SYS_IMMR;	/* base  of IMMR register     */
#endif
#if defined(CONFIG_MPC5xxx)
	bd->bi_mbar_base = CONFIG_SYS_MBAR;	/* base of internal registers */
#endif
#if defined(CONFIG_MPC83xx)
	bd->bi_immrbar = CONFIG_SYS_IMMR;
#endif

	return 0;
}

static int setup_board_part2(void)
{
	bd_t *bd = gd->bd;

	bd->bi_intfreq = gd->cpu_clk;	/* Internal Freq, in Hz */
	bd->bi_busfreq = gd->bus_clk;	/* Bus Freq,      in Hz */
#if defined(CONFIG_CPM2)
	bd->bi_cpmfreq = gd->arch.cpm_clk;
	bd->bi_brgfreq = gd->arch.brg_clk;
	bd->bi_sccfreq = gd->arch.scc_clk;
	bd->bi_vco = gd->arch.vco_out;
#endif /* CONFIG_CPM2 */
#if defined(CONFIG_MPC512X)
	bd->bi_ipsfreq = gd->arch.ips_clk;
#endif /* CONFIG_MPC512X */
#if defined(CONFIG_MPC5xxx)
	bd->bi_ipbfreq = gd->arch.ipb_clk;
	bd->bi_pcifreq = gd->pci_clk;
#endif /* CONFIG_MPC5xxx */

	return 0;
}
#endif

#ifdef CONFIG_SYS_EXTBDINFO
static int setup_board_extra(void)
{
	bd_t *bd = gd->bd;

	strncpy((char *) bd->bi_s_version, "1.2", sizeof(bd->bi_s_version));
	strncpy((char *) bd->bi_r_version, U_BOOT_VERSION,
		sizeof(bd->bi_r_version));

	bd->bi_procfreq = gd->cpu_clk;	/* Processor Speed, In Hz */
	bd->bi_plb_busfreq = gd->bus_clk;
#if defined(CONFIG_405GP) || defined(CONFIG_405EP) || \
		defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
		defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	bd->bi_pci_busfreq = get_PCI_freq();
	bd->bi_opbfreq = get_OPB_freq();
#elif defined(CONFIG_XILINX_405)
	bd->bi_pci_busfreq = get_PCI_freq();
#endif

	return 0;
}
#endif

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
	gd->reloc_off = gd->relocaddr - CONFIG_SYS_TEXT_BASE;
	memcpy(gd->new_gd, (char *)gd, sizeof(gd_t));

	debug("Relocation Offset is: %08lx\n", gd->reloc_off);
	debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
	      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
	      gd->start_addr_sp);

	return 0;
}

/* ARM calls relocate_code from its crt0.S */
#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

static int jump_to_copy(void)
{
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#ifdef CONFIG_X86
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	board_init_f_r_trampoline(gd->start_addr_sp);
#else
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

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
#ifdef CONFIG_SANDBOX
	setup_ram_buf,
#endif
	setup_mon_len,
	setup_fdt,
	trace_early_init,
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/* TODO: can this go into arch_cpu_init()? */
	probecpu,
#endif
	arch_cpu_init,		/* basic arch cpu dependent setup */
#ifdef CONFIG_X86
	cpu_init_f,		/* TODO(sjg@chromium.org): remove */
# ifdef CONFIG_OF_CONTROL
	find_fdt,		/* TODO(sjg@chromium.org): remove */
# endif
#endif
	mark_bootstage,
#ifdef CONFIG_OF_CONTROL
	fdtdec_check_fdt,
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
	/* TODO: can any of this go into arch_cpu_init()? */
#if defined(CONFIG_PPC) && !defined(CONFIG_8xx_CPUCLK_DEFAULT)
	get_clocks,		/* get CPU and bus clocks (etc.) */
#if defined(CONFIG_TQM8xxL) && !defined(CONFIG_TQM866M) \
		&& !defined(CONFIG_TQM885D)
	adjust_sdram_tbs_8xx,
#endif
	/* TODO: can we rename this to timer_init()? */
	init_timebase,
#endif
#ifdef CONFIG_ARM
	timer_init,		/* initialize timer */
#endif
#ifdef CONFIG_SYS_ALLOC_DPRAM
#if !defined(CONFIG_CPM2)
	dpram_init,
#endif
#endif
#if defined(CONFIG_BOARD_POSTCLK_INIT)
	board_postclk_init,
#endif
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	env_init,		/* initialize environment */
#if defined(CONFIG_8xx_CPUCLK_DEFAULT)
	/* get CPU and bus clocks according to the environment variable */
	get_clocks_866,
	/* adjust sdram refresh rate according to the new clock */
	sdram_adjust_866,
	init_timebase,
#endif
	init_baud_rate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
#ifdef CONFIG_SANDBOX
	sandbox_early_getopt_check,
#endif
#ifdef CONFIG_OF_CONTROL
	fdtdec_prepare_fdt,
#endif
	display_options,	/* say that we are here */
	display_text_info,	/* show debugging info if required */
#if defined(CONFIG_8260)
	prt_8260_rsr,
	prt_8260_clks,
#endif /* CONFIG_8260 */
#if defined(CONFIG_MPC83xx)
	prt_83xx_rsr,
#endif
#ifdef CONFIG_PPC
	checkcpu,
#endif
	print_cpuinfo,		/* display cpu info (and speed) */
#if defined(CONFIG_MPC5xxx)
	prt_mpc5xxx_clks,
#endif /* CONFIG_MPC5xxx */
#if defined(CONFIG_DISPLAY_BOARDINFO)
	checkboard,		/* display board info */
#endif
	INIT_FUNC_WATCHDOG_INIT
#if defined(CONFIG_MISC_INIT_F)
	misc_init_f,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SYS_I2C)
	init_func_i2c,
#endif
#if defined(CONFIG_HARD_SPI)
	init_func_spi,
#endif
#ifdef CONFIG_X86
	dram_init_f,		/* configure available RAM banks */
	calculate_relocation_address,
#endif
	announce_dram_init,
	/* TODO: unify all these dram functions? */
#ifdef CONFIG_ARM
	dram_init,		/* configure available RAM banks */
#endif
#ifdef CONFIG_PPC
	init_func_ram,
#endif
#ifdef CONFIG_POST
	post_init_f,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_SYS_DRAM_TEST)
	testdram,
#endif /* CONFIG_SYS_DRAM_TEST */
	INIT_FUNC_WATCHDOG_RESET

#ifdef CONFIG_POST
	init_post,
#endif
	INIT_FUNC_WATCHDOG_RESET
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
	reserve_trace,
	/* TODO: Why the dependency on CONFIG_8xx? */
#if defined(CONFIG_VIDEO) && (!defined(CONFIG_PPC) || defined(CONFIG_8xx)) \
		&& !defined(CONFIG_ARM) && !defined(CONFIG_X86)
	reserve_video,
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
#ifdef CONFIG_PPC
	setup_board_part1,
	INIT_FUNC_WATCHDOG_RESET
	setup_board_part2,
#endif
	setup_baud_rate,
	display_new_sp,
#ifdef CONFIG_SYS_EXTBDINFO
	setup_board_extra,
#endif
	INIT_FUNC_WATCHDOG_RESET
	reloc_fdt,
	setup_reloc,
#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)
	jump_to_copy,
#endif
	NULL,
};

void board_init_f(ulong boot_flags)
{
#ifndef CONFIG_X86
	gd_t data;

	gd = &data;
#endif

	/*
	 * Clear global data before it is accessed at debug print
	 * in initcall_run_list. Otherwise the debug print probably
	 * get the wrong vaule of gd->have_console.
	 */
#if !defined(CONFIG_CPM2) && !defined(CONFIG_MPC512X) && \
		!defined(CONFIG_MPC83xx) && !defined(CONFIG_MPC85xx) && \
		!defined(CONFIG_MPC86xx) && !defined(CONFIG_X86)
	zero_global_data();
#endif

	gd->flags = boot_flags;
	gd->have_console = 0;

	if (initcall_run_list(init_sequence_f))
		hang();

#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#ifdef CONFIG_X86
/*
 * For now this code is only used on x86.
 *
 * init_sequence_f_r is the list of init functions which are run when
 * U-Boot is executing from Flash with a semi-limited 'C' environment.
 * The following limitations must be considered when implementing an
 * '_f_r' function:
 *  - 'static' variables are read-only
 *  - Global Data (gd->xxx) is read/write
 *
 * The '_f_r' sequence must, as a minimum, copy U-Boot to RAM (if
 * supported).  It _should_, if possible, copy global data to RAM and
 * initialise the CPU caches (to speed up the relocation process)
 *
 * NOTE: At present only x86 uses this route, but it is intended that
 * all archs will move to this when generic relocation is implemented.
 */
static init_fnc_t init_sequence_f_r[] = {
	init_cache_f_r,
	copy_uboot_to_ram,
	clear_bss,
	do_elf_reloc_fixups,

	NULL,
};

void board_init_f_r(void)
{
	if (initcall_run_list(init_sequence_f_r))
		hang();

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)(gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#endif /* CONFIG_X86 */
