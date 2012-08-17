/*
 * reset.c - logic for resetting the cpu
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/bootrom.h>
#include "cpu.h"

/* A system soft reset makes external memory unusable so force
 * this function into L1.  We use the compiler ssync here rather
 * than SSYNC() because it's safe (no interrupts and such) and
 * we save some L1.  We do not need to force sanity in the SYSCR
 * register as the BMODE selection bit is cleared by the soft
 * reset while the Core B bit (on dual core parts) is cleared by
 * the core reset.
 */
__attribute__ ((__l1_text__, __noreturn__))
static void bfin_reset(void)
{
	/* Wait for completion of "system" events such as cache line
	 * line fills so that we avoid infinite stalls later on as
	 * much as possible.  This code is in L1, so it won't trigger
	 * any such event after this point in time.
	 */
	__builtin_bfin_ssync();

	/* Initiate System software reset. */
	bfin_write_SWRST(0x7);

	/* Due to the way reset is handled in the hardware, we need
	 * to delay for 10 SCLKS.  The only reliable way to do this is
	 * to calculate the CCLK/SCLK ratio and multiply 10.  For now,
	 * we'll assume worse case which is a 1:15 ratio.
	 */
	asm(
		"LSETUP (1f, 1f) LC0 = %0\n"
		"1: nop;"
		:
		: "a" (15 * 10)
		: "LC0", "LB0", "LT0"
	);

	/* Clear System software reset */
	bfin_write_SWRST(0);

	/* The BF526 ROM will crash during reset */
#if defined(__ADSPBF522__) || defined(__ADSPBF524__) || defined(__ADSPBF526__)
	/* Seems to be fixed with newer parts though ... */
	if (__SILICON_REVISION__ < 1 && bfin_revid() < 1)
		bfin_read_SWRST();
#endif

	/* Wait for the SWRST write to complete.  Cannot rely on SSYNC
	 * though as the System state is all reset now.
	 */
	asm(
		"LSETUP (1f, 1f) LC1 = %0\n"
		"1: nop;"
		:
		: "a" (15 * 1)
		: "LC1", "LB1", "LT1"
	);

	while (1)
		/* Issue core reset */
		asm("raise 1");
}

/* We need to trampoline ourselves up into L1 since our linker
 * does not have relaxtion support and will only generate a
 * PC relative call with a 25 bit immediate.  This is not enough
 * to get us from the top of SDRAM into L1.
 */
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (board_reset)
		board_reset();
	if (ANOMALY_05000353 || ANOMALY_05000386)
		while (1)
			asm("jump (%0);" : : "a" (bfin_reset));
	else
		bfrom_SoftReset((void *)(L1_SRAM_SCRATCH_END - 20));
	return 0;
}
