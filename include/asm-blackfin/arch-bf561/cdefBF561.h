/*
 * cdefBF561.h
 *
 * (c) Copyright 2001-2004 Analog Devices, Inc.  All rights reserved.
 *
 */

/* C POINTERS TO SYSTEM MMR REGISTER AND MEMORY MAP FOR ADSP-BF561 */

#ifndef _CDEF_BF561_H
#define _CDEF_BF561_H

/*
 * #if !defined(__ADSPBF561__)
 * #warning cdefBF561.h should only be included for BF561 chip.
 * #endif
 */

/* include all Core registers and bit definitions */
#include <asm/arch-bf561/defBF561.h>
#include <asm/arch-common/cdef_LPBlackfin.h>

/*
 * System MMR Register Map
 */

/* Clock and System Control (0xFFC00000 - 0xFFC000FF) */
#define pPLL_CTL		(volatile unsigned short *)PLL_CTL
#define pPLL_DIV		(volatile unsigned short *)PLL_DIV
#define pVR_CTL			(volatile unsigned short *)VR_CTL
#define pPLL_STAT		(volatile unsigned short *)PLL_STAT
#define pPLL_LOCKCNT		(volatile unsigned short *)PLL_LOCKCNT

/*
 * System Reset and Interrupt Controller registers for
 * core A (0xFFC0 0100-0xFFC0 01FF)
 */
#define pSICA_SWRST		(volatile unsigned short *)SICA_SWRST
#define pSICA_SYSCR		(volatile unsigned short *)SICA_SYSCR
#define pSICA_RVECT		(volatile unsigned short *)SICA_RVECT
#define pSICA_IMASK		(volatile unsigned long *)SICA_IMASK
#define pSICA_IMASK0		(volatile unsigned long *)SICA_IMASK0
#define pSICA_IMASK1		(volatile unsigned long *)SICA_IMASK1
#define pSICA_IAR0		(volatile unsigned long *)SICA_IAR0
#define pSICA_IAR1		(volatile unsigned long *)SICA_IAR1
#define pSICA_IAR2		(volatile unsigned long *)SICA_IAR2
#define pSICA_IAR3		(volatile unsigned long *)SICA_IAR3
#define pSICA_IAR4		(volatile unsigned long *)SICA_IAR4
#define pSICA_IAR5		(volatile unsigned long *)SICA_IAR5
#define pSICA_IAR6		(volatile unsigned long *)SICA_IAR6
#define pSICA_IAR7		(volatile unsigned long *)SICA_IAR7
#define pSICA_ISR0		(volatile unsigned long *)SICA_ISR0
#define pSICA_ISR1		(volatile unsigned long *)SICA_ISR1
#define pSICA_IWR0		(volatile unsigned long *)SICA_IWR0
#define pSICA_IWR1		(volatile unsigned long *)SICA_IWR1

/*
 * System Reset and Interrupt Controller registers for
 * Core B (0xFFC0 1100-0xFFC0 11FF)
 */
#define pSICB_SWRST		(volatile unsigned short *)SICB_SWRST
#define pSICB_SYSCR		(volatile unsigned short *)SICB_SYSCR
#define pSICB_RVECT		(volatile unsigned short *)SICB_RVECT
#define pSICB_IMASK0		(volatile unsigned long *)SICB_IMASK0
#define pSICB_IMASK1		(volatile unsigned long *)SICB_IMASK1
#define pSICB_IAR0		(volatile unsigned long *)SICB_IAR0
#define pSICB_IAR1		(volatile unsigned long *)SICB_IAR1
#define pSICB_IAR2		(volatile unsigned long *)SICB_IAR2
#define pSICB_IAR3		(volatile unsigned long *)SICB_IAR3
#define pSICB_IAR4		(volatile unsigned long *)SICB_IAR4
#define pSICB_IAR5		(volatile unsigned long *)SICB_IAR5
#define pSICB_IAR6		(volatile unsigned long *)SICB_IAR6
#define pSICB_IAR7		(volatile unsigned long *)SICB_IAR7
#define pSICB_ISR0		(volatile unsigned long *)SICB_ISR0
#define pSICB_ISR1		(volatile unsigned long *)SICB_ISR1
#define pSICB_IWR0		(volatile unsigned long *)SICB_IWR0
#define pSICB_IWR1		(volatile unsigned long *)SICB_IWR1

/* Watchdog Timer registers for Core A (0xFFC0 0200-0xFFC0 02FF) */
#define pWDOGA_CTL		(volatile unsigned short *)WDOGA_CTL
#define pWDOGA_CNT		(volatile unsigned long *)WDOGA_CNT
#define pWDOGA_STAT		(volatile unsigned long *)WDOGA_STAT

/* Watchdog Timer registers for Core B (0xFFC0 1200-0xFFC0 12FF) */
#define pWDOGB_CTL		(volatile unsigned short *)WDOGB_CTL
#define pWDOGB_CNT		(volatile unsigned long *)WDOGB_CNT
#define pWDOGB_STAT		(volatile unsigned long *)WDOGB_STAT

/* UART Controller (0xFFC00400 - 0xFFC004FF) */
#define pUART_THR		(volatile unsigned short *)UART_THR
#define pUART_RBR		(volatile unsigned short *)UART_RBR
#define pUART_DLL		(volatile unsigned short *)UART_DLL
#define pUART_IER		(volatile unsigned short *)UART_IER
#define pUART_DLH		(volatile unsigned short *)UART_DLH
#define pUART_IIR		(volatile unsigned short *)UART_IIR
#define pUART_LCR		(volatile unsigned short *)UART_LCR
#define pUART_MCR		(volatile unsigned short *)UART_MCR
#define pUART_LSR		(volatile unsigned short *)UART_LSR
#define pUART_MSR		(volatile unsigned short *)UART_MSR
#define pUART_SCR		(volatile unsigned short *)UART_SCR
#define pUART_GCTL		(volatile unsigned short *)UART_GCTL

/* SPI Controller (0xFFC00500 - 0xFFC005FF) */
#define pSPI_CTL		(volatile unsigned short *)SPI_CTL
#define pSPI_FLG		(volatile unsigned short *)SPI_FLG
#define pSPI_STAT		(volatile unsigned short *)SPI_STAT
#define pSPI_TDBR		(volatile unsigned short *)SPI_TDBR
#define pSPI_RDBR		(volatile unsigned short *)SPI_RDBR
#define pSPI_BAUD		(volatile unsigned short *)SPI_BAUD
#define pSPI_SHADOW		(volatile unsigned short *)SPI_SHADOW

/* Timer 0-7 registers (0xFFC0 0600-0xFFC0 06FF) */
#define pTIMER0_CONFIG		(volatile unsigned short *)TIMER0_CONFIG
#define pTIMER0_COUNTER		(volatile unsigned long *)TIMER0_COUNTER
#define pTIMER0_PERIOD		(volatile unsigned long *)TIMER0_PERIOD
#define pTIMER0_WIDTH		(volatile unsigned long *)TIMER0_WIDTH
#define pTIMER1_CONFIG		(volatile unsigned short *)TIMER1_CONFIG
#define pTIMER1_COUNTER		(volatile unsigned long *)TIMER1_COUNTER
#define pTIMER1_PERIOD		(volatile unsigned long *)TIMER1_PERIOD
#define pTIMER1_WIDTH		(volatile unsigned long *)TIMER1_WIDTH
#define pTIMER2_CONFIG		(volatile unsigned short *)TIMER2_CONFIG
#define pTIMER2_COUNTER		(volatile unsigned long *)TIMER2_COUNTER
#define pTIMER2_PERIOD		(volatile unsigned long *)TIMER2_PERIOD
#define pTIMER2_WIDTH		(volatile unsigned long *)TIMER2_WIDTH
#define pTIMER3_CONFIG		(volatile unsigned short *)TIMER3_CONFIG
#define pTIMER3_COUNTER		(volatile unsigned long *)TIMER3_COUNTER
#define pTIMER3_PERIOD		(volatile unsigned long *)TIMER3_PERIOD
#define pTIMER3_WIDTH		(volatile unsigned long *)TIMER3_WIDTH
#define pTIMER4_CONFIG		(volatile unsigned short *)TIMER4_CONFIG
#define pTIMER4_COUNTER		(volatile unsigned long *)TIMER4_COUNTER
#define pTIMER4_PERIOD		(volatile unsigned long *)TIMER4_PERIOD
#define pTIMER4_WIDTH		(volatile unsigned long *)TIMER4_WIDTH
#define pTIMER5_CONFIG		(volatile unsigned short *)TIMER5_CONFIG
#define pTIMER5_COUNTER		(volatile unsigned long *)TIMER5_COUNTER
#define pTIMER5_PERIOD		(volatile unsigned long *)TIMER5_PERIOD
#define pTIMER5_WIDTH		(volatile unsigned long *)TIMER5_WIDTH
#define pTIMER6_CONFIG		(volatile unsigned short *)TIMER6_CONFIG
#define pTIMER6_COUNTER		(volatile unsigned long *)TIMER6_COUNTER
#define pTIMER6_PERIOD		(volatile unsigned long *)TIMER6_PERIOD
#define pTIMER6_WIDTH		(volatile unsigned long *)TIMER6_WIDTH
#define pTIMER7_CONFIG		(volatile unsigned short *)TIMER7_CONFIG
#define pTIMER7_COUNTER		(volatile unsigned long *)TIMER7_COUNTER
#define pTIMER7_PERIOD		(volatile unsigned long *)TIMER7_PERIOD
#define pTIMER7_WIDTH		(volatile unsigned long *)TIMER7_WIDTH

/* Timer registers 8-11 (0xFFC0 1600-0xFFC0 16FF) */
#define pTMRS8_ENABLE		(volatile unsigned short *)TMRS8_ENABLE
#define pTMRS8_DISABLE		(volatile unsigned short *)TMRS8_DISABLE
#define pTMRS8_STATUS		(volatile unsigned long *)TMRS8_STATUS
#define pTIMER8_CONFIG		(volatile unsigned short *)TIMER8_CONFIG
#define pTIMER8_COUNTER		(volatile unsigned long *)TIMER8_COUNTER
#define pTIMER8_PERIOD		(volatile unsigned long *)TIMER8_PERIOD
#define pTIMER8_WIDTH		(volatile unsigned long *)TIMER8_WIDTH
#define pTIMER9_CONFIG		(volatile unsigned short *)TIMER9_CONFIG
#define pTIMER9_COUNTER		(volatile unsigned long *)TIMER9_COUNTER
#define pTIMER9_PERIOD		(volatile unsigned long *)TIMER9_PERIOD
#define pTIMER9_WIDTH		(volatile unsigned long *)TIMER9_WIDTH
#define pTIMER10_CONFIG		(volatile unsigned short *)TIMER10_CONFIG
#define pTIMER10_COUNTER	(volatile unsigned long *)TIMER10_COUNTER
#define pTIMER10_PERIOD		(volatile unsigned long *)TIMER10_PERIOD
#define pTIMER10_WIDTH		(volatile unsigned long *)TIMER10_WIDTH
#define pTIMER11_CONFIG		(volatile unsigned short *)TIMER11_CONFIG
#define pTIMER11_COUNTER	(volatile unsigned long *)TIMER11_COUNTER
#define pTIMER11_PERIOD		(volatile unsigned long *)TIMER11_PERIOD
#define pTIMER11_WIDTH		(volatile unsigned long *)TIMER11_WIDTH
#define pTMRS4_ENABLE		(volatile unsigned short *)TMRS4_ENABLE
#define pTMRS4_DISABLE		(volatile unsigned short *)TMRS4_DISABLE
#define pTMRS4_STATUS		(volatile unsigned long *)TMRS4_STATUS

