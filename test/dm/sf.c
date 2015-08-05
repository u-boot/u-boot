/*
 * Copyright (C) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/state.h>
#include <dm/test.h>
#include <dm/util.h>
#include <test/ut.h>

/* Test that sandbox SPI flash works correctly */
static int dm_test_spi_flash(struct unit_test_state *uts)
{
	/*
	 * Create an empty test file and run the SPI flash tests. This is a
	 * long way from being a unit test, but it does test SPI device and
	 * emulator binding, probing, the SPI flash emulator including
	 * device tree decoding, plus the file-based backing store of SPI.
	 *
	 * More targeted tests could be created to perform the above steps
	 * one at a time. This might not increase test coverage much, but
	 * it would make bugs easier to find. It's not clear whether the
	 * benefit is worth the extra complexity.
	 */
	ut_asserteq(0, run_command_list(
		"sb save hostfs - 0 spi.bin 200000;"
		"sf probe;"
		"sf test 0 10000", -1,  0));
	/*
	 * Since we are about to destroy all devices, we must tell sandbox
	 * to forget the emulation device
	 */
	sandbox_sf_unbind_emul(state_get_current(), 0, 0);

	return 0;
}
DM_TEST(dm_test_spi_flash, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
