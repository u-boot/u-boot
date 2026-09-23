// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * Per die core voltage trim for the Nodebox v3 +0V8_VDD rail
 */

#include <command.h>
#include <dm.h>
#include <env.h>
#include <event.h>
#include <linux/delay.h>
#include <log.h>
#include <vsprintf.h>
#include <asm/io.h>
#include <asm/arch/immap_lsch3.h>
#include <power/regulator.h>

#define NBXV3_VDD_REGULATOR_NAME	"+0V8_VDD"
#define NBXV3_VDD_OVERRIDE_ENV		"nbxv3_vdd_mv"
#define NBXV3_VDD_MIN_MV		600
#define NBXV3_VDD_MAX_MV		1100

#define NBXV3_VID_LSB_uV		1953	/* VOUT_MODE 0x17: 2^-9 V = one PMBus step */

#define NBXV3_VID_SLEW_SETTLE_US	5000	/* >> slew time for a 30 mV step */

/*
 * From LX2160ARM
 * "This is the minimum voltage required to reach the frequency
 * specified."
 * Everything not listed is Reserved, it maps to
 * 0 = "leave the rail untouched" below.
 */
static const u16 lx2160a_vid_table[32] = {
	[0x00] = 8500,	/* default */
	[0x02] = 7750,
	[0x10] = 8000,
	[0x12] = 8250,
	[0x14] = 8500,
};

/*
 * Write VOUT_COMMAND, let the slew finish, read the rail back.
 * Returns the rail in uV, or a negative errno from the write.
 */
static int nbxv3_vid_write_readback(struct udevice *reg, int cmd_uV)
{
	int ret = regulator_set_value(reg, cmd_uV);

	if (ret)
		return ret;
	udelay(NBXV3_VID_SLEW_SETTLE_US);
	return regulator_get_value(reg);
}

static int nbxv3_vid_handoff(void)
{
	struct ccsr_gur *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	struct udevice *reg;
	u32 fusesr;
	u8 vid;
	int target_mv, target_uV, actual_uV, cmd_uV;
	char *override_str;
	unsigned long override_mv;
	int ret;

	if (regulator_get_by_platname(NBXV3_VDD_REGULATOR_NAME, &reg) || !reg)
		return 0;

	fusesr = in_le32(&gur->dcfg_fusesr);
	vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
	       FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if (vid == 0 || vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK)
		vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
		       FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;

	target_mv = (lx2160a_vid_table[vid] + 5) / 10;
	if (target_mv == 0) {
		printf("MPQ8785 (%s): VID handoff fuse=0x%02x reserved/unknown, leaving rail untouched\n",
		       NBXV3_VDD_REGULATOR_NAME, vid);
		return 0;
	}

	/* Env override */
	override_str = env_get(NBXV3_VDD_OVERRIDE_ENV);
	if (override_str && !strict_strtoul(override_str, 10, &override_mv) &&
	    override_mv >= NBXV3_VDD_MIN_MV &&
	    override_mv <= NBXV3_VDD_MAX_MV) {
		printf("MPQ8785 (%s): VID handoff override %s=%lu mV (fuse VID=0x%02x would have been %d mV)\n",
		       NBXV3_VDD_REGULATOR_NAME, NBXV3_VDD_OVERRIDE_ENV,
		       override_mv, vid, target_mv);
		target_mv = (int)override_mv;
	} else if (override_str) {
		printf("MPQ8785 (%s): VID handoff %s=\"%s\" out of range [%d, %d] mV, ignored\n",
		       NBXV3_VDD_REGULATOR_NAME, NBXV3_VDD_OVERRIDE_ENV,
		       override_str, NBXV3_VDD_MIN_MV, NBXV3_VDD_MAX_MV);
	}

	target_uV = target_mv * 1000;
	actual_uV = regulator_get_value(reg);

	if (abs(actual_uV - target_uV) <= NBXV3_VID_LSB_uV) {
		printf("MPQ8785 (%s): VID handoff fuse=0x%02x target=%d mV (rail already at %d uV, untouched)\n",
		       NBXV3_VDD_REGULATOR_NAME, vid, target_mv, actual_uV);
		return 0;
	}

	printf("MPQ8785 (%s): VID handoff fuse=0x%02x rail %d uV, target %d mV -> write %d uV\n",
	       NBXV3_VDD_REGULATOR_NAME, vid, actual_uV, target_mv, target_uV);

	ret = nbxv3_vid_write_readback(reg, target_uV);
	if (ret < 0) {
		printf("WARNING: MPQ8785 (%s): VID handoff set_value(%d uV) failed (%d), rail left at %d uV\n",
		       NBXV3_VDD_REGULATOR_NAME, target_uV, ret, actual_uV);
		return 0;
	}
	actual_uV = ret;
	if (abs(actual_uV - target_uV) <= NBXV3_VID_LSB_uV) {
		printf("MPQ8785 (%s): VID handoff verified %d uV (target %d uV, cmd %d uV)\n",
		       NBXV3_VDD_REGULATOR_NAME, actual_uV, target_uV, target_uV);
		return 0;
	}

	cmd_uV = target_uV + (target_uV - actual_uV);
	ret = nbxv3_vid_write_readback(reg, cmd_uV);
	if (ret < 0) {
		printf("WARNING: MPQ8785 (%s): VID handoff correction set_value(%d uV) failed (%d), rail left at %d uV\n",
		       NBXV3_VDD_REGULATOR_NAME, cmd_uV, ret, actual_uV);
		return 0;
	}
	actual_uV = ret;
	if (abs(actual_uV - target_uV) <= NBXV3_VID_LSB_uV)
		printf("MPQ8785 (%s): VID handoff verified %d uV (target %d uV, cmd %d uV after 1 correction)\n",
		       NBXV3_VDD_REGULATOR_NAME, actual_uV, target_uV, cmd_uV);
	else
		printf("WARNING: MPQ8785 (%s): VID handoff rail %d uV %s target %d uV after correction (cmd %d uV) -- VOUT_MAX / non-linear load line, fix on the VR side\n",
		       NBXV3_VDD_REGULATOR_NAME, actual_uV,
		       actual_uV < target_uV ? "below" : "above", target_uV,
		       cmd_uV);
	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, nbxv3_vid_handoff);
