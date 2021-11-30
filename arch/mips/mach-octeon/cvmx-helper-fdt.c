// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * FDT Helper functions similar to those provided to U-Boot.
 */

#include <log.h>
#include <malloc.h>
#include <net.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-fdt.h>
#include <mach/cvmx-helper-gpio.h>

/** Structure used to get type of GPIO from device tree */
struct gpio_compat {
	char *compatible;	  /** Compatible string */
	enum cvmx_gpio_type type; /** Type */
	int8_t size;		  /** (max) Number of pins */
};

#define GPIO_REG_PCA953X_IN	0
#define GPIO_REG_PCA953X_OUT	1
#define GPIO_REG_PCA953X_INVERT 2
#define GPIO_REG_PCA953X_DIR	3

#define GPIO_REG_PCA957X_IN	0
#define GPIO_REG_PCA957X_INVERT 1
#define GPIO_REG_PCA957X_CFG	4
#define GPIO_REG_PCA957X_OUT	5

enum cvmx_i2c_mux_type { I2C_MUX, I2C_SWITCH };

/** Structure used to get type of GPIO from device tree */
struct mux_compat {
	char *compatible;		 /** Compatible string */
	enum cvmx_i2c_bus_type type;	 /** Mux chip type */
	enum cvmx_i2c_mux_type mux_type; /** Type of mux */
	u8 enable;			 /** Enable bit for mux */
	u8 size;			 /** (max) Number of channels */
};

/**
 * Local allocator to handle both SE and U-Boot that also zeroes out memory
 *
 * @param	size	number of bytes to allocate
 *
 * @return	pointer to allocated memory or NULL if out of memory.
 *		Alignment is set to 8-bytes.
 */
void *__cvmx_fdt_alloc(size_t size)
{
	return calloc(size, 1);
}

/**
 * Free allocated memory.
 *
 * @param	ptr	pointer to memory to free
 *
 * NOTE: This only works in U-Boot since SE does not really have a freeing
 *	 mechanism.  In SE the memory is zeroed out.
 */
void __cvmx_fdt_free(void *ptr, size_t size)
{
	free(ptr);
}

/**
 * Look up a phandle and follow it to its node then return the offset of that
 * node.
 *
 * @param[in]	fdt_addr	pointer to FDT blob
 * @param	node		node to read phandle from
 * @param[in]	prop_name	name of property to find
 * @param[in,out] lenp		Number of phandles, input max number
 * @param[out]	nodes		Array of phandle nodes
 *
 * @return	-ve error code on error or 0 for success
 */
int cvmx_fdt_lookup_phandles(const void *fdt_addr, int node,
			     const char *prop_name, int *lenp,
			     int *nodes)
{
	const u32 *phandles;
	int count;
	int i;

	phandles = fdt_getprop(fdt_addr, node, prop_name, &count);
	if (!phandles || count < 0)
		return -FDT_ERR_NOTFOUND;

	count /= 4;
	if (count > *lenp)
		count = *lenp;

	for (i = 0; i < count; i++)
		nodes[i] = fdt_node_offset_by_phandle(fdt_addr,
						      fdt32_to_cpu(phandles[i]));
	*lenp = count;
	return 0;
}

/**
 * Given a FDT node return the CPU node number
 *
 * @param[in]	fdt_addr	Address of FDT
 * @param	node		FDT node number
 *
 * @return	CPU node number or error if negative
 */
int cvmx_fdt_get_cpu_node(const void *fdt_addr, int node)
{
	int parent = node;
	const u32 *ranges;
	int len = 0;

	while (fdt_node_check_compatible(fdt_addr, parent, "simple-bus") != 0) {
		parent = fdt_parent_offset(fdt_addr, parent);
		if (parent < 0)
			return parent;
	}
	ranges = fdt_getprop(fdt_addr, parent, "ranges", &len);
	if (!ranges)
		return len;

	if (len == 0)
		return 0;

	if (len < 24)
		return -FDT_ERR_TRUNCATED;

	return fdt32_to_cpu(ranges[2]) / 0x10;
}

/**
 * Get the total size of the flat device tree
 *
 * @param[in]	fdt_addr	Address of FDT
 *
 * @return	Size of flat device tree in bytes or error if negative.
 */
int cvmx_fdt_get_fdt_size(const void *fdt_addr)
{
	int rc;

	rc = fdt_check_header(fdt_addr);
	if (rc)
		return rc;
	return fdt_totalsize(fdt_addr);
}

/**
 * Returns if a node is compatible with one of the items in the string list
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	node		Node offset to check
 * @param[in]	strlist		Array of FDT device compatibility strings,
 *				must end with NULL or empty string.
 *
 * @return	0 if at least one item matches, 1 if no matches
 */
