/*
 * (C) Copyright 2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#ifdef CONFIG_LXT971_NO_SLEEP
#include <miiphy.h>
#endif


#ifdef CONFIG_LXT971_NO_SLEEP
void lxt971_no_sleep(void)
{
	unsigned short reg;

	miiphy_read("ppc_4xx_eth0", CONFIG_PHY_ADDR, 0x10, &reg);
	reg &= ~0x0040;                  /* disable sleep mode */
	miiphy_write("ppc_4xx_eth0", CONFIG_PHY_ADDR, 0x10, reg);
}
#endif /* CONFIG_LXT971_NO_SLEEP */
