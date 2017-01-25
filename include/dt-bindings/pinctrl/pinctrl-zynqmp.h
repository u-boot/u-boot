/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MIO pin configuration defines for Xilinx ZynqMP
 *
 * Copyright (C) 2017 Xilinx, Inc.
 * Author: Chirag Parekh <chirag.parekh@xilinx.com>
 */

#ifndef _DT_BINDINGS_PINCTRL_ZYNQMP_H
#define _DT_BINDINGS_PINCTRL_ZYNQMP_H

/* Bit value for IO standards */
#define IO_STANDARD_LVCMOS33      0
#define IO_STANDARD_LVCMOS18      1

/* Bit values for Slew Rates */
#define SLEW_RATE_FAST            0
#define SLEW_RATE_SLOW            1

/* Bit values for Pin inputs */
#define PIN_INPUT_TYPE_CMOS       0
#define PIN_INPUT_TYPE_SCHMITT    1

#endif /* _DT_BINDINGS_PINCTRL_ZYNQMP_H */
