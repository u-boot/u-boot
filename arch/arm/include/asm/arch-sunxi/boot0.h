/*
 * Configuration settings for the Allwinner A64 (sun50i) CPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BOOT0_H
#define __BOOT0_H

/* reserve space for BOOT0 header information */
#define ARM_SOC_BOOT0_HOOK	\
	.space	1532

#endif /* __BOOT0_H */
