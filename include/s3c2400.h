/*
 * (C) Copyright 2003
 * David Müller ELSOFT AG Switzerland. d.mueller@elsoft.ch
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

/************************************************
 * NAME	    : s3c2400.h
 * Version  : 31.3.2003
 *
 * Based on S3C2400X User's manual Rev 1.1
 ************************************************/

#ifndef __S3C2400_H__
#define __S3C2400_H__

#define S3C24X0_UART_CHANNELS	2
#define S3C24X0_SPI_CHANNELS	1
#define PALETTE			(0x14A00400)	/* SJS */

typedef enum {
	S3C24X0_UART0,
	S3C24X0_UART1,
} S3C24X0_UARTS_NR;

/* S3C2400 device base addresses */
#define S3C24X0_MEMCTL_BASE		0x14000000
#define S3C24X0_USB_HOST_BASE		0x14200000
#define S3C24X0_INTERRUPT_BASE		0x14400000
#define S3C24X0_DMA_BASE		0x14600000
#define S3C24X0_CLOCK_POWER_BASE	0x14800000
#define S3C24X0_LCD_BASE		0x14A00000
#define S3C24X0_UART_BASE		0x15000000
#define S3C24X0_TIMER_BASE		0x15100000
#define S3C24X0_USB_DEVICE_BASE		0x15200140
#define S3C24X0_WATCHDOG_BASE		0x15300000
#define S3C24X0_I2C_BASE		0x15400000
#define S3C24X0_I2S_BASE		0x15508000
#define S3C24X0_GPIO_BASE		0x15600000
#define S3C24X0_RTC_BASE		0x15700000
#define S3C24X0_ADC_BASE		0x15800000
#define S3C24X0_SPI_BASE		0x15900000
#define S3C2400_MMC_BASE		0x15A00000

/* include common stuff */
#include <s3c24x0.h>


static inline S3C24X0_MEMCTL * const S3C24X0_GetBase_MEMCTL(void)
{
	return (S3C24X0_MEMCTL * const)S3C24X0_MEMCTL_BASE;
}
static inline S3C24X0_USB_HOST * const S3C24X0_GetBase_USB_HOST(void)
{
	return (S3C24X0_USB_HOST * const)S3C24X0_USB_HOST_BASE;
}
static inline S3C24X0_INTERRUPT * const S3C24X0_GetBase_INTERRUPT(void)
{
	return (S3C24X0_INTERRUPT * const)S3C24X0_INTERRUPT_BASE;
}
static inline S3C24X0_DMAS * const S3C24X0_GetBase_DMAS(void)
{
	return (S3C24X0_DMAS * const)S3C24X0_DMA_BASE;
}
static inline S3C24X0_CLOCK_POWER * const S3C24X0_GetBase_CLOCK_POWER(void)
{
	return (S3C24X0_CLOCK_POWER * const)S3C24X0_CLOCK_POWER_BASE;
}
static inline S3C24X0_LCD * const S3C24X0_GetBase_LCD(void)
{
	return (S3C24X0_LCD * const)S3C24X0_LCD_BASE;
}
static inline S3C24X0_UART * const S3C24X0_GetBase_UART(S3C24X0_UARTS_NR nr)
{
	return (S3C24X0_UART * const)(S3C24X0_UART_BASE + (nr * 0x4000));
}
static inline S3C24X0_TIMERS * const S3C24X0_GetBase_TIMERS(void)
{
	return (S3C24X0_TIMERS * const)S3C24X0_TIMER_BASE;
}
static inline S3C24X0_USB_DEVICE * const S3C24X0_GetBase_USB_DEVICE(void)
{
	return (S3C24X0_USB_DEVICE * const)S3C24X0_USB_DEVICE_BASE;
}
static inline S3C24X0_WATCHDOG * const S3C24X0_GetBase_WATCHDOG(void)
{
	return (S3C24X0_WATCHDOG * const)S3C24X0_WATCHDOG_BASE;
}
static inline S3C24X0_I2C * const S3C24X0_GetBase_I2C(void)
{
	return (S3C24X0_I2C * const)S3C24X0_I2C_BASE;
}
static inline S3C24X0_I2S * const S3C24X0_GetBase_I2S(void)
{
	return (S3C24X0_I2S * const)S3C24X0_I2S_BASE;
}
static inline S3C24X0_GPIO * const S3C24X0_GetBase_GPIO(void)
{
	return (S3C24X0_GPIO * const)S3C24X0_GPIO_BASE;
}
static inline S3C24X0_RTC * const S3C24X0_GetBase_RTC(void)
{
	return (S3C24X0_RTC * const)S3C24X0_RTC_BASE;
}
static inline S3C2400_ADC * const S3C2400_GetBase_ADC(void)
{
	return (S3C2400_ADC * const)S3C24X0_ADC_BASE;
}
static inline S3C24X0_SPI * const S3C24X0_GetBase_SPI(void)
{
	return (S3C24X0_SPI * const)S3C24X0_SPI_BASE;
}
static inline S3C2400_MMC * const S3C2400_GetBase_MMC(void)
{
	return (S3C2400_MMC * const)S3C2400_MMC_BASE;
}

