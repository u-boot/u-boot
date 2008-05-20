/*
 *  linux/include/asm-arm/hardware/clps7111.h
 *
 *  This file contains the hardware definitions of the CLPS7111 internal
 *  registers.
 *
 *  Copyright (C) 2000 Deep Blue Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_HARDWARE_CLPS7111_H
#define __ASM_HARDWARE_CLPS7111_H

#define CLPS7111_PHYS_BASE	(0x80000000)

#ifndef __ASSEMBLY__
#define clps_readb(off)		__raw_readb(CLPS7111_BASE + (off))
#define clps_readl(off)		__raw_readl(CLPS7111_BASE + (off))
#define clps_writeb(val,off)	__raw_writeb(val, CLPS7111_BASE + (off))
#define clps_writel(val,off)	__raw_writel(val, CLPS7111_BASE + (off))
#endif

#define PADR		(0x0000)
#define PBDR		(0x0001)
#define PDDR		(0x0003)
#define PADDR		(0x0040)
#define PBDDR		(0x0041)
#define PDDDR		(0x0043)
#define PEDR		(0x0080)
#define PEDDR		(0x00c0)
#define SYSCON1		(0x0100)
#define SYSFLG1		(0x0140)
#define MEMCFG1		(0x0180)
#define MEMCFG2		(0x01c0)
#define DRFPR		(0x0200)
#define INTSR1		(0x0240)
#define INTMR1		(0x0280)
#define LCDCON		(0x02c0)
#define TC1D		(0x0300)
#define TC2D		(0x0340)
#define RTCDR		(0x0380)
#define RTCMR		(0x03c0)
#define PMPCON		(0x0400)
#define CODR		(0x0440)
#define UARTDR1		(0x0480)
#define UBRLCR1		(0x04c0)
#define SYNCIO		(0x0500)
#define PALLSW		(0x0540)
#define PALMSW		(0x0580)
#define STFCLR		(0x05c0)
#define BLEOI		(0x0600)
#define MCEOI		(0x0640)
#define TEOI		(0x0680)
#define TC1EOI		(0x06c0)
#define TC2EOI		(0x0700)
#define RTCEOI		(0x0740)
#define UMSEOI		(0x0780)
#define COEOI		(0x07c0)
#define HALT		(0x0800)
#define STDBY		(0x0840)

#define FBADDR		(0x1000)
#define SYSCON2		(0x1100)
#define SYSFLG2		(0x1140)
#define INTSR2		(0x1240)
#define INTMR2		(0x1280)
#define UARTDR2		(0x1480)
#define UBRLCR2		(0x14c0)
#define SS2DR		(0x1500)
#define SRXEOF		(0x1600)
#define SS2POP		(0x16c0)
#define KBDEOI		(0x1700)

/* common bits: SYSCON1 / SYSCON2 */
#define SYSCON_UARTEN		(1 << 8)

#define SYSCON1_KBDSCAN(x)	((x) & 15)
#define SYSCON1_KBDSCANMASK	(15)
#define SYSCON1_TC1M		(1 << 4)
#define SYSCON1_TC1S		(1 << 5)
#define SYSCON1_TC2M		(1 << 6)
#define SYSCON1_TC2S		(1 << 7)
#define SYSCON1_UART1EN		SYSCON_UARTEN
#define SYSCON1_BZTOG		(1 << 9)
#define SYSCON1_BZMOD		(1 << 10)
#define SYSCON1_DBGEN		(1 << 11)
#define SYSCON1_LCDEN		(1 << 12)
#define SYSCON1_CDENTX		(1 << 13)
#define SYSCON1_CDENRX		(1 << 14)
#define SYSCON1_SIREN		(1 << 15)
#define SYSCON1_ADCKSEL(x)	(((x) & 3) << 16)
#define SYSCON1_ADCKSEL_MASK	(3 << 16)
#define SYSCON1_EXCKEN		(1 << 18)
#define SYSCON1_WAKEDIS		(1 << 19)
#define SYSCON1_IRTXM		(1 << 20)

/* common bits: SYSFLG1 / SYSFLG2 */
#define SYSFLG_UBUSY		(1 << 11)
#define SYSFLG_URXFE		(1 << 22)
#define SYSFLG_UTXFF		(1 << 23)

