#ifndef	_ASM_CPU_SH7785_H_
#define	_ASM_CPU_SH7785_H_

/*
 * Copyright (c) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * Copyright (c) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 * Copyright (c) 2008 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
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
 *
 */

#define	CACHE_OC_NUM_WAYS	1
#define	CCR_CACHE_INIT		0x0000090b

/*	Exceptions	*/
#define	TRA		0xFF000020
#define	EXPEVT	0xFF000024
#define	INTEVT	0xFF000028

/* Cache Controller */
#define	CCR	0xFF00001C
#define	QACR0	0xFF000038
#define	QACR1	0xFF00003C
#define	RAMCR	0xFF000074

/* Watchdog Timer and Reset */
#define	WTCNT	WDTCNT
#define	WDTST	0xFFCC0000
#define	WDTCSR	0xFFCC0004
#define	WDTBST	0xFFCC0008
#define	WDTCNT	0xFFCC0010
#define	WDTBCNT	0xFFCC0018

/* Timer Unit */
#define TMU_BASE	0xFFD80000

/* Serial Communication	Interface with FIFO */
#define	SCIF1_BASE	0xffeb0000

/* LBSC */
#define MMSELR		0xfc400020
#define LBSC_BASE	0xff800000
#define BCR		(LBSC_BASE + 0x1000)
#define CS0BCR		(LBSC_BASE + 0x2000)
#define CS1BCR		(LBSC_BASE + 0x2010)
#define CS2BCR		(LBSC_BASE + 0x2020)
#define CS3BCR		(LBSC_BASE + 0x2030)
#define CS4BCR		(LBSC_BASE + 0x2040)
#define CS5BCR		(LBSC_BASE + 0x2050)
#define CS6BCR		(LBSC_BASE + 0x2060)
#define CS0WCR		(LBSC_BASE + 0x2008)
#define CS1WCR		(LBSC_BASE + 0x2018)
#define CS2WCR		(LBSC_BASE + 0x2028)
#define CS3WCR		(LBSC_BASE + 0x2038)
#define CS4WCR		(LBSC_BASE + 0x2048)
#define CS5WCR		(LBSC_BASE + 0x2058)
#define CS6WCR		(LBSC_BASE + 0x2068)
#define CS5PCR		(LBSC_BASE + 0x2070)
#define CS6PCR		(LBSC_BASE + 0x2080)

/* PCI	Controller */
#define	SH7780_PCIECR		0xFE000008
#define	SH7780_PCIVID		0xFE040000
#define	SH7780_PCIDID		0xFE040002
#define	SH7780_PCICMD		0xFE040004
#define	SH7780_PCISTATUS	0xFE040006
#define	SH7780_PCIRID		0xFE040008
#define	SH7780_PCIPIF		0xFE040009
#define	SH7780_PCISUB		0xFE04000A
#define	SH7780_PCIBCC		0xFE04000B
#define	SH7780_PCICLS		0xFE04000C
#define	SH7780_PCILTM		0xFE04000D
#define	SH7780_PCIHDR		0xFE04000E
#define	SH7780_PCIBIST		0xFE04000F
#define	SH7780_PCIIBAR		0xFE040010
#define	SH7780_PCIMBAR0		0xFE040014
#define	SH7780_PCIMBAR1		0xFE040018
#define	SH7780_PCISVID		0xFE04002C
#define	SH7780_PCISID		0xFE04002E
#define	SH7780_PCICP		0xFE040034
#define	SH7780_PCIINTLINE	0xFE04003C
#define	SH7780_PCIINTPIN	0xFE04003D
#define	SH7780_PCIMINGNT	0xFE04003E
#define	SH7780_PCIMAXLAT	0xFE04003F
#define	SH7780_PCICID		0xFE040040
#define	SH7780_PCINIP		0xFE040041
#define	SH7780_PCIPMC		0xFE040042
#define	SH7780_PCIPMCSR		0xFE040044
#define	SH7780_PCIPMCSRBSE	0xFE040046
#define	SH7780_PCI_CDD		0xFE040047
#define	SH7780_PCICR		0xFE040100
#define	SH7780_PCILSR0		0xFE040104
#define	SH7780_PCILSR1		0xFE040108
#define	SH7780_PCILAR0		0xFE04010C
#define	SH7780_PCILAR1		0xFE040110
#define	SH7780_PCIIR		0xFE040114
#define	SH7780_PCIIMR		0xFE040118
#define	SH7780_PCIAIR		0xFE04011C
#define	SH7780_PCICIR		0xFE040120
#define	SH7780_PCIAINT		0xFE040130
#define	SH7780_PCIAINTM		0xFE040134
#define	SH7780_PCIBMIR		0xFE040138
#define	SH7780_PCIPAR		0xFE0401C0
#define	SH7780_PCIPINT		0xFE0401CC
#define	SH7780_PCIPINTM		0xFE0401D0
#define	SH7780_PCIMBR0		0xFE0401E0
#define	SH7780_PCIMBMR0		0xFE0401E4
#define	SH7780_PCIMBR1		0xFE0401E8
#define	SH7780_PCIMBMR1		0xFE0401EC
#define	SH7780_PCIMBR2		0xFE0401F0
#define	SH7780_PCIMBMR2		0xFE0401F4
#define	SH7780_PCIIOBR		0xFE0401F8
#define	SH7780_PCIIOBMR		0xFE0401FC
#define	SH7780_PCICSCR0		0xFE040210
#define	SH7780_PCICSCR1		0xFE040214
#define	SH7780_PCICSAR0		0xFE040218
#define	SH7780_PCICSAR1		0xFE04021C
#define	SH7780_PCIPDR		0xFE040220

#endif	/* _ASM_CPU_SH7780_H_ */