/* Programmable Flag 0 registers (0xFFC0 0700-0xFFC0 07FF) */
#define pFIO0_FLAG_D		(volatile unsigned short *)FIO0_FLAG_D
#define pFIO0_FLAG_C		(volatile unsigned short *)FIO0_FLAG_C
#define pFIO0_FLAG_S		(volatile unsigned short *)FIO0_FLAG_S
#define pFIO0_FLAG_T		(volatile unsigned short *)FIO0_FLAG_T
#define pFIO0_MASKA_D		(volatile unsigned short *)FIO0_MASKA_D
#define pFIO0_MASKA_C		(volatile unsigned short *)FIO0_MASKA_C
#define pFIO0_MASKA_S		(volatile unsigned short *)FIO0_MASKA_S
#define pFIO0_MASKA_T		(volatile unsigned short *)FIO0_MASKA_T
#define pFIO0_MASKB_D		(volatile unsigned short *)FIO0_MASKB_D
#define pFIO0_MASKB_C		(volatile unsigned short *)FIO0_MASKB_C
#define pFIO0_MASKB_S		(volatile unsigned short *)FIO0_MASKB_S
#define pFIO0_MASKB_T		(volatile unsigned short *)FIO0_MASKB_T
#define pFIO0_DIR		(volatile unsigned short *)FIO0_DIR
#define pFIO0_POLAR		(volatile unsigned short *)FIO0_POLAR
#define pFIO0_EDGE		(volatile unsigned short *)FIO0_EDGE
#define pFIO0_BOTH		(volatile unsigned short *)FIO0_BOTH
#define pFIO0_INEN		(volatile unsigned short *)FIO0_INEN

/* Programmable Flag 1 registers (0xFFC0 1500-0xFFC0 15FF) */
#define pFIO1_FLAG_D		(volatile unsigned short *)FIO1_FLAG_D
#define pFIO1_FLAG_C		(volatile unsigned short *)FIO1_FLAG_C
#define pFIO1_FLAG_S		(volatile unsigned short *)FIO1_FLAG_S
#define pFIO1_FLAG_T		(volatile unsigned short *)FIO1_FLAG_T
#define pFIO1_MASKA_D		(volatile unsigned short *)FIO1_MASKA_D
#define pFIO1_MASKA_C		(volatile unsigned short *)FIO1_MASKA_C
#define pFIO1_MASKA_S		(volatile unsigned short *)FIO1_MASKA_S
#define pFIO1_MASKA_T		(volatile unsigned short *)FIO1_MASKA_T
#define pFIO1_MASKB_D		(volatile unsigned short *)FIO1_MASKB_D
#define pFIO1_MASKB_C		(volatile unsigned short *)FIO1_MASKB_C
#define pFIO1_MASKB_S		(volatile unsigned short *)FIO1_MASKB_S
#define pFIO1_MASKB_T		(volatile unsigned short *)FIO1_MASKB_T
#define pFIO1_DIR		(volatile unsigned short *)FIO1_DIR
#define pFIO1_POLAR		(volatile unsigned short *)FIO1_POLAR
#define pFIO1_EDGE		(volatile unsigned short *)FIO1_EDGE
#define pFIO1_BOTH		(volatile unsigned short *)FIO1_BOTH
#define pFIO1_INEN		(volatile unsigned short *)FIO1_INEN

/* Programmable Flag registers (0xFFC0 1700-0xFFC0 17FF) */
#define pFIO2_FLAG_D		(volatile unsigned short *)FIO2_FLAG_D
#define pFIO2_FLAG_C		(volatile unsigned short *)FIO2_FLAG_C
#define pFIO2_FLAG_S		(volatile unsigned short *)FIO2_FLAG_S
#define pFIO2_FLAG_T		(volatile unsigned short *)FIO2_FLAG_T
#define pFIO2_MASKA_D		(volatile unsigned short *)FIO2_MASKA_D
#define pFIO2_MASKA_C		(volatile unsigned short *)FIO2_MASKA_C
#define pFIO2_MASKA_S		(volatile unsigned short *)FIO2_MASKA_S
#define pFIO2_MASKA_T		(volatile unsigned short *)FIO2_MASKA_T
#define pFIO2_MASKB_D		(volatile unsigned short *)FIO2_MASKB_D
#define pFIO2_MASKB_C		(volatile unsigned short *)FIO2_MASKB_C
#define pFIO2_MASKB_S		(volatile unsigned short *)FIO2_MASKB_S
#define pFIO2_MASKB_T		(volatile unsigned short *)FIO2_MASKB_T
#define pFIO2_DIR		(volatile unsigned short *)FIO2_DIR
#define pFIO2_POLAR		(volatile unsigned short *)FIO2_POLAR
#define pFIO2_EDGE		(volatile unsigned short *)FIO2_EDGE
#define pFIO2_BOTH		(volatile unsigned short *)FIO2_BOTH
#define pFIO2_INEN		(volatile unsigned short *)FIO2_INEN

/* SPORT0 Controller (0xFFC00800 - 0xFFC008FF) */
#define pSPORT0_TCR1		(volatile unsigned short *)SPORT0_TCR1
#define pSPORT0_TCR2		(volatile unsigned short *)SPORT0_TCR2
#define pSPORT0_TCLKDIV		(volatile unsigned short *)SPORT0_TCLKDIV
#define pSPORT0_TFSDIV		(volatile unsigned short *)SPORT0_TFSDIV
#define pSPORT0_TX		(volatile unsigned long *)SPORT0_TX
#define pSPORT0_RX		(volatile unsigned long *)SPORT0_RX
#define pSPORT0_TX32		((volatile long *)SPORT0_TX)
#define pSPORT0_RX32		((volatile long *)SPORT0_RX)
#define pSPORT0_TX16		((volatile unsigned short *)SPORT0_TX)
#define pSPORT0_RX16		((volatile unsigned short *)SPORT0_RX)
#define pSPORT0_RCR1		(volatile unsigned short *)SPORT0_RCR1
#define pSPORT0_RCR2		(volatile unsigned short *)SPORT0_RCR2
#define pSPORT0_RCLKDIV		(volatile unsigned short *)SPORT0_RCLKDIV
#define pSPORT0_RFSDIV		(volatile unsigned short *)SPORT0_RFSDIV
#define pSPORT0_STAT		(volatile unsigned short *)SPORT0_STAT
#define pSPORT0_CHNL		(volatile unsigned short *)SPORT0_CHNL
#define pSPORT0_MCMC1		(volatile unsigned short *)SPORT0_MCMC1
#define pSPORT0_MCMC2		(volatile unsigned short *)SPORT0_MCMC2
#define pSPORT0_MTCS0		(volatile unsigned long *)SPORT0_MTCS0
#define pSPORT0_MTCS1		(volatile unsigned long *)SPORT0_MTCS1
#define pSPORT0_MTCS2		(volatile unsigned long *)SPORT0_MTCS2
#define pSPORT0_MTCS3		(volatile unsigned long *)SPORT0_MTCS3
#define pSPORT0_MRCS0		(volatile unsigned long *)SPORT0_MRCS0
#define pSPORT0_MRCS1		(volatile unsigned long *)SPORT0_MRCS1
#define pSPORT0_MRCS2		(volatile unsigned long *)SPORT0_MRCS2
#define pSPORT0_MRCS3		(volatile unsigned long *)SPORT0_MRCS3

/* SPORT1 Controller (0xFFC00900 - 0xFFC009FF) */
#define pSPORT1_TCR1		(volatile unsigned short *)SPORT1_TCR1
#define pSPORT1_TCR2		(volatile unsigned short *)SPORT1_TCR2
#define pSPORT1_TCLKDIV		(volatile unsigned short *)SPORT1_TCLKDIV
#define pSPORT1_TFSDIV		(volatile unsigned short *)SPORT1_TFSDIV
#define pSPORT1_TX		(volatile unsigned long *)SPORT1_TX
#define pSPORT1_RX		(volatile unsigned long *)SPORT1_RX
#define pSPORT1_TX32		((volatile long *)SPORT1_TX)
#define pSPORT1_RX32		((volatile long *)SPORT1_RX)
#define pSPORT1_TX16		((volatile unsigned short *)SPORT1_TX)
#define pSPORT1_RX16		((volatile unsigned short *)SPORT1_RX)
#define pSPORT1_RCR1		(volatile unsigned short *)SPORT1_RCR1
#define pSPORT1_RCR2		(volatile unsigned short *)SPORT1_RCR2
#define pSPORT1_RCLKDIV		(volatile unsigned short *)SPORT1_RCLKDIV
#define pSPORT1_RFSDIV		(volatile unsigned short *)SPORT1_RFSDIV
#define pSPORT1_STAT		(volatile unsigned short *)SPORT1_STAT
#define pSPORT1_CHNL		(volatile unsigned short *)SPORT1_CHNL
#define pSPORT1_MCMC1		(volatile unsigned short *)SPORT1_MCMC1
#define pSPORT1_MCMC2		(volatile unsigned short *)SPORT1_MCMC2
#define pSPORT1_MTCS0		(volatile unsigned long *)SPORT1_MTCS0
#define pSPORT1_MTCS1		(volatile unsigned long *)SPORT1_MTCS1
#define pSPORT1_MTCS2		(volatile unsigned long *)SPORT1_MTCS2
#define pSPORT1_MTCS3		(volatile unsigned long *)SPORT1_MTCS3
#define pSPORT1_MRCS0		(volatile unsigned long *)SPORT1_MRCS0
#define pSPORT1_MRCS1		(volatile unsigned long *)SPORT1_MRCS1
#define pSPORT1_MRCS2		(volatile unsigned long *)SPORT1_MRCS2
#define pSPORT1_MRCS3		(volatile unsigned long *)SPORT1_MRCS3

/* Asynchronous Memory Controller - External Bus Interface Unit */
#define pEBIU_AMGCTL		(volatile unsigned short *)EBIU_AMGCTL
#define pEBIU_AMBCTL0		(volatile unsigned long *)EBIU_AMBCTL0
#define pEBIU_AMBCTL1		(volatile unsigned long *)EBIU_AMBCTL1

/* SDRAM Controller External Bus Interface Unit (0xFFC00A00 - 0xFFC00AFF) */
#define pEBIU_SDGCTL		(volatile unsigned long *)EBIU_SDGCTL
#define pEBIU_SDBCTL		(volatile unsigned long *)EBIU_SDBCTL
#define pEBIU_SDRRC		(volatile unsigned short *)EBIU_SDRRC
#define pEBIU_SDSTAT		(volatile unsigned short *)EBIU_SDSTAT

/* Parallel Peripheral Interface (PPI) 0 registers (0xFFC0 1000-0xFFC0 10FF)*/
#define pPPI0_CONTROL		(volatile unsigned short *)PPI0_CONTROL
#define pPPI0_STATUS		(volatile unsigned short *)PPI0_STATUS
#define pPPI0_COUNT		(volatile unsigned short *)PPI0_COUNT
#define pPPI0_DELAY		(volatile unsigned short *)PPI0_DELAY
#define pPPI0_FRAME		(volatile unsigned short *)PPI0_FRAME

/* Parallel Peripheral Interface (PPI) 1 registers (0xFFC0 1300-0xFFC0 13FF)*/
#define pPPI1_CONTROL		(volatile unsigned short *)PPI1_CONTROL
#define pPPI1_STATUS		(volatile unsigned short *)PPI1_STATUS
#define pPPI1_COUNT		(volatile unsigned short *)PPI1_COUNT
#define pPPI1_DELAY		(volatile unsigned short *)PPI1_DELAY
#define pPPI1_FRAME		(volatile unsigned short *)PPI1_FRAME

/*DMA Traffic controls*/
#define pDMA_TCPER		((volatile unsigned short *)DMA_TCPER)
#define pDMA_TCCNT		((volatile unsigned short *)DMA_TCCNT)
#define pDMA_TC_PER		((volatile unsigned short *)DMA_TC_PER)
#define pDMA_TC_CNT		((volatile unsigned short *)DMA_TC_CNT)

