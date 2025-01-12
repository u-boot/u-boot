// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * NOTE: Please do not add new devicetree-reading functionality into this file.
 * Add it to the ofnode API instead, since that is compatible with livetree.
 */

#ifndef USE_HOSTCC

#define LOG_CATEGORY	LOGC_DT

#include <bloblist.h>
#include <boot_fit.h>
#include <display_options.h>
#include <dm.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <spl.h>
#include <env.h>
#include <errno.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <gzip.h>
#include <mapmem.h>
#include <linux/libfdt.h>
#include <serial.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/ofnode.h>
#include <dm/of_extra.h>
#include <linux/ctype.h>
#include <linux/lzo.h>
#include <linux/ioport.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Here are the type we know about. One day we might allow drivers to
 * register. For now we just put them here. The COMPAT macro allows us to
 * turn this into a sparse list later, and keeps the ID with the name.
 *
 * NOTE: This list is basically a TODO list for things that need to be
 * converted to driver model. So don't add new things here unless there is a
 * good reason why driver-model conversion is infeasible. Examples include
 * things which are used before driver model is available.
 */
#define COMPAT(id, name) name
static const char * const compat_names[COMPAT_COUNT] = {
	COMPAT(UNKNOWN, "<none>"),
	COMPAT(NVIDIA_TEGRA20_EMC, "nvidia,tegra20-emc"),
	COMPAT(NVIDIA_TEGRA20_EMC_TABLE, "nvidia,tegra20-emc-table"),
	COMPAT(NVIDIA_TEGRA20_NAND, "nvidia,tegra20-nand"),
	COMPAT(NVIDIA_TEGRA124_XUSB_PADCTL, "nvidia,tegra124-xusb-padctl"),
	COMPAT(NVIDIA_TEGRA210_XUSB_PADCTL, "nvidia,tegra210-xusb-padctl"),
	COMPAT(SAMSUNG_EXYNOS_USB_PHY, "samsung,exynos-usb-phy"),
	COMPAT(SAMSUNG_EXYNOS5_USB3_PHY, "samsung,exynos5250-usb3-phy"),
	COMPAT(SAMSUNG_EXYNOS_TMU, "samsung,exynos-tmu"),
	COMPAT(SAMSUNG_EXYNOS_MIPI_DSI, "samsung,exynos-mipi-dsi"),
	COMPAT(SAMSUNG_EXYNOS_DWMMC, "samsung,exynos-dwmmc"),
	COMPAT(GENERIC_SPI_FLASH, "jedec,spi-nor"),
	COMPAT(SAMSUNG_EXYNOS_SYSMMU, "samsung,sysmmu-v3.3"),
	COMPAT(INTEL_MICROCODE, "intel,microcode"),
	COMPAT(INTEL_QRK_MRC, "intel,quark-mrc"),
	COMPAT(ALTERA_SOCFPGA_DWMAC, "altr,socfpga-stmmac"),
	COMPAT(ALTERA_SOCFPGA_DWMMC, "altr,socfpga-dw-mshc"),
	COMPAT(ALTERA_SOCFPGA_DWC2USB, "snps,dwc2"),
	COMPAT(INTEL_BAYTRAIL_FSP, "intel,baytrail-fsp"),
	COMPAT(INTEL_BAYTRAIL_FSP_MDP, "intel,baytrail-fsp-mdp"),
	COMPAT(INTEL_IVYBRIDGE_FSP, "intel,ivybridge-fsp"),
	COMPAT(ALTERA_SOCFPGA_CLK, "altr,clk-mgr"),
	COMPAT(ALTERA_SOCFPGA_PINCTRL_SINGLE, "pinctrl-single"),
	COMPAT(ALTERA_SOCFPGA_H2F_BRG, "altr,socfpga-hps2fpga-bridge"),
	COMPAT(ALTERA_SOCFPGA_LWH2F_BRG, "altr,socfpga-lwhps2fpga-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2H_BRG, "altr,socfpga-fpga2hps-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2SDR0, "altr,socfpga-fpga2sdram0-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2SDR1, "altr,socfpga-fpga2sdram1-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2SDR2, "altr,socfpga-fpga2sdram2-bridge"),
	COMPAT(ALTERA_SOCFPGA_FPGA0, "altr,socfpga-a10-fpga-mgr"),
	COMPAT(ALTERA_SOCFPGA_NOC, "altr,socfpga-a10-noc"),
	COMPAT(ALTERA_SOCFPGA_CLK_INIT, "altr,socfpga-a10-clk-init")
};

static const char *const fdt_src_name[] = {
	[FDTSRC_SEPARATE] = "separate",
	[FDTSRC_FIT] = "fit",
	[FDTSRC_BOARD] = "board",
	[FDTSRC_EMBED] = "embed",
	[FDTSRC_ENV] = "env",
	[FDTSRC_BLOBLIST] = "bloblist",
};

extern u8 __dtb_dt_begin[];	/* embedded device tree blob */
extern u8 __dtb_dt_spl_begin[];	/* embedded device tree blob for SPL/TPL */

/* Get a pointer to the embedded devicetree, if there is one, else NULL */
static u8 *dtb_dt_embedded(void)
{
	u8 *addr = NULL;

	if (IS_ENABLED(CONFIG_OF_EMBED)) {
		addr = __dtb_dt_begin;

		if (IS_ENABLED(CONFIG_XPL_BUILD))
			addr = __dtb_dt_spl_begin;
	}
	return addr;
}

const char *fdtdec_get_srcname(void)
{
	return fdt_src_name[gd->fdt_src];
}

const char *fdtdec_get_compatible(enum fdt_compat_id id)
{
	/* We allow reading of the 'unknown' ID for testing purposes */
	assert(id >= 0 && id < COMPAT_COUNT);
	return compat_names[id];
}

fdt_addr_t fdtdec_get_addr_size_fixed(const void *blob, int node,
				      const char *prop_name, int index, int na,
				      int ns, fdt_size_t *sizep,
				      bool translate)
{
	const fdt32_t *prop, *prop_end;
	const fdt32_t *prop_addr, *prop_size, *prop_after_size;
	int len;
	fdt_addr_t addr;

	debug("%s: %s: ", __func__, prop_name);

	prop = fdt_getprop(blob, node, prop_name, &len);
	if (!prop) {
		debug("(not found)\n");
		return FDT_ADDR_T_NONE;
	}
	prop_end = prop + (len / sizeof(*prop));

	prop_addr = prop + (index * (na + ns));
	prop_size = prop_addr + na;
	prop_after_size = prop_size + ns;
	if (prop_after_size > prop_end) {
		debug("(not enough data: expected >= %d cells, got %d cells)\n",
		      (u32)(prop_after_size - prop), ((u32)(prop_end - prop)));
		return FDT_ADDR_T_NONE;
	}

#if CONFIG_IS_ENABLED(OF_TRANSLATE)
	if (translate)
		addr = fdt_translate_address(blob, node, prop_addr);
	else
#endif
		addr = fdtdec_get_number(prop_addr, na);

	if (sizep) {
		*sizep = fdtdec_get_number(prop_size, ns);
		debug("addr=%08llx, size=%llx\n", (unsigned long long)addr,
		      (unsigned long long)*sizep);
	} else {
		debug("addr=%08llx\n", (unsigned long long)addr);
	}

	return addr;
}

