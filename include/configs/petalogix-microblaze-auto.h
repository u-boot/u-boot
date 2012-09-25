/*
 * (C) Copyright 2007-2008 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
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

#ifndef __CONFIG_H
#define __CONFIG_H

#include "../board/petalogix/microblaze-auto/xparameters.h"

#define	CONFIG_MICROBLAZE	1	/* MicroBlaze CPU */
#define	MICROBLAZE_V5		1

/* Memory test handling */
#define	CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define	CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x1000)

/* global pointer */
/* start of global data */
#define	CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_SDRAM_SIZE - GENERATED_GBL_DATA_SIZE)

/* monitor code */
#define	SIZE			0x40000
#define	CONFIG_SYS_MONITOR_LEN		SIZE
#define	CONFIG_SYS_MONITOR_BASE  (CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_GBL_DATA_OFFSET - CONFIG_SYS_MONITOR_LEN - GENERATED_BD_INFO_SIZE)

#define	CONFIG_SYS_MONITOR_END		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define	CONFIG_SYS_MALLOC_LEN		SIZE
#define	CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MONITOR_BASE - CONFIG_SYS_MALLOC_LEN)

/* stack */
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_MALLOC_BASE

#define   CONFIG_SYS_BOOTMAPSZ	(1 << 31) /* Initial Memory map for Linux */

#undef CONFIG_PHYLIB
#define CONFIG_LMB		1

/* Default cache size if not specified */
#ifndef XILINX_DCACHE_BYTE_SIZE
# define XILINX_DCACHE_BYTE_SIZE	32768
#endif

/* Get common PetaLinux board setup */
#include <configs/petalinux-auto-board.h>
#endif	/* __CONFIG_H */
