/*
 * linux/include/asm-arm/arch-netarm/netarm_ser_module.h
 *
 * Copyright (C) 2000 NETsilicon, Inc.
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This software is copyrighted by Red Hat. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall Red Hat
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 *             Clark Williams
 */

#ifndef __NETARM_SER_MODULE_REGISTERS_H
#define __NETARM_SER_MODULE_REGISTERS_H

#ifndef	__ASSEMBLER__

/* (--sub)#include "types.h" */

/* serial channel control structure */
typedef struct {
  u32	ctrl_a;
  u32	ctrl_b;
  u32	status_a;
  u32	bitrate;
  u32	fifo;
  u32	rx_buf_timer;
  u32	rx_char_timer;
  u32	rx_match;
  u32	rx_match_mask;
  u32	ctrl_c;
  u32	status_b;
  u32	status_c;
  u32	fifo_last;
  u32	unused[3];
} netarm_serial_channel_t;

#endif

/* SER unit register offsets */

/* #ifdef CONFIG_ARCH_NETARM */
#define	NETARM_SER_MODULE_BASE		(0xFFD00000)
/* #else */
/* extern serial_channel_t netarm_dummy_registers[]; */
/* #define NETARM_SER_MODULE_BASE		(netarm_dummy_registers) */
/* #ifndef NETARM_XTAL_FREQ */
/* #define NETARM_XTAL_FREQ                18432000 */
/* #endif */
/* #endif */

/* calculate the sysclk value from the pll setting */
#define	NETARM_PLLED_SYSCLK_FREQ	(( NETARM_XTAL_FREQ / 5 ) * \
					 ( NETARM_PLL_COUNT_VAL + 3 ))

#define get_serial_channel(c) (&(((netarm_serial_channel_t *)NETARM_SER_MODULE_BASE)[c]))

#define	NETARM_SER_CH1_CTRL_A		(0x00)
#define	NETARM_SER_CH1_CTRL_B		(0x04)
#define	NETARM_SER_CH1_STATUS_A		(0x08)
#define	NETARM_SER_CH1_BITRATE		(0x0C)
#define	NETARM_SER_CH1_FIFO		(0x10)
#define	NETARM_SER_CH1_RX_BUF_TMR	(0x14)
#define	NETARM_SER_CH1_RX_CHAR_TMR	(0x18)
#define	NETARM_SER_CH1_RX_MATCH		(0x1c)
#define	NETARM_SER_CH1_RX_MATCH_MASK	(0x20)
#define	NETARM_SER_CH1_CTRL_C		(0x24)
#define	NETARM_SER_CH1_STATUS_B		(0x28)
#define	NETARM_SER_CH1_STATUS_C		(0x2c)
#define	NETARM_SER_CH1_FIFO_LAST	(0x30)

#define	NETARM_SER_CH2_CTRL_A		(0x40)
#define	NETARM_SER_CH2_CTRL_B		(0x44)
#define	NETARM_SER_CH2_STATUS_A		(0x48)
#define	NETARM_SER_CH2_BITRATE		(0x4C)
#define	NETARM_SER_CH2_FIFO		(0x50)
#define	NETARM_SER_CH2_RX_BUF_TMR	(0x54)
#define	NETARM_SER_CH2_RX_CHAR_TMR	(0x58)
#define	NETARM_SER_CH2_RX_MATCH		(0x5c)
#define	NETARM_SER_CH2_RX_MATCH_MASK	(0x60)
#define	NETARM_SER_CH2_CTRL_C		(0x64)
#define	NETARM_SER_CH2_STATUS_B		(0x68)
#define	NETARM_SER_CH2_STATUS_C		(0x6c)
#define	NETARM_SER_CH2_FIFO_LAST	(0x70)

/* select bitfield defintions */

/* Control Register A */

#define	NETARM_SER_CTLA_ENABLE		(0x80000000)
#define	NETARM_SER_CTLA_BRK		(0x40000000)

#define	NETARM_SER_CTLA_STICKP		(0x20000000)

#define	NETARM_SER_CTLA_P_EVEN		(0x18000000)
#define	NETARM_SER_CTLA_P_ODD		(0x08000000)
#define	NETARM_SER_CTLA_P_NONE		(0x00000000)

/* if you read the errata, you will find that the STOP bits don't work right */
#define	NETARM_SER_CTLA_2STOP		(0x00000000)
#define	NETARM_SER_CTLA_3STOP		(0x04000000)

