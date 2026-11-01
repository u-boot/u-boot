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
 *   2. the RCW build stamp in DCFG SCRATCHRW13, see below
 *   3. NBXV3_CARRIER_DEFAULT
 */

#include <env.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <vsprintf.h>
#include <asm/io.h>
#include <asm/arch/immap_lsch3.h>
#include <asm/arch/soc.h>

#include "carrier.h"

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
 *   4..F  unassigned, treated like 0
 */
#define NBXV3_STAMP_SCRATCHRW		12	/* SCRATCHRW13 */
#define NBXV3_STAMP_CARRIER		GENMASK(31, 28)
#define NBXV3_STAMP_TAG			GENMASK(27, 24)
#define NBXV3_STAMP_TAG_NBXV3		0xb

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

void nbxv3_carrier_env_init(void)
{
	const char *carrier = env_get("carrier");
	const char *origin;
	int digit;

	if (carrier && *carrier) {
		printf("Carrier:       %s (from env)\n", carrier);
		return;
	}

	digit = nbxv3_stamp_carrier();
	carrier = nbxv3_carrier_detect(digit);
	if (carrier) {
		origin = "RCW build stamp";
	} else {
		carrier = NBXV3_CARRIER_DEFAULT;
		origin = digit < 0 ? "default, no RCW build stamp" :
				     "default, the RCW names no carrier";
	}

	if (env_set("carrier", carrier)) {
		printf("Carrier:       WARNING: cannot set ${carrier}\n");
		return;
	}

	/* Not saved on purpose: saveenv on every boot would burn the NOR */
	printf("Carrier:       %s (%s, not saved; setenv carrier <name> && saveenv to pin)\n",
	       carrier, origin);
}
