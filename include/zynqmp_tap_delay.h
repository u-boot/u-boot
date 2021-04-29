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
int arasan_zynqmp_set_in_tapdelay(u8 device_id, u32 type, u32 itap_delay);
int arasan_zynqmp_set_out_tapdelay(u8 device_id, u32 type, u32 otap_delay);
#else
inline int arasan_zynqmp_set_in_tapdelay(u8 device_id, u32 type, u32 itap_delay)
{
	return 0;
}

int arasan_zynqmp_set_out_tapdelay(u8 device_id, u32 type, u32 otap_delay)
{
	return 0;
}
#endif

static inline int zynqmp_pm_sd_dll_reset(u32 node_id, u32 type)
{
	return xilinx_pm_request(PM_IOCTL, node_id, IOCTL_SD_DLL_RESET,
				 type, 0, NULL);
}

#endif
