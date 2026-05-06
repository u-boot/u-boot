// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * U-Boot sandbox DM tests for PHY common props
 *
 * Ported from Linux KUnit test:
 *   linux/drivers/phy/phy-common-props-test.c
 *
 * Copyright 2025-2026 NXP
 */
#include <dm.h>
#include <dm/ofnode.h>
#include <dm/test.h>
#include <linux/bitops.h>
#include <linux/phy/phy-common-props.h>
#include <dt-bindings/phy/phy.h>
#include <test/test.h>
#include <test/ut.h>

/* --- RX polarity tests -------------------------------------------------- */

/* Test: rx-polarity property is missing => default PHY_POL_NORMAL */
static int dm_test_phy_common_props_rx_missing(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-missing");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_NORMAL, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_missing, UTF_SCAN_FDT);

/* Test: rx-polarity has more values than rx-polarity-names => -EINVAL */
static int dm_test_phy_common_props_rx_more_values(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-more-values");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(-EINVAL, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_more_values, UTF_SCAN_FDT);

/* Test: rx-polarity has 1 value and rx-polarity-names does not exist */
static int dm_test_phy_common_props_rx_single_value(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-single");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_INVERT, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_single_value, UTF_SCAN_FDT);

/* Test: rx-polarity-names has more values than rx-polarity => -EINVAL */
static int dm_test_phy_common_props_rx_more_names(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-more-names");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(-EINVAL, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_more_names, UTF_SCAN_FDT);

/* Test: valid arrays, find polarity by mode name */
static int dm_test_phy_common_props_rx_find_by_name(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-find-by-name");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_NORMAL, val);

	ret = phy_get_manual_rx_polarity(node, "2500base-x", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_INVERT, val);

	/* "usb-ss" has PHY_POL_AUTO; auto is supported here */
	ret = phy_get_rx_polarity(node, "usb-ss", BIT(PHY_POL_AUTO),
				  PHY_POL_AUTO, &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_AUTO, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_find_by_name, UTF_SCAN_FDT);

/* Test: name not found, no "default" entry => -EINVAL */
static int dm_test_phy_common_props_rx_no_default(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-no-default");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(-EINVAL, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_no_default, UTF_SCAN_FDT);

/* Test: name not found, "default" entry exists => use default polarity */
static int dm_test_phy_common_props_rx_with_default(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-with-default");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_INVERT, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_with_default, UTF_SCAN_FDT);

/* Test: polarity value found but not in supported set => -EOPNOTSUPP */
static int dm_test_phy_common_props_rx_unsupported(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-unsupported");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_rx_polarity(node, "sgmii", &val);
	ut_asserteq(-EOPNOTSUPP, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_rx_unsupported, UTF_SCAN_FDT);

/* --- TX polarity tests -------------------------------------------------- */

/* Test: tx-polarity property is missing => default PHY_POL_NORMAL */
static int dm_test_phy_common_props_tx_missing(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-missing");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_NORMAL, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_missing, UTF_SCAN_FDT);

/* Test: tx-polarity has more values than tx-polarity-names => -EINVAL */
static int dm_test_phy_common_props_tx_more_values(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-more-values");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(-EINVAL, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_more_values, UTF_SCAN_FDT);

/* Test: tx-polarity has 1 value and tx-polarity-names does not exist */
static int dm_test_phy_common_props_tx_single_value(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-single");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_INVERT, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_single_value, UTF_SCAN_FDT);

/* Test: tx-polarity-names has more values than tx-polarity => -EINVAL */
static int dm_test_phy_common_props_tx_more_names(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-more-names");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(-EINVAL, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_more_names, UTF_SCAN_FDT);

/* Test: valid arrays, find polarity by mode name */
static int dm_test_phy_common_props_tx_find_by_name(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-find-by-name");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_NORMAL, val);

	ret = phy_get_manual_tx_polarity(node, "2500base-x", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_INVERT, val);

	ret = phy_get_manual_tx_polarity(node, "1000base-x", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_NORMAL, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_find_by_name, UTF_SCAN_FDT);

/* Test: name not found, no "default" entry => -EINVAL */
static int dm_test_phy_common_props_tx_no_default(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-no-default");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(-EINVAL, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_no_default, UTF_SCAN_FDT);

/* Test: name not found, "default" entry exists => use default polarity */
static int dm_test_phy_common_props_tx_with_default(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-with-default");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(0, ret);
	ut_asserteq(PHY_POL_INVERT, val);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_with_default, UTF_SCAN_FDT);

/* Test: polarity value found but not in supported set => -EOPNOTSUPP */
static int dm_test_phy_common_props_tx_unsupported(struct unit_test_state *uts)
{
	ofnode node = ofnode_path("/phy-common-props-unsupported");
	unsigned int val;
	int ret;

	ut_assert(ofnode_valid(node));

	ret = phy_get_manual_tx_polarity(node, "sgmii", &val);
	ut_asserteq(-EOPNOTSUPP, ret);

	return 0;
}

DM_TEST(dm_test_phy_common_props_tx_unsupported, UTF_SCAN_FDT);
