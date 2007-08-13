#ifndef __ASM_PPC_PROCESSOR_H
#define __ASM_PPC_PROCESSOR_H

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#include <linux/config.h>

#include <asm/ptrace.h>
#include <asm/types.h>

/* Machine State Register (MSR) Fields */

#ifdef CONFIG_PPC64BRIDGE
#define MSR_SF		(1<<63)
#define MSR_ISF		(1<<61)
#endif /* CONFIG_PPC64BRIDGE */
#define MSR_UCLE        (1<<26)         /* User-mode cache lock enable (e500) */
#define MSR_VEC		(1<<25)		/* Enable AltiVec(74xx) */
#define MSR_SPE         (1<<25)         /* Enable SPE(e500) */
#define MSR_POW		(1<<18)		/* Enable Power Management */
#define MSR_WE		(1<<18)		/* Wait State Enable */
#define MSR_TGPR	(1<<17)		/* TLB Update registers in use */
#define MSR_CE		(1<<17)		/* Critical Interrupt Enable */
#define MSR_ILE		(1<<16)		/* Interrupt Little Endian */
#define MSR_EE		(1<<15)		/* External Interrupt Enable */
#define MSR_PR		(1<<14)		/* Problem State / Privilege Level */
#define MSR_FP		(1<<13)		/* Floating Point enable */
#define MSR_ME		(1<<12)		/* Machine Check Enable */
#define MSR_FE0		(1<<11)		/* Floating Exception mode 0 */
#define MSR_SE		(1<<10)		/* Single Step */
#define MSR_DWE         (1<<10)         /* Debug Wait Enable (4xx) */
#define MSR_UBLE        (1<<10)         /* BTB lock enable (e500) */
#define MSR_BE		(1<<9)		/* Branch Trace */
#define MSR_DE		(1<<9)		/* Debug Exception Enable */
#define MSR_FE1		(1<<8)		/* Floating Exception mode 1 */
#define MSR_IP		(1<<6)		/* Exception prefix 0x000/0xFFF */
#define MSR_IR		(1<<5)		/* Instruction Relocate */
#define MSR_IS          (1<<5)          /* Book E Instruction space */
#define MSR_DR		(1<<4)		/* Data Relocate */
#define MSR_DS          (1<<4)          /* Book E Data space */
#define MSR_PE		(1<<3)		/* Protection Enable */
#define MSR_PX		(1<<2)		/* Protection Exclusive Mode */
#define MSR_PMM         (1<<2)          /* Performance monitor mark bit (e500) */
#define MSR_RI		(1<<1)		/* Recoverable Exception */
#define MSR_LE		(1<<0)		/* Little Endian */

#ifdef CONFIG_APUS_FAST_EXCEPT
#define MSR_		MSR_ME|MSR_IP|MSR_RI
#else
#define MSR_		MSR_ME|MSR_RI
#endif
#ifndef CONFIG_E500
#define MSR_KERNEL      MSR_|MSR_IR|MSR_DR
#else
#define MSR_KERNEL	MSR_ME
#endif

/* Floating Point Status and Control Register (FPSCR) Fields */

#define FPSCR_FX	0x80000000	/* FPU exception summary */
#define FPSCR_FEX	0x40000000	/* FPU enabled exception summary */
#define FPSCR_VX	0x20000000	/* Invalid operation summary */
#define FPSCR_OX	0x10000000	/* Overflow exception summary */
#define FPSCR_UX	0x08000000	/* Underflow exception summary */
#define FPSCR_ZX	0x04000000	/* Zero-devide exception summary */
#define FPSCR_XX	0x02000000	/* Inexact exception summary */
#define FPSCR_VXSNAN	0x01000000	/* Invalid op for SNaN */
#define FPSCR_VXISI	0x00800000	/* Invalid op for Inv - Inv */
#define FPSCR_VXIDI	0x00400000	/* Invalid op for Inv / Inv */
#define FPSCR_VXZDZ	0x00200000	/* Invalid op for Zero / Zero */
#define FPSCR_VXIMZ	0x00100000	/* Invalid op for Inv * Zero */
#define FPSCR_VXVC	0x00080000	/* Invalid op for Compare */
#define FPSCR_FR	0x00040000	/* Fraction rounded */
#define FPSCR_FI	0x00020000	/* Fraction inexact */
#define FPSCR_FPRF	0x0001f000	/* FPU Result Flags */
#define FPSCR_FPCC	0x0000f000	/* FPU Condition Codes */
#define FPSCR_VXSOFT	0x00000400	/* Invalid op for software request */
#define FPSCR_VXSQRT	0x00000200	/* Invalid op for square root */
#define FPSCR_VXCVI	0x00000100	/* Invalid op for integer convert */
#define FPSCR_VE	0x00000080	/* Invalid op exception enable */
#define FPSCR_OE	0x00000040	/* IEEE overflow exception enable */
#define FPSCR_UE	0x00000020	/* IEEE underflow exception enable */
#define FPSCR_ZE	0x00000010	/* IEEE zero divide exception enable */
#define FPSCR_XE	0x00000008	/* FP inexact exception enable */
#define FPSCR_NI	0x00000004	/* FPU non IEEE-Mode */
#define FPSCR_RN	0x00000003	/* FPU rounding control */

/* Special Purpose Registers (SPRNs)*/

