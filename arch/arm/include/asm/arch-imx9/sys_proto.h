/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 NXP
 */

#ifndef __ARCH_IMX9_SYS_PROTO_H
#define __ARCH_IMX9_SYS_PROTO_H

#include <asm/mach-imx/sys_proto.h>

enum imx9_soc_voltage_mode {
	VOLT_LOW_DRIVE = 0,
	VOLT_NOMINAL_DRIVE,
	VOLT_OVER_DRIVE,
};

void soc_power_init(void);
bool m33_is_rom_kicked(void);
int m33_prepare(void);

enum imx9_soc_voltage_mode soc_target_voltage_mode(void);

#define is_voltage_mode(mode) (soc_target_voltage_mode() == (mode))

#endif
