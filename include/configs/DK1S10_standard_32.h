/*
 * (C) Copyright 2003, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_DK1S10_STANDARD_32_H
#define __CONFIG_DK1S10_STANDARD_32_H

/*
 * NIOS CPU configuration. (PART OF configs/DK1S10.h)
 *
 * Here we must define CPU dependencies. Any unsupported option have to
 * be defined with zero, example CPU without data cache / OCI:
 *
 *	#define	CFG_NIOS_CPU_ICACHE	4096
 *	#define	CFG_NIOS_CPU_DCACHE	0
 *	#define	CFG_NIOS_CPU_OCI_BASE	0
 *	#define	CFG_NIOS_CPU_OCI_SIZE	0
 */

/* CPU core */
#define	CFG_NIOS_CPU_CLK	50000000	/* NIOS CPU clock	*/
#define	CFG_NIOS_CPU_ICACHE	(4 * 1024)	/* instruction cache	*/
#define	CFG_NIOS_CPU_DCACHE	(4 * 1024)	/* data cache		*/
#define	CFG_NIOS_CPU_REG_NUMS	256		/* number of register	*/
#define	CFG_NIOS_CPU_MUL	0		/* 16x16 MUL:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_MSTEP	1		/* 16x16 MSTEP:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_STACK	0x008fff00	/* stack top	addr	*/
#define	CFG_NIOS_CPU_VEC_BASE	0x008fff00	/* IRQ vectors	addr	*/
#define	CFG_NIOS_CPU_VEC_SIZE	256		/*		size	*/
#define	CFG_NIOS_CPU_VEC_NUMS	64		/*		numbers	*/
#define	CFG_NIOS_CPU_RST_VECT	0x00920000	/* RESET vector	addr	*/
#define	CFG_NIOS_CPU_DBG_CORE	0		/* CPU debug:	no(0)	*/
						/*		yes(1)	*/

/* on-chip extensions */
#define	CFG_NIOS_CPU_RAM_BASE	0x00900000	/* on chip RAM	addr	*/
#define	CFG_NIOS_CPU_RAM_SIZE	(64 * 1024)	/* 64 KB	size	*/

#define	CFG_NIOS_CPU_ROM_BASE	0x00920000	/* on chip ROM	addr	*/
#define	CFG_NIOS_CPU_ROM_SIZE	(2 * 1024)	/*  2 KB	size	*/

#define	CFG_NIOS_CPU_OCI_BASE	0x00920800	/* OCI core	addr	*/
#define	CFG_NIOS_CPU_OCI_SIZE	256		/*		size	*/

/* timer */
#define	CFG_NIOS_CPU_TIMER_NUMS	2		/* number of timer	*/

#define	CFG_NIOS_CPU_TIMER0	0x00920940	/* TIMER0	addr	*/
#define	CFG_NIOS_CPU_TIMER0_IRQ	16		/*		IRQ	*/
#define	CFG_NIOS_CPU_TIMER0_PER	1000		/*  periode	usec	*/
#define	CFG_NIOS_CPU_TIMER0_AR	0		/*  always run:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_TIMER0_FP	0		/*  fixed per:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_TIMER0_SS	1		/*  snaphot:	no(0)	*/
						/*		yes(1)	*/

#define	CFG_NIOS_CPU_TIMER1	0x009209e0	/* TIMER1	addr	*/
#define	CFG_NIOS_CPU_TIMER1_IRQ	50		/*		IRQ	*/
#define	CFG_NIOS_CPU_TIMER1_PER	10000		/*  periode	usec	*/
#define	CFG_NIOS_CPU_TIMER1_AR	1		/*  always run:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_TIMER1_FP	1		/*  fixed per:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_TIMER1_SS	0		/*  snaphot:	no(0)	*/
						/*		yes(1)	*/

/* serial i/o */
#define	CFG_NIOS_CPU_UART_NUMS	1		/* number of uarts	*/

#define	CFG_NIOS_CPU_UART0	0x00920900	/* UART0	addr	*/
#define	CFG_NIOS_CPU_UART0_IRQ	25		/*		IRQ	*/
#define	CFG_NIOS_CPU_UART0_BR	115200		/*  baudrate	var(0)	*/
#define	CFG_NIOS_CPU_UART0_DB	8		/*  data bit		*/
#define	CFG_NIOS_CPU_UART0_SB	1		/*  stop bit		*/
#define	CFG_NIOS_CPU_UART0_PA	0		/*  parity	none(0)	*/
						/*		odd(1)	*/
						/*		even(2)	*/
#define	CFG_NIOS_CPU_UART0_HS	0		/*  handshake:	no(0)	*/
						/*		crts(1)	*/
#define	CFG_NIOS_CPU_UART0_EOP	0		/*  eop reg:	no(0)	*/
						/*		yes(1)	*/

