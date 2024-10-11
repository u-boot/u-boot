/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UPL handoff generation
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __UPL_WRITE_H
#define __UPL_WRITE_H

#ifndef USE_HOSTCC

#include <alist.h>
#include <image.h>
#include <dm/ofnode_decl.h>

struct unit_test_state;

#define UPLP_ADDRESS_CELLS	"#address-cells"
#define UPLP_SIZE_CELLS		"#size-cells"

#define UPLN_OPTIONS		"options"
#define UPLN_UPL_PARAMS		"upl-params"
#define UPLP_SMBIOS		"smbios"
#define UPLP_ACPI		"acpi"
#define UPLP_BOOTMODE		"bootmode"
#define UPLP_ADDR_WIDTH		"addr-width"
#define UPLP_ACPI_NVS_SIZE	"acpi-nvs-size"

#define UPLPATH_UPL_IMAGE	"/options/upl-image"
#define UPLN_UPL_IMAGE		"upl-image"
#define UPLN_IMAGE		"image"
#define UPLP_FIT		"fit"
#define UPLP_CONF_OFFSET	"conf-offset"
#define UPLP_LOAD		"load"
#define UPLP_SIZE		"size"
#define UPLP_OFFSET		"offset"
#define UPLP_DESCRIPTION	"description"

#define UPLN_MEMORY		"memory"
#define UPLP_HOTPLUGGABLE	"hotpluggable"

#define UPLPATH_MEMORY_MAP	"/memory-map"
#define UPLN_MEMORY_MAP		"memory-map"
#define UPLP_USAGE		"usage"

#define UPLN_MEMORY_RESERVED	"reserved-memory"
#define UPLPATH_MEMORY_RESERVED	"/reserved-memory"
#define UPLP_NO_MAP		"no-map"

#define UPLN_SERIAL		"serial"
#define UPLP_REG		"reg"
#define UPLP_COMPATIBLE		"compatible"
#define UPLP_CLOCK_FREQUENCY	"clock-frequency"
#define UPLP_CURRENT_SPEED	"current-speed"
#define UPLP_REG_IO_SHIFT	"reg-io-shift"
#define UPLP_REG_OFFSET		"reg-offset"
#define UPLP_REG_IO_WIDTH	"reg-io-width"
#define UPLP_VIRTUAL_REG	"virtual-reg"
#define UPLP_ACCESS_TYPE	"access-type"

#define UPLN_GRAPHICS		"framebuffer"
#define UPLC_GRAPHICS		"simple-framebuffer"
#define UPLP_WIDTH		"width"
#define UPLP_HEIGHT		"height"
#define UPLP_STRIDE		"stride"
#define UPLP_GRAPHICS_FORMAT	"format"

/**
 * enum upl_boot_mode - Encodes the boot mode
 *
 * Each is a bit number from the boot_mode mask
 */
enum upl_boot_mode {
	UPLBM_FULL,
	UPLBM_MINIMAL,
	UPLBM_FAST,
	UPLBM_DIAG,
	UPLBM_DEFAULT,
	UPLBM_S2,
	UPLBM_S3,
	UPLBM_S4,
	UPLBM_S5,
	UPLBM_FACTORY,
	UPLBM_FLASH,
	UPLBM_RECOVERY,

	UPLBM_COUNT,
};

/**
 * struct upl_image - UPL image informaiton
 *
 * @load: Address image was loaded to
 * @size: Size of image in bytes
 * @offset: Offset of the image in the FIT (0=none)
 * @desc: Description of the iamge (taken from the FIT)
 */
struct upl_image {
	ulong load;
	ulong size;
	uint offset;
	const char *description;
};

/**
 * struct memregion - Information about a region of memory
 *
 * @base: Base address
 * @size: Size in bytes
 */
struct memregion {
	ulong base;
	ulong size;
};