#define SYSFLG1_MCDR		(1 << 0)
#define SYSFLG1_DCDET		(1 << 1)
#define SYSFLG1_WUDR		(1 << 2)
#define SYSFLG1_WUON		(1 << 3)
#define SYSFLG1_CTS		(1 << 8)
#define SYSFLG1_DSR		(1 << 9)
#define SYSFLG1_DCD		(1 << 10)
#define SYSFLG1_UBUSY		SYSFLG_UBUSY
#define SYSFLG1_NBFLG		(1 << 12)
#define SYSFLG1_RSTFLG		(1 << 13)
#define SYSFLG1_PFFLG		(1 << 14)
#define SYSFLG1_CLDFLG		(1 << 15)
#define SYSFLG1_URXFE		SYSFLG_URXFE
#define SYSFLG1_UTXFF		SYSFLG_UTXFF
#define SYSFLG1_CRXFE		(1 << 24)
#define SYSFLG1_CTXFF		(1 << 25)
#define SYSFLG1_SSIBUSY		(1 << 26)
#define SYSFLG1_ID		(1 << 29)

#define SYSFLG2_SSRXOF		(1 << 0)
#define SYSFLG2_RESVAL		(1 << 1)
#define SYSFLG2_RESFRM		(1 << 2)
#define SYSFLG2_SS2RXFE		(1 << 3)
#define SYSFLG2_SS2TXFF		(1 << 4)
#define SYSFLG2_SS2TXUF		(1 << 5)
#define SYSFLG2_CKMODE		(1 << 6)
#define SYSFLG2_UBUSY		SYSFLG_UBUSY
#define SYSFLG2_URXFE		SYSFLG_URXFE
#define SYSFLG2_UTXFF		SYSFLG_UTXFF

#define LCDCON_GSEN		(1 << 30)
#define LCDCON_GSMD		(1 << 31)

#define SYSCON2_SERSEL		(1 << 0)
#define SYSCON2_KBD6		(1 << 1)
#define SYSCON2_DRAMZ		(1 << 2)
#define SYSCON2_KBWEN		(1 << 3)
#define SYSCON2_SS2TXEN		(1 << 4)
#define SYSCON2_PCCARD1		(1 << 5)
#define SYSCON2_PCCARD2		(1 << 6)
#define SYSCON2_SS2RXEN		(1 << 7)
#define SYSCON2_UART2EN		SYSCON_UARTEN
#define SYSCON2_SS2MAEN		(1 << 9)
#define SYSCON2_OSTB		(1 << 12)
#define SYSCON2_CLKENSL		(1 << 13)
#define SYSCON2_BUZFREQ		(1 << 14)

/* common bits: UARTDR1 / UARTDR2 */
#define UARTDR_FRMERR		(1 << 8)
#define UARTDR_PARERR		(1 << 9)
#define UARTDR_OVERR		(1 << 10)

/* common bits: UBRLCR1 / UBRLCR2 */
#define UBRLCR_BAUD_MASK	((1 << 12) - 1)
#define UBRLCR_BREAK		(1 << 12)
#define UBRLCR_PRTEN		(1 << 13)
#define UBRLCR_EVENPRT		(1 << 14)
#define UBRLCR_XSTOP		(1 << 15)
#define UBRLCR_FIFOEN		(1 << 16)
#define UBRLCR_WRDLEN5		(0 << 17)
#define UBRLCR_WRDLEN6		(1 << 17)
#define UBRLCR_WRDLEN7		(2 << 17)
#define UBRLCR_WRDLEN8		(3 << 17)
#define UBRLCR_WRDLEN_MASK	(3 << 17)

#define SYNCIO_SMCKEN		(1 << 13)
#define SYNCIO_TXFRMEN		(1 << 14)

#define SYSCON3 0x2200  /* System Control register 3 ----------------------- */
#define ADCCON  0x00000001  /* ADC configuration */
#define CLKCTL  0x00000006  /* processor clock control */
#define CLKCTL_18      0x0  /* 18.432 MHz */
#define CLKCTL_36      0x2  /* 36.864 MHz */
#define CLKCTL_49      0x4  /* 49.152 MHz */
#define CLKCTL_73      0x6  /* 73.728 MHz */
#define MCPSEL  0x00000008  /* MCP select */
#define ADCCKNSEN 0x000010  /* ADC clock sense */
#define VERSN   0x000000e0  /* additional version bits */
#define VERSN_SHIFT     5
#define FASTWAKE 0x0000100  /* Wakeup clock select: 0=8Hz, 1=4kHz */

#define INTSR3  0x2240  /* Interrupt Status register 3 --------------------- */
#define MCPINT  0x00000001  /* MCP interface interrupt (FIQ) */

