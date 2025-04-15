// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 NXP
 */

#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/arch/sys_proto.h>

static void disable_thermal_cpu_nodes(void *blob, u32 num_disabled_cores, u32 max_cores)
{
	static const char * const thermal_path[] = {
		"/thermal-zones/cpu-thermal/cooling-maps/map0"
	};

	int nodeoff, cnt, i, ret, j;
	u32 num_le32 = max_cores * 3;
	u32 *cooling_dev = (u32 *)malloc(num_le32 * sizeof(__le32));

	if (!cooling_dev) {
		printf("failed to alloc cooling dev\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(thermal_path); i++) {
		nodeoff = fdt_path_offset(blob, thermal_path[i]);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		cnt = fdtdec_get_int_array_count(blob, nodeoff, "cooling-device",
						 cooling_dev, num_le32);
		if (cnt < 0)
			continue;

		if (cnt != num_le32)
			printf("Warning: %s, cooling-device count %d\n", thermal_path[i], cnt);

		for (j = 0; j < cnt; j++)
			cooling_dev[j] = cpu_to_fdt32(cooling_dev[j]);

		ret = fdt_setprop(blob, nodeoff, "cooling-device", cooling_dev,
				  sizeof(__le32) * (num_le32 - num_disabled_cores * 3));
		if (ret < 0) {
			printf("Warning: %s, cooling-device setprop failed %d\n",
			       thermal_path[i], ret);
			continue;
		}

		printf("Update node %s, cooling-device prop\n", thermal_path[i]);
	}

	free(cooling_dev);
}

int disable_cpu_nodes(void *blob, const char * const *nodes_path, u32 num_disabled_cores,
		      u32 max_cores)
{
	u32 i = 0;
	int rc;
	int nodeoff;

	if (max_cores == 0 || (num_disabled_cores > (max_cores - 1)))
		return -EINVAL;

	i = max_cores - num_disabled_cores;

	for (; i < max_cores; i++) {
		nodeoff = fdt_path_offset(blob, nodes_path[i]);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		debug("Found %s node\n", nodes_path[i]);

		rc = fdt_del_node(blob, nodeoff);
		if (rc < 0) {
			printf("Unable to delete node %s, err=%s\n",
			       nodes_path[i], fdt_strerror(rc));
		} else {
			printf("Delete node %s\n", nodes_path[i]);
		}
	}

	disable_thermal_cpu_nodes(blob, num_disabled_cores, max_cores);

	return 0;
}

int fixup_thermal_trips(void *blob, const char *name)
{
	int minc, maxc;
	int node, trip;

	node = fdt_path_offset(blob, "/thermal-zones");
	if (node < 0)
		return node;

	node = fdt_subnode_offset(blob, node, name);
	if (node < 0)
		return node;

	node = fdt_subnode_offset(blob, node, "trips");
	if (node < 0)
		return node;

	get_cpu_temp_grade(&minc, &maxc);

	fdt_for_each_subnode(trip, blob, node) {
		const char *type;
		int temp, ret;

		type = fdt_getprop(blob, trip, "type", NULL);
		if (!type)
			continue;

		temp = 0;
		if (!strcmp(type, "critical"))
			temp = 1000 * maxc;
		else if (!strcmp(type, "passive"))
			temp = 1000 * (maxc - 10);
		if (temp) {
			ret = fdt_setprop_u32(blob, trip, "temperature", temp);
			if (ret)
				return ret;
		}
	}

	return 0;
}
