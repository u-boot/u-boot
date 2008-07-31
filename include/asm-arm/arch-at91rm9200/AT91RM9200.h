/*
 * (C) Copyright 2003
 * AT91RM9200 definitions
 * Author : ATMEL AT91 application group
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

#ifndef AT91RM9200_H
#define AT91RM9200_H

#ifndef __ASSEMBLY__
typedef volatile unsigned int AT91_REG;		/* Hardware register definition */

/*****************************************************************************/
/*        SOFTWARE API DEFINITION  FOR Timer Counter Channel Interface       */
/*****************************************************************************/
typedef struct _AT91S_TC
{
	AT91_REG	 TC_CCR;	/* Channel Control Register */
	AT91_REG	 TC_CMR;	/* Channel Mode Register */
	AT91_REG	 Reserved0[2];	/*  */
	AT91_REG	 TC_CV;		/* Counter Value */
	AT91_REG	 TC_RA;		/* Register A */
	AT91_REG	 TC_RB;		/* Register B */
	AT91_REG	 TC_RC;		/* Register C */
	AT91_REG	 TC_SR;		/* Status Register */
	AT91_REG	 TC_IER;	/* Interrupt Enable Register */
	AT91_REG	 TC_IDR;	/* Interrupt Disable Register */
	AT91_REG	 TC_IMR;	/* Interrupt Mask Register */
} AT91S_TC, *AT91PS_TC;

#define AT91C_TC_TIMER_DIV1_CLOCK ((unsigned int) 0x0 <<  0) /* (TC) MCK/2 */
#define AT91C_TC_TIMER_DIV2_CLOCK ((unsigned int) 0x1 <<  0) /* (TC) MCK/8 */
#define AT91C_TC_TIMER_DIV3_CLOCK ((unsigned int) 0x2 <<  0) /* (TC) MCK/32 */
#define AT91C_TC_TIMER_DIV4_CLOCK ((unsigned int) 0x3 <<  0) /* (TC) MCK/128 */
#define AT91C_TC_SLOW_CLOCK	  ((unsigned int) 0x4 <<  0) /* (TC) SLOW CLK*/
#define AT91C_TC_XC0_CLOCK	  ((unsigned int) 0x5 <<  0) /* (TC) XC0 */
#define AT91C_TC_XC1_CLOCK	  ((unsigned int) 0x6 <<  0) /* (TC) XC1 */
#define AT91C_TC_XC2_CLOCK	  ((unsigned int) 0x7 <<  0) /* (TC) XC2 */
#define AT91C_TCB_TC0XC0S_NONE	  ((unsigned int) 0x1)       /* (TCB) None signal connected to XC0 */
#define AT91C_TCB_TC1XC1S_NONE	  ((unsigned int) 0x1 <<  2) /* (TCB) None signal connected to XC1 */
#define AT91C_TCB_TC2XC2S_NONE	  ((unsigned int) 0x1 <<  4) /* (TCB) None signal connected to XC2 */
#define AT91C_TC_CLKDIS		  ((unsigned int) 0x1 <<  1) /* (TC) Counter Clock Disable Command */
#define AT91C_TC_SWTRG		  ((unsigned int) 0x1 <<  2) /* (TC) Software Trigger Command */
#define AT91C_TC_CLKEN		  ((unsigned int) 0x1 <<  0) /* (TC) Counter Clock Enable Command */

/*****************************************************************************/
/*                  SOFTWARE API DEFINITION  FOR Usart                       */
/*****************************************************************************/
typedef struct _AT91S_USART
{
	AT91_REG	 US_CR;		/* Control Register */
	AT91_REG	 US_MR;		/* Mode Register */
	AT91_REG	 US_IER;	/* Interrupt Enable Register */
	AT91_REG	 US_IDR;	/* Interrupt Disable Register */
	AT91_REG	 US_IMR;	/* Interrupt Mask Register */
	AT91_REG	 US_CSR;	/* Channel Status Register */
	AT91_REG	 US_RHR;	/* Receiver Holding Register */
	AT91_REG	 US_THR;	/* Transmitter Holding Register */
	AT91_REG	 US_BRGR;	/* Baud Rate Generator Register */
	AT91_REG	 US_RTOR;	/* Receiver Time-out Register */
	AT91_REG	 US_TTGR;	/* Transmitter Time-guard Register */
	AT91_REG	 Reserved0[5];	/*  */
	AT91_REG	 US_FIDI;	/* FI_DI_Ratio Register */
	AT91_REG	 US_NER;	/* Nb Errors Register */
	AT91_REG	 US_XXR;	/* XON_XOFF Register */
	AT91_REG	 US_IF;		/* IRDA_FILTER Register */
	AT91_REG	 Reserved1[44];	/*  */
	AT91_REG	 US_RPR;	/* Receive Pointer Register */
	AT91_REG	 US_RCR;	/* Receive Counter Register */
	AT91_REG	 US_TPR;	/* Transmit Pointer Register */
	AT91_REG	 US_TCR;	/* Transmit Counter Register */
	AT91_REG	 US_RNPR;	/* Receive Next Pointer Register */
	AT91_REG	 US_RNCR;	/* Receive Next Counter Register */
	AT91_REG	 US_TNPR;	/* Transmit Next Pointer Register */
	AT91_REG	 US_TNCR;	/* Transmit Next Counter Register */
	AT91_REG	 US_PTCR;	/* PDC Transfer Control Register */
	AT91_REG	 US_PTSR;	/* PDC Transfer Status Register */
} AT91S_USART, *AT91PS_USART;

/*****************************************************************************/
/*          SOFTWARE API DEFINITION  FOR Clock Generator Controler           */
/*****************************************************************************/
typedef struct _AT91S_CKGR
{
	AT91_REG	 CKGR_MOR;	/* Main Oscillator Register */
	AT91_REG	 CKGR_MCFR;	/* Main Clock  Frequency Register */
	AT91_REG	 CKGR_PLLAR;	/* PLL A Register */
	AT91_REG	 CKGR_PLLBR;	/* PLL B Register */
} AT91S_CKGR, *AT91PS_CKGR;

/* -------- CKGR_MOR : (CKGR Offset: 0x0) Main Oscillator Register -------- */
#define AT91C_CKGR_MOSCEN	((unsigned int) 0x1  <<  0)	/* (CKGR) Main Oscillator Enable */
#define AT91C_CKGR_OSCTEST	((unsigned int) 0x1  <<  1)	/* (CKGR) Oscillator Test */
#define AT91C_CKGR_OSCOUNT	((unsigned int) 0xFF <<  8)	/* (CKGR) Main Oscillator Start-up Time */

/* -------- CKGR_MCFR : (CKGR Offset: 0x4) Main Clock Frequency Register -------- */
#define AT91C_CKGR_MAINF	((unsigned int) 0xFFFF <<  0)	/* (CKGR) Main Clock Frequency */
#define AT91C_CKGR_MAINRDY	((unsigned int) 0x1 << 16)	/* (CKGR) Main Clock Ready */

/* -------- CKGR_PLLAR : (CKGR Offset: 0x8) PLL A Register -------- */
#define AT91C_CKGR_DIVA		((unsigned int) 0xFF  <<  0)	/* (CKGR) Divider Selected */
#define AT91C_CKGR_DIVA_0	((unsigned int) 0x0)		/* (CKGR) Divider output is 0 */
#define AT91C_CKGR_DIVA_BYPASS	((unsigned int) 0x1)		/* (CKGR) Divider is bypassed */
#define AT91C_CKGR_PLLACOUNT	((unsigned int) 0x3F  <<  8)	/* (CKGR) PLL A Counter */
#define AT91C_CKGR_OUTA		((unsigned int) 0x3   << 14)	/* (CKGR) PLL A Output Frequency Range */
#define AT91C_CKGR_OUTA_0	((unsigned int) 0x0   << 14)	/* (CKGR) Please refer to the PLLA datasheet */
#define AT91C_CKGR_OUTA_1	((unsigned int) 0x1   << 14)	/* (CKGR) Please refer to the PLLA datasheet */
#define AT91C_CKGR_OUTA_2	((unsigned int) 0x2   << 14)	/* (CKGR) Please refer to the PLLA datasheet */
#define AT91C_CKGR_OUTA_3	((unsigned int) 0x3   << 14)	/* (CKGR) Please refer to the PLLA datasheet */
#define AT91C_CKGR_MULA		((unsigned int) 0x7FF << 16)	/* (CKGR) PLL A Multiplier */
#define AT91C_CKGR_SRCA		((unsigned int) 0x1   << 29)	/* (CKGR) PLL A Source */

