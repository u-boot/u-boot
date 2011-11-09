/*
 * Adapted from:
 * df_arm/trunk/ep3_1/sw/standalone/drivers/uart_v1_00_a/src/xuartepb_hw.h
 * as of svn rev 1377.
 */

#ifndef __XILINX_DF_UART_H__
#define __XILINX_DF_UART_H__

#include <asm/io.h>

#if defined(CONFIG_UART0)
# define UART_ID   0
# define UART_BASE XPSS_UART0_BASEADDR
# define XDFUART_MASTER XPAR_XUARTPSS_0_CLOCK_HZ
#elif defined(CONFIG_UART1)
# define UART_ID   1
# define UART_BASE XPSS_UART1_BASEADDR
# define XDFUART_MASTER XPAR_XUARTPSS_1_CLOCK_HZ
#else
# error "Need to configure a UART (0 or 1)"
#endif

/* UART register offsets */
#define XDFUART_CR_OFFSET          0x00  /* Control Register [8:0] */
#define XDFUART_MR_OFFSET          0x04  /* Mode Register [10:0] */
#define XDFUART_IER_OFFSET         0x08  /* Interrupt Enable [10:0] */
#define XDFUART_IDR_OFFSET         0x0C  /* Interrupt Disable [10:0] */
#define XDFUART_IMR_OFFSET         0x10  /* Interrupt Mask [10:0] */
#define XDFUART_ISR_OFFSET         0x14  /* Interrupt Status [10:0]*/
#define XDFUART_BAUDGEN_OFFSET     0x18  /* Baud Rate Generator [15:0] */
#define XDFUART_RXTOUT_OFFSET      0x1C  /* RX Timeout [7:0] */
#define XDFUART_RXWM_OFFSET        0x20  /* RX FIFO Trigger Level [5:0] */
#define XDFUART_MODEMCR_OFFSET     0x24  /* Modem Control [5:0] */
#define XDFUART_MODEMSR_OFFSET     0x28  /* Modem Status [8:0] */
#define XDFUART_SR_OFFSET          0x2C  /* Channel Status [11:0] */
#define XDFUART_FIFO_OFFSET        0x30  /* FIFO [15:0] or [7:0] */
#define XDFUART_BAUDDIV_OFFSET     0x34  /* Baud Rate Divider [7:0] */
#define XDFUART_FLOWDEL_OFFSET     0x38  /* Flow Delay [15:0] */

/* Control register bits */
#define XDFUART_CR_STOPBRK     0x00000100  /* Stop transmission of break */
#define XDFUART_CR_STARTBRK    0x00000080  /* Set break */
#define XDFUART_CR_TORST       0x00000040  /* RX timeout counter restart */
#define XDFUART_CR_TX_DIS      0x00000020  /* TX disabled. */
#define XDFUART_CR_TX_EN       0x00000010  /* TX enabled */
#define XDFUART_CR_RX_DIS      0x00000008  /* RX disabled. */
#define XDFUART_CR_RX_EN       0x00000004  /* RX enabled */
#define XDFUART_CR_EN_DIS_MASK 0x0000003C  /* Enable/disable Mask */
#define XDFUART_CR_TXRST       0x00000002  /* TX logic reset */
#define XDFUART_CR_RXRST       0x00000001  /* RX logic reset */

