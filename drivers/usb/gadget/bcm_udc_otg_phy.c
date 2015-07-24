/*
 * Copyright 2015 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sysmap.h>

#include <usb/s3c_udc.h>
#include "bcm_udc_otg.h"

void otg_phy_init(struct s3c_udc *dev)
{
	/* set Phy to driving mode */
	wfld_clear(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		   HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK);

	udelay(100);

	/* clear Soft Disconnect */
	wfld_clear(HSOTG_BASE_ADDR + HSOTG_DCTL_OFFSET,
		   HSOTG_DCTL_SFTDISCON_MASK);

	/* invoke Reset (active low) */
	wfld_clear(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		   HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK);

	/* Reset needs to be asserted for 2ms */
	udelay(2000);

	/* release Reset */
	wfld_set(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		 HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK,
		 HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK);
}

void otg_phy_off(struct s3c_udc *dev)
{
	/* Soft Disconnect */
	wfld_set(HSOTG_BASE_ADDR + HSOTG_DCTL_OFFSET,
		 HSOTG_DCTL_SFTDISCON_MASK,
		 HSOTG_DCTL_SFTDISCON_MASK);

	/* set Phy to non-driving (reset) mode */
	wfld_set(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		 HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK,
		 HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK);
}
