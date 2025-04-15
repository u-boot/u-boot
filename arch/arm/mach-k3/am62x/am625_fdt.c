// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 */

#include <asm/hardware.h>
#include <fdt_support.h>
#include <fdtdec.h>

#include "../common_fdt.h"

static void fdt_fixup_cores_nodes_am625(void *blob, int core_nr)
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

static void fdt_fixup_gpu_nodes_am625(void *blob, int has_gpu)
{
	if (!has_gpu) {
		fdt_del_node_path(blob, "/bus@f0000/gpu@fd00000");
		fdt_del_node_path(blob, "/bus@f0000/watchdog@e0f0000");
	}
}

static void fdt_fixup_pru_node_am625(void *blob, int has_pru)
{
	if (!has_pru)
		fdt_del_node_path(blob, "/bus@f0000/pruss@30040000");
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

static void fdt_fixup_thermal_zone_nodes_am625(void *blob, int maxc)
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

static void fdt_fixup_thermal_cooling_device_cpus_am625(void *blob, int core_nr)
{
	static const char * const thermal_path[] = {
		"/thermal-zones/main0-thermal/cooling-maps/map0",
		"/thermal-zones/main1-thermal/cooling-maps/map0"
	};

	int node, cnt, i, ret;
	u32 cooling_dev[12];

	for (i = 0; i < ARRAY_SIZE(thermal_path); i++) {
		int new_count = core_nr * 3;  /* Each CPU has 3 entries */
		int j;

		node = fdt_path_offset(blob, thermal_path[i]);
		if (node < 0)
			continue; /* Not found, skip it */

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
	fdt_fixup_cores_nodes_am625(blob, k3_get_core_nr());
	fdt_fixup_gpu_nodes_am625(blob, k3_has_gpu());
	fdt_fixup_pru_node_am625(blob, k3_has_pru());
	fdt_fixup_thermal_zone_nodes_am625(blob, k3_get_max_temp());
	fdt_fixup_thermal_cooling_device_cpus_am625(blob, k3_get_core_nr());
	fdt_fixup_reserved(blob, "tfa", CONFIG_K3_ATF_LOAD_ADDR, 0x80000);
	fdt_fixup_reserved(blob, "optee", CONFIG_K3_OPTEE_LOAD_ADDR, 0x1800000);

	return 0;
}
