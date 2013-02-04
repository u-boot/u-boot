/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *	Roy Zang <tie-fei.zang@freescale.com>
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

u32 port_to_devdisr[] = {
	[FM1_DTSEC1] = FSL_CORENET_DEVDISR2_DTSEC1_1,
	[FM1_DTSEC2] = FSL_CORENET_DEVDISR2_DTSEC1_2,
	[FM1_DTSEC3] = FSL_CORENET_DEVDISR2_DTSEC1_3,
	[FM1_DTSEC4] = FSL_CORENET_DEVDISR2_DTSEC1_4,
	[FM1_DTSEC5] = FSL_CORENET_DEVDISR2_DTSEC1_5,
	[FM1_DTSEC6] = FSL_CORENET_DEVDISR2_DTSEC1_6,
	[FM1_DTSEC9] = FSL_CORENET_DEVDISR2_DTSEC1_9,
	[FM1_DTSEC10] = FSL_CORENET_DEVDISR2_DTSEC1_10,
	[FM1_10GEC1] = FSL_CORENET_DEVDISR2_10GEC1_1,
	[FM1_10GEC2] = FSL_CORENET_DEVDISR2_10GEC1_2,
	[FM2_DTSEC1] = FSL_CORENET_DEVDISR2_DTSEC2_1,
	[FM2_DTSEC2] = FSL_CORENET_DEVDISR2_DTSEC2_2,
	[FM2_DTSEC3] = FSL_CORENET_DEVDISR2_DTSEC2_3,
	[FM2_DTSEC4] = FSL_CORENET_DEVDISR2_DTSEC2_4,
	[FM2_DTSEC5] = FSL_CORENET_DEVDISR2_DTSEC2_5,
	[FM2_DTSEC6] = FSL_CORENET_DEVDISR2_DTSEC2_6,
	[FM2_DTSEC9] = FSL_CORENET_DEVDISR2_DTSEC2_9,
	[FM2_DTSEC10] = FSL_CORENET_DEVDISR2_DTSEC2_10,
	[FM2_10GEC1] = FSL_CORENET_DEVDISR2_10GEC2_1,
	[FM2_10GEC2] = FSL_CORENET_DEVDISR2_10GEC2_2,
};

static int is_device_disabled(enum fm_port port)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr2 = in_be32(&gur->devdisr2);

	return port_to_devdisr[port] & devdisr2;
}

void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 rcwsr13 = in_be32(&gur->rcwsr[13]);

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NONE;

	if ((port == FM1_10GEC1 || port == FM1_10GEC2)
			&& (is_serdes_configured(XAUI_FM1)))
		return PHY_INTERFACE_MODE_XGMII;

	if ((port == FM2_10GEC1 || port == FM2_10GEC2)
			&& (is_serdes_configured(XAUI_FM2)))
		return PHY_INTERFACE_MODE_XGMII;

#define FSL_CORENET_RCWSR13_EC1			0x60000000 /* bits 417..418 */
#define FSL_CORENET_RCWSR13_EC1_FM2_DTSEC5_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC1_FM2_GPIO		0x40000000
#define FSL_CORENET_RCWSR13_EC2			0x18000000 /* bits 419..420 */
#define FSL_CORENET_RCWSR13_EC2_FM1_DTSEC5_RGMII	0x00000000
#define FSL_CORENET_RCWSR13_EC2_FM2_DTSEC6_RGMII	0x08000000
#define FSL_CORENET_RCWSR13_EC2_FM1_GPIO		0x10000000
	/* handle RGMII first */
	if ((port == FM2_DTSEC5) && ((rcwsr13 & FSL_CORENET_RCWSR13_EC1) ==
		FSL_CORENET_RCWSR13_EC1_FM2_DTSEC5_RGMII))
		return PHY_INTERFACE_MODE_RGMII;

	if ((port == FM1_DTSEC5) && ((rcwsr13 & FSL_CORENET_RCWSR13_EC2) ==
		FSL_CORENET_RCWSR13_EC2_FM1_DTSEC5_RGMII))
		return PHY_INTERFACE_MODE_RGMII;

	if ((port == FM2_DTSEC6) && ((rcwsr13 & FSL_CORENET_RCWSR13_EC2) ==
		FSL_CORENET_RCWSR13_EC2_FM2_DTSEC6_RGMII))
		return PHY_INTERFACE_MODE_RGMII;
	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
	case FM1_DTSEC3:
	case FM1_DTSEC4:
	case FM1_DTSEC5:
	case FM1_DTSEC6:
	case FM1_DTSEC9:
	case FM1_DTSEC10:
		if (is_serdes_configured(SGMII_FM1_DTSEC1 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		break;
	case FM2_DTSEC1:
	case FM2_DTSEC2:
	case FM2_DTSEC3:
	case FM2_DTSEC4:
	case FM2_DTSEC5:
	case FM2_DTSEC6:
	case FM2_DTSEC9:
	case FM2_DTSEC10:
		if (is_serdes_configured(SGMII_FM2_DTSEC1 + port - FM2_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		break;
	default:
		return PHY_INTERFACE_MODE_NONE;
	}

	return PHY_INTERFACE_MODE_NONE;
}
