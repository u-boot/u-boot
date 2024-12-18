/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#ifndef __SANDBOX_CPU_H
#define __SANDBOX_CPU_H

void cpu_sandbox_set_current(const char *name);

/* show the mapping of sandbox addresses to pointers */
void sandbox_map_list(void);

#endif /* __SANDBOX_CPU_H */
