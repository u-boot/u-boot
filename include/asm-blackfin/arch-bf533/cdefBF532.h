/*
 * cdefBF532.h
 *
 * This file is subject to the terms and conditions of the GNU Public
 * License. See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Non-GPL License also available as part of VisualDSP++
 *
 * http://www.analog.com/processors/resources/crosscore/visualDspDevSoftware.html
 *
 * (c) Copyright 2001-2005 Analog Devices, Inc. All rights reserved
 *
 * This file under source code control, please send bugs or changes to:
 * dsptools.support@analog.com
 *
 */

#ifndef _CDEF_BF532_H
#define _CDEF_BF532_H

/*
 * #if !defined(__ADSPLPBLACKFIN__)
 * #warning cdefBF532.h should only be included for 532 compatible chips.
 * #endif
 */

/* include all Core registers and bit definitions */
#include <asm/arch-bf533/defBF532.h>

/* include core specific register pointer definitions */
#include <asm/arch-common/cdef_LPBlackfin.h>

/* Clock and System Control (0xFFC0 0400-0xFFC0 07FF) */
#define pPLL_CTL ((volatile unsigned short *)PLL_CTL)
#define pPLL_STAT ((volatile unsigned short *)PLL_STAT)
#define pPLL_LOCKCNT ((volatile unsigned short *)PLL_LOCKCNT)
#define pCHIPID ((volatile unsigned long *)CHIPID)
#define pSWRST ((volatile unsigned short *)SWRST)
#define pSYSCR ((volatile unsigned short *)SYSCR)
#define pPLL_DIV ((volatile unsigned short *)PLL_DIV)
#define pVR_CTL ((volatile unsigned short *)VR_CTL)

/* System Interrupt Controller (0xFFC0 0C00-0xFFC0 0FFF) */
#define pSIC_IAR0 ((volatile unsigned long *)SIC_IAR0)
#define pSIC_IAR1 ((volatile unsigned long *)SIC_IAR1)
#define pSIC_IAR2 ((volatile unsigned long *)SIC_IAR2)
#define pSIC_IAR3 ((volatile unsigned long *)SIC_IAR3)
#define pSIC_IMASK ((volatile unsigned long *)SIC_IMASK)
#define pSIC_ISR ((volatile unsigned long *)SIC_ISR)
#define pSIC_IWR ((volatile unsigned long *)SIC_IWR)

/* Watchdog Timer (0xFFC0 1000-0xFFC0 13FF) */
#define pWDOG_CTL ((volatile unsigned short *)WDOG_CTL)
#define pWDOG_CNT ((volatile unsigned long *)WDOG_CNT)
#define pWDOG_STAT ((volatile unsigned long *)WDOG_STAT)

/* Real Time Clock (0xFFC0 1400-0xFFC0 17FF) */
#define pRTC_STAT ((volatile unsigned long *)RTC_STAT)
#define pRTC_ICTL ((volatile unsigned short *)RTC_ICTL)
#define pRTC_ISTAT ((volatile unsigned short *)RTC_ISTAT)
#define pRTC_SWCNT ((volatile unsigned short *)RTC_SWCNT)
#define pRTC_ALARM ((volatile unsigned long *)RTC_ALARM)
#define pRTC_FAST ((volatile unsigned short *)RTC_FAST)
#define pRTC_PREN ((volatile unsigned short *)RTC_PREN)

/* General Purpose IO (0xFFC0 2400-0xFFC0 27FF) */
#define pFIO_DIR ((volatile unsigned short *)FIO_DIR)
#define pFIO_FLAG_C ((volatile unsigned short *)FIO_FLAG_C)
#define pFIO_FLAG_S ((volatile unsigned short *)FIO_FLAG_S)
#define pFIO_MASKA_C ((volatile unsigned short *)FIO_MASKA_C)
#define pFIO_MASKA_S ((volatile unsigned short *)FIO_MASKA_S)
#define pFIO_MASKB_C ((volatile unsigned short *)FIO_MASKB_C)
#define pFIO_MASKB_S ((volatile unsigned short *)FIO_MASKB_S)
#define pFIO_POLAR ((volatile unsigned short *)FIO_POLAR)
#define pFIO_EDGE ((volatile unsigned short *)FIO_EDGE)
#define pFIO_BOTH ((volatile unsigned short *)FIO_BOTH)
#define pFIO_INEN ((volatile unsigned short *)FIO_INEN)
#define pFIO_FLAG_D ((volatile unsigned short *)FIO_FLAG_D)
#define pFIO_FLAG_T ((volatile unsigned short *)FIO_FLAG_T)
#define pFIO_MASKA_D ((volatile unsigned short *)FIO_MASKA_D)
#define pFIO_MASKA_T ((volatile unsigned short *)FIO_MASKA_T)
#define pFIO_MASKB_D ((volatile unsigned short *)FIO_MASKB_D)
#define pFIO_MASKB_T ((volatile unsigned short *)FIO_MASKB_T)

