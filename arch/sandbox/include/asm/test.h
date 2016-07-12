/*
 * Test-related constants for sandbox
 *
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_TEST_H
#define __ASM_TEST_H

/* The sandbox driver always permits an I2C device with this address */
#define SANDBOX_I2C_TEST_ADDR		0x59

#define SANDBOX_PCI_VENDOR_ID		0x1234
#define SANDBOX_PCI_DEVICE_ID		0x5678
#define SANDBOX_PCI_CLASS_CODE		PCI_CLASS_CODE_COMM
#define SANDBOX_PCI_CLASS_SUB_CODE	PCI_CLASS_SUB_CODE_COMM_SERIAL

#define SANDBOX_CLK_RATE		32768

/* System controller driver data */
enum {
	SYSCON0		= 32,
	SYSCON1,

	SYSCON_COUNT
};

/**
 * sandbox_i2c_set_test_mode() - set test mode for running unit tests
 *
 * See sandbox_i2c_xfer() for the behaviour changes.
 *
 * @bus:	sandbox I2C bus to adjust
 * @test_mode:	true to select test mode, false to run normally
 */
void sandbox_i2c_set_test_mode(struct udevice *bus, bool test_mode);

enum sandbox_i2c_eeprom_test_mode {
	SIE_TEST_MODE_NONE,
	/* Permits read/write of only one byte per I2C transaction */
	SIE_TEST_MODE_SINGLE_BYTE,
};

void sandbox_i2c_eeprom_set_test_mode(struct udevice *dev,
				      enum sandbox_i2c_eeprom_test_mode mode);

void sandbox_i2c_eeprom_set_offset_len(struct udevice *dev, int offset_len);

/*
 * sandbox_timer_add_offset()
 *
 * Allow tests to add to the time reported through lib/time.c functions
 * offset: number of milliseconds to advance the system time
 */
void sandbox_timer_add_offset(unsigned long offset);

/**
 * sandbox_i2c_rtc_set_offset() - set the time offset from system/base time
 *
 * @dev:		RTC device to adjust
 * @use_system_time:	true to use system time, false to use @base_time
 * @offset:		RTC offset from current system/base time (-1 for no
 *			change)
 * @return old value of RTC offset
 */
long sandbox_i2c_rtc_set_offset(struct udevice *dev, bool use_system_time,
				int offset);

/**
 * sandbox_i2c_rtc_get_set_base_time() - get and set the base time
 *
 * @dev:		RTC device to adjust
 * @base_time:		New base system time (set to -1 for no change)
 * @return old base time
 */
long sandbox_i2c_rtc_get_set_base_time(struct udevice *dev, long base_time);

int sandbox_usb_keyb_add_string(struct udevice *dev, const char *str);

#endif
