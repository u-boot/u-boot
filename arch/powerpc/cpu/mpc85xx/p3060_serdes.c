/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <asm/processor.h>
#include <asm/io.h>
#include "fsl_corenet_serdes.h"

static u8 serdes_cfg_tbl[][SRDS_MAX_LANES] = {
	[0x03] = {PCIE1, PCIE1, PCIE1, PCIE1, PCIE2, PCIE2, PCIE2, PCIE2,
		  SGMII_FM2_DTSEC4, SGMII_FM1_DTSEC4, SGMII_FM2_DTSEC1,
		  SGMII_FM1_DTSEC1, SGMII_FM2_DTSEC2, SGMII_FM1_DTSEC2,
		  NONE, NONE, AURORA, AURORA},
	[0x06] = {PCIE1, PCIE1, PCIE1, PCIE1, NONE, NONE, SGMII_FM2_DTSEC3,
		  SGMII_FM1_DTSEC3, SGMII_FM2_DTSEC4, SGMII_FM1_DTSEC4,
		  SGMII_FM2_DTSEC1, SGMII_FM1_DTSEC1, SGMII_FM2_DTSEC2,
		  SGMII_FM1_DTSEC2, NONE, NONE, AURORA, AURORA},
	[0x16] = {SRIO2, SRIO2, SRIO2, SRIO2, SRIO1, SRIO1, SRIO1, SRIO1,
		  AURORA, AURORA, SGMII_FM2_DTSEC1, SGMII_FM1_DTSEC1,
		  SGMII_FM2_DTSEC2, SGMII_FM1_DTSEC2, SGMII_FM2_DTSEC3,
		  SGMII_FM1_DTSEC3, SGMII_FM2_DTSEC4, SGMII_FM1_DTSEC4},
	[0x19] = {SRIO2, SRIO2, SRIO2, SRIO2, SRIO1, SRIO1, SRIO1, SRIO1,
		  AURORA, AURORA, PCIE2, PCIE2, PCIE2, PCIE2, SGMII_FM2_DTSEC3,
		  SGMII_FM1_DTSEC3, SGMII_FM2_DTSEC4, SGMII_FM1_DTSEC4},
	[0x1c] = {NONE, NONE, SRIO1, SRIO2,  NONE, NONE, NONE, NONE,
		  AURORA, AURORA, SGMII_FM2_DTSEC1, SGMII_FM1_DTSEC1,
		  SGMII_FM2_DTSEC2, SGMII_FM1_DTSEC2, SGMII_FM2_DTSEC3,
		  SGMII_FM1_DTSEC3, SGMII_FM2_DTSEC4, SGMII_FM1_DTSEC4},
};

enum srds_prtcl serdes_get_prtcl(int cfg, int lane)
{
	if (!serdes_lane_enabled(lane))
		return NONE;

	return serdes_cfg_tbl[cfg][lane];
}

int is_serdes_prtcl_valid(u32 prtcl)
{
	int i;

	if (prtcl > ARRAY_SIZE(serdes_cfg_tbl))
		return 0;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_cfg_tbl[prtcl][i] != NONE)
			return 1;
	}

	return 0;
}

void soc_serdes_init(void)
{
	/*
	 * On the P3060 the devdisr2 register does not correctly reflect
	 * the state of the MACs based on the RCW fields. So disable the MACs
	 * based on the srds_prtcl and ec1, ec2, ec3 fields
	 */

	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr2 = in_be32(&gur->devdisr2);
	u32 rcwsr11 = in_be32(&gur->rcwsr[11]);

	/* NOTE: Leave FM1-1,FM1-2 alone for MDIO access */

	if (!is_serdes_configured(SGMII_FM1_DTSEC3))
		devdisr2 |= FSL_CORENET_DEVDISR2_DTSEC1_3;

	if (!is_serdes_configured(SGMII_FM1_DTSEC4))
		devdisr2 |= FSL_CORENET_DEVDISR2_DTSEC1_4;

	if (!is_serdes_configured(SGMII_FM2_DTSEC1))
		devdisr2 |= FSL_CORENET_DEVDISR2_DTSEC2_1;

	if (!is_serdes_configured(SGMII_FM2_DTSEC2))
		devdisr2 |= FSL_CORENET_DEVDISR2_DTSEC2_2;

	if (!is_serdes_configured(SGMII_FM2_DTSEC3))
		devdisr2 |= FSL_CORENET_DEVDISR2_DTSEC2_3;

	if (!is_serdes_configured(SGMII_FM2_DTSEC4))
		devdisr2 |= FSL_CORENET_DEVDISR2_DTSEC2_4;

	if ((rcwsr11 & FSL_CORENET_RCWSR11_EC2) ==
		FSL_CORENET_RCWSR11_EC2_FM1_DTSEC2) {
		devdisr2 &= ~FSL_CORENET_DEVDISR2_DTSEC1_2;
	}

	if ((rcwsr11 & FSL_CORENET_RCWSR11_EC2) ==
		FSL_CORENET_RCWSR11_EC2_FM2_DTSEC1) {
		devdisr2 &= ~FSL_CORENET_DEVDISR2_DTSEC2_1;
	}

	out_be32(&gur->devdisr2, devdisr2);
}
