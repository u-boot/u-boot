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
 * For testing, we can enforce, change it from the prompt and make it persistent:
 *   setenv carrier nbv32 && saveenv && reset
 */

#include <env.h>
#include <linux/kernel.h>
#include <vsprintf.h>

#include "carrier.h"

#ifndef NBXV3_CARRIER_DEFAULT
#define NBXV3_CARRIER_DEFAULT "nbv30"
#endif

/*
 * Placeholder for strap/EEPROM based identification.
 * Returning NULL means "cannot tell".
 */
static const char *nbxv3_carrier_detect(void)
{
	/* TODO, not available yet */
	return NULL;
}

void nbxv3_carrier_env_init(void)
{
	const char *carrier = env_get("carrier");
	const char *detected;

	if (carrier && *carrier) {
		printf("Carrier:       %s (from env)\n", carrier);
		return;
	}

	detected = nbxv3_carrier_detect();
	if (!detected)
		detected = NBXV3_CARRIER_DEFAULT;

	if (env_set("carrier", detected)) {
		printf("Carrier:       WARNING: cannot set ${carrier}\n");
		return;
	}

	/* Not saved on purpose: saveenv on every boot would burn the NOR */
	printf("Carrier:       %s (default, not saved; setenv carrier <name> && saveenv to pin)\n",
	       detected);
}
