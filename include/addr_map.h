#ifndef __ADDR_MAP_H
#define __ADDR_MAP_H

/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#include <asm/types.h>

extern phys_addr_t addrmap_virt_to_phys(void *vaddr);
extern unsigned long addrmap_phys_to_virt(phys_addr_t paddr);
extern void addrmap_set_entry(unsigned long vaddr, phys_addr_t paddr,
				phys_size_t size, int idx);

#endif
