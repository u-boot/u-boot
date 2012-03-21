/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
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

typedef struct serdes_85xx {
	u32	srdscr0;	/* 0x00 - SRDS Control Register 0 */
	u32	srdscr1;	/* 0x04 - SRDS Control Register 1 */
	u32	srdscr2;	/* 0x08 - SRDS Control Register 2 */
	u32	srdscr3;	/* 0x0C - SRDS Control Register 3 */
	u32	srdscr4;	/* 0x10 - SRDS Control Register 4 */
} serdes_85xx_t;
#define FSL_SRDSCR3_EIC0(x)	(((x) & 0x1f) << 8)
#define FSL_SRDSCR3_EIC0_MASK	FSL_SRDSCR3_EIC0(0x1f)
#define FSL_SRDSCR3_EIC1(x)	(((x) & 0x1f) << 0)
#define FSL_SRDSCR3_EIC1_MASK	FSL_SRDSCR3_EIC1(0x1f)
#define FSL_SRDSCR4_EIC2(x)	(((x) & 0x1f) << 8)
#define FSL_SRDSCR4_EIC2_MASK	FSL_SRDSCR4_EIC2(0x1f)
#define FSL_SRDSCR4_EIC3(x)	(((x) & 0x1f) << 0)
#define FSL_SRDSCR4_EIC3_MASK	FSL_SRDSCR4_EIC3(0x1f)
#define EIC_PCIE	0x13
#define EIC_SGMII	0x04

#define SRDS1_MAX_LANES		4

static u32 serdes1_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x0] = {PCIE1, NONE, NONE, NONE},
	[0x6] = {PCIE1, PCIE1, PCIE1, PCIE1},
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
	serdes_85xx_t *serdes = (void *)CONFIG_SYS_MPC85xx_SERDES1_ADDR;

	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	int lane;
	u32 mask, val;

	debug("PORDEVSR[IO_SEL_SRDS] = %x\n", srds_cfg);

	if (srds_cfg > ARRAY_SIZE(serdes1_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL_SRDS] = %d\n", srds_cfg);
		return;
	}

	for (lane = 0; lane < SRDS1_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes1_cfg_tbl[srds_cfg][lane];
		serdes1_prtcl_map |= (1 << lane_prtcl);
	}

	/* Init SERDES Receiver electrical idle detection control for PCIe */

	/* Lane 0 is always PCIe 1 */
	mask = FSL_SRDSCR3_EIC0_MASK;
	val = FSL_SRDSCR3_EIC0(EIC_PCIE);

	/* Lane 1 */
	if ((serdes1_cfg_tbl[srds_cfg][1] == PCIE1) ||
	    (serdes1_cfg_tbl[srds_cfg][1] == PCIE2)) {
		mask |= FSL_SRDSCR3_EIC1_MASK;
		val |= FSL_SRDSCR3_EIC1(EIC_PCIE);
	}

	/* Handle lanes 0 & 1 */
	clrsetbits_be32(&serdes->srdscr3, mask, val);

	/* Handle lanes 2 & 3 */
	if (srds_cfg == 0x6) {
		mask = FSL_SRDSCR4_EIC2_MASK | FSL_SRDSCR4_EIC3_MASK;
		val = FSL_SRDSCR4_EIC2(EIC_PCIE) | FSL_SRDSCR4_EIC3(EIC_PCIE);
		clrsetbits_be32(&serdes->srdscr4, mask, val);
	}

	/* 100 ms delay */
	udelay(100000);
}
