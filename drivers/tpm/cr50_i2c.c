// SPDX-License-Identifier: GPL-2.0
/*
 * Cr50 / H1 TPM support
 *
 * Copyright 2018 Google LLC
 */

#define LOG_CATEGORY UCLASS_TPM

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <irq.h>
#include <log.h>
#include <spl.h>
#include <tpm-v2.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/iomap.h>
#include <asm/arch/pm.h>
#include <linux/delay.h>

enum {
	TIMEOUT_INIT_MS		= 30000, /* Very long timeout for TPM init */
	TIMEOUT_LONG_US		= 2 * 1000 * 1000,
	TIMEOUT_SHORT_US	= 2 * 1000,
	TIMEOUT_NO_IRQ_US	= 20 * 1000,
	TIMEOUT_IRQ_US		= 100 * 1000,
};

enum {
	CR50_DID_VID = 0x00281ae0L
};

enum {
	CR50_MAX_BUF_SIZE = 63,
};

/**
 * struct cr50_priv - Private driver data
 *
 * @ready_gpio: GPIO to use to check if the TPM is ready
 * @irq: IRQ to use check if the TPM is ready (has priority over @ready_gpio)
 * @locality: Currenttly claimed locality (-1 if none)
 * @vendor: vendor: Vendor ID for TPM
 * @use_irq: true to use @irq, false to use @ready if available
 */
struct cr50_priv {
	struct gpio_desc ready_gpio;
	struct irq irq;
	int locality;
	uint vendor;
	bool use_irq;
};

/* Wait for interrupt to indicate TPM is ready */
static int cr50_i2c_wait_tpm_ready(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	ulong timeout, base;
	int i;

	if (!priv->use_irq && !dm_gpio_is_valid(&priv->ready_gpio)) {
		/* Fixed delay if interrupt not supported */
		udelay(TIMEOUT_NO_IRQ_US);
		return 0;
	}

	base = timer_get_us();
	timeout = base + TIMEOUT_IRQ_US;

	i = 0;
	while (priv->use_irq ? !irq_read_and_clear(&priv->irq) :
	       !dm_gpio_get_value(&priv->ready_gpio)) {
		i++;
		if ((int)(timer_get_us() - timeout) >= 0) {
			log_warning("Timeout\n");
			/* Use this instead of the -ETIMEDOUT used by i2c */
			return -ETIME;
		}
	}
	log_debug("i=%d\n", i);

	return 0;
}

/* Clear pending interrupts */
static void cr50_i2c_clear_tpm_irq(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);

	if (priv->use_irq)
		irq_read_and_clear(&priv->irq);
}

/*
 * cr50_i2c_read() - read from TPM register
 *
 * @dev: TPM chip information
 * @addr: register address to read from
 * @buffer: provided by caller
 * @len: number of bytes to read
 *
 * 1) send register address byte 'addr' to the TPM
 * 2) wait for TPM to indicate it is ready
 * 3) read 'len' bytes of TPM response into the provided 'buffer'
 *
 * Return 0 on success. -ve on error
 */
static int cr50_i2c_read(struct udevice *dev, u8 addr, u8 *buffer,
			 size_t len)
{
	int ret;

	/* Clear interrupt before starting transaction */
	cr50_i2c_clear_tpm_irq(dev);

	/* Send the register address byte to the TPM */
	ret = dm_i2c_write(dev, 0, &addr, 1);
	if (ret) {
		log_err("Address write failed (err=%d)\n", ret);
		return ret;
	}

	/* Wait for TPM to be ready with response data */
	ret = cr50_i2c_wait_tpm_ready(dev);
	if (ret)
		return ret;

	/* Read response data frrom the TPM */
	ret = dm_i2c_read(dev, 0, buffer, len);
	if (ret) {
		log_err("Read response failed (err=%d)\n", ret);
		return ret;
	}

	return 0;
}

