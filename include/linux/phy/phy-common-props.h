/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * phy-common-props.h -- Common properties for generic PHYs
 *
 * Copyright 2025-2026 NXP
 */

#ifndef __PHY_COMMON_PROPS_H
#define __PHY_COMMON_PROPS_H

#include <dt-bindings/phy/phy.h>
#include <dm/ofnode.h>

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
			unsigned int *val);

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
			unsigned int *val);

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
			       unsigned int *val);

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
			       unsigned int *val);

#endif /* __PHY_COMMON_PROPS_H */
