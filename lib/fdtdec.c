/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <errno.h>
#include <serial.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <asm/sections.h>
#include <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Here are the type we know about. One day we might allow drivers to
 * register. For now we just put them here. The COMPAT macro allows us to
 * turn this into a sparse list later, and keeps the ID with the name.
 */
#define COMPAT(id, name) name
static const char * const compat_names[COMPAT_COUNT] = {
	COMPAT(UNKNOWN, "<none>"),
	COMPAT(NVIDIA_TEGRA20_EMC, "nvidia,tegra20-emc"),
	COMPAT(NVIDIA_TEGRA20_EMC_TABLE, "nvidia,tegra20-emc-table"),
	COMPAT(NVIDIA_TEGRA20_KBC, "nvidia,tegra20-kbc"),
	COMPAT(NVIDIA_TEGRA20_NAND, "nvidia,tegra20-nand"),
	COMPAT(NVIDIA_TEGRA20_PWM, "nvidia,tegra20-pwm"),
	COMPAT(NVIDIA_TEGRA124_DC, "nvidia,tegra124-dc"),
	COMPAT(NVIDIA_TEGRA124_SOR, "nvidia,tegra124-sor"),
	COMPAT(NVIDIA_TEGRA124_PMC, "nvidia,tegra124-pmc"),
	COMPAT(NVIDIA_TEGRA20_DC, "nvidia,tegra20-dc"),
	COMPAT(NVIDIA_TEGRA124_SDMMC, "nvidia,tegra124-sdhci"),
	COMPAT(NVIDIA_TEGRA30_SDMMC, "nvidia,tegra30-sdhci"),
	COMPAT(NVIDIA_TEGRA20_SDMMC, "nvidia,tegra20-sdhci"),
	COMPAT(NVIDIA_TEGRA124_PCIE, "nvidia,tegra124-pcie"),
	COMPAT(NVIDIA_TEGRA30_PCIE, "nvidia,tegra30-pcie"),
	COMPAT(NVIDIA_TEGRA20_PCIE, "nvidia,tegra20-pcie"),
	COMPAT(NVIDIA_TEGRA124_XUSB_PADCTL, "nvidia,tegra124-xusb-padctl"),
	COMPAT(SMSC_LAN9215, "smsc,lan9215"),
	COMPAT(SAMSUNG_EXYNOS5_SROMC, "samsung,exynos-sromc"),
	COMPAT(SAMSUNG_S3C2440_I2C, "samsung,s3c2440-i2c"),
	COMPAT(SAMSUNG_EXYNOS5_SOUND, "samsung,exynos-sound"),
	COMPAT(WOLFSON_WM8994_CODEC, "wolfson,wm8994-codec"),
	COMPAT(GOOGLE_CROS_EC_KEYB, "google,cros-ec-keyb"),
	COMPAT(SAMSUNG_EXYNOS_USB_PHY, "samsung,exynos-usb-phy"),
	COMPAT(SAMSUNG_EXYNOS5_USB3_PHY, "samsung,exynos5250-usb3-phy"),
	COMPAT(SAMSUNG_EXYNOS_TMU, "samsung,exynos-tmu"),
	COMPAT(SAMSUNG_EXYNOS_FIMD, "samsung,exynos-fimd"),
	COMPAT(SAMSUNG_EXYNOS_MIPI_DSI, "samsung,exynos-mipi-dsi"),
	COMPAT(SAMSUNG_EXYNOS5_DP, "samsung,exynos5-dp"),
	COMPAT(SAMSUNG_EXYNOS_DWMMC, "samsung,exynos-dwmmc"),
	COMPAT(SAMSUNG_EXYNOS_MMC, "samsung,exynos-mmc"),
	COMPAT(SAMSUNG_EXYNOS_SERIAL, "samsung,exynos4210-uart"),
	COMPAT(MAXIM_MAX77686_PMIC, "maxim,max77686"),
	COMPAT(GENERIC_SPI_FLASH, "spi-flash"),
	COMPAT(MAXIM_98095_CODEC, "maxim,max98095-codec"),
	COMPAT(INFINEON_SLB9635_TPM, "infineon,slb9635-tpm"),
	COMPAT(INFINEON_SLB9645_TPM, "infineon,slb9645tt"),
	COMPAT(SAMSUNG_EXYNOS5_I2C, "samsung,exynos5-hsi2c"),
	COMPAT(SANDBOX_LCD_SDL, "sandbox,lcd-sdl"),
	COMPAT(TI_TPS65090, "ti,tps65090"),
	COMPAT(COMPAT_NXP_PTN3460, "nxp,ptn3460"),
	COMPAT(SAMSUNG_EXYNOS_SYSMMU, "samsung,sysmmu-v3.3"),
	COMPAT(PARADE_PS8625, "parade,ps8625"),
	COMPAT(INTEL_MICROCODE, "intel,microcode"),
	COMPAT(MEMORY_SPD, "memory-spd"),
	COMPAT(INTEL_PANTHERPOINT_AHCI, "intel,pantherpoint-ahci"),
	COMPAT(INTEL_MODEL_206AX, "intel,model-206ax"),
	COMPAT(INTEL_GMA, "intel,gma"),
	COMPAT(AMS_AS3722, "ams,as3722"),
	COMPAT(INTEL_ICH_SPI, "intel,ich-spi"),
	COMPAT(INTEL_QRK_MRC, "intel,quark-mrc"),
	COMPAT(INTEL_X86_PINCTRL, "intel,x86-pinctrl"),
	COMPAT(SOCIONEXT_XHCI, "socionext,uniphier-xhci"),
	COMPAT(COMPAT_INTEL_PCH, "intel,bd82x6x"),
	COMPAT(COMPAT_INTEL_IRQ_ROUTER, "intel,irq-router"),
};