/* Mode register bits */
#define XDFUART_MR_CCLK             0x00000400  /* Input clock selection */
#define XDFUART_MR_CHMODE_R_LOOP    0x00000300  /* Remote loopback mode */
#define XDFUART_MR_CHMODE_L_LOOP    0x00000200  /* Local loopback mode */
#define XDFUART_MR_CHMODE_ECHO      0x00000100  /* Auto echo mode */
#define XDFUART_MR_CHMODE_NORM      0x00000000  /* Normal mode */
#define XDFUART_MR_CHMODE_SHIFT              8  /* Mode shift */
#define XDFUART_MR_CHMODE_MASK      0x00000300  /* Mode mask */
#define XDFUART_MR_STOPMODE_2_BIT   0x00000080  /* 2 stop bits */
#define XDFUART_MR_STOPMODE_1_5_BIT 0x00000040  /* 1.5 stop bits */
#define XDFUART_MR_STOPMODE_1_BIT   0x00000000  /* 1 stop bit */
#define XDFUART_MR_STOPMODE_SHIFT            6  /* Stop bits setting shift */
#define XDFUART_MR_STOPMODE_MASK    0x000000A0  /* Stop bits setting mask */
#define XDFUART_MR_PARITY_NONE      0x00000020  /* No parity mode */
#define XDFUART_MR_PARITY_MARK      0x00000018  /* Mark parity mode */
#define XDFUART_MR_PARITY_SPACE     0x00000010  /* Space parity mode */
#define XDFUART_MR_PARITY_ODD       0x00000008  /* Odd parity mode */
#define XDFUART_MR_PARITY_EVEN      0x00000000  /* Even parity mode */
#define XDFUART_MR_PARITY_SHIFT              3  /* Parity setting shift */
#define XDFUART_MR_PARITY_MASK      0x00000038  /* Parity mask */
#define XDFUART_MR_CHARLEN_6_BIT    0x00000006  /* 6 bits data */
#define XDFUART_MR_CHARLEN_7_BIT    0x00000004  /* 7 bits data */
#define XDFUART_MR_CHARLEN_8_BIT    0x00000000  /* 8 bits data */
#define XDFUART_MR_CHARLEN_SHIFT             1  /* data Length setting shift */
#define XDFUART_MR_CHARLEN_MASK     0x00000006  /* Data length mask. */
#define XDFUART_MR_CLKSEL           0x00000001  /* Input clock selection */


/*
 * Interrupt registers
 *
 * Interrupt control logic uses the interrupt enable register (IER) and the
 * interrupt disable register (IDR) to set the value of the bits in the
 * interrupt mask register (IMR). The IMR determines whether to pass an
 * interrupt to the interrupt status register (ISR).
 * Writing a 1 to IER Enbables an interrupt, writing a 1 to IDR disables an
 * interrupt. IMR and ISR are read only, and IER and IDR are write only.
 * Reading either IER or IDR returns 0x00.
 *
 * All four registers have the same bit definitions.
 */
#define XDFUART_IXR_DMS     0x00000200	/* Modem status change interrupt */
#define XDFUART_IXR_TOUT    0x00000100	/* Timeout error interrupt */
#define XDFUART_IXR_PARITY  0x00000080	/* Parity error interrupt */
#define XDFUART_IXR_FRAMING 0x00000040	/* Framing error interrupt */
#define XDFUART_IXR_OVER    0x00000020	/* Overrun error interrupt */
#define XDFUART_IXR_TXFULL  0x00000010	/* TX FIFO full interrupt. */
#define XDFUART_IXR_TXEMPTY 0x00000008	/* TX FIFO empty interrupt. */
#define XDFUART_IXR_RXFULL  0x00000004	/* RX FIFO full interrupt. */
#define XDFUART_IXR_RXEMPTY 0x00000002	/* RX FIFO empty interrupt. */
#define XDFUART_IXR_RXOVR   0x00000001	/* RX FIFO trigger interrupt. */
#define XDFUART_IXR_MASK    0x000003FF	/* Valid bit mask */


/* Baud rate generator register
 *
 * The baud rate generator control register (BRGR) is a 16 bit register that
 * controls the receiver bit sample clock and baud rate.
 * Valid values are 1 - 65535.
 *
 * Bit Sample Rate = CCLK / BRGR, where the CCLK is selected by the MR_CCLK bit
 * in the MR register.
 */
#define XDFUART_BAUDGEN_DISABLE 0x00000000    /* Disable clock */
#define XDFUART_BAUDGEN_MASK    0x0000FFFF    /* Valid bits mask */

/* Baud divisor rate register
 *
 * The baud rate divider register (BDIV) controls how much the bit sample
 * rate is divided by. It sets the baud rate.
 * Valid values are 0x04 to 0xFF. Writing a value less than 4 will be ignored.
 *
 * Baud rate = CCLK / ((BAUDDIV + 1) x BRGR), where the CCLK is selected by
 * the MR_CCLK bit in the MR register.
 */
#define XDFUART_BAUDDIV_MASK  0x000000FF    /* 8 bit baud divider mask */


/* Receiver timeout register
 *
 * Use the receiver timeout register (RTR) to detect an idle condition on
 * the receiver data line.
 *
 */
#define XDFUART_RXTOUT_DISABLE    0x00000000  /* Disable time out */
#define XDFUART_RXTOUT_MASK       0x000000FF  /* Valid bits mask */

