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
#include <asm/blackfin.h>
#include <asm/cplb.h>
#include <asm/mach-common/bits/core.h>
#include <asm/mach-common/bits/ebiu.h>
#include <asm/mach-common/bits/trace.h>
#include <asm/serial.h>

#include "cpu.h"
#include "initcode.h"

ulong bfin_poweron_retx;

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

	serial_early_puts("Board init flash\n");
	board_init_f(bootflag);

	/* should not be reached */
	while (1);
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
