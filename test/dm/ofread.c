// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_ofnode_get_property_by_prop(struct unit_test_state *uts)
{
	ofnode node;
	struct ofprop prop;
	const void *value;
	const char *propname;
	int res, len, count = 0;

	node = ofnode_path("/cros-ec/flash");
	for (res = ofnode_get_first_property(node, &prop);
	     !res;
	     res = ofnode_get_next_property(&prop)) {
		value = ofnode_get_property_by_prop(&prop, &propname, &len);
		ut_assertnonnull(value);
		switch (count) {
		case 0:
			ut_asserteq_str("image-pos", propname);
			ut_asserteq(4, len);
			break;
		case 1:
			ut_asserteq_str("size", propname);
			ut_asserteq(4, len);
			break;
		case 2:
			ut_asserteq_str("erase-value", propname);
			ut_asserteq(4, len);
			break;
		case 3:
			/* only for platdata */
			ut_asserteq_str("name", propname);
			ut_asserteq(6, len);
			ut_asserteq_str("flash", value);
			break;
		default:
			break;
		}
		count++;
	}

	return 0;
}
DM_TEST(dm_test_ofnode_get_property_by_prop,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