/* -------- CKGR_PLLBR : (CKGR Offset: 0xc) PLL B Register -------- */
#define AT91C_CKGR_DIVB		((unsigned int) 0xFF  <<  0)	/* (CKGR) Divider Selected */
#define AT91C_CKGR_DIVB_0	((unsigned int) 0x0)		/* (CKGR) Divider output is 0 */
#define AT91C_CKGR_DIVB_BYPASS	((unsigned int) 0x1)		/* (CKGR) Divider is bypassed */
#define AT91C_CKGR_PLLBCOUNT	((unsigned int) 0x3F  <<  8)	/* (CKGR) PLL B Counter */
#define AT91C_CKGR_OUTB		((unsigned int) 0x3   << 14)	/* (CKGR) PLL B Output Frequency Range */
#define AT91C_CKGR_OUTB_0	((unsigned int) 0x0   << 14)	/* (CKGR) Please refer to the PLLB datasheet */
#define AT91C_CKGR_OUTB_1	((unsigned int) 0x1   << 14)	/* (CKGR) Please refer to the PLLB datasheet */
#define AT91C_CKGR_OUTB_2	((unsigned int) 0x2   << 14)	/* (CKGR) Please refer to the PLLB datasheet */
#define AT91C_CKGR_OUTB_3	((unsigned int) 0x3   << 14)	/* (CKGR) Please refer to the PLLB datasheet */
#define AT91C_CKGR_MULB		((unsigned int) 0x7FF << 16)	/* (CKGR) PLL B Multiplier */
#define AT91C_CKGR_USB_96M	((unsigned int) 0x1   << 28)	/* (CKGR) Divider for USB Ports */
#define AT91C_CKGR_USB_PLL	((unsigned int) 0x1   << 29)	/* (CKGR) PLL Use */

/*****************************************************************************/
/*        SOFTWARE API DEFINITION  FOR Parallel Input Output Controler       */
/*****************************************************************************/
typedef struct _AT91S_PIO
{
	AT91_REG	 PIO_PER;	/* PIO Enable Register */
	AT91_REG	 PIO_PDR;	/* PIO Disable Register */
	AT91_REG	 PIO_PSR;	/* PIO Status Register */
	AT91_REG	 Reserved0[1];	/*  */
	AT91_REG	 PIO_OER;	/* Output Enable Register */
	AT91_REG	 PIO_ODR;	/* Output Disable Registerr */
	AT91_REG	 PIO_OSR;	/* Output Status Register */
	AT91_REG	 Reserved1[1];	/*  */
	AT91_REG	 PIO_IFER;	/* Input Filter Enable Register */
	AT91_REG	 PIO_IFDR;	/* Input Filter Disable Register */
	AT91_REG	 PIO_IFSR;	/* Input Filter Status Register */
	AT91_REG	 Reserved2[1];	/*  */
	AT91_REG	 PIO_SODR;	/* Set Output Data Register */
	AT91_REG	 PIO_CODR;	/* Clear Output Data Register */
	AT91_REG	 PIO_ODSR;	/* Output Data Status Register */
	AT91_REG	 PIO_PDSR;	/* Pin Data Status Register */
	AT91_REG	 PIO_IER;	/* Interrupt Enable Register */
	AT91_REG	 PIO_IDR;	/* Interrupt Disable Register */
	AT91_REG	 PIO_IMR;	/* Interrupt Mask Register */
	AT91_REG	 PIO_ISR;	/* Interrupt Status Register */
	AT91_REG	 PIO_MDER;	/* Multi-driver Enable Register */
	AT91_REG	 PIO_MDDR;	/* Multi-driver Disable Register */
	AT91_REG	 PIO_MDSR;	/* Multi-driver Status Register */
	AT91_REG	 Reserved3[1];	/*  */
	AT91_REG	 PIO_PPUDR;	/* Pull-up Disable Register */
	AT91_REG	 PIO_PPUER;	/* Pull-up Enable Register */
	AT91_REG	 PIO_PPUSR;	/* Pad Pull-up Status Register */
	AT91_REG	 Reserved4[1];	/*  */
	AT91_REG	 PIO_ASR;	/* Select A Register */
	AT91_REG	 PIO_BSR;	/* Select B Register */
	AT91_REG	 PIO_ABSR;	/* AB Select Status Register */
	AT91_REG	 Reserved5[9];	/*  */
	AT91_REG	 PIO_OWER;	/* Output Write Enable Register */
	AT91_REG	 PIO_OWDR;	/* Output Write Disable Register */
	AT91_REG	 PIO_OWSR;	/* Output Write Status Register */
} AT91S_PIO, *AT91PS_PIO;


/*****************************************************************************/
/*              SOFTWARE API DEFINITION  FOR Debug Unit                      */
/*****************************************************************************/
typedef struct _AT91S_DBGU
{
	AT91_REG	 DBGU_CR;	/* Control Register */
	AT91_REG	 DBGU_MR;	/* Mode Register */
	AT91_REG	 DBGU_IER;	/* Interrupt Enable Register */
	AT91_REG	 DBGU_IDR;	/* Interrupt Disable Register */
	AT91_REG	 DBGU_IMR;	/* Interrupt Mask Register */
	AT91_REG	 DBGU_CSR;	/* Channel Status Register */
	AT91_REG	 DBGU_RHR;	/* Receiver Holding Register */
	AT91_REG	 DBGU_THR;	/* Transmitter Holding Register */
	AT91_REG	 DBGU_BRGR;	/* Baud Rate Generator Register */
	AT91_REG	 Reserved0[7];	/*  */
	AT91_REG	 DBGU_C1R;	/* Chip ID1 Register */
	AT91_REG	 DBGU_C2R;	/* Chip ID2 Register */
	AT91_REG	 DBGU_FNTR;	/* Force NTRST Register */
	AT91_REG	 Reserved1[45];	/*  */
	AT91_REG	 DBGU_RPR;	/* Receive Pointer Register */
	AT91_REG	 DBGU_RCR;	/* Receive Counter Register */
	AT91_REG	 DBGU_TPR;	/* Transmit Pointer Register */
	AT91_REG	 DBGU_TCR;	/* Transmit Counter Register */
	AT91_REG	 DBGU_RNPR;	/* Receive Next Pointer Register */
	AT91_REG	 DBGU_RNCR;	/* Receive Next Counter Register */
	AT91_REG	 DBGU_TNPR;	/* Transmit Next Pointer Register */
	AT91_REG	 DBGU_TNCR;	/* Transmit Next Counter Register */
	AT91_REG	 DBGU_PTCR;	/* PDC Transfer Control Register */
	AT91_REG	 DBGU_PTSR;	/* PDC Transfer Status Register */
} AT91S_DBGU, *AT91PS_DBGU;

/* -------- DBGU_IER : (DBGU Offset: 0x8) Debug Unit Interrupt Enable Register --------  */
#define AT91C_US_RXRDY		((unsigned int) 0x1 <<  0) /* (DBGU) RXRDY Interrupt */
#define AT91C_US_TXRDY		((unsigned int) 0x1 <<  1) /* (DBGU) TXRDY Interrupt */
#define AT91C_US_ENDRX		((unsigned int) 0x1 <<  3) /* (DBGU) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX		((unsigned int) 0x1 <<  4) /* (DBGU) End of Transmit Interrupt */
#define AT91C_US_OVRE		((unsigned int) 0x1 <<  5) /* (DBGU) Overrun Interrupt */
#define AT91C_US_FRAME		((unsigned int) 0x1 <<  6) /* (DBGU) Framing Error Interrupt */
#define AT91C_US_PARE		((unsigned int) 0x1 <<  7) /* (DBGU) Parity Error Interrupt */
#define AT91C_US_TXEMPTY	((unsigned int) 0x1 <<  9) /* (DBGU) TXEMPTY Interrupt */
#define AT91C_US_TXBUFE		((unsigned int) 0x1 << 11) /* (DBGU) TXBUFE Interrupt */
#define AT91C_US_RXBUFF		((unsigned int) 0x1 << 12) /* (DBGU) RXBUFF Interrupt */
#define AT91C_US_COMM_TX	((unsigned int) 0x1 << 30) /* (DBGU) COMM_TX Interrupt */
#define AT91C_US_COMM_RX	((unsigned int) 0x1 << 31) /* (DBGU) COMM_RX Interrupt */

/* -------- DBGU_CR : (DBGU Offset: 0x0) Debug Unit Control Register --------  */
#define AT91C_US_RSTRX		((unsigned int) 0x1 <<  2) /* (DBGU) Reset Receiver */
#define AT91C_US_RSTTX		((unsigned int) 0x1 <<  3) /* (DBGU) Reset Transmitter */
#define AT91C_US_RXEN		((unsigned int) 0x1 <<  4) /* (DBGU) Receiver Enable */
#define AT91C_US_RXDIS		((unsigned int) 0x1 <<  5) /* (DBGU) Receiver Disable */
#define AT91C_US_TXEN		((unsigned int) 0x1 <<  6) /* (DBGU) Transmitter Enable */
#define AT91C_US_TXDIS		((unsigned int) 0x1 <<  7) /* (DBGU) Transmitter Disable */

#define AT91C_US_CLKS_CLOCK	((unsigned int) 0x0 <<  4) /* (USART) Clock */
#define AT91C_US_CHRL_8_BITS	((unsigned int) 0x3 <<  6) /* (USART) Character Length: 8 bits */
#define AT91C_US_PAR_NONE	((unsigned int) 0x4 <<  9) /* (DBGU) No Parity */
#define AT91C_US_NBSTOP_1_BIT	((unsigned int) 0x0 << 12) /* (USART) 1 stop bit */

