/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __fdtdec_h
#define __fdtdec_h

/*
 * This file contains convenience functions for decoding useful and
 * enlightening information from FDTs. It is intended to be used by device
 * drivers and board-specific code within U-Boot. It aims to reduce the
 * amount of FDT munging required within U-Boot itself, so that driver code
 * changes to support FDT are minimized.
 */

#include <libfdt.h>
#include <pci.h>

/*
 * A typedef for a physical address. Note that fdt data is always big
 * endian even on a litle endian machine.
 */
#ifdef CONFIG_PHYS_64BIT
typedef u64 fdt_addr_t;
typedef u64 fdt_size_t;
#define FDT_ADDR_T_NONE (-1ULL)
#define fdt_addr_to_cpu(reg) be64_to_cpu(reg)
#define fdt_size_to_cpu(reg) be64_to_cpu(reg)
#else
typedef u32 fdt_addr_t;
typedef u32 fdt_size_t;
#define FDT_ADDR_T_NONE (-1U)
#define fdt_addr_to_cpu(reg) be32_to_cpu(reg)
#define fdt_size_to_cpu(reg) be32_to_cpu(reg)
#endif

/* Information obtained about memory from the FDT */
struct fdt_memory {
	fdt_addr_t start;
	fdt_addr_t end;
};

/*
 * Information about a resource. start is the first address of the resource
 * and end is the last address (inclusive). The length of the resource will
 * be equal to: end - start + 1.
 */
struct fdt_resource {
	fdt_addr_t start;
	fdt_addr_t end;
};

enum fdt_pci_space {
	FDT_PCI_SPACE_CONFIG = 0,
	FDT_PCI_SPACE_IO = 0x01000000,
	FDT_PCI_SPACE_MEM32 = 0x02000000,
	FDT_PCI_SPACE_MEM64 = 0x03000000,
	FDT_PCI_SPACE_MEM32_PREF = 0x42000000,
	FDT_PCI_SPACE_MEM64_PREF = 0x43000000,
};

#define FDT_PCI_ADDR_CELLS	3
#define FDT_PCI_SIZE_CELLS	2
#define FDT_PCI_REG_SIZE	\
	((FDT_PCI_ADDR_CELLS + FDT_PCI_SIZE_CELLS) * sizeof(u32))

/*
 * The Open Firmware spec defines PCI physical address as follows:
 *
 *          bits# 31 .... 24 23 .... 16 15 .... 08 07 .... 00
 *
 * phys.hi  cell:  npt000ss   bbbbbbbb   dddddfff   rrrrrrrr
 * phys.mid cell:  hhhhhhhh   hhhhhhhh   hhhhhhhh   hhhhhhhh
 * phys.lo  cell:  llllllll   llllllll   llllllll   llllllll
 *
 * where:
 *
 * n:        is 0 if the address is relocatable, 1 otherwise
 * p:        is 1 if addressable region is prefetchable, 0 otherwise
 * t:        is 1 if the address is aliased (for non-relocatable I/O) below 1MB
 *           (for Memory), or below 64KB (for relocatable I/O)
 * ss:       is the space code, denoting the address space
 * bbbbbbbb: is the 8-bit Bus Number
 * ddddd:    is the 5-bit Device Number
 * fff:      is the 3-bit Function Number
 * rrrrrrrr: is the 8-bit Register Number
 * hhhhhhhh: is a 32-bit unsigned number
 * llllllll: is a 32-bit unsigned number
 */
struct fdt_pci_addr {
	u32	phys_hi;
	u32	phys_mid;
	u32	phys_lo;
};

/**
 * Compute the size of a resource.
 *
 * @param res	the resource to operate on
 * @return the size of the resource
 */
static inline fdt_size_t fdt_resource_size(const struct fdt_resource *res)
{
	return res->end - res->start + 1;
}

/**
 * Compat types that we know about and for which we might have drivers.
 * Each is named COMPAT_<dir>_<filename> where <dir> is the directory
 * within drivers.
 */
