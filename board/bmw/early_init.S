#include <ppc_asm.tmpl>
#include <mpc824x.h>
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>

#define USE_V2_INIT 1 /* Jimmy Blair's initialization. */


/*
 * Initialize the MMU using BAT entries and hardwired TLB
 * This obviates the need for any code in cpu_init_f which
 * configures the BAT registers.
*/
#define MEMORY_MGMT_MSR_BITS (MSR_DR | MSR_IR) /* Data and Inst Relocate */
	.global	iommu_setup
	/* Initialize IO/MMU mappings via BAT method Ch. 7,
	 * PPC Programming Reference
	 */
iommu_setup:

/* initialize the BAT registers (SPRs 528 - 543 */
#define	mtibat0u(x)	mtspr	528,(x)			/* SPR 528 (IBAT0U) */
#define mtibat0l(x)	mtspr	529,(x)			/* SPR 529 (IBAT0L) */
#define mtibat1u(x)	mtspr	530,(x)			/* SPR 530 (IBAT1U) */
#define mtibat1l(x)	mtspr	531,(x)			/* SPR 531 (IBAT1L) */
#define mtibat2u(x)	mtspr	532,(x)			/* SPR 532 (IBAT2U) */
#define mtibat2l(x)	mtspr	533,(x)			/* SPR 533 (IBAT2L) */
#define mtibat3u(x)	mtspr	534,(x)			/* SPR 534 (IBAT3U) */
#define mtibat3l(x)	mtspr	535,(x)			/* SPR 535 (IBAT3L) */
#define mtdbat0u(x)	mtspr	536,(x)			/* SPR 536 (DBAT0U) */
#define mtdbat0l(x)	mtspr	537,(x)			/* SPR 537 (DBAT0L) */
#define mtdbat1u(x)	mtspr	538,(x)			/* SPR 538 (DBAT1U) */
#define mtdbat1l(x)	mtspr	539,(x)			/* SPR 539 (DBAT1L) */
#define mtdbat2u(x)	mtspr	540,(x)			/* SPR 540 (DBAT2U) */
#define mtdbat2l(x)	mtspr	541,(x)			/* SPR 541 (DBAT2L) */
#define mtdbat3u(x)	mtspr	542,(x)			/* SPR 542 (DBAT3U) */
#define mtdbat3l(x)	mtspr	543,(x)			/* SPR 543 (DBAT3L) */


/* PowerPC processors do not necessarily initialize the BAT
   registers on power-up or reset.  So they are in an unknown
   state.  Before programming the BATs for the first time, all
   BAT registers MUST have their Vs and Vp bits cleared in the
   upper BAT half in order to avoid possibly having 2 BATs
   valid and mapping the same memory region.

   The reason for this is that, even with address translation
   disabled, multiple BAT hits for an address are treated as
   programming errors and can cause unpredictable results.

   It is up to the software to make sure it never has 2 IBAT
   mappings or 2 DBAT mappings that are valid for the same
   addresses.  It is not necessary to perform this code
   sequence every time the BATs are programmed, only when
   there is a possibility that there may be overlapping BAT
   entries.

   When programming the BATs in non-reset scenarios, even if
   you are sure that your new mapping will not temporarily
   create overlapping regions, it is still a wise idea to
   invalidate a BAT entry by setting its upper BAT register to
   all 0's before programming it.  This will avoid having a
   BAT marked valid that is in an unknown or transient state
*/

	addis   r5,0,0x0000
	mtibat0u(r5)
	mtibat0l(r5)
	mtibat1u(r5)
	mtibat1l(r5)
	mtibat2u(r5)
	mtibat2l(r5)
	mtibat3u(r5)
	mtibat3l(r5)
	mtdbat0u(r5)
	mtdbat0l(r5)
	mtdbat1u(r5)
	mtdbat1l(r5)
	mtdbat2u(r5)
	mtdbat2l(r5)
	mtdbat3u(r5)
	mtdbat3l(r5)
	isync

/*
 *  Set up I/D BAT0
 */
	lis     r4, CFG_DBAT0L@h
	ori     r4, r4, CFG_DBAT0L@l
	lis     r3, CFG_DBAT0U@h
	ori     r3, r3, CFG_DBAT0U@l

	mtdbat0l(r4)
	isync
	mtdbat0u(r3)
	isync
	sync

	lis     r4, CFG_IBAT0L@h
	ori     r4, r4, CFG_IBAT0L@l
	lis     r3, CFG_IBAT0U@h
	ori     r3, r3, CFG_IBAT0U@l

	isync
	mtibat0l(r4)
	isync
	mtibat0u(r3)
	isync

/*
 *  Set up I/D BAT1
 */
	lis     r4, CFG_IBAT1L@h
	ori     r4, r4, CFG_IBAT1L@l
	lis     r3, CFG_IBAT1U@h
	ori     r3, r3, CFG_IBAT1U@l

	isync
	mtibat1l(r4)
	isync
	mtibat1u(r3)
	isync
	mtdbat1l(r4)
	isync
	mtdbat1u(r3)
	isync
	sync

