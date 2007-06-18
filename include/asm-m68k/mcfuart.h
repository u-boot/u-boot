/*
 * mcfuart.h -- ColdFire internal UART support defines.
 *
 * File copied from mcfuart.h of uCLinux distribution:
 *      (C) Copyright 1999, Greg Ungerer (gerg@snapgear.com)
 *      (C) Copyright 2000, Lineo Inc. (www.lineo.com)
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

/****************************************************************************/
#ifndef	mcfuart_h
#define	mcfuart_h
/****************************************************************************/

#include <linux/config.h>

/*
 *	Define the base address of the UARTS within the MBAR address
 *	space.
 */
#if defined(CONFIG_M5272)
#define	MCFUART_BASE1		0x100	/* Base address of UART1 */
#define	MCFUART_BASE2		0x140	/* Base address of UART2 */
#elif defined(CONFIG_M5204) || defined(CONFIG_M5206) || defined(CONFIG_M5206e)
#if defined(CONFIG_NETtel)
#define	MCFUART_BASE1		0x180	/* Base address of UART1 */
#define	MCFUART_BASE2		0x140	/* Base address of UART2 */
#else
#define	MCFUART_BASE1		0x140	/* Base address of UART1 */
#define	MCFUART_BASE2		0x180	/* Base address of UART2 */
#endif
#elif defined(CONFIG_M5282) || defined(CONFIG_M5271)
#define MCFUART_BASE1		0x200	/* Base address of UART1 */
#define MCFUART_BASE2		0x240	/* Base address of UART2 */
#define MCFUART_BASE3		0x280	/* Base address of UART3 */
#elif defined(CONFIG_M5249) || defined(CONFIG_M5307) || defined(CONFIG_M5407)
#if defined(CONFIG_NETtel) || defined(CONFIG_DISKtel) || defined(CONFIG_SECUREEDGEMP3)
#define MCFUART_BASE1		0x200	/* Base address of UART1 */
#define MCFUART_BASE2		0x1c0	/* Base address of UART2 */
#else
#define MCFUART_BASE1		0x1c0	/* Base address of UART1 */
#define MCFUART_BASE2		0x200	/* Base address of UART2 */
#endif
#endif

/*
 *	Define the ColdFire UART register set addresses.
 */
#define	MCFUART_UMR		0x00	/* Mode register (r/w) */
#define	MCFUART_USR		0x04	/* Status register (r) */
#define	MCFUART_UCSR		0x04	/* Clock Select (w) */
#define	MCFUART_UCR		0x08	/* Command register (w) */
#define	MCFUART_URB		0x0c	/* Receiver Buffer (r) */
#define	MCFUART_UTB		0x0c	/* Transmit Buffer (w) */
#define	MCFUART_UIPCR		0x10	/* Input Port Change (r) */
#define	MCFUART_UACR		0x10	/* Auxiliary Control (w) */
#define	MCFUART_UISR		0x14	/* Interrup Status (r) */
#define	MCFUART_UIMR		0x14	/* Interrupt Mask (w) */
#define	MCFUART_UBG1		0x18	/* Baud Rate MSB (r/w) */
#define	MCFUART_UBG2		0x1c	/* Baud Rate LSB (r/w) */
#ifdef	CONFIG_M5272
#define	MCFUART_UTF		0x28	/* Transmitter FIFO (r/w) */
#define	MCFUART_URF		0x2c	/* Receiver FIFO (r/w) */
#define	MCFUART_UFPD		0x30	/* Frac Prec. Divider (r/w) */
#else
#define	MCFUART_UIVR		0x30	/* Interrupt Vector (r/w) */
#endif
#define	MCFUART_UIPR		0x34	/* Input Port (r) */
#define	MCFUART_UOP1		0x38	/* Output Port Bit Set (w) */
#define	MCFUART_UOP0		0x3c	/* Output Port Bit Reset (w) */

#ifdef	CONFIG_M5249
/* Note: This isn't in the 5249 docs */
#define	MCFUART_UFPD		0x30	/* Frac Prec. Divider (r/w) */
#endif

/*
 *	Define bit flags in Mode Register 1 (MR1).
 */
#define	MCFUART_MR1_RXRTS	0x80	/* Auto RTS flow control */
#define	MCFUART_MR1_RXIRQFULL	0x40	/* RX IRQ type FULL */
#define	MCFUART_MR1_RXIRQRDY	0x00	/* RX IRQ type RDY */
#define	MCFUART_MR1_RXERRBLOCK	0x20	/* RX block error mode */
#define	MCFUART_MR1_RXERRCHAR	0x00	/* RX char error mode */