#define SPRN_CDBCR	0x3D7	/* Cache Debug Control Register */
#define SPRN_CTR	0x009	/* Count Register */
#define SPRN_DABR	0x3F5	/* Data Address Breakpoint Register */
#ifndef CONFIG_BOOKE
#define SPRN_DAC1	0x3F6	/* Data Address Compare 1 */
#define SPRN_DAC2	0x3F7	/* Data Address Compare 2 */
#else
#define SPRN_DAC1       0x13C   /* Book E Data Address Compare 1 */
#define SPRN_DAC2       0x13D   /* Book E Data Address Compare 2 */
#endif  /* CONFIG_BOOKE */
#define SPRN_DAR	0x013	/* Data Address Register */
#define SPRN_DBAT0L	0x219	/* Data BAT 0 Lower Register */
#define SPRN_DBAT0U	0x218	/* Data BAT 0 Upper Register */
#define SPRN_DBAT1L	0x21B	/* Data BAT 1 Lower Register */
#define SPRN_DBAT1U	0x21A	/* Data BAT 1 Upper Register */
#define SPRN_DBAT2L	0x21D	/* Data BAT 2 Lower Register */
#define SPRN_DBAT2U	0x21C	/* Data BAT 2 Upper Register */
#define SPRN_DBAT3L	0x21F	/* Data BAT 3 Lower Register */
#define SPRN_DBAT3U	0x21E	/* Data BAT 3 Upper Register */
#define SPRN_DBAT4L	0x239   /* Data BAT 4 Lower Register */
#define SPRN_DBAT4U	0x238   /* Data BAT 4 Upper Register */
#define SPRN_DBAT5L	0x23B   /* Data BAT 5 Lower Register */
#define SPRN_DBAT5U	0x23A   /* Data BAT 5 Upper Register */
#define SPRN_DBAT6L	0x23D   /* Data BAT 6 Lower Register */
#define SPRN_DBAT6U	0x23C   /* Data BAT 6 Upper Register */
#define SPRN_DBAT7L	0x23F   /* Data BAT 7 Lower Register */
#define SPRN_DBAT7U	0x23E   /* Data BAT 7 Lower Register */
#define SPRN_DBCR	0x3F2	/* Debug Control Regsiter */
#define   DBCR_EDM	0x80000000
#define   DBCR_IDM	0x40000000
#define   DBCR_RST(x)	(((x) & 0x3) << 28)
#define     DBCR_RST_NONE		0
#define     DBCR_RST_CORE		1
#define     DBCR_RST_CHIP		2
#define     DBCR_RST_SYSTEM		3
#define   DBCR_IC	0x08000000	/* Instruction Completion Debug Evnt */
#define   DBCR_BT	0x04000000	/* Branch Taken Debug Event */
#define   DBCR_EDE	0x02000000	/* Exception Debug Event */
#define   DBCR_TDE	0x01000000	/* TRAP Debug Event */
#define   DBCR_FER	0x00F80000	/* First Events Remaining Mask */
#define   DBCR_FT	0x00040000	/* Freeze Timers on Debug Event */
#define   DBCR_IA1	0x00020000	/* Instr. Addr. Compare 1 Enable */
#define   DBCR_IA2	0x00010000	/* Instr. Addr. Compare 2 Enable */
#define   DBCR_D1R	0x00008000	/* Data Addr. Compare 1 Read Enable */
#define   DBCR_D1W	0x00004000	/* Data Addr. Compare 1 Write Enable */
#define   DBCR_D1S(x)	(((x) & 0x3) << 12)	/* Data Adrr. Compare 1 Size */
#define     DAC_BYTE	0
#define     DAC_HALF	1
#define     DAC_WORD	2
#define     DAC_QUAD	3
#define   DBCR_D2R	0x00000800	/* Data Addr. Compare 2 Read Enable */
#define   DBCR_D2W	0x00000400	/* Data Addr. Compare 2 Write Enable */
#define   DBCR_D2S(x)	(((x) & 0x3) << 8)	/* Data Addr. Compare 2 Size */
#define   DBCR_SBT	0x00000040	/* Second Branch Taken Debug Event */
#define   DBCR_SED	0x00000020	/* Second Exception Debug Event */
#define   DBCR_STD	0x00000010	/* Second Trap Debug Event */
#define   DBCR_SIA	0x00000008	/* Second IAC Enable */
#define   DBCR_SDA	0x00000004	/* Second DAC Enable */
#define   DBCR_JOI	0x00000002	/* JTAG Serial Outbound Int. Enable */
#define   DBCR_JII	0x00000001	/* JTAG Serial Inbound Int. Enable */
#ifndef CONFIG_BOOKE
#define SPRN_DBCR0      0x3F2           /* Debug Control Register 0 */
#else
#define SPRN_DBCR0      0x134           /* Book E Debug Control Register 0 */
#endif /* CONFIG_BOOKE */
#ifndef CONFIG_BOOKE
#define SPRN_DBCR1	0x3BD	/* Debug Control Register 1 */
#define SPRN_DBSR	0x3F0	/* Debug Status Register */
#else
#define SPRN_DBCR1      0x135           /* Book E Debug Control Register 1 */
#define SPRN_DBSR       0x130           /* Book E Debug Status Register */
#define   DBSR_IC           0x08000000  /* Book E Instruction Completion  */
#define   DBSR_TIE          0x01000000  /* Book E Trap Instruction Event */
#endif /* CONFIG_BOOKE */
#define SPRN_DCCR	0x3FA	/* Data Cache Cacheability Register */
#define   DCCR_NOCACHE		0	/* Noncacheable */
#define   DCCR_CACHE		1	/* Cacheable */
#define SPRN_DCMP	0x3D1	/* Data TLB Compare Register */
#define SPRN_DCWR	0x3BA	/* Data Cache Write-thru Register */
#define   DCWR_COPY		0	/* Copy-back */
#define   DCWR_WRITE		1	/* Write-through */
#ifndef CONFIG_BOOKE
#define SPRN_DEAR	0x3D5	/* Data Error Address Register */
#else
#define SPRN_DEAR       0x03D   /* Book E Data Error Address Register */
#endif /* CONFIG_BOOKE */
#define SPRN_DEC	0x016	/* Decrement Register */
#define SPRN_DMISS	0x3D0	/* Data TLB Miss Register */
#define SPRN_DSISR	0x012	/* Data Storage Interrupt Status Register */
#define SPRN_EAR	0x11A	/* External Address Register */
#ifndef CONFIG_BOOKE
#define SPRN_ESR	0x3D4	/* Exception Syndrome Register */
#else
#define SPRN_ESR        0x03E           /* Book E Exception Syndrome Register */
#endif /* CONFIG_BOOKE */
#define   ESR_IMCP	0x80000000	/* Instr. Machine Check - Protection */
#define   ESR_IMCN	0x40000000	/* Instr. Machine Check - Non-config */
#define   ESR_IMCB	0x20000000	/* Instr. Machine Check - Bus error */
#define   ESR_IMCT	0x10000000	/* Instr. Machine Check - Timeout */
#define   ESR_PIL	0x08000000	/* Program Exception - Illegal */
#define   ESR_PPR	0x04000000	/* Program Exception - Priveleged */
#define   ESR_PTR	0x02000000	/* Program Exception - Trap */
#define   ESR_DST	0x00800000	/* Storage Exception - Data miss */
#define   ESR_DIZ	0x00400000	/* Storage Exception - Zone fault */
#define SPRN_EVPR	0x3D6	/* Exception Vector Prefix Register */
#define SPRN_HASH1	0x3D2	/* Primary Hash Address Register */
#define SPRN_HASH2	0x3D3	/* Secondary Hash Address Resgister */
#define SPRN_HID0	0x3F0	/* Hardware Implementation Register 0 */

#define HID0_ICE_SHIFT		15
#define HID0_DCE_SHIFT		14
#define HID0_DLOCK_SHIFT	12

