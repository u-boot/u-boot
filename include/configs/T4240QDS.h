/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * T4240 QDS board configuration file
 */
#define CONFIG_T4240QDS
#define CONFIG_PHYS_64BIT
#define CONFIG_PPC_T4240

#define CONFIG_FSL_SATA_V2
#define CONFIG_PCIE4

#define CONFIG_ICS307_REFCLK_HZ		25000000  /* ICS307 ref clk freq */

#include "t4qds.h"
