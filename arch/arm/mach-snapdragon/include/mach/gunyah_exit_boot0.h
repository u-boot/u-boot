/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Early Gunyah Hypervisor Exit for Dragonwing Family Platforms
 *
 * Exit Gunyah hypervisor and switch to EL2 during early boot on Qualcomm
 * Dragonwing family platforms. This must happen before EL register
 * configuration in start.S so that U-Boot can run properly at EL2 to
 * support hypervisors requiring EL2 privilege level.
 */

#include <asm/macro.h>
#include <linux/arm-smccc.h>

/* TrustZone SMC IDs for hypervisor configuration */
#define TZ_EL2_SWITCH_SMC_ID			0x02000121
#define TZ_EL2_SWITCH_PARAM_ID			0x00000023
#define TZ_EL2_SWITCH_PARAM2_EXIT_GUNYAH	0x1

	/* Only perform hypervisor switch if we're at EL1 */
	switch_el x9, 3f, 2f, 1f

	/* Save FDT address before we modify x0 */
1:	mov	x9, x0

	/* Switch to EL2 (exit Gunyah) */
	ldr	w0, =TZ_EL2_SWITCH_SMC_ID
	ldr	w1, =TZ_EL2_SWITCH_PARAM_ID
	mov	w2, wzr
	mov	w3, wzr
	ldr	w4, =TZ_EL2_SWITCH_PARAM2_EXIT_GUNYAH
	smc	#0

	/* Restore FDT address */
	mov	x0, x9
2:
3:	b	reset