#define   HID0_EMCP	(1<<31)		/* Enable Machine Check pin */
#define   HID0_EBA	(1<<29)		/* Enable Bus Address Parity */
#define   HID0_EBD	(1<<28)		/* Enable Bus Data Parity */
#define   HID0_SBCLK	(1<<27)
#define   HID0_EICE	(1<<26)
#define   HID0_ECLK	(1<<25)
#define   HID0_PAR	(1<<24)
#define   HID0_DOZE	(1<<23)
#define   HID0_NAP	(1<<22)
#define   HID0_SLEEP	(1<<21)
#define   HID0_DPM	(1<<20)
#define   HID0_ICE	(1<<HID0_ICE_SHIFT)	/* Instruction Cache Enable */
#define   HID0_DCE	(1<<HID0_DCE_SHIFT)	/* Data Cache Enable */
#define   HID0_ILOCK	(1<<13)		/* Instruction Cache Lock */
#define   HID0_DLOCK	(1<<HID0_DLOCK_SHIFT)	/* Data Cache Lock */
#define   HID0_ICFI	(1<<11)		/* Instr. Cache Flash Invalidate */
#define   HID0_DCFI	(1<<10)		/* Data Cache Flash Invalidate */
#define   HID0_DCI	HID0_DCFI
#define   HID0_SPD	(1<<9)		/* Speculative disable */
#define   HID0_SGE	(1<<7)		/* Store Gathering Enable */
#define   HID0_SIED	HID_SGE		/* Serial Instr. Execution [Disable] */
#define   HID0_DCFA	(1<<6)		/* Data Cache Flush Assist */
#define   HID0_BTIC	(1<<5)		/* Branch Target Instruction Cache Enable */
#define   HID0_ABE	(1<<3)		/* Address Broadcast Enable */
#define   HID0_BHTE	(1<<2)		/* Branch History Table Enable */
#define   HID0_BTCD	(1<<1)		/* Branch target cache disable */
#define SPRN_HID1	0x3F1	/* Hardware Implementation Register 1 */
#define	  HID1_RFXE	(1<<17)		/* Read Fault Exception Enable */
#define	  HID1_ASTME	(1<<13)		/* Address bus streaming mode */
#define	  HID1_ABE	(1<<12)		/* Address broadcast enable */
#define SPRN_IABR	0x3F2	/* Instruction Address Breakpoint Register */
#ifndef CONFIG_BOOKE
#define SPRN_IAC1	0x3F4	/* Instruction Address Compare 1 */
#define SPRN_IAC2	0x3F5	/* Instruction Address Compare 2 */
#else
#define SPRN_IAC1       0x138   /* Book E Instruction Address Compare 1 */
#define SPRN_IAC2       0x139   /* Book E Instruction Address Compare 2 */
#endif /* CONFIG_BOOKE */
#define SPRN_IBAT0L	0x211	/* Instruction BAT 0 Lower Register */
#define SPRN_IBAT0U	0x210	/* Instruction BAT 0 Upper Register */
#define SPRN_IBAT1L	0x213	/* Instruction BAT 1 Lower Register */
#define SPRN_IBAT1U	0x212	/* Instruction BAT 1 Upper Register */
#define SPRN_IBAT2L	0x215	/* Instruction BAT 2 Lower Register */
#define SPRN_IBAT2U	0x214	/* Instruction BAT 2 Upper Register */
#define SPRN_IBAT3L	0x217	/* Instruction BAT 3 Lower Register */
#define SPRN_IBAT3U	0x216	/* Instruction BAT 3 Upper Register */
#define SPRN_IBAT4L	0x231   /* Instruction BAT 4 Lower Register */
#define SPRN_IBAT4U	0x230   /* Instruction BAT 4 Upper Register */
#define SPRN_IBAT5L	0x233   /* Instruction BAT 5 Lower Register */
#define SPRN_IBAT5U	0x232   /* Instruction BAT 5 Upper Register */
#define SPRN_IBAT6L	0x235   /* Instruction BAT 6 Lower Register */
#define SPRN_IBAT6U	0x234   /* Instruction BAT 6 Upper Register */
#define SPRN_IBAT7L	0x237   /* Instruction BAT 7 Lower Register */
#define SPRN_IBAT7U	0x236   /* Instruction BAT 7 Upper Register */
#define SPRN_ICCR	0x3FB	/* Instruction Cache Cacheability Register */
#define   ICCR_NOCACHE		0	/* Noncacheable */
#define   ICCR_CACHE		1	/* Cacheable */
#define SPRN_ICDBDR	0x3D3	/* Instruction Cache Debug Data Register */
#define SPRN_ICMP	0x3D5	/* Instruction TLB Compare Register */
#define SPRN_ICTC	0x3FB	/* Instruction Cache Throttling Control Reg */
#define SPRN_IMISS	0x3D4	/* Instruction TLB Miss Register */
#define SPRN_IMMR	0x27E	/* Internal Memory Map Register */
#define SPRN_LDSTCR	0x3F8   /* Load/Store Control Register */
#define SPRN_L2CR	0x3F9	/* Level 2 Cache Control Regsiter */
#define SPRN_LR		0x008	/* Link Register */
#define SPRN_MBAR       0x137   /* System memory base address */
#define SPRN_MMCR0	0x3B8	/* Monitor Mode Control Register 0 */
#define SPRN_MMCR1	0x3BC	/* Monitor Mode Control Register 1 */
#define SPRN_PBL1	0x3FC	/* Protection Bound Lower 1 */
#define SPRN_PBL2	0x3FE	/* Protection Bound Lower 2 */
#define SPRN_PBU1	0x3FD	/* Protection Bound Upper 1 */
#define SPRN_PBU2	0x3FF	/* Protection Bound Upper 2 */
#ifndef CONFIG_BOOKE
#define SPRN_PID	0x3B1	/* Process ID */
#define SPRN_PIR	0x3FF	/* Processor Identification Register */
#else
#define SPRN_PID        0x030   /* Book E Process ID */
#define SPRN_PIR        0x11E   /* Book E Processor Identification Register */
#endif /* CONFIG_BOOKE */
#define SPRN_PIT	0x3DB	/* Programmable Interval Timer */
#define SPRN_PMC1	0x3B9	/* Performance Counter Register 1 */
#define SPRN_PMC2	0x3BA	/* Performance Counter Register 2 */
#define SPRN_PMC3	0x3BD	/* Performance Counter Register 3 */
#define SPRN_PMC4	0x3BE	/* Performance Counter Register 4 */
#define SPRN_PVR	0x11F	/* Processor Version Register */
#define SPRN_RPA	0x3D6	/* Required Physical Address Register */
#define SPRN_SDA	0x3BF	/* Sampled Data Address Register */
#define SPRN_SDR1	0x019	/* MMU Hash Base Register */
#define SPRN_SGR	0x3B9	/* Storage Guarded Register */
#define   SGR_NORMAL		0
#define   SGR_GUARDED		1
#define SPRN_SIA	0x3BB	/* Sampled Instruction Address Register */
#define SPRN_SPRG0	0x110	/* Special Purpose Register General 0 */
#define SPRN_SPRG1	0x111	/* Special Purpose Register General 1 */
#define SPRN_SPRG2	0x112	/* Special Purpose Register General 2 */
#define SPRN_SPRG3	0x113	/* Special Purpose Register General 3 */
#define SPRN_SPRG4	0x114	/* Special Purpose Register General 4 */
#define SPRN_SPRG5	0x115	/* Special Purpose Register General 5 */
#define SPRN_SPRG6	0x116	/* Special Purpose Register General 6 */
#define SPRN_SPRG7	0x117	/* Special Purpose Register General 7 */
#define SPRN_SRR0	0x01A	/* Save/Restore Register 0 */
#define SPRN_SRR1	0x01B	/* Save/Restore Register 1 */
#define SPRN_SRR2	0x3DE	/* Save/Restore Register 2 */
#define SPRN_SRR3	0x3DF	/* Save/Restore Register 3 */
#ifdef CONFIG_BOOKE
#define SPRN_SVR	0x3FF	/* System Version Register */
#else
#define SPRN_SVR	0x11E	/* System Version Register */
#endif
#define SPRN_TBHI	0x3DC	/* Time Base High */
#define SPRN_TBHU	0x3CC	/* Time Base High User-mode */
#define SPRN_TBLO	0x3DD	/* Time Base Low */
#define SPRN_TBLU	0x3CD	/* Time Base Low User-mode */
#define SPRN_TBRL	0x10C	/* Time Base Read Lower Register */
#define SPRN_TBRU	0x10D	/* Time Base Read Upper Register */
#define SPRN_TBWL	0x11C	/* Time Base Write Lower Register */
#define SPRN_TBWU	0x11D	/* Time Base Write Upper Register */
#ifndef CONFIG_BOOKE
#define SPRN_TCR	0x3DA	/* Timer Control Register */
#else
#define SPRN_TCR        0x154   /* Book E Timer Control Register */
#endif /* CONFIG_BOOKE */
#define   TCR_WP(x)		(((x)&0x3)<<30)	/* WDT Period */
#define     WP_2_17		0		/* 2^17 clocks */
#define     WP_2_21		1		/* 2^21 clocks */
#define     WP_2_25		2		/* 2^25 clocks */
#define     WP_2_29		3		/* 2^29 clocks */
#define   TCR_WRC(x)		(((x)&0x3)<<28)	/* WDT Reset Control */
#define     WRC_NONE		0		/* No reset will occur */
#define     WRC_CORE		1		/* Core reset will occur */
#define     WRC_CHIP		2		/* Chip reset will occur */
#define     WRC_SYSTEM		3		/* System reset will occur */
#define   TCR_WIE		0x08000000	/* WDT Interrupt Enable */
#define   TCR_PIE		0x04000000	/* PIT Interrupt Enable */
#define   TCR_FP(x)		(((x)&0x3)<<24)	/* FIT Period */
#define     FP_2_9		0		/* 2^9 clocks */
#define     FP_2_13		1		/* 2^13 clocks */
#define     FP_2_17		2		/* 2^17 clocks */
#define     FP_2_21		3		/* 2^21 clocks */
#define   TCR_FIE		0x00800000	/* FIT Interrupt Enable */
#define   TCR_ARE		0x00400000	/* Auto Reload Enable */
#define SPRN_THRM1	0x3FC	/* Thermal Management Register 1 */
#define   THRM1_TIN		(1<<0)
#define   THRM1_TIV		(1<<1)
#define   THRM1_THRES		(0x7f<<2)
#define   THRM1_TID		(1<<29)
#define   THRM1_TIE		(1<<30)
#define   THRM1_V		(1<<31)
#define SPRN_THRM2	0x3FD	/* Thermal Management Register 2 */
#define SPRN_THRM3	0x3FE	/* Thermal Management Register 3 */
#define   THRM3_E		(1<<31)
#define SPRN_TLBMISS    0x3D4   /* 980 7450 TLB Miss Register */
#ifndef CONFIG_BOOKE
#define SPRN_TSR	0x3D8	/* Timer Status Register */
#else
#define SPRN_TSR        0x150   /* Book E Timer Status Register */
#endif /* CONFIG_BOOKE */
#define   TSR_ENW		0x80000000	/* Enable Next Watchdog */
#define   TSR_WIS		0x40000000	/* WDT Interrupt Status */
#define   TSR_WRS(x)		(((x)&0x3)<<28)	/* WDT Reset Status */
#define     WRS_NONE		0		/* No WDT reset occurred */
#define     WRS_CORE		1		/* WDT forced core reset */
#define     WRS_CHIP		2		/* WDT forced chip reset */
#define     WRS_SYSTEM		3		/* WDT forced system reset */
#define   TSR_PIS		0x08000000	/* PIT Interrupt Status */
#define   TSR_FIS		0x04000000	/* FIT Interrupt Status */
#define SPRN_UMMCR0	0x3A8	/* User Monitor Mode Control Register 0 */
#define SPRN_UMMCR1	0x3AC	/* User Monitor Mode Control Register 0 */
#define SPRN_UPMC1	0x3A9	/* User Performance Counter Register 1 */
#define SPRN_UPMC2	0x3AA	/* User Performance Counter Register 2 */
#define SPRN_UPMC3	0x3AD	/* User Performance Counter Register 3 */
#define SPRN_UPMC4	0x3AE	/* User Performance Counter Register 4 */
#define SPRN_USIA	0x3AB	/* User Sampled Instruction Address Register */
#define SPRN_XER	0x001	/* Fixed Point Exception Register */
#define SPRN_ZPR	0x3B0	/* Zone Protection Register */

