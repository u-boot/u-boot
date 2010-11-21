/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * Copyright 2010 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <stdio_dev.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <exports.h>

/*
 * Global data (for the gd->bd)
 */
DECLARE_GLOBAL_DATA_PTR;

/**
 * fdt_getprop_u32_default - Find a node and return it's property or a default
 *
 * @fdt: ptr to device tree
 * @path: path of node
 * @prop: property name
 * @dflt: default value if the property isn't found
 *
 * Convenience function to find a node and return it's property or a
 * default value if it doesn't exist.
 */
u32 fdt_getprop_u32_default(void *fdt, const char *path, const char *prop,
				const u32 dflt)
{
	const u32 *val;
	int off;

	off = fdt_path_offset(fdt, path);
	if (off < 0)
		return dflt;

	val = fdt_getprop(fdt, off, prop, NULL);
	if (val)
		return *val;
	else
		return dflt;
}

/**
 * fdt_find_and_setprop: Find a node and set it's property
 *
 * @fdt: ptr to device tree
 * @node: path of node
 * @prop: property name
 * @val: ptr to new value
 * @len: length of new property value
 * @create: flag to create the property if it doesn't exist
 *
 * Convenience function to directly set a property given the path to the node.
 */
int fdt_find_and_setprop(void *fdt, const char *node, const char *prop,
			 const void *val, int len, int create)
{
	int nodeoff = fdt_path_offset(fdt, node);

	if (nodeoff < 0)
		return nodeoff;

	if ((!create) && (fdt_get_property(fdt, nodeoff, prop, 0) == NULL))
		return 0; /* create flag not set; so exit quietly */

	return fdt_setprop(fdt, nodeoff, prop, val, len);
}

#ifdef CONFIG_OF_STDOUT_VIA_ALIAS

#ifdef CONFIG_SERIAL_MULTI
static void fdt_fill_multisername(char *sername, size_t maxlen)
{
	const char *outname = stdio_devices[stdout]->name;

	if (strcmp(outname, "serial") > 0)
		strncpy(sername, outname, maxlen);

	/* eserial? */
	if (strcmp(outname + 1, "serial") > 0)
		strncpy(sername, outname + 1, maxlen);
}
#else
static inline void fdt_fill_multisername(char *sername, size_t maxlen) {}
#endif /* CONFIG_SERIAL_MULTI */

static int fdt_fixup_stdout(void *fdt, int chosenoff)
{
	int err = 0;
#ifdef CONFIG_CONS_INDEX
	int node;
	char sername[9] = { 0 };
	const char *path;

	fdt_fill_multisername(sername, sizeof(sername) - 1);
	if (!sername[0])
		sprintf(sername, "serial%d", CONFIG_CONS_INDEX - 1);

	err = node = fdt_path_offset(fdt, "/aliases");
	if (node >= 0) {
		int len;
		path = fdt_getprop(fdt, node, sername, &len);
		if (path) {
			char *p = malloc(len);
			err = -FDT_ERR_NOSPACE;
			if (p) {
				memcpy(p, path, len);
				err = fdt_setprop(fdt, chosenoff,
					"linux,stdout-path", p, len);
				free(p);
			}
		} else {
			err = len;
		}
	}
#endif
	if (err < 0)
		printf("WARNING: could not set linux,stdout-path %s.\n",
				fdt_strerror(err));

	return err;
}
#endif

int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end, int force)
{
	int   nodeoffset;
	int   err, j, total;
	u32   tmp;
	const char *path;
	uint64_t addr, size;

	/* Find the "chosen" node.  */
	nodeoffset = fdt_path_offset (fdt, "/chosen");

	/* If there is no "chosen" node in the blob return */
	if (nodeoffset < 0) {
		printf("fdt_initrd: %s\n", fdt_strerror(nodeoffset));
		return nodeoffset;
	}

	/* just return if initrd_start/end aren't valid */
	if ((initrd_start == 0) || (initrd_end == 0))
		return 0;

	total = fdt_num_mem_rsv(fdt);

	/*
	 * Look for an existing entry and update it.  If we don't find
	 * the entry, we will j be the next available slot.
	 */
	for (j = 0; j < total; j++) {
		err = fdt_get_mem_rsv(fdt, j, &addr, &size);
		if (addr == initrd_start) {
			fdt_del_mem_rsv(fdt, j);
			break;
		}
	}

	err = fdt_add_mem_rsv(fdt, initrd_start, initrd_end - initrd_start + 1);
	if (err < 0) {
		printf("fdt_initrd: %s\n", fdt_strerror(err));
		return err;
	}

	path = fdt_getprop(fdt, nodeoffset, "linux,initrd-start", NULL);
	if ((path == NULL) || force) {
		tmp = __cpu_to_be32(initrd_start);
		err = fdt_setprop(fdt, nodeoffset,
			"linux,initrd-start", &tmp, sizeof(tmp));
		if (err < 0) {
			printf("WARNING: "
				"could not set linux,initrd-start %s.\n",
				fdt_strerror(err));
			return err;
		}
		tmp = __cpu_to_be32(initrd_end);
		err = fdt_setprop(fdt, nodeoffset,
			"linux,initrd-end", &tmp, sizeof(tmp));
		if (err < 0) {
			printf("WARNING: could not set linux,initrd-end %s.\n",
				fdt_strerror(err));

			return err;
		}
	}

	return 0;
}

