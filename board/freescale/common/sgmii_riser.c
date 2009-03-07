/*
 * Freescale SGMII Riser Card
 *
 * This driver supports the SGMII Riser card found on the
 * "DS" style of development board from Freescale.
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <libfdt.h>
#include <tsec.h>

void fsl_sgmii_riser_init(struct tsec_info_struct *tsec_info, int num)
{
	int i;

	for (i = 0; i < num; i++)
		if (tsec_info[i].flags & TSEC_SGMII)
			tsec_info[i].phyaddr += SGMII_RISER_PHY_OFFSET;
}

void fsl_sgmii_riser_fdt_fixup(void *fdt)
{
	struct eth_device *dev;
	int node;
	int i = -1;
	int etsec_num = 0;

	node = fdt_path_offset(fdt, "/aliases");
	if (node < 0)
		return;

	while ((dev = eth_get_dev_by_index(++i)) != NULL) {
		struct tsec_private *priv;
		int enet_node;
		char enet[16];
		const u32 *phyh;
		int phynode;
		const char *model;
		const char *path;

		if (!strstr(dev->name, "eTSEC"))
			continue;

		sprintf(enet, "ethernet%d", etsec_num++);
		path = fdt_getprop(fdt, node, enet, NULL);
		if (!path) {
			debug("No alias for %s\n", enet);
			continue;
		}

		enet_node = fdt_path_offset(fdt, path);
		if (enet_node < 0)
			continue;

		model = fdt_getprop(fdt, enet_node, "model", NULL);

		/*
		 * We only want to do this to eTSECs.  On some platforms
		 * there are more than one type of gianfar-style ethernet
		 * controller, and as we are creating an implicit connection
		 * between ethernet nodes and eTSEC devices, it is best to
		 * make the connection use as much explicit information
		 * as exists.
		 */
		if (!strstr(model, "TSEC"))
			continue;

		phyh = fdt_getprop(fdt, enet_node, "phy-handle", NULL);
		if (!phyh)
			continue;

		phynode = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*phyh));

		priv = dev->priv;

		if (priv->flags & TSEC_SGMII)
			fdt_setprop_cell(fdt, phynode, "reg", priv->phyaddr);
	}
}
