/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common header file for bootdev, bootflow, bootmeth tests
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootstd_common_h
#define __bootstd_common_h

#include <version_string.h>

/* Declare a new bootdev test */
#define BOOTSTD_TEST(_name, _flags)	UNIT_TEST(_name, _flags, bootstd)
#define BOOTSTD_TEST_INIT(_name, _flags) UNIT_TEST_INIT(_name, _flags, bootstd)

#define NVDATA_START_BLK	((0x400 + 0x400) / MMC_MAX_BLOCK_LEN)
#define VERSION_START_BLK	((0x400 + 0x800) / MMC_MAX_BLOCK_LEN)
#define TEST_VERSION		"U-Boot v2022.04-local2"
#define TEST_VERNUM		0x00010002

enum {
	MAX_HUNTER	= 9,
	MMC_HUNTER	= 3,	/* ID of MMC hunter */
};

struct unit_test_state;

/**
 * bootstd_test_drop_bootdev_order() - Remove the existing boot order
 *
 * Drop the boot order so that all bootdevs are used in their alias order
 *
 * @uts: Unit test state to use for ut_assert...() functions
 */
int bootstd_test_drop_bootdev_order(struct unit_test_state *uts);

/**
 * bootstd_test_check_mmc_hunter() - Check that the mmc bootdev hunter was used
 *
 * @uts: Unit test state to use for ut_assert...() functions
 * Returns: 0 if OK (used), other value on error (not used)
 */
int bootstd_test_check_mmc_hunter(struct unit_test_state *uts);

/**
 * bootstd_reset_usb() - Reset the USB subsystem
 *
 * Resets USB so that it can be started (and scanning) again. This is useful in
 * tests which need to use USB.
 */
void bootstd_reset_usb(void);

#endif
