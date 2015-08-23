/*
 * Copyright (C) 2011 Infineon Technologies
 *
 * Authors:
 * Peter Huewe <huewe.external@infineon.com>
 *
 * Description:
 * Device driver for TCG/TCPA TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org
 *
 * This device driver implements the TPM interface as defined in
 * the TCG TPM Interface Spec version 1.2, revision 1.0 and the
 * Infineon I2C Protocol Stack Specification v0.20.
 *
 * It is based on the Linux kernel driver tpm.c from Leendert van
 * Dorn, Dave Safford, Reiner Sailer, and Kyleen Hall.
 *
 * Version: 2.1.1
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/compiler.h>
#include <i2c.h>
#include <tis.h>
#include <tpm.h>
#include <asm-generic/errno.h>
#include <linux/types.h>
#include <linux/unaligned/be_byteshift.h>

#include "tpm_tis_i2c.h"
#include "tpm_internal.h"

DECLARE_GLOBAL_DATA_PTR;

static const char * const chip_name[] = {
	[SLB9635] = "slb9635tt",
	[SLB9645] = "slb9645tt",
	[UNKNOWN] = "unknown/fallback to slb9635",
};

static struct tpm_chip g_chip;

/*
 * tpm_tis_i2c_read() - read from TPM register
 * @addr: register address to read from
 * @buffer: provided by caller
 * @len: number of bytes to read
 *
 * Read len bytes from TPM register and put them into
 * buffer (little-endian format, i.e. first byte is put into buffer[0]).
 *
 * NOTE: TPM is big-endian for multi-byte values. Multi-byte
 * values have to be swapped.
 *
 * Return -EIO on error, 0 on success.
 */
static int tpm_tis_i2c_read(u8 addr, u8 *buffer, size_t len)
{
	int rc;
	int count;
	uint32_t addrbuf = addr;

	if ((g_chip.chip_type == SLB9635) || (g_chip.chip_type == UNKNOWN)) {
		/* slb9635 protocol should work in both cases */
		for (count = 0; count < MAX_COUNT; count++) {
			rc = dm_i2c_write(g_chip.dev, 0, (uchar *)&addrbuf, 1);
			if (rc == 0)
				break;  /* Success, break to skip sleep */
			udelay(SLEEP_DURATION_US);
		}
		if (rc)
			return -rc;

		/* After the TPM has successfully received the register address
		 * it needs some time, thus we're sleeping here again, before
		 * retrieving the data
		 */
		for (count = 0; count < MAX_COUNT; count++) {
			udelay(SLEEP_DURATION_US);
			rc = dm_i2c_read(g_chip.dev, 0, buffer, len);
			if (rc == 0)
				break;  /* success, break to skip sleep */
		}
	} else {
		/*
		 * Use a combined read for newer chips.
		 * Unfortunately the smbus functions are not suitable due to
		 * the 32 byte limit of the smbus.
		 * Retries should usually not be needed, but are kept just to
		 * be safe on the safe side.
		 */
		for (count = 0; count < MAX_COUNT; count++) {
			rc = dm_i2c_read(g_chip.dev, addr, buffer, len);
			if (rc == 0)
				break;  /* break here to skip sleep */
			udelay(SLEEP_DURATION_US);
		}
	}

	/* Take care of 'guard time' */
	udelay(SLEEP_DURATION_US);
	if (rc)
		return -rc;

	return 0;
}

static int tpm_tis_i2c_write_generic(u8 addr, u8 *buffer, size_t len,
				     unsigned int sleep_time_us, u8 max_count)
{
	int rc = 0;
	int count;

	for (count = 0; count < max_count; count++) {
		rc = dm_i2c_write(g_chip.dev, addr, buffer, len);
		if (rc == 0)
			break;  /* Success, break to skip sleep */
		udelay(sleep_time_us);
	}

	/* take care of 'guard time' */
	udelay(sleep_time_us);
	if (rc)
		return -rc;

	return 0;
}

/*
 * tpm_tis_i2c_write() - write to TPM register
 * @addr: register address to write to
 * @buffer: containing data to be written
 * @len: number of bytes to write
 *
 * Write len bytes from provided buffer to TPM register (little
 * endian format, i.e. buffer[0] is written as first byte).
 *
 * NOTE: TPM is big-endian for multi-byte values. Multi-byte
 * values have to be swapped.
 *
 * NOTE: use this function instead of the tpm_tis_i2c_write_generic function.
 *
 * Return -EIO on error, 0 on success
 */
