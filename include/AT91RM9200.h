/*  ---------------------------------------------------------------------------- */
/*           ATMEL Microcontroller Software Support  -  ROUSSET  - */
/*  ---------------------------------------------------------------------------- */
/*   The software is delivered "AS IS" without warranty or condition of any */
/*   kind, either express, implied or statutory. This includes without */
/*   limitation any warranty or condition with respect to merchantability or */
/*   fitness for any particular purpose, or against the infringements of */
/*   intellectual property rights of others. */
/*  ---------------------------------------------------------------------------- */
/*  File Name           : AT91RM9200.h */
/*  Object              : AT91RM9200 definitions */
/*  Generated           : AT91 SW Application Group  10/29/2002 (16:10:51) */
#ifndef AT91RM9200_H
#define AT91RM9200_H

typedef volatile unsigned int AT91_REG;/*  Hardware register definition */

/*  ***************************************************************************** */
/*               SOFTWARE API DEFINITION  FOR Timer Counter Channel Interface */
/*  ***************************************************************************** */
typedef struct _AT91S_TC {
	AT91_REG	 TC_CCR; 	/*  Channel Control Register */
	AT91_REG	 TC_CMR; 	/*  Channel Mode Register */
	AT91_REG	 Reserved0[2]; 	/*   */
	AT91_REG	 TC_CV; 	/*  Counter Value */
	AT91_REG	 TC_RA; 	/*  Register A */
	AT91_REG	 TC_RB; 	/*  Register B */
	AT91_REG	 TC_RC; 	/*  Register C */
	AT91_REG	 TC_SR; 	/*  Status Register */
	AT91_REG	 TC_IER; 	/*  Interrupt Enable Register */
	AT91_REG	 TC_IDR; 	/*  Interrupt Disable Register */
	AT91_REG	 TC_IMR; 	/*  Interrupt Mask Register */
} AT91S_TC, *AT91PS_TC;

/*  ***************************************************************************** */
/*               SOFTWARE API DEFINITION  FOR Usart */
/*  ***************************************************************************** */
typedef struct _AT91S_USART {
	AT91_REG	 US_CR; 	/*  Control Register */
	AT91_REG	 US_MR; 	/*  Mode Register */
	AT91_REG	 US_IER; 	/*  Interrupt Enable Register */
	AT91_REG	 US_IDR; 	/*  Interrupt Disable Register */
	AT91_REG	 US_IMR; 	/*  Interrupt Mask Register */
	AT91_REG	 US_CSR; 	/*  Channel Status Register */
	AT91_REG	 US_RHR; 	/*  Receiver Holding Register */
	AT91_REG	 US_THR; 	/*  Transmitter Holding Register */
	AT91_REG	 US_BRGR; 	/*  Baud Rate Generator Register */
	AT91_REG	 US_RTOR; 	/*  Receiver Time-out Register */
	AT91_REG	 US_TTGR; 	/*  Transmitter Time-guard Register */
	AT91_REG	 Reserved0[5]; 	/*   */
	AT91_REG	 US_FIDI; 	/*  FI_DI_Ratio Register */
	AT91_REG	 US_NER; 	/*  Nb Errors Register */
	AT91_REG	 US_XXR; 	/*  XON_XOFF Register */
	AT91_REG	 US_IF; 	/*  IRDA_FILTER Register */
	AT91_REG	 Reserved1[44]; 	/*   */
	AT91_REG	 US_RPR; 	/*  Receive Pointer Register */
	AT91_REG	 US_RCR; 	/*  Receive Counter Register */
	AT91_REG	 US_TPR; 	/*  Transmit Pointer Register */
	AT91_REG	 US_TCR; 	/*  Transmit Counter Register */
	AT91_REG	 US_RNPR; 	/*  Receive Next Pointer Register */
	AT91_REG	 US_RNCR; 	/*  Receive Next Counter Register */
	AT91_REG	 US_TNPR; 	/*  Transmit Next Pointer Register */
	AT91_REG	 US_TNCR; 	/*  Transmit Next Counter Register */
	AT91_REG	 US_PTCR; 	/*  PDC Transfer Control Register */
	AT91_REG	 US_PTSR; 	/*  PDC Transfer Status Register */
} AT91S_USART, *AT91PS_USART;

