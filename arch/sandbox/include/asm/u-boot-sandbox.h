/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _U_BOOT_SANDBOX_H_
#define _U_BOOT_SANDBOX_H_

/* board/.../... */
int board_init(void);
int dram_init(void);

/* start.c */
int sandbox_early_getopt_check(void);
int sandbox_main_loop_init(void);

#endif	/* _U_BOOT_SANDBOX_H_ */