/*
 *  Set up I/D BAT2
 */
	lis     r4, CFG_IBAT2L@h
	ori     r4, r4, CFG_IBAT2L@l
	lis     r3, CFG_IBAT2U@h
	ori     r3, r3, CFG_IBAT2U@l

	isync
	mtibat2l(r4)
	isync
	mtibat2u(r3)
	isync
	mtdbat2l(r4)
	isync
	mtdbat2u(r3)
	isync
	sync

/*
 *  Setup I/D BAT3
 */
	lis     r4, CFG_IBAT3L@h
	ori     r4, r4, CFG_IBAT3L@l
	lis     r3, CFG_IBAT3U@h
	ori     r3, r3, CFG_IBAT3U@l

	isync
	mtibat3l(r4)
	isync
	mtibat3u(r3)
	isync
	mtdbat3l(r4)
	isync
	mtdbat3u(r3)
	isync
	sync


/*
 *  Invalidate all 64 TLB's
 */
	lis     r3, 0
	mtctr   r3
	lis     r5, 4

tlblp:
	tlbie   r3
	sync
	addi    r3, r3, 0x1000
	cmplw   r3, r5
	blt     tlblp

	sync

/*
 *  Enable Data Translation
 */
	lis	r4, MEMORY_MGMT_MSR_BITS@h
	ori	r4, r4, MEMORY_MGMT_MSR_BITS@l
	mfmsr	r3
	or	r3, r4, r3
	mtmsr	r3
	isync
	sync

	blr


#ifdef USE_V2_INIT
/* #define USER_I_CACHE_ENABLE 1*/ /* Fast rom boots */
/* Macro for hiadjust and lo */
#define HIADJ(arg)	arg@ha
#define HI(arg)		arg@h
#define LO(arg)		arg@l

#undef LOADPTR
#define	LOADPTR(reg,const32) \
	  addis reg,r0,HIADJ(const32); addi reg,reg,LO(const32)

.globl  early_init_f

early_init_f:
/* MPC8245/BMW CPCI System Init
 * Jimmy Blair, Broadcom Corp, 2002.
 */
	mflr    r11
	/* Zero-out registers */

	addis	r0,r0,0
	mtspr	SPRG0,r0
	mtspr	SPRG1,r0
	mtspr	SPRG2,r0
	mtspr	SPRG3,r0

	/* Set MPU/MSR to a known state. Turn on FP */

	LOADPTR (r3, MSR_FP)
	sync
	mtmsr	r3
	isync

	/* Init the floating point control/status register */

	mtfsfi	7,0x0
	mtfsfi	6,0x0
	mtfsfi	5,0x0
	mtfsfi	4,0x0
	mtfsfi	3,0x0
	mtfsfi	2,0x0
	mtfsfi	1,0x0
	mtfsfi	0,0x0
	isync

	/* Set MPU/MSR to a known state. Turn off FP */

#if 1	/* Turn off floating point (remove to keep FP on) */
	andi.	r3, r3, 0
	sync
	mtmsr 	r3
	isync
#endif

	/* Init the Segment registers */

	andi.	r3, r3, 0
	isync
	mtsr	0,r3
	isync
	mtsr	1,r3
	isync
	mtsr	2,r3
	isync
	mtsr	3,r3
	isync
	mtsr	4,r3
	isync
	mtsr	5,r3
	isync
	mtsr	6,r3
	isync
	mtsr	7,r3
	isync
	mtsr	8,r3
	isync
	mtsr	9,r3
	isync
	mtsr	10,r3
	isync
	mtsr	11,r3
	isync
	mtsr	12,r3
	isync
	mtsr	13,r3
	isync
	mtsr	14,r3
	isync
	mtsr	15,r3
	isync

	/* Turn off data and instruction cache control bits */

	mfspr	r3, HID0
	isync
	rlwinm	r4, r3, 0, 18, 15	/* r4 has ICE and DCE bits cleared */
	sync
	isync
	mtspr	HID0, r4		/* HID0 = r4 */
	isync

	/* Get cpu type */

	mfspr	r28, PVR
	rlwinm	r28, r28, 16, 16, 31

	/* invalidate the MPU's data/instruction caches */

	lis	r3, 0x0
	cmpli	0, 0, r28, CPU_TYPE_603
	beq	cpuIs603
	cmpli	0, 0, r28, CPU_TYPE_603E
	beq	cpuIs603
	cmpli	0, 0, r28, CPU_TYPE_603P
	beq	cpuIs603
	cmpli	0, 0, r28, CPU_TYPE_604R
	bne	cpuNot604R

cpuIs604R:
	lis	r3, 0x0
	mtspr	HID0, r3		/* disable the caches */
	isync
	ori	r4, r4, 0x0002		/* disable BTAC by setting bit 30 */

cpuNot604R:
	ori	r3, r3, (HID0_ICFI |HID0_DCI)

cpuIs603:
	ori	r3, r3, (HID0_ICE | HID0_DCE)
	or	r4, r4, r3		/* set bits */
	sync
	isync
	mtspr	HID0, r4		/* HID0 = r4 */
	andc	r4, r4, r3		/* clear bits */
	isync
	cmpli	0, 0, r28, CPU_TYPE_604
	beq	cpuIs604
	cmpli	0, 0, r28, CPU_TYPE_604E
	beq	cpuIs604
	cmpli	0, 0, r28, CPU_TYPE_604R
	beq	cpuIs604
	mtspr	HID0, r4
	isync