int cvmx_fdt_node_check_compatible_list(const void *fdt_addr, int node, const char *const *strlist)
{
	while (*strlist && **strlist) {
		if (!fdt_node_check_compatible(fdt_addr, node, *strlist))
			return 0;
		strlist++;
	}
	return 1;
}

/**
 * Given a FDT node, return the next compatible node.
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	start_offset	Starting node offset or -1 to find the first
 * @param	strlist		Array of FDT device compatibility strings, must
 *				end with NULL or empty string.
 *
 * @return	next matching node or -1 if no more matches.
 */
int cvmx_fdt_node_offset_by_compatible_list(const void *fdt_addr, int startoffset,
					    const char *const *strlist)
{
	int offset;

	for (offset = fdt_next_node(fdt_addr, startoffset, NULL); offset >= 0;
	     offset = fdt_next_node(fdt_addr, offset, NULL)) {
		if (!cvmx_fdt_node_check_compatible_list(fdt_addr, offset, strlist))
			return offset;
	}
	return -1;
}

/**
 * Attaches a PHY to a SFP or QSFP.
 *
 * @param	sfp		sfp to attach PHY to
 * @param	phy_info	phy descriptor to attach or NULL to detach
 */
void cvmx_sfp_attach_phy(struct cvmx_fdt_sfp_info *sfp, struct cvmx_phy_info *phy_info)
{
	sfp->phy_info = phy_info;
	if (phy_info)
		phy_info->sfp_info = sfp;
}

/**
 * Assigns an IPD port to a SFP slot
 *
 * @param	sfp		Handle to SFP data structure
 * @param	ipd_port	Port to assign it to
 *
 * @return	0 for success, -1 on error
 */
int cvmx_sfp_set_ipd_port(struct cvmx_fdt_sfp_info *sfp, int ipd_port)
{
	int i;

	if (sfp->is_qsfp) {
		int xiface;
		cvmx_helper_interface_mode_t mode;

		xiface = cvmx_helper_get_interface_num(ipd_port);
		mode = cvmx_helper_interface_get_mode(xiface);
		sfp->ipd_port[0] = ipd_port;

		switch (mode) {
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
		case CVMX_HELPER_INTERFACE_MODE_XFI:
		case CVMX_HELPER_INTERFACE_MODE_10G_KR:
			for (i = 1; i < 4; i++)
				sfp->ipd_port[i] = cvmx_helper_get_ipd_port(xiface, i);
			break;
		case CVMX_HELPER_INTERFACE_MODE_XLAUI:
		case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
			sfp->ipd_port[0] = ipd_port;
			for (i = 1; i < 4; i++)
				sfp->ipd_port[i] = -1;
			break;
		default:
			debug("%s: Interface mode %s for interface 0x%x, ipd_port %d not supported for QSFP\n",
			      __func__, cvmx_helper_interface_mode_to_string(mode), xiface,
			      ipd_port);
			return -1;
		}
	} else {
		sfp->ipd_port[0] = ipd_port;
		for (i = 1; i < 4; i++)
			sfp->ipd_port[i] = -1;
	}
	return 0;
}

/**
 * Parses all of the channels assigned to a VSC7224 device
 *
 * @param[in]		fdt_addr	Address of flat device tree
 * @param		of_offset	Offset of vsc7224 node
 * @param[in,out]	vsc7224		Data structure to hold the data
 *
 * @return	0 for success, -1 on error
 */
static int cvmx_fdt_parse_vsc7224_channels(const void *fdt_addr, int of_offset,
					   struct cvmx_vsc7224 *vsc7224)
{
	int parent_offset = of_offset;
	int err = 0;
	int reg;
	int num_chan = 0;
	struct cvmx_vsc7224_chan *channel;
	struct cvmx_fdt_sfp_info *sfp_info;
	int len;
	int num_taps;
	int i;
	const u32 *tap_values;
	int of_mac;
	int xiface, index;
	bool is_tx;
	bool is_qsfp;
	const char *mac_str;

