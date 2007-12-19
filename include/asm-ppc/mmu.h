/*
 * PowerPC memory management structures
 */

#ifndef _PPC_MMU_H_
#define _PPC_MMU_H_

#include <linux/config.h>

#ifndef __ASSEMBLY__
/* Hardware Page Table Entry */
typedef struct _PTE {
#ifdef CONFIG_PPC64BRIDGE
	unsigned long long vsid:52;
	unsigned long api:5;
	unsigned long :5;
	unsigned long h:1;
	unsigned long v:1;
	unsigned long long rpn:52;
#else /* CONFIG_PPC64BRIDGE */
	unsigned long v:1;	/* Entry is valid */
	unsigned long vsid:24;	/* Virtual segment identifier */
	unsigned long h:1;	/* Hash algorithm indicator */
	unsigned long api:6;	/* Abbreviated page index */
	unsigned long rpn:20;	/* Real (physical) page number */
#endif /* CONFIG_PPC64BRIDGE */
	unsigned long    :3;	/* Unused */
	unsigned long r:1;	/* Referenced */
	unsigned long c:1;	/* Changed */
	unsigned long w:1;	/* Write-thru cache mode */
	unsigned long i:1;	/* Cache inhibited */
	unsigned long m:1;	/* Memory coherence */
	unsigned long g:1;	/* Guarded */
	unsigned long  :1;	/* Unused */
	unsigned long pp:2;	/* Page protection */
} PTE;

/* Values for PP (assumes Ks=0, Kp=1) */
#define PP_RWXX	0	/* Supervisor read/write, User none */
#define PP_RWRX 1	/* Supervisor read/write, User read */
#define PP_RWRW 2	/* Supervisor read/write, User read/write */
#define PP_RXRX 3	/* Supervisor read,       User read */

/* Segment Register */
typedef struct _SEGREG {
	unsigned long t:1;	/* Normal or I/O  type */
	unsigned long ks:1;	/* Supervisor 'key' (normally 0) */
	unsigned long kp:1;	/* User 'key' (normally 1) */
	unsigned long n:1;	/* No-execute */
	unsigned long :4;	/* Unused */
	unsigned long vsid:24;	/* Virtual Segment Identifier */
} SEGREG;

/* Block Address Translation (BAT) Registers */
typedef struct _P601_BATU {	/* Upper part of BAT for 601 processor */
	unsigned long bepi:15;	/* Effective page index (virtual address) */
	unsigned long :8;	/* unused */
	unsigned long w:1;
	unsigned long i:1;	/* Cache inhibit */
	unsigned long m:1;	/* Memory coherence */
	unsigned long ks:1;	/* Supervisor key (normally 0) */
	unsigned long kp:1;	/* User key (normally 1) */
	unsigned long pp:2;	/* Page access protections */
} P601_BATU;

typedef struct _BATU {		/* Upper part of BAT (all except 601) */
#ifdef CONFIG_PPC64BRIDGE
	unsigned long long bepi:47;
#else /* CONFIG_PPC64BRIDGE */
	unsigned long bepi:15;	/* Effective page index (virtual address) */
#endif /* CONFIG_PPC64BRIDGE */
	unsigned long :4;	/* Unused */
	unsigned long bl:11;	/* Block size mask */
	unsigned long vs:1;	/* Supervisor valid */
	unsigned long vp:1;	/* User valid */
} BATU;

typedef struct _P601_BATL {	/* Lower part of BAT for 601 processor */
	unsigned long brpn:15;	/* Real page index (physical address) */
	unsigned long :10;	/* Unused */
	unsigned long v:1;	/* Valid bit */
	unsigned long bl:6;	/* Block size mask */
} P601_BATL;

typedef struct _BATL {		/* Lower part of BAT (all except 601) */
#ifdef CONFIG_PPC64BRIDGE
	unsigned long long brpn:47;
#else /* CONFIG_PPC64BRIDGE */
	unsigned long brpn:15;	/* Real page index (physical address) */
#endif /* CONFIG_PPC64BRIDGE */
	unsigned long :10;	/* Unused */
	unsigned long w:1;	/* Write-thru cache */
	unsigned long i:1;	/* Cache inhibit */
	unsigned long m:1;	/* Memory coherence */
	unsigned long g:1;	/* Guarded (MBZ in IBAT) */
	unsigned long :1;	/* Unused */
	unsigned long pp:2;	/* Page access protections */
} BATL;