/*  ***************************************************************************** */
/*               SOFTWARE API DEFINITION  FOR Parallel Input Output Controler */
/*  ***************************************************************************** */
typedef struct _AT91S_PIO {
	AT91_REG	 PIO_PER; 	/*  PIO Enable Register */
	AT91_REG	 PIO_PDR; 	/*  PIO Disable Register */
	AT91_REG	 PIO_PSR; 	/*  PIO Status Register */
	AT91_REG	 Reserved0[1]; 	/*   */
	AT91_REG	 PIO_OER; 	/*  Output Enable Register */
	AT91_REG	 PIO_ODR; 	/*  Output Disable Registerr */
	AT91_REG	 PIO_OSR; 	/*  Output Status Register */
	AT91_REG	 Reserved1[1]; 	/*   */
	AT91_REG	 PIO_IFER; 	/*  Input Filter Enable Register */
	AT91_REG	 PIO_IFDR; 	/*  Input Filter Disable Register */
	AT91_REG	 PIO_IFSR; 	/*  Input Filter Status Register */
	AT91_REG	 Reserved2[1]; 	/*   */
	AT91_REG	 PIO_SODR; 	/*  Set Output Data Register */
	AT91_REG	 PIO_CODR; 	/*  Clear Output Data Register */
	AT91_REG	 PIO_ODSR; 	/*  Output Data Status Register */
	AT91_REG	 PIO_PDSR; 	/*  Pin Data Status Register */
	AT91_REG	 PIO_IER; 	/*  Interrupt Enable Register */
	AT91_REG	 PIO_IDR; 	/*  Interrupt Disable Register */
	AT91_REG	 PIO_IMR; 	/*  Interrupt Mask Register */
	AT91_REG	 PIO_ISR; 	/*  Interrupt Status Register */
	AT91_REG	 PIO_MDER; 	/*  Multi-driver Enable Register */
	AT91_REG	 PIO_MDDR; 	/*  Multi-driver Disable Register */
	AT91_REG	 PIO_MDSR; 	/*  Multi-driver Status Register */
	AT91_REG	 Reserved3[1]; 	/*   */
	AT91_REG	 PIO_PPUDR; 	/*  Pull-up Disable Register */
	AT91_REG	 PIO_PPUER; 	/*  Pull-up Enable Register */
	AT91_REG	 PIO_PPUSR; 	/*  Pad Pull-up Status Register */
	AT91_REG	 Reserved4[1]; 	/*   */
	AT91_REG	 PIO_ASR; 	/*  Select A Register */
	AT91_REG	 PIO_BSR; 	/*  Select B Register */
	AT91_REG	 PIO_ABSR; 	/*  AB Select Status Register */
	AT91_REG	 Reserved5[9]; 	/*   */
	AT91_REG	 PIO_OWER; 	/*  Output Write Enable Register */
	AT91_REG	 PIO_OWDR; 	/*  Output Write Disable Register */
	AT91_REG	 PIO_OWSR; 	/*  Output Write Status Register */
} AT91S_PIO, *AT91PS_PIO;