static int tpm_tis_i2c_write(u8 addr, u8 *buffer, size_t len)
{
	return tpm_tis_i2c_write_generic(addr, buffer, len, SLEEP_DURATION_US,
					 MAX_COUNT);
}

/*
 * This function is needed especially for the cleanup situation after
 * sending TPM_READY
 */
static int tpm_tis_i2c_write_long(u8 addr, u8 *buffer, size_t len)
{
	return tpm_tis_i2c_write_generic(addr, buffer, len,
					 SLEEP_DURATION_LONG_US,
					 MAX_COUNT_LONG);
}

static int tpm_tis_i2c_check_locality(struct tpm_chip *chip, int loc)
{
	const u8 mask = TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID;
	u8 buf;
	int rc;

	rc = tpm_tis_i2c_read(TPM_ACCESS(loc), &buf, 1);
	if (rc < 0)
		return rc;

	if ((buf & mask) == mask) {
		chip->locality = loc;
		return loc;
	}

	return -1;
}

static void tpm_tis_i2c_release_locality(struct tpm_chip *chip, int loc,
					 int force)
{
	const u8 mask = TPM_ACCESS_REQUEST_PENDING | TPM_ACCESS_VALID;
	u8 buf;

	if (tpm_tis_i2c_read(TPM_ACCESS(loc), &buf, 1) < 0)
		return;

	if (force || (buf & mask) == mask) {
		buf = TPM_ACCESS_ACTIVE_LOCALITY;
		tpm_tis_i2c_write(TPM_ACCESS(loc), &buf, 1);
	}
}

static int tpm_tis_i2c_request_locality(struct tpm_chip *chip, int loc)
{
	unsigned long start, stop;
	u8 buf = TPM_ACCESS_REQUEST_USE;
	int rc;

	if (tpm_tis_i2c_check_locality(chip, loc) >= 0)
		return loc;  /* We already have the locality */

	rc = tpm_tis_i2c_write(TPM_ACCESS(loc), &buf, 1);
	if (rc)
		return rc;

	/* Wait for burstcount */
	start = get_timer(0);
	stop = chip->timeout_a;
	do {
		if (tpm_tis_i2c_check_locality(chip, loc) >= 0)
			return loc;
		mdelay(TPM_TIMEOUT_MS);
	} while (get_timer(start) < stop);

	return -1;
}

static u8 tpm_tis_i2c_status(struct tpm_chip *chip)
{
	/* NOTE: Since i2c read may fail, return 0 in this case --> time-out */
	u8 buf;

	if (tpm_tis_i2c_read(TPM_STS(chip->locality), &buf, 1) < 0)
		return 0;
	else
		return buf;
}

static void tpm_tis_i2c_ready(struct tpm_chip *chip)
{
	int rc;

	/* This causes the current command to be aborted */
	u8 buf = TPM_STS_COMMAND_READY;

	debug("%s\n", __func__);
	rc = tpm_tis_i2c_write_long(TPM_STS(chip->locality), &buf, 1);
	if (rc)
		debug("%s: rc=%d\n", __func__, rc);
}

static ssize_t tpm_tis_i2c_get_burstcount(struct tpm_chip *chip)
{
	unsigned long start, stop;
	ssize_t burstcnt;
	u8 addr, buf[3];

	/* Wait for burstcount */
	/* XXX: Which timeout value? Spec has 2 answers (c & d) */
	start = get_timer(0);
	stop = chip->timeout_d;
	do {
		/* Note: STS is little endian */
		addr = TPM_STS(chip->locality) + 1;
		if (tpm_tis_i2c_read(addr, buf, 3) < 0)
			burstcnt = 0;
		else
			burstcnt = (buf[2] << 16) + (buf[1] << 8) + buf[0];

		if (burstcnt)
			return burstcnt;
		mdelay(TPM_TIMEOUT_MS);
	} while (get_timer(start) < stop);

	return -EBUSY;
}

static int tpm_tis_i2c_wait_for_stat(struct tpm_chip *chip, u8 mask,
				     unsigned long timeout, int *status)
{
	unsigned long start, stop;

	/* Check current status */
	*status = tpm_tis_i2c_status(chip);
	if ((*status & mask) == mask)
		return 0;

	start = get_timer(0);
	stop = timeout;
	do {
		mdelay(TPM_TIMEOUT_MS);
		*status = tpm_tis_i2c_status(chip);
		if ((*status & mask) == mask)
			return 0;
	} while (get_timer(start) < stop);

	return -ETIME;
}