int fdt_chosen(void *fdt, int force)
{
	int   nodeoffset;
	int   err;
	char  *str;		/* used to set string properties */
	const char *path;

	err = fdt_check_header(fdt);
	if (err < 0) {
		printf("fdt_chosen: %s\n", fdt_strerror(err));
		return err;
	}

	/*
	 * Find the "chosen" node.
	 */
	nodeoffset = fdt_path_offset (fdt, "/chosen");

	/*
	 * If there is no "chosen" node in the blob, create it.
	 */
	if (nodeoffset < 0) {
		/*
		 * Create a new node "/chosen" (offset 0 is root level)
		 */
		nodeoffset = fdt_add_subnode(fdt, 0, "chosen");
		if (nodeoffset < 0) {
			printf("WARNING: could not create /chosen %s.\n",
				fdt_strerror(nodeoffset));
			return nodeoffset;
		}
	}

	/*
	 * Create /chosen properites that don't exist in the fdt.
	 * If the property exists, update it only if the "force" parameter
	 * is true.
	 */
	str = getenv("bootargs");
	if (str != NULL) {
		path = fdt_getprop(fdt, nodeoffset, "bootargs", NULL);
		if ((path == NULL) || force) {
			err = fdt_setprop(fdt, nodeoffset,
				"bootargs", str, strlen(str)+1);
			if (err < 0)
				printf("WARNING: could not set bootargs %s.\n",
					fdt_strerror(err));
		}
	}

#ifdef CONFIG_OF_STDOUT_VIA_ALIAS
	path = fdt_getprop(fdt, nodeoffset, "linux,stdout-path", NULL);
	if ((path == NULL) || force)
		err = fdt_fixup_stdout(fdt, nodeoffset);
#endif

#ifdef OF_STDOUT_PATH
	path = fdt_getprop(fdt, nodeoffset, "linux,stdout-path", NULL);
	if ((path == NULL) || force) {
		err = fdt_setprop(fdt, nodeoffset,
			"linux,stdout-path", OF_STDOUT_PATH, strlen(OF_STDOUT_PATH)+1);
		if (err < 0)
			printf("WARNING: could not set linux,stdout-path %s.\n",
				fdt_strerror(err));
	}
#endif

	return err;
}

void do_fixup_by_path(void *fdt, const char *path, const char *prop,
		      const void *val, int len, int create)
{
#if defined(DEBUG)
	int i;
	debug("Updating property '%s/%s' = ", path, prop);
	for (i = 0; i < len; i++)
		debug(" %.2x", *(u8*)(val+i));
	debug("\n");
#endif
	int rc = fdt_find_and_setprop(fdt, path, prop, val, len, create);
	if (rc)
		printf("Unable to update property %s:%s, err=%s\n",
			path, prop, fdt_strerror(rc));
}

void do_fixup_by_path_u32(void *fdt, const char *path, const char *prop,
			  u32 val, int create)
{
	val = cpu_to_fdt32(val);
	do_fixup_by_path(fdt, path, prop, &val, sizeof(val), create);
}

void do_fixup_by_prop(void *fdt,
		      const char *pname, const void *pval, int plen,
		      const char *prop, const void *val, int len,
		      int create)
{
	int off;
#if defined(DEBUG)
	int i;
	debug("Updating property '%s' = ", prop);
	for (i = 0; i < len; i++)
		debug(" %.2x", *(u8*)(val+i));
	debug("\n");
#endif
	off = fdt_node_offset_by_prop_value(fdt, -1, pname, pval, plen);
	while (off != -FDT_ERR_NOTFOUND) {
		if (create || (fdt_get_property(fdt, off, prop, 0) != NULL))
			fdt_setprop(fdt, off, prop, val, len);
		off = fdt_node_offset_by_prop_value(fdt, off, pname, pval, plen);
	}
}