/*  ***************************************************************************** */
/*               SOFTWARE API DEFINITION  FOR Debug Unit */
/*  ***************************************************************************** */
typedef struct _AT91S_DBGU {
	AT91_REG	 DBGU_CR; 	/*  Control Register */
	AT91_REG	 DBGU_MR; 	/*  Mode Register */
	AT91_REG	 DBGU_IER; 	/*  Interrupt Enable Register */
	AT91_REG	 DBGU_IDR; 	/*  Interrupt Disable Register */
	AT91_REG	 DBGU_IMR; 	/*  Interrupt Mask Register */
	AT91_REG	 DBGU_CSR; 	/*  Channel Status Register */
	AT91_REG	 DBGU_RHR; 	/*  Receiver Holding Register */
	AT91_REG	 DBGU_THR; 	/*  Transmitter Holding Register */
	AT91_REG	 DBGU_BRGR; 	/*  Baud Rate Generator Register */
	AT91_REG	 Reserved0[7]; 	/*   */
	AT91_REG	 DBGU_C1R; 	/*  Chip ID1 Register */
	AT91_REG	 DBGU_C2R; 	/*  Chip ID2 Register */
	AT91_REG	 DBGU_FNTR; 	/*  Force NTRST Register */
	AT91_REG	 Reserved1[45]; 	/*   */
	AT91_REG	 DBGU_RPR; 	/*  Receive Pointer Register */
	AT91_REG	 DBGU_RCR; 	/*  Receive Counter Register */
	AT91_REG	 DBGU_TPR; 	/*  Transmit Pointer Register */
	AT91_REG	 DBGU_TCR; 	/*  Transmit Counter Register */
	AT91_REG	 DBGU_RNPR; 	/*  Receive Next Pointer Register */
	AT91_REG	 DBGU_RNCR; 	/*  Receive Next Counter Register */
	AT91_REG	 DBGU_TNPR; 	/*  Transmit Next Pointer Register */
	AT91_REG	 DBGU_TNCR; 	/*  Transmit Next Counter Register */
	AT91_REG	 DBGU_PTCR; 	/*  PDC Transfer Control Register */
	AT91_REG	 DBGU_PTSR; 	/*  PDC Transfer Status Register */
} AT91S_DBGU, *AT91PS_DBGU;


/*  ***************************************************************************** */
/*               SOFTWARE API DEFINITION  FOR Static Memory Controller 2 Interface */
/*  ***************************************************************************** */
typedef struct _AT91S_SMC2 {
	AT91_REG	 SMC2_CSR[8]; 	/*  SMC2 Chip Select Register */
} AT91S_SMC2, *AT91PS_SMC2;

/*  ***************************************************************************** */
/*               SOFTWARE API DEFINITION  FOR Ethernet MAC */
/*  ***************************************************************************** */
typedef struct _AT91S_EMAC {
	AT91_REG	 EMAC_CTL; 	/*  Network Control Register */
	AT91_REG	 EMAC_CFG; 	/*  Network Configuration Register */
	AT91_REG	 EMAC_SR; 	/*  Network Status Register */
	AT91_REG	 EMAC_TAR; 	/*  Transmit Address Register */
	AT91_REG	 EMAC_TCR; 	/*  Transmit Control Register */
	AT91_REG	 EMAC_TSR; 	/*  Transmit Status Register */
	AT91_REG	 EMAC_RBQP; 	/*  Receive Buffer Queue Pointer */
	AT91_REG	 Reserved0[1]; 	/*   */
	AT91_REG	 EMAC_RSR; 	/*  Receive Status Register */
	AT91_REG	 EMAC_ISR; 	/*  Interrupt Status Register */
	AT91_REG	 EMAC_IER; 	/*  Interrupt Enable Register */
	AT91_REG	 EMAC_IDR; 	/*  Interrupt Disable Register */
	AT91_REG	 EMAC_IMR; 	/*  Interrupt Mask Register */
	AT91_REG	 EMAC_MAN; 	/*  PHY Maintenance Register */
	AT91_REG	 Reserved1[2]; 	/*   */
	AT91_REG	 EMAC_FRA; 	/*  Frames Transmitted OK Register */
	AT91_REG	 EMAC_SCOL; 	/*  Single Collision Frame Register */
	AT91_REG	 EMAC_MCOL; 	/*  Multiple Collision Frame Register */
	AT91_REG	 EMAC_OK; 	/*  Frames Received OK Register */
	AT91_REG	 EMAC_SEQE; 	/*  Frame Check Sequence Error Register */
	AT91_REG	 EMAC_ALE; 	/*  Alignment Error Register */
	AT91_REG	 EMAC_DTE; 	/*  Deferred Transmission Frame Register */
	AT91_REG	 EMAC_LCOL; 	/*  Late Collision Register */
	AT91_REG	 EMAC_ECOL; 	/*  Excessive Collision Register */
	AT91_REG	 EMAC_CSE; 	/*  Carrier Sense Error Register */
	AT91_REG	 EMAC_TUE; 	/*  Transmit Underrun Error Register */
	AT91_REG	 EMAC_CDE; 	/*  Code Error Register */
	AT91_REG	 EMAC_ELR; 	/*  Excessive Length Error Register */
	AT91_REG	 EMAC_RJB; 	/*  Receive Jabber Register */
	AT91_REG	 EMAC_USF; 	/*  Undersize Frame Register */
	AT91_REG	 EMAC_SQEE; 	/*  SQE Test Error Register */
	AT91_REG	 EMAC_DRFC; 	/*  Discarded RX Frame Register */
	AT91_REG	 Reserved2[3]; 	/*   */
	AT91_REG	 EMAC_HSH; 	/*  Hash Address High[63:32] */
	AT91_REG	 EMAC_HSL; 	/*  Hash Address Low[31:0] */
	AT91_REG	 EMAC_SA1L; 	/*  Specific Address 1 Low, First 4 bytes */
	AT91_REG	 EMAC_SA1H; 	/*  Specific Address 1 High, Last 2 bytes */
	AT91_REG	 EMAC_SA2L; 	/*  Specific Address 2 Low, First 4 bytes */
	AT91_REG	 EMAC_SA2H; 	/*  Specific Address 2 High, Last 2 bytes */
	AT91_REG	 EMAC_SA3L; 	/*  Specific Address 3 Low, First 4 bytes */
	AT91_REG	 EMAC_SA3H; 	/*  Specific Address 3 High, Last 2 bytes */
	AT91_REG	 EMAC_SA4L; 	/*  Specific Address 4 Low, First 4 bytes */
	AT91_REG	 EMAC_SA4H; 	/*  Specific Address 4 High, Last 2 bytesr */
} AT91S_EMAC, *AT91PS_EMAC;

