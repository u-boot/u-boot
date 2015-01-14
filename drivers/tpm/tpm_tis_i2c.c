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
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <fdtdec.h>
#include <linux/compiler.h>
#include <i2c.h>
#include <tpm.h>
#include <asm-generic/errno.h>
#include <linux/types.h>
#include <linux/unaligned/be_byteshift.h>

#include "tpm_private.h"

DECLARE_GLOBAL_DATA_PTR;

/* Address of the TPM on the I2C bus */
#define TPM_I2C_ADDR		0x20

/* Max buffer size supported by our tpm */
#define TPM_DEV_BUFSIZE		1260

/* Max number of iterations after i2c NAK */
#define MAX_COUNT		3

/*
 * Max number of iterations after i2c NAK for 'long' commands
 *
 * We need this especially for sending TPM_READY, since the cleanup after the
 * transtion to the ready state may take some time, but it is unpredictable
 * how long it will take.
 */
#define MAX_COUNT_LONG		50

#define SLEEP_DURATION		60	/* in usec */
#define SLEEP_DURATION_LONG	210	/* in usec */

#define TPM_HEADER_SIZE		10

/*
 * Expected value for DIDVID register
 *
 * The only device the system knows about at this moment is Infineon slb9635.
 */
#define TPM_TIS_I2C_DID_VID	0x000b15d1L

enum tis_access {
	TPM_ACCESS_VALID		= 0x80,
	TPM_ACCESS_ACTIVE_LOCALITY	= 0x20,
	TPM_ACCESS_REQUEST_PENDING	= 0x04,
	TPM_ACCESS_REQUEST_USE		= 0x02,
};

enum tis_status {
	TPM_STS_VALID			= 0x80,
	TPM_STS_COMMAND_READY		= 0x40,
	TPM_STS_GO			= 0x20,
	TPM_STS_DATA_AVAIL		= 0x10,
	TPM_STS_DATA_EXPECT		= 0x08,
};

enum tis_defaults {
	TIS_SHORT_TIMEOUT		= 750,	/* ms */
	TIS_LONG_TIMEOUT		= 2000,	/* ms */
};

/* expected value for DIDVID register */
#define TPM_TIS_I2C_DID_VID_9635 0x000b15d1L
#define TPM_TIS_I2C_DID_VID_9645 0x001a15d1L

enum i2c_chip_type {
	SLB9635,
	SLB9645,
	UNKNOWN,
};

static const char * const chip_name[] = {
	[SLB9635] = "slb9635tt",
	[SLB9645] = "slb9645tt",
	[UNKNOWN] = "unknown/fallback to slb9635",
};

#define	TPM_ACCESS(l)			(0x0000 | ((l) << 4))
#define	TPM_STS(l)			(0x0001 | ((l) << 4))
#define	TPM_DATA_FIFO(l)		(0x0005 | ((l) << 4))
#define	TPM_DID_VID(l)			(0x0006 | ((l) << 4))

/* Structure to store I2C TPM specific stuff */
struct tpm_dev {
	uint addr;
	u8 buf[TPM_DEV_BUFSIZE + sizeof(u8)];  /* Max buffer size + addr */
	enum i2c_chip_type chip_type;
};

static struct tpm_dev tpm_dev = {
	.addr = TPM_I2C_ADDR
};

static struct tpm_dev tpm_dev;

/*
 * iic_tpm_read() - read from TPM register
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
static int iic_tpm_read(u8 addr, u8 *buffer, size_t len)
{
	int rc;
	int count;
	uint32_t addrbuf = addr;

	if ((tpm_dev.chip_type == SLB9635) || (tpm_dev.chip_type == UNKNOWN)) {
		/* slb9635 protocol should work in both cases */
		for (count = 0; count < MAX_COUNT; count++) {
			rc = i2c_write(tpm_dev.addr, 0, 0,
				       (uchar *)&addrbuf, 1);
			if (rc == 0)
				break;  /* Success, break to skip sleep */
			udelay(SLEEP_DURATION);
		}
		if (rc)
			return -rc;

		/* After the TPM has successfully received the register address
		 * it needs some time, thus we're sleeping here again, before
		 * retrieving the data
		 */
		for (count = 0; count < MAX_COUNT; count++) {
			udelay(SLEEP_DURATION);
			rc = i2c_read(tpm_dev.addr, 0, 0, buffer, len);
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
			rc = i2c_read(tpm_dev.addr, addr, 1, buffer, len);
			if (rc == 0)
				break;  /* break here to skip sleep */
			udelay(SLEEP_DURATION);
		}
	}

	/* Take care of 'guard time' */
	udelay(SLEEP_DURATION);
	if (rc)
		return -rc;

	return 0;
}

