/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