/*  -------- DBGU_IER : (DBGU Offset: 0x8) Debug Unit Interrupt Enable Register --------  */
#define AT91C_US_RXRDY        ((unsigned int) 0x1 <<  0) /*  (DBGU) RXRDY Interrupt */
#define AT91C_US_TXRDY        ((unsigned int) 0x1 <<  1) /*  (DBGU) TXRDY Interrupt */
#define AT91C_US_ENDRX        ((unsigned int) 0x1 <<  3) /*  (DBGU) End of Receive Transfer Interrupt */
#define AT91C_US_ENDTX        ((unsigned int) 0x1 <<  4) /*  (DBGU) End of Transmit Interrupt */
#define AT91C_US_OVRE         ((unsigned int) 0x1 <<  5) /*  (DBGU) Overrun Interrupt */
#define AT91C_US_FRAME        ((unsigned int) 0x1 <<  6) /*  (DBGU) Framing Error Interrupt */
#define AT91C_US_PARE         ((unsigned int) 0x1 <<  7) /*  (DBGU) Parity Error Interrupt */
#define AT91C_US_TXEMPTY      ((unsigned int) 0x1 <<  9) /*  (DBGU) TXEMPTY Interrupt */
#define AT91C_US_TXBUFE       ((unsigned int) 0x1 << 11) /*  (DBGU) TXBUFE Interrupt */
#define AT91C_US_RXBUFF       ((unsigned int) 0x1 << 12) /*  (DBGU) RXBUFF Interrupt */
#define AT91C_US_COMM_TX      ((unsigned int) 0x1 << 30) /*  (DBGU) COMM_TX Interrupt */
#define AT91C_US_COMM_RX      ((unsigned int) 0x1 << 31) /*  (DBGU) COMM_RX Interrupt */

/*  -------- DBGU_CR : (DBGU Offset: 0x0) Debug Unit Control Register --------  */
#define AT91C_US_RSTRX        ((unsigned int) 0x1 <<  2) /*  (DBGU) Reset Receiver */
#define AT91C_US_RSTTX        ((unsigned int) 0x1 <<  3) /*  (DBGU) Reset Transmitter */
#define AT91C_US_RXEN         ((unsigned int) 0x1 <<  4) /*  (DBGU) Receiver Enable */
#define AT91C_US_RXDIS        ((unsigned int) 0x1 <<  5) /*  (DBGU) Receiver Disable */
#define AT91C_US_TXEN         ((unsigned int) 0x1 <<  6) /*  (DBGU) Transmitter Enable */
#define AT91C_US_TXDIS        ((unsigned int) 0x1 <<  7) /*  (DBGU) Transmitter Disable */

