/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995, 1996, 1997, 2000, 2001 by Ralf Baechle
 * Copyright (C) 2000 Silicon Graphics, Inc.
 * Modified for further R[236]000 support by Paul M. Antoine, 1996.
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000, 07 MIPS Technologies, Inc.
 * Copyright (C) 2003, 2004  Maciej W. Rozycki
 */
#ifndef _ASM_MIPSREGS_H
#define _ASM_MIPSREGS_H

#if 0
#include <linux/linkage.h>
#endif

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ULCAST_
#else
#define _ULCAST_ (unsigned long)
#endif

/*
 * Coprocessor 0 register names
 */
#define CP0_INDEX $0
#define CP0_RANDOM $1
#define CP0_ENTRYLO0 $2
#define CP0_ENTRYLO1 $3
#define CP0_CONF $3
#define CP0_CONTEXT $4
#define CP0_PAGEMASK $5
#define CP0_WIRED $6
#define CP0_INFO $7
#define CP0_BADVADDR $8
#define CP0_COUNT $9
#define CP0_ENTRYHI $10
#define CP0_COMPARE $11
#define CP0_STATUS $12
#define CP0_CAUSE $13
#define CP0_EPC $14
#define CP0_PRID $15
#define CP0_CONFIG $16
#define CP0_LLADDR $17
#define CP0_WATCHLO $18
#define CP0_WATCHHI $19
#define CP0_XCONTEXT $20
#define CP0_FRAMEMASK $21
#define CP0_DIAGNOSTIC $22
#define CP0_DEBUG $23
#define CP0_DEPC $24
#define CP0_PERFORMANCE $25
#define CP0_ECC $26
#define CP0_CACHEERR $27
#define CP0_TAGLO $28
#define CP0_TAGHI $29
#define CP0_ERROREPC $30
#define CP0_DESAVE $31

/*
 * R4640/R4650 cp0 register names.  These registers are listed
 * here only for completeness; without MMU these CPUs are not useable
 * by Linux.  A future ELKS port might take make Linux run on them
 * though ...
 */
#define CP0_IBASE $0
#define CP0_IBOUND $1
#define CP0_DBASE $2
#define CP0_DBOUND $3
#define CP0_CALG $17
#define CP0_IWATCH $18
#define CP0_DWATCH $19

/*
 * Coprocessor 0 Set 1 register names
 */
#define CP0_S1_DERRADDR0  $26
#define CP0_S1_DERRADDR1  $27
#define CP0_S1_INTCONTROL $20

/*
 * Coprocessor 0 Set 2 register names
 */
#define CP0_S2_SRSCTL	$12	/* MIPSR2 */

/*
 * Coprocessor 0 Set 3 register names
 */
#define CP0_S3_SRSMAP	$12	/* MIPSR2 */

/*
 *  TX39 Series
 */
#define CP0_TX39_CACHE	$7

/*
 * Coprocessor 1 (FPU) register names
 */
#define CP1_REVISION	$0
#define CP1_STATUS	$31

/*
 * FPU Status Register Values
 */
/*
 * Status Register Values
 */

#define FPU_CSR_FLUSH	0x01000000	/* flush denormalised results to 0 */
#define FPU_CSR_COND	0x00800000	/* $fcc0 */
#define FPU_CSR_COND0	0x00800000	/* $fcc0 */
#define FPU_CSR_COND1	0x02000000	/* $fcc1 */
#define FPU_CSR_COND2	0x04000000	/* $fcc2 */
#define FPU_CSR_COND3	0x08000000	/* $fcc3 */
#define FPU_CSR_COND4	0x10000000	/* $fcc4 */
#define FPU_CSR_COND5	0x20000000	/* $fcc5 */
#define FPU_CSR_COND6	0x40000000	/* $fcc6 */
#define FPU_CSR_COND7	0x80000000	/* $fcc7 */