static int iic_tpm_write_generic(u8 addr, u8 *buffer, size_t len,
		unsigned int sleep_time, u8 max_count)
{
	int rc = 0;
	int count;

	/* Prepare send buffer */
	tpm_dev.buf[0] = addr;
	memcpy(&(tpm_dev.buf[1]), buffer, len);

	for (count = 0; count < max_count; count++) {
		rc = i2c_write(tpm_dev.addr, 0, 0, tpm_dev.buf, len + 1);
		if (rc == 0)
			break;  /* Success, break to skip sleep */
		udelay(sleep_time);
	}

	/* take care of 'guard time' */
	udelay(SLEEP_DURATION);
	if (rc)
		return -rc;

	return 0;
}

/*
 * iic_tpm_write() - write to TPM register
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
 * NOTE: use this function instead of the iic_tpm_write_generic function.
 *
 * Return -EIO on error, 0 on success
 */
static int iic_tpm_write(u8 addr, u8 *buffer, size_t len)
{
	return iic_tpm_write_generic(addr, buffer, len, SLEEP_DURATION,
			MAX_COUNT);
}

/*
 * This function is needed especially for the cleanup situation after
 * sending TPM_READY
 */
static int iic_tpm_write_long(u8 addr, u8 *buffer, size_t len)
{
	return iic_tpm_write_generic(addr, buffer, len, SLEEP_DURATION_LONG,
			MAX_COUNT_LONG);
}

static int check_locality(struct tpm_chip *chip, int loc)
{
	const u8 mask = TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID;
	u8 buf;
	int rc;

	rc = iic_tpm_read(TPM_ACCESS(loc), &buf, 1);
	if (rc < 0)
		return rc;

	if ((buf & mask) == mask) {
		chip->vendor.locality = loc;
		return loc;
	}

	return -1;
}

static void release_locality(struct tpm_chip *chip, int loc, int force)
{
	const u8 mask = TPM_ACCESS_REQUEST_PENDING | TPM_ACCESS_VALID;
	u8 buf;

	if (iic_tpm_read(TPM_ACCESS(loc), &buf, 1) < 0)
		return;

	if (force || (buf & mask) == mask) {
		buf = TPM_ACCESS_ACTIVE_LOCALITY;
		iic_tpm_write(TPM_ACCESS(loc), &buf, 1);
	}
}

static int request_locality(struct tpm_chip *chip, int loc)
{
	unsigned long start, stop;
	u8 buf = TPM_ACCESS_REQUEST_USE;

	if (check_locality(chip, loc) >= 0)
		return loc;  /* We already have the locality */

	iic_tpm_write(TPM_ACCESS(loc), &buf, 1);

	/* Wait for burstcount */
	start = get_timer(0);
	stop = chip->vendor.timeout_a;
	do {
		if (check_locality(chip, loc) >= 0)
			return loc;
		udelay(TPM_TIMEOUT * 1000);
	} while (get_timer(start) < stop);

	return -1;
}

static u8 tpm_tis_i2c_status(struct tpm_chip *chip)
{
	/* NOTE: Since i2c read may fail, return 0 in this case --> time-out */
	u8 buf;

	if (iic_tpm_read(TPM_STS(chip->vendor.locality), &buf, 1) < 0)
		return 0;
	else
		return buf;
}

static void tpm_tis_i2c_ready(struct tpm_chip *chip)
{
	/* This causes the current command to be aborted */
	u8 buf = TPM_STS_COMMAND_READY;

	iic_tpm_write_long(TPM_STS(chip->vendor.locality), &buf, 1);
}