const char *fdtdec_get_compatible(enum fdt_compat_id id)
{
	/* We allow reading of the 'unknown' ID for testing purposes */
	assert(id >= 0 && id < COMPAT_COUNT);
	return compat_names[id];
}

fdt_addr_t fdtdec_get_addr_size(const void *blob, int node,
		const char *prop_name, fdt_size_t *sizep)
{
	const fdt32_t *ptr, *end;
	int parent, na, ns, len;
	fdt_addr_t addr;

	debug("%s: %s: ", __func__, prop_name);

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("(no parent found)\n");
		return FDT_ADDR_T_NONE;
	}

	na = fdt_address_cells(blob, parent);
	ns = fdt_size_cells(blob, parent);

	ptr = fdt_getprop(blob, node, prop_name, &len);
	if (!ptr) {
		debug("(not found)\n");
		return FDT_ADDR_T_NONE;
	}

	end = ptr + len / sizeof(*ptr);

	if (ptr + na + ns > end) {
		debug("(not enough data: expected %d bytes, got %d bytes)\n",
		      (na + ns) * 4, len);
		return FDT_ADDR_T_NONE;
	}

	addr = fdtdec_get_number(ptr, na);

	if (sizep) {
		*sizep = fdtdec_get_number(ptr + na, ns);
		debug("addr=%pa, size=%pa\n", &addr, sizep);
	} else {
		debug("%pa\n", &addr);
	}

	return addr;
}

fdt_addr_t fdtdec_get_addr(const void *blob, int node,
		const char *prop_name)
{
	return fdtdec_get_addr_size(blob, node, prop_name, NULL);
}

#ifdef CONFIG_PCI
int fdtdec_get_pci_addr(const void *blob, int node, enum fdt_pci_space type,
		const char *prop_name, struct fdt_pci_addr *addr)
{
	const u32 *cell;
	int len;
	int ret = -ENOENT;

	debug("%s: %s: ", __func__, prop_name);

	/*
	 * If we follow the pci bus bindings strictly, we should check
	 * the value of the node's parent node's #address-cells and
	 * #size-cells. They need to be 3 and 2 accordingly. However,
	 * for simplicity we skip the check here.
	 */
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		goto fail;

	if ((len % FDT_PCI_REG_SIZE) == 0) {
		int num = len / FDT_PCI_REG_SIZE;
		int i;

		for (i = 0; i < num; i++) {
			debug("pci address #%d: %08lx %08lx %08lx\n", i,
			      (ulong)fdt_addr_to_cpu(cell[0]),
			      (ulong)fdt_addr_to_cpu(cell[1]),
			      (ulong)fdt_addr_to_cpu(cell[2]));
			if ((fdt_addr_to_cpu(*cell) & type) == type) {
				addr->phys_hi = fdt_addr_to_cpu(cell[0]);
				addr->phys_mid = fdt_addr_to_cpu(cell[1]);
				addr->phys_lo = fdt_addr_to_cpu(cell[2]);
				break;
			} else {
				cell += (FDT_PCI_ADDR_CELLS +
					 FDT_PCI_SIZE_CELLS);
			}
		}

		if (i == num) {
			ret = -ENXIO;
			goto fail;
		}

		return 0;
	} else {
		ret = -EINVAL;
	}

fail:
	debug("(not found)\n");
	return ret;
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
		char *s;

		len = strlen(list);
		if (len >= strlen("pciVVVV,DDDD")) {
			s = strstr(list, "pci");

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
		} else {
			list += (len + 1);
		}
	}

	return -ENOENT;
}