/* DMA Test Registers */
#define pDMA_CCOMP	((volatile unsigned long *)DMA_CCOMP)
#define	pDMA_ACOMP	((volatile unsigned long *)DMA_ACOMP)
#define	pDMA_MISR	((volatile unsigned long *)DMA_MISR)
#define	pDMA_TCPER	((volatile unsigned short *)DMA_TCPER)
#define	pDMA_TCCNT	((volatile unsigned short *)DMA_TCCNT)
#define	pDMA_TMODE	((volatile unsigned short *)DMA_TMODE)
#define	pDMA_TMCHAN	((volatile unsigned short *)DMA_TMCHAN)
#define	pDMA_TMSTAT	((volatile unsigned short *)DMA_TMSTAT)
#define	pDMA_TMBD	((volatile unsigned short *)DMA_TMBD)
#define	pDMA_TMM0D	((volatile unsigned short *)DMA_TMM0D)
#define	pDMA_TMM1D	((volatile unsigned short *)DMA_TMM1D)
#define pDMA_TMMA	((volatile void **)DMA_TMMA)

/* DMA Controller */
#define pDMA0_CONFIG ((volatile unsigned short *)DMA0_CONFIG)
#define pDMA0_NEXT_DESC_PTR ((volatile void **)DMA0_NEXT_DESC_PTR)
#define pDMA0_START_ADDR ((volatile void **)DMA0_START_ADDR)
#define pDMA0_X_COUNT ((volatile unsigned short *)DMA0_X_COUNT)
#define pDMA0_Y_COUNT ((volatile unsigned short *)DMA0_Y_COUNT)
#define pDMA0_X_MODIFY ((volatile signed short *)DMA0_X_MODIFY)
#define pDMA0_Y_MODIFY ((volatile signed short *)DMA0_Y_MODIFY)
#define pDMA0_CURR_DESC_PTR ((volatile void **)DMA0_CURR_DESC_PTR)
#define pDMA0_CURR_ADDR ((volatile void **)DMA0_CURR_ADDR)
#define pDMA0_CURR_X_COUNT ((volatile unsigned short *)DMA0_CURR_X_COUNT)
#define pDMA0_CURR_Y_COUNT ((volatile unsigned short *)DMA0_CURR_Y_COUNT)
#define pDMA0_IRQ_STATUS ((volatile unsigned short *)DMA0_IRQ_STATUS)
#define pDMA0_PERIPHERAL_MAP ((volatile unsigned short *)DMA0_PERIPHERAL_MAP)

#define pDMA1_CONFIG ((volatile unsigned short *)DMA1_CONFIG)
#define pDMA1_NEXT_DESC_PTR ((volatile void **)DMA1_NEXT_DESC_PTR)
#define pDMA1_START_ADDR ((volatile void **)DMA1_START_ADDR)
#define pDMA1_X_COUNT ((volatile unsigned short *)DMA1_X_COUNT)
#define pDMA1_Y_COUNT ((volatile unsigned short *)DMA1_Y_COUNT)
#define pDMA1_X_MODIFY ((volatile signed short *)DMA1_X_MODIFY)
#define pDMA1_Y_MODIFY ((volatile signed short *)DMA1_Y_MODIFY)
#define pDMA1_CURR_DESC_PTR ((volatile void **)DMA1_CURR_DESC_PTR)
#define pDMA1_CURR_ADDR ((volatile void **)DMA1_CURR_ADDR)
#define pDMA1_CURR_X_COUNT ((volatile unsigned short *)DMA1_CURR_X_COUNT)
#define pDMA1_CURR_Y_COUNT ((volatile unsigned short *)DMA1_CURR_Y_COUNT)
#define pDMA1_IRQ_STATUS ((volatile unsigned short *)DMA1_IRQ_STATUS)
#define pDMA1_PERIPHERAL_MAP ((volatile unsigned short *)DMA1_PERIPHERAL_MAP)

