// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 */

#include <asm/arch/k3-common-fdt.h>
#include "common.h"
#include <dm.h>
#include <fdt_support.h>
#include <linux/soc/ti/ti_sci_protocol.h>

static int fdt_fixup_msmc_ram(void *blob, char *parent_path, char *node_name)
{
	u64 msmc_start = 0, msmc_end = 0, msmc_size, reg[2];
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	int ret, node, subnode, len, prev_node;
	u32 range[4], addr, size;
	const fdt32_t *sub_reg;

	ti_sci->ops.core_ops.query_msmc(ti_sci, &msmc_start, &msmc_end);
	msmc_size = msmc_end - msmc_start + 1;
	debug("%s: msmc_start = 0x%llx, msmc_size = 0x%llx\n", __func__,
	      msmc_start, msmc_size);

	/* find or create "msmc_sram node */
	ret = fdt_path_offset(blob, parent_path);
	if (ret < 0)
		return ret;

	node = fdt_find_or_add_subnode(blob, ret, node_name);
	if (node < 0)
		return node;

	ret = fdt_setprop_string(blob, node, "compatible", "mmio-sram");
	if (ret < 0)
		return ret;

	reg[0] = cpu_to_fdt64(msmc_start);
	reg[1] = cpu_to_fdt64(msmc_size);
	ret = fdt_setprop(blob, node, "reg", reg, sizeof(reg));
	if (ret < 0)
		return ret;

	fdt_setprop_cell(blob, node, "#address-cells", 1);
	fdt_setprop_cell(blob, node, "#size-cells", 1);

	range[0] = 0;
	range[1] = cpu_to_fdt32(msmc_start >> 32);
	range[2] = cpu_to_fdt32(msmc_start & 0xffffffff);
	range[3] = cpu_to_fdt32(msmc_size);
	ret = fdt_setprop(blob, node, "ranges", range, sizeof(range));
	if (ret < 0)
		return ret;

	subnode = fdt_first_subnode(blob, node);
	prev_node = 0;

	/* Look for invalid subnodes and delete them */
	while (subnode >= 0) {
		sub_reg = fdt_getprop(blob, subnode, "reg", &len);
		addr = fdt_read_number(sub_reg, 1);
		sub_reg++;
		size = fdt_read_number(sub_reg, 1);
		debug("%s: subnode = %d, addr = 0x%x. size = 0x%x\n", __func__,
		      subnode, addr, size);
		if (addr + size > msmc_size ||
		    !strncmp(fdt_get_name(blob, subnode, &len), "sysfw", 5) ||
		    !strncmp(fdt_get_name(blob, subnode, &len), "tifs", 4)  ||
		    !strncmp(fdt_get_name(blob, subnode, &len), "l3cache", 7)) {
			fdt_del_node(blob, subnode);
			debug("%s: deleting subnode %d\n", __func__, subnode);
			if (!prev_node)
				subnode = fdt_first_subnode(blob, node);
			else
				subnode = fdt_next_subnode(blob, prev_node);
		} else {
			prev_node = subnode;
			subnode = fdt_next_subnode(blob, prev_node);
		}
	}

	return 0;
}

int fdt_fixup_msmc_ram_k3(void *blob)
{
	int ret;

	ret = fdt_fixup_msmc_ram(blob, "/bus@100000", "sram@70000000");
	if (ret < 0)
		ret = fdt_fixup_msmc_ram(blob, "/interconnect@100000",
					 "sram@70000000");
	if (ret)
		printf("%s: fixing up msmc ram failed %d\n", __func__, ret);

	return ret;
}

int fdt_del_node_path(void *blob, const char *path)
{
	int ret;
	int nodeoff;

	nodeoff = fdt_path_offset(blob, path);
	if (nodeoff < 0)
		return 0; /* Not found, skip it */

	ret = fdt_del_node(blob, nodeoff);
	if (ret < 0)
		printf("Unable to delete node %s, err=%s\n", path, fdt_strerror(ret));
	else
		debug("Deleted node %s\n", path);

	return ret;
}

int fdt_fixup_reserved(void *blob, const char *name,
		       unsigned int new_address, unsigned int new_size)
{
	int nodeoffset, subnode;
	int ret;
	struct fdt_memory carveout = {
		.start = new_address,
	};

	/* Find reserved-memory */
	nodeoffset = fdt_subnode_offset(blob, 0, "reserved-memory");
	if (nodeoffset < 0)
		goto add_carveout;

	/* Find existing matching subnode and remove it */
	fdt_for_each_subnode(subnode, blob, nodeoffset) {
		const char *node_name;
		fdt_addr_t addr;
		fdt_size_t size;

		/* Name matching */
		node_name = fdt_get_name(blob, subnode, NULL);
		if (!name)
			return -EINVAL;
		if (!strncmp(node_name, name, strlen(name))) {
			/* Read out old size first */
			addr = fdtdec_get_addr_size(blob, subnode, "reg", &size);
			if (addr == FDT_ADDR_T_NONE)
				return -EINVAL;
			new_size = size;

			/* Delete node */
			ret = fdt_del_node(blob, subnode);
			if (ret < 0)
				return ret;

			/* Only one matching node */
			break;
		}
	}

add_carveout:
	carveout.end = new_address + new_size - 1;
	ret = fdtdec_add_reserved_memory(blob, name, &carveout, NULL, 0, NULL,
					 FDTDEC_RESERVED_MEMORY_NO_MAP);
	if (ret < 0)
		return ret;

	return 0;
}

static int fdt_fixup_critical_trips(void *blob, int zoneoffset, int maxc)
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

void fdt_fixup_thermal_critical_trips_k3(void *blob, int maxc)
{
	int node, zone;

	node = fdt_path_offset(blob, "/thermal-zones");
	if (node < 0)
		return;

	fdt_for_each_subnode(zone, blob, node) {
		if (fdt_fixup_critical_trips(blob, zone, maxc) < 0)
			printf("Failed to set temperature in %s critical trips\n",
			       fdt_get_name(blob, zone, NULL));
	}
}