/*
 * cr50_i2c_write() - write to TPM register
 *
 * @dev: TPM chip information
 * @addr: register address to write to
 * @buffer: data to write
 * @len: number of bytes to write
 *
 * 1) prepend the provided address to the provided data
 * 2) send the address+data to the TPM
 * 3) wait for TPM to indicate it is done writing
 *
 * Returns -1 on error, 0 on success.
 */
static int cr50_i2c_write(struct udevice *dev, u8 addr, const u8 *buffer,
			  size_t len)
{
	u8 buf[len + 1];
	int ret;

	if (len > CR50_MAX_BUF_SIZE) {
		log_err("Length %zd is too large\n", len);
		return -E2BIG;
	}

	/* Prepend the 'register address' to the buffer */
	buf[0] = addr;
	memcpy(buf + 1, buffer, len);

	/* Clear interrupt before starting transaction */
	cr50_i2c_clear_tpm_irq(dev);

	/* Send write request buffer with address */
	ret = dm_i2c_write(dev, 0, buf, len + 1);
	if (ret) {
		log_err("Error writing to TPM (err=%d)\n", ret);
		return ret;
	}

	/* Wait for TPM to be ready */
	return cr50_i2c_wait_tpm_ready(dev);
}

static inline u8 tpm_access(u8 locality)
{
	return 0x0 | (locality << 4);
}

static inline u8 tpm_sts(u8 locality)
{
	return 0x1 | (locality << 4);
}

static inline u8 tpm_data_fifo(u8 locality)
{
	return 0x5 | (locality << 4);
}

static inline u8 tpm_did_vid(u8 locality)
{
	return 0x6 | (locality << 4);
}

static int release_locality(struct udevice *dev, int force)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 mask = TPM_ACCESS_VALID | TPM_ACCESS_REQUEST_PENDING;
	u8 addr = tpm_access(priv->locality);
	int ret;
	u8 buf;

	ret = cr50_i2c_read(dev, addr, &buf, 1);
	if (ret)
		return ret;

	if (force || (buf & mask) == mask) {
		buf = TPM_ACCESS_ACTIVE_LOCALITY;
		cr50_i2c_write(dev, addr, &buf, 1);
	}

	priv->locality = -1;

	return 0;
}

/* cr50 requires all 4 bytes of status register to be read */
static int cr50_i2c_status(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 buf[4];
	int ret;

	ret = cr50_i2c_read(dev, tpm_sts(priv->locality), buf, sizeof(buf));
	if (ret) {
		log_warning("%s: Failed to read status\n", __func__);
		return ret;
	}

	return buf[0];
}

/* cr50 requires all 4 bytes of status register to be written */
static int cr50_i2c_ready(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 buf[4] = { TPM_STS_COMMAND_READY };
	int ret;

	ret = cr50_i2c_write(dev, tpm_sts(priv->locality), buf, sizeof(buf));
	if (ret)
		return ret;

	udelay(TIMEOUT_SHORT_US);

	return 0;
}

static int cr50_i2c_wait_burststs(struct udevice *dev, u8 mask,
				  size_t *burst, int *status)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	ulong timeout;
	u32 buf;

	/*
	 * cr50 uses bytes 3:2 of status register for burst count and all 4
	 * bytes must be read
	 */
	timeout = timer_get_us() + TIMEOUT_LONG_US;
	while (timer_get_us() < timeout) {
		if (cr50_i2c_read(dev, tpm_sts(priv->locality),
				  (u8 *)&buf, sizeof(buf)) < 0) {
			udelay(TIMEOUT_SHORT_US);
			continue;
		}

		*status = buf & 0xff;
		*burst = le16_to_cpu((buf >> 8) & 0xffff);

		if ((*status & mask) == mask &&
		    *burst > 0 && *burst <= CR50_MAX_BUF_SIZE)
			return 0;

		udelay(TIMEOUT_SHORT_US);
	}

	log_warning("Timeout reading burst and status\n");

	return -ETIMEDOUT;
}

