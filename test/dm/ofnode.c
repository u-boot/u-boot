// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <log.h>
#include <of_live.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_extra.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_ofnode_compatible(struct unit_test_state *uts)
{
	ofnode root_node = ofnode_path("/");

	ut_assert(ofnode_valid(root_node));
	ut_assert(ofnode_device_is_compatible(root_node, "sandbox"));

	return 0;
}
DM_TEST(dm_test_ofnode_compatible, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_get_by_phandle(struct unit_test_state *uts)
{
	/* test invalid phandle */
	ut_assert(!ofnode_valid(ofnode_get_by_phandle(0)));
	ut_assert(!ofnode_valid(ofnode_get_by_phandle(-1)));

	/* test first valid phandle */
	ut_assert(ofnode_valid(ofnode_get_by_phandle(1)));

	/* test unknown phandle */
	ut_assert(!ofnode_valid(ofnode_get_by_phandle(0x1000000)));

	return 0;
}
DM_TEST(dm_test_ofnode_get_by_phandle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_by_prop_value(struct unit_test_state *uts)
{
	const char propname[] = "compatible";
	const char propval[] = "denx,u-boot-fdt-test";
	const char *str;
	ofnode node = ofnode_null();

	/* Find first matching node, there should be at least one */
	node = ofnode_by_prop_value(node, propname, propval, sizeof(propval));
	ut_assert(ofnode_valid(node));
	str = ofnode_read_string(node, propname);
	ut_assert(str && !strcmp(str, propval));

	/* Find the rest of the matching nodes */
	while (true) {
		node = ofnode_by_prop_value(node, propname, propval,
					    sizeof(propval));
		if (!ofnode_valid(node))
			break;
		str = ofnode_read_string(node, propname);
		ut_assert(str && !strcmp(str, propval));
	}

	return 0;
}
DM_TEST(dm_test_ofnode_by_prop_value, UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_fmap(struct unit_test_state *uts)
{
	struct fmap_entry entry;
	ofnode node;

	node = ofnode_path("/cros-ec/flash");
	ut_assert(ofnode_valid(node));
	ut_assertok(ofnode_read_fmap_entry(node, &entry));
	ut_asserteq(0x08000000, entry.offset);
	ut_asserteq(0x20000, entry.length);

	return 0;
}
DM_TEST(dm_test_ofnode_fmap, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_read(struct unit_test_state *uts)
{
	const u32 *val;
	ofnode node;
	int size;

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	val = ofnode_read_prop(node, "int-value", &size);
	ut_assertnonnull(val);
	ut_asserteq(4, size);
	ut_asserteq(1234, fdt32_to_cpu(val[0]));

	val = ofnode_read_prop(node, "missing", &size);
	ut_assertnull(val);
	ut_asserteq(-FDT_ERR_NOTFOUND, size);

	/* Check it works without a size parameter */
	val = ofnode_read_prop(node, "missing", NULL);
	ut_assertnull(val);

	return 0;
}
DM_TEST(dm_test_ofnode_read, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_phandle(struct unit_test_state *uts)
{
	struct ofnode_phandle_args args;
	ofnode node;
	int ret;
	const char prop[] = "test-gpios";
	const char cell[] = "#gpio-cells";
	const char prop2[] = "phandle-value";

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	/* Test ofnode_count_phandle_with_args with cell name */
	ret = ofnode_count_phandle_with_args(node, "missing", cell, 0);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_count_phandle_with_args(node, prop, "#invalid", 0);
	ut_asserteq(-EINVAL, ret);
	ret = ofnode_count_phandle_with_args(node, prop, cell, 0);
	ut_asserteq(5, ret);

	/* Test ofnode_parse_phandle_with_args with cell name */
	ret = ofnode_parse_phandle_with_args(node, "missing", cell, 0, 0,
					     &args);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_parse_phandle_with_args(node, prop, "#invalid", 0, 0,
					     &args);
	ut_asserteq(-EINVAL, ret);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 0, &args);
	ut_assertok(ret);
	ut_asserteq(1, args.args_count);
	ut_asserteq(1, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 1, &args);
	ut_assertok(ret);
	ut_asserteq(1, args.args_count);
	ut_asserteq(4, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 2, &args);
	ut_assertok(ret);
	ut_asserteq(5, args.args_count);
	ut_asserteq(5, args.args[0]);
	ut_asserteq(1, args.args[4]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 3, &args);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 4, &args);
	ut_assertok(ret);
	ut_asserteq(1, args.args_count);
	ut_asserteq(12, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop, cell, 0, 5, &args);
	ut_asserteq(-ENOENT, ret);

	/* Test ofnode_count_phandle_with_args with cell count */
	ret = ofnode_count_phandle_with_args(node, "missing", NULL, 2);
	ut_asserteq(-ENOENT, ret);
	ret = ofnode_count_phandle_with_args(node, prop2, NULL, 1);
	ut_asserteq(3, ret);

	/* Test ofnode_parse_phandle_with_args with cell count */
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 0, &args);
	ut_assertok(ret);
	ut_asserteq(1, ofnode_valid(args.node));
	ut_asserteq(1, args.args_count);
	ut_asserteq(10, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 1, &args);
	ut_asserteq(-EINVAL, ret);
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 2, &args);
	ut_assertok(ret);
	ut_asserteq(1, ofnode_valid(args.node));
	ut_asserteq(1, args.args_count);
	ut_asserteq(30, args.args[0]);
	ret = ofnode_parse_phandle_with_args(node, prop2, NULL, 1, 3, &args);
	ut_asserteq(-ENOENT, ret);

	return 0;
}
DM_TEST(dm_test_ofnode_phandle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_read_chosen(struct unit_test_state *uts)
{
	const char *str;
	const u32 *val;
	ofnode node;
	int size;

	str = ofnode_read_chosen_string("setting");
	ut_assertnonnull(str);
	ut_asserteq_str("sunrise ohoka", str);
	ut_asserteq_ptr(NULL, ofnode_read_chosen_string("no-setting"));

	node = ofnode_get_chosen_node("other-node");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("c-test@5", ofnode_get_name(node));

	node = ofnode_get_chosen_node("setting");
	ut_assert(!ofnode_valid(node));

	val = ofnode_read_chosen_prop("int-values", &size);
	ut_assertnonnull(val);
	ut_asserteq(8, size);
	ut_asserteq(0x1937, fdt32_to_cpu(val[0]));
	ut_asserteq(72993, fdt32_to_cpu(val[1]));

	return 0;
}
DM_TEST(dm_test_ofnode_read_chosen, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_read_aliases(struct unit_test_state *uts)
{
	const void *val;
	ofnode node;
	int size;

	node = ofnode_get_aliases_node("ethernet3");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("sbe5", ofnode_get_name(node));

	node = ofnode_get_aliases_node("unknown");
	ut_assert(!ofnode_valid(node));

	val = ofnode_read_aliases_prop("spi0", &size);
	ut_assertnonnull(val);
	ut_asserteq(7, size);
	ut_asserteq_str("/spi@0", (const char *)val);

	return 0;
}
DM_TEST(dm_test_ofnode_read_aliases, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_get_child_count(struct unit_test_state *uts)
{
	ofnode node, child_node;
	u32 val;

	node = ofnode_path("/i-test");
	ut_assert(ofnode_valid(node));

	val = ofnode_get_child_count(node);
	ut_asserteq(3, val);

	child_node = ofnode_first_subnode(node);
	ut_assert(ofnode_valid(child_node));
	val = ofnode_get_child_count(child_node);
	ut_asserteq(0, val);

	return 0;
}
DM_TEST(dm_test_ofnode_get_child_count,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_is_enabled(struct unit_test_state *uts)
{
	ofnode root_node = ofnode_path("/");
	ofnode node = ofnode_path("/usb@0");

	ut_assert(ofnode_is_enabled(root_node));
	ut_assert(!ofnode_is_enabled(node));

	return 0;
}
DM_TEST(dm_test_ofnode_is_enabled, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_get_reg(struct unit_test_state *uts)
{
	ofnode node;
	fdt_addr_t addr;
	fdt_size_t size;

	node = ofnode_path("/translation-test@8000");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr(node);
	size = ofnode_get_size(node);
	ut_asserteq(0x8000, addr);
	ut_asserteq(0x4000, size);

	node = ofnode_path("/translation-test@8000/dev@1,100");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr(node);
	size = ofnode_get_size(node);
	ut_asserteq(0x9000, addr);
	ut_asserteq(0x1000, size);

	node = ofnode_path("/emul-mux-controller");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr(node);
	size = ofnode_get_size(node);
	ut_asserteq_64(FDT_ADDR_T_NONE, addr);
	ut_asserteq(FDT_SIZE_T_NONE, size);

	node = ofnode_path("/translation-test@8000/noxlatebus@3,300/dev@42");
	ut_assert(ofnode_valid(node));
	addr = ofnode_get_addr_size_index_notrans(node, 0, &size);
	ut_asserteq_64(0x42, addr);

	return 0;
}
DM_TEST(dm_test_ofnode_get_reg, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_get_path(struct unit_test_state *uts)
{
	const char *path = "/translation-test@8000/noxlatebus@3,300/dev@42";
	char buf[64];
	ofnode node;
	int res;

	node = ofnode_path(path);
	ut_assert(ofnode_valid(node));

	res = ofnode_get_path(node, buf, 64);
	ut_asserteq(0, res);
	ut_asserteq_str(path, buf);

	res = ofnode_get_path(node, buf, 32);
	ut_asserteq(-ENOSPC, res);

	return 0;
}
DM_TEST(dm_test_ofnode_get_path, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_conf(struct unit_test_state *uts)
{
	ut_assert(!ofnode_conf_read_bool("missing"));
	ut_assert(ofnode_conf_read_bool("testing-bool"));

	ut_asserteq(123, ofnode_conf_read_int("testing-int", 0));
	ut_asserteq(6, ofnode_conf_read_int("missing", 6));

	ut_assertnull(ofnode_conf_read_str("missing"));
	ut_asserteq_str("testing", ofnode_conf_read_str("testing-str"));

	return 0;
}
DM_TEST(dm_test_ofnode_conf, 0);

static int dm_test_ofnode_for_each_compatible_node(struct unit_test_state *uts)
{
	const char compatible[] = "denx,u-boot-fdt-test";
	bool found = false;
	ofnode node;

	ofnode_for_each_compatible_node(node, compatible) {
		ut_assert(ofnode_device_is_compatible(node, compatible));
		found = true;
	}

	/* There should be at least one matching node */
	ut_assert(found);

	return 0;
}
DM_TEST(dm_test_ofnode_for_each_compatible_node, UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_string(struct unit_test_state *uts)
{
	const char **val;
	const char *out;
	ofnode node;

	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	/* single string */
	ut_asserteq(1, ofnode_read_string_count(node, "str-value"));
	ut_assertok(ofnode_read_string_index(node, "str-value", 0, &out));
	ut_asserteq_str("test string", out);
	ut_asserteq(0, ofnode_stringlist_search(node, "str-value",
						"test string"));
	ut_asserteq(1, ofnode_read_string_list(node, "str-value", &val));
	ut_asserteq_str("test string", val[0]);
	ut_assertnull(val[1]);
	free(val);

	/* list of strings */
	ut_asserteq(5, ofnode_read_string_count(node, "mux-control-names"));
	ut_assertok(ofnode_read_string_index(node, "mux-control-names", 0,
					     &out));
	ut_asserteq_str("mux0", out);
	ut_asserteq(0, ofnode_stringlist_search(node, "mux-control-names",
						"mux0"));
	ut_asserteq(5, ofnode_read_string_list(node, "mux-control-names",
					       &val));
	ut_asserteq_str("mux0", val[0]);
	ut_asserteq_str("mux1", val[1]);
	ut_asserteq_str("mux2", val[2]);
	ut_asserteq_str("mux3", val[3]);
	ut_asserteq_str("mux4", val[4]);
	ut_assertnull(val[5]);
	free(val);

	ut_assertok(ofnode_read_string_index(node, "mux-control-names", 4,
					     &out));
	ut_asserteq_str("mux4", out);
	ut_asserteq(4, ofnode_stringlist_search(node, "mux-control-names",
						"mux4"));

	return 0;
}
DM_TEST(dm_test_ofnode_string, 0);

static int dm_test_ofnode_string_err(struct unit_test_state *uts)
{
	const char **val;
	const char *out;
	ofnode node;

	/*
	 * Test error codes only on livetree, as they are different with
	 * flattree
	 */
	node = ofnode_path("/a-test");
	ut_assert(ofnode_valid(node));

	/* non-existent property */
	ut_asserteq(-EINVAL, ofnode_read_string_count(node, "missing"));
	ut_asserteq(-EINVAL, ofnode_read_string_index(node, "missing", 0,
						      &out));
	ut_asserteq(-EINVAL, ofnode_read_string_list(node, "missing", &val));

	/* empty property */
	ut_asserteq(-ENODATA, ofnode_read_string_count(node, "bool-value"));
	ut_asserteq(-ENODATA, ofnode_read_string_index(node, "bool-value", 0,
						       &out));
	ut_asserteq(-ENODATA, ofnode_read_string_list(node, "bool-value",
						     &val));

	/* badly formatted string list */
	ut_asserteq(-EILSEQ, ofnode_read_string_count(node, "int64-value"));
	ut_asserteq(-EILSEQ, ofnode_read_string_index(node, "int64-value", 0,
						       &out));
	ut_asserteq(-EILSEQ, ofnode_read_string_list(node, "int64-value",
						     &val));

	/* out of range / not found */
	ut_asserteq(-ENODATA, ofnode_read_string_index(node, "str-value", 1,
						       &out));
	ut_asserteq(-ENODATA, ofnode_stringlist_search(node, "str-value",
						       "other"));

	/* negative value for index is not allowed, so don't test for that */

	ut_asserteq(-ENODATA, ofnode_read_string_index(node,
						       "mux-control-names", 5,
						       &out));

	return 0;
}
DM_TEST(dm_test_ofnode_string_err, UT_TESTF_LIVE_TREE);

static int dm_test_ofnode_get_phy(struct unit_test_state *uts)
{
	ofnode eth_node, phy_node;
	phy_interface_t mode;
	u32 reg;

	eth_node = ofnode_path("/phy-test-eth");
	ut_assert(ofnode_valid(eth_node));

	mode = ofnode_read_phy_mode(eth_node);
	ut_assert(mode == PHY_INTERFACE_MODE_2500BASEX);

	phy_node = ofnode_get_phy_node(eth_node);
	ut_assert(ofnode_valid(phy_node));

	reg = ofnode_read_u32_default(phy_node, "reg", -1U);
	ut_asserteq_64(0x1, reg);

	return 0;
}
DM_TEST(dm_test_ofnode_get_phy, 0);

/**
 * make_ofnode_fdt() - Create an FDT for testing with ofnode
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_ofnode_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);

	ut_assert(fdt_begin_node(fdt, "aliases") >= 0);
	ut_assertok(fdt_property_string(fdt, "mmc0", "/new-mmc"));
	ut_assertok(fdt_end_node(fdt));

	ut_assert(fdt_begin_node(fdt, "new-mmc") >= 0);
	ut_assertok(fdt_end_node(fdt));

	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

static int dm_test_ofnode_root(struct unit_test_state *uts)
{
	struct device_node *root = NULL;
	char fdt[256];
	oftree tree;
	ofnode node;

	/* Check that aliases work on the control FDT */
	node = ofnode_get_aliases_node("ethernet3");
	ut_assert(ofnode_valid(node));
	ut_asserteq_str("sbe5", ofnode_get_name(node));

	ut_assertok(make_ofnode_fdt(uts, fdt, sizeof(fdt)));
	if (of_live_active()) {
		ut_assertok(unflatten_device_tree(fdt, &root));
		tree.np = root;
	} else {
		tree.fdt = fdt;
	}

	/* Make sure they don't work on this new tree */
	node = ofnode_path_root(tree, "mmc0");
	ut_assert(!ofnode_valid(node));

	/* It should appear in the new tree */
	node = ofnode_path_root(tree, "/new-mmc");
	ut_assert(ofnode_valid(node));

	/* ...and not in the control FDT */
	node = ofnode_path_root(oftree_default(), "/new-mmc");
	ut_assert(!ofnode_valid(node));

	free(root);

	return 0;
}
DM_TEST(dm_test_ofnode_root, UT_TESTF_SCAN_FDT);

static int dm_test_ofnode_livetree_writing(struct unit_test_state *uts)
{
	struct udevice *dev;
	ofnode node;

	/* Test enabling devices */
	node = ofnode_path("/usb@2");

	ut_assert(!ofnode_is_enabled(node));
	ut_assertok(ofnode_set_enabled(node, true));
	ut_asserteq(true, ofnode_is_enabled(node));

	device_bind_driver_to_node(dm_root(), "usb_sandbox", "usb@2", node,
				   &dev);
	ut_assertok(uclass_find_device_by_seq(UCLASS_USB, 2, &dev));

	/* Test string property setting */
	ut_assert(device_is_compatible(dev, "sandbox,usb"));
	ofnode_write_string(node, "compatible", "gdsys,super-usb");
	ut_assert(device_is_compatible(dev, "gdsys,super-usb"));
	ofnode_write_string(node, "compatible", "sandbox,usb");
	ut_assert(device_is_compatible(dev, "sandbox,usb"));

	/* Test setting generic properties */

	/* Non-existent in DTB */
	ut_asserteq_64(FDT_ADDR_T_NONE, dev_read_addr(dev));
	/* reg = 0x42, size = 0x100 */
	ut_assertok(ofnode_write_prop(node, "reg",
				      "\x00\x00\x00\x42\x00\x00\x01\x00", 8));
	ut_asserteq(0x42, dev_read_addr(dev));

	/* Test disabling devices */
	device_remove(dev, DM_REMOVE_NORMAL);
	device_unbind(dev);

	ut_assert(ofnode_is_enabled(node));
	ut_assertok(ofnode_set_enabled(node, false));
	ut_assert(!ofnode_is_enabled(node));

	return 0;
}
DM_TEST(dm_test_ofnode_livetree_writing,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_LIVE_OR_FLAT);

static int dm_test_ofnode_u32(struct unit_test_state *uts)
{
	ofnode node;

	node = ofnode_path("/lcd");
	ut_assert(ofnode_valid(node));
	ut_asserteq(1366, ofnode_read_u32_default(node, "xres", 123));
	ut_assertok(ofnode_write_u32(node, "xres", 1367));
	ut_asserteq(1367, ofnode_read_u32_default(node, "xres", 123));
	ut_assertok(ofnode_write_u32(node, "xres", 1366));

	return 0;
}
DM_TEST(dm_test_ofnode_u32,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_LIVE_OR_FLAT);
