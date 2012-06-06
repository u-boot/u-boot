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
#include <asm/immap_86xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4
#define SRDS2_MAX_LANES		4

static u32 serdes1_prtcl_map, serdes2_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x1] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x4] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x7] = {NONE, NONE, NONE, NONE},
};

static u8 serdes2_cfg_tbl[][SRDS2_MAX_LANES] = {
	[0x0] = {PCIE2, PCIE2, PCIE2, PCIE2},
	[0x4] = {PCIE2, PCIE2, PCIE2, PCIE2},
	[0x7] = {NONE, NONE, NONE, NONE},
};

int is_serdes_configured(enum srds_prtcl device)
{
	int ret = (1 << device) & serdes1_prtcl_map;

	if (ret)
		return ret;

	return (1 << device) & serdes2_prtcl_map;
}

void fsl_serdes_init(void)
{
	immap_t *immap = (immap_t *) CONFIG_SYS_CCSRBAR;
	ccsr_gur_t *gur = &immap->im_gur;
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC8610_PORDEVSR_IO_SEL) >>
				MPC8610_PORDEVSR_IO_SEL_SHIFT;
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

	if (srds_cfg > ARRAY_SIZE(serdes2_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL_SRDS] = %d\n", srds_cfg);
		return;
	}

	for (lane = 0; lane < SRDS2_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes2_cfg_tbl[srds_cfg][lane];
		serdes2_prtcl_map |= (1 << lane_prtcl);
	}
}
