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

#include <config.h>
#include <common.h>
#include <compiler.h>
#include <fdtdec.h>
#include <i2c.h>
#include <tpm.h>
#include <asm-generic/errno.h>
#include <linux/types.h>
#include <linux/unaligned/be_byteshift.h>

#include "tpm_private.h"

DECLARE_GLOBAL_DATA_PTR;

/* TPM configuration */
struct tpm {
	int i2c_bus;
	int slave_addr;
	char inited;
	int old_bus;
} tpm;

/* Global structure for tpm chip data */
static struct tpm_chip g_chip;

enum tpm_duration {
	TPM_SHORT = 0,
	TPM_MEDIUM = 1,
	TPM_LONG = 2,
	TPM_UNDEFINED,
};

/* Extended error numbers from linux (see errno.h) */
#define ECANCELED	125	/* Operation Canceled */

/* Timer frequency. Corresponds to msec timer resolution*/
#define HZ		1000

#define TPM_MAX_ORDINAL			243
#define TPM_MAX_PROTECTED_ORDINAL	12
#define TPM_PROTECTED_ORDINAL_MASK	0xFF

#define TPM_CMD_COUNT_BYTE	2
#define TPM_CMD_ORDINAL_BYTE	6

/*
 * Array with one entry per ordinal defining the maximum amount
 * of time the chip could take to return the result.  The ordinal
 * designation of short, medium or long is defined in a table in
 * TCG Specification TPM Main Part 2 TPM Structures Section 17. The
 * values of the SHORT, MEDIUM, and LONG durations are retrieved
 * from the chip during initialization with a call to tpm_get_timeouts.
 */
static const u8 tpm_protected_ordinal_duration[TPM_MAX_PROTECTED_ORDINAL] = {
	TPM_UNDEFINED,		/* 0 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 5 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 10 */
	TPM_SHORT,
};

static const u8 tpm_ordinal_duration[TPM_MAX_ORDINAL] = {
	TPM_UNDEFINED,		/* 0 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 5 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 10 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_LONG,
	TPM_LONG,
	TPM_MEDIUM,		/* 15 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_LONG,
	TPM_SHORT,		/* 20 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_SHORT,		/* 25 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,		/* 30 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 35 */
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 40 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 45 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_LONG,
	TPM_MEDIUM,		/* 50 */
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 55 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 60 */
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,		/* 65 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 70 */
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 75 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_LONG,		/* 80 */
	TPM_UNDEFINED,
	TPM_MEDIUM,
	TPM_LONG,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 85 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 90 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 95 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 100 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 105 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 110 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 115 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_LONG,		/* 120 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_SHORT,		/* 125 */
	TPM_SHORT,
	TPM_LONG,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 130 */
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_UNDEFINED,		/* 135 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 140 */
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 145 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 150 */
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 155 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 160 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 165 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_LONG,		/* 170 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 175 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 180 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,		/* 185 */
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 190 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 195 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 200 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_SHORT,		/* 205 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,		/* 210 */
	TPM_UNDEFINED,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_UNDEFINED,		/* 215 */
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_SHORT,		/* 220 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 225 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 230 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 235 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 240 */
	TPM_UNDEFINED,
	TPM_MEDIUM,
};

/* Returns max number of milliseconds to wait */
static unsigned long tpm_calc_ordinal_duration(struct tpm_chip *chip,
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
		duration = chip->vendor.duration[duration_idx];

	if (duration <= 0)
		return 2 * 60 * HZ; /* Two minutes timeout */
	else
		return duration;
}

static ssize_t tpm_transmit(const unsigned char *buf, size_t bufsiz)
{
	ssize_t rc;
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

	rc = chip->vendor.send(chip, (u8 *)buf, count);
	if (rc < 0) {
		error("tpm_transmit: tpm_send: error %zd\n", rc);
		goto out;
	}

	if (chip->vendor.irq)
		goto out_recv;

	start = get_timer(0);
	stop = tpm_calc_ordinal_duration(chip, ordinal);
	do {
		debug("waiting for status...\n");
		u8 status = chip->vendor.status(chip);
		if ((status & chip->vendor.req_complete_mask) ==
		    chip->vendor.req_complete_val) {
			debug("...got it;\n");
			goto out_recv;
		}

		if ((status == chip->vendor.req_canceled)) {
			error("Operation Canceled\n");
			rc = -ECANCELED;
			goto out;
		}
		udelay(TPM_TIMEOUT * 1000);
	} while (get_timer(start) < stop);

	chip->vendor.cancel(chip);
	error("Operation Timed out\n");
	rc = -ETIME;
	goto out;

out_recv:
	debug("out_recv: reading response...\n");
	rc = chip->vendor.recv(chip, (u8 *)buf, TPM_BUFSIZE);
	if (rc < 0)
		error("tpm_transmit: tpm_recv: error %zd\n", rc);

out:
	return rc;
}

