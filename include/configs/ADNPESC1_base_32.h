/*
 * (C) Copyright 2004, Li-Pro.Net <www.li-pro.net>
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

#ifndef __CONFIG_ADNPESC1_BASE_32_H
#define __CONFIG_ADNPESC1_BASE_32_H

/*
 * NIOS CPU configuration. (PART OF configs/ADNPESC1.h)
 *
 * Here we must define CPU dependencies. Any unsupported option have to
 * be undefined or defined with zero, example CPU without data cache / OCI:
 *
 *	#define	CFG_NIOS_CPU_ICACHE	4096
 *	#define	CFG_NIOS_CPU_DCACHE	0
 *	#undef	CFG_NIOS_CPU_OCI_BASE
 *	#undef	CFG_NIOS_CPU_OCI_SIZE
 */

/* CPU core */
#define	CFG_NIOS_CPU_CLK	50000000	/* NIOS CPU clock	*/
#define	CFG_NIOS_CPU_ICACHE	(0)		/* instruction cache	*/
#define	CFG_NIOS_CPU_DCACHE	(0)		/* data cache		*/
#define	CFG_NIOS_CPU_REG_NUMS	512		/* number of register	*/
#define	CFG_NIOS_CPU_MUL	0		/* 16x16 MUL:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_MSTEP	1		/* 16x16 MSTEP:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_STACK	0x03000000	/* stack top	addr	*/
#define	CFG_NIOS_CPU_VEC_BASE	0x02000000	/* IRQ vectors	addr	*/
#define	CFG_NIOS_CPU_VEC_SIZE	256		/*		size	*/
#define	CFG_NIOS_CPU_VEC_NUMS	64		/*		numbers	*/
#define	CFG_NIOS_CPU_RST_VECT	0x00000000	/* RESET vector	addr	*/
#define	CFG_NIOS_CPU_DBG_CORE	0		/* CPU debug:	no(0)	*/
						/*		yes(1)	*/

/* The offset address in flash to check for the Nios signature "Ni".
 * (see GM_FlashExec in germs_monitor.s) */
#define	CFG_NIOS_CPU_EXES_OFFS	0x0C

/* on-chip extensions */
#undef	CFG_NIOS_CPU_RAM_BASE			/* on chip RAM	addr	*/
#undef	CFG_NIOS_CPU_RAM_SIZE			/* 64 KB	size	*/

#define	CFG_NIOS_CPU_ROM_BASE	0x00000000	/* on chip ROM	addr	*/
#define	CFG_NIOS_CPU_ROM_SIZE	(2 * 1024)	/*  2 KB	size	*/

#undef	CFG_NIOS_CPU_OCI_BASE			/* OCI core	addr	*/
#undef	CFG_NIOS_CPU_OCI_SIZE			/*		size	*/

/* timer */
#define	CFG_NIOS_CPU_TIMER_NUMS	1		/* number of timer	*/

#define	CFG_NIOS_CPU_TIMER0	0x00000840	/* TIMER0	addr	*/
#define	CFG_NIOS_CPU_TIMER0_IRQ	16		/*		IRQ	*/
#define	CFG_NIOS_CPU_TIMER0_PER	1000		/*  periode	usec	*/
#define	CFG_NIOS_CPU_TIMER0_AR	0		/*  always run:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_TIMER0_FP	0		/*  fixed per:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_TIMER0_SS	1		/*  snaphot:	no(0)	*/
						/*		yes(1)	*/

/* serial i/o */
#define	CFG_NIOS_CPU_UART_NUMS	2		/* number of uarts	*/

#define	CFG_NIOS_CPU_UART0	0x00000800	/* UART0	addr	*/
#define	CFG_NIOS_CPU_UART0_IRQ	17		/*		IRQ	*/
#define	CFG_NIOS_CPU_UART0_BR	115200		/*  baudrate	var(0)	*/
#define	CFG_NIOS_CPU_UART0_DB	8		/*  data bit		*/
#define	CFG_NIOS_CPU_UART0_SB	1		/*  stop bit		*/
#define	CFG_NIOS_CPU_UART0_PA	0		/*  parity	none(0)	*/
						/*		odd(1)	*/
						/*		even(2)	*/
#define	CFG_NIOS_CPU_UART0_HS	1		/*  handshake:	no(0)	*/
						/*		crts(1)	*/
#define	CFG_NIOS_CPU_UART0_EOP	0		/*  eop reg:	no(0)	*/
						/*		yes(1)	*/

#define	CFG_NIOS_CPU_UART1	0x00000820	/* UART1	addr	*/
#define	CFG_NIOS_CPU_UART1_IRQ	18		/*		IRQ	*/
#define	CFG_NIOS_CPU_UART1_BR	115200		/*  baudrate	var(0)	*/
#define	CFG_NIOS_CPU_UART1_DB	8		/*  data bit		*/
#define	CFG_NIOS_CPU_UART1_SB	1		/*  stop bit		*/
#define	CFG_NIOS_CPU_UART1_PA	0		/*  parity	none(0)	*/
						/*		odd(1)	*/
						/*		even(2)	*/
