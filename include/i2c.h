/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2009 - 2013 Heiko Schocher <hs@denx.de>
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 *
 * The original I2C interface was
 *   (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
 *   AIRVENT SAM s.p.a - RIMINI(ITALY)
 * but has been changed substantially.
 */

#ifndef _I2C_H_
#define _I2C_H_

#include <linker_lists.h>

/*
 * For now there are essentially two parts to this file - driver model
 * here at the top, and the older code below (with CONFIG_SYS_I2C_LEGACY being
 * most recent). The plan is to migrate everything to driver model.
 * The driver model structures and API are separate as they are different
 * enough as to be incompatible for compilation purposes.
 */

enum dm_i2c_chip_flags {
	DM_I2C_CHIP_10BIT	= 1 << 0, /* Use 10-bit addressing */
	DM_I2C_CHIP_RD_ADDRESS	= 1 << 1, /* Send address for each read byte */
	DM_I2C_CHIP_WR_ADDRESS	= 1 << 2, /* Send address for each write byte */
};

/** enum i2c_speed_mode - standard I2C speed modes */
enum i2c_speed_mode {
	IC_SPEED_MODE_STANDARD,
	IC_SPEED_MODE_FAST,
	IC_SPEED_MODE_FAST_PLUS,
	IC_SPEED_MODE_HIGH,
	IC_SPEED_MODE_FAST_ULTRA,

	IC_SPEED_MODE_COUNT,
};

/** enum i2c_speed_rate - standard I2C speeds in Hz */
enum i2c_speed_rate {
	I2C_SPEED_STANDARD_RATE		= 100000,
	I2C_SPEED_FAST_RATE		= 400000,
	I2C_SPEED_FAST_PLUS_RATE	= 1000000,
	I2C_SPEED_HIGH_RATE		= 3400000,
	I2C_SPEED_FAST_ULTRA_RATE	= 5000000,
};

/** enum i2c_address_mode - available address modes */
enum i2c_address_mode {
	I2C_MODE_7_BIT,
	I2C_MODE_10_BIT
};

/** enum i2c_device_t - Types of I2C devices, used for compatible strings */
enum i2c_device_t {
	I2C_DEVICE_GENERIC,
	I2C_DEVICE_HID_OVER_I2C,
};

struct udevice;
/**
 * struct dm_i2c_chip - information about an i2c chip
 *
 * An I2C chip is a device on the I2C bus. It sits at a particular address
 * and normally supports 7-bit or 10-bit addressing.
 *
 * To obtain this structure, use dev_get_parent_plat(dev) where dev is
 * the chip to examine.
 *
 * @chip_addr:	Chip address on bus
 * @offset_len: Length of offset in bytes. A single byte offset can
 *		represent up to 256 bytes. A value larger than 1 may be
 *		needed for larger devices.
 * @flags:	Flags for this chip (dm_i2c_chip_flags)
 * @chip_addr_offset_mask: Mask of offset bits within chip_addr. Used for
 *			   devices which steal addresses as part of offset.
 *			   If offset_len is zero, then the offset is encoded
 *			   completely within the chip address itself.
 *			   e.g. a devce with chip address of 0x2c with 512
 *			   registers might use the bottom bit of the address
 *			   to indicate which half of the address space is being
 *			   accessed while still only using 1 byte offset.
 *			   This means it will respond to  chip address 0x2c and
 *			   0x2d.
 *			   A real world example is the Atmel AT24C04. It's
 *			   datasheet explains it's usage of this addressing
 *			   mode.
 * @emul: Emulator for this chip address (only used for emulation)
 * @emul_idx: Emulator index, used for of-platdata and set by each i2c chip's
 *	bind() method. This allows i2c_emul_find() to work with of-platdata.
 */
struct dm_i2c_chip {
	uint chip_addr;
	uint offset_len;
	uint flags;
	uint chip_addr_offset_mask;
#ifdef CONFIG_SANDBOX
	struct udevice *emul;
	bool test_mode;
	int emul_idx;
#endif
};

/**
 * struct dm_i2c_bus- information about an i2c bus
 *
 * An I2C bus contains 0 or more chips on it, each at its own address. The
 * bus can operate at different speeds (measured in Hz, typically 100KHz
 * or 400KHz).
 *
 * To obtain this structure, use dev_get_uclass_priv(bus) where bus is the
 * I2C bus udevice.
 *
 * @speed_hz: Bus speed in hertz (typically 100000)
 * @max_transaction_bytes: Maximal size of single I2C transfer
 */
struct dm_i2c_bus {
	int speed_hz;
	int max_transaction_bytes;
};

/*
 * Not all of these flags are implemented in the U-Boot API
 */
enum dm_i2c_msg_flags {
	I2C_M_TEN		= 0x0010, /* ten-bit chip address */
	I2C_M_RD		= 0x0001, /* read data, from slave to master */
	I2C_M_STOP		= 0x8000, /* send stop after this message */
	I2C_M_NOSTART		= 0x4000, /* no start before this message */
	I2C_M_REV_DIR_ADDR	= 0x2000, /* invert polarity of R/W bit */
	I2C_M_IGNORE_NAK	= 0x1000, /* continue after NAK */
	I2C_M_NO_RD_ACK		= 0x0800, /* skip the Ack bit on reads */
	I2C_M_RECV_LEN		= 0x0400, /* length is first received byte */
};

/**
 * struct i2c_msg - an I2C message
 *
 * @addr:	Slave address
 * @flags:	Flags (see enum dm_i2c_msg_flags)
 * @len:	Length of buffer in bytes, may be 0 for a probe
 * @buf:	Buffer to send/receive, or NULL if no data
 */
struct i2c_msg {
	uint addr;
	uint flags;
	uint len;
	u8 *buf;
};

/**
 * struct i2c_msg_list - a list of I2C messages
 *
 * This is called i2c_rdwr_ioctl_data in Linux but the name does not seem
 * appropriate in U-Boot.
 *
 * @msg:	Pointer to i2c_msg array
 * @nmsgs:	Number of elements in the array
 */
struct i2c_msg_list {
	struct i2c_msg *msgs;
	uint nmsgs;
};

/**
 * dm_i2c_read() - read bytes from an I2C chip
 *
 * To obtain an I2C device (called a 'chip') given the I2C bus address you
 * can use i2c_get_chip(). To obtain a bus by bus number use
 * uclass_get_device_by_seq(UCLASS_I2C, <bus number>).
 *
 * To set the address length of a devce use i2c_set_addr_len(). It
 * defaults to 1.
 *
 * @dev:	Chip to read from
 * @offset:	Offset within chip to start reading
 * @buffer:	Place to put data
 * @len:	Number of bytes to read
 *
 * Return: 0 on success, -ve on failure
 */
int dm_i2c_read(struct udevice *dev, uint offset, uint8_t *buffer, int len);

/**
 * dm_i2c_write() - write bytes to an I2C chip
 *
 * See notes for dm_i2c_read() above.
 *
 * @dev:	Chip to write to
 * @offset:	Offset within chip to start writing
 * @buffer:	Buffer containing data to write
 * @len:	Number of bytes to write
 *
 * Return: 0 on success, -ve on failure
 */
int dm_i2c_write(struct udevice *dev, uint offset, const uint8_t *buffer,
		 int len);

/**
 * dm_i2c_probe() - probe a particular chip address
 *
 * This can be useful to check for the existence of a chip on the bus.
 * It is typically implemented by writing the chip address to the bus
 * and checking that the chip replies with an ACK.
 *
 * @bus:	Bus to probe
 * @chip_addr:	7-bit address to probe (10-bit and others are not supported)
 * @chip_flags:	Flags for the probe (see enum dm_i2c_chip_flags)
 * @devp:	Returns the device found, or NULL if none
 * Return: 0 if a chip was found at that address, -ve if not
 */
int dm_i2c_probe(struct udevice *bus, uint chip_addr, uint chip_flags,
		 struct udevice **devp);

/**
 * dm_i2c_reg_read() - Read a value from an I2C register
 *
 * This reads a single value from the given address in an I2C chip
 *
 * @dev:	Device to use for transfer
 * @addr:	Address to read from
 * Return: value read, or -ve on error
 */
int dm_i2c_reg_read(struct udevice *dev, uint offset);

/**
 * dm_i2c_reg_write() - Write a value to an I2C register
 *
 * This writes a single value to the given address in an I2C chip
 *
 * @dev:	Device to use for transfer
 * @addr:	Address to write to
 * @val:	Value to write (normally a byte)
 * Return: 0 on success, -ve on error
 */
int dm_i2c_reg_write(struct udevice *dev, uint offset, unsigned int val);

/**
 * dm_i2c_reg_clrset() - Apply bitmask to an I2C register
 *
 * Read value, apply bitmask and write modified value back to the
 * given address in an I2C chip
 *
 * @dev:	Device to use for transfer
 * @offset:	Address for the R/W operation
 * @clr:	Bitmask of bits that should be cleared
 * @set:	Bitmask of bits that should be set
 * Return: 0 on success, -ve on error
 */
int dm_i2c_reg_clrset(struct udevice *dev, uint offset, u32 clr, u32 set);

/**
 * dm_i2c_xfer() - Transfer messages over I2C
 *
 * This transfers a raw message. It is best to use dm_i2c_reg_read/write()
 * instead.
 *
 * @dev:	Device to use for transfer
 * @msg:	List of messages to transfer
 * @nmsgs:	Number of messages to transfer
 * Return: 0 on success, -ve on error
 */
int dm_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs);

