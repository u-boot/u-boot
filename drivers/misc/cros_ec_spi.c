/*
 * Chromium OS cros_ec driver - SPI interface
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
#include <cros_ec.h>
#include <dm.h>
#include <errno.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DM_CROS_EC
int cros_ec_spi_packet(struct udevice *udev, int out_bytes, int in_bytes)
{
	struct cros_ec_dev *dev = udev->uclass_priv;
#else
int cros_ec_spi_packet(struct cros_ec_dev *dev, int out_bytes, int in_bytes)
{
#endif
	struct spi_slave *slave = dev_get_parentdata(dev->dev);
	int rv;

	/* Do the transfer */
	if (spi_claim_bus(slave)) {
		debug("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

	rv = spi_xfer(slave, max(out_bytes, in_bytes) * 8,
		      dev->dout, dev->din,
		      SPI_XFER_BEGIN | SPI_XFER_END);

	spi_release_bus(slave);

	if (rv) {
		debug("%s: Cannot complete SPI transfer\n", __func__);
		return -1;
	}

	return in_bytes;
}

/**
 * Send a command to a LPC CROS_EC device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		CROS_EC device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout		Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param dinp		Returns pointer to response data. This will be
 *			untouched unless we return a value > 0.
 * @param din_len	Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
#ifdef CONFIG_DM_CROS_EC
int cros_ec_spi_command(struct udevice *udev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len)
{
	struct cros_ec_dev *dev = udev->uclass_priv;
#else
int cros_ec_spi_command(struct cros_ec_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len)
{
#endif
	struct spi_slave *slave = dev_get_parentdata(dev->dev);
	int in_bytes = din_len + 4;	/* status, length, checksum, trailer */
	uint8_t *out;
	uint8_t *p;
	int csum, len;
	int rv;

	if (dev->protocol_version != 2) {
		debug("%s: Unsupported EC protcol version %d\n",
		      __func__, dev->protocol_version);
		return -1;
	}

	/*
	 * Sanity-check input size to make sure it plus transaction overhead
	 * fits in the internal device buffer.
	 */
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	/* We represent message length as a byte */
	if (dout_len > 0xff) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}

	/*
	 * Clear input buffer so we don't get false hits for MSG_HEADER
	 */
	memset(dev->din, '\0', in_bytes);

	if (spi_claim_bus(slave)) {
		debug("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

	out = dev->dout;
	out[0] = EC_CMD_VERSION0 + cmd_version;
	out[1] = cmd;
	out[2] = (uint8_t)dout_len;
	memcpy(out + 3, dout, dout_len);
	csum = cros_ec_calc_checksum(out, 3)
	       + cros_ec_calc_checksum(dout, dout_len);
	out[3 + dout_len] = (uint8_t)csum;

	/*
	 * Send output data and receive input data starting such that the
	 * message body will be dword aligned.
	 */
	p = dev->din + sizeof(int64_t) - 2;
	len = dout_len + 4;
	cros_ec_dump_data("out", cmd, out, len);
	rv = spi_xfer(slave, max(len, in_bytes) * 8, out, p,
		      SPI_XFER_BEGIN | SPI_XFER_END);

	spi_release_bus(slave);

	if (rv) {
		debug("%s: Cannot complete SPI transfer\n", __func__);
		return -1;
	}

	len = min((int)p[1], din_len);
	cros_ec_dump_data("in", -1, p, len + 3);

	/* Response code is first byte of message */
	if (p[0] != EC_RES_SUCCESS) {
		printf("%s: Returned status %d\n", __func__, p[0]);
		return -(int)(p[0]);
	}

	/* Check checksum */
	csum = cros_ec_calc_checksum(p, len + 2);
	if (csum != p[len + 2]) {
		debug("%s: Invalid checksum rx %#02x, calced %#02x\n", __func__,
		      p[2 + len], csum);
		return -1;
	}

	/* Anything else is the response data */
	*dinp = p + 2;

	return len;
}

#ifndef CONFIG_DM_CROS_EC
int cros_ec_spi_decode_fdt(struct cros_ec_dev *dev, const void *blob)
{
	/* Decode interface-specific FDT params */
	dev->max_frequency = fdtdec_get_int(blob, dev->node,
					    "spi-max-frequency", 500000);
	dev->cs = fdtdec_get_int(blob, dev->node, "reg", 0);

	return 0;
}

/**
 * Initialize SPI protocol.
 *
 * @param dev		CROS_EC device
 * @param blob		Device tree blob
 * @return 0 if ok, -1 on error
 */
int cros_ec_spi_init(struct cros_ec_dev *dev, const void *blob)
{
	int ret;

	ret = spi_setup_slave_fdt(blob, dev->node, dev->parent_node,
				  &slave);
	if (ret) {
		debug("%s: Could not setup SPI slave\n", __func__);
		return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_DM_CROS_EC
int cros_ec_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parentdata(dev);
	int ret;

	/*
	 * TODO(sjg@chromium.org)
	 *
	 * This is really horrible at present. It is an artifact of removing
	 * the child_pre_probe() method for SPI. Everything here could go in
	 * an automatic function, except that spi_get_bus_and_cs() wants to
	 * set it up manually and call device_probe_child().
	 *
	 * The solution may be to re-enable the child_pre_probe() method for
	 * SPI and have it do nothing if the child is already passed in via
	 * device_probe_child().
	 */
	slave->dev = dev;
	ret = spi_ofdata_to_platdata(gd->fdt_blob, dev->of_offset, slave);
	if (ret)
		return ret;
	return cros_ec_register(dev);
}

struct dm_cros_ec_ops cros_ec_ops = {
	.packet = cros_ec_spi_packet,
	.command = cros_ec_spi_command,
};

static const struct udevice_id cros_ec_ids[] = {
	{ .compatible = "google,cros-ec" },
	{ }
};

U_BOOT_DRIVER(cros_ec_spi) = {
	.name		= "cros_ec",
	.id		= UCLASS_CROS_EC,
	.of_match	= cros_ec_ids,
	.probe		= cros_ec_probe,
	.ops		= &cros_ec_ops,
};
#endif
