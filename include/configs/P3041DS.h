/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
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
 * P3041 DS board configuration file
 *
 */
#define CONFIG_P3041DS
#define CONFIG_PHYS_64BIT
#define CONFIG_PPC_P3041

#define CONFIG_FSL_NGPIXIS		/* use common ngPIXIS code */

#define CONFIG_MMC
#define CONFIG_NAND_FSL_ELBC
#define CONFIG_FSL_SATA_V2
#define CONFIG_PCIE3
#define CONFIG_PCIE4
#define CONFIG_SYS_DPAA_RMAN

#define CONFIG_SYS_SRIO
#define CONFIG_SRIO1			/* SRIO port 1 */
#define CONFIG_SRIO2			/* SRIO port 2 */

#define CONFIG_ICS307_REFCLK_HZ		25000000  /* ICS307 ref clk freq */

#include "corenet_ds.h"