fdt_addr_t fdtdec_get_addr_size_auto_parent(const void *blob, int parent,
					    int node, const char *prop_name,
					    int index, fdt_size_t *sizep,
					    bool translate)
{
	int na, ns;

	debug("%s: ", __func__);

	na = fdt_address_cells(blob, parent);
	if (na < 1) {
		debug("(bad #address-cells)\n");
		return FDT_ADDR_T_NONE;
	}

	ns = fdt_size_cells(blob, parent);
	if (ns < 0) {
		debug("(bad #size-cells)\n");
		return FDT_ADDR_T_NONE;
	}

	debug("na=%d, ns=%d, ", na, ns);

	return fdtdec_get_addr_size_fixed(blob, node, prop_name, index, na,
					  ns, sizep, translate);
}

fdt_addr_t fdtdec_get_addr_size_auto_noparent(const void *blob, int node,
					      const char *prop_name, int index,
					      fdt_size_t *sizep,
					      bool translate)
{
	int parent;

	debug("%s: ", __func__);

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("(no parent found)\n");
		return FDT_ADDR_T_NONE;
	}

	return fdtdec_get_addr_size_auto_parent(blob, parent, node, prop_name,
						index, sizep, translate);
}

fdt_addr_t fdtdec_get_addr_size(const void *blob, int node,
				const char *prop_name, fdt_size_t *sizep)
{
	int ns = sizep ? (sizeof(fdt_size_t) / sizeof(fdt32_t)) : 0;

	return fdtdec_get_addr_size_fixed(blob, node, prop_name, 0,
					  sizeof(fdt_addr_t) / sizeof(fdt32_t),
					  ns, sizep, false);
}

fdt_addr_t fdtdec_get_addr(const void *blob, int node, const char *prop_name)
{
	return fdtdec_get_addr_size(blob, node, prop_name, NULL);
}

int fdtdec_get_pci_vendev(const void *blob, int node, u16 *vendor, u16 *device)
{
	const char *list, *end;
	int len;

	list = fdt_getprop(blob, node, "compatible", &len);
	if (!list)
		return -ENOENT;

	end = list + len;
	while (list < end) {
		len = strlen(list);
		if (len >= strlen("pciVVVV,DDDD")) {
			char *s = strstr(list, "pci");

			/*
			 * check if the string is something like pciVVVV,DDDD.RR
			 * or just pciVVVV,DDDD
			 */
			if (s && s[7] == ',' &&
			    (s[12] == '.' || s[12] == 0)) {
				s += 3;
				*vendor = simple_strtol(s, NULL, 16);

				s += 5;
				*device = simple_strtol(s, NULL, 16);

				return 0;
			}
		}
		list += (len + 1);
	}

	return -ENOENT;
}

int fdtdec_get_pci_bar32(const struct udevice *dev, struct fdt_pci_addr *addr,
			 u32 *bar)
{
	int barnum;

	/* extract the bar number from fdt_pci_addr */
	barnum = addr->phys_hi & 0xff;
	if (barnum < PCI_BASE_ADDRESS_0 || barnum > PCI_CARDBUS_CIS)
		return -EINVAL;

	barnum = (barnum - PCI_BASE_ADDRESS_0) / 4;

	*bar = dm_pci_read_bar32(dev, barnum);

	return 0;
}

int fdtdec_get_pci_bus_range(const void *blob, int node,
			     struct fdt_resource *res)
{
	const u32 *values;
	int len;

	values = fdt_getprop(blob, node, "bus-range", &len);
	if (!values || len < sizeof(*values) * 2)
		return -EINVAL;

	res->start = fdt32_to_cpu(*values++);
	res->end = fdt32_to_cpu(*values);

	return 0;
}

uint64_t fdtdec_get_uint64(const void *blob, int node, const char *prop_name,
			   uint64_t default_val)
{
	const unaligned_fdt64_t *cell64;
	int length;

	cell64 = fdt_getprop(blob, node, prop_name, &length);
	if (!cell64 || length < sizeof(*cell64))
		return default_val;

	return fdt64_to_cpu(*cell64);
}

int fdtdec_get_is_enabled(const void *blob, int node)
{
	const char *cell;

	/*
	 * It should say "okay", so only allow that. Some fdts use "ok" but
	 * this is a bug. Please fix your device tree source file. See here
	 * for discussion:
	 *
	 * http://www.mail-archive.com/u-boot@lists.denx.de/msg71598.html
	 */
	cell = fdt_getprop(blob, node, "status", NULL);
	if (cell)
		return strcmp(cell, "okay") == 0;
	return 1;
}

enum fdt_compat_id fdtdec_lookup(const void *blob, int node)
{
	enum fdt_compat_id id;

	/* Search our drivers */
	for (id = COMPAT_UNKNOWN; id < COMPAT_COUNT; id++)
		if (fdt_node_check_compatible(blob, node,
					      compat_names[id]) == 0)
			return id;
	return COMPAT_UNKNOWN;
}

int fdtdec_next_compatible(const void *blob, int node, enum fdt_compat_id id)
{
	return fdt_node_offset_by_compatible(blob, node, compat_names[id]);
}

int fdtdec_next_compatible_subnode(const void *blob, int node,
				   enum fdt_compat_id id, int *depthp)
{
	do {
		node = fdt_next_node(blob, node, depthp);
	} while (*depthp > 1);

	/* If this is a direct subnode, and compatible, return it */
	if (*depthp == 1 && 0 == fdt_node_check_compatible(
						blob, node, compat_names[id]))
		return node;

	return -FDT_ERR_NOTFOUND;
}

int fdtdec_next_alias(const void *blob, const char *name, enum fdt_compat_id id,
		      int *upto)
{
#define MAX_STR_LEN 20
	char str[MAX_STR_LEN + 20];
	int node, err;

	/* snprintf() is not available */
	assert(strlen(name) < MAX_STR_LEN);
	sprintf(str, "%.*s%d", MAX_STR_LEN, name, *upto);
	node = fdt_path_offset(blob, str);
	if (node < 0)
		return node;
	err = fdt_node_check_compatible(blob, node, compat_names[id]);
	if (err < 0)
		return err;
	if (err)
		return -FDT_ERR_NOTFOUND;
	(*upto)++;
	return node;
}

int fdtdec_find_aliases_for_id(const void *blob, const char *name,
			       enum fdt_compat_id id, int *node_list,
			       int maxcount)
{
	memset(node_list, '\0', sizeof(*node_list) * maxcount);

	return fdtdec_add_aliases_for_id(blob, name, id, node_list, maxcount);
}