/* DMA1 Controller registers (0xFFC0 1C00-0xFFC0 1FFF) */
#define pDMA1_0_CONFIG		(volatile unsigned short *)DMA1_0_CONFIG
#define pDMA1_0_NEXT_DESC_PTR	(volatile void **)DMA1_0_NEXT_DESC_PTR
#define pDMA1_0_START_ADDR	(volatile void **)DMA1_0_START_ADDR
#define pDMA1_0_X_COUNT		(volatile unsigned short *)DMA1_0_X_COUNT
#define pDMA1_0_Y_COUNT		(volatile unsigned short *)DMA1_0_Y_COUNT
#define pDMA1_0_X_MODIFY	(volatile unsigned short *)DMA1_0_X_MODIFY
#define pDMA1_0_Y_MODIFY	(volatile unsigned short *)DMA1_0_Y_MODIFY
#define pDMA1_0_CURR_DESC_PTR	(volatile void **)DMA1_0_CURR_DESC_PTR
#define pDMA1_0_CURR_ADDR	(volatile void **)DMA1_0_CURR_ADDR
#define pDMA1_0_CURR_X_COUNT	(volatile unsigned short *)DMA1_0_CURR_X_COUNT
#define pDMA1_0_CURR_Y_COUNT	(volatile unsigned short *)DMA1_0_CURR_Y_COUNT
#define pDMA1_0_IRQ_STATUS	(volatile unsigned short *)DMA1_0_IRQ_STATUS
#define pDMA1_0_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_0_PERIPHERAL_MAP
#define pDMA1_1_CONFIG		(volatile unsigned short *)DMA1_1_CONFIG
#define pDMA1_1_NEXT_DESC_PTR	(volatile void **)DMA1_1_NEXT_DESC_PTR
#define pDMA1_1_START_ADDR	(volatile void **)DMA1_1_START_ADDR
#define pDMA1_1_X_COUNT		(volatile unsigned short *)DMA1_1_X_COUNT
#define pDMA1_1_Y_COUNT		(volatile unsigned short *)DMA1_1_Y_COUNT
#define pDMA1_1_X_MODIFY	(volatile unsigned short *)DMA1_1_X_MODIFY
#define pDMA1_1_Y_MODIFY	(volatile unsigned short *)DMA1_1_Y_MODIFY
#define pDMA1_1_CURR_DESC_PTR	(volatile void **)DMA1_1_CURR_DESC_PTR
#define pDMA1_1_CURR_ADDR	(volatile void **)DMA1_1_CURR_ADDR
#define pDMA1_1_CURR_X_COUNT	(volatile unsigned short *)DMA1_1_CURR_X_COUNT
#define pDMA1_1_CURR_Y_COUNT	(volatile unsigned short *)DMA1_1_CURR_Y_COUNT
#define pDMA1_1_IRQ_STATUS	(volatile unsigned short *)DMA1_1_IRQ_STATUS
#define pDMA1_1_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_1_PERIPHERAL_MAP
#define pDMA1_2_CONFIG		(volatile unsigned short *)DMA1_2_CONFIG
#define pDMA1_2_NEXT_DESC_PTR	(volatile void **)DMA1_2_NEXT_DESC_PTR
#define pDMA1_2_START_ADDR	(volatile void **)DMA1_2_START_ADDR
#define pDMA1_2_X_COUNT		(volatile unsigned short *)DMA1_2_X_COUNT
#define pDMA1_2_Y_COUNT		(volatile unsigned short *)DMA1_2_Y_COUNT
#define pDMA1_2_X_MODIFY	(volatile unsigned short *)DMA1_2_X_MODIFY
#define pDMA1_2_Y_MODIFY	(volatile unsigned short *)DMA1_2_Y_MODIFY
#define pDMA1_2_CURR_DESC_PTR	(volatile void **)DMA1_2_CURR_DESC_PTR
#define pDMA1_2_CURR_ADDR	(volatile void **)DMA1_2_CURR_ADDR
#define pDMA1_2_CURR_X_COUNT	(volatile unsigned short *)DMA1_2_CURR_X_COUNT
#define pDMA1_2_CURR_Y_COUNT	(volatile unsigned short *)DMA1_2_CURR_Y_COUNT
#define pDMA1_2_IRQ_STATUS	(volatile unsigned short *)DMA1_2_IRQ_STATUS
#define pDMA1_2_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_2_PERIPHERAL_MAP
#define pDMA1_3_CONFIG		(volatile unsigned short *)DMA1_3_CONFIG
#define pDMA1_3_NEXT_DESC_PTR	(volatile void **)DMA1_3_NEXT_DESC_PTR
#define pDMA1_3_START_ADDR	(volatile void **)DMA1_3_START_ADDR
#define pDMA1_3_X_COUNT		(volatile unsigned short *)DMA1_3_X_COUNT
#define pDMA1_3_Y_COUNT		(volatile unsigned short *)DMA1_3_Y_COUNT
#define pDMA1_3_X_MODIFY	(volatile unsigned short *)DMA1_3_X_MODIFY
#define pDMA1_3_Y_MODIFY	(volatile unsigned short *)DMA1_3_Y_MODIFY
#define pDMA1_3_CURR_DESC_PTR	(volatile void **)DMA1_3_CURR_DESC_PTR
#define pDMA1_3_CURR_ADDR	(volatile void **)DMA1_3_CURR_ADDR
#define pDMA1_3_CURR_X_COUNT	(volatile unsigned short *)DMA1_3_CURR_X_COUNT
#define pDMA1_3_CURR_Y_COUNT	(volatile unsigned short *)DMA1_3_CURR_Y_COUNT
#define pDMA1_3_IRQ_STATUS	(volatile unsigned short *)DMA1_3_IRQ_STATUS
#define pDMA1_3_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_3_PERIPHERAL_MAP
#define pDMA1_4_CONFIG		(volatile unsigned short *)DMA1_4_CONFIG
#define pDMA1_4_NEXT_DESC_PTR	(volatile void **)DMA1_4_NEXT_DESC_PTR
#define pDMA1_4_START_ADDR	(volatile void **)DMA1_4_START_ADDR
#define pDMA1_4_X_COUNT		(volatile unsigned short *)DMA1_4_X_COUNT
#define pDMA1_4_Y_COUNT		(volatile unsigned short *)DMA1_4_Y_COUNT
#define pDMA1_4_X_MODIFY	(volatile unsigned short *)DMA1_4_X_MODIFY
#define pDMA1_4_Y_MODIFY	(volatile unsigned short *)DMA1_4_Y_MODIFY
#define pDMA1_4_CURR_DESC_PTR	(volatile void **)DMA1_4_CURR_DESC_PTR
#define pDMA1_4_CURR_ADDR	(volatile void **)DMA1_4_CURR_ADDR
#define pDMA1_4_CURR_X_COUNT	(volatile unsigned short *)DMA1_4_CURR_X_COUNT
#define pDMA1_4_CURR_Y_COUNT	(volatile unsigned short *)DMA1_4_CURR_Y_COUNT
#define pDMA1_4_IRQ_STATUS	(volatile unsigned short *)DMA1_4_IRQ_STATUS
#define pDMA1_4_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_4_PERIPHERAL_MAP
#define pDMA1_5_CONFIG		(volatile unsigned short *)DMA1_5_CONFIG
#define pDMA1_5_NEXT_DESC_PTR	(volatile void **)DMA1_5_NEXT_DESC_PTR
#define pDMA1_5_START_ADDR	(volatile void **)DMA1_5_START_ADDR
#define pDMA1_5_X_COUNT		(volatile unsigned short *)DMA1_5_X_COUNT
#define pDMA1_5_Y_COUNT		(volatile unsigned short *)DMA1_5_Y_COUNT
#define pDMA1_5_X_MODIFY	(volatile unsigned short *)DMA1_5_X_MODIFY
#define pDMA1_5_Y_MODIFY	(volatile unsigned short *)DMA1_5_Y_MODIFY
#define pDMA1_5_CURR_DESC_PTR	(volatile void **)DMA1_5_CURR_DESC_PTR
#define pDMA1_5_CURR_ADDR	(volatile void **)DMA1_5_CURR_ADDR
#define pDMA1_5_CURR_X_COUNT	(volatile unsigned short *)DMA1_5_CURR_X_COUNT
#define pDMA1_5_CURR_Y_COUNT	(volatile unsigned short *)DMA1_5_CURR_Y_COUNT
#define pDMA1_5_IRQ_STATUS	(volatile unsigned short *)DMA1_5_IRQ_STATUS
#define pDMA1_5_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_5_PERIPHERAL_MAP
#define pDMA1_6_CONFIG		(volatile unsigned short *)DMA1_6_CONFIG
#define pDMA1_6_NEXT_DESC_PTR	(volatile void **)DMA1_6_NEXT_DESC_PTR
#define pDMA1_6_START_ADDR	(volatile void **)DMA1_6_START_ADDR
#define pDMA1_6_X_COUNT		(volatile unsigned short *)DMA1_6_X_COUNT
#define pDMA1_6_Y_COUNT		(volatile unsigned short *)DMA1_6_Y_COUNT
#define pDMA1_6_X_MODIFY	(volatile unsigned short *)DMA1_6_X_MODIFY
#define pDMA1_6_Y_MODIFY	(volatile unsigned short *)DMA1_6_Y_MODIFY
#define pDMA1_6_CURR_DESC_PTR	(volatile void **)DMA1_6_CURR_DESC_PTR
#define pDMA1_6_CURR_ADDR	(volatile void **)DMA1_6_CURR_ADDR
#define pDMA1_6_CURR_X_COUNT	(volatile unsigned short *)DMA1_6_CURR_X_COUNT
#define pDMA1_6_CURR_Y_COUNT	(volatile unsigned short *)DMA1_6_CURR_Y_COUNT
#define pDMA1_6_IRQ_STATUS	(volatile unsigned short *)DMA1_6_IRQ_STATUS
#define pDMA1_6_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_6_PERIPHERAL_MAP
#define pDMA1_7_CONFIG		(volatile unsigned short *)DMA1_7_CONFIG
#define pDMA1_7_NEXT_DESC_PTR	(volatile void **)DMA1_7_NEXT_DESC_PTR
#define pDMA1_7_START_ADDR	(volatile void **)DMA1_7_START_ADDR
#define pDMA1_7_X_COUNT		(volatile unsigned short *)DMA1_7_X_COUNT
#define pDMA1_7_Y_COUNT		(volatile unsigned short *)DMA1_7_Y_COUNT
#define pDMA1_7_X_MODIFY	(volatile unsigned short *)DMA1_7_X_MODIFY
#define pDMA1_7_Y_MODIFY	(volatile unsigned short *)DMA1_7_Y_MODIFY
#define pDMA1_7_CURR_DESC_PTR	(volatile void **)DMA1_7_CURR_DESC_PTR
#define pDMA1_7_CURR_ADDR	(volatile void **)DMA1_7_CURR_ADDR
#define pDMA1_7_CURR_X_COUNT	(volatile unsigned short *)DMA1_7_CURR_X_COUNT
#define pDMA1_7_CURR_Y_COUNT	(volatile unsigned short *)DMA1_7_CURR_Y_COUNT
#define pDMA1_7_IRQ_STATUS	(volatile unsigned short *)DMA1_7_IRQ_STATUS
#define pDMA1_7_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_7_PERIPHERAL_MAP
#define pDMA1_8_CONFIG		(volatile unsigned short *)DMA1_8_CONFIG
#define pDMA1_8_NEXT_DESC_PTR	(volatile void **)DMA1_8_NEXT_DESC_PTR
#define pDMA1_8_START_ADDR	(volatile void **)DMA1_8_START_ADDR
#define pDMA1_8_X_COUNT		(volatile unsigned short *)DMA1_8_X_COUNT
#define pDMA1_8_Y_COUNT		(volatile unsigned short *)DMA1_8_Y_COUNT
#define pDMA1_8_X_MODIFY	(volatile unsigned short *)DMA1_8_X_MODIFY
#define pDMA1_8_Y_MODIFY	(volatile unsigned short *)DMA1_8_Y_MODIFY
#define pDMA1_8_CURR_DESC_PTR	(volatile void **)DMA1_8_CURR_DESC_PTR
#define pDMA1_8_CURR_ADDR	(volatile void **)DMA1_8_CURR_ADDR
#define pDMA1_8_CURR_X_COUNT	(volatile unsigned short *)DMA1_8_CURR_X_COUNT
#define pDMA1_8_CURR_Y_COUNT	(volatile unsigned short *)DMA1_8_CURR_Y_COUNT
#define pDMA1_8_IRQ_STATUS	(volatile unsigned short *)DMA1_8_IRQ_STATUS
#define pDMA1_8_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_8_PERIPHERAL_MAP
#define pDMA1_9_CONFIG		(volatile unsigned short *)DMA1_9_CONFIG
#define pDMA1_9_NEXT_DESC_PTR	(volatile void **)DMA1_9_NEXT_DESC_PTR
#define pDMA1_9_START_ADDR	(volatile void **)DMA1_9_START_ADDR
#define pDMA1_9_X_COUNT		(volatile unsigned short *)DMA1_9_X_COUNT
#define pDMA1_9_Y_COUNT		(volatile unsigned short *)DMA1_9_Y_COUNT
#define pDMA1_9_X_MODIFY	(volatile unsigned short *)DMA1_9_X_MODIFY
#define pDMA1_9_Y_MODIFY	(volatile unsigned short *)DMA1_9_Y_MODIFY
#define pDMA1_9_CURR_DESC_PTR	(volatile void **)DMA1_9_CURR_DESC_PTR
#define pDMA1_9_CURR_ADDR	(volatile void **)DMA1_9_CURR_ADDR
#define pDMA1_9_CURR_X_COUNT	(volatile unsigned short *)DMA1_9_CURR_X_COUNT
#define pDMA1_9_CURR_Y_COUNT	(volatile unsigned short *)DMA1_9_CURR_Y_COUNT
#define pDMA1_9_IRQ_STATUS	(volatile unsigned short *)DMA1_9_IRQ_STATUS
#define pDMA1_9_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_9_PERIPHERAL_MAP
#define pDMA1_10_CONFIG		(volatile unsigned short *)DMA1_10_CONFIG
#define pDMA1_10_NEXT_DESC_PTR	(volatile void **)DMA1_10_NEXT_DESC_PTR
#define pDMA1_10_START_ADDR	(volatile void **)DMA1_10_START_ADDR
#define pDMA1_10_X_COUNT	(volatile unsigned short *)DMA1_10_X_COUNT
#define pDMA1_10_Y_COUNT	(volatile unsigned short *)DMA1_10_Y_COUNT
#define pDMA1_10_X_MODIFY	(volatile unsigned short *)DMA1_10_X_MODIFY
#define pDMA1_10_Y_MODIFY	(volatile unsigned short *)DMA1_10_Y_MODIFY
#define pDMA1_10_CURR_DESC_PTR	(volatile void **)DMA1_10_CURR_DESC_PTR
#define pDMA1_10_CURR_ADDR	(volatile void **)DMA1_10_CURR_ADDR
#define pDMA1_10_CURR_X_COUNT	(volatile unsigned short *)DMA1_10_CURR_X_COUNT
#define pDMA1_10_CURR_Y_COUNT	(volatile unsigned short *)DMA1_10_CURR_Y_COUNT
#define pDMA1_10_IRQ_STATUS	(volatile unsigned short *)DMA1_10_IRQ_STATUS
#define pDMA1_10_PERIPHERAL_MAP (volatile unsigned short *)DMA1_10_PERIPHERAL_MAP
#define pDMA1_11_CONFIG		(volatile unsigned short *)DMA1_11_CONFIG
#define pDMA1_11_NEXT_DESC_PTR	(volatile void **)DMA1_11_NEXT_DESC_PTR
#define pDMA1_11_START_ADDR	(volatile void **)DMA1_11_START_ADDR
#define pDMA1_11_X_COUNT	(volatile unsigned short *)DMA1_11_X_COUNT
#define pDMA1_11_Y_COUNT	(volatile unsigned short *)DMA1_11_Y_COUNT
#define pDMA1_11_X_MODIFY	(volatile signed short *)DMA1_11_X_MODIFY
#define pDMA1_11_Y_MODIFY	(volatile signed short *)DMA1_11_Y_MODIFY
#define pDMA1_11_CURR_DESC_PTR	(volatile void **)DMA1_11_CURR_DESC_PTR
#define pDMA1_11_CURR_ADDR	(volatile void **)DMA1_11_CURR_ADDR
#define pDMA1_11_CURR_X_COUNT	(volatile unsigned short *)DMA1_11_CURR_X_COUNT
#define pDMA1_11_CURR_Y_COUNT	(volatile unsigned short *)DMA1_11_CURR_Y_COUNT
#define pDMA1_11_IRQ_STATUS	(volatile unsigned short *)DMA1_11_IRQ_STATUS
#define pDMA1_11_PERIPHERAL_MAP (volatile unsigned short *)DMA1_11_PERIPHERAL_MAP

