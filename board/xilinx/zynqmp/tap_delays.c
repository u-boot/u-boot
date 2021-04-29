// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx ZynqMP SoC Tap Delay Programming
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <common.h>
#include <zynqmp_tap_delay.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>
#include <mmc.h>
#include <zynqmp_firmware.h>

int arasan_zynqmp_set_in_tapdelay(u8 deviceid, u32 type, u32 itap_delay)
{

	return xilinx_pm_request(PM_IOCTL, (u32)deviceid, IOCTL_SET_SD_TAPDELAY,
			  type, itap_delay, NULL);
}

int arasan_zynqmp_set_out_tapdelay(u8 deviceid, u32 type, u32 otap_delay)
{
	return xilinx_pm_request(PM_IOCTL, (u32)deviceid, IOCTL_SET_SD_TAPDELAY,
			  type, otap_delay, NULL);
}
