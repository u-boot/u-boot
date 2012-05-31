/*
 *  Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
 *
 * (C) Copyright 2008-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _PPC4xx_UIC_H_
#define _PPC4xx_UIC_H_

/*
 * Define the number of UIC's
 */
#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_460SX) || defined(CONFIG_APM821XX)
#define UIC_MAX		4
#elif defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_405EX)
#define UIC_MAX		3
#elif defined(CONFIG_440GP) || defined(CONFIG_440SP) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define UIC_MAX		2
#else
#define UIC_MAX		1
#endif

#define IRQ_MAX		(UIC_MAX * 32)

/*
 * UIC register
 */
#define UIC_SR	0x0			/* UIC status			*/
#define UIC_ER	0x2			/* UIC enable			*/
#define UIC_CR	0x3			/* UIC critical			*/
#define UIC_PR	0x4			/* UIC polarity			*/
#define UIC_TR	0x5			/* UIC triggering		*/
#define UIC_MSR 0x6			/* UIC masked status		*/
#define UIC_VR	0x7			/* UIC vector			*/
#define UIC_VCR 0x8			/* UIC vector configuration	*/

/*
 * On 440GX we use the UICB0 as UIC0. Its the root UIC where all other UIC's
 * are cascaded on. With this trick we can use the common UIC code for 440GX
 * too.
 */
#if defined(CONFIG_440GX)
#define UIC0_DCR_BASE 0x200
#define UIC1_DCR_BASE 0xc0
#define UIC2_DCR_BASE 0xd0
#define UIC3_DCR_BASE 0x210
#else
#define UIC0_DCR_BASE 0xc0
#define UIC1_DCR_BASE 0xd0
#define UIC2_DCR_BASE 0xe0
#define UIC3_DCR_BASE 0xf0
#endif

#define UIC0SR	(UIC0_DCR_BASE+0x0)	/* UIC0 status			*/
#define UIC0ER	(UIC0_DCR_BASE+0x2)	/* UIC0 enable			*/
#define UIC0CR	(UIC0_DCR_BASE+0x3)	/* UIC0 critical		*/
#define UIC0PR	(UIC0_DCR_BASE+0x4)	/* UIC0 polarity		*/
#define UIC0TR	(UIC0_DCR_BASE+0x5)	/* UIC0 triggering		*/
#define UIC0MSR (UIC0_DCR_BASE+0x6)	/* UIC0 masked status		*/
#define UIC0VR	(UIC0_DCR_BASE+0x7)	/* UIC0 vector			*/
#define UIC0VCR (UIC0_DCR_BASE+0x8)	/* UIC0 vector configuration	*/

#define UIC1SR	(UIC1_DCR_BASE+0x0)	/* UIC1 status			*/
#define UIC1ER	(UIC1_DCR_BASE+0x2)	/* UIC1 enable			*/
#define UIC1CR	(UIC1_DCR_BASE+0x3)	/* UIC1 critical		*/
#define UIC1PR	(UIC1_DCR_BASE+0x4)	/* UIC1 polarity		*/
#define UIC1TR	(UIC1_DCR_BASE+0x5)	/* UIC1 triggering		*/
#define UIC1MSR (UIC1_DCR_BASE+0x6)	/* UIC1 masked status		*/
#define UIC1VR	(UIC1_DCR_BASE+0x7)	/* UIC1 vector			*/
#define UIC1VCR (UIC1_DCR_BASE+0x8)	/* UIC1 vector configuration	*/

#define UIC2SR	(UIC2_DCR_BASE+0x0)	/* UIC2 status-Read Clear	*/
#define UIC2ER	(UIC2_DCR_BASE+0x2)	/* UIC2 enable			*/
#define UIC2CR	(UIC2_DCR_BASE+0x3)	/* UIC2 critical		*/
#define UIC2PR	(UIC2_DCR_BASE+0x4)	/* UIC2 polarity		*/
#define UIC2TR	(UIC2_DCR_BASE+0x5)	/* UIC2 triggering		*/
#define UIC2MSR (UIC2_DCR_BASE+0x6)	/* UIC2 masked status		*/
#define UIC2VR	(UIC2_DCR_BASE+0x7)	/* UIC2 vector			*/
#define UIC2VCR (UIC2_DCR_BASE+0x8)	/* UIC2 vector configuration	*/

