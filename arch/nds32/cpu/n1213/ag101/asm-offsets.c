/*
 * Adapted from Linux v2.6.36 kernel: arch/powerpc/kernel/asm-offsets.c
 *
 * Generate definitions needed by assembly language modules.
 * This code generates raw asm output which is post-processed to extract
 * and format the required data.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>

#include <linux/kbuild.h>

int main(void)
{
#ifdef CONFIG_FTSMC020
	OFFSET(FTSMC020_BANK0_CR,	ftsmc020, bank[0].cr);
	OFFSET(FTSMC020_BANK0_TPR,	ftsmc020, bank[0].tpr);
#endif
	BLANK();
#ifdef CONFIG_FTAHBC020S
	OFFSET(FTAHBC020S_SLAVE_BSR_6,	ftahbc02s, s_bsr[6]);
	OFFSET(FTAHBC020S_CR,		ftahbc02s, cr);
#endif
	BLANK();
#ifdef CONFIG_FTPMU010
	OFFSET(FTPMU010_PDLLCR0,	ftpmu010, PDLLCR0);
#endif
	BLANK();
#ifdef CONFIG_FTSDMC021
	OFFSET(FTSDMC021_TP1,		ftsdmc021, tp1);
	OFFSET(FTSDMC021_TP2,		ftsdmc021, tp2);
	OFFSET(FTSDMC021_CR1,		ftsdmc021, cr1);
	OFFSET(FTSDMC021_CR2,		ftsdmc021, cr2);
	OFFSET(FTSDMC021_BANK0_BSR,	ftsdmc021, bank0_bsr);
	OFFSET(FTSDMC021_BANK1_BSR,	ftsdmc021, bank1_bsr);
	OFFSET(FTSDMC021_BANK2_BSR,	ftsdmc021, bank2_bsr);
	OFFSET(FTSDMC021_BANK3_BSR,	ftsdmc021, bank3_bsr);
#endif
	return 0;
}