	debug("%s(%p, %d, %s)\n", __func__, fdt_addr, of_offset, vsc7224->name);
	do {
		/* Walk through all channels */
		of_offset = fdt_node_offset_by_compatible(fdt_addr, of_offset,
							  "vitesse,vsc7224-channel");
		if (of_offset == -FDT_ERR_NOTFOUND) {
			break;
		} else if (of_offset < 0) {
			debug("%s: Failed finding compatible channel\n",
			      __func__);
			err = -1;
			break;
		}
		if (fdt_parent_offset(fdt_addr, of_offset) != parent_offset)
			break;
		reg = cvmx_fdt_get_int(fdt_addr, of_offset, "reg", -1);
		if (reg < 0 || reg > 3) {
			debug("%s: channel reg is either not present or out of range\n",
			      __func__);
			err = -1;
			break;
		}
		is_tx = cvmx_fdt_get_bool(fdt_addr, of_offset, "direction-tx");

		debug("%s(%s): Adding %cx channel %d\n",
		      __func__, vsc7224->name, is_tx ? 't' : 'r',
		      reg);
		tap_values = (const uint32_t *)fdt_getprop(fdt_addr, of_offset, "taps", &len);
		if (!tap_values) {
			debug("%s: Error: no taps defined for vsc7224 channel %d\n",
			      __func__, reg);
			err = -1;
			break;
		}

		if (vsc7224->channel[reg]) {
			debug("%s: Error: channel %d already assigned at %p\n",
			      __func__, reg,
			      vsc7224->channel[reg]);
			err = -1;
			break;
		}
		if (len % 16) {
			debug("%s: Error: tap format error for channel %d\n",
			      __func__, reg);
			err = -1;
			break;
		}
		num_taps = len / 16;
		debug("%s: Adding %d taps\n", __func__, num_taps);

		channel = __cvmx_fdt_alloc(sizeof(*channel) +
					   num_taps * sizeof(struct cvmx_vsc7224_tap));
		if (!channel) {
			debug("%s: Out of memory\n", __func__);
			err = -1;
			break;
		}
		vsc7224->channel[reg] = channel;
		channel->num_taps = num_taps;
		channel->lane = reg;
		channel->of_offset = of_offset;
		channel->is_tx = is_tx;
		channel->pretap_disable = cvmx_fdt_get_bool(fdt_addr, of_offset, "pretap-disable");
		channel->posttap_disable =
			cvmx_fdt_get_bool(fdt_addr, of_offset, "posttap-disable");
		channel->vsc7224 = vsc7224;
		/* Read all the tap values */
		for (i = 0; i < num_taps; i++) {
			channel->taps[i].len = fdt32_to_cpu(tap_values[i * 4 + 0]);
			channel->taps[i].main_tap = fdt32_to_cpu(tap_values[i * 4 + 1]);
			channel->taps[i].pre_tap = fdt32_to_cpu(tap_values[i * 4 + 2]);
			channel->taps[i].post_tap = fdt32_to_cpu(tap_values[i * 4 + 3]);
			debug("%s: tap %d: len: %d, main_tap: 0x%x, pre_tap: 0x%x, post_tap: 0x%x\n",
			      __func__, i, channel->taps[i].len, channel->taps[i].main_tap,
			      channel->taps[i].pre_tap, channel->taps[i].post_tap);
		}
		/* Now find out which interface it's mapped to */
		channel->ipd_port = -1;

		mac_str = "sfp-mac";
		if (fdt_getprop(fdt_addr, of_offset, mac_str, NULL)) {
			is_qsfp = false;
		} else if (fdt_getprop(fdt_addr, of_offset, "qsfp-mac", NULL)) {
			is_qsfp = true;
			mac_str = "qsfp-mac";
		} else {
			debug("%s: Error: MAC not found for %s channel %d\n", __func__,
			      vsc7224->name, reg);
			return -1;
		}
		of_mac = cvmx_fdt_lookup_phandle(fdt_addr, of_offset, mac_str);
		if (of_mac < 0) {
			debug("%s: Error %d with MAC %s phandle for %s\n", __func__, of_mac,
			      mac_str, vsc7224->name);
			return -1;
		}

		debug("%s: Found mac at offset %d\n", __func__, of_mac);
		err = cvmx_helper_cfg_get_xiface_index_by_fdt_node_offset(of_mac, &xiface, &index);
		if (!err) {
			channel->xiface = xiface;
			channel->index = index;
			channel->ipd_port = cvmx_helper_get_ipd_port(xiface, index);

			debug("%s: Found MAC, xiface: 0x%x, index: %d, ipd port: %d\n", __func__,
			      xiface, index, channel->ipd_port);
			if (channel->ipd_port >= 0) {
				cvmx_helper_cfg_set_vsc7224_chan_info(xiface, index, channel);
				debug("%s: Storing config channel for xiface 0x%x, index %d\n",
				      __func__, xiface, index);
			}
			sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
			if (!sfp_info) {
				debug("%s: Warning: no (Q)SFP+ slot found for xinterface 0x%x, index %d for channel %d\n",
				      __func__, xiface, index, channel->lane);
				continue;
			}

			/* Link it */
			channel->next = sfp_info->vsc7224_chan;
			if (sfp_info->vsc7224_chan)
				sfp_info->vsc7224_chan->prev = channel;
			sfp_info->vsc7224_chan = channel;
			sfp_info->is_vsc7224 = true;
			debug("%s: Registering VSC7224 %s channel %d with SFP %s\n", __func__,
			      vsc7224->name, channel->lane, sfp_info->name);
			if (!sfp_info->mod_abs_changed) {
				debug("%s: Registering cvmx_sfp_vsc7224_mod_abs_changed at %p for xinterface 0x%x, index %d\n",
				      __func__, &cvmx_sfp_vsc7224_mod_abs_changed, xiface, index);
				cvmx_sfp_register_mod_abs_changed(
					sfp_info,
					&cvmx_sfp_vsc7224_mod_abs_changed,
					NULL);
			}
		}
	} while (!err && num_chan < 4);

