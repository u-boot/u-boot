// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * MPS MPQ8785 / MPM3695 / MPM3695-25 / MPM82504 PMBus voltage
 * regulator driver. Adapted from
 *   linux/drivers/hwmon/pmbus/mpq8785.c
 * (Charles Hsu, GPL-2.0-or-later) with the kernel hwmon publication
 * and caching layers stripped.
 *
 * Hooks the shared pmbus_helper UCLASS_REGULATOR ops + adds the MPS
 * specific identify (VOUT_MODE switch between LINEAR16 and DIRECT
 * m=64 R=1 for the chip's "VID" mode), the MPS vendor extension
 * (pmbus mps last|clear last|clear force), and ADDR_VBOOT auto
 * promotion when the DT declared address fails the MFR_ID probe.
 */

#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <pmbus.h>
#include <vsprintf.h>
#include <linux/bitops.h>
#include <power/regulator.h>

#include "pmbus_helper.h"

/* Chip family identifiers (driver_data). */
enum mpq_chip_id {
	MPQ_MPM3695     = 0,
	MPQ_MPM3695_25  = 1,
	MPQ_MPM82504    = 2,
	MPQ_MPQ8785     = 3,
};

/*
 * MPS vendor extended command codes (NOT in PMBus 1.3 Part II).
 *
 *   CLEAR_LAST_FAULT (08h) clears the NVM backed PROTECTION_LAST
 *                          register. Gated by MFR_CFG_EXT (F5h)
 *                          bit[6] = 1; chip silently no ops if the
 *                          gate is cleared.
 *   PROTECTION_LAST  (FBh) single event, NVM backed log of the last
 *                          protection event. Survives the chip's own
 *                          power cycle. The boot time post mortem the
 *                          SoC has no other way to obtain.
 *
 * NEVER issue CLEAR_LAST_FAULT (08h) implicitly; it would erase the
 * post mortem trail. Only the explicit pmbus mps clear last and
 * pmbus mps clear force subcommands write it.
 */
#define PMBUS_CLEAR_LAST_FAULT		0x08
#define MPS_PROTECTION_LAST		0xfb

/*
 * MFR_CFG_EXT (F5h) is an MPS extended config WORD (16 bits, not a
 * byte). Bit[6] (MFR_CLR_FAULT_CFG) gates CLEAR_LAST_FAULT (08h)
 * clearing PROTECTION_LAST. Several other bits are fixed and MUST be
 * preserved on writeback; always read modify write the full 16 bits,
 * only flip bit[6], restore on the way out.
 */
#define MPS_MFR_CFG_EXT			0xf5
#define MPS_MFR_CFG_EXT_CLR_LAST_EN	BIT(6)

/*
 * MPQ8785 driver_info (transcribed from Linux's
 * drivers/hwmon/pmbus/mpq8785.c::mpq8785_info). DIRECT format with
 * chip specific m / b / R coefficients on VIN, IOUT, TEMPERATURE.
 * VOUT format is selected at probe time from VOUT_MODE: bits[7:5] == 0
 * selects LINEAR16, bits[7:5] == 1 or 2 selects DIRECT m=64 R=1.
 */

/*
 * MPS-extended STATUS_* bit names. The MPQ8785 family reuses several
 * bit positions documented as RESERVED / UNKNOWN / NONE_ABOVE /
 * MFR_SPECIFIC by PMBus 1.3 for chip specific signals. The override
 * table below substitutes the chip name for the standard one when
 * the bit is set, leaving every other PMBus 1.3 standard bit
 * (VOUT_OV, IOUT_OC, TEMP, CML, ...) unchanged.
 *
 *   STATUS_WORD bit[12]  spec MFR_SPECIFIC   chip NVM_SUMMARY (NVM
 *                                            backed PROTECTION_LAST
 *                                            register is non zero)
 *   STATUS_WORD bit[8]   spec UNKNOWN        chip WATCH_DOG (internal
 *                                            calculation FSM watchdog
 *                                            overflow)
 *   STATUS_WORD bit[0]   spec NONE_ABOVE     chip DRMOS_FAULT (DrMOS
 *                                            stage fault)
 *   STATUS_CML  bit[4]   spec MEMORY         chip MTP_CRC_FAULT (NVM
 *                                            CRC mismatch on restore)
 *   STATUS_CML  bit[0]   spec OTHER_MEM_LOGIC chip MTP_FAULT (NVM
 *                                            signature fault)
 *   STATUS_TEMPERATURE bit[0] (PMBus leaves bits[3:0] reserved on
 *                                            this register) chip
 *                                            OT_SELF (controller die
 *                                            OT condition)
 */
static const struct pmbus_status_override mpq8785_status_overrides[] = {
	{ PMBUS_STATUS_WORD,        BIT(12), "NVM_SUMMARY" },
	{ PMBUS_STATUS_WORD,        BIT(8),  "WATCH_DOG" },
	{ PMBUS_STATUS_WORD,        BIT(0),  "DRMOS_FAULT" },
	{ PMBUS_STATUS_CML,         BIT(4),  "MTP_CRC_FAULT" },
	{ PMBUS_STATUS_CML,         BIT(0),  "MTP_FAULT" },
	{ PMBUS_STATUS_TEMPERATURE, BIT(0),  "OT_SELF" },
	{ /* sentinel */ }
};

static struct pmbus_driver_info mpq8785_info = {
	.pages = 1,
	.format[PSC_VOLTAGE_IN]  = pmbus_fmt_direct,
	.format[PSC_VOLTAGE_OUT] = pmbus_fmt_linear,	/* refined per VOUT_MODE */
	.format[PSC_CURRENT_OUT] = pmbus_fmt_direct,
	.format[PSC_TEMPERATURE] = pmbus_fmt_direct,
	.m[PSC_VOLTAGE_IN]  = 4,  .b[PSC_VOLTAGE_IN]  = 0, .R[PSC_VOLTAGE_IN]  = 1,
	.m[PSC_CURRENT_OUT] = 16, .b[PSC_CURRENT_OUT] = 0, .R[PSC_CURRENT_OUT] = 0,
	.m[PSC_TEMPERATURE] = 1,  .b[PSC_TEMPERATURE] = 0, .R[PSC_TEMPERATURE] = 0,
	/*
	 * Sensor set this family actually implements with calibrated
	 * coefficients: VIN, VOUT, IOUT, TEMP. READ_IIN / READ_POUT are
	 * ACKed by the silicon but uncalibrated here (the kernel's mpq8646
	 * / mpq8785 drivers expose neither), so declaring the set keeps
	 * pmbus_print_telemetry from printing a bogus POUT / IIN -- matching
	 * the kernel's per-chip sensor list.
	 */
	.classes_present    = BIT(PSC_VOLTAGE_IN) | BIT(PSC_VOLTAGE_OUT) |
			      BIT(PSC_CURRENT_OUT) | BIT(PSC_TEMPERATURE),
	.status_overrides   = mpq8785_status_overrides,
};

/*
 * MPM3695 / MPM3695-25 / MPM82504 driver_info: VOUT in DIRECT format
 * with chip family default m=8 R=2. Other sensor classes default to
 * LINEAR (the chip family does not document non standard formats for
 * VIN / IOUT / TEMPERATURE; the helper falls back to LINEAR11 when
 * the active info is non NULL but format[c] is linear).
 */
static struct pmbus_driver_info mpm82504_info = {
	.pages = 1,
	.format[PSC_VOLTAGE_OUT] = pmbus_fmt_direct,
	.m[PSC_VOLTAGE_OUT] = 8,  .b[PSC_VOLTAGE_OUT] = 0, .R[PSC_VOLTAGE_OUT] = 2,
	.format[PSC_VOLTAGE_IN]  = pmbus_fmt_linear,
	.format[PSC_CURRENT_OUT] = pmbus_fmt_linear,
	.format[PSC_TEMPERATURE] = pmbus_fmt_linear,
};

/*
 * Chip match for the framework's pmbus dev <bus>:<addr> raw I2C
 * path. Used when the operator selects the chip directly by address
 * instead of by regulator-name; the framework probes MFR_ID, sees
 * "MPS" (after the byte reverse helper), and caches mpq8785_info.
 */
static const struct pmbus_chip_match mpq8785_match = {
	.mfr_id          = "MPS",
	.mfr_id_reverse  = true,
	.vendor          = "mps",
	.info            = &mpq8785_info,
};

static const struct pmbus_bit mpq_protection_last_bits[] = {
	{ 1u << 15, "INIT_FAULT" },
	{ 1u << 14, "NVM_CRC_ERROR" },
	{ 1u << 13, "NVM_FAULT" },
	{ 1u << 12, "OC_PHASE_FAULT" },
	{ 1u << 11, "OTP_SELF_FAULT" },
	{ 1u <<  9, "SWITCH_PRD_FAULT" },
	{ 1u <<  8, "VIN_OV_FAULT" },
	{ 1u <<  7, "VOUT_OV_FAULT" },
	{ 1u <<  6, "VOUT_UV_FAULT" },
	{ 1u <<  5, "OC_TOT_FAULT" },
	{ 1u <<  4, "VIN_UVLO_FAULT" },
	{ 1u <<  3, "DRMOS_OTP" },
	{ /* sentinel */ }
};

static int mps_require_active(struct udevice **chip)
{
	const struct pmbus_active_dev *act = pmbus_active();

	if (!act) {
		printf("pmbus mps: no active device. Use 'pmbus dev <bus>:<addr>' first.\n");
		return CMD_RET_FAILURE;
	}
	if (strcmp(act->vendor, "mps") != 0) {
		printf("pmbus mps: active device is not from vendor 'mps' (got '%s')\n",
		       act->vendor[0] ? act->vendor : "(generic)");
		return CMD_RET_FAILURE;
	}
	if (pmbus_active_get_i2c(chip)) {
		printf("pmbus mps: cannot reach i2c%d:0x%02x\n",
		       act->bus_seq, act->addr);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static int mps_do_last(struct udevice *chip)
{
	u16 prot_last = 0;

	if (pmbus_read_word(chip, MPS_PROTECTION_LAST, &prot_last)) {
		printf("pmbus mps: PROTECTION_LAST (FBh) read failed\n");
		return CMD_RET_FAILURE;
	}
	printf("PROTECTION_LAST (FBh) = 0x%04x  [", prot_last);
	pmbus_print_bits(prot_last, mpq_protection_last_bits);
	printf("]  (NVM, survives MPQ power cycle)\n");
	return CMD_RET_SUCCESS;
}

static int mps_do_clear_last(struct udevice *chip)
{
	int ret;

	printf("pmbus mps: WARNING, erasing NVM PROTECTION_LAST (FBh) post mortem\n");
	ret = dm_i2c_write(chip, PMBUS_CLEAR_LAST_FAULT, NULL, 0);
	if (ret) {
		printf("pmbus mps: CLEAR_LAST_FAULT (08h) write failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	printf("pmbus mps: CLEAR_LAST_FAULT (08h) issued; gated by MFR_CFG_EXT bit[6]\n");
	printf("           (chip silently no ops if F5h bit[6] = 0; verify by re reading FBh)\n");
	return CMD_RET_SUCCESS;
}

static int mps_do_clear_force(struct udevice *chip)
{
	u8 wp_orig = 0;
	u16 cfg_orig = 0, cfg_unlocked;
	int ret, last_rc = 0, rc;

	printf("pmbus mps: FORCE; temporarily lowering WRITE_PROTECT and MFR_CFG_EXT.CLEAR_LAST_EN\n");
	printf("pmbus mps: WARNING, erasing NVM PROTECTION_LAST (FBh) post mortem\n");

	ret = pmbus_read_byte(chip, PMBUS_WRITE_PROTECT, &wp_orig);
	if (ret) {
		printf("pmbus mps: WRITE_PROTECT (10h) read failed (%d), aborting force\n", ret);
		return CMD_RET_FAILURE;
	}
	ret = pmbus_read_word(chip, MPS_MFR_CFG_EXT, &cfg_orig);
	if (ret) {
		printf("pmbus mps: MFR_CFG_EXT (F5h) read failed (%d), aborting force\n", ret);
		return CMD_RET_FAILURE;
	}
	printf("pmbus mps: saved WRITE_PROTECT=0x%02x MFR_CFG_EXT=0x%04x\n",
	       wp_orig, cfg_orig);

	if (wp_orig != 0) {
		u8 wp_open = 0x00;

		ret = dm_i2c_write(chip, PMBUS_WRITE_PROTECT, &wp_open, 1);
		if (ret) {
			printf("pmbus mps: WRITE_PROTECT clear failed (%d), chip refuses unlock\n",
			       ret);
			return CMD_RET_FAILURE;
		}
	}

	cfg_unlocked = cfg_orig | MPS_MFR_CFG_EXT_CLR_LAST_EN;
	ret = pmbus_write_word(chip, MPS_MFR_CFG_EXT, cfg_unlocked);
	if (ret) {
		printf("pmbus mps: MFR_CFG_EXT <- 0x%04x failed (%d)\n",
		       cfg_unlocked, ret);
		goto restore_wp;
	}

	last_rc = dm_i2c_write(chip, PMBUS_CLEAR_LAST_FAULT, NULL, 0);
	if (last_rc)
		printf("pmbus mps: CLEAR_LAST_FAULT (08h) write failed (%d) even with gate open\n",
		       last_rc);
	else
		printf("pmbus mps: CLEAR_LAST_FAULT (08h) issued with MFR_CFG_EXT bit[6]=1, PROTECTION_LAST should now read 0x0000\n");

	rc = pmbus_write_word(chip, MPS_MFR_CFG_EXT, cfg_orig);
	if (rc)
		printf("pmbus mps: MFR_CFG_EXT restore failed (%d), gate may stay open until POR\n",
		       rc);

restore_wp:
	if (wp_orig != 0) {
		rc = dm_i2c_write(chip, PMBUS_WRITE_PROTECT, &wp_orig, 1);
		if (rc)
			printf("pmbus mps: WRITE_PROTECT restore failed (%d), chip stays unlocked until POR\n",
			       rc);
	}

	return (ret || last_rc) ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int mps_vendor_handler(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	struct udevice *chip;
	int rc;

	if (argc < 2)
		return CMD_RET_USAGE;

	rc = mps_require_active(&chip);
	if (rc)
		return rc;

	if (!strcmp(argv[1], "last") && argc == 2)
		return mps_do_last(chip);
	if (!strcmp(argv[1], "clear") && argc >= 3) {
		if (!strcmp(argv[2], "last"))
			return mps_do_clear_last(chip);
		if (!strcmp(argv[2], "force"))
			return mps_do_clear_force(chip);
	}
	return CMD_RET_USAGE;
}

static const struct pmbus_vendor_op mps_vendor_op = {
	.vendor  = "mps",
	.handler = mps_vendor_handler,
	.help    = "pmbus mps last         : read MPS PROTECTION_LAST (FBh)\n"
		   "pmbus mps clear last   : issue MPS CLEAR_LAST_FAULT (08h) (DESTRUCTIVE)\n"
		   "pmbus mps clear force  : force clear via MFR_CFG_EXT bit[6] (DESTRUCTIVE)\n",
};

static void mpq8785_identify_vout(struct udevice *i2c_dev)
{
	enum pmbus_data_format fmt;

	/*
	 * Let the shared helper read VOUT_MODE and pick the base format
	 * (the single source of truth for the bit layout). The MPS quirk:
	 * this family encodes VOUT in DIRECT with m=64 R=1 whenever
	 * VOUT_MODE reports VID *or* DIRECT -- override the helper's
	 * generic DIRECT m=1 / VID-unwired result in those two modes.
	 * LINEAR and IEEE754 keep the helper's selection unchanged.
	 */
	fmt = pmbus_regulator_identify_vout(i2c_dev, &mpq8785_info);
	if (fmt == pmbus_fmt_vid || fmt == pmbus_fmt_direct) {
		mpq8785_info.format[PSC_VOLTAGE_OUT] = pmbus_fmt_direct;
		mpq8785_info.m[PSC_VOLTAGE_OUT] = 64;
		mpq8785_info.b[PSC_VOLTAGE_OUT] = 0;
		mpq8785_info.R[PSC_VOLTAGE_OUT] = 1;
	}
}

/*
 * The MPQ8785 datasheet revision letter changes which window the
 * analog ADDR_VBOOT level resolves to. Boards have been observed at
 * 0x10 (later die rev) versus the 0x20 the original driver assumed.
 * If the DT declared address fails the MFR_ID probe at probe time,
 * walk the three documented windows looking for an MPS responder.
 *
 * Each window covers 16 consecutive 7 bit I2C addresses; the low
 * nibble selects the chip's MFR_ADDR_PMBUS slot within the window.
 */
#define MPS_ADDR_VBOOT_WINDOW_SIZE	16
static const u8 mps_addr_window_starts[] = { 0x10, 0x20, 0x60 };

static int mpq8785_probe_addr(struct udevice *bus, u8 addr,
			      struct udevice **chip_out)
{
	char id[PMBUS_MFR_STRING_MAX] = "";
	struct udevice *chip;
	int ret;

	ret = i2c_get_chip(bus, addr, 1, &chip);
	if (ret)
		return ret;
	ret = pmbus_read_string(chip, PMBUS_MFR_ID, id, sizeof(id), true);
	if (ret < 0)
		return ret;
	if (strncmp(id, "MPS", 3) != 0)
		return -ENODEV;
	*chip_out = chip;
	return 0;
}

static int mpq8785_scan_windows(struct udevice *bus, u8 *found_addr,
				struct udevice **chip_out)
{
	unsigned int i, j;

	for (i = 0; i < ARRAY_SIZE(mps_addr_window_starts); i++) {
		for (j = 0; j < MPS_ADDR_VBOOT_WINDOW_SIZE; j++) {
			u8 addr = mps_addr_window_starts[i] + j;

			if (mpq8785_probe_addr(bus, addr, chip_out) == 0) {
				*found_addr = addr;
				return 0;
			}
		}
	}
	return -ENODEV;
}

static struct pmbus_driver_info *mpq8785_pick_info(enum mpq_chip_id chip_id)
{
	switch (chip_id) {
	case MPQ_MPM3695:
	case MPQ_MPM3695_25:
	case MPQ_MPM82504:
		return &mpm82504_info;
	case MPQ_MPQ8785:
	default:
		return &mpq8785_info;
	}
}

static int mpq8785_probe(struct udevice *dev)
{
	enum mpq_chip_id chip_id = (enum mpq_chip_id)dev_get_driver_data(dev);
	struct pmbus_regulator_priv *priv = dev_get_priv(dev);
	struct pmbus_driver_info *info = mpq8785_pick_info(chip_id);
	static bool match_registered;
	static bool vendor_registered;
	u32 fb_div;
	int ret;

	ret = pmbus_regulator_probe_common(dev, info, 0);
	if (ret)
		return ret;

	/*
	 * Verify the chip answers MFR_ID="MPS" at the DT declared
	 * address. If it doesn't, walk the documented ADDR_VBOOT windows
	 * looking for it (a die rev address shift). On a hit, replace
	 * priv->i2c_dev with the discovered chip handle and continue.
	 */
	{
		char id[PMBUS_MFR_STRING_MAX] = "";

		ret = pmbus_read_string(priv->i2c_dev, PMBUS_MFR_ID, id,
					sizeof(id), true);
		if (ret < 0 || strncmp(id, "MPS", 3) != 0) {
			struct udevice *bus = dev_get_parent(dev);
			struct udevice *promoted;
			u8 found = 0;

			if (mpq8785_scan_windows(bus, &found, &promoted) == 0) {
				printf("MPQ8785: DT addr 0x%02x silent, auto promoted to 0x%02x\n",
				       (unsigned int)dev_read_addr(dev), found);
				priv->i2c_dev = promoted;
			} else {
				printf("MPQ8785: no MPS responder found in 0x10..0x1f / 0x20..0x2f / 0x60..0x6f\n");
				return -ENODEV;
			}
		}
	}

	/* MPQ8785 specific: refine VOUT format from VOUT_MODE. */
	if (chip_id == MPQ_MPQ8785)
		mpq8785_identify_vout(priv->i2c_dev);

	/* Apply mps,vout-fb-divider-ratio-permille if present in DT. */
	fb_div = dev_read_u32_default(dev, "mps,vout-fb-divider-ratio-permille", 0);
	if (fb_div) {
		ret = pmbus_regulator_apply_voltage_scale(dev, fb_div);
		if (ret) {
			printf("MPQ8785: VOUT_SCALE_LOOP write failed (%d)\n", ret);
			return ret;
		}
	}

	/*
	 * Register the chip match and the MPS vendor handler exactly
	 * once across all bound MPS regulators (a board could legally
	 * carry several). Both registries are global and idempotent
	 * matches return -ENOSPC, so the static guards keep things
	 * tidy.
	 */
	if (!match_registered) {
		if (pmbus_register_chip(&mpq8785_match) == 0)
			match_registered = true;
	}
	if (!vendor_registered) {
		if (pmbus_register_vendor_handler(&mps_vendor_op) == 0)
			vendor_registered = true;
	}
	return 0;
}

static const struct udevice_id mpq8785_ids[] = {
	{ .compatible = "mps,mpm3695",    .data = MPQ_MPM3695 },
	{ .compatible = "mps,mpm3695-25", .data = MPQ_MPM3695_25 },
	{ .compatible = "mps,mpm82504",   .data = MPQ_MPM82504 },
	{ .compatible = "mps,mpq8785",    .data = MPQ_MPQ8785 },
	{ }
};

U_BOOT_DRIVER(mpq8785_regulator) = {
	.name      = "mpq8785_regulator",
	.id        = UCLASS_REGULATOR,
	.of_match  = mpq8785_ids,
	.probe     = mpq8785_probe,
	.ops       = &pmbus_regulator_ops,
	.priv_auto = sizeof(struct pmbus_regulator_priv),
};
