/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Author: Timur Tabi <timur@freescale.com>
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
#define SRDS2_MAX_LANES		2

static const u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x00] = {NONE, NONE, NONE, NONE},
	[0x01] = {NONE, NONE, NONE, NONE},
	[0x02] = {NONE, NONE, NONE, NONE},
	[0x03] = {NONE, NONE, NONE, NONE},
	[0x04] = {NONE, NONE, NONE, NONE},
	[0x06] = {PCIE1, PCIE3, SGMII_TSEC1, PCIE2},
	[0x07] = {PCIE1, PCIE3, SGMII_TSEC1, PCIE2},
	[0x09] = {PCIE1, NONE, NONE, NONE},
	[0x0a] = {PCIE1, PCIE3, SGMII_TSEC1, SGMII_TSEC2},
	[0x0b] = {PCIE1, PCIE3, SGMII_TSEC1, SGMII_TSEC2},
	[0x0d] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x0e] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x0f] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x15] = {PCIE1, PCIE3, PCIE2, PCIE2},
	[0x16] = {PCIE1, PCIE3, PCIE2, PCIE2},
	[0x17] = {PCIE1, PCIE3, PCIE2, PCIE2},
	[0x18] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x19] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1a] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1b] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1c] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x1d] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1e] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1f] = {PCIE1, PCIE1, PCIE2, PCIE2},
};

static const u8 serdes2_cfg_tbl[][SRDS2_MAX_LANES] = {
	[0x00] = {PCIE3, PCIE3},
	[0x01] = {PCIE2, PCIE3},
	[0x02] = {SATA1, SATA2},
	[0x03] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x04] = {NONE, NONE},
	[0x06] = {SATA1, SATA2},
	[0x07] = {NONE, NONE},
	[0x09] = {PCIE3, PCIE2},
	[0x0a] = {SATA1, SATA2},
	[0x0b] = {NONE, NONE},
	[0x0d] = {PCIE3, PCIE2},
	[0x0e] = {SATA1, SATA2},
	[0x0f] = {NONE, NONE},
	[0x15] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x16] = {SATA1, SATA2},
	[0x17] = {NONE, NONE},
	[0x18] = {PCIE3, PCIE3},
	[0x19] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x1a] = {SATA1, SATA2},
	[0x1b] = {NONE, NONE},
	[0x1c] = {PCIE3, PCIE3},
	[0x1d] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x1e] = {SATA1, SATA2},
	[0x1f] = {NONE, NONE},
};

int is_serdes_configured(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	unsigned int i;

	debug("%s: dev = %d\n", __FUNCTION__, device);
	debug("PORDEVSR[IO_SEL] = 0x%x\n", srds_cfg);

	if (srds_cfg > ARRAY_SIZE(serdes1_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL] = %d\n", srds_cfg);
		return 0;
	}

	for (i = 0; i < SRDS1_MAX_LANES; i++) {
		if (serdes1_cfg_tbl[srds_cfg][i] == device)
			return 1;
		if (serdes2_cfg_tbl[srds_cfg][i] == device)
			return 1;
	}

	return 0;
}
