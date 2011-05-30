/*
 * Code for early processor initialization
 *
 * Copyright (c) 2004-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BFIN_INITCODE_H__
#define __BFIN_INITCODE_H__

#include <asm/mach-common/bits/bootrom.h>

#ifndef BFIN_IN_INITCODE
# define serial_putc(c)
#endif

#ifndef CONFIG_EBIU_RSTCTL_VAL
# define CONFIG_EBIU_RSTCTL_VAL 0 /* only MDDRENABLE is useful */
#endif
#if ((CONFIG_EBIU_RSTCTL_VAL & 0xFFFFFFC4) != 0)
# error invalid EBIU_RSTCTL value: must not set reserved bits
#endif

#ifndef CONFIG_EBIU_MBSCTL_VAL
# define CONFIG_EBIU_MBSCTL_VAL 0
#endif

#if defined(CONFIG_EBIU_DDRQUE_VAL) && ((CONFIG_EBIU_DDRQUE_VAL & 0xFFFF8000) != 0)
# error invalid EBIU_DDRQUE value: must not set reserved bits
#endif

__attribute__((always_inline)) static inline void
program_async_controller(ADI_BOOT_DATA *bs)
{
#ifdef BFIN_IN_INITCODE
	/*
	 * We really only need to setup the async banks early if we're
	 * booting out of it.  Otherwise, do it later on in cpu_init.
	 */
	if (CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_BYPASS &&
	    CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_PARA)
		return;
#endif

	serial_putc('a');

	/* Program the async banks controller. */
	bfin_write_EBIU_AMBCTL0(CONFIG_EBIU_AMBCTL0_VAL);
	bfin_write_EBIU_AMBCTL1(CONFIG_EBIU_AMBCTL1_VAL);
	bfin_write_EBIU_AMGCTL(CONFIG_EBIU_AMGCTL_VAL);

	serial_putc('b');

	/* Not all parts have these additional MMRs. */
#ifdef EBIU_MBSCTL
	bfin_write_EBIU_MBSCTL(CONFIG_EBIU_MBSCTL_VAL);
#endif
#ifdef EBIU_MODE
# ifdef CONFIG_EBIU_MODE_VAL
	bfin_write_EBIU_MODE(CONFIG_EBIU_MODE_VAL);
# endif
# ifdef CONFIG_EBIU_FCTL_VAL
	bfin_write_EBIU_FCTL(CONFIG_EBIU_FCTL_VAL);
# endif
#endif

	serial_putc('c');
}

#endif
