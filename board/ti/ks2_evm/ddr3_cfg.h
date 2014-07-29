/*
 * Keystone2: DDR3 configuration
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __DDR3_CFG_H
#define __DDR3_CFG_H

extern struct ddr3_phy_config ddr3phy_1600_8g;
extern struct ddr3_emif_config ddr3_1600_8g;

extern struct ddr3_phy_config ddr3phy_1333_2g;
extern struct ddr3_emif_config ddr3_1333_2g;

extern struct ddr3_phy_config ddr3phy_1600_4g;
extern struct ddr3_emif_config ddr3_1600_4g;

int ddr3_get_dimm_params(char *dimm_name);

#endif /* __DDR3_CFG_H */