enum fdt_compat_id {
	COMPAT_UNKNOWN,
	COMPAT_NVIDIA_TEGRA20_USB,	/* Tegra20 USB port */
	COMPAT_NVIDIA_TEGRA30_USB,	/* Tegra30 USB port */
	COMPAT_NVIDIA_TEGRA114_USB,	/* Tegra114 USB port */
	COMPAT_NVIDIA_TEGRA20_EMC,	/* Tegra20 memory controller */
	COMPAT_NVIDIA_TEGRA20_EMC_TABLE, /* Tegra20 memory timing table */
	COMPAT_NVIDIA_TEGRA20_KBC,	/* Tegra20 Keyboard */
	COMPAT_NVIDIA_TEGRA20_NAND,	/* Tegra2 NAND controller */
	COMPAT_NVIDIA_TEGRA20_PWM,	/* Tegra 2 PWM controller */
	COMPAT_NVIDIA_TEGRA20_DC,	/* Tegra 2 Display controller */
	COMPAT_NVIDIA_TEGRA124_SDMMC,	/* Tegra124 SDMMC controller */
	COMPAT_NVIDIA_TEGRA30_SDMMC,	/* Tegra30 SDMMC controller */
	COMPAT_NVIDIA_TEGRA20_SDMMC,	/* Tegra20 SDMMC controller */
	COMPAT_NVIDIA_TEGRA124_PCIE,	/* Tegra 124 PCIe controller */
	COMPAT_NVIDIA_TEGRA30_PCIE,	/* Tegra 30 PCIe controller */
	COMPAT_NVIDIA_TEGRA20_PCIE,	/* Tegra 20 PCIe controller */
	COMPAT_NVIDIA_TEGRA124_XUSB_PADCTL,
					/* Tegra124 XUSB pad controller */
	COMPAT_SMSC_LAN9215,		/* SMSC 10/100 Ethernet LAN9215 */
	COMPAT_SAMSUNG_EXYNOS5_SROMC,	/* Exynos5 SROMC */
	COMPAT_SAMSUNG_S3C2440_I2C,	/* Exynos I2C Controller */
	COMPAT_SAMSUNG_EXYNOS5_SOUND,	/* Exynos Sound */
	COMPAT_WOLFSON_WM8994_CODEC,	/* Wolfson WM8994 Sound Codec */
	COMPAT_GOOGLE_CROS_EC,		/* Google CROS_EC Protocol */
	COMPAT_GOOGLE_CROS_EC_KEYB,	/* Google CROS_EC Keyboard */
	COMPAT_SAMSUNG_EXYNOS_EHCI,	/* Exynos EHCI controller */
	COMPAT_SAMSUNG_EXYNOS5_XHCI,	/* Exynos5 XHCI controller */
	COMPAT_SAMSUNG_EXYNOS_USB_PHY,	/* Exynos phy controller for usb2.0 */
	COMPAT_SAMSUNG_EXYNOS5_USB3_PHY,/* Exynos phy controller for usb3.0 */
	COMPAT_SAMSUNG_EXYNOS_TMU,	/* Exynos TMU */
	COMPAT_SAMSUNG_EXYNOS_FIMD,	/* Exynos Display controller */
	COMPAT_SAMSUNG_EXYNOS_MIPI_DSI,	/* Exynos mipi dsi */
	COMPAT_SAMSUNG_EXYNOS5_DP,	/* Exynos Display port controller */
	COMPAT_SAMSUNG_EXYNOS_DWMMC,	/* Exynos DWMMC controller */
	COMPAT_SAMSUNG_EXYNOS_MMC,	/* Exynos MMC controller */
	COMPAT_SAMSUNG_EXYNOS_SERIAL,	/* Exynos UART */
	COMPAT_MAXIM_MAX77686_PMIC,	/* MAX77686 PMIC */
	COMPAT_GENERIC_SPI_FLASH,	/* Generic SPI Flash chip */
	COMPAT_MAXIM_98095_CODEC,	/* MAX98095 Codec */
	COMPAT_INFINEON_SLB9635_TPM,	/* Infineon SLB9635 TPM */
	COMPAT_INFINEON_SLB9645_TPM,	/* Infineon SLB9645 TPM */
	COMPAT_SAMSUNG_EXYNOS5_I2C,	/* Exynos5 High Speed I2C Controller */
	COMPAT_SANDBOX_HOST_EMULATION,	/* Sandbox emulation of a function */
	COMPAT_SANDBOX_LCD_SDL,		/* Sandbox LCD emulation with SDL */
	COMPAT_TI_TPS65090,		/* Texas Instrument TPS65090 */
	COMPAT_NXP_PTN3460,		/* NXP PTN3460 DP/LVDS bridge */
	COMPAT_SAMSUNG_EXYNOS_SYSMMU,	/* Exynos sysmmu */
	COMPAT_PARADE_PS8625,		/* Parade PS8622 EDP->LVDS bridge */
	COMPAT_INTEL_LPC,		/* Intel Low Pin Count I/F */
	COMPAT_INTEL_MICROCODE,		/* Intel microcode update */
	COMPAT_MEMORY_SPD,		/* Memory SPD information */
	COMPAT_INTEL_PANTHERPOINT_AHCI,	/* Intel Pantherpoint AHCI */
	COMPAT_INTEL_MODEL_206AX,	/* Intel Model 206AX CPU */
	COMPAT_INTEL_GMA,		/* Intel Graphics Media Accelerator */
	COMPAT_AMS_AS3722,		/* AMS AS3722 PMIC */
	COMPAT_INTEL_ICH_SPI,		/* Intel ICH7/9 SPI controller */
	COMPAT_INTEL_QRK_MRC,		/* Intel Quark MRC */
	COMPAT_SOCIONEXT_XHCI,		/* Socionext UniPhier xHCI */

