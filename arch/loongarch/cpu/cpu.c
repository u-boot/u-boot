// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 * Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include <hang.h>
#include <stdio.h>

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
	printf("reset not supported yet\n");
	hang();
}
#endif