#define	NETARM_SER_CTLA_5BITS		(0x00000000)
#define	NETARM_SER_CTLA_6BITS		(0x01000000)
#define	NETARM_SER_CTLA_7BITS		(0x02000000)
#define	NETARM_SER_CTLA_8BITS		(0x03000000)

#define	NETARM_SER_CTLA_CTSTX		(0x00800000)
#define	NETARM_SER_CTLA_RTSRX		(0x00400000)

#define	NETARM_SER_CTLA_LOOP_REM	(0x00200000)
#define	NETARM_SER_CTLA_LOOP_LOC	(0x00100000)

#define	NETARM_SER_CTLA_GPIO2		(0x00080000)
#define	NETARM_SER_CTLA_GPIO1		(0x00040000)

#define	NETARM_SER_CTLA_DTR_EN		(0x00020000)
#define	NETARM_SER_CTLA_RTS_EN		(0x00010000)

#define	NETARM_SER_CTLA_IE_RX_BRK	(0x00008000)
#define	NETARM_SER_CTLA_IE_RX_FRMERR	(0x00004000)
#define	NETARM_SER_CTLA_IE_RX_PARERR	(0x00002000)
#define	NETARM_SER_CTLA_IE_RX_OVERRUN	(0x00001000)
#define	NETARM_SER_CTLA_IE_RX_RDY	(0x00000800)
#define	NETARM_SER_CTLA_IE_RX_HALF	(0x00000400)
#define	NETARM_SER_CTLA_IE_RX_FULL	(0x00000200)
#define	NETARM_SER_CTLA_IE_RX_DMAEN	(0x00000100)
#define	NETARM_SER_CTLA_IE_RX_DCD	(0x00000080)
#define	NETARM_SER_CTLA_IE_RX_RI	(0x00000040)
#define	NETARM_SER_CTLA_IE_RX_DSR	(0x00000020)

#define NETARM_SER_CTLA_IE_RX_ALL	(NETARM_SER_CTLA_IE_RX_BRK \
					|NETARM_SER_CTLA_IE_RX_FRMERR \
					|NETARM_SER_CTLA_IE_RX_PARERR \
					|NETARM_SER_CTLA_IE_RX_OVERRUN \
					|NETARM_SER_CTLA_IE_RX_RDY \
					|NETARM_SER_CTLA_IE_RX_HALF \
					|NETARM_SER_CTLA_IE_RX_FULL \
					|NETARM_SER_CTLA_IE_RX_DMAEN \
					|NETARM_SER_CTLA_IE_RX_DCD \
					|NETARM_SER_CTLA_IE_RX_RI \
					|NETARM_SER_CTLA_IE_RX_DSR)

#define	NETARM_SER_CTLA_IE_TX_CTS	(0x00000010)
#define	NETARM_SER_CTLA_IE_TX_EMPTY	(0x00000008)
#define	NETARM_SER_CTLA_IE_TX_HALF	(0x00000004)
#define	NETARM_SER_CTLA_IE_TX_FULL	(0x00000002)
#define	NETARM_SER_CTLA_IE_TX_DMAEN	(0x00000001)

#define NETARM_SER_CTLA_IE_TX_ALL	(NETARM_SER_CTLA_IE_TX_CTS \
					|NETARM_SER_CTLA_IE_TX_EMPTY \
					|NETARM_SER_CTLA_IE_TX_HALF \
					|NETARM_SER_CTLA_IE_TX_FULL \
					|NETARM_SER_CTLA_IE_TX_DMAEN)

/* Control Register B */

#define	NETARM_SER_CTLB_MATCH1_EN	(0x80000000)
#define	NETARM_SER_CTLB_MATCH2_EN	(0x40000000)
#define	NETARM_SER_CTLB_MATCH3_EN	(0x20000000)
#define	NETARM_SER_CTLB_MATCH4_EN	(0x10000000)

#define	NETARM_SER_CTLB_RBGT_EN		(0x08000000)
#define	NETARM_SER_CTLB_RCGT_EN		(0x04000000)

#define	NETARM_SER_CTLB_UART_MODE	(0x00000000)
#define	NETARM_SER_CTLB_HDLC_MODE	(0x00100000)
#define	NETARM_SER_CTLB_SPI_MAS_MODE	(0x00200000)
#define	NETARM_SER_CTLB_SPI_SLV_MODE	(0x00300000)