/* TODO: Can we tighten this code up a little? */
int fdtdec_add_aliases_for_id(const void *blob, const char *name,
			      enum fdt_compat_id id, int *node_list,
			      int maxcount)
{
	int name_len = strlen(name);
	int nodes[maxcount];
	int num_found = 0;
	int offset, node;
	int alias_node;
	int count;
	int i, j;

	/* find the alias node if present */
	alias_node = fdt_path_offset(blob, "/aliases");

	/*
	 * start with nothing, and we can assume that the root node can't
	 * match
	 */
	memset(nodes, '\0', sizeof(nodes));

	/* First find all the compatible nodes */
	for (node = count = 0; node >= 0 && count < maxcount;) {
		node = fdtdec_next_compatible(blob, node, id);
		if (node >= 0)
			nodes[count++] = node;
	}
	if (node >= 0)
		debug("%s: warning: maxcount exceeded with alias '%s'\n",
		      __func__, name);

	/* Now find all the aliases */
	for (offset = fdt_first_property_offset(blob, alias_node);
			offset > 0;
			offset = fdt_next_property_offset(blob, offset)) {
		const struct fdt_property *prop;
		const char *path;
		int number;
		int found;

		node = 0;
		prop = fdt_get_property_by_offset(blob, offset, NULL);
		path = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
		if (prop->len && 0 == strncmp(path, name, name_len))
			node = fdt_path_offset(blob, prop->data);
		if (node <= 0)
			continue;

		/* Get the alias number */
		number = dectoul(path + name_len, NULL);
		if (number < 0 || number >= maxcount) {
			debug("%s: warning: alias '%s' is out of range\n",
			      __func__, path);
			continue;
		}

		/* Make sure the node we found is actually in our list! */
		found = -1;
		for (j = 0; j < count; j++)
			if (nodes[j] == node) {
				found = j;
				break;
			}

		if (found == -1) {
			debug("%s: warning: alias '%s' points to a node "
				"'%s' that is missing or is not compatible "
				" with '%s'\n", __func__, path,
				fdt_get_name(blob, node, NULL),
			       compat_names[id]);
			continue;
		}

		/*
		 * Add this node to our list in the right place, and mark
		 * it as done.
		 */
		if (fdtdec_get_is_enabled(blob, node)) {
			if (node_list[number]) {
				debug("%s: warning: alias '%s' requires that "
				      "a node be placed in the list in a "
				      "position which is already filled by "
				      "node '%s'\n", __func__, path,
				      fdt_get_name(blob, node, NULL));
				continue;
			}
			node_list[number] = node;
			if (number >= num_found)
				num_found = number + 1;
		}
		nodes[found] = 0;
	}

	/* Add any nodes not mentioned by an alias */
	for (i = j = 0; i < maxcount; i++) {
		if (!node_list[i]) {
			for (; j < maxcount; j++)
				if (nodes[j] &&
				    fdtdec_get_is_enabled(blob, nodes[j]))
					break;

			/* Have we run out of nodes to add? */
			if (j == maxcount)
				break;

			assert(!node_list[i]);
			node_list[i] = nodes[j++];
			if (i >= num_found)
				num_found = i + 1;
		}
	}

	return num_found;
}

int fdtdec_get_alias_seq(const void *blob, const char *base, int offset,
			 int *seqp)
{
	int base_len = strlen(base);
	const char *find_name;
	int find_namelen;
	int prop_offset;
	int aliases;

	find_name = fdt_get_name(blob, offset, &find_namelen);
	debug("Looking for '%s' at %d, name %s\n", base, offset, find_name);

	aliases = fdt_path_offset(blob, "/aliases");
	for (prop_offset = fdt_first_property_offset(blob, aliases);
	     prop_offset > 0;
	     prop_offset = fdt_next_property_offset(blob, prop_offset)) {
		const char *prop;
		const char *name;
		const char *slash;
		int len, val;

		prop = fdt_getprop_by_offset(blob, prop_offset, &name, &len);
		debug("   - %s, %s\n", name, prop);
		if (len < find_namelen || *prop != '/' || prop[len - 1] ||
		    strncmp(name, base, base_len))
			continue;

		slash = strrchr(prop, '/');
		if (strcmp(slash + 1, find_name))
			continue;

		/*
		 * Adding an extra check to distinguish DT nodes with
		 * same name
		 */
		if (IS_ENABLED(CONFIG_PHANDLE_CHECK_SEQ)) {
			if (fdt_get_phandle(blob, offset) !=
			    fdt_get_phandle(blob, fdt_path_offset(blob, prop)))
				continue;
		}

		val = trailing_strtol(name);
		if (val != -1) {
			*seqp = val;
			debug("Found seq %d\n", *seqp);
			return 0;
		}
	}

	debug("Not found\n");
	return -ENOENT;
}

int fdtdec_get_alias_highest_id(const void *blob, const char *base)
{
	int base_len = strlen(base);
	int prop_offset;
	int aliases;
	int max = -1;

	debug("Looking for highest alias id for '%s'\n", base);

	aliases = fdt_path_offset(blob, "/aliases");
	for (prop_offset = fdt_first_property_offset(blob, aliases);
	     prop_offset > 0;
	     prop_offset = fdt_next_property_offset(blob, prop_offset)) {
		const char *prop;
		const char *name;
		int len, val;

		prop = fdt_getprop_by_offset(blob, prop_offset, &name, &len);
		debug("   - %s, %s\n", name, prop);
		if (*prop != '/' || prop[len - 1] ||
		    strncmp(name, base, base_len))
			continue;

		val = trailing_strtol(name);
		if (val > max) {
			debug("Found seq %d\n", val);
			max = val;
		}
	}

	return max;
}

const char *fdtdec_get_chosen_prop(const void *blob, const char *name)
{
	int chosen_node;

	if (!blob)
		return NULL;
	chosen_node = fdt_path_offset(blob, "/chosen");
	return fdt_getprop(blob, chosen_node, name, NULL);
}

int fdtdec_get_chosen_node(const void *blob, const char *name)
{
	const char *prop;

	prop = fdtdec_get_chosen_prop(blob, name);
	if (!prop)
		return -FDT_ERR_NOTFOUND;
	return fdt_path_offset(blob, prop);
}

/**
 * fdtdec_prepare_fdt() - Check we have a valid fdt available to control U-Boot
 *
 * @blob: Blob to check
 *
 * If not, a message is printed to the console if the console is ready.
 *
 * Return: 0 if all ok, -ENOENT if not
 */
static int fdtdec_prepare_fdt(const void *blob)
{
	if (!blob || ((uintptr_t)blob & 3) || fdt_check_header(blob)) {
		if (xpl_phase() <= PHASE_SPL) {
			puts("Missing DTB\n");
		} else {
			printf("No valid device tree binary found at %p\n",
			       blob);
			if (_DEBUG && blob) {
				printf("fdt_blob=%p\n", blob);
				print_buffer((ulong)blob, blob, 4, 32, 0);
			}
		}
		return -ENOENT;
	}

	return 0;
}

int fdtdec_check_fdt(void)
{
	/*
	 * We must have an FDT, but we cannot panic() yet since the console
	 * is not ready. So for now, just assert(). Boards which need an early
	 * FDT (prior to console ready) will need to make their own
	 * arrangements and do their own checks.
	 */
	assert(!fdtdec_prepare_fdt(gd->fdt_blob));
	return 0;
}