/*
 * X the exception cause indicator
 * E the exception enable
 * S the sticky/flag bit
 */
#define FPU_CSR_ALL_X	0x0003f000
#define FPU_CSR_UNI_X	0x00020000
#define FPU_CSR_INV_X	0x00010000
#define FPU_CSR_DIV_X	0x00008000
#define FPU_CSR_OVF_X	0x00004000
#define FPU_CSR_UDF_X	0x00002000
#define FPU_CSR_INE_X	0x00001000

#define FPU_CSR_ALL_E	0x00000f80
#define FPU_CSR_INV_E	0x00000800
#define FPU_CSR_DIV_E	0x00000400
#define FPU_CSR_OVF_E	0x00000200
#define FPU_CSR_UDF_E	0x00000100
#define FPU_CSR_INE_E	0x00000080

#define FPU_CSR_ALL_S	0x0000007c
#define FPU_CSR_INV_S	0x00000040
#define FPU_CSR_DIV_S	0x00000020
#define FPU_CSR_OVF_S	0x00000010
#define FPU_CSR_UDF_S	0x00000008
#define FPU_CSR_INE_S	0x00000004

/* rounding mode */
#define FPU_CSR_RN	0x0	/* nearest */
#define FPU_CSR_RZ	0x1	/* towards zero */
#define FPU_CSR_RU	0x2	/* towards +Infinity */
#define FPU_CSR_RD	0x3	/* towards -Infinity */

/*
 * Values for PageMask register
 */
#ifdef CONFIG_CPU_VR41XX

/* Why doesn't stupidity hurt ... */

#define PM_1K		0x00000000
#define PM_4K		0x00001800
#define PM_16K		0x00007800
#define PM_64K		0x0001f800
#define PM_256K		0x0007f800

#else

#define PM_4K		0x00000000
#define PM_16K		0x00006000
#define PM_64K		0x0001e000
#define PM_256K		0x0007e000
#define PM_1M		0x001fe000
#define PM_4M		0x007fe000
#define PM_16M		0x01ffe000
#define PM_64M		0x07ffe000
#define PM_256M		0x1fffe000

#endif

/*
 * Values used for computation of new tlb entries
 */
#define PL_4K		12
#define PL_16K		14
#define PL_64K		16
#define PL_256K		18
#define PL_1M		20
#define PL_4M		22
#define PL_16M		24
#define PL_64M		26
#define PL_256M		28

/*
 * Macros to access the system control coprocessor
 */
#define read_32bit_cp0_register(source)				\
({ int __res;							\
	__asm__ __volatile__(					\
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
	"mfc0\t%0,"STR(source)"\n\t"				\
	".set\tpop"						\
	: "=r" (__res));					\
	__res;})

#define read_32bit_cp0_set1_register(source)			\
({ int __res;							\
	__asm__ __volatile__(					\
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
	"cfc0\t%0,"STR(source)"\n\t"				\
	".set\tpop"						\
	: "=r" (__res));					\
	__res;})

/*
 * For now use this only with interrupts disabled!
 */
#define read_64bit_cp0_register(source)				\
({ int __res;							\
	__asm__ __volatile__(					\
	".set\tmips3\n\t"					\
	"dmfc0\t%0,"STR(source)"\n\t"				\
	".set\tmips0"						\
	: "=r" (__res));					\
	__res;})

#define write_32bit_cp0_register(register,value)		\
	__asm__ __volatile__(					\
	"mtc0\t%0,"STR(register)"\n\t"				\
	"nop"							\
	: : "r" (value));

#define write_32bit_cp0_set1_register(register,value)		\
	__asm__ __volatile__(					\
	"ctc0\t%0,"STR(register)"\n\t"				\
	"nop"							\
	: : "r" (value));

#define write_64bit_cp0_register(register,value)		\
	__asm__ __volatile__(					\
	".set\tmips3\n\t"					\
	"dmtc0\t%0,"STR(register)"\n\t"				\
	".set\tmips0"						\
	: : "r" (value))