#define	NETARM_SER_CTLB_REV_BIT_ORDER	(0x00080000)

#define	NETARM_SER_CTLB_MAM1		(0x00040000)
#define	NETARM_SER_CTLB_MAM2		(0x00020000)

/* Status Register A */

#define	NETARM_SER_STATA_MATCH1		(0x80000000)
#define	NETARM_SER_STATA_MATCH2		(0x40000000)
#define	NETARM_SER_STATA_MATCH3		(0x20000000)
#define	NETARM_SER_STATA_MATCH4		(0x10000000)

#define	NETARM_SER_STATA_BGAP		(0x80000000)
#define	NETARM_SER_STATA_CGAP		(0x40000000)

#define	NETARM_SER_STATA_RX_1B		(0x00100000)
#define	NETARM_SER_STATA_RX_2B		(0x00200000)
#define	NETARM_SER_STATA_RX_3B		(0x00300000)
#define	NETARM_SER_STATA_RX_4B		(0x00000000)

/* downshifted values */

#define	NETARM_SER_STATA_RXFDB_1BYTES	(0x001)
#define	NETARM_SER_STATA_RXFDB_2BYTES	(0x002)
#define	NETARM_SER_STATA_RXFDB_3BYTES	(0x003)
#define	NETARM_SER_STATA_RXFDB_4BYTES	(0x000)

#define	NETARM_SER_STATA_RXFDB_MASK	(0x00300000)
#define	NETARM_SER_STATA_RXFDB(x)	(((x) & NETARM_SER_STATA_RXFDB_MASK) \
					 >> 20)

#define	NETARM_SER_STATA_DCD		(0x00080000)
#define	NETARM_SER_STATA_RI		(0x00040000)
#define	NETARM_SER_STATA_DSR		(0x00020000)
#define	NETARM_SER_STATA_CTS		(0x00010000)

#define	NETARM_SER_STATA_RX_BRK		(0x00008000)
#define	NETARM_SER_STATA_RX_FRMERR	(0x00004000)
#define	NETARM_SER_STATA_RX_PARERR	(0x00002000)
#define	NETARM_SER_STATA_RX_OVERRUN	(0x00001000)
#define	NETARM_SER_STATA_RX_RDY		(0x00000800)
#define	NETARM_SER_STATA_RX_HALF	(0x00000400)
#define	NETARM_SER_STATA_RX_CLOSED	(0x00000200)
#define	NETARM_SER_STATA_RX_FULL	(0x00000100)
#define	NETARM_SER_STATA_RX_DCD		(0x00000080)
#define	NETARM_SER_STATA_RX_RI		(0x00000040)
#define	NETARM_SER_STATA_RX_DSR		(0x00000020)

#define	NETARM_SER_STATA_TX_CTS		(0x00000010)
#define	NETARM_SER_STATA_TX_RDY		(0x00000008)
#define	NETARM_SER_STATA_TX_HALF	(0x00000004)
#define	NETARM_SER_STATA_TX_FULL	(0x00000002)
#define	NETARM_SER_STATA_TX_DMAEN	(0x00000001)

/* you have to clear all receive signals to get the fifo to move forward */
#define NETARM_SER_STATA_CLR_ALL	(NETARM_SER_STATA_RX_BRK | \
					 NETARM_SER_STATA_RX_FRMERR | \
					 NETARM_SER_STATA_RX_PARERR | \
					 NETARM_SER_STATA_RX_OVERRUN | \
					 NETARM_SER_STATA_RX_HALF | \
					 NETARM_SER_STATA_RX_CLOSED | \
					 NETARM_SER_STATA_RX_FULL | \
					 NETARM_SER_STATA_RX_DCD | \
					 NETARM_SER_STATA_RX_RI | \
					 NETARM_SER_STATA_RX_DSR | \
					 NETARM_SER_STATA_TX_CTS )

/* Bit Rate Registers */

#define	NETARM_SER_BR_EN		(0x80000000)
#define	NETARM_SER_BR_TMODE		(0x40000000)

#define	NETARM_SER_BR_RX_CLK_INT	(0x00000000)
#define	NETARM_SER_BR_RX_CLK_EXT	(0x20000000)
#define	NETARM_SER_BR_TX_CLK_INT	(0x00000000)
#define	NETARM_SER_BR_TX_CLK_EXT	(0x10000000)