#define 	AT91C_US_CLKS_CLOCK                ((unsigned int) 0x0 <<  4) /*  (USART) Clock */
#define 	AT91C_US_CHRL_8_BITS               ((unsigned int) 0x3 <<  6) /*  (USART) Character Length: 8 bits */
#define 	AT91C_US_PAR_NONE                 ((unsigned int) 0x4 <<  9) /*  (DBGU) No Parity */
#define 	AT91C_US_NBSTOP_1_BIT                ((unsigned int) 0x0 << 12) /*  (USART) 1 stop bit */

#define AT91C_PMC_PCER  ((AT91_REG *) 	0xFFFFFC10) /*  (PMC) Peripheral Clock Enable Register */
#define AT91C_PIOA_PDR  ((AT91_REG *) 	0xFFFFF404) /*  (PIOA) PIO Disable Register */
#define AT91C_PIO_PA30       ((unsigned int) 1 << 30) /*  Pin Controlled by PA30 */
#define AT91C_PIO_PC0        ((unsigned int) 1 <<  0) /*  Pin Controlled by PC0 */
#define AT91C_PC0_BFCK     ((unsigned int) AT91C_PIO_PC0) /*   Burst Flash Clock */
#define AT91C_PA30_DRXD     ((unsigned int) AT91C_PIO_PA30) /*   DBGU Debug Receive Data */
#define AT91C_PIO_PA31       ((unsigned int) 1 << 31) /*  Pin Controlled by PA31 */
#define AT91C_PA31_DTXD     ((unsigned int) AT91C_PIO_PA31) /*   DBGU Debug Transmit Data */

#define AT91C_ID_SYS    ((unsigned int)  1) /*  System Peripheral */
#define AT91C_ID_TC0    ((unsigned int) 17) /*  Timer Counter 0 */
#define AT91C_ID_EMAC   ((unsigned int) 24) /*  Ethernet MAC */

#define AT91C_PIO_PC1        ((unsigned int) 1 <<  1) /*  Pin Controlled by PC1 */
#define AT91C_PC1_BFRDY_SMOE ((unsigned int) AT91C_PIO_PC1) /*   Burst Flash Ready */
#define AT91C_PIO_PC3        ((unsigned int) 1 <<  3) /*  Pin Controlled by PC3 */
#define AT91C_PC3_BFBAA_SMWE ((unsigned int) AT91C_PIO_PC3) /*   Burst Flash Address Advance / SmartMedia Write Enable */
#define AT91C_PIO_PC2        ((unsigned int) 1 <<  2) /*  Pin Controlled by PC2 */
#define AT91C_PC2_BFAVD    ((unsigned int) AT91C_PIO_PC2) /*   Burst Flash Address Valid */
#define AT91C_PIO_PB1        ((unsigned int) 1 <<  1) /*  Pin Controlled by PB1 */

#define AT91C_TC_TIMER_DIV1_CLOCK      ((unsigned int) 0x0 <<  0) /*  (TC) MCK/2 */
#define AT91C_TC_TIMER_DIV2_CLOCK      ((unsigned int) 0x1 <<  0) /*  (TC) MCK/8 */
#define AT91C_TC_TIMER_DIV3_CLOCK      ((unsigned int) 0x2 <<  0) /*  (TC) MCK/32 */
#define AT91C_TC_TIMER_DIV4_CLOCK      ((unsigned int) 0x3 <<  0) /*  (TC) MCK/128 */
#define AT91C_TC_SLOW_CLOCK            ((unsigned int) 0x4 <<  0) /*  (TC) SLOW CLK */
#define AT91C_TC_XC0_CLOCK             ((unsigned int) 0x5 <<  0) /*  (TC) XC0 */
#define AT91C_TC_XC1_CLOCK             ((unsigned int) 0x6 <<  0) /*  (TC) XC1 */
#define AT91C_TC_XC2_CLOCK             ((unsigned int) 0x7 <<  0) /*  (TC) XC2 */
#define 	AT91C_TCB_TC0XC0S_NONE                 ((unsigned int) 0x1) /*  (TCB) None signal connected to XC0 */
#define 	AT91C_TCB_TC1XC1S_NONE                 ((unsigned int) 0x1 <<  2) /*  (TCB) None signal connected to XC1 */
#define 	AT91C_TCB_TC2XC2S_NONE                 ((unsigned int) 0x1 <<  4) /*  (TCB) None signal connected to XC2 */
#define AT91C_TC_CLKDIS       ((unsigned int) 0x1 <<  1) /*  (TC) Counter Clock Disable Command */
#define AT91C_TC_SWTRG        ((unsigned int) 0x1 <<  2) /*  (TC) Software Trigger Command */
#define AT91C_TC_CLKEN        ((unsigned int) 0x1 <<  0) /*  (TC) Counter Clock Enable Command */

