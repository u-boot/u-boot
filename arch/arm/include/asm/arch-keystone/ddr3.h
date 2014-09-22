/*
 * DDR3
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _DDR3_H_
#define _DDR3_H_

#include <asm/arch/hardware.h>

struct ddr3_phy_config {
	unsigned int pllcr;
	unsigned int pgcr1_mask;
	unsigned int pgcr1_val;
	unsigned int ptr0;
	unsigned int ptr1;
	unsigned int ptr2;
	unsigned int ptr3;
	unsigned int ptr4;
	unsigned int dcr_mask;
	unsigned int dcr_val;
	unsigned int dtpr0;
	unsigned int dtpr1;
	unsigned int dtpr2;
	unsigned int mr0;
	unsigned int mr1;
	unsigned int mr2;
	unsigned int dtcr;
	unsigned int pgcr2;
	unsigned int zq0cr1;
	unsigned int zq1cr1;
	unsigned int zq2cr1;
	unsigned int pir_v1;
	unsigned int pir_v2;
};

struct ddr3_emif_config {
	unsigned int sdcfg;
	unsigned int sdtim1;
	unsigned int sdtim2;
	unsigned int sdtim3;
	unsigned int sdtim4;
	unsigned int zqcfg;
	unsigned int sdrfc;
};

void ddr3_init(void);
void ddr3_reset_ddrphy(void);
void ddr3_err_reset_workaround(void);
void ddr3_init_ddrphy(u32 base, struct ddr3_phy_config *phy_cfg);
void ddr3_init_ddremif(u32 base, struct ddr3_emif_config *emif_cfg);

#endif