static ssize_t get_burstcount(struct tpm_chip *chip)
{
	unsigned long start, stop;
	ssize_t burstcnt;
	u8 addr, buf[3];

	/* Wait for burstcount */
	/* XXX: Which timeout value? Spec has 2 answers (c & d) */
	start = get_timer(0);
	stop = chip->vendor.timeout_d;
	do {
		/* Note: STS is little endian */
		addr = TPM_STS(chip->vendor.locality) + 1;
		if (iic_tpm_read(addr, buf, 3) < 0)
			burstcnt = 0;
		else
			burstcnt = (buf[2] << 16) + (buf[1] << 8) + buf[0];

		if (burstcnt)
			return burstcnt;
		udelay(TPM_TIMEOUT * 1000);
	} while (get_timer(start) < stop);

	return -EBUSY;
}

static int wait_for_stat(struct tpm_chip *chip, u8 mask, unsigned long timeout,
		int *status)
{
	unsigned long start, stop;

	/* Check current status */
	*status = tpm_tis_i2c_status(chip);
	if ((*status & mask) == mask)
		return 0;

	start = get_timer(0);
	stop = timeout;
	do {
		udelay(TPM_TIMEOUT * 1000);
		*status = tpm_tis_i2c_status(chip);
		if ((*status & mask) == mask)
			return 0;
	} while (get_timer(start) < stop);

	return -ETIME;
}