	return err;
}

/**
 * @INTERNAL
 * Parses all instances of the Vitesse VSC7224 reclocking chip
 *
 * @param[in]	fdt_addr	Address of flat device tree
 *
 * @return	0 for success, error otherwise
 */
int __cvmx_fdt_parse_vsc7224(const void *fdt_addr)
{
	int of_offset = -1;
	struct cvmx_vsc7224 *vsc7224 = NULL;
	struct cvmx_fdt_gpio_info *gpio_info = NULL;
	int err = 0;
	int of_parent;
	static bool parsed;

	debug("%s(%p)\n", __func__, fdt_addr);

	if (parsed) {
		debug("%s: Already parsed\n", __func__);
		return 0;
	}
	do {
		of_offset = fdt_node_offset_by_compatible(fdt_addr, of_offset,
							  "vitesse,vsc7224");
		debug("%s: of_offset: %d\n", __func__, of_offset);
		if (of_offset == -FDT_ERR_NOTFOUND) {
			break;
		} else if (of_offset < 0) {
			err = -1;
			debug("%s: Error %d parsing FDT\n",
			      __func__, of_offset);
			break;
		}

		vsc7224 = __cvmx_fdt_alloc(sizeof(*vsc7224));

		if (!vsc7224) {
			debug("%s: Out of memory!\n", __func__);
			return -1;
		}
		vsc7224->of_offset = of_offset;
		vsc7224->i2c_addr = cvmx_fdt_get_int(fdt_addr, of_offset,
						     "reg", -1);
		of_parent = fdt_parent_offset(fdt_addr, of_offset);
		vsc7224->i2c_bus = cvmx_fdt_get_i2c_bus(fdt_addr, of_parent);
		if (vsc7224->i2c_addr < 0) {
			debug("%s: Error: reg field missing\n", __func__);
			err = -1;
			break;
		}
		if (!vsc7224->i2c_bus) {
			debug("%s: Error getting i2c bus\n", __func__);
			err = -1;
			break;
		}
		vsc7224->name = fdt_get_name(fdt_addr, of_offset, NULL);
		debug("%s: Adding %s\n", __func__, vsc7224->name);
		if (fdt_getprop(fdt_addr, of_offset, "reset", NULL)) {
			gpio_info = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "reset");
			vsc7224->reset_gpio = gpio_info;
		}
		if (fdt_getprop(fdt_addr, of_offset, "los", NULL)) {
			gpio_info = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "los");
			vsc7224->los_gpio = gpio_info;
		}
		debug("%s: Parsing channels\n", __func__);
		err = cvmx_fdt_parse_vsc7224_channels(fdt_addr, of_offset, vsc7224);
		if (err) {
			debug("%s: Error parsing VSC7224 channels\n", __func__);
			break;
		}
	} while (of_offset > 0);

	if (err) {
		debug("%s(): Error\n", __func__);
		if (vsc7224) {
			if (vsc7224->reset_gpio)
				__cvmx_fdt_free(vsc7224->reset_gpio, sizeof(*vsc7224->reset_gpio));
			if (vsc7224->los_gpio)
				__cvmx_fdt_free(vsc7224->los_gpio, sizeof(*vsc7224->los_gpio));
			if (vsc7224->i2c_bus)
				cvmx_fdt_free_i2c_bus(vsc7224->i2c_bus);
			__cvmx_fdt_free(vsc7224, sizeof(*vsc7224));
		}
	}
	if (!err)
		parsed = true;

	return err;
}

/**
 * @INTERNAL
 * Parses all instances of the Avago AVSP5410 gearbox phy
 *
 * @param[in]	fdt_addr	Address of flat device tree
 *
 * @return	0 for success, error otherwise
 */
