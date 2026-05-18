// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <linux/types.h>

#include "tq_bb.h"

int __weak tq_bb_board_mmc_getwp(struct mmc *mmc)
{
	return 0;
}

int __weak tq_bb_board_mmc_getcd(struct mmc *mmc)
{
	return 0;
}

int __weak tq_bb_board_mmc_init(struct bd_info *bis)
{
	return 0;
}

int __weak tq_bb_board_early_init_f(void)
{
	return 0;
}

int __weak tq_bb_board_init(void)
{
	return 0;
}

int __weak tq_bb_board_late_init(void)
{
	return 0;
}

int __weak tq_bb_checkboard(void)
{
	return 0;
}

void __weak tq_bb_board_quiesce_devices(void)
{
	;
}

const char * __weak tq_bb_get_boardname(void)
{
	return "INVALID";
}

#if IS_ENABLED(CONFIG_SPL_BUILD)
void __weak tq_bb_board_init_f(ulong dummy)
{
	;
}

void __weak tq_bb_spl_board_init(void)
{
	;
}
#endif /* IS_ENABLED(CONFIG_SPL_BUILD) */

/*
 * Device Tree Support
 */
#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)
int __weak tq_bb_ft_board_setup(void *blob, struct bd_info *bis)
{
	return 0;
}

#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT) */