#define	NETARM_SER_BR_RX_CLK_DRV	(0x08000000)
#define	NETARM_SER_BR_TX_CLK_DRV	(0x04000000)

#define	NETARM_SER_BR_CLK_EXT_5		(0x00000000)
#define	NETARM_SER_BR_CLK_SYSTEM	(0x01000000)
#define	NETARM_SER_BR_CLK_OUT1A		(0x02000000)
#define	NETARM_SER_BR_CLK_OUT2A		(0x03000000)

#define	NETARM_SER_BR_TX_CLK_INV	(0x00800000)
#define	NETARM_SER_BR_RX_CLK_INV	(0x00400000)

/* complete settings assuming system clock input is 18MHz */

#define	NETARM_SER_BR_MASK		(0x000007FF)

/* bit rate determined from equation Fbr = Fxtal / [ 10 * ( N + 1 ) ] */
/* from section 7.5.4 of HW Ref Guide */

/* #ifdef CONFIG_NETARM_PLL_BYPASS */
#define	NETARM_SER_BR_X16(x)	( NETARM_SER_BR_EN | 			\
				  NETARM_SER_BR_RX_CLK_INT | 		\
				  NETARM_SER_BR_TX_CLK_INT | 		\
				  NETARM_SER_BR_CLK_EXT_5 | 		\
				  ( ( ( ( NETARM_XTAL_FREQ / 		\
				          ( x * 10 ) ) - 1 ) /	16 ) & 	\
				    NETARM_SER_BR_MASK ) )
/*
#else
#define	NETARM_SER_BR_X16(x)	( NETARM_SER_BR_EN | 			\
				  NETARM_SER_BR_RX_CLK_INT | 		\
				  NETARM_SER_BR_TX_CLK_INT | 		\
				  NETARM_SER_BR_CLK_SYSTEM | 		\
				  ( ( ( ( NETARM_PLLED_SYSCLK_FREQ / 		\
				          ( x * 2 ) ) - 1 ) /	16 ) & 	\
				    NETARM_SER_BR_MASK ) )
#endif
*/

/* Receive Buffer Gap Timer */

#define	NETARM_SER_RX_GAP_TIMER_EN	(0x80000000)
#define	NETARM_SER_RX_GAP_MASK		(0x00003FFF)

/* rx gap is a function of bit rate x */

/* #ifdef CONFIG_NETARM_PLL_BYPASS */
#define	NETARM_SER_RXGAP(x)	( NETARM_SER_RX_GAP_TIMER_EN |		\
				  ( ( ( ( 10 * NETARM_XTAL_FREQ ) /	\
				        ( x * 5 * 512 ) ) - 1 ) & 	\
			              NETARM_SER_RX_GAP_MASK ) )
/*
#else
#define	NETARM_SER_RXGAP(x)	( NETARM_SER_RX_GAP_TIMER_EN |			\
				  ( ( ( ( 2 * NETARM_PLLED_SYSCLK_FREQ ) /	\
				        ( x * 512 ) ) - 1 ) & 			\
			              NETARM_SER_RX_GAP_MASK ) )
#endif
*/

#if 0
#define	NETARM_SER_RXGAP(x)	( NETARM_SER_RX_GAP_TIMER_EN |		\
				  ( ( ( ( 2 * NETARM_PLLED_SYSCLK_FREQ ) /	\
				        ( x * 5 * 512 ) ) - 1 ) & 	\
			              NETARM_SER_RX_GAP_MASK ) )
#define	NETARM_SER_RXGAP(x)	( NETARM_SER_RX_GAP_TIMER_EN |		\
				  ( ( ( ( 10 * NETARM_XTAL_FREQ ) /	\
				        ( x * 512 ) ) - 1 ) & 	\
			              NETARM_SER_RX_GAP_MASK ) )
#endif

#define MIN_BAUD_RATE        600
#define MAX_BAUD_RATE     115200

/* the default BAUD rate for the BOOTLOADER, there is a separate */
/* setting in the serial driver <arch/armnommu/drivers/char/serial-netarm.h> */
#define DEFAULT_BAUD_RATE 9600
#define NETARM_SER_FIFO_SIZE 32
#define MIN_GAP 0

#endif
