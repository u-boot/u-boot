/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
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
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <asm/processor.h>
#include "fsl_corenet_serdes.h"

static u32 serdes_prtcl_map;

#ifdef DEBUG
static const char *serdes_prtcl_str[] = {
	[NONE] = "NA",
	[PCIE1] = "PCIE1",
	[PCIE2] = "PCIE2",
	[PCIE3] = "PCIE3",
	[PCIE4] = "PCIE4",
	[SATA1] = "SATA1",
	[SATA2] = "SATA2",
	[SRIO1] = "SRIO1",
	[SRIO2] = "SRIO2",
	[SGMII_FM1_DTSEC1] = "SGMII_FM1_DTSEC1",
	[SGMII_FM1_DTSEC2] = "SGMII_FM1_DTSEC2",
	[SGMII_FM1_DTSEC3] = "SGMII_FM1_DTSEC3",
	[SGMII_FM1_DTSEC4] = "SGMII_FM1_DTSEC4",
	[SGMII_FM1_DTSEC5] = "SGMII_FM1_DTSEC5",
	[SGMII_FM2_DTSEC1] = "SGMII_FM2_DTSEC1",
	[SGMII_FM2_DTSEC2] = "SGMII_FM2_DTSEC2",
	[SGMII_FM2_DTSEC3] = "SGMII_FM2_DTSEC3",
	[SGMII_FM2_DTSEC4] = "SGMII_FM2_DTSEC4",
	[XAUI_FM1] = "XAUI_FM1",
	[XAUI_FM2] = "XAUI_FM2",
	[AURORA] = "DEBUG",
};
#endif

static const struct {
	int idx;
	unsigned int lpd; /* RCW lane powerdown bit */
	int bank;
} lanes[SRDS_MAX_LANES] = {
	{ 0, 152, FSL_SRDS_BANK_1 },
	{ 1, 153, FSL_SRDS_BANK_1 },
	{ 2, 154, FSL_SRDS_BANK_1 },
	{ 3, 155, FSL_SRDS_BANK_1 },
	{ 4, 156, FSL_SRDS_BANK_1 },
	{ 5, 157, FSL_SRDS_BANK_1 },
	{ 6, 158, FSL_SRDS_BANK_1 },
	{ 7, 159, FSL_SRDS_BANK_1 },
	{ 8, 160, FSL_SRDS_BANK_1 },
	{ 9, 161, FSL_SRDS_BANK_1 },
	{ 16, 162, FSL_SRDS_BANK_2 },
	{ 17, 163, FSL_SRDS_BANK_2 },
	{ 18, 164, FSL_SRDS_BANK_2 },
	{ 19, 165, FSL_SRDS_BANK_2 },
	{ 20, 170, FSL_SRDS_BANK_3 },
	{ 21, 171, FSL_SRDS_BANK_3 },
	{ 22, 172, FSL_SRDS_BANK_3 },
	{ 23, 173, FSL_SRDS_BANK_3 },
};

int serdes_get_lane_idx(int lane)
{
	return lanes[lane].idx;
}

int serdes_get_bank(int lane)
{
	return lanes[lane].bank;
}

int serdes_lane_enabled(int lane)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes_corenet_t *regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;

	int bank = lanes[lane].bank;
	int word = lanes[lane].lpd / 32;
	int bit = lanes[lane].lpd % 32;

	if (in_be32(&regs->bank[bank].rstctl) & SRDS_RSTCTL_SDPD)
		return 0;

	return !(in_be32(&gur->rcwsr[word]) & (0x80000000 >> bit));
}

int is_serdes_configured(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* Is serdes enabled at all? */
	if (!(in_be32(&gur->rcwsr[5]) & FSL_CORENET_RCWSR5_SRDS_EN))
		return 0;

	return (1 << device) & serdes_prtcl_map;
}

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int cfg;
	serdes_corenet_t *srds_regs;
	int lane, bank, idx;
	enum srds_prtcl lane_prtcl;
	long long end_tick;
	int have_bank[SRDS_MAX_BANK] = {};

	/* Is serdes enabled at all? */
	if (!(in_be32(&gur->rcwsr[5]) & FSL_CORENET_RCWSR5_SRDS_EN))
		return;

	srds_regs = (void *)(CONFIG_SYS_FSL_CORENET_SERDES_ADDR);
	cfg = (in_be32(&gur->rcwsr[4]) & FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;
	debug("Using SERDES configuration 0x%x, lane settings:\n", cfg);

	if (!is_serdes_prtcl_valid(cfg)) {
		printf("SERDES[PRTCL] = 0x%x is not valid\n", cfg);
		return;
	}

	/* Look for banks with all lanes disabled, and power down the bank. */
	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(cfg, lane);
		if (serdes_lane_enabled(lane)) {
			have_bank[serdes_get_bank(lane)] = 1;
			serdes_prtcl_map |= (1 << lane_prtcl);
		}
	}

	for (bank = 0; bank < SRDS_MAX_BANK; bank++) {
		if (!have_bank[bank]) {
			printf("SERDES: bank %d disabled\n", bank + 1);
			setbits_be32(&srds_regs->bank[bank].rstctl,
				     SRDS_RSTCTL_SDPD);
		}
	}

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		idx = serdes_get_lane_idx(lane);
		lane_prtcl = serdes_get_prtcl(cfg, lane);

#ifdef DEBUG
		switch (lane) {
		case 0:
			puts("Bank1: ");
			break;
		case 10:
			puts("\nBank2: ");
			break;
		case 14:
			puts("\nBank3: ");
			break;
		default:
			break;
		}

		printf("%s ", serdes_prtcl_str[lane_prtcl]);
#endif
	}

#ifdef DEBUG
	puts("\n");
#endif

	for (idx = 0; idx < SRDS_MAX_BANK; idx++) {
		u32 rstctl;

		bank = idx;

		/* Skip disabled banks */
		if (!have_bank[bank])
			continue;

		/* reset banks for errata */
		setbits_be32(&srds_regs->bank[bank].rstctl, SRDS_RSTCTL_RST);

		/* wait for reset complete or 1-second timeout */
		end_tick = usec2ticks(1000000) + get_ticks();
		do {
			rstctl = in_be32(&srds_regs->bank[bank].rstctl);
			if (rstctl & SRDS_RSTCTL_RSTDONE)
				break;
		} while (end_tick > get_ticks());

		if (!(rstctl & SRDS_RSTCTL_RSTDONE)) {
			printf("SERDES: timeout resetting bank %d\n",
			       bank + 1);
			continue;
		}
	}
}