/* Memory DMA1 Controller registers (0xFFC0 1E80-0xFFC0 1FFF)*/
#define pMDMA1_D0_CONFIG	(volatile unsigned short *)MDMA1_D0_CONFIG
#define pMDMA1_D0_NEXT_DESC_PTR (volatile void **)MDMA1_D0_NEXT_DESC_PTR
#define pMDMA1_D0_START_ADDR	(volatile void **)MDMA1_D0_START_ADDR
#define pMDMA1_D0_X_COUNT	(volatile unsigned short *)MDMA1_D0_X_COUNT
#define pMDMA1_D0_Y_COUNT	(volatile unsigned short *)MDMA1_D0_Y_COUNT
#define pMDMA1_D0_X_MODIFY	(volatile signed short *)MDMA1_D0_X_MODIFY
#define pMDMA1_D0_Y_MODIFY	(volatile signed short *)MDMA1_D0_Y_MODIFY
#define pMDMA1_D0_CURR_DESC_PTR (volatile void **)MDMA1_D0_CURR_DESC_PTR
#define pMDMA1_D0_CURR_ADDR	(volatile void **)MDMA1_D0_CURR_ADDR
#define pMDMA1_D0_CURR_X_COUNT	(volatile unsigned short *)MDMA1_D0_CURR_X_COUNT
#define pMDMA1_D0_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_D0_CURR_Y_COUNT
#define pMDMA1_D0_IRQ_STATUS	(volatile unsigned short *)MDMA1_D0_IRQ_STATUS
#define pMDMA1_D0_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_D0_PERIPHERAL_MAP
#define pMDMA1_S0_CONFIG	(volatile unsigned short *)MDMA1_S0_CONFIG
#define pMDMA1_S0_NEXT_DESC_PTR (volatile void **)MDMA1_S0_NEXT_DESC_PTR
#define pMDMA1_S0_START_ADDR	(volatile void **)MDMA1_S0_START_ADDR
#define pMDMA1_S0_X_COUNT	(volatile unsigned short *)MDMA1_S0_X_COUNT
#define pMDMA1_S0_Y_COUNT	(volatile unsigned short *)MDMA1_S0_Y_COUNT
#define pMDMA1_S0_X_MODIFY	(volatile signed short *)MDMA1_S0_X_MODIFY
#define pMDMA1_S0_Y_MODIFY	(volatile signed short *)MDMA1_S0_Y_MODIFY
#define pMDMA1_S0_CURR_DESC_PTR (volatile void **)MDMA1_S0_CURR_DESC_PTR
#define pMDMA1_S0_CURR_ADDR	(volatile void **)MDMA1_S0_CURR_ADDR
#define pMDMA1_S0_CURR_X_COUNT	(volatile unsigned short *)MDMA1_S0_CURR_X_COUNT
#define pMDMA1_S0_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_S0_CURR_Y_COUNT
#define pMDMA1_S0_IRQ_STATUS	(volatile unsigned short *)MDMA1_S0_IRQ_STATUS
#define pMDMA1_S0_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_S0_PERIPHERAL_MAP
#define pMDMA1_D1_CONFIG	(volatile unsigned short *)MDMA1_D1_CONFIG
#define pMDMA1_D1_NEXT_DESC_PTR (volatile void **)MDMA1_D1_NEXT_DESC_PTR
#define pMDMA1_D1_START_ADDR	(volatile void **)MDMA1_D1_START_ADDR
#define pMDMA1_D1_X_COUNT	(volatile unsigned short *)MDMA1_D1_X_COUNT
#define pMDMA1_D1_Y_COUNT	(volatile unsigned short *)MDMA1_D1_Y_COUNT
#define pMDMA1_D1_X_MODIFY	(volatile signed short *)MDMA1_D1_X_MODIFY
#define pMDMA1_D1_Y_MODIFY	(volatile signed short *)MDMA1_D1_Y_MODIFY
#define pMDMA1_D1_CURR_DESC_PTR (volatile void **)MDMA1_D1_CURR_DESC_PTR
#define pMDMA1_D1_CURR_ADDR	(volatile void **)MDMA1_D1_CURR_ADDR
#define pMDMA1_D1_CURR_X_COUNT	(volatile unsigned short *)MDMA1_D1_CURR_X_COUNT
#define pMDMA1_D1_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_D1_CURR_Y_COUNT
#define pMDMA1_D1_IRQ_STATUS	(volatile unsigned short *)MDMA1_D1_IRQ_STATUS
#define pMDMA1_D1_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_D1_PERIPHERAL_MAP
#define pMDMA1_S1_CONFIG	(volatile unsigned short *)MDMA1_S1_CONFIG
#define pMDMA1_S1_NEXT_DESC_PTR (volatile void **)MDMA1_S1_NEXT_DESC_PTR
#define pMDMA1_S1_START_ADDR	(volatile void **)MDMA1_S1_START_ADDR
#define pMDMA1_S1_X_COUNT	(volatile unsigned short *)MDMA1_S1_X_COUNT
#define pMDMA1_S1_Y_COUNT	(volatile unsigned short *)MDMA1_S1_Y_COUNT
#define pMDMA1_S1_X_MODIFY	(volatile signed short *)MDMA1_S1_X_MODIFY
#define pMDMA1_S1_Y_MODIFY	(volatile signed short *)MDMA1_S1_Y_MODIFY
#define pMDMA1_S1_CURR_DESC_PTR (volatile void **)MDMA1_S1_CURR_DESC_PTR
#define pMDMA1_S1_CURR_ADDR	(volatile void **)MDMA1_S1_CURR_ADDR
#define pMDMA1_S1_CURR_X_COUNT	(volatile unsigned short *)MDMA1_S1_CURR_X_COUNT
#define pMDMA1_S1_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_S1_CURR_Y_COUNT
#define pMDMA1_S1_IRQ_STATUS	(volatile unsigned short *)MDMA1_S1_IRQ_STATUS
#define pMDMA1_S1_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_S1_PERIPHERAL_MAP

