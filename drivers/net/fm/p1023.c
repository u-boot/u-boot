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
	[FM1_DTSEC1] = MPC85xx_DEVDISR_TSEC1,
	[FM1_DTSEC2] = MPC85xx_DEVDISR_TSEC2,
};

static int is_device_disabled(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr = in_be32(&gur->devdisr);

	return port_to_devdisr[port] & devdisr;
}

void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* don't allow disabling of DTSEC1 as its needed for MDIO */
	if (port == FM1_DTSEC1)
		return;

	setbits_be32(&gur->devdisr, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 pordevsr = in_be32(&gur->pordevsr);

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NONE;

	/* DTSEC1 can be SGMII, RGMII or RMII */
	if (port == FM1_DTSEC1) {
		if (is_serdes_configured(SGMII_FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		if (pordevsr & MPC85xx_PORDEVSR_SGMII1_DIS) {
			if (pordevsr & MPC85xx_PORDEVSR_TSEC1_PRTC)
				return PHY_INTERFACE_MODE_RGMII;
			else
				return PHY_INTERFACE_MODE_RMII;
		}
	}

	/* DTSEC2 only supports SGMII or RGMII */
	if (port == FM1_DTSEC2) {
		if (is_serdes_configured(SGMII_FM1_DTSEC2))
			return PHY_INTERFACE_MODE_SGMII;
		if (pordevsr & MPC85xx_PORDEVSR_SGMII2_DIS)
			return PHY_INTERFACE_MODE_RGMII;
	}

	return PHY_INTERFACE_MODE_NONE;
}
