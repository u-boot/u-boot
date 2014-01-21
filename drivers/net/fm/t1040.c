/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	return PHY_INTERFACE_MODE_NONE;
}