int __cvmx_fdt_parse_avsp5410(const void *fdt_addr)
{
	int of_offset = -1;
	struct cvmx_avsp5410 *avsp5410 = NULL;
	struct cvmx_fdt_sfp_info *sfp_info;
	int err = 0;
	int of_parent;
	static bool parsed;
	int of_mac;
	int xiface, index;
	bool is_qsfp;
	const char *mac_str;

	debug("%s(%p)\n", __func__, fdt_addr);

	if (parsed) {
		debug("%s: Already parsed\n", __func__);
		return 0;
	}

	do {
		of_offset = fdt_node_offset_by_compatible(fdt_addr, of_offset,
							  "avago,avsp-5410");
		debug("%s: of_offset: %d\n", __func__, of_offset);
		if (of_offset == -FDT_ERR_NOTFOUND) {
			break;
		} else if (of_offset < 0) {
			err = -1;
			debug("%s: Error %d parsing FDT\n", __func__, of_offset);
			break;
		}

		avsp5410 = __cvmx_fdt_alloc(sizeof(*avsp5410));

		if (!avsp5410) {
			debug("%s: Out of memory!\n", __func__);
			return -1;
		}
		avsp5410->of_offset = of_offset;
		avsp5410->i2c_addr = cvmx_fdt_get_int(fdt_addr, of_offset,
						      "reg", -1);
		of_parent = fdt_parent_offset(fdt_addr, of_offset);
		avsp5410->i2c_bus = cvmx_fdt_get_i2c_bus(fdt_addr, of_parent);
		if (avsp5410->i2c_addr < 0) {
			debug("%s: Error: reg field missing\n", __func__);
			err = -1;
			break;
		}
		if (!avsp5410->i2c_bus) {
			debug("%s: Error getting i2c bus\n", __func__);
			err = -1;
			break;
		}
		avsp5410->name = fdt_get_name(fdt_addr, of_offset, NULL);
		debug("%s: Adding %s\n", __func__, avsp5410->name);

		/* Now find out which interface it's mapped to */
		avsp5410->ipd_port = -1;

		mac_str = "sfp-mac";
		if (fdt_getprop(fdt_addr, of_offset, mac_str, NULL)) {
			is_qsfp = false;
		} else if (fdt_getprop(fdt_addr, of_offset, "qsfp-mac", NULL)) {
			is_qsfp = true;
			mac_str = "qsfp-mac";
		} else {
			debug("%s: Error: MAC not found for %s\n", __func__, avsp5410->name);
			return -1;
		}
		of_mac = cvmx_fdt_lookup_phandle(fdt_addr, of_offset, mac_str);
		if (of_mac < 0) {
			debug("%s: Error %d with MAC %s phandle for %s\n", __func__, of_mac,
			      mac_str, avsp5410->name);
			return -1;
		}

		debug("%s: Found mac at offset %d\n", __func__, of_mac);
		err = cvmx_helper_cfg_get_xiface_index_by_fdt_node_offset(of_mac, &xiface, &index);
		if (!err) {
			avsp5410->xiface = xiface;
			avsp5410->index = index;
			avsp5410->ipd_port = cvmx_helper_get_ipd_port(xiface, index);

			debug("%s: Found MAC, xiface: 0x%x, index: %d, ipd port: %d\n", __func__,
			      xiface, index, avsp5410->ipd_port);
			if (avsp5410->ipd_port >= 0) {
				cvmx_helper_cfg_set_avsp5410_info(xiface, index, avsp5410);
				debug("%s: Storing config phy for xiface 0x%x, index %d\n",
				      __func__, xiface, index);
			}
			sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
			if (!sfp_info) {
				debug("%s: Warning: no (Q)SFP+ slot found for xinterface 0x%x, index %d\n",
				      __func__, xiface, index);
				continue;
			}

			sfp_info->is_avsp5410 = true;
			sfp_info->avsp5410 = avsp5410;
			debug("%s: Registering AVSP5410 %s with SFP %s\n", __func__, avsp5410->name,
			      sfp_info->name);
			if (!sfp_info->mod_abs_changed) {
				debug("%s: Registering cvmx_sfp_avsp5410_mod_abs_changed at %p for xinterface 0x%x, index %d\n",
				      __func__, &cvmx_sfp_avsp5410_mod_abs_changed, xiface, index);
				cvmx_sfp_register_mod_abs_changed(
					sfp_info,
					&cvmx_sfp_avsp5410_mod_abs_changed,
					NULL);
			}
		}
	} while (of_offset > 0);

	if (err) {
		debug("%s(): Error\n", __func__);
		if (avsp5410) {
			if (avsp5410->i2c_bus)
				cvmx_fdt_free_i2c_bus(avsp5410->i2c_bus);
			__cvmx_fdt_free(avsp5410, sizeof(*avsp5410));
		}
	}
	if (!err)
		parsed = true;

	return err;
}

/**
 * Parse QSFP GPIOs for SFP
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	of_offset	Offset of QSFP node
 * @param[out]	sfp_info	Pointer to sfp info to fill in
 *
 * @return	0 for success
 */
static int cvmx_parse_qsfp(const void *fdt_addr, int of_offset, struct cvmx_fdt_sfp_info *sfp_info)
{
	sfp_info->select = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "select");
	sfp_info->mod_abs = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "mod_prs");
	sfp_info->reset = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "reset");
	sfp_info->interrupt = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "interrupt");
	sfp_info->lp_mode = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "lp_mode");
	return 0;
}