#define	MCFUART_MR1_PARITYNONE	0x10	/* No parity */
#define	MCFUART_MR1_PARITYEVEN	0x00	/* Even parity */
#define	MCFUART_MR1_PARITYODD	0x04	/* Odd parity */
#define	MCFUART_MR1_PARITYSPACE	0x08	/* Space parity */
#define	MCFUART_MR1_PARITYMARK	0x0c	/* Mark parity */

#define	MCFUART_MR1_CS5		0x00	/* 5 bits per char */
#define	MCFUART_MR1_CS6		0x01	/* 6 bits per char */
#define	MCFUART_MR1_CS7		0x02	/* 7 bits per char */
#define	MCFUART_MR1_CS8		0x03	/* 8 bits per char */

/*
 *	Define bit flags in Mode Register 2 (MR2).
 */
#define	MCFUART_MR2_LOOPBACK	0x80	/* Loopback mode */
#define	MCFUART_MR2_REMOTELOOP	0xc0	/* Remote loopback mode */
#define	MCFUART_MR2_AUTOECHO	0x40	/* Automatic echo */
#define	MCFUART_MR2_TXRTS	0x20	/* Assert RTS on TX */
#define	MCFUART_MR2_TXCTS	0x10	/* Auto CTS flow control */

#define	MCFUART_MR2_STOP1	0x07	/* 1 stop bit */
#define	MCFUART_MR2_STOP15	0x08	/* 1.5 stop bits */
#define	MCFUART_MR2_STOP2	0x0f	/* 2 stop bits */

/*
 *	Define bit flags in Status Register (USR).
 */
#define	MCFUART_USR_RXBREAK	0x80	/* Received BREAK */
#define	MCFUART_USR_RXFRAMING	0x40	/* Received framing error */
#define	MCFUART_USR_RXPARITY	0x20	/* Received parity error */
#define	MCFUART_USR_RXOVERRUN	0x10	/* Received overrun error */
#define	MCFUART_USR_TXEMPTY	0x08	/* Transmitter empty */
#define	MCFUART_USR_TXREADY	0x04	/* Transmitter ready */
#define	MCFUART_USR_RXFULL	0x02	/* Receiver full */
#define	MCFUART_USR_RXREADY	0x01	/* Receiver ready */

#define	MCFUART_USR_RXERR	(MCFUART_USR_RXBREAK | MCFUART_USR_RXFRAMING | \
				MCFUART_USR_RXPARITY | MCFUART_USR_RXOVERRUN)

/*
 *	Define bit flags in Clock Select Register (UCSR).
 */
#define	MCFUART_UCSR_RXCLKTIMER	0xd0	/* RX clock is timer */
#define	MCFUART_UCSR_RXCLKEXT16	0xe0	/* RX clock is external x16 */
#define	MCFUART_UCSR_RXCLKEXT1	0xf0	/* RX clock is external x1 */

#define	MCFUART_UCSR_TXCLKTIMER	0x0d	/* TX clock is timer */
#define	MCFUART_UCSR_TXCLKEXT16	0x0e	/* TX clock is external x16 */
#define	MCFUART_UCSR_TXCLKEXT1	0x0f	/* TX clock is external x1 */

/*
 *	Define bit flags in Command Register (UCR).
 */
#define	MCFUART_UCR_CMDNULL		0x00	/* No command */
#define	MCFUART_UCR_CMDRESETMRPTR	0x10	/* Reset MR pointer */
#define	MCFUART_UCR_CMDRESETRX		0x20	/* Reset receiver */
#define	MCFUART_UCR_CMDRESETTX		0x30	/* Reset transmitter */
#define	MCFUART_UCR_CMDRESETERR		0x40	/* Reset error status */
#define	MCFUART_UCR_CMDRESETBREAK	0x50	/* Reset BREAK change */
#define	MCFUART_UCR_CMDBREAKSTART	0x60	/* Start BREAK */
#define	MCFUART_UCR_CMDBREAKSTOP	0x70	/* Stop BREAK */

#define	MCFUART_UCR_TXNULL	0x00	/* No TX command */
#define	MCFUART_UCR_TXENABLE	0x04	/* Enable TX */
#define	MCFUART_UCR_TXDISABLE	0x08	/* Disable TX */
#define	MCFUART_UCR_RXNULL	0x00	/* No RX command */
#define	MCFUART_UCR_RXENABLE	0x01	/* Enable RX */
#define	MCFUART_UCR_RXDISABLE	0x02	/* Disable RX */

