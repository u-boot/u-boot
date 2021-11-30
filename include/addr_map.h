/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ADDR_MAP_H
#define __ADDR_MAP_H

/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

#include <asm/types.h>

struct addrmap {
	phys_addr_t paddr;
	phys_size_t size;
	unsigned long vaddr;
};

extern struct addrmap address_map[CONFIG_SYS_NUM_ADDR_MAP];

phys_addr_t addrmap_virt_to_phys(void *vaddr);
void *addrmap_phys_to_virt(phys_addr_t paddr);
void addrmap_set_entry(unsigned long vaddr, phys_addr_t paddr,
		       phys_size_t size, int idx);

#endif