#define INTMR3  0x2280  /* Interrupt Mask register 3 ----------------------- */
#define LEDFLSH 0x22C0  /* LED Flash control register ---------------------- */
#define LEDFLSH_RATE       0x03  /* flash rate */
#define LEDFLSH_RATE_SHIFT 0
#define LEDFLSH_DUTY       0x3c  /* duty ratio */
#define LEDFLSH_DUTY_SHIFT 2
#define LEDFLSH_ENABLE     0x40  /* enable */

#define IO_START	CLPS7111_PHYS_BASE

#define IO(offset)	(IO_START + (offset))

#define IO_BYTE(offset)	(*(volatile unsigned char *)(IO_START + (offset)))
#define IO_WORD(offset)	(*(volatile unsigned long *)(IO_START + (offset)))

#define IO_PADR		IO_BYTE(PADR)
#define IO_PBDR		IO_BYTE(PBDR)
#define IO_PDDR		IO_BYTE(PDDR)
#define IO_PADDR	IO_BYTE(PADDR)
#define IO_PBDDR	IO_BYTE(PBDDR)
#define IO_PDDDR	IO_BYTE(PDDDR)
#define IO_PEDR		IO_BYTE(PEDR)
#define IO_PEDDR	IO_BYTE(PEDDR)
#define IO_SYSCON	IO_WORD(SYSCON)
#define	IO_SYSFLG	IO_WORD(SYSFLG)
#define	IO_MEMCFG1	IO_WORD(MEMCFG1)
#define	IO_MEMCFG2	IO_WORD(MEMCFG2)
#define IO_DRFPR	IO_WORD(DRFPR)
#define IO_INTSR	IO_WORD(INTSR)
#define IO_INTMR	IO_WORD(INTMR)
#define	IO_LCDCON	IO_WORD(LCDCON)
#define IO_TC1D		IO_WORD(TC1D)
#define IO_TC2D		IO_WORD(TC2D)
#define IO_RTCDR	IO_WORD(RTCDR)
#define IO_RTCMR	IO_WORD(RTCMR)
#define IO_PMPCON	IO_WORD(PMPCON)
#define IO_CODR		IO_BYTE(CODR)
#define IO_UARTDR	IO_WORD(UARTDR)
#define IO_UBRLCR	IO_WORD(UBRLCR)
#define IO_SYNCIO	IO_WORD(SYNCIO)
#define	IO_PALLSW	IO_WORD(PALLSW)
#define	IO_PALMSW	IO_WORD(PALMSW)
#define IO_STFCLR	IO_WORD(STFCLR)
#define IO_BLEOI	IO_WORD(BLEOI)
#define IO_MCEOI	IO_WORD(MCEOI)
#define IO_TEOI		IO_WORD(TEOI)
#define IO_TC1EOI	IO_WORD(TC1EOI)
#define IO_TC2EOI	IO_WORD(TC2EOI)
#define IO_RTCEOI	IO_WORD(RTCEOI)
#define IO_UMSEOI	IO_WORD(UMSEOI)
#define IO_COEOI	IO_WORD(COEOI)
#define IO_HALT		IO_WORD(HALT)
#define IO_STDBY	IO_WORD(STDBY)
#define IO_SYSCON1	IO_WORD(SYSCON1)
#define IO_SYSFLG1	IO_WORD(SYSFLG1)
#define IO_INTSR1	IO_WORD(INTSR1)
#define IO_INTMR1	IO_WORD(INTMR1)
#define IO_UARTDR1	IO_WORD(UARTDR1)
#define IO_UBRLCR1	IO_WORD(UBRLCR1)
#define IO_FRBADDR	IO_WORD(FRBADDR)
#define IO_SYSCON2	IO_WORD(SYSCON2)
#define IO_SYSFLG2	IO_WORD(SYSFLG2)
#define IO_INTSR2	IO_WORD(INTSR2)
#define IO_INTMR2	IO_WORD(INTMR2)
#define IO_UARTDR2	IO_WORD(UARTDR2)
#define IO_UBRLCR2	IO_WORD(UBRLCR2)
#define IO_KBDEOI	IO_WORD(KBDEOI)

#define IO_MCCR		IO_WORD(MCCR)
#define IO_MCDR0	IO_WORD(MCDR0)
#define IO_MCDR1	IO_WORD(MCDR1)
#define IO_MCDR2	IO_WORD(MCDR2)
#define IO_MCSR		IO_WORD(MCSR)
#define IO_SYSCON3	IO_WORD(SYSCON3)
#define IO_INTSR3	IO_WORD(INTSR3)
#define IO_INTMR3	IO_WORD(INTMR3)
#define IO_LEDFLSH	IO_WORD(LEDFLSH)

#endif /* __ASM_HARDWARE_CLPS7111_H */