#ifdef USER_I_CACHE_ENABLE
	b	instCacheOn603
#else
	b	cacheEnableDone
#endif

cpuIs604:
	LOADPTR (r5, 0x1000)		/* loop count, 0x1000 */
	mtspr	CTR, r5
loopDelay:
	nop
	bdnz	loopDelay
	isync
	mtspr	HID0, r4
	isync

	/* turn the Instruction cache ON for faster FLASH ROM boots */

#ifdef USER_I_CACHE_ENABLE

	ori	r4, r4, (HID0_ICE | HID0_ICFI)
	isync				/* Synchronize for ICE enable */
	b	writeReg4
instCacheOn603:
	ori	r4, r4, (HID0_ICE | HID0_ICFI)
	rlwinm	r3, r4, 0, 21, 19	/* clear the ICFI bit */

	/*
	 * The setting of the instruction cache enable (ICE) bit must be
	 * preceded by an isync instruction to prevent the cache from being
	 * enabled or disabled while an instruction access is in progress.
	 */
	isync
writeReg4:
	mtspr	HID0, r4		/* Enable Instr Cache & Inval cache */
	cmpli	0, 0, r28, CPU_TYPE_604
	beq	cacheEnableDone
	cmpli	0, 0, r28, CPU_TYPE_604E
	beq	cacheEnableDone

	mtspr	HID0, r3		/* using 2 consec instructions */
					/* PPC603 recommendation */
#endif
cacheEnableDone:

	/* Detect map A or B */

	addis	r5,r0, HI(CHRP_REG_ADDR)
	addis	r6,r0, HI(CHRP_REG_DATA)
	LOADPTR (r7, KAHLUA_ID)		/* Kahlua PCI controller ID */
	LOADPTR (r8, BMC_BASE)

	stwbrx	r8,0,(r5)
	lwbrx	r3,0,(r6)		/* Store read value to r3 */
	cmp	0,0,r3,r7
	beq	cr0, X4_KAHLUA_START

	/* It's not an 8240, is it an 8245? */

	LOADPTR (r7, KAHLUA2_ID)	/* Kahlua PCI controller ID */
	cmp	0,0,r3,r7
	beq	cr0, X4_KAHLUA_START

	/* Save the PCI controller type in r7 */
	mr	r7, r3

	LOADPTR (r5, PREP_REG_ADDR)
	LOADPTR (r6, PREP_REG_DATA)

X4_KAHLUA_START:
	/* MPC8245 changes begin here */
	LOADPTR (r3, MPC107_PCI_CMD)	/* PCI command reg */
	stwbrx	r3,0,r5
	li	r4, 6			/* Command register value */
	sthbrx	r4, 0, r6

	LOADPTR (r3, MPC107_PCI_STAT)	/* PCI status reg */
	stwbrx	r3,0,r5
	li	r4, -1			/* Write-to-clear all bits */
	li	r3, 2			/* PCI_STATUS is at +2 offset */
	sthbrx	r4, r3, r6

	/*-------PROC_INT1_ADR */

	LOADPTR (r3, PROC_INT1_ADR)	/* Processor I/F Config 1 reg. */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0xff141b98)
	stwbrx	r4,0,r6

	/*-------PROC_INT2_ADR */

	LOADPTR (r3, PROC_INT2_ADR)	/* Processor I/F Config 2 reg. */
	stwbrx	r3,0,r5
	lis	r4, 0x2000		/* Flush PCI config writes */
	stwbrx	r4,0,r6

	LOADPTR (r9, KAHLUA2_ID)
	cmpl	0, 0, r7, r9
	bne     L1not8245

	/* MIOCR1 -- turn on bit for DLL delay */

	LOADPTR (r3, MIOCR1_ADR_X)
	stwbrx	r3,0,r5
	li      r4, 0x04
	stb	r4, MIOCR1_SHIFT(r6)

	/* For the MPC8245, set register 77 to %00100000 (see Errata #15) */
	/* SDRAM_CLK_DEL (0x77)*/

	LOADPTR (r3, MIOCR2_ADR_X)
	stwbrx	r3,0,r5
	li      r4, 0x10
	stb	r4, MIOCR2_SHIFT(r6)

	/* PMCR2 -- set PCI hold delay to <10>b for 33 MHz */

	LOADPTR (r3, PMCR2_ADR_X)
	stwbrx	r3,0,r5
	li      r4, 0x20
	stb	r4, PMCR2_SHIFT(r6)

	/* Initialize EUMBBAR early since 8245 has internal UART in EUMB */

	LOADPTR (r3, EUMBBAR)
	stwbrx	r3,0,r5
	LOADPTR (r4, CFG_EUMB_ADDR)
	stwbrx	r4,0,r6