typedef struct _BAT {
	BATU batu;		/* Upper register */
	BATL batl;		/* Lower register */
} BAT;

typedef struct _P601_BAT {
	P601_BATU batu;		/* Upper register */
	P601_BATL batl;		/* Lower register */
} P601_BAT;

/*
 * Simulated two-level MMU.  This structure is used by the kernel
 * to keep track of MMU mappings and is used to update/maintain
 * the hardware HASH table which is really a cache of mappings.
 *
 * The simulated structures mimic the hardware available on other
 * platforms, notably the 80x86 and 680x0.
 */

typedef struct _pte {
	unsigned long page_num:20;
	unsigned long flags:12;		/* Page flags (some unused bits) */
} pte;

#define PD_SHIFT (10+12)		/* Page directory */
#define PD_MASK  0x02FF
#define PT_SHIFT (12)			/* Page Table */
#define PT_MASK  0x02FF
#define PG_SHIFT (12)			/* Page Entry */


/* MMU context */

typedef struct _MMU_context {
	SEGREG	segs[16];	/* Segment registers */
	pte	**pmap;		/* Two-level page-map structure */
} MMU_context;

extern void _tlbie(unsigned long va);	/* invalidate a TLB entry */
extern void _tlbia(void);		/* invalidate all TLB entries */

typedef enum {
	IBAT0 = 0, IBAT1, IBAT2, IBAT3,
	DBAT0, DBAT1, DBAT2, DBAT3
} ppc_bat_t;

extern int read_bat(ppc_bat_t bat, unsigned long *upper, unsigned long *lower);
extern int write_bat(ppc_bat_t bat, unsigned long upper, unsigned long lower);

#endif /* __ASSEMBLY__ */

/* Block size masks */
#define BL_128K	0x000
#define BL_256K 0x001
#define BL_512K 0x003
#define BL_1M   0x007
#define BL_2M   0x00F
#define BL_4M   0x01F
#define BL_8M   0x03F
#define BL_16M  0x07F
#define BL_32M  0x0FF
#define BL_64M  0x1FF
#define BL_128M 0x3FF
#define BL_256M 0x7FF

/* BAT Access Protection */
#define BPP_XX	0x00		/* No access */
#define BPP_RX	0x01		/* Read only */
#define BPP_RW	0x02		/* Read/write */

/* Used to set up SDR1 register */
#define HASH_TABLE_SIZE_64K	0x00010000
#define HASH_TABLE_SIZE_128K	0x00020000
#define HASH_TABLE_SIZE_256K	0x00040000
#define HASH_TABLE_SIZE_512K	0x00080000
#define HASH_TABLE_SIZE_1M	0x00100000
#define HASH_TABLE_SIZE_2M	0x00200000
#define HASH_TABLE_SIZE_4M	0x00400000
#define HASH_TABLE_MASK_64K	0x000
#define HASH_TABLE_MASK_128K	0x001
#define HASH_TABLE_MASK_256K	0x003
#define HASH_TABLE_MASK_512K	0x007
#define HASH_TABLE_MASK_1M	0x00F
#define HASH_TABLE_MASK_2M	0x01F
#define HASH_TABLE_MASK_4M	0x03F

/* Control/status registers for the MPC8xx.
 * A write operation to these registers causes serialized access.
 * During software tablewalk, the registers used perform mask/shift-add
 * operations when written/read.  A TLB entry is created when the Mx_RPN
 * is written, and the contents of several registers are used to
 * create the entry.
 */