void do_fixup_by_prop_u32(void *fdt,
			  const char *pname, const void *pval, int plen,
			  const char *prop, u32 val, int create)
{
	val = cpu_to_fdt32(val);
	do_fixup_by_prop(fdt, pname, pval, plen, prop, &val, 4, create);
}

void do_fixup_by_compat(void *fdt, const char *compat,
			const char *prop, const void *val, int len, int create)
{
	int off = -1;
#if defined(DEBUG)
	int i;
	debug("Updating property '%s' = ", prop);
	for (i = 0; i < len; i++)
		debug(" %.2x", *(u8*)(val+i));
	debug("\n");
#endif
	off = fdt_node_offset_by_compatible(fdt, -1, compat);
	while (off != -FDT_ERR_NOTFOUND) {
		if (create || (fdt_get_property(fdt, off, prop, 0) != NULL))
			fdt_setprop(fdt, off, prop, val, len);
		off = fdt_node_offset_by_compatible(fdt, off, compat);
	}
}

void do_fixup_by_compat_u32(void *fdt, const char *compat,
			    const char *prop, u32 val, int create)
{
	val = cpu_to_fdt32(val);
	do_fixup_by_compat(fdt, compat, prop, &val, 4, create);
}

/*
 * Get cells len in bytes
 *     if #NNNN-cells property is 2 then len is 8
 *     otherwise len is 4
 */
static int get_cells_len(void *blob, char *nr_cells_name)
{
	const u32 *cell;

	cell = fdt_getprop(blob, 0, nr_cells_name, NULL);
	if (cell && *cell == 2)
		return 8;

	return 4;
}

/*
 * Write a 4 or 8 byte big endian cell
 */
static void write_cell(u8 *addr, u64 val, int size)
{
	int shift = (size - 1) * 8;
	while (size-- > 0) {
		*addr++ = (val >> shift) & 0xff;
		shift -= 8;
	}
}

int fdt_fixup_memory_banks(void *blob, u64 start[], u64 size[], int banks)
{
	int err, nodeoffset;
	int addr_cell_len, size_cell_len, len;
	u8 tmp[banks * 8];
	int bank;

	err = fdt_check_header(blob);
	if (err < 0) {
		printf("%s: %s\n", __FUNCTION__, fdt_strerror(err));
		return err;
	}

	/* update, or add and update /memory node */
	nodeoffset = fdt_path_offset(blob, "/memory");
	if (nodeoffset < 0) {
		nodeoffset = fdt_add_subnode(blob, 0, "memory");
		if (nodeoffset < 0)
			printf("WARNING: could not create /memory: %s.\n",
					fdt_strerror(nodeoffset));
		return nodeoffset;
	}
	err = fdt_setprop(blob, nodeoffset, "device_type", "memory",
			sizeof("memory"));
	if (err < 0) {
		printf("WARNING: could not set %s %s.\n", "device_type",
				fdt_strerror(err));
		return err;
	}

	addr_cell_len = get_cells_len(blob, "#address-cells");
	size_cell_len = get_cells_len(blob, "#size-cells");

	for (bank = 0, len = 0; bank < banks; bank++) {
		write_cell(tmp + len, start[bank], addr_cell_len);
		len += addr_cell_len;

		write_cell(tmp + len, size[bank], size_cell_len);
		len += size_cell_len;
	}

	err = fdt_setprop(blob, nodeoffset, "reg", tmp, len);
	if (err < 0) {
		printf("WARNING: could not set %s %s.\n",
				"reg", fdt_strerror(err));
		return err;
	}
	return 0;
}

int fdt_fixup_memory(void *blob, u64 start, u64 size)
{
	return fdt_fixup_memory_banks(blob, &start, &size, 1);
}

void fdt_fixup_ethernet(void *fdt)
{
	int node, i, j;
	char enet[16], *tmp, *end;
	char mac[16] = "ethaddr";
	const char *path;
	unsigned char mac_addr[6];

	node = fdt_path_offset(fdt, "/aliases");
	if (node < 0)
		return;

	i = 0;
	while ((tmp = getenv(mac)) != NULL) {
		sprintf(enet, "ethernet%d", i);
		path = fdt_getprop(fdt, node, enet, NULL);
		if (!path) {
			debug("No alias for %s\n", enet);
			sprintf(mac, "eth%daddr", ++i);
			continue;
		}

		for (j = 0; j < 6; j++) {
			mac_addr[j] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
			if (tmp)
				tmp = (*end) ? end+1 : end;
		}

		do_fixup_by_path(fdt, path, "mac-address", &mac_addr, 6, 0);
		do_fixup_by_path(fdt, path, "local-mac-address",
				&mac_addr, 6, 1);

		sprintf(mac, "eth%daddr", ++i);
	}
}