L1not8245:

	/* Toggle the DLL reset bit in AMBOR */

	LOADPTR (r3, AMBOR)
	stwbrx	r3,0,r5
	lbz	r4, 0(r6)

	andi.   r4, r4, 0xdf
	stb	r4, 0(r6)		/* Clear DLL_RESET */
	sync

	ori     r4, r4, 0x20		/* Set DLL_RESET */
	stb	r4, 0(r6)
	sync

	andi.   r4, r4, 0xdf
	stb	r4, 0(r6)		/* Clear DLL_RESET */


	/* Enable RCS2, use supplied timings */
	LOADPTR (r3, ERCR1)
	stwbrx	r3,0,r5
	LOADPTR (r4, 0x80408000)
	stwbrx	r4,0,r6

	/* Disable RCS3 parameters */
	LOADPTR (r3, ERCR2)
	stwbrx	r3,0,r5
	LOADPTR (r4, 0x00000000)
	stwbrx	r4,0,r6

	/* RCS3 at 0x70000000, 64KBytes */
	LOADPTR (r3, ERCR2)
	stwbrx	r3,0,r5
	LOADPTR (r4, 0x00000004)
	stwbrx	r4,0,r6

	/*-------MCCR1 */

#ifdef INCLUDE_ECC
#define MC_ECC				1
#else /* INCLUDE_ECC */
#define MC_ECC				0
#endif /* INCLUDE_ECC */

#define MC1_ROMNAL			8		/* 0-15 */
#define MC1_ROMFAL			11		/* 0-31 */
#define MC1_DBUS_SIZE			0		/* 0-3, read only */
#define MC1_BURST			0		/* 0-1 */
#define MC1_MEMGO			0		/* 0-1 */
#define MC1_SREN			1		/* 0-1 */
#define MC1_RAM_TYPE			0		/* 0-1 */
#define MC1_PCKEN			MC_ECC		/* 0-1 */
#define MC1_BANKBITS			0x5555		/* 2 bits/bank 7-0 */

	LOADPTR (r3, MEM_CONT1_ADR)	/* Set MCCR1 (F0) */
	stwbrx	r3,0,r5
	LOADPTR(r4, \
		MC1_ROMNAL << 28 | MC1_ROMFAL << 23 | \
		MC1_DBUS_SIZE << 21 | MC1_BURST << 20 | \
		MC1_MEMGO << 19 | MC1_SREN << 18 | \
		MC1_RAM_TYPE << 17 | MC1_PCKEN << 16 )
	li	r3, MC1_BANKBITS
	cmpl	0, 0, r7, r9		/* Check for Kahlua2 */
	bne     BankBitsAdd
	cmpli   0, 0, r3, 0x5555
	beq     K2BankBitsHack		/* On 8245, 5555 ==> 0 */
BankBitsAdd:
	ori	r4, r3, 0
K2BankBitsHack:
	stwbrx	r4, 0, r6

	/*------- MCCR2 */

#define MC2_TS_WAIT_TIMER		0		/* 0-7 */
#define MC2_ASRISE			8		/* 0-15 */
#define MC2_ASFALL			4		/* 0-15 */
#define MC2_INLINE_PAR_NOT_ECC		0		/* 0-1 */
#define MC2_WRITE_PARITY_CHK_EN		MC_ECC		/* 0-1 */
#define MC2_INLRD_PARECC_CHK_EN		MC_ECC		/* 0-1 */
#define MC2_ECC_EN			0		/* 0-1 */
#define MC2_EDO				0		/* 0-1 */
/*
*  N.B. This refresh interval looks good up to 85 MHz with Hynix SDRAM.
*  May need to be decreased for 100 MHz
*/
#define MC2_REFINT			0x3a5		/* 0-0x3fff */
#define MC2_RSV_PG			0		/* 0-1 */
#define MC2_RMW_PAR			MC_ECC		/* 0-1 */

	LOADPTR (r3, MEM_CONT2_ADR)	/* Set MCCR2 (F4) */
	stwbrx	r3,0,r5
	LOADPTR(r4, \
		MC2_TS_WAIT_TIMER << 29 | MC2_ASRISE << 25 | \
		MC2_ASFALL << 21 | MC2_INLINE_PAR_NOT_ECC << 20 | \
		MC2_WRITE_PARITY_CHK_EN << 19 | \
		MC2_INLRD_PARECC_CHK_EN << 18 | \
		MC2_ECC_EN << 17 | MC2_EDO << 16 | \
		MC2_REFINT << 2 | MC2_RSV_PG << 1 | MC2_RMW_PAR)
	cmpl	0, 0, r7, r9		/* Check for Kahlua2 */
	bne     notK2
	/* clear Kahlua2 reserved bits */
	LOADPTR (r3, 0xfffcffff)
	and	r4, r4, r3
notK2:
	stwbrx	r4,0,r6

	/*------- MCCR3 */

#define MC_BSTOPRE			0x079		/* 0-0x7ff */