#if 0
/* Memory control */
#define rBWSCON		(*(volatile unsigned *)0x14000000)
#define rBANKCON0	(*(volatile unsigned *)0x14000004)
#define rBANKCON1	(*(volatile unsigned *)0x14000008)
#define rBANKCON2	(*(volatile unsigned *)0x1400000C)
#define rBANKCON3	(*(volatile unsigned *)0x14000010)
#define rBANKCON4	(*(volatile unsigned *)0x14000014)
#define rBANKCON5	(*(volatile unsigned *)0x14000018)
#define rBANKCON6	(*(volatile unsigned *)0x1400001C)
#define rBANKCON7	(*(volatile unsigned *)0x14000020)
#define rREFRESH	(*(volatile unsigned *)0x14000024)
#define rBANKSIZE	(*(volatile unsigned *)0x14000028)
#define rMRSRB6		(*(volatile unsigned *)0x1400002C)
#define rMRSRB7		(*(volatile unsigned *)0x14000030)


/* INTERRUPT */
#define rSRCPND		(*(volatile unsigned *)0x14400000)
#define rINTMOD		(*(volatile unsigned *)0x14400004)
#define rINTMSK		(*(volatile unsigned *)0x14400008)
#define rPRIORITY	(*(volatile unsigned *)0x1440000C)
#define rINTPND		(*(volatile unsigned *)0x14400010)
#define rINTOFFSET	(*(volatile unsigned *)0x14400014)


/* DMA */
#define rDISRC0		(*(volatile unsigned *)0x14600000)
#define rDIDST0		(*(volatile unsigned *)0x14600004)
#define rDCON0		(*(volatile unsigned *)0x14600008)
#define rDSTAT0		(*(volatile unsigned *)0x1460000C)
#define rDCSRC0		(*(volatile unsigned *)0x14600010)
#define rDCDST0		(*(volatile unsigned *)0x14600014)
#define rDMASKTRIG0	(*(volatile unsigned *)0x14600018)
#define rDISRC1		(*(volatile unsigned *)0x14600020)
#define rDIDST1		(*(volatile unsigned *)0x14600024)
#define rDCON1		(*(volatile unsigned *)0x14600028)
#define rDSTAT1		(*(volatile unsigned *)0x1460002C)
#define rDCSRC1		(*(volatile unsigned *)0x14600030)
#define rDCDST1		(*(volatile unsigned *)0x14600034)
#define rDMASKTRIG1	(*(volatile unsigned *)0x14600038)
#define rDISRC2		(*(volatile unsigned *)0x14600040)
#define rDIDST2		(*(volatile unsigned *)0x14600044)
#define rDCON2		(*(volatile unsigned *)0x14600048)
#define rDSTAT2		(*(volatile unsigned *)0x1460004C)
#define rDCSRC2		(*(volatile unsigned *)0x14600050)
#define rDCDST2		(*(volatile unsigned *)0x14600054)
#define rDMASKTRIG2	(*(volatile unsigned *)0x14600058)
#define rDISRC3		(*(volatile unsigned *)0x14600060)
#define rDIDST3		(*(volatile unsigned *)0x14600064)
#define rDCON3		(*(volatile unsigned *)0x14600068)
#define rDSTAT3		(*(volatile unsigned *)0x1460006C)
#define rDCSRC3		(*(volatile unsigned *)0x14600070)
#define rDCDST3		(*(volatile unsigned *)0x14600074)
#define rDMASKTRIG3	(*(volatile unsigned *)0x14600078)


