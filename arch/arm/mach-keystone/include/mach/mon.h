/*
 * K2HK: secure kernel command header file
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _MACH_MON_H_
#define _MACH_MON_H_

int mon_install(u32 addr, u32 dpsc, u32 freq);
int mon_power_on(int core_id, void *ep);
int mon_power_off(int core_id);

#endif
