/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007 (C)
 * Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 *
 * Copyright 2008 (C)
 * Mark Jonas <mark.jonas@de.bosch.com>
 *
 * SH7720 Internal I/O register
 */

#ifndef _ASM_CPU_SH7720_H_
#define _ASM_CPU_SH7720_H_

#define CACHE_OC_NUM_WAYS	4
#define CCR_CACHE_INIT		0x0000000B

/*	EXP	*/
#define TRA		0xFFFFFFD0
#define EXPEVT		0xFFFFFFD4
#define INTEVT		0xFFFFFFD8

/*	MMU	*/
#define MMUCR		0xFFFFFFE0
#define PTEH		0xFFFFFFF0
#define PTEL		0xFFFFFFF4
#define TTB		0xFFFFFFF8

/*	CACHE	*/
#define CCR		0xFFFFFFEC

/*	INTC	*/
#define IPRF		0xA4080000
#define IPRG		0xA4080002
#define IPRH		0xA4080004
#define IPRI		0xA4080006
#define IPRJ		0xA4080008
#define IRR5		0xA4080020
#define IRR6		0xA4080022
#define IRR7		0xA4080024
#define IRR8		0xA4080026
#define IRR9		0xA4080028
#define IRR0		0xA4140004
#define IRR1		0xA4140006
#define IRR2		0xA4140008
#define IRR3		0xA414000A
#define IRR4		0xA414000C
#define ICR1		0xA4140010
#define ICR2		0xA4140012
#define PINTER		0xA4140014
#define IPRC		0xA4140016
#define IPRD		0xA4140018
#define IPRE		0xA414001A
#define ICR0		0xA414FEE0
#define IPRA		0xA414FEE2
#define IPRB		0xA414FEE4

/*	BSC	*/
#define BSC_BASE	0xA4FD0000
#define CMNCR		(BSC_BASE + 0x00)
#define CS0BCR		(BSC_BASE + 0x04)
#define CS2BCR		(BSC_BASE + 0x08)
#define CS3BCR		(BSC_BASE + 0x0C)
#define CS4BCR		(BSC_BASE + 0x10)
#define CS5ABCR		(BSC_BASE + 0x14)
#define CS5BBCR		(BSC_BASE + 0x18)
#define CS6ABCR		(BSC_BASE + 0x1C)
#define CS6BBCR		(BSC_BASE + 0x20)
#define CS0WCR		(BSC_BASE + 0x24)
#define CS2WCR		(BSC_BASE + 0x28)
#define CS3WCR		(BSC_BASE + 0x2C)
#define CS4WCR		(BSC_BASE + 0x30)
#define CS5AWCR		(BSC_BASE + 0x34)
#define CS5BWCR		(BSC_BASE + 0x38)
#define CS6AWCR		(BSC_BASE + 0x3C)
#define CS6BWCR		(BSC_BASE + 0x40)
#define SDCR		(BSC_BASE + 0x44)
#define RTCSR		(BSC_BASE + 0x48)
#define RTCNR		(BSC_BASE + 0x4C)
#define RTCOR		(BSC_BASE + 0x50)
#define SDMR2		(BSC_BASE + 0x4000)
#define SDMR3		(BSC_BASE + 0x5000)

/*	DMAC	*/

/*	CPG	*/
#define UCLKCR		0xA40A0008
#define FRQCR		0xA415FF80

/*	LOW POWER MODE	*/

/*	TMU	*/
#define TMU_BASE	0xA412FE90