/* DMA2 Controller registers (0xFFC0 0C00-0xFFC0 0DFF) */
#define pDMA2_0_CONFIG		(volatile unsigned short *)DMA2_0_CONFIG
#define pDMA2_0_NEXT_DESC_PTR	(volatile void **)DMA2_0_NEXT_DESC_PTR
#define pDMA2_0_START_ADDR	(volatile void **)DMA2_0_START_ADDR
#define pDMA2_0_X_COUNT		(volatile unsigned short *)DMA2_0_X_COUNT
#define pDMA2_0_Y_COUNT		(volatile unsigned short *)DMA2_0_Y_COUNT
#define pDMA2_0_X_MODIFY	(volatile signed short *)DMA2_0_X_MODIFY
#define pDMA2_0_Y_MODIFY	(volatile signed short *)DMA2_0_Y_MODIFY
#define pDMA2_0_CURR_DESC_PTR	(volatile void **)DMA2_0_CURR_DESC_PTR
#define pDMA2_0_CURR_ADDR	(volatile void **)DMA2_0_CURR_ADDR
#define pDMA2_0_CURR_X_COUNT	(volatile unsigned short *)DMA2_0_CURR_X_COUNT
#define pDMA2_0_CURR_Y_COUNT	(volatile unsigned short *)DMA2_0_CURR_Y_COUNT
#define pDMA2_0_IRQ_STATUS	(volatile unsigned short *)DMA2_0_IRQ_STATUS
#define pDMA2_0_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_0_PERIPHERAL_MAP
#define pDMA2_1_CONFIG		(volatile unsigned short *)DMA2_1_CONFIG
#define pDMA2_1_NEXT_DESC_PTR	(volatile void **)DMA2_1_NEXT_DESC_PTR
#define pDMA2_1_START_ADDR	(volatile void **)DMA2_1_START_ADDR
#define pDMA2_1_X_COUNT		(volatile unsigned short *)DMA2_1_X_COUNT
#define pDMA2_1_Y_COUNT		(volatile unsigned short *)DMA2_1_Y_COUNT
#define pDMA2_1_X_MODIFY	(volatile signed short *)DMA2_1_X_MODIFY
#define pDMA2_1_Y_MODIFY	(volatile signed short *)DMA2_1_Y_MODIFY
#define pDMA2_1_CURR_DESC_PTR	(volatile void **)DMA2_1_CURR_DESC_PTR
#define pDMA2_1_CURR_ADDR	(volatile void **)DMA2_1_CURR_ADDR
#define pDMA2_1_CURR_X_COUNT	(volatile unsigned short *)DMA2_1_CURR_X_COUNT
#define pDMA2_1_CURR_Y_COUNT	(volatile unsigned short *)DMA2_1_CURR_Y_COUNT
#define pDMA2_1_IRQ_STATUS	(volatile unsigned short *)DMA2_1_IRQ_STATUS
#define pDMA2_1_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_1_PERIPHERAL_MAP
#define pDMA2_2_CONFIG		(volatile unsigned short *)DMA2_2_CONFIG
#define pDMA2_2_NEXT_DESC_PTR	(volatile void **)DMA2_2_NEXT_DESC_PTR
#define pDMA2_2_START_ADDR	(volatile void **)DMA2_2_START_ADDR
#define pDMA2_2_X_COUNT		(volatile unsigned short *)DMA2_2_X_COUNT
#define pDMA2_2_Y_COUNT		(volatile unsigned short *)DMA2_2_Y_COUNT
#define pDMA2_2_X_MODIFY	(volatile signed short *)DMA2_2_X_MODIFY
#define pDMA2_2_Y_MODIFY	(volatile signed short *)DMA2_2_Y_MODIFY
#define pDMA2_2_CURR_DESC_PTR	(volatile void **)DMA2_2_CURR_DESC_PTR
#define pDMA2_2_CURR_ADDR	(volatile void **)DMA2_2_CURR_ADDR
#define pDMA2_2_CURR_X_COUNT	(volatile unsigned short *)DMA2_2_CURR_X_COUNT
#define pDMA2_2_CURR_Y_COUNT	(volatile unsigned short *)DMA2_2_CURR_Y_COUNT
#define pDMA2_2_IRQ_STATUS	(volatile unsigned short *)DMA2_2_IRQ_STATUS
#define pDMA2_2_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_2_PERIPHERAL_MAP
#define pDMA2_3_CONFIG		(volatile unsigned short *)DMA2_3_CONFIG
#define pDMA2_3_NEXT_DESC_PTR	(volatile void **)DMA2_3_NEXT_DESC_PTR
#define pDMA2_3_START_ADDR	(volatile void **)DMA2_3_START_ADDR
#define pDMA2_3_X_COUNT		(volatile unsigned short *)DMA2_3_X_COUNT
#define pDMA2_3_Y_COUNT		(volatile unsigned short *)DMA2_3_Y_COUNT
#define pDMA2_3_X_MODIFY	(volatile signed short *)DMA2_3_X_MODIFY
#define pDMA2_3_Y_MODIFY	(volatile signed short *)DMA2_3_Y_MODIFY
#define pDMA2_3_CURR_DESC_PTR	(volatile void **)DMA2_3_CURR_DESC_PTR
#define pDMA2_3_CURR_ADDR	(volatile void **)DMA2_3_CURR_ADDR
#define pDMA2_3_CURR_X_COUNT	(volatile unsigned short *)DMA2_3_CURR_X_COUNT
#define pDMA2_3_CURR_Y_COUNT	(volatile unsigned short *)DMA2_3_CURR_Y_COUNT
#define pDMA2_3_IRQ_STATUS	(volatile unsigned short *)DMA2_3_IRQ_STATUS
#define pDMA2_3_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_3_PERIPHERAL_MAP
#define pDMA2_4_CONFIG		(volatile unsigned short *)DMA2_4_CONFIG
#define pDMA2_4_NEXT_DESC_PTR	(volatile void **)DMA2_4_NEXT_DESC_PTR
#define pDMA2_4_START_ADDR	(volatile void **)DMA2_4_START_ADDR
#define pDMA2_4_X_COUNT		(volatile unsigned short *)DMA2_4_X_COUNT
#define pDMA2_4_Y_COUNT		(volatile unsigned short *)DMA2_4_Y_COUNT
#define pDMA2_4_X_MODIFY	(volatile signed short *)DMA2_4_X_MODIFY
#define pDMA2_4_Y_MODIFY	(volatile signed short *)DMA2_4_Y_MODIFY
#define pDMA2_4_CURR_DESC_PTR	(volatile void **)DMA2_4_CURR_DESC_PTR
#define pDMA2_4_CURR_ADDR	(volatile void **)DMA2_4_CURR_ADDR
#define pDMA2_4_CURR_X_COUNT	(volatile unsigned short *)DMA2_4_CURR_X_COUNT
#define pDMA2_4_CURR_Y_COUNT	(volatile unsigned short *)DMA2_4_CURR_Y_COUNT
#define pDMA2_4_IRQ_STATUS	(volatile unsigned short *)DMA2_4_IRQ_STATUS
#define pDMA2_4_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_4_PERIPHERAL_MAP
#define pDMA2_5_CONFIG		(volatile unsigned short *)DMA2_5_CONFIG
#define pDMA2_5_NEXT_DESC_PTR	(volatile void **)DMA2_5_NEXT_DESC_PTR
#define pDMA2_5_START_ADDR	(volatile void **)DMA2_5_START_ADDR
#define pDMA2_5_X_COUNT		(volatile unsigned short *)DMA2_5_X_COUNT
#define pDMA2_5_Y_COUNT		(volatile unsigned short *)DMA2_5_Y_COUNT
#define pDMA2_5_X_MODIFY	(volatile signed short *)DMA2_5_X_MODIFY
#define pDMA2_5_Y_MODIFY	(volatile signed short *)DMA2_5_Y_MODIFY
#define pDMA2_5_CURR_DESC_PTR	(volatile void **)DMA2_5_CURR_DESC_PTR
#define pDMA2_5_CURR_ADDR	(volatile void **)DMA2_5_CURR_ADDR
#define pDMA2_5_CURR_X_COUNT	(volatile unsigned short *)DMA2_5_CURR_X_COUNT
#define pDMA2_5_CURR_Y_COUNT	(volatile unsigned short *)DMA2_5_CURR_Y_COUNT
#define pDMA2_5_IRQ_STATUS	(volatile unsigned short *)DMA2_5_IRQ_STATUS
#define pDMA2_5_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_5_PERIPHERAL_MAP
#define pDMA2_6_CONFIG		(volatile unsigned short *)DMA2_6_CONFIG
#define pDMA2_6_NEXT_DESC_PTR	(volatile void **)DMA2_6_NEXT_DESC_PTR
#define pDMA2_6_START_ADDR	(volatile void **)DMA2_6_START_ADDR
#define pDMA2_6_X_COUNT		(volatile unsigned short *)DMA2_6_X_COUNT
#define pDMA2_6_Y_COUNT		(volatile unsigned short *)DMA2_6_Y_COUNT
#define pDMA2_6_X_MODIFY	(volatile signed short *)DMA2_6_X_MODIFY
#define pDMA2_6_Y_MODIFY	(volatile signed short *)DMA2_6_Y_MODIFY
#define pDMA2_6_CURR_DESC_PTR	(volatile void **)DMA2_6_CURR_DESC_PTR
#define pDMA2_6_CURR_ADDR	(volatile void **)DMA2_6_CURR_ADDR
#define pDMA2_6_CURR_X_COUNT	(volatile unsigned short *)DMA2_6_CURR_X_COUNT
#define pDMA2_6_CURR_Y_COUNT	(volatile unsigned short *)DMA2_6_CURR_Y_COUNT
#define pDMA2_6_IRQ_STATUS	(volatile unsigned short *)DMA2_6_IRQ_STATUS
#define pDMA2_6_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_6_PERIPHERAL_MAP
#define pDMA2_7_CONFIG		(volatile unsigned short *)DMA2_7_CONFIG
#define pDMA2_7_NEXT_DESC_PTR	(volatile void **)DMA2_7_NEXT_DESC_PTR
#define pDMA2_7_START_ADDR	(volatile void **)DMA2_7_START_ADDR
#define pDMA2_7_X_COUNT		(volatile unsigned short *)DMA2_7_X_COUNT
#define pDMA2_7_Y_COUNT		(volatile unsigned short *)DMA2_7_Y_COUNT
#define pDMA2_7_X_MODIFY	(volatile signed short *)DMA2_7_X_MODIFY
#define pDMA2_7_Y_MODIFY	(volatile signed short *)DMA2_7_Y_MODIFY
#define pDMA2_7_CURR_DESC_PTR	(volatile void **)DMA2_7_CURR_DESC_PTR
#define pDMA2_7_CURR_ADDR	(volatile void **)DMA2_7_CURR_ADDR
#define pDMA2_7_CURR_X_COUNT	(volatile unsigned short *)DMA2_7_CURR_X_COUNT
#define pDMA2_7_CURR_Y_COUNT	(volatile unsigned short *)DMA2_7_CURR_Y_COUNT
#define pDMA2_7_IRQ_STATUS	(volatile unsigned short *)DMA2_7_IRQ_STATUS
#define pDMA2_7_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_7_PERIPHERAL_MAP
#define pDMA2_8_CONFIG		(volatile unsigned short *)DMA2_8_CONFIG
#define pDMA2_8_NEXT_DESC_PTR	(volatile void **)DMA2_8_NEXT_DESC_PTR
#define pDMA2_8_START_ADDR	(volatile void **)DMA2_8_START_ADDR
#define pDMA2_8_X_COUNT		(volatile unsigned short *)DMA2_8_X_COUNT
#define pDMA2_8_Y_COUNT		(volatile unsigned short *)DMA2_8_Y_COUNT
#define pDMA2_8_X_MODIFY	(volatile signed short *)DMA2_8_X_MODIFY
#define pDMA2_8_Y_MODIFY	(volatile signed short *)DMA2_8_Y_MODIFY
#define pDMA2_8_CURR_DESC_PTR	(volatile void **)DMA2_8_CURR_DESC_PTR
#define pDMA2_8_CURR_ADDR	(volatile void **)DMA2_8_CURR_ADDR
#define pDMA2_8_CURR_X_COUNT	(volatile unsigned short *)DMA2_8_CURR_X_COUNT
#define pDMA2_8_CURR_Y_COUNT	(volatile unsigned short *)DMA2_8_CURR_Y_COUNT
#define pDMA2_8_IRQ_STATUS	(volatile unsigned short *)DMA2_8_IRQ_STATUS
#define pDMA2_8_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_8_PERIPHERAL_MAP
#define pDMA2_9_CONFIG		(volatile unsigned short *)DMA2_9_CONFIG
#define pDMA2_9_NEXT_DESC_PTR	(volatile void **)DMA2_9_NEXT_DESC_PTR
#define pDMA2_9_START_ADDR	(volatile void **)DMA2_9_START_ADDR
#define pDMA2_9_X_COUNT		(volatile unsigned short *)DMA2_9_X_COUNT
#define pDMA2_9_Y_COUNT		(volatile unsigned short *)DMA2_9_Y_COUNT
#define pDMA2_9_X_MODIFY	(volatile signed short *)DMA2_9_X_MODIFY
#define pDMA2_9_Y_MODIFY	(volatile signed short *)DMA2_9_Y_MODIFY
#define pDMA2_9_CURR_DESC_PTR	(volatile void **)DMA2_9_CURR_DESC_PTR
#define pDMA2_9_CURR_ADDR	(volatile void **)DMA2_9_CURR_ADDR
#define pDMA2_9_CURR_X_COUNT	(volatile unsigned short *)DMA2_9_CURR_X_COUNT
#define pDMA2_9_CURR_Y_COUNT	(volatile unsigned short *)DMA2_9_CURR_Y_COUNT
#define pDMA2_9_IRQ_STATUS	(volatile unsigned short *)DMA2_9_IRQ_STATUS
#define pDMA2_9_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_9_PERIPHERAL_MAP
#define pDMA2_10_CONFIG		(volatile unsigned short *)DMA2_10_CONFIG
#define pDMA2_10_NEXT_DESC_PTR	(volatile void **)DMA2_10_NEXT_DESC_PTR
#define pDMA2_10_START_ADDR	(volatile void **)DMA2_10_START_ADDR
#define pDMA2_10_X_COUNT	(volatile unsigned short *)DMA2_10_X_COUNT
#define pDMA2_10_Y_COUNT	(volatile unsigned short *)DMA2_10_Y_COUNT
#define pDMA2_10_X_MODIFY	(volatile signed short *)DMA2_10_X_MODIFY
#define pDMA2_10_Y_MODIFY	(volatile signed short *)DMA2_10_Y_MODIFY
#define pDMA2_10_CURR_DESC_PTR	(volatile void **)DMA2_10_CURR_DESC_PTR
#define pDMA2_10_CURR_ADDR	(volatile void **)DMA2_10_CURR_ADDR
#define pDMA2_10_CURR_X_COUNT	(volatile unsigned short *)DMA2_10_CURR_X_COUNT
#define pDMA2_10_CURR_Y_COUNT	(volatile unsigned short *)DMA2_10_CURR_Y_COUNT
#define pDMA2_10_IRQ_STATUS	(volatile unsigned short *)DMA2_10_IRQ_STATUS
#define pDMA2_10_PERIPHERAL_MAP (volatile unsigned short *)DMA2_10_PERIPHERAL_MAP
#define pDMA2_11_CONFIG		(volatile unsigned short *)DMA2_11_CONFIG
#define pDMA2_11_NEXT_DESC_PTR	(volatile void **)DMA2_11_NEXT_DESC_PTR
#define pDMA2_11_START_ADDR	(volatile void **)DMA2_11_START_ADDR
#define pDMA2_11_X_COUNT	(volatile unsigned short *)DMA2_11_X_COUNT
#define pDMA2_11_Y_COUNT	(volatile unsigned short *)DMA2_11_Y_COUNT
#define pDMA2_11_X_MODIFY	(volatile signed short *)DMA2_11_X_MODIFY
#define pDMA2_11_Y_MODIFY	(volatile signed short *)DMA2_11_Y_MODIFY
#define pDMA2_11_CURR_DESC_PTR	(volatile void **)DMA2_11_CURR_DESC_PTR
#define pDMA2_11_CURR_ADDR	(volatile void **)DMA2_11_CURR_ADDR
#define pDMA2_11_CURR_X_COUNT	(volatile unsigned short *)DMA2_11_CURR_X_COUNT
#define pDMA2_11_CURR_Y_COUNT	(volatile unsigned short *)DMA2_11_CURR_Y_COUNT
#define pDMA2_11_IRQ_STATUS	(volatile unsigned short *)DMA2_11_IRQ_STATUS
#define pDMA2_11_PERIPHERAL_MAP (volatile unsigned short *)DMA2_11_PERIPHERAL_MAP

