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

#ifdef CONFIG_OF_LIBFDT

#include <asm/global_data.h>
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>

/*
 * Global data (for the gd->bd)
 */
DECLARE_GLOBAL_DATA_PTR;

/*
 * fdt points to our working device tree.
 */
struct fdt_header *fdt;

/********************************************************************/

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
		struct fdt_reserve_entry re;
		int  used;
		int  total;
		int  j;

		err = fdt_num_reservemap(fdt, &used, &total);
		if (err < 0) {
			printf("fdt_chosen: %s\n", fdt_strerror(err));
			return err;
		}
		if (used >= total) {
			printf("WARNING: "
				"no room in the reserved map (%d of %d)\n",
				used, total);
			return -1;
		}
		/*
		 * Look for an existing entry and update it.  If we don't find
		 * the entry, we will j be the next available slot.
		 */
		for (j = 0; j < used; j++) {
			err = fdt_get_reservemap(fdt, j, &re);
			if (re.address == initrd_start) {
				break;
			}
		}
		err = fdt_replace_reservemap_entry(fdt, j,
			initrd_start, initrd_end - initrd_start + 1);
		if (err < 0) {
			printf("fdt_chosen: %s\n", fdt_strerror(err));
			return err;
		}
	}

	/*
	 * Find the "chosen" node.
	 */
	nodeoffset = fdt_find_node_by_path (fdt, "/chosen");

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
	nodeoffset = fdt_find_node_by_path (fdt, "/u-boot-env");
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
	nodeoffset = fdt_find_node_by_path (fdt, "/bd_t");
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

#endif /* CONFIG_OF_LIBFDT */
