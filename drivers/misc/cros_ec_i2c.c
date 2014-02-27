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
#include <i2c.h>
#include <cros_ec.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

int cros_ec_i2c_command(struct cros_ec_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len)
{
	int old_bus = 0;
	/* version8, cmd8, arglen8, out8[dout_len], csum8 */
	int out_bytes = dout_len + 4;
	/* response8, arglen8, in8[din_len], checksum8 */
	int in_bytes = din_len + 3;
	uint8_t *ptr;
	/* Receive input data, so that args will be dword aligned */
	uint8_t *in_ptr;
	int len, csum, ret;

	old_bus = i2c_get_bus_num();

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

	/* Set to the proper i2c bus */
	if (i2c_set_bus_num(dev->bus_num)) {
		debug("%s: Cannot change to I2C bus %d\n", __func__,
			dev->bus_num);
		return -1;
	}

	/* Send output data */
	cros_ec_dump_data("out", -1, dev->dout, out_bytes);
	ret = i2c_write(dev->addr, 0, 0, dev->dout, out_bytes);
	if (ret) {
		debug("%s: Cannot complete I2C write to 0x%x\n",
			__func__, dev->addr);
		ret = -1;
	}

	if (!ret) {
		ret = i2c_read(dev->addr, 0, 0, in_ptr, in_bytes);
		if (ret) {
			debug("%s: Cannot complete I2C read from 0x%x\n",
				__func__, dev->addr);
			ret = -1;
		}
	}

	/* Return to original bus number */
	i2c_set_bus_num(old_bus);
	if (ret)
		return ret;

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

int cros_ec_i2c_decode_fdt(struct cros_ec_dev *dev, const void *blob)
{
	/* Decode interface-specific FDT params */
	dev->max_frequency = fdtdec_get_int(blob, dev->node,
					    "i2c-max-frequency", 100000);
	dev->bus_num = i2c_get_bus_num_fdt(dev->parent_node);
	if (dev->bus_num == -1) {
		debug("%s: Failed to read bus number\n", __func__);
		return -1;
	}
	dev->addr = fdtdec_get_int(blob, dev->node, "reg", -1);
	if (dev->addr == -1) {
		debug("%s: Failed to read device address\n", __func__);
		return -1;
	}

	return 0;
}

/**
 * Initialize I2C protocol.
 *
 * @param dev		CROS_EC device
 * @param blob		Device tree blob
 * @return 0 if ok, -1 on error
 */
int cros_ec_i2c_init(struct cros_ec_dev *dev, const void *blob)
{
	i2c_init(dev->max_frequency, dev->addr);

	return 0;
}
