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

#include <common.h>
#include <addr_map.h>

static struct {
	phys_addr_t paddr;
	phys_size_t size;
	unsigned long vaddr;
} address_map[CONFIG_SYS_NUM_ADDR_MAP];

phys_addr_t addrmap_virt_to_phys(void * vaddr)
{
	int i;

	for (i = 0; i < CONFIG_SYS_NUM_ADDR_MAP; i++) {
		u64 base, upper, addr;

		if (address_map[i].size == 0)
			continue;

		addr = (u64)((u32)vaddr);
		base = (u64)(address_map[i].vaddr);
		upper = (u64)(address_map[i].size) + base - 1;

		if (addr >= base && addr <= upper) {
			return addr - address_map[i].vaddr + address_map[i].paddr;
		}
	}

	return (phys_addr_t)(~0);
}

unsigned long addrmap_phys_to_virt(phys_addr_t paddr)
{
	int i;

	for (i = 0; i < CONFIG_SYS_NUM_ADDR_MAP; i++) {
		u64 base, upper, addr;

		if (address_map[i].size == 0)
			continue;

		addr = (u64)paddr;
		base = (u64)(address_map[i].paddr);
		upper = (u64)(address_map[i].size) + base - 1;

		if (addr >= base && addr <= upper) {
			return paddr - address_map[i].paddr + address_map[i].vaddr;
		}
	}

	return (unsigned long)(~0);
}

void addrmap_set_entry(unsigned long vaddr, phys_addr_t paddr,
			phys_size_t size, int idx)
{
	if (idx > CONFIG_SYS_NUM_ADDR_MAP)
		return;

	address_map[idx].vaddr = vaddr;
	address_map[idx].paddr = paddr;
	address_map[idx].size  = size;
}
