/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Author: Roy Zang <tie-fei.zang@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4

static u32 serdes1_prtcl_map;

static const u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x00] = {PCIE1, PCIE2, NONE, NONE},
	[0x01] = {PCIE1, PCIE2, PCIE3, NONE},
	[0x02] = {PCIE1, PCIE2, PCIE3, SGMII_FM1_DTSEC2},
	[0x03] = {PCIE1, PCIE2, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2},
};

int is_serdes_configured(enum srds_prtcl device)
{
	int ret = (1 << device) & serdes1_prtcl_map;
	return ret;
}

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
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