/* Book E definitions */
#define SPRN_DECAR	0x036	/* Decrementer Auto Reload Register */
#define SPRN_CSRR0	0x03A	/* Critical SRR0 */
#define SPRN_CSRR1	0x03B	/* Critical SRR0 */
#define SPRN_IVPR	0x03F	/* Interrupt Vector Prefix Register */
#define SPRN_USPRG0	0x100	/* User Special Purpose Register General 0 */
#define SPRN_SPRG4R	0x104	/* Special Purpose Register General 4 Read */
#define SPRN_SPRG5R	0x105	/* Special Purpose Register General 5 Read */
#define SPRN_SPRG6R	0x106	/* Special Purpose Register General 6 Read */
#define SPRN_SPRG7R	0x107	/* Special Purpose Register General 7 Read */
#define SPRN_SPRG4W	0x114	/* Special Purpose Register General 4 Write */
#define SPRN_SPRG5W	0x115	/* Special Purpose Register General 5 Write */
#define SPRN_SPRG6W	0x116	/* Special Purpose Register General 6 Write */
#define SPRN_SPRG7W	0x117	/* Special Purpose Register General 7 Write */
#define SPRN_DBCR2	0x136	/* Debug Control Register 2 */
#define SPRN_IAC3	0x13A	/* Instruction Address Compare 3 */
#define SPRN_IAC4	0x13B	/* Instruction Address Compare 4 */
#define SPRN_DVC1	0x13E	/* Data Value Compare Register 1 */
#define SPRN_DVC2	0x13F	/* Data Value Compare Register 2 */
#define SPRN_IVOR0	0x190	/* Interrupt Vector Offset Register 0 */
#define SPRN_IVOR1	0x191	/* Interrupt Vector Offset Register 1 */
#define SPRN_IVOR2	0x192	/* Interrupt Vector Offset Register 2 */
#define SPRN_IVOR3	0x193	/* Interrupt Vector Offset Register 3 */
#define SPRN_IVOR4	0x194	/* Interrupt Vector Offset Register 4 */
#define SPRN_IVOR5	0x195	/* Interrupt Vector Offset Register 5 */
#define SPRN_IVOR6	0x196	/* Interrupt Vector Offset Register 6 */
#define SPRN_IVOR7	0x197	/* Interrupt Vector Offset Register 7 */
#define SPRN_IVOR8	0x198	/* Interrupt Vector Offset Register 8 */
#define SPRN_IVOR9	0x199	/* Interrupt Vector Offset Register 9 */
#define SPRN_IVOR10	0x19a	/* Interrupt Vector Offset Register 10 */
#define SPRN_IVOR11	0x19b	/* Interrupt Vector Offset Register 11 */
#define SPRN_IVOR12	0x19c	/* Interrupt Vector Offset Register 12 */
#define SPRN_IVOR13	0x19d	/* Interrupt Vector Offset Register 13 */
#define SPRN_IVOR14	0x19e	/* Interrupt Vector Offset Register 14 */
#define SPRN_IVOR15	0x19f	/* Interrupt Vector Offset Register 15 */

/* e500 definitions */
#define SPRN_L1CSR0     0x3f2   /* L1 Data Cache Control and Status Register 0 */
#define   L1CSR0_CPE            0x00010000	/* Data Cache Parity Enable */
#define   L1CSR0_DCFI           0x00000002      /* Data Cache Flash Invalidate */
#define   L1CSR0_DCE            0x00000001      /* Data Cache Enable */
#define SPRN_L1CSR1     0x3f3   /* L1 Instruction Cache Control and Status Register 1 */
#define   L1CSR1_CPE            0x00010000	/* Instruction Cache Parity Enable */
#define   L1CSR1_ICFI           0x00000002      /* Instruction Cache Flash Invalidate */
#define   L1CSR1_ICE            0x00000001      /* Instruction Cache Enable */

#define SPRN_MMUCSR0	0x3f4	/* MMU control and status register 0 */
#define SPRN_MAS0       0x270   /* MMU Assist Register 0 */
#define SPRN_MAS1       0x271   /* MMU Assist Register 1 */
#define SPRN_MAS2       0x272   /* MMU Assist Register 2 */
#define SPRN_MAS3       0x273   /* MMU Assist Register 3 */
#define SPRN_MAS4       0x274   /* MMU Assist Register 4 */
#define SPRN_MAS5       0x275   /* MMU Assist Register 5 */
#define SPRN_MAS6       0x276   /* MMU Assist Register 6 */
#define SPRN_MAS7	0x3B0	/* MMU Assist Register 7 */

#define SPRN_IVOR32     0x210   /* Interrupt Vector Offset Register 32 */
#define SPRN_IVOR33     0x211   /* Interrupt Vector Offset Register 33 */
#define SPRN_IVOR34     0x212   /* Interrupt Vector Offset Register 34 */
#define SPRN_IVOR35     0x213   /* Interrupt Vector Offset Register 35 */
#define SPRN_SPEFSCR    0x200   /* SPE & Embedded FP Status & Control */

#define SPRN_MCSRR0     0x23a   /* Machine Check Save and Restore Register 0 */
#define SPRN_MCSRR1     0x23b   /* Machine Check Save and Restore Register 1 */
#define SPRN_BUCSR	0x3f5	/* Branch Control and Status Register */
#define SPRN_BBEAR      0x201   /* Branch Buffer Entry Address Register */
#define SPRN_BBTAR      0x202   /* Branch Buffer Target Address Register */
#define SPRN_PID1       0x279   /* Process ID Register 1 */
#define SPRN_PID2       0x27a   /* Process ID Register 2 */
#define SPRN_MCSR	0x23c	/* Machine Check Syndrome register */
#ifdef CONFIG_440
#define MCSR_MCS	0x80000000	/* Machine Check Summary */
#define MCSR_IB		0x40000000	/* Instruction PLB Error */
#define MCSR_DRB	0x20000000	/* Data Read PLB Error */
#define MCSR_DWB	0x10000000	/* Data Write PLB Error */
#define MCSR_TLBP	0x08000000	/* TLB Parity Error */
#define MCSR_ICP	0x04000000	/* I-Cache Parity Error */
#define MCSR_DCSP	0x02000000	/* D-Cache Search Parity Error */
#define MCSR_DCFP	0x01000000	/* D-Cache Flush Parity Error */
#define MCSR_IMPE	0x00800000	/* Imprecise Machine Check Exception */
#endif
#define ESR_ST          0x00800000      /* Store Operation */

#if defined(CONFIG_MPC86xx)
#define SPRN_MSSCRO	0x3f6
#endif


/* Short-hand versions for a number of the above SPRNs */

