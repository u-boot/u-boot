// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/of_extra.h>
#include <dm/test.h>
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
	ut_asserteq(FDT_ADDR_T_NONE, addr);
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
