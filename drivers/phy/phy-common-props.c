// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * phy-common-props.c  --  Common PHY properties
 *
 * Copyright 2025-2026 NXP
 */
#include <dm/ofnode.h>
#include <log.h>
#include <malloc.h>
#include <linux/phy/phy-common-props.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>

/**
 * ofnode_count_u32_prop - Count number of u32 elements in a property
 * @node: Device tree node
 * @propname: Property name
 *
 * Return: number of u32 elements, or negative error code
 */
static int ofnode_count_u32_prop(ofnode node, const char *propname)
{
	int size;

	size = ofnode_read_size(node, propname);
	if (size < 0) {
		pr_debug("%s: property '%s' not found (err=%d)\n",
			 __func__, propname, size);
		return size;
	}

	pr_debug("%s: property '%s' has %zu bytes (%zu elements)\n",
		 __func__, propname, (size_t)size, (size_t)(size / sizeof(u32)));

	return size / sizeof(u32);
}

/**
 * ofnode_get_u32_prop_for_name - Find u32 property by name, or default value
 * @node: Device tree node; if invalid or @props_title is absent, @default_val is used
 * @name: Property name used as lookup key in @names_title (must not be NULL)
 * @props_title: Name of u32 array property holding values
 * @names_title: Name of string array property holding lookup keys
 * @default_val: Default value if @node is invalid, @props_title is absent, or empty
 * @val: Pointer to store the returned value
 *
 * This function retrieves a u32 value from @props_title based on a name lookup
 * in @names_title. The value stored in @val is determined as follows:
 *
 * - If @node is invalid or @props_title is absent: @default_val is used
 * - If @props_title exists but is empty: @default_val is used
 * - If @props_title has exactly one element and @names_title is empty:
 *   that element is used
 * - Otherwise: @val is set to the element at the same index where @name is
 *   found in @names_title.
 * - If @name is not found, the function looks for a "default" entry in
 *   @names_title and uses the corresponding value from @props_title
 *
 * When both @props_title and @names_title are present, they must have the
 * same number of elements (except when @props_title has exactly one element).
 *
 * Return: zero on success, negative error on failure.
 */
static int ofnode_get_u32_prop_for_name(ofnode node, const char *name,
					const char *props_title,
					const char *names_title,
					unsigned int default_val,
					unsigned int *val)
{
	int err, n_props, n_names, idx;
	u32 *props;

	if (!name) {
		pr_err("Error: Lookup key inside \"%s\" is mandatory\n",
		       names_title);
		return -EINVAL;
	}

	pr_debug("%s: looking up '%s' in props='%s' names='%s' default=%u\n",
		 __func__, name, props_title, names_title, default_val);

	n_props = ofnode_count_u32_prop(node, props_title);
	if (n_props < 0) {
		/* property is absent */
		pr_debug("%s: '%s' is absent, using default value %u\n",
			 __func__, props_title, default_val);
		*val = default_val;
		return 0;
	}
	if (n_props == 0) {
		/* property exists but is empty, use default */
		pr_debug("%s: '%s' is empty, using default value %u\n",
			 __func__, props_title, default_val);
		*val = default_val;
		return 0;
	}

	n_names = ofnode_read_string_count(node, names_title);

	pr_debug("%s: '%s' has %d elements, '%s' has %d entries\n",
		 __func__, props_title, n_props, names_title, n_names);
	if (n_names >= 0 && n_props != n_names) {
		pr_err("Error: mismatch between \"%s\" and \"%s\" property count (%d vs %d)\n",
		       props_title, names_title, n_props, n_names);
		return -EINVAL;
	}

	idx = ofnode_stringlist_search(node, names_title, name);
	if (idx >= 0) {
		pr_debug("%s: found '%s' at index %d in '%s'\n",
			 __func__, name, idx, names_title);
	} else {
		pr_debug("%s: '%s' not found in '%s', trying 'default'\n",
			 __func__, name, names_title);
		idx = ofnode_stringlist_search(node, names_title, "default");
		if (idx >= 0)
			pr_debug("%s: 'default' entry found at index %d\n",
				 __func__, idx);
		else
			pr_debug("%s: 'default' entry not found in '%s'\n",
				 __func__, names_title);
	}
	/*
	 * If the mode name is missing, it can only mean the specified property
	 * is the default one for all modes, so reject any other property count
	 * than 1.
	 */
	if (idx < 0 && n_props != 1) {
		pr_err("Error: \"%s\" property has %d elements, but cannot find \"%s\" in \"%s\" and there is no default value\n",
		       props_title, n_props, name, names_title);
		return -EINVAL;
	}

	if (n_props == 1) {
		pr_debug("%s: single-element '%s', reading directly\n",
			 __func__, props_title);
		err = ofnode_read_u32(node, props_title, val);
		if (err) {
			pr_debug("%s: failed to read '%s' (err=%d)\n",
				 __func__, props_title, err);
			return err;
		}
		pr_debug("%s: resolved value %u for name '%s' from '%s'\n",
			 __func__, *val, name, props_title);
		return 0;
	}

	/* We implicitly know idx >= 0 here */
	props = calloc(n_props, sizeof(*props));
	if (!props)
		return -ENOMEM;

	err = ofnode_read_u32_array(node, props_title, props, n_props);
	if (err >= 0) {
		*val = props[idx];
		pr_debug("%s: resolved value %u at index %d for name '%s' from '%s'\n",
			 __func__, *val, idx, name, props_title);
	} else {
		pr_debug("%s: failed to read u32 array '%s' (err=%d)\n",
			 __func__, props_title, err);
	}

	free(props);

	return err;
}

