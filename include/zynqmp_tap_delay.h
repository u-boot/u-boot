/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Xilinx ZynqMP SoC Tap Delay Programming
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#ifndef __ZYNQMP_TAP_DELAY_H__
#define __ZYNQMP_TAP_DELAY_H__

#include <zynqmp_firmware.h>

#ifdef CONFIG_ARCH_ZYNQMP
int zynqmp_dll_reset(u8 node_id, u32 type);
int arasan_zynqmp_set_in_tapdelay(u8 device_id, u32 itap_delay);
int arasan_zynqmp_set_out_tapdelay(u8 device_id, u32 otap_delay);
#else
inline int zynqmp_dll_reset(u8 deviceid, u32 type)
{
	return 0;
}

inline int arasan_zynqmp_set_in_tapdelay(u8 device_id, u32 itap_delay)
{
	return 0;
}

inline int arasan_zynqmp_set_out_tapdelay(u8 device_id, u32 otap_delay)
{
	return 0;
}
#endif

#endif