#define pDMA2_CONFIG ((volatile unsigned short *)DMA2_CONFIG)
#define pDMA2_NEXT_DESC_PTR ((volatile void **)DMA2_NEXT_DESC_PTR)
#define pDMA2_START_ADDR ((volatile void **)DMA2_START_ADDR)
#define pDMA2_X_COUNT ((volatile unsigned short *)DMA2_X_COUNT)
#define pDMA2_Y_COUNT ((volatile unsigned short *)DMA2_Y_COUNT)
#define pDMA2_X_MODIFY ((volatile signed short *)DMA2_X_MODIFY)
#define pDMA2_Y_MODIFY ((volatile signed short *)DMA2_Y_MODIFY)
#define pDMA2_CURR_DESC_PTR ((volatile void **)DMA2_CURR_DESC_PTR)
#define pDMA2_CURR_ADDR ((volatile void **)DMA2_CURR_ADDR)
#define pDMA2_CURR_X_COUNT ((volatile unsigned short *)DMA2_CURR_X_COUNT)
#define pDMA2_CURR_Y_COUNT ((volatile unsigned short *)DMA2_CURR_Y_COUNT)
#define pDMA2_IRQ_STATUS ((volatile unsigned short *)DMA2_IRQ_STATUS)
#define pDMA2_PERIPHERAL_MAP ((volatile unsigned short *)DMA2_PERIPHERAL_MAP)

#define pDMA3_CONFIG ((volatile unsigned short *)DMA3_CONFIG)
#define pDMA3_NEXT_DESC_PTR ((volatile void **)DMA3_NEXT_DESC_PTR)
#define pDMA3_START_ADDR ((volatile void **)DMA3_START_ADDR)
#define pDMA3_X_COUNT ((volatile unsigned short *)DMA3_X_COUNT)
#define pDMA3_Y_COUNT ((volatile unsigned short *)DMA3_Y_COUNT)
#define pDMA3_X_MODIFY ((volatile signed short *)DMA3_X_MODIFY)
#define pDMA3_Y_MODIFY ((volatile signed short *)DMA3_Y_MODIFY)
#define pDMA3_CURR_DESC_PTR ((volatile void **)DMA3_CURR_DESC_PTR)
#define pDMA3_CURR_ADDR ((volatile void **)DMA3_CURR_ADDR)
#define pDMA3_CURR_X_COUNT ((volatile unsigned short *)DMA3_CURR_X_COUNT)
#define pDMA3_CURR_Y_COUNT ((volatile unsigned short *)DMA3_CURR_Y_COUNT)
#define pDMA3_IRQ_STATUS ((volatile unsigned short *)DMA3_IRQ_STATUS)
#define pDMA3_PERIPHERAL_MAP ((volatile unsigned short *)DMA3_PERIPHERAL_MAP)

#define pDMA4_CONFIG ((volatile unsigned short *)DMA4_CONFIG)
#define pDMA4_NEXT_DESC_PTR ((volatile void **)DMA4_NEXT_DESC_PTR)
#define pDMA4_START_ADDR ((volatile void **)DMA4_START_ADDR)
#define pDMA4_X_COUNT ((volatile unsigned short *)DMA4_X_COUNT)
#define pDMA4_Y_COUNT ((volatile unsigned short *)DMA4_Y_COUNT)
#define pDMA4_X_MODIFY ((volatile signed short *)DMA4_X_MODIFY)
#define pDMA4_Y_MODIFY ((volatile signed short *)DMA4_Y_MODIFY)
#define pDMA4_CURR_DESC_PTR ((volatile void **)DMA4_CURR_DESC_PTR)
#define pDMA4_CURR_ADDR ((volatile void **)DMA4_CURR_ADDR)
#define pDMA4_CURR_X_COUNT ((volatile unsigned short *)DMA4_CURR_X_COUNT)
#define pDMA4_CURR_Y_COUNT ((volatile unsigned short *)DMA4_CURR_Y_COUNT)
#define pDMA4_IRQ_STATUS ((volatile unsigned short *)DMA4_IRQ_STATUS)
#define pDMA4_PERIPHERAL_MAP ((volatile unsigned short *)DMA4_PERIPHERAL_MAP)

