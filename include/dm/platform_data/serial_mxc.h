/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014 Google, Inc
 */

#ifndef __serial_mxc_h
#define __serial_mxc_h

/* Information about a serial port */
struct mxc_serial_plat {
	struct mxc_uart *reg;  /* address of registers in physical memory */
#if CONFIG_IS_ENABLED(CLK_CCF)
	struct clk_bulk clks;
#endif
	bool use_dte;
};

#endif
