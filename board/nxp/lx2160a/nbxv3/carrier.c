// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 carrier selector.
 *
 * The CPU module is used on several carrier boards (nbv30, nbv32, ...).
 * They share a same kernel Image but they need different DTBs and different
 * DPAA2 DPC/DPL.
 *
 *   images:         fdt-<carrier>, dpc-<carrier>, dpl-<carrier>
 *   configurations: conf-<carrier>
 *
 * and then the boot env picks one with ${carrier}:
 *   mcinitcmd  imxtract ... dpc-${carrier} / dpl-${carrier}
 *   mc_init    bootm ${kernel_addr_r}#conf-${carrier}
 *
 * ${carrier} comes, first match wins, from:
 *   1. the environment, e.g. a saved one:
 *        setenv carrier nbv32 && saveenv && reset
 *   2. the NOR overlay, imported by ${cfg_import}
 *   3. the RCW build stamp in DCFG SCRATCHRW13, see below
 *   4. NBXV3_CARRIER_DEFAULT
 */

#include <command.h>
#include <env.h>
#include <hwconfig.h>
#include <malloc.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <vsprintf.h>
#include <asm/io.h>
#include <asm/arch/immap_lsch3.h>
#include <asm/arch/soc.h>

#include "carrier.h"
#include "../serdes_split.h"

#ifndef NBXV3_CARRIER_DEFAULT
#define NBXV3_CARRIER_DEFAULT "nbv30"
#endif

/*
 * RCW build stamp. The RCW must already match the carrier, because it
 * sets the SerDes protocols and the pinmux, so its PBI also writes the
 * carrier it was built for into DCFG SCRATCHRW13:
 *
 *   31   28 27   24 23   16 15    8 7     0
 *  +-------+-------+-------+-------+-------+
 *  |CARRIER|  TAG  |  YY   |  MM   |  DD   |
 *  +-------+-------+-------+-------+-------+
 *
 * TAG is 0xB. Any other TAG, including the reset value 0 of the
 * register, means no stamp. YY MM DD is the RCW build date in BCD, for
 * the operator only.
 *
 * CARRIER:
 *   0     any nbv3 carrier: one RCW for all of them, it names none
 *   1     NBV30 / NBV31
 *   2     NBV32
 *   3     NBV32 with SerDes 1 at protocol 20 (2x 40GE) in the RCW
 *   4..E  unassigned, treated like 0
 *   F     bring-up RCW: no SerDes IO, it names no carrier either
 */
#define NBXV3_STAMP_SCRATCHRW		12	/* SCRATCHRW13 */
#define NBXV3_STAMP_CARRIER		GENMASK(31, 28)
#define NBXV3_STAMP_TAG			GENMASK(27, 24)
#define NBXV3_STAMP_TAG_NBXV3		0xb
#define NBXV3_CARRIER_BRINGUP		0xf

/* CARRIER digit of the RCW build stamp, -ENOENT without a stamp */
static int nbxv3_stamp_carrier(void)
{
	struct ccsr_gur __iomem *gur = (void __iomem *)CFG_SYS_FSL_GUTS_ADDR;
	u32 stamp = gur_in32(&gur->scratchrw[NBXV3_STAMP_SCRATCHRW]);

	if (FIELD_GET(NBXV3_STAMP_TAG, stamp) != NBXV3_STAMP_TAG_NBXV3)
		return -ENOENT;

	return FIELD_GET(NBXV3_STAMP_CARRIER, stamp);
}

/* NULL when the stamp names no carrier */
static const char *nbxv3_carrier_detect(int digit)
{
	switch (digit) {
	case 1:
		return "nbv30";
	case 2:
		return "nbv32";
	case 3:
		return "nbv32-40g";
	default:
		return NULL;
	}
}

/*
 * The RCW of bringup powers down every SerDes PLL: there is no WRIOP for
 * the MC to manage, so do not start the MC.
 */
static void nbxv3_carrier_disable_mc(void)
{
	if (!env_get("mcinitcmd"))
		return;

	if (env_set("mcinitcmd", NULL)) {
		printf("Carrier:       WARNING: cannot clear ${mcinitcmd}, the MC start is expected to hang\n");
		return;
	}

	printf("Carrier:       bring-up RCW (no SerDes IO), MC not started\n");
}

static void nbxv3_carrier_import_overlay(void)
{
	if (!env_get("cfg_import"))
		return;

	if (run_command("run cfg_import", 0))
		debug("Carrier:       no usable overlay at ${cfg_nor_offset}\n");
}

