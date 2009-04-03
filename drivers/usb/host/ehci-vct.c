/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <usb.h>

#include "ehci.h"
#include "ehci-core.h"

int vct_ehci_hcd_init(u32 *hccr, u32 *hcor);

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(void)
{
	int ret;
	u32 vct_hccr;
	u32 vct_hcor;

	/*
	 * Init VCT specific stuff
	 */
	ret = vct_ehci_hcd_init(&vct_hccr, &vct_hcor);
	if (ret)
		return ret;

	hccr = (struct ehci_hccr *)vct_hccr;
	hcor = (struct ehci_hcor *)vct_hcor;

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	return 0;
}
