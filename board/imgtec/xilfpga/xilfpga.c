/*
 * Imagination Technologies MIPSfpga platform code
 *
 * Copyright (C) 2016, Imagination Technologies Ltd.
 *
 * Zubair Lutfullah Kakakhel <Zubair.Kakakhel@imgtec.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#include <common.h>

/* initialize the DDR Controller and PHY */
phys_size_t initdram(int board_type)
{
	/* MIG IP block is smart and doesn't need SW
	 * to do any init */
	return CONFIG_SYS_SDRAM_SIZE;	/* in bytes */
}
