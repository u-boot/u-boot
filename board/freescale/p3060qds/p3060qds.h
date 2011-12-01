/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
 */

#ifndef __P3060QDS_H__
#define __P3060QDS_H__

#include <asm/fsl_ddr_sdram.h>
#include <asm/u-boot.h>

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, bd_t *bd);
extern fixed_ddr_parm_t fixed_ddr_parm_0[];

#endif
