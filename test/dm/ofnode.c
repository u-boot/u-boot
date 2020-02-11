// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <dm/of_extra.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_ofnode_compatible(struct unit_test_state *uts)
{
	ofnode root_node = ofnode_path("/");

	ut_assert(ofnode_valid(root_node));
	ut_assert(ofnode_device_is_compatible(root_node, "sandbox"));

	return 0;
}
DM_TEST(dm_test_ofnode_compatible, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_ofnode_by_prop_value, DM_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_ofnode_fmap, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_ofnode_read, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_ofnode_read_chosen, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
