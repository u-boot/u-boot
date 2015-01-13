/*
 * U-boot - cpu.c CPU specific functions
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <serial.h>
#include <version.h>
#include <i2c.h>

#include <asm/blackfin.h>
#include <asm/cplb.h>
#include <asm/clock.h>
#include <asm/mach-common/bits/core.h>
#include <asm/mach-common/bits/ebiu.h>
#include <asm/mach-common/bits/trace.h>

#include "cpu.h"
#include "initcode.h"

ulong bfin_poweron_retx;
DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CORE1_RUN) && defined(COREB_L1_CODE_START)
void bfin_core1_start(void)
{
#ifdef BF561_FAMILY
	/* Enable core 1 */
	bfin_write_SYSCR(bfin_read_SYSCR() & ~0x0020);
#else
	/* Enable core 1 */
	bfin_write32(RCU0_SVECT1, COREB_L1_CODE_START);
	bfin_write32(RCU0_CRCTL, 0);

	bfin_write32(RCU0_CRCTL, 0x2);

	/* Check if core 1 starts */
	while (!(bfin_read32(RCU0_CRSTAT) & 0x2))
		continue;

	bfin_write32(RCU0_CRCTL, 0);

	/* flag to notify cces core 1 application */
	bfin_write32(SDU0_MSG_SET, (1 << 19));
#endif
}
#endif

__attribute__((always_inline))
static inline void serial_early_puts(const char *s)
{
#ifdef CONFIG_DEBUG_EARLY_SERIAL
	serial_puts("Early: ");
	serial_puts(s);
#endif
}

static int global_board_data_init(void)
{
#ifndef CONFIG_SYS_GBL_DATA_ADDR
# define CONFIG_SYS_GBL_DATA_ADDR 0
#endif
#ifndef CONFIG_SYS_BD_INFO_ADDR
# define CONFIG_SYS_BD_INFO_ADDR 0
#endif

	bd_t *bd;

	if (CONFIG_SYS_GBL_DATA_ADDR) {
		gd = (gd_t *)(CONFIG_SYS_GBL_DATA_ADDR);
		memset((void *)gd, 0, GENERATED_GBL_DATA_SIZE);
	} else {
		static gd_t _bfin_gd;
		gd = &_bfin_gd;
	}
	if (CONFIG_SYS_BD_INFO_ADDR) {
		bd = (bd_t *)(CONFIG_SYS_BD_INFO_ADDR);
		memset(bd, 0, GENERATED_BD_INFO_SIZE);
	} else {
		static bd_t _bfin_bd;
		bd = &_bfin_bd;
	}

	gd->bd = bd;

	bd->bi_r_version = version_string;
	bd->bi_cpu = __stringify(CONFIG_BFIN_CPU);
	bd->bi_board_name = CONFIG_SYS_BOARD;
	bd->bi_vco = get_vco();
	bd->bi_cclk = get_cclk();
	bd->bi_sclk = get_sclk();
	bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	bd->bi_memsize = CONFIG_SYS_MAX_RAM_SIZE;

	gd->ram_size = CONFIG_SYS_MAX_RAM_SIZE;

	return 0;
}

static void display_global_data(void)
{
	bd_t *bd;

#ifndef CONFIG_DEBUG_EARLY_SERIAL
	return;
#endif

	bd = gd->bd;
	printf(" gd: %p\n", gd);
	printf(" |-flags: %lx\n", gd->flags);
	printf(" |-board_type: %lx\n", gd->arch.board_type);
	printf(" |-baudrate: %u\n", gd->baudrate);
	printf(" |-have_console: %lx\n", gd->have_console);
	printf(" |-ram_size: %lx\n", gd->ram_size);
	printf(" |-env_addr: %lx\n", gd->env_addr);
	printf(" |-env_valid: %lx\n", gd->env_valid);
	printf(" |-jt(%p): %p\n", gd->jt, *(gd->jt));
	printf(" \\-bd: %p\n", gd->bd);
	printf("   |-bi_boot_params: %lx\n", bd->bi_boot_params);
	printf("   |-bi_memstart: %lx\n", bd->bi_memstart);
	printf("   |-bi_memsize: %lx\n", bd->bi_memsize);
	printf("   |-bi_flashstart: %lx\n", bd->bi_flashstart);
	printf("   |-bi_flashsize: %lx\n", bd->bi_flashsize);
	printf("   \\-bi_flashoffset: %lx\n", bd->bi_flashoffset);
}