/*
 *	Define bit flags in Input Port Change Register (UIPCR).
 */
#define	MCFUART_UIPCR_CTSCOS	0x10	/* CTS change of state */
#define	MCFUART_UIPCR_CTS	0x01	/* CTS value */

/*
 *	Define bit flags in Input Port Register (UIP).
 */
#define	MCFUART_UIPR_CTS	0x01	/* CTS value */

/*
 *	Define bit flags in Output Port Registers (UOP).
 *	Clear bit by writing to UOP0, set by writing to UOP1.
 */
#define	MCFUART_UOP_RTS		0x01	/* RTS set or clear */

/*
 *	Define bit flags in the Auxiliary Control Register (UACR).
 */
#define	MCFUART_UACR_IEC	0x01	/* Input enable control */

/*
 *	Define bit flags in Interrupt Status Register (UISR).
 *	These same bits are used for the Interrupt Mask Register (UIMR).
 */
#define	MCFUART_UIR_COS		0x80	/* Change of state (CTS) */
#define	MCFUART_UIR_DELTABREAK	0x04	/* Break start or stop */
#define	MCFUART_UIR_RXREADY	0x02	/* Receiver ready */
#define	MCFUART_UIR_TXREADY	0x01	/* Transmitter ready */

#ifdef	CONFIG_M5272
/*
 *	Define bit flags in the Transmitter FIFO Register (UTF).
 */
#define	MCFUART_UTF_TXB		0x1f	/* transmitter data level */
#define	MCFUART_UTF_FULL	0x20	/* transmitter fifo full */
#define	MCFUART_UTF_TXS		0xc0	/* transmitter status */

/*
 *	Define bit flags in the Receiver FIFO Register (URF).
 */
#define	MCFUART_URF_RXB		0x1f	/* receiver data level */
#define	MCFUART_URF_FULL	0x20	/* receiver fifo full */
#define	MCFUART_URF_RXS		0xc0	/* receiver status */
#endif

#ifdef	CONFIG_MCFUART
/* UART module registers */
/* Register read/write struct */
typedef struct uart {
	u8 umr;			/* 0x00 Mode Register */
	u8 resv0[0x3];
	union {
		u8 usr;		/* 0x04 Status Register */
		u8 ucsr;	/* 0x04 Clock Select Register */
	};
	u8 resv1[0x3];
	u8 ucr;			/* 0x08 Command Register */
	u8 resv2[0x3];
	union {
		u8 utb;		/* 0x0c Transmit Buffer */
		u8 urb;		/* 0x0c Receive Buffer */
	};
	u8 resv3[0x3];
	union {
		u8 uipcr;	/* 0x10 Input Port Change Register */
		u8 uacr;	/* 0x10 Auxiliary Control reg */
	};
	u8 resv4[0x3];
	union {
		u8 uimr;	/* 0x14 Interrupt Mask reg */
		u8 uisr;	/* 0x14 Interrupt Status reg */
	};
	u8 resv5[0x3];
	u8 ubg1;		/* 0x18 Counter Timer Upper Register */
	u8 resv6[0x3];
	u8 ubg2;		/* 0x1c Counter Timer Lower Register */
	u8 resv7[0x17];
	u8 uip;			/* 0x34 Input Port Register */
	u8 resv8[0x3];
	u8 uop1;		/* 0x38 Output Port Set Register */
	u8 resv9[0x3];
	u8 uop0;		/* 0x3c Output Port Reset Register */
} uart_t;

