/*
 * Copyright (C) 2015 Google, Inc
2 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Base test of register maps */
static int dm_test_regmap_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct regmap *map;
	int i;

	ut_assertok(uclass_get_device(UCLASS_SYSCON, 0, &dev));
	map = syscon_get_regmap(dev);
	ut_assertok_ptr(map);
	ut_asserteq(1, map->range_count);
	ut_asserteq(0x10, map->base);
	ut_asserteq(0x10, map->range->start);
	ut_asserteq(4, map->range->size);
	ut_asserteq_ptr(&map->base_range, map->range);
	ut_asserteq(0x10, map_to_sysmem(regmap_get_range(map, 0)));

	ut_assertok(uclass_get_device(UCLASS_SYSCON, 1, &dev));
	map = syscon_get_regmap(dev);
	ut_assertok_ptr(map);
	ut_asserteq(4, map->range_count);
	ut_asserteq(0x20, map->base);
	ut_assert(&map->base_range != map->range);
	for (i = 0; i < 4; i++) {
		const unsigned long addr = 0x20 + 8 * i;

		ut_asserteq(addr, map->range[i].start);
		ut_asserteq(5 + i, map->range[i].size);
		ut_asserteq(addr, map_to_sysmem(regmap_get_range(map, i)));
	}

	/* Check that we can't pretend a different device is a syscon */
	ut_assertok(uclass_get_device(UCLASS_I2C, 0, &dev));
	map = syscon_get_regmap(dev);
	ut_asserteq_ptr(ERR_PTR(-ENOEXEC), map);

	return 0;
}
DM_TEST(dm_test_regmap_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test we can access a regmap through syscon */
static int dm_test_regmap_syscon(struct unit_test_state *uts)
{
	struct regmap *map;

	map = syscon_get_regmap_by_driver_data(SYSCON0);
	ut_assertok_ptr(map);
	ut_asserteq(1, map->range_count);

	map = syscon_get_regmap_by_driver_data(SYSCON1);
	ut_assertok_ptr(map);
	ut_asserteq(4, map->range_count);

	map = syscon_get_regmap_by_driver_data(SYSCON_COUNT);
	ut_asserteq_ptr(ERR_PTR(-ENODEV), map);

	ut_asserteq(0x10, map_to_sysmem(syscon_get_first_range(SYSCON0)));
	ut_asserteq(0x20, map_to_sysmem(syscon_get_first_range(SYSCON1)));
	ut_asserteq_ptr(ERR_PTR(-ENODEV),
			syscon_get_first_range(SYSCON_COUNT));

	return 0;
}

DM_TEST(dm_test_regmap_syscon, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