/* Resize the fdt to its actual size + a bit of padding */
int fdt_resize(void *blob)
{
	int i;
	uint64_t addr, size;
	int total, ret;
	uint actualsize;

	if (!blob)
		return 0;

	total = fdt_num_mem_rsv(blob);
	for (i = 0; i < total; i++) {
		fdt_get_mem_rsv(blob, i, &addr, &size);
		if (addr == (uint64_t)(u32)blob) {
			fdt_del_mem_rsv(blob, i);
			break;
		}
	}

	/*
	 * Calculate the actual size of the fdt
	 * plus the size needed for 5 fdt_add_mem_rsv, one
	 * for the fdt itself and 4 for a possible initrd
	 * ((initrd-start + initrd-end) * 2 (name & value))
	 */
	actualsize = fdt_off_dt_strings(blob) +
		fdt_size_dt_strings(blob) + 5 * sizeof(struct fdt_reserve_entry);

	/* Make it so the fdt ends on a page boundary */
	actualsize = ALIGN(actualsize + ((uint)blob & 0xfff), 0x1000);
	actualsize = actualsize - ((uint)blob & 0xfff);

	/* Change the fdt header to reflect the correct size */
	fdt_set_totalsize(blob, actualsize);

	/* Add the new reservation */
	ret = fdt_add_mem_rsv(blob, (uint)blob, actualsize);
	if (ret < 0)
		return ret;

	return actualsize;
}

#ifdef CONFIG_PCI
#define CONFIG_SYS_PCI_NR_INBOUND_WIN 4

#define FDT_PCI_PREFETCH	(0x40000000)
#define FDT_PCI_MEM32		(0x02000000)
#define FDT_PCI_IO		(0x01000000)
#define FDT_PCI_MEM64		(0x03000000)

int fdt_pci_dma_ranges(void *blob, int phb_off, struct pci_controller *hose) {

	int addrcell, sizecell, len, r;
	u32 *dma_range;
	/* sized based on pci addr cells, size-cells, & address-cells */
	u32 dma_ranges[(3 + 2 + 2) * CONFIG_SYS_PCI_NR_INBOUND_WIN];

	addrcell = fdt_getprop_u32_default(blob, "/", "#address-cells", 1);
	sizecell = fdt_getprop_u32_default(blob, "/", "#size-cells", 1);

	dma_range = &dma_ranges[0];
	for (r = 0; r < hose->region_count; r++) {
		u64 bus_start, phys_start, size;

		/* skip if !PCI_REGION_SYS_MEMORY */
		if (!(hose->regions[r].flags & PCI_REGION_SYS_MEMORY))
			continue;

		bus_start = (u64)hose->regions[r].bus_start;
		phys_start = (u64)hose->regions[r].phys_start;
		size = (u64)hose->regions[r].size;

		dma_range[0] = 0;
		if (size >= 0x100000000ull)
			dma_range[0] |= FDT_PCI_MEM64;
		else
			dma_range[0] |= FDT_PCI_MEM32;
		if (hose->regions[r].flags & PCI_REGION_PREFETCH)
			dma_range[0] |= FDT_PCI_PREFETCH;
#ifdef CONFIG_SYS_PCI_64BIT
		dma_range[1] = bus_start >> 32;
#else
		dma_range[1] = 0;
#endif
		dma_range[2] = bus_start & 0xffffffff;

		if (addrcell == 2) {
			dma_range[3] = phys_start >> 32;
			dma_range[4] = phys_start & 0xffffffff;
		} else {
			dma_range[3] = phys_start & 0xffffffff;
		}

		if (sizecell == 2) {
			dma_range[3 + addrcell + 0] = size >> 32;
			dma_range[3 + addrcell + 1] = size & 0xffffffff;
		} else {
			dma_range[3 + addrcell + 0] = size & 0xffffffff;
		}

		dma_range += (3 + addrcell + sizecell);
	}

	len = dma_range - &dma_ranges[0];
	if (len)
		fdt_setprop(blob, phb_off, "dma-ranges", &dma_ranges[0], len*4);

	return 0;
}
#endif

#ifdef CONFIG_FDT_FIXUP_NOR_FLASH_SIZE
/*
 * Provide a weak default function to return the flash bank size.
 * There might be multiple non-identical flash chips connected to one
 * chip-select, so we need to pass an index as well.
 */
