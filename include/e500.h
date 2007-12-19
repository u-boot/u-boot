/*
 * Copyright 2003 Motorola,Inc.
 * Xianghua Xiao(x.xiao@motorola.com)
 */

#ifndef	__E500_H__
#define __E500_H__

#ifndef __ASSEMBLY__

typedef struct
{
  unsigned long freqProcessor;
  unsigned long freqSystemBus;
  unsigned long freqDDRBus;
} MPC85xx_SYS_INFO;

#endif  /* _ASMLANGUAGE */

/* Motorola E500 core provides 16 TLB1 entries; they can be used for
 * initial memory mapping like legacy BAT registers do. Usually we
 * use four MAS registers(MAS0-3) to operate on TLB1 entries.
 *
 * While there are 16 Entries with variable Page Sizes in TLB1,
 * there are also 256 Entries with fixed 4K pages in TLB0.
 *
 * We also need LAWs(Local Access Window) to associate a range of
 * the local 32-bit address space with a particular target interface
 * such as PCI/PCI-X, RapidIO, Local Bus and DDR SDRAM.
 *
 * We put TLB1/LAW code here because memory mapping is board-specific
 * instead of cpu-specific.
 *
 * While these macros are all nominally for TLB1 by name, they can
 * also be used for TLB0 as well.
 */


/*
 * Convert addresses to Effective and Real Page Numbers.
 * Grab the high 20-bits and shift 'em down, dropping the "byte offset".
 */
#define E500_TLB_EPN(addr)	(((addr) >> 12) & 0xfffff)
#define E500_TLB_RPN(addr)	(((addr) >> 12) & 0xfffff)


/* MAS0
 * tlbsel(TLB Select):0,1
 * esel(Entry Select): 0,1,2,...,15 for TLB1
 * nv(Next victim):0,1
 */
#define TLB1_MAS0(tlbsel,esel,nv) \
			(MAS0_TLBSEL(tlbsel) | MAS0_ESEL(esel) | MAS0_NV(nv))

/* MAS1
 * v(TLB valid bit):0,1
 * iprot(invalidate protect):0,1
 * tid(translation identity):8bit to match process IDs
 * ts(translation space,comparing with MSR[IS,DS]): 0,1
 * tsize(translation size):1,2,...,9(4K,16K,64K,256K,1M,4M,16M,64M,256M)
 */
#define TLB1_MAS1(v,iprot,tid,ts,tsize) \
			((((v) << 31) & MAS1_VALID)             |\
			(((iprot) << 30) & MAS1_IPROT)          |\
			(MAS1_TID(tid))				|\
			(((ts) << 12) & MAS1_TS)                |\
			(MAS1_TSIZE(tsize)))

/* MAS2
 * epn(effective page number):20bits
 * sharen(Shared cache state):0,1
 * x0,x1(implementation specific page attribute):0,1
 * w,i,m,g,e(write-through,cache-inhibited,memory coherency,guarded,
 *      endianness):0,1
 */
#define TLB1_MAS2(epn,sharen,x0,x1,w,i,m,g,e) \
			((((epn) << 12) & MAS2_EPN)             |\
			(((x0) << 6) & MAS2_X0)                 |\
			(((x1) << 5) & MAS2_X1)                 |\
			(((w) << 4) & MAS2_W)                   |\
			(((i) << 3) & MAS2_I)                   |\
			(((m) << 2) & MAS2_M)                   |\
			(((g) << 1) & MAS2_G)                   |\
			(e) )

/* MAS3
 * rpn(real page number):20bits
 * u0-u3(user bits, useful for page table management in OS):0,1
 * ux,sx,uw,sw,ur,sr(permission bits, user and supervisor read,
 *      write,execute permission).
 */
#define TLB1_MAS3(rpn,u0,u1,u2,u3,ux,sx,uw,sw,ur,sr) \
			((((rpn) << 12) & MAS3_RPN)             |\
			(((u0) << 9) & MAS3_U0)                 |\
			(((u1) << 8) & MAS3_U1)                 |\
			(((u2) << 7) & MAS3_U2)                 |\
			(((u3) << 6) & MAS3_U3)                 |\
			(((ux) << 5) & MAS3_UX)                 |\
			(((sx) << 4) & MAS3_SX)                 |\
			(((uw) << 3) & MAS3_UW)                 |\
			(((sw) << 2) & MAS3_SW)                 |\
			(((ur) << 1) & MAS3_UR)                 |\
			(sr) )


#define RESET_VECTOR	0xfffffffc
#define CACHELINE_MASK	(CFG_CACHELINE_SIZE - 1) /* Address mask for cache
						     line aligned data. */

#endif	/* __E500_H__ */