static int cr50_i2c_recv(struct udevice *dev, u8 *buf, size_t buf_len)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	size_t burstcnt, expected, current, len;
	u8 addr = tpm_data_fifo(priv->locality);
	u8 mask = TPM_STS_VALID | TPM_STS_DATA_AVAIL;
	u32 expected_buf;
	int status;
	int ret;

	log_debug("%s: len=%x\n", __func__, buf_len);
	if (buf_len < TPM_HEADER_SIZE)
		return -E2BIG;

	ret = cr50_i2c_wait_burststs(dev, mask, &burstcnt, &status);
	if (ret < 0) {
		log_warning("First chunk not available\n");
		goto out_err;
	}

	/* Read first chunk of burstcnt bytes */
	if (cr50_i2c_read(dev, addr, buf, burstcnt) < 0) {
		log_warning("Read failed\n");
		goto out_err;
	}

	/* Determine expected data in the return buffer */
	memcpy(&expected_buf, buf + TPM_CMD_COUNT_OFFSET, sizeof(expected_buf));
	expected = be32_to_cpu(expected_buf);
	if (expected > buf_len) {
		log_warning("Too much data: %zu > %zu\n", expected, buf_len);
		goto out_err;
	}

	/* Now read the rest of the data */
	current = burstcnt;
	while (current < expected) {
		/* Read updated burst count and check status */
		if (cr50_i2c_wait_burststs(dev, mask, &burstcnt, &status) < 0) {
			log_warning("- burst failure1\n");
			goto out_err;
			}

		len = min(burstcnt, expected - current);
		if (cr50_i2c_read(dev, addr, buf + current, len) != 0) {
			log_warning("Read failed\n");
			goto out_err;
		}

		current += len;
	}

	if (cr50_i2c_wait_burststs(dev, TPM_STS_VALID, &burstcnt,
				   &status) < 0) {
		log_warning("- burst failure2\n");
		goto out_err;
	}
	if (status & TPM_STS_DATA_AVAIL) {
		log_warning("Data still available\n");
		goto out_err;
	}

	return current;

out_err:
	/* Abort current transaction if still pending */
	ret = cr50_i2c_status(dev);
	if (ret < 0)
		return ret;
	if (ret & TPM_STS_COMMAND_READY) {
		ret = cr50_i2c_ready(dev);
		if (ret)
			return ret;
	}

	return -EIO;
}

static int cr50_i2c_send(struct udevice *dev, const u8 *buf, size_t len)
{
	struct cr50_priv *priv = dev_get_priv(dev);

	int status;
	size_t burstcnt, limit, sent = 0;
	u8 tpm_go[4] = { TPM_STS_GO };
	ulong timeout;
	int ret;

	log_debug("%s: len=%x\n", __func__, len);
	timeout = timer_get_us() + TIMEOUT_LONG_US;
	do {
		ret = cr50_i2c_status(dev);
		if (ret < 0)
			goto out_err;
		if (ret & TPM_STS_COMMAND_READY)
			break;

		if (timer_get_us() > timeout)
			goto out_err;

		ret = cr50_i2c_ready(dev);
		if (ret)
			goto out_err;
	} while (1);

	while (len > 0) {
		u8 mask = TPM_STS_VALID;

		/* Wait for data if this is not the first chunk */
		if (sent > 0)
			mask |= TPM_STS_DATA_EXPECT;

		if (cr50_i2c_wait_burststs(dev, mask, &burstcnt, &status) < 0)
			goto out_err;

		/*
		 * Use burstcnt - 1 to account for the address byte
		 * that is inserted by cr50_i2c_write()
		 */
		limit = min(burstcnt - 1, len);
		if (cr50_i2c_write(dev, tpm_data_fifo(priv->locality),
				   &buf[sent], limit) != 0) {
			log_warning("Write failed\n");
			goto out_err;
		}

		sent += limit;
		len -= limit;
	}

	/* Ensure TPM is not expecting more data */
	if (cr50_i2c_wait_burststs(dev, TPM_STS_VALID, &burstcnt, &status) < 0)
		goto out_err;
	if (status & TPM_STS_DATA_EXPECT) {
		log_warning("Data still expected\n");
		goto out_err;
	}

	/* Start the TPM command */
	ret = cr50_i2c_write(dev, tpm_sts(priv->locality), tpm_go,
			     sizeof(tpm_go));
	if (ret) {
		log_warning("Start command failed\n");
		goto out_err;
	}

	return sent;

out_err:
	/* Abort current transaction if still pending */
	ret = cr50_i2c_status(dev);

	if (ret < 0 || (ret & TPM_STS_COMMAND_READY)) {
		ret = cr50_i2c_ready(dev);
		if (ret)
			return ret;
	}

	return -EIO;
}

