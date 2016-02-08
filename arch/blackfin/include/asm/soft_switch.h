/*
 * U-Boot - main board file
 *
 * Copyright (c) 2008-2012 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __SOFT_SWITCH_H__
#define __SOFT_SWITCH_H__

#define IO_PORT_A              0
#define IO_PORT_B              1
#define IO_PORT_INPUT          0
#define IO_PORT_OUTPUT         1

int config_switch_bit(int num, int port, int bit, int dir, uchar value);
#endif