/*********************************************************************
* Universal Asynchronous Receiver Transmitter (UART)
*********************************************************************/
/* Bit definitions and macros for UMR */
#define UART_UMR_BC(x)			(((x)&0x03))
#define UART_UMR_PT			(0x04)
#define UART_UMR_PM(x)			(((x)&0x03)<<3)
#define UART_UMR_ERR			(0x20)
#define UART_UMR_RXIRQ			(0x40)
#define UART_UMR_RXRTS			(0x80)
#define UART_UMR_SB(x)			(((x)&0x0F))
#define UART_UMR_TXCTS			(0x10)	/* Trsnsmit CTS */
#define UART_UMR_TXRTS			(0x20)	/* Transmit RTS */
#define UART_UMR_CM(x)			(((x)&0x03)<<6)	/* CM bits */
#define UART_UMR_PM_MULTI_ADDR		(0x1C)
#define UART_UMR_PM_MULTI_DATA		(0x18)
#define UART_UMR_PM_NONE		(0x10)
#define UART_UMR_PM_FORCE_HI		(0x0C)
#define UART_UMR_PM_FORCE_LO		(0x08)
#define UART_UMR_PM_ODD			(0x04)
#define UART_UMR_PM_EVEN		(0x00)
#define UART_UMR_BC_5			(0x00)
#define UART_UMR_BC_6			(0x01)
#define UART_UMR_BC_7			(0x02)
#define UART_UMR_BC_8			(0x03)
#define UART_UMR_CM_NORMAL		(0x00)
#define UART_UMR_CM_ECH			(0x40)
#define UART_UMR_CM_LOCAL_LOOP		(0x80)
#define UART_UMR_CM_REMOTE_LOOP		(0xC0)
#define UART_UMR_SB_STOP_BITS_1		(0x07)
#define UART_UMR_SB_STOP_BITS_15	(0x08)
#define UART_UMR_SB_STOP_BITS_2		(0x0F)

/* Bit definitions and macros for USR */
#define UART_USR_RXRDY			(0x01)
#define UART_USR_FFULL			(0x02)
#define UART_USR_TXRDY			(0x04)
#define UART_USR_TXEMP			(0x08)
#define UART_USR_OE			(0x10)
#define UART_USR_PE			(0x20)
#define UART_USR_FE			(0x40)
#define UART_USR_RB			(0x80)

/* Bit definitions and macros for UCSR */
#define UART_UCSR_TCS(x)		(((x)&0x0F))
#define UART_UCSR_RCS(x)		(((x)&0x0F)<<4)
#define UART_UCSR_RCS_SYS_CLK		(0xD0)
#define UART_UCSR_RCS_CTM16		(0xE0)
#define UART_UCSR_RCS_CTM		(0xF0)
#define UART_UCSR_TCS_SYS_CLK		(0x0D)
#define UART_UCSR_TCS_CTM16		(0x0E)
#define UART_UCSR_TCS_CTM		(0x0F)

/* Bit definitions and macros for UCR */
#define UART_UCR_RXC(x)			(((x)&0x03))
#define UART_UCR_TXC(x)			(((x)&0x03)<<2)
#define UART_UCR_MISC(x)		(((x)&0x07)<<4)
#define UART_UCR_NONE			(0x00)
#define UART_UCR_STOP_BREAK		(0x70)
#define UART_UCR_START_BREAK		(0x60)
#define UART_UCR_BKCHGINT		(0x50)
#define UART_UCR_RESET_ERROR		(0x40)
#define UART_UCR_RESET_TX		(0x30)
#define UART_UCR_RESET_RX		(0x20)
#define UART_UCR_RESET_MR		(0x10)
#define UART_UCR_TX_DISABLED		(0x08)
#define UART_UCR_TX_ENABLED		(0x04)
#define UART_UCR_RX_DISABLED		(0x02)
#define UART_UCR_RX_ENABLED		(0x01)

/* Bit definitions and macros for UIPCR */
#define UART_UIPCR_CTS			(0x01)
#define UART_UIPCR_COS			(0x10)

/* Bit definitions and macros for UACR */
#define UART_UACR_IEC			(0x01)

/* Bit definitions and macros for UIMR */
#define UART_UIMR_TXRDY			(0x01)
#define UART_UIMR_RXRDY_FU		(0x02)
#define UART_UIMR_DB			(0x04)
#define UART_UIMR_COS			(0x80)

/* Bit definitions and macros for UISR */
#define UART_UISR_TXRDY			(0x01)
#define UART_UISR_RXRDY_FU		(0x02)
#define UART_UISR_DB			(0x04)
#define UART_UISR_RXFTO			(0x08)
#define UART_UISR_TXFIFO		(0x10)
#define UART_UISR_RXFIFO		(0x20)
#define UART_UISR_COS			(0x80)

/* Bit definitions and macros for UIP */
#define UART_UIP_CTS			(0x01)

/* Bit definitions and macros for UOP1 */
#define UART_UOP1_RTS			(0x01)

/* Bit definitions and macros for UOP0 */
#define UART_UOP0_RTS			(0x01)
#endif				/* CONFIG_MCFUART */

/****************************************************************************/
#endif				/* mcfuart_h */