#define	CFG_NIOS_CPU_UART1_HS	0		/*  handshake:	no(0)	*/
						/*		crts(1)	*/
#define	CFG_NIOS_CPU_UART1_EOP	0		/*  eop reg:	no(0)	*/
						/*		yes(1)	*/

/* serial peripheral i/o */
#define	CFG_NIOS_CPU_SPI_NUMS	1		/* number of spis	*/

#define	CFG_NIOS_CPU_SPI0	0x000008c0	/* SPI0		addr	*/
#define	CFG_NIOS_CPU_SPI0_IRQ	25		/*		IRQ	*/
#define	CFG_NIOS_CPU_SPI0_BITS	16		/*  data bit		*/
#define	CFG_NIOS_CPU_SPI0_MA	1		/*  is master:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_SPI0_SLN	1		/*  num slaves		*/
#define	CFG_NIOS_CPU_SPI0_TCLK	250000		/*  clock (Hz)		*/
#define	CFG_NIOS_CPU_SPI0_TDELAY 2		/*  delay (usec)	*/
#define	CFG_NIOS_CPU_SPI0_FB	0		/*  first bit	msb(0)	*/
						/*		lsb(1)	*/

/* parallel i/o */
#define	CFG_NIOS_CPU_PIO_NUMS	14		/* number of parports	*/

#define	CFG_NIOS_CPU_PIO0	0x00000860	/* PIO0		addr	*/
#undef	CFG_NIOS_CPU_PIO0_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO0_BITS	8		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO0_TYPE	0		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO0_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO0_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO0_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO1	0x00000870	/* PIO1		addr	*/
#undef	CFG_NIOS_CPU_PIO1_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO1_BITS	8		/*  number  of  bits	*/
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

#define	CFG_NIOS_CPU_PIO2	0x00000880	/* PIO2		addr	*/
#undef	CFG_NIOS_CPU_PIO2_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO2_BITS	4		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO2_TYPE	0		/*  io type:	tris(0)	*/
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

#define	CFG_NIOS_CPU_PIO3	0x00000890	/* PIO3		addr	*/
#undef	CFG_NIOS_CPU_PIO3_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO3_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO3_TYPE	2		/*  io type:	tris(0)	*/
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

#define	CFG_NIOS_CPU_PIO3	0x00000890	/* PIO3		addr	*/
#undef	CFG_NIOS_CPU_PIO3_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO3_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO3_TYPE	2		/*  io type:	tris(0)	*/
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

#define	CFG_NIOS_CPU_PIO4	0x000008a0	/* PIO4		addr	*/
#undef	CFG_NIOS_CPU_PIO4_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO4_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO4_TYPE	1		/*  io type:	tris(0)	*/
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

#define	CFG_NIOS_CPU_PIO5	0x000008b0	/* PIO5		addr	*/
#undef	CFG_NIOS_CPU_PIO5_IRQ			/*		w/o IRQ	*/
#define	CFG_NIOS_CPU_PIO5_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO5_TYPE	1		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO5_CAP	0		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO5_EDGE	0		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO5_ITYPE	0		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO6	0x00000900	/* PIO6		addr	*/
#define	CFG_NIOS_CPU_PIO6_IRQ	20		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO6_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO6_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO6_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO6_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO6_ITYPE	1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO7	0x00000910	/* PIO7		addr	*/
#define	CFG_NIOS_CPU_PIO7_IRQ	31		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO7_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO7_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO7_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO7_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO7_ITYPE	1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO8	0x00000920	/* PIO8		addr	*/
#define	CFG_NIOS_CPU_PIO8_IRQ	32		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO8_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO8_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO8_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO8_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO8_ITYPE	1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO9	0x00000930	/* PIO9		addr	*/
#define	CFG_NIOS_CPU_PIO9_IRQ	33		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO9_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO9_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO9_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO9_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO9_ITYPE	1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO10	0x00000940	/* PIO10	addr	*/
#define	CFG_NIOS_CPU_PIO10_IRQ	34		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO10_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO10_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO10_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO10_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO10_ITYPE 1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO11	0x00000950	/* PIO11	addr	*/
#define	CFG_NIOS_CPU_PIO11_IRQ	35		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO11_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO11_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO11_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO11_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO11_ITYPE 1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO12	0x00000960	/* PIO12	addr	*/
#define	CFG_NIOS_CPU_PIO12_IRQ	36		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO12_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO12_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO12_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO12_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO12_ITYPE 1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

#define	CFG_NIOS_CPU_PIO13	0x00000970	/* PIO113	addr	*/
#define	CFG_NIOS_CPU_PIO13_IRQ	37		/*		IRQ	*/
#define	CFG_NIOS_CPU_PIO13_BITS	1		/*  number  of  bits	*/
#define	CFG_NIOS_CPU_PIO13_TYPE	2		/*  io type:	tris(0)	*/
						/*		out(1)	*/
						/*		in(2)	*/