/*
 * This should be changed when we get a compiler that support the MIPS32 ISA.
 */
#define read_mips32_cp0_config1()				\
({ int __res;							\
	__asm__ __volatile__(					\
	".set\tnoreorder\n\t"					\
	".set\tnoat\n\t"					\
	".word\t0x40018001\n\t"					\
	"move\t%0,$1\n\t"					\
	".set\tat\n\t"						\
	".set\treorder"						\
	:"=r" (__res));						\
	__res;})

#define tlb_write_indexed()					\
	__asm__ __volatile__(					\
		".set noreorder\n\t"				\
		"tlbwi\n\t"					\
".set reorder")

/*
 * R4x00 interrupt enable / cause bits
 */
#define IE_SW0		(_ULCAST_(1) <<  8)
#define IE_SW1		(_ULCAST_(1) <<  9)
#define IE_IRQ0		(_ULCAST_(1) << 10)
#define IE_IRQ1		(_ULCAST_(1) << 11)
#define IE_IRQ2		(_ULCAST_(1) << 12)
#define IE_IRQ3		(_ULCAST_(1) << 13)
#define IE_IRQ4		(_ULCAST_(1) << 14)
#define IE_IRQ5		(_ULCAST_(1) << 15)

/*
 * R4x00 interrupt cause bits
 */
#define C_SW0		(_ULCAST_(1) <<  8)
#define C_SW1		(_ULCAST_(1) <<  9)
#define C_IRQ0		(_ULCAST_(1) << 10)
#define C_IRQ1		(_ULCAST_(1) << 11)
#define C_IRQ2		(_ULCAST_(1) << 12)
#define C_IRQ3		(_ULCAST_(1) << 13)
#define C_IRQ4		(_ULCAST_(1) << 14)
#define C_IRQ5		(_ULCAST_(1) << 15)

#ifndef _LANGUAGE_ASSEMBLY
/*
 * Manipulate the status register.
 * Mostly used to access the interrupt bits.
 */
#define __BUILD_SET_CP0(name,register)				\
extern __inline__ unsigned int					\
set_cp0_##name(unsigned int set)				\
{								\
	unsigned int res;					\
								\
	res = read_32bit_cp0_register(register);		\
	res |= set;						\
	write_32bit_cp0_register(register, res);		\
								\
	return res;						\
}								\
								\
extern __inline__ unsigned int					\
clear_cp0_##name(unsigned int clear)				\
{								\
	unsigned int res;					\
								\
	res = read_32bit_cp0_register(register);		\
	res &= ~clear;						\
	write_32bit_cp0_register(register, res);		\
								\
	return res;						\
}								\
								\
extern __inline__ unsigned int					\
change_cp0_##name(unsigned int change, unsigned int new)	\
{								\
	unsigned int res;					\
								\
	res = read_32bit_cp0_register(register);		\
	res &= ~change;						\
	res |= (new & change);					\
	if(change)						\
		write_32bit_cp0_register(register, res);	\
								\
	return res;						\
}

__BUILD_SET_CP0(status,CP0_STATUS)
__BUILD_SET_CP0(cause,CP0_CAUSE)
__BUILD_SET_CP0(config,CP0_CONFIG)

#endif /* defined (_LANGUAGE_ASSEMBLY) */

/*
 * Bitfields in the R4xx0 cp0 status register
 */
#define ST0_IE			0x00000001
#define ST0_EXL			0x00000002
#define ST0_ERL			0x00000004
#define ST0_KSU			0x00000018
#  define KSU_USER		0x00000010
#  define KSU_SUPERVISOR	0x00000008
#  define KSU_KERNEL		0x00000000
#define ST0_UX			0x00000020
#define ST0_SX			0x00000040
#define ST0_KX			0x00000080
#define ST0_DE			0x00010000
#define ST0_CE			0x00020000

