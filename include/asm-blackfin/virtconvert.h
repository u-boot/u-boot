/*
 * U-boot - virtconvert.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
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

#ifndef __BLACKFIN_VIRT_CONVERT__
#define __BLACKFIN_VIRT_CONVERT__

/*
 * Macros used for converting between virtual and physical mappings.
 */

#ifdef __KERNEL__

#include <linux/config.h>
#include <asm/setup.h>
#include <asm/page.h>

#define mm_vtop(vaddr)		((unsigned long) vaddr)
#define mm_ptov(vaddr)		((unsigned long) vaddr)
#define phys_to_virt(vaddr)	((unsigned long) vaddr)
#define virt_to_phys(vaddr)	((unsigned long) vaddr)

#define virt_to_bus		virt_to_phys
#define bus_to_virt		phys_to_virt

#endif
#endif
