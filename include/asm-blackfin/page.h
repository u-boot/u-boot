/*
 * U-boot -  page.h
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

#ifndef _BLACKFIN_PAGE_H
#define _BLACKFIN_PAGE_H

#include <linux/config.h>

/* PAGE_SHIFT determines the page size */

#define PAGE_SHIFT			(12)
#define PAGE_SIZE			(4096)
#define PAGE_MASK			(~(PAGE_SIZE-1))

#ifdef __KERNEL__

#include <asm/setup.h>

#if PAGE_SHIFT < 13
#define					KTHREAD_SIZE (8192)
#else
#define					KTHREAD_SIZE PAGE_SIZE
#endif

#ifndef __ASSEMBLY__

#define get_user_page(vaddr)		__get_free_page(GFP_KERNEL)
#define free_user_page(page, addr)	free_page(addr)

#define clear_page(page)		memset((page), 0, PAGE_SIZE)
#define copy_page(to,from)		memcpy((to), (from), PAGE_SIZE)

#define clear_user_page(page, vaddr)	clear_page(page)
#define copy_user_page(to, from, vaddr)	copy_page(to, from)

/*
 * These are used to make use of C type-checking..
 */
typedef struct {
	unsigned long pte;
} pte_t;
typedef struct {
	unsigned long pmd[16];
} pmd_t;
typedef struct {
	unsigned long pgd;
} pgd_t;
typedef struct {
	unsigned long pgprot;
} pgprot_t;

#define pte_val(x)			((x).pte)
#define pmd_val(x)			((&x)->pmd[0])
#define pgd_val(x)			((x).pgd)
#define pgprot_val(x)			((x).pgprot)

#define __pte(x)			((pte_t) { (x) } )
#define __pmd(x)			((pmd_t) { (x) } )
#define __pgd(x)			((pgd_t) { (x) } )
#define __pgprot(x)			((pgprot_t) { (x) } )

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)		(((addr)+PAGE_SIZE-1)&PAGE_MASK)

/* Pure 2^n version of get_order */
extern __inline__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
	do {
		size >>= 1;
		order++;
	} while (size);
	return order;
}

#endif	/* !__ASSEMBLY__ */

#include <asm/page_offset.h>

#define PAGE_OFFSET			(PAGE_OFFSET_RAW)

#ifndef __ASSEMBLY__

#define __pa(vaddr)			virt_to_phys((void *)vaddr)
#define __va(paddr)			phys_to_virt((unsigned long)paddr)

#define MAP_NR(addr)			(((unsigned long)(addr)-PAGE_OFFSET) >> PAGE_SHIFT)
#define virt_to_page(addr)		(mem_map + (((unsigned long)(addr)-PAGE_OFFSET) >> PAGE_SHIFT))
#define VALID_PAGE(page)		((page - mem_map) < max_mapnr)

#define BUG() do	{ \
	 \
	while (1);	/* dead-loop */ \
} while (0)

#define PAGE_BUG(page) do	{ \
	BUG(); \
} while (0)

#endif

#endif

#endif
