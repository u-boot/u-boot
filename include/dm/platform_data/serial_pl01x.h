/*
 * Copyright (c) 2014 Google, Inc
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __serial_pl01x_h
#define __serial_pl01x_h

enum pl01x_type {
	TYPE_PL010,
	TYPE_PL011,
};

/*
 *Information about a serial port
 *
 * @base: Register base address
 * @type: Port type
 * @clock: Input clock rate, used for calculating the baud rate divisor
 */
struct pl01x_serial_platdata {
	unsigned long base;
	enum pl01x_type type;
	unsigned int clock;
};

#endif
