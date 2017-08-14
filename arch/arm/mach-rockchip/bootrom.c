/**
 * Copyright (c) 2017 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/bootrom.h>

void back_to_bootrom(void)
{
#if CONFIG_IS_ENABLED(LIBCOMMON_SUPPORT)
	puts("Returning to boot ROM...\n");
#endif
	_back_to_bootrom_s();
}
