// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/arch/k3-common-fdt.h>
#include <asm/hardware.h>
#include <fdt_support.h>
#include <fdtdec.h>

static void fdt_fixup_cores_wdt_nodes_am62p(void *blob, int core_nr)
{
	char node_path[32];

	if (core_nr < 1)
		return;

	for (; core_nr < 4; core_nr++) {
		snprintf(node_path, sizeof(node_path), "/cpus/cpu@%d", core_nr);
		fdt_del_node_path(blob, node_path);
		snprintf(node_path, sizeof(node_path), "/cpus/cpu-map/cluster0/core%d", core_nr);
		fdt_del_node_path(blob, node_path);
		snprintf(node_path, sizeof(node_path), "/bus@f0000/watchdog@e0%d0000", core_nr);
		fdt_del_node_path(blob, node_path);
	}
}

static void fdt_fixup_video_codec_nodes_am62p(void *blob, bool has_video_codec)
{
	if (!has_video_codec)
		fdt_del_node_path(blob, "/bus@f0000/video-codec@30210000");
}

static void fdt_fixup_canfd_nodes_am62p(void *blob, bool has_canfd)
{
	if (!has_canfd) {
		fdt_del_node_path(blob, "/bus@f0000/can@20701000");
		fdt_del_node_path(blob, "/bus@f0000/can@20711000");
	}
}

static void fdt_fixup_cpu_freq_nodes_am62p(void *blob, int max_freq)
{
	if (max_freq >= 1250000000)
		return;

	if (max_freq <= 1000000000) {
		fdt_del_node_path(blob, "/opp-table/opp-1250000000");
		fdt_del_node_path(blob, "/opp-table/opp-1400000000");
	}
}

static void fdt_fixup_thermal_cooling_device_cpus_am62p(void *blob, int core_nr)
{
	static const char * const thermal_path[] = {
		"/thermal-zones/main0-thermal/cooling-maps/map0",
		"/thermal-zones/main1-thermal/cooling-maps/map0",
		"/thermal-zones/main2-thermal/cooling-maps/map0"
	};

	int node, cnt, i, ret;
	u32 cooling_dev[12];

	for (i = 0; i < ARRAY_SIZE(thermal_path); i++) {
		int new_count = core_nr * 3;  /* Each CPU has 3 entries */
		int j;

		node = fdt_path_offset(blob, thermal_path[i]);
		if (node < 0)
			continue;

		cnt = fdtdec_get_int_array_count(blob, node, "cooling-device",
						 cooling_dev, ARRAY_SIZE(cooling_dev));
		if (cnt < 0)
			continue;

		for (j = 0; j < new_count; j++)
			cooling_dev[j] = cpu_to_fdt32(cooling_dev[j]);

		ret = fdt_setprop(blob, node, "cooling-device", cooling_dev,
				  new_count * sizeof(u32));
		if (ret < 0)
			printf("Error %s, cooling-device setprop failed %d\n",
			       thermal_path[i], ret);
	}
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_cores_wdt_nodes_am62p(blob, k3_get_core_nr());
	fdt_fixup_video_codec_nodes_am62p(blob, k3_has_video_codec());
	fdt_fixup_canfd_nodes_am62p(blob, k3_has_canfd());
	fdt_fixup_thermal_critical_trips_k3(blob, k3_get_max_temp());
	fdt_fixup_thermal_cooling_device_cpus_am62p(blob, k3_get_core_nr());
	fdt_fixup_cpu_freq_nodes_am62p(blob, k3_get_a53_max_frequency());
	fdt_fixup_reserved(blob, "tfa", CONFIG_K3_ATF_LOAD_ADDR, 0x80000);
	fdt_fixup_reserved(blob, "optee", CONFIG_K3_OPTEE_LOAD_ADDR, 0x1800000);

	return 0;
}
