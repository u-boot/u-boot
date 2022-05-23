/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 *	Andy Fleming <afleming@gmail.com>
 *
 * This file pretty much stolen from Linux's mii.h/ethtool.h/phy.h
 */

#ifndef _PHY_INTERFACE_H
#define _PHY_INTERFACE_H

#include <string.h>

typedef enum {
	PHY_INTERFACE_MODE_NA, /* don't touch */
	PHY_INTERFACE_MODE_MII,
	PHY_INTERFACE_MODE_GMII,
	PHY_INTERFACE_MODE_SGMII,
	PHY_INTERFACE_MODE_SGMII_2500,
	PHY_INTERFACE_MODE_QSGMII,
	PHY_INTERFACE_MODE_TBI,
	PHY_INTERFACE_MODE_RMII,
	PHY_INTERFACE_MODE_RGMII,
	PHY_INTERFACE_MODE_RGMII_ID,
	PHY_INTERFACE_MODE_RGMII_RXID,
	PHY_INTERFACE_MODE_RGMII_TXID,
	PHY_INTERFACE_MODE_RTBI,
	PHY_INTERFACE_MODE_1000BASEX,
	PHY_INTERFACE_MODE_2500BASEX,
	PHY_INTERFACE_MODE_XGMII,
	PHY_INTERFACE_MODE_XAUI,
	PHY_INTERFACE_MODE_RXAUI,
	PHY_INTERFACE_MODE_SFI,
	PHY_INTERFACE_MODE_INTERNAL,
	PHY_INTERFACE_MODE_25G_AUI,
	PHY_INTERFACE_MODE_XLAUI,
	PHY_INTERFACE_MODE_CAUI2,
	PHY_INTERFACE_MODE_CAUI4,
	PHY_INTERFACE_MODE_NCSI,
	PHY_INTERFACE_MODE_10GBASER,
	PHY_INTERFACE_MODE_USXGMII,
	PHY_INTERFACE_MODE_MAX,
} phy_interface_t;

static const char * const phy_interface_strings[] = {
	[PHY_INTERFACE_MODE_NA]		= "",
	[PHY_INTERFACE_MODE_MII]		= "mii",
	[PHY_INTERFACE_MODE_GMII]		= "gmii",
	[PHY_INTERFACE_MODE_SGMII]		= "sgmii",
	[PHY_INTERFACE_MODE_SGMII_2500]		= "sgmii-2500",
	[PHY_INTERFACE_MODE_QSGMII]		= "qsgmii",
	[PHY_INTERFACE_MODE_TBI]		= "tbi",
	[PHY_INTERFACE_MODE_RMII]		= "rmii",
	[PHY_INTERFACE_MODE_RGMII]		= "rgmii",
	[PHY_INTERFACE_MODE_RGMII_ID]		= "rgmii-id",
	[PHY_INTERFACE_MODE_RGMII_RXID]		= "rgmii-rxid",
	[PHY_INTERFACE_MODE_RGMII_TXID]		= "rgmii-txid",
	[PHY_INTERFACE_MODE_RTBI]		= "rtbi",
	[PHY_INTERFACE_MODE_1000BASEX]		= "1000base-x",
	[PHY_INTERFACE_MODE_2500BASEX]		= "2500base-x",
	[PHY_INTERFACE_MODE_XGMII]		= "xgmii",
	[PHY_INTERFACE_MODE_XAUI]		= "xaui",
	[PHY_INTERFACE_MODE_RXAUI]		= "rxaui",
	[PHY_INTERFACE_MODE_SFI]		= "sfi",
	[PHY_INTERFACE_MODE_INTERNAL]		= "internal",
	[PHY_INTERFACE_MODE_25G_AUI]		= "25g-aui",
	[PHY_INTERFACE_MODE_XLAUI]		= "xlaui4",
	[PHY_INTERFACE_MODE_CAUI2]		= "caui2",
	[PHY_INTERFACE_MODE_CAUI4]		= "caui4",
	[PHY_INTERFACE_MODE_NCSI]		= "NC-SI",
	[PHY_INTERFACE_MODE_10GBASER]		= "10gbase-r",
	[PHY_INTERFACE_MODE_USXGMII]		= "usxgmii",
};

/* Backplane modes:
 * are considered a sub-type of phy_interface_t: XGMII
 * and are specified in "phy-connection-type" with one of the following strings
 */
static const char * const backplane_mode_strings[] = {
	"10gbase-kr",
	"40gbase-kr4",
};

static inline const char *phy_string_for_interface(phy_interface_t i)
{
	/* Default to unknown */
	if (i >= PHY_INTERFACE_MODE_MAX)
		i = PHY_INTERFACE_MODE_NA;

	return phy_interface_strings[i];
}

static inline bool is_backplane_mode(const char *phyconn)
{
	int i;

	if (!phyconn)
		return false;
	for (i = 0; i < ARRAY_SIZE(backplane_mode_strings); i++) {
		if (!strcmp(phyconn, backplane_mode_strings[i]))
			return true;
	}
	return false;
}

#endif /* _PHY_INTERFACE_H */