int fdtdec_lookup_phandle(const void *blob, int node, const char *prop_name)
{
	const u32 *phandle;
	int lookup;

	debug("%s: %s\n", __func__, prop_name);
	phandle = fdt_getprop(blob, node, prop_name, NULL);
	if (!phandle)
		return -FDT_ERR_NOTFOUND;

	lookup = fdt_node_offset_by_phandle(blob, fdt32_to_cpu(*phandle));
	return lookup;
}

/**
 * Look up a property in a node and check that it has a minimum length.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param min_len	minimum property length in bytes
 * @param err		0 if ok, or -FDT_ERR_NOTFOUND if the property is not
			found, or -FDT_ERR_BADLAYOUT if not enough data
 * Return: pointer to cell, which is only valid if err == 0
 */
static const void *get_prop_check_min_len(const void *blob, int node,
					  const char *prop_name, int min_len,
					  int *err)
{
	const void *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		*err = -FDT_ERR_NOTFOUND;
	else if (len < min_len)
		*err = -FDT_ERR_BADLAYOUT;
	else
		*err = 0;
	return cell;
}

int fdtdec_get_int_array(const void *blob, int node, const char *prop_name,
			 u32 *array, int count)
{
	const u32 *cell;
	int err = 0;

	debug("%s: %s\n", __func__, prop_name);
	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	if (!err) {
		int i;

		for (i = 0; i < count; i++)
			array[i] = fdt32_to_cpu(cell[i]);
	}
	return err;
}

int fdtdec_get_int_array_count(const void *blob, int node,
			       const char *prop_name, u32 *array, int count)
{
	const u32 *cell;
	int len, elems;
	int i;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		return -FDT_ERR_NOTFOUND;
	elems = len / sizeof(u32);
	if (count > elems)
		count = elems;
	for (i = 0; i < count; i++)
		array[i] = fdt32_to_cpu(cell[i]);

	return count;
}

const u32 *fdtdec_locate_array(const void *blob, int node,
			       const char *prop_name, int count)
{
	const u32 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	return err ? NULL : cell;
}

int fdtdec_get_bool(const void *blob, int node, const char *prop_name)
{
	const s32 *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	return cell != NULL;
}

int fdtdec_parse_phandle_with_args(const void *blob, int src_node,
				   const char *list_name,
				   const char *cells_name,
				   int cell_count, int index,
				   struct fdtdec_phandle_args *out_args)
{
	const __be32 *list, *list_end;
	int rc = 0, size, cur_index = 0;
	uint32_t count = 0;
	int node = -1;
	int phandle;

	/* Retrieve the phandle list property */
	list = fdt_getprop(blob, src_node, list_name, &size);
	if (!list)
		return -ENOENT;
	list_end = list + size / sizeof(*list);

	/* Loop over the phandles until all the requested entry is found */
	while (list < list_end) {
		rc = -EINVAL;
		count = 0;

		/*
		 * If phandle is 0, then it is an empty entry with no
		 * arguments.  Skip forward to the next entry.
		 */
		phandle = be32_to_cpup(list++);
		if (phandle) {
			/*
			 * Find the provider node and parse the #*-cells
			 * property to determine the argument length.
			 *
			 * This is not needed if the cell count is hard-coded
			 * (i.e. cells_name not set, but cell_count is set),
			 * except when we're going to return the found node
			 * below.
			 */
			if (cells_name || cur_index == index) {
				node = fdt_node_offset_by_phandle(blob,
								  phandle);
				if (node < 0) {
					debug("%s: could not find phandle\n",
					      fdt_get_name(blob, src_node,
							   NULL));
					goto err;
				}
			}

			if (cells_name) {
				count = fdtdec_get_int(blob, node, cells_name,
						       -1);
				if (count == -1) {
					debug("%s: could not get %s for %s\n",
					      fdt_get_name(blob, src_node,
							   NULL),
					      cells_name,
					      fdt_get_name(blob, node,
							   NULL));
					goto err;
				}
			} else {
				count = cell_count;
			}

			/*
			 * Make sure that the arguments actually fit in the
			 * remaining property data length
			 */
			if (list + count > list_end) {
				debug("%s: arguments longer than property\n",
				      fdt_get_name(blob, src_node, NULL));
				goto err;
			}
		}

		/*
		 * All of the error cases above bail out of the loop, so at
		 * this point, the parsing is successful. If the requested
		 * index matches, then fill the out_args structure and return,
		 * or return -ENOENT for an empty entry.
		 */
		rc = -ENOENT;
		if (cur_index == index) {
			if (!phandle)
				goto err;

			if (out_args) {
				int i;

				if (count > MAX_PHANDLE_ARGS) {
					debug("%s: too many arguments %d\n",
					      fdt_get_name(blob, src_node,
							   NULL), count);
					count = MAX_PHANDLE_ARGS;
				}
				out_args->node = node;
				out_args->args_count = count;
				for (i = 0; i < count; i++) {
					out_args->args[i] =
							be32_to_cpup(list++);
				}
			}

			/* Found it! return success */
			return 0;
		}

		node = -1;
		list += count;
		cur_index++;
	}

	/*
	 * Result will be one of:
	 * -ENOENT : index is for empty phandle
	 * -EINVAL : parsing error on data
	 * [1..n]  : Number of phandle (count mode; when index = -1)
	 */
	rc = index < 0 ? cur_index : -ENOENT;
 err:
	return rc;
}

int fdtdec_get_byte_array(const void *blob, int node, const char *prop_name,
			  u8 *array, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (!err)
		memcpy(array, cell, count);
	return err;
}

const u8 *fdtdec_locate_byte_array(const void *blob, int node,
				   const char *prop_name, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (err)
		return NULL;
	return cell;
}

u64 fdtdec_get_number(const fdt32_t *ptr, unsigned int cells)
{
	u64 number = 0;

	while (cells--)
		number = (number << 32) | fdt32_to_cpu(*ptr++);

	return number;
}

int fdt_get_resource(const void *fdt, int node, const char *property,
		     unsigned int index, struct fdt_resource *res)
{
	const fdt32_t *ptr, *end;
	int na, ns, len, parent;
	unsigned int i = 0;

	parent = fdt_parent_offset(fdt, node);
	if (parent < 0)
		return parent;

	na = fdt_address_cells(fdt, parent);
	ns = fdt_size_cells(fdt, parent);

	ptr = fdt_getprop(fdt, node, property, &len);
	if (!ptr)
		return len;

	end = ptr + len / sizeof(*ptr);

	while (ptr + na + ns <= end) {
		if (i == index) {
			if (CONFIG_IS_ENABLED(OF_TRANSLATE))
				res->start = fdt_translate_address(fdt, node, ptr);
			else
				res->start = fdtdec_get_number(ptr, na);

			res->end = res->start;
			res->end += fdtdec_get_number(&ptr[na], ns) - 1;
			return 0;
		}

		ptr += na + ns;
		i++;
	}

	return -FDT_ERR_NOTFOUND;
}

int fdt_get_named_resource(const void *fdt, int node, const char *property,
			   const char *prop_names, const char *name,
			   struct fdt_resource *res)
{
	int index;