int fdtdec_get_pci_bdf(const void *blob, int node,
		struct fdt_pci_addr *addr, pci_dev_t *bdf)
{
	u16 dt_vendor, dt_device, vendor, device;
	int ret;

	/* get vendor id & device id from the compatible string */
	ret = fdtdec_get_pci_vendev(blob, node, &dt_vendor, &dt_device);
	if (ret)
		return ret;

	/* extract the bdf from fdt_pci_addr */
	*bdf = addr->phys_hi & 0xffff00;

	/* read vendor id & device id based on bdf */
	pci_read_config_word(*bdf, PCI_VENDOR_ID, &vendor);
	pci_read_config_word(*bdf, PCI_DEVICE_ID, &device);

	/*
	 * Note there are two places in the device tree to fully describe
	 * a pci device: one is via compatible string with a format of
	 * "pciVVVV,DDDD" and the other one is the bdf numbers encoded in
	 * the device node's reg address property. We read the vendor id
	 * and device id based on bdf and compare the values with the
	 * "VVVV,DDDD". If they are the same, then we are good to use bdf
	 * to read device's bar. But if they are different, we have to rely
	 * on the vendor id and device id extracted from the compatible
	 * string and locate the real bdf by pci_find_device(). This is
	 * because normally we may only know device's device number and
	 * function number when writing device tree. The bus number is
	 * dynamically assigned during the pci enumeration process.
	 */
	if ((dt_vendor != vendor) || (dt_device != device)) {
		*bdf = pci_find_device(dt_vendor, dt_device, 0);
		if (*bdf == -1)
			return -ENODEV;
	}

	return 0;
}

int fdtdec_get_pci_bar32(const void *blob, int node,
		struct fdt_pci_addr *addr, u32 *bar)
{
	pci_dev_t bdf;
	int barnum;
	int ret;

	/* get pci devices's bdf */
	ret = fdtdec_get_pci_bdf(blob, node, addr, &bdf);
	if (ret)
		return ret;

	/* extract the bar number from fdt_pci_addr */
	barnum = addr->phys_hi & 0xff;
	if ((barnum < PCI_BASE_ADDRESS_0) || (barnum > PCI_CARDBUS_CIS))
		return -EINVAL;

	barnum = (barnum - PCI_BASE_ADDRESS_0) / 4;
	*bar = pci_read_bar32(pci_bus_to_hose(PCI_BUS(bdf)), bdf, barnum);

	return 0;
}
#endif

uint64_t fdtdec_get_uint64(const void *blob, int node, const char *prop_name,
		uint64_t default_val)
{
	const uint64_t *cell64;
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
		return 0 == strcmp(cell, "okay");
	return 1;
}

enum fdt_compat_id fdtdec_lookup(const void *blob, int node)
{
	enum fdt_compat_id id;

	/* Search our drivers */
	for (id = COMPAT_UNKNOWN; id < COMPAT_COUNT; id++)
		if (0 == fdt_node_check_compatible(blob, node,
				compat_names[id]))
			return id;
	return COMPAT_UNKNOWN;
}

int fdtdec_next_compatible(const void *blob, int node,
		enum fdt_compat_id id)
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

int fdtdec_next_alias(const void *blob, const char *name,
		enum fdt_compat_id id, int *upto)
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
			enum fdt_compat_id id, int *node_list, int maxcount)
{
	memset(node_list, '\0', sizeof(*node_list) * maxcount);

	return fdtdec_add_aliases_for_id(blob, name, id, node_list, maxcount);
}