#define MI_CTR		784	/* Instruction TLB control register */
#define MI_GPM		0x80000000	/* Set domain manager mode */
#define MI_PPM		0x40000000	/* Set subpage protection */
#define MI_CIDEF	0x20000000	/* Set cache inhibit when MMU dis */
#define MI_RSV4I	0x08000000	/* Reserve 4 TLB entries */
#define MI_PPCS		0x02000000	/* Use MI_RPN prob/priv state */
#define MI_IDXMASK	0x00001f00	/* TLB index to be loaded */
#define MI_RESETVAL	0x00000000	/* Value of register at reset */

/* These are the Ks and Kp from the PowerPC books.  For proper operation,
 * Ks = 0, Kp = 1.
 */
#define MI_AP		786
#define MI_Ks		0x80000000	/* Should not be set */
#define MI_Kp		0x40000000	/* Should always be set */

/* The effective page number register.  When read, contains the information
 * about the last instruction TLB miss.  When MI_RPN is written, bits in
 * this register are used to create the TLB entry.
 */
#define MI_EPN		787
#define MI_EPNMASK	0xfffff000	/* Effective page number for entry */
#define MI_EVALID	0x00000200	/* Entry is valid */
#define MI_ASIDMASK	0x0000000f	/* ASID match value */
					/* Reset value is undefined */

/* A "level 1" or "segment" or whatever you want to call it register.
 * For the instruction TLB, it contains bits that get loaded into the
 * TLB entry when the MI_RPN is written.
 */
#define MI_TWC		789
#define MI_APG		0x000001e0	/* Access protection group (0) */
#define MI_GUARDED	0x00000010	/* Guarded storage */
#define MI_PSMASK	0x0000000c	/* Mask of page size bits */
#define MI_PS8MEG	0x0000000c	/* 8M page size */
#define MI_PS512K	0x00000004	/* 512K page size */
#define MI_PS4K_16K	0x00000000	/* 4K or 16K page size */
#define MI_SVALID	0x00000001	/* Segment entry is valid */
					/* Reset value is undefined */

/* Real page number.  Defined by the pte.  Writing this register
 * causes a TLB entry to be created for the instruction TLB, using
 * additional information from the MI_EPN, and MI_TWC registers.
 */
#define MI_RPN		790

/* Define an RPN value for mapping kernel memory to large virtual
 * pages for boot initialization.  This has real page number of 0,
 * large page size, shared page, cache enabled, and valid.
 * Also mark all subpages valid and write access.
 */
#define MI_BOOTINIT	0x000001fd

#define MD_CTR		792	/* Data TLB control register */
#define MD_GPM		0x80000000	/* Set domain manager mode */
#define MD_PPM		0x40000000	/* Set subpage protection */
#define MD_CIDEF	0x20000000	/* Set cache inhibit when MMU dis */
#define MD_WTDEF	0x10000000	/* Set writethrough when MMU dis */
#define MD_RSV4I	0x08000000	/* Reserve 4 TLB entries */
#define MD_TWAM		0x04000000	/* Use 4K page hardware assist */
#define MD_PPCS		0x02000000	/* Use MI_RPN prob/priv state */
#define MD_IDXMASK	0x00001f00	/* TLB index to be loaded */
#define MD_RESETVAL	0x04000000	/* Value of register at reset */

#define M_CASID		793	/* Address space ID (context) to match */
#define MC_ASIDMASK	0x0000000f	/* Bits used for ASID value */


/* These are the Ks and Kp from the PowerPC books.  For proper operation,
 * Ks = 0, Kp = 1.
 */
#define MD_AP		794
#define MD_Ks		0x80000000	/* Should not be set */
#define MD_Kp		0x40000000	/* Should always be set */

/* The effective page number register.  When read, contains the information
 * about the last instruction TLB miss.  When MD_RPN is written, bits in
 * this register are used to create the TLB entry.
 */
#define MD_EPN		795
#define MD_EPNMASK	0xfffff000	/* Effective page number for entry */
#define MD_EVALID	0x00000200	/* Entry is valid */
#define MD_ASIDMASK	0x0000000f	/* ASID match value */
					/* Reset value is undefined */

/* The pointer to the base address of the first level page table.
 * During a software tablewalk, reading this register provides the address
 * of the entry associated with MD_EPN.
 */