/* Memory DMA2 Controller registers (0xFFC0 0E80-0xFFC0 0FFF) */
#define pMDMA2_D0_CONFIG	(volatile unsigned short *)MDMA2_D0_CONFIG
#define pMDMA2_D0_NEXT_DESC_PTR (volatile void **)MDMA2_D0_NEXT_DESC_PTR
#define pMDMA2_D0_START_ADDR	(volatile void **)MDMA2_D0_START_ADDR
#define pMDMA2_D0_X_COUNT	(volatile unsigned short *)MDMA2_D0_X_COUNT
#define pMDMA2_D0_Y_COUNT	(volatile unsigned short *)MDMA2_D0_Y_COUNT
#define pMDMA2_D0_X_MODIFY	(volatile signed short *)MDMA2_D0_X_MODIFY
#define pMDMA2_D0_Y_MODIFY	(volatile signed short *)MDMA2_D0_Y_MODIFY
#define pMDMA2_D0_CURR_DESC_PTR (volatile void **)MDMA2_D0_CURR_DESC_PTR
#define pMDMA2_D0_CURR_ADDR	(volatile void **)MDMA2_D0_CURR_ADDR
#define pMDMA2_D0_CURR_X_COUNT	(volatile unsigned short *)MDMA2_D0_CURR_X_COUNT
#define pMDMA2_D0_CURR_Y_COUNT	(volatile unsigned short *)MDMA2_D0_CURR_Y_COUNT
#define pMDMA2_D0_IRQ_STATUS	(volatile unsigned short *)MDMA2_D0_IRQ_STATUS
#define pMDMA2_D0_PERIPHERAL_MAP (volatile unsigned short *)MDMA2_D0_PERIPHERAL_MAP
#define pMDMA2_S0_CONFIG	(volatile unsigned short *)MDMA2_S0_CONFIG
#define pMDMA2_S0_NEXT_DESC_PTR (volatile void **)MDMA2_S0_NEXT_DESC_PTR
#define pMDMA2_S0_START_ADDR	(volatile void **)MDMA2_S0_START_ADDR
#define pMDMA2_S0_X_COUNT	(volatile unsigned short *)MDMA2_S0_X_COUNT
#define pMDMA2_S0_Y_COUNT	(volatile unsigned short *)MDMA2_S0_Y_COUNT
#define pMDMA2_S0_X_MODIFY	(volatile signed short *)MDMA2_S0_X_MODIFY
#define pMDMA2_S0_Y_MODIFY	(volatile signed short *)MDMA2_S0_Y_MODIFY
#define pMDMA2_S0_CURR_DESC_PTR (volatile void **)MDMA2_S0_CURR_DESC_PTR
#define pMDMA2_S0_CURR_ADDR	(volatile void **)MDMA2_S0_CURR_ADDR
#define pMDMA2_S0_CURR_X_COUNT	(volatile unsigned short *)MDMA2_S0_CURR_X_COUNT
#define pMDMA2_S0_CURR_Y_COUNT	(volatile unsigned short *)MDMA2_S0_CURR_Y_COUNT
#define pMDMA2_S0_IRQ_STATUS	(volatile unsigned short *)MDMA2_S0_IRQ_STATUS
#define pMDMA2_S0_PERIPHERAL_MAP (volatile unsigned short *)MDMA2_S0_PERIPHERAL_MAP
#define pMDMA2_D1_CONFIG	(volatile unsigned short *)MDMA2_D1_CONFIG
#define pMDMA2_D1_NEXT_DESC_PTR (volatile void **)MDMA2_D1_NEXT_DESC_PTR
#define pMDMA2_D1_START_ADDR	(volatile void **)MDMA2_D1_START_ADDR
#define pMDMA2_D1_X_COUNT	(volatile unsigned short *)MDMA2_D1_X_COUNT
#define pMDMA2_D1_Y_COUNT	(volatile unsigned short *)MDMA2_D1_Y_COUNT
#define pMDMA2_D1_X_MODIFY	(volatile signed short *)MDMA2_D1_X_MODIFY
#define pMDMA2_D1_Y_MODIFY	(volatile signed short *)MDMA2_D1_Y_MODIFY
#define pMDMA2_D1_CURR_DESC_PTR (volatile void **)MDMA2_D1_CURR_DESC_PTR
#define pMDMA2_D1_CURR_ADDR	(volatile void **)MDMA2_D1_CURR_ADDR
#define pMDMA2_D1_CURR_X_COUNT	(volatile unsigned short *)MDMA2_D1_CURR_X_COUNT
#define pMDMA2_D1_CURR_Y_COUNT	(volatile unsigned short *)MDMA2_D1_CURR_Y_COUNT
#define pMDMA2_D1_IRQ_STATUS	(volatile unsigned short *)MDMA2_D1_IRQ_STATUS
#define pMDMA2_D1_PERIPHERAL_MAP (volatile unsigned short *)MDMA2_D1_PERIPHERAL_MAP
#define pMDMA2_S1_CONFIG	(volatile unsigned short *)MDMA2_S1_CONFIG
#define pMDMA2_S1_NEXT_DESC_PTR (volatile void **)MDMA2_S1_NEXT_DESC_PTR
#define pMDMA2_S1_START_ADDR	(volatile void **)MDMA2_S1_START_ADDR
#define pMDMA2_S1_X_COUNT	(volatile unsigned short *)MDMA2_S1_X_COUNT
#define pMDMA2_S1_Y_COUNT	(volatile unsigned short *)MDMA2_S1_Y_COUNT
#define pMDMA2_S1_X_MODIFY	(volatile signed short *)MDMA2_S1_X_MODIFY
#define pMDMA2_S1_Y_MODIFY	(volatile signed short *)MDMA2_S1_Y_MODIFY
#define pMDMA2_S1_CURR_DESC_PTR (volatile void **)MDMA2_S1_CURR_DESC_PTR
#define pMDMA2_S1_CURR_ADDR	(volatile void **)MDMA2_S1_CURR_ADDR
#define pMDMA2_S1_CURR_X_COUNT	(volatile unsigned short *)MDMA2_S1_CURR_X_COUNT
#define pMDMA2_S1_CURR_Y_COUNT	(volatile unsigned short *)MDMA2_S1_CURR_Y_COUNT
#define pMDMA2_S1_IRQ_STATUS	(volatile unsigned short *)MDMA2_S1_IRQ_STATUS
#define pMDMA2_S1_PERIPHERAL_MAP (volatile unsigned short *)MDMA2_S1_PERIPHERAL_MAP

