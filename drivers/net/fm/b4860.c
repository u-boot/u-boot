/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *	Roy Zang <tie-fei.zang@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
	[FM1_10GEC1] = FSL_CORENET_DEVDISR2_10GEC1_1,
	[FM1_10GEC2] = FSL_CORENET_DEVDISR2_10GEC1_2,
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

	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

void fman_enable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	clrbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NONE;

	/*B4860 has two 10Gig Mac*/
	if ((port == FM1_10GEC1 || port == FM1_10GEC2)	&&
	    ((is_serdes_configured(XAUI_FM1_MAC9))	||
	    (is_serdes_configured(XAUI_FM1_MAC10))))
		return PHY_INTERFACE_MODE_XGMII;

	/* Fix me need to handle RGMII here first */

	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
	case FM1_DTSEC3:
	case FM1_DTSEC4:
	case FM1_DTSEC5:
	case FM1_DTSEC6:
		if (is_serdes_configured(SGMII_FM1_DTSEC1 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		break;
	default:
		return PHY_INTERFACE_MODE_NONE;
	}

	return PHY_INTERFACE_MODE_NONE;
}