#define M_TWB		796
#define	M_L1TB		0xfffff000	/* Level 1 table base address */
#define M_L1INDX	0x00000ffc	/* Level 1 index, when read */
					/* Reset value is undefined */

/* A "level 1" or "segment" or whatever you want to call it register.
 * For the data TLB, it contains bits that get loaded into the TLB entry
 * when the MD_RPN is written.  It is also provides the hardware assist
 * for finding the PTE address during software tablewalk.
 */
#define MD_TWC		797
#define MD_L2TB		0xfffff000	/* Level 2 table base address */
#define MD_L2INDX	0xfffffe00	/* Level 2 index (*pte), when read */
#define MD_APG		0x000001e0	/* Access protection group (0) */
#define MD_GUARDED	0x00000010	/* Guarded storage */
#define MD_PSMASK	0x0000000c	/* Mask of page size bits */
#define MD_PS8MEG	0x0000000c	/* 8M page size */
#define MD_PS512K	0x00000004	/* 512K page size */
#define MD_PS4K_16K	0x00000000	/* 4K or 16K page size */
#define MD_WT		0x00000002	/* Use writethrough page attribute */
#define MD_SVALID	0x00000001	/* Segment entry is valid */
					/* Reset value is undefined */


/* Real page number.  Defined by the pte.  Writing this register
 * causes a TLB entry to be created for the data TLB, using
 * additional information from the MD_EPN, and MD_TWC registers.
 */
#define MD_RPN		798

/* This is a temporary storage register that could be used to save
 * a processor working register during a tablewalk.
 */
#define M_TW		799

/*
 * At present, all PowerPC 400-class processors share a similar TLB
 * architecture. The instruction and data sides share a unified,
 * 64-entry, fully-associative TLB which is maintained totally under
 * software control. In addition, the instruction side has a
 * hardware-managed, 4-entry, fully- associative TLB which serves as a
 * first level to the shared TLB. These two TLBs are known as the UTLB
 * and ITLB, respectively.
 */

#define        PPC4XX_TLB_SIZE 64

/*
 * TLB entries are defined by a "high" tag portion and a "low" data
 * portion.  On all architectures, the data portion is 32-bits.
 *
 * TLB entries are managed entirely under software control by reading,
 * writing, and searchoing using the 4xx-specific tlbre, tlbwr, and tlbsx
 * instructions.
 */

/*
 * FSL Book-E support
 */

#define MAS0_TLBSEL(x)	((x << 28) & 0x30000000)
#define MAS0_ESEL(x)	((x << 16) & 0x0FFF0000)
#define MAS0_NV(x)	((x) & 0x00000FFF)

#define MAS1_VALID 	0x80000000
#define MAS1_IPROT	0x40000000
#define MAS1_TID(x)	((x << 16) & 0x3FFF0000)
#define MAS1_TS		0x00001000
#define MAS1_TSIZE(x)	((x << 8) & 0x00000F00)

#define MAS2_EPN	0xFFFFF000
#define MAS2_X0		0x00000040
#define MAS2_X1		0x00000020
#define MAS2_W		0x00000010
#define MAS2_I		0x00000008
#define MAS2_M		0x00000004
#define MAS2_G		0x00000002
#define MAS2_E		0x00000001

#define MAS3_RPN	0xFFFFF000
#define MAS3_U0		0x00000200
#define MAS3_U1		0x00000100
#define MAS3_U2		0x00000080
#define MAS3_U3		0x00000040
#define MAS3_UX		0x00000020
#define MAS3_SX		0x00000010
#define MAS3_UW		0x00000008
#define MAS3_SW		0x00000004
#define MAS3_UR		0x00000002
#define MAS3_SR		0x00000001

#define MAS4_TLBSELD(x) MAS0_TLBSEL(x)
#define MAS4_TIDDSEL	0x000F0000
#define MAS4_TSIZED(x)	MAS1_TSIZE(x)
#define MAS4_X0D	0x00000040
#define MAS4_X1D	0x00000020
#define MAS4_WD		0x00000010
#define MAS4_ID		0x00000008
#define MAS4_MD		0x00000004
#define MAS4_GD		0x00000002
#define MAS4_ED		0x00000001

