/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef __ASM_RISCV_SYSTEM_H
#define __ASM_RISCV_SYSTEM_H

struct event;

/*
 * Interrupt configuring macros.
 *
 * TODO
 *
 */

/* Hook to set up the CPU (called from SPL too) */
int riscv_cpu_setup(void *ctx, struct event *event);

#endif	/* __ASM_RISCV_SYSTEM_H */