#define CPLB_PAGE_SIZE (4 * 1024 * 1024)
#define CPLB_PAGE_MASK (~(CPLB_PAGE_SIZE - 1))
#if defined(__ADSPBF60x__)
#define CPLB_EX_PAGE_SIZE (16 * 1024 * 1024)
#define CPLB_EX_PAGE_MASK (~(CPLB_EX_PAGE_SIZE - 1))
#else
#define CPLB_EX_PAGE_SIZE CPLB_PAGE_SIZE
#define CPLB_EX_PAGE_MASK CPLB_PAGE_MASK
#endif
void init_cplbtables(void)
{
	uint32_t *ICPLB_ADDR, *ICPLB_DATA;
	uint32_t *DCPLB_ADDR, *DCPLB_DATA;
	uint32_t extern_memory;
	size_t i;

	void icplb_add(uint32_t addr, uint32_t data)
	{
		bfin_write32(ICPLB_ADDR + i, addr);
		bfin_write32(ICPLB_DATA + i, data);
	}
	void dcplb_add(uint32_t addr, uint32_t data)
	{
		bfin_write32(DCPLB_ADDR + i, addr);
		bfin_write32(DCPLB_DATA + i, data);
	}

	/* populate a few common entries ... we'll let
	 * the memory map and cplb exception handler do
	 * the rest of the work.
	 */
	i = 0;
	ICPLB_ADDR = (uint32_t *)ICPLB_ADDR0;
	ICPLB_DATA = (uint32_t *)ICPLB_DATA0;
	DCPLB_ADDR = (uint32_t *)DCPLB_ADDR0;
	DCPLB_DATA = (uint32_t *)DCPLB_DATA0;

	icplb_add(0xFFA00000, L1_IMEMORY);
	dcplb_add(0xFF800000, L1_DMEMORY);
	++i;
#if defined(__ADSPBF60x__)
	icplb_add(0x0, 0x0);
	dcplb_add(CONFIG_SYS_FLASH_BASE, PAGE_SIZE_16MB | CPLB_DIRTY |
		CPLB_SUPV_WR | CPLB_USER_WR | CPLB_USER_RD | CPLB_VALID);
	++i;
#endif

	if (CONFIG_MEM_SIZE) {
		uint32_t mbase = CONFIG_SYS_MONITOR_BASE;
		uint32_t mend  = mbase + CONFIG_SYS_MONITOR_LEN - 1;
		mbase &= CPLB_PAGE_MASK;
		mend &= CPLB_PAGE_MASK;

		icplb_add(mbase, SDRAM_IKERNEL);
		dcplb_add(mbase, SDRAM_DKERNEL);
		++i;

		/*
		 * If the monitor crosses a 4 meg boundary, we'll need
		 * to lock two entries for it.  We assume it doesn't
		 * cross two 4 meg boundaries ...
		 */
		if (mbase != mend) {
			icplb_add(mend, SDRAM_IKERNEL);
			dcplb_add(mend, SDRAM_DKERNEL);
			++i;
		}
	}

#ifndef __ADSPBF60x__
	icplb_add(0x20000000, SDRAM_INON_CHBL);
	dcplb_add(0x20000000, SDRAM_EBIU);
	++i;
#endif

	/* Add entries for the rest of external RAM up to the bootrom */
	extern_memory = 0;

#ifdef CONFIG_DEBUG_NULL_PTR
	icplb_add(extern_memory,
		  (SDRAM_IKERNEL & ~PAGE_SIZE_MASK) | PAGE_SIZE_1KB);
	dcplb_add(extern_memory,
		  (SDRAM_DKERNEL & ~PAGE_SIZE_MASK) | PAGE_SIZE_1KB);
	++i;
	icplb_add(extern_memory, SDRAM_IKERNEL);
	dcplb_add(extern_memory, SDRAM_DKERNEL);
	extern_memory += CPLB_PAGE_SIZE;
	++i;
#endif

	while (i < 16 && extern_memory <
		(CONFIG_SYS_MONITOR_BASE & CPLB_EX_PAGE_MASK)) {
		icplb_add(extern_memory, SDRAM_IGENERIC);
		dcplb_add(extern_memory, SDRAM_DGENERIC);
		extern_memory += CPLB_EX_PAGE_SIZE;
		++i;
	}
	while (i < 16) {
		icplb_add(0, 0);
		dcplb_add(0, 0);
		++i;
	}
}

int print_cpuinfo(void)
{
	char buf[32];

	printf("CPU:   ADSP %s (Detected Rev: 0.%d) (%s boot)\n",
	       gd->bd->bi_cpu,
	       bfin_revid(),
	       get_bfin_boot_mode(CONFIG_BFIN_BOOT_MODE));

	printf("Clock: VCO: %s MHz, ", strmhz(buf, get_vco()));
	printf("Core: %s MHz, ", strmhz(buf, get_cclk()));
#if defined(__ADSPBF60x__)
	printf("System0: %s MHz, ", strmhz(buf, get_sclk0()));
	printf("System1: %s MHz, ", strmhz(buf, get_sclk1()));
	printf("Dclk: %s MHz\n", strmhz(buf, get_dclk()));
#else
	printf("System: %s MHz\n", strmhz(buf, get_sclk()));
#endif

	return 0;
}