/**
 * phy_get_polarity_for_mode - Get polarity for a specific PHY mode
 * @node: Device tree node
 * @mode_name: The name of the PHY mode to look up
 * @supported: Bit mask of supported polarity values
 * @default_val: Default polarity value if property is missing
 * @polarity_prop: Name of the polarity property
 * @names_prop: Name of the names property
 * @val: Pointer to returned polarity
 *
 * Return: zero on success, negative error on failure.
 */
static int phy_get_polarity_for_mode(ofnode node, const char *mode_name,
				     unsigned int supported,
				     unsigned int default_val,
				     const char *polarity_prop,
				     const char *names_prop,
				     unsigned int *val)
{
	int err;

	pr_debug("%s: querying '%s' for mode '%s' (supported=0x%x, default=%u)\n",
		 __func__, polarity_prop, mode_name, supported, default_val);

	err = ofnode_get_u32_prop_for_name(node, mode_name, polarity_prop,
					   names_prop, default_val, val);
	if (err) {
		pr_debug("%s: '%s' lookup failed for mode '%s' (err=%d)\n",
			 __func__, polarity_prop, mode_name, err);
		return err;
	}

	pr_debug("%s: '%s' for mode '%s' = %u\n",
		 __func__, polarity_prop, mode_name, *val);

	if (!(supported & BIT(*val))) {
		pr_err("Error: %d is not a supported value for '%s' element '%s'\n",
		       *val, polarity_prop, mode_name);
		err = -EOPNOTSUPP;
	}

	return err;
}

/**
 * phy_get_rx_polarity - Get RX polarity for PHY differential lane
 * @node: Pointer to the PHY's device tree node.
 * @mode_name: The name of the PHY mode to look up.
 * @supported: Bit mask of PHY_POL_NORMAL, PHY_POL_INVERT and PHY_POL_AUTO
 * @default_val: Default polarity value if property is missing
 * @val: Pointer to returned polarity.
 *
 * Return: zero on success, negative error on failure.
 */
int phy_get_rx_polarity(ofnode node, const char *mode_name,
			unsigned int supported, unsigned int default_val,
			unsigned int *val)
{
	return phy_get_polarity_for_mode(node, mode_name, supported,
					 default_val, "rx-polarity",
					 "rx-polarity-names", val);
}

/**
 * phy_get_tx_polarity - Get TX polarity for PHY differential lane
 * @node: Pointer to the PHY's device tree node.
 * @mode_name: The name of the PHY mode to look up.
 * @supported: Bit mask of PHY_POL_NORMAL, PHY_POL_INVERT and PHY_POL_AUTO
 * @default_val: Default polarity value if property is missing
 * @val: Pointer to returned polarity.
 *
 * Return: zero on success, negative error on failure.
 */
int phy_get_tx_polarity(ofnode node, const char *mode_name,
			unsigned int supported, unsigned int default_val,
			unsigned int *val)
{
	return phy_get_polarity_for_mode(node, mode_name, supported,
					 default_val, "tx-polarity",
					 "tx-polarity-names", val);
}

/**
 * phy_get_manual_rx_polarity - Get manual RX polarity for PHY differential lane
 * @node: Pointer to the PHY's device tree node.
 * @mode_name: The name of the PHY mode to look up.
 * @val: Pointer to returned polarity.
 *
 * Helper for PHYs which do not support protocols with automatic RX polarity
 * detection and correction.
 *
 * Return: zero on success, negative error on failure.
 */
int phy_get_manual_rx_polarity(ofnode node, const char *mode_name,
			       unsigned int *val)
{
	return phy_get_rx_polarity(node, mode_name,
				   BIT(PHY_POL_NORMAL) | BIT(PHY_POL_INVERT),
				   PHY_POL_NORMAL, val);
}

/**
 * phy_get_manual_tx_polarity - Get manual TX polarity for PHY differential lane
 * @node: Pointer to the PHY's device tree node.
 * @mode_name: The name of the PHY mode to look up.
 * @val: Pointer to returned polarity.
 *
 * Helper for PHYs without any custom default value for the TX polarity.
 *
 * Return: zero on success, negative error on failure.
 */
int phy_get_manual_tx_polarity(ofnode node, const char *mode_name,
			       unsigned int *val)
{
	return phy_get_tx_polarity(node, mode_name,
				   BIT(PHY_POL_NORMAL) | BIT(PHY_POL_INVERT),
				   PHY_POL_NORMAL, val);
}
