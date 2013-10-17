/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#ifdef CONFIG_OMAP
#include <asm/omap_boot.h>
#endif

/* Architecture-specific global data */
struct arch_global_data {
#if defined(CONFIG_FSL_ESDHC)
	u32 sdhc_clk;
#endif
#ifdef CONFIG_AT91FAMILY
	/* "static data" needed by at91's clock.c */
	unsigned long	cpu_clk_rate_hz;
	unsigned long	main_clk_rate_hz;
	unsigned long	mck_rate_hz;
	unsigned long	plla_rate_hz;
	unsigned long	pllb_rate_hz;
	unsigned long	at91_pllb_usb_init;
#endif
	/* "static data" needed by most of timer.c on ARM platforms */
	unsigned long timer_rate_hz;
	unsigned long tbu;
	unsigned long tbl;
	unsigned long lastinc;
	unsigned long long timer_reset_value;
#ifdef CONFIG_IXP425
	unsigned long timestamp;
#endif
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	unsigned long tlb_addr;
	unsigned long tlb_size;
#endif

#ifdef CONFIG_OMAP
	struct omap_boot_parameters omap_boot_params;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("r9")

#endif /* __ASM_GBL_DATA_H */