/**
 * dm_i2c_set_bus_speed() - set the speed of a bus
 *
 * @bus:	Bus to adjust
 * @speed:	Requested speed in Hz
 * Return: 0 if OK, -EINVAL for invalid values
 */
int dm_i2c_set_bus_speed(struct udevice *bus, unsigned int speed);

/**
 * dm_i2c_get_bus_speed() - get the speed of a bus
 *
 * @bus:	Bus to check
 * Return: speed of selected I2C bus in Hz, -ve on error
 */
int dm_i2c_get_bus_speed(struct udevice *bus);

/**
 * i2c_set_chip_flags() - set flags for a chip
 *
 * Typically addresses are 7 bits, but for 10-bit addresses you should set
 * flags to DM_I2C_CHIP_10BIT. All accesses will then use 10-bit addressing.
 *
 * @dev:	Chip to adjust
 * @flags:	New flags
 * Return: 0 if OK, -EINVAL if value is unsupported, other -ve value on error
 */
int i2c_set_chip_flags(struct udevice *dev, uint flags);

/**
 * i2c_get_chip_flags() - get flags for a chip
 *
 * @dev:	Chip to check
 * @flagsp:	Place to put flags
 * Return: 0 if OK, other -ve value on error
 */
int i2c_get_chip_flags(struct udevice *dev, uint *flagsp);

