/*
 * NS16550 Serial Port
 * originally from linux source (arch/ppc/boot/ns16550.h)
 *
 * Cleanup and unification
 * (C) 2009 by Detlev Zundel, DENX Software Engineering GmbH
 *
 * modified slightly to
 * have addresses as offsets from CONFIG_SYS_ISA_BASE
 * added a few more definitions
 * added prototypes for ns16550.c
 * reduced no of com ports to 2
 * modifications (c) Rob Taylor, Flying Pig Systems. 2000.
 *
 * added support for port on 64-bit bus
 * by Richard Danter (richard.danter@windriver.com), (C) 2005 Wind River Systems
 */

#if (CONFIG_SYS_NS16550_REG_SIZE == 1)
struct NS16550 {
	unsigned char rbr;		/* 0 */
	unsigned char ier;		/* 1 */
	unsigned char fcr;		/* 2 */
	unsigned char lcr;		/* 3 */
	unsigned char mcr;		/* 4 */
	unsigned char lsr;		/* 5 */
	unsigned char msr;		/* 6 */
	unsigned char scr;		/* 7 */
#if defined(CONFIG_OMAP730)
	unsigned char mdr1;		/* 8 */
	unsigned char reg9;		/* 9 */
	unsigned char regA;		/* A */
	unsigned char regB;		/* B */
	unsigned char regC;		/* C */
	unsigned char regD;		/* D */
	unsigned char regE;		/* E */
	unsigned char regF;		/* F */
	unsigned char reg10;		/* 10 */
	unsigned char ssr;		/* 11*/
#endif
} __attribute__ ((packed));
#elif (CONFIG_SYS_NS16550_REG_SIZE == 2)
struct NS16550 {
	unsigned short rbr;		/* 0 */
	unsigned short ier;		/* 1 */
	unsigned short fcr;		/* 2 */
	unsigned short lcr;		/* 3 */
	unsigned short mcr;		/* 4 */
	unsigned short lsr;		/* 5 */
	unsigned short msr;		/* 6 */
	unsigned short scr;		/* 7 */
} __attribute__ ((packed));
#elif (CONFIG_SYS_NS16550_REG_SIZE == 4)
struct NS16550 {
	unsigned long rbr;		/* 0 r  */
	unsigned long ier;		/* 1 rw */
	unsigned long fcr;		/* 2 w  */
	unsigned long lcr;		/* 3 rw */
	unsigned long mcr;		/* 4 rw */
	unsigned long lsr;		/* 5 r  */
	unsigned long msr;		/* 6 r  */
	unsigned long scr;		/* 7 rw */
}; /* No need to pack an already aligned struct */
#elif (CONFIG_SYS_NS16550_REG_SIZE == -4)
struct NS16550 {
	unsigned char rbr;		/* 0 */
	int pad1:24;
	unsigned char ier;		/* 1 */
	int pad2:24;
	unsigned char fcr;		/* 2 */
	int pad3:24;
	unsigned char lcr;		/* 3 */
	int pad4:24;
	unsigned char mcr;		/* 4 */
	int pad5:24;
	unsigned char lsr;		/* 5 */
	int pad6:24;
	unsigned char msr;		/* 6 */
	int pad7:24;
	unsigned char scr;		/* 7 */
	int pad8:24;
#if defined(CONFIG_OMAP)
	unsigned char mdr1;		/* mode select reset TL16C750*/
#endif
#ifdef CONFIG_OMAP1510
	int pad9:24;
	unsigned long pad[10];
	unsigned char osc_12m_sel;
	int pad10:24;
#endif
} __attribute__ ((packed));
#elif (CONFIG_SYS_NS16550_REG_SIZE == -8)
struct NS16550 {
	unsigned char rbr;		/* 0 */
	unsigned char pad0[7];
	unsigned char ier;		/* 1 */
	unsigned char pad1[7];
	unsigned char fcr;		/* 2 */
	unsigned char pad2[7];
	unsigned char lcr;		/* 3 */
	unsigned char pad3[7];
	unsigned char mcr;		/* 4 */
	unsigned char pad4[7];
	unsigned char lsr;		/* 5 */
	unsigned char pad5[7];
	unsigned char msr;		/* 6 */
	unsigned char pad6[7];
	unsigned char scr;		/* 7 */
	unsigned char pad7[7];
} __attribute__ ((packed));
#else
#error "Please define NS16550 registers size."
#endif

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

typedef volatile struct NS16550 *NS16550_t;

/*
 * These are the definitions for the FIFO Control Register
 */
