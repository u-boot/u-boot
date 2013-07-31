/*
 * Chromium OS cros_ec driver - LPC interface
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
#include <command.h>
#include <cros_ec.h>
#include <asm/io.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, ##b)
#else
#define debug_trace(fmt, b...)
#endif

static int wait_for_sync(struct cros_ec_dev *dev)
{
	unsigned long start;

	start = get_timer(0);
	while (inb(EC_LPC_ADDR_HOST_CMD) & EC_LPC_STATUS_BUSY_MASK) {
		if (get_timer(start) > 1000) {
			debug("%s: Timeout waiting for CROS_EC sync\n",
			      __func__);
			return -1;
		}
	}

	return 0;
}

/**
 * Send a command to a LPC CROS_EC device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		CROS_EC device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param dinp          Place to put pointer to response data
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
static int old_lpc_command(struct cros_ec_dev *dev, uint8_t cmd,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len)
{
	int ret, i;

	if (dout_len > EC_OLD_PARAM_SIZE) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}

	if (din_len > EC_OLD_PARAM_SIZE) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	debug_trace("cmd: %02x, ", cmd);
	for (i = 0; i < dout_len; i++) {
		debug_trace("%02x ", dout[i]);
		outb(dout[i], EC_LPC_ADDR_OLD_PARAM + i);
	}
	outb(cmd, EC_LPC_ADDR_HOST_CMD);
	debug_trace("\n");

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	ret = inb(EC_LPC_ADDR_HOST_DATA);
	if (ret) {
		debug("%s: CROS_EC result code %d\n", __func__, ret);
		return -ret;
	}

	debug_trace("resp: %02x, ", ret);
	for (i = 0; i < din_len; i++) {
		dev->din[i] = inb(EC_LPC_ADDR_OLD_PARAM + i);
		debug_trace("%02x ", dev->din[i]);
	}
	debug_trace("\n");
	*dinp = dev->din;

	return din_len;
}

int cros_ec_lpc_command(struct cros_ec_dev *dev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len)
{
	const int cmd_addr = EC_LPC_ADDR_HOST_CMD;
	const int data_addr = EC_LPC_ADDR_HOST_DATA;
	const int args_addr = EC_LPC_ADDR_HOST_ARGS;
	const int param_addr = EC_LPC_ADDR_HOST_PARAM;

	struct ec_lpc_host_args args;
	uint8_t *d;
	int csum;
	int i;

	/* Fall back to old-style command interface if args aren't supported */
	if (!dev->cmd_version_is_supported)
		return old_lpc_command(dev, cmd, dout, dout_len, dinp,
				       din_len);

	if (dout_len > EC_HOST_PARAM_SIZE) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}

	/* Fill in args */
	args.flags = EC_HOST_ARGS_FLAG_FROM_HOST;
	args.command_version = cmd_version;
	args.data_size = dout_len;

	/* Calculate checksum */
	csum = cmd + args.flags + args.command_version + args.data_size;
	for (i = 0, d = (uint8_t *)dout; i < dout_len; i++, d++)
		csum += *d;

	args.checksum = (uint8_t)csum;

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	/* Write args */
	for (i = 0, d = (uint8_t *)&args; i < sizeof(args); i++, d++)
		outb(*d, args_addr + i);

	/* Write data, if any */
	debug_trace("cmd: %02x, ver: %02x", cmd, cmd_version);
	for (i = 0, d = (uint8_t *)dout; i < dout_len; i++, d++) {
		outb(*d, param_addr + i);
		debug_trace("%02x ", *d);
	}

	outb(cmd, cmd_addr);
	debug_trace("\n");

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting for response\n", __func__);
		return -1;
	}

	/* Check result */
	i = inb(data_addr);
	if (i) {
		debug("%s: CROS_EC result code %d\n", __func__, i);
		return -i;
	}

	/* Read back args */
	for (i = 0, d = (uint8_t *)&args; i < sizeof(args); i++, d++)
		*d = inb(args_addr + i);

	/*
	 * If EC didn't modify args flags, then somehow we sent a new-style
	 * command to an old EC, which means it would have read its params
	 * from the wrong place.
	 */
	if (!(args.flags & EC_HOST_ARGS_FLAG_TO_HOST)) {
		debug("%s: CROS_EC protocol mismatch\n", __func__);
		return -EC_RES_INVALID_RESPONSE;
	}

	if (args.data_size > din_len) {
		debug("%s: CROS_EC returned too much data %d > %d\n",
		      __func__, args.data_size, din_len);
		return -EC_RES_INVALID_RESPONSE;
	}

	/* Read data, if any */
	for (i = 0, d = (uint8_t *)dev->din; i < args.data_size; i++, d++) {
		*d = inb(param_addr + i);
		debug_trace("%02x ", *d);
	}
	debug_trace("\n");

	/* Verify checksum */
	csum = cmd + args.flags + args.command_version + args.data_size;
	for (i = 0, d = (uint8_t *)dev->din; i < args.data_size; i++, d++)
		csum += *d;

	if (args.checksum != (uint8_t)csum) {
		debug("%s: CROS_EC response has invalid checksum\n", __func__);
		return -EC_RES_INVALID_CHECKSUM;
	}
	*dinp = dev->din;

	/* Return actual amount of data received */
	return args.data_size;
}

/**
 * Initialize LPC protocol.
 *
 * @param dev		CROS_EC device
 * @param blob		Device tree blob
 * @return 0 if ok, -1 on error
 */
int cros_ec_lpc_init(struct cros_ec_dev *dev, const void *blob)
{
	int byte, i;

	/* See if we can find an EC at the other end */
	byte = 0xff;
	byte &= inb(EC_LPC_ADDR_HOST_CMD);
	byte &= inb(EC_LPC_ADDR_HOST_DATA);
	for (i = 0; i < EC_HOST_PARAM_SIZE && (byte == 0xff); i++)
		byte &= inb(EC_LPC_ADDR_HOST_PARAM + i);
	if (byte == 0xff) {
		debug("%s: CROS_EC device not found on LPC bus\n",
			__func__);
		return -1;
	}

	return 0;
}

/*
 * Test if LPC command args are supported.
 *
 * The cheapest way to do this is by looking for the memory-mapped
 * flag.  This is faster than sending a new-style 'hello' command and
 * seeing whether the EC sets the EC_HOST_ARGS_FLAG_FROM_HOST flag
 * in args when it responds.
 */
int cros_ec_lpc_check_version(struct cros_ec_dev *dev)
{
	if (inb(EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID) == 'E' &&
			inb(EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID + 1)
				== 'C' &&
			(inb(EC_LPC_ADDR_MEMMAP +
				EC_MEMMAP_HOST_CMD_FLAGS) &
				EC_HOST_CMD_FLAG_LPC_ARGS_SUPPORTED)) {
		dev->cmd_version_is_supported = 1;
	} else {
		/* We are going to use the old IO ports */
		dev->cmd_version_is_supported = 0;
	}
	debug("lpc: version %s\n", dev->cmd_version_is_supported ?
			"new" : "old");

	return 0;
}
