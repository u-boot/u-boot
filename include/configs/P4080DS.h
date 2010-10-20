/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
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
 * P4080 DS board configuration file
 */
#define CONFIG_P4080DS
#define CONFIG_PHYS_64BIT
#define CONFIG_PPC_P4080
#define CONFIG_SYS_NUM_FMAN		2
#define CONFIG_SYS_NUM_FM1_DTSEC	4
#define CONFIG_SYS_NUM_FM2_DTSEC	4
#define CONFIG_SYS_NUM_FM1_10GEC	1
#define CONFIG_SYS_NUM_FM2_10GEC	1
#define CONFIG_NUM_DDR_CONTROLLERS	2

#define CONFIG_SYS_P4080_ERRATUM_CPU22
#define CONFIG_SYS_P4080_ERRATUM_SERDES8

#include "corenet_ds.h"