#define pDMA5_CONFIG ((volatile unsigned short *)DMA5_CONFIG)
#define pDMA5_NEXT_DESC_PTR ((volatile void **)DMA5_NEXT_DESC_PTR)
#define pDMA5_START_ADDR ((volatile void **)DMA5_START_ADDR)
#define pDMA5_X_COUNT ((volatile unsigned short *)DMA5_X_COUNT)
#define pDMA5_Y_COUNT ((volatile unsigned short *)DMA5_Y_COUNT)
#define pDMA5_X_MODIFY ((volatile signed short *)DMA5_X_MODIFY)
#define pDMA5_Y_MODIFY ((volatile signed short *)DMA5_Y_MODIFY)
#define pDMA5_CURR_DESC_PTR ((volatile void **)DMA5_CURR_DESC_PTR)
#define pDMA5_CURR_ADDR ((volatile void **)DMA5_CURR_ADDR)
#define pDMA5_CURR_X_COUNT ((volatile unsigned short *)DMA5_CURR_X_COUNT)
#define pDMA5_CURR_Y_COUNT ((volatile unsigned short *)DMA5_CURR_Y_COUNT)
#define pDMA5_IRQ_STATUS ((volatile unsigned short *)DMA5_IRQ_STATUS)
#define pDMA5_PERIPHERAL_MAP ((volatile unsigned short *)DMA5_PERIPHERAL_MAP)

#define pDMA6_CONFIG ((volatile unsigned short *)DMA6_CONFIG)
#define pDMA6_NEXT_DESC_PTR ((volatile void **)DMA6_NEXT_DESC_PTR)
#define pDMA6_START_ADDR ((volatile void **)DMA6_START_ADDR)
#define pDMA6_X_COUNT ((volatile unsigned short *)DMA6_X_COUNT)
#define pDMA6_Y_COUNT ((volatile unsigned short *)DMA6_Y_COUNT)
#define pDMA6_X_MODIFY ((volatile signed short *)DMA6_X_MODIFY)
#define pDMA6_Y_MODIFY ((volatile signed short *)DMA6_Y_MODIFY)
#define pDMA6_CURR_DESC_PTR ((volatile void **)DMA6_CURR_DESC_PTR)
#define pDMA6_CURR_ADDR ((volatile void **)DMA6_CURR_ADDR)
#define pDMA6_CURR_X_COUNT ((volatile unsigned short *)DMA6_CURR_X_COUNT)
#define pDMA6_CURR_Y_COUNT ((volatile unsigned short *)DMA6_CURR_Y_COUNT)
#define pDMA6_IRQ_STATUS ((volatile unsigned short *)DMA6_IRQ_STATUS)
#define pDMA6_PERIPHERAL_MAP ((volatile unsigned short *)DMA6_PERIPHERAL_MAP)

#define pDMA7_CONFIG ((volatile unsigned short *)DMA7_CONFIG)
#define pDMA7_NEXT_DESC_PTR ((volatile void **)DMA7_NEXT_DESC_PTR)
#define pDMA7_START_ADDR ((volatile void **)DMA7_START_ADDR)
#define pDMA7_X_COUNT ((volatile unsigned short *)DMA7_X_COUNT)
#define pDMA7_Y_COUNT ((volatile unsigned short *)DMA7_Y_COUNT)
#define pDMA7_X_MODIFY ((volatile signed short *)DMA7_X_MODIFY)
#define pDMA7_Y_MODIFY ((volatile signed short *)DMA7_Y_MODIFY)
#define pDMA7_CURR_DESC_PTR ((volatile void **)DMA7_CURR_DESC_PTR)
#define pDMA7_CURR_ADDR ((volatile void **)DMA7_CURR_ADDR)
#define pDMA7_CURR_X_COUNT ((volatile unsigned short *)DMA7_CURR_X_COUNT)
#define pDMA7_CURR_Y_COUNT ((volatile unsigned short *)DMA7_CURR_Y_COUNT)
#define pDMA7_IRQ_STATUS ((volatile unsigned short *)DMA7_IRQ_STATUS)
#define pDMA7_PERIPHERAL_MAP ((volatile unsigned short *)DMA7_PERIPHERAL_MAP)

