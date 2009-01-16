/*
 modified from SH-IPL+g
 Renesaso SuperH / Solution Enginge MS775xSE01 BSC setting.

 Support CPU : SH7750/SH7750S/SH7750R/SH7751/SH7751R

 Coyright (c) 2007 Nobuhiro Iwamatsu <iwmatsu@nigauri.org>

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

#include <config.h>
#include <version.h>

#include <asm/processor.h>
#include <asm/macro.h>

#ifdef CONFIG_CPU_SH7751
#define BCR2_D_VALUE	0x2FFC		/* Area 1-6 width: 32/32/32/32/32/16 */
#define WCR1_D_VALUE	0x02770771	/* DMA:0 A6:2 A3:0 A0:1 Others:15 */
#ifdef CONFIG_MARUBUN_PCCARD
#define WCR2_D_VALUE	0xFFFE4FE7	/* A6:15 A6B:7 A5:15 A5B:7 A4:15
					   A3:2  A2:15 A1:15 A0:6  A0B:7  */
#else /* CONFIG_MARUBUN_PCCARD */
#define WCR2_D_VALUE	0x7FFE4FE7	/* A6:3  A6B:7 A5:15 A5B:7 A4:15
					   A3:2  A2:15 A1:15 A0:6  A0B:7  */
#endif /* CONFIG_MARUBUN_PCCARD */
#define WCR3_D_VALUE	0x01777771	/* A6: 0-1 A5: 1-3 A4: 1-3 A3: 1-3
					   A2: 1-3 A1: 1-3 A0: 0-1 */
#define RTCOR_D_VALUE	0xA50D		/* Write code A5, data 0D (~15us?) */
#define SDMR3_ADDRESS	0xFF940088	/* SDMR3 address on 32-bit bus */
#define MCR_D1_VALUE	0x100901B4	/* SDRAM 32-bit, CAS/RAS Refresh, .. */
#define MCR_D2_VALUE	0x500901B4	/* Same w/MRSET now 1 (mode reg cmd) */
#else /* CONFIG_CPU_SH7751 */
#define BCR2_D_VALUE	0x2E3C		/* Area 1-6 width: 32/32/64/16/32/16 */
#define WCR1_D_VALUE	0x02720777	/* DMA:0 A6:2 A4:2 A3:0 Others:15 */
#define WCR2_D_VALUE	0xFFFE4FFF	/* A6:15 A6B:7 A5:15 A5B:7 A4:15
					   A3:2  A2:15 A1:15 A0:15 A0B:7  */
#define WCR3_D_VALUE	0x01717771	/* A6: 0-1 A5: 1-3 A4: 0-1 A3: 1-3
					   A2: 1-3 A1: 1-3 A0: 0-1 */
#define RTCOR_D_VALUE	0xA510		/* Write code A5, data 10 (~15us?) */
#define SDMR3_ADDRESS	0xFF940110	/* SDMR3 address on 64-bit bus */
#define MCR_D1_VALUE	0x8801001C	/* SDRAM 64-bit, CAS/RAS Refresh, .. */
#define MCR_D2_VALUE	0xC801001C	/* Same w/MRSET now 1 (mode reg cmd) */
#endif /* CONFIG_CPU_SH7751 */

	.global lowlevel_init
	.text
	.align	2

lowlevel_init:

	write32	CCR_A, CCR_D_DISABLE

init_bsc:
	write16	FRQCR_A, FRQCR_D

	write32	BCR1_A, BCR1_D

	write16	BCR2_A, BCR2_D

	write32	WCR1_A, WCR1_D

	write32	WCR2_A, WCR2_D

	write32	WCR3_A, WCR3_D

	write32	MCR_A, MCR_D1

	/* Set SDRAM mode */
	write8	SDMR3_A, SDMR3_D

	! Do you need PCMCIA setting?
	! If so, please add the lines here...

	write16	RTCNT_A, RTCNT_D

	write16	RTCOR_A, RTCOR_D

	write16	RTCSR_A, RTCSR_D

	write16	RFCR_A, RFCR_D

	/* Wait DRAM refresh 30 times */
	mov	#30, r3
1:
	mov.w	@r1, r0
	extu.w	r0, r2
	cmp/hi	r3, r2
	bf	1b

	write32	MCR_A, MCR_D2

	/* Set SDRAM mode */
	write8	SDMR3_A, SDMR3_D

	rts
	nop

	.align	2

CCR_A:		 .long	CCR
CCR_D_DISABLE:	.long	0x0808
FRQCR_A:	.long	FRQCR
FRQCR_D:
#ifdef CONFIG_CPU_TYPE_R
		.long	0x00000e1a	/* 12:3:3 */
#else	/* CONFIG_CPU_TYPE_R */
#ifdef CONFIG_GOOD_SESH4
		.long	0x00000e13	/* 6:2:1 */
#else
		.long	0x00000e23	/* 6:1:1 */
#endif
#endif	/* CONFIG_CPU_TYPE_R */

BCR1_A:		.long	BCR1
BCR1_D:		.long	0x00000008	/* Area 3 SDRAM */
BCR2_A:		.long	BCR2
BCR2_D:		.long	BCR2_D_VALUE	/* Bus width settings */
WCR1_A:		.long	WCR1
WCR1_D:		.long	WCR1_D_VALUE	/* Inter-area or turnaround wait states */
WCR2_A:		.long	WCR2
WCR2_D:		.long	WCR2_D_VALUE	/* Per-area access and burst wait states */
WCR3_A:		.long	WCR3
WCR3_D:		.long	WCR3_D_VALUE	/* Address setup and data hold cycles */
RTCSR_A:	.long	RTCSR
RTCSR_D:	.long	0xA518		/* RTCSR Write Code A5h Data 18h */
RTCNT_A:	.long	RTCNT
RTCNT_D:	.long	0xA500		/* RTCNT Write Code A5h Data 00h */
RTCOR_A:	.long	RTCOR
RTCOR_D:	.long	RTCOR_D_VALUE	/* Set refresh time (about 15us) */
SDMR3_A:	.long	SDMR3_ADDRESS
SDMR3_D:	.long	0x00
MCR_A:		.long	MCR
MCR_D1:		.long	MCR_D1_VALUE
MCR_D2:		.long	MCR_D2_VALUE
RFCR_A:		.long	RFCR
RFCR_D:		.long	0xA400		/* RFCR Write Code A4h Data 00h */