/*
 * Setting c0_status.co enables Hit_Writeback and Hit_Writeback_Invalidate
 * cacheops in userspace.  This bit exists only on RM7000 and RM9000
 * processors.
 */
#define ST0_CO			0x08000000

/*
 * Bitfields in the R[23]000 cp0 status register.
 */
#define ST0_IEC			0x00000001
#define ST0_KUC			0x00000002
#define ST0_IEP			0x00000004
#define ST0_KUP			0x00000008
#define ST0_IEO			0x00000010
#define ST0_KUO			0x00000020
/* bits 6 & 7 are reserved on R[23]000 */
#define ST0_ISC			0x00010000
#define ST0_SWC			0x00020000
#define ST0_CM			0x00080000

/*
 * Bits specific to the R4640/R4650
 */
#define ST0_UM			(_ULCAST_(1) <<  4)
#define ST0_IL			(_ULCAST_(1) << 23)
#define ST0_DL			(_ULCAST_(1) << 24)

/*
 * Enable the MIPS MDMX and DSP ASEs
 */
#define ST0_MX			0x01000000

/*
 * Bitfields in the TX39 family CP0 Configuration Register 3
 */
#define TX39_CONF_ICS_SHIFT	19
#define TX39_CONF_ICS_MASK	0x00380000
#define TX39_CONF_ICS_1KB	0x00000000
#define TX39_CONF_ICS_2KB	0x00080000
#define TX39_CONF_ICS_4KB	0x00100000
#define TX39_CONF_ICS_8KB	0x00180000
#define TX39_CONF_ICS_16KB	0x00200000

#define TX39_CONF_DCS_SHIFT	16
#define TX39_CONF_DCS_MASK	0x00070000
#define TX39_CONF_DCS_1KB	0x00000000
#define TX39_CONF_DCS_2KB	0x00010000
#define TX39_CONF_DCS_4KB	0x00020000
#define TX39_CONF_DCS_8KB	0x00030000
#define TX39_CONF_DCS_16KB	0x00040000

#define TX39_CONF_CWFON		0x00004000
#define TX39_CONF_WBON		0x00002000
#define TX39_CONF_RF_SHIFT	10
#define TX39_CONF_RF_MASK	0x00000c00
#define TX39_CONF_DOZE		0x00000200
#define TX39_CONF_HALT		0x00000100
#define TX39_CONF_LOCK		0x00000080
#define TX39_CONF_ICE		0x00000020
#define TX39_CONF_DCE		0x00000010
#define TX39_CONF_IRSIZE_SHIFT	2
#define TX39_CONF_IRSIZE_MASK	0x0000000c
#define TX39_CONF_DRSIZE_SHIFT	0
#define TX39_CONF_DRSIZE_MASK	0x00000003

/*
 * Status register bits available in all MIPS CPUs.
 */