/**
 * i2c_set_offset_len() - set the offset length for a chip
 *
 * The offset used to access a chip may be up to 4 bytes long. Typically it
 * is only 1 byte, which is enough for chips with 256 bytes of memory or
 * registers. The default value is 1, but you can call this function to
 * change it.
 *
 * @offset_len:	New offset length value (typically 1 or 2)
 */
int i2c_set_chip_offset_len(struct udevice *dev, uint offset_len);

/**
 * i2c_get_offset_len() - get the offset length for a chip
 *
 * @return:	Current offset length value (typically 1 or 2)
 */
int i2c_get_chip_offset_len(struct udevice *dev);

/**
 * i2c_set_chip_addr_offset_mask() - set mask of address bits usable by offset
 *
 * Some devices listen on multiple chip addresses to achieve larger offsets
 * than their single or multiple byte offsets would allow for. You can use this
 * function to set the bits that are valid to be used for offset overflow.
 *
 * @mask: The mask to be used for high offset bits within address
 * Return: 0 if OK, other -ve value on error
 */
int i2c_set_chip_addr_offset_mask(struct udevice *dev, uint mask);

/*
 * i2c_get_chip_addr_offset_mask() - get mask of address bits usable by offset
 *
 * Return: current chip addr offset mask
 */
uint i2c_get_chip_addr_offset_mask(struct udevice *dev);