#define	CFG_NIOS_CPU_PIO13_CAP	1		/*  capture:	no(0)	*/
						/*		yes(1)	*/
#define	CFG_NIOS_CPU_PIO13_EDGE	2		/*  edge type:	none(0)	*/
						/*		fall(1)	*/
						/*		rise(2)	*/
						/*		any(3)	*/
#define	CFG_NIOS_CPU_PIO13_ITYPE 1		/*  IRQ type:	none(0)	*/
						/*		level(1)*/
						/*		edge(2)	*/

/* IDE i/f */
#define	CFG_NIOS_CPU_IDE_NUMS	2		/* number of IDE contr.	*/

#define	CFG_NIOS_CPU_IDE0	0x00001000	/* IDE0		addr	*/
#define	CFG_NIOS_CPU_IDE0_IRQ	36		/*		IRQ	*/

#define	CFG_NIOS_CPU_IDE1	0x00001020	/* IDE1		addr	*/
#define	CFG_NIOS_CPU_IDE1_IRQ	37		/*		IRQ	*/

/* memory accessibility */
#undef	CFG_NIOS_CPU_SRAM_BASE			/* board SRAM	addr	*/
#undef	CFG_NIOS_CPU_SRAM_SIZE			/*  1 MB	size	*/

#define	CFG_NIOS_CPU_SDRAM_BASE	0x02000000	/* board SDRAM	addr	*/
#define	CFG_NIOS_CPU_SDRAM_SIZE	(16*1024*1024)	/* 16 MB	size	*/

#define	CFG_NIOS_CPU_FLASH_BASE	0x01000000	/* board Flash	addr	*/
#define	CFG_NIOS_CPU_FLASH_SIZE	(8*1024*1024)	/*  8 MB	size	*/

/* LAN */
#define	CFG_NIOS_CPU_LAN_NUMS	1		/* number of LAN i/f	*/

#define	CFG_NIOS_CPU_LAN0_BASE	0x00010000	/* LAN0		addr	*/
#define	CFG_NIOS_CPU_LAN0_OFFS	(0)		/*		offset	*/
#define	CFG_NIOS_CPU_LAN0_IRQ	20		/*		IRQ	*/
#define	CFG_NIOS_CPU_LAN0_BUSW	16		/*	        buswidth*/
#define	CFG_NIOS_CPU_LAN0_TYPE	0		/*	smc91111(0)	*/
						/*	cs8900(1)	*/
						/* ex:	openmac(2)	*/
						/* ex:	alteramac(3)	*/

/* external extension */
#define	CFG_NIOS_CPU_CS0_BASE	0x40000000	/* board EXT0	addr	*/
#define	CFG_NIOS_CPU_CS0_SIZE	(16*1024*1024)	/*  max. 16 MB	size	*/

#define	CFG_NIOS_CPU_CS1_BASE	0x41000000	/* board EXT1	addr	*/
#define	CFG_NIOS_CPU_CS1_SIZE	(16*1024*1024)	/*  max. 16 MB	size	*/

#define	CFG_NIOS_CPU_CS2_BASE	0x42000000	/* board EXT2	addr	*/
#define	CFG_NIOS_CPU_CS2_SIZE	(16*1024*1024)	/*  max. 16 MB	size	*/

#define	CFG_NIOS_CPU_CS3_BASE	0x43000000	/* board EXT3	addr	*/
#define	CFG_NIOS_CPU_CS3_SIZE	(16*1024*1024)	/*  max. 16 MB	size	*/

/* symbolic redefinition (undef, if not present) */
#define	CFG_NIOS_CPU_TICK_TIMER		0	/* TIMER0: tick (needed)*/
#undef	CFG_NIOS_CPU_USER_TIMER			/* TIMERx: users choice	*/

#define	CFG_NIOS_CPU_PORTA_PIO		0	/* PIO0: Port A		*/
#define	CFG_NIOS_CPU_PORTB_PIO		1	/* PIO1: Port D		*/
#define	CFG_NIOS_CPU_PORTC_PIO		2	/* PIO2: Port C		*/
#define	CFG_NIOS_CPU_RCM_PIO		3	/* PIO3: RCM jumper	*/
#define	CFG_NIOS_CPU_WDENA_PIO		4	/* PIO4: watchdog enable*/
#define	CFG_NIOS_CPU_WDTOG_PIO		5	/* PIO5: watchdog trigg.*/

/* PIOx: LED bar */
#ifdef	CONFIG_DNPEVA2			/* DNP/EVA2 base board */
#define	CFG_NIOS_CPU_LED_PIO		CFG_NIOS_CPU_PORTA_PIO
#else
#undef	CFG_NIOS_CPU_LED_PIO			/* no LED bar		*/
#endif

#endif	/* __CONFIG_ADNPESC1_BASE_32_H */
