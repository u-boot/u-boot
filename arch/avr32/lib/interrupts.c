/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/sysreg.h>

void enable_interrupts(void)
{
	asm volatile("csrf	%0" : : "n"(SYSREG_GM_OFFSET));
}

int disable_interrupts(void)
{
	unsigned long sr;

	sr = sysreg_read(SR);
	asm volatile("ssrf	%0" : : "n"(SYSREG_GM_OFFSET));

#ifdef CONFIG_AT32UC3A0xxx
	/* Two NOPs are required after masking interrupts on the
	 * AT32UC3A0512ES. See errata 41.4.5.5. */
	asm("nop");
	asm("nop");
#endif

	return !SYSREG_BFEXT(GM, sr);
}