/**
 * process_reset() - Wait for the Cr50 to reset
 *
 * Cr50 processes reset requests asynchronously and conceivably could be busy
 * executing a long command and not reacting to the reset pulse for a while.
 *
 * This function will make sure that the AP does not proceed with boot until
 * TPM finished reset processing.
 *
 * @dev: Cr50 device
 * @return 0 if OK, -EPERM if locality could not be taken
 */
static int process_reset(struct udevice *dev)
{
	const int loc = 0;
	u8 access;
	ulong start;

	/*
	 * Locality is released by TPM reset.
	 *
	 * If locality is taken at this point, this could be due to the fact
	 * that the TPM is performing a long operation and has not processed
	 * reset request yet. We'll wait up to CR50_TIMEOUT_INIT_MS and see if
	 * it releases locality when reset is processed.
	 */
	start = get_timer(0);
	do {
		const u8 mask = TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY;
		int ret;

		ret = cr50_i2c_read(dev, tpm_access(loc),
				    &access, sizeof(access));
		if (ret || ((access & mask) == mask)) {
			/*
			 * Don't bombard the chip with traffic; let it keep
			 * processing the command.
			 */
			mdelay(2);
			continue;
		}

		log_warning("TPM ready after %ld ms\n", get_timer(start));

		return 0;
	} while (get_timer(start) < TIMEOUT_INIT_MS);

	log_warning("TPM failed to reset after %ld ms, status: %#x\n",
		    get_timer(start), access);

	return -EPERM;
}

/*
 * Locality could be already claimed (if this is a later U-Boot phase and the
 * read-only U-Boot did not release it), or not yet claimed, if this is TPL or
 * the older read-only U-Boot did release it.
 */
static int claim_locality(struct udevice *dev, int loc)
{
	const u8 mask = TPM_ACCESS_VALID | TPM_ACCESS_ACTIVE_LOCALITY;
	struct cr50_priv *priv = dev_get_priv(dev);
	u8 access;
	int ret;

	ret = cr50_i2c_read(dev, tpm_access(loc), &access, sizeof(access));
	if (ret)
		return log_msg_ret("read1", ret);

	if ((access & mask) == mask) {
		log_warning("Locality already claimed\n");
		return 0;
	}

	access = TPM_ACCESS_REQUEST_USE;
	ret = cr50_i2c_write(dev, tpm_access(loc), &access, sizeof(access));
	if (ret)
		return log_msg_ret("write", ret);

	ret = cr50_i2c_read(dev, tpm_access(loc), &access, sizeof(access));
	if (ret)
		return log_msg_ret("read2", ret);

	if ((access & mask) != mask) {
		log_err("Failed to claim locality\n");
		return -EPERM;
	}
	log_info("Claimed locality %d\n", loc);
	priv->locality = loc;

	return 0;
}

static int cr50_i2c_get_desc(struct udevice *dev, char *buf, int size)
{
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);
	struct cr50_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "cr50 TPM 2.0 (i2c %02x id %x) irq=%d",
			chip->chip_addr, priv->vendor >> 16, priv->use_irq);
}

