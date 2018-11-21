/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __pch_h
#define __pch_h

#define PCH_RCBA		0xf0

#define BIOS_CTRL_BIOSWE	BIT(0)

/* Operations for the Platform Controller Hub */
struct pch_ops {
	/**
	 * get_spi_base() - get the address of SPI base
	 *
	 * @dev:	PCH device to check
	 * @sbasep:	Returns address of SPI base if available, else 0
	 * @return 0 if OK, -ve on error (e.g. there is no SPI base)
	 */
	int (*get_spi_base)(struct udevice *dev, ulong *sbasep);

	/**
	 * set_spi_protect() - set whether SPI flash is protected or not
	 *
	 * @dev:	PCH device to adjust
	 * @protect:	true to protect, false to unprotect
	 *
	 * @return 0 on success, -ENOSYS if not implemented
	 */
	int (*set_spi_protect)(struct udevice *dev, bool protect);

	/**
	 * get_gpio_base() - get the address of GPIO base
	 *
	 * @dev:	PCH device to check
	 * @gbasep:	Returns address of GPIO base if available, else 0
	 * @return 0 if OK, -ve on error (e.g. there is no GPIO base)
	 */
	int (*get_gpio_base)(struct udevice *dev, u32 *gbasep);

	/**
	 * get_io_base() - get the address of IO base
	 *
	 * @dev:	PCH device to check
	 * @iobasep:	Returns address of IO base if available, else 0
	 * @return 0 if OK, -ve on error (e.g. there is no IO base)
	 */
	int (*get_io_base)(struct udevice *dev, u32 *iobasep);
};

#define pch_get_ops(dev)        ((struct pch_ops *)(dev)->driver->ops)

/**
 * pch_get_spi_base() - get the address of SPI base
 *
 * @dev:	PCH device to check
 * @sbasep:	Returns address of SPI base if available, else 0
 * @return 0 if OK, -ve on error (e.g. there is no SPI base)
 */
int pch_get_spi_base(struct udevice *dev, ulong *sbasep);

/**
 * set_spi_protect() - set whether SPI flash is protected or not
 *
 * @dev:	PCH device to adjust
 * @protect:	true to protect, false to unprotect
 *
 * @return 0 on success, -ENOSYS if not implemented
 */
int pch_set_spi_protect(struct udevice *dev, bool protect);

/**
 * pch_get_gpio_base() - get the address of GPIO base
 *
 * @dev:	PCH device to check
 * @gbasep:	Returns address of GPIO base if available, else 0
 * @return 0 if OK, -ve on error (e.g. there is no GPIO base)
 */
int pch_get_gpio_base(struct udevice *dev, u32 *gbasep);

/**
 * pch_get_io_base() - get the address of IO base
 *
 * @dev:	PCH device to check
 * @iobasep:	Returns address of IO base if available, else 0
 * @return 0 if OK, -ve on error (e.g. there is no IO base)
 */
int pch_get_io_base(struct udevice *dev, u32 *iobasep);

#endif