#define MAS6_SPID0	0x3FFF0000
#define MAS6_SPID1	0x00007FFE
#define MAS6_SAS	0x00000001
#define MAS6_SPID	MAS6_SPID0

#define MAS7_RPN	0xFFFFFFFF

#define FSL_BOOKE_MAS0(tlbsel,esel,nv) \
		(MAS0_TLBSEL(tlbsel) | MAS0_ESEL(esel) | MAS0_NV(nv))
#define FSL_BOOKE_MAS1(v,iprot,tid,ts,tsize) \
		((((v) << 31) & MAS1_VALID)             |\
		(((iprot) << 30) & MAS1_IPROT)          |\
		(MAS1_TID(tid))				|\
		(((ts) << 12) & MAS1_TS)                |\
		(MAS1_TSIZE(tsize)))
#define FSL_BOOKE_MAS2(epn, wimge) \
		(((epn) & MAS3_RPN) | (wimge))
#define FSL_BOOKE_MAS3(rpn, user, perms) \
		(((rpn) & MAS3_RPN) | (user) | (perms))

#define BOOKE_PAGESZ_1K         0
#define BOOKE_PAGESZ_4K         1
#define BOOKE_PAGESZ_16K        2
#define BOOKE_PAGESZ_64K        3
#define BOOKE_PAGESZ_256K       4
#define BOOKE_PAGESZ_1M         5
#define BOOKE_PAGESZ_4M         6
#define BOOKE_PAGESZ_16M        7
#define BOOKE_PAGESZ_64M        8
#define BOOKE_PAGESZ_256M       9
#define BOOKE_PAGESZ_1G		10
#define BOOKE_PAGESZ_4G		11
#define BOOKE_PAGESZ_16GB	12
#define BOOKE_PAGESZ_64GB	13
#define BOOKE_PAGESZ_256GB	14
#define BOOKE_PAGESZ_1TB	15

#if defined(CONFIG_MPC86xx)
#define LAWBAR_BASE_ADDR	0x00FFFFFF
#define LAWAR_TRGT_IF		0x01F00000
#else
#define LAWBAR_BASE_ADDR	0x000FFFFF
#define LAWAR_TRGT_IF		0x00F00000
#endif
#define LAWAR_EN		0x80000000
#define LAWAR_SIZE		0x0000003F

#define LAWAR_TRGT_IF_PCI	0x00000000
#define LAWAR_TRGT_IF_PCI1	0x00000000
#define LAWAR_TRGT_IF_PCIX	0x00000000
#define LAWAR_TRGT_IF_PCI2	0x00100000
#define LAWAR_TRGT_IF_PCIE1	0x00200000
#define LAWAR_TRGT_IF_PCIE2	0x00100000
#define LAWAR_TRGT_IF_PCIE3	0x00300000
#define LAWAR_TRGT_IF_LBC	0x00400000
#define LAWAR_TRGT_IF_CCSR	0x00800000
#define LAWAR_TRGT_IF_DDR_INTERLEAVED 0x00B00000
#define LAWAR_TRGT_IF_RIO	0x00c00000
#define LAWAR_TRGT_IF_DDR	0x00f00000
#define LAWAR_TRGT_IF_DDR1	0x00f00000
#define LAWAR_TRGT_IF_DDR2	0x01600000