/* TODO: Can we tighten this code up a little? */
int fdtdec_add_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount)
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
		number = simple_strtoul(path + name_len, NULL, 10);
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
		const char *p;
		int len;

		prop = fdt_getprop_by_offset(blob, prop_offset, &name, &len);
		debug("   - %s, %s\n", name, prop);
		if (len < find_namelen || *prop != '/' || prop[len - 1] ||
		    strncmp(name, base, base_len))
			continue;

		slash = strrchr(prop, '/');
		if (strcmp(slash + 1, find_name))
			continue;
		for (p = name + strlen(name) - 1; p > name; p--) {
			if (!isdigit(*p)) {
				*seqp = simple_strtoul(p + 1, NULL, 10);
				debug("Found seq %d\n", *seqp);
				return 0;
			}
		}
	}

	debug("Not found\n");
	return -ENOENT;
}

int fdtdec_get_chosen_node(const void *blob, const char *name)
{
	const char *prop;
	int chosen_node;
	int len;

	if (!blob)
		return -FDT_ERR_NOTFOUND;
	chosen_node = fdt_path_offset(blob, "/chosen");
	prop = fdt_getprop(blob, chosen_node, name, &len);
	if (!prop)
		return -FDT_ERR_NOTFOUND;
	return fdt_path_offset(blob, prop);
}

int fdtdec_check_fdt(void)
{
	/*
	 * We must have an FDT, but we cannot panic() yet since the console
	 * is not ready. So for now, just assert(). Boards which need an early
	 * FDT (prior to console ready) will need to make their own
	 * arrangements and do their own checks.
	 */
	assert(!fdtdec_prepare_fdt());
	return 0;
}

/*
 * This function is a little odd in that it accesses global data. At some
 * point if the architecture board.c files merge this will make more sense.
 * Even now, it is common code.
 */
