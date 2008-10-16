/*
 * Basic Flash Driver for Freescale MCF 5282 internal FLASH
 *
 * (C) Copyright 2005 BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
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
 */

#ifndef __CFM_FLASH_H_
#define __CFM_FLASH_H_

#define	FREESCALE_MANUFACT 0xFACFFACF
#define	FREESCALE_ID_MCF5281 0x5281
#define	FREESCALE_ID_MCF5282 0x5282

extern void cfm_flash_print_info (flash_info_t * info);
extern int cfm_flash_erase_sector (flash_info_t * info, int sector);
extern void cfm_flash_init (flash_info_t * info);
extern int cfm_flash_write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt);
#ifdef CONFIG_SYS_FLASH_PROTECTION
extern int cfm_flash_protect(flash_info_t * info,long sector,int prot);
#endif

#endif
