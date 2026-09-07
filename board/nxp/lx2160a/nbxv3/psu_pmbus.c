// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * Nodebox v3 boot-time info of the carrier PMBus PSU(s).
 */

#include <dm.h>
#include <event.h>
#include <i2c.h>
#include <pmbus.h>
#include <dm/uclass.h>
#include <power/regulator.h>

#define NBXV3_GENERIC_PMBUS_DRIVER	"pmbus_generic_regulator"

static void nbxv3_psu_snapshot_one(struct udevice *reg)
{
	struct udevice *parent = dev_get_parent(reg);
	struct udevice *chip;
	int bus_seq, addr;
	const struct pmbus_active_dev *act;

	if (!parent || device_get_uclass_id(parent) != UCLASS_I2C)
		return;
	bus_seq = dev_seq(parent);
	addr = dev_read_addr(reg);
	if (addr < 0 || addr > 0x7f)
		return;

	/* Select the device: caches MFR_* + reuses the generic info. */
	if (pmbus_set_active(bus_seq, (u8)addr))
		return;
	act = pmbus_active();
	if (!act)
		return;
	if (pmbus_active_get_i2c(&chip))
		return;

	printf("PMBus PSU @ i2c%d:0x%02x", act->bus_seq, act->addr);
	if (act->name[0])
		printf("  rail=\"%s\"", act->name);
	if (act->mfr_id[0])
		printf("  MFR_ID=\"%s\"", act->mfr_id);
	if (act->mfr_model[0])
		printf("  MODEL=\"%s\"", act->mfr_model);
	printf("\n");

	pmbus_print_telemetry(chip);

	pmbus_print_status_word(chip);
}

static int nbxv3_psu_pmbus_last_stage_init(void)
{
	struct udevice *reg;
	struct uclass *uc;

	if (uclass_get(UCLASS_REGULATOR, &uc))
		return 0;

	uclass_foreach_dev(reg, uc) {
		if (!reg->driver || !reg->driver->name)
			continue;
		if (strcmp(reg->driver->name, NBXV3_GENERIC_PMBUS_DRIVER))
			continue;
		nbxv3_psu_snapshot_one(reg);
	}
	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, nbxv3_psu_pmbus_last_stage_init);
