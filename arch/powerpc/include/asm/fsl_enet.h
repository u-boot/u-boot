/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __ASM_PPC_FSL_ENET_H
#define __ASM_PPC_FSL_ENET_H

#include <phy.h>

struct tsec_mii_mng {
	u32 miimcfg;		/* MII management configuration reg */
	u32 miimcom;		/* MII management command reg */
	u32 miimadd;		/* MII management address reg */
	u32 miimcon;		/* MII management control reg */
	u32 miimstat;		/* MII management status reg  */
	u32 miimind;		/* MII management indication reg */
	u32 ifstat;		/* Interface Status Register */
} __attribute__ ((packed));

int fdt_fixup_phy_connection(void *blob, int offset, phy_interface_t phyc);

#endif /* __ASM_PPC_FSL_ENET_H */
