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

struct serdes_config {
	u8 protocol;
	u8 lanes[SRDS_MAX_LANES];
};

static struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{0x0D, {CPRI8, CPRI7, CPRI6, CPRI5,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x0E, {CPRI8, CPRI7,	CPRI6, CPRI5,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x12, {CPRI8, CPRI7,	CPRI6, CPRI5,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x2a, {SGMII_FM1_DTSEC5, SGMII_FM1_DTSEC6,
		CPRI6, CPRI5, CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x30, {AURORA, AURORA,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x32, {AURORA, AURORA,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x33, {AURORA, AURORA,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x34, {AURORA, AURORA,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{0x3E, {CPRI8, CPRI7,	CPRI6, CPRI5,
		CPRI4, CPRI3, CPRI2, CPRI1}},
	{}
};
static struct serdes_config serdes2_cfg_tbl[] = {
	/* SerDes 2 */
	{0x18, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		AURORA, AURORA,	SRIO1, SRIO1}},
	{0x1D, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		AURORA, AURORA,	SRIO1, SRIO1}},
	{0x2B, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SRIO2, SRIO2,
		AURORA, AURORA, SRIO1, SRIO1}},
	{0x30, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SRIO2, SRIO2,
		AURORA, AURORA,
		SRIO1, SRIO1}},
	{0x49, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SGMII_FM1_DTSEC3, AURORA,
		SRIO1, SRIO1, SRIO1, SRIO1}},
	{0x4A, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SGMII_FM1_DTSEC3, AURORA,
		SRIO1, SRIO1, SRIO1, SRIO1}},
	{0x4C, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SGMII_FM1_DTSEC3, AURORA,
		SRIO1, SRIO1, SRIO1, SRIO1}},
	{0x4E, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SGMII_FM1_DTSEC3, AURORA,
		SRIO1, SRIO1, SRIO1, SRIO1}},
	{0x84, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SRIO2, SRIO2, AURORA, AURORA,
		XFI_FM1_MAC9, XFI_FM1_MAC10}},
	{0x85, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SRIO2, SRIO2, AURORA, AURORA,
		XFI_FM1_MAC9, XFI_FM1_MAC10}},
	{0x87, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SRIO2, SRIO2,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		XFI_FM1_MAC9, XFI_FM1_MAC10}},
	{0x93, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		XAUI_FM1_MAC10, XAUI_FM1_MAC10,
		XAUI_FM1_MAC10, XAUI_FM1_MAC10}},
	{0x9E, {PCIE1, PCIE1,	PCIE1, PCIE1,
		XAUI_FM1_MAC10, XAUI_FM1_MAC10,
		XAUI_FM1_MAC10, XAUI_FM1_MAC10}},
	{0x9A, {PCIE1, PCIE1,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		XAUI_FM1_MAC10, XAUI_FM1_MAC10,
		XAUI_FM1_MAC10, XAUI_FM1_MAC10}},
	{0xB2, {PCIE1, PCIE1, PCIE1, PCIE1,
		SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC4,
		XFI_FM1_MAC9, XFI_FM1_MAC10}},
	{0xC3, {XAUI_FM1_MAC9, XAUI_FM1_MAC9,
		XAUI_FM1_MAC9, XAUI_FM1_MAC9,
		SRIO1, SRIO1, SRIO1, SRIO1}},
	{}
};
static struct serdes_config *serdes_cfg_tbl[] = {
	serdes1_cfg_tbl,
	serdes2_cfg_tbl,
};

enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane)
{
	struct serdes_config *ptr;

	if (serdes >= ARRAY_SIZE(serdes_cfg_tbl))
		return 0;

	ptr = serdes_cfg_tbl[serdes];
	while (ptr->protocol) {
		if (ptr->protocol == cfg)
			return ptr->lanes[lane];
		ptr++;
	}

	return 0;
}

int is_serdes_prtcl_valid(int serdes, u32 prtcl)
{
	int i;
	struct serdes_config *ptr;

	if (serdes >= ARRAY_SIZE(serdes_cfg_tbl))
		return 0;

	ptr = serdes_cfg_tbl[serdes];
	while (ptr->protocol) {
		if (ptr->protocol == prtcl)
			break;
		ptr++;
	}

	if (!ptr->protocol)
		return 0;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (ptr->lanes[i] != NONE)
			return 1;
	}

	return 0;
}
