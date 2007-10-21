/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996 by Ralf Baechle
 * Copyright (C) 2000 by Maciej W. Rozycki
 *
 * Defitions for the address spaces of the MIPS CPUs.
 */
#ifndef __ASM_MIPS_ADDRSPACE_H
#define __ASM_MIPS_ADDRSPACE_H

/*
 * Memory segments (32bit kernel mode addresses)
 */
#define KUSEG			0x00000000
#define KSEG0			0x80000000
#define KSEG1			0xa0000000
#define KSEG2			0xc0000000
#define KSEG3			0xe0000000

#define K0BASE	KSEG0

/*
 * Returns the kernel segment base of a given address
 */
#ifndef __ASSEMBLY__
#define KSEGX(a)		(((unsigned long)(a)) & 0xe0000000)
#else
#define KSEGX(a)		((a) & 0xe0000000)
#endif

/*
 * Returns the physical address of a KSEG0/KSEG1 address
 */
#ifndef __ASSEMBLY__
#define PHYSADDR(a)		(((unsigned long)(a)) & 0x1fffffff)
#else
#define PHYSADDR(a)		((a) & 0x1fffffff)
#endif

/*
 * Returns the uncached address of a sdram address
 */
#ifndef __ASSEMBLY__
#if defined(CONFIG_AU1X00) || defined(CONFIG_TB0229)
/* We use a 36 bit physical address map here and
   cannot access physical memory directly from core */
#define UNCACHED_SDRAM(a) (((unsigned long)(a)) | 0x20000000)
#else	/* !CONFIG_AU1X00 */
#define UNCACHED_SDRAM(a) KSEG1ADDR(a)
#endif	/* CONFIG_AU1X00 */
#endif	/* __ASSEMBLY__ */
/*
 * Map an address to a certain kernel segment
 */
#ifndef __ASSEMBLY__
#define KSEG0ADDR(a)		((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG0))
#define KSEG1ADDR(a)		((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG1))
#define KSEG2ADDR(a)		((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG2))
#define KSEG3ADDR(a)		((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG3))
#else
#define KSEG0ADDR(a)		(((a) & 0x1fffffff) | KSEG0)
#define KSEG1ADDR(a)		(((a) & 0x1fffffff) | KSEG1)
#define KSEG2ADDR(a)		(((a) & 0x1fffffff) | KSEG2)
#define KSEG3ADDR(a)		(((a) & 0x1fffffff) | KSEG3)
#endif

/*
 * Memory segments (64bit kernel mode addresses)
 */
#define XKUSEG			0x0000000000000000
#define XKSSEG			0x4000000000000000
#define XKPHYS			0x8000000000000000
#define XKSEG			0xc000000000000000
#define CKSEG0			0xffffffff80000000
#define CKSEG1			0xffffffffa0000000
#define CKSSEG			0xffffffffc0000000
#define CKSEG3			0xffffffffe0000000

#endif /* __ASM_MIPS_ADDRSPACE_H */
