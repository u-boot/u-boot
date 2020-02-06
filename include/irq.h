/* SPDX-License-Identifier: GPL-2.0 */
/*
 * IRQ is a type of interrupt controller used on recent Intel SoC.
 *
 * Copyright 2019 Google LLC
 */

#ifndef __irq_H
#define __irq_H

/*
 * Interrupt controller types available. You can find a particular one with
 * irq_first_device_type()
 */
enum irq_dev_t {
	X86_IRQT_BASE,		/* Base controller */
	X86_IRQT_ITSS,		/* ITSS controller, e.g. on APL */
	X86_IRQT_ACPI_GPE,	/* ACPI General-Purpose Events controller */
	SANDBOX_IRQT_BASE,	/* Sandbox testing */
};

/**
 * struct irq_ops - Operations for the IRQ
 */
struct irq_ops {
	/**
	 * route_pmc_gpio_gpe() - Get the GPIO for an event
	 *
	 * @dev: IRQ device
	 * @pmc_gpe_num: Event number to check
	 * @returns GPIO for the event, or -ENOENT if none
	 */
	int (*route_pmc_gpio_gpe)(struct udevice *dev, uint pmc_gpe_num);

	/**
	 * set_polarity() - Set the IRQ polarity
	 *
	 * @dev: IRQ device
	 * @irq: Interrupt number to set
	 * @active_low: true if active low, false for active high
	 * @return 0 if OK, -EINVAL if @irq is invalid
	 */
	int (*set_polarity)(struct udevice *dev, uint irq, bool active_low);

	/**
	 * snapshot_polarities() - record IRQ polarities for later restore
	 *
	 * @dev: IRQ device
	 * @return 0
	 */
	int (*snapshot_polarities)(struct udevice *dev);

	/**
	 * restore_polarities() - restore IRQ polarities
	 *
	 * @dev: IRQ device
	 * @return 0
	 */
	int (*restore_polarities)(struct udevice *dev);
};

#define irq_get_ops(dev)	((struct irq_ops *)(dev)->driver->ops)

/**
 * irq_route_pmc_gpio_gpe() - Get the GPIO for an event
 *
 * @dev: IRQ device
 * @pmc_gpe_num: Event number to check
 * @returns GPIO for the event, or -ENOENT if none
 */
int irq_route_pmc_gpio_gpe(struct udevice *dev, uint pmc_gpe_num);

/**
 * irq_set_polarity() - Set the IRQ polarity
 *
 * @dev: IRQ device
 * @irq: Interrupt number to set
 * @active_low: true if active low, false for active high
 * @return 0 if OK, -EINVAL if @irq is invalid
 */
int irq_set_polarity(struct udevice *dev, uint irq, bool active_low);

/**
 * irq_snapshot_polarities() - record IRQ polarities for later restore
 *
 * @dev: IRQ device
 * @return 0
 */
int irq_snapshot_polarities(struct udevice *dev);

/**
 * irq_restore_polarities() - restore IRQ polarities
 *
 * @dev: IRQ device
 * @return 0
 */
int irq_restore_polarities(struct udevice *dev);

/**
 * irq_first_device_type() - Get a particular interrupt controller
 *
 * On success this returns an activated interrupt device.
 *
 * @type: Type to find
 * @devp: Returns the device, if found
 * @return 0 if OK, -ENODEV if not found, other -ve error if uclass failed to
 *	probe
 */
int irq_first_device_type(enum irq_dev_t type, struct udevice **devp);

#endif