int fdtdec_prepare_fdt(void)
{
	if (!gd->fdt_blob || ((uintptr_t)gd->fdt_blob & 3) ||
	    fdt_check_header(gd->fdt_blob)) {
#ifdef CONFIG_SPL_BUILD
		puts("Missing DTB\n");
#else
		puts("No valid device tree binary found - please append one to U-Boot binary, use u-boot-dtb.bin or define CONFIG_OF_EMBED. For sandbox, use -d <file.dtb>\n");
#endif
		return -1;
	}
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
 * @return pointer to cell, which is only valid if err == 0
 */
static const void *get_prop_check_min_len(const void *blob, int node,
		const char *prop_name, int min_len, int *err)
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
	int i, err = 0;

	debug("%s: %s\n", __func__, prop_name);
	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	if (!err) {
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
				if (!node) {
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

int fdtdec_get_config_int(const void *blob, const char *prop_name,
		int default_val)
{
	int config_node;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return default_val;
	return fdtdec_get_int(blob, config_node, prop_name, default_val);
}

int fdtdec_get_config_bool(const void *blob, const char *prop_name)
{
	int config_node;
	const void *prop;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return 0;
	prop = fdt_get_property(blob, config_node, prop_name, NULL);

	return prop != NULL;
}

char *fdtdec_get_config_string(const void *blob, const char *prop_name)
{
	const char *nodep;
	int nodeoffset;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	nodeoffset = fdt_path_offset(blob, "/config");
	if (nodeoffset < 0)
		return NULL;

	nodep = fdt_getprop(blob, nodeoffset, prop_name, &len);
	if (!nodep)
		return NULL;

	return (char *)nodep;
}

int fdtdec_decode_region(const void *blob, int node, const char *prop_name,
			 fdt_addr_t *basep, fdt_size_t *sizep)
{
	const fdt_addr_t *cell;
	int len;

	debug("%s: %s: %s\n", __func__, fdt_get_name(blob, node, NULL),
	      prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell || (len < sizeof(fdt_addr_t) * 2)) {
		debug("cell=%p, len=%d\n", cell, len);
		return -1;
	}

	*basep = fdt_addr_to_cpu(*cell);
	*sizep = fdt_size_to_cpu(cell[1]);
	debug("%s: base=%08lx, size=%lx\n", __func__, (ulong)*basep,
	      (ulong)*sizep);

	return 0;
}

/**
 * Read a flash entry from the fdt
 *
 * @param blob		FDT blob
 * @param node		Offset of node to read
 * @param name		Name of node being read
 * @param entry		Place to put offset and size of this node
 * @return 0 if ok, -ve on error
 */
int fdtdec_read_fmap_entry(const void *blob, int node, const char *name,
			   struct fmap_entry *entry)
{
	const char *prop;
	u32 reg[2];

	if (fdtdec_get_int_array(blob, node, "reg", reg, 2)) {
		debug("Node '%s' has bad/missing 'reg' property\n", name);
		return -FDT_ERR_NOTFOUND;
	}
	entry->offset = reg[0];
	entry->length = reg[1];
	entry->used = fdtdec_get_int(blob, node, "used", entry->length);
	prop = fdt_getprop(blob, node, "compress", NULL);
	entry->compress_algo = prop && !strcmp(prop, "lzo") ?
		FMAP_COMPRESS_LZO : FMAP_COMPRESS_NONE;
	prop = fdt_getprop(blob, node, "hash", &entry->hash_size);
	entry->hash_algo = prop ? FMAP_HASH_SHA256 : FMAP_HASH_NONE;
	entry->hash = (uint8_t *)prop;

	return 0;
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
			res->start = res->end = fdtdec_get_number(ptr, na);
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

	index = fdt_find_string(fdt, node, prop_names, name);
	if (index < 0)
		return index;

	return fdt_get_resource(fdt, node, property, index, res);
}

int fdtdec_decode_memory_region(const void *blob, int config_node,
				const char *mem_type, const char *suffix,
				fdt_addr_t *basep, fdt_size_t *sizep)
{
	char prop_name[50];
	const char *mem;
	fdt_size_t size, offset_size;
	fdt_addr_t base, offset;
	int node;

	if (config_node == -1) {
		config_node = fdt_path_offset(blob, "/config");
		if (config_node < 0) {
			debug("%s: Cannot find /config node\n", __func__);
			return -ENOENT;
		}
	}
	if (!suffix)
		suffix = "";

	snprintf(prop_name, sizeof(prop_name), "%s-memory%s", mem_type,
		 suffix);
	mem = fdt_getprop(blob, config_node, prop_name, NULL);
	if (!mem) {
		debug("%s: No memory type for '%s', using /memory\n", __func__,
		      prop_name);
		mem = "/memory";
	}

	node = fdt_path_offset(blob, mem);
	if (node < 0) {
		debug("%s: Failed to find node '%s': %s\n", __func__, mem,
		      fdt_strerror(node));
		return -ENOENT;
	}

	/*
	 * Not strictly correct - the memory may have multiple banks. We just
	 * use the first
	 */
	if (fdtdec_decode_region(blob, node, "reg", &base, &size)) {
		debug("%s: Failed to decode memory region %s\n", __func__,
		      mem);
		return -EINVAL;
	}

	snprintf(prop_name, sizeof(prop_name), "%s-offset%s", mem_type,
		 suffix);
	if (fdtdec_decode_region(blob, config_node, prop_name, &offset,
				 &offset_size)) {
		debug("%s: Failed to decode memory region '%s'\n", __func__,
		      prop_name);
		return -EINVAL;
	}

	*basep = base + offset;
	*sizep = offset_size;

	return 0;
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

	return 0;
}

int fdtdec_setup(void)
{
#ifdef CONFIG_OF_CONTROL
# ifdef CONFIG_OF_EMBED
	/* Get a pointer to the FDT */
	gd->fdt_blob = __dtb_dt_begin;
# elif defined CONFIG_OF_SEPARATE
#  ifdef CONFIG_SPL_BUILD
	/* FDT is at end of BSS */
	gd->fdt_blob = (ulong *)&__bss_end;
#  else
	/* FDT is at end of image */
	gd->fdt_blob = (ulong *)&_end;
#endif
# elif defined(CONFIG_OF_HOSTFILE)
	if (sandbox_read_fdt_from_file()) {
		puts("Failed to read control FDT\n");
		return -1;
	}
# endif
# ifndef CONFIG_SPL_BUILD
	/* Allow the early environment to override the fdt address */
	gd->fdt_blob = (void *)getenv_ulong("fdtcontroladdr", 16,
						(uintptr_t)gd->fdt_blob);
# endif
#endif
	return fdtdec_prepare_fdt();
}

#endif /* !USE_HOSTCC */