/* Internal Memory DMA Registers (0xFFC0_1800 - 0xFFC0_19FF) */
#define pIMDMA_D0_CONFIG	(volatile unsigned short *)IMDMA_D0_CONFIG
#define pIMDMA_D0_NEXT_DESC_PTR (volatile void **)IMDMA_D0_NEXT_DESC_PTR
#define pIMDMA_D0_START_ADDR	(volatile void **)IMDMA_D0_START_ADDR
#define pIMDMA_D0_X_COUNT	(volatile unsigned short *)IMDMA_D0_X_COUNT
#define pIMDMA_D0_Y_COUNT	(volatile unsigned short *)IMDMA_D0_Y_COUNT
#define pIMDMA_D0_X_MODIFY	(volatile signed short *)IMDMA_D0_X_MODIFY
#define pIMDMA_D0_Y_MODIFY	(volatile signed short *)IMDMA_D0_Y_MODIFY
#define pIMDMA_D0_CURR_DESC_PTR (volatile void **)IMDMA_D0_CURR_DESC_PTR
#define pIMDMA_D0_CURR_ADDR	(volatile void **)IMDMA_D0_CURR_ADDR
#define pIMDMA_D0_CURR_X_COUNT	(volatile unsigned short *)IMDMA_D0_CURR_X_COUNT
#define pIMDMA_D0_CURR_Y_COUNT	(volatile unsigned short *)IMDMA_D0_CURR_Y_COUNT
#define pIMDMA_D0_IRQ_STATUS	(volatile unsigned short *)IMDMA_D0_IRQ_STATUS
#define pIMDMA_S0_CONFIG	(volatile unsigned short *)IMDMA_S0_CONFIG
#define pIMDMA_S0_NEXT_DESC_PTR (volatile void **)IMDMA_S0_NEXT_DESC_PTR
#define pIMDMA_S0_START_ADDR	(volatile void **)IMDMA_S0_START_ADDR
#define pIMDMA_S0_X_COUNT	(volatile unsigned short *)IMDMA_S0_X_COUNT
#define pIMDMA_S0_Y_COUNT	(volatile unsigned short *)IMDMA_S0_Y_COUNT
#define pIMDMA_S0_X_MODIFY	(volatile signed short *)IMDMA_S0_X_MODIFY
#define pIMDMA_S0_Y_MODIFY	(volatile signed short *)IMDMA_S0_Y_MODIFY
#define pIMDMA_S0_CURR_DESC_PTR (volatile void **)IMDMA_S0_CURR_DESC_PTR
#define pIMDMA_S0_CURR_ADDR	(volatile void **)IMDMA_S0_CURR_ADDR
#define pIMDMA_S0_CURR_X_COUNT	(volatile unsigned short *)IMDMA_S0_CURR_X_COUNT
#define pIMDMA_S0_CURR_Y_COUNT	(volatile unsigned short *)IMDMA_S0_CURR_Y_COUNT
#define pIMDMA_S0_IRQ_STATUS	(volatile unsigned short *)IMDMA_S0_IRQ_STATUS
#define pIMDMA_D1_CONFIG	(volatile unsigned short *)IMDMA_D1_CONFIG
#define pIMDMA_D1_NEXT_DESC_PTR (volatile void **)IMDMA_D1_NEXT_DESC_PTR
#define pIMDMA_D1_START_ADDR	(volatile void **)IMDMA_D1_START_ADDR
#define pIMDMA_D1_X_COUNT	(volatile unsigned short *)IMDMA_D1_X_COUNT
#define pIMDMA_D1_Y_COUNT	(volatile unsigned short *)IMDMA_D1_Y_COUNT
#define pIMDMA_D1_X_MODIFY	(volatile signed short *)IMDMA_D1_X_MODIFY
#define pIMDMA_D1_Y_MODIFY	(volatile signed short *)IMDMA_D1_Y_MODIFY
#define pIMDMA_D1_CURR_DESC_PTR (volatile void **)IMDMA_D1_CURR_DESC_PTR
#define pIMDMA_D1_CURR_ADDR	(volatile void **)IMDMA_D1_CURR_ADDR
#define pIMDMA_D1_CURR_X_COUNT	(volatile unsigned short *)IMDMA_D1_CURR_X_COUNT
#define pIMDMA_D1_CURR_Y_COUNT	(volatile unsigned short *)IMDMA_D1_CURR_Y_COUNT
#define pIMDMA_D1_IRQ_STATUS	(volatile unsigned short *)IMDMA_D1_IRQ_STATUS
#define pIMDMA_S1_CONFIG	(volatile unsigned short *)IMDMA_S1_CONFIG
#define pIMDMA_S1_NEXT_DESC_PTR (volatile void **)IMDMA_S1_NEXT_DESC_PTR
#define pIMDMA_S1_START_ADDR	(volatile void **)IMDMA_S1_START_ADDR
#define pIMDMA_S1_X_COUNT	(volatile unsigned short *)IMDMA_S1_X_COUNT
#define pIMDMA_S1_Y_COUNT	(volatile unsigned short *)IMDMA_S1_Y_COUNT
#define pIMDMA_S1_X_MODIFY	(volatile signed short *)IMDMA_S1_X_MODIFY
#define pIMDMA_S1_Y_MODIFY	(volatile signed short *)IMDMA_S1_Y_MODIFY
#define pIMDMA_S1_CURR_DESC_PTR (volatile void **)IMDMA_S1_CURR_DESC_PTR
#define pIMDMA_S1_CURR_ADDR	(volatile void **)IMDMA_S1_CURR_ADDR
#define pIMDMA_S1_CURR_X_COUNT	(volatile unsigned short *)IMDMA_S1_CURR_X_COUNT
#define pIMDMA_S1_CURR_Y_COUNT	(volatile unsigned short *)IMDMA_S1_CURR_Y_COUNT
#define pIMDMA_S1_IRQ_STATUS	(volatile unsigned short *)IMDMA_S1_IRQ_STATUS

/*
 * System Reset and Interrupt Controller registers for
 * core A (0xFFC0 0100-0xFFC0 01FF)
 */
#define pSWRST			(volatile unsigned short *)SICA_SWRST
#define pSYSCR			(volatile unsigned short *)SICA_SYSCR
#define pRVECT			(volatile unsigned short *)SICA_RVECT
#define pSIC_SWRST		(volatile unsigned short *)SICA_SWRST
#define pSIC_SYSCR		(volatile unsigned short *)SICA_SYSCR
#define pSIC_RVECT		(volatile unsigned short *)SICA_RVECT
#define pSIC_IMASK		(volatile unsigned long *)SICA_IMASK
#define pSIC_IAR0		((volatile unsigned long *)SICA_IAR0)
#define pSIC_IAR1		(volatile unsigned long *)SICA_IAR1
#define pSIC_IAR2		(volatile unsigned long *)SICA_IAR2
#define pSIC_ISR		(volatile unsigned long *)SICA_ISR0
#define pSIC_IWR		(volatile unsigned long *)SICA_IWR0

/* Watchdog Timer registers for Core A (0xFFC0 0200-0xFFC0 02FF) */
#define pWDOG_CTL		(volatile unsigned short *)WDOGA_CTL
#define pWDOG_CNT		(volatile unsigned long *)WDOGA_CNT
#define pWDOG_STAT		(volatile unsigned long *)WDOGA_STAT

/* Programmable Flag 0 registers (0xFFC0 0700-0xFFC0 07FF) */
#define pFIO_FLAG_D		(volatile unsigned short *)FIO0_FLAG_D
#define pFIO_FLAG_C		(volatile unsigned short *)FIO0_FLAG_C
#define pFIO_FLAG_S		(volatile unsigned short *)FIO0_FLAG_S
#define pFIO_FLAG_T		(volatile unsigned short *)FIO0_FLAG_T
#define pFIO_MASKA_D		(volatile unsigned short *)FIO0_MASKA_D
#define pFIO_MASKA_C		(volatile unsigned short *)FIO0_MASKA_C
#define pFIO_MASKA_S		(volatile unsigned short *)FIO0_MASKA_S
#define pFIO_MASKA_T		(volatile unsigned short *)FIO0_MASKA_T
#define pFIO_MASKB_D		(volatile unsigned short *)FIO0_MASKB_D
#define pFIO_MASKB_C		(volatile unsigned short *)FIO0_MASKB_C
#define pFIO_MASKB_S		(volatile unsigned short *)FIO0_MASKB_S
#define pFIO_MASKB_T		(volatile unsigned short *)FIO0_MASKB_T
#define pFIO_DIR		(volatile unsigned short *)FIO0_DIR
#define pFIO_POLAR		(volatile unsigned short *)FIO0_POLAR
#define pFIO_EDGE		(volatile unsigned short *)FIO0_EDGE
#define pFIO_BOTH		(volatile unsigned short *)FIO0_BOTH
#define pFIO_INEN		(volatile unsigned short *)FIO0_INEN

/* Parallel Peripheral Interface (PPI) 0 registers (0xFFC0 1000-0xFFC0 10FF)*/
#define pPPI_CONTROL		(volatile unsigned short *)PPI0_CONTROL
#define pPPI_STATUS		(volatile unsigned short *)PPI0_STATUS
#define pPPI_COUNT		(volatile unsigned short *)PPI0_COUNT
#define pPPI_DELAY		(volatile unsigned short *)PPI0_DELAY
#define pPPI_FRAME		(volatile unsigned short *)PPI0_FRAME

/* DMA1 Controller registers (0xFFC0 1C00-0xFFC0 1FFF) */
#define pDMA0_CONFIG		(volatile unsigned short *)DMA1_0_CONFIG
#define pDMA0_NEXT_DESC_PTR	(volatile void **)DMA1_0_NEXT_DESC_PTR
#define pDMA0_START_ADDR	(volatile void **)DMA1_0_START_ADDR
#define pDMA0_X_COUNT		(volatile unsigned short *)DMA1_0_X_COUNT
#define pDMA0_Y_COUNT		(volatile unsigned short *)DMA1_0_Y_COUNT
#define pDMA0_X_MODIFY		(volatile unsigned short *)DMA1_0_X_MODIFY
#define pDMA0_Y_MODIFY		(volatile unsigned short *)DMA1_0_Y_MODIFY
#define pDMA0_CURR_DESC_PTR	(volatile void **)DMA1_0_CURR_DESC_PTR
#define pDMA0_CURR_ADDR		(volatile void **)DMA1_0_CURR_ADDR
#define pDMA0_CURR_X_COUNT	(volatile unsigned short *)DMA1_0_CURR_X_COUNT
#define pDMA0_CURR_Y_COUNT	(volatile unsigned short *)DMA1_0_CURR_Y_COUNT
#define pDMA0_IRQ_STATUS	(volatile unsigned short *)DMA1_0_IRQ_STATUS
#define pDMA0_PERIPHERAL_MAP	(volatile unsigned short *)DMA1_0_PERIPHERAL_MAP

/* Memory DMA1 Controller registers (0xFFC0 1E80-0xFFC0 1FFF) */
#define pMDMA_D0_CONFIG		(volatile unsigned short *)MDMA1_D0_CONFIG
#define pMDMA_D0_NEXT_DESC_PTR	(volatile void **)MDMA1_D0_NEXT_DESC_PTR
#define pMDMA_D0_START_ADDR	(volatile void **)MDMA1_D0_START_ADDR
#define pMDMA_D0_X_COUNT	(volatile unsigned short *)MDMA1_D0_X_COUNT
#define pMDMA_D0_Y_COUNT	(volatile unsigned short *)MDMA1_D0_Y_COUNT
#define pMDMA_D0_X_MODIFY	(volatile unsigned short *)MDMA1_D0_X_MODIFY
#define pMDMA_D0_Y_MODIFY	(volatile unsigned short *)MDMA1_D0_Y_MODIFY
#define pMDMA_D0_CURR_DESC_PTR	(volatile void **)MDMA1_D0_CURR_DESC_PTR
#define pMDMA_D0_CURR_ADDR	(volatile void **)MDMA1_D0_CURR_ADDR
#define pMDMA_D0_CURR_X_COUNT	(volatile unsigned short *)MDMA1_D0_CURR_X_COUNT
#define pMDMA_D0_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_D0_CURR_Y_COUNT
#define pMDMA_D0_IRQ_STATUS	(volatile unsigned short *)MDMA1_D0_IRQ_STATUS
#define pMDMA_D0_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_D0_PERIPHERAL_MAP
#define pMDMA_S0_CONFIG		(volatile unsigned short *)MDMA1_S0_CONFIG
#define pMDMA_S0_NEXT_DESC_PTR	(volatile void **)MDMA1_S0_NEXT_DESC_PTR
#define pMDMA_S0_START_ADDR	(volatile void **)MDMA1_S0_START_ADDR
#define pMDMA_S0_X_COUNT	(volatile unsigned short *)MDMA1_S0_X_COUNT
#define pMDMA_S0_Y_COUNT	(volatile unsigned short *)MDMA1_S0_Y_COUNT
#define pMDMA_S0_X_MODIFY	(volatile unsigned short *)MDMA1_S0_X_MODIFY
#define pMDMA_S0_Y_MODIFY	(volatile unsigned short *)MDMA1_S0_Y_MODIFY
#define pMDMA_S0_CURR_DESC_PTR	(volatile void **)MDMA1_S0_CURR_DESC_PTR
#define pMDMA_S0_CURR_ADDR	(volatile void **)MDMA1_S0_CURR_ADDR
#define pMDMA_S0_CURR_X_COUNT	(volatile unsigned short *)MDMA1_S0_CURR_X_COUNT
#define pMDMA_S0_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_S0_CURR_Y_COUNT
#define pMDMA_S0_IRQ_STATUS	(volatile unsigned short *)MDMA1_S0_IRQ_STATUS
#define pMDMA_S0_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_S0_PERIPHERAL_MAP
#define pMDMA_D1_CONFIG		(volatile unsigned short *)MDMA1_D1_CONFIG
#define pMDMA_D1_NEXT_DESC_PTR	(volatile void **)MDMA1_D1_NEXT_DESC_PTR
#define pMDMA_D1_START_ADDR	(volatile void **)MDMA1_D1_START_ADDR
#define pMDMA_D1_X_COUNT	(volatile unsigned short *)MDMA1_D1_X_COUNT
#define pMDMA_D1_Y_COUNT	(volatile unsigned short *)MDMA1_D1_Y_COUNT
#define pMDMA_D1_X_MODIFY	(volatile unsigned short *)MDMA1_D1_X_MODIFY
#define pMDMA_D1_Y_MODIFY	(volatile unsigned short *)MDMA1_D1_Y_MODIFY
#define pMDMA_D1_CURR_DESC_PTR	(volatile void **)MDMA1_D1_CURR_DESC_PTR
#define pMDMA_D1_CURR_ADDR	(volatile void **)MDMA1_D1_CURR_ADDR
#define pMDMA_D1_CURR_X_COUNT	(volatile unsigned short *)MDMA1_D1_CURR_X_COUNT
#define pMDMA_D1_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_D1_CURR_Y_COUNT
#define pMDMA_D1_IRQ_STATUS	(volatile unsigned short *)MDMA1_D1_IRQ_STATUS
#define pMDMA_D1_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_D1_PERIPHERAL_MAP
#define pMDMA_S1_CONFIG		(volatile unsigned short *)MDMA1_S1_CONFIG
#define pMDMA_S1_NEXT_DESC_PTR	(volatile void **)MDMA1_S1_NEXT_DESC_PTR
#define pMDMA_S1_START_ADDR	(volatile void **)MDMA1_S1_START_ADDR
#define pMDMA_S1_X_COUNT	(volatile unsigned short *)MDMA1_S1_X_COUNT
#define pMDMA_S1_Y_COUNT	(volatile unsigned short *)MDMA1_S1_Y_COUNT
#define pMDMA_S1_X_MODIFY	(volatile unsigned short *)MDMA1_S1_X_MODIFY
#define pMDMA_S1_Y_MODIFY	(volatile unsigned short *)MDMA1_S1_Y_MODIFY
#define pMDMA_S1_CURR_DESC_PTR	(volatile void **)MDMA1_S1_CURR_DESC_PTR
#define pMDMA_S1_CURR_ADDR	(volatile void **)MDMA1_S1_CURR_ADDR
#define pMDMA_S1_CURR_X_COUNT	(volatile unsigned short *)MDMA1_S1_CURR_X_COUNT
#define pMDMA_S1_CURR_Y_COUNT	(volatile unsigned short *)MDMA1_S1_CURR_Y_COUNT
#define pMDMA_S1_IRQ_STATUS	(volatile unsigned short *)MDMA1_S1_IRQ_STATUS
#define pMDMA_S1_PERIPHERAL_MAP (volatile unsigned short *)MDMA1_S1_PERIPHERAL_MAP

