/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

/*
 * Create a dummy LAW entry for the DDR SDRAM which will be replaced when
 * the DDR SPD setup code runs.
 *
 * This table would be empty, except that it is used before the BSS section is
 * initialized, and therefore must have at least one entry to push it into
 * the DATA section.
 */
struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_SDRAM_BASE, LAW_SIZE_4K, LAW_TRGT_IF_DDR),
};

int num_law_entries = ARRAY_SIZE(law_table);
