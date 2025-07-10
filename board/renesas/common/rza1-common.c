// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Renesas Electronics
 * Copyright (C) Chris Brandt
 */

#include <init.h>
#include <asm/io.h>

#define RZA1_WDT_BASE	0xfcfe0000
#define WTCSR		0x00
#define WTCNT		0x02
#define WRCSR		0x04

void __weak reset_cpu(void)
{
	/* Dummy read (must read WRCSR:WOVF at least once before clearing) */
	readb(RZA1_WDT_BASE + WRCSR);

	writew(0xa500, RZA1_WDT_BASE + WRCSR);
	writew(0x5a5f, RZA1_WDT_BASE + WRCSR);
	writew(0x5a00, RZA1_WDT_BASE + WTCNT);
	writew(0xa578, RZA1_WDT_BASE + WTCSR);

	for (;;)
		asm volatile("wfi");
}