/* DMA2 Controller registers (0xFFC0 0C00-0xFFC0 0DFF) */
#define pDMA1_CONFIG		(volatile unsigned short *)DMA2_0_CONFIG
#define pDMA1_NEXT_DESC_PTR	(volatile void **)DMA2_0_NEXT_DESC_PTR
#define pDMA1_START_ADDR	(volatile void **)DMA2_0_START_ADDR
#define pDMA1_X_COUNT		(volatile unsigned short *)DMA2_0_X_COUNT
#define pDMA1_Y_COUNT		(volatile unsigned short *)DMA2_0_Y_COUNT
#define pDMA1_X_MODIFY		(volatile unsigned short *)DMA2_0_X_MODIFY
#define pDMA1_Y_MODIFY		(volatile unsigned short *)DMA2_0_Y_MODIFY
#define pDMA1_CURR_DESC_PTR	(volatile void **)DMA2_0_CURR_DESC_PTR
#define pDMA1_CURR_ADDR		(volatile void **)DMA2_0_CURR_ADDR
#define pDMA1_CURR_X_COUNT	(volatile unsigned short *)DMA2_0_CURR_X_COUNT
#define pDMA1_CURR_Y_COUNT	(volatile unsigned short *)DMA2_0_CURR_Y_COUNT
#define pDMA1_IRQ_STATUS	(volatile unsigned short *)DMA2_0_IRQ_STATUS
#define pDMA1_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_0_PERIPHERAL_MAP
#define pDMA2_CONFIG		(volatile unsigned short *)DMA2_1_CONFIG
#define pDMA2_NEXT_DESC_PTR	(volatile void **)DMA2_1_NEXT_DESC_PTR
#define pDMA2_START_ADDR	(volatile void **)DMA2_1_START_ADDR
#define pDMA2_X_COUNT		(volatile unsigned short *)DMA2_1_X_COUNT
#define pDMA2_Y_COUNT		(volatile unsigned short *)DMA2_1_Y_COUNT
#define pDMA2_X_MODIFY		(volatile unsigned short *)DMA2_1_X_MODIFY
#define pDMA2_Y_MODIFY		(volatile unsigned short *)DMA2_1_Y_MODIFY
#define pDMA2_CURR_DESC_PTR	(volatile void **)DMA2_1_CURR_DESC_PTR
#define pDMA2_CURR_ADDR		(volatile void **)DMA2_1_CURR_ADDR
#define pDMA2_CURR_X_COUNT	(volatile unsigned short *)DMA2_1_CURR_X_COUNT
#define pDMA2_CURR_Y_COUNT	(volatile unsigned short *)DMA2_1_CURR_Y_COUNT
#define pDMA2_IRQ_STATUS	(volatile unsigned short *)DMA2_1_IRQ_STATUS
#define pDMA2_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_1_PERIPHERAL_MAP
#define pDMA3_CONFIG		(volatile unsigned short *)DMA2_2_CONFIG
#define pDMA3_NEXT_DESC_PTR	(volatile void **)DMA2_2_NEXT_DESC_PTR
#define pDMA3_START_ADDR	(volatile void **)DMA2_2_START_ADDR
#define pDMA3_X_COUNT		(volatile unsigned short *)DMA2_2_X_COUNT
#define pDMA3_Y_COUNT		(volatile unsigned short *)DMA2_2_Y_COUNT
#define pDMA3_X_MODIFY		(volatile unsigned short *)DMA2_2_X_MODIFY
#define pDMA3_Y_MODIFY		(volatile unsigned short *)DMA2_2_Y_MODIFY
#define pDMA3_CURR_DESC_PTR	(volatile void **)DMA2_2_CURR_DESC_PTR
#define pDMA3_CURR_ADDR		(volatile void **)DMA2_2_CURR_ADDR
#define pDMA3_CURR_X_COUNT	(volatile unsigned short *)DMA2_2_CURR_X_COUNT
#define pDMA3_CURR_Y_COUNT	(volatile unsigned short *)DMA2_2_CURR_Y_COUNT
#define pDMA3_IRQ_STATUS	(volatile unsigned short *)DMA2_2_IRQ_STATUS
#define pDMA3_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_2_PERIPHERAL_MAP
#define pDMA4_CONFIG		(volatile unsigned short *)DMA2_3_CONFIG
#define pDMA4_NEXT_DESC_PTR	(volatile void **)DMA2_3_NEXT_DESC_PTR
#define pDMA4_START_ADDR	(volatile void **)DMA2_3_START_ADDR
#define pDMA4_X_COUNT		(volatile unsigned short *)DMA2_3_X_COUNT
#define pDMA4_Y_COUNT		(volatile unsigned short *)DMA2_3_Y_COUNT
#define pDMA4_X_MODIFY		(volatile unsigned short *)DMA2_3_X_MODIFY
#define pDMA4_Y_MODIFY		(volatile unsigned short *)DMA2_3_Y_MODIFY
#define pDMA4_CURR_DESC_PTR	(volatile void **)DMA2_3_CURR_DESC_PTR
#define pDMA4_CURR_ADDR		(volatile void **)DMA2_3_CURR_ADDR
#define pDMA4_CURR_X_COUNT	(volatile unsigned short *)DMA2_3_CURR_X_COUNT
#define pDMA4_CURR_Y_COUNT	(volatile unsigned short *)DMA2_3_CURR_Y_COUNT
#define pDMA4_IRQ_STATUS	(volatile unsigned short *)DMA2_3_IRQ_STATUS
#define pDMA4_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_3_PERIPHERAL_MAP
#define pDMA5_CONFIG		(volatile unsigned short *)DMA2_4_CONFIG
#define pDMA5_NEXT_DESC_PTR	(volatile void **)DMA2_4_NEXT_DESC_PTR
#define pDMA5_START_ADDR	(volatile void **)DMA2_4_START_ADDR
#define pDMA5_X_COUNT		(volatile unsigned short *)DMA2_4_X_COUNT
#define pDMA5_Y_COUNT		(volatile unsigned short *)DMA2_4_Y_COUNT
#define pDMA5_X_MODIFY		(volatile unsigned short *)DMA2_4_X_MODIFY
#define pDMA5_Y_MODIFY		(volatile unsigned short *)DMA2_4_Y_MODIFY
#define pDMA5_CURR_DESC_PTR	(volatile void **)DMA2_4_CURR_DESC_PTR
#define pDMA5_CURR_ADDR		(volatile void **)DMA2_4_CURR_ADDR
#define pDMA5_CURR_X_COUNT	(volatile unsigned short *)DMA2_4_CURR_X_COUNT
#define pDMA5_CURR_Y_COUNT	(volatile unsigned short *)DMA2_4_CURR_Y_COUNT
#define pDMA5_IRQ_STATUS	(volatile unsigned short *)DMA2_4_IRQ_STATUS
#define pDMA5_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_4_PERIPHERAL_MAP
#define pDMA6_CONFIG		(volatile unsigned short *)DMA2_5_CONFIG
#define pDMA6_NEXT_DESC_PTR	(volatile void **)DMA2_5_NEXT_DESC_PTR
#define pDMA6_START_ADDR	(volatile void **)DMA2_5_START_ADDR
#define pDMA6_X_COUNT		(volatile unsigned short *)DMA2_5_X_COUNT
#define pDMA6_Y_COUNT		(volatile unsigned short *)DMA2_5_Y_COUNT
#define pDMA6_X_MODIFY		(volatile unsigned short *)DMA2_5_X_MODIFY
#define pDMA6_Y_MODIFY		(volatile unsigned short *)DMA2_5_Y_MODIFY
#define pDMA6_CURR_DESC_PTR	(volatile void **)DMA2_5_CURR_DESC_PTR
#define pDMA6_CURR_ADDR		(volatile void **)DMA2_5_CURR_ADDR
#define pDMA6_CURR_X_COUNT	(volatile unsigned short *)DMA2_5_CURR_X_COUNT
#define pDMA6_CURR_Y_COUNT	(volatile unsigned short *)DMA2_5_CURR_Y_COUNT
#define pDMA6_IRQ_STATUS	(volatile unsigned short *)DMA2_5_IRQ_STATUS
#define pDMA6_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_5_PERIPHERAL_MAP
#define pDMA7_CONFIG		(volatile unsigned short *)DMA2_6_CONFIG
#define pDMA7_NEXT_DESC_PTR	(volatile void **)DMA2_6_NEXT_DESC_PTR
#define pDMA7_START_ADDR	(volatile void **)DMA2_6_START_ADDR
#define pDMA7_X_COUNT		(volatile unsigned short *)DMA2_6_X_COUNT
#define pDMA7_Y_COUNT		(volatile unsigned short *)DMA2_6_Y_COUNT
#define pDMA7_X_MODIFY		(volatile unsigned short *)DMA2_6_X_MODIFY
#define pDMA7_Y_MODIFY		(volatile unsigned short *)DMA2_6_Y_MODIFY
#define pDMA7_CURR_DESC_PTR	(volatile void **)DMA2_6_CURR_DESC_PTR
#define pDMA7_CURR_ADDR		(volatile void **)DMA2_6_CURR_ADDR
#define pDMA7_CURR_X_COUNT	(volatile unsigned short *)DMA2_6_CURR_X_COUNT
#define pDMA7_CURR_Y_COUNT	(volatile unsigned short *)DMA2_6_CURR_Y_COUNT
#define pDMA7_IRQ_STATUS	(volatile unsigned short *)DMA2_6_IRQ_STATUS
#define pDMA7_PERIPHERAL_MAP	(volatile unsigned short *)DMA2_6_PERIPHERAL_MAP

#endif				/* _CDEF_BF561_H */
