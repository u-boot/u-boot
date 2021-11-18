/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MIO pin configuration defines for Xilinx ZynqMP
 *
 * Copyright (C) 2020 Xilinx, Inc.
 */

#ifndef _DT_BINDINGS_PINCTRL_ZYNQMP_H
#define _DT_BINDINGS_PINCTRL_ZYNQMP_H

/* Bit value for different voltage levels */
#define IO_STANDARD_LVCMOS33	0
#define IO_STANDARD_LVCMOS18	1

/* Bit values for Slew Rates */
#define SLEW_RATE_FAST		0
#define SLEW_RATE_SLOW		1

/* Bit values for Pin drive strength */
#define DRIVE_STRENGTH_2MA	2
#define DRIVE_STRENGTH_4MA	4
#define DRIVE_STRENGTH_8MA	8
#define DRIVE_STRENGTH_12MA	12

#endif /* _DT_BINDINGS_PINCTRL_ZYNQMP_H */
