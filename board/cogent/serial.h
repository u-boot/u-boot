/* Line Status Register bits */
#define LSR_DR		0x01	/* Data ready */
#define LSR_OE		0x02	/* Overrun */
#define LSR_PE		0x04	/* Parity error */
#define LSR_FE		0x08	/* Framing error */
#define LSR_BI		0x10	/* Break */
#define LSR_THRE	0x20	/* Xmit holding register empty */
#define LSR_TEMT	0x40	/* Xmitter empty */
#define LSR_ERR		0x80	/* Error */

#define CLKRATE		3686400	/* cogent motherboard serial clk = 3.6864MHz */
#define DEFDIV		1	/* default to 230400 bps */

#define br_to_div(br)	(CLKRATE / (16 * (br)))
#define div_to_br(div)	(CLKRATE / (16 * (div)))