#define MC3_BSTOPRE_U			(MC_BSTOPRE >> 4 & 0xf)
#define MC3_REFREC			8		/* 0-15 */
#define MC3_RDLAT			(4+MC_ECC)	/* 0-15 */
#define MC3_CPX				0		/* 0-1 */
#define MC3_RAS6P			0		/* 0-15 */
#define MC3_CAS5			0		/* 0-7 */
#define MC3_CP4				0		/* 0-7 */
#define MC3_CAS3			0		/* 0-7 */
#define MC3_RCD2			0		/* 0-7 */
#define MC3_RP1				0		/* 0-7 */

	LOADPTR (r3, MEM_CONT3_ADR)	/* Set MCCR3 (F8) */
	stwbrx	r3,0,r5
	LOADPTR(r4, \
		MC3_BSTOPRE_U << 28 | MC3_REFREC << 24 | \
		MC3_RDLAT << 20 | MC3_CPX << 19 | \
		MC3_RAS6P << 15 | MC3_CAS5 << 12 | MC3_CP4 << 9 | \
		MC3_CAS3 << 6 | MC3_RCD2 << 3 | MC3_RP1)
	cmpl	0, 0, r7, r9              /* Check for Kahlua2 */
	bne     notK2b
	/* clear Kahlua2 reserved bits */
	LOADPTR (r3, 0xff000000)
	and	r4, r4, r3
notK2b:
	stwbrx	r4,0,r6

	/*------- MCCR4 */

#define MC4_PRETOACT			3		/* 0-15 */
#define MC4_ACTOPRE			5		/* 0-15 */
#define MC4_WMODE			0		/* 0-1 */
#define MC4_INLINE			MC_ECC		/* 0-1 */
#define MC4_REGISTERED			(1-MC_ECC)	/* 0-1 */
#define MC4_BSTOPRE_UU			(MC_BSTOPRE >> 8 & 3)
#define MC4_REGDIMM			0		/* 0-1 */
#define MC4_SDMODE_CAS			2		/* 0-7 */
#define MC4_DBUS_RCS1			1		/* 0-1, 8-bit */
#define MC4_SDMODE_WRAP			0		/* 0-1 */
#define MC4_SDMODE_BURST		2		/* 0-7 */
#define MC4_ACTORW			3		/* 0-15 */
#define MC4_BSTOPRE_L			(MC_BSTOPRE & 0xf)

	LOADPTR (r3, MEM_CONT4_ADR)	/* Set MCCR4 (FC) */
	stwbrx	r3,0,r5
	LOADPTR(r4, \
		MC4_PRETOACT << 28 | MC4_ACTOPRE << 24 | \
		MC4_WMODE << 23 | MC4_INLINE << 22 | \
		MC4_REGISTERED << 20 | MC4_BSTOPRE_UU << 18 | \
		MC4_DBUS_RCS1 << 17 | \
		MC4_REGDIMM << 15 | MC4_SDMODE_CAS << 12 | \
		MC4_SDMODE_WRAP << 11 | MC4_SDMODE_BURST << 8 | \
		MC4_ACTORW << 4 | MC4_BSTOPRE_L)
	cmpl	0, 0, r7, r9                /* Check for Kahlua 2 */
	bne     notK2c
	/* Turn on Kahlua2 extended ROM space */
	LOADPTR (r3, 0x00200000)
	or	r4, r4, r3
notK2c:
	stwbrx	r4,0,r6

#ifdef INCLUDE_ECC
	/*------- MEM_ERREN1 */

	LOADPTR (r3, MEM_ERREN1_ADR)	/* Set MEM_ERREN1 (c0) */
	stwbrx	r3,0,r5
	lwbrx	r4,0,r6
	ori	r4,r4,4			/* Set MEM_PERR_EN */
	stwbrx	r4,0,r6
#endif /* INCLUDE_ECC */

	/*------- MSAR/MEAR */

	LOADPTR (r3, MEM_START1_ADR)	/* Set MSAR1 (80) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0xc0804000)
	stwbrx	r4,0,r6

	LOADPTR (r3, MEM_START2_ADR)	/* Set MSAR2 (84) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0xc0804000)
	stwbrx	r4,0,r6

	LOADPTR (r3, XMEM_START1_ADR)	/* Set MESAR1 (88) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0x00000000)
	stwbrx	r4,0,r6

	LOADPTR (r3, XMEM_START2_ADR)	/* Set MESAR2 (8c) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0x01010101)
	stwbrx	r4,0,r6

	LOADPTR (r3, MEM_END1_ADR)	/* Set MEAR1 (90) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0xffbf7f3f)
	stwbrx	r4,0,r6

	LOADPTR (r3, MEM_END2_ADR)	/* Set MEAR2 (94) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0xffbf7f3f)
	stwbrx	r4,0,r6

	LOADPTR (r3, XMEM_END1_ADR)	/* MEEAR1 (98) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0x00000000)
	stwbrx	r4,0,r6

	LOADPTR (r3, XMEM_END2_ADR)	/* MEEAR2 (9c) */
	stwbrx	r3,0,r5
	LOADPTR (r4, 0x01010101)
	stwbrx	r4,0,r6

	/*-------ODCR */

	LOADPTR (r3, ODCR_ADR_X)	/* Set ODCR */
	stwbrx	r3,0,r5

	li	r4, 0x7f
	stb	r4, ODCR_SHIFT(r6)	/* ODCR is at +3 offset */

	/*-------MBEN */

	LOADPTR (r3, MEM_EN_ADR)	/* Set MBEN (a0) */
	stwbrx	r3,0,r5
	li	r4, 0x01		/* Enable bank 0 */
	stb	r4, 0(r6)		/* MBEN is at +0 offset */

