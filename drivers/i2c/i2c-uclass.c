/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <i2c.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

#define I2C_MAX_OFFSET_LEN	4

/**
 * i2c_setup_offset() - Set up a new message with a chip offset
 *
 * @chip:	Chip to use
 * @offset:	Byte offset within chip
 * @offset_buf:	Place to put byte offset
 * @msg:	Message buffer
 * @return 0 if OK, -EADDRNOTAVAIL if the offset length is 0. In that case the
 * message is still set up but will not contain an offset.
 */
static int i2c_setup_offset(struct dm_i2c_chip *chip, uint offset,
			    uint8_t offset_buf[], struct i2c_msg *msg)
{
	int offset_len;

	msg->addr = chip->chip_addr;
	msg->flags = chip->flags & DM_I2C_CHIP_10BIT ? I2C_M_TEN : 0;
	msg->len = chip->offset_len;
	msg->buf = offset_buf;
	if (!chip->offset_len)
		return -EADDRNOTAVAIL;
	assert(chip->offset_len <= I2C_MAX_OFFSET_LEN);
	offset_len = chip->offset_len;
	while (offset_len--)
		*offset_buf++ = offset >> (8 * offset_len);

	return 0;
}

static int i2c_read_bytewise(struct udevice *dev, uint offset,
			     uint8_t *buffer, int len)
{
	struct dm_i2c_chip *chip = dev_get_parentdata(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[2], *ptr;
	uint8_t offset_buf[I2C_MAX_OFFSET_LEN];
	int ret;
	int i;

	for (i = 0; i < len; i++) {
		if (i2c_setup_offset(chip, offset + i, offset_buf, msg))
			return -EINVAL;
		ptr = msg + 1;
		ptr->addr = chip->chip_addr;
		ptr->flags = msg->flags | I2C_M_RD;
		ptr->len = 1;
		ptr->buf = &buffer[i];
		ptr++;

		ret = ops->xfer(bus, msg, ptr - msg);
		if (ret)
			return ret;
	}

	return 0;
}

static int i2c_write_bytewise(struct udevice *dev, uint offset,
			     const uint8_t *buffer, int len)
{
	struct dm_i2c_chip *chip = dev_get_parentdata(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[1];
	uint8_t buf[I2C_MAX_OFFSET_LEN + 1];
	int ret;
	int i;

	for (i = 0; i < len; i++) {
		if (i2c_setup_offset(chip, offset + i, buf, msg))
			return -EINVAL;
		buf[msg->len++] = buffer[i];

		ret = ops->xfer(bus, msg, 1);
		if (ret)
			return ret;
	}

	return 0;
}

int i2c_read(struct udevice *dev, uint offset, uint8_t *buffer, int len)
{
	struct dm_i2c_chip *chip = dev_get_parentdata(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[2], *ptr;
	uint8_t offset_buf[I2C_MAX_OFFSET_LEN];
	int msg_count;

	if (!ops->xfer)
		return -ENOSYS;
	if (chip->flags & DM_I2C_CHIP_RD_ADDRESS)
		return i2c_read_bytewise(dev, offset, buffer, len);
	ptr = msg;
	if (!i2c_setup_offset(chip, offset, offset_buf, ptr))
		ptr++;

	if (len) {
		ptr->addr = chip->chip_addr;
		ptr->flags = chip->flags & DM_I2C_CHIP_10BIT ? I2C_M_TEN : 0;
		ptr->flags |= I2C_M_RD;
		ptr->len = len;
		ptr->buf = buffer;
		ptr++;
	}
	msg_count = ptr - msg;

	return ops->xfer(bus, msg, msg_count);
}

int i2c_write(struct udevice *dev, uint offset, const uint8_t *buffer, int len)
{
	struct dm_i2c_chip *chip = dev_get_parentdata(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[1];

	if (!ops->xfer)
		return -ENOSYS;

	if (chip->flags & DM_I2C_CHIP_WR_ADDRESS)
		return i2c_write_bytewise(dev, offset, buffer, len);
	/*
	 * The simple approach would be to send two messages here: one to
	 * set the offset and one to write the bytes. However some drivers
	 * will not be expecting this, and some chips won't like how the
	 * driver presents this on the I2C bus.
	 *
	 * The API does not support separate offset and data. We could extend
	 * it with a flag indicating that there is data in the next message
	 * that needs to be processed in the same transaction. We could
	 * instead add an additional buffer to each message. For now, handle
	 * this in the uclass since it isn't clear what the impact on drivers
	 * would be with this extra complication. Unfortunately this means
	 * copying the message.
	 *
	 * Use the stack for small messages, malloc() for larger ones. We
	 * need to allow space for the offset (up to 4 bytes) and the message
	 * itself.
	 */
	if (len < 64) {
		uint8_t buf[I2C_MAX_OFFSET_LEN + len];

		i2c_setup_offset(chip, offset, buf, msg);
		msg->len += len;
		memcpy(buf + chip->offset_len, buffer, len);

		return ops->xfer(bus, msg, 1);
	} else {
		uint8_t *buf;
		int ret;

		buf = malloc(I2C_MAX_OFFSET_LEN + len);
		if (!buf)
			return -ENOMEM;
		i2c_setup_offset(chip, offset, buf, msg);
		msg->len += len;
		memcpy(buf + chip->offset_len, buffer, len);

		ret = ops->xfer(bus, msg, 1);
		free(buf);
		return ret;
	}
}

/**
 * i2c_probe_chip() - probe for a chip on a bus
 *
 * @bus:	Bus to probe
 * @chip_addr:	Chip address to probe
 * @flags:	Flags for the chip
 * @return 0 if found, -ENOSYS if the driver is invalid, -EREMOTEIO if the chip
 * does not respond to probe
 */
static int i2c_probe_chip(struct udevice *bus, uint chip_addr,
			  enum dm_i2c_chip_flags chip_flags)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[1];
	int ret;

	if (ops->probe_chip) {
		ret = ops->probe_chip(bus, chip_addr, chip_flags);
		if (!ret || ret != -ENOSYS)
			return ret;
	}

	if (!ops->xfer)
		return -ENOSYS;

	/* Probe with a zero-length message */
	msg->addr = chip_addr;
	msg->flags = chip_flags & DM_I2C_CHIP_10BIT ? I2C_M_TEN : 0;
	msg->len = 0;
	msg->buf = NULL;

	return ops->xfer(bus, msg, 1);
}

static int i2c_bind_driver(struct udevice *bus, uint chip_addr,
			   struct udevice **devp)
{
	struct dm_i2c_chip chip;
	char name[30], *str;
	struct udevice *dev;
	int ret;

	snprintf(name, sizeof(name), "generic_%x", chip_addr);
	str = strdup(name);
	ret = device_bind_driver(bus, "i2c_generic_chip_drv", str, &dev);
	debug("%s:  device_bind_driver: ret=%d\n", __func__, ret);
	if (ret)
		goto err_bind;

	/* Tell the device what we know about it */
	memset(&chip, '\0', sizeof(chip));
	chip.chip_addr = chip_addr;
	chip.offset_len = 1;	/* we assume */
	ret = device_probe_child(dev, &chip);
	debug("%s:  device_probe_child: ret=%d\n", __func__, ret);
	if (ret)
		goto err_probe;

	*devp = dev;
	return 0;

err_probe:
	device_unbind(dev);
err_bind:
	free(str);
	return ret;
}

int i2c_get_chip(struct udevice *bus, uint chip_addr, struct udevice **devp)
{
	struct udevice *dev;

	debug("%s: Searching bus '%s' for address %02x: ", __func__,
	      bus->name, chip_addr);
	for (device_find_first_child(bus, &dev); dev;
			device_find_next_child(&dev)) {
		struct dm_i2c_chip store;
		struct dm_i2c_chip *chip = dev_get_parentdata(dev);
		int ret;

		if (!chip) {
			chip = &store;
			i2c_chip_ofdata_to_platdata(gd->fdt_blob,
						    dev->of_offset, chip);
		}
		if (chip->chip_addr == chip_addr) {
			ret = device_probe(dev);
			debug("found, ret=%d\n", ret);
			if (ret)
				return ret;
			*devp = dev;
			return 0;
		}
	}
	debug("not found\n");
	return i2c_bind_driver(bus, chip_addr, devp);
}

int i2c_get_chip_for_busnum(int busnum, int chip_addr, struct udevice **devp)
{
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus);
	if (ret) {
		debug("Cannot find I2C bus %d\n", busnum);
		return ret;
	}
	ret = i2c_get_chip(bus, chip_addr, devp);
	if (ret) {
		debug("Cannot find I2C chip %02x on bus %d\n", chip_addr,
		      busnum);
		return ret;
	}

	return 0;
}

int i2c_probe(struct udevice *bus, uint chip_addr, uint chip_flags,
	      struct udevice **devp)
{
	int ret;

	*devp = NULL;

	/* First probe that chip */
	ret = i2c_probe_chip(bus, chip_addr, chip_flags);
	debug("%s: bus='%s', address %02x, ret=%d\n", __func__, bus->name,
	      chip_addr, ret);
	if (ret)
		return ret;

	/* The chip was found, see if we have a driver, and probe it */
	ret = i2c_get_chip(bus, chip_addr, devp);
	debug("%s:  i2c_get_chip: ret=%d\n", __func__, ret);

	return ret;
}

int i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct dm_i2c_bus *i2c = bus->uclass_priv;
	int ret;

	/*
	 * If we have a method, call it. If not then the driver probably wants
	 * to deal with speed changes on the next transfer. It can easily read
	 * the current speed from this uclass
	 */
	if (ops->set_bus_speed) {
		ret = ops->set_bus_speed(bus, speed);
		if (ret)
			return ret;
	}
	i2c->speed_hz = speed;

	return 0;
}

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of selected I2C bus in Hz
 */
int i2c_get_bus_speed(struct udevice *bus)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct dm_i2c_bus *i2c = bus->uclass_priv;

	if (!ops->get_bus_speed)
		return i2c->speed_hz;

	return ops->get_bus_speed(bus);
}

