// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Freescale Semiconductor
 */

#include <common.h>
#include <asm/io.h>
#include <asm/types.h>
#include <malloc.h>
#include <net.h>
#include <linux/compat.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/ldpaa_wriop.h>

struct wriop_dpmac_info dpmac_info[NUM_WRIOP_PORTS];

__weak phy_interface_t wriop_dpmac_enet_if(int dpmac_id, int lane_prtc)
{
	return PHY_INTERFACE_MODE_NONE;
}

void wriop_init_dpmac(int sd, int dpmac_id, int lane_prtcl)
{
	phy_interface_t enet_if;

	dpmac_info[dpmac_id].enabled = 0;
	dpmac_info[dpmac_id].id = 0;
	dpmac_info[dpmac_id].phy_addr = -1;
	dpmac_info[dpmac_id].enet_if = PHY_INTERFACE_MODE_NONE;

	enet_if = wriop_dpmac_enet_if(dpmac_id, lane_prtcl);
	if (enet_if != PHY_INTERFACE_MODE_NONE) {
		dpmac_info[dpmac_id].enabled = 1;
		dpmac_info[dpmac_id].id = dpmac_id;
		dpmac_info[dpmac_id].enet_if = enet_if;
	}
}

void wriop_init_dpmac_enet_if(int dpmac_id, phy_interface_t enet_if)
{
	dpmac_info[dpmac_id].enabled = 1;
	dpmac_info[dpmac_id].id = dpmac_id;
	dpmac_info[dpmac_id].phy_addr = -1;
	dpmac_info[dpmac_id].enet_if = enet_if;
}


/*TODO what it do */
static int wriop_dpmac_to_index(int dpmac_id)
{
	int i;

	for (i = WRIOP1_DPMAC1; i < NUM_WRIOP_PORTS; i++) {
		if (dpmac_info[i].id == dpmac_id)
			return i;
	}

	return -1;
}

void wriop_disable_dpmac(int dpmac_id)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return;

	dpmac_info[i].enabled = 0;
	wriop_dpmac_disable(dpmac_id);
}

void wriop_enable_dpmac(int dpmac_id)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return;

	dpmac_info[i].enabled = 1;
	wriop_dpmac_enable(dpmac_id);
}

u8 wriop_is_enabled_dpmac(int dpmac_id)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return -1;

	return dpmac_info[i].enabled;
}


void wriop_set_mdio(int dpmac_id, struct mii_dev *bus)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return;

	dpmac_info[i].bus = bus;
}

struct mii_dev *wriop_get_mdio(int dpmac_id)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return NULL;

	return dpmac_info[i].bus;
}

void wriop_set_phy_address(int dpmac_id, int address)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return;

	dpmac_info[i].phy_addr = address;
}

int wriop_get_phy_address(int dpmac_id)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return -1;

	return dpmac_info[i].phy_addr;
}

void wriop_set_phy_dev(int dpmac_id, struct phy_device *phydev)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return;

	dpmac_info[i].phydev = phydev;
}

struct phy_device *wriop_get_phy_dev(int dpmac_id)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return NULL;

	return dpmac_info[i].phydev;
}

phy_interface_t wriop_get_enet_if(int dpmac_id)
{
	int i = wriop_dpmac_to_index(dpmac_id);

	if (i == -1)
		return PHY_INTERFACE_MODE_NONE;

	if (dpmac_info[i].enabled)
		return dpmac_info[i].enet_if;

	return PHY_INTERFACE_MODE_NONE;
}