#if 0   /* Jimmy:  I think page made is broken */
	/*-------PGMAX */

	LOADPTR (r3, MPM_ADR_X)
	stwbrx	r3,0,r5
	li	r4, 0x32
	stb	r4, MPM_SHIFT(r6)		/* PAGE_MODE is at +3 offset */
#endif

	/* Wait before initializing other registers */

	lis	r4,0x0001
	mtctr	r4

KahluaX4wait200us:
	bdnz	KahluaX4wait200us

	/* Set MEMGO bit */

	LOADPTR (r3, MEM_CONT1_ADR)	/* MCCR1 (F0) |= PGMAX */
	stwbrx	r3,0,r5
	lwbrx	r4,0,r6			/* old MCCR1 */
	oris	r4,r4,0x0008		/* MEMGO=1 */
	stwbrx	r4, 0, r6

	/* Wait again */

	addis	r4,r0,0x0002
	ori	r4,r4,0xffff

	mtctr	r4

KahluaX4wait8ref:
	bdnz	KahluaX4wait8ref

	sync
	eieio
	mtlr    r11
	blr

#else /* USE_V2_INIT */


/* U-Boot works, but memory will not run reliably for all address ranges.
 * Early U-Boot Working init, but 2.4.19 kernel will crash since memory is not
 * initialized correctly. Could work if debugged.
 */
/* PCI Support routines */

    .globl __pci_config_read_32
__pci_config_read_32:
    lis     r4, 0xfec0
    stwbrx   r3, r0, r4
    sync
    lis     r4, 0xfee0
    lwbrx   r3, 0, r4
    blr
    .globl __pci_config_read_16
__pci_config_read_16:
    lis     r4, 0xfec0
    andi.    r5, r3, 2
    stwbrx  r3, r0, r4
    sync
    oris     r4, r5, 0xfee0
    lhbrx    r3, r0, r4
    blr
    .globl __pci_config_read_8
__pci_config_read_8:
    lis     r4, 0xfec0
    andi.    r5, r3, 3
    stwbrx  r3, r0, r4
    sync
    oris     r4, r5, 0xfee0
    lbz      r3, 0(4)
    blr
    .globl __pci_config_write_32
__pci_config_write_32:
    lis     r5, 0xfec0
    stwbrx   r3, r0, r5
    sync
    lis      r5, 0xfee0
    stwbrx   r4, r0, r5
    sync
    blr
    .globl __pci_config_write_16
__pci_config_write_16:
    lis     r5, 0xfec0
    andi.    r6, r3, 2
    stwbrx  r3, r0, 5
    sync
    oris     r5, r6, 0xfee0
    sthbrx    r4, r0, r5
    sync
    blr
    .globl __pci_config_write_8
__pci_config_write_8:
    lis      r5, 0xfec0
    andi.    r6, r3, 3
    stwbrx   r3, r0, r5
    sync
    oris      r5, r6, 0xfee0
    stb       r4, 0(r5)
    sync
    blr
    .globl  in_8
in_8:
    oris    r3, r3, 0xfe00
    lbz     r3,0(r3)
    blr
    .globl  in_16
in_16:
    oris    r3, r3, 0xfe00
    lhbrx   r3, 0, r3
    blr
    .globl in_16_ne
in_16_ne:
    oris    r3, r3, 0xfe00
    lhzx    r3, 0, r3
    blr
    .globl  in_32
in_32:
    oris    r3, r3, 0xfe00
    lwbrx   r3, 0, r3
    blr
    .globl  out_8
out_8:
    oris    r3, r3, 0xfe00
    stb     r4, 0(r3)
    eieio
    blr
    .globl  out_16
out_16:
    oris    r3, r3, 0xfe00
    sthbrx  r4, 0, r3
    eieio
    blr
    .globl  out_16_ne
out_16_ne:
    oris    r3, r3, 0xfe00
    sth     r4, 0(r3)
    eieio
    blr
    .globl  out_32
out_32:
    oris    r3, r3, 0xfe00
    stwbrx  r4, 0, r3
    eieio
    blr
    .globl  read_8
read_8:
    lbz     r3,0(r3)
    blr
    .globl  read_16
read_16:
    lhbrx   r3, 0, r3
    blr
    .globl  read_32
read_32:
    lwbrx   r3, 0, r3
    blr
    .globl  read_32_ne
read_32_ne:
    lwz     r3, 0(r3)
    blr
    .globl  write_8
write_8:
    stb     r4, 0(r3)
    eieio
    blr
    .globl  write_16
write_16:
    sthbrx  r4, 0, r3
    eieio
    blr
    .globl  write_32
write_32:
    stwbrx  r4, 0, r3
    eieio
    blr
    .globl  write_32_ne
write_32_ne:
    stw     r4, 0(r3)
    eieio
    blr


.globl  early_init_f

