/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef _U_BOOT_LOONGARCH_H_
#define _U_BOOT_LOONGARCH_H_

/* cpu/.../cpu.c */
int cleanup_before_linux(void);

/* board/.../... */
int board_init(void);
void board_quiesce_devices(void);

#endif
