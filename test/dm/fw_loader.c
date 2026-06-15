// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/test.h>
#include <fw_loader.h>
#include <test/test.h>
#include <test/ut.h>

/* uuidgen -n @dns --md5 --name test = 45a113ac-c7f2-30b0-90a5-a399ab912716*/
#define TEST_UUID	"45a113ac-c7f2-30b0-90a5-a399ab912716"
#define TEST_INVAL_UUID	"300b74d6-cc34-4e68-ba5d-733157f633f7"

static int bind_mmc(struct unit_test_state *uts, const char *mmc)
{
	struct udevice *dev;
	ofnode root, node;

	/* Enable the mmc node for this test */
	root = oftree_root(oftree_default());
	node = ofnode_find_subnode(root, mmc);
	ut_assert(ofnode_valid(node));
	ut_assertok(lists_bind_fdt(gd->dm_root, node, &dev, NULL, false));

	return 0;
}

static int fw_loader_get_dev(struct unit_test_state *uts,
			     const char *name, struct udevice **devp)
{
	struct udevice *mdio_dev;
	ofnode mdio_node, phy_node;

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-test",
					      &mdio_dev));

	mdio_node = dev_ofnode(mdio_dev);
	ut_assert(ofnode_valid(mdio_node));

	phy_node = ofnode_find_subnode(mdio_node, name);
	ut_assert(ofnode_valid(phy_node));

	return get_fw_loader_from_node(phy_node, devp);
}

static int dm_test_fs_loader_get(struct unit_test_state *uts)
{
	struct udevice *fw_loader_dev = NULL;
	int ret;

	ut_assertok(bind_mmc(uts, "mmc12"));

	ret = fw_loader_get_dev(uts, "ethernet-phy@1", &fw_loader_dev);
	ut_asserteq(-ENODEV, ret);
	ut_assertnull(fw_loader_dev);

	fw_loader_dev = NULL;
	ret = fw_loader_get_dev(uts, "ethernet-phy@2", &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	return 0;
}
DM_TEST(dm_test_fs_loader_get, UTF_SCAN_FDT);

static int dm_test_fip_loader_get(struct unit_test_state *uts)
{
	struct udevice *fw_loader_dev = NULL;
	int ret;

	ut_assertok(bind_mmc(uts, "mmc13"));

	ret = fw_loader_get_dev(uts, "ethernet-phy@3", &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	return 0;
}
DM_TEST(dm_test_fip_loader_get, UTF_SCAN_FDT);

static int dm_test_fs_loader_get_fw_size(struct unit_test_state *uts)
{
	struct udevice *fw_loader_dev = NULL;
	int ret;

	ut_assertok(bind_mmc(uts, "mmc12"));

	ret = fw_loader_get_dev(uts, "ethernet-phy@2", &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	ret = request_firmware_size(fw_loader_dev, "firmware.bin");
	ut_asserteq(15, ret);

	return 0;
}
DM_TEST(dm_test_fs_loader_get_fw_size, UTF_SCAN_FDT);

static int dm_test_fip_loader_get_fw_size(struct unit_test_state *uts)
{
	struct udevice *fw_loader_dev = NULL;
	int ret;

	ut_assertok(bind_mmc(uts, "mmc13"));

	ret = fw_loader_get_dev(uts, "ethernet-phy@3", &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	ret = request_firmware_size(fw_loader_dev, TEST_UUID);
	ut_asserteq(5, ret);

	return 0;
}
DM_TEST(dm_test_fip_loader_get_fw_size, UTF_SCAN_FDT);

static int dm_test_fs_loader_get_fw_in_buf(struct unit_test_state *uts)
{
	struct udevice *fw_loader_dev = NULL;
	int firmware_size = 15;
	char *buf;
	int ret;

	ut_assertok(bind_mmc(uts, "mmc12"));

	ret = fw_loader_get_dev(uts, "ethernet-phy@2", &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	ret = request_firmware_size(fw_loader_dev, "firmware.bin");
	ut_asserteq(firmware_size, ret);

	ret = request_firmware_size(fw_loader_dev, "firmware2.bin");
	ut_asserteq(-ENOENT, ret);

	buf = calloc(firmware_size, sizeof(*buf));
	ut_assertnonnull(buf);

	/* Normal read */
	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size, 0);
	ut_asserteq(firmware_size, ret);
	ut_asserteq_mem("This is a test\n", buf, firmware_size);

	/* Read up to size */
	memset(buf, 0, firmware_size);
	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size - 5, 0);
	ut_asserteq(firmware_size - 5, ret);
	ut_asserteq_mem("This is a \0\0\0\0\0", buf, firmware_size);

	/* Read at offset */
	memset(buf, 0, firmware_size);
	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size - 10, 10);
	ut_asserteq(firmware_size - 10, ret);
	ut_asserteq_mem("test\n\0\0\0\0\0\0\0\0\0\0", buf, firmware_size);

	/* Read at offset with bigger buffer but firmware ends */
	memset(buf, 0, firmware_size);
	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size, 10);
	ut_asserteq(firmware_size - 10, ret);
	ut_asserteq_mem("test\n\0\0\0\0\0\0\0\0\0\0", buf, firmware_size);

	/* Read at offset equal to firmware size */
	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size, firmware_size);
	ut_asserteq(0, ret);

	/* Read at offset beyond firmware size */
	ret = request_firmware_into_buf(fw_loader_dev, "firmware.bin",
					buf, firmware_size, firmware_size + 1);
	ut_asserteq(-EINVAL, ret);

	/* Non existent file */
	ret = request_firmware_into_buf(fw_loader_dev, "firmware2.bin",
					buf, firmware_size, 0);
	ut_asserteq(-ENOENT, ret);

	free(buf);
	return 0;
}
DM_TEST(dm_test_fs_loader_get_fw_in_buf, UTF_SCAN_FDT);