void nbxv3_carrier_env_init(void)
{
	const char *carrier = env_get("carrier");
	const char *origin;
	int digit;

	/* Whatever ${carrier} says, the SerDes have no IO */
	digit = nbxv3_stamp_carrier();
	if (digit == NBXV3_CARRIER_BRINGUP)
		nbxv3_carrier_disable_mc();

	if (carrier && *carrier) {
		printf("Carrier:       %s (from env)\n", carrier);
		return;
	}

	nbxv3_carrier_import_overlay();

	carrier = env_get("carrier");
	if (carrier && *carrier) {
		printf("Carrier:       %s (from the NOR overlay at %s)\n",
		       carrier, env_get("cfg_nor_offset"));
		return;
	}

	carrier = nbxv3_carrier_detect(digit);
	if (carrier) {
		origin = "RCW build stamp";
	} else {
		carrier = NBXV3_CARRIER_DEFAULT;
		if (digit < 0)
			origin = "default, no RCW build stamp";
		else if (digit == NBXV3_CARRIER_BRINGUP)
			origin = "default, the bring-up RCW names no carrier";
		else
			origin = "default, the RCW names no carrier";
	}

	if (env_set("carrier", carrier)) {
		printf("Carrier:       WARNING: cannot set ${carrier}\n");
		return;
	}

	/* Not saved on purpose: saveenv on every boot would burn the NOR */
	printf("Carrier:       %s (%s, not saved; setenv carrier <name> && saveenv to pin)\n",
	       carrier, origin);
}

/*
 * serdes 1 lane grouping of each carrier, as a hwconfig serdes1:<mode>
 * of board/nxp/lx2160a/serdes_split.c. A carrier without an entry
 * keeps what the RCW provides.
 *
 * nbv32-40g used to need an RCW at serdes 1 protocol 20. serdes1:2x40g
 * builds the same grouping from the protocol 13 RCW, and it is void
 * for a protocol 20 one.
 */
static const struct {
	const char *carrier;
	const char *mode;
} nbxv3_serdes1_modes[] = {
	{ .carrier = "nbv32",			.mode = "100g4x25g" },
	{ .carrier = "nbv32-40g",		.mode = "2x40g" },
	{ .carrier = "nbv32-40g-4x10g",		.mode = "40g4x10g" },
	{ .carrier = "nbv32-100g-4x10g",	.mode = "100g4x10g" },
};

/* Is ${hwconfig} serdes1:@mode ? */
static bool nbxv3_serdes1_is(const char *mode)
{
	const char *arg;
	size_t len;

	arg = hwconfig_arg("serdes1", &len);

	return arg && len == strlen(mode) && !strncmp(arg, mode, len);
}

/* Does ${hwconfig} name a SerDes 1 mode ? */
static bool nbxv3_serdes1_named(void)
{
	int i;

	if (nbxv3_serdes1_is("2x100g"))
		return true;

	for (i = 0; i < ARRAY_SIZE(nbxv3_serdes1_modes); i++)
		if (nbxv3_serdes1_is(nbxv3_serdes1_modes[i].mode))
			return true;

	return false;
}

void nbxv3_serdes1_apply(void)
{
	const char *carrier = env_get("carrier");
	const char *mode = NULL;
	const char *cur;
	char *saved = NULL;
	char *hw = NULL;
	bool set = false;
	int i;

	for (i = 0; carrier && i < ARRAY_SIZE(nbxv3_serdes1_modes); i++)
		if (!strcmp(carrier, nbxv3_serdes1_modes[i].carrier))
			mode = nbxv3_serdes1_modes[i].mode;

	/* Put serdes1:<mode> in front of ${hwconfig} */
	if (mode && !nbxv3_serdes1_named()) {
		if (hwconfig("serdes1"))
			printf("Carrier:       ${hwconfig} serdes1 names no mode, serdes1:%s of %s used\n",
			       mode, carrier);
		cur = env_get("hwconfig");
		if (cur && *cur)
			saved = strdup(cur);
		hw = malloc(sizeof("serdes1:") + strlen(mode) +
			    (saved ? strlen(saved) + 1 : 0));
		if (hw && (saved || !cur || !*cur)) {
			sprintf(hw, "serdes1:%s%s%s", mode, saved ? ";" : "",
				saved ? saved : "");
			set = !env_set("hwconfig", hw);
		}
		if (!set)
			printf("Carrier:       WARNING: cannot add serdes1:%s to ${hwconfig}\n",
			       mode);
	}

	lx2160a_serdes_apply_hwconfig();

	/* The DS250 retimers do not follow the SerDes on their own */
	if (nbxv3_serdes1_is("100g4x10g"))
		printf("Carrier:       serdes1:100g4x10g mixes 25.78125 and 10.3125 Gbd lanes, set each retimer CDR to the rate of its lane\n");

	/* Put ${hwconfig} back; NULL deletes it again if it was not set */
	if (set && env_set("hwconfig", saved))
		printf("Carrier:       WARNING: cannot restore ${hwconfig}\n");

	free(hw);
	free(saved);
}
