/*
 * NS16550 Serial Port
 * originally from linux source (arch/ppc/boot/ns16550.h)
 * modified slightly to
 * have addresses as offsets from CONFIG_SYS_ISA_BASE
 * added a few more definitions
 * added prototypes for ns16550.c
 * reduced no of com ports to 2
 * modifications (c) Rob Taylor, Flying Pig Systems. 2000.
 *
 * further modified to support the DUART in the Galileo eval board
 * modifications (c) Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 */

#ifndef __NS16550_H__
#define __NS16550_H__

/* the padding is necessary because on the galileo board the UART is
   wired in with the 3 address lines shifted over by 2 bits */
struct NS16550
{
	unsigned char rbr;  /* 0 = 0-3*/
	int pad1:24;

	unsigned char ier;  /* 1 = 4-7*/
	int pad2:24;

	unsigned char fcr;  /* 2 = 8-b*/
	int pad3:24;

	unsigned char lcr;  /* 3 = c-f*/
	int pad4:24;

	unsigned char mcr;  /* 4 = 10-13*/
	int pad5:24;

	unsigned char lsr;  /* 5 = 14-17*/
	int pad6:24;

	unsigned char msr;  /* 6 =18-1b*/
	int pad7:24;

	unsigned char scr;  /* 7 =1c-1f*/
	int pad8:24;
} __attribute__ ((packed));

/* aliases */
#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

#define FCR_FIFO_EN     0x01    /*fifo enable*/
#define FCR_RXSR        0x02    /*reciever soft reset*/
#define FCR_TXSR        0x04    /*transmitter soft reset*/


#define MCR_DTR         0x01
#define MCR_RTS         0x02
#define MCR_DMA_EN      0x04
#define MCR_TX_DFR      0x08


#define LCR_WLS_MSK 0x03    /* character length slect mask*/
#define LCR_WLS_5   0x00    /* 5 bit character length */
#define LCR_WLS_6   0x01    /* 6 bit character length */
#define LCR_WLS_7   0x02    /* 7 bit character length */
#define LCR_WLS_8   0x03    /* 8 bit character length */
#define LCR_STB     0x04    /* Number of stop Bits, off = 1, on = 1.5 or 2) */
#define LCR_PEN     0x08    /* Parity eneble*/
#define LCR_EPS     0x10    /* Even Parity Select*/
#define LCR_STKP    0x20    /* Stick Parity*/
#define LCR_SBRK    0x40    /* Set Break*/
#define LCR_BKSE    0x80    /* Bank select enable*/

#define LSR_DR      0x01    /* Data ready */
#define LSR_OE      0x02    /* Overrun */
#define LSR_PE      0x04    /* Parity error */
#define LSR_FE      0x08    /* Framing error */
#define LSR_BI      0x10    /* Break */
#define LSR_THRE    0x20    /* Xmit holding register empty */
#define LSR_TEMT    0x40    /* Xmitter empty */
#define LSR_ERR     0x80    /* Error */

/* useful defaults for LCR*/
#define LCR_8N1     0x03


#define COM1 0x03F8
#define COM2 0x02F8

volatile struct NS16550 * NS16550_init(int chan, int baud_divisor);
void NS16550_putc(volatile struct NS16550 *com_port, unsigned char c);
unsigned char NS16550_getc(volatile struct NS16550 *com_port);
int NS16550_tstc(volatile struct NS16550 *com_port);
void NS16550_reinit(volatile struct NS16550 *com_port, int baud_divisor);

typedef struct NS16550 *NS16550_t;

extern const NS16550_t COM_PORTS[];

#endif
