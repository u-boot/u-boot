/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <asm/io.h>
#include <asm/arch/reset_manager.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_reset_manager *reset_manager_base =
		(void *)SOCFPGA_RSTMGR_ADDRESS;

/* Toggle reset signal to watchdog (WDT is disabled after this operation!) */
void socfpga_watchdog_reset(void)
{
	/* assert reset for watchdog */
	setbits_le32(&reset_manager_base->per_mod_reset,
		     1 << RSTMGR_PERMODRST_L4WD0_LSB);

	/* deassert watchdog from reset (watchdog in not running state) */
	clrbits_le32(&reset_manager_base->per_mod_reset,
		     1 << RSTMGR_PERMODRST_L4WD0_LSB);
}

/*
 * Write the reset manager register to cause reset
 */
void reset_cpu(ulong addr)
{
	/* request a warm reset */
	writel((1 << RSTMGR_CTRL_SWWARMRSTREQ_LSB),
		&reset_manager_base->ctrl);
	/*
	 * infinite loop here as watchdog will trigger and reset
	 * the processor
	 */
	while (1)
		;
}

/*
 * Release peripherals from reset based on handoff
 */
void reset_deassert_peripherals_handoff(void)
{
	writel(0, &reset_manager_base->per_mod_reset);
}
