/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include "ddr3_cfg.h"
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>

static int ddr3_size;

struct pll_init_data ddr3a_333 = DDR3_PLL_333(A);
struct pll_init_data ddr3a_400 = DDR3_PLL_400(A);

void ddr3_init(void)
{
	char dimm_name[32];

	ddr3_get_dimm_params(dimm_name);

	printf("Detected SO-DIMM [%s]\n", dimm_name);

	if (!strcmp(dimm_name, "18KSF1G72HZ-1G6E2 ")) {
		init_pll(&ddr3a_400);
		if (cpu_revision() > 0) {
			if (cpu_revision() > 1) {
				/* PG 2.0 */
				/* Reset DDR3A PHY after PLL enabled */
				ddr3_reset_ddrphy();
				ddr3phy_1600_8g.zq0cr1 |= 0x10000;
				ddr3phy_1600_8g.zq1cr1 |= 0x10000;
				ddr3phy_1600_8g.zq2cr1 |= 0x10000;
				ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC,
						 &ddr3phy_1600_8g);
			} else {
				/* PG 1.1 */
				ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC,
						 &ddr3phy_1600_8g);
			}

			ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE,
					  &ddr3_1600_8g);
			printf("DRAM:  Capacity 8 GiB (includes reported below)\n");
			ddr3_size = 8;
		} else {
			ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_1600_8g);
			ddr3_1600_8g.sdcfg |= 0x1000;
			ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE,
					  &ddr3_1600_8g);
			printf("DRAM:  Capacity 4 GiB (includes reported below)\n");
			ddr3_size = 4;
		}
	} else if (!strcmp(dimm_name, "SQR-SD3T-2G1333SED")) {
		init_pll(&ddr3a_333);
		if (cpu_revision() > 0) {
			if (cpu_revision() > 1) {
				/* PG 2.0 */
				/* Reset DDR3A PHY after PLL enabled */
				ddr3_reset_ddrphy();
				ddr3phy_1333_2g.zq0cr1 |= 0x10000;
				ddr3phy_1333_2g.zq1cr1 |= 0x10000;
				ddr3phy_1333_2g.zq2cr1 |= 0x10000;
				ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC,
						 &ddr3phy_1333_2g);
			} else {
				/* PG 1.1 */
				ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC,
						 &ddr3phy_1333_2g);
			}
			ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE,
					  &ddr3_1333_2g);
			ddr3_size = 2;
			printf("DRAM:  2 GiB");
		} else {
			ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_1333_2g);
			ddr3_1333_2g.sdcfg |= 0x1000;
			ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE,
					  &ddr3_1333_2g);
			ddr3_size = 1;
			printf("DRAM:  1 GiB");
		}
	} else {
		printf("Unknown SO-DIMM. Cannot configure DDR3\n");
		while (1)
			;
	}

	/* Apply the workaround for PG 1.0 and 1.1 Silicons */
	if (cpu_revision() <= 1)
		ddr3_err_reset_workaround();
}

/**
 * ddr3_get_size - return ddr3 size in GiB
 */
int ddr3_get_size(void)
{
	return ddr3_size;
}
