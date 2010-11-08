/*
 * (C) Copyright 2006, Imagos S.a.s <www.imagos.it>
 * Renato Andreola <renato.andreola@imagos.it>
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

/*************************************************************************
 * Altera NiosII YANU serial interface by Imagos
 * please see  http://www.opencores.org/project,yanu for
 * information/downloads
 ************************************************************************/

#ifndef __NIOS2_YANU_H__
#define __NIOS2_YANU_H__

#define YANU_MAX_PRESCALER_N   ((1 << 4) - 1)	/* 15 */
#define YANU_MAX_PRESCALER_M   ((1 << 11) -1)	/* 2047 */
#define YANU_FIFO_SIZE         (16)
#define YANU_RXFIFO_SIZE       (YANU_FIFO_SIZE)
#define YANU_TXFIFO_SIZE       (YANU_FIFO_SIZE)

#define YANU_RXFIFO_DLY        (10*11)
#define YANU_TXFIFO_THR        (10)
#define YANU_DATA_CHAR_MASK    (0xFF)

/* data register */
#define YANU_DATA_OFFSET       (0)	/* data register offset */

#define YANU_CONTROL_OFFSET    (4)	/* control register offset */
/* interrupt enable */
#define YANU_CONTROL_IE_RRDY   (1<<0)	/* ie on received character ready */
#define YANU_CONTROL_IE_OE     (1<<1)	/* ie on rx overrun    */
#define YANU_CONTROL_IE_BRK    (1<<2)	/* ie on break detect  */
#define YANU_CONTROL_IE_FE     (1<<3)	/* ie on framing error */
#define YANU_CONTROL_IE_PE     (1<<4)	/* ie on parity error  */
#define YANU_CONTROL_IE_TRDY   (1<<5)	/* ie interrupt on tranmitter ready */
/* control bits */
#define YANU_CONTROL_BITS_POS  (6)	/* bits number pos */
#define YANU_CONTROL_BITS      (1<<YANU_CONTROL_BITS_POS)	/* number of rx/tx bits per word. 3 bit unsigned integer */
#define YANU_CONTROL_BITS_N    (3)	/* ... its bit filed length */
#define YANU_CONTROL_PARENA    (1<<9)	/*  enable parity bit transmission/reception */
#define YANU_CONTROL_PAREVEN   (1<<10)	/* parity even */
#define YANU_CONTROL_STOPS     (1<<11)	/* number of stop bits */
#define YANU_CONTROL_HHENA     (1<<12)	/* Harware Handshake enable... */
#define YANU_CONTROL_FORCEBRK  (1<<13)	/* if set than txd = active (0) */
/* tuning part */
#define YANU_CONTROL_RDYDLY    (1<<14)	/* delay from "first" before setting rrdy (in bit) */
#define YANU_CONTROL_RDYDLY_N  (8)	/* ... its bit filed length */
#define YANU_CONTROL_TXTHR     (1<<22)	/* tx interrupt threshold: the trdy set if txfifo_chars<= txthr (chars) */
#define YANU_CONTROL_TXTHR_N   (4)	/* ... its bit field length */

#define YANU_BAUD_OFFSET  (8)	/* baud register offset */
#define YANU_BAUDM        (1<<0)	/* baud mantissa lsb */
#define YANU_BAUDM_N      (12)	/* ...its bit filed length */
#define YANU_BAUDE        (1<<12)	/* baud exponent lsb */
#define YANU_BAUDE_N      (4)	/* ...its bit field length */

#define YANU_ACTION_OFFSET   (12)	/* action register... write only */
#define YANU_ACTION_RRRDY    (1<<0)	/* reset rrdy */
#define YANU_ACTION_ROE      (1<<1)	/* reset oe */
#define YANU_ACTION_RBRK     (1<<2)	/* reset brk */
#define YANU_ACTION_RFE      (1<<3)	/* reset fe  */
#define YANU_ACTION_RPE      (1<<4)	/* reset pe  */
#define YANU_ACTION_SRRDY    (1<<5)	/* set rrdy  */
#define YANU_ACTION_SOE      (1<<6)	/* set oe    */
#define YANU_ACTION_SBRK     (1<<7)	/* set brk   */
#define YANU_ACTION_SFE      (1<<8)	/* set fe    */
#define YANU_ACTION_SPE      (1<<9)	/* set pe    */
#define YANU_ACTION_RFIFO_PULL  (1<<10)	/* pull a char from rx fifo we MUST do it before taking a char */
#define YANU_ACTION_RFIFO_CLEAR (1<<11)	/* clear rx fifo */
#define YANU_ACTION_TFIFO_CLEAR (1<<12)	/* clear tx fifo */
#define YANU_ACTION_RTRDY       (1<<13)	/* clear trdy    */
#define YANU_ACTION_STRDY       (1<<14)	/* set trdy      */

#define YANU_STATUS_OFFSET   (16)
#define YANU_STATUS_RRDY     (1<<0)	/* rxrdy flag */
#define YANU_STATUS_TRDY     (1<<1)	/* txrdy flag */
#define YANU_STATUS_OE       (1<<2)	/* rx overrun error */
#define YANU_STATUS_BRK      (1<<3)	/* rx break detect flag */
#define YANU_STATUS_FE       (1<<4)	/* rx framing error flag */
#define YANU_STATUS_PE       (1<<5)	/* rx parity erro flag */
#define YANU_RFIFO_CHARS_POS (6)
#define YANU_RFIFO_CHARS     (1<<RFIFO_CHAR_POS)	/* number of chars into rx fifo */
#define YANU_RFIFO_CHARS_N   (5)	/* ...its bit field length: 32 chars */
#define YANU_TFIFO_CHARS_POS (11)
#define YANU_TFIFO_CHARS     (1<<TFIFO_CHAR_POS)	/* number of chars into tx fifo */
#define YANU_TFIFO_CHARS_N   (5)	/* ...its bit field length: 32 chars */

typedef volatile struct yanu_uart_t {
	volatile unsigned data;
	volatile unsigned control;	/* control register (RW) 32-bit   */
	volatile unsigned baud;	/* baud/prescaler register (RW) 32-bit */
	volatile unsigned action;	/* action register (W) 32-bit */
	volatile unsigned status;	/* status register (R) 32-bit */
	volatile unsigned magic;	/* magic register (R) 32-bit */
} yanu_uart_t;

#endif
