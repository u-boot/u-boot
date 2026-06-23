// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <dm/device.h>
#include <dm/test.h>
#include <fw_loader.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_fw_loader_get(struct unit_test_state *uts)
{
	struct udevice *mdio_dev, *fw_loader_dev = NULL;
	ofnode mdio_node, phy_node;
	int ret;

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-test",
					      &mdio_dev));

	mdio_node = dev_ofnode(mdio_dev);
	ut_assert(ofnode_valid(mdio_node));

	phy_node = ofnode_find_subnode(mdio_node, "ethernet-phy@1");
	ut_assert(ofnode_valid(phy_node));

	ret = get_fw_loader_from_node(phy_node, &fw_loader_dev);
	ut_asserteq(-ENODEV, ret);
	ut_assertnull(fw_loader_dev);

	phy_node = ofnode_find_subnode(mdio_node, "ethernet-phy@2");
	ut_assert(ofnode_valid(phy_node));

	fw_loader_dev = NULL;
	ret = get_fw_loader_from_node(phy_node, &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	return 0;
}
DM_TEST(dm_test_fw_loader_get, UTF_SCAN_FDT);

static int dm_test_fw_loader_get_fw_size(struct unit_test_state *uts)
{
	struct udevice *mdio_dev, *fw_loader_dev = NULL;
	ofnode mdio_node, phy_node;
	int ret;

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-test",
					      &mdio_dev));

	mdio_node = dev_ofnode(mdio_dev);
	ut_assert(ofnode_valid(mdio_node));

	phy_node = ofnode_find_subnode(mdio_node, "ethernet-phy@2");
	ut_assert(ofnode_valid(phy_node));

	ret = get_fw_loader_from_node(phy_node, &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	ret = request_firmware_size(fw_loader_dev, "firmware.bin");
	ut_asserteq(256, ret);

	return 0;
}
DM_TEST(dm_test_fw_loader_get_fw_size, UTF_SCAN_FDT);

static int dm_test_fw_loader_get_fw_in_buf(struct unit_test_state *uts)
{
	struct udevice *mdio_dev, *fw_loader_dev = NULL;
	ofnode mdio_node, phy_node;
	int firmware_size = 256;
	char *buf;
	int ret;

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-test",
					      &mdio_dev));

	mdio_node = dev_ofnode(mdio_dev);
	ut_assert(ofnode_valid(mdio_node));

	phy_node = ofnode_find_subnode(mdio_node, "ethernet-phy@2");
	ut_assert(ofnode_valid(phy_node));

	ret = get_fw_loader_from_node(phy_node, &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	ret = request_firmware_size(fw_loader_dev, "firmware.bin");
	ut_asserteq(firmware_size, ret);

	buf = calloc(firmware_size, sizeof(*buf));
	ut_assertnonnull(buf);

	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size - 1, 0);
	ut_asserteq(firmware_size - 1, ret);

	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size, 10);
	ut_asserteq(firmware_size - 10, ret);

	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size, firmware_size);
	ut_asserteq(0, ret);

	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size, 0);
	ut_asserteq(firmware_size, ret);

	return 0;
}
DM_TEST(dm_test_fw_loader_get_fw_in_buf, UTF_SCAN_FDT);
