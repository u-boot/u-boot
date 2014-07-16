/*
 * Keystone: PSC configuration module
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm-generic/errno.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/psc_defs.h>

#define DEVICE_REG32_R(addr)			__raw_readl((u32 *)(addr))
#define DEVICE_REG32_W(addr, val)		__raw_writel(val, (u32 *)(addr))

#ifdef CONFIG_SOC_K2HK
#define DEVICE_PSC_BASE				K2HK_PSC_BASE
#endif

int psc_delay(void)
{
	udelay(10);
	return 10;
}

/*
 * FUNCTION PURPOSE: Wait for end of transitional state
 *
 * DESCRIPTION: Polls pstat for the selected domain and waits for transitions
 *              to be complete.
 *
 *              Since this is boot loader code it is *ASSUMED* that interrupts
 *              are disabled and no other core is mucking around with the psc
 *              at the same time.
 *
 *              Returns 0 when the domain is free. Returns -1 if a timeout
 *              occurred waiting for the completion.
 */
int psc_wait(u32 domain_num)
{
	u32 retry;
	u32 ptstat;

	/*
	 * Do nothing if the power domain is in transition. This should never
	 * happen since the boot code is the only software accesses psc.
	 * It's still remotely possible that the hardware state machines
	 * initiate transitions.
	 * Don't trap if the domain (or a module in this domain) is
	 * stuck in transition.
	 */
	retry = 0;

	do {
		ptstat = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_PSTAT);
		ptstat = ptstat & (1 << domain_num);
	} while ((ptstat != 0) && ((retry += psc_delay()) <
		 PSC_PTSTAT_TIMEOUT_LIMIT));

	if (retry >= PSC_PTSTAT_TIMEOUT_LIMIT)
		return -1;

	return 0;
}

u32 psc_get_domain_num(u32 mod_num)
{
	u32 domain_num;

	/* Get the power domain associated with the module number */
	domain_num = DEVICE_REG32_R(DEVICE_PSC_BASE +
				    PSC_REG_MDCFG(mod_num));
	domain_num = PSC_REG_MDCFG_GET_PD(domain_num);

	return domain_num;
}

/*
 * FUNCTION PURPOSE: Power up/down a module
 *
 * DESCRIPTION: Powers up/down the requested module and the associated power
 *		domain if required. No action is taken it the module is
 *		already powered up/down.
 *
 *              This only controls modules. The domain in which the module
 *              resides will be left in the power on state. Multiple modules
 *              can exist in a power domain, so powering down the domain based
 *              on a single module is not done.
 *
 *              Returns 0 on success, -1 if the module can't be powered up, or
 *              if there is a timeout waiting for the transition.
 */
int psc_set_state(u32 mod_num, u32 state)
{
	u32 domain_num;
	u32 pdctl;
	u32 mdctl;
	u32 ptcmd;
	u32 reset_iso;
	u32 v;

	/*
	 * Get the power domain associated with the module number, and reset
	 * isolation functionality
	 */
	v = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_MDCFG(mod_num));
	domain_num = PSC_REG_MDCFG_GET_PD(v);
	reset_iso  = PSC_REG_MDCFG_GET_RESET_ISO(v);

	/* Wait for the status of the domain/module to be non-transitional */
	if (psc_wait(domain_num) != 0)
		return -1;

	/*
	 * Perform configuration even if the current status matches the
	 * existing state
	 *
	 * Set the next state of the power domain to on. It's OK if the domain
	 * is always on. This code will not ever power down a domain, so no
	 * change is made if the new state is power down.
	 */
	if (state == PSC_REG_VAL_MDCTL_NEXT_ON) {
		pdctl = DEVICE_REG32_R(DEVICE_PSC_BASE +
				       PSC_REG_PDCTL(domain_num));
		pdctl = PSC_REG_PDCTL_SET_NEXT(pdctl,
					       PSC_REG_VAL_PDCTL_NEXT_ON);
		DEVICE_REG32_W(DEVICE_PSC_BASE + PSC_REG_PDCTL(domain_num),
			       pdctl);
	}

	/* Set the next state for the module to enabled/disabled */
	mdctl = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_MDCTL(mod_num));
	mdctl = PSC_REG_MDCTL_SET_NEXT(mdctl, state);
	mdctl = PSC_REG_MDCTL_SET_RESET_ISO(mdctl, reset_iso);
	DEVICE_REG32_W(DEVICE_PSC_BASE + PSC_REG_MDCTL(mod_num), mdctl);

	/* Trigger the enable */
	ptcmd = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_PTCMD);
	ptcmd |= (u32)(1<<domain_num);
	DEVICE_REG32_W(DEVICE_PSC_BASE + PSC_REG_PTCMD, ptcmd);

	/* Wait on the complete */
	return psc_wait(domain_num);
}

