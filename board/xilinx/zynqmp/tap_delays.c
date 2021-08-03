// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx ZynqMP SoC Tap Delay Programming
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <common.h>
#include <zynqmp_tap_delay.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <linux/delay.h>
#include <mmc.h>
#include <zynqmp_firmware.h>

#define SD_DLL_CTRL			0xFF180358
#define SD_ITAP_DLY			0xFF180314
#define SD_OTAP_DLY			0xFF180318
#define SD0_DLL_RST			BIT(2)
#define SD1_DLL_RST			BIT(18)
#define SD0_ITAPCHGWIN			BIT(9)
#define SD1_ITAPCHGWIN			BIT(25)
#define SD0_ITAPDLYENA			BIT(8)
#define SD1_ITAPDLYENA			BIT(24)
#define SD0_ITAPDLYSEL_MASK		GENMASK(7, 0)
#define SD1_ITAPDLYSEL_MASK		GENMASK(23, 16)
#define SD0_OTAPDLYSEL_MASK		GENMASK(5, 0)
#define SD1_OTAPDLYSEL_MASK		GENMASK(21, 16)

int zynqmp_dll_reset(u8 node_id, u32 type)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
		if (node_id == NODE_SD_0)
			return zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST,
						 type == PM_DLL_RESET_ASSERT ?
						 SD0_DLL_RST : 0);

		return zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST,
					 type == PM_DLL_RESET_ASSERT ?
					 SD1_DLL_RST : 0);
	} else {
		return xilinx_pm_request(PM_IOCTL, (u32)node_id,
					 IOCTL_SD_DLL_RESET, type, 0, NULL);
	}
}

int arasan_zynqmp_set_in_tapdelay(u8 node_id, u32 itap_delay)
{
	int ret;

	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
		if (node_id == NODE_SD_0) {
			ret = zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN,
						SD0_ITAPCHGWIN);
			if (ret)
				return ret;

			ret = zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYENA,
						SD0_ITAPDLYENA);
			if (ret)
				return ret;

			ret = zynqmp_mmio_write(SD_ITAP_DLY,
						SD0_ITAPDLYSEL_MASK,
						itap_delay);
			if (ret)
				return ret;

			ret = zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN, 0);
			if (ret)
				return ret;
		}
		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN,
					SD1_ITAPCHGWIN);
		if (ret)
			return ret;

		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYENA,
					SD1_ITAPDLYENA);
		if (ret)
			return ret;

		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYSEL_MASK,
					(itap_delay << 16));
		if (ret)
			return ret;

		ret = zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN, 0);
		if (ret)
			return ret;
	} else {
		return xilinx_pm_request(PM_IOCTL, (u32)node_id,
					 IOCTL_SET_SD_TAPDELAY,
					 PM_TAPDELAY_INPUT, itap_delay, NULL);
	}

	return 0;
}

int arasan_zynqmp_set_out_tapdelay(u8 node_id, u32 otap_delay)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
		if (node_id == NODE_SD_0)
			return zynqmp_mmio_write(SD_OTAP_DLY,
						 SD0_OTAPDLYSEL_MASK,
						 otap_delay);

		return zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					 (otap_delay << 16));
	} else {
		return xilinx_pm_request(PM_IOCTL, (u32)node_id,
					 IOCTL_SET_SD_TAPDELAY,
					 PM_TAPDELAY_OUTPUT, otap_delay, NULL);
	}
}