static int tpm_tis_i2c_recv_data(struct tpm_chip *chip, u8 *buf, size_t count)
{
	size_t size = 0;
	ssize_t burstcnt;
	int rc;

	while (size < count) {
		burstcnt = tpm_tis_i2c_get_burstcount(chip);

		/* burstcount < 0 -> tpm is busy */
		if (burstcnt < 0)
			return burstcnt;

		/* Limit received data to max left */
		if (burstcnt > (count - size))
			burstcnt = count - size;

		rc = tpm_tis_i2c_read(TPM_DATA_FIFO(chip->locality),
				&(buf[size]), burstcnt);
		if (rc == 0)
			size += burstcnt;
	}

	return size;
}

static int tpm_tis_i2c_recv(struct tpm_chip *chip, u8 *buf, size_t count)
{
	int size = 0;
	int expected, status;

	if (count < TPM_HEADER_SIZE) {
		size = -EIO;
		goto out;
	}

	/* Read first 10 bytes, including tag, paramsize, and result */
	size = tpm_tis_i2c_recv_data(chip, buf, TPM_HEADER_SIZE);
	if (size < TPM_HEADER_SIZE) {
		error("Unable to read header\n");
		goto out;
	}

	expected = get_unaligned_be32(buf + TPM_RSP_SIZE_BYTE);
	if ((size_t)expected > count) {
		error("Error size=%x, expected=%x, count=%x\n", size, expected,
		      count);
		size = -EIO;
		goto out;
	}

	size += tpm_tis_i2c_recv_data(chip, &buf[TPM_HEADER_SIZE],
				      expected - TPM_HEADER_SIZE);
	if (size < expected) {
		error("Unable to read remainder of result\n");
		size = -ETIME;
		goto out;
	}

	tpm_tis_i2c_wait_for_stat(chip, TPM_STS_VALID, chip->timeout_c,
				  &status);
	if (status & TPM_STS_DATA_AVAIL) {  /* Retry? */
		error("Error left over data\n");
		size = -EIO;
		goto out;
	}

out:
	tpm_tis_i2c_ready(chip);
	/*
	 * The TPM needs some time to clean up here,
	 * so we sleep rather than keeping the bus busy
	 */
	mdelay(2);
	tpm_tis_i2c_release_locality(chip, chip->locality, 0);

	return size;
}

static int tpm_tis_i2c_send(struct tpm_chip *chip, u8 *buf, size_t len)
{
	int rc, status;
	size_t burstcnt;
	size_t count = 0;
	int retry = 0;
	u8 sts = TPM_STS_GO;

	debug("%s: len=%d\n", __func__, len);
	if (len > TPM_DEV_BUFSIZE)
		return -E2BIG;  /* Command is too long for our tpm, sorry */

	if (tpm_tis_i2c_request_locality(chip, 0) < 0)
		return -EBUSY;

	status = tpm_tis_i2c_status(chip);
	if ((status & TPM_STS_COMMAND_READY) == 0) {
		tpm_tis_i2c_ready(chip);
		if (tpm_tis_i2c_wait_for_stat(chip, TPM_STS_COMMAND_READY,
					      chip->timeout_b, &status) < 0) {
			rc = -ETIME;
			goto out_err;
		}
	}

	burstcnt = tpm_tis_i2c_get_burstcount(chip);

	/* burstcount < 0 -> tpm is busy */
	if (burstcnt < 0)
		return burstcnt;

	while (count < len) {
		udelay(300);
		if (burstcnt > len - count)
			burstcnt = len - count;

#ifdef CONFIG_TPM_TIS_I2C_BURST_LIMITATION
		if (retry && burstcnt > CONFIG_TPM_TIS_I2C_BURST_LIMITATION)
			burstcnt = CONFIG_TPM_TIS_I2C_BURST_LIMITATION;
#endif /* CONFIG_TPM_TIS_I2C_BURST_LIMITATION */

		rc = tpm_tis_i2c_write(TPM_DATA_FIFO(chip->locality),
				&(buf[count]), burstcnt);
		if (rc == 0)
			count += burstcnt;
		else {
			debug("%s: error\n", __func__);
			if (retry++ > 10) {
				rc = -EIO;
				goto out_err;
			}
			rc = tpm_tis_i2c_wait_for_stat(chip, TPM_STS_VALID,
						       chip->timeout_c,
						       &status);
			if (rc)
				goto out_err;

			if ((status & TPM_STS_DATA_EXPECT) == 0) {
				rc = -EIO;
				goto out_err;
			}
		}
	}

	/* Go and do it */
	tpm_tis_i2c_write(TPM_STS(chip->locality), &sts, 1);
	debug("done\n");

	return len;

out_err:
	debug("%s: out_err\n", __func__);
	tpm_tis_i2c_ready(chip);
	/*
	 * The TPM needs some time to clean up here,
	 * so we sleep rather than keeping the bus busy
	 */
	mdelay(2);
	tpm_tis_i2c_release_locality(chip, chip->locality, 0);

	return rc;
}