#define ST0_IM			0x0000ff00
#define  STATUSB_IP0		8
#define  STATUSF_IP0		(_ULCAST_(1) <<  8)
#define  STATUSB_IP1		9
#define  STATUSF_IP1		(_ULCAST_(1) <<  9)
#define  STATUSB_IP2		10
#define  STATUSF_IP2		(_ULCAST_(1) << 10)
#define  STATUSB_IP3		11
#define  STATUSF_IP3		(_ULCAST_(1) << 11)
#define  STATUSB_IP4		12
#define  STATUSF_IP4		(_ULCAST_(1) << 12)
#define  STATUSB_IP5		13
#define  STATUSF_IP5		(_ULCAST_(1) << 13)
#define  STATUSB_IP6		14
#define  STATUSF_IP6		(_ULCAST_(1) << 14)
#define  STATUSB_IP7		15
#define  STATUSF_IP7		(_ULCAST_(1) << 15)
#define  STATUSB_IP8		0
#define  STATUSF_IP8		(_ULCAST_(1) <<  0)
#define  STATUSB_IP9		1
#define  STATUSF_IP9		(_ULCAST_(1) <<  1)
#define  STATUSB_IP10		2
#define  STATUSF_IP10		(_ULCAST_(1) <<  2)
#define  STATUSB_IP11		3
#define  STATUSF_IP11		(_ULCAST_(1) <<  3)
#define  STATUSB_IP12		4
#define  STATUSF_IP12		(_ULCAST_(1) <<  4)
#define  STATUSB_IP13		5
#define  STATUSF_IP13		(_ULCAST_(1) <<  5)
#define  STATUSB_IP14		6
#define  STATUSF_IP14		(_ULCAST_(1) <<  6)
#define  STATUSB_IP15		7
#define  STATUSF_IP15		(_ULCAST_(1) <<  7)
#define ST0_CH			0x00040000
#define ST0_SR			0x00100000
#define ST0_TS			0x00200000
#define ST0_BEV			0x00400000
#define ST0_RE			0x02000000
#define ST0_FR			0x04000000
#define ST0_CU			0xf0000000
#define ST0_CU0			0x10000000
#define ST0_CU1			0x20000000
#define ST0_CU2			0x40000000
#define ST0_CU3			0x80000000
#define ST0_XX			0x80000000	/* MIPS IV naming */

/*
 * Bitfields and bit numbers in the coprocessor 0 cause register.
 *
 * Refer to your MIPS R4xx0 manual, chapter 5 for explanation.
 */
#define  CAUSEB_EXCCODE		2
#define  CAUSEF_EXCCODE		(_ULCAST_(31)  <<  2)
#define  CAUSEB_IP		8
#define  CAUSEF_IP		(_ULCAST_(255) <<  8)
#define  CAUSEB_IP0		8
#define  CAUSEF_IP0		(_ULCAST_(1)   <<  8)
#define  CAUSEB_IP1		9
#define  CAUSEF_IP1		(_ULCAST_(1)   <<  9)
#define  CAUSEB_IP2		10
#define  CAUSEF_IP2		(_ULCAST_(1)   << 10)
#define  CAUSEB_IP3		11
#define  CAUSEF_IP3		(_ULCAST_(1)   << 11)
#define  CAUSEB_IP4		12
#define  CAUSEF_IP4		(_ULCAST_(1)   << 12)
#define  CAUSEB_IP5		13
#define  CAUSEF_IP5		(_ULCAST_(1)   << 13)
#define  CAUSEB_IP6		14
#define  CAUSEF_IP6		(_ULCAST_(1)   << 14)
#define  CAUSEB_IP7		15
#define  CAUSEF_IP7		(_ULCAST_(1)   << 15)
#define  CAUSEB_IV		23
#define  CAUSEF_IV		(_ULCAST_(1)   << 23)
#define  CAUSEB_CE		28
#define  CAUSEF_CE		(_ULCAST_(3)   << 28)
#define  CAUSEB_BD		31
#define  CAUSEF_BD		(_ULCAST_(1)   << 31)

/*
 * Bits in the coprocessor 0 config register.
 */
/* Generic bits.  */
#define CONF_CM_CACHABLE_NO_WA		0
#define CONF_CM_CACHABLE_WA		1
#define CONF_CM_UNCACHED		2
#define CONF_CM_CACHABLE_NONCOHERENT	3
#define CONF_CM_CACHABLE_CE		4
#define CONF_CM_CACHABLE_COW		5
#define CONF_CM_CACHABLE_CUW		6
#define CONF_CM_CACHABLE_ACCELERATED	7
#define CONF_CM_CMASK			7
#define CONF_BE			(_ULCAST_(1) << 15)