u32 __flash_get_bank_size(int cs, int idx)
{
	extern flash_info_t flash_info[];

	/*
	 * As default, a simple 1:1 mapping is provided. Boards with
	 * a different mapping need to supply a board specific mapping
	 * routine.
	 */
	return flash_info[cs].size;
}
u32 flash_get_bank_size(int cs, int idx)
	__attribute__((weak, alias("__flash_get_bank_size")));

/*
 * This function can be used to update the size in the "reg" property
 * of all NOR FLASH device nodes. This is necessary for boards with
 * non-fixed NOR FLASH sizes.
 */
int fdt_fixup_nor_flash_size(void *blob)
{
	char compat[][16] = { "cfi-flash", "jedec-flash" };
	int off;
	int len;
	struct fdt_property *prop;
	u32 *reg, *reg2;
	int i;

	for (i = 0; i < 2; i++) {
		off = fdt_node_offset_by_compatible(blob, -1, compat[i]);
		while (off != -FDT_ERR_NOTFOUND) {
			int idx;

			/*
			 * Found one compatible node, so fixup the size
			 * int its reg properties
			 */
			prop = fdt_get_property_w(blob, off, "reg", &len);
			if (prop) {
				int tuple_size = 3 * sizeof(reg);

				/*
				 * There might be multiple reg-tuples,
				 * so loop through them all
				 */
				reg = reg2 = (u32 *)&prop->data[0];
				for (idx = 0; idx < (len / tuple_size); idx++) {
					/*
					 * Update size in reg property
					 */
					reg[2] = flash_get_bank_size(reg[0],
								     idx);

					/*
					 * Point to next reg tuple
					 */
					reg += 3;
				}

				fdt_setprop(blob, off, "reg", reg2, len);
			}

			/* Move to next compatible node */
			off = fdt_node_offset_by_compatible(blob, off,
							    compat[i]);
		}
	}

	return 0;
}
#endif

int fdt_increase_size(void *fdt, int add_len)
{
	int newlen;

	newlen = fdt_totalsize(fdt) + add_len;

	/* Open in place with a new len */
	return fdt_open_into(fdt, fdt, newlen);
}

#ifdef CONFIG_FDT_FIXUP_PARTITIONS
#include <jffs2/load_kernel.h>
#include <mtd_node.h>

struct reg_cell {
	unsigned int r0;
	unsigned int r1;
};

int fdt_del_subnodes(const void *blob, int parent_offset)
{
	int off, ndepth;
	int ret;

	for (ndepth = 0, off = fdt_next_node(blob, parent_offset, &ndepth);
	     (off >= 0) && (ndepth > 0);
	     off = fdt_next_node(blob, off, &ndepth)) {
		if (ndepth == 1) {
			debug("delete %s: offset: %x\n",
				fdt_get_name(blob, off, 0), off);
			ret = fdt_del_node((void *)blob, off);
			if (ret < 0) {
				printf("Can't delete node: %s\n",
					fdt_strerror(ret));
				return ret;
			} else {
				ndepth = 0;
				off = parent_offset;
			}
		}
	}
	return 0;
}

int fdt_del_partitions(void *blob, int parent_offset)
{
	const void *prop;
	int ndepth = 0;
	int off;
	int ret;

	off = fdt_next_node(blob, parent_offset, &ndepth);
	if (off > 0 && ndepth == 1) {
		prop = fdt_getprop(blob, off, "label", NULL);
		if (prop == NULL) {
			/*
			 * Could not find label property, nand {}; node?
			 * Check subnode, delete partitions there if any.
			 */
			return fdt_del_partitions(blob, off);
		} else {
			ret = fdt_del_subnodes(blob, parent_offset);
			if (ret < 0) {
				printf("Can't remove subnodes: %s\n",
					fdt_strerror(ret));
				return ret;
			}
		}
	}
	return 0;
}

int fdt_node_set_part_info(void *blob, int parent_offset,
			   struct mtd_device *dev)
{
	struct list_head *pentry;
	struct part_info *part;
	struct reg_cell cell;
	int off, ndepth = 0;
	int part_num, ret;
	char buf[64];

	ret = fdt_del_partitions(blob, parent_offset);
	if (ret < 0)
		return ret;

	/*
	 * Check if it is nand {}; subnode, adjust
	 * the offset in this case
	 */
	off = fdt_next_node(blob, parent_offset, &ndepth);
	if (off > 0 && ndepth == 1)
		parent_offset = off;