#define AT91C_EMAC_BNQ        ((unsigned int) 0x1 <<  4) /*  (EMAC)  */
#define AT91C_EMAC_COMP       ((unsigned int) 0x1 <<  5) /*  (EMAC)  */
#define AT91C_EMAC_REC        ((unsigned int) 0x1 <<  1) /*  (EMAC)  */
#define AT91C_EMAC_RE         ((unsigned int) 0x1 <<  2) /*  (EMAC) Receive enable.  */
#define AT91C_EMAC_TE         ((unsigned int) 0x1 <<  3) /*  (EMAC) Transmit enable.  */
#define AT91C_EMAC_CLK        ((unsigned int) 0x3 << 10) /*  (EMAC)  */
#define AT91C_EMAC_RMII       ((unsigned int) 0x1 << 13) /*  (EMAC)  */
#define AT91C_EMAC_NBC        ((unsigned int) 0x1 <<  5) /*  (EMAC) No broadcast.  */
#define AT91C_EMAC_CAF        ((unsigned int) 0x1 <<  4) /*  (EMAC) Copy all frames.  */
#define AT91C_EMAC_BNA        ((unsigned int) 0x1 <<  0) /*  (EMAC)  */
#define AT91C_EMAC_REC        ((unsigned int) 0x1 <<  1) /*  (EMAC)  */
#define AT91C_EMAC_RSR_OVR    ((unsigned int) 0x1 <<  2) /*  (EMAC)  */
#define AT91C_EMAC_CSR        ((unsigned int) 0x1 <<  5) /*  (EMAC) Clear statistics registers.  */
#define AT91C_EMAC_SPD        ((unsigned int) 0x1 <<  0) /*  (EMAC) Speed.  */
#define AT91C_EMAC_FD         ((unsigned int) 0x1 <<  1) /*  (EMAC) Full duplex.  */
#define AT91C_EMAC_LINK       ((unsigned int) 0x1 <<  0) /*  (EMAC)  */
#define AT91C_EMAC_MPE        ((unsigned int) 0x1 <<  4) /*  (EMAC) Management port enable.  */
#define AT91C_PIO_PA16       ((unsigned int) 1 << 16) /*  Pin Controlled by PA16 */
#define AT91C_PA16_EMDIO    ((unsigned int) AT91C_PIO_PA16) /*   Ethernet MAC Management Data Input/Output */
#define AT91C_PIO_PA15       ((unsigned int) 1 << 15) /*  Pin Controlled by PA15 */
#define AT91C_PA15_EMDC     ((unsigned int) AT91C_PIO_PA15) /*   Ethernet MAC Management Data Clock */
#define AT91C_PIO_PA14       ((unsigned int) 1 << 14) /*  Pin Controlled by PA14 */
#define AT91C_PA14_ERXER    ((unsigned int) AT91C_PIO_PA14) /*   Ethernet MAC Receive Error */
#define AT91C_PIO_PA13       ((unsigned int) 1 << 13) /*  Pin Controlled by PA13 */
#define AT91C_PA13_ERX1     ((unsigned int) AT91C_PIO_PA13) /*   Ethernet MAC Receive Data 1 */
#define AT91C_PIO_PA12       ((unsigned int) 1 << 12) /*  Pin Controlled by PA12 */
#define AT91C_PA12_ERX0     ((unsigned int) AT91C_PIO_PA12) /*   Ethernet MAC Receive Data 0 */
#define AT91C_PIO_PA11       ((unsigned int) 1 << 11) /*  Pin Controlled by PA11 */
#define AT91C_PA11_ECRS_ECRSDV ((unsigned int) AT91C_PIO_PA11) /*   Ethernet MAC Carrier Sense/Carrier Sense and Data Valid */
#define AT91C_PIO_PA10       ((unsigned int) 1 << 10) /*  Pin Controlled by PA10 */
#define AT91C_PA10_ETX1     ((unsigned int) AT91C_PIO_PA10) /*   Ethernet MAC Transmit Data 1 */
#define AT91C_PIO_PA9        ((unsigned int) 1 <<  9) /*  Pin Controlled by PA9 */
#define AT91C_PA9_ETX0     ((unsigned int) AT91C_PIO_PA9) /*   Ethernet MAC Transmit Data 0 */
#define AT91C_PIO_PA8        ((unsigned int) 1 <<  8) /*  Pin Controlled by PA8 */
#define AT91C_PA8_ETXEN    ((unsigned int) AT91C_PIO_PA8) /*   Ethernet MAC Transmit Enable */
#define AT91C_PIO_PA7        ((unsigned int) 1 <<  7) /*  Pin Controlled by PA7 */
#define AT91C_PA7_ETXCK_EREFCK ((unsigned int) AT91C_PIO_PA7) /*   Ethernet MAC Transmit Clock/Reference Clock */
#define AT91C_PIO_PB25       ((unsigned int) 1 << 25) /*  Pin Controlled by PB25 */
#define AT91C_PB25_DSR1     ((unsigned int) AT91C_PIO_PB25) /*   USART 1 Data Set ready */
#define AT91C_PB25_EF100    ((unsigned int) AT91C_PIO_PB25) /*   Ethernet MAC Force 100 Mbits */
#define AT91C_PIO_PB19       ((unsigned int) 1 << 19) /*  Pin Controlled by PB19 */
#define AT91C_PB19_DTR1     ((unsigned int) AT91C_PIO_PB19) /*   USART 1 Data Terminal ready */
#define AT91C_PB19_ERXCK    ((unsigned int) AT91C_PIO_PB19) /*   Ethernet MAC Receive Clock */
#define AT91C_PIO_PB18       ((unsigned int) 1 << 18) /*  Pin Controlled by PB18 */
#define AT91C_PB18_RI1      ((unsigned int) AT91C_PIO_PB18) /*   USART 1 Ring Indicator */
#define AT91C_PB18_ECOL     ((unsigned int) AT91C_PIO_PB18) /*   Ethernet MAC Collision Detected */
#define AT91C_PIO_PB17       ((unsigned int) 1 << 17) /*  Pin Controlled by PB17 */
#define AT91C_PB17_RF2      ((unsigned int) AT91C_PIO_PB17) /*   SSC Receive Frame Sync 2 */
#define AT91C_PB17_ERXDV    ((unsigned int) AT91C_PIO_PB17) /*   Ethernet MAC Receive Data Valid */
#define AT91C_PIO_PB16       ((unsigned int) 1 << 16) /*  Pin Controlled by PB16 */
#define AT91C_PB16_RK2      ((unsigned int) AT91C_PIO_PB16) /*   SSC Receive Clock 2 */
#define AT91C_PB16_ERX3     ((unsigned int) AT91C_PIO_PB16) /*   Ethernet MAC Receive Data 3 */
#define AT91C_PIO_PB15       ((unsigned int) 1 << 15) /*  Pin Controlled by PB15 */
#define AT91C_PB15_RD2      ((unsigned int) AT91C_PIO_PB15) /*   SSC Receive Data 2 */
#define AT91C_PB15_ERX2     ((unsigned int) AT91C_PIO_PB15) /*   Ethernet MAC Receive Data 2 */
#define AT91C_PIO_PB14       ((unsigned int) 1 << 14) /*  Pin Controlled by PB14 */
#define AT91C_PB14_TD2      ((unsigned int) AT91C_PIO_PB14) /*   SSC Transmit Data 2 */
#define AT91C_PB14_ETXER    ((unsigned int) AT91C_PIO_PB14) /*   Ethernet MAC Transmikt Coding Error */
#define AT91C_PIO_PB13       ((unsigned int) 1 << 13) /*  Pin Controlled by PB13 */
#define AT91C_PB13_TK2      ((unsigned int) AT91C_PIO_PB13) /*   SSC Transmit Clock 2 */
#define AT91C_PB13_ETX3     ((unsigned int) AT91C_PIO_PB13) /*   Ethernet MAC Transmit Data 3 */
#define AT91C_PIO_PB12       ((unsigned int) 1 << 12) /*  Pin Controlled by PB12 */
#define AT91C_PB12_TF2      ((unsigned int) AT91C_PIO_PB12) /*   SSC Transmit Frame Sync 2 */
#define AT91C_PB12_ETX2     ((unsigned int) AT91C_PIO_PB12) /*   Ethernet MAC Transmit Data 2 */