	COMPAT_COUNT,
};

#define MAX_PHANDLE_ARGS 16
struct fdtdec_phandle_args {
	int node;
	int args_count;
	uint32_t args[MAX_PHANDLE_ARGS];
};

/**
 * fdtdec_parse_phandle_with_args() - Find a node pointed by phandle in a list
 *
 * This function is useful to parse lists of phandles and their arguments.
 *
 * Example:
 *
 * phandle1: node1 {
 *	#list-cells = <2>;
 * }
 *
 * phandle2: node2 {
 *	#list-cells = <1>;
 * }
 *
 * node3 {
 *	list = <&phandle1 1 2 &phandle2 3>;
 * }
 *
 * To get a device_node of the `node2' node you may call this:
 * fdtdec_parse_phandle_with_args(blob, node3, "list", "#list-cells", 0, 1,
 *				  &args);
 *
 * (This function is a modified version of __of_parse_phandle_with_args() from
 * Linux 3.18)
 *
 * @blob:	Pointer to device tree
 * @src_node:	Offset of device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies the phandles' arguments count,
 *		or NULL to use @cells_count
 * @cells_count: Cell count to use if @cells_name is NULL
 * @index:	index of a phandle to parse out
 * @out_args:	optional pointer to output arguments structure (will be filled)
 * @return 0 on success (with @out_args filled out if not NULL), -ENOENT if
 *	@list_name does not exist, a phandle was not found, @cells_name
 *	could not be found, the arguments were truncated or there were too
 *	many arguments.
 *
 */
int fdtdec_parse_phandle_with_args(const void *blob, int src_node,
				   const char *list_name,
				   const char *cells_name,
				   int cell_count, int index,
				   struct fdtdec_phandle_args *out_args);

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
 * Find the compatible ID for a given node.
 *
 * Generally each node has at least one compatible string attached to it.
 * This function looks through our list of known compatible strings and
 * returns the corresponding ID which matches the compatible string.
 *
 * @param blob		FDT blob to use
 * @param node		Node containing compatible string to find
 * @return compatible ID, or COMPAT_UNKNOWN if we cannot find a match
 */
enum fdt_compat_id fdtdec_lookup(const void *blob, int node);

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
 * Find the next compatible subnode for a peripheral.
 *
 * Do the first call with node set to the parent and depth = 0. This
 * function will return the offset of the next compatible node. Next time
 * you call this function, pass the node value returned last time, with
 * depth unchanged, and the next node will be provided.
 *
 * @param blob		FDT blob to use
 * @param node		Start node for search
 * @param id		Compatible ID to look for (enum fdt_compat_id)
 * @param depthp	Current depth (set to 0 before first call)
 * @return offset of next compatible node, or -FDT_ERR_NOTFOUND if no more
 */
int fdtdec_next_compatible_subnode(const void *blob, int node,
		enum fdt_compat_id id, int *depthp);

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
 * Look up an address property in a node and return it as an address.
 * The property must hold one address with a length. This is only tested
 * on 32-bit machines.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @return address, if found, or FDT_ADDR_T_NONE if not
 */
fdt_addr_t fdtdec_get_addr_size(const void *blob, int node,
		const char *prop_name, fdt_size_t *sizep);

/**
 * Look at an address property in a node and return the pci address which
 * corresponds to the given type in the form of fdt_pci_addr.
 * The property must hold one fdt_pci_addr with a lengh.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param type		pci address type (FDT_PCI_SPACE_xxx)
 * @param prop_name	name of property to find
 * @param addr		returns pci address in the form of fdt_pci_addr
 * @return 0 if ok, negative on error
 */
