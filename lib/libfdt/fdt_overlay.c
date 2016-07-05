#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"

static uint32_t overlay_get_target_phandle(const void *fdto, int fragment)
{
	const uint32_t *val;
	int len;

	val = fdt_getprop(fdto, fragment, "target", &len);
	if (!val)
		return 0;

	if ((*val == 0xffffffff) || (len != sizeof(*val)))
		return 0xffffffff;

	return fdt32_to_cpu(*val);
}

static int overlay_get_target(const void *fdt, const void *fdto,
			      int fragment)
{
	uint32_t phandle;
	const char *path;

	/* Try first to do a phandle based lookup */
	phandle = overlay_get_target_phandle(fdto, fragment);
	if (phandle == -1)
		return -FDT_ERR_BADPHANDLE;

	if (phandle)
		return fdt_node_offset_by_phandle(fdt, phandle);

	/* And then a path based lookup */
	path = fdt_getprop(fdto, fragment, "target-path", NULL);
	if (!path)
		return -FDT_ERR_NOTFOUND;

	return fdt_path_offset(fdt, path);
}

static int overlay_phandle_add_offset(void *fdt, int node,
				      const char *name, uint32_t delta)
{
	const uint32_t *val;
	uint32_t adj_val;
	int len;

	val = fdt_getprop(fdt, node, name, &len);
	if (!val)
		return len;

	if (len != sizeof(*val))
		return -FDT_ERR_BADSTRUCTURE;

	adj_val = fdt32_to_cpu(*val);
	if ((adj_val + delta) < adj_val)
		return -FDT_ERR_BADPHANDLE;

	adj_val += delta;
	return fdt_setprop_inplace_u32(fdt, node, name, adj_val);
}

static int overlay_adjust_node_phandles(void *fdto, int node,
					uint32_t delta)
{
	bool found = false;
	int child;
	int ret;

	ret = overlay_phandle_add_offset(fdto, node, "phandle", delta);
	if (ret && ret != -FDT_ERR_NOTFOUND)
		return ret;

	if (!ret)
		found = true;

	ret = overlay_phandle_add_offset(fdto, node, "linux,phandle", delta);
	if (ret && ret != -FDT_ERR_NOTFOUND)
		return ret;

	/*
	 * If neither phandle nor linux,phandle have been found return
	 * an error.
	 */
	if (!found && !ret)
		return ret;

	fdt_for_each_subnode(fdto, child, node)
		overlay_adjust_node_phandles(fdto, child, delta);

	return 0;
}

static int overlay_adjust_local_phandles(void *fdto, uint32_t delta)
{
	/*
	 * Start adjusting the phandles from the overlay root
	 */
	return overlay_adjust_node_phandles(fdto, 0, delta);
}

static int overlay_update_local_node_references(void *fdto,
						int tree_node,
						int fixup_node,
						uint32_t delta)
{
	int fixup_prop;
	int fixup_child;
	int ret;

	fdt_for_each_property_offset(fixup_prop, fdto, fixup_node) {
		const uint32_t *val = NULL;
		uint32_t adj_val, index;
		const char *name;
		int fixup_len;
		int tree_len;

		val = fdt_getprop_by_offset(fdto, fixup_prop,
					    &name, &fixup_len);
		if (!val)
			return fixup_len;
		index = fdt32_to_cpu(*val);

		val = fdt_getprop(fdto, tree_node, name, &tree_len);
		if (!val)
			return tree_len;

		/*
		 * The index can be unaligned.
		 *
		 * Use a memcpy for the architectures that do not
		 * support unaligned accesses.
		 */
		memcpy(&adj_val, (unsigned char *)val + index,
		       sizeof(uint32_t));

		adj_val = fdt32_to_cpu(adj_val);
		adj_val += delta;
		adj_val = cpu_to_fdt32(adj_val);

		ret = fdt_setprop_inplace_namelen_by_index(fdto, tree_node,
							   name, strlen(name),
							   index, &adj_val,
							   sizeof(adj_val));
		if (ret)
			return ret;
	}

	fdt_for_each_subnode(fdto, fixup_child, fixup_node) {
		const char *fixup_child_name = fdt_get_name(fdto, fixup_child,
							    NULL);
		int tree_child;

		tree_child = fdt_subnode_offset(fdto, tree_node,
						fixup_child_name);
		if (tree_child < 0)
			return tree_child;

		ret = overlay_update_local_node_references(fdto,
							   tree_child,
							   fixup_child,
							   delta);
		if (ret)
			return ret;
	}

	return 0;
}

static int overlay_update_local_references(void *dto, uint32_t delta)
{
	int fixups;

	fixups = fdt_path_offset(dto, "/__local_fixups__");
	if (fixups < 0) {
		/* There's no local phandles to adjust, bail out */
		if (fixups == -FDT_ERR_NOTFOUND)
			return 0;

		return fixups;
	}

	/*
	 * Update our local references from the root of the tree
	 */
	return overlay_update_local_node_references(dto, 0, fixups,
						    delta);
}

