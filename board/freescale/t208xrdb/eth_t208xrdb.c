// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 *
 * Shengzhou Liu <Shengzhou.Liu@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <fdt_support.h>
#include <net.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_dtsec.h>
#include <asm/fsl_serdes.h>

/* Disable the MAC5 and MAC6 "fsl,fman-memac" nodes and the two
 * "fsl,dpa-ethernet" nodes that reference them.
 */
void fdt_fixup_board_fman_ethernet(void *fdt)
{
	int mac_off, eth_off, i;
	char mac_path[2][42] = {
		"/soc@ffe000000/fman@400000/ethernet@e8000",
		"/soc@ffe000000/fman@400000/ethernet@ea000",
	};
	u32 eth_ph;

	for (i = 0; i < 2; i++) {
		/* Disable the MAC node */
		mac_off = fdt_path_offset(fdt, mac_path[i]);
		if (mac_off < 0)
			continue;
		fdt_status_disabled(fdt, mac_off);

		/* Disable the fsl,dpa-ethernet node that points to the MAC.
		 * The fsl,fman-mac property refers to the MAC's phandle.
		 */
		eth_ph = fdt_get_phandle(fdt, mac_off);
		if (eth_ph <= 0)
			continue;

		eth_off = fdt_node_offset_by_prop_value(fdt, -1, "fsl,fman-mac",
							&eth_ph,
							sizeof(eth_ph));
		if (eth_off >= 0)
			fdt_status_disabled(fdt, eth_off);
	}
}

void fdt_fixup_board_enet(void *fdt)
{
	return;
}
