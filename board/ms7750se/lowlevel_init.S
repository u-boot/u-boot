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

#ifdef CONFIG_CPU_SH7751
#define BCR2_D_VALUE	0x2FFC	   /* Area 1-6 width: 32/32/32/32/32/16 */
#define WCR1_D_VALUE    0x02770771 /* DMA:0 A6:2 A3:0 A0:1 Others:15 */
#ifdef CONFIG_MARUBUN_PCCARD
#define WCR2_D_VALUE    0xFFFE4FE7 /* A6:15 A6B:7 A5:15 A5B:7 A4:15
				      A3:2  A2:15 A1:15 A0:6  A0B:7  */
#else /* CONFIG_MARUBUN_PCCARD */
#define WCR2_D_VALUE    0x7FFE4FE7 /* A6:3  A6B:7 A5:15 A5B:7 A4:15
				      A3:2  A2:15 A1:15 A0:6  A0B:7  */
#endif /* CONFIG_MARUBUN_PCCARD */
#define WCR3_D_VALUE	0x01777771 /* A6: 0-1 A5: 1-3 A4: 1-3 A3: 1-3
				      A2: 1-3 A1: 1-3 A0: 0-1 */
#define RTCOR_D_VALUE	0xA50D	   /* Write code A5, data 0D (~15us?) */
#define SDMR3_ADDRESS	0xFF940088 /* SDMR3 address on 32-bit bus */
#define MCR_D1_VALUE	0x100901B4 /* SDRAM 32-bit, CAS/RAS Refresh, ... */
#define MCR_D2_VALUE	0x500901B4 /* Same w/MRSET now 1 (mode reg cmd) */
#else /* CONFIG_CPU_SH7751 */
#define BCR2_D_VALUE	0x2E3C	   /* Area 1-6 width: 32/32/64/16/32/16 */
#define WCR1_D_VALUE	0x02720777 /* DMA:0 A6:2 A4:2 A3:0 Others:15 */
#define WCR2_D_VALUE	0xFFFE4FFF /* A6:15 A6B:7 A5:15 A5B:7 A4:15
				      A3:2  A2:15 A1:15 A0:15 A0B:7  */
#define WCR3_D_VALUE	0x01717771 /* A6: 0-1 A5: 1-3 A4: 0-1 A3: 1-3
				      A2: 1-3 A1: 1-3 A0: 0-1 */
#define RTCOR_D_VALUE	0xA510	   /* Write code A5, data 10 (~15us?) */
#define SDMR3_ADDRESS	0xFF940110 /* SDMR3 address on 64-bit bus */
#define MCR_D1_VALUE	0x8801001C /* SDRAM 64-bit, CAS/RAS Refresh, ... */
#define MCR_D2_VALUE	0xC801001C /* Same w/MRSET now 1 (mode reg cmd) */
#endif /* CONFIG_CPU_SH7751 */

	.global lowlevel_init
	.text
	.align  2

lowlevel_init:

	mov.l   CCR_A, r1               ! CCR Address
	mov.l   CCR_D_DISABLE, r0       ! CCR Data
	mov.l   r0, @r1

init_bsc:
	mov.l	FRQCR_A,r1	/* FRQCR Address */
	mov.l	FRQCR_D,r0	/* FRQCR Data */
	mov.w	r0,@r1

	mov.l	BCR1_A,r1	/* BCR1 Address */
	mov.l	BCR1_D,r0	/* BCR1 Data */
	mov.l	r0,@r1

	mov.l	BCR2_A,r1	/* BCR2 Address */
	mov.l	BCR2_D,r0	/* BCR2 Data */
	mov.w	r0,@r1

	mov.l	WCR1_A,r1	/* WCR1 Address */
	mov.l	WCR1_D,r0	/* WCR1 Data */
	mov.l	r0,@r1

	mov.l	WCR2_A,r1	/* WCR2 Address */
	mov.l	WCR2_D,r0	/* WCR2 Data */
	mov.l	r0,@r1

	mov.l	WCR3_A,r1	/* WCR3 Address */
	mov.l	WCR3_D,r0	/* WCR3 Data */
	mov.l	r0,@r1

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D1,r0	/* MCR Data1 */
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	! Do you need PCMCIA setting?
	! If so, please add the lines here...

	mov.l	RTCNT_A,r1	/* RTCNT Address */
	mov.l	RTCNT_D,r0	/* RTCNT Data */
	mov.w	r0,@r1

	mov.l	RTCOR_A,r1	/* RTCOR Address */
	mov.l	RTCOR_D,r0	/* RTCOR Data */
	mov.w	r0,@r1

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.l	RTCSR_D,r0	/* RTCSR Data */
	mov.w	r0,@r1

	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */
	/* Wait DRAM refresh 30 times */
	mov	#30,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D2,r0	/* MCR Data2 */
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	rts
	 nop

	.align	2

CCR_A:          .long   CCR
CCR_D_DISABLE:  .long   0x0808
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
MCR_A:		.long	MCR
MCR_D1:		.long	MCR_D1_VALUE
MCR_D2:		.long	MCR_D2_VALUE
RFCR_A:		.long	RFCR
RFCR_D:		.long	0xA400		/* RFCR Write Code A4h Data 00h */