/* parallel i/o */
#define	CFG_NIOS_CPU_PIO_NUMS	8		/* number of parports	*/

#define	CFG_NIOS_CPU_PIO0	0x00920960	/* PIO0		addr	*/
#define	CFG_NIOS_CPU_PIO0_IRQ	40		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO0_BITS	4		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO0_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO0_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO0_EDGE	3		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO0_ITYPE	2		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO1	0x00920970	/* PIO1		addr	*/
#undef	CFG_NIOS_CPU_PIO1_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO1_BITS	11		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO1_TYPE	0		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO1_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO1_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO1_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO2	0x00920980	/* PIO2		addr	*/
#undef	CFG_NIOS_CPU_PIO2_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO2_BITS	8		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO2_TYPE	1		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO2_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO2_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO2_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO3	0x00920990	/* PIO3		addr	*/
#undef	CFG_NIOS_CPU_PIO3_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO3_BITS	16		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO3_TYPE	1		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO3_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO3_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO3_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO4	0x009209a0	/* PIO4		addr	*/
#undef	CFG_NIOS_CPU_PIO4_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO4_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO4_TYPE	0		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO4_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO4_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO4_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO5	0x009209b0	/* PIO5		addr	*/
#define	CFG_NIOS_CPU_PIO5_IRQ	35		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO5_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO5_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO5_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO5_EDGE	3		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO5_ITYPE	2		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO6	0x009209c0	/* PIO6		addr	*/
#undef	CFG_NIOS_CPU_PIO6_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO6_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO6_TYPE	1		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO6_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO6_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO6_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO7	0x009209d0	/* PIO7		addr	*/
#undef	CFG_NIOS_CPU_PIO7_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO7_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO7_TYPE	1		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO7_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO7_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO7_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

/* IDE i/f */
#define	CFG_NIOS_CPU_IDE_NUMS	1		/* number of IDE contr.	*/
#define	CFG_NIOS_CPU_IDE0	0x00920a00	/* IDE0		addr	*/

/* memory accessibility */
#define	CFG_NIOS_CPU_SRAM_BASE	0x00800000	/* board SRAM	addr	*/
#define	CFG_NIOS_CPU_SRAM_SIZE	(1024 * 1024)	/*  1 MB	size	*/

#define	CFG_NIOS_CPU_SDRAM_BASE	0x01000000	/* board SDRAM	addr	*/
#define	CFG_NIOS_CPU_SDRAM_SIZE	(16*1024*1024)	/* 16 MB	size	*/

#define	CFG_NIOS_CPU_FLASH_BASE	0x00000000	/* board Flash	addr	*/
#define	CFG_NIOS_CPU_FLASH_SIZE	(8*1024*1024)	/*  8 MB	size	*/

/* LAN */
#define	CFG_NIOS_CPU_LAN_NUMS	1		/* number of LAN i/f	*/

#define	CFG_NIOS_CPU_LAN0_BASE	0x00910000	/* LAN0		addr	*/
#define	CFG_NIOS_CPU_LAN0_OFFS	0x0300		/*		offset	*/
#define	CFG_NIOS_CPU_LAN0_IRQ	30		/*		IRQ	*/
#define	CFG_NIOS_CPU_LAN0_BUSW	32		/*	        buswidth*/
#define	CFG_NIOS_CPU_LAN0_TYPE	0		/*	smc91111(0)	*/
						/*	cs8900(1)	*/
						/* ex:	alteramac(2)	*/

/* symbolic redefinition (undef, if not present) */
#define	CFG_NIOS_CPU_USER_TIMER		0	/* TIMER0: users choice	*/
#define	CFG_NIOS_CPU_TICK_TIMER		1	/* TIMER1: tick (needed)*/

#define	CFG_NIOS_CPU_BUTTON_PIO		0	/* PIO0: buttons	*/
#define	CFG_NIOS_CPU_LCD_PIO		1	/* PIO1: ASCII LCD	*/
#define	CFG_NIOS_CPU_LED_PIO		2	/* PIO2: LED bar	*/
#define	CFG_NIOS_CPU_SEVENSEG_PIO	3	/* PIO3: 7-seg. display	*/
#define	CFG_NIOS_CPU_RECONF_PIO		4	/* PIO4: reconf pin	*/
#define	CFG_NIOS_CPU_CFPRESENT_PIO	5	/* PIO5: CF present IRQ	*/
#define	CFG_NIOS_CPU_CFPOWER_PIO	6	/* PIO6: CF power/sw.	*/
#define	CFG_NIOS_CPU_CFATASEL_PIO	7	/* PIO7: CF ATA select	*/

#endif	/* __CONFIG_DK1S10_STANDARD_32_H */
