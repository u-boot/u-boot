/*
 * NS16550 Serial Port
 * originally from linux source (arch/ppc/boot/ns16550.h)
 * modified slightly to
 * have addresses as offsets from CFG_ISA_BASE
 * added a few more definitions
 * added prototypes for ns16550.c
 * reduced no of com ports to 2
 * modifications (c) Rob Taylor, Flying Pig Systems. 2000.
 */

#if (CFG_NS16550_REG_SIZE == 1)
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
#elif (CFG_NS16550_REG_SIZE == 2)
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
#elif (CFG_NS16550_REG_SIZE == 4)
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
#elif (CFG_NS16550_REG_SIZE == -4)
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
#else
#error "Please define NS16550 registers size."
#endif

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

typedef volatile struct NS16550 *NS16550_t;

#define FCR_FIFO_EN     0x01		/* Fifo enable */
#define FCR_RXSR        0x02		/* Receiver soft reset */
#define FCR_TXSR        0x04		/* Transmitter soft reset */

#define MCR_DTR         0x01
#define MCR_RTS         0x02
#define MCR_DMA_EN      0x04
#define MCR_TX_DFR      0x08

#define LCR_WLS_MSK	0x03		/* character length select mask */
#define LCR_WLS_5	0x00		/* 5 bit character length */
#define LCR_WLS_6	0x01		/* 6 bit character length */
#define LCR_WLS_7	0x02		/* 7 bit character length */
#define LCR_WLS_8	0x03		/* 8 bit character length */
#define LCR_STB		0x04		/* Number of stop Bits, off = 1, on = 1.5 or 2) */
#define LCR_PEN		0x08		/* Parity eneble */
#define LCR_EPS		0x10		/* Even Parity Select */
#define LCR_STKP	0x20		/* Stick Parity */
#define LCR_SBRK	0x40		/* Set Break */
#define LCR_BKSE	0x80		/* Bank select enable */

#define LSR_DR		0x01		/* Data ready */
#define LSR_OE		0x02		/* Overrun */
#define LSR_PE		0x04		/* Parity error */
#define LSR_FE		0x08		/* Framing error */
#define LSR_BI		0x10		/* Break */
#define LSR_THRE	0x20		/* Xmit holding register empty */
#define LSR_TEMT	0x40		/* Xmitter empty */
#define LSR_ERR		0x80		/* Error */

#ifdef CONFIG_OMAP1510
#define OSC_12M_SEL	0x01		/* selects 6.5 * current clk div */
#endif

/* useful defaults for LCR */
#define LCR_8N1		0x03

void	NS16550_init   (NS16550_t com_port, int baud_divisor);
void	NS16550_putc   (NS16550_t com_port, char c);
char	NS16550_getc   (NS16550_t com_port);
int	NS16550_tstc   (NS16550_t com_port);
void	NS16550_reinit (NS16550_t com_port, int baud_divisor);
