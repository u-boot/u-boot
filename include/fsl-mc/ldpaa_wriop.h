/*
 * Copyright (C) 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LDPAA_WRIOP_H
#define __LDPAA_WRIOP_H

 #include <phy.h>

enum wriop_port {
	WRIOP1_DPMAC1 = 1,
	WRIOP1_DPMAC2,
	WRIOP1_DPMAC3,
	WRIOP1_DPMAC4,
	WRIOP1_DPMAC5,
	WRIOP1_DPMAC6,
	WRIOP1_DPMAC7,
	WRIOP1_DPMAC8,
	WRIOP1_DPMAC9,
	WRIOP1_DPMAC10,
	WRIOP1_DPMAC11,
	WRIOP1_DPMAC12,
	WRIOP1_DPMAC13,
	WRIOP1_DPMAC14,
	WRIOP1_DPMAC15,
	WRIOP1_DPMAC16,
	WRIOP1_DPMAC17,
	WRIOP1_DPMAC18,
	WRIOP1_DPMAC19,
	WRIOP1_DPMAC20,
	WRIOP1_DPMAC21,
	WRIOP1_DPMAC22,
	WRIOP1_DPMAC23,
	WRIOP1_DPMAC24,
	NUM_WRIOP_PORTS,
};

struct wriop_dpmac_info {
	u8 enabled;
	u8 id;
	u8 phy_addr;
	u8 board_mux;
	void *phy_regs;
	phy_interface_t enet_if;
	struct phy_device *phydev;
	struct mii_dev *bus;
};

extern struct wriop_dpmac_info dpmac_info[NUM_WRIOP_PORTS];

#define DEFAULT_WRIOP_MDIO1_NAME "FSL_MDIO0"
#define DEFAULT_WRIOP_MDIO2_NAME "FSL_MDIO1"

void wriop_init_dpmac(int, int, int);
void wriop_disable_dpmac(int);
void wriop_enable_dpmac(int);
void wriop_set_mdio(int, struct mii_dev *);
struct mii_dev *wriop_get_mdio(int);
void wriop_set_phy_address(int, int);
int wriop_get_phy_address(int);
void wriop_set_phy_dev(int, struct phy_device *);
struct phy_device *wriop_get_phy_dev(int);
phy_interface_t wriop_get_enet_if(int);

void wriop_dpmac_disable(int);
void wriop_dpmac_enable(int);
phy_interface_t wriop_dpmac_enet_if(int, int);
#endif	/* __LDPAA_WRIOP_H */