int exception_init(void)
{
	bfin_write_EVT3(trap);
	return 0;
}

int irq_init(void)
{
#ifdef SIC_IMASK0
	bfin_write_SIC_IMASK0(0);
	bfin_write_SIC_IMASK1(0);
# ifdef SIC_IMASK2
	bfin_write_SIC_IMASK2(0);
# endif
#elif defined(SICA_IMASK0)
	bfin_write_SICA_IMASK0(0);
	bfin_write_SICA_IMASK1(0);
#elif defined(SIC_IMASK)
	bfin_write_SIC_IMASK(0);
#endif
	/* Set up a dummy NMI handler if needed.  */
	if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS || ANOMALY_05000219)
		bfin_write_EVT2(evt_nmi);	/* NMI */
	bfin_write_EVT5(evt_default);	/* hardware error */
	bfin_write_EVT6(evt_default);	/* core timer */
	bfin_write_EVT7(evt_default);
	bfin_write_EVT8(evt_default);
	bfin_write_EVT9(evt_default);
	bfin_write_EVT10(evt_default);
	bfin_write_EVT11(evt_default);
	bfin_write_EVT12(evt_default);
	bfin_write_EVT13(evt_default);
	bfin_write_EVT14(evt_default);
	bfin_write_EVT15(evt_default);
	bfin_write_ILAT(0);
	CSYNC();
	/* enable hardware error irq */
	irq_flags = 0x3f;
	local_irq_enable();
	return 0;
}

__attribute__ ((__noreturn__))
void cpu_init_f(ulong bootflag, ulong loaded_from_ldr)
{
#ifndef CONFIG_BFIN_BOOTROM_USES_EVT1
	/* Build a NOP slide over the LDR jump block.  Whee! */
	char nops[0xC];
	serial_early_puts("NOP Slide\n");
	memset(nops, 0x00, sizeof(nops));
	memcpy((void *)L1_INST_SRAM, nops, sizeof(nops));
#endif

	if (!loaded_from_ldr) {
		/* Relocate sections into L1 if the LDR didn't do it -- don't
		 * check length because the linker script does the size
		 * checking at build time.
		 */
		serial_early_puts("L1 Relocate\n");
		extern char _stext_l1[], _text_l1_lma[], _text_l1_len[];
		memcpy(&_stext_l1, &_text_l1_lma, (unsigned long)_text_l1_len);
		extern char _sdata_l1[], _data_l1_lma[], _data_l1_len[];
		memcpy(&_sdata_l1, &_data_l1_lma, (unsigned long)_data_l1_len);
	}

	/*
	 * Make sure our async settings are committed.  Some bootroms
	 * (like the BF537) will reset some registers on us after it
	 * has finished loading the LDR.  Or if we're booting over
	 * JTAG, the initcode never got a chance to run.  Or if we
	 * aren't booting from parallel flash, the initcode skipped
	 * this step completely.
	 */
	program_async_controller(NULL);

	/* Save RETX so we can pass it while booting Linux */
	bfin_poweron_retx = bootflag;

#ifdef CONFIG_DEBUG_DUMP
	/* Turn on hardware trace buffer */
	bfin_write_TBUFCTL(TBUFPWR | TBUFEN);
#endif

#ifndef CONFIG_PANIC_HANG
	/* Reset upon a double exception rather than just hanging.
	 * Do not do bfin_read on SWRST as that will reset status bits.
	 */
# ifdef SWRST
	bfin_write_SWRST(DOUBLE_FAULT);
# endif
#endif

#if defined(CONFIG_CORE1_RUN) && defined(COREB_L1_CODE_START)
	bfin_core1_start();
#endif

	serial_early_puts("Init global data\n");
	global_board_data_init();

	board_init_f(0);

	/* should not be reached */
	while (1);
}

int arch_cpu_init(void)
{
	serial_early_puts("Init CPLB tables\n");
	init_cplbtables();

	serial_early_puts("Exceptions setup\n");
	exception_init();

#ifndef CONFIG_ICACHE_OFF
	serial_early_puts("Turn on ICACHE\n");
	icache_enable();
#endif
#ifndef CONFIG_DCACHE_OFF
	serial_early_puts("Turn on DCACHE\n");
	dcache_enable();
#endif

#ifdef DEBUG
	if (GENERATED_GBL_DATA_SIZE < sizeof(*gd))
		hang();
#endif

	/* Initialize */
	serial_early_puts("IRQ init\n");
	irq_init();

	return 0;
}

int arch_misc_init(void)
{
#if defined(CONFIG_SYS_I2C)
	i2c_reloc_fixup();
#endif

	display_global_data();

	if (CONFIG_MEM_SIZE && bfin_os_log_check()) {
		puts("\nLog buffer from operating system:\n");
		bfin_os_log_dump();
		puts("\n");
	}

	return 0;
}

int interrupt_init(void)
{
	return 0;
}