/*****************************************************************************/
/*      SOFTWARE API DEFINITION  FOR Static Memory Controller 2 Interface    */
/*****************************************************************************/
typedef struct _AT91S_SMC2
{
	AT91_REG	 SMC2_CSR[8];	/* SMC2 Chip Select Register */
} AT91S_SMC2, *AT91PS_SMC2;

/* -------- SMC2_CSR : (SMC2 Offset: 0x0) SMC2 Chip Select Register --------  */
#define AT91C_SMC2_NWS			((unsigned int) 0x7F << 0) /* (SMC2) Number of Wait States */
#define AT91C_SMC2_WSEN			((unsigned int) 0x1 <<  7) /* (SMC2) Wait State Enable */
#define AT91C_SMC2_TDF			((unsigned int) 0xF <<  8) /* (SMC2) Data Float Time */
#define AT91C_SMC2_BAT			((unsigned int) 0x1 << 12) /* (SMC2) Byte Access Type */
#define AT91C_SMC2_DBW			((unsigned int) 0x1 << 13) /* (SMC2) Data Bus Width */
#define AT91C_SMC2_DBW_16		((unsigned int) 0x1 << 13) /* (SMC2) 16-bit. */
#define AT91C_SMC2_DBW_8		((unsigned int) 0x2 << 13) /* (SMC2) 8-bit. */
#define AT91C_SMC2_DRP			((unsigned int) 0x1 << 15) /* (SMC2) Data Read Protocol */
#define AT91C_SMC2_ACSS			((unsigned int) 0x3 << 16) /* (SMC2) Address to Chip Select Setup */
#define AT91C_SMC2_ACSS_STANDARD	((unsigned int) 0x0 << 16) /* (SMC2) Standard, asserted at the beginning of the access and deasserted at the end. */
#define AT91C_SMC2_ACSS_1_CYCLE		((unsigned int) 0x1 << 16) /* (SMC2) One cycle less at the beginning and the end of the access. */
#define AT91C_SMC2_ACSS_2_CYCLES	((unsigned int) 0x2 << 16) /* (SMC2) Two cycles less at the beginning and the end of the access. */
#define AT91C_SMC2_ACSS_3_CYCLES	((unsigned int) 0x3 << 16) /* (SMC2) Three cycles less at the beginning and the end of the access. */
#define AT91C_SMC2_RWSETUP		((unsigned int) 0x7 << 24) /* (SMC2) Read and Write Signal Setup Time */
#define AT91C_SMC2_RWHOLD		((unsigned int) 0x7 << 29) /* (SMC2) Read and Write Signal Hold Time */

/*****************************************************************************/
/*           SOFTWARE API DEFINITION  FOR Power Management Controler         */
/*****************************************************************************/
typedef struct _AT91S_PMC
{
	AT91_REG	 PMC_SCER;	/* System Clock Enable Register */
	AT91_REG	 PMC_SCDR;	/* System Clock Disable Register */
	AT91_REG	 PMC_SCSR;	/* System Clock Status Register */
	AT91_REG	 Reserved0[1];	/* */
	AT91_REG	 PMC_PCER;	/* Peripheral Clock Enable Register */
	AT91_REG	 PMC_PCDR;	/* Peripheral Clock Disable Register */
	AT91_REG	 PMC_PCSR;	/* Peripheral Clock Status Register */
	AT91_REG	 Reserved1[5];	/* */
	AT91_REG	 PMC_MCKR;	/* Master Clock Register */
	AT91_REG	 Reserved2[3];	/* */
	AT91_REG	 PMC_PCKR[8];	/* Programmable Clock Register */
	AT91_REG	 PMC_IER;	/* Interrupt Enable Register */
	AT91_REG	 PMC_IDR;	/* Interrupt Disable Register */
	AT91_REG	 PMC_SR;	/* Status Register */
	AT91_REG	 PMC_IMR;	/* Interrupt Mask Register */
} AT91S_PMC, *AT91PS_PMC;

/*------- PMC_SCER : (PMC Offset: 0x0) System Clock Enable Register --------*/
#define AT91C_PMC_PCK		((unsigned int) 0x1 <<  0) /* (PMC) Processor Clock */
#define AT91C_PMC_UDP		((unsigned int) 0x1 <<  1) /* (PMC) USB Device Port Clock */
#define AT91C_PMC_MCKUDP	((unsigned int) 0x1 <<  2) /* (PMC) USB Device Port Master Clock Automatic Disable on Suspend */
#define AT91C_PMC_UHP		((unsigned int) 0x1 <<  4) /* (PMC) USB Host Port Clock */
#define AT91C_PMC_PCK0		((unsigned int) 0x1 <<  8) /* (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK1		((unsigned int) 0x1 <<  9) /* (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK2		((unsigned int) 0x1 << 10) /* (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK3		((unsigned int) 0x1 << 11) /* (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK4		((unsigned int) 0x1 << 12) /* (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK5		((unsigned int) 0x1 << 13) /* (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK6		((unsigned int) 0x1 << 14) /* (PMC) Programmable Clock Output */
#define AT91C_PMC_PCK7		((unsigned int) 0x1 << 15) /* (PMC) Programmable Clock Output */
/*-------- PMC_SCDR : (PMC Offset: 0x4) System Clock Disable Register ------*/
/*-------- PMC_SCSR : (PMC Offset: 0x8) System Clock Status Register -------*/
/*-------- PMC_MCKR : (PMC Offset: 0x30) Master Clock Register --------*/
#define AT91C_PMC_CSS		((unsigned int) 0x3 <<  0) /* (PMC) Programmable Clock Selection */
#define AT91C_PMC_CSS_SLOW_CLK	((unsigned int) 0x0)       /* (PMC) Slow Clock is selected */
#define AT91C_PMC_CSS_MAIN_CLK	((unsigned int) 0x1)       /* (PMC) Main Clock is selected */
#define AT91C_PMC_CSS_PLLA_CLK	((unsigned int) 0x2)       /* (PMC) Clock from PLL A is selected */
#define AT91C_PMC_CSS_PLLB_CLK	((unsigned int) 0x3)       /* (PMC) Clock from PLL B is selected */
#define AT91C_PMC_PRES		((unsigned int) 0x7 <<  2) /* (PMC) Programmable Clock Prescaler */
#define AT91C_PMC_PRES_CLK	((unsigned int) 0x0 <<  2) /* (PMC) Selected clock */
#define AT91C_PMC_PRES_CLK_2	((unsigned int) 0x1 <<  2) /* (PMC) Selected clock divided by 2 */
#define AT91C_PMC_PRES_CLK_4	((unsigned int) 0x2 <<  2) /* (PMC) Selected clock divided by 4 */
#define AT91C_PMC_PRES_CLK_8	((unsigned int) 0x3 <<  2) /* (PMC) Selected clock divided by 8 */
#define AT91C_PMC_PRES_CLK_16	((unsigned int) 0x4 <<  2) /* (PMC) Selected clock divided by 16 */
#define AT91C_PMC_PRES_CLK_32	((unsigned int) 0x5 <<  2) /* (PMC) Selected clock divided by 32 */
#define AT91C_PMC_PRES_CLK_64	((unsigned int) 0x6 <<  2) /* (PMC) Selected clock divided by 64 */
#define AT91C_PMC_MDIV		((unsigned int) 0x3 <<  8) /* (PMC) Master Clock Division */
#define AT91C_PMC_MDIV_1	((unsigned int) 0x0 <<  8) /* (PMC) The master clock and the processor clock are the same */
#define AT91C_PMC_MDIV_2	((unsigned int) 0x1 <<  8) /* (PMC) The processor clock is twice as fast as the master clock */
#define AT91C_PMC_MDIV_3	((unsigned int) 0x2 <<  8) /* (PMC) The processor clock is three times faster than the master clock */
#define AT91C_PMC_MDIV_4	((unsigned int) 0x3 <<  8) /* (PMC) The processor clock is four times faster than the master clock */
/*------ PMC_PCKR : (PMC Offset: 0x40) Programmable Clock Register --------*/
/*------ PMC_IER : (PMC Offset: 0x60) PMC Interrupt Enable Register -------*/
#define AT91C_PMC_MOSCS		((unsigned int) 0x1 <<  0) /* (PMC) MOSC Status/Enable/Disable/Mask */
#define AT91C_PMC_LOCKA		((unsigned int) 0x1 <<  1) /* (PMC) PLL A Status/Enable/Disable/Mask */
#define AT91C_PMC_LOCKB		((unsigned int) 0x1 <<  2) /* (PMC) PLL B Status/Enable/Disable/Mask */
#define AT91C_PMC_MCKRDY	((unsigned int) 0x1 <<  3) /* (PMC) MCK_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK0RDY	((unsigned int) 0x1 <<  8) /* (PMC) PCK0_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK1RDY	((unsigned int) 0x1 <<  9) /* (PMC) PCK1_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK2RDY	((unsigned int) 0x1 << 10) /* (PMC) PCK2_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK3RDY	((unsigned int) 0x1 << 11) /* (PMC) PCK3_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK4RDY	((unsigned int) 0x1 << 12) /* (PMC) PCK4_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK5RDY	((unsigned int) 0x1 << 13) /* (PMC) PCK5_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK6RDY	((unsigned int) 0x1 << 14) /* (PMC) PCK6_RDY Status/Enable/Disable/Mask */
#define AT91C_PMC_PCK7RDY	((unsigned int) 0x1 << 15) /* (PMC) PCK7_RDY Status/Enable/Disable/Mask */
/*---- PMC_IDR : (PMC Offset: 0x64) PMC Interrupt Disable Register --------*/
/*-------- PMC_SR : (PMC Offset: 0x68) PMC Status Register --------*/
/*-------- PMC_IMR : (PMC Offset: 0x6c) PMC Interrupt Mask Register --------*/