/* CLOCK & POWER MANAGEMENT */
#define rLOCKTIME	(*(volatile unsigned *)0x14800000)
#define rMPLLCON	(*(volatile unsigned *)0x14800004)
#define rUPLLCON	(*(volatile unsigned *)0x14800008)
#define rCLKCON		(*(volatile unsigned *)0x1480000C)
#define rCLKSLOW	(*(volatile unsigned *)0x14800010)
#define rCLKDIVN	(*(volatile unsigned *)0x14800014)


/* LCD CONTROLLER */
#define rLCDCON1	(*(volatile unsigned *)0x14A00000)
#define rLCDCON2	(*(volatile unsigned *)0x14A00004)
#define rLCDCON3	(*(volatile unsigned *)0x14A00008)
#define rLCDCON4	(*(volatile unsigned *)0x14A0000C)
#define rLCDCON5	(*(volatile unsigned *)0x14A00010)
#define rLCDSADDR1	(*(volatile unsigned *)0x14A00014)
#define rLCDSADDR2	(*(volatile unsigned *)0x14A00018)
#define rLCDSADDR3	(*(volatile unsigned *)0x14A0001C)
#define rREDLUT		(*(volatile unsigned *)0x14A00020)
#define rGREENLUT	(*(volatile unsigned *)0x14A00024)
#define rBLUELUT	(*(volatile unsigned *)0x14A00028)
#define rDP1_2		(*(volatile unsigned *)0x14A0002C)
#define rDP4_7		(*(volatile unsigned *)0x14A00030)
#define rDP3_5		(*(volatile unsigned *)0x14A00034)
#define rDP2_3		(*(volatile unsigned *)0x14A00038)
#define rDP5_7		(*(volatile unsigned *)0x14A0003c)
#define rDP3_4		(*(volatile unsigned *)0x14A00040)
#define rDP4_5		(*(volatile unsigned *)0x14A00044)
#define rDP6_7		(*(volatile unsigned *)0x14A00048)
#define rDITHMODE	(*(volatile unsigned *)0x14A0004C)
#define rTPAL		(*(volatile unsigned *)0x14A00050)
#define PALETTE		(0x14A00400)	/* SJS */


/* UART */
#define rULCON0		(*(volatile unsigned char *)0x15000000)
#define rUCON0		(*(volatile unsigned short *)0x15000004)
#define rUFCON0		(*(volatile unsigned char *)0x15000008)
#define rUMCON0		(*(volatile unsigned char *)0x1500000C)
#define rUTRSTAT0	(*(volatile unsigned char *)0x15000010)
#define rUERSTAT0	(*(volatile unsigned char *)0x15000014)
#define rUFSTAT0	(*(volatile unsigned short *)0x15000018)
#define rUMSTAT0	(*(volatile unsigned char *)0x1500001C)
#define rUBRDIV0	(*(volatile unsigned short *)0x15000028)

