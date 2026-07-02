// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <command.h>
#include <cpu.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/lists.h>
#include <event.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <asm/system.h>
#include <dm/uclass-internal.h>
#include <linux/bitops.h>

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
	printf("resetting ...\n");

	printf("reset not supported yet\n");
	hang();
}
#endif