/*****************************************************************************/
/*              SOFTWARE API DEFINITION  FOR Ethernet MAC                    */
/*****************************************************************************/
typedef struct _AT91S_EMAC
{
	AT91_REG	 EMAC_CTL;	/* Network Control Register */
	AT91_REG	 EMAC_CFG;	/* Network Configuration Register */
	AT91_REG	 EMAC_SR;	/* Network Status Register */
	AT91_REG	 EMAC_TAR;	/* Transmit Address Register */
	AT91_REG	 EMAC_TCR;	/* Transmit Control Register */
	AT91_REG	 EMAC_TSR;	/* Transmit Status Register */
	AT91_REG	 EMAC_RBQP;	/* Receive Buffer Queue Pointer */
	AT91_REG	 Reserved0[1];	/*  */
	AT91_REG	 EMAC_RSR;	/* Receive Status Register */
	AT91_REG	 EMAC_ISR;	/* Interrupt Status Register */
	AT91_REG	 EMAC_IER;	/* Interrupt Enable Register */
	AT91_REG	 EMAC_IDR;	/* Interrupt Disable Register */
	AT91_REG	 EMAC_IMR;	/* Interrupt Mask Register */
	AT91_REG	 EMAC_MAN;	/* PHY Maintenance Register */
	AT91_REG	 Reserved1[2];	/*  */
	AT91_REG	 EMAC_FRA;	/* Frames Transmitted OK Register */
	AT91_REG	 EMAC_SCOL;	/* Single Collision Frame Register */
	AT91_REG	 EMAC_MCOL;	/* Multiple Collision Frame Register */
	AT91_REG	 EMAC_OK;	/* Frames Received OK Register */
	AT91_REG	 EMAC_SEQE;	/* Frame Check Sequence Error Register */
	AT91_REG	 EMAC_ALE;	/* Alignment Error Register */
	AT91_REG	 EMAC_DTE;	/* Deferred Transmission Frame Register */
	AT91_REG	 EMAC_LCOL;	/* Late Collision Register */
	AT91_REG	 EMAC_ECOL;	/* Excessive Collision Register */
	AT91_REG	 EMAC_CSE;	/* Carrier Sense Error Register */
	AT91_REG	 EMAC_TUE;	/* Transmit Underrun Error Register */
	AT91_REG	 EMAC_CDE;	/* Code Error Register */
	AT91_REG	 EMAC_ELR;	/* Excessive Length Error Register */
	AT91_REG	 EMAC_RJB;	/* Receive Jabber Register */
	AT91_REG	 EMAC_USF;	/* Undersize Frame Register */
	AT91_REG	 EMAC_SQEE;	/* SQE Test Error Register */
	AT91_REG	 EMAC_DRFC;	/* Discarded RX Frame Register */
	AT91_REG	 Reserved2[3];	/*  */
	AT91_REG	 EMAC_HSH;	/* Hash Address High[63:32] */
	AT91_REG	 EMAC_HSL;	/* Hash Address Low[31:0] */
	AT91_REG	 EMAC_SA1L;	/* Specific Address 1 Low, First 4 bytes */
	AT91_REG	 EMAC_SA1H;	/* Specific Address 1 High, Last 2 bytes */
	AT91_REG	 EMAC_SA2L;	/* Specific Address 2 Low, First 4 bytes */
	AT91_REG	 EMAC_SA2H;	/* Specific Address 2 High, Last 2 bytes */
	AT91_REG	 EMAC_SA3L;	/* Specific Address 3 Low, First 4 bytes */
	AT91_REG	 EMAC_SA3H;	/* Specific Address 3 High, Last 2 bytes */
	AT91_REG	 EMAC_SA4L;	/* Specific Address 4 Low, First 4 bytes */
	AT91_REG	 EMAC_SA4H;	/* Specific Address 4 High, Last 2 bytesr */
} AT91S_EMAC, *AT91PS_EMAC;

/* -------- EMAC_CTL : (EMAC Offset: 0x0)  --------  */
#define AT91C_EMAC_LB		((unsigned int) 0x1 <<  0) /* (EMAC) Loopback. Optional. When set, loopback signal is at high level. */
#define AT91C_EMAC_LBL		((unsigned int) 0x1 <<  1) /* (EMAC) Loopback local. */
#define AT91C_EMAC_RE		((unsigned int) 0x1 <<  2) /* (EMAC) Receive enable. */
#define AT91C_EMAC_TE		((unsigned int) 0x1 <<  3) /* (EMAC) Transmit enable. */
#define AT91C_EMAC_MPE		((unsigned int) 0x1 <<  4) /* (EMAC) Management port enable. */
#define AT91C_EMAC_CSR		((unsigned int) 0x1 <<  5) /* (EMAC) Clear statistics registers. */
#define AT91C_EMAC_ISR		((unsigned int) 0x1 <<  6) /* (EMAC) Increment statistics registers. */
#define AT91C_EMAC_WES		((unsigned int) 0x1 <<  7) /* (EMAC) Write enable for statistics registers. */
#define AT91C_EMAC_BP		((unsigned int) 0x1 <<  8) /* (EMAC) Back pressure. */

/* -------- EMAC_CFG : (EMAC Offset: 0x4) Network Configuration Register --------  */
#define AT91C_EMAC_SPD		((unsigned int) 0x1 <<  0) /* (EMAC) Speed. */
#define AT91C_EMAC_FD		((unsigned int) 0x1 <<  1) /* (EMAC) Full duplex. */
#define AT91C_EMAC_BR		((unsigned int) 0x1 <<  2) /* (EMAC) Bit rate. */
#define AT91C_EMAC_CAF		((unsigned int) 0x1 <<  4) /* (EMAC) Copy all frames. */
#define AT91C_EMAC_NBC		((unsigned int) 0x1 <<  5) /* (EMAC) No broadcast. */
#define AT91C_EMAC_MTI		((unsigned int) 0x1 <<  6) /* (EMAC) Multicast hash enable */
#define AT91C_EMAC_UNI		((unsigned int) 0x1 <<  7) /* (EMAC) Unicast hash enable. */
#define AT91C_EMAC_BIG		((unsigned int) 0x1 <<  8) /* (EMAC) Receive 1522 bytes. */
#define AT91C_EMAC_EAE		((unsigned int) 0x1 <<  9) /* (EMAC) External address match enable. */
#define AT91C_EMAC_CLK		((unsigned int) 0x3 << 10) /* (EMAC) */
#define AT91C_EMAC_CLK_HCLK_8	((unsigned int) 0x0 << 10) /* (EMAC) HCLK divided by 8 */
#define AT91C_EMAC_CLK_HCLK_16	((unsigned int) 0x1 << 10) /* (EMAC) HCLK divided by 16 */
#define AT91C_EMAC_CLK_HCLK_32	((unsigned int) 0x2 << 10) /* (EMAC) HCLK divided by 32 */
#define AT91C_EMAC_CLK_HCLK_64	((unsigned int) 0x3 << 10) /* (EMAC) HCLK divided by 64 */
#define AT91C_EMAC_RTY		((unsigned int) 0x1 << 12) /* (EMAC) */
#define AT91C_EMAC_RMII		((unsigned int) 0x1 << 13) /* (EMAC) */

/* -------- EMAC_SR : (EMAC Offset: 0x8) Network Status Register --------  */
#define AT91C_EMAC_MDIO		((unsigned int) 0x1 <<  1) /* (EMAC) */
#define AT91C_EMAC_IDLE		((unsigned int) 0x1 <<  2) /* (EMAC) */

/* -------- EMAC_TCR : (EMAC Offset: 0x10) Transmit Control Register ------- */
#define AT91C_EMAC_LEN		((unsigned int) 0x7FF <<  0) /* (EMAC) */
#define AT91C_EMAC_NCRC		((unsigned int) 0x1 << 15) /* (EMAC) */

/* -------- EMAC_TSR : (EMAC Offset: 0x14) Transmit Control Register ------- */
#define AT91C_EMAC_OVR		((unsigned int) 0x1 <<  0) /* (EMAC) */
#define AT91C_EMAC_COL		((unsigned int) 0x1 <<  1) /* (EMAC) */
#define AT91C_EMAC_RLE		((unsigned int) 0x1 <<  2) /* (EMAC) */
#define AT91C_EMAC_TXIDLE	((unsigned int) 0x1 <<  3) /* (EMAC) */
#define AT91C_EMAC_BNQ		((unsigned int) 0x1 <<  4) /* (EMAC) */
#define AT91C_EMAC_COMP		((unsigned int) 0x1 <<  5) /* (EMAC) */
#define AT91C_EMAC_UND		((unsigned int) 0x1 <<  6) /* (EMAC) */