/**
 * struct upl_mem - Information about physical-memory layout
 *
 * TODO: Figure out initial-mapped-area
 *
 * @region: Memory region list (struct memregion)
 * @hotpluggable: true if hotpluggable
 */
struct upl_mem {
	struct alist region;
	bool hotpluggable;
};

/**
 * enum upl_usage - Encodes the usage
 *
 * Each is a bit number from the usage mask
 */
enum upl_usage {
	UPLUS_ACPI_RECLAIM,
	UPLUS_ACPI_NVS,
	UPLUS_BOOT_CODE,
	UPLUS_BOOT_DATA,
	UPLUS_RUNTIME_CODE,
	UPLUS_RUNTIME_DATA,
	UPLUS_COUNT
};

/**
 * struct upl_memmap - Information about logical-memory layout
 *
 * @name: Node name to use
 * @region: Memory region list (struct memregion)
 * @usage: Memory-usage mask (enum upl_usage)
 */
struct upl_memmap {
	const char *name;
	struct alist region;
	uint usage;
};

/**
 * struct upl_memres - Reserved memory
 *
 * @name: Node name to use
 * @region: Reserved memory region list (struct memregion)
 * @no_map: true to indicate that a virtual mapping must not be created
 */
struct upl_memres {
	const char *name;
	struct alist region;
	bool no_map;
};

enum upl_serial_access_type {
	UPLSAT_MMIO,
	UPLSAT_IO,
};

/* serial defaults */
enum {
	UPLD_REG_IO_SHIFT	= 0,
	UPLD_REG_OFFSET		= 0,
	UPLD_REG_IO_WIDTH	= 1,
};

/**
 * enum upl_access_type - Access types
 *
 * @UPLAT_MMIO: Memory-mapped I/O
 * @UPLAT_IO: Separate I/O
 */
enum upl_access_type {
	UPLAT_MMIO,
	UPLAT_IO,
};

/**
 * struct upl_serial - Serial console
 *
 * @compatible: Compatible string (NULL if there is no serial console)
 * @clock_frequency: Input clock frequency of UART
 * @current_speed: Current baud rate of UART
 * @reg: List of base address and size of registers (struct memregion)
 * @reg_shift_log2: log2 of distance between each register
 * @reg_offset: Offset of registers from the base address
 * @reg_width: Register width in bytes
 * @virtual_reg: Virtual register access (0 for none)
 * @access_type: Register access type to use
 */
struct upl_serial {
	const char *compatible;
	uint clock_frequency;
	uint current_speed;
	struct alist reg;
	uint reg_io_shift;
	uint reg_offset;
	uint reg_io_width;
	ulong virtual_reg;
	enum upl_serial_access_type access_type;
};

/**
 * enum upl_graphics_format - Graphics formats
 *
 * @UPLGF_ARGB32: 32bpp format using 0xaarrggbb
 * @UPLGF_ABGR32: 32bpp format using 0xaabbggrr
 * @UPLGF_ARGB64: 64bpp format using 0xaaaabbbbggggrrrr
 */
enum upl_graphics_format {
	UPLGF_ARGB32,
	UPLGF_ABGR32,
	UPLGF_ABGR64,
};

/**
 * @reg: List of base address and size of registers (struct memregion)
 * @width: Width of display in pixels
 * @height: Height of display in pixels
 * @stride: Number of bytes from one line to the next
 * @format: Pixel format
 */
struct upl_graphics {
	struct alist reg;
	uint width;
	uint height;
	uint stride;
	enum upl_graphics_format format;
};

/*
 * Information about the UPL state
 *
 * @addr_cells: Number of address cells used in the handoff
 * @size_cells: Number of size cells used in the handoff
 * @bootmode: Boot-mode mask (enum upl_boot_mode)
 * @fit: Address of FIT image that was loaded
 * @conf_offset: Offset in FIT of the configuration that was selected
 * @addr_width: Adress-bus width of machine, e.g. 46 for 46 bits
 * @acpi_nvs_size: Size of the ACPI non-volatile-storage area in bytes
 * @image: Information about each image (struct upl_image)
 * @mem: Information about physical-memory regions (struct upl_mem)
 * @nennap: Information about logical-memory regions (struct upl_memmap)
 * @nennap: Information about reserved-memory regions (struct upl_memres)
 */
