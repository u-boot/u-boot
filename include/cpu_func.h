/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Copyright 2019 Google LLC
 */

#ifndef __CPU_LEGACY_H
#define __CPU_LEGACY_H

#include <linux/types.h>

/*
 * Multicore arch functions
 *
 * These should be moved to use the CPU uclass.
 */
int cpu_status(u32 nr);
int cpu_reset(u32 nr);
int cpu_disable(u32 nr);
int cpu_release(u32 nr, int argc, char * const argv[]);

#endif