/* -------- EMAC_RSR : (EMAC Offset: 0x20) Receive Status Register -------- */
#define AT91C_EMAC_BNA		((unsigned int) 0x1 <<  0) /* (EMAC) */
#define AT91C_EMAC_REC		((unsigned int) 0x1 <<  1) /* (EMAC) */
#define AT91C_EMAC_RSR_OVR	((unsigned int) 0x1 <<  2) /* (EMAC) */

/* -------- EMAC_ISR : (EMAC Offset: 0x24) Interrupt Status Register ------- */
#define AT91C_EMAC_DONE		((unsigned int) 0x1 <<  0) /* (EMAC) */
#define AT91C_EMAC_RCOM		((unsigned int) 0x1 <<  1) /* (EMAC) */
#define AT91C_EMAC_RBNA		((unsigned int) 0x1 <<  2) /* (EMAC) */
#define AT91C_EMAC_TOVR		((unsigned int) 0x1 <<  3) /* (EMAC) */
#define AT91C_EMAC_TUND		((unsigned int) 0x1 <<  4) /* (EMAC) */
#define AT91C_EMAC_RTRY		((unsigned int) 0x1 <<  5) /* (EMAC) */
#define AT91C_EMAC_TBRE		((unsigned int) 0x1 <<  6) /* (EMAC) */
#define AT91C_EMAC_TCOM		((unsigned int) 0x1 <<  7) /* (EMAC) */
#define AT91C_EMAC_TIDLE	((unsigned int) 0x1 <<  8) /* (EMAC) */
#define AT91C_EMAC_LINK		((unsigned int) 0x1 <<  9) /* (EMAC) */
#define AT91C_EMAC_ROVR		((unsigned int) 0x1 << 10) /* (EMAC) */
#define AT91C_EMAC_HRESP	((unsigned int) 0x1 << 11) /* (EMAC) */

/* -------- EMAC_IER : (EMAC Offset: 0x28) Interrupt Enable Register ------- */
/* -------- EMAC_IDR : (EMAC Offset: 0x2c) Interrupt Disable Register ------ */
/* -------- EMAC_IMR : (EMAC Offset: 0x30) Interrupt Mask Register -------- */
/* -------- EMAC_MAN : (EMAC Offset: 0x34) PHY Maintenance Register -------- */
#define AT91C_EMAC_DATA		((unsigned int) 0xFFFF <<  0) /* (EMAC) */
#define AT91C_EMAC_CODE		((unsigned int) 0x3  << 16) /* (EMAC) */
#define AT91C_EMAC_CODE_802_3	((unsigned int) 0x2  << 16) /* (EMAC) Write Operation */
#define AT91C_EMAC_REGA		((unsigned int) 0x1F << 18) /* (EMAC) */
#define AT91C_EMAC_PHYA		((unsigned int) 0x1F << 23) /* (EMAC) */
#define AT91C_EMAC_RW		((unsigned int) 0x3  << 28) /* (EMAC) */
#define AT91C_EMAC_RW_R		((unsigned int) 0x2  << 28) /* (EMAC) Read Operation */
#define AT91C_EMAC_RW_W		((unsigned int) 0x1  << 28) /* (EMAC) Write Operation */
#define AT91C_EMAC_HIGH		((unsigned int) 0x1  << 30) /* (EMAC) */
#define AT91C_EMAC_LOW		((unsigned int) 0x1  << 31) /* (EMAC) */

/*****************************************************************************/
/*           SOFTWARE API DEFINITION  FOR Serial Parallel Interface          */
/*****************************************************************************/
typedef struct _AT91S_SPI
{
	AT91_REG	 SPI_CR;	/* Control Register */
	AT91_REG	 SPI_MR;	/* Mode Register */
	AT91_REG	 SPI_RDR;	/* Receive Data Register */
	AT91_REG	 SPI_TDR;	/* Transmit Data Register */
	AT91_REG	 SPI_SR;	/* Status Register */
	AT91_REG	 SPI_IER;	/* Interrupt Enable Register */
	AT91_REG	 SPI_IDR;	/* Interrupt Disable Register */
	AT91_REG	 SPI_IMR;	/* Interrupt Mask Register */
	AT91_REG	 Reserved0[4];	/* */
	AT91_REG	 SPI_CSR[4];	/* Chip Select Register */
	AT91_REG	 Reserved1[48]; /* */
	AT91_REG	 SPI_RPR;	/* Receive Pointer Register */
	AT91_REG	 SPI_RCR;	/* Receive Counter Register */
	AT91_REG	 SPI_TPR;	/* Transmit Pointer Register */
	AT91_REG	 SPI_TCR;	/* Transmit Counter Register */
	AT91_REG	 SPI_RNPR;	/* Receive Next Pointer Register */
	AT91_REG	 SPI_RNCR;	/* Receive Next Counter Register */
	AT91_REG	 SPI_TNPR;	/* Transmit Next Pointer Register */
	AT91_REG	 SPI_TNCR;	/* Transmit Next Counter Register */
	AT91_REG	 SPI_PTCR;	/* PDC Transfer Control Register */
	AT91_REG	 SPI_PTSR;	/* PDC Transfer Status Register */
} AT91S_SPI, *AT91PS_SPI;

/* -------- SPI_CR : (SPI Offset: 0x0) SPI Control Register -------- */
#define AT91C_SPI_SPIEN		((unsigned int) 0x1 <<  0) /* (SPI) SPI Enable */
#define AT91C_SPI_SPIDIS	((unsigned int) 0x1 <<  1) /* (SPI) SPI Disable */
#define AT91C_SPI_SWRST		((unsigned int) 0x1 <<  7) /* (SPI) SPI Software reset */

/* -------- SPI_MR : (SPI Offset: 0x4) SPI Mode Register -------- */
#define AT91C_SPI_MSTR		((unsigned int) 0x1 <<  0) /* (SPI) Master/Slave Mode */
#define AT91C_SPI_PS		((unsigned int) 0x1 <<  1) /* (SPI) Peripheral Select */
#define AT91C_SPI_PS_FIXED	((unsigned int) 0x0 <<  1) /* (SPI) Fixed Peripheral Select */
#define AT91C_SPI_PS_VARIABLE	((unsigned int) 0x1 <<  1) /* (SPI) Variable Peripheral Select */
#define AT91C_SPI_PCSDEC	((unsigned int) 0x1 <<  2) /* (SPI) Chip Select Decode */
#define AT91C_SPI_DIV32		((unsigned int) 0x1 <<  3) /* (SPI) Clock Selection */
#define AT91C_SPI_MODFDIS	((unsigned int) 0x1 <<  4) /* (SPI) Mode Fault Detection */
#define AT91C_SPI_LLB		((unsigned int) 0x1 <<  7) /* (SPI) Clock Selection */
#define AT91C_SPI_PCS		((unsigned int) 0xF << 16) /* (SPI) Peripheral Chip Select */
#define AT91C_SPI_DLYBCS	((unsigned int) 0xFF << 24) /* (SPI) Delay Between Chip Selects */

/* -------- SPI_RDR : (SPI Offset: 0x8) Receive Data Register -------- */
#define AT91C_SPI_RD		((unsigned int) 0xFFFF <<  0) /* (SPI) Receive Data */
#define AT91C_SPI_RPCS		((unsigned int) 0xF << 16) /* (SPI) Peripheral Chip Select Status */

/* -------- SPI_TDR : (SPI Offset: 0xc) Transmit Data Register -------- */
#define AT91C_SPI_TD		((unsigned int) 0xFFFF <<  0) /* (SPI) Transmit Data */
#define AT91C_SPI_TPCS		((unsigned int) 0xF << 16) /* (SPI) Peripheral Chip Select Status */

/* -------- SPI_SR : (SPI Offset: 0x10) Status Register -------- */
#define AT91C_SPI_RDRF		((unsigned int) 0x1 <<  0) /* (SPI) Receive Data Register Full */
#define AT91C_SPI_TDRE		((unsigned int) 0x1 <<  1) /* (SPI) Transmit Data Register Empty */
#define AT91C_SPI_MODF		((unsigned int) 0x1 <<  2) /* (SPI) Mode Fault Error */
#define AT91C_SPI_OVRES		((unsigned int) 0x1 <<  3) /* (SPI) Overrun Error Status */
#define AT91C_SPI_SPENDRX	((unsigned int) 0x1 <<  4) /* (SPI) End of Receiver Transfer */
#define AT91C_SPI_SPENDTX	((unsigned int) 0x1 <<  5) /* (SPI) End of Receiver Transfer */
#define AT91C_SPI_RXBUFF	((unsigned int) 0x1 <<  6) /* (SPI) RXBUFF Interrupt */
#define AT91C_SPI_TXBUFE	((unsigned int) 0x1 <<  7) /* (SPI) TXBUFE Interrupt */
#define AT91C_SPI_SPIENS	((unsigned int) 0x1 << 16) /* (SPI) Enable Status */

