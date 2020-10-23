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

#define SD_DLL_CTRL			0xFF180358
#define SD_ITAP_DLY			0xFF180314
#define SD_OTAP_DLY			0xFF180318
#define SD0_DLL_RST_MASK		0x00000004
#define SD0_DLL_RST			0x00000004
#define SD1_DLL_RST_MASK		0x00040000
#define SD1_DLL_RST			0x00040000
#define SD0_ITAPCHGWIN_MASK		0x00000200
#define SD0_ITAPCHGWIN			0x00000200
#define SD1_ITAPCHGWIN_MASK		0x02000000
#define SD1_ITAPCHGWIN			0x02000000
#define SD0_ITAPDLYENA_MASK		0x00000100
#define SD0_ITAPDLYENA			0x00000100
#define SD1_ITAPDLYENA_MASK		0x01000000
#define SD1_ITAPDLYENA			0x01000000
#define SD0_ITAPDLYSEL_MASK		0x000000FF
#define SD1_ITAPDLYSEL_MASK		0x00FF0000
#define SD0_OTAPDLYSEL_MASK		0x0000003F
#define SD1_OTAPDLYSEL_MASK		0x003F0000

void zynqmp_dll_reset(u8 deviceid)
{
	/* Issue DLL Reset */
	if (deviceid == 0)
		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK,
				  SD0_DLL_RST);
	else
		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK,
				  SD1_DLL_RST);

	mdelay(1);

	/* Release DLL Reset */
	if (deviceid == 0)
		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK, 0x0);
	else
		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK, 0x0);
}

void arasan_zynqmp_set_tapdelay(u8 deviceid, u32 itap_delay, u32 otap_delay)
{
	if (deviceid == 0) {
		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK,
				  SD0_DLL_RST);
		/* Program ITAP */
		if (itap_delay) {
			zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN_MASK,
					  SD0_ITAPCHGWIN);
			zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYENA_MASK,
					  SD0_ITAPDLYENA);
			zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYSEL_MASK,
					  itap_delay);
			zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN_MASK,
					  0x0);
		}

		/* Program OTAP */
		if (otap_delay)
			zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
					  otap_delay);

		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK, 0x0);
	} else {
		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK,
				  SD1_DLL_RST);
		/* Program ITAP */
		if (itap_delay) {
			zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN_MASK,
					  SD1_ITAPCHGWIN);
			zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYENA_MASK,
					  SD1_ITAPDLYENA);
			zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYSEL_MASK,
					  (itap_delay << 16));
			zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN_MASK,
					  0x0);
		}

		/* Program OTAP */
		if (otap_delay)
			zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					  (otap_delay << 16));

		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK, 0x0);
	}
}
