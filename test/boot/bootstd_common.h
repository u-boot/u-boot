/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common header file for bootdev, bootflow, bootmeth tests
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootstd_common_h
#define __bootstd_common_h

/* Declare a new bootdev test */
#define BOOTSTD_TEST(_name, _flags) \
		UNIT_TEST(_name, _flags, bootstd_test)

struct unit_test_state;

/**
 * bootstd_test_drop_bootdev_order() - Remove the existing boot order
 *
 * Drop the boot order so that all bootdevs are used in their alias order
 *
 * @uts: Unit test state to use for ut_assert...() functions
 */
int bootstd_test_drop_bootdev_order(struct unit_test_state *uts);

#endif
