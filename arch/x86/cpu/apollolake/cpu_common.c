// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/cpu_common.h>
#include <asm/msr.h>
#include <asm/arch/cpu.h>
#include <asm/arch/iomap.h>
#include <power/acpi_pmc.h>

void cpu_flush_l1d_to_l2(void)
{
	struct msr_t msr;

	msr = msr_read(MSR_POWER_MISC);
	msr.lo |= FLUSH_DL1_L2;
	msr_write(MSR_POWER_MISC, msr);
}

void enable_pm_timer_emulation(const struct udevice *pmc)
{
	struct acpi_pmc_upriv *upriv = dev_get_uclass_priv(pmc);
	msr_t msr;

	/*
	 * The derived frequency is calculated as follows:
	 *    (CTC_FREQ * msr[63:32]) >> 32 = target frequency.
	 *
	 * Back-solve the multiplier so the 3.579545MHz ACPI timer frequency is
	 * used.
	 */
	msr.hi = (3579545ULL << 32) / CTC_FREQ;

	/* Set PM1 timer IO port and enable */
	msr.lo = EMULATE_PM_TMR_EN | (upriv->acpi_base + R_ACPI_PM1_TMR);
	debug("PM timer %x %x\n", msr.hi, msr.lo);
	msr_write(MSR_EMULATE_PM_TIMER, msr);
}