#define CTR	SPRN_CTR	/* Counter Register */
#define DAR	SPRN_DAR	/* Data Address Register */
#define DABR	SPRN_DABR	/* Data Address Breakpoint Register */
#define DAC1	SPRN_DAC1	/* Data Address Register 1 */
#define DAC2	SPRN_DAC2	/* Data Address Register 2 */
#define DBAT0L	SPRN_DBAT0L	/* Data BAT 0 Lower Register */
#define DBAT0U	SPRN_DBAT0U	/* Data BAT 0 Upper Register */
#define DBAT1L	SPRN_DBAT1L	/* Data BAT 1 Lower Register */
#define DBAT1U	SPRN_DBAT1U	/* Data BAT 1 Upper Register */
#define DBAT2L	SPRN_DBAT2L	/* Data BAT 2 Lower Register */
#define DBAT2U	SPRN_DBAT2U	/* Data BAT 2 Upper Register */
#define DBAT3L	SPRN_DBAT3L	/* Data BAT 3 Lower Register */
#define DBAT3U	SPRN_DBAT3U	/* Data BAT 3 Upper Register */
#define DBAT4L	SPRN_DBAT4L     /* Data BAT 4 Lower Register */
#define DBAT4U	SPRN_DBAT4U     /* Data BAT 4 Upper Register */
#define DBAT5L	SPRN_DBAT5L     /* Data BAT 5 Lower Register */
#define DBAT5U	SPRN_DBAT5U     /* Data BAT 5 Upper Register */
#define DBAT6L	SPRN_DBAT6L     /* Data BAT 6 Lower Register */
#define DBAT6U	SPRN_DBAT6U     /* Data BAT 6 Upper Register */
#define DBAT7L	SPRN_DBAT7L     /* Data BAT 7 Lower Register */
#define DBAT7U	SPRN_DBAT7U     /* Data BAT 7 Upper Register */
#define DBCR0	SPRN_DBCR0	/* Debug Control Register 0 */
#define DBCR1	SPRN_DBCR1	/* Debug Control Register 1 */
#define DBSR	SPRN_DBSR	/* Debug Status Register */
#define DCMP	SPRN_DCMP	/* Data TLB Compare Register */
#define DEC	SPRN_DEC	/* Decrement Register */
#define DMISS	SPRN_DMISS	/* Data TLB Miss Register */
#define DSISR	SPRN_DSISR	/* Data Storage Interrupt Status Register */
#define EAR	SPRN_EAR	/* External Address Register */
#define ESR	SPRN_ESR	/* Exception Syndrome Register */
#define HASH1	SPRN_HASH1	/* Primary Hash Address Register */
#define HASH2	SPRN_HASH2	/* Secondary Hash Address Register */
#define HID0	SPRN_HID0	/* Hardware Implementation Register 0 */
#define HID1	SPRN_HID1	/* Hardware Implementation Register 1 */
#define IABR	SPRN_IABR	/* Instruction Address Breakpoint Register */
#define IAC1	SPRN_IAC1	/* Instruction Address Register 1 */
#define IAC2	SPRN_IAC2	/* Instruction Address Register 2 */
#define IBAT0L	SPRN_IBAT0L	/* Instruction BAT 0 Lower Register */
#define IBAT0U	SPRN_IBAT0U	/* Instruction BAT 0 Upper Register */
#define IBAT1L	SPRN_IBAT1L	/* Instruction BAT 1 Lower Register */
#define IBAT1U	SPRN_IBAT1U	/* Instruction BAT 1 Upper Register */
#define IBAT2L	SPRN_IBAT2L	/* Instruction BAT 2 Lower Register */
#define IBAT2U	SPRN_IBAT2U	/* Instruction BAT 2 Upper Register */
#define IBAT3L	SPRN_IBAT3L	/* Instruction BAT 3 Lower Register */
#define IBAT3U	SPRN_IBAT3U	/* Instruction BAT 3 Upper Register */
#define IBAT4L	SPRN_IBAT4L	/* Instruction BAT 4 Lower Register */
#define IBAT4U	SPRN_IBAT4U	/* Instruction BAT 4 Upper Register */
#define IBAT5L	SPRN_IBAT5L	/* Instruction BAT 5 Lower Register */
#define IBAT5U	SPRN_IBAT5U	/* Instruction BAT 5 Upper Register */
#define IBAT6L	SPRN_IBAT6L	/* Instruction BAT 6 Lower Register */
#define IBAT6U	SPRN_IBAT6U	/* Instruction BAT 6 Upper Register */
#define IBAT7L	SPRN_IBAT7L	/* Instruction BAT 7 Lower Register */
#define IBAT7U	SPRN_IBAT7U	/* Instruction BAT 7 Lower Register */
#define ICMP	SPRN_ICMP	/* Instruction TLB Compare Register */
#define IMISS	SPRN_IMISS	/* Instruction TLB Miss Register */
#define IMMR	SPRN_IMMR	/* PPC 860/821 Internal Memory Map Register */
#define LDSTCR	SPRN_LDSTCR     /* Load/Store Control Register */
#define L2CR	SPRN_L2CR	/* PPC 750 L2 control register */
#define LR	SPRN_LR
#define MBAR    SPRN_MBAR       /* System memory base address */
#if defined(CONFIG_MPC86xx)
#define MSSCR0	SPRN_MSSCRO
#endif
#if defined(CONFIG_E500) || defined(CONFIG_MPC86xx)
#define PIR	SPRN_PIR
#endif
#define SVR	SPRN_SVR	/* System-On-Chip Version Register */
#define PVR	SPRN_PVR	/* Processor Version */
#define RPA	SPRN_RPA	/* Required Physical Address Register */
#define SDR1	SPRN_SDR1	/* MMU hash base register */
#define SPR0	SPRN_SPRG0	/* Supervisor Private Registers */
#define SPR1	SPRN_SPRG1
#define SPR2	SPRN_SPRG2
#define SPR3	SPRN_SPRG3
#define SPRG0   SPRN_SPRG0
#define SPRG1   SPRN_SPRG1
#define SPRG2   SPRN_SPRG2
#define SPRG3   SPRN_SPRG3
#define SPRG4   SPRN_SPRG4
#define SPRG5   SPRN_SPRG5
#define SPRG6   SPRN_SPRG6
#define SPRG7   SPRN_SPRG7
#define SRR0	SPRN_SRR0	/* Save and Restore Register 0 */
#define SRR1	SPRN_SRR1	/* Save and Restore Register 1 */
#define SRR2	SPRN_SRR2	/* Save and Restore Register 2 */
#define SRR3	SPRN_SRR3	/* Save and Restore Register 3 */
#define SVR	SPRN_SVR	/* System Version Register */
#define TBRL	SPRN_TBRL	/* Time Base Read Lower Register */
#define TBRU	SPRN_TBRU	/* Time Base Read Upper Register */
#define TBWL	SPRN_TBWL	/* Time Base Write Lower Register */
#define TBWU	SPRN_TBWU	/* Time Base Write Upper Register */
#define TCR	SPRN_TCR	/* Timer Control Register */
#define TSR	SPRN_TSR	/* Timer Status Register */
#define ICTC	1019
#define THRM1	SPRN_THRM1	/* Thermal Management Register 1 */
#define THRM2	SPRN_THRM2	/* Thermal Management Register 2 */
#define THRM3	SPRN_THRM3	/* Thermal Management Register 3 */
#define XER	SPRN_XER

