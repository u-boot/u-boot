/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

typedef int (*init_fnc_t)(void);

int initcall_run_list(init_fnc_t init_sequence[]);