struct upl {
	int addr_cells;
	int size_cells;

	ulong smbios;
	ulong acpi;
	uint bootmode;
	ulong fit;
	uint conf_offset;
	uint addr_width;
	uint acpi_nvs_size;

	struct alist image;
	struct alist mem;
	struct alist memmap;
	struct alist memres;
	struct upl_serial serial;
	struct upl_graphics graphics;
};

/**
 * upl_write_handoff() - Write a Unversal Payload handoff structure
 *
 * upl: UPL state to write
 * @root: root node to write it to
 * @skip_existing: Avoid recreating any nodes which already exist in the
 * devicetree. For example, if there is a serial node, just leave it alone,
 * since don't need to create a new one
 * Return: 0 on success, -ve on error
 */
int upl_write_handoff(const struct upl *upl, ofnode root, bool skip_existing);

/**
 * upl_create_handoff_tree() - Write a Unversal Payload handoff structure
 *
 * upl: UPL state to write
 * @treep: Returns a new tree containing the handoff
 * Return: 0 on success, -ve on error
 */
int upl_create_handoff_tree(const struct upl *upl, oftree *treep);

/**
 * upl_read_handoff() - Read a Unversal Payload handoff structure
 *
 * upl: UPL state to read into
 * @tree: Devicetree containing the data to read
 * Return: 0 on success, -ve on error
 */
int upl_read_handoff(struct upl *upl, oftree tree);

/**
 * upl_get_test_data() - Fill a UPL with some test data
 *
 * @uts: Test state (can be uninited)
 * @upl: Returns test data
 * Return: 0 on success, 1 on error
 */
int upl_get_test_data(struct unit_test_state *uts, struct upl *upl);
#endif /* USE_HOSTCC */

#if CONFIG_IS_ENABLED(UPL) && defined(CONFIG_XPL_BUILD)

/**
 * upl_set_fit_info() - Set up basic info about the FIT
 *
 * @fit: Address of FIT
 * @conf_offset: Configuration node being used
 * @entry_addr: Entry address for next phase
 */
void upl_set_fit_info(ulong fit, int conf_offset, ulong entry_addr);

/**
 * upl_set_fit_addr() - Set up the address of the FIT
 *
 * @fit: Address of FIT
 */
void upl_set_fit_addr(ulong fit);

#else
static inline void upl_set_fit_addr(ulong fit) {}
static inline void upl_set_fit_info(ulong fit, int conf_offset,
				    ulong entry_addr) {}
#endif /* UPL && SPL */

/**
 * _upl_add_image() - Internal function to add a new image to the UPL
 *
 * @node: Image node offset in FIT
 * @load_addr: Address to which images was loaded
 * @size: Image size in bytes
 * @desc: Description of image
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int _upl_add_image(int node, ulong load_addr, ulong size, const char *desc);

/**
 * upl_add_image() - Add a new image to the UPL
 *
 * @fit: Pointer to FIT
 * @node: Image node offset in FIT
 * @load_addr: Address to which images was loaded
 * @size: Image size in bytes
 * Return: 0 if OK, -ENOMEM if out of memory
 */
static inline int upl_add_image(const void *fit, int node, ulong load_addr,
				ulong size)
{
	if (CONFIG_IS_ENABLED(UPL) && IS_ENABLED(CONFIG_XPL_BUILD)) {
		const char *desc = fdt_getprop(fit, node, FIT_DESC_PROP, NULL);

		return _upl_add_image(node, load_addr, size, desc);
	}

	return 0;
}

/** upl_init() - Set up a UPL struct */
void upl_init(struct upl *upl);

#endif /* __UPL_WRITE_H */
