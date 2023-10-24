// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

/* CPU specific code */
#include <cpu_func.h>
#include <irq_func.h>
#include <asm/cache.h>
#include <asm/csr.h>
#include <asm/arch-andes/csr.h>

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	cache_flush();

	return 0;
}

void harts_early_init(void)
{
	/* Enable I/D-cache in SPL */
	if (CONFIG_IS_ENABLED(RISCV_MMODE)) {
		unsigned long mcache_ctl_val = csr_read(CSR_MCACHE_CTL);

		mcache_ctl_val |= (MCACHE_CTL_DC_COHEN | MCACHE_CTL_IC_EN |
				   MCACHE_CTL_DC_EN | MCACHE_CTL_CCTL_SUEN);

		csr_write(CSR_MCACHE_CTL, mcache_ctl_val);

		/*
		 * Check mcache_ctl.DC_COHEN, we assume this platform does
		 * not support CM if the bit is hard-wired to 0.
		 */
		if (csr_read(CSR_MCACHE_CTL) & MCACHE_CTL_DC_COHEN) {
			/* Wait for DC_COHSTA bit to be set */
			while (!(csr_read(CSR_MCACHE_CTL) & MCACHE_CTL_DC_COHSTA));
		}
	}
}