#define rULCON1		(*(volatile unsigned char *)0x15004000)
#define rUCON1		(*(volatile unsigned short *)0x15004004)
#define rUFCON1		(*(volatile unsigned char *)0x15004008)
#define rUMCON1		(*(volatile unsigned char *)0x1500400C)
#define rUTRSTAT1	(*(volatile unsigned char *)0x15004010)
#define rUERSTAT1	(*(volatile unsigned char *)0x15004014)
#define rUFSTAT1	(*(volatile unsigned short *)0x15004018)
#define rUMSTAT1	(*(volatile unsigned char *)0x1500401C)
#define rUBRDIV1	(*(volatile unsigned short *)0x15004028)

#ifdef __BIG_ENDIAN
#define rUTXH0		(*(volatile unsigned char *)0x15000023)
#define rURXH0		(*(volatile unsigned char *)0x15000027)
#define rUTXH1		(*(volatile unsigned char *)0x15004023)
#define rURXH1		(*(volatile unsigned char *)0x15004027)

#define WrUTXH0(ch)	(*(volatile unsigned char *)0x15000023)=(unsigned char)(ch)
#define RdURXH0()	(*(volatile unsigned char *)0x15000027)
#define WrUTXH1(ch)	(*(volatile unsigned char *)0x15004023)=(unsigned char)(ch)
#define RdURXH1()	(*(volatile unsigned char *)0x15004027)

#define UTXH0		(0x15000020+3)  /* byte_access address by DMA */
#define URXH0		(0x15000024+3)
#define UTXH1		(0x15004020+3)
#define URXH1		(0x15004024+3)

#else /* Little Endian */
#define rUTXH0		(*(volatile unsigned char *)0x15000020)
#define rURXH0		(*(volatile unsigned char *)0x15000024)
#define rUTXH1		(*(volatile unsigned char *)0x15004020)
#define rURXH1		(*(volatile unsigned char *)0x15004024)

#define WrUTXH0(ch)	(*(volatile unsigned char *)0x15000020)=(unsigned char)(ch)
#define RdURXH0()	(*(volatile unsigned char *)0x15000024)
#define WrUTXH1(ch)	(*(volatile unsigned char *)0x15004020)=(unsigned char)(ch)
#define RdURXH1()	(*(volatile unsigned char *)0x15004024)

#define UTXH0		(0x15000020)    /* byte_access address by DMA */
#define URXH0		(0x15000024)
#define UTXH1		(0x15004020)
#define URXH1		(0x15004024)
#endif


/* PWM TIMER */
#define rTCFG0		(*(volatile unsigned *)0x15100000)
#define rTCFG1		(*(volatile unsigned *)0x15100004)
#define rTCON		(*(volatile unsigned *)0x15100008)
#define rTCNTB0		(*(volatile unsigned *)0x1510000C)
#define rTCMPB0		(*(volatile unsigned *)0x15100010)
#define rTCNTO0		(*(volatile unsigned *)0x15100014)
#define rTCNTB1		(*(volatile unsigned *)0x15100018)
#define rTCMPB1		(*(volatile unsigned *)0x1510001C)
#define rTCNTO1		(*(volatile unsigned *)0x15100020)
#define rTCNTB2		(*(volatile unsigned *)0x15100024)
#define rTCMPB2		(*(volatile unsigned *)0x15100028)
#define rTCNTO2		(*(volatile unsigned *)0x1510002C)
#define rTCNTB3		(*(volatile unsigned *)0x15100030)
#define rTCMPB3		(*(volatile unsigned *)0x15100034)
#define rTCNTO3		(*(volatile unsigned *)0x15100038)
#define rTCNTB4		(*(volatile unsigned *)0x1510003C)
#define rTCNTO4		(*(volatile unsigned *)0x15100040)