	index = fdt_stringlist_search(fdt, node, prop_names, name);
	if (index < 0)
		return index;

	return fdt_get_resource(fdt, node, property, index, res);
}

static int decode_timing_property(const void *blob, int node, const char *name,
				  struct timing_entry *result)
{
	int length, ret = 0;
	const u32 *prop;

	prop = fdt_getprop(blob, node, name, &length);
	if (!prop) {
		debug("%s: could not find property %s\n",
		      fdt_get_name(blob, node, NULL), name);
		return length;
	}

	if (length == sizeof(u32)) {
		result->typ = fdtdec_get_int(blob, node, name, 0);
		result->min = result->typ;
		result->max = result->typ;
	} else {
		ret = fdtdec_get_int_array(blob, node, name, &result->min, 3);
	}

	return ret;
}

int fdtdec_decode_display_timing(const void *blob, int parent, int index,
				 struct display_timing *dt)
{
	int i, node, timings_node;
	u32 val = 0;
	int ret = 0;

	timings_node = fdt_subnode_offset(blob, parent, "display-timings");
	if (timings_node < 0)
		return timings_node;

	for (i = 0, node = fdt_first_subnode(blob, timings_node);
	     node > 0 && i != index;
	     node = fdt_next_subnode(blob, node))
		i++;

	if (node < 0)
		return node;

	memset(dt, 0, sizeof(*dt));

	ret |= decode_timing_property(blob, node, "hback-porch",
				      &dt->hback_porch);
	ret |= decode_timing_property(blob, node, "hfront-porch",
				      &dt->hfront_porch);
	ret |= decode_timing_property(blob, node, "hactive", &dt->hactive);
	ret |= decode_timing_property(blob, node, "hsync-len", &dt->hsync_len);
	ret |= decode_timing_property(blob, node, "vback-porch",
				      &dt->vback_porch);
	ret |= decode_timing_property(blob, node, "vfront-porch",
				      &dt->vfront_porch);
	ret |= decode_timing_property(blob, node, "vactive", &dt->vactive);
	ret |= decode_timing_property(blob, node, "vsync-len", &dt->vsync_len);
	ret |= decode_timing_property(blob, node, "clock-frequency",
				      &dt->pixelclock);

	dt->flags = 0;
	val = fdtdec_get_int(blob, node, "vsync-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_VSYNC_HIGH :
				DISPLAY_FLAGS_VSYNC_LOW;
	}
	val = fdtdec_get_int(blob, node, "hsync-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_HSYNC_HIGH :
				DISPLAY_FLAGS_HSYNC_LOW;
	}
	val = fdtdec_get_int(blob, node, "de-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_DE_HIGH :
				DISPLAY_FLAGS_DE_LOW;
	}
	val = fdtdec_get_int(blob, node, "pixelclk-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_PIXDATA_POSEDGE :
				DISPLAY_FLAGS_PIXDATA_NEGEDGE;
	}

	if (fdtdec_get_bool(blob, node, "interlaced"))
		dt->flags |= DISPLAY_FLAGS_INTERLACED;
	if (fdtdec_get_bool(blob, node, "doublescan"))
		dt->flags |= DISPLAY_FLAGS_DOUBLESCAN;
	if (fdtdec_get_bool(blob, node, "doubleclk"))
		dt->flags |= DISPLAY_FLAGS_DOUBLECLK;

	return ret;
}

int fdtdec_setup_mem_size_base(void)
{
	int ret;
	ofnode mem;
	struct resource res;

	mem = ofnode_path("/memory");
	if (!ofnode_valid(mem)) {
		debug("%s: Missing /memory node\n", __func__);
		return -EINVAL;
	}

	ret = ofnode_read_resource(mem, 0, &res);
	if (ret != 0) {
		debug("%s: Unable to decode first memory bank\n", __func__);
		return -EINVAL;
	}

	gd->ram_size = (phys_size_t)(res.end - res.start + 1);
	gd->ram_base = (unsigned long)res.start;
	debug("%s: Initial DRAM size %llx\n", __func__,
	      (unsigned long long)gd->ram_size);

	return 0;
}

ofnode get_next_memory_node(ofnode mem)
{
	do {
		mem = ofnode_by_prop_value(mem, "device_type", "memory", 7);
	} while (!ofnode_is_enabled(mem));

	return mem;
}

int fdtdec_setup_memory_banksize(void)
{
	int bank, ret, reg = 0;
	struct resource res;
	ofnode mem = ofnode_null();

	mem = get_next_memory_node(mem);
	if (!ofnode_valid(mem)) {
		debug("%s: Missing /memory node\n", __func__);
		return -EINVAL;
	}

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		ret = ofnode_read_resource(mem, reg++, &res);
		if (ret < 0) {
			reg = 0;
			mem = get_next_memory_node(mem);
			if (!ofnode_valid(mem))
				break;

			ret = ofnode_read_resource(mem, reg++, &res);
			if (ret < 0)
				break;
		}

		if (ret != 0)
			return -EINVAL;

		gd->bd->bi_dram[bank].start = (phys_addr_t)res.start;
		gd->bd->bi_dram[bank].size =
			(phys_size_t)(res.end - res.start + 1);

		debug("%s: DRAM Bank #%d: start = 0x%llx, size = 0x%llx\n",
		      __func__, bank,
		      (unsigned long long)gd->bd->bi_dram[bank].start,
		      (unsigned long long)gd->bd->bi_dram[bank].size);
	}

	return 0;
}

int fdtdec_setup_mem_size_base_lowest(void)
{
	int bank, ret, reg = 0;
	struct resource res;
	unsigned long base;
	phys_size_t size;
	ofnode mem = ofnode_null();

	gd->ram_base = (unsigned long)~0;

	mem = get_next_memory_node(mem);
	if (!ofnode_valid(mem)) {
		debug("%s: Missing /memory node\n", __func__);
		return -EINVAL;
	}

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		ret = ofnode_read_resource(mem, reg++, &res);
		if (ret < 0) {
			reg = 0;
			mem = get_next_memory_node(mem);
			if (!ofnode_valid(mem))
				break;

			ret = ofnode_read_resource(mem, reg++, &res);
			if (ret < 0)
				break;
		}

		if (ret != 0)
			return -EINVAL;

		base = (unsigned long)res.start;
		size = (phys_size_t)(res.end - res.start + 1);

		if (gd->ram_base > base && size) {
			gd->ram_base = base;
			gd->ram_size = size;
			debug("%s: Initial DRAM base %lx size %lx\n",
			      __func__, base, (unsigned long)size);
		}
	}

	return 0;
}

