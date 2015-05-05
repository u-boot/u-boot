/*
 * Chromium OS cros_ec driver - I2C interface
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * The Matrix Keyboard Protocol driver handles talking to the keyboard
 * controller chip. Mostly this is for keyboard functions, but some other
 * things have slipped in, so we provide generic services to talk to the
 * KBC.
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <cros_ec.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

static int cros_ec_i2c_command(struct udevice *udev, uint8_t cmd,
			       int cmd_version, const uint8_t *dout,
			       int dout_len, uint8_t **dinp, int din_len)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	/* version8, cmd8, arglen8, out8[dout_len], csum8 */
	int out_bytes = dout_len + 4;
	/* response8, arglen8, in8[din_len], checksum8 */
	int in_bytes = din_len + 3;
	uint8_t *ptr;
	/* Receive input data, so that args will be dword aligned */
	uint8_t *in_ptr;
	int len, csum, ret;

	/*
	 * Sanity-check I/O sizes given transaction overhead in internal
	 * buffers.
	 */
	if (out_bytes > sizeof(dev->dout)) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}
	assert(dout_len >= 0);
	assert(dinp);

	/*
	 * Copy command and data into output buffer so we can do a single I2C
	 * burst transaction.
	 */
	ptr = dev->dout;

	/*
	 * in_ptr starts of pointing to a dword-aligned input data buffer.
	 * We decrement it back by the number of header bytes we expect to
	 * receive, so that the first parameter of the resulting input data
	 * will be dword aligned.
	 */
	in_ptr = dev->din + sizeof(int64_t);

	if (dev->protocol_version != 2) {
		/* Something we don't support */
		debug("%s: Protocol version %d unsupported\n",
		      __func__, dev->protocol_version);
		return -1;
	}

	*ptr++ = EC_CMD_VERSION0 + cmd_version;
	*ptr++ = cmd;
	*ptr++ = dout_len;
	in_ptr -= 2;	/* Expect status, length bytes */

	memcpy(ptr, dout, dout_len);
	ptr += dout_len;

	*ptr++ = (uint8_t)
		cros_ec_calc_checksum(dev->dout, dout_len + 3);

	/* Send output data */
	cros_ec_dump_data("out", -1, dev->dout, out_bytes);
	ret = dm_i2c_write(udev, 0, dev->dout, out_bytes);
	if (ret) {
		debug("%s: Cannot complete I2C write to %s\n", __func__,
		      udev->name);
		ret = -1;
	}

	if (!ret) {
		ret = dm_i2c_read(udev, 0, in_ptr, in_bytes);
		if (ret) {
			debug("%s: Cannot complete I2C read from %s\n",
			      __func__, udev->name);
			ret = -1;
		}
	}

	if (*in_ptr != EC_RES_SUCCESS) {
		debug("%s: Received bad result code %d\n", __func__, *in_ptr);
		return -(int)*in_ptr;
	}

	len = in_ptr[1];
	if (len + 3 > sizeof(dev->din)) {
		debug("%s: Received length %#02x too large\n",
		      __func__, len);
		return -1;
	}
	csum = cros_ec_calc_checksum(in_ptr, 2 + len);
	if (csum != in_ptr[2 + len]) {
		debug("%s: Invalid checksum rx %#02x, calced %#02x\n",
		      __func__, in_ptr[2 + din_len], csum);
		return -1;
	}
	din_len = min(din_len, len);
	cros_ec_dump_data("in", -1, in_ptr, din_len + 3);

	/* Return pointer to dword-aligned input data, if any */
	*dinp = dev->din + sizeof(int64_t);

	return din_len;
}

static int cros_ec_probe(struct udevice *dev)
{
	return cros_ec_register(dev);
}

static struct dm_cros_ec_ops cros_ec_ops = {
	.command = cros_ec_i2c_command,
};

static const struct udevice_id cros_ec_ids[] = {
	{ .compatible = "google,cros-ec-i2c" },
	{ }
};

U_BOOT_DRIVER(cros_ec_i2c) = {
	.name		= "cros_ec_i2c",
	.id		= UCLASS_CROS_EC,
	.of_match	= cros_ec_ids,
	.probe		= cros_ec_probe,
	.ops		= &cros_ec_ops,
};