/**
 * Parse SFP GPIOs for SFP
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	of_offset	Offset of SFP node
 * @param[out]	sfp_info	Pointer to sfp info to fill in
 *
 * @return	0 for success
 */
static int cvmx_parse_sfp(const void *fdt_addr, int of_offset, struct cvmx_fdt_sfp_info *sfp_info)
{
	sfp_info->mod_abs = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "mod_abs");
	sfp_info->rx_los = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "rx_los");
	sfp_info->tx_disable = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "tx_disable");
	sfp_info->tx_error = cvmx_fdt_gpio_get_info_phandle(fdt_addr, of_offset, "tx_error");
	return 0;
}

/**
 * Parse SFP/QSFP EEPROM and diag
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	of_offset	Offset of SFP node
 * @param[out]	sfp_info	Pointer to sfp info to fill in
 *
 * @return	0 for success, -1 on error
 */
static int cvmx_parse_sfp_eeprom(const void *fdt_addr, int of_offset,
				 struct cvmx_fdt_sfp_info *sfp_info)
{
	int of_eeprom;
	int of_diag;

	debug("%s(%p, %d, %s)\n", __func__, fdt_addr, of_offset, sfp_info->name);
	of_eeprom = cvmx_fdt_lookup_phandle(fdt_addr, of_offset, "eeprom");
	if (of_eeprom < 0) {
		debug("%s: Missing \"eeprom\" from device tree for %s\n", __func__, sfp_info->name);
		return -1;
	}

	sfp_info->i2c_bus = cvmx_fdt_get_i2c_bus(fdt_addr, fdt_parent_offset(fdt_addr, of_eeprom));
	sfp_info->i2c_eeprom_addr = cvmx_fdt_get_int(fdt_addr, of_eeprom, "reg", 0x50);

	debug("%s(%p, %d, %s, %d)\n", __func__, fdt_addr, of_offset, sfp_info->name,
	      sfp_info->i2c_eeprom_addr);

	if (!sfp_info->i2c_bus) {
		debug("%s: Error: could not determine i2c bus for eeprom for %s\n", __func__,
		      sfp_info->name);
		return -1;
	}
	of_diag = cvmx_fdt_lookup_phandle(fdt_addr, of_offset, "diag");
	if (of_diag >= 0)
		sfp_info->i2c_diag_addr = cvmx_fdt_get_int(fdt_addr, of_diag, "reg", 0x51);
	else
		sfp_info->i2c_diag_addr = 0x51;
	return 0;
}

/**
 * Parse SFP information from device tree
 *
 * @param[in]	fdt_addr	Address of flat device tree
 *
 * @return pointer to sfp info or NULL if error
 */
struct cvmx_fdt_sfp_info *cvmx_helper_fdt_parse_sfp_info(const void *fdt_addr, int of_offset)
{
	struct cvmx_fdt_sfp_info *sfp_info = NULL;
	int err = -1;
	bool is_qsfp;

	if (!fdt_node_check_compatible(fdt_addr, of_offset, "ethernet,sfp-slot")) {
		is_qsfp = false;
	} else if (!fdt_node_check_compatible(fdt_addr, of_offset, "ethernet,qsfp-slot")) {
		is_qsfp = true;
	} else {
		debug("%s: Error: incompatible sfp/qsfp slot, compatible=%s\n", __func__,
		      (char *)fdt_getprop(fdt_addr, of_offset, "compatible", NULL));
		goto error_exit;
	}

	debug("%s: %ssfp module found at offset %d\n", __func__, is_qsfp ? "q" : "", of_offset);
	sfp_info = __cvmx_fdt_alloc(sizeof(*sfp_info));
	if (!sfp_info) {
		debug("%s: Error: out of memory\n", __func__);
		goto error_exit;
	}
	sfp_info->name = fdt_get_name(fdt_addr, of_offset, NULL);
	sfp_info->of_offset = of_offset;
	sfp_info->is_qsfp = is_qsfp;
	sfp_info->last_mod_abs = -1;
	sfp_info->last_rx_los = -1;

	if (is_qsfp)
		err = cvmx_parse_qsfp(fdt_addr, of_offset, sfp_info);
	else
		err = cvmx_parse_sfp(fdt_addr, of_offset, sfp_info);
	if (err) {
		debug("%s: Error in %s parsing %ssfp GPIO info\n", __func__, sfp_info->name,
		      is_qsfp ? "q" : "");
		goto error_exit;
	}
	debug("%s: Parsing %ssfp module eeprom\n", __func__, is_qsfp ? "q" : "");
	err = cvmx_parse_sfp_eeprom(fdt_addr, of_offset, sfp_info);
	if (err) {
		debug("%s: Error parsing eeprom info for %s\n", __func__, sfp_info->name);
		goto error_exit;
	}

