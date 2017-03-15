/*
 * Copyright 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Execution starts on the instruction following this 4-byte header
 * (containing the magic 'RK33').
 *
 * To make life easier for everyone, we build the SPL binary with
 * space for this 4-byte header already included in the binary.
 */

#ifdef CONFIG_SPL_BUILD
	.space 0x4         /* space for the 'RK33' */
#endif
	b reset
