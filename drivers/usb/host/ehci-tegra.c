/*
 * Copyright (c) 2009 NVIDIA Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <asm/errno.h>
#include <asm/arch/usb.h>


/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(void)
{
	u32 our_hccr, our_hcor;

	/*
	 * Select the first port, as we don't have a way of selecting others
	 * yet
	 */
	if (tegrausb_start_port(0, &our_hccr, &our_hcor))
		return -1;

	hccr = (struct ehci_hccr *)our_hccr;
	hcor = (struct ehci_hcor *)our_hcor;

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	tegrausb_stop_port();
	return 0;
}