	/* Register default check for mod_abs changed */
	if (!err)
		cvmx_sfp_register_check_mod_abs(sfp_info, cvmx_sfp_check_mod_abs, NULL);

error_exit:
	/* Note: we don't free any data structures on error since it gets
	 * rather complicated with i2c buses and whatnot.
	 */
	return err ? NULL : sfp_info;
}

/**
 * @INTERNAL
 * Parse a slice of the Inphi/Cortina CS4343 in the device tree
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	of_offset	fdt offset of slice
 * @param	phy_info	phy_info data structure
 *
 * @return	slice number if non-negative, otherwise error
 */
static int cvmx_fdt_parse_cs4343_slice(const void *fdt_addr, int of_offset,
				       struct cvmx_phy_info *phy_info)
{
	struct cvmx_cs4343_slice_info *slice;
	int reg;
	int reg_offset;

	reg = cvmx_fdt_get_int(fdt_addr, of_offset, "reg", -1);
	reg_offset = cvmx_fdt_get_int(fdt_addr, of_offset, "slice_offset", -1);

	if (reg < 0 || reg >= 4) {
		debug("%s(%p, %d, %p): Error: reg %d undefined or out of range\n", __func__,
		      fdt_addr, of_offset, phy_info, reg);
		return -1;
	}
	if (reg_offset % 0x1000 || reg_offset > 0x3000 || reg_offset < 0) {
		debug("%s(%p, %d, %p): Error: reg_offset 0x%x undefined or out of range\n",
		      __func__, fdt_addr, of_offset, phy_info, reg_offset);
		return -1;
	}
	if (!phy_info->cs4343_info) {
		debug("%s: Error: phy info cs4343 datastructure is NULL\n", __func__);
		return -1;
	}
	debug("%s(%p, %d, %p): %s, reg: %d, slice offset: 0x%x\n", __func__, fdt_addr, of_offset,
	      phy_info, fdt_get_name(fdt_addr, of_offset, NULL), reg, reg_offset);
	slice = &phy_info->cs4343_info->slice[reg];
	slice->name = fdt_get_name(fdt_addr, of_offset, NULL);
	slice->mphy = phy_info->cs4343_info;
	slice->phy_info = phy_info;
	slice->of_offset = of_offset;
	slice->slice_no = reg;
	slice->reg_offset = reg_offset;
	/* SR settings */
	slice->sr_stx_cmode_res = cvmx_fdt_get_int(fdt_addr, of_offset, "sr-stx-cmode-res", 3);
	slice->sr_stx_drv_lower_cm =
		cvmx_fdt_get_int(fdt_addr, of_offset, "sr-stx-drv-lower-cm", 8);
	slice->sr_stx_level = cvmx_fdt_get_int(fdt_addr, of_offset, "sr-stx-level", 0x1c);
	slice->sr_stx_pre_peak = cvmx_fdt_get_int(fdt_addr, of_offset, "sr-stx-pre-peak", 1);
	slice->sr_stx_muxsubrate_sel =
		cvmx_fdt_get_int(fdt_addr, of_offset, "sr-stx-muxsubrate-sel", 0);
	slice->sr_stx_post_peak = cvmx_fdt_get_int(fdt_addr, of_offset, "sr-stx-post-peak", 8);
	/* CX settings */
	slice->cx_stx_cmode_res = cvmx_fdt_get_int(fdt_addr, of_offset, "cx-stx-cmode-res", 3);
	slice->cx_stx_drv_lower_cm =
		cvmx_fdt_get_int(fdt_addr, of_offset, "cx-stx-drv-lower-cm", 8);
	slice->cx_stx_level = cvmx_fdt_get_int(fdt_addr, of_offset, "cx-stx-level", 0x1c);
	slice->cx_stx_pre_peak = cvmx_fdt_get_int(fdt_addr, of_offset, "cx-stx-pre-peak", 1);
	slice->cx_stx_muxsubrate_sel =
		cvmx_fdt_get_int(fdt_addr, of_offset, "cx-stx-muxsubrate-sel", 0);
	slice->cx_stx_post_peak = cvmx_fdt_get_int(fdt_addr, of_offset, "cx-stx-post-peak", 0xC);
	/* 1000Base-X settings */
	/* CX settings */
	slice->basex_stx_cmode_res =
		cvmx_fdt_get_int(fdt_addr, of_offset, "basex-stx-cmode-res", 3);
	slice->basex_stx_drv_lower_cm =
		cvmx_fdt_get_int(fdt_addr, of_offset, "basex-stx-drv-lower-cm", 8);
	slice->basex_stx_level = cvmx_fdt_get_int(fdt_addr, of_offset,
						  "basex-stx-level", 0x1c);
	slice->basex_stx_pre_peak = cvmx_fdt_get_int(fdt_addr, of_offset,
						     "basex-stx-pre-peak", 1);
	slice->basex_stx_muxsubrate_sel =
		cvmx_fdt_get_int(fdt_addr, of_offset,
				 "basex-stx-muxsubrate-sel", 0);
	slice->basex_stx_post_peak =
		cvmx_fdt_get_int(fdt_addr, of_offset, "basex-stx-post-peak", 8);
	/* Get the link LED gpio pin */
	slice->link_gpio = cvmx_fdt_get_int(fdt_addr, of_offset,
					    "link-led-gpio", -1);
	slice->error_gpio = cvmx_fdt_get_int(fdt_addr, of_offset,
					     "error-led-gpio", -1);
	slice->los_gpio = cvmx_fdt_get_int(fdt_addr, of_offset,
					   "los-input-gpio", -1);
	slice->link_inverted = cvmx_fdt_get_bool(fdt_addr, of_offset,
						 "link-led-gpio-inverted");
	slice->error_inverted = cvmx_fdt_get_bool(fdt_addr, of_offset,
						  "error-led-gpio-inverted");
	slice->los_inverted = cvmx_fdt_get_bool(fdt_addr, of_offset,
						"los-input-gpio-inverted");
	/* Convert GPIOs to be die based if they're not already */
	if (slice->link_gpio > 4 && slice->link_gpio <= 8)
		slice->link_gpio -= 4;
	if (slice->error_gpio > 4 && slice->error_gpio <= 8)
		slice->error_gpio -= 4;
	if (slice->los_gpio > 4 && slice->los_gpio <= 8)
		slice->los_gpio -= 4;

