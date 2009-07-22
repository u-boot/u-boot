/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *
 * This file contains routines that fetch data from ARM-dependent sources
 * (bd_info etc.)
 *
 */

#include <config.h>
#include <linux/types.h>
#include <api_public.h>

#include <asm/u-boot.h>
#include <asm/global_data.h>

#include "api_private.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Important notice: handling of individual fields MUST be kept in sync with
 * include/asm-arm/u-boot.h and include/asm-arm/global_data.h, so any changes
 * need to reflect their current state and layout of structures involved!
 */
int platform_sys_info(struct sys_info *si)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		platform_set_mr(si, gd->bd->bi_dram[i].start,
				gd->bd->bi_dram[i].size, MR_ATTR_DRAM);

	return 1;
}
