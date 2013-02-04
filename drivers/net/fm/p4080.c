/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

static u32 port_to_devdisr[] = {
	[FM1_DTSEC1] = FSL_CORENET_DEVDISR2_DTSEC1_1,
	[FM1_DTSEC2] = FSL_CORENET_DEVDISR2_DTSEC1_2,
	[FM1_DTSEC3] = FSL_CORENET_DEVDISR2_DTSEC1_3,
	[FM1_DTSEC4] = FSL_CORENET_DEVDISR2_DTSEC1_4,
	[FM1_10GEC1] = FSL_CORENET_DEVDISR2_10GEC1,
	[FM2_DTSEC1] = FSL_CORENET_DEVDISR2_DTSEC2_1,
	[FM2_DTSEC2] = FSL_CORENET_DEVDISR2_DTSEC2_2,
	[FM2_DTSEC3] = FSL_CORENET_DEVDISR2_DTSEC2_3,
	[FM2_DTSEC4] = FSL_CORENET_DEVDISR2_DTSEC2_4,
	[FM2_10GEC1] = FSL_CORENET_DEVDISR2_10GEC2,
};

static int is_device_disabled(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr2 = in_be32(&gur->devdisr2);

	return port_to_devdisr[port] & devdisr2;
}

void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* don't allow disabling of DTSEC1 as its needed for MDIO */
	if (port == FM1_DTSEC1)
		return;

	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 rcwsr11 = in_be32(&gur->rcwsr[11]);

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NONE;

	if ((port == FM1_10GEC1) && (is_serdes_configured(XAUI_FM1)))
		return PHY_INTERFACE_MODE_XGMII;

	if ((port == FM2_10GEC1) && (is_serdes_configured(XAUI_FM2)))
		return PHY_INTERFACE_MODE_XGMII;

	/* handle RGMII first */
	if ((port == FM1_DTSEC1) && ((rcwsr11 & FSL_CORENET_RCWSR11_EC1) ==
		FSL_CORENET_RCWSR11_EC1_FM1_DTSEC1))
		return PHY_INTERFACE_MODE_RGMII;

	if ((port == FM1_DTSEC2) && ((rcwsr11 & FSL_CORENET_RCWSR11_EC2) ==
		FSL_CORENET_RCWSR11_EC2_FM1_DTSEC2))
		return PHY_INTERFACE_MODE_RGMII;

	if ((port == FM2_DTSEC1) && ((rcwsr11 & FSL_CORENET_RCWSR11_EC2) ==
		FSL_CORENET_RCWSR11_EC2_FM2_DTSEC1))
		return PHY_INTERFACE_MODE_RGMII;

	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
	case FM1_DTSEC3:
	case FM1_DTSEC4:
		if (is_serdes_configured(SGMII_FM1_DTSEC1 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		break;
	case FM2_DTSEC1:
	case FM2_DTSEC2:
	case FM2_DTSEC3:
	case FM2_DTSEC4:
		if (is_serdes_configured(SGMII_FM2_DTSEC1 + port - FM2_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		break;
	default:
		return PHY_INTERFACE_MODE_NONE;
	}

	return PHY_INTERFACE_MODE_NONE;
}
