// SPDX-License-Identifier: GPL-2.0-only
/*
 * Tests for the SPI EEPROM uclass
 *
 * Copyright (c) 2024 Koninklijke Philips N.V.
 * Copyright (c) 2026 João Loureiro <joaofl@gmail.com>
 */

#include <dm.h>
#include <spi_eeprom.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Read from the emulated SPI EEPROM and check the uclass operations */
static int dm_test_spi_eeprom(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 buf[16];
	int i;

	ut_assertok(uclass_get_device_by_name(UCLASS_SPI_EEPROM, "eeprom@3",
					      &dev));

	/* The emulator advertises a 2 KiB array */
	ut_asserteq(2048, spi_eeprom_size(dev));

	/* The backing store is filled with a (addr & 0xff) pattern */
	ut_assertok(spi_eeprom_read(dev, 0, buf, sizeof(buf)));
	for (i = 0; i < sizeof(buf); i++)
		ut_asserteq(i & 0xff, buf[i]);

	/* Reads honour the requested offset */
	ut_assertok(spi_eeprom_read(dev, 0x100, buf, sizeof(buf)));
	for (i = 0; i < sizeof(buf); i++)
		ut_asserteq((0x100 + i) & 0xff, buf[i]);

	/* Reads past the end of the array are rejected */
	ut_asserteq(-EINVAL, spi_eeprom_read(dev, 2040, buf, sizeof(buf)));

	/* Writing is not implemented yet */
	ut_asserteq(-ENOSYS, spi_eeprom_write(dev, 0, buf, 1));

	return 0;
}
DM_TEST(dm_test_spi_eeprom, UTF_SCAN_PDATA | UTF_SCAN_FDT);
