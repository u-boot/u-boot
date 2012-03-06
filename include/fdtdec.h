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
	COMPAT_NVIDIA_TEGRA20_USB,	/* Tegra2 USB port */
	COMPAT_NVIDIA_TEGRA20_I2C,	/* Tegra2 i2c */
	COMPAT_NVIDIA_TEGRA20_DVC,	/* Tegra2 dvc (really just i2c) */

	COMPAT_COUNT,
};

/* GPIOs are numbered from 0 */
enum {
	FDT_GPIO_NONE = -1U,	/* an invalid GPIO used to end our list */

	FDT_GPIO_ACTIVE_LOW = 1 << 0,	/* input is active low (else high) */
};

/* This is the state of a GPIO pin as defined by the fdt */
struct fdt_gpio_state {
	const char *name;	/* name of the fdt property defining this */
	uint gpio;		/* GPIO number, or FDT_GPIO_NONE if none */
	u8 flags;		/* FDT_GPIO_... flags */
};

/* This tells us whether a fdt_gpio_state record is valid or not */
#define fdt_gpio_isvalid(x) ((x)->gpio != FDT_GPIO_NONE)

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
 * Find the next compatible node for a peripheral.
 *
 * Do the first call with node = 0. This function will return a pointer to
 * the next compatible node. Next time you call this function, pass the
 * value returned, and the next node will be provided.
 *
 * @param blob		FDT blob to use
 * @param node		Start node for search
 * @param id		Compatible ID to look for (enum fdt_compat_id)
 * @return offset of next compatible node, or -FDT_ERR_NOTFOUND if no more
 */
int fdtdec_next_compatible(const void *blob, int node,
		enum fdt_compat_id id);

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
 * it returns 1 on the assumption that anything mentioned should be enabled
 * by default.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @return integer value 0 (not enabled) or 1 (enabled)
 */
int fdtdec_get_is_enabled(const void *blob, int node);

/**
 * Make sure we have a valid fdt available to control U-Boot.
 *
 * If not, a message is printed to the console if the console is ready.
 *
 * @return 0 if all ok, -1 if not
 */
int fdtdec_prepare_fdt(void);

/**
 * Checks that we have a valid fdt available to control U-Boot.

 * However, if not then for the moment nothing is done, since this function
 * is called too early to panic().
 *
 * @returns 0
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
 * This function is similar to fdtdec_find_aliases_for_id() except that it
 * adds to the node_list that is passed in. Any 0 elements are considered
 * available for allocation - others are considered already used and are
 * skipped.
 *
 * You can use this by calling fdtdec_find_aliases_for_id() with an
 * uninitialised array, then setting the elements that are returned to -1,
 * say, then calling this function, perhaps with a different compat id.
 * Any elements you get back that are >0 are new nodes added by the call
 * to this function.
 *
 * Note that if you have some nodes with aliases and some without, you are
 * sailing close to the wind. The call to fdtdec_find_aliases_for_id() with
 * one compat_id may fill in positions for which you have aliases defined
 * for another compat_id. When you later call *this* function with the second
 * compat_id, the alias positions may already be used. A debug warning may
 * be generated in this case, but it is safest to define aliases for all
 * nodes when you care about the ordering.
 */
int fdtdec_add_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount);

/*
 * Get the name for a compatible ID
 *
 * @param id		Compatible ID to look for
 * @return compatible string for that id
 */
const char *fdtdec_get_compatible(enum fdt_compat_id id);

/* Look up a phandle and follow it to its node. Then return the offset
 * of that node.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @return node offset if found, -ve error code on error
 */
int fdtdec_lookup_phandle(const void *blob, int node, const char *prop_name);

/**
 * Look up a property in a node and return its contents in an integer
 * array of given length. The property must have at least enough data for
 * the array (4*count bytes). It may have more, but this will be ignored.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param array		array to fill with data
 * @param count		number of array elements
 * @return 0 if ok, or -FDT_ERR_NOTFOUND if the property is not found,
 *		or -FDT_ERR_BADLAYOUT if not enough data
 */
int fdtdec_get_int_array(const void *blob, int node, const char *prop_name,
		u32 *array, int count);

/**
 * Look up a boolean property in a node and return it.
 *
 * A boolean properly is true if present in the device tree and false if not
 * present, regardless of its value.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @return 1 if the properly is present; 0 if it isn't present
 */
int fdtdec_get_bool(const void *blob, int node, const char *prop_name);

/**
 * Decode a single GPIOs from an FDT.
 *
 * If the property is not found, then the GPIO structure will still be
 * initialised, with gpio set to FDT_GPIO_NONE. This makes it easy to
 * provide optional GPIOs.
 *
 * @param blob		FDT blob to use
 * @param node		Node to look at
 * @param prop_name	Node property name
 * @param gpio		gpio elements to fill from FDT
 * @return 0 if ok, -FDT_ERR_NOTFOUND if the property is missing.
 */
int fdtdec_decode_gpio(const void *blob, int node, const char *prop_name,
		struct fdt_gpio_state *gpio);

/**
 * Set up a GPIO pin according to the provided gpio information. At present this
 * just requests the GPIO.
 *
 * If the gpio is FDT_GPIO_NONE, no action is taken. This makes it easy to
 * deal with optional GPIOs.
 *
 * @param gpio		GPIO info to use for set up
 * @return 0 if all ok or gpio was FDT_GPIO_NONE; -1 on error
 */
int fdtdec_setup_gpio(struct fdt_gpio_state *gpio);