int i2c_set_chip_flags(struct udevice *dev, uint flags)
{
	struct udevice *bus = dev->parent;
	struct dm_i2c_chip *chip = dev_get_parentdata(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	int ret;

	if (ops->set_flags) {
		ret = ops->set_flags(dev, flags);
		if (ret)
			return ret;
	}
	chip->flags = flags;

	return 0;
}

int i2c_get_chip_flags(struct udevice *dev, uint *flagsp)
{
	struct dm_i2c_chip *chip = dev_get_parentdata(dev);

	*flagsp = chip->flags;

	return 0;
}

int i2c_set_chip_offset_len(struct udevice *dev, uint offset_len)
{
	struct dm_i2c_chip *chip = dev_get_parentdata(dev);

	if (offset_len > I2C_MAX_OFFSET_LEN)
		return -EINVAL;
	chip->offset_len = offset_len;

	return 0;
}

int i2c_deblock(struct udevice *bus)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);

	/*
	 * We could implement a software deblocking here if we could get
	 * access to the GPIOs used by I2C, and switch them to GPIO mode
	 * and then back to I2C. This is somewhat beyond our powers in
	 * driver model at present, so for now just fail.
	 *
	 * See https://patchwork.ozlabs.org/patch/399040/
	 */
	if (!ops->deblock)
		return -ENOSYS;

	return ops->deblock(bus);
}