static int uncompress_blob(const void *src, ulong sz_src, void **dstp)
{
#if CONFIG_IS_ENABLED(MULTI_DTB_FIT_GZIP) ||\
	CONFIG_IS_ENABLED(MULTI_DTB_FIT_LZO)
	size_t sz_out = CONFIG_VAL(MULTI_DTB_FIT_UNCOMPRESS_SZ);
	bool gzip = 0, lzo = 0;
	ulong sz_in = sz_src;
	void *dst;
	int rc;

	if (CONFIG_IS_ENABLED(GZIP) && CONFIG_IS_ENABLED(MULTI_DTB_FIT_GZIP))
		if (gzip_parse_header(src, sz_in) >= 0)
			gzip = 1;
	if (CONFIG_IS_ENABLED(LZO) && CONFIG_IS_ENABLED(MULTI_DTB_FIT_LZO))
		if (!gzip && lzop_is_valid_header(src))
			lzo = 1;

	if (!gzip && !lzo)
		return -EBADMSG;

	if (CONFIG_IS_ENABLED(MULTI_DTB_FIT_DYN_ALLOC)) {
		dst = malloc(sz_out);
		if (!dst) {
			puts("uncompress_blob: Unable to allocate memory\n");
			return -ENOMEM;
		}
	} else  {
# if CONFIG_IS_ENABLED(MULTI_DTB_FIT_USER_DEFINED_AREA)
		dst = (void *)CONFIG_VAL(MULTI_DTB_FIT_USER_DEF_ADDR);
# else
		return -ENOTSUPP;
# endif
	}

	if (CONFIG_IS_ENABLED(GZIP) && gzip)
		rc = gunzip(dst, sz_out, (u8 *)src, &sz_in);
	else if (CONFIG_IS_ENABLED(LZO) && lzo)
		rc = lzop_decompress(src, sz_in, dst, &sz_out);
	else
		hang();

	if (rc < 0) {
		/* not a valid compressed blob */
		puts("uncompress_blob: Unable to uncompress\n");
		if (CONFIG_IS_ENABLED(MULTI_DTB_FIT_DYN_ALLOC))
			free(dst);
		return -EBADMSG;
	}
	*dstp = dst;
#else
	*dstp = (void *)src;
	*dstp = (void *)src;
#endif
	return 0;
}

/**
 * fdt_find_separate() - Find a devicetree at the end of the image
 *
 * Return: pointer to FDT blob
 */
static void *fdt_find_separate(void)
{
	void *fdt_blob = NULL;

	if (IS_ENABLED(CONFIG_SANDBOX))
		return NULL;

#ifdef CONFIG_XPL_BUILD
	/* FDT is at end of BSS unless it is in a different memory region */
	if (CONFIG_IS_ENABLED(SEPARATE_BSS))
		fdt_blob = (ulong *)_image_binary_end;
	else
		fdt_blob = (ulong *)__bss_end;
#else
	/* FDT is at end of image */
	fdt_blob = (ulong *)_end;

	if (_DEBUG && !fdtdec_prepare_fdt(fdt_blob)) {
		int stack_ptr;
		const void *top = fdt_blob + fdt_totalsize(fdt_blob);

		/*
		 * Perform a sanity check on the memory layout. If this fails,
		 * it indicates that the device tree is positioned above the
		 * global data pointer or the stack pointer. This should not
		 * happen.
		 *
		 * If this fails, check that SYS_INIT_SP_ADDR has enough space
		 * below it for SYS_MALLOC_F_LEN and global_data, as well as the
		 * stack, without overwriting the device tree or U-Boot itself.
		 * Since the device tree is sitting at _end (the start of the
		 * BSS region), we need the top of the device tree to be below
		 * any memory allocated by board_init_f_alloc_reserve().
		 */
		if (top > (void *)gd || top > (void *)&stack_ptr) {
			printf("FDT %p gd %p\n", fdt_blob, gd);
			panic("FDT overlap");
		}
	}
#endif

	return fdt_blob;
}

int fdtdec_set_ethernet_mac_address(void *fdt, const u8 *mac, size_t size)
{
	const char *path;
	int offset, err;

	if (!is_valid_ethaddr(mac))
		return -EINVAL;

	path = fdt_get_alias(fdt, "ethernet");
	if (!path)
		return 0;

	debug("ethernet alias found: %s\n", path);

	offset = fdt_path_offset(fdt, path);
	if (offset < 0) {
		debug("ethernet alias points to absent node %s\n", path);
		return -ENOENT;
	}

	err = fdt_setprop_inplace(fdt, offset, "local-mac-address", mac, size);
	if (err < 0)
		return err;

	debug("MAC address: %pM\n", mac);

	return 0;
}

static int fdtdec_init_reserved_memory(void *blob)
{
	int na, ns, node, err;
	fdt32_t value;

	/* inherit #address-cells and #size-cells from the root node */
	na = fdt_address_cells(blob, 0);
	ns = fdt_size_cells(blob, 0);

	node = fdt_add_subnode(blob, 0, "reserved-memory");
	if (node < 0)
		return node;

	err = fdt_setprop(blob, node, "ranges", NULL, 0);
	if (err < 0)
		return err;

	value = cpu_to_fdt32(ns);

	err = fdt_setprop(blob, node, "#size-cells", &value, sizeof(value));
	if (err < 0)
		return err;

	value = cpu_to_fdt32(na);

	err = fdt_setprop(blob, node, "#address-cells", &value, sizeof(value));
	if (err < 0)
		return err;

	return node;
}

int fdtdec_add_reserved_memory(void *blob, const char *basename,
			       const struct fdt_memory *carveout,
			       const char **compatibles, unsigned int count,
			       uint32_t *phandlep, unsigned long flags)
{
	fdt32_t cells[4] = {}, *ptr = cells;
	uint32_t upper, lower, phandle;
	int parent, node, na, ns, err;
	fdt_size_t size;
	char name[64];

	/* create an empty /reserved-memory node if one doesn't exist */
	parent = fdt_path_offset(blob, "/reserved-memory");
	if (parent < 0) {
		parent = fdtdec_init_reserved_memory(blob);
		if (parent < 0)
			return parent;
	}

	/* only 1 or 2 #address-cells and #size-cells are supported */
	na = fdt_address_cells(blob, parent);
	if (na < 1 || na > 2)
		return -FDT_ERR_BADNCELLS;

	ns = fdt_size_cells(blob, parent);
	if (ns < 1 || ns > 2)
		return -FDT_ERR_BADNCELLS;

	/* find a matching node and return the phandle to that */
	fdt_for_each_subnode(node, blob, parent) {
		const char *name = fdt_get_name(blob, node, NULL);
		fdt_addr_t addr;
		fdt_size_t size;

		addr = fdtdec_get_addr_size_fixed(blob, node, "reg", 0, na, ns,
						  &size, false);
		if (addr == FDT_ADDR_T_NONE) {
			debug("failed to read address/size for %s\n", name);
			continue;
		}

		if (addr == carveout->start && (addr + size - 1) ==
						carveout->end) {
			if (phandlep)
				*phandlep = fdt_get_phandle(blob, node);
			return 0;
		}
	}

	/*
	 * Unpack the start address and generate the name of the new node
	 * base on the basename and the unit-address.
	 */
	upper = upper_32_bits(carveout->start);
	lower = lower_32_bits(carveout->start);

	if (na > 1 && upper > 0)
		snprintf(name, sizeof(name), "%s@%x,%x", basename, upper,
			 lower);
	else {
		if (upper > 0) {
			debug("address %08x:%08x exceeds addressable space\n",
			      upper, lower);
			return -FDT_ERR_BADVALUE;
		}

		snprintf(name, sizeof(name), "%s@%x", basename, lower);
	}

	node = fdt_add_subnode(blob, parent, name);
	if (node < 0)
		return node;

	if (flags & FDTDEC_RESERVED_MEMORY_NO_MAP) {
		err = fdt_setprop(blob, node, "no-map", NULL, 0);
		if (err < 0)
			return err;
	}

	if (phandlep) {
		err = fdt_generate_phandle(blob, &phandle);
		if (err < 0)
			return err;

		err = fdtdec_set_phandle(blob, node, phandle);
		if (err < 0)
			return err;
	}

	/* store one or two address cells */
	if (na > 1)
		*ptr++ = cpu_to_fdt32(upper);

	*ptr++ = cpu_to_fdt32(lower);

	/* store one or two size cells */
	size = carveout->end - carveout->start + 1;
	upper = upper_32_bits(size);
	lower = lower_32_bits(size);

	if (ns > 1)
		*ptr++ = cpu_to_fdt32(upper);

	*ptr++ = cpu_to_fdt32(lower);

	err = fdt_setprop(blob, node, "reg", cells, (na + ns) * sizeof(*cells));
	if (err < 0)
		return err;

	if (compatibles && count > 0) {
		size_t length = 0, len = 0;
		unsigned int i;
		char *buffer;

		for (i = 0; i < count; i++)
			length += strlen(compatibles[i]) + 1;

		buffer = malloc(length);
		if (!buffer)
			return -FDT_ERR_INTERNAL;

		for (i = 0; i < count; i++)
			len += strlcpy(buffer + len, compatibles[i],
				       length - len) + 1;

		err = fdt_setprop(blob, node, "compatible", buffer, length);
		free(buffer);
		if (err < 0)
			return err;
	}

	/* return the phandle for the new node for the caller to use */
	if (phandlep)
		*phandlep = phandle;

	return 0;
}