#define pMDMA_D1_CONFIG ((volatile unsigned short *)MDMA_D1_CONFIG)
#define pMDMA_D1_NEXT_DESC_PTR ((volatile void **)MDMA_D1_NEXT_DESC_PTR)
#define pMDMA_D1_START_ADDR ((volatile void **)MDMA_D1_START_ADDR)
#define pMDMA_D1_X_COUNT ((volatile unsigned short *)MDMA_D1_X_COUNT)
#define pMDMA_D1_Y_COUNT ((volatile unsigned short *)MDMA_D1_Y_COUNT)
#define pMDMA_D1_X_MODIFY ((volatile signed short *)MDMA_D1_X_MODIFY)
#define pMDMA_D1_Y_MODIFY ((volatile signed short *)MDMA_D1_Y_MODIFY)
#define pMDMA_D1_CURR_DESC_PTR ((volatile void **)MDMA_D1_CURR_DESC_PTR)
#define pMDMA_D1_CURR_ADDR ((volatile void **)MDMA_D1_CURR_ADDR)
#define pMDMA_D1_CURR_X_COUNT ((volatile unsigned short *)MDMA_D1_CURR_X_COUNT)
#define pMDMA_D1_CURR_Y_COUNT ((volatile unsigned short *)MDMA_D1_CURR_Y_COUNT)
#define pMDMA_D1_IRQ_STATUS ((volatile unsigned short *)MDMA_D1_IRQ_STATUS)
#define pMDMA_D1_PERIPHERAL_MAP ((volatile unsigned short *)MDMA_D1_PERIPHERAL_MAP)

#define pMDMA_S1_CONFIG ((volatile unsigned short *)MDMA_S1_CONFIG)
#define pMDMA_S1_NEXT_DESC_PTR ((volatile void **)MDMA_S1_NEXT_DESC_PTR)
#define pMDMA_S1_START_ADDR ((volatile void **)MDMA_S1_START_ADDR)
#define pMDMA_S1_X_COUNT ((volatile unsigned short *)MDMA_S1_X_COUNT)
#define pMDMA_S1_Y_COUNT ((volatile unsigned short *)MDMA_S1_Y_COUNT)
#define pMDMA_S1_X_MODIFY ((volatile signed short *)MDMA_S1_X_MODIFY)
#define pMDMA_S1_Y_MODIFY ((volatile signed short *)MDMA_S1_Y_MODIFY)
#define pMDMA_S1_CURR_DESC_PTR ((volatile void **)MDMA_S1_CURR_DESC_PTR)
#define pMDMA_S1_CURR_ADDR ((volatile void **)MDMA_S1_CURR_ADDR)
#define pMDMA_S1_CURR_X_COUNT ((volatile unsigned short *)MDMA_S1_CURR_X_COUNT)
#define pMDMA_S1_CURR_Y_COUNT ((volatile unsigned short *)MDMA_S1_CURR_Y_COUNT)
#define pMDMA_S1_IRQ_STATUS ((volatile unsigned short *)MDMA_S1_IRQ_STATUS)
#define pMDMA_S1_PERIPHERAL_MAP ((volatile unsigned short *)MDMA_S1_PERIPHERAL_MAP)