/**
 * i2c_deblock() - recover a bus that is in an unknown state
 *
 * See the deblock() method in 'struct dm_i2c_ops' for full information
 *
 * @bus:	Bus to recover
 * Return: 0 if OK, -ve on error
 */
int i2c_deblock(struct udevice *bus);

/**
 * i2c_deblock_gpio_loop() - recover a bus from an unknown state by toggling SDA/SCL
 *
 * This is the inner logic used for toggling I2C SDA/SCL lines as GPIOs
 * for deblocking the I2C bus.
 *
 * @sda_pin:	SDA GPIO
 * @scl_pin:	SCL GPIO
 * @scl_count:	Number of SCL clock cycles generated to deblock SDA
 * @start_count:Number of I2C start conditions sent after deblocking SDA
 * @delay:	Delay between SCL clock line changes
 * Return: 0 if OK, -ve on error
 */
struct gpio_desc;
int i2c_deblock_gpio_loop(struct gpio_desc *sda_pin, struct gpio_desc *scl_pin,
			  unsigned int scl_count, unsigned int start_count,
			  unsigned int delay);

/**
 * struct dm_i2c_ops - driver operations for I2C uclass
 *
 * Drivers should support these operations unless otherwise noted. These
 * operations are intended to be used by uclass code, not directly from
 * other code.
 */
struct dm_i2c_ops {
	/**
	 * xfer() - transfer a list of I2C messages
	 *
	 * @bus:	Bus to read from
	 * @msg:	List of messages to transfer
	 * @nmsgs:	Number of messages in the list
	 * @return 0 if OK, -EREMOTEIO if the slave did not ACK a byte,
	 *	-ECOMM if the speed cannot be supported, -EPROTO if the chip
	 *	flags cannot be supported, other -ve value on some other error
	 */
	int (*xfer)(struct udevice *bus, struct i2c_msg *msg, int nmsgs);

	/**
	 * probe_chip() - probe for the presense of a chip address
	 *
	 * This function is optional. If omitted, the uclass will send a zero
	 * length message instead.
	 *
	 * @bus:	Bus to probe
	 * @chip_addr:	Chip address to probe
	 * @chip_flags:	Probe flags (enum dm_i2c_chip_flags)
	 * @return 0 if chip was found, -EREMOTEIO if not, -ENOSYS to fall back
	 * to default probem other -ve value on error
	 */
	int (*probe_chip)(struct udevice *bus, uint chip_addr, uint chip_flags);

	/**
	 * set_bus_speed() - set the speed of a bus (optional)
	 *
	 * The bus speed value will be updated by the uclass if this function
	 * does not return an error. This method is optional - if it is not
	 * provided then the driver can read the speed from
	 * dev_get_uclass_priv(bus)->speed_hz
	 *
	 * @bus:	Bus to adjust
	 * @speed:	Requested speed in Hz
	 * @return 0 if OK, -EINVAL for invalid values
	 */
	int (*set_bus_speed)(struct udevice *bus, unsigned int speed);

	/**
	 * get_bus_speed() - get the speed of a bus (optional)
	 *
	 * Normally this can be provided by the uclass, but if you want your
	 * driver to check the bus speed by looking at the hardware, you can
	 * implement that here. This method is optional. This method would
	 * normally be expected to return dev_get_uclass_priv(bus)->speed_hz.
	 *
	 * @bus:	Bus to check
	 * @return speed of selected I2C bus in Hz, -ve on error
	 */
	int (*get_bus_speed)(struct udevice *bus);

