/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Exception entry for LoongArch CPU
 *
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <linux/linkage.h>
#include <asm/asm.h>
#include <asm/loongarch.h>
#include <asm/stackframe.h>

.align 12
ENTRY(exception_entry)
	BACKUP_T0T1
	SAVE_ALL
	move		a0, sp
	la.pcrel	t0, do_exceptions
	jirl		ra, t0, 0
	RESTORE_ALL_AND_RET
END(exception_entry)
