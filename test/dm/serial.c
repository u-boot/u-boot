// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018, STMicroelectronics
 */

#include <common.h>
#include <serial.h>
#include <dm.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_serial(struct unit_test_state *uts)
{
	struct udevice *dev_serial;

	ut_assertok(uclass_get_device_by_name(UCLASS_SERIAL, "serial",
					      &dev_serial));

	ut_assertok(serial_tstc());
	/*
	 * test with default config which is the only one supported by
	 * sandbox_serial driver
	 */
	ut_assertok(serial_setconfig(SERIAL_DEFAULT_CONFIG));
	/*
	 * test with a serial config which is not supported by
	 * sandbox_serial driver: test with wrong parity
	 */
	ut_asserteq(-ENOTSUPP,
		    serial_setconfig(SERIAL_CONFIG(SERIAL_PAR_ODD,
						   SERIAL_8_BITS,
						   SERIAL_ONE_STOP)));
	/*
	 * test with a serial config which is not supported by
	 * sandbox_serial driver: test with wrong bits number
	 */
	ut_asserteq(-ENOTSUPP,
		    serial_setconfig(SERIAL_CONFIG(SERIAL_PAR_NONE,
						   SERIAL_6_BITS,
						   SERIAL_ONE_STOP)));

	/*
	 * test with a serial config which is not supported by
	 * sandbox_serial driver: test with wrong stop bits number
	 */
	ut_asserteq(-ENOTSUPP,
		    serial_setconfig(SERIAL_CONFIG(SERIAL_PAR_NONE,
						   SERIAL_8_BITS,
						   SERIAL_TWO_STOP)));

	return 0;
}

DM_TEST(dm_test_serial, DM_TESTF_SCAN_FDT);