/* USB DEVICE */
#define rFUNC_ADDR_REG	(*(volatile unsigned *)0x15200140)
#define rPWR_REG	(*(volatile unsigned *)0x15200144)
#define rINT_REG	(*(volatile unsigned *)0x15200148)
#define rINT_MASK_REG	(*(volatile unsigned *)0x1520014C)
#define rFRAME_NUM_REG	(*(volatile unsigned *)0x15200150)
#define rRESUME_CON_REG	(*(volatile unsigned *)0x15200154)
#define rEP0_CSR	(*(volatile unsigned *)0x15200160)
#define rEP0_MAXP	(*(volatile unsigned *)0x15200164)
#define rEP0_OUT_CNT	(*(volatile unsigned *)0x15200168)
#define rEP0_FIFO	(*(volatile unsigned *)0x1520016C)
#define rEP1_IN_CSR	(*(volatile unsigned *)0x15200180)
#define rEP1_IN_MAXP	(*(volatile unsigned *)0x15200184)
#define rEP1_FIFO	(*(volatile unsigned *)0x15200188)
#define rEP2_IN_CSR	(*(volatile unsigned *)0x15200190)
#define rEP2_IN_MAXP	(*(volatile unsigned *)0x15200194)
#define rEP2_FIFO	(*(volatile unsigned *)0x15200198)
#define rEP3_OUT_CSR	(*(volatile unsigned *)0x152001A0)
#define rEP3_OUT_MAXP	(*(volatile unsigned *)0x152001A4)
#define rEP3_OUT_CNT	(*(volatile unsigned *)0x152001A8)
#define rEP3_FIFO	(*(volatile unsigned *)0x152001AC)
#define rEP4_OUT_CSR	(*(volatile unsigned *)0x152001B0)
#define rEP4_OUT_MAXP	(*(volatile unsigned *)0x152001B4)
#define rEP4_OUT_CNT	(*(volatile unsigned *)0x152001B8)
#define rEP4_FIFO	(*(volatile unsigned *)0x152001BC)
#define rDMA_CON	(*(volatile unsigned *)0x152001C0)
#define rDMA_UNIT	(*(volatile unsigned *)0x152001C4)
#define rDMA_FIFO	(*(volatile unsigned *)0x152001C8)
#define rDMA_TX		(*(volatile unsigned *)0x152001CC)
#define rTEST_MODE	(*(volatile unsigned *)0x152001F4)
#define rIN_CON_REG	(*(volatile unsigned *)0x152001F8)


/* WATCH DOG TIMER */
#define rWTCON		(*(volatile unsigned *)0x15300000)
#define rWTDAT		(*(volatile unsigned *)0x15300004)
#define rWTCNT		(*(volatile unsigned *)0x15300008)


/* IIC */
#define rIICCON		(*(volatile unsigned *)0x15400000)
#define rIICSTAT	(*(volatile unsigned *)0x15400004)
#define rIICADD		(*(volatile unsigned *)0x15400008)
#define rIICDS		(*(volatile unsigned *)0x1540000C)


/* IIS */
#define rIISCON		(*(volatile unsigned *)0x15508000)
#define rIISMOD		(*(volatile unsigned *)0x15508004)
#define rIISPSR		(*(volatile unsigned *)0x15508008)
#define rIISFIFCON	(*(volatile unsigned *)0x1550800C)

#ifdef __BIG_ENDIAN
#define IISFIF		((volatile unsigned short *)0x15508012)

#else /* Little Endian */
#define IISFIF		((volatile unsigned short *)0x15508010)
#endif


/* I/O PORT */
#define rPACON		(*(volatile unsigned *)0x15600000)
#define rPADAT		(*(volatile unsigned *)0x15600004)

#define rPBCON		(*(volatile unsigned *)0x15600008)
#define rPBDAT		(*(volatile unsigned *)0x1560000C)
#define rPBUP		(*(volatile unsigned *)0x15600010)

#define rPCCON		(*(volatile unsigned *)0x15600014)
#define rPCDAT		(*(volatile unsigned *)0x15600018)
#define rPCUP		(*(volatile unsigned *)0x1560001C)