#define LAWAR_SIZE_BASE		0xa
#define LAWAR_SIZE_4K		(LAWAR_SIZE_BASE+1)
#define LAWAR_SIZE_8K		(LAWAR_SIZE_BASE+2)
#define LAWAR_SIZE_16K		(LAWAR_SIZE_BASE+3)
#define LAWAR_SIZE_32K		(LAWAR_SIZE_BASE+4)
#define LAWAR_SIZE_64K		(LAWAR_SIZE_BASE+5)
#define LAWAR_SIZE_128K		(LAWAR_SIZE_BASE+6)
#define LAWAR_SIZE_256K		(LAWAR_SIZE_BASE+7)
#define LAWAR_SIZE_512K		(LAWAR_SIZE_BASE+8)
#define LAWAR_SIZE_1M		(LAWAR_SIZE_BASE+9)
#define LAWAR_SIZE_2M		(LAWAR_SIZE_BASE+10)
#define LAWAR_SIZE_4M		(LAWAR_SIZE_BASE+11)
#define LAWAR_SIZE_8M		(LAWAR_SIZE_BASE+12)
#define LAWAR_SIZE_16M		(LAWAR_SIZE_BASE+13)
#define LAWAR_SIZE_32M		(LAWAR_SIZE_BASE+14)
#define LAWAR_SIZE_64M		(LAWAR_SIZE_BASE+15)
#define LAWAR_SIZE_128M		(LAWAR_SIZE_BASE+16)
#define LAWAR_SIZE_256M		(LAWAR_SIZE_BASE+17)
#define LAWAR_SIZE_512M		(LAWAR_SIZE_BASE+18)
#define LAWAR_SIZE_1G		(LAWAR_SIZE_BASE+19)
#define LAWAR_SIZE_2G		(LAWAR_SIZE_BASE+20)
#define LAWAR_SIZE_4G		(LAWAR_SIZE_BASE+21)
#define LAWAR_SIZE_8G		(LAWAR_SIZE_BASE+22)
#define LAWAR_SIZE_16G		(LAWAR_SIZE_BASE+23)
#define LAWAR_SIZE_32G		(LAWAR_SIZE_BASE+24)

#ifdef CONFIG_440
/* General */
#define TLB_VALID   0x00000200

/* Supported page sizes */

#define SZ_1K	0x00000000
#define SZ_4K	0x00000010
#define SZ_16K	0x00000020
#define SZ_64K	0x00000030
#define SZ_256K	0x00000040
#define SZ_1M	0x00000050
#define SZ_16M	0x00000070
#define SZ_256M	0x00000090

/* Storage attributes */
#define SA_W	0x00000800	/* Write-through */
#define SA_I	0x00000400	/* Caching inhibited */
#define SA_M	0x00000200	/* Memory coherence */
#define SA_G	0x00000100	/* Guarded */
#define SA_E	0x00000080	/* Endian */

/* Access control */
#define AC_X	0x00000024	/* Execute */
#define AC_W	0x00000012	/* Write */
#define AC_R	0x00000009	/* Read */

/* Some handy macros */

#define EPN(e)		((e) & 0xfffffc00)
#define TLB0(epn,sz)	((EPN((epn)) | (sz) | TLB_VALID ))
#define TLB1(rpn,erpn)	(((rpn) & 0xfffffc00) | (erpn))
#define TLB2(a)		((a) & 0x00000fbf)

#define tlbtab_start\
	mflr	r1	;\
	bl	0f	;

#define tlbtab_end\
	.long 0, 0, 0	;\
0:	mflr	r0	;\
	mtlr	r1	;\
	blr		;

#define tlbentry(epn,sz,rpn,erpn,attr)\
	.long TLB0(epn,sz),TLB1(rpn,erpn),TLB2(attr)

/*----------------------------------------------------------------------------+
| TLB specific defines.
+----------------------------------------------------------------------------*/
#define TLB_256MB_ALIGN_MASK 0xF0000000
#define TLB_16MB_ALIGN_MASK  0xFF000000
#define TLB_1MB_ALIGN_MASK   0xFFF00000
#define TLB_256KB_ALIGN_MASK 0xFFFC0000
#define TLB_64KB_ALIGN_MASK  0xFFFF0000
#define TLB_16KB_ALIGN_MASK  0xFFFFC000
#define TLB_4KB_ALIGN_MASK   0xFFFFF000
#define TLB_1KB_ALIGN_MASK   0xFFFFFC00
#define TLB_256MB_SIZE       0x10000000
#define TLB_16MB_SIZE        0x01000000
#define TLB_1MB_SIZE         0x00100000
#define TLB_256KB_SIZE       0x00040000
#define TLB_64KB_SIZE        0x00010000
#define TLB_16KB_SIZE        0x00004000
#define TLB_4KB_SIZE         0x00001000
#define TLB_1KB_SIZE         0x00000400

