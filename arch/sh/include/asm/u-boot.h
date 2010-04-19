/*
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
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __ASM_SH_U_BOOT_H_
#define __ASM_SH_U_BOOT_H_

typedef struct bd_info {
	unsigned long   bi_memstart;    /* start of DRAM memory */
	phys_size_t	bi_memsize;     /* size  of DRAM memory in bytes */
	unsigned long   bi_flashstart;  /* start of FLASH memory */
	unsigned long   bi_flashsize;   /* size  of FLASH memory */
	unsigned long   bi_flashoffset; /* reserved area for startup monitor */
	unsigned long   bi_sramstart;   /* start of SRAM memory */
	unsigned long   bi_sramsize;    /* size  of SRAM memory */
	unsigned long   bi_ip_addr;     /* IP Address */
	unsigned long   bi_baudrate;    /* Console Baudrate */
	unsigned long	bi_boot_params; /* where this board expects params */
} bd_t;

#endif