#define rPDCON		(*(volatile unsigned *)0x15600020)
#define rPDDAT		(*(volatile unsigned *)0x15600024)
#define rPDUP		(*(volatile unsigned *)0x15600028)

#define rPECON		(*(volatile unsigned *)0x1560002C)
#define rPEDAT		(*(volatile unsigned *)0x15600030)
#define rPEUP		(*(volatile unsigned *)0x15600034)

#define rPFCON		(*(volatile unsigned *)0x15600038)
#define rPFDAT		(*(volatile unsigned *)0x1560003C)
#define rPFUP		(*(volatile unsigned *)0x15600040)

#define rPGCON		(*(volatile unsigned *)0x15600044)
#define rPGDAT		(*(volatile unsigned *)0x15600048)
#define rPGUP		(*(volatile unsigned *)0x1560004C)

#define rOPENCR		(*(volatile unsigned *)0x15600050)
#define rMISCCR		(*(volatile unsigned *)0x15600054)
#define rEXTINT		(*(volatile unsigned *)0x15600058)


/* RTC */
#ifdef __BIG_ENDIAN
#define rRTCCON		(*(volatile unsigned char *)0x15700043)
#define rRTCALM		(*(volatile unsigned char *)0x15700053)
#define rALMSEC		(*(volatile unsigned char *)0x15700057)
#define rALMMIN		(*(volatile unsigned char *)0x1570005B)
#define rALMHOUR	(*(volatile unsigned char *)0x1570005F)
#define rALMDAY		(*(volatile unsigned char *)0x15700063)
#define rALMMON		(*(volatile unsigned char *)0x15700067)
#define rALMYEAR	(*(volatile unsigned char *)0x1570006B)
#define rRTCRST		(*(volatile unsigned char *)0x1570006F)
#define rBCDSEC		(*(volatile unsigned char *)0x15700073)
#define rBCDMIN		(*(volatile unsigned char *)0x15700077)
#define rBCDHOUR	(*(volatile unsigned char *)0x1570007B)
#define rBCDDAY		(*(volatile unsigned char *)0x1570007F)
#define rBCDDATE	(*(volatile unsigned char *)0x15700083)
#define rBCDMON		(*(volatile unsigned char *)0x15700087)
#define rBCDYEAR	(*(volatile unsigned char *)0x1570008B)
#define rTICINT		(*(volatile unsigned char *)0x15700047)

#else /* Little Endian */
#define rRTCCON		(*(volatile unsigned char *)0x15700040)
#define rRTCALM		(*(volatile unsigned char *)0x15700050)
#define rALMSEC		(*(volatile unsigned char *)0x15700054)
#define rALMMIN		(*(volatile unsigned char *)0x15700058)
#define rALMHOUR	(*(volatile unsigned char *)0x1570005C)
#define rALMDAY		(*(volatile unsigned char *)0x15700060)
#define rALMMON		(*(volatile unsigned char *)0x15700064)
#define rALMYEAR	(*(volatile unsigned char *)0x15700068)
#define rRTCRST		(*(volatile unsigned char *)0x1570006C)
#define rBCDSEC		(*(volatile unsigned char *)0x15700070)
#define rBCDMIN		(*(volatile unsigned char *)0x15700074)
#define rBCDHOUR	(*(volatile unsigned char *)0x15700078)
#define rBCDDAY		(*(volatile unsigned char *)0x1570007C)
#define rBCDDATE	(*(volatile unsigned char *)0x15700080)
#define rBCDMON		(*(volatile unsigned char *)0x15700084)
#define rBCDYEAR	(*(volatile unsigned char *)0x15700088)
#define rTICINT		(*(volatile unsigned char *)0x15700044)
#endif


/* ADC */
#define rADCCON		(*(volatile unsigned *)0x15800000)
#define rADCDAT		(*(volatile unsigned *)0x15800004)