static int tpm_open(uint32_t dev_addr)
{
	int rc;
	if (g_chip.is_open)
		return -EBUSY;
	rc = tpm_vendor_init(dev_addr);
	if (rc < 0)
		g_chip.is_open = 0;
	return rc;
}

static void tpm_close(void)
{
	if (g_chip.is_open) {
		tpm_vendor_cleanup(&g_chip);
		g_chip.is_open = 0;
	}
}

static int tpm_select(void)
{
	int ret;

	tpm.old_bus = i2c_get_bus_num();
	if (tpm.old_bus != tpm.i2c_bus) {
		ret = i2c_set_bus_num(tpm.i2c_bus);
		if (ret) {
			debug("%s: Fail to set i2c bus %d\n", __func__,
			      tpm.i2c_bus);
			return -1;
		}
	}
	return 0;
}

static int tpm_deselect(void)
{
	int ret;

	if (tpm.old_bus != i2c_get_bus_num()) {
		ret = i2c_set_bus_num(tpm.old_bus);
		if (ret) {
			debug("%s: Fail to restore i2c bus %d\n",
			      __func__, tpm.old_bus);
			return -1;
		}
	}
	tpm.old_bus = -1;
	return 0;
}

/**
 * Decode TPM configuration.
 *
 * @param dev	Returns a configuration of TPM device
 * @return 0 if ok, -1 on error
 */
static int tpm_decode_config(struct tpm *dev)
{
#ifdef CONFIG_OF_CONTROL
	const void *blob = gd->fdt_blob;
	int node, parent;
	int i2c_bus;

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
	i2c_bus = i2c_get_bus_num_fdt(parent);
	if (i2c_bus < 0)
		return -1;
	dev->i2c_bus = i2c_bus;
	dev->slave_addr = fdtdec_get_addr(blob, node, "reg");
#else
	dev->i2c_bus = CONFIG_TPM_TIS_I2C_BUS_NUMBER;
	dev->slave_addr = CONFIG_TPM_TIS_I2C_SLAVE_ADDRESS;
#endif
	return 0;
}

struct tpm_chip *tpm_register_hardware(const struct tpm_vendor_specific *entry)
{
	struct tpm_chip *chip;

	/* Driver specific per-device data */
	chip = &g_chip;
	memcpy(&chip->vendor, entry, sizeof(struct tpm_vendor_specific));
	chip->is_open = 1;

	return chip;
}

int tis_init(void)
{
	if (tpm.inited)
		return 0;

	if (tpm_decode_config(&tpm))
		return -1;

	if (tpm_select())
		return -1;

	/*
	 * Probe TPM twice; the first probing might fail because TPM is asleep,
	 * and the probing can wake up TPM.
	 */
	if (i2c_probe(tpm.slave_addr) && i2c_probe(tpm.slave_addr)) {
		debug("%s: fail to probe i2c addr 0x%x\n", __func__,
		      tpm.slave_addr);
		return -1;
	}

	tpm_deselect();

	tpm.inited = 1;

	return 0;
}

int tis_open(void)
{
	int rc;

	if (!tpm.inited)
		return -1;

	if (tpm_select())
		return -1;

	rc = tpm_open(tpm.slave_addr);

	tpm_deselect();

	return rc;
}

int tis_close(void)
{
	if (!tpm.inited)
		return -1;

	if (tpm_select())
		return -1;

	tpm_close();

	tpm_deselect();

	return 0;
}

int tis_sendrecv(const uint8_t *sendbuf, size_t sbuf_size,
		uint8_t *recvbuf, size_t *rbuf_len)
{
	int len;
	uint8_t buf[4096];

	if (!tpm.inited)
		return -1;

	if (sizeof(buf) < sbuf_size)
		return -1;

	memcpy(buf, sendbuf, sbuf_size);

	if (tpm_select())
		return -1;

	len = tpm_transmit(buf, sbuf_size);

	tpm_deselect();

	if (len < 10) {
		*rbuf_len = 0;
		return -1;
	}

	memcpy(recvbuf, buf, len);
	*rbuf_len = len;

	return 0;
}
