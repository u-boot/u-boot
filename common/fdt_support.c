/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
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

/*
 * fdt points to our working device tree.
 */
struct fdt_header *fdt;

/********************************************************************/

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
static int fdt_fixup_stdout(void *fdt, int choosenoff)
{
	int err = 0;
#ifdef CONFIG_CONS_INDEX
	int node;
	char sername[9] = { 0 };
	const char *path;

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
				err = fdt_setprop(fdt, choosenoff,
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

int fdt_chosen(void *fdt, ulong initrd_start, ulong initrd_end, int force)
{
	int   nodeoffset;
	int   err;
	u32   tmp;		/* used to set 32 bit integer properties */
	char  *str;		/* used to set string properties */

	err = fdt_check_header(fdt);
	if (err < 0) {
		printf("fdt_chosen: %s\n", fdt_strerror(err));
		return err;
	}

	if (initrd_start && initrd_end) {
		uint64_t addr, size;
		int  total = fdt_num_mem_rsv(fdt);
		int  j;

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
			printf("fdt_chosen: %s\n", fdt_strerror(err));
			return err;
		}
	}

	/*
	 * Find the "chosen" node.
	 */
	nodeoffset = fdt_path_offset (fdt, "/chosen");

	/*
	 * If we have a "chosen" node already the "force the writing"
	 * is not set, our job is done.
	 */
	if ((nodeoffset >= 0) && !force)
		return 0;

	/*
	 * No "chosen" node in the blob: create it.
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
	 * Update pre-existing properties, create them if non-existant.
	 */
	str = getenv("bootargs");
	if (str != NULL) {
		err = fdt_setprop(fdt, nodeoffset,
			"bootargs", str, strlen(str)+1);
		if (err < 0)
			printf("WARNING: could not set bootargs %s.\n",
				fdt_strerror(err));
	}
	if (initrd_start && initrd_end) {
		tmp = __cpu_to_be32(initrd_start);
		err = fdt_setprop(fdt, nodeoffset,
			 "linux,initrd-start", &tmp, sizeof(tmp));
		if (err < 0)
			printf("WARNING: "
				"could not set linux,initrd-start %s.\n",
				fdt_strerror(err));
		tmp = __cpu_to_be32(initrd_end);
		err = fdt_setprop(fdt, nodeoffset,
			"linux,initrd-end", &tmp, sizeof(tmp));
		if (err < 0)
			printf("WARNING: could not set linux,initrd-end %s.\n",
				fdt_strerror(err));
	}

#ifdef CONFIG_OF_STDOUT_VIA_ALIAS
	err = fdt_fixup_stdout(fdt, nodeoffset);
#endif

#ifdef OF_STDOUT_PATH
	err = fdt_setprop(fdt, nodeoffset,
		"linux,stdout-path", OF_STDOUT_PATH, strlen(OF_STDOUT_PATH)+1);
	if (err < 0)
		printf("WARNING: could not set linux,stdout-path %s.\n",
			fdt_strerror(err));
#endif

	return err;
}

/********************************************************************/

#ifdef CONFIG_OF_HAS_UBOOT_ENV

/* Function that returns a character from the environment */
extern uchar(*env_get_char) (int);


int fdt_env(void *fdt)
{
	int   nodeoffset;
	int   err;
	int   k, nxt;
	int i;
	static char tmpenv[256];

	err = fdt_check_header(fdt);
	if (err < 0) {
		printf("fdt_env: %s\n", fdt_strerror(err));
		return err;
	}

	/*
	 * See if we already have a "u-boot-env" node, delete it if so.
	 * Then create a new empty node.
	 */
	nodeoffset = fdt_path_offset (fdt, "/u-boot-env");
	if (nodeoffset >= 0) {
		err = fdt_del_node(fdt, nodeoffset);
		if (err < 0) {
			printf("fdt_env: %s\n", fdt_strerror(err));
			return err;
		}
	}
	/*
	 * Create a new node "/u-boot-env" (offset 0 is root level)
	 */
	nodeoffset = fdt_add_subnode(fdt, 0, "u-boot-env");
	if (nodeoffset < 0) {
		printf("WARNING: could not create /u-boot-env %s.\n",
			fdt_strerror(nodeoffset));
		return nodeoffset;
	}

	for (i = 0; env_get_char(i) != '\0'; i = nxt + 1) {
		char *s, *lval, *rval;

		/*
		 * Find the end of the name=definition
		 */
		for (nxt = i; env_get_char(nxt) != '\0'; ++nxt)
			;
		s = tmpenv;
		for (k = i; k < nxt && s < &tmpenv[sizeof(tmpenv) - 1]; ++k)
			*s++ = env_get_char(k);
		*s++ = '\0';
		lval = tmpenv;
		/*
		 * Find the first '=': it separates the name from the value
		 */
		s = strchr(tmpenv, '=');
		if (s != NULL) {
			*s++ = '\0';
			rval = s;
		} else
			continue;
		err = fdt_setprop(fdt, nodeoffset, lval, rval, strlen(rval)+1);
		if (err < 0) {
			printf("WARNING: could not set %s %s.\n",
				lval, fdt_strerror(err));
			return err;
		}
	}
	return 0;
}
#endif /* ifdef CONFIG_OF_HAS_UBOOT_ENV */

/********************************************************************/

#ifdef CONFIG_OF_HAS_BD_T

#define BDM(x)	{	.name = #x, .offset = offsetof(bd_t, bi_ ##x ) }

static const struct {
	const char *name;
	int offset;
} bd_map[] = {
	BDM(memstart),
	BDM(memsize),
	BDM(flashstart),
	BDM(flashsize),
	BDM(flashoffset),
	BDM(sramstart),
	BDM(sramsize),
#if defined(CONFIG_5xx) || defined(CONFIG_8xx) || defined(CONFIG_8260) \
	|| defined(CONFIG_E500)
	BDM(immr_base),
#endif
#if defined(CONFIG_MPC5xxx)
	BDM(mbar_base),
#endif
#if defined(CONFIG_MPC83XX)
	BDM(immrbar),
#endif
#if defined(CONFIG_MPC8220)
	BDM(mbar_base),
	BDM(inpfreq),
	BDM(pcifreq),
	BDM(pevfreq),
	BDM(flbfreq),
	BDM(vcofreq),
#endif
	BDM(bootflags),
	BDM(ip_addr),
	BDM(intfreq),
	BDM(busfreq),
#ifdef CONFIG_CPM2
	BDM(cpmfreq),
	BDM(brgfreq),
	BDM(sccfreq),
	BDM(vco),
#endif
#if defined(CONFIG_MPC5xxx)
	BDM(ipbfreq),
	BDM(pcifreq),
#endif
	BDM(baudrate),
};


int fdt_bd_t(void *fdt)
{
	bd_t *bd = gd->bd;
	int   nodeoffset;
	int   err;
	u32   tmp;		/* used to set 32 bit integer properties */
	int i;

	err = fdt_check_header(fdt);
	if (err < 0) {
		printf("fdt_bd_t: %s\n", fdt_strerror(err));
		return err;
	}

	/*
	 * See if we already have a "bd_t" node, delete it if so.
	 * Then create a new empty node.
	 */
	nodeoffset = fdt_path_offset (fdt, "/bd_t");
	if (nodeoffset >= 0) {
		err = fdt_del_node(fdt, nodeoffset);
		if (err < 0) {
			printf("fdt_bd_t: %s\n", fdt_strerror(err));
			return err;
		}
	}
	/*
	 * Create a new node "/bd_t" (offset 0 is root level)
	 */
	nodeoffset = fdt_add_subnode(fdt, 0, "bd_t");
	if (nodeoffset < 0) {
		printf("WARNING: could not create /bd_t %s.\n",
			fdt_strerror(nodeoffset));
		printf("fdt_bd_t: %s\n", fdt_strerror(nodeoffset));
		return nodeoffset;
	}
	/*
	 * Use the string/pointer structure to create the entries...
	 */
	for (i = 0; i < sizeof(bd_map)/sizeof(bd_map[0]); i++) {
		tmp = cpu_to_be32(getenv("bootargs"));
		err = fdt_setprop(fdt, nodeoffset,
			bd_map[i].name, &tmp, sizeof(tmp));
		if (err < 0)
			printf("WARNING: could not set %s %s.\n",
				bd_map[i].name, fdt_strerror(err));
	}
	/*
	 * Add a couple of oddball entries...
	 */
	err = fdt_setprop(fdt, nodeoffset, "enetaddr", &bd->bi_enetaddr, 6);
	if (err < 0)
		printf("WARNING: could not set enetaddr %s.\n",
			fdt_strerror(err));
	err = fdt_setprop(fdt, nodeoffset, "ethspeed", &bd->bi_ethspeed, 4);
	if (err < 0)
		printf("WARNING: could not set ethspeed %s.\n",
			fdt_strerror(err));
	return 0;
}
#endif /* ifdef CONFIG_OF_HAS_BD_T */

void do_fixup_by_path(void *fdt, const char *path, const char *prop,
		      const void *val, int len, int create)
{
#if defined(DEBUG)
	int i;
	debug("Updating property '%s/%s' = ", node, prop);
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
	debug("Updating property '%s/%s' = ", node, prop);
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
	debug("Updating property '%s/%s' = ", node, prop);
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

int fdt_fixup_memory(void *blob, u64 start, u64 size)
{
	int err, nodeoffset, len = 0;
	u8 tmp[16];
	const u32 *addrcell, *sizecell;

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

	addrcell = fdt_getprop(blob, 0, "#address-cells", NULL);
	/* use shifts and mask to ensure endianness */
	if ((addrcell) && (*addrcell == 2)) {
		tmp[0] = (start >> 56) & 0xff;
		tmp[1] = (start >> 48) & 0xff;
		tmp[2] = (start >> 40) & 0xff;
		tmp[3] = (start >> 32) & 0xff;
		tmp[4] = (start >> 24) & 0xff;
		tmp[5] = (start >> 16) & 0xff;
		tmp[6] = (start >>  8) & 0xff;
		tmp[7] = (start      ) & 0xff;
		len = 8;
	} else {
		tmp[0] = (start >> 24) & 0xff;
		tmp[1] = (start >> 16) & 0xff;
		tmp[2] = (start >>  8) & 0xff;
		tmp[3] = (start      ) & 0xff;
		len = 4;
	}

	sizecell = fdt_getprop(blob, 0, "#size-cells", NULL);
	/* use shifts and mask to ensure endianness */
	if ((sizecell) && (*sizecell == 2)) {
		tmp[0+len] = (size >> 56) & 0xff;
		tmp[1+len] = (size >> 48) & 0xff;
		tmp[2+len] = (size >> 40) & 0xff;
		tmp[3+len] = (size >> 32) & 0xff;
		tmp[4+len] = (size >> 24) & 0xff;
		tmp[5+len] = (size >> 16) & 0xff;
		tmp[6+len] = (size >>  8) & 0xff;
		tmp[7+len] = (size      ) & 0xff;
		len += 8;
	} else {
		tmp[0+len] = (size >> 24) & 0xff;
		tmp[1+len] = (size >> 16) & 0xff;
		tmp[2+len] = (size >>  8) & 0xff;
		tmp[3+len] = (size      ) & 0xff;
		len += 4;
	}

	err = fdt_setprop(blob, nodeoffset, "reg", tmp, len);
	if (err < 0) {
		printf("WARNING: could not set %s %s.\n",
				"reg", fdt_strerror(err));
		return err;
	}
	return 0;
}

void fdt_fixup_ethernet(void *fdt, bd_t *bd)
{
	int node;
	const char *path;

	node = fdt_path_offset(fdt, "/aliases");
	if (node >= 0) {
#if defined(CONFIG_HAS_ETH0)
		path = fdt_getprop(fdt, node, "ethernet0", NULL);
		if (path) {
			do_fixup_by_path(fdt, path, "mac-address",
				bd->bi_enetaddr, 6, 0);
			do_fixup_by_path(fdt, path, "local-mac-address",
				bd->bi_enetaddr, 6, 1);
		}
#endif
#if defined(CONFIG_HAS_ETH1)
		path = fdt_getprop(fdt, node, "ethernet1", NULL);
		if (path) {
			do_fixup_by_path(fdt, path, "mac-address",
				bd->bi_enet1addr, 6, 0);
			do_fixup_by_path(fdt, path, "local-mac-address",
				bd->bi_enet1addr, 6, 1);
		}
#endif
#if defined(CONFIG_HAS_ETH2)
		path = fdt_getprop(fdt, node, "ethernet2", NULL);
		if (path) {
			do_fixup_by_path(fdt, path, "mac-address",
				bd->bi_enet2addr, 6, 0);
			do_fixup_by_path(fdt, path, "local-mac-address",
				bd->bi_enet2addr, 6, 1);
		}
#endif
#if defined(CONFIG_HAS_ETH3)
		path = fdt_getprop(fdt, node, "ethernet3", NULL);
		if (path) {
			do_fixup_by_path(fdt, path, "mac-address",
				bd->bi_enet3addr, 6, 0);
			do_fixup_by_path(fdt, path, "local-mac-address",
				bd->bi_enet3addr, 6, 1);
		}
#endif
	}
}