#define UIC3SR	(UIC3_DCR_BASE+0x0)	/* UIC3 status-Read Clear	*/
#define UIC3ER	(UIC3_DCR_BASE+0x2)	/* UIC3 enable			*/
#define UIC3CR	(UIC3_DCR_BASE+0x3)	/* UIC3 critical		*/
#define UIC3PR	(UIC3_DCR_BASE+0x4)	/* UIC3 polarity		*/
#define UIC3TR	(UIC3_DCR_BASE+0x5)	/* UIC3 triggering		*/
#define UIC3MSR (UIC3_DCR_BASE+0x6)	/* UIC3 masked status		*/
#define UIC3VR	(UIC3_DCR_BASE+0x7)	/* UIC3 vector			*/
#define UIC3VCR (UIC3_DCR_BASE+0x8)	/* UIC3 vector configuration	*/

/*
 * Now the interrupt vector definitions. They are different for most of
 * the 4xx variants, so we need some more #ifdef's here. No mask
 * definitions anymore here. For this please use the UIC_MASK macro below.
 *
 * Note: Please only define the interrupts really used in U-Boot here.
 * Those are the cascading and EMAC/MAL related interrupt.
 */

#if defined(CONFIG_405EP) || defined(CONFIG_405GP)
#define VECNUM_MAL_SERR		10
#define VECNUM_MAL_TXEOB	11
#define VECNUM_MAL_RXEOB	12
#define VECNUM_MAL_TXDE		13
#define VECNUM_MAL_RXDE		14
#define VECNUM_ETH0		15
#define VECNUM_ETH1_OFFS	2
#define VECNUM_EIRQ6		29
#endif /* defined(CONFIG_405EP) */

#if defined(CONFIG_405EZ)
#define VECNUM_USBDEV		15
#define VECNUM_ETH0		16
#define VECNUM_MAL_SERR		18
#define VECNUM_MAL_TXDE		18
#define VECNUM_MAL_RXDE		18
#define VECNUM_MAL_TXEOB	19
#define VECNUM_MAL_RXEOB	21
#endif /* CONFIG_405EX */

#if defined(CONFIG_405EX)
/* UIC 0 */
#define VECNUM_MAL_TXEOB	10
#define VECNUM_MAL_RXEOB	11
#define VECNUM_ETH0		24
#define VECNUM_ETH1_OFFS	1
#define VECNUM_UIC2NCI		28
#define VECNUM_UIC2CI		29
#define VECNUM_UIC1NCI		30
#define VECNUM_UIC1CI		31

/* UIC 1 */
#define VECNUM_MAL_SERR		(32 + 0)
#define VECNUM_MAL_TXDE		(32 + 1)
#define VECNUM_MAL_RXDE		(32 + 2)
#endif /* CONFIG_405EX */

#if defined(CONFIG_440GP) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR)
/* UIC 0 */
#define VECNUM_MAL_TXEOB	10
#define VECNUM_MAL_RXEOB	11
#define VECNUM_UIC1NCI		30
#define VECNUM_UIC1CI		31

/* UIC 1 */
#define VECNUM_MAL_SERR		(32 + 0)
#define VECNUM_MAL_TXDE		(32 + 1)
#define VECNUM_MAL_RXDE		(32 + 2)
#define VECNUM_USBDEV		(32 + 23)
#define VECNUM_ETH0		(32 + 28)
#define VECNUM_ETH1_OFFS	2
#endif /* CONFIG_440GP */

#if defined(CONFIG_440GX)
/* UICB 0 (440GX only) */
/*
 * All those defines below are off-by-one, so that the common UIC code
 * can be used. So VECNUM_UIC1CI refers to VECNUM_UIC0CI etc.
 */
#define VECNUM_UIC1CI		0
#define VECNUM_UIC1NCI		1
#define VECNUM_UIC2CI		2
#define VECNUM_UIC2NCI		3
#define VECNUM_UIC3CI		4
#define VECNUM_UIC3NCI		5

/* UIC 0, used as UIC1 on 440GX because of UICB0 */
#define VECNUM_MAL_TXEOB	(32 + 10)
#define VECNUM_MAL_RXEOB	(32 + 11)