#define pMDMA_D0_CONFIG ((volatile unsigned short *)MDMA_D0_CONFIG)
#define pMDMA_D0_NEXT_DESC_PTR ((volatile void **)MDMA_D0_NEXT_DESC_PTR)
#define pMDMA_D0_START_ADDR ((volatile void **)MDMA_D0_START_ADDR)
#define pMDMA_D0_X_COUNT ((volatile unsigned short *)MDMA_D0_X_COUNT)
#define pMDMA_D0_Y_COUNT ((volatile unsigned short *)MDMA_D0_Y_COUNT)
#define pMDMA_D0_X_MODIFY ((volatile signed short *)MDMA_D0_X_MODIFY)
#define pMDMA_D0_Y_MODIFY ((volatile signed short *)MDMA_D0_Y_MODIFY)
#define pMDMA_D0_CURR_DESC_PTR ((volatile void **)MDMA_D0_CURR_DESC_PTR)
#define pMDMA_D0_CURR_ADDR ((volatile void **)MDMA_D0_CURR_ADDR)
#define pMDMA_D0_CURR_X_COUNT ((volatile unsigned short *)MDMA_D0_CURR_X_COUNT)
#define pMDMA_D0_CURR_Y_COUNT ((volatile unsigned short *)MDMA_D0_CURR_Y_COUNT)
#define pMDMA_D0_IRQ_STATUS ((volatile unsigned short *)MDMA_D0_IRQ_STATUS)
#define pMDMA_D0_PERIPHERAL_MAP ((volatile unsigned short *)MDMA_D0_PERIPHERAL_MAP)

#define pMDMA_S0_CONFIG ((volatile unsigned short *)MDMA_S0_CONFIG)
#define pMDMA_S0_NEXT_DESC_PTR ((volatile void **)MDMA_S0_NEXT_DESC_PTR)
#define pMDMA_S0_START_ADDR ((volatile void **)MDMA_S0_START_ADDR)
#define pMDMA_S0_X_COUNT ((volatile unsigned short *)MDMA_S0_X_COUNT)
#define pMDMA_S0_Y_COUNT ((volatile unsigned short *)MDMA_S0_Y_COUNT)
#define pMDMA_S0_X_MODIFY ((volatile signed short *)MDMA_S0_X_MODIFY)
#define pMDMA_S0_Y_MODIFY ((volatile signed short *)MDMA_S0_Y_MODIFY)
#define pMDMA_S0_CURR_DESC_PTR ((volatile void **)MDMA_S0_CURR_DESC_PTR)
#define pMDMA_S0_CURR_ADDR ((volatile void **)MDMA_S0_CURR_ADDR)
#define pMDMA_S0_CURR_X_COUNT ((volatile unsigned short *)MDMA_S0_CURR_X_COUNT)
#define pMDMA_S0_CURR_Y_COUNT ((volatile unsigned short *)MDMA_S0_CURR_Y_COUNT)
#define pMDMA_S0_IRQ_STATUS ((volatile unsigned short *)MDMA_S0_IRQ_STATUS)
#define pMDMA_S0_PERIPHERAL_MAP ((volatile unsigned short *)MDMA_S0_PERIPHERAL_MAP)

/* Aysnchronous Memory Controller - External Bus Interface Unit (0xFFC0 3C00-0xFFC0 3FFF) */
#define pEBIU_AMGCTL ((volatile unsigned short *)EBIU_AMGCTL)
#define pEBIU_AMBCTL0 ((volatile unsigned long *)EBIU_AMBCTL0)
#define pEBIU_AMBCTL1 ((volatile unsigned long *)EBIU_AMBCTL1)

/* System Bus Interface Unit (0xFFC0 4800-0xFFC0 4FFF) */
/* #define L1SBAR 0xFFC04840 */	/* L1 SRAM Base Address Register */
/* #define L1CSR  0xFFC04844 */	/* L1 SRAM Control Initialization Register */

/*
 * #define pDB_ACOMP ((volatile void **)DB_ACOMP)
 * #define pDB_CCOMP ((volatile unsigned long *)DB_CCOMP)
 */

/* SDRAM Controller External Bus Interface Unit (0xFFC0 4C00-0xFFC0 4FFF) */
#define pEBIU_SDGCTL ((volatile unsigned long *)EBIU_SDGCTL)
#define pEBIU_SDRRC ((volatile unsigned short *)EBIU_SDRRC)
#define pEBIU_SDSTAT ((volatile unsigned short *)EBIU_SDSTAT)
#define pEBIU_SDBCTL ((volatile unsigned short *)EBIU_SDBCTL)

