// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_init.h"

/* Design Guidelines parameters */
u32 g_zpri_data = 123;		/* controller data - P drive strength */
u32 g_znri_data = 123;		/* controller data - N drive strength */
u32 g_zpri_ctrl = 74;		/* controller C/A - P drive strength */
u32 g_znri_ctrl = 74;		/* controller C/A - N drive strength */
u32 g_zpodt_data = 45;		/* controller data - P ODT */
u32 g_znodt_data = 45;		/* controller data - N ODT */
u32 g_zpodt_ctrl = 45;		/* controller data - P ODT */
u32 g_znodt_ctrl = 45;		/* controller data - N ODT */
u32 g_odt_config_2cs = 0x120012;
u32 g_odt_config_1cs = 0x10000;
u32 g_rtt_nom = 0x44;
u32 g_dic = 0x2;


/*
 * Configure phy (called by static init controller) for static flow
 */
int ddr3_tip_configure_phy(u32 dev_num)
{
	u32 if_id, phy_id;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      PAD_ZRI_CALIB_PHY_REG,
		      ((0x7f & g_zpri_data) << 7 | (0x7f & g_znri_data))));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		      PAD_ZRI_CALIB_PHY_REG,
		      ((0x7f & g_zpri_ctrl) << 7 | (0x7f & g_znri_ctrl))));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      PAD_ODT_CALIB_PHY_REG,
		      ((0x3f & g_zpodt_data) << 6 | (0x3f & g_znodt_data))));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		      PAD_ODT_CALIB_PHY_REG,
		      ((0x3f & g_zpodt_ctrl) << 6 | (0x3f & g_znodt_ctrl))));

	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      PAD_PRE_DISABLE_PHY_REG, 0));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      CMOS_CONFIG_PHY_REG, 0));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		      CMOS_CONFIG_PHY_REG, 0));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		/* check if the interface is enabled */
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);

		for (phy_id = 0;
		     phy_id < tm->num_of_bus_per_interface;
		     phy_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, phy_id);
			/* Vref & clamp */
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, phy_id, DDR_PHY_DATA,
				      PAD_CONFIG_PHY_REG,
				      ((clamp_tbl[if_id] << 4) | vref),
				      ((0x7 << 4) | 0x7)));
			/* clamp not relevant for control */
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, phy_id, DDR_PHY_CONTROL,
				      PAD_CONFIG_PHY_REG, 0x4, 0x7));
		}
	}

	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA, 0x90,
		      0x6002));

	return MV_OK;
}
