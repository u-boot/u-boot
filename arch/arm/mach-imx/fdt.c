// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 NXP
 */

#include <env.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/arch/sys_proto.h>

static void disable_thermal_cpu_nodes(void *blob)
{
	static const char * const thermal_path[] = {
		"/thermal-zones/cpu-thermal/cooling-maps/map0",
		"/thermal-zones/soc-thermal/cooling-maps/map0"
	};

	int nodeoff, len, i, ret, j, k;
	const fdt32_t *prop;
	fdt32_t *cooling_dev;

	for (i = 0; i < ARRAY_SIZE(thermal_path); i++) {
		nodeoff = fdt_path_offset(blob, thermal_path[i]);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		prop = fdt_getprop(blob, nodeoff, "cooling-device", &len);
		if (!prop || len % (3 * sizeof(fdt32_t)))
			continue;

		cooling_dev = malloc(len);
		if (!cooling_dev) {
			printf("failed to alloc cooling dev\n");
			return;
		}

		/* Drop the entries referring to a deleted node */
		for (j = 0, k = 0; j < len / sizeof(fdt32_t); j += 3) {
			if (fdt_node_offset_by_phandle(blob, fdt32_to_cpu(prop[j])) < 0)
				continue;
			cooling_dev[k++] = prop[j];
			cooling_dev[k++] = prop[j + 1];
			cooling_dev[k++] = prop[j + 2];
		}

		if (k * sizeof(fdt32_t) != len) {
			ret = fdt_setprop(blob, nodeoff, "cooling-device", cooling_dev,
					  k * sizeof(fdt32_t));
			if (ret < 0)
				printf("Warning: %s, cooling-device setprop failed %d\n",
				       thermal_path[i], ret);
			else
				printf("Update node %s, cooling-device prop\n", thermal_path[i]);
		}

		free(cooling_dev);
	}
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

	disable_thermal_cpu_nodes(blob);

	return 0;
}

int fixup_thermal_trips(void *blob, const char *name)
{
	int minc, maxc;
	int node, trip;

	/*
	 * During development or various dangerous experiments, it may
	 * be necessary to override the trip points. Allow users to do
	 * that. However, do keep in mind that this may damage the SoC.
	 */
	if (CONFIG_IS_ENABLED(ENV_SUPPORT))
		if (env_get("imx_skip_fixup_thermal_trips"))
			return 0;

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