#define DECAR	SPRN_DECAR
#define CSRR0	SPRN_CSRR0
#define CSRR1	SPRN_CSRR1
#define IVPR	SPRN_IVPR
#define USPRG0	SPRN_USPRG
#define SPRG4R	SPRN_SPRG4R
#define SPRG5R	SPRN_SPRG5R
#define SPRG6R	SPRN_SPRG6R
#define SPRG7R	SPRN_SPRG7R
#define SPRG4W	SPRN_SPRG4W
#define SPRG5W	SPRN_SPRG5W
#define SPRG6W	SPRN_SPRG6W
#define SPRG7W	SPRN_SPRG7W
#define DEAR	SPRN_DEAR
#define DBCR2	SPRN_DBCR2
#define IAC3	SPRN_IAC3
#define IAC4	SPRN_IAC4
#define DVC1	SPRN_DVC1
#define DVC2	SPRN_DVC2
#define IVOR0	SPRN_IVOR0
#define IVOR1	SPRN_IVOR1
#define IVOR2	SPRN_IVOR2
#define IVOR3	SPRN_IVOR3
#define IVOR4	SPRN_IVOR4
#define IVOR5	SPRN_IVOR5
#define IVOR6	SPRN_IVOR6
#define IVOR7	SPRN_IVOR7
#define IVOR8	SPRN_IVOR8
#define IVOR9	SPRN_IVOR9
#define IVOR10	SPRN_IVOR10
#define IVOR11	SPRN_IVOR11
#define IVOR12	SPRN_IVOR12
#define IVOR13	SPRN_IVOR13
#define IVOR14	SPRN_IVOR14
#define IVOR15	SPRN_IVOR15
#define IVOR32	SPRN_IVOR32
#define IVOR33	SPRN_IVOR33
#define IVOR34	SPRN_IVOR34
#define IVOR35	SPRN_IVOR35
#define MCSRR0	SPRN_MCSRR0
#define MCSRR1	SPRN_MCSRR1
#define L1CSR0	SPRN_L1CSR0
#define L1CSR1	SPRN_L1CSR1
#define MCSR	SPRN_MCSR
#define MMUCSR0	SPRN_MMUCSR0
#define BUCSR	SPRN_BUCSR
#define PID0	SPRN_PID
#define PID1	SPRN_PID1
#define PID2	SPRN_PID2
#define MAS0	SPRN_MAS0
#define MAS1	SPRN_MAS1
#define MAS2	SPRN_MAS2
#define MAS3	SPRN_MAS3
#define MAS4	SPRN_MAS4
#define MAS5	SPRN_MAS5
#define MAS6	SPRN_MAS6
#define MAS7	SPRN_MAS7

#if defined(CONFIG_4xx) || defined(CONFIG_44x) || defined(CONFIG_MPC85xx)
#define DAR_DEAR DEAR
#else
#define DAR_DEAR DAR
#endif

/* Device Control Registers */

#define DCRN_BEAR	0x090	/* Bus Error Address Register */
#define DCRN_BESR	0x091	/* Bus Error Syndrome Register */
#define   BESR_DSES	0x80000000	/* Data-Side Error Status */
#define   BESR_DMES	0x40000000	/* DMA Error Status */
#define   BESR_RWS	0x20000000	/* Read/Write Status */
#define   BESR_ETMASK	0x1C000000	/* Error Type */
#define     ET_PROT	0
#define     ET_PARITY	1
#define     ET_NCFG	2
#define     ET_BUSERR	4
#define     ET_BUSTO	6
#define DCRN_DMACC0	0x0C4	/* DMA Chained Count Register 0 */
#define DCRN_DMACC1	0x0CC	/* DMA Chained Count Register 1 */
#define DCRN_DMACC2	0x0D4	/* DMA Chained Count Register 2 */
#define DCRN_DMACC3	0x0DC    /* DMA Chained Count Register 3 */
#define DCRN_DMACR0	0x0C0    /* DMA Channel Control Register 0 */
#define DCRN_DMACR1	0x0C8    /* DMA Channel Control Register 1 */
#define DCRN_DMACR2	0x0D0    /* DMA Channel Control Register 2 */
#define DCRN_DMACR3	0x0D8    /* DMA Channel Control Register 3 */
#define DCRN_DMACT0	0x0C1    /* DMA Count Register 0 */
#define DCRN_DMACT1	0x0C9    /* DMA Count Register 1 */
#define DCRN_DMACT2	0x0D1    /* DMA Count Register 2 */
#define DCRN_DMACT3	0x0D9    /* DMA Count Register 3 */
#define DCRN_DMADA0	0x0C2    /* DMA Destination Address Register 0 */
#define DCRN_DMADA1	0x0CA    /* DMA Destination Address Register 1 */
#define DCRN_DMADA2	0x0D2    /* DMA Destination Address Register 2 */
#define DCRN_DMADA3	0x0DA    /* DMA Destination Address Register 3 */
#define DCRN_DMASA0	0x0C3    /* DMA Source Address Register 0 */
#define DCRN_DMASA1	0x0CB    /* DMA Source Address Register 1 */
#define DCRN_DMASA2	0x0D3    /* DMA Source Address Register 2 */
#define DCRN_DMASA3	0x0DB    /* DMA Source Address Register 3 */
#define DCRN_DMASR	0x0E0    /* DMA Status Register */
#define DCRN_EXIER	0x042    /* External Interrupt Enable Register */
#define   EXIER_CIE	0x80000000	/* Critical Interrupt Enable */
#define   EXIER_SRIE	0x08000000	/* Serial Port Rx Int. Enable */
#define   EXIER_STIE	0x04000000	/* Serial Port Tx Int. Enable */
#define   EXIER_JRIE	0x02000000	/* JTAG Serial Port Rx Int. Enable */
#define   EXIER_JTIE	0x01000000	/* JTAG Serial Port Tx Int. Enable */
#define   EXIER_D0IE	0x00800000	/* DMA Channel 0 Interrupt Enable */
#define   EXIER_D1IE	0x00400000	/* DMA Channel 1 Interrupt Enable */
#define   EXIER_D2IE	0x00200000	/* DMA Channel 2 Interrupt Enable */
#define   EXIER_D3IE	0x00100000	/* DMA Channel 3 Interrupt Enable */
#define   EXIER_E0IE	0x00000010	/* External Interrupt 0 Enable */
#define   EXIER_E1IE	0x00000008	/* External Interrupt 1 Enable */
#define   EXIER_E2IE	0x00000004	/* External Interrupt 2 Enable */
#define   EXIER_E3IE	0x00000002	/* External Interrupt 3 Enable */
#define   EXIER_E4IE	0x00000001	/* External Interrupt 4 Enable */
#define DCRN_EXISR	0x040    /* External Interrupt Status Register */
#define DCRN_IOCR	0x0A0    /* Input/Output Configuration Register */
#define   IOCR_E0TE	0x80000000
#define   IOCR_E0LP	0x40000000
#define   IOCR_E1TE	0x20000000
#define   IOCR_E1LP	0x10000000
#define   IOCR_E2TE	0x08000000
#define   IOCR_E2LP	0x04000000
#define   IOCR_E3TE	0x02000000
#define   IOCR_E3LP	0x01000000
#define   IOCR_E4TE	0x00800000
#define   IOCR_E4LP	0x00400000
#define   IOCR_EDT	0x00080000
#define   IOCR_SOR	0x00040000
#define   IOCR_EDO	0x00008000
#define   IOCR_2XC	0x00004000
#define   IOCR_ATC	0x00002000
#define   IOCR_SPD	0x00001000
#define   IOCR_BEM	0x00000800
#define   IOCR_PTD	0x00000400
#define   IOCR_ARE	0x00000080
#define   IOCR_DRC	0x00000020
#define   IOCR_RDM(x)	(((x) & 0x3) << 3)
#define   IOCR_TCS	0x00000004
#define   IOCR_SCS	0x00000002
#define   IOCR_SPC	0x00000001

/* System-On-Chip Version Register */

/* System-On-Chip Version Register (SVR) field extraction */

#define SVR_VER(svr)	(((svr) >> 16) & 0xFFFF) /* Version field */
#define SVR_REV(svr)	(((svr) >>  0) & 0xFFFF) /* Revision field */

#define SVR_CID(svr)	(((svr) >> 28) & 0x0F)   /* Company or manufacturer ID */
#define SVR_SOCOP(svr)	(((svr) >> 22) & 0x3F)   /* SOC integration options */
#define SVR_SID(svr)	(((svr) >> 16) & 0x3F)   /* SOC ID */
#define SVR_PROC(svr)	(((svr) >> 12) & 0x0F)   /* Process revision field */
#define SVR_MFG(svr)	(((svr) >>  8) & 0x0F)   /* Manufacturing revision */
#define SVR_MJREV(svr)	(((svr) >>  4) & 0x0F)   /* Major SOC design revision indicator */
#define SVR_MNREV(svr)	(((svr) >>  0) & 0x0F)   /* Minor SOC design revision indicator */


/* Processor Version Register */

/* Processor Version Register (PVR) field extraction */

#define PVR_VER(pvr)  (((pvr) >>  16) & 0xFFFF)	/* Version field */
#define PVR_REV(pvr)  (((pvr) >>   0) & 0xFFFF)	/* Revison field */