	part_num = 0;
	list_for_each_prev(pentry, &dev->parts) {
		int newoff;

		part = list_entry(pentry, struct part_info, link);

		debug("%2d: %-20s0x%08x\t0x%08x\t%d\n",
			part_num, part->name, part->size,
			part->offset, part->mask_flags);

		sprintf(buf, "partition@%x", part->offset);
add_sub:
		ret = fdt_add_subnode(blob, parent_offset, buf);
		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_sub;
			else
				goto err_size;
		} else if (ret < 0) {
			printf("Can't add partition node: %s\n",
				fdt_strerror(ret));
			return ret;
		}
		newoff = ret;

		/* Check MTD_WRITEABLE_CMD flag */
		if (part->mask_flags & 1) {
add_ro:
			ret = fdt_setprop(blob, newoff, "read_only", NULL, 0);
			if (ret == -FDT_ERR_NOSPACE) {
				ret = fdt_increase_size(blob, 512);
				if (!ret)
					goto add_ro;
				else
					goto err_size;
			} else if (ret < 0)
				goto err_prop;
		}

		cell.r0 = cpu_to_fdt32(part->offset);
		cell.r1 = cpu_to_fdt32(part->size);
add_reg:
		ret = fdt_setprop(blob, newoff, "reg", &cell, sizeof(cell));
		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_reg;
			else
				goto err_size;
		} else if (ret < 0)
			goto err_prop;

add_label:
		ret = fdt_setprop_string(blob, newoff, "label", part->name);
		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_label;
			else
				goto err_size;
		} else if (ret < 0)
			goto err_prop;

		part_num++;
	}
	return 0;
err_size:
	printf("Can't increase blob size: %s\n", fdt_strerror(ret));
	return ret;
err_prop:
	printf("Can't add property: %s\n", fdt_strerror(ret));
	return ret;
}

/*
 * Update partitions in nor/nand nodes using info from
 * mtdparts environment variable. The nodes to update are
 * specified by node_info structure which contains mtd device
 * type and compatible string: E. g. the board code in
 * ft_board_setup() could use:
 *
 *	struct node_info nodes[] = {
 *		{ "fsl,mpc5121-nfc",    MTD_DEV_TYPE_NAND, },
 *		{ "cfi-flash",          MTD_DEV_TYPE_NOR,  },
 *	};
 *
 *	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
 */
void fdt_fixup_mtdparts(void *blob, void *node_info, int node_info_size)
{
	struct node_info *ni = node_info;
	struct mtd_device *dev;
	char *parts;
	int i, idx;
	int noff;

	parts = getenv("mtdparts");
	if (!parts)
		return;

	if (mtdparts_init() != 0)
		return;

	for (i = 0; i < node_info_size; i++) {
		idx = 0;
		noff = fdt_node_offset_by_compatible(blob, -1, ni[i].compat);
		while (noff != -FDT_ERR_NOTFOUND) {
			debug("%s: %s, mtd dev type %d\n",
				fdt_get_name(blob, noff, 0),
				ni[i].compat, ni[i].type);
			dev = device_find(ni[i].type, idx++);
			if (dev) {
				if (fdt_node_set_part_info(blob, noff, dev))
					return; /* return on error */
			}

			/* Jump to next flash node */
			noff = fdt_node_offset_by_compatible(blob, noff,
							     ni[i].compat);
		}
	}
}
#endif

void fdt_del_node_and_alias(void *blob, const char *alias)
{
	int off = fdt_path_offset(blob, alias);

	if (off < 0)
		return;

	fdt_del_node(blob, off);

	off = fdt_path_offset(blob, "/aliases");
	fdt_delprop(blob, off, alias);
}

/* Helper to read a big number; size is in cells (not bytes) */
static inline u64 of_read_number(const __be32 *cell, int size)
{
	u64 r = 0;
	while (size--)
		r = (r << 32) | be32_to_cpu(*(cell++));
	return r;
}

#define PRu64	"%llx"

/* Max address size we deal with */
#define OF_MAX_ADDR_CELLS	4
#define OF_BAD_ADDR	((u64)-1)
#define OF_CHECK_COUNTS(na, ns)	((na) > 0 && (na) <= OF_MAX_ADDR_CELLS && \
			(ns) > 0)

/* Debug utility */
#ifdef DEBUG
static void of_dump_addr(const char *s, const u32 *addr, int na)
{
	printf("%s", s);
	while(na--)
		printf(" %08x", *(addr++));
	printf("\n");
}
#else
static void of_dump_addr(const char *s, const u32 *addr, int na) { }
#endif

