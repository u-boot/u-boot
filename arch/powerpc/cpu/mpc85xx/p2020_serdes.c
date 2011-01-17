/*
 * Copyright 2010 Freescale Semiconductor, Inc.
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

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4

static u32 serdes1_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x0] = {PCIE1, NONE, NONE, NONE},
	[0x2] = {PCIE1, PCIE2, PCIE3, PCIE3},
	[0x4] = {PCIE1, PCIE1, PCIE3, PCIE3},
	[0x6] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x7] = {SRIO2, SRIO1, NONE, NONE},
	[0x8] = {SRIO2, SRIO2, SRIO2, SRIO2},
	[0x9] = {SRIO2, SRIO2, SRIO2, SRIO2},
	[0xa] = {SRIO2, SRIO2, SRIO2, SRIO2},
	[0xb] = {SRIO2, SRIO1, SGMII_TSEC2, SGMII_TSEC3},
	[0xc] = {SRIO2, SRIO1, SGMII_TSEC2, SGMII_TSEC3},
	[0xd] = {PCIE1, SRIO1, SGMII_TSEC2, SGMII_TSEC3},
	[0xe] = {PCIE1, PCIE2, SGMII_TSEC2, SGMII_TSEC3},
	[0xf] = {PCIE1, PCIE1, SGMII_TSEC2, SGMII_TSEC3},
};

int is_serdes_configured(enum srds_prtcl prtcl)
{
	return (1 << prtcl) & serdes1_prtcl_map;
}

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	int lane;

	debug("PORDEVSR[IO_SEL_SRDS] = %x\n", srds_cfg);

	if (srds_cfg > ARRAY_SIZE(serdes1_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL_SRDS] = %d\n", srds_cfg);
		return;
	}

	for (lane = 0; lane < SRDS1_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes1_cfg_tbl[srds_cfg][lane];
		serdes1_prtcl_map |= (1 << lane_prtcl);
	}
}