int fdtdec_get_carveout(const void *blob, const char *node,
			const char *prop_name, unsigned int index,
			struct fdt_memory *carveout, const char **name,
			const char ***compatiblesp, unsigned int *countp,
			unsigned long *flags)
{
	const fdt32_t *prop;
	uint32_t phandle;
	int offset, len;
	fdt_size_t size;

	offset = fdt_path_offset(blob, node);
	if (offset < 0)
		return offset;

	prop = fdt_getprop(blob, offset, prop_name, &len);
	if (!prop) {
		debug("failed to get %s for %s\n", prop_name, node);
		return -FDT_ERR_NOTFOUND;
	}

	if ((len % sizeof(phandle)) != 0) {
		debug("invalid phandle property\n");
		return -FDT_ERR_BADPHANDLE;
	}

	if (len < (sizeof(phandle) * (index + 1))) {
		debug("invalid phandle index\n");
		return -FDT_ERR_NOTFOUND;
	}

	phandle = fdt32_to_cpu(prop[index]);

	offset = fdt_node_offset_by_phandle(blob, phandle);
	if (offset < 0) {
		debug("failed to find node for phandle %u\n", phandle);
		return offset;
	}

	if (name)
		*name = fdt_get_name(blob, offset, NULL);

	if (compatiblesp) {
		const char **compatibles = NULL;
		const char *start, *end, *ptr;
		unsigned int count = 0;

		prop = fdt_getprop(blob, offset, "compatible", &len);
		if (!prop)
			goto skip_compat;

		start = ptr = (const char *)prop;
		end = start + len;

		while (ptr < end) {
			ptr = strchrnul(ptr, '\0');
			count++;
			ptr++;
		}

		compatibles = malloc(sizeof(ptr) * count);
		if (!compatibles)
			return -FDT_ERR_INTERNAL;

		ptr = start;
		count = 0;

		while (ptr < end) {
			compatibles[count] = ptr;
			ptr = strchrnul(ptr, '\0');
			count++;
			ptr++;
		}

skip_compat:
		*compatiblesp = compatibles;

		if (countp)
			*countp = count;
	}

	carveout->start = fdtdec_get_addr_size_auto_noparent(blob, offset,
							     "reg", 0, &size,
							     true);
	if (carveout->start == FDT_ADDR_T_NONE) {
		debug("failed to read address/size from \"reg\" property\n");
		return -FDT_ERR_NOTFOUND;
	}

	carveout->end = carveout->start + size - 1;

	if (flags) {
		*flags = 0;

		if (fdtdec_get_bool(blob, offset, "no-map"))
			*flags |= FDTDEC_RESERVED_MEMORY_NO_MAP;
	}

	return 0;
}

int fdtdec_set_carveout(void *blob, const char *node, const char *prop_name,
			unsigned int index, const struct fdt_memory *carveout,
			const char *name, const char **compatibles,
			unsigned int count, unsigned long flags)
{
	uint32_t phandle;
	int err, offset, len;
	fdt32_t value;
	void *prop;

	err = fdtdec_add_reserved_memory(blob, name, carveout, compatibles,
					 count, &phandle, flags);
	if (err < 0) {
		debug("failed to add reserved memory: %d\n", err);
		return err;
	}

	offset = fdt_path_offset(blob, node);
	if (offset < 0) {
		debug("failed to find offset for node %s: %d\n", node, offset);
		return offset;
	}

	value = cpu_to_fdt32(phandle);

	if (!fdt_getprop(blob, offset, prop_name, &len)) {
		if (len == -FDT_ERR_NOTFOUND)
			len = 0;
		else
			return len;
	}

	if ((index + 1) * sizeof(value) > len) {
		err = fdt_setprop_placeholder(blob, offset, prop_name,
					      (index + 1) * sizeof(value),
					      &prop);
		if (err < 0) {
			debug("failed to resize reserved memory property: %s\n",
			      fdt_strerror(err));
			return err;
		}
	}

	err = fdt_setprop_inplace_namelen_partial(blob, offset, prop_name,
						  strlen(prop_name),
						  index * sizeof(value),
						  &value, sizeof(value));
	if (err < 0) {
		debug("failed to update %s property for node %s: %s\n",
		      prop_name, node, fdt_strerror(err));
		return err;
	}

	return 0;
}

/* TODO(sjg@chromium.org): This function should not be weak */
__weak int fdtdec_board_setup(const void *fdt_blob)
{
	return 0;
}

/**
 * setup_multi_dtb_fit() - locate the correct dtb from a FIT
 *
 * This supports the CONFIG_MULTI_DTB_FIT feature, looking for the dtb in a
 * supplied FIT
 *
 * It accepts the current value of gd->fdt_blob, which points to the FIT, then
 * updates that gd->fdt_blob, to point to the chosen dtb so that U-Boot uses the
 * correct one
 */