static int cr50_i2c_open(struct udevice *dev)
{
	char buf[80];
	int ret;

	ret = process_reset(dev);
	if (ret)
		return log_msg_ret("reset", ret);

	ret = claim_locality(dev, 0);
	if (ret)
		return log_msg_ret("claim", ret);

	cr50_i2c_get_desc(dev, buf, sizeof(buf));
	log_debug("%s\n", buf);

	return 0;
}

static int cr50_i2c_cleanup(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);

	printf("%s: cleanup %d\n", __func__, priv->locality);
	if (priv->locality != -1)
		release_locality(dev, 1);

	return 0;
}

enum {
	TPM_TIMEOUT_MS		= 5,
	SHORT_TIMEOUT_MS	= 750,
	LONG_TIMEOUT_MS		= 2000,
};

static int cr50_i2c_ofdata_to_platdata(struct udevice *dev)
{
	struct tpm_chip_priv *upriv = dev_get_uclass_priv(dev);
	struct cr50_priv *priv = dev_get_priv(dev);
	struct irq irq;
	int ret;

	upriv->version = TPM_V2;
	upriv->duration_ms[TPM_SHORT] = SHORT_TIMEOUT_MS;
	upriv->duration_ms[TPM_MEDIUM] = LONG_TIMEOUT_MS;
	upriv->duration_ms[TPM_LONG] = LONG_TIMEOUT_MS;
	upriv->retry_time_ms = TPM_TIMEOUT_MS;

	upriv->pcr_count = 32;
	upriv->pcr_select_min = 2;

	/* Optional GPIO to track when cr50 is ready */
	ret = irq_get_by_index(dev, 0, &irq);
	if (!ret) {
		priv->irq = irq;
		priv->use_irq = true;
	} else {
		ret = gpio_request_by_name(dev, "ready-gpios", 0,
					   &priv->ready_gpio, GPIOD_IS_IN);
		if (ret) {
			log_warning("Cr50 does not have an ready GPIO/interrupt (err=%d)\n",
				    ret);
		}
	}

	return 0;
}

static int cr50_i2c_probe(struct udevice *dev)
{
	struct cr50_priv *priv = dev_get_priv(dev);
	u32 vendor = 0;
	ulong start;

	/*
	 * 150ms should be enough to synchronise with the TPM even under the
	 * worst nested-reset-request conditions. In the vast majority of cases
	 * there will be no wait at all.
	 */
	start = get_timer(0);
	while (get_timer(start) < 150) {
		int ret;

		/* Exit once DID and VID verified */
		ret = cr50_i2c_read(dev, tpm_did_vid(0), (u8 *)&vendor, 4);
		if (!ret && vendor == CR50_DID_VID)
			break;

		/* TPM might be resetting; let's retry in a bit */
		mdelay(10);
	}
	if (vendor != CR50_DID_VID) {
		log_debug("DID_VID %08x not recognised\n", vendor);
		return log_msg_ret("vendor-id", -EXDEV);
	}
	priv->vendor = vendor;
	priv->locality = -1;

	return 0;
}

static const struct tpm_ops cr50_i2c_ops = {
	.open		= cr50_i2c_open,
	.get_desc	= cr50_i2c_get_desc,
	.send		= cr50_i2c_send,
	.recv		= cr50_i2c_recv,
	.cleanup	= cr50_i2c_cleanup,
};

static const struct udevice_id cr50_i2c_ids[] = {
	{ .compatible = "google,cr50" },
	{ }
};

U_BOOT_DRIVER(cr50_i2c) = {
	.name   = "cr50_i2c",
	.id     = UCLASS_TPM,
	.of_match = cr50_i2c_ids,
	.ops    = &cr50_i2c_ops,
	.ofdata_to_platdata	= cr50_i2c_ofdata_to_platdata,
	.probe	= cr50_i2c_probe,
	.remove	= cr50_i2c_cleanup,
	.priv_auto_alloc_size = sizeof(struct cr50_priv),
	.flags		= DM_FLAG_OS_PREPARE,
};