/* -------- SPI_IER : (SPI Offset: 0x14) Interrupt Enable Register -------- */
/* -------- SPI_IDR : (SPI Offset: 0x18) Interrupt Disable Register ------- */
/* -------- SPI_IMR : (SPI Offset: 0x1c) Interrupt Mask Register -------- */
/* -------- SPI_CSR : (SPI Offset: 0x30) Chip Select Register -------- */
#define AT91C_SPI_CPOL		((unsigned int) 0x1  <<  0) /* (SPI) Clock Polarity */
#define AT91C_SPI_NCPHA		((unsigned int) 0x1  <<  1) /* (SPI) Clock Phase */
#define AT91C_SPI_BITS		((unsigned int) 0xF  <<  4) /* (SPI) Bits Per Transfer */
#define AT91C_SPI_BITS_8	((unsigned int) 0x0  <<  4) /* (SPI) 8 Bits Per transfer */
#define AT91C_SPI_BITS_9	((unsigned int) 0x1  <<  4) /* (SPI) 9 Bits Per transfer */
#define AT91C_SPI_BITS_10	((unsigned int) 0x2  <<  4) /* (SPI) 10 Bits Per transfer */
#define AT91C_SPI_BITS_11	((unsigned int) 0x3  <<  4) /* (SPI) 11 Bits Per transfer */
#define AT91C_SPI_BITS_12	((unsigned int) 0x4  <<  4) /* (SPI) 12 Bits Per transfer */
#define AT91C_SPI_BITS_13	((unsigned int) 0x5  <<  4) /* (SPI) 13 Bits Per transfer */
#define AT91C_SPI_BITS_14	((unsigned int) 0x6  <<  4) /* (SPI) 14 Bits Per transfer */
#define AT91C_SPI_BITS_15	((unsigned int) 0x7  <<  4) /* (SPI) 15 Bits Per transfer */
#define AT91C_SPI_BITS_16	((unsigned int) 0x8  <<  4) /* (SPI) 16 Bits Per transfer */
#define AT91C_SPI_SCBR		((unsigned int) 0xFF <<  8) /* (SPI) Serial Clock Baud Rate */
#define AT91C_SPI_DLYBS		((unsigned int) 0xFF << 16) /* (SPI) Serial Clock Baud Rate */
#define AT91C_SPI_DLYBCT	((unsigned int) 0xFF << 24) /* (SPI) Delay Between Consecutive Transfers */

/*****************************************************************************/
/*           SOFTWARE API DEFINITION  FOR Peripheral Data Controller         */
/*****************************************************************************/
typedef struct _AT91S_PDC
{
	AT91_REG	 PDC_RPR;	/* Receive Pointer Register */
	AT91_REG	 PDC_RCR;	/* Receive Counter Register */
	AT91_REG	 PDC_TPR;	/* Transmit Pointer Register */
	AT91_REG	 PDC_TCR;	/* Transmit Counter Register */
	AT91_REG	 PDC_RNPR;	/* Receive Next Pointer Register */
	AT91_REG	 PDC_RNCR;	/* Receive Next Counter Register */
	AT91_REG	 PDC_TNPR;	/* Transmit Next Pointer Register */
	AT91_REG	 PDC_TNCR;	/* Transmit Next Counter Register */
	AT91_REG	 PDC_PTCR;	/* PDC Transfer Control Register */
	AT91_REG	 PDC_PTSR;	/* PDC Transfer Status Register */
} AT91S_PDC, *AT91PS_PDC;

/* -------- PDC_PTCR : (PDC Offset: 0x20) PDC Transfer Control Register -------- */
#define AT91C_PDC_RXTEN		((unsigned int) 0x1 <<  0) /* (PDC) Receiver Transfer Enable */
#define AT91C_PDC_RXTDIS	((unsigned int) 0x1 <<  1) /* (PDC) Receiver Transfer Disable */
#define AT91C_PDC_TXTEN		((unsigned int) 0x1 <<  8) /* (PDC) Transmitter Transfer Enable */
#define AT91C_PDC_TXTDIS	((unsigned int) 0x1 <<  9) /* (PDC) Transmitter Transfer Disable */
/* -------- PDC_PTSR : (PDC Offset: 0x24) PDC Transfer Status Register --------  */

/* ========== Register definition ==================================== */
#define AT91C_SPI_CSR		((AT91_REG *)	0xFFFE0030) /* (SPI) Chip Select Register */
#define AT91C_PMC_PCER		((AT91_REG *)	0xFFFFFC10) /* (PMC) Peripheral Clock Enable Register */
#define AT91C_PMC_PCDR		((AT91_REG *)	0xFFFFFC14) /* (PMC) Peripheral Clock Enable Register */
#define AT91C_PMC_SCER		((AT91_REG *)	0xFFFFFC00) /* (PMC) Peripheral Clock Enable Register */
#define AT91C_PMC_SCDR		((AT91_REG *)	0xFFFFFC04) /* (PMC) Peripheral Clock Enable Register */
#define AT91C_PIOA_PER		((AT91_REG *)	0xFFFFF400) /* (PIOA) PIO Enable Register */
#define AT91C_PIOA_PDR		((AT91_REG *)	0xFFFFF404) /* (PIOA) PIO Disable Register */
#define AT91C_PIOA_PSR		((AT91_REG *)	0xFFFFF408) /* (PIOA) PIO Status Register */
#define AT91C_PIOA_OER		((AT91_REG *)	0xFFFFF410) /* (PIOA) PIO Output Enable Register */
#define AT91C_PIOA_ODR		((AT91_REG *)	0xFFFFF414) /* (PIOA) PIO Output Disable Register */
#define AT91C_PIOA_OSR		((AT91_REG *)	0xFFFFF418) /* (PIOA) PIO Output Status Register */
#define AT91C_PIOA_IFER		((AT91_REG *)	0xFFFFF420) /* (PIOA) PIO Glitch Input Filter Enable Register */
#define AT91C_PIOA_IFDR		((AT91_REG *)	0xFFFFF424) /* (PIOA) PIO Glitch Input Filter Disable Register */
#define AT91C_PIOA_IFSR		((AT91_REG *)	0xFFFFF428) /* (PIOA) PIO Glitch Input Filter Status Register */
#define AT91C_PIOA_SODR		((AT91_REG *)	0xFFFFF430) /* (PIOA) PIO Set Output Data Register */
#define AT91C_PIOA_CODR		((AT91_REG *)	0xFFFFF434) /* (PIOA) PIO Clear Output Data Register */
#define AT91C_PIOA_ODSR		((AT91_REG *)	0xFFFFF438) /* (PIOA) PIO Output Data Status Register */
#define AT91C_PIOA_PDSR		((AT91_REG *)	0xFFFFF43C) /* (PIOA) PIO Pin Data Status Register */
#define AT91C_PIOA_IER		((AT91_REG *)	0xFFFFF440) /* (PIOA) PIO Interrupt Enable Register */
#define AT91C_PIOA_IDR		((AT91_REG *)	0xFFFFF444) /* (PIOA) PIO Interrupt Disable Register */
#define AT91C_PIOA_IMR		((AT91_REG *)	0xFFFFF448) /* (PIOA) PIO Interrupt Mask Register */
#define AT91C_PIOA_ISR		((AT91_REG *)	0xFFFFF44C) /* (PIOA) PIO Interrupt Status Register */
#define AT91C_PIOA_MDER		((AT91_REG *)	0xFFFFF450) /* (PIOA) PIO Multi-drive Enable Register */
#define AT91C_PIOA_MDDR		((AT91_REG *)	0xFFFFF454) /* (PIOA) PIO Multi-drive Disable Register */
#define AT91C_PIOA_MDSR		((AT91_REG *)	0xFFFFF458) /* (PIOA) PIO Multi-drive Status Register */
#define AT91C_PIOA_PUDR		((AT91_REG *)	0xFFFFF460) /* (PIOA) PIO Pull-up Disable Register */
#define AT91C_PIOA_PUER		((AT91_REG *)	0xFFFFF464) /* (PIOA) PIO Pull-up Enable Register */
#define AT91C_PIOA_PUSR		((AT91_REG *)	0xFFFFF468) /* (PIOA) PIO Pull-up Status Register */
#define AT91C_PIOA_ASR		((AT91_REG *)	0xFFFFF470) /* (PIOA) PIO Peripheral A Select Register */
#define AT91C_PIOA_BSR		((AT91_REG *)	0xFFFFF474) /* (PIOA) PIO Peripheral B Select Register */
#define AT91C_PIOA_ABSR		((AT91_REG *)	0xFFFFF478) /* (PIOA) PIO Peripheral AB Select Register */
#define AT91C_PIOA_OWER		((AT91_REG *)	0xFFFFF4A0) /* (PIOA) PIO Output Write Enable Register */
#define AT91C_PIOA_OWDR		((AT91_REG *)	0xFFFFF4A4) /* (PIOA) PIO Output Write Disable Register */
#define AT91C_PIOA_OWSR		((AT91_REG *)	0xFFFFF4A8) /* (PIOA) PIO Output Write Status Register */
#define AT91C_PIOB_PDR		((AT91_REG *)	0xFFFFF604) /* (PIOB) PIO Disable Register */