#define TLB_WORD0_EPN_MASK   0xFFFFFC00
#define TLB_WORD0_EPN_ENCODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD0_EPN_DECODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD0_V_MASK     0x00000200
#define TLB_WORD0_V_ENABLE   0x00000200
#define TLB_WORD0_V_DISABLE  0x00000000
#define TLB_WORD0_TS_MASK    0x00000100
#define TLB_WORD0_TS_1       0x00000100
#define TLB_WORD0_TS_0       0x00000000
#define TLB_WORD0_SIZE_MASK  0x000000F0
#define TLB_WORD0_SIZE_1KB   0x00000000
#define TLB_WORD0_SIZE_4KB   0x00000010
#define TLB_WORD0_SIZE_16KB  0x00000020
#define TLB_WORD0_SIZE_64KB  0x00000030
#define TLB_WORD0_SIZE_256KB 0x00000040
#define TLB_WORD0_SIZE_1MB   0x00000050
#define TLB_WORD0_SIZE_16MB  0x00000070
#define TLB_WORD0_SIZE_256MB 0x00000090
#define TLB_WORD0_TPAR_MASK  0x0000000F
#define TLB_WORD0_TPAR_ENCODE(n) ((((unsigned long)(n))&0x0F)<<0)
#define TLB_WORD0_TPAR_DECODE(n) ((((unsigned long)(n))>>0)&0x0F)

#define TLB_WORD1_RPN_MASK   0xFFFFFC00
#define TLB_WORD1_RPN_ENCODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD1_RPN_DECODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD1_PAR1_MASK  0x00000300
#define TLB_WORD1_PAR1_ENCODE(n) ((((unsigned long)(n))&0x03)<<8)
#define TLB_WORD1_PAR1_DECODE(n) ((((unsigned long)(n))>>8)&0x03)
#define TLB_WORD1_PAR1_0     0x00000000
#define TLB_WORD1_PAR1_1     0x00000100
#define TLB_WORD1_PAR1_2     0x00000200
#define TLB_WORD1_PAR1_3     0x00000300
#define TLB_WORD1_ERPN_MASK  0x0000000F
#define TLB_WORD1_ERPN_ENCODE(n) ((((unsigned long)(n))&0x0F)<<0)
#define TLB_WORD1_ERPN_DECODE(n) ((((unsigned long)(n))>>0)&0x0F)