/* Receiver fifo trigger level register
 *
 * Use the Receiver FIFO Trigger Level Register (RTRIG) to set the value at
 * which the RX FIFO triggers an interrupt event.
 */
#define XDFUART_RXWM_DISABLE 0x00000000  /* Disable RX trigger interrupt */
#define XDFUART_RXWM_MASK    0x0000001F  /* Valid bits mask */

/* Modem control register
 *
 * This register (MODEMCR) controls the interface with the modem or data set,
 * or a peripheral device emulating a modem.
 *
 */
#define XDFUART_MODEMCR_FCM 0x00000010	 /* Flow control mode */
#define XDFUART_MODEMCR_RTS 0x00000002	 /* Request to send */
#define XDFUART_MODEMCR_DTR 0x00000001	 /* Data terminal ready */

/* Modem status register
 *
 * This register (MODEMSR) indicates the current state of the control lines
 * from a modem, or another peripheral device, to the CPU. In addition, four
 * bits of the modem status register provide change information. These bits
 * are set to a logic 1 whenever a control input from the modem changes state.
 *
 * Note: Whenever the DCTS, DDSR, TERI, or DDCD bit is set to logic 1, a modem
 * status interrupt is generated and this is reflected in the modem status
 * register.
 *
 */
#define XDFUART_MODEMSR_FCMS  0x00000100  /* Flow control mode (FCMS) */
#define XDFUART_MODEMSR_DCD   0x00000080  /* Complement of DCD input */
#define XDFUART_MODEMSR_RI    0x00000040  /* Complement of RI input */
#define XDFUART_MODEMSR_DSR   0x00000020  /* Complement of DSR input */
#define XDFUART_MODEMSR_CTS   0x00000010  /* Complement of CTS input */
#define XDFUART_MEDEMSR_DCDX  0x00000008  /* Delta DCD indicator */
#define XDFUART_MEDEMSR_RIX   0x00000004  /* Change of RI */
#define XDFUART_MEDEMSR_DSRX  0x00000002  /* Change of DSR */
#define XDFUART_MEDEMSR_CTSX  0x00000001  /* Change of CTS */

/* Channel status register
 *
 * The channel status register (CSR) is provided to enable the control logic
 * to monitor the status of bits in the channel interrupt status register,
 * even if these are masked out by the interrupt mask register.
 *
 */
#define XDFUART_SR_FLOWDEL  0x00001000	 /* RX FIFO fill over flow delay */
#define XDFUART_SR_TACTIVE  0x00000800	 /* TX active */
#define XDFUART_SR_RACTIVE  0x00000400	 /* RX active */
#define XDFUART_SR_DMS      0x00000200	 /* Delta modem status change */
#define XDFUART_SR_TOUT     0x00000100	 /* RX timeout */
#define XDFUART_SR_PARITY   0x00000080	 /* RX parity error */
#define XDFUART_SR_FRAME    0x00000040	 /* RX frame error */
#define XDFUART_SR_OVER     0x00000020	 /* RX overflow error */
#define XDFUART_SR_TXFULL   0x00000010	 /* TX FIFO full */
#define XDFUART_SR_TXEMPTY  0x00000008	 /* TX FIFO empty */
#define XDFUART_SR_RXFULL   0x00000004	 /* RX FIFO full */
#define XDFUART_SR_RXEMPTY  0x00000002	 /* RX FIFO empty */
#define XDFUART_SR_RXOVR    0x00000001	 /* RX FIFO fill over trigger */

/* Flow delay register
 *
 * Operation of the flow delay register (FLOWDEL) is very similar to the
 * receive FIFO trigger register. An internal trigger signal activates when the
 * FIFO is filled to the level set by this register. This trigger will not
 * cause an interrupt, although it can be read through the channel status
 * register. In hardware flow control mode, RTS is deactivated when the trigger
 * becomes active. RTS only resets when the FIFO level is four less than the
 * level of the flow delay trigger and the flow delay trigger is not activated.
 * A value less than 4 disables the flow delay.
 */
#define XDFUART_FLOWDEL_MASK    XDFUART_RXWM_MASK    /* Valid bit mask */

/* Some access macros */
#define xdfuart_readl(reg) \
	readl((void*)UART_BASE + XDFUART_##reg##_OFFSET)
#define xdfuart_writel(reg,value) \
	writel((value),(void*)UART_BASE + XDFUART_##reg##_OFFSET)

#endif /* __XILINX_DF_UART_H__ */