early_init_f:
	mflr    r11
	lis     r10, 0x8000

	/* PCI Latency Timer */
	li      r4, 0x0d
	ori     r3, r10, PLTR@l
	bl      __pci_config_write_8

	/* Cache Line Size */
	li      r4, 0x08
	ori     r3, r10, PCLSR@l
	bl      __pci_config_write_8

	/* PCI Cmd */
	li      r4, 6
	ori     r3, r10, PCICR@l
	bl      __pci_config_write_16

#if 1
	/* PCI Stat */
	ori     r3, r10, PCISR@l
	bl      __pci_config_read_16
	ori     r4, r4, 0xffff
	ori     r3, r10, PCISR@l
	bl      __pci_config_write_16
#endif

	/* PICR1 */
	lis     r4, 0xff14
	ori     r4, r4, 0x1b98
	ori     r3, r10, PICR1@l
	bl      __pci_config_write_32


	/* PICR2 */
	lis     r4, 0x0404
	ori     r4, r4, 0x0004
	ori     r3, r10, PICR2@l
	bl      __pci_config_write_32

	/* MIOCR1 */
	li      r4, 0x04
	ori     r3, r10, MIOCR1@l
	bl      __pci_config_write_8

	/* For the MPC8245, set register 77 to %00100000 (see Errata #15) */
	/* SDRAM_CLK_DEL (0x77)*/
	li      r4, 0x10
	ori     r3, r10, MIOCR2@l
	bl      __pci_config_write_8

	/* EUMBBAR */
	lis     r4, 0xfc00
	ori     r3, r10, EUMBBAR@l
	bl      __pci_config_write_32

	/* AMBOR */

       /* Even if Address Map B is not being used (though it should),
	* the memory DLL needs to be cleared/set/cleared before using memory.
	*/

	ori     r3, r10, AMBOR@l
	bl      __pci_config_read_8	/* get Current bits */

	andi.   r4, r4, 0xffdf
	ori     r3, r10, AMBOR@l
	bl      __pci_config_write_16	/* Clear DLL_RESET */

	ori    r4, r4, 0x0020
	ori     r3, r10, AMBOR@l
	bl      __pci_config_write_16	/* Set DLL_RESET */

	andi.   r4, r4, 0xffdf
	ori     r3, r10, AMBOR@l
	bl      __pci_config_write_16	/* Clear DLL_RESET */

	/* ERCR1 */
	lis     r4, 0x8040		/* Enable RCS2, use supplied timings */
	ori     r4, r4, 0x8000
	ori     r3, r10, ERCR1@l
	bl      __pci_config_write_32

	/* ERCR2 */
	lis     r4, 0x0000		/* Disable RCS3 parms */
	ori     r4, r4, 0x0000
	ori     r3, r10, ERCR2@l
	bl      __pci_config_write_32

	/* ERCR3 */
	lis     r4, 0x0000		/* RCS3 at 0x70000000, 64K bytes */
	ori     r4, r4, 0x0004
	ori     r3, r10, ERCR2@l
	bl      __pci_config_write_32

	/* Preserve memgo bit */
	/* MCCR1 */

/*	lis     r4, 0x75a8		/  Safe Local ROM = 11+3 clocks */
	lis     r4, 0x75a0              /* Safe Local ROM = 11+3 clocks */
/*      lis     r4, 0x73a0              /  Fast Local ROM = 7+3 clocks */
/*      oris    r4, r4, 0x0010          /  Burst ROM/Flash enable */
/*      oris    r4, r4, 0x0004          /  Self-refresh enable */

/*      ori     r4,r4,0xFFFF            /  16Mbit  2bank SDRAM */
/*      ori     r4,r4,0xAAAA            /  256Mbit 4bank SDRAM (8245 only) */
/*      ori     r4,r4,0x5555            /  64Mbit  2bank SDRAM */
	ori     r4,r4,0x0000            /* 64Mbit  4bank SDRAM */

	ori     r3, r10, MCCR1@l
	bl      __pci_config_write_32

	/* MCCR2 */

	lis     r4,0x0000
/*      oris    r4,r4,0x4000            /  TS_WAIT_TIMER = 3 clocks */
	oris    r4,r4,0x1000            /* ASRISE = 8 clocks */
	oris    r4,r4,0x0080            /* ASFALL = 8 clocks */
/*      oris    r4,r4,0x0010            /  SDRAM Parity (else ECC) */
/*      oris    r4,r4,0x0008            /  Write parity check */
/*      oris    r4,r4,0x0004            /  SDRAM inline reads */


/* Select a refresh rate; it needs to match the bus speed; if too */
/* slow, data may be lost; if too fast, performance is lost.  We */
/* use the fastest value so we run at all speeds. */
/* Refresh = (15600ns/busclk) - (213 (see UM)). */

/*      ori     r4,r4,0x1d2c            /  133 MHz mem bus        = 1867 */
/*      ori     r4,r4,0x150c            /  100 MHz mem bus        = 1347 */
/*      ori     r4,r4,0x10fc            /   83 MHz mem bus        = 1087 */
/*      ori     r4,r4,0x0cc4            /   66 MHz mem bus        =  817 */
	ori     r4,r4,0x04cc            /*  33 MHz mem bus (SAFE) =  307 */