static int dm_test_fip_loader_get_fw_in_buf(struct unit_test_state *uts)
{
	struct udevice *fw_loader_dev = NULL;
	int firmware_size = 5;
	char *buf;
	int ret;

	ut_assertok(bind_mmc(uts, "mmc13"));

	ret = fw_loader_get_dev(uts, "ethernet-phy@3", &fw_loader_dev);
	ut_assertok(ret);
	ut_assertnonnull(fw_loader_dev);

	ret = request_firmware_size(fw_loader_dev, TEST_UUID);
	ut_asserteq(firmware_size, ret);

	ret = request_firmware_size(fw_loader_dev, TEST_INVAL_UUID);
	ut_asserteq(-ENOENT, ret);

	buf = calloc(firmware_size, sizeof(*buf));
	ut_assertnonnull(buf);

	/* Normal read */
	ret = request_firmware_into_buf(fw_loader_dev, TEST_UUID,
					buf, firmware_size, 0);
	ut_asserteq(firmware_size, ret);
	ut_asserteq_mem("test\n", buf, firmware_size);

	/* Read up to size */
	memset(buf, 0, firmware_size);
	ret = request_firmware_into_buf(fw_loader_dev, TEST_UUID,
					buf, firmware_size - 1, 0);
	ut_asserteq(firmware_size - 1, ret);
	ut_asserteq_mem("test\0", buf, firmware_size);

	/* Read at offset */
	memset(buf, 0, firmware_size);
	ret = request_firmware_into_buf(fw_loader_dev, TEST_UUID,
					buf, firmware_size - 2, 2);
	ut_asserteq(firmware_size - 2, ret);
	ut_asserteq_mem("st\n\0\0", buf, firmware_size);

	/* Read at offset with bigger buffer but firmware ends */
	memset(buf, 0, firmware_size);
	ret = request_firmware_into_buf(fw_loader_dev, TEST_UUID,
					buf, firmware_size, 2);
	ut_asserteq(firmware_size - 2, ret);
	ut_asserteq_mem("st\n\0\0", buf, firmware_size);

	/* Read at offset equal to firmware size */
	ret = request_firmware_into_buf(fw_loader_dev, TEST_UUID,
					buf, firmware_size, firmware_size);
	ut_asserteq(0, ret);

	/* Read at offset beyond firmware size */
	ret = request_firmware_into_buf(fw_loader_dev, TEST_UUID,
					buf, firmware_size, firmware_size + 1);
	ut_asserteq(-EINVAL, ret);

	/* Non existent file */
	ret = request_firmware_into_buf(fw_loader_dev, TEST_INVAL_UUID,
					buf, firmware_size, 0);
	ut_asserteq(-ENOENT, ret);

	free(buf);
	return 0;
}
DM_TEST(dm_test_fip_loader_get_fw_in_buf, UTF_SCAN_FDT);
