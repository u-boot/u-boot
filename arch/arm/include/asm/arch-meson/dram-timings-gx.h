/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024, Ferass El Hafidi <funderscore@postmarketos.org>
 */
#ifndef DRAM_TIMINGS_GX_H
#define DRAM_TIMINGS_GX_H

/*
 * DRAM timings
 * It looks like those are quite similar in regular boards based on reference
 * designs and not using counterfeit RAM chips. Those are hacked around by lowbin
 * TV box vendors to support lowbin RAM chips, however. Here, we are hardcoding
 * timings, which *will* cause issues on lowbin boards, but should be fine on other
 * boards derived from Amlogic reference designs.
 */

/*
 * TODO:
 * - Add timings for different DRAM clocks
 * - Support overwriting those if board needs different timings (how?)
 * - Other things
 */

#if defined(CONFIG_DRAM_DDR3)
/* DDR3: 912 MHz */
const struct meson_gx_dram_timings timings = {
	.drv        = 0,
	.odt        = 2,

	/* Timings */
	.rtp        = 0x7,
	.wtr        = 0x7,
	.rp         = 0xd,
	.rcd        = 0xd,
	.ras        = 0x25,
	.rrd        = 0x7,
	.rc         = 0x34,
	.mrd        = 0x6, /* Should be < 8 */
	.mod        = 0x4,
	.faw        = 0x21,
	.wlmrd      = 0x28,
	.wlo        = 0x7,
	.rfc        = 0x118,
	.xp         = 0x7,
	.xs         = 0x200,
	.dllk       = 0x200,
	.cke        = 0x5,
	.rtodt      = 0x0,
	.rtw        = 0x7,
	.refi       = 0x4e,
	.refi_mddr3 = 0x4,
	.cl         = 0xd,
	.wr         = 0x10,
	.cwl        = 0x9,
	.al         = 0x0,
	.dqs        = 0x17,
	.cksre      = 0xf,
	.cksrx      = 0xf,
	.zqcs       = 0x40,
	.xpdll      = 0x17,
	.exsr       = 0x200, /* Should be < 0x3ff */
	.zqcl       = 0x88,
	.zqcsi      = 0x3e8,
	.rpab       = 0x0,
	.rppb       = 0x0,
	.tdqsck     = 0x0,
	.tdqsckmax  = 0x0,
	.tckesr     = 0x0,
	.tdpd       = 0x0,
	.taond_aofd = 0x0,
	.tccdl      = 0, /* Unused on GXBB */
};

#elif defined(CONFIG_DRAM_DDR4)
/* DDR4: 1080 MHz */
const struct meson_gx_dram_timings timings = {
	.drv        = 1,
	.odt        = 1,

	/* Timings */
	.rtp        = 9,
	.wtr        = 9,
	.rp         = 0x10, // ddr_clk < 1200
	.rcd        = 0x10, // ddr_clk < 1200
	.ras        = 35 * 1.2,
	.rrd        = 6,
	.rc         = 0x3a,
	.mrd        = 8,
	.mod        = 24,
	.faw        = 35 * 1.2,
	.rfc        = 350 * 1.2,
	.wlmrd      = 40,
	.wlo        = 9.5 * 1.2,
	.xs         = 512,
	.xp         = 7,
	.cke        = 5,
	.dllk       = 1024,
	.rtodt      = 0,
	.rtw        = 8,
	.refi       = 76,
	.refi_mddr3 = 4,
	.cl         = 0x10, // ddr_clk < 1200
	.wr         = 0x12,
	.cwl        = 12,
	.al         = 0,
	.exsr       = 1024,
	.dqs        = 9,
	.cksre      = 15,
	.cksrx      = 15,
	.zqcs       = 128,
	.zqcl       = 256,
	.xpdll      = 23,
	.zqcsi      = 1000,
	.tccdl      = 6, // ddr_clk < 1200
};
#endif
#endif