#define UART_FCR_FIFO_EN 	0x01 /* Fifo enable */
#define UART_FCR_CLEAR_RCVR	0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04 /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT	0x08 /* For DMA applications */
#define UART_FCR_TRIGGER_MASK	0xC0 /* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1	0x00 /* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4	0x40 /* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8	0x80 /* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14	0xC0 /* Mask for trigger set at 14 */

#define UART_FCR_RXSR		0x02 /* Receiver soft reset */
#define UART_FCR_TXSR		0x04 /* Transmitter soft reset */

/*
 * These are the definitions for the Modem Control Register
 */
#define UART_MCR_DTR	0x01		/* DTR   */
#define UART_MCR_RTS	0x02		/* RTS   */
#define UART_MCR_OUT1	0x04		/* Out 1 */
#define UART_MCR_OUT2	0x08		/* Out 2 */
#define UART_MCR_LOOP	0x10		/* Enable loopback test mode */

#define UART_MCR_DMA_EN	0x04
#define UART_MCR_TX_DFR	0x08

/*
 * These are the definitions for the Line Control Register
 *
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
 * UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */
#define UART_LCR_WLS_MSK 0x03		/* character length select mask */
#define UART_LCR_WLS_5	0x00		/* 5 bit character length */
#define UART_LCR_WLS_6	0x01		/* 6 bit character length */
#define UART_LCR_WLS_7	0x02		/* 7 bit character length */
#define UART_LCR_WLS_8	0x03		/* 8 bit character length */
#define UART_LCR_STB	0x04		/* Number of stop Bits, off = 1, on = 1.5 or 2) */
#define UART_LCR_PEN	0x08		/* Parity eneble */
#define UART_LCR_EPS	0x10		/* Even Parity Select */
#define UART_LCR_STKP	0x20		/* Stick Parity */
#define UART_LCR_SBRK	0x40		/* Set Break */
#define UART_LCR_BKSE	0x80		/* Bank select enable */
#define UART_LCR_DLAB	0x80		/* Divisor latch access bit */

/*
 * These are the definitions for the Line Status Register
 */
#define UART_LSR_DR	0x01		/* Data ready */
#define UART_LSR_OE	0x02		/* Overrun */
#define UART_LSR_PE	0x04		/* Parity error */
#define UART_LSR_FE	0x08		/* Framing error */
#define UART_LSR_BI	0x10		/* Break */
#define UART_LSR_THRE	0x20		/* Xmit holding register empty */
#define UART_LSR_TEMT	0x40		/* Xmitter empty */
#define UART_LSR_ERR	0x80		/* Error */

#define UART_MSR_DCD	0x80		/* Data Carrier Detect */
#define UART_MSR_RI	0x40		/* Ring Indicator */
#define UART_MSR_DSR	0x20		/* Data Set Ready */
#define UART_MSR_CTS	0x10		/* Clear to Send */
#define UART_MSR_DDCD	0x08		/* Delta DCD */
#define UART_MSR_TERI	0x04		/* Trailing edge ring indicator */
#define UART_MSR_DDSR	0x02		/* Delta DSR */
#define UART_MSR_DCTS	0x01		/* Delta CTS */

/*
 * These are the definitions for the Interrupt Identification Register
 */
#define UART_IIR_NO_INT	0x01	/* No interrupts pending */
#define UART_IIR_ID	0x06	/* Mask for the interrupt ID */

#define UART_IIR_MSI	0x00	/* Modem status interrupt */
#define UART_IIR_THRI	0x02	/* Transmitter holding register empty */
#define UART_IIR_RDI	0x04	/* Receiver data interrupt */
#define UART_IIR_RLSI	0x06	/* Receiver line status interrupt */

/*
 * These are the definitions for the Interrupt Enable Register
 */
#define UART_IER_MSI	0x08	/* Enable Modem status interrupt */
#define UART_IER_RLSI	0x04	/* Enable receiver line status interrupt */
#define UART_IER_THRI	0x02	/* Enable Transmitter holding register int. */
#define UART_IER_RDI	0x01	/* Enable receiver data interrupt */


#ifdef CONFIG_OMAP1510
#define OSC_12M_SEL	0x01	/* selects 6.5 * current clk div */
#endif

/* useful defaults for LCR */
#define UART_LCR_8N1	0x03

void	NS16550_init   (NS16550_t com_port, int baud_divisor);
void	NS16550_putc   (NS16550_t com_port, char c);
char	NS16550_getc   (NS16550_t com_port);
int	NS16550_tstc   (NS16550_t com_port);
void	NS16550_reinit (NS16550_t com_port, int baud_divisor);