#define AT91C_PIO_PA30		((unsigned int) 1 << 30)	/* Pin Controlled by PA30 */
#define AT91C_PIO_PC0		((unsigned int) 1 <<  0)	/* Pin Controlled by PC0 */
#define AT91C_PC0_BFCK		((unsigned int) AT91C_PIO_PC0)	/* Burst Flash Clock */
#define AT91C_PA30_DRXD		((unsigned int) AT91C_PIO_PA30)	/* DBGU Debug Receive Data */
#define AT91C_PIO_PA31		((unsigned int) 1 << 31)	/* Pin Controlled by PA31 */
#define AT91C_PA25_TWD		((unsigned int) 1 << 25)
#define AT91C_PA26_TWCK		((unsigned int) 1 << 26)
#define AT91C_PA31_DTXD		((unsigned int) AT91C_PIO_PA31)	/* DBGU Debug Transmit Data */
#define AT91C_PIO_PA17		((unsigned int) 1 << 17)	/* Pin Controlled by PA17 */
#define AT91C_PA17_TXD0		AT91C_PIO_PA17			/* USART0 Transmit Data */
#define AT91C_PIO_PA18		((unsigned int) 1 << 18)	/* Pin Controlled by PA18 */
#define AT91C_PA18_RXD0		AT91C_PIO_PA18			/* USART0 Receive Data */
#define AT91C_PIO_PB20		((unsigned int) 1 << 20)	/* Pin Controlled by PB20 */
#define AT91C_PB20_RXD1		AT91C_PIO_PB20			/* USART1 Receive Data */
#define AT91C_PIO_PB21		((unsigned int) 1 << 21)	/* Pin Controlled by PB21 */
#define AT91C_PB21_TXD1		AT91C_PIO_PB21			/* USART1 Transmit Data */

#define AT91C_ID_SYS		((unsigned int)  1) /* System Peripheral */
#define AT91C_ID_PIOA		((unsigned int)  2) /* PIO port A */
#define AT91C_ID_PIOB		((unsigned int)  3) /* PIO port B */
#define AT91C_ID_PIOC		((unsigned int)  4) /* PIO port C */
#define AT91C_ID_USART0		((unsigned int)  6) /* USART 0 */
#define AT91C_ID_USART1		((unsigned int)  7) /* USART 1 */
#define AT91C_ID_TWI		((unsigned int) 12) /* Two Wire Interface */
#define AT91C_ID_SPI		((unsigned int) 13) /* Serial Peripheral Interface */
#define AT91C_ID_TC0		((unsigned int) 17) /* Timer Counter 0 */
#define AT91C_ID_UHP		((unsigned int) 23) /* OHCI USB Host Port */
#define AT91C_ID_EMAC		((unsigned int) 24) /* Ethernet MAC */

#define AT91C_PIO_PC1		((unsigned int) 1 <<  1)	/* Pin Controlled by PC1 */
#define AT91C_PC1_BFRDY_SMOE	((unsigned int) AT91C_PIO_PC1)	/*  Burst Flash Ready */
#define AT91C_PIO_PC3		((unsigned int) 1 <<  3)	/* Pin Controlled by PC3 */
#define AT91C_PC3_BFBAA_SMWE	((unsigned int) AT91C_PIO_PC3)	/*  Burst Flash Address Advance / SmartMedia Write Enable */
#define AT91C_PIO_PC2		((unsigned int) 1 <<  2)	/* Pin Controlled by PC2 */
#define AT91C_PC2_BFAVD		((unsigned int) AT91C_PIO_PC2)	/*  Burst Flash Address Valid */
#define AT91C_PIO_PB1		((unsigned int) 1 <<  1)	/* Pin Controlled by PB1 */

#define AT91C_PIO_PA23		((unsigned int) 1 << 23)	/* Pin Controlled by PA23 */
#define AT91C_PA23_TXD2		((unsigned int) AT91C_PIO_PA23)	/* USART 2 Transmit Data */

#define AT91C_PIO_PA0		((unsigned int) 1 <<  0)	/* Pin Controlled by PA0 */
#define AT91C_PA0_MISO		((unsigned int) AT91C_PIO_PA0)	/* SPI Master In Slave */
#define AT91C_PIO_PA1		((unsigned int) 1 <<  1)	/* Pin Controlled by PA1 */
#define AT91C_PA1_MOSI		((unsigned int) AT91C_PIO_PA1)	/* SPI Master Out Slave */
#define AT91C_PIO_PA2		((unsigned int) 1 <<  2)	/* Pin Controlled by PA2 */
#define AT91C_PA2_SPCK		((unsigned int) AT91C_PIO_PA2)	/* SPI Serial Clock */
#define AT91C_PIO_PA3		((unsigned int) 1 <<  3)	/* Pin Controlled by PA3 */
#define AT91C_PA3_NPCS0		((unsigned int) AT91C_PIO_PA3)	/* SPI Peripheral Chip Select 0 */
#define AT91C_PIO_PA4		((unsigned int) 1 <<  4)	/* Pin Controlled by PA4 */
#define AT91C_PA4_NPCS1		((unsigned int) AT91C_PIO_PA4)	/* SPI Peripheral Chip Select 1 */
#define AT91C_PIO_PA5		((unsigned int) 1 <<  5)	/* Pin Controlled by PA5 */
#define AT91C_PA5_NPCS2		((unsigned int) AT91C_PIO_PA5)	/* SPI Peripheral Chip Select 2 */
#define AT91C_PIO_PA6		((unsigned int) 1 <<  6)	/* Pin Controlled by PA6 */
#define AT91C_PA6_NPCS3		((unsigned int) AT91C_PIO_PA6)	/* SPI Peripheral Chip Select 3 */

#define AT91C_PIO_PA16		((unsigned int) 1 << 16)	/* Pin Controlled by PA16 */
#define AT91C_PA16_EMDIO	((unsigned int) AT91C_PIO_PA16)	/* Ethernet MAC Management Data Input/Output */
#define AT91C_PIO_PA15		((unsigned int) 1 << 15)	/* Pin Controlled by PA15 */
#define AT91C_PA15_EMDC		((unsigned int) AT91C_PIO_PA15)	/* Ethernet MAC Management Data Clock */
#define AT91C_PIO_PA14		((unsigned int) 1 << 14)	/* Pin Controlled by PA14 */
#define AT91C_PA14_ERXER	((unsigned int) AT91C_PIO_PA14)	/* Ethernet MAC Receive Error */
#define AT91C_PIO_PA13		((unsigned int) 1 << 13)	/* Pin Controlled by PA13 */
#define AT91C_PA13_ERX1		((unsigned int) AT91C_PIO_PA13)	/* Ethernet MAC Receive Data 1 */
#define AT91C_PIO_PA12		((unsigned int) 1 << 12)	/* Pin Controlled by PA12 */
#define AT91C_PA12_ERX0		((unsigned int) AT91C_PIO_PA12)	/* Ethernet MAC Receive Data 0 */
#define AT91C_PIO_PA11		((unsigned int) 1 << 11)	/* Pin Controlled by PA11 */
#define AT91C_PA11_ECRS_ECRSDV	((unsigned int) AT91C_PIO_PA11)	/* Ethernet MAC Carrier Sense/Carrier Sense and Data Valid */
#define AT91C_PIO_PA10		((unsigned int) 1 << 10)	/* Pin Controlled by PA10 */
#define AT91C_PA10_ETX1		((unsigned int) AT91C_PIO_PA10)	/* Ethernet MAC Transmit Data 1 */
#define AT91C_PIO_PA9		((unsigned int) 1 <<  9)	/* Pin Controlled by PA9 */
#define AT91C_PA9_ETX0		((unsigned int) AT91C_PIO_PA9)	/* Ethernet MAC Transmit Data 0 */
#define AT91C_PIO_PA8		((unsigned int) 1 <<  8)	/* Pin Controlled by PA8 */
#define AT91C_PA8_ETXEN		((unsigned int) AT91C_PIO_PA8)	/* Ethernet MAC Transmit Enable */
#define AT91C_PIO_PA7		((unsigned int) 1 <<  7)	/* Pin Controlled by PA7 */
#define AT91C_PA7_ETXCK_EREFCK	((unsigned int) AT91C_PIO_PA7)	/* Ethernet MAC Transmit Clock/Reference Clock */