static enum i2c_chip_type tpm_tis_i2c_chip_type(void)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	const void *blob = gd->fdt_blob;

	if (fdtdec_next_compatible(blob, 0, COMPAT_INFINEON_SLB9645_TPM) >= 0)
		return SLB9645;

	if (fdtdec_next_compatible(blob, 0, COMPAT_INFINEON_SLB9635_TPM) >= 0)
		return SLB9635;
#endif
	return UNKNOWN;
}

static int tpm_tis_i2c_init(struct udevice *dev)
{
	struct tpm_chip *chip = &g_chip;
	u32 vendor;
	u32 expected_did_vid;

	g_chip.dev = dev;
	g_chip.chip_type = tpm_tis_i2c_chip_type();
	chip->is_open = 1;

	/* Disable interrupts (not supported) */
	chip->irq = 0;

	/* Default timeouts - these could move to the device tree */
	chip->timeout_a = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_b = TIS_LONG_TIMEOUT_MS;
	chip->timeout_c = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_d = TIS_SHORT_TIMEOUT_MS;
	chip->req_complete_mask = TPM_STS_DATA_AVAIL | TPM_STS_VALID;
	chip->req_complete_val = TPM_STS_DATA_AVAIL | TPM_STS_VALID;
	chip->req_canceled = TPM_STS_COMMAND_READY;

	if (tpm_tis_i2c_request_locality(chip, 0) < 0)
		return  -ENODEV;

	/* Read four bytes from DID_VID register */
	if (tpm_tis_i2c_read(TPM_DID_VID(0), (uchar *)&vendor, 4) < 0) {
		tpm_tis_i2c_release_locality(chip, 0, 1);
		return -EIO;
	}

	if (g_chip.chip_type == SLB9635) {
		vendor = be32_to_cpu(vendor);
		expected_did_vid = TPM_TIS_I2C_DID_VID_9635;
	} else {
		/* device id and byte order has changed for newer i2c tpms */
		expected_did_vid = TPM_TIS_I2C_DID_VID_9645;
	}

	if (g_chip.chip_type != UNKNOWN && vendor != expected_did_vid) {
		error("Vendor id did not match! ID was %08x\n", vendor);
		return -ENODEV;
	}

	debug("1.2 TPM (chip type %s device-id 0x%X)\n",
	      chip_name[g_chip.chip_type], vendor >> 16);

	/*
	 * A timeout query to TPM can be placed here.
	 * Standard timeout values are used so far
	 */

	return 0;
}

/* Returns max number of milliseconds to wait */
static unsigned long tpm_tis_i2c_calc_ordinal_duration(struct tpm_chip *chip,
						       u32 ordinal)
{
	int duration_idx = TPM_UNDEFINED;
	int duration = 0;

	if (ordinal < TPM_MAX_ORDINAL) {
		duration_idx = tpm_ordinal_duration[ordinal];
	} else if ((ordinal & TPM_PROTECTED_ORDINAL_MASK) <
			TPM_MAX_PROTECTED_ORDINAL) {
		duration_idx = tpm_protected_ordinal_duration[
				ordinal & TPM_PROTECTED_ORDINAL_MASK];
	}

	if (duration_idx != TPM_UNDEFINED)
		duration = chip->duration[duration_idx];

	if (duration <= 0)
		return 2 * 60 * HZ; /* Two minutes timeout */
	else
		return duration;
}