/*      ori     r4,r4,0x0002            /  Reserve a page */
/*      ori     r4,r4,0x0001            /  RWM parity */

	ori     r3, r10, MCCR2@l
	bl      __pci_config_write_32


	/* MCCR3 */
	lis     r4,0x0000               /* BSTOPRE_M = 7 (see A/N) */
	oris    r4,r4,0x0500            /* REFREC    = 8 clocks */
	ori     r3, r10, MCCR3@l
	bl      __pci_config_write_32

	/* MCCR4 */                     /* Turn on registered buffer mode */
	lis     r4, 0x2000              /* PRETOACT = 3 clocks */
	oris    r4,r4,0x0400            /* ACTOPRE  = 5 clocks */
/*      oris    r4,r4,0x0080            /  Enable 8-beat burst (32-bit bus) */
/*      oris    r4,r4,0x0040            /  Enable Inline ECC/Parity */
	oris    r4,r4,0x0020            /* EXTROM enabled */
	oris    r4,r4,0x0010            /* Registered buffers */
/*      oris    r4,r4,0x0000            /  BSTOPRE_U = 0 (see A/N) */
	oris    r4,r4,0x0002            /* DBUS_SIZ[2] (8 bit on RCS1) */

/*      ori     r4,r4,0x8000            /  Registered DIMMs */
	ori     r4,r4,0x2000            /*CAS Latency (CL=3) (see RDLAT) */
/*      ori     r4,r4,0x2000            /  CAS Latency (CL=2) (see RDLAT) */
/*      ori     r4,r4,0x0300            /  Sequential wrap/8-beat burst */
	ori     r4,r4,0x0200            /* Sequential wrap/4-beat burst */
	ori     r4,r4,0x0030            /* ACTORW  = 3 clocks */
	ori     r4,r4,0x0009            /* BSTOPRE_L = 9 (see A/N) */

	ori     r3, r10, MCCR4@l
	bl      __pci_config_write_32

	/* MSAR1 */
	lis     r4, 0xc0804000@h
	ori     r4, r4, 0xc0804000@l
	ori     r3, r10, MSAR1@l
	bl      __pci_config_write_32

	/* MSAR2 */
	lis     r4, 0xc0804000@h
	ori     r4, r4, 0xc0804000@l
	ori     r3, r10, MSAR2@l
	bl      __pci_config_write_32

	/* MESAR1 */
	lis     r4, 0x00000000@h
	ori     r4, r4, 0x00000000@l
	ori     r3, r10, EMSAR1@l
	bl      __pci_config_write_32

	/* MESAR2 */
	lis     r4, 0x01010101@h
	ori     r4, r4, 0x01010101@l
	ori     r3, r10, EMSAR2@l
	bl      __pci_config_write_32

	/* MEAR1 */
	lis     r4, 0xffbf7f3f@h
	ori     r4, r4, 0xffbf7f3f@l
	ori     r3, r10, MEAR1@l
	bl      __pci_config_write_32

	/* MEAR2 */
	lis     r4, 0xffbf7f3f@h
	ori     r4, r4, 0xffbf7f3f@l
	ori     r3, r10, MEAR2@l
	bl      __pci_config_write_32

	/* MEEAR1 */
	lis     r4, 0x00000000@h
	ori     r4, r4, 0x00000000@l
	ori     r3, r10, EMEAR1@l
	bl      __pci_config_write_32

	/* MEEAR2 */
	lis     r4, 0x01010101@h
	ori     r4, r4, 0x01010101@l
	ori     r3, r10, EMEAR2@l
	bl      __pci_config_write_32

	/* ODCR */
	li      r4, 0x7f
	ori     r3, r10, ODCR@l
	bl      __pci_config_write_8

	/* MBER */
	li      r4, 0x01
	ori     r3, r10, MBER@l
	bl      __pci_config_write_8

	/* Page CTR aka PGMAX */
	li      r4, 0x32
	ori     r3, r10, 0x70
	bl      __pci_config_write_8

#if 0
	/* CLK Drive */
	ori     r4, r10, 0xfc01 /* Top bit will be ignored */
	ori     r3, r10, 0x74
	bl      __pci_config_write_16
#endif

	/* delay */
	lis     r7, 1
	mtctr   r7
label1: 	bdnz    label1

	/* Set memgo bit */
	/* MCCR1 */
	ori     r3, r10, MCCR1@l
	bl      __pci_config_read_32
	lis	r7, 0x0008
	or	r4, r3, r7
	ori     r3, r10, MCCR1@l
	bl      __pci_config_write_32

	/* delay again */
	lis     r7, 1
	mtctr   r7
label2: 	bdnz    label2
#if 0
/* DEBUG: Infinite loop, write then read */
loop:
	lis     r7, 0xffff
	mtctr   r7
	li	r3, 0x5004
	lis	r4, 0xa0a0
	ori	r4, r4, 0x5050
	bl write_32_ne
	li	r3, 0x5004
	bl read_32_ne
	bdnz    loop
#endif
	mtlr    r11
	blr
#endif