/* UIC 1, used as UIC2 on 440GX because of UICB0 */
#define VECNUM_MAL_SERR		(64 + 0)
#define VECNUM_MAL_TXDE		(64 + 1)
#define VECNUM_MAL_RXDE		(64 + 2)
#define VECNUM_ETH0		(64 + 28)
#define VECNUM_ETH1_OFFS	2
#endif /* CONFIG_440GX */

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
/* UIC 0 */
#define VECNUM_MAL_TXEOB	10
#define VECNUM_MAL_RXEOB	11
#define VECNUM_USBDEV		20
#define VECNUM_ETH0		24
#define VECNUM_ETH1_OFFS	1
#define VECNUM_UIC2NCI		28
#define VECNUM_UIC2CI		29
#define VECNUM_UIC1NCI		30
#define VECNUM_UIC1CI		31

/* UIC 1 */
#define VECNUM_MAL_SERR		(32 + 0)
#define VECNUM_MAL_TXDE		(32 + 1)
#define VECNUM_MAL_RXDE		(32 + 2)

/* UIC 2 */
#define VECNUM_EIRQ2		(64 + 3)
#endif /* CONFIG_440EPX */

#if defined(CONFIG_440SP)
/* UIC 0 */
#define VECNUM_UIC1NCI		30
#define VECNUM_UIC1CI		31

/* UIC 1 */
#define VECNUM_MAL_SERR		(32 + 1)
#define VECNUM_MAL_TXDE		(32 + 2)
#define VECNUM_MAL_RXDE		(32 + 3)
#define VECNUM_MAL_TXEOB	(32 + 6)
#define VECNUM_MAL_RXEOB	(32 + 7)
#define VECNUM_ETH0		(32 + 28)
#endif /* CONFIG_440SP */

#if defined(CONFIG_440SPE)
/* UIC 0 */
#define VECNUM_UIC2NCI		10
#define VECNUM_UIC2CI		11
#define VECNUM_UIC3NCI		16
#define VECNUM_UIC3CI		17
#define VECNUM_UIC1NCI		30
#define VECNUM_UIC1CI		31

/* UIC 1 */
#define VECNUM_MAL_SERR		(32 + 1)
#define VECNUM_MAL_TXDE		(32 + 2)
#define VECNUM_MAL_RXDE		(32 + 3)
#define VECNUM_MAL_TXEOB	(32 + 6)
#define VECNUM_MAL_RXEOB	(32 + 7)
#define VECNUM_ETH0		(32 + 28)
#endif /* CONFIG_440SPE */

#if defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_APM821XX)
/* UIC 0 */
#define VECNUM_UIC2NCI		10
#define VECNUM_UIC2CI		11
#define VECNUM_UIC3NCI		16
#define VECNUM_UIC3CI		17
#define VECNUM_UIC1NCI		30
#define VECNUM_UIC1CI		31

/* UIC 2 */
#define VECNUM_MAL_SERR		(64 + 3)
#define	VECNUM_MAL_TXDE		(64 + 4)
#define	VECNUM_MAL_RXDE		(64 + 5)
#define VECNUM_MAL_TXEOB	(64 + 6)
#define	VECNUM_MAL_RXEOB	(64 + 7)
#define	VECNUM_ETH0		(64 + 16)
#define VECNUM_ETH1_OFFS	1
#endif /* CONFIG_460EX */

#if defined(CONFIG_460SX)
/* UIC 0 */
#define VECNUM_UIC2NCI		10
#define VECNUM_UIC2CI		11
#define VECNUM_UIC3NCI		16
#define VECNUM_UIC3CI		17
#define	VECNUM_ETH0		19
#define VECNUM_ETH1_OFFS	1
#define VECNUM_UIC1NCI		30
#define VECNUM_UIC1CI		31

/* UIC 1 */
#define VECNUM_MAL_SERR		(32 + 1)
#define	VECNUM_MAL_TXDE		(32 + 2)
#define	VECNUM_MAL_RXDE		(32 + 3)
#define VECNUM_MAL_TXEOB	(32 + 6)
#define	VECNUM_MAL_RXEOB	(32 + 7)
#endif /* CONFIG_460EX */

#if !defined(VECNUM_ETH1_OFFS)
#define VECNUM_ETH1_OFFS	1
#endif

/*
 * Mask definitions (used for example in 4xx_enet.c)
 */
#define UIC_MASK(vec)		(0x80000000 >> ((vec) & 0x1f))
/* UIC_NR won't work for 440GX because of its specific UIC DCR addresses */
#define UIC_NR(vec)		((vec) >> 5)

#endif /* _PPC4xx_UIC_H_ */