static int recv_data(struct tpm_chip *chip, u8 *buf, size_t count)
{
	size_t size = 0;
	ssize_t burstcnt;
	int rc;

	while (size < count) {
		burstcnt = get_burstcount(chip);

		/* burstcount < 0 -> tpm is busy */
		if (burstcnt < 0)
			return burstcnt;

		/* Limit received data to max left */
		if (burstcnt > (count - size))
			burstcnt = count - size;

		rc = iic_tpm_read(TPM_DATA_FIFO(chip->vendor.locality),
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
	size = recv_data(chip, buf, TPM_HEADER_SIZE);
	if (size < TPM_HEADER_SIZE) {
		error("Unable to read header\n");
		goto out;
	}

	expected = get_unaligned_be32(buf + TPM_RSP_SIZE_BYTE);
	if ((size_t)expected > count) {
		size = -EIO;
		goto out;
	}

	size += recv_data(chip, &buf[TPM_HEADER_SIZE],
			expected - TPM_HEADER_SIZE);
	if (size < expected) {
		error("Unable to read remainder of result\n");
		size = -ETIME;
		goto out;
	}

	wait_for_stat(chip, TPM_STS_VALID, chip->vendor.timeout_c, &status);
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
	udelay(2000);
	release_locality(chip, chip->vendor.locality, 0);

	return size;
}

static int tpm_tis_i2c_send(struct tpm_chip *chip, u8 *buf, size_t len)
{
	int rc, status;
	ssize_t burstcnt;
	size_t count = 0;
	int retry = 0;
	u8 sts = TPM_STS_GO;

	if (len > TPM_DEV_BUFSIZE)
		return -E2BIG;  /* Command is too long for our tpm, sorry */

	if (request_locality(chip, 0) < 0)
		return -EBUSY;

	status = tpm_tis_i2c_status(chip);
	if ((status & TPM_STS_COMMAND_READY) == 0) {
		tpm_tis_i2c_ready(chip);
		if (wait_for_stat(chip, TPM_STS_COMMAND_READY,
				  chip->vendor.timeout_b, &status) < 0) {
			rc = -ETIME;
			goto out_err;
		}
	}

	burstcnt = get_burstcount(chip);

	/* burstcount < 0 -> tpm is busy */
	if (burstcnt < 0)
		return burstcnt;

	while (count < len - 1) {
		if (burstcnt > len - 1 - count)
			burstcnt = len - 1 - count;

#ifdef CONFIG_TPM_TIS_I2C_BURST_LIMITATION
		if (retry && burstcnt > CONFIG_TPM_TIS_I2C_BURST_LIMITATION)
			burstcnt = CONFIG_TPM_TIS_I2C_BURST_LIMITATION;
#endif /* CONFIG_TPM_TIS_I2C_BURST_LIMITATION */

		rc = iic_tpm_write(TPM_DATA_FIFO(chip->vendor.locality),
				&(buf[count]), burstcnt);
		if (rc == 0)
			count += burstcnt;
		else {
			retry++;
			wait_for_stat(chip, TPM_STS_VALID,
				      chip->vendor.timeout_c, &status);

			if ((status & TPM_STS_DATA_EXPECT) == 0) {
				rc = -EIO;
				goto out_err;
			}
		}
	}

	/* Write last byte */
	iic_tpm_write(TPM_DATA_FIFO(chip->vendor.locality), &(buf[count]), 1);
	wait_for_stat(chip, TPM_STS_VALID, chip->vendor.timeout_c, &status);
	if ((status & TPM_STS_DATA_EXPECT) != 0) {
		rc = -EIO;
		goto out_err;
	}

	/* Go and do it */
	iic_tpm_write(TPM_STS(chip->vendor.locality), &sts, 1);

	return len;

out_err:
	tpm_tis_i2c_ready(chip);
	/*
	 * The TPM needs some time to clean up here,
	 * so we sleep rather than keeping the bus busy
	 */
	udelay(2000);
	release_locality(chip, chip->vendor.locality, 0);

	return rc;
}

static struct tpm_vendor_specific tpm_tis_i2c = {
	.status = tpm_tis_i2c_status,
	.recv = tpm_tis_i2c_recv,
	.send = tpm_tis_i2c_send,
	.cancel = tpm_tis_i2c_ready,
	.req_complete_mask = TPM_STS_DATA_AVAIL | TPM_STS_VALID,
	.req_complete_val = TPM_STS_DATA_AVAIL | TPM_STS_VALID,
	.req_canceled = TPM_STS_COMMAND_READY,
};


static enum i2c_chip_type tpm_vendor_chip_type(void)
{
#ifdef CONFIG_OF_CONTROL
	const void *blob = gd->fdt_blob;

	if (fdtdec_next_compatible(blob, 0, COMPAT_INFINEON_SLB9645_TPM) >= 0)
		return SLB9645;

	if (fdtdec_next_compatible(blob, 0, COMPAT_INFINEON_SLB9635_TPM) >= 0)
		return SLB9635;
#endif
	return UNKNOWN;
}

/* Initialisation of i2c tpm */
int tpm_vendor_init(uint32_t dev_addr)
{
	u32 vendor;
	u32 expected_did_vid;
	uint old_addr;
	int rc = 0;
	struct tpm_chip *chip;

	old_addr = tpm_dev.addr;
	if (dev_addr != 0)
		tpm_dev.addr = dev_addr;

	tpm_dev.chip_type = tpm_vendor_chip_type();

	chip = tpm_register_hardware(&tpm_tis_i2c);
	if (chip < 0) {
		rc = -ENODEV;
		goto out_err;
	}

	/* Disable interrupts (not supported) */
	chip->vendor.irq = 0;

	/* Default timeouts */
	chip->vendor.timeout_a = TIS_SHORT_TIMEOUT;
	chip->vendor.timeout_b = TIS_LONG_TIMEOUT;
	chip->vendor.timeout_c = TIS_SHORT_TIMEOUT;
	chip->vendor.timeout_d = TIS_SHORT_TIMEOUT;

	if (request_locality(chip, 0) < 0) {
		rc = -ENODEV;
		goto out_err;
	}

	/* Read four bytes from DID_VID register */
	if (iic_tpm_read(TPM_DID_VID(0), (uchar *)&vendor, 4) < 0) {
		rc = -EIO;
		goto out_release;
	}

	if (tpm_dev.chip_type == SLB9635) {
		vendor = be32_to_cpu(vendor);
		expected_did_vid = TPM_TIS_I2C_DID_VID_9635;
	} else {
		/* device id and byte order has changed for newer i2c tpms */
		expected_did_vid = TPM_TIS_I2C_DID_VID_9645;
	}

	if (tpm_dev.chip_type != UNKNOWN && vendor != expected_did_vid) {
		error("Vendor id did not match! ID was %08x\n", vendor);
		rc = -ENODEV;
		goto out_release;
	}

	debug("1.2 TPM (chip type %s device-id 0x%X)\n",
	      chip_name[tpm_dev.chip_type], vendor >> 16);

	/*
	 * A timeout query to TPM can be placed here.
	 * Standard timeout values are used so far
	 */

	return 0;

out_release:
	release_locality(chip, 0, 1);

out_err:
	tpm_dev.addr = old_addr;
	return rc;
}

void tpm_vendor_cleanup(struct tpm_chip *chip)
{
	release_locality(chip, chip->vendor.locality, 1);
}