/* Bits common to various processors.  */
#define CONF_CU			(_ULCAST_(1) <<  3)
#define CONF_DB			(_ULCAST_(1) <<  4)
#define CONF_IB			(_ULCAST_(1) <<  5)
#define CONF_DC			(_ULCAST_(7) <<  6)
#define CONF_IC			(_ULCAST_(7) <<  9)
#define CONF_EB			(_ULCAST_(1) << 13)
#define CONF_EM			(_ULCAST_(1) << 14)
#define CONF_SM			(_ULCAST_(1) << 16)
#define CONF_SC			(_ULCAST_(1) << 17)
#define CONF_EW			(_ULCAST_(3) << 18)
#define CONF_EP			(_ULCAST_(15)<< 24)
#define CONF_EC			(_ULCAST_(7) << 28)
#define CONF_CM			(_ULCAST_(1) << 31)

/* Bits specific to the R4xx0.  */
#define R4K_CONF_SW		(_ULCAST_(1) << 20)
#define R4K_CONF_SS		(_ULCAST_(1) << 21)
#define R4K_CONF_SB		(_ULCAST_(3) << 22)

/* Bits specific to the R5000.  */
#define R5K_CONF_SE		(_ULCAST_(1) << 12)
#define R5K_CONF_SS		(_ULCAST_(3) << 20)

/* Bits specific to the RM7000.  */
#define RM7K_CONF_SE		(_ULCAST_(1) <<  3)
#define RM7K_CONF_TE		(_ULCAST_(1) << 12)
#define RM7K_CONF_CLK		(_ULCAST_(1) << 16)
#define RM7K_CONF_TC		(_ULCAST_(1) << 17)
#define RM7K_CONF_SI		(_ULCAST_(3) << 20)
#define RM7K_CONF_SC		(_ULCAST_(1) << 31)

/* Bits specific to the R10000.  */
#define R10K_CONF_DN		(_ULCAST_(3) <<  3)
#define R10K_CONF_CT		(_ULCAST_(1) <<  5)
#define R10K_CONF_PE		(_ULCAST_(1) <<  6)
#define R10K_CONF_PM		(_ULCAST_(3) <<  7)
#define R10K_CONF_EC		(_ULCAST_(15)<<  9)
#define R10K_CONF_SB		(_ULCAST_(1) << 13)
#define R10K_CONF_SK		(_ULCAST_(1) << 14)
#define R10K_CONF_SS		(_ULCAST_(7) << 16)
#define R10K_CONF_SC		(_ULCAST_(7) << 19)
#define R10K_CONF_DC		(_ULCAST_(7) << 26)
#define R10K_CONF_IC		(_ULCAST_(7) << 29)

/* Bits specific to the VR41xx.  */
#define VR41_CONF_CS		(_ULCAST_(1) << 12)
#define VR41_CONF_P4K		(_ULCAST_(1) << 13)
#define VR41_CONF_BP		(_ULCAST_(1) << 16)
#define VR41_CONF_M16		(_ULCAST_(1) << 20)
#define VR41_CONF_AD		(_ULCAST_(1) << 23)

/* Bits specific to the R30xx.  */
#define R30XX_CONF_FDM		(_ULCAST_(1) << 19)
#define R30XX_CONF_REV		(_ULCAST_(1) << 22)
#define R30XX_CONF_AC		(_ULCAST_(1) << 23)
#define R30XX_CONF_RF		(_ULCAST_(1) << 24)
#define R30XX_CONF_HALT		(_ULCAST_(1) << 25)
#define R30XX_CONF_FPINT	(_ULCAST_(7) << 26)
#define R30XX_CONF_DBR		(_ULCAST_(1) << 29)
#define R30XX_CONF_SB		(_ULCAST_(1) << 30)
#define R30XX_CONF_LOCK		(_ULCAST_(1) << 31)

/* Bits specific to the TX49.  */
#define TX49_CONF_DC		(_ULCAST_(1) << 16)
#define TX49_CONF_IC		(_ULCAST_(1) << 17)  /* conflict with CONF_SC */
#define TX49_CONF_HALT		(_ULCAST_(1) << 18)
#define TX49_CONF_CWFON		(_ULCAST_(1) << 27)