/* Callbacks for bus specific translators */
struct of_bus {
	const char	*name;
	const char	*addresses;
	void		(*count_cells)(void *blob, int parentoffset,
				int *addrc, int *sizec);
	u64		(*map)(u32 *addr, const u32 *range,
				int na, int ns, int pna);
	int		(*translate)(u32 *addr, u64 offset, int na);
};

/* Default translator (generic bus) */
static void of_bus_default_count_cells(void *blob, int parentoffset,
					int *addrc, int *sizec)
{
	const u32 *prop;

	if (addrc) {
		prop = fdt_getprop(blob, parentoffset, "#address-cells", NULL);
		if (prop)
			*addrc = be32_to_cpup((u32 *)prop);
		else
			*addrc = 2;
	}

	if (sizec) {
		prop = fdt_getprop(blob, parentoffset, "#size-cells", NULL);
		if (prop)
			*sizec = be32_to_cpup((u32 *)prop);
		else
			*sizec = 1;
	}
}

static u64 of_bus_default_map(u32 *addr, const u32 *range,
		int na, int ns, int pna)
{
	u64 cp, s, da;

	cp = of_read_number(range, na);
	s  = of_read_number(range + na + pna, ns);
	da = of_read_number(addr, na);

	debug("OF: default map, cp="PRu64", s="PRu64", da="PRu64"\n",
	    cp, s, da);

	if (da < cp || da >= (cp + s))
		return OF_BAD_ADDR;
	return da - cp;
}

static int of_bus_default_translate(u32 *addr, u64 offset, int na)
{
	u64 a = of_read_number(addr, na);
	memset(addr, 0, na * 4);
	a += offset;
	if (na > 1)
		addr[na - 2] = a >> 32;
	addr[na - 1] = a & 0xffffffffu;

	return 0;
}

/* Array of bus specific translators */
static struct of_bus of_busses[] = {
	/* Default */
	{
		.name = "default",
		.addresses = "reg",
		.count_cells = of_bus_default_count_cells,
		.map = of_bus_default_map,
		.translate = of_bus_default_translate,
	},
};

static int of_translate_one(void * blob, int parent, struct of_bus *bus,
			    struct of_bus *pbus, u32 *addr,
			    int na, int ns, int pna, const char *rprop)
{
	const u32 *ranges;
	int rlen;
	int rone;
	u64 offset = OF_BAD_ADDR;

	/* Normally, an absence of a "ranges" property means we are
	 * crossing a non-translatable boundary, and thus the addresses
	 * below the current not cannot be converted to CPU physical ones.
	 * Unfortunately, while this is very clear in the spec, it's not
	 * what Apple understood, and they do have things like /uni-n or
	 * /ht nodes with no "ranges" property and a lot of perfectly
	 * useable mapped devices below them. Thus we treat the absence of
	 * "ranges" as equivalent to an empty "ranges" property which means
	 * a 1:1 translation at that level. It's up to the caller not to try
	 * to translate addresses that aren't supposed to be translated in
	 * the first place. --BenH.
	 */
	ranges = (u32 *)fdt_getprop(blob, parent, rprop, &rlen);
	if (ranges == NULL || rlen == 0) {
		offset = of_read_number(addr, na);
		memset(addr, 0, pna * 4);
		debug("OF: no ranges, 1:1 translation\n");
		goto finish;
	}

	debug("OF: walking ranges...\n");

	/* Now walk through the ranges */
	rlen /= 4;
	rone = na + pna + ns;
	for (; rlen >= rone; rlen -= rone, ranges += rone) {
		offset = bus->map(addr, ranges, na, ns, pna);
		if (offset != OF_BAD_ADDR)
			break;
	}
	if (offset == OF_BAD_ADDR) {
		debug("OF: not found !\n");
		return 1;
	}
	memcpy(addr, ranges + na, 4 * pna);

 finish:
	of_dump_addr("OF: parent translation for:", addr, pna);
	debug("OF: with offset: "PRu64"\n", offset);

	/* Translate it into parent bus space */
	return pbus->translate(addr, offset, pna);
}

/*
 * Translate an address from the device-tree into a CPU physical address,
 * this walks up the tree and applies the various bus mappings on the
 * way.
 *
 * Note: We consider that crossing any level with #size-cells == 0 to mean
 * that translation is impossible (that is we are not dealing with a value
 * that can be mapped to a cpu physical address). This is not really specified
 * that way, but this is traditionally the way IBM at least do things
 */
