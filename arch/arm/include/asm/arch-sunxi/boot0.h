/*
 * Configuration settings for the Allwinner A64 (sun50i) CPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* reserve space for BOOT0 header information */
	b	reset
	.space	1532
