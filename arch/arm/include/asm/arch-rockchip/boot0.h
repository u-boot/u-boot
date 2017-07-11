
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
	/*
	 * We need to add 4 bytes of space for the 'RK33' at the
	 * beginning of the executable.	 However, as we want to keep
	 * this generic and make it applicable to builds that are like
	 * the RK3368 (TPL needs this, SPL doesn't) or the RK3399 (no
	 * TPL, but extra space needed in the SPL), we simply repeat
	 * the 'b reset' with the expectation that the first one will
	 * be overwritten, if this is the first stage contained in the
	 * final image created with mkimage)...
	 */
	b reset	 /* may be overwritten --- should be 'nop' or a 'b reset' */
#endif
	b reset

#if defined(CONFIG_ROCKCHIP_RK3399) && defined(CONFIG_SPL_BUILD)
	.space CONFIG_ROCKCHIP_SPL_RESERVE_IRAM	/* space for the ATF data */
#endif