int fdtdec_get_pci_addr(const void *blob, int node, enum fdt_pci_space type,
		const char *prop_name, struct fdt_pci_addr *addr);

/**
 * Look at the compatible property of a device node that represents a PCI
 * device and extract pci vendor id and device id from it.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param vendor	vendor id of the pci device
 * @param device	device id of the pci device
 * @return 0 if ok, negative on error
 */
int fdtdec_get_pci_vendev(const void *blob, int node,
		u16 *vendor, u16 *device);

/**
 * Look at the pci address of a device node that represents a PCI device
 * and parse the bus, device and function number from it. For some cases
 * like the bus number encoded in reg property is not correct after pci
 * enumeration, this function looks through the node's compatible strings
 * to get these numbers extracted instead.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param addr		pci address in the form of fdt_pci_addr
 * @param bdf		returns bus, device, function triplet
 * @return 0 if ok, negative on error
 */
int fdtdec_get_pci_bdf(const void *blob, int node,
		struct fdt_pci_addr *addr, pci_dev_t *bdf);

/**
 * Look at the pci address of a device node that represents a PCI device
 * and return base address of the pci device's registers.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param addr		pci address in the form of fdt_pci_addr
 * @param bar		returns base address of the pci device's registers
 * @return 0 if ok, negative on error
 */
int fdtdec_get_pci_bar32(const void *blob, int node,
		struct fdt_pci_addr *addr, u32 *bar);

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
 * Look up a 64-bit integer property in a node and return it. The property
 * must have at least 8 bytes of data (2 cells). The first two cells are
 * concatenated to form a 8 bytes value, where the first cell is top half and
 * the second cell is bottom half.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param default_val	default value to return if the property is not found
 * @return integer value, if found, or default_val if not
 */
uint64_t fdtdec_get_uint64(const void *blob, int node, const char *prop_name,
		uint64_t default_val);

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

/**
 * Get the alias sequence number of a node
 *
 * This works out whether a node is pointed to by an alias, and if so, the
 * sequence number of that alias. Aliases are of the form <base><num> where
 * <num> is the sequence number. For example spi2 would be sequence number
 * 2.
 *
 * @param blob		Device tree blob (if NULL, then error is returned)
 * @param base		Base name for alias (before the underscore)
 * @param node		Node to look up
 * @param seqp		This is set to the sequence number if one is found,
 *			but otherwise the value is left alone
 * @return 0 if a sequence was found, -ve if not
 */
int fdtdec_get_alias_seq(const void *blob, const char *base, int node,
			 int *seqp);

/**
 * Get the offset of the given chosen node
 *
 * This looks up a property in /chosen containing the path to another node,
 * then finds the offset of that node.
 *
 * @param blob		Device tree blob (if NULL, then error is returned)
 * @param name		Property name, e.g. "stdout-path"
 * @return Node offset referred to by that chosen node, or -ve FDT_ERR_...
 */
int fdtdec_get_chosen_node(const void *blob, const char *name);

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
 * Look up a property in a node and return its contents in an integer
 * array of given length. The property must exist but may have less data that
 * expected (4*count bytes). It may have more, but this will be ignored.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param array		array to fill with data
 * @param count		number of array elements
 * @return number of array elements if ok, or -FDT_ERR_NOTFOUND if the
 *		property is not found
 */
int fdtdec_get_int_array_count(const void *blob, int node,
			       const char *prop_name, u32 *array, int count);

/**
 * Look up a property in a node and return a pointer to its contents as a
 * unsigned int array of given length. The property must have at least enough
 * data for the array ('count' cells). It may have more, but this will be
 * ignored. The data is not copied.
 *
 * Note that you must access elements of the array with fdt32_to_cpu(),
 * since the elements will be big endian even on a little endian machine.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param count		number of array elements
 * @return pointer to array if found, or NULL if the property is not
 *		found or there is not enough data
 */
const u32 *fdtdec_locate_array(const void *blob, int node,
			       const char *prop_name, int count);

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
 * Look in the FDT for a config item with the given name and return its value
 * as a 32-bit integer. The property must have at least 4 bytes of data. The
 * value of the first cell is returned.
 *
 * @param blob		FDT blob to use
 * @param prop_name	Node property name
 * @param default_val	default value to return if the property is not found
 * @return integer value, if found, or default_val if not
 */
int fdtdec_get_config_int(const void *blob, const char *prop_name,
		int default_val);

