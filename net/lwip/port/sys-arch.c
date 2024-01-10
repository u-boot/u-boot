// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <common.h>
#include "lwip/opt.h"

u32_t sys_now(void)
{
	return get_timer(0);
}