	/**
	 * set_flags() - set the flags for a chip (optional)
	 *
	 * This is generally implemented by the uclass, but drivers can
	 * check the value to ensure that unsupported options are not used.
	 * This method is optional. If provided, this method will always be
	 * called when the flags change.
	 *
	 * @dev:	Chip to adjust
	 * @flags:	New flags value
	 * @return 0 if OK, -EINVAL if value is unsupported
	 */
	int (*set_flags)(struct udevice *dev, uint flags);

	/**
	 * deblock() - recover a bus that is in an unknown state
	 *
	 * I2C is a synchronous protocol and resets of the processor in the
	 * middle of an access can block the I2C Bus until a powerdown of
	 * the full unit is done. This is because slaves can be stuck
	 * waiting for addition bus transitions for a transaction that will
	 * never complete. Resetting the I2C master does not help. The only
	 * way is to force the bus through a series of transitions to make
	 * sure that all slaves are done with the transaction. This method
	 * performs this 'deblocking' if support by the driver.
	 *
	 * This method is optional.
	 */
	int (*deblock)(struct udevice *bus);
};

#define i2c_get_ops(dev)	((struct dm_i2c_ops *)(dev)->driver->ops)

/**
 * struct i2c_mux_ops - operations for an I2C mux
 *
 * The current mux state is expected to be stored in the mux itself since
 * it is the only thing that knows how to make things work. The mux can
 * record the current state and then avoid switching unless it is necessary.
 * So select() can be skipped if the mux is already in the correct state.
 * Also deselect() can be made a nop if required.
 */
struct i2c_mux_ops {
	/**
	 * select() - select one of of I2C buses attached to a mux
	 *
	 * This will be called when there is no bus currently selected by the
	 * mux. This method does not need to deselect the old bus since
	 * deselect() will be already have been called if necessary.
	 *
	 * @mux:	Mux device
	 * @bus:	I2C bus to select
	 * @channel:	Channel number correponding to the bus to select
	 * @return 0 if OK, -ve on error
	 */
	int (*select)(struct udevice *mux, struct udevice *bus, uint channel);

	/**
	 * deselect() - select one of of I2C buses attached to a mux
	 *
	 * This is used to deselect the currently selected I2C bus.
	 *
	 * @mux:	Mux device
	 * @bus:	I2C bus to deselect
	 * @channel:	Channel number correponding to the bus to deselect
	 * @return 0 if OK, -ve on error
	 */
	int (*deselect)(struct udevice *mux, struct udevice *bus, uint channel);
};

#define i2c_mux_get_ops(dev)	((struct i2c_mux_ops *)(dev)->driver->ops)

/**
 * i2c_get_chip() - get a device to use to access a chip on a bus
 *
 * This returns the device for the given chip address. The device can then
 * be used with calls to i2c_read(), i2c_write(), i2c_probe(), etc.
 *
 * @bus:	Bus to examine
 * @chip_addr:	Chip address for the new device
 * @offset_len:	Length of a register offset in bytes (normally 1)
 * @devp:	Returns pointer to new device if found or -ENODEV if not
 *		found
 */
int i2c_get_chip(struct udevice *bus, uint chip_addr, uint offset_len,
		 struct udevice **devp);

/**
 * i2c_get_chip_for_busnum() - get a device to use to access a chip on
 *			       a bus number
 *
 * This returns the device for the given chip address on a particular bus
 * number.
 *
 * @busnum:	Bus number to examine
 * @chip_addr:	Chip address for the new device
 * @offset_len:	Length of a register offset in bytes (normally 1)
 * @devp:	Returns pointer to new device if found or -ENODEV if not
 *		found
 */
int i2c_get_chip_for_busnum(int busnum, int chip_addr, uint offset_len,
			    struct udevice **devp);

/**
 * i2c_get_chip_by_phandle() - get a device to use to access a chip
 *			       based on a phandle property pointing to it
 *
 * @parent: Parent device containing the phandle pointer
 * @name:   Name of phandle property in the parent device node
 * @devp:   Returns pointer to new device or NULL if not found
 * Return:  0 on success, -ve on failure
 */
