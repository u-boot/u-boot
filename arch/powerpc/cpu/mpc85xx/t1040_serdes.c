/*
 * Copyright 2012 Freescale Semiconductor, Inc.
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
#include <asm/fsl_serdes.h>
#include <asm/processor.h>
#include <asm/io.h>
#include "fsl_corenet2_serdes.h"

static u8 serdes_cfg_tbl[MAX_SERDES][0xC4][SRDS_MAX_LANES] = {
	{	/* SerDes 1 */
	[0x69] = {PCIE1, SGMII_FM1_DTSEC3, QSGMII_SW1_A, QSGMII_SW1_B,
		PCIE2, PCIE3, SGMII_FM1_DTSEC4, SATA1},
	[0x66] = {PCIE1, SGMII_FM1_DTSEC3, QSGMII_SW1_A, QSGMII_SW1_B,
		PCIE2, PCIE3, PCIE4, SATA1},
	[0x67] = {PCIE1, SGMII_FM1_DTSEC3, QSGMII_SW1_A, QSGMII_SW1_B,
		PCIE2, PCIE3, PCIE4, SGMII_FM1_DTSEC5},
	[0x60] = {PCIE1, SGMII_FM1_DTSEC3, QSGMII_SW1_A, QSGMII_SW1_B,
		PCIE2, PCIE2, PCIE2, PCIE2},
	[0x8D] = {PCIE1, SGMII_SW1_DTSEC3, SGMII_SW1_DTSEC1, SGMII_SW1_DTSEC2,
		PCIE2, SGMII_SW1_DTSEC6, SGMII_SW1_DTSEC4, SGMII_SW1_DTSEC5},
	[0x89] = {PCIE1, SGMII_SW1_DTSEC3, SGMII_SW1_DTSEC1, SGMII_SW1_DTSEC2,
		PCIE2, PCIE3, SGMII_SW1_DTSEC4, SATA1},
	[0x86] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		PCIE2, PCIE3, PCIE4, SATA1},
	[0x87] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		PCIE2, PCIE3, PCIE4, SGMII_FM1_DTSEC5},
	[0xA7] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		 PCIE2, PCIE3, PCIE4, SGMII_FM1_DTSEC5},
	[0xAA] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		 PCIE2, PCIE3, SGMII_FM1_DTSEC4, SGMII_FM1_DTSEC5},
	[0x40] = {PCIE1, PCIE1, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		PCIE2, PCIE2, PCIE2, PCIE2},
	[0x06] = {PCIE1, PCIE1, PCIE1, PCIE1,
		PCIE2, PCIE3, PCIE4, SATA1},
	[0x08] = {PCIE1, PCIE1, PCIE1, PCIE1,
		PCIE2, PCIE3, SATA2, SATA1},
	[0x8F] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		AURORA, NONE, SGMII_FM1_DTSEC4, SGMII_FM1_DTSEC5},
	[0x85] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		PCIE2, PCIE2, SGMII_FM1_DTSEC4, SGMII_FM1_DTSEC5},
	[0xA5] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		PCIE2, PCIE2, SGMII_FM1_DTSEC4, SGMII_FM1_DTSEC5},
	[0x00] = {PCIE1, PCIE1, PCIE1, PCIE1,
		PCIE2, PCIE2, PCIE2, PCIE2},
	},
	{
	},
	{
	},
	{
	},
};


enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane)
{
	return serdes_cfg_tbl[serdes][cfg][lane];
}

int is_serdes_prtcl_valid(int serdes, u32 prtcl)
{
	int i;

	if (prtcl >= ARRAY_SIZE(serdes_cfg_tbl[serdes]))
		return 0;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_cfg_tbl[serdes][prtcl][i] != NONE)
			return 1;
	}

	return 0;
}