	return reg;
}

/**
 * @INTERNAL
 * Parses either a CS4343 phy or a slice of the phy from the device tree
 * @param[in]	fdt_addr	Address of FDT
 * @param	of_offset	offset of slice or phy in device tree
 * @param	phy_info	phy_info data structure to fill in
 *
 * @return	0 for success, -1 on error
 */
int cvmx_fdt_parse_cs4343(const void *fdt_addr, int of_offset, struct cvmx_phy_info *phy_info)
{
	int of_slice = -1;
	struct cvmx_cs4343_info *cs4343;
	int err = -1;
	int reg;

	debug("%s(%p, %d, %p): %s (%s)\n", __func__,
	      fdt_addr, of_offset, phy_info,
	      fdt_get_name(fdt_addr, of_offset, NULL),
	      (const char *)fdt_getprop(fdt_addr, of_offset, "compatible", NULL));

	if (!phy_info->cs4343_info)
		phy_info->cs4343_info = __cvmx_fdt_alloc(sizeof(struct cvmx_cs4343_info));
	if (!phy_info->cs4343_info) {
		debug("%s: Error: out of memory!\n", __func__);
		return -1;
	}
	cs4343 = phy_info->cs4343_info;
	/* If we're passed to a slice then process only that slice */
	if (!fdt_node_check_compatible(fdt_addr, of_offset, "cortina,cs4343-slice")) {
		err = 0;
		of_slice = of_offset;
		of_offset = fdt_parent_offset(fdt_addr, of_offset);
		reg = cvmx_fdt_parse_cs4343_slice(fdt_addr, of_slice, phy_info);
		if (reg >= 0)
			phy_info->cs4343_slice_info = &cs4343->slice[reg];
		else
			err = reg;
	} else if (!fdt_node_check_compatible(fdt_addr, of_offset,
					      "cortina,cs4343")) {
		/* Walk through and process all of the slices */
		of_slice =
			fdt_node_offset_by_compatible(fdt_addr, of_offset, "cortina,cs4343-slice");
		while (of_slice > 0 && fdt_parent_offset(fdt_addr, of_slice) ==
		       of_offset) {
			debug("%s: Parsing slice %s\n", __func__,
			      fdt_get_name(fdt_addr, of_slice, NULL));
			err = cvmx_fdt_parse_cs4343_slice(fdt_addr, of_slice,
							  phy_info);
			if (err < 0)
				break;
			of_slice = fdt_node_offset_by_compatible(fdt_addr,
								 of_slice,
								 "cortina,cs4343-slice");
		}
	} else {
		debug("%s: Error: unknown compatible string %s for %s\n", __func__,
		      (const char *)fdt_getprop(fdt_addr, of_offset,
						"compatible", NULL),
		      fdt_get_name(fdt_addr, of_offset, NULL));
	}

	if (err >= 0) {
		cs4343->name = fdt_get_name(fdt_addr, of_offset, NULL);
		cs4343->phy_info = phy_info;
		cs4343->of_offset = of_offset;
	}

	return err < 0 ? -1 : 0;
}