/**
 * Look in the FDT for a config item with the given name
 * and return whether it exists.
 *
 * @param blob		FDT blob
 * @param prop_name	property name to look up
 * @return 1, if it exists, or 0 if not
 */
int fdtdec_get_config_bool(const void *blob, const char *prop_name);

/**
 * Look in the FDT for a config item with the given name and return its value
 * as a string.
 *
 * @param blob          FDT blob
 * @param prop_name     property name to look up
 * @returns property string, NULL on error.
 */
char *fdtdec_get_config_string(const void *blob, const char *prop_name);

/*
 * Look up a property in a node and return its contents in a byte
 * array of given length. The property must have at least enough data for
 * the array (count bytes). It may have more, but this will be ignored.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param array		array to fill with data
 * @param count		number of array elements
 * @return 0 if ok, or -FDT_ERR_MISSING if the property is not found,
 *		or -FDT_ERR_BADLAYOUT if not enough data
 */
int fdtdec_get_byte_array(const void *blob, int node, const char *prop_name,
		u8 *array, int count);

/**
 * Look up a property in a node and return a pointer to its contents as a
 * byte array of given length. The property must have at least enough data
 * for the array (count bytes). It may have more, but this will be ignored.
 * The data is not copied.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param count		number of array elements
 * @return pointer to byte array if found, or NULL if the property is not
 *		found or there is not enough data
 */
const u8 *fdtdec_locate_byte_array(const void *blob, int node,
			     const char *prop_name, int count);

/**
 * Look up a property in a node which contains a memory region address and
 * size. Then return a pointer to this address.
 *
 * The property must hold one address with a length. This is only tested on
 * 32-bit machines.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param basep		Returns base address of region
 * @param size		Returns size of region
 * @return 0 if ok, -1 on error (property not found)
 */
int fdtdec_decode_region(const void *blob, int node, const char *prop_name,
			 fdt_addr_t *basep, fdt_size_t *sizep);

enum fmap_compress_t {
	FMAP_COMPRESS_NONE,
	FMAP_COMPRESS_LZO,
};

enum fmap_hash_t {
	FMAP_HASH_NONE,
	FMAP_HASH_SHA1,
	FMAP_HASH_SHA256,
};

/* A flash map entry, containing an offset and length */
struct fmap_entry {
	uint32_t offset;
	uint32_t length;
	uint32_t used;			/* Number of bytes used in region */
	enum fmap_compress_t compress_algo;	/* Compression type */
	enum fmap_hash_t hash_algo;		/* Hash algorithm */
	const uint8_t *hash;			/* Hash value */
	int hash_size;				/* Hash size */
};

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
			   struct fmap_entry *entry);

/**
 * Obtain an indexed resource from a device property.
 *
 * @param fdt		FDT blob
 * @param node		node to examine
 * @param property	name of the property to parse
 * @param index		index of the resource to retrieve
 * @param res		returns the resource
 * @return 0 if ok, negative on error
 */
int fdt_get_resource(const void *fdt, int node, const char *property,
		     unsigned int index, struct fdt_resource *res);

/**
 * Obtain a named resource from a device property.
 *
 * Look up the index of the name in a list of strings and return the resource
 * at that index.
 *
 * @param fdt		FDT blob
 * @param node		node to examine
 * @param property	name of the property to parse
 * @param prop_names	name of the property containing the list of names
 * @param name		the name of the entry to look up
 * @param res		returns the resource
 */
int fdt_get_named_resource(const void *fdt, int node, const char *property,
			   const char *prop_names, const char *name,
			   struct fdt_resource *res);

/**
 * Decode a named region within a memory bank of a given type.
 *
 * This function handles selection of a memory region. The region is
 * specified as an offset/size within a particular type of memory.
 *
 * The properties used are:
 *
 *	<mem_type>-memory<suffix> for the name of the memory bank
 *	<mem_type>-offset<suffix> for the offset in that bank
 *
 * The property value must have an offset and a size. The function checks
 * that the region is entirely within the memory bank.5
 *
 * @param blob		FDT blob
 * @param node		Node containing the properties (-1 for /config)
 * @param mem_type	Type of memory to use, which is a name, such as
 *			"u-boot" or "kernel".
 * @param suffix	String to append to the memory/offset
 *			property names
 * @param basep		Returns base of region
 * @param sizep		Returns size of region
 * @return 0 if OK, -ive on error
 */
int fdtdec_decode_memory_region(const void *blob, int node,
				const char *mem_type, const char *suffix,
				fdt_addr_t *basep, fdt_size_t *sizep);
#endif
