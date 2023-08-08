/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef _MVEBU_THERMAL_H_
#define _MVEBU_THERMAL_H_

#include <dt-bindings/comphy/comphy_data.h>
#include <fdtdec.h>

struct thermal_unit_config {
	/* thermal temperature parameters */
	s32 tsen_offset;
	s32 tsen_gain;
	s32 tsen_divisor;
	/* thermal data */
	s32 tsen_ready;
	void __iomem *regs_base;
	/* thermal functionality */
	u32 (*ptr_thermal_sensor_probe)(struct thermal_unit_config *cfg);
	s32 (*ptr_thermal_sensor_read)(struct thermal_unit_config *cfg,
				       int *temp);
	u32 fw_smc_support;
};

/* Thermal sensors APIs */
s32 mvebu_thermal_sensor_read(struct thermal_unit_config *thermal_config,
			      int *temp);
u32 mvebu_thermal_sensor_probe(struct thermal_unit_config *thermal_config);

/* External Thermal sensors APIs */
s32 mvebu_thermal_ext_sensor_read(struct thermal_unit_config *thermal_config,
				  int *temp);
u32 mvebu_thermal_ext_sensor_probe(struct thermal_unit_config *thermal_config);

#endif /* _THERMAL_H_ */

