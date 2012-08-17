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

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

/* Starting TLB number for the TLB entried for 3.5 G space created by ISBC */
#if defined(CONFIG_FSL_CORENET)
#define CONFIG_SYS_ISBC_START_TLB		3
#else
#define CONFIG_SYS_ISBC_START_TLB		0
#endif

/* Number fo TLB's created by ISBC */
#define CONFIG_SYS_ISBC_NUM_TLBS		5

#if defined(CONFIG_FSL_CORENET)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc0000000
#else
#define CONFIG_SYS_PBI_FLASH_BASE		0xce000000
#endif
#define CONFIG_SYS_PBI_FLASH_WINDOW		0xcff80000

#endif