/*
 * FUNCTION PURPOSE: Power up a module
 *
 * DESCRIPTION: Powers up the requested module and the associated power domain
 *              if required. No action is taken it the module is already
 *              powered up.
 *
 *              Returns 0 on success, -1 if the module can't be powered up, or
 *              if there is a timeout waiting for the transition.
 */
int psc_enable_module(u32 mod_num)
{
	u32 mdctl;

	/* Set the bit to apply reset */
	mdctl = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_MDCTL(mod_num));
	if ((mdctl & 0x3f) == PSC_REG_VAL_MDSTAT_STATE_ON)
		return 0;

	return psc_set_state(mod_num, PSC_REG_VAL_MDCTL_NEXT_ON);
}

/*
 * FUNCTION PURPOSE: Power down a module
 *
 * DESCRIPTION: Powers down the requested module.
 *
 *              Returns 0 on success, -1 on failure or timeout.
 */
int psc_disable_module(u32 mod_num)
{
	u32 mdctl;

	/* Set the bit to apply reset */
	mdctl = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_MDCTL(mod_num));
	if ((mdctl & 0x3f) == 0)
		return 0;
	mdctl = PSC_REG_MDCTL_SET_LRSTZ(mdctl, 0);
	DEVICE_REG32_W(DEVICE_PSC_BASE + PSC_REG_MDCTL(mod_num), mdctl);

	return psc_set_state(mod_num, PSC_REG_VAL_MDCTL_NEXT_SWRSTDISABLE);
}

/*
 * FUNCTION PURPOSE: Set the reset isolation bit in mdctl
 *
 * DESCRIPTION: The reset isolation enable bit is set. The state of the module
 *              is not changed. Returns 0 if the module config showed that
 *              reset isolation is supported. Returns 1 otherwise. This is not
 *              an error, but setting the bit in mdctl has no effect.
 */
int psc_set_reset_iso(u32 mod_num)
{
	u32 v;
	u32 mdctl;

	/* Set the reset isolation bit */
	mdctl = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_MDCTL(mod_num));
	mdctl = PSC_REG_MDCTL_SET_RESET_ISO(mdctl, 1);
	DEVICE_REG32_W(DEVICE_PSC_BASE + PSC_REG_MDCTL(mod_num), mdctl);

	v = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_MDCFG(mod_num));
	if (PSC_REG_MDCFG_GET_RESET_ISO(v) == 1)
		return 0;

	return 1;
}

/*
 * FUNCTION PURPOSE: Disable a power domain
 *
 * DESCRIPTION: The power domain is disabled
 */
int psc_disable_domain(u32 domain_num)
{
	u32 pdctl;
	u32 ptcmd;

	pdctl = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_PDCTL(domain_num));
	pdctl = PSC_REG_PDCTL_SET_NEXT(pdctl, PSC_REG_VAL_PDCTL_NEXT_OFF);
	pdctl = PSC_REG_PDCTL_SET_PDMODE(pdctl, PSC_REG_VAL_PDCTL_PDMODE_SLEEP);
	DEVICE_REG32_W(DEVICE_PSC_BASE + PSC_REG_PDCTL(domain_num), pdctl);

	ptcmd = DEVICE_REG32_R(DEVICE_PSC_BASE + PSC_REG_PTCMD);
	ptcmd |= (u32)(1 << domain_num);
	DEVICE_REG32_W(DEVICE_PSC_BASE + PSC_REG_PTCMD, ptcmd);

	return psc_wait(domain_num);
}