/*	TPU	*/
#define TPU_BASE	0xA4480000
#define TPU_TSTR	(TPU_BASE + 0x00)
#define TPU_TCR0	(TPU_BASE + 0x10)
#define TPU_TMDR0	(TPU_BASE + 0x14)
#define TPU_TIOR0	(TPU_BASE + 0x18)
#define TPU_TIER0	(TPU_BASE + 0x1C)
#define TPU_TSR0	(TPU_BASE + 0x20)
#define TPU_TCNT0	(TPU_BASE + 0x24)
#define TPU_TGRA0	(TPU_BASE + 0x28)
#define TPU_TGRB0	(TPU_BASE + 0x2C)
#define TPU_TGRC0	(TPU_BASE + 0x30)
#define TPU_TGRD0	(TPU_BASE + 0x34)
#define TPU_TCR1	(TPU_BASE + 0x50)
#define TPU_TMDR1	(TPU_BASE + 0x54)
#define TPU_TIOR1	(TPU_BASE + 0x58)
#define TPU_TIER1	(TPU_BASE + 0x5C)
#define TPU_TSR1	(TPU_BASE + 0x60)
#define TPU_TCNT1	(TPU_BASE + 0x64)
#define TPU_TGRA1	(TPU_BASE + 0x68)
#define TPU_TGRB1	(TPU_BASE + 0x6C)
#define TPU_TGRC1	(TPU_BASE + 0x70)
#define TPU_TGRD1	(TPU_BASE + 0x74)
#define TPU_TCR2	(TPU_BASE + 0x90)
#define TPU_TMDR2	(TPU_BASE + 0x94)
#define TPU_TIOR2	(TPU_BASE + 0x98)
#define TPU_TIER2	(TPU_BASE + 0x9C)
#define TPU_TSR2	(TPU_BASE + 0xB0)
#define TPU_TCNT2	(TPU_BASE + 0xB4)
#define TPU_TGRA2	(TPU_BASE + 0xB8)
#define TPU_TGRB2	(TPU_BASE + 0xBC)
#define TPU_TGRC2	(TPU_BASE + 0xC0)
#define TPU_TGRD2	(TPU_BASE + 0xC4)
#define TPU_TCR3	(TPU_BASE + 0xD0)
#define TPU_TMDR3	(TPU_BASE + 0xD4)
#define TPU_TIOR3	(TPU_BASE + 0xD8)
#define TPU_TIER3	(TPU_BASE + 0xDC)
#define TPU_TSR3	(TPU_BASE + 0xE0)
#define TPU_TCNT3	(TPU_BASE + 0xE4)
#define TPU_TGRA3	(TPU_BASE + 0xE8)
#define TPU_TGRB3	(TPU_BASE + 0xEC)
#define TPU_TGRC3	(TPU_BASE + 0xF0)
#define TPU_TGRD3	(TPU_BASE + 0xF4)

/*	CMT	*/

/*	SIOF	*/

/*	SCIF	*/
#define SCIF0_BASE	0xA4430000

/*	SIM	*/

/*	IrDA	*/

/*	IIC	*/

/*	LCDC	*/

/*	USBF	*/

/*	MMCIF	*/

/*	PFC	*/
#define PFC_BASE	0xA4050100
#define PACR		(PFC_BASE + 0x00)
#define PBCR		(PFC_BASE + 0x02)
#define PCCR		(PFC_BASE + 0x04)
#define PDCR		(PFC_BASE + 0x06)
#define PECR		(PFC_BASE + 0x08)
#define PFCR		(PFC_BASE + 0x0A)
#define PGCR		(PFC_BASE + 0x0C)
#define PHCR		(PFC_BASE + 0x0E)
#define PJCR		(PFC_BASE + 0x10)
#define PKCR		(PFC_BASE + 0x12)
#define PLCR		(PFC_BASE + 0x14)
#define PMCR		(PFC_BASE + 0x16)
#define PPCR		(PFC_BASE + 0x18)
#define PRCR		(PFC_BASE + 0x1A)
#define PSCR		(PFC_BASE + 0x1C)
#define PTCR		(PFC_BASE + 0x1E)
#define PUCR		(PFC_BASE + 0x20)
#define PVCR		(PFC_BASE + 0x22)
#define PSELA		(PFC_BASE + 0x24)
#define PSELB		(PFC_BASE + 0x26)
#define PSELC		(PFC_BASE + 0x28)
#define PSELD		(PFC_BASE + 0x2A)

/*	I/O Port	*/
#define PORT_BASE	0xA4050100
#define PADR		(PORT_BASE + 0x40)
#define PBDR		(PORT_BASE + 0x42)
#define PCDR		(PORT_BASE + 0x44)
#define PDDR		(PORT_BASE + 0x46)
#define PEDR		(PORT_BASE + 0x48)
#define PFDR		(PORT_BASE + 0x4A)
#define PGDR		(PORT_BASE + 0x4C)
#define PHDR		(PORT_BASE + 0x4E)
#define PJDR		(PORT_BASE + 0x50)
#define PKDR		(PORT_BASE + 0x52)
#define PLDR		(PORT_BASE + 0x54)
#define PMDR		(PORT_BASE + 0x56)
#define PPDR		(PORT_BASE + 0x58)
#define PRDR		(PORT_BASE + 0x5A)
#define PSDR		(PORT_BASE + 0x5C)
#define PTDR		(PORT_BASE + 0x5E)
#define PUDR		(PORT_BASE + 0x60)
#define PVDR		(PORT_BASE + 0x62)

/*	H-UDI	*/

#endif /* _ASM_CPU_SH7720_H_ */
