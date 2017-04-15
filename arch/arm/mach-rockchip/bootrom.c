/**
 * Copyright (c) 2017 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/bootrom.h>

void back_to_bootrom(void)
{
#if defined(CONFIG_SPL_LIBGENERIC_SUPPORT) && !defined(CONFIG_TPL_BUILD)
	printf("Returning to boot ROM...");
#endif
	_back_to_bootrom_s();
}
