/*
 * (C) Copyright 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * SH7750/SH7750S/SH7750R/SH7751/SH7751R
 *  Internal I/O register
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _ASM_CPU_SH7750_H_
#define _ASM_CPU_SH7750_H_

#ifdef CONFIG_CPU_TYPE_R
#define CACHE_OC_NUM_WAYS     2
#define CCR_CACHE_INIT   0x8000090d     /* EMODE,ICI,ICE(16k),OCI,P1-wb,OCE(32k) */
#else
#define CACHE_OC_NUM_WAYS     1
#define CCR_CACHE_INIT   0x0000090b
#endif

/*      OCN     */
#define PTEH 	0xFF000000
#define PTEL 	0xFF000004
#define TTB 	0xFF000008
#define TEA 	0xFF00000C
#define MMUCR 	0xFF000010
#define BASRA 	0xFF000014
#define BASRB	0xFF000018
#define CCR	0xFF00001C
#define TRA 	0xFF000020
#define EXPEVT 	0xFF000024
#define INTEVT 	0xFF000028
#define PTEA 	0xFF000034
#define QACR0 	0xFF000038
#define QACR1 	0xFF00003C

/*      UBC     */
#define BARA 	0xFF200000
#define BAMRA 	0xFF200004
#define BBRA 	0xFF200008
#define BARB 	0xFF20000C
#define BAMRB 	0xFF200010
#define BBRB 	0xFF200014
#define BDRB 	0xFF200018
#define BDMRB 	0xFF20001C
#define BRCR 	0xFF200020

/*      BSC     */
#define BCR1	0xFF800000
#define BCR2	0xFF800004
#define BCR3 	0xFF800050
#define BCR4	0xFE0A00F0
#define WCR1 	0xFF800008
#define WCR2 	0xFF80000C
#define WCR3 	0xFF800010
#define MCR 	0xFF800014
#define PCR 	0xFF800018
#define RTCSR 	0xFF80001C
#define RTCNT 	0xFF800020
#define RTCOR 	0xFF800024
#define RFCR 	0xFF800028
#define PCTRA 	0xFF80002C
#define PDTRA 	0xFF800030
#define PCTRB 	0xFF800040
#define PDTRB 	0xFF800044
#define GPIOIC 	0xFF800048

/*      DMAC    */
#define SAR0 	0xFFA00000
#define DAR0 	0xFFA00004
#define DMATCR0 0xFFA00008
#define CHCR0	0xFFA0000C
#define SAR1 	0xFFA00010
#define DAR1 	0xFFA00014
#define DMATCR1 0xFFA00018
#define CHCR1 	0xFFA0001C
#define SAR2 	0xFFA00020
#define DAR2 	0xFFA00024
#define DMATCR2 0xFFA00028
#define CHCR2 	0xFFA0002C
#define SAR3 	0xFFA00030
#define DAR3 	0xFFA00034
#define DMATCR3 0xFFA00038
#define CHCR3 	0xFFA0003C
#define DMAOR 	0xFFA00040
#define SAR4	0xFFA00050
#define DAR4 	0xFFA00054
#define DMATCR4 0xFFA00058

/*      CPG     */
#define FRQCR 	0xFFC00000
#define STBCR 	0xFFC00004
#define WTCNT 	0xFFC00008
#define WTCSR 	0xFFC0000C
#define STBCR2 	0xFFC00010

/*      RTC     */
#define R64CNT	0xFFC80000
#define RSECCNT 0xFFC80004
#define RMINCNT 0xFFC80008
#define RHRCNT 	0xFFC8000C
#define RWKCNT 	0xFFC80010
#define RDAYCNT 0xFFC80014
#define RMONCNT 0xFFC80018
#define RYRCNT 	0xFFC8001C
#define RSECAR 	0xFFC80020
#define RMINAR 	0xFFC80024
#define RHRAR 	0xFFC80028
#define RWKAR 	0xFFC8002C
#define RDAYAR 	0xFFC80030
#define RMONAR 	0xFFC80034
#define RCR1 	0xFFC80038
#define RCR2 	0xFFC8003C
#define RCR3 	0xFFC80050
#define RYRAR 	0xFFC80054

/*      ICR     */
#define ICR 	0xFFD00000
#define IPRA 	0xFFD00004
#define IPRB 	0xFFD00008
#define IPRC	0xFFD0000C
#define IPRD 	0xFFD00010
#define INTPRI 	0xFE080000
#define INTREQ	0xFE080020
#define INTMSK	0xFE080040
#define INTMSKCL	0xFE080060

/*      CPG     */
#define CLKSTP		0xFE0A0000
#define CLKSTPCLR	0xFE0A0008

/*      TMU     */
#define TSTR2 	0xFE100004
#define TCOR3 	0xFE100008
#define TCNT3 	0xFE10000C
#define TCR3 	0xFE100010
#define TCOR4 	0xFE100014
#define TCNT4 	0xFE100018
#define TCR4 	0xFE10001C
#define TOCR 	0xFFD80000
#define TSTR0 	0xFFD80004
#define TCOR0	0xFFD80008
#define TCNT0 	0xFFD8000C
#define TCR0 	0xFFD80010
#define TCOR1 	0xFFD80014
#define TCNT1 	0xFFD80018
#define TCR1 	0xFFD8001C
#define TCOR2 	0xFFD80020
#define TCNT2 	0xFFD80024
#define TCR2 	0xFFD80028
#define TCPR2 	0xFFD8002C
#define TSTR	TSTR0

/*      SCI     */
#define SCSMR1 	0xFFE00000
#define SCBRR1 	0xFFE00004
#define SCSCR1 	0xFFE00008
#define SCTDR1 	0xFFE0000C
#define SCSSR1 	0xFFE00010
#define SCRDR1 	0xFFE00014
#define SCSCMR1 0xFFE00018
#define SCSPTR1 0xFFE0001C
#define SCF0_BASE	SCSMR1

/*      SCIF    */
#define SCSMR2 	0xFFE80000
#define SCBRR2 	0xFFE80004
#define SCSCR2 	0xFFE80008
#define SCFTDR2 0xFFE8000C
#define SCFSR2 	0xFFE80010
#define SCFRDR2	0xFFE80014
#define SCFCR2 	0xFFE80018
#define SCFDR2 	0xFFE8001C
#define SCSPTR2	0xFFE80020
#define SCLSR2 	0xFFE80024
#define SCIF1_BASE	SCSMR2

/*      H-UDI   */
#define SDIR 	0xFFF00000
#define SDDR 	0xFFF00008
#define SDINT 	0xFFF00014

#endif	/* _ASM_CPU_SH7750_H_ */