static int overlay_fixup_one_phandle(void *fdt, void *fdto,
				     int symbols_off,
				     const char *path, uint32_t path_len,
				     const char *name, uint32_t name_len,
				     int index, const char *label)
{
	const char *symbol_path;
	uint32_t phandle;
	int symbol_off, fixup_off;
	int prop_len;

	symbol_path = fdt_getprop(fdt, symbols_off, label,
				  &prop_len);
	if (!symbol_path)
		return -FDT_ERR_NOTFOUND;

	symbol_off = fdt_path_offset(fdt, symbol_path);
	if (symbol_off < 0)
		return symbol_off;

	phandle = fdt_get_phandle(fdt, symbol_off);
	if (!phandle)
		return -FDT_ERR_NOTFOUND;

	fixup_off = fdt_path_offset_namelen(fdto, path, path_len);
	if (fixup_off < 0)
		return fixup_off;

	phandle = cpu_to_fdt32(phandle);
	return fdt_setprop_inplace_namelen_by_index(fdto, fixup_off,
						    name, name_len, index,
						    &phandle, sizeof(phandle));
};

static int overlay_fixup_phandle(void *fdt, void *fdto, int symbols_off,
				 int property)
{
	const char *value;
	const char *label;
	int len;

	value = fdt_getprop_by_offset(fdto, property,
				      &label, &len);
	if (!value)
		return len;

	do {
		const char *prop_string = value;
		const char *path, *name;
		uint32_t prop_len = strlen(value);
		uint32_t path_len, name_len;
		char *sep, *endptr;
		int index;
		int ret;

		path = prop_string;
		sep = memchr(prop_string, ':', prop_len);
		if (*sep != ':')
			return -FDT_ERR_BADSTRUCTURE;
		path_len = sep - path;

		name = sep + 1;
		sep = memchr(name, ':', prop_len);
		if (*sep != ':')
			return -FDT_ERR_BADSTRUCTURE;
		name_len = sep - name;

		index = strtoul(sep + 1, &endptr, 10);
		if ((*endptr != '\0') || (endptr <= (sep + 1)))
			return -FDT_ERR_BADSTRUCTURE;

		len -= prop_len + 1;
		value += prop_len + 1;

		ret = overlay_fixup_one_phandle(fdt, fdto, symbols_off,
						path, path_len, name, name_len,
						index, label);
		if (ret)
			return ret;
	} while (len > 0);

	return 0;
}

static int overlay_fixup_phandles(void *dt, void *dto)
{
	int fixups_off, symbols_off;
	int property;

	symbols_off = fdt_path_offset(dt, "/__symbols__");
	fixups_off = fdt_path_offset(dto, "/__fixups__");

	fdt_for_each_property_offset(property, dto, fixups_off)
		overlay_fixup_phandle(dt, dto, symbols_off, property);

	return 0;
}

static int apply_overlay_node(void *dt, int target,
			      void *dto, int fragment)
{
	int property;
	int node;

	fdt_for_each_property_offset(property, dto, fragment) {
		const char *name;
		const void *prop;
		int prop_len;
		int ret;

		prop = fdt_getprop_by_offset(dto, property, &name,
					     &prop_len);
		if (prop_len == -FDT_ERR_NOTFOUND)
			return -FDT_ERR_INTERNAL;
		if (prop_len < 0)
			return prop_len;

		ret = fdt_setprop(dt, target, name, prop, prop_len);
		if (ret)
			return ret;
	}

	fdt_for_each_subnode(dto, node, fragment) {
		const char *name = fdt_get_name(dto, node, NULL);
		int nnode;
		int ret;

		nnode = fdt_add_subnode(dt, target, name);
		if (nnode == -FDT_ERR_EXISTS)
			nnode = fdt_subnode_offset(dt, target, name);

		if (nnode < 0)
			return nnode;

		ret = apply_overlay_node(dt, nnode, dto, node);
		if (ret)
			return ret;
	}

	return 0;
}

static int overlay_merge(void *dt, void *dto)
{
	int fragment;

	fdt_for_each_subnode(dto, fragment, 0) {
		int overlay;
		int target;
		int ret;

		target = overlay_get_target(dt, dto, fragment);
		if (target < 0)
			continue;

		overlay = fdt_subnode_offset(dto, fragment, "__overlay__");
		if (overlay < 0)
			return overlay;

		ret = apply_overlay_node(dt, target, dto, overlay);
		if (ret)
			return ret;
	}

	return 0;
}

int fdt_overlay_apply(void *fdt, void *fdto)
{
	uint32_t delta = fdt_get_max_phandle(fdt) + 1;
	int ret;

	FDT_CHECK_HEADER(fdt);
	FDT_CHECK_HEADER(fdto);

	ret = overlay_adjust_local_phandles(fdto, delta);
	if (ret)
		goto err;

	ret = overlay_update_local_references(fdto, delta);
	if (ret)
		goto err;

	ret = overlay_fixup_phandles(fdt, fdto);
	if (ret)
		goto err;

	ret = overlay_merge(fdt, fdto);
	if (ret)
		goto err;

	/*
	 * The overlay has been damaged, erase its magic.
	 */
	fdt_set_magic(fdto, ~0);

	return 0;

err:
	/*
	 * The overlay might have been damaged, erase its magic.
	 */
	fdt_set_magic(fdto, ~0);

	/*
	 * The base device tree might have been damaged, erase its
	 * magic.
	 */
	fdt_set_magic(fdt, ~0);

	return ret;
}