int i2c_chip_ofdata_to_platdata(const void *blob, int node,
				struct dm_i2c_chip *chip)
{
	chip->offset_len = 1;	/* default */
	chip->flags = 0;
	chip->chip_addr = fdtdec_get_int(gd->fdt_blob, node, "reg", -1);
	if (chip->chip_addr == -1) {
		debug("%s: I2C Node '%s' has no 'reg' property\n", __func__,
		      fdt_get_name(blob, node, NULL));
		return -EINVAL;
	}

	return 0;
}

static int i2c_post_probe(struct udevice *dev)
{
	struct dm_i2c_bus *i2c = dev->uclass_priv;

	i2c->speed_hz = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				     "clock-frequency", 100000);

	return i2c_set_bus_speed(dev, i2c->speed_hz);
}

int i2c_post_bind(struct udevice *dev)
{
	/* Scan the bus for devices */
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

UCLASS_DRIVER(i2c) = {
	.id		= UCLASS_I2C,
	.name		= "i2c",
	.per_device_auto_alloc_size = sizeof(struct dm_i2c_bus),
	.post_bind	= i2c_post_bind,
	.post_probe	= i2c_post_probe,
};

UCLASS_DRIVER(i2c_generic) = {
	.id		= UCLASS_I2C_GENERIC,
	.name		= "i2c_generic",
};

U_BOOT_DRIVER(i2c_generic_chip_drv) = {
	.name		= "i2c_generic_chip_drv",
	.id		= UCLASS_I2C_GENERIC,
};