int i2c_get_chip_by_phandle(const struct udevice *parent, const char *prop_name,
			    struct udevice **devp);

/**
 * i2c_chip_of_to_plat() - Decode standard I2C platform data
 *
 * This decodes the chip address from a device tree node and puts it into
 * its dm_i2c_chip structure. This should be called in your driver's
 * of_to_plat() method.
 *
 * @blob:	Device tree blob
 * @node:	Node offset to read from
 * @spi:	Place to put the decoded information
 */
int i2c_chip_of_to_plat(struct udevice *dev, struct dm_i2c_chip *chip);

/**
 * i2c_dump_msgs() - Dump a list of I2C messages
 *
 * This may be useful for debugging.
 *
 * @msg:	Message list to dump
 * @nmsgs:	Number of messages
 */
void i2c_dump_msgs(struct i2c_msg *msg, int nmsgs);

/**
 * i2c_emul_find() - Find an emulator for an i2c sandbox device
 *
 * This looks at the device's 'emul' phandle
 *
 * @dev: Device to find an emulator for
 * @emulp: Returns the associated emulator, if found *
 * Return: 0 if OK, -ENOENT or -ENODEV if not found
 */
int i2c_emul_find(struct udevice *dev, struct udevice **emulp);

/**
 * i2c_emul_set_idx() - Set the emulator index for an i2c sandbox device
 *
 * With of-platdata we cannot find the emulator using the device tree, so rely
 * on the bind() method of each i2c driver calling this function to tell us
 * the of-platdata idx of the emulator
 *
 * @dev: i2c device to set the emulator for
 * @emul_idx: of-platdata index for that emulator
 */
void i2c_emul_set_idx(struct udevice *dev, int emul_idx);

/**
 * i2c_emul_get_device() - Find the device being emulated
 *
 * Given an emulator this returns the associated device
 *
 * @emul: Emulator for the device
 * Return: device that @emul is emulating
 */
struct udevice *i2c_emul_get_device(struct udevice *emul);

/* ACPI operations for generic I2C devices */
extern struct acpi_ops i2c_acpi_ops;

/**
 * acpi_i2c_of_to_plat() - Read properties intended for ACPI
 *
 * This reads the generic I2C properties from the device tree, so that these
 * can be used to create ACPI information for the device.
 *
 * See the i2c/generic-acpi.txt binding file for information about the
 * properties.
 *
 * @dev: I2C device to process
 * Return: 0 if OK, -EINVAL if acpi,hid is not present
 */
int acpi_i2c_of_to_plat(struct udevice *dev);

#ifdef CONFIG_SYS_I2C_EARLY_INIT
void i2c_early_init_f(void);
#endif

#if !CONFIG_IS_ENABLED(DM_I2C)

/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 * The implementation MUST NOT use static or global variables if the
 * I2C routines are used to read SDRAM configuration information
 * because this is done before the memories are initialized. Limited
 * use of stack-based variables are OK (the initial stack size is
 * limited).
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

/*
 * Configuration items.
 */
#define I2C_RXTX_LEN	128	/* maximum tx/rx buffer length */

/* no muxes used bus = i2c adapters */
#define CFG_SYS_NUM_I2C_BUSES	ll_entry_count(struct i2c_adapter, i2c)

struct i2c_adapter {
	void		(*init)(struct i2c_adapter *adap, int speed,
				int slaveaddr);
	int		(*probe)(struct i2c_adapter *adap, uint8_t chip);
	int		(*read)(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len);
	int		(*write)(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len);
	uint		(*set_bus_speed)(struct i2c_adapter *adap,
				uint speed);
	int		speed;
	int		waitdelay;
	int		slaveaddr;
	int		init_done;
	int		hwadapnr;
	char		*name;
};