/* UART Controller */
#define pUART_THR ((volatile unsigned short *)UART_THR)
#define pUART_RBR ((volatile unsigned short *)UART_RBR)
#define pUART_DLL ((volatile unsigned short *)UART_DLL)
#define pUART_IER ((volatile unsigned short *)UART_IER)
#define pUART_DLH ((volatile unsigned short *)UART_DLH)
#define pUART_IIR ((volatile unsigned short *)UART_IIR)
#define pUART_LCR ((volatile unsigned short *)UART_LCR)
#define pUART_MCR ((volatile unsigned short *)UART_MCR)
#define pUART_LSR ((volatile unsigned short *)UART_LSR)

/*
 * #define UART_MSR
 */
#define pUART_SCR ((volatile unsigned short *)UART_SCR)
#define pUART_GCTL ((volatile unsigned short *)UART_GCTL)

/* SPI Controller */
#define pSPI_CTL ((volatile unsigned short *)SPI_CTL)
#define pSPI_FLG ((volatile unsigned short *)SPI_FLG)
#define pSPI_STAT ((volatile unsigned short *)SPI_STAT)
#define pSPI_TDBR ((volatile unsigned short *)SPI_TDBR)
#define pSPI_RDBR ((volatile unsigned short *)SPI_RDBR)
#define pSPI_BAUD ((volatile unsigned short *)SPI_BAUD)
#define pSPI_SHADOW ((volatile unsigned short *)SPI_SHADOW)

/* TIMER 0, 1, 2 Registers */
#define pTIMER0_CONFIG ((volatile unsigned short *)TIMER0_CONFIG)
#define pTIMER0_COUNTER ((volatile unsigned long *)TIMER0_COUNTER)
#define pTIMER0_PERIOD ((volatile unsigned long *)TIMER0_PERIOD)
#define pTIMER0_WIDTH ((volatile unsigned long *)TIMER0_WIDTH)

#define pTIMER1_CONFIG ((volatile unsigned short *)TIMER1_CONFIG)
#define pTIMER1_COUNTER ((volatile unsigned long *)TIMER1_COUNTER)
#define pTIMER1_PERIOD ((volatile unsigned long *)TIMER1_PERIOD)
#define pTIMER1_WIDTH ((volatile unsigned long *)TIMER1_WIDTH)

#define pTIMER2_CONFIG ((volatile unsigned short *)TIMER2_CONFIG)
#define pTIMER2_COUNTER ((volatile unsigned long *)TIMER2_COUNTER)
#define pTIMER2_PERIOD ((volatile unsigned long *)TIMER2_PERIOD)
#define pTIMER2_WIDTH ((volatile unsigned long *)TIMER2_WIDTH)

#define pTIMER_ENABLE ((volatile unsigned short *)TIMER_ENABLE)
#define pTIMER_DISABLE ((volatile unsigned short *)TIMER_DISABLE)
#define pTIMER_STATUS ((volatile unsigned short *)TIMER_STATUS)

/* SPORT0 Controller */
#define pSPORT0_TCR1 ((volatile unsigned short *)SPORT0_TCR1)
#define pSPORT0_TCR2 ((volatile unsigned short *)SPORT0_TCR2)
#define pSPORT0_TCLKDIV ((volatile unsigned short *)SPORT0_TCLKDIV)
#define pSPORT0_TFSDIV ((volatile unsigned short *)SPORT0_TFSDIV)
#define pSPORT0_TX ((volatile long *)SPORT0_TX)
#define pSPORT0_RX ((volatile long *)SPORT0_RX)
#define pSPORT0_TX32 ((volatile long *)SPORT0_TX)
#define pSPORT0_RX32 ((volatile long *)SPORT0_RX)
#define pSPORT0_TX16 ((volatile unsigned short *)SPORT0_TX)
#define pSPORT0_RX16 ((volatile unsigned short *)SPORT0_RX)
#define pSPORT0_RCR1 ((volatile unsigned short *)SPORT0_RCR1)
#define pSPORT0_RCR2 ((volatile unsigned short *)SPORT0_RCR2)
#define pSPORT0_RCLKDIV ((volatile unsigned short *)SPORT0_RCLKDIV)
#define pSPORT0_RFSDIV ((volatile unsigned short *)SPORT0_RFSDIV)
#define pSPORT0_STAT ((volatile unsigned short *)SPORT0_STAT)
#define pSPORT0_CHNL ((volatile unsigned short *)SPORT0_CHNL)
#define pSPORT0_MCMC1 ((volatile unsigned short *)SPORT0_MCMC1)
#define pSPORT0_MCMC2 ((volatile unsigned short *)SPORT0_MCMC2)
#define pSPORT0_MTCS0 ((volatile unsigned long *)SPORT0_MTCS0)
#define pSPORT0_MTCS1 ((volatile unsigned long *)SPORT0_MTCS1)
#define pSPORT0_MTCS2 ((volatile unsigned long *)SPORT0_MTCS2)
#define pSPORT0_MTCS3 ((volatile unsigned long *)SPORT0_MTCS3)
#define pSPORT0_MRCS0 ((volatile unsigned long *)SPORT0_MRCS0)
#define pSPORT0_MRCS1 ((volatile unsigned long *)SPORT0_MRCS1)
#define pSPORT0_MRCS2 ((volatile unsigned long *)SPORT0_MRCS2)
#define pSPORT0_MRCS3 ((volatile unsigned long *)SPORT0_MRCS3)

