/*
 * Copyright (c) 2011 The Chromium OS Authors.
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


/*
 * This file contains convenience functions for decoding useful and
 * enlightening information from FDTs. It is intended to be used by device
 * drivers and board-specific code within U-Boot. It aims to reduce the
 * amount of FDT munging required within U-Boot itself, so that driver code
 * changes to support FDT are minimized.
 */

#include <libfdt.h>

/*
 * A typedef for a physical address. Note that fdt data is always big
 * endian even on a litle endian machine.
 */
#ifdef CONFIG_PHYS_64BIT
typedef u64 fdt_addr_t;
#define FDT_ADDR_T_NONE (-1ULL)
#define fdt_addr_to_cpu(reg) be64_to_cpu(reg)
#else
typedef u32 fdt_addr_t;
#define FDT_ADDR_T_NONE (-1U)
#define fdt_addr_to_cpu(reg) be32_to_cpu(reg)
#endif

/* Information obtained about memory from the FDT */
struct fdt_memory {
	fdt_addr_t start;
	fdt_addr_t end;
};

/**
 * Compat types that we know about and for which we might have drivers.
 * Each is named COMPAT_<dir>_<filename> where <dir> is the directory
 * within drivers.
 */
enum fdt_compat_id {
	COMPAT_UNKNOWN,

	COMPAT_COUNT,
};

/**
 * Find the next numbered alias for a peripheral. This is used to enumerate
 * all the peripherals of a certain type.
 *
 * Do the first call with *upto = 0. Assuming /aliases/<name>0 exists then
 * this function will return a pointer to the node the alias points to, and
 * then update *upto to 1. Next time you call this function, the next node
 * will be returned.
 *
 * All nodes returned will match the compatible ID, as it is assumed that
 * all peripherals use the same driver.
 *
 * @param blob		FDT blob to use
 * @param name		Root name of alias to search for
 * @param id		Compatible ID to look for
 * @return offset of next compatible node, or -FDT_ERR_NOTFOUND if no more
 */
int fdtdec_next_alias(const void *blob, const char *name,
		enum fdt_compat_id id, int *upto);

/**
 * Look up an address property in a node and return it as an address.
 * The property must hold either one address with no trailing data or
 * one address with a length. This is only tested on 32-bit machines.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @return address, if found, or FDT_ADDR_T_NONE if not
 */
fdt_addr_t fdtdec_get_addr(const void *blob, int node,
		const char *prop_name);

/**
 * Look up a 32-bit integer property in a node and return it. The property
 * must have at least 4 bytes of data. The value of the first cell is
 * returned.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param default_val	default value to return if the property is not found
 * @return integer value, if found, or default_val if not
 */
s32 fdtdec_get_int(const void *blob, int node, const char *prop_name,
		s32 default_val);

/**
 * Checks whether a node is enabled.
 * This looks for a 'status' property. If this exists, then returns 1 if
 * the status is 'ok' and 0 otherwise. If there is no status property,
 * it returns the default value.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param default_val	default value to return if no 'status' property exists
 * @return integer value 0/1, if found, or default_val if not
 */
int fdtdec_get_is_enabled(const void *blob, int node, int default_val);

/**
 * Checks whether we have a valid fdt available to control U-Boot, and panic
 * if not.
 */
int fdtdec_check_fdt(void);

/**
 * Find the nodes for a peripheral and return a list of them in the correct
 * order. This is used to enumerate all the peripherals of a certain type.
 *
 * To use this, optionally set up a /aliases node with alias properties for
 * a peripheral. For example, for usb you could have:
 *
 * aliases {
 *		usb0 = "/ehci@c5008000";
 *		usb1 = "/ehci@c5000000";
 * };
 *
 * Pass "usb" as the name to this function and will return a list of two
 * nodes offsets: /ehci@c5008000 and ehci@c5000000.
 *
 * All nodes returned will match the compatible ID, as it is assumed that
 * all peripherals use the same driver.
 *
 * If no alias node is found, then the node list will be returned in the
 * order found in the fdt. If the aliases mention a node which doesn't
 * exist, then this will be ignored. If nodes are found with no aliases,
 * they will be added in any order.
 *
 * If there is a gap in the aliases, then this function return a 0 node at
 * that position. The return value will also count these gaps.
 *
 * This function checks node properties and will not return nodes which are
 * marked disabled (status = "disabled").
 *
 * @param blob		FDT blob to use
 * @param name		Root name of alias to search for
 * @param id		Compatible ID to look for
 * @param node_list	Place to put list of found nodes
 * @param maxcount	Maximum number of nodes to find
 * @return number of nodes found on success, FTD_ERR_... on error
 */
int fdtdec_find_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount);

/*
 * Get the name for a compatible ID
 *
 * @param id		Compatible ID to look for
 * @return compatible string for that id
 */
const char *fdtdec_get_compatible(enum fdt_compat_id id);