u64 __of_translate_address(void *blob, int node_offset, const u32 *in_addr,
			   const char *rprop)
{
	int parent;
	struct of_bus *bus, *pbus;
	u32 addr[OF_MAX_ADDR_CELLS];
	int na, ns, pna, pns;
	u64 result = OF_BAD_ADDR;

	debug("OF: ** translation for device %s **\n",
		fdt_get_name(blob, node_offset, NULL));

	/* Get parent & match bus type */
	parent = fdt_parent_offset(blob, node_offset);
	if (parent < 0)
		goto bail;
	bus = &of_busses[0];

	/* Cound address cells & copy address locally */
	bus->count_cells(blob, parent, &na, &ns);
	if (!OF_CHECK_COUNTS(na, ns)) {
		printf("%s: Bad cell count for %s\n", __FUNCTION__,
		       fdt_get_name(blob, node_offset, NULL));
		goto bail;
	}
	memcpy(addr, in_addr, na * 4);

	debug("OF: bus is %s (na=%d, ns=%d) on %s\n",
	    bus->name, na, ns, fdt_get_name(blob, parent, NULL));
	of_dump_addr("OF: translating address:", addr, na);

	/* Translate */
	for (;;) {
		/* Switch to parent bus */
		node_offset = parent;
		parent = fdt_parent_offset(blob, node_offset);

		/* If root, we have finished */
		if (parent < 0) {
			debug("OF: reached root node\n");
			result = of_read_number(addr, na);
			break;
		}

		/* Get new parent bus and counts */
		pbus = &of_busses[0];
		pbus->count_cells(blob, parent, &pna, &pns);
		if (!OF_CHECK_COUNTS(pna, pns)) {
			printf("%s: Bad cell count for %s\n", __FUNCTION__,
				fdt_get_name(blob, node_offset, NULL));
			break;
		}

		debug("OF: parent bus is %s (na=%d, ns=%d) on %s\n",
		    pbus->name, pna, pns, fdt_get_name(blob, parent, NULL));

		/* Apply bus translation */
		if (of_translate_one(blob, node_offset, bus, pbus,
					addr, na, ns, pna, rprop))
			break;

		/* Complete the move up one level */
		na = pna;
		ns = pns;
		bus = pbus;

		of_dump_addr("OF: one level translation:", addr, na);
	}
 bail:

	return result;
}

u64 fdt_translate_address(void *blob, int node_offset, const u32 *in_addr)
{
	return __of_translate_address(blob, node_offset, in_addr, "ranges");
}

/**
 * fdt_node_offset_by_compat_reg: Find a node that matches compatiable and
 * who's reg property matches a physical cpu address
 *
 * @blob: ptr to device tree
 * @compat: compatiable string to match
 * @compat_off: property name
 *
 */
int fdt_node_offset_by_compat_reg(void *blob, const char *compat,
					phys_addr_t compat_off)
{
	int len, off = fdt_node_offset_by_compatible(blob, -1, compat);
	while (off != -FDT_ERR_NOTFOUND) {
		u32 *reg = (u32 *)fdt_getprop(blob, off, "reg", &len);
		if (reg) {
			if (compat_off == fdt_translate_address(blob, off, reg))
				return off;
		}
		off = fdt_node_offset_by_compatible(blob, off, compat);
	}

	return -FDT_ERR_NOTFOUND;
}

/**
 * fdt_alloc_phandle: Return next free phandle value
 *
 * @blob: ptr to device tree
 */
int fdt_alloc_phandle(void *blob)
{
	int offset, len, phandle = 0;
	const u32 *val;

	for (offset = fdt_next_node(blob, -1, NULL); offset >= 0;
	     offset = fdt_next_node(blob, offset, NULL)) {
		val = fdt_getprop(blob, offset, "linux,phandle", &len);
		if (val)
			phandle = max(*val, phandle);
	}

	return phandle + 1;
}

#if defined(CONFIG_VIDEO)
int fdt_add_edid(void *blob, const char *compat, unsigned char *edid_buf)
{
	int noff;
	int ret;

	noff = fdt_node_offset_by_compatible(blob, -1, compat);
	if (noff != -FDT_ERR_NOTFOUND) {
		debug("%s: %s\n", fdt_get_name(blob, noff, 0), compat);
add_edid:
		ret = fdt_setprop(blob, noff, "edid", edid_buf, 128);
		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_edid;
			else
				goto err_size;
		} else if (ret < 0) {
			printf("Can't add property: %s\n", fdt_strerror(ret));
			return ret;
		}
	}
	return 0;
err_size:
	printf("Can't increase blob size: %s\n", fdt_strerror(ret));
	return ret;
}
#endif