/* SPI */
#define rSPCON		(*(volatile unsigned *)0x15900000)
#define rSPSTA		(*(volatile unsigned *)0x15900004)
#define rSPPIN		(*(volatile unsigned *)0x15900008)
#define rSPPRE		(*(volatile unsigned *)0x1590000C)
#define rSPTDAT		(*(volatile unsigned *)0x15900010)
#define rSPRDAT		(*(volatile unsigned *)0x15900014)


/* MMC INTERFACE */
#define rMMCON		(*(volatile unsigned *)0x15a00000)
#define rMMCRR		(*(volatile unsigned *)0x15a00004)
#define rMMFCON		(*(volatile unsigned *)0x15a00008)
#define rMMSTA		(*(volatile unsigned *)0x15a0000C)
#define rMMFSTA		(*(volatile unsigned *)0x15a00010)
#define rMMPRE		(*(volatile unsigned *)0x15a00014)
#define rMMLEN		(*(volatile unsigned *)0x15a00018)
#define rMMCR7		(*(volatile unsigned *)0x15a0001C)
#define rMMRSP0		(*(volatile unsigned *)0x15a00020)
#define rMMRSP1		(*(volatile unsigned *)0x15a00024)
#define rMMRSP2		(*(volatile unsigned *)0x15a00028)
#define rMMRSP3		(*(volatile unsigned *)0x15a0002C)
#define rMMCMD0		(*(volatile unsigned *)0x15a00030)
#define rMMCMD1		(*(volatile unsigned *)0x15a00034)
#define rMMCR16		(*(volatile unsigned *)0x15a00038)
#define rMMDAT		(*(volatile unsigned *)0x15a0003C)


/* ISR */
#define pISR_RESET	(*(unsigned *)(_ISR_STARTADDRESS+0x0))
#define pISR_UNDEF	(*(unsigned *)(_ISR_STARTADDRESS+0x4))
#define pISR_SWI	(*(unsigned *)(_ISR_STARTADDRESS+0x8))
#define pISR_PABORT	(*(unsigned *)(_ISR_STARTADDRESS+0xC))
#define pISR_DABORT	(*(unsigned *)(_ISR_STARTADDRESS+0x10))
#define pISR_RESERVED	(*(unsigned *)(_ISR_STARTADDRESS+0x14))
#define pISR_IRQ	(*(unsigned *)(_ISR_STARTADDRESS+0x18))
#define pISR_FIQ	(*(unsigned *)(_ISR_STARTADDRESS+0x1C))