static ssize_t tpm_tis_i2c_transmit(const unsigned char *buf, size_t bufsiz)
{
	int rc;
	u32 count, ordinal;
	unsigned long start, stop;

	struct tpm_chip *chip = &g_chip;

	/* switch endianess: big->little */
	count = get_unaligned_be32(buf + TPM_CMD_COUNT_BYTE);
	ordinal = get_unaligned_be32(buf + TPM_CMD_ORDINAL_BYTE);

	if (count == 0) {
		error("no data\n");
		return -ENODATA;
	}
	if (count > bufsiz) {
		error("invalid count value %x %zx\n", count, bufsiz);
		return -E2BIG;
	}

	debug("Calling send\n");
	rc = tpm_tis_i2c_send(chip, (u8 *)buf, count);
	debug("   ... done calling send\n");
	if (rc < 0) {
		error("tpm_transmit: tpm_send: error %d\n", rc);
		goto out;
	}

	if (chip->irq)
		goto out_recv;

	start = get_timer(0);
	stop = tpm_tis_i2c_calc_ordinal_duration(chip, ordinal);
	do {
		debug("waiting for status... %ld %ld\n", start, stop);
		u8 status = tpm_tis_i2c_status(chip);
		if ((status & chip->req_complete_mask) ==
		    chip->req_complete_val) {
			debug("...got it;\n");
			goto out_recv;
		}

		if (status == chip->req_canceled) {
			error("Operation Canceled\n");
			rc = -ECANCELED;
			goto out;
		}
		mdelay(TPM_TIMEOUT_MS);
	} while (get_timer(start) < stop);

	tpm_tis_i2c_ready(chip);
	error("Operation Timed out\n");
	rc = -ETIME;
	goto out;

out_recv:
	debug("out_recv: reading response...\n");
	rc = tpm_tis_i2c_recv(chip, (u8 *)buf, TPM_BUFSIZE);
	if (rc < 0)
		error("tpm_transmit: tpm_recv: error %d\n", rc);

out:
	return rc;
}

/**
 * Decode TPM configuration.
 *
 * @param dev	Returns a configuration of TPM device
 * @return 0 if ok, -1 on error
 */
static int tpm_tis_i2c_decode_config(struct tpm_chip *chip)
{
	const void *blob = gd->fdt_blob;
	struct udevice *bus;
	int chip_addr;
	int parent;
	int node;
	int ret;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INFINEON_SLB9635_TPM);
	if (node < 0) {
		node = fdtdec_next_compatible(blob, 0,
				COMPAT_INFINEON_SLB9645_TPM);
	}
	if (node < 0) {
		debug("%s: Node not found\n", __func__);
		return -1;
	}
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	/*
	 * TODO(sjg@chromium.org): Remove this when driver model supports
	 * TPMs
	 */
	ret = uclass_get_device_by_of_offset(UCLASS_I2C, parent, &bus);
	if (ret) {
		debug("Cannot find bus for node '%s: ret=%d'\n",
		      fdt_get_name(blob, parent, NULL), ret);
		return ret;
	}

	chip_addr = fdtdec_get_int(blob, node, "reg", -1);
	if (chip_addr == -1) {
		debug("Cannot find reg property for node '%s: ret=%d'\n",
		      fdt_get_name(blob, node, NULL), ret);
		return ret;
	}
	/*
	 * TODO(sjg@chromium.org): Older TPMs will need to use the older method
	 * in tpm_tis_i2c_read() so the offset length needs to be 0 here.
	 */
	ret = i2c_get_chip(bus, chip_addr, 1, &chip->dev);
	if (ret) {
		debug("Cannot find device for node '%s: ret=%d'\n",
		      fdt_get_name(blob, node, NULL), ret);
		return ret;
	}

	return 0;
}

int tis_init(void)
{
	if (g_chip.inited)
		return 0;

	if (tpm_tis_i2c_decode_config(&g_chip))
		return -1;

	debug("%s: done\n", __func__);

	g_chip.inited = 1;

	return 0;
}

int tis_open(void)
{
	int rc;

	if (!g_chip.inited)
		return -1;

	debug("%s: start\n", __func__);
	if (g_chip.is_open)
		return -EBUSY;
	rc = tpm_tis_i2c_init(g_chip.dev);
	if (rc < 0)
		g_chip.is_open = 0;

	return rc;
}

int tis_close(void)
{
	if (!g_chip.inited)
		return -1;

	if (g_chip.is_open) {
		tpm_tis_i2c_release_locality(&g_chip, g_chip.locality, 1);
		g_chip.is_open = 0;
	}

	return 0;
}

int tis_sendrecv(const uint8_t *sendbuf, size_t sbuf_size,
		uint8_t *recvbuf, size_t *rbuf_len)
{
	int len;
	uint8_t buf[4096];

	if (!g_chip.inited)
		return -1;

	if (sizeof(buf) < sbuf_size)
		return -1;

	memcpy(buf, sendbuf, sbuf_size);

	len = tpm_tis_i2c_transmit(buf, sbuf_size);

	if (len < 10) {
		*rbuf_len = 0;
		return -1;
	}

	memcpy(recvbuf, buf, len);
	*rbuf_len = len;

	return 0;
}
