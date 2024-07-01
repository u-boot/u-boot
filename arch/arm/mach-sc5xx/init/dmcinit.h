/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#ifndef DMCINIT_H_
#define DMCINIT_H_

#include <config.h>

#ifdef MEM_MT41K512M16HA
	#include "mem/mt41k512m16ha.h"
#elif defined(MEM_MT41K128M16JT)
	#include "mem/mt41k128m16jt.h"
#elif defined(MEM_MT47H128M16RT)
	#include "mem/mt47h128m16rt.h"
#elif defined(MEM_IS43TR16512BL)
	#include "mem/is43tr16512bl.h"
#else
	#error "No DDR part name is defined for this board."
#endif

void DMC_Config(void);
void adi_dmc_reset_lanes(bool reset);

#endif
