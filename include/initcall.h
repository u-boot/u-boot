/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __INITCALL_H
#define __INITCALL_H

typedef int (*init_fnc_t)(void);

int initcall_run_list(const init_fnc_t init_sequence[]);

#endif
