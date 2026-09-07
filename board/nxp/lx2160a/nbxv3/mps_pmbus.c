// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * The MPS MPQ8785 or MPQ8646 chips are bound through DT
 *
 * The only thing this file still does is to provide console traces.
 */

#include <command.h>
#include <dm.h>
#include <event.h>
#include <i2c.h>
#include <log.h>
#include <pmbus.h>
#include <vsprintf.h>
#include <power/regulator.h>

#define NBXV3_VDD_REGULATOR_NAME	"+0V8_VDD"

static void mps_pmbus_print_snapshot(struct udevice *reg)
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	u16 raw, status_word = 0, prot_last = 0;
	u8 vout_mode = 0;
	int uV, en;

	uV = regulator_get_value(reg);
	en = regulator_get_enable(reg);

	act = pmbus_active();
	if (!act || pmbus_active_get_i2c(&chip)) {
		printf("MPQ8785 (+0V8_VDD): VOUT=%d uV  enabled=%d  (no PMBus active dev)\n",
		       uV, en);
		return;
	}

	pmbus_read_byte(chip, PMBUS_VOUT_MODE, &vout_mode);
	pmbus_read_word(chip, PMBUS_STATUS_WORD, &status_word);
	pmbus_read_word(chip, 0xfb /* MPS_PROTECTION_LAST */, &prot_last);

	{
		u8 rev_byte = 0;

		(void)pmbus_read_byte(chip, PMBUS_MFR_REVISION, &rev_byte);
		printf("MPQ8785 @ i2c%d:0x%02x  MFR_ID=\"%s\" MODEL=\"%s\" REV=0x%02x VOUT_MODE=0x%02x\n",
		       act->bus_seq, act->addr,
		       act->mfr_id[0] ? act->mfr_id : "?",
		       act->mfr_model[0] ? act->mfr_model : "?",
		       rev_byte, vout_mode);
	}
	printf("  +0V8_VDD: VOUT=%d uV  enabled=%d\n", uV, en);

	if (pmbus_read_word(chip, PMBUS_READ_VIN, &raw) == 0)
		printf("            VIN  raw=0x%04x  =%lld uV\n", raw,
		       (long long)pmbus_reg2data(act->info, PSC_VOLTAGE_IN,
						 raw, vout_mode));
	if (pmbus_read_word(chip, PMBUS_READ_IOUT, &raw) == 0)
		printf("            IOUT raw=0x%04x  =%lld uA\n", raw,
		       (long long)pmbus_reg2data(act->info, PSC_CURRENT_OUT,
						 raw, vout_mode));
	if (pmbus_read_word(chip, PMBUS_READ_TEMPERATURE_1, &raw) == 0)
		printf("            TEMP raw=0x%04x  =%lld udegC\n", raw,
		       (long long)pmbus_reg2data(act->info, PSC_TEMPERATURE,
						 raw & 0x00ff, vout_mode));

	printf("  STATUS_WORD = 0x%04x  [", status_word);
	pmbus_print_status_bits(PMBUS_STATUS_WORD, status_word,
				pmbus_status_word_bits,
				act->info ? act->info->status_overrides : NULL);
	printf("]\n");
	printf("  PROTECTION_LAST = 0x%04x (NVM)\n", prot_last);
	printf("  Use `pmbus telemetry`, `pmbus status`, `pmbus mps last` for details.\n");
}

static int mps_pmbus_last_stage_init(void)
{
	struct udevice *reg = NULL;

	if (regulator_get_by_platname(NBXV3_VDD_REGULATOR_NAME, &reg))
		return 0;
	if (!reg)
		return 0;

	{
		int bus_seq;
		u8 addr;

		if (pmbus_resolve_by_name(NBXV3_VDD_REGULATOR_NAME,
					  &bus_seq, &addr) == 0)
			pmbus_set_active(bus_seq, addr);
	}

	mps_pmbus_print_snapshot(reg);
	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, mps_pmbus_last_stage_init);