/* SPORT1 Controller */
#define pSPORT1_TCR1 ((volatile unsigned short *)SPORT1_TCR1)
#define pSPORT1_TCR2 ((volatile unsigned short *)SPORT1_TCR2)
#define pSPORT1_TCLKDIV ((volatile unsigned short *)SPORT1_TCLKDIV)
#define pSPORT1_TFSDIV ((volatile unsigned short *)SPORT1_TFSDIV)
#define pSPORT1_TX ((volatile long *)SPORT1_TX)
#define pSPORT1_RX ((volatile long *)SPORT1_RX)
#define pSPORT1_TX32 ((volatile long *)SPORT1_TX)
#define pSPORT1_RX32 ((volatile long *)SPORT1_RX)
#define pSPORT1_TX16 ((volatile unsigned short *)SPORT1_TX)
#define pSPORT1_RX16 ((volatile unsigned short *)SPORT1_RX)
#define pSPORT1_RCR1 ((volatile unsigned short *)SPORT1_RCR1)
#define pSPORT1_RCR2 ((volatile unsigned short *)SPORT1_RCR2)
#define pSPORT1_RCLKDIV ((volatile unsigned short *)SPORT1_RCLKDIV)
#define pSPORT1_RFSDIV ((volatile unsigned short *)SPORT1_RFSDIV)
#define pSPORT1_STAT ((volatile unsigned short *)SPORT1_STAT)
#define pSPORT1_CHNL ((volatile unsigned short *)SPORT1_CHNL)
#define pSPORT1_MCMC1 ((volatile unsigned short *)SPORT1_MCMC1)
#define pSPORT1_MCMC2 ((volatile unsigned short *)SPORT1_MCMC2)
#define pSPORT1_MTCS0 ((volatile unsigned long *)SPORT1_MTCS0)
#define pSPORT1_MTCS1 ((volatile unsigned long *)SPORT1_MTCS1)
#define pSPORT1_MTCS2 ((volatile unsigned long *)SPORT1_MTCS2)
#define pSPORT1_MTCS3 ((volatile unsigned long *)SPORT1_MTCS3)
#define pSPORT1_MRCS0 ((volatile unsigned long *)SPORT1_MRCS0)
#define pSPORT1_MRCS1 ((volatile unsigned long *)SPORT1_MRCS1)
#define pSPORT1_MRCS2 ((volatile unsigned long *)SPORT1_MRCS2)
#define pSPORT1_MRCS3 ((volatile unsigned long *)SPORT1_MRCS3)

/* Parallel Peripheral Interface (PPI) */
#define pPPI_CONTROL ((volatile unsigned short *)PPI_CONTROL)
#define pPPI_STATUS ((volatile unsigned short *)PPI_STATUS)
#define pPPI_DELAY ((volatile unsigned short *)PPI_DELAY)
#define pPPI_COUNT ((volatile unsigned short *)PPI_COUNT)
#define pPPI_FRAME ((volatile unsigned short *)PPI_FRAME)

#endif	/* _CDEF_BF532_H */