#define pISR_EINT0	(*(unsigned *)(_ISR_STARTADDRESS+0x20))
#define pISR_EINT1	(*(unsigned *)(_ISR_STARTADDRESS+0x24))
#define pISR_EINT2	(*(unsigned *)(_ISR_STARTADDRESS+0x28))
#define pISR_EINT3	(*(unsigned *)(_ISR_STARTADDRESS+0x2C))
#define pISR_EINT4	(*(unsigned *)(_ISR_STARTADDRESS+0x30))
#define pISR_EINT5	(*(unsigned *)(_ISR_STARTADDRESS+0x34))
#define pISR_EINT6	(*(unsigned *)(_ISR_STARTADDRESS+0x38))
#define pISR_EINT7	(*(unsigned *)(_ISR_STARTADDRESS+0x3C))
#define pISR_TICK	(*(unsigned *)(_ISR_STARTADDRESS+0x40))
#define pISR_WDT	(*(unsigned *)(_ISR_STARTADDRESS+0x44))
#define pISR_TIMER0	(*(unsigned *)(_ISR_STARTADDRESS+0x48))
#define pISR_TIMER1	(*(unsigned *)(_ISR_STARTADDRESS+0x4C))
#define pISR_TIMER2	(*(unsigned *)(_ISR_STARTADDRESS+0x50))
#define pISR_TIMER3	(*(unsigned *)(_ISR_STARTADDRESS+0x54))
#define pISR_TIMER4	(*(unsigned *)(_ISR_STARTADDRESS+0x58))
#define pISR_UERR01	(*(unsigned *)(_ISR_STARTADDRESS+0x5C))
#define pISR_NOTUSED	(*(unsigned *)(_ISR_STARTADDRESS+0x60))
#define pISR_DMA0	(*(unsigned *)(_ISR_STARTADDRESS+0x64))
#define pISR_DMA1	(*(unsigned *)(_ISR_STARTADDRESS+0x68))
#define pISR_DMA2	(*(unsigned *)(_ISR_STARTADDRESS+0x6C))
#define pISR_DMA3	(*(unsigned *)(_ISR_STARTADDRESS+0x70))
#define pISR_MMC	(*(unsigned *)(_ISR_STARTADDRESS+0x74))
#define pISR_SPI	(*(unsigned *)(_ISR_STARTADDRESS+0x78))
#define pISR_URXD0	(*(unsigned *)(_ISR_STARTADDRESS+0x7C))
#define pISR_URXD1	(*(unsigned *)(_ISR_STARTADDRESS+0x80))
#define pISR_USBD	(*(unsigned *)(_ISR_STARTADDRESS+0x84))
#define pISR_USBH	(*(unsigned *)(_ISR_STARTADDRESS+0x88))
#define pISR_IIC	(*(unsigned *)(_ISR_STARTADDRESS+0x8C))
#define pISR_UTXD0	(*(unsigned *)(_ISR_STARTADDRESS+0x90))
#define pISR_UTXD1	(*(unsigned *)(_ISR_STARTADDRESS+0x94))
#define pISR_RTC	(*(unsigned *)(_ISR_STARTADDRESS+0x98))
#define pISR_ADC	(*(unsigned *)(_ISR_STARTADDRESS+0xA0))


/* PENDING BIT */
#define BIT_EINT0	(0x1)
#define BIT_EINT1	(0x1<<1)
#define BIT_EINT2	(0x1<<2)
#define BIT_EINT3	(0x1<<3)
#define BIT_EINT4	(0x1<<4)
#define BIT_EINT5	(0x1<<5)
#define BIT_EINT6	(0x1<<6)
#define BIT_EINT7	(0x1<<7)
#define BIT_TICK	(0x1<<8)
#define BIT_WDT		(0x1<<9)
#define BIT_TIMER0	(0x1<<10)
#define BIT_TIMER1	(0x1<<11)
#define BIT_TIMER2	(0x1<<12)
#define BIT_TIMER3	(0x1<<13)
#define BIT_TIMER4	(0x1<<14)
#define BIT_UERR01	(0x1<<15)
#define BIT_NOTUSED	(0x1<<16)
#define BIT_DMA0	(0x1<<17)
#define BIT_DMA1	(0x1<<18)
#define BIT_DMA2	(0x1<<19)
#define BIT_DMA3	(0x1<<20)
#define BIT_MMC		(0x1<<21)
#define BIT_SPI		(0x1<<22)
#define BIT_URXD0	(0x1<<23)
#define BIT_URXD1	(0x1<<24)
#define BIT_USBD	(0x1<<25)
#define BIT_USBH	(0x1<<26)
#define BIT_IIC		(0x1<<27)
#define BIT_UTXD0	(0x1<<28)
#define BIT_UTXD1	(0x1<<29)
#define BIT_RTC		(0x1<<30)
#define BIT_ADC		(0x1<<31)
#define BIT_ALLMSK	(0xFFFFFFFF)

#define ClearPending(bit) {\
		 rSRCPND = bit;\
		 rINTPND = bit;\
		 rINTPND;\
		 }
/* Wait until rINTPND is changed for the case that the ISR is very short. */
#endif
#endif /*__S3C2400_H__*/