static void setup_multi_dtb_fit(void)
{
	void *blob;

	/*
	 * Try and uncompress the blob.
	 * Unfortunately there is no way to know how big the input blob really
	 * is. So let us set the maximum input size arbitrarily high. 16MB
	 * ought to be more than enough for packed DTBs.
	 */
	if (uncompress_blob(gd->fdt_blob, 0x1000000, &blob) == 0)
		gd->fdt_blob = blob;

	/*
	 * Check if blob is a FIT images containings DTBs.
	 * If so, pick the most relevant
	 */
	blob = locate_dtb_in_fit(gd->fdt_blob);
	if (blob) {
		gd_set_multi_dtb_fit(gd->fdt_blob);
		gd->fdt_blob = blob;
		gd->fdt_src = FDTSRC_FIT;
	}
}

void fdtdec_setup_embed(void)
{
	gd->fdt_blob = dtb_dt_embedded();
	gd->fdt_src = FDTSRC_EMBED;
}

int fdtdec_setup(void)
{
	int ret = -ENOENT;

	/*
	 * Only scan for a bloblist and then if that bloblist contains a device
	 * tree if we have been configured to expect one.
	 */
	if (CONFIG_IS_ENABLED(BLOBLIST_PRIOR_STAGE)) {
		ret = bloblist_maybe_init();
		if (!ret) {
			gd->fdt_blob = bloblist_find(BLOBLISTT_CONTROL_FDT, 0);
			if (gd->fdt_blob) {
				gd->fdt_src = FDTSRC_BLOBLIST;
				log_debug("Devicetree is in bloblist at %p\n",
					  gd->fdt_blob);
				ret = 0;
			} else {
				log_debug("No FDT found in bloblist\n");
				ret = -ENOENT;
			}
		}
	}

	/* Otherwise, the devicetree is typically appended to U-Boot */
	if (ret) {
		if (IS_ENABLED(CONFIG_OF_SEPARATE)) {
			gd->fdt_blob = fdt_find_separate();
			gd->fdt_src = FDTSRC_SEPARATE;
		} else { /* embed dtb in ELF file for testing / development */
			fdtdec_setup_embed();
		}
	}

	/* Allow the board to override the fdt address. */
	if (IS_ENABLED(CONFIG_OF_BOARD)) {
		void *blob;

		blob = (void *)gd->fdt_blob;
		ret = board_fdt_blob_setup(&blob);
		if (ret) {
			if (ret != -EEXIST)
				return ret;
		} else {
			gd->fdt_src = FDTSRC_BOARD;
			gd->fdt_blob = blob;
		}
	}

	/* Allow the early environment to override the fdt address */
	if (!IS_ENABLED(CONFIG_XPL_BUILD)) {
		ulong addr;

		addr = env_get_hex("fdtcontroladdr", 0);
		if (addr) {
			gd->fdt_blob = map_sysmem(addr, 0);
			gd->fdt_src = FDTSRC_ENV;
		}
	}

	if (CONFIG_IS_ENABLED(MULTI_DTB_FIT))
		setup_multi_dtb_fit();

	ret = fdtdec_prepare_fdt(gd->fdt_blob);
	if (!ret)
		ret = fdtdec_board_setup(gd->fdt_blob);
	oftree_reset();

	return ret;
}

int fdtdec_resetup(int *rescan)
{
	void *fdt_blob;

	/*
	 * If the current DTB is part of a compressed FIT image,
	 * try to locate the best match from the uncompressed
	 * FIT image stillpresent there. Save the time and space
	 * required to uncompress it again.
	 */
	if (gd_multi_dtb_fit()) {
		fdt_blob = locate_dtb_in_fit(gd_multi_dtb_fit());

		if (fdt_blob == gd->fdt_blob) {
			/*
			 * The best match did not change. no need to tear down
			 * the DM and rescan the fdt.
			 */
			*rescan = 0;
			return 0;
		}

		*rescan = 1;
		gd->fdt_blob = fdt_blob;
		return fdtdec_prepare_fdt(fdt_blob);
	}

	/*
	 * If multi_dtb_fit is NULL, it means that blob appended to u-boot is
	 * not a FIT image containings DTB, but a single DTB. There is no need
	 * to teard down DM and rescan the DT in this case.
	 */
	*rescan = 0;
	return 0;
}

int fdtdec_decode_ram_size(const void *blob, const char *area, int board_id,
			   phys_addr_t *basep, phys_size_t *sizep,
			   struct bd_info *bd)
{
	int addr_cells, size_cells;
	const u32 *cell, *end;
	u64 total_size, size, addr;
	int node, child;
	bool auto_size;
	int bank;
	int len;

	debug("%s: board_id=%d\n", __func__, board_id);
	if (!area)
		area = "/memory";
	node = fdt_path_offset(blob, area);
	if (node < 0) {
		debug("No %s node found\n", area);
		return -ENOENT;
	}

	cell = fdt_getprop(blob, node, "reg", &len);
	if (!cell) {
		debug("No reg property found\n");
		return -ENOENT;
	}

	addr_cells = fdt_address_cells(blob, node);
	size_cells = fdt_size_cells(blob, node);

	/* Check the board id and mask */
	for (child = fdt_first_subnode(blob, node);
	     child >= 0;
	     child = fdt_next_subnode(blob, child)) {
		int match_mask, match_value;

		match_mask = fdtdec_get_int(blob, child, "match-mask", -1);
		match_value = fdtdec_get_int(blob, child, "match-value", -1);

		if (match_value >= 0 &&
		    ((board_id & match_mask) == match_value)) {
			/* Found matching mask */
			debug("Found matching mask %d\n", match_mask);
			node = child;
			cell = fdt_getprop(blob, node, "reg", &len);
			if (!cell) {
				debug("No memory-banks property found\n");
				return -EINVAL;
			}
			break;
		}
	}
	/* Note: if no matching subnode was found we use the parent node */

	if (bd) {
		memset(bd->bi_dram, '\0', sizeof(bd->bi_dram[0]) *
						CONFIG_NR_DRAM_BANKS);
	}

	auto_size = fdtdec_get_bool(blob, node, "auto-size");

	total_size = 0;
	end = cell + len / 4 - addr_cells - size_cells;
	debug("cell at %p, end %p\n", cell, end);
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (cell > end)
			break;
		addr = 0;
		if (addr_cells == 2)
			addr += (u64)fdt32_to_cpu(*cell++) << 32UL;
		addr += fdt32_to_cpu(*cell++);
		if (bd)
			bd->bi_dram[bank].start = addr;
		if (basep && !bank)
			*basep = (phys_addr_t)addr;

		size = 0;
		if (size_cells == 2)
			size += (u64)fdt32_to_cpu(*cell++) << 32UL;
		size += fdt32_to_cpu(*cell++);

		if (auto_size) {
			u64 new_size;

			debug("Auto-sizing %llx, size %llx: ", addr, size);
			new_size = get_ram_size((long *)(uintptr_t)addr, size);
			if (new_size == size) {
				debug("OK\n");
			} else {
				debug("sized to %llx\n", new_size);
				size = new_size;
			}
		}

		if (bd)
			bd->bi_dram[bank].size = size;
		total_size += size;
	}

	debug("Memory size %llu\n", total_size);
	if (sizep)
		*sizep = (phys_size_t)total_size;

	return 0;
}

#endif /* !USE_HOSTCC */
