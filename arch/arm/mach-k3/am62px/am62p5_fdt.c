// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/hardware.h>
#include "../common_fdt.h"
#include <fdt_support.h>

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

static int fdt_fixup_trips_node(void *blob, int zoneoffset, int maxc)
{
	int node, trip;

	node = fdt_subnode_offset(blob, zoneoffset, "trips");
	if (node < 0)
		return -1;

	fdt_for_each_subnode(trip, blob, node) {
		const char *type = fdt_getprop(blob, trip, "type", NULL);

		if (!type || (strncmp(type, "critical", 8) != 0))
			continue;

		if (fdt_setprop_u32(blob, trip, "temperature", 1000 * maxc) < 0)
			return -1;
	}

	return 0;
}

static void fdt_fixup_thermal_zone_nodes_am62p(void *blob, int maxc)
{
	int node, zone;

	node = fdt_path_offset(blob, "/thermal-zones");
	if (node < 0)
		return;

	fdt_for_each_subnode(zone, blob, node) {
		if (fdt_fixup_trips_node(blob, zone, maxc) < 0)
			printf("Failed to set temperature in %s critical trips\n",
			       fdt_get_name(blob, zone, NULL));
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

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_cores_wdt_nodes_am62p(blob, k3_get_core_nr());
	fdt_fixup_video_codec_nodes_am62p(blob, k3_has_video_codec());
	fdt_fixup_canfd_nodes_am62p(blob, k3_has_canfd());
	fdt_fixup_thermal_zone_nodes_am62p(blob, k3_get_max_temp());
	fdt_fixup_cpu_freq_nodes_am62p(blob, k3_get_a53_max_frequency());
	fdt_fixup_reserved(blob, "tfa", CONFIG_K3_ATF_LOAD_ADDR, 0x80000);
	fdt_fixup_reserved(blob, "optee", CONFIG_K3_OPTEE_LOAD_ADDR, 0x1800000);

	return 0;
}