#define U_BOOT_I2C_MKENT_COMPLETE(_init, _probe, _read, _write, \
		_set_speed, _speed, _slaveaddr, _hwadapnr, _name) \
	{ \
		.init		=	_init, \
		.probe		=	_probe, \
		.read		=	_read, \
		.write		=	_write, \
		.set_bus_speed	=	_set_speed, \
		.speed		=	_speed, \
		.slaveaddr	=	_slaveaddr, \
		.init_done	=	0, \
		.hwadapnr	=	_hwadapnr, \
		.name		=	#_name \
};

#define U_BOOT_I2C_ADAP_COMPLETE(_name, _init, _probe, _read, _write, \
			_set_speed, _speed, _slaveaddr, _hwadapnr) \
	ll_entry_declare(struct i2c_adapter, _name, i2c) = \
	U_BOOT_I2C_MKENT_COMPLETE(_init, _probe, _read, _write, \
		 _set_speed, _speed, _slaveaddr, _hwadapnr, _name);

struct i2c_adapter *i2c_get_adapter(int index);

#define I2C_ADAPTER(bus)	bus
#define	I2C_BUS			gd->cur_i2c_bus

#define	I2C_ADAP_NR(bus)	i2c_get_adapter(I2C_ADAPTER(bus))
#define	I2C_ADAP		I2C_ADAP_NR(gd->cur_i2c_bus)
#define I2C_ADAP_HWNR		(I2C_ADAP->hwadapnr)

#ifndef I2C_SOFT_DECLARATIONS
# if (defined(CONFIG_AT91RM9200) || \
	defined(CONFIG_AT91SAM9260) ||  defined(CONFIG_AT91SAM9261) || \
	defined(CONFIG_AT91SAM9263))
#  define I2C_SOFT_DECLARATIONS	at91_pio_t *pio	= (at91_pio_t *) ATMEL_BASE_PIOA;
# else
#  define I2C_SOFT_DECLARATIONS
# endif
#endif

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
void i2c_init(int speed, int slaveaddr);
void i2c_init_board(void);

#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */
unsigned int i2c_get_bus_num(void);

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 *	bus - bus index, zero based
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_num(unsigned int bus);

/*
 * i2c_init_all():
 *
 * Initializes all I2C adapters in the system. All i2c_adap structures must
 * be initialized beforehead with function pointers and data, including
 * speed and slaveaddr. Returns 0 on success, non-0 on failure.
 */
void i2c_init_all(void);

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uint8_t chip);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len);

int i2c_write(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
uint8_t i2c_reg_read(uint8_t addr, uint8_t reg);

void i2c_reg_write(uint8_t addr, uint8_t reg, uint8_t val);

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 *	speed - bus speed in Hz
 *
 *	Returns: new bus speed
 *
 */
unsigned int i2c_set_bus_speed(unsigned int speed);

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

unsigned int i2c_get_bus_speed(void);

#else

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uchar chip);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
static inline u8 i2c_reg_read(u8 addr, u8 reg)
{
	u8 buf;

#ifdef DEBUG
	printf("%s: addr=0x%02x, reg=0x%02x\n", __func__, addr, reg);
#endif

	i2c_read(addr, reg, 1, &buf, 1);

	return buf;
}

static inline void i2c_reg_write(u8 addr, u8 reg, u8 val)
{
#ifdef DEBUG
	printf("%s: addr=0x%02x, reg=0x%02x, val=0x%02x\n",
	       __func__, addr, reg, val);
#endif

	i2c_write(addr, reg, 1, &val, 1);
}

/*
 * Functions for setting the current I2C bus and its speed
 */

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 *	bus - bus index, zero based
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_num(unsigned int bus);

/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */

unsigned int i2c_get_bus_num(void);

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 *	speed - bus speed in Hz
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_speed(unsigned int);

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

unsigned int i2c_get_bus_speed(void);
#endif /* CONFIG_SYS_I2C_LEGACY */

#endif /* !CONFIG_DM_I2C */

#endif	/* _I2C_H_ */
