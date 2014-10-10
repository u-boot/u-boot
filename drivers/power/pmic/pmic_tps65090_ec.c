/*
 * Copyright (c) 2013 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>
#include <errno.h>
#include <power/tps65090_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#define TPS65090_ADDR		0x48

static struct tps65090 {
	struct cros_ec_dev *dev;		/* The CROS_EC device */
} config;

/* TPS65090 register addresses */
enum {
	REG_IRQ1 = 0,
	REG_CG_CTRL0 = 4,
	REG_CG_STATUS1 = 0xa,
	REG_FET1_CTRL = 0x0f,
	REG_FET2_CTRL,
	REG_FET3_CTRL,
	REG_FET4_CTRL,
	REG_FET5_CTRL,
	REG_FET6_CTRL,
	REG_FET7_CTRL,
	TPS65090_NUM_REGS,
};

enum {
	IRQ1_VBATG = 1 << 3,
	CG_CTRL0_ENC_MASK	= 0x01,

	MAX_FET_NUM	= 7,
	MAX_CTRL_READ_TRIES = 5,

	/* TPS65090 FET_CTRL register values */
	FET_CTRL_TOFET		= 1 << 7,  /* Timeout, startup, overload */
	FET_CTRL_PGFET		= 1 << 4,  /* Power good for FET status */
	FET_CTRL_WAIT		= 3 << 2,  /* Overcurrent timeout max */
	FET_CTRL_ADENFET	= 1 << 1,  /* Enable output auto discharge */
	FET_CTRL_ENFET		= 1 << 0,  /* Enable FET */
};

/**
 * tps65090_read - read a byte from tps6090
 *
 * @param reg		The register address to read from.
 * @param val		We'll return value value read here.
 * @return 0 if ok; error if EC returns failure.
 */
static int tps65090_read(u32 reg, u8 *val)
{
	return cros_ec_i2c_xfer(config.dev, TPS65090_ADDR, reg, 1,
				val, 1, true);
}

/**
 * tps65090_write - write a byte to tps6090
 *
 * @param reg		The register address to write to.
 * @param val		The value to write.
 * @return 0 if ok; error if EC returns failure.
 */
static int tps65090_write(u32 reg, u8 val)
{
	return cros_ec_i2c_xfer(config.dev, TPS65090_ADDR, reg, 1,
				&val, 1, false);
}

/**
 * Checks for a valid FET number
 *
 * @param fet_id	FET number to check
 * @return 0 if ok, -EINVAL if FET value is out of range
 */
static int tps65090_check_fet(unsigned int fet_id)
{
	if (fet_id == 0 || fet_id > MAX_FET_NUM) {
		debug("parameter fet_id is out of range, %u not in 1 ~ %u\n",
		      fet_id, MAX_FET_NUM);
		return -EINVAL;
	}

	return 0;
}

/**
 * Set the power state for a FET
 *
 * @param fet_id	Fet number to set (1..MAX_FET_NUM)
 * @param set		1 to power on FET, 0 to power off
 * @return -EIO if we got a comms error, -EAGAIN if the FET failed to
 * change state. If all is ok, returns 0.
 */
static int tps65090_fet_set(int fet_id, bool set)
{
	int retry;
	u8 reg, value;

	value = FET_CTRL_ADENFET | FET_CTRL_WAIT;
	if (set)
		value |= FET_CTRL_ENFET;

	if (tps65090_write(REG_FET1_CTRL + fet_id - 1, value))
		return -EIO;

	/* Try reading until we get a result */
	for (retry = 0; retry < MAX_CTRL_READ_TRIES; retry++) {
		if (tps65090_read(REG_FET1_CTRL + fet_id - 1, &reg))
			return -EIO;

		/* Check that the fet went into the expected state */
		if (!!(reg & FET_CTRL_PGFET) == set)
			return 0;

		/* If we got a timeout, there is no point in waiting longer */
		if (reg & FET_CTRL_TOFET)
			break;

		mdelay(1);
	}

	debug("FET %d: Power good should have set to %d but reg=%#02x\n",
	      fet_id, set, reg);
	return -EAGAIN;
}

int tps65090_fet_enable(unsigned int fet_id)
{
	ulong start;
	int loops;
	int ret;

	ret = tps65090_check_fet(fet_id);
	if (ret)
		return ret;

	start = get_timer(0);
	for (loops = 0;; loops++) {
		ret = tps65090_fet_set(fet_id, true);
		if (!ret)
			break;

		if (get_timer(start) > 100)
			break;

		/* Turn it off and try again until we time out */
		tps65090_fet_set(fet_id, false);
	}

	if (ret) {
		debug("%s: FET%d failed to power on: time=%lums, loops=%d\n",
		      __func__, fet_id, get_timer(start), loops);
	} else if (loops) {
		debug("%s: FET%d powered on after %lums, loops=%d\n",
		      __func__, fet_id, get_timer(start), loops);
	}
	/*
	 * Unfortunately, there are some conditions where the power
	 * good bit will be 0, but the fet still comes up. One such
	 * case occurs with the lcd backlight. We'll just return 0 here
	 * and assume that the fet will eventually come up.
	 */
	if (ret == -EAGAIN)
		ret = 0;

	return ret;
}

int tps65090_fet_disable(unsigned int fet_id)
{
	int ret;

	ret = tps65090_check_fet(fet_id);
	if (ret)
		return ret;

	ret = tps65090_fet_set(fet_id, false);

	return ret;
}

int tps65090_fet_is_enabled(unsigned int fet_id)
{
	u8 reg = 0;
	int ret;

	ret = tps65090_check_fet(fet_id);
	if (ret)
		return ret;
	ret = tps65090_read(REG_FET1_CTRL + fet_id - 1, &reg);
	if (ret) {
		debug("fail to read FET%u_CTRL register over I2C", fet_id);
		return -EIO;
	}

	return reg & FET_CTRL_ENFET;
}

int tps65090_init(void)
{
	puts("TPS65090 PMIC EC init\n");

	config.dev = board_get_cros_ec_dev();
	if (!config.dev) {
		debug("%s: no cros_ec device: cannot init tps65090\n",
		      __func__);
		return -ENODEV;
	}

	return 0;
}
