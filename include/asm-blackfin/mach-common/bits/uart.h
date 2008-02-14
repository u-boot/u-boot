/*
 * UART Masks
 */

#ifndef __BFIN_PERIPHERAL_UART__
#define __BFIN_PERIPHERAL_UART__

/* UARTx_LCR Masks */
#define WLS			0x03	/* Word Length Select */
#define WLS_5			0x00	/* 5 bit word */
#define WLS_6			0x01	/* 6 bit word */
#define WLS_7			0x02	/* 7 bit word */
#define WLS_8			0x03	/* 8 bit word */
#define STB			0x04	/* Stop Bits */
#define PEN			0x08	/* Parity Enable */
#define EPS			0x10	/* Even Parity Select */
#define STP			0x20	/* Stick Parity */
#define SB			0x40	/* Set Break */
#define DLAB			0x80	/* Divisor Latch Access */

#define DLAB_P			0x07
#define SB_P			0x06
#define STP_P			0x05
#define EPS_P			0x04
#define PEN_P			0x03
#define STB_P			0x02
#define WLS_P1			0x01
#define WLS_P0			0x00

/* UARTx_MCR Mask */
#define XOFF			0x01	/* Transmitter off */
#define MRTS			0x02	/* Manual Request to Send */
#define RFIT			0x04	/* Receive FIFO IRQ Threshold */
#define RFRT			0x08	/* Receive FIFO RTS Threshold */
#define LOOP_ENA		0x10	/* Loopback Mode Enable */
#define FCPOL			0x20	/* Flow Control Pin Polarity */
#define ARTS			0x40	/* Auto RTS generation for RX handshake */
#define ACTS			0x80	/* Auto CTS operation for TX handshake */

#define XOFF_P			0
#define MRTS_P			1
#define RFIT_P			2
#define RFRT_P			3
#define LOOP_ENA_P		4
#define FCPOL_P			5
#define ARTS_P			6
#define ACTS_P			7

/* UARTx_LSR Masks */
#define DR			0x01	/* Data Ready */
#define OE			0x02	/* Overrun Error */
#define PE			0x04	/* Parity Error */
#define FE			0x08	/* Framing Error */
#define BI			0x10	/* Break Interrupt */
#define THRE			0x20	/* THR Empty */
#define TEMT			0x40	/* TSR and UART_THR Empty */

#define DR_P			0x00
#define OE_P			0x01
#define PE_P			0x02
#define FE_P			0x03
#define BI_P			0x04
#define THRE_P			0x05
#define TEMT_P			0x06

/* UARTx_IER Masks */
#define ERBFI			0x01	/* Enable Receive Buffer Full Interrupt */
#define ETBEI			0x02	/* Enable Transmit Buffer Empty Interrupt */
#define ELSI			0x04	/* Enable RX Status Interrupt */

#define ERBFI_P			0x00
#define ETBEI_P			0x01
#define ELSI_P			0x02

/* UARTx_IIR Masks */
#define NINT			0x01	/* Pending Interrupt */
#define STATUS			0x06	/* Highest Priority Pending Interrupt */

#define NINT_P			0x00
#define STATUS_P0		0x01
#define STATUS_P1		0x02

/* UARTx_GCTL Masks */
#define UCEN			0x01	/* Enable UARTx Clocks */
#define IREN			0x02	/* Enable IrDA Mode */
#define TPOLC			0x04	/* IrDA TX Polarity Change */
#define RPOLC			0x08	/* IrDA RX Polarity Change */
#define FPE			0x10	/* Force Parity Error On Transmit */
#define FFE			0x20	/* Force Framing Error On Transmit */

#define UCEN_P			0x00
#define IREN_P			0x01
#define TPOLC_P			0x02
#define RPOLC_P			0x03
#define FPE_P			0x04
#define FFE_P			0x05

#endif