/*
 * AMCC has further subdivided the standard PowerPC 16-bit version and
 * revision subfields of the PVR for the PowerPC 403s into the following:
 */

#define PVR_FAM(pvr)	(((pvr) >> 20) & 0xFFF)	/* Family field */
#define PVR_MEM(pvr)	(((pvr) >> 16) & 0xF)	/* Member field */
#define PVR_CORE(pvr)	(((pvr) >> 12) & 0xF)	/* Core field */
#define PVR_CFG(pvr)	(((pvr) >>  8) & 0xF)	/* Configuration field */
#define PVR_MAJ(pvr)	(((pvr) >>  4) & 0xF)	/* Major revision field */
#define PVR_MIN(pvr)	(((pvr) >>  0) & 0xF)	/* Minor revision field */

/* Processor Version Numbers */

#define PVR_403GA	0x00200000
#define PVR_403GB	0x00200100
#define PVR_403GC	0x00200200
#define PVR_403GCX	0x00201400
#define PVR_405GP	0x40110000
#define PVR_405GP_RB	0x40110040
#define PVR_405GP_RC	0x40110082
#define PVR_405GP_RD	0x401100C4
#define PVR_405GP_RE	0x40110145  /* same as pc405cr rev c */
#define PVR_405CR_RA	0x40110041
#define PVR_405CR_RB	0x401100C5
#define PVR_405CR_RC	0x40110145  /* same as pc405gp rev e */
#define PVR_405EP_RA	0x51210950
#define PVR_405GPR_RB	0x50910951
#define PVR_405EZ_RA	0x41511460
#define PVR_440GP_RB	0x40120440
#define PVR_440GP_RC	0x40120481
#define PVR_440EP_RA	0x42221850
#define PVR_440EP_RB	0x422218D3 /* 440EP rev B and 440GR rev A have same PVR */
#define PVR_440EP_RC	0x422218D4 /* 440EP rev C and 440GR rev B have same PVR */
#define PVR_440GR_RA	0x422218D3 /* 440EP rev B and 440GR rev A have same PVR */
#define PVR_440GR_RB	0x422218D4 /* 440EP rev C and 440GR rev B have same PVR */
#define PVR_440EPX1_RA  0x216218D0 /* 440EPX rev A with Security / Kasumi */
#define PVR_440EPX2_RA  0x216218D4 /* 440EPX rev A without Security / Kasumi */
#define PVR_440GRX1_RA  0x216218D0 /* 440GRX rev A with Security / Kasumi */
#define PVR_440GRX2_RA  0x216218D4 /* 440GRX rev A without Security / Kasumi */
#define PVR_440GX_RA	0x51B21850
#define PVR_440GX_RB	0x51B21851
#define PVR_440GX_RC	0x51B21892
#define PVR_440GX_RF	0x51B21894
#define PVR_405EP_RB	0x51210950
#define PVR_440SP_6_RAB	0x53221850 /* 440SP rev A&B with RAID 6 support enabled	*/
#define PVR_440SP_RAB	0x53321850 /* 440SP rev A&B without RAID 6 support	*/
#define PVR_440SP_6_RC	0x53221891 /* 440SP rev C with RAID 6 support enabled	*/
#define PVR_440SP_RC	0x53321891 /* 440SP rev C without RAID 6 support	*/
#define PVR_440SPe_6_RA	0x53421890 /* 440SPe rev A with RAID 6 support enabled	*/
#define PVR_440SPe_RA	0x53521890 /* 440SPe rev A without RAID 6 support	*/
#define PVR_440SPe_6_RB	0x53421891 /* 440SPe rev B with RAID 6 support enabled	*/
#define PVR_440SPe_RB	0x53521891 /* 440SPe rev B without RAID 6 support	*/
#define PVR_601		0x00010000
#define PVR_602		0x00050000
#define PVR_603		0x00030000
#define PVR_603e	0x00060000
#define PVR_603ev	0x00070000
#define PVR_603r	0x00071000
#define PVR_604		0x00040000
#define PVR_604e	0x00090000
#define PVR_604r	0x000A0000
#define PVR_620		0x00140000
#define PVR_740		0x00080000
#define PVR_750		PVR_740
#define PVR_740P	0x10080000
#define PVR_750P	PVR_740P
#define PVR_7400        0x000C0000
#define PVR_7410        0x800C0000
#define PVR_7450        0x80000000

#define PVR_85xx	0x80200000
#define PVR_85xx_REV1	(PVR_85xx | 0x0010)
#define PVR_85xx_REV2	(PVR_85xx | 0x0020)

#define PVR_86xx	0x80040000
#define PVR_86xx_REV1	(PVR_86xx | 0x0010)

/*
 * For the 8xx processors, all of them report the same PVR family for
 * the PowerPC core. The various versions of these processors must be
 * differentiated by the version number in the Communication Processor
 * Module (CPM).
 */
#define PVR_821		0x00500000
#define PVR_823		PVR_821
#define PVR_850		PVR_821
#define PVR_860		PVR_821
#define PVR_7400	0x000C0000
#define PVR_8240	0x00810100

/*
 * PowerQUICC II family processors report different PVR values depending
 * on silicon process (HiP3, HiP4, HiP7, etc.)
 */
#define PVR_8260        PVR_8240
#define PVR_8260_HIP3   0x00810101
#define PVR_8260_HIP4   0x80811014
#define PVR_8260_HIP7   0x80822011
#define PVR_8260_HIP7R1 0x80822013
#define PVR_8260_HIP7RA	0x80822014

/*
 * MPC 52xx
 */
#define PVR_5200	0x80822011
#define PVR_5200B	0x80822014


/*
 * System Version Register
 */

/* System Version Register (SVR) field extraction */

#define SVR_VER(svr)	(((svr) >>  16) & 0xFFFF)	/* Version field */
#define SVR_REV(svr)	(((svr) >>   0) & 0xFFFF)	/* Revison field */

#define SVR_SUBVER(svr)	(((svr) >>  8) & 0xFF)	/* Process/MFG sub-version */

#define SVR_FAM(svr)	(((svr) >> 20) & 0xFFF)	/* Family field */
#define SVR_MEM(svr)	(((svr) >> 16) & 0xF)	/* Member field */

#define SVR_MAJ(svr)	(((svr) >>  4) & 0xF)	/* Major revision field*/
#define SVR_MIN(svr)	(((svr) >>  0) & 0xF)	/* Minor revision field*/


/*
 * SVR_VER() Version Values
 */

#define SVR_8540	0x8030
#define SVR_8560	0x8070
#define SVR_8555	0x8079
#define SVR_8541	0x807A
#define SVR_8544	0x8034
#define SVR_8544_E	0x803C
#define SVR_8548	0x8031
#define SVR_8548_E	0x8039
#define SVR_8641	0x8090
#define SVR_8568_E	0x807D


/* I am just adding a single entry for 8260 boards.  I think we may be
 * able to combine mbx, fads, rpxlite, bseip, and classic into a single
 * generic 8xx as well.  The boards containing these processors are either
 * identical at the processor level (due to the high integration) or so
 * wildly different that testing _machine at run time is best replaced by
 * conditional compilation by board type (found in their respective .h file).
 *	-- Dan
 */
#define _MACH_prep	0x00000001
#define _MACH_Pmac	0x00000002	/* pmac or pmac clone (non-chrp) */
#define _MACH_chrp	0x00000004	/* chrp machine */
#define _MACH_mbx	0x00000008	/* Motorola MBX board */
#define _MACH_apus	0x00000010	/* amiga with phase5 powerup */
#define _MACH_fads	0x00000020	/* Motorola FADS board */
#define _MACH_rpxlite	0x00000040	/* RPCG RPX-Lite 8xx board */
#define _MACH_bseip	0x00000080	/* Bright Star Engineering ip-Engine */
#define _MACH_yk	0x00000100	/* Motorola Yellowknife */
#define _MACH_gemini	0x00000200	/* Synergy Microsystems gemini board */
#define _MACH_classic	0x00000400	/* RPCG RPX-Classic 8xx board */
#define _MACH_oak	0x00000800	/* IBM "Oak" 403 eval. board */
#define _MACH_walnut	0x00001000	/* AMCC "Walnut" 405GP eval. board */
#define _MACH_8260	0x00002000	/* Generic 8260 */
#define _MACH_sandpoint 0x00004000	/* Motorola SPS Processor eval board */
#define _MACH_tqm860	0x00008000	/* TQM860/L */
#define _MACH_tqm8xxL	0x00010000	/* TQM8xxL */
#define _MACH_hidden_dragon 0x00020000	/* Motorola Hidden Dragon eval board */