#define TLB_WORD2_PAR2_MASK  0xC0000000
#define TLB_WORD2_PAR2_ENCODE(n) ((((unsigned long)(n))&0x03)<<30)
#define TLB_WORD2_PAR2_DECODE(n) ((((unsigned long)(n))>>30)&0x03)
#define TLB_WORD2_PAR2_0     0x00000000
#define TLB_WORD2_PAR2_1     0x40000000
#define TLB_WORD2_PAR2_2     0x80000000
#define TLB_WORD2_PAR2_3     0xC0000000
#define TLB_WORD2_U0_MASK    0x00008000
#define TLB_WORD2_U0_ENABLE  0x00008000
#define TLB_WORD2_U0_DISABLE 0x00000000
#define TLB_WORD2_U1_MASK    0x00004000
#define TLB_WORD2_U1_ENABLE  0x00004000
#define TLB_WORD2_U1_DISABLE 0x00000000
#define TLB_WORD2_U2_MASK    0x00002000
#define TLB_WORD2_U2_ENABLE  0x00002000
#define TLB_WORD2_U2_DISABLE 0x00000000
#define TLB_WORD2_U3_MASK    0x00001000
#define TLB_WORD2_U3_ENABLE  0x00001000
#define TLB_WORD2_U3_DISABLE 0x00000000
#define TLB_WORD2_W_MASK     0x00000800
#define TLB_WORD2_W_ENABLE   0x00000800
#define TLB_WORD2_W_DISABLE  0x00000000
#define TLB_WORD2_I_MASK     0x00000400
#define TLB_WORD2_I_ENABLE   0x00000400
#define TLB_WORD2_I_DISABLE  0x00000000
#define TLB_WORD2_M_MASK     0x00000200
#define TLB_WORD2_M_ENABLE   0x00000200
#define TLB_WORD2_M_DISABLE  0x00000000
#define TLB_WORD2_G_MASK     0x00000100
#define TLB_WORD2_G_ENABLE   0x00000100
#define TLB_WORD2_G_DISABLE  0x00000000
#define TLB_WORD2_E_MASK     0x00000080
#define TLB_WORD2_E_ENABLE   0x00000080
#define TLB_WORD2_E_DISABLE  0x00000000
#define TLB_WORD2_UX_MASK    0x00000020
#define TLB_WORD2_UX_ENABLE  0x00000020
#define TLB_WORD2_UX_DISABLE 0x00000000
#define TLB_WORD2_UW_MASK    0x00000010
#define TLB_WORD2_UW_ENABLE  0x00000010
#define TLB_WORD2_UW_DISABLE 0x00000000
#define TLB_WORD2_UR_MASK    0x00000008
#define TLB_WORD2_UR_ENABLE  0x00000008
#define TLB_WORD2_UR_DISABLE 0x00000000
#define TLB_WORD2_SX_MASK    0x00000004
#define TLB_WORD2_SX_ENABLE  0x00000004
#define TLB_WORD2_SX_DISABLE 0x00000000
#define TLB_WORD2_SW_MASK    0x00000002
#define TLB_WORD2_SW_ENABLE  0x00000002
#define TLB_WORD2_SW_DISABLE 0x00000000
#define TLB_WORD2_SR_MASK    0x00000001
#define TLB_WORD2_SR_ENABLE  0x00000001
#define TLB_WORD2_SR_DISABLE 0x00000000

/*----------------------------------------------------------------------------+
| Following instructions are not available in Book E mode of the GNU assembler.
+----------------------------------------------------------------------------*/
#define DCCCI(ra,rb)			.long 0x7c000000|\
					(ra<<16)|(rb<<11)|(454<<1)

#define ICCCI(ra,rb)			.long 0x7c000000|\
					(ra<<16)|(rb<<11)|(966<<1)

#define DCREAD(rt,ra,rb)		.long 0x7c000000|\
					(rt<<21)|(ra<<16)|(rb<<11)|(486<<1)

#define ICREAD(ra,rb)			.long 0x7c000000|\
					(ra<<16)|(rb<<11)|(998<<1)

#define TLBSX(rt,ra,rb)			.long 0x7c000000|\
					(rt<<21)|(ra<<16)|(rb<<11)|(914<<1)

#define TLBWE(rs,ra,ws)			.long 0x7c000000|\
					(rs<<21)|(ra<<16)|(ws<<11)|(978<<1)

#define TLBRE(rt,ra,ws)			.long 0x7c000000|\
					(rt<<21)|(ra<<16)|(ws<<11)|(946<<1)

#define TLBSXDOT(rt,ra,rb)		.long 0x7c000001|\
					(rt<<21)|(ra<<16)|(rb<<11)|(914<<1)

#define MSYNC				.long 0x7c000000|\
					(598<<1)

#define MBAR_INST 				.long 0x7c000000|\
					(854<<1)

#ifndef __ASSEMBLY__
/* Prototypes */
void mttlb1(unsigned long index, unsigned long value);
void mttlb2(unsigned long index, unsigned long value);
void mttlb3(unsigned long index, unsigned long value);
unsigned long mftlb1(unsigned long index);
unsigned long mftlb2(unsigned long index);
unsigned long mftlb3(unsigned long index);

void program_tlb(u32 phys_addr, u32 virt_addr, u32 size, u32 tlb_word2_i_value);
void remove_tlb(u32 vaddr, u32 size);
void change_tlb(u32 vaddr, u32 size, u32 tlb_word2_i_value);
#endif /* __ASSEMBLY__ */

#endif /* CONFIG_440 */
#endif /* _PPC_MMU_H_ */
