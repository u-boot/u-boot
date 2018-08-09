/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef _AXI_H_
#define _AXI_H_

enum axi_size_t {
	AXI_SIZE_8,
	AXI_SIZE_16,
	AXI_SIZE_32,
};

/**
 * struct axi_ops - driver operations for AXI uclass
 *
 * Drivers should support these operations unless otherwise noted. These
 * operations are intended to be used by uclass code, not directly from
 * other code.
 */
struct axi_ops {
	/**
	 * read() - Read a single value from a specified address on a AXI bus
	 *
	 * @dev:	AXI bus to read from.
	 * @address:	The address to read from.
	 * @data:	Pointer to a variable that takes the data value read
	 *		from the address on the AXI bus.
	 * @size:	The size of the data to be read.
	 * @return 0 if OK, -ve on error.
	 */
	int (*read)(struct udevice *dev, ulong address, void *data,
		    enum axi_size_t size);

	/**
	 * write() - Write a single value to a specified address on a AXI bus
	 *
	 * @dev:	AXI bus to write to.
	 * @address:	The address to write to.
	 * @data:	Pointer to the data value to be written to the address
	 *		on the AXI bus.
	 * @size:	The size of the data to write.
	 * @return 0 if OK, -ve on error.
	 */
	int (*write)(struct udevice *dev, ulong address, void *data,
		     enum axi_size_t size);
};

#define axi_get_ops(dev)	((struct axi_ops *)(dev)->driver->ops)

/**
 * axi_read() - Read a single value from a specified address on a AXI bus
 *
 * @dev:	AXI bus to read from.
 * @address:	The address to read from.
 * @data:	Pointer to a variable that takes the data value read from the
 *              address on the AXI bus.
 * @size:	The size of the data to write.
 * @return 0 if OK, -ve on error.
 */
int axi_read(struct udevice *dev, ulong address, void *data,
	     enum axi_size_t size);

/**
 * axi_write() - Write a single value to a specified address on a AXI bus
 *
 * @dev:	AXI bus to write to.
 * @address:	The address to write to.
 * @data:	Pointer to the data value to be written to the address on the
 *		AXI bus.
 * @size:	The size of the data to write.
 * @return 0 if OK, -ve on error.
 */
int axi_write(struct udevice *dev, ulong address, void *data,
	      enum axi_size_t size);
#endif
