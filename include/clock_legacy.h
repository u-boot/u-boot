/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __CLOCK_LEGACY_H
#define __CLOCK_LEGACY_H

int get_clocks(void);
unsigned long get_bus_freq(unsigned long dummy);
int get_serial_clock(void);

#endif