/* see residual.h for these */
#define _PREP_Motorola 0x01  /* motorola prep */
#define _PREP_Firm     0x02  /* firmworks prep */
#define _PREP_IBM      0x00  /* ibm prep */
#define _PREP_Bull     0x03  /* bull prep */
#define _PREP_Radstone 0x04  /* Radstone Technology PLC prep */

/*
 * Radstone board types
 */
#define RS_SYS_TYPE_PPC1   0
#define RS_SYS_TYPE_PPC2   1
#define RS_SYS_TYPE_PPC1a  2
#define RS_SYS_TYPE_PPC2a  3
#define RS_SYS_TYPE_PPC4   4
#define RS_SYS_TYPE_PPC4a  5
#define RS_SYS_TYPE_PPC2ep 6

/* these are arbitrary */
#define _CHRP_Motorola 0x04  /* motorola chrp, the cobra */
#define _CHRP_IBM      0x05  /* IBM chrp, the longtrail and longtrail 2 */

#define _GLOBAL(n)\
	.globl n;\
n:

/* Macros for setting and retrieving special purpose registers */

#define stringify(s)	tostring(s)
#define tostring(s)	#s

#define mfdcr(rn)	({unsigned int rval; \
			asm volatile("mfdcr %0," stringify(rn) \
				     : "=r" (rval)); rval;})
#define mtdcr(rn, v)	asm volatile("mtdcr " stringify(rn) ",%0" : : "r" (v))

#define mfmsr()		({unsigned int rval; \
			asm volatile("mfmsr %0" : "=r" (rval)); rval;})
#define mtmsr(v)	asm volatile("mtmsr %0" : : "r" (v))

#define mfspr(rn)	({unsigned int rval; \
			asm volatile("mfspr %0," stringify(rn) \
				     : "=r" (rval)); rval;})
#define mtspr(rn, v)	asm volatile("mtspr " stringify(rn) ",%0" : : "r" (v))

#define tlbie(v)	asm volatile("tlbie %0 \n sync" : : "r" (v))

/* Segment Registers */

#define SR0	0
#define SR1	1
#define SR2	2
#define SR3	3
#define SR4	4
#define SR5	5
#define SR6	6
#define SR7	7
#define SR8	8
#define SR9	9
#define SR10	10
#define SR11	11
#define SR12	12
#define SR13	13
#define SR14	14
#define SR15	15

#ifndef __ASSEMBLY__
#ifndef CONFIG_MACH_SPECIFIC
extern int _machine;
extern int have_of;
#endif /* CONFIG_MACH_SPECIFIC */

/* what kind of prep workstation we are */
extern int _prep_type;
/*
 * This is used to identify the board type from a given PReP board
 * vendor. Board revision is also made available.
 */
extern unsigned char ucSystemType;
extern unsigned char ucBoardRev;
extern unsigned char ucBoardRevMaj, ucBoardRevMin;

struct task_struct;
void start_thread(struct pt_regs *regs, unsigned long nip, unsigned long sp);
void release_thread(struct task_struct *);

/*
 * Create a new kernel thread.
 */
extern long kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);

/*
 * Bus types
 */
#define EISA_bus 0
#define EISA_bus__is_a_macro /* for versions in ksyms.c */
#define MCA_bus 0
#define MCA_bus__is_a_macro /* for versions in ksyms.c */

/* Lazy FPU handling on uni-processor */
extern struct task_struct *last_task_used_math;
extern struct task_struct *last_task_used_altivec;

/*
 * this is the minimum allowable io space due to the location
 * of the io areas on prep (first one at 0x80000000) but
 * as soon as I get around to remapping the io areas with the BATs
 * to match the mac we can raise this. -- Cort
 */
#define TASK_SIZE	(0x80000000UL)

/* This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
#define TASK_UNMAPPED_BASE	(TASK_SIZE / 8 * 3)

typedef struct {
	unsigned long seg;
} mm_segment_t;

struct thread_struct {
	unsigned long	ksp;		/* Kernel stack pointer */
	unsigned long	wchan;		/* Event task is sleeping on */
	struct pt_regs	*regs;		/* Pointer to saved register state */
	mm_segment_t	fs;		/* for get_fs() validation */
	void		*pgdir;		/* root of page-table tree */
	signed long     last_syscall;
	double		fpr[32];	/* Complete floating point set */
	unsigned long	fpscr_pad;	/* fpr ... fpscr must be contiguous */
	unsigned long	fpscr;		/* Floating point status */
#ifdef CONFIG_ALTIVEC
	vector128	vr[32];		/* Complete AltiVec set */
	vector128	vscr;		/* AltiVec status */
	unsigned long	vrsave;
#endif /* CONFIG_ALTIVEC */
};

#define INIT_SP		(sizeof(init_stack) + (unsigned long) &init_stack)

#define INIT_THREAD  { \
	INIT_SP, /* ksp */ \
	0, /* wchan */ \
	(struct pt_regs *)INIT_SP - 1, /* regs */ \
	KERNEL_DS, /*fs*/ \
	swapper_pg_dir, /* pgdir */ \
	0, /* last_syscall */ \
	{0}, 0, 0 \
}

/*
 * Note: the vm_start and vm_end fields here should *not*
 * be in kernel space.  (Could vm_end == vm_start perhaps?)
 */
#define INIT_MMAP { &init_mm, 0, 0x1000, NULL, \
		    PAGE_SHARED, VM_READ | VM_WRITE | VM_EXEC, \
		    1, NULL, NULL }

/*
 * Return saved PC of a blocked thread. For now, this is the "user" PC
 */
static inline unsigned long thread_saved_pc(struct thread_struct *t)
{
	return (t->regs) ? t->regs->nip : 0;
}

#define copy_segments(tsk, mm)		do { } while (0)
#define release_segments(mm)		do { } while (0)
#define forget_segments()		do { } while (0)

unsigned long get_wchan(struct task_struct *p);

#define KSTK_EIP(tsk)  ((tsk)->thread.regs->nip)
#define KSTK_ESP(tsk)  ((tsk)->thread.regs->gpr[1])

/*
 * NOTE! The task struct and the stack go together
 */
#define THREAD_SIZE (2*PAGE_SIZE)
#define alloc_task_struct() \
	((struct task_struct *) __get_free_pages(GFP_KERNEL,1))
#define free_task_struct(p)	free_pages((unsigned long)(p),1)
#define get_task_struct(tsk)      atomic_inc(&mem_map[MAP_NR(tsk)].count)

/* in process.c - for early bootup debug -- Cort */
int ll_printk(const char *, ...);
void ll_puts(const char *);

#define init_task	(init_task_union.task)
#define init_stack	(init_task_union.stack)

/* In misc.c */
void _nmask_and_or_msr(unsigned long nmask, unsigned long or_val);

#endif /* ndef ASSEMBLY*/

#ifdef CONFIG_MACH_SPECIFIC
#if defined(CONFIG_8xx)
#define _machine _MACH_8xx
#define have_of 0
#elif defined(CONFIG_OAK)
#define _machine _MACH_oak
#define have_of	0
#elif defined(CONFIG_WALNUT)
#define _machine _MACH_walnut
#define have_of 0
#elif defined(CONFIG_APUS)
#define _machine _MACH_apus
#define have_of 0
#elif defined(CONFIG_GEMINI)
#define _machine _MACH_gemini
#define have_of 0
#elif defined(CONFIG_8260)
#define _machine _MACH_8260
#define have_of 0
#elif defined(CONFIG_SANDPOINT)
#define _machine _MACH_sandpoint
#elif defined(CONFIG_HIDDEN_DRAGON)
#define _machine _MACH_hidden_dragon
#define have_of 0
#else
#error "Machine not defined correctly"
#endif
#endif /* CONFIG_MACH_SPECIFIC */

#endif /* __ASM_PPC_PROCESSOR_H */