#define AT91C_PIOB_BSR  ((AT91_REG *) 	0xFFFFF674) /*  (PIOB) Select B Register */
#define AT91C_BASE_EMAC      ((AT91PS_EMAC) 	0xFFFBC000) /*  (EMAC) Base Address */
#define AT91C_PIOB_PDR  ((AT91_REG *) 	0xFFFFF604) /*  (PIOB) PIO Disable Register */

#define 	AT91C_EBI_CS3A_SMC_SmartMedia       ((unsigned int) 0x1 <<  3) /*  (EBI) Chip Select 3 is assigned to the Static Memory Controller and the SmartMedia Logic is activated. */
#define	AT91C_SMC2_ACSS_STANDARD ((unsigned int) 0x0 << 16) /*  (SMC2) Standard, asserted at the beginning of the access and deasserted at the end. */
#define AT91C_SMC2_DBW_8      ((unsigned int) 0x2 << 13) /*  (SMC2) 8-bit. */
#define AT91C_SMC2_WSEN       ((unsigned int) 0x1 <<  7) /*  (SMC2) Wait State Enable */
#define AT91C_PIOC_ASR  ((AT91_REG *) 	0xFFFFF870) /*  (PIOC) Select A Register */

#define AT91C_BASE_TC0       ((AT91PS_TC) 	0xFFFA0000) /*  (TC0) Base Address */
#define AT91C_BASE_DBGU      ((AT91PS_DBGU) 	0xFFFFF200) /*  (DBGU) Base Address */
#define AT91C_BASE_PIOA      ((AT91PS_PIO) 	0xFFFFF400) /*  (PIOA) Base Address */
#define AT91C_EBI_CSA   ((AT91_REG *) 	0xFFFFFF60) /*  (EBI) Chip Select Assignment Register */
#define AT91C_BASE_SMC2      ((AT91PS_SMC2) 	0xFFFFFF70) /*  (SMC2) Base Address */
#define AT91C_BASE_US1       ((AT91PS_USART) 	0xFFFC4000) /*  (US1) Base Address */
#define AT91C_TCB0_BMR  ((AT91_REG *) 	0xFFFA00C4) /*  (TCB0) TC Block Mode Register */
#define AT91C_TCB0_BCR  ((AT91_REG *) 	0xFFFA00C0) /*  (TCB0) TC Block Control Register */
#define AT91C_PIOC_PDR  ((AT91_REG *) 	0xFFFFF804) /*  (PIOC) PIO Disable Register */
#define AT91C_PIOC_PER  ((AT91_REG *) 	0xFFFFF800) /*  (PIOC) PIO Enable Register */
#define AT91C_PIOC_ODR  ((AT91_REG *) 	0xFFFFF814) /*  (PIOC) Output Disable Registerr */
#define AT91C_PIOB_PER  ((AT91_REG *) 	0xFFFFF600) /*  (PIOB) PIO Enable Register */
#define AT91C_PIOB_ODR  ((AT91_REG *) 	0xFFFFF614) /*  (PIOB) Output Disable Registerr */
#define AT91C_PIOB_PDSR ((AT91_REG *) 	0xFFFFF63C) /*  (PIOB) Pin Data Status Register */
#endif
