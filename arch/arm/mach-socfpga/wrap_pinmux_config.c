/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <common.h>
#include <errno.h>
/*
 * Yes, dear reader, we're including a C file here, this is no mistake.
 * But this time around, we do even more perverse hacking here to be
 * compatible with QTS headers and obtain reasonably nice results too.
 *
 * First, we define _PRELOADER_PINMUX_CONFIG_H_, which will neutralise
 * the pinmux_config.h inclusion in pinmux_config.c . Since we are
 * probing everything from DT, we do NOT want those macros from the
 * pinmux_config.h to ooze into our build system, anywhere, ever. So
 * we nip it at the bud.
 *
 * Next, pinmux_config.c needs CONFIG_HPS_PINMUX_NUM and uses it to
 * specify sized array explicitly. Instead, we want to use ARRAY_SIZE
 * to figure out the size of the array, so define this macro as an
 * empty one, so that the preprocessor optimizes things such that the
 * arrays are not sized by default.
 */
#define _PRELOADER_PINMUX_CONFIG_H_
#define CONFIG_HPS_PINMUX_NUM
#include <qts/pinmux_config.c>

void sysmgr_get_pinmux_table(const unsigned long **table,
			     unsigned int *table_len)
{
	*table = sys_mgr_init_table;
	*table_len = ARRAY_SIZE(sys_mgr_init_table);
}