/* Bits specific to the MIPS32/64 PRA.  */
#define MIPS_CONF_MT		(_ULCAST_(7) <<  7)
#define MIPS_CONF_AR		(_ULCAST_(7) << 10)
#define MIPS_CONF_AT		(_ULCAST_(3) << 13)
#define MIPS_CONF_M		(_ULCAST_(1) << 31)

/*
 * Bits in the MIPS32/64 PRA coprocessor 0 config registers 1 and above.
 */
#define MIPS_CONF1_FP		(_ULCAST_(1) <<  0)
#define MIPS_CONF1_EP		(_ULCAST_(1) <<  1)
#define MIPS_CONF1_CA		(_ULCAST_(1) <<  2)
#define MIPS_CONF1_WR		(_ULCAST_(1) <<  3)
#define MIPS_CONF1_PC		(_ULCAST_(1) <<  4)
#define MIPS_CONF1_MD		(_ULCAST_(1) <<  5)
#define MIPS_CONF1_C2		(_ULCAST_(1) <<  6)
#define MIPS_CONF1_DA		(_ULCAST_(7) <<  7)
#define MIPS_CONF1_DL		(_ULCAST_(7) << 10)
#define MIPS_CONF1_DS		(_ULCAST_(7) << 13)
#define MIPS_CONF1_IA		(_ULCAST_(7) << 16)
#define MIPS_CONF1_IL		(_ULCAST_(7) << 19)
#define MIPS_CONF1_IS		(_ULCAST_(7) << 22)
#define MIPS_CONF1_TLBS		(_ULCAST_(63)<< 25)

#define MIPS_CONF2_SA		(_ULCAST_(15)<<  0)
#define MIPS_CONF2_SL		(_ULCAST_(15)<<  4)
#define MIPS_CONF2_SS		(_ULCAST_(15)<<  8)
#define MIPS_CONF2_SU		(_ULCAST_(15)<< 12)
#define MIPS_CONF2_TA		(_ULCAST_(15)<< 16)
#define MIPS_CONF2_TL		(_ULCAST_(15)<< 20)
#define MIPS_CONF2_TS		(_ULCAST_(15)<< 24)
#define MIPS_CONF2_TU		(_ULCAST_(7) << 28)

#define MIPS_CONF3_TL		(_ULCAST_(1) <<  0)
#define MIPS_CONF3_SM		(_ULCAST_(1) <<  1)
#define MIPS_CONF3_MT		(_ULCAST_(1) <<  2)
#define MIPS_CONF3_SP		(_ULCAST_(1) <<  4)
#define MIPS_CONF3_VINT		(_ULCAST_(1) <<  5)
#define MIPS_CONF3_VEIC		(_ULCAST_(1) <<  6)
#define MIPS_CONF3_LPA		(_ULCAST_(1) <<  7)
#define MIPS_CONF3_DSP		(_ULCAST_(1) << 10)
#define MIPS_CONF3_ULRI		(_ULCAST_(1) << 13)

#define MIPS_CONF7_WII		(_ULCAST_(1) << 31)

#define MIPS_CONF7_RPS		(_ULCAST_(1) << 2)

/*
 * Bits in the MIPS32/64 coprocessor 1 (FPU) revision register.
 */
#define MIPS_FPIR_S		(_ULCAST_(1) << 16)
#define MIPS_FPIR_D		(_ULCAST_(1) << 17)
#define MIPS_FPIR_PS		(_ULCAST_(1) << 18)
#define MIPS_FPIR_3D		(_ULCAST_(1) << 19)
#define MIPS_FPIR_W		(_ULCAST_(1) << 20)
#define MIPS_FPIR_L		(_ULCAST_(1) << 21)
#define MIPS_FPIR_F64		(_ULCAST_(1) << 22)

#endif /* _ASM_MIPSREGS_H */
