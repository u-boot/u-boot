/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file of the same name
 */

#ifndef __ACPI_DEVICE_H
#define __ACPI_DEVICE_H

#include <linux/bitops.h>

struct udevice;

/* ACPI descriptor values for common descriptors: SERIAL_BUS means I2C */
#define ACPI_DESCRIPTOR_LARGE		BIT(7)
#define ACPI_DESCRIPTOR_INTERRUPT	(ACPI_DESCRIPTOR_LARGE | 9)
#define ACPI_DESCRIPTOR_GPIO		(ACPI_DESCRIPTOR_LARGE | 12)
#define ACPI_DESCRIPTOR_SERIAL_BUS	(ACPI_DESCRIPTOR_LARGE | 14)

/* Length of a full path to an ACPI device */
#define ACPI_PATH_MAX		30

/* Values that can be returned for ACPI device _STA method */
enum acpi_dev_status {
	ACPI_DSTATUS_PRESENT		= BIT(0),
	ACPI_DSTATUS_ENABLED		= BIT(1),
	ACPI_DSTATUS_SHOW_IN_UI		= BIT(2),
	ACPI_DSTATUS_OK			= BIT(3),
	ACPI_DSTATUS_HAS_BATTERY	= BIT(4),

	ACPI_DSTATUS_ALL_OFF	= 0,
	ACPI_DSTATUS_HIDDEN_ON	= ACPI_DSTATUS_PRESENT | ACPI_DSTATUS_ENABLED |
		ACPI_DSTATUS_OK,
	ACPI_DSTATUS_ALL_ON	= ACPI_DSTATUS_HIDDEN_ON |
		ACPI_DSTATUS_SHOW_IN_UI,
};

/** enum acpi_irq_mode - edge/level trigger mode */
enum acpi_irq_mode {
	ACPI_IRQ_EDGE_TRIGGERED,
	ACPI_IRQ_LEVEL_TRIGGERED,
};

/**
 * enum acpi_irq_polarity - polarity of interrupt
 *
 * @ACPI_IRQ_ACTIVE_LOW - for ACPI_IRQ_EDGE_TRIGGERED this means falling edge
 * @ACPI_IRQ_ACTIVE_HIGH - for ACPI_IRQ_EDGE_TRIGGERED this means rising edge
 * @ACPI_IRQ_ACTIVE_BOTH - not meaningful for ACPI_IRQ_EDGE_TRIGGERED
 */
enum acpi_irq_polarity {
	ACPI_IRQ_ACTIVE_LOW,
	ACPI_IRQ_ACTIVE_HIGH,
	ACPI_IRQ_ACTIVE_BOTH,
};

/**
 * enum acpi_irq_shared - whether interrupt is shared or not
 *
 * @ACPI_IRQ_EXCLUSIVE: only this device uses the interrupt
 * @ACPI_IRQ_SHARED: other devices may use this interrupt
 */
enum acpi_irq_shared {
	ACPI_IRQ_EXCLUSIVE,
	ACPI_IRQ_SHARED,
};

/** enum acpi_irq_wake - indicates whether this interrupt can wake the device */
enum acpi_irq_wake {
	ACPI_IRQ_NO_WAKE,
	ACPI_IRQ_WAKE,
};

/**
 * struct acpi_irq - representation of an ACPI interrupt
 *
 * @pin: ACPI pin that is monitored for the interrupt
 * @mode: Edge/level triggering
 * @polarity: Interrupt polarity
 * @shared: Whether interrupt is shared or not
 * @wake: Whether interrupt can wake the device from sleep
 */
struct acpi_irq {
	unsigned int pin;
	enum acpi_irq_mode mode;
	enum acpi_irq_polarity polarity;
	enum acpi_irq_shared shared;
	enum acpi_irq_wake wake;
};

/**
 * acpi_device_path() - Get the full path to an ACPI device
 *
 * This gets the full path in the form XXXX.YYYY.ZZZZ where XXXX is the root
 * and ZZZZ is the device. All parent devices are added to the path.
 *
 * @dev: Device to check
 * @buf: Buffer to place the path in (should be ACPI_PATH_MAX long)
 * @maxlen: Size of buffer (typically ACPI_PATH_MAX)
 * @return 0 if OK, -ve on error
 */
int acpi_device_path(const struct udevice *dev, char *buf, int maxlen);

/**
 * acpi_device_scope() - Get the scope of an ACPI device
 *
 * This gets the scope which is the full path of the parent device, as per
 * acpi_device_path().
 *
 * @dev: Device to check
 * @buf: Buffer to place the path in (should be ACPI_PATH_MAX long)
 * @maxlen: Size of buffer (typically ACPI_PATH_MAX)
 * @return 0 if OK, -EINVAL if the device has no parent, other -ve on other
 *	error
 */
int acpi_device_scope(const struct udevice *dev, char *scope, int maxlen);

/**
 * acpi_device_status() - Get the status of a device
 *
 * This currently just returns ACPI_DSTATUS_ALL_ON. It does not support
 * inactive or hidden devices.
 *
 * @dev: Device to check
 * @return device status, as ACPI_DSTATUS_...
 */
enum acpi_dev_status acpi_device_status(const struct udevice *dev);

#endif