#define AT91C_PIO_PB0		((unsigned int) 1 <<  0)	/* Pin Controlled by PB3 */
#define AT91C_PIO_PB1		((unsigned int) 1 <<  1)	/* Pin Controlled by PB3 */
#define AT91C_PIO_PB2		((unsigned int) 1 <<  2)	/* Pin Controlled by PB3 */
#define AT91C_PIO_PB3		((unsigned int) 1 <<  3)	/* Pin Controlled by PB3 */
#define AT91C_PIO_PB4		((unsigned int) 1 <<  4)	/* Pin Controlled by PB4 */
#define AT91C_PIO_PB5		((unsigned int) 1 <<  5)	/* Pin Controlled by PB5 */
#define AT91C_PIO_PB6		((unsigned int) 1 <<  6)	/* Pin Controlled by PB6 */
#define AT91C_PIO_PB7		((unsigned int) 1 <<  7)	/* Pin Controlled by PB7 */
#define AT91C_PIO_PB22		((unsigned int) 1 << 22)	/* Pin Controlled by PB22 */
#define AT91C_PIO_PB25		((unsigned int) 1 << 25)	/* Pin Controlled by PB25 */
#define AT91C_PB25_DSR1		((unsigned int) AT91C_PIO_PB25)	/* USART 1 Data Set ready */
#define AT91C_PB25_EF100	((unsigned int) AT91C_PIO_PB25)	/* Ethernet MAC Force 100 Mbits */
#define AT91C_PIO_PB19		((unsigned int) 1 << 19)	/* Pin Controlled by PB19 */
#define AT91C_PB19_DTR1		((unsigned int) AT91C_PIO_PB19)	/* USART 1 Data Terminal ready */
#define AT91C_PB19_ERXCK	((unsigned int) AT91C_PIO_PB19)	/* Ethernet MAC Receive Clock */
#define AT91C_PIO_PB18		((unsigned int) 1 << 18)	/* Pin Controlled by PB18 */
#define AT91C_PB18_RI1		((unsigned int) AT91C_PIO_PB18)	/* USART 1 Ring Indicator */
#define AT91C_PB18_ECOL		((unsigned int) AT91C_PIO_PB18)	/* Ethernet MAC Collision Detected */
#define AT91C_PIO_PB17		((unsigned int) 1 << 17)	/* Pin Controlled by PB17 */
#define AT91C_PB17_RF2		((unsigned int) AT91C_PIO_PB17)	/* SSC Receive Frame Sync 2 */
#define AT91C_PB17_ERXDV	((unsigned int) AT91C_PIO_PB17)	/* Ethernet MAC Receive Data Valid */
#define AT91C_PIO_PB16		((unsigned int) 1 << 16)	/* Pin Controlled by PB16 */
#define AT91C_PB16_RK2		((unsigned int) AT91C_PIO_PB16)	/* SSC Receive Clock 2 */
#define AT91C_PB16_ERX3		((unsigned int) AT91C_PIO_PB16)	/* Ethernet MAC Receive Data 3 */
#define AT91C_PIO_PB15		((unsigned int) 1 << 15)	/* Pin Controlled by PB15 */
#define AT91C_PB15_RD2		((unsigned int) AT91C_PIO_PB15)	/* SSC Receive Data 2 */
#define AT91C_PB15_ERX2		((unsigned int) AT91C_PIO_PB15)	/* Ethernet MAC Receive Data 2 */
#define AT91C_PIO_PB14		((unsigned int) 1 << 14)	/* Pin Controlled by PB14 */
#define AT91C_PB14_TD2		((unsigned int) AT91C_PIO_PB14)	/* SSC Transmit Data 2 */
#define AT91C_PB14_ETXER	((unsigned int) AT91C_PIO_PB14)	/* Ethernet MAC Transmikt Coding Error */
#define AT91C_PIO_PB13		((unsigned int) 1 << 13)	/* Pin Controlled by PB13 */
#define AT91C_PB13_TK2		((unsigned int) AT91C_PIO_PB13)	/* SSC Transmit Clock 2 */
#define AT91C_PB13_ETX3		((unsigned int) AT91C_PIO_PB13)	/* Ethernet MAC Transmit Data 3 */
#define AT91C_PIO_PB12		((unsigned int) 1 << 12)	/* Pin Controlled by PB12 */
#define AT91C_PB12_TF2		((unsigned int) AT91C_PIO_PB12)	/* SSC Transmit Frame Sync 2 */
#define AT91C_PB12_ETX2		((unsigned int) AT91C_PIO_PB12)	/* Ethernet MAC Transmit Data 2 */

#define AT91C_PIOB_BSR		((AT91_REG *)	0xFFFFF674)	/* (PIOB) Select B Register */
#define AT91C_PIOB_PDR		((AT91_REG *)	0xFFFFF604)	/* (PIOB) PIO Disable Register */

#define AT91C_EBI_CS3A_SMC_SmartMedia	((unsigned int) 0x1 <<  3) /* (EBI) Chip Select 3 is assigned to the Static Memory Controller and the SmartMedia Logic is activated. */
#define AT91C_SMC2_ACSS_STANDARD	((unsigned int) 0x0 << 16) /* (SMC2) Standard, asserted at the beginning of the access and deasserted at the end. */
#define AT91C_SMC2_DBW_8	((unsigned int) 0x2 << 13) /* (SMC2) 8-bit. */
#define AT91C_SMC2_WSEN		((unsigned int) 0x1 <<  7) /* (SMC2) Wait State Enable */
#define AT91C_PIOC_ASR		((AT91_REG *)	0xFFFFF870) /* (PIOC) Select A Register */
#define AT91C_PIOC_SODR		((AT91_REG *)	0xFFFFF830) /* (PIOC) Set Output Data Register */
#define AT91C_PIOC_CODR		((AT91_REG *)	0xFFFFF834) /* (PIOC) Clear Output Data Register */
#define AT91C_PIOC_PDSR		((AT91_REG *)	0xFFFFF83C) /* (PIOC) Pin Data Status Register */

#define AT91C_BASE_AIC		((AT91PS_AIC)	0xFFFFF000) /* (AIC) Base Address */
#define AT91C_BASE_DBGU		((AT91PS_DBGU)	0xFFFFF200) /* (DBGU) Base Address */
#define AT91C_BASE_PIOA		((AT91PS_PIO)	0xFFFFF400) /* (PIOA) Base Address */
#define AT91C_BASE_PIOB		((AT91PS_PIO)	0xFFFFF600) /* (PIOB) Base Address */
#define AT91C_BASE_PIOC		((AT91PS_PIO)	0xFFFFF800) /* (PIOC) Base Address */
#define AT91C_BASE_PIOD		((AT91PS_PIO)	0xFFFFFA00) /* (PIOC) Base Address */
#define AT91C_BASE_PMC		((AT91PS_PMC)	0xFFFFFC00) /* (PMC) Base Address */
#if	0
#define AT91C_BASE_ST		((AT91PS_ST)	0xFFFFFD00) /* (PMC) Base Address */
#define AT91C_BASE_RTC		((AT91PS_RTC)	0xFFFFFE00) /* (PMC) Base Address */
#define AT91C_BASE_MC		((AT91PS_MC)	0xFFFFFF00) /* (PMC) Base Address */
#endif

#define AT91C_BASE_TC0		((AT91PS_TC)	0xFFFA0000) /* (TC0) Base Address */
#define AT91C_BASE_TC1		((AT91PS_TC)	0xFFFA4000) /* (TC0) Base Address */
#if	0
#define AT91C_BASE_UDP		((AT91PS_UDP)	0xFFFB0000) /* (TC0) Base Address */
#define AT91C_BASE_MCI		((AT91PS_MCI)	0xFFFB4000) /* (TC0) Base Address */
#define AT91C_BASE_TWI		((AT91PS_TWI)	0xFFFB8000) /* (TC0) Base Address */
#endif
#define AT91C_BASE_EMAC		((AT91PS_EMAC)	0xFFFBC000) /* (EMAC) Base Address */
#define AT91C_BASE_US0		((AT91PS_USART)	0xFFFC0000) /* (US0) Base Address */
#define AT91C_BASE_US1		((AT91PS_USART) 0xFFFC4000) /* (US1) Base Address */
#define AT91C_BASE_US2		((AT91PS_USART) 0xFFFC8000) /* (US1) Base Address */
#define AT91C_BASE_US3		((AT91PS_USART) 0xFFFCC000) /* (US1) Base Address */
#define AT91C_BASE_SPI		((AT91PS_SPI)	0xFFFE0000) /* (SPI) Base Address */

#define AT91C_BASE_CKGR		((AT91PS_CKGR)	0xFFFFFC20) /* (CKGR) Base Address */
#define AT91C_EBI_CSA		((AT91_REG *)	0xFFFFFF60) /* (EBI) Chip Select Assignment Register */
#define AT91C_BASE_SMC2		((AT91PS_SMC2)	0xFFFFFF70) /* (SMC2) Base Address */
#define AT91C_TCB0_BMR		((AT91_REG *)	0xFFFA00C4) /* (TCB0) TC Block Mode Register */
#define AT91C_TCB0_BCR		((AT91_REG *)	0xFFFA00C0) /* (TCB0) TC Block Control Register */
#define AT91C_PIOC_PDR		((AT91_REG *)	0xFFFFF804) /* (PIOC) PIO Disable Register */
#define AT91C_PIOC_PER		((AT91_REG *)	0xFFFFF800) /* (PIOC) PIO Enable Register */
#define AT91C_PIOC_ODR		((AT91_REG *)	0xFFFFF814) /* (PIOC) Output Disable Registerr */
#define AT91C_PIOB_PER		((AT91_REG *)	0xFFFFF600) /* (PIOB) PIO Enable Register */
#define AT91C_PIOB_ODR		((AT91_REG *)	0xFFFFF614) /* (PIOB) Output Disable Registerr */
#define AT91C_PIOB_PDSR		((AT91_REG *)	0xFFFFF63C) /* (PIOB) Pin Data Status Register */

#endif /* __ASSEMBLY__ */
#endif /* AT91RM9200_H */
