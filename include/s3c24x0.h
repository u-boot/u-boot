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
 * NAME	    : s3c24x0.h
 * Version  : 31.3.2003
 *
 * common stuff for SAMSUNG S3C24X0 SoC
 ************************************************/

#ifndef __S3C24X0_H__
#define __S3C24X0_H__

typedef volatile u8	S3C24X0_REG8;
typedef volatile u16	S3C24X0_REG16;
typedef volatile u32	S3C24X0_REG32;

/* Memory controller (see manual chapter 5) */
typedef struct {
	S3C24X0_REG32	BWSCON;
	S3C24X0_REG32	BANKCON[8];
	S3C24X0_REG32	REFRESH;
	S3C24X0_REG32	BANKSIZE;
	S3C24X0_REG32	MRSRB6;
	S3C24X0_REG32	MRSRB7;
} /*__attribute__((__packed__))*/ S3C24X0_MEMCTL;


/* USB HOST (see manual chapter 12) */
typedef struct {
	S3C24X0_REG32	HcRevision;
	S3C24X0_REG32	HcControl;
	S3C24X0_REG32	HcCommonStatus;
	S3C24X0_REG32	HcInterruptStatus;
	S3C24X0_REG32	HcInterruptEnable;
	S3C24X0_REG32	HcInterruptDisable;
	S3C24X0_REG32	HcHCCA;
	S3C24X0_REG32	HcPeriodCuttendED;
	S3C24X0_REG32	HcControlHeadED;
	S3C24X0_REG32	HcControlCurrentED;
	S3C24X0_REG32	HcBulkHeadED;
	S3C24X0_REG32	HcBuldCurrentED;
	S3C24X0_REG32	HcDoneHead;
	S3C24X0_REG32	HcRmInterval;
	S3C24X0_REG32	HcFmRemaining;
	S3C24X0_REG32	HcFmNumber;
	S3C24X0_REG32	HcPeriodicStart;
	S3C24X0_REG32	HcLSThreshold;
	S3C24X0_REG32	HcRhDescriptorA;
	S3C24X0_REG32	HcRhDescriptorB;
	S3C24X0_REG32	HcRhStatus;
	S3C24X0_REG32	HcRhPortStatus1;
	S3C24X0_REG32	HcRhPortStatus2;
} /*__attribute__((__packed__))*/ S3C24X0_USB_HOST;


/* INTERRUPT (see manual chapter 14) */
typedef struct {
	S3C24X0_REG32	SRCPND;
	S3C24X0_REG32	INTMOD;
	S3C24X0_REG32	INTMSK;
	S3C24X0_REG32	PRIORITY;
	S3C24X0_REG32	INTPND;
	S3C24X0_REG32	INTOFFSET;
#ifdef CONFIG_S3C2410
	S3C24X0_REG32	SUBSRCPND;
	S3C24X0_REG32	INTSUBMSK;
#endif
} /*__attribute__((__packed__))*/ S3C24X0_INTERRUPT;


/* DMAS (see manual chapter 8) */
typedef struct {
	S3C24X0_REG32	DISRC;
#ifdef CONFIG_S3C2410
	S3C24X0_REG32	DISRCC;
#endif
	S3C24X0_REG32	DIDST;
#ifdef CONFIG_S3C2410
	S3C24X0_REG32	DIDSTC;
#endif
	S3C24X0_REG32	DCON;
	S3C24X0_REG32	DSTAT;
	S3C24X0_REG32	DCSRC;
	S3C24X0_REG32	DCDST;
	S3C24X0_REG32	DMASKTRIG;
#ifdef CONFIG_S3C2400
	S3C24X0_REG32	res[1];
#endif
#ifdef CONFIG_S3C2410
	S3C24X0_REG32	res[7];
#endif
} /*__attribute__((__packed__))*/ S3C24X0_DMA;

typedef struct {
	S3C24X0_DMA	dma[4];
} /*__attribute__((__packed__))*/ S3C24X0_DMAS;


/* CLOCK & POWER MANAGEMENT (see S3C2400 manual chapter 6) */
/*                          (see S3C2410 manual chapter 7) */
typedef struct {
	S3C24X0_REG32	LOCKTIME;
	S3C24X0_REG32	MPLLCON;
	S3C24X0_REG32	UPLLCON;
	S3C24X0_REG32	CLKCON;
	S3C24X0_REG32	CLKSLOW;
	S3C24X0_REG32	CLKDIVN;
} /*__attribute__((__packed__))*/ S3C24X0_CLOCK_POWER;


/* LCD CONTROLLER (see manual chapter 15) */
typedef struct {
	S3C24X0_REG32	LCDCON1;
	S3C24X0_REG32	LCDCON2;
	S3C24X0_REG32	LCDCON3;
	S3C24X0_REG32	LCDCON4;
	S3C24X0_REG32	LCDCON5;
	S3C24X0_REG32	LCDSADDR1;
	S3C24X0_REG32	LCDSADDR2;
	S3C24X0_REG32	LCDSADDR3;
	S3C24X0_REG32	REDLUT;
	S3C24X0_REG32	GREENLUT;
	S3C24X0_REG32	BLUELUT;
	S3C24X0_REG32	res[8];
	S3C24X0_REG32	DITHMODE;
	S3C24X0_REG32	TPAL;
#ifdef CONFIG_S3C2410
	S3C24X0_REG32	LCDINTPND;
	S3C24X0_REG32	LCDSRCPND;
	S3C24X0_REG32	LCDINTMSK;
	S3C24X0_REG32	LPCSEL;
#endif
} /*__attribute__((__packed__))*/ S3C24X0_LCD;


/* NAND FLASH (see S3C2410 manual chapter 6) */
typedef struct {
	S3C24X0_REG32	NFCONF;
	S3C24X0_REG32	NFCMD;
	S3C24X0_REG32	NFADDR;
	S3C24X0_REG32	NFDATA;
	S3C24X0_REG32	NFSTAT;
	S3C24X0_REG32	NFECC;
} /*__attribute__((__packed__))*/ S3C2410_NAND;


/* UART (see manual chapter 11) */
typedef struct {
	S3C24X0_REG32	ULCON;
	S3C24X0_REG32	UCON;
	S3C24X0_REG32	UFCON;
	S3C24X0_REG32	UMCON;
	S3C24X0_REG32	UTRSTAT;
	S3C24X0_REG32	UERSTAT;
	S3C24X0_REG32	UFSTAT;
	S3C24X0_REG32	UMSTAT;
#ifdef __BIG_ENDIAN
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	UTXH;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	URXH;
#else /* Little Endian */
	S3C24X0_REG8	UTXH;
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	URXH;
	S3C24X0_REG8	res2[3];
#endif
	S3C24X0_REG32	UBRDIV;
} /*__attribute__((__packed__))*/ S3C24X0_UART;


/* PWM TIMER (see manual chapter 10) */
typedef struct {
	S3C24X0_REG32	TCNTB;
	S3C24X0_REG32	TCMPB;
	S3C24X0_REG32	TCNTO;
} /*__attribute__((__packed__))*/ S3C24X0_TIMER;

typedef struct {
	S3C24X0_REG32	TCFG0;
	S3C24X0_REG32	TCFG1;
	S3C24X0_REG32	TCON;
	S3C24X0_TIMER	ch[4];
	S3C24X0_REG32	TCNTB4;
	S3C24X0_REG32	TCNTO4;
} /*__attribute__((__packed__))*/ S3C24X0_TIMERS;


/* USB DEVICE (see manual chapter 13) */
typedef struct {
#ifdef __BIG_ENDIAN
	S3C24X0_REG8	res[3];
	S3C24X0_REG8	EP_FIFO_REG;
#else /*  little endian */
	S3C24X0_REG8	EP_FIFO_REG;
	S3C24X0_REG8	res[3];
#endif
} /*__attribute__((__packed__))*/ S3C24X0_USB_DEV_FIFOS;

typedef struct {
#ifdef __BIG_ENDIAN
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	EP_DMA_CON;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	EP_DMA_UNIT;
	S3C24X0_REG8	res3[3];
	S3C24X0_REG8	EP_DMA_FIFO;
	S3C24X0_REG8	res4[3];
	S3C24X0_REG8	EP_DMA_TTC_L;
	S3C24X0_REG8	res5[3];
	S3C24X0_REG8	EP_DMA_TTC_M;
	S3C24X0_REG8	res6[3];
	S3C24X0_REG8	EP_DMA_TTC_H;
#else /*  little endian */
	S3C24X0_REG8	EP_DMA_CON;
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	EP_DMA_UNIT;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	EP_DMA_FIFO;
	S3C24X0_REG8	res3[3];
	S3C24X0_REG8	EP_DMA_TTC_L;
	S3C24X0_REG8	res4[3];
	S3C24X0_REG8	EP_DMA_TTC_M;
	S3C24X0_REG8	res5[3];
	S3C24X0_REG8	EP_DMA_TTC_H;
	S3C24X0_REG8	res6[3];
#endif
} /*__attribute__((__packed__))*/ S3C24X0_USB_DEV_DMAS;

typedef struct {
#ifdef __BIG_ENDIAN
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	FUNC_ADDR_REG;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	PWR_REG;
	S3C24X0_REG8	res3[3];
	S3C24X0_REG8	EP_INT_REG;
	S3C24X0_REG8	res4[15];
	S3C24X0_REG8	USB_INT_REG;
	S3C24X0_REG8	res5[3];
	S3C24X0_REG8	EP_INT_EN_REG;
	S3C24X0_REG8	res6[15];
	S3C24X0_REG8	USB_INT_EN_REG;
	S3C24X0_REG8	res7[3];
	S3C24X0_REG8	FRAME_NUM1_REG;
	S3C24X0_REG8	res8[3];
	S3C24X0_REG8	FRAME_NUM2_REG;
	S3C24X0_REG8	res9[3];
	S3C24X0_REG8	INDEX_REG;
	S3C24X0_REG8	res10[7];
	S3C24X0_REG8	MAXP_REG;
	S3C24X0_REG8	res11[3];
	S3C24X0_REG8	EP0_CSR_IN_CSR1_REG;
	S3C24X0_REG8	res12[3];
	S3C24X0_REG8	IN_CSR2_REG;
	S3C24X0_REG8	res13[7];
	S3C24X0_REG8	OUT_CSR1_REG;
	S3C24X0_REG8	res14[3];
	S3C24X0_REG8	OUT_CSR2_REG;
	S3C24X0_REG8	res15[3];
	S3C24X0_REG8	OUT_FIFO_CNT1_REG;
	S3C24X0_REG8	res16[3];
	S3C24X0_REG8	OUT_FIFO_CNT2_REG;
#else /*  little endian */
	S3C24X0_REG8	FUNC_ADDR_REG;
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	PWR_REG;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	EP_INT_REG;
	S3C24X0_REG8	res3[15];
	S3C24X0_REG8	USB_INT_REG;
	S3C24X0_REG8	res4[3];
	S3C24X0_REG8	EP_INT_EN_REG;
	S3C24X0_REG8	res5[15];
	S3C24X0_REG8	USB_INT_EN_REG;
	S3C24X0_REG8	res6[3];
	S3C24X0_REG8	FRAME_NUM1_REG;
	S3C24X0_REG8	res7[3];
	S3C24X0_REG8	FRAME_NUM2_REG;
	S3C24X0_REG8	res8[3];
	S3C24X0_REG8	INDEX_REG;
	S3C24X0_REG8	res9[7];
	S3C24X0_REG8	MAXP_REG;
	S3C24X0_REG8	res10[7];
	S3C24X0_REG8	EP0_CSR_IN_CSR1_REG;
	S3C24X0_REG8	res11[3];
	S3C24X0_REG8	IN_CSR2_REG;
	S3C24X0_REG8	res12[3];
	S3C24X0_REG8	OUT_CSR1_REG;
	S3C24X0_REG8	res13[7];
	S3C24X0_REG8	OUT_CSR2_REG;
	S3C24X0_REG8	res14[3];
	S3C24X0_REG8	OUT_FIFO_CNT1_REG;
	S3C24X0_REG8	res15[3];
	S3C24X0_REG8	OUT_FIFO_CNT2_REG;
	S3C24X0_REG8	res16[3];
#endif /*  __BIG_ENDIAN */
	S3C24X0_USB_DEV_FIFOS	fifo[5];
	S3C24X0_USB_DEV_DMAS	dma[5];
} /*__attribute__((__packed__))*/ S3C24X0_USB_DEVICE;


/* WATCH DOG TIMER (see manual chapter 18) */
typedef struct {
	S3C24X0_REG32	WTCON;
	S3C24X0_REG32	WTDAT;
	S3C24X0_REG32	WTCNT;
} /*__attribute__((__packed__))*/ S3C24X0_WATCHDOG;


/* IIC (see manual chapter 20) */
typedef struct {
	S3C24X0_REG32	IICCON;
	S3C24X0_REG32	IICSTAT;
	S3C24X0_REG32	IICADD;
	S3C24X0_REG32	IICDS;
} /*__attribute__((__packed__))*/ S3C24X0_I2C;


/* IIS (see manual chapter 21) */
typedef struct {
#ifdef __BIG_ENDIAN
	S3C24X0_REG16	res1;
	S3C24X0_REG16	IISCON;
	S3C24X0_REG16	res2;
	S3C24X0_REG16	IISMOD;
	S3C24X0_REG16	res3;
	S3C24X0_REG16	IISPSR;
	S3C24X0_REG16	res4;
	S3C24X0_REG16	IISFCON;
	S3C24X0_REG16	res5;
	S3C24X0_REG16	IISFIFO;
#else /*  little endian */
	S3C24X0_REG16	IISCON;
	S3C24X0_REG16	res1;
	S3C24X0_REG16	IISMOD;
	S3C24X0_REG16	res2;
	S3C24X0_REG16	IISPSR;
	S3C24X0_REG16	res3;
	S3C24X0_REG16	IISFCON;
	S3C24X0_REG16	res4;
	S3C24X0_REG16	IISFIFO;
	S3C24X0_REG16	res5;
#endif
} /*__attribute__((__packed__))*/ S3C24X0_I2S;


/* I/O PORT (see manual chapter 9) */
typedef struct {
#ifdef CONFIG_S3C2400
	S3C24X0_REG32	PACON;
	S3C24X0_REG32	PADAT;

	S3C24X0_REG32	PBCON;
	S3C24X0_REG32	PBDAT;
	S3C24X0_REG32	PBUP;

	S3C24X0_REG32	PCCON;
	S3C24X0_REG32	PCDAT;
	S3C24X0_REG32	PCUP;

	S3C24X0_REG32	PDCON;
	S3C24X0_REG32	PDDAT;
	S3C24X0_REG32	PDUP;

	S3C24X0_REG32	PECON;
	S3C24X0_REG32	PEDAT;
	S3C24X0_REG32	PEUP;

	S3C24X0_REG32	PFCON;
	S3C24X0_REG32	PFDAT;
	S3C24X0_REG32	PFUP;

	S3C24X0_REG32	PGCON;
	S3C24X0_REG32	PGDAT;
	S3C24X0_REG32	PGUP;

	S3C24X0_REG32	OPENCR;

	S3C24X0_REG32	MISCCR;
	S3C24X0_REG32	EXTINT;
#endif
#ifdef CONFIG_S3C2410
	S3C24X0_REG32	GPACON;
	S3C24X0_REG32	GPADAT;
	S3C24X0_REG32	res1[2];
	S3C24X0_REG32	GPBCON;
	S3C24X0_REG32	GPBDAT;
	S3C24X0_REG32	GPBUP;
	S3C24X0_REG32	res2;
	S3C24X0_REG32	GPCCON;
	S3C24X0_REG32	GPCDAT;
	S3C24X0_REG32	GPCUP;
	S3C24X0_REG32	res3;
	S3C24X0_REG32	GPDCON;
	S3C24X0_REG32	GPDDAT;
	S3C24X0_REG32	GPDUP;
	S3C24X0_REG32	res4;
	S3C24X0_REG32	GPECON;
	S3C24X0_REG32	GPEDAT;
	S3C24X0_REG32	GPEUP;
	S3C24X0_REG32	res5;
	S3C24X0_REG32	GPFCON;
	S3C24X0_REG32	GPFDAT;
	S3C24X0_REG32	GPFUP;
	S3C24X0_REG32	res6;
	S3C24X0_REG32	GPGCON;
	S3C24X0_REG32	GPGDAT;
	S3C24X0_REG32	GPGUP;
	S3C24X0_REG32	res7;
	S3C24X0_REG32	GPHCON;
	S3C24X0_REG32	GPHDAT;
	S3C24X0_REG32	GPHUP;
	S3C24X0_REG32	res8;

	S3C24X0_REG32	MISCCR;
	S3C24X0_REG32	DCLKCON;
	S3C24X0_REG32	EXTINT0;
	S3C24X0_REG32	EXTINT1;
	S3C24X0_REG32	EXTINT2;
	S3C24X0_REG32	EINTFLT0;
	S3C24X0_REG32	EINTFLT1;
	S3C24X0_REG32	EINTFLT2;
	S3C24X0_REG32	EINTFLT3;
	S3C24X0_REG32	EINTMASK;
	S3C24X0_REG32	EINTPEND;
	S3C24X0_REG32	GSTATUS0;
	S3C24X0_REG32	GSTATUS1;
	S3C24X0_REG32	GSTATUS2;
	S3C24X0_REG32	GSTATUS3;
	S3C24X0_REG32	GSTATUS4;
#endif
} /*__attribute__((__packed__))*/ S3C24X0_GPIO;


/* RTC (see manual chapter 17) */
typedef struct {
#ifdef __BIG_ENDIAN
	S3C24X0_REG8	res1[67];
	S3C24X0_REG8	RTCCON;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	TICNT;
	S3C24X0_REG8	res3[11];
	S3C24X0_REG8	RTCALM;
	S3C24X0_REG8	res4[3];
	S3C24X0_REG8	ALMSEC;
	S3C24X0_REG8	res5[3];
	S3C24X0_REG8	ALMMIN;
	S3C24X0_REG8	res6[3];
	S3C24X0_REG8	ALMHOUR;
	S3C24X0_REG8	res7[3];
	S3C24X0_REG8	ALMDATE;
	S3C24X0_REG8	res8[3];
	S3C24X0_REG8	ALMMON;
	S3C24X0_REG8	res9[3];
	S3C24X0_REG8	ALMYEAR;
	S3C24X0_REG8	res10[3];
	S3C24X0_REG8	RTCRST;
	S3C24X0_REG8	res11[3];
	S3C24X0_REG8	BCDSEC;
	S3C24X0_REG8	res12[3];
	S3C24X0_REG8	BCDMIN;
	S3C24X0_REG8	res13[3];
	S3C24X0_REG8	BCDHOUR;
	S3C24X0_REG8	res14[3];
	S3C24X0_REG8	BCDDATE;
	S3C24X0_REG8	res15[3];
	S3C24X0_REG8	BCDDAY;
	S3C24X0_REG8	res16[3];
	S3C24X0_REG8	BCDMON;
	S3C24X0_REG8	res17[3];
	S3C24X0_REG8	BCDYEAR;
#else /*  little endian */
	S3C24X0_REG8	res0[64];
	S3C24X0_REG8	RTCCON;
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	TICNT;
	S3C24X0_REG8	res2[11];
	S3C24X0_REG8	RTCALM;
	S3C24X0_REG8	res3[3];
	S3C24X0_REG8	ALMSEC;
	S3C24X0_REG8	res4[3];
	S3C24X0_REG8	ALMMIN;
	S3C24X0_REG8	res5[3];
	S3C24X0_REG8	ALMHOUR;
	S3C24X0_REG8	res6[3];
	S3C24X0_REG8	ALMDATE;
	S3C24X0_REG8	res7[3];
	S3C24X0_REG8	ALMMON;
	S3C24X0_REG8	res8[3];
	S3C24X0_REG8	ALMYEAR;
	S3C24X0_REG8	res9[3];
	S3C24X0_REG8	RTCRST;
	S3C24X0_REG8	res10[3];
	S3C24X0_REG8	BCDSEC;
	S3C24X0_REG8	res11[3];
	S3C24X0_REG8	BCDMIN;
	S3C24X0_REG8	res12[3];
	S3C24X0_REG8	BCDHOUR;
	S3C24X0_REG8	res13[3];
	S3C24X0_REG8	BCDDATE;
	S3C24X0_REG8	res14[3];
	S3C24X0_REG8	BCDDAY;
	S3C24X0_REG8	res15[3];
	S3C24X0_REG8	BCDMON;
	S3C24X0_REG8	res16[3];
	S3C24X0_REG8	BCDYEAR;
	S3C24X0_REG8	res17[3];
#endif
} /*__attribute__((__packed__))*/ S3C24X0_RTC;


/* ADC (see manual chapter 16) */
typedef struct {
	S3C24X0_REG32	ADCCON;
	S3C24X0_REG32	ADCDAT;
} /*__attribute__((__packed__))*/ S3C2400_ADC;


/* ADC (see manual chapter 16) */
typedef struct {
	S3C24X0_REG32	ADCCON;
	S3C24X0_REG32	ADCTSC;
	S3C24X0_REG32	ADCDLY;
	S3C24X0_REG32	ADCDAT0;
	S3C24X0_REG32	ADCDAT1;
} /*__attribute__((__packed__))*/ S3C2410_ADC;


/* SPI (see manual chapter 22) */
typedef struct {
	S3C24X0_REG32	SPCON;
	S3C24X0_REG32	SPSTA;
	S3C24X0_REG32	SPPIN;
	S3C24X0_REG32	SPPRE;
	S3C24X0_REG32	SPTDAT;
	S3C24X0_REG32	SPRDAT;
	S3C24X0_REG32	res[2];
} __attribute__((__packed__)) S3C24X0_SPI_CHANNEL;

typedef struct {
	S3C24X0_SPI_CHANNEL	ch[S3C24X0_SPI_CHANNELS];
} /*__attribute__((__packed__))*/ S3C24X0_SPI;


/* MMC INTERFACE (see S3C2400 manual chapter 19) */
typedef struct {
#ifdef __BIG_ENDIAN
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	MMCON;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	MMCRR;
	S3C24X0_REG8	res3[3];
	S3C24X0_REG8	MMFCON;
	S3C24X0_REG8	res4[3];
	S3C24X0_REG8	MMSTA;
	S3C24X0_REG16	res5;
	S3C24X0_REG16	MMFSTA;
	S3C24X0_REG8	res6[3];
	S3C24X0_REG8	MMPRE;
	S3C24X0_REG16	res7;
	S3C24X0_REG16	MMLEN;
	S3C24X0_REG8	res8[3];
	S3C24X0_REG8	MMCR7;
	S3C24X0_REG32	MMRSP[4];
	S3C24X0_REG8	res9[3];
	S3C24X0_REG8	MMCMD0;
	S3C24X0_REG32	MMCMD1;
	S3C24X0_REG16	res10;
	S3C24X0_REG16	MMCR16;
	S3C24X0_REG8	res11[3];
	S3C24X0_REG8	MMDAT;
#else
	S3C24X0_REG8	MMCON;
	S3C24X0_REG8	res1[3];
	S3C24X0_REG8	MMCRR;
	S3C24X0_REG8	res2[3];
	S3C24X0_REG8	MMFCON;
	S3C24X0_REG8	res3[3];
	S3C24X0_REG8	MMSTA;
	S3C24X0_REG8	res4[3];
	S3C24X0_REG16	MMFSTA;
	S3C24X0_REG16	res5;
	S3C24X0_REG8	MMPRE;
	S3C24X0_REG8	res6[3];
	S3C24X0_REG16	MMLEN;
	S3C24X0_REG16	res7;
	S3C24X0_REG8	MMCR7;
	S3C24X0_REG8	res8[3];
	S3C24X0_REG32	MMRSP[4];
	S3C24X0_REG8	MMCMD0;
	S3C24X0_REG8	res9[3];
	S3C24X0_REG32	MMCMD1;
	S3C24X0_REG16	MMCR16;
	S3C24X0_REG16	res10;
	S3C24X0_REG8	MMDAT;
	S3C24X0_REG8	res11[3];
#endif
} /*__attribute__((__packed__))*/ S3C2400_MMC;


/* SD INTERFACE (see S3C2410 manual chapter 19) */
typedef struct {
	S3C24X0_REG32	SDICON;
	S3C24X0_REG32	SDIPRE;
	S3C24X0_REG32	SDICARG;
	S3C24X0_REG32	SDICCON;
	S3C24X0_REG32	SDICSTA;
	S3C24X0_REG32	SDIRSP0;
	S3C24X0_REG32	SDIRSP1;
	S3C24X0_REG32	SDIRSP2;
	S3C24X0_REG32	SDIRSP3;
	S3C24X0_REG32	SDIDTIMER;
	S3C24X0_REG32	SDIBSIZE;
	S3C24X0_REG32	SDIDCON;
	S3C24X0_REG32	SDIDCNT;
	S3C24X0_REG32	SDIDSTA;
	S3C24X0_REG32	SDIFSTA;
#ifdef __BIG_ENDIAN
	S3C24X0_REG8	res[3];
	S3C24X0_REG8	SDIDAT;
#else
	S3C24X0_REG8	SDIDAT;
	S3C24X0_REG8	res[3];
#endif
	S3C24X0_REG32	SDIIMSK;
} /*__attribute__((__packed__))*/ S3C2410_SDI;


#if 0
/* Memory control */
#define rBWSCON			(*(volatile unsigned *)0x48000000)
#define rBANKCON0		(*(volatile unsigned *)0x48000004)
#define rBANKCON1		(*(volatile unsigned *)0x48000008)
#define rBANKCON2		(*(volatile unsigned *)0x4800000C)
#define rBANKCON3		(*(volatile unsigned *)0x48000010)
#define rBANKCON4		(*(volatile unsigned *)0x48000014)
#define rBANKCON5		(*(volatile unsigned *)0x48000018)
#define rBANKCON6		(*(volatile unsigned *)0x4800001C)
#define rBANKCON7		(*(volatile unsigned *)0x48000020)
#define rREFRESH		(*(volatile unsigned *)0x48000024)
#define rBANKSIZE		(*(volatile unsigned *)0x48000028)
#define rMRSRB6			(*(volatile unsigned *)0x4800002C)
#define rMRSRB7			(*(volatile unsigned *)0x48000030)


/* USB HOST */
#define rHcRevision		(*(volatile unsigned *)0x49000000)
#define rHcControl		(*(volatile unsigned *)0x49000004)
#define rHcCommonStatus		(*(volatile unsigned *)0x49000008)
#define rHcInterruptStatus	(*(volatile unsigned *)0x4900000C)
#define rHcInterruptEnable	(*(volatile unsigned *)0x49000010)
#define rHcInterruptDisable	(*(volatile unsigned *)0x49000014)
#define rHcHCCA			(*(volatile unsigned *)0x49000018)
#define rHcPeriodCuttendED	(*(volatile unsigned *)0x4900001C)
#define rHcControlHeadED	(*(volatile unsigned *)0x49000020)
#define rHcControlCurrentED	(*(volatile unsigned *)0x49000024)
#define rHcBulkHeadED		(*(volatile unsigned *)0x49000028)
#define rHcBuldCurrentED	(*(volatile unsigned *)0x4900002C)
#define rHcDoneHead		(*(volatile unsigned *)0x49000030)
#define rHcRmInterval		(*(volatile unsigned *)0x49000034)
#define rHcFmRemaining		(*(volatile unsigned *)0x49000038)
#define rHcFmNumber		(*(volatile unsigned *)0x4900003C)
#define rHcPeriodicStart	(*(volatile unsigned *)0x49000040)
#define rHcLSThreshold		(*(volatile unsigned *)0x49000044)
#define rHcRhDescriptorA	(*(volatile unsigned *)0x49000048)
#define rHcRhDescriptorB	(*(volatile unsigned *)0x4900004C)
#define rHcRhStatus		(*(volatile unsigned *)0x49000050)
#define rHcRhPortStatus1	(*(volatile unsigned *)0x49000054)
#define rHcRhPortStatus2	(*(volatile unsigned *)0x49000058)


/* INTERRUPT */
#define rSRCPND			(*(volatile unsigned *)0x4A000000)
#define rINTMOD			(*(volatile unsigned *)0x4A000004)
#define rINTMSK			(*(volatile unsigned *)0x4A000008)
#define rPRIORITY		(*(volatile unsigned *)0x4A00000C)
#define rINTPND			(*(volatile unsigned *)0x4A000010)
#define rINTOFFSET		(*(volatile unsigned *)0x4A000014)
#define rSUBSRCPND		(*(volatile unsigned *)0x4A000018)
#define rINTSUBMSK		(*(volatile unsigned *)0x4A00001C)


/* DMA */
#define rDISRC0			(*(volatile unsigned *)0x4B000000)
#define rDISRCC0		(*(volatile unsigned *)0x4B000004)
#define rDIDST0			(*(volatile unsigned *)0x4B000008)
#define rDIDSTC0		(*(volatile unsigned *)0x4B00000C)
#define rDCON0			(*(volatile unsigned *)0x4B000010)
#define rDSTAT0			(*(volatile unsigned *)0x4B000014)
#define rDCSRC0			(*(volatile unsigned *)0x4B000018)
#define rDCDST0			(*(volatile unsigned *)0x4B00001C)
#define rDMASKTRIG0		(*(volatile unsigned *)0x4B000020)
#define rDISRC1			(*(volatile unsigned *)0x4B000040)
#define rDISRCC1		(*(volatile unsigned *)0x4B000044)
#define rDIDST1			(*(volatile unsigned *)0x4B000048)
#define rDIDSTC1		(*(volatile unsigned *)0x4B00004C)
#define rDCON1			(*(volatile unsigned *)0x4B000050)
#define rDSTAT1			(*(volatile unsigned *)0x4B000054)
#define rDCSRC1			(*(volatile unsigned *)0x4B000058)
#define rDCDST1			(*(volatile unsigned *)0x4B00005C)
#define rDMASKTRIG1		(*(volatile unsigned *)0x4B000060)
#define rDISRC2			(*(volatile unsigned *)0x4B000080)
#define rDISRCC2		(*(volatile unsigned *)0x4B000084)
#define rDIDST2			(*(volatile unsigned *)0x4B000088)
#define rDIDSTC2		(*(volatile unsigned *)0x4B00008C)
#define rDCON2			(*(volatile unsigned *)0x4B000090)
#define rDSTAT2			(*(volatile unsigned *)0x4B000094)
#define rDCSRC2			(*(volatile unsigned *)0x4B000098)
#define rDCDST2			(*(volatile unsigned *)0x4B00009C)
#define rDMASKTRIG2		(*(volatile unsigned *)0x4B0000A0)
#define rDISRC3			(*(volatile unsigned *)0x4B0000C0)
#define rDISRCC3		(*(volatile unsigned *)0x4B0000C4)
#define rDIDST3			(*(volatile unsigned *)0x4B0000C8)
#define rDIDSTC3		(*(volatile unsigned *)0x4B0000CC)
#define rDCON3			(*(volatile unsigned *)0x4B0000D0)
#define rDSTAT3			(*(volatile unsigned *)0x4B0000D4)
#define rDCSRC3			(*(volatile unsigned *)0x4B0000D8)
#define rDCDST3			(*(volatile unsigned *)0x4B0000DC)
#define rDMASKTRIG3		(*(volatile unsigned *)0x4B0000E0)


/* CLOCK & POWER MANAGEMENT */
#define rLOCKTIME		(*(volatile unsigned *)0x4C000000)
#define rMPLLCON		(*(volatile unsigned *)0x4C000004)
#define rUPLLCON		(*(volatile unsigned *)0x4C000008)
#define rCLKCON			(*(volatile unsigned *)0x4C00000C)
#define rCLKSLOW		(*(volatile unsigned *)0x4C000010)
#define rCLKDIVN		(*(volatile unsigned *)0x4C000014)


/* LCD CONTROLLER */
#define rLCDCON1		(*(volatile unsigned *)0x4D000000)
#define rLCDCON2		(*(volatile unsigned *)0x4D000004)
#define rLCDCON3		(*(volatile unsigned *)0x4D000008)
#define rLCDCON4		(*(volatile unsigned *)0x4D00000C)
#define rLCDCON5		(*(volatile unsigned *)0x4D000010)
#define rLCDSADDR1		(*(volatile unsigned *)0x4D000014)
#define rLCDSADDR2		(*(volatile unsigned *)0x4D000018)
#define rLCDSADDR3		(*(volatile unsigned *)0x4D00001C)
#define rREDLUT			(*(volatile unsigned *)0x4D000020)
#define rGREENLUT		(*(volatile unsigned *)0x4D000024)
#define rBLUELUT		(*(volatile unsigned *)0x4D000028)
#define rDITHMODE		(*(volatile unsigned *)0x4D00004C)
#define rTPAL			(*(volatile unsigned *)0x4D000050)
#define rLCDINTPND		(*(volatile unsigned *)0x4D000054)
#define rLCDSRCPND		(*(volatile unsigned *)0x4D000058)
#define rLCDINTMSK		(*(volatile unsigned *)0x4D00005C)


/* NAND FLASH */
#define rNFCONF			(*(volatile unsigned *)0x4E000000)
#define rNFCMD			(*(volatile unsigned *)0x4E000004)
#define rNFADDR			(*(volatile unsigned *)0x4E000008)
#define rNFDATA			(*(volatile unsigned *)0x4E00000C)
#define rNFSTAT			(*(volatile unsigned *)0x4E000010)
#define rNFECC			(*(volatile unsigned *)0x4E000014)


/* UART */
#define rULCON0			(*(volatile unsigned *)0x50000000)
#define rUCON0			(*(volatile unsigned *)0x50000004)
#define rUFCON0			(*(volatile unsigned *)0x50000008)
#define rUMCON0			(*(volatile unsigned *)0x5000000C)
#define rUTRSTAT0		(*(volatile unsigned *)0x50000010)
#define rUERSTAT0		(*(volatile unsigned *)0x50000014)
#define rUFSTAT0		(*(volatile unsigned *)0x50000018)
#define rUMSTAT0		(*(volatile unsigned *)0x5000001C)
#define rUBRDIV0		(*(volatile unsigned *)0x50000028)

#define rULCON1			(*(volatile unsigned *)0x50004000)
#define rUCON1			(*(volatile unsigned *)0x50004004)
#define rUFCON1			(*(volatile unsigned *)0x50004008)
#define rUMCON1			(*(volatile unsigned *)0x5000400C)
#define rUTRSTAT1		(*(volatile unsigned *)0x50004010)
#define rUERSTAT1		(*(volatile unsigned *)0x50004014)
#define rUFSTAT1		(*(volatile unsigned *)0x50004018)
#define rUMSTAT1		(*(volatile unsigned *)0x5000401C)
#define rUBRDIV1		(*(volatile unsigned *)0x50004028)

#define rULCON2			(*(volatile unsigned *)0x50008000)
#define rUCON2			(*(volatile unsigned *)0x50008004)
#define rUFCON2			(*(volatile unsigned *)0x50008008)
#define rUTRSTAT2		(*(volatile unsigned *)0x50008010)
#define rUERSTAT2		(*(volatile unsigned *)0x50008014)
#define rUFSTAT2		(*(volatile unsigned *)0x50008018)
#define rUBRDIV2		(*(volatile unsigned *)0x50008028)

#ifdef __BIG_ENDIAN
#define rUTXH0			(*(volatile unsigned char *)0x50000023)
#define rURXH0			(*(volatile unsigned char *)0x50000027)
#define rUTXH1			(*(volatile unsigned char *)0x50004023)
#define rURXH1			(*(volatile unsigned char *)0x50004027)
#define rUTXH2			(*(volatile unsigned char *)0x50008023)
#define rURXH2			(*(volatile unsigned char *)0x50008027)

#define WrUTXH0(ch)		(*(volatile unsigned char *)0x50000023)=(unsigned char)(ch)
#define RdURXH0()		(*(volatile unsigned char *)0x50000027)
#define WrUTXH1(ch)		(*(volatile unsigned char *)0x50004023)=(unsigned char)(ch)
#define RdURXH1()		(*(volatile unsigned char *)0x50004027)
#define WrUTXH2(ch)		(*(volatile unsigned char *)0x50008023)=(unsigned char)(ch)
#define RdURXH2()		(*(volatile unsigned char *)0x50008027)

#define UTXH0			(0x50000020+3)  /* byte_access address by DMA */
#define URXH0			(0x50000024+3)
#define UTXH1			(0x50004020+3)
#define URXH1			(0x50004024+3)
#define UTXH2			(0x50008020+3)
#define URXH2			(0x50008024+3)

#else /* Little Endian */
#define rUTXH0			(*(volatile unsigned char *)0x50000020)
#define rURXH0			(*(volatile unsigned char *)0x50000024)
#define rUTXH1			(*(volatile unsigned char *)0x50004020)
#define rURXH1			(*(volatile unsigned char *)0x50004024)
#define rUTXH2			(*(volatile unsigned char *)0x50008020)
#define rURXH2			(*(volatile unsigned char *)0x50008024)

#define WrUTXH0(ch)		(*(volatile unsigned char *)0x50000020)=(unsigned char)(ch)
#define RdURXH0()		(*(volatile unsigned char *)0x50000024)
#define WrUTXH1(ch)		(*(volatile unsigned char *)0x50004020)=(unsigned char)(ch)
#define RdURXH1()		(*(volatile unsigned char *)0x50004024)
#define WrUTXH2(ch)		(*(volatile unsigned char *)0x50008020)=(unsigned char)(ch)
#define RdURXH2()		(*(volatile unsigned char *)0x50008024)

#define UTXH0			(0x50000020)    /* byte_access address by DMA */
#define URXH0			(0x50000024)
#define UTXH1			(0x50004020)
#define URXH1			(0x50004024)
#define UTXH2			(0x50008020)
#define URXH2			(0x50008024)
#endif


/* PWM TIMER */
#define rTCFG0			(*(volatile unsigned *)0x51000000)
#define rTCFG1			(*(volatile unsigned *)0x51000004)
#define rTCON			(*(volatile unsigned *)0x51000008)
#define rTCNTB0			(*(volatile unsigned *)0x5100000C)
#define rTCMPB0			(*(volatile unsigned *)0x51000010)
#define rTCNTO0			(*(volatile unsigned *)0x51000014)
#define rTCNTB1			(*(volatile unsigned *)0x51000018)
#define rTCMPB1			(*(volatile unsigned *)0x5100001C)
#define rTCNTO1			(*(volatile unsigned *)0x51000020)
#define rTCNTB2			(*(volatile unsigned *)0x51000024)
#define rTCMPB2			(*(volatile unsigned *)0x51000028)
#define rTCNTO2			(*(volatile unsigned *)0x5100002C)
#define rTCNTB3			(*(volatile unsigned *)0x51000030)
#define rTCMPB3			(*(volatile unsigned *)0x51000034)
#define rTCNTO3			(*(volatile unsigned *)0x51000038)
#define rTCNTB4			(*(volatile unsigned *)0x5100003C)
#define rTCNTO4			(*(volatile unsigned *)0x51000040)


/* USB DEVICE */
#ifdef __BIG_ENDIAN
#define rFUNC_ADDR_REG		(*(volatile unsigned char *)0x52000143)
#define rPWR_REG		(*(volatile unsigned char *)0x52000147)
#define rEP_INT_REG		(*(volatile unsigned char *)0x5200014B)
#define rUSB_INT_REG		(*(volatile unsigned char *)0x5200015B)
#define rEP_INT_EN_REG		(*(volatile unsigned char *)0x5200015F)
#define rUSB_INT_EN_REG		(*(volatile unsigned char *)0x5200016F)
#define rFRAME_NUM1_REG		(*(volatile unsigned char *)0x52000173)
#define rFRAME_NUM2_REG		(*(volatile unsigned char *)0x52000177)
#define rINDEX_REG		(*(volatile unsigned char *)0x5200017B)
#define rMAXP_REG		(*(volatile unsigned char *)0x52000183)
#define rEP0_CSR		(*(volatile unsigned char *)0x52000187)
#define rIN_CSR1_REG		(*(volatile unsigned char *)0x52000187)
#define rIN_CSR2_REG		(*(volatile unsigned char *)0x5200018B)
#define rOUT_CSR1_REG		(*(volatile unsigned char *)0x52000193)
#define rOUT_CSR2_REG		(*(volatile unsigned char *)0x52000197)
#define rOUT_FIFO_CNT1_REG	(*(volatile unsigned char *)0x5200019B)
#define rOUT_FIFO_CNT2_REG	(*(volatile unsigned char *)0x5200019F)
#define rEP0_FIFO		(*(volatile unsigned char *)0x520001C3)
#define rEP1_FIFO		(*(volatile unsigned char *)0x520001C7)
#define rEP2_FIFO		(*(volatile unsigned char *)0x520001CB)
#define rEP3_FIFO		(*(volatile unsigned char *)0x520001CF)
#define rEP4_FIFO		(*(volatile unsigned char *)0x520001D3)
#define rEP1_DMA_CON		(*(volatile unsigned char *)0x52000203)
#define rEP1_DMA_UNIT		(*(volatile unsigned char *)0x52000207)
#define rEP1_DMA_FIFO		(*(volatile unsigned char *)0x5200020B)
#define rEP1_DMA_TX_LO		(*(volatile unsigned char *)0x5200020F)
#define rEP1_DMA_TX_MD		(*(volatile unsigned char *)0x52000213)
#define rEP1_DMA_TX_HI		(*(volatile unsigned char *)0x52000217)
#define rEP2_DMA_CON		(*(volatile unsigned char *)0x5200021B)
#define rEP2_DMA_UNIT		(*(volatile unsigned char *)0x5200021F)
#define rEP2_DMA_FIFO		(*(volatile unsigned char *)0x52000223)
#define rEP2_DMA_TX_LO		(*(volatile unsigned char *)0x52000227)
#define rEP2_DMA_TX_MD		(*(volatile unsigned char *)0x5200022B)
#define rEP2_DMA_TX_HI		(*(volatile unsigned char *)0x5200022F)
#define rEP3_DMA_CON		(*(volatile unsigned char *)0x52000243)
#define rEP3_DMA_UNIT		(*(volatile unsigned char *)0x52000247)
#define rEP3_DMA_FIFO		(*(volatile unsigned char *)0x5200024B)
#define rEP3_DMA_TX_LO		(*(volatile unsigned char *)0x5200024F)
#define rEP3_DMA_TX_MD		(*(volatile unsigned char *)0x52000253)
#define rEP3_DMA_TX_HI		(*(volatile unsigned char *)0x52000257)
#define rEP4_DMA_CON		(*(volatile unsigned char *)0x5200025B)
#define rEP4_DMA_UNIT		(*(volatile unsigned char *)0x5200025F)
#define rEP4_DMA_FIFO		(*(volatile unsigned char *)0x52000263)
#define rEP4_DMA_TX_LO		(*(volatile unsigned char *)0x52000267)
#define rEP4_DMA_TX_MD		(*(volatile unsigned char *)0x5200026B)
#define rEP4_DMA_TX_HI		(*(volatile unsigned char *)0x5200026F)
#else /*  little endian */
#define rFUNC_ADDR_REG		(*(volatile unsigned char *)0x52000140)
#define rPWR_REG		(*(volatile unsigned char *)0x52000144)
#define rEP_INT_REG		(*(volatile unsigned char *)0x52000148)
#define rUSB_INT_REG		(*(volatile unsigned char *)0x52000158)
#define rEP_INT_EN_REG		(*(volatile unsigned char *)0x5200015C)
#define rUSB_INT_EN_REG		(*(volatile unsigned char *)0x5200016C)
#define rFRAME_NUM1_REG		(*(volatile unsigned char *)0x52000170)
#define rFRAME_NUM2_REG		(*(volatile unsigned char *)0x52000174)
#define rINDEX_REG		(*(volatile unsigned char *)0x52000178)
#define rMAXP_REG		(*(volatile unsigned char *)0x52000180)
#define rEP0_CSR		(*(volatile unsigned char *)0x52000184)
#define rIN_CSR1_REG		(*(volatile unsigned char *)0x52000184)
#define rIN_CSR2_REG		(*(volatile unsigned char *)0x52000188)
#define rOUT_CSR1_REG		(*(volatile unsigned char *)0x52000190)
#define rOUT_CSR2_REG		(*(volatile unsigned char *)0x52000194)
#define rOUT_FIFO_CNT1_REG	(*(volatile unsigned char *)0x52000198)
#define rOUT_FIFO_CNT2_REG	(*(volatile unsigned char *)0x5200019C)
#define rEP0_FIFO		(*(volatile unsigned char *)0x520001C0)
#define rEP1_FIFO		(*(volatile unsigned char *)0x520001C4)
#define rEP2_FIFO		(*(volatile unsigned char *)0x520001C8)
#define rEP3_FIFO		(*(volatile unsigned char *)0x520001CC)
#define rEP4_FIFO		(*(volatile unsigned char *)0x520001D0)
#define rEP1_DMA_CON		(*(volatile unsigned char *)0x52000200)
#define rEP1_DMA_UNIT		(*(volatile unsigned char *)0x52000204)
#define rEP1_DMA_FIFO		(*(volatile unsigned char *)0x52000208)
#define rEP1_DMA_TX_LO		(*(volatile unsigned char *)0x5200020C)
#define rEP1_DMA_TX_MD		(*(volatile unsigned char *)0x52000210)
#define rEP1_DMA_TX_HI		(*(volatile unsigned char *)0x52000214)
#define rEP2_DMA_CON		(*(volatile unsigned char *)0x52000218)
#define rEP2_DMA_UNIT		(*(volatile unsigned char *)0x5200021C)
#define rEP2_DMA_FIFO		(*(volatile unsigned char *)0x52000220)
#define rEP2_DMA_TX_LO		(*(volatile unsigned char *)0x52000224)
#define rEP2_DMA_TX_MD		(*(volatile unsigned char *)0x52000228)
#define rEP2_DMA_TX_HI		(*(volatile unsigned char *)0x5200022C)
#define rEP3_DMA_CON		(*(volatile unsigned char *)0x52000240)
#define rEP3_DMA_UNIT		(*(volatile unsigned char *)0x52000244)
#define rEP3_DMA_FIFO		(*(volatile unsigned char *)0x52000248)
#define rEP3_DMA_TX_LO		(*(volatile unsigned char *)0x5200024C)
#define rEP3_DMA_TX_MD		(*(volatile unsigned char *)0x52000250)
#define rEP3_DMA_TX_HI		(*(volatile unsigned char *)0x52000254)
#define rEP4_DMA_CON		(*(volatile unsigned char *)0x52000258)
#define rEP4_DMA_UNIT		(*(volatile unsigned char *)0x5200025C)
#define rEP4_DMA_FIFO		(*(volatile unsigned char *)0x52000260)
#define rEP4_DMA_TX_LO		(*(volatile unsigned char *)0x52000264)
#define rEP4_DMA_TX_MD		(*(volatile unsigned char *)0x52000268)
#define rEP4_DMA_TX_HI		(*(volatile unsigned char *)0x5200026C)
#endif /*  __BIG_ENDIAN */


/* WATCH DOG TIMER */
#define rWTCON			(*(volatile unsigned *)0x53000000)
#define rWTDAT			(*(volatile unsigned *)0x53000004)
#define rWTCNT			(*(volatile unsigned *)0x53000008)


/* IIC */
#define rIICCON			(*(volatile unsigned *)0x54000000)
#define rIICSTAT		(*(volatile unsigned *)0x54000004)
#define rIICADD			(*(volatile unsigned *)0x54000008)
#define rIICDS			(*(volatile unsigned *)0x5400000C)


/* IIS */
#define rIISCON			(*(volatile unsigned *)0x55000000)
#define rIISMOD			(*(volatile unsigned *)0x55000004)
#define rIISPSR			(*(volatile unsigned *)0x55000008)
#define rIISFCON		(*(volatile unsigned *)0x5500000C)

#ifdef __BIG_ENDIAN
#define IISFIF			((volatile unsigned short *)0x55000012)
#else /*  little endian */
#define IISFIF			((volatile unsigned short *)0x55000010)
#endif


/* I/O PORT */
#define rGPACON			(*(volatile unsigned *)0x56000000)
#define rGPADAT			(*(volatile unsigned *)0x56000004)

#define rGPBCON			(*(volatile unsigned *)0x56000010)
#define rGPBDAT			(*(volatile unsigned *)0x56000014)
#define rGPBUP			(*(volatile unsigned *)0x56000018)

#define rGPCCON			(*(volatile unsigned *)0x56000020)
#define rGPCDAT			(*(volatile unsigned *)0x56000024)
#define rGPCUP			(*(volatile unsigned *)0x56000028)

#define rGPDCON			(*(volatile unsigned *)0x56000030)
#define rGPDDAT			(*(volatile unsigned *)0x56000034)
#define rGPDUP			(*(volatile unsigned *)0x56000038)

#define rGPECON			(*(volatile unsigned *)0x56000040)
#define rGPEDAT			(*(volatile unsigned *)0x56000044)
#define rGPEUP			(*(volatile unsigned *)0x56000048)

#define rGPFCON			(*(volatile unsigned *)0x56000050)
#define rGPFDAT			(*(volatile unsigned *)0x56000054)
#define rGPFUP			(*(volatile unsigned *)0x56000058)

#define rGPGCON			(*(volatile unsigned *)0x56000060)
#define rGPGDAT			(*(volatile unsigned *)0x56000064)
#define rGPGUP			(*(volatile unsigned *)0x56000068)

#define rGPHCON			(*(volatile unsigned *)0x56000070)
#define rGPHDAT			(*(volatile unsigned *)0x56000074)
#define rGPHUP			(*(volatile unsigned *)0x56000078)

#define rMISCCR			(*(volatile unsigned *)0x56000080)
#define rDCLKCON		(*(volatile unsigned *)0x56000084)
#define rEXTINT0		(*(volatile unsigned *)0x56000088)
#define rEXTINT1		(*(volatile unsigned *)0x5600008C)
#define rEXTINT2		(*(volatile unsigned *)0x56000090)
#define rEINTFLT0		(*(volatile unsigned *)0x56000094)
#define rEINTFLT1		(*(volatile unsigned *)0x56000098)
#define rEINTFLT2		(*(volatile unsigned *)0x5600009C)
#define rEINTFLT3		(*(volatile unsigned *)0x560000A0)
#define rEINTMASK		(*(volatile unsigned *)0x560000A4)
#define rEINTPEND		(*(volatile unsigned *)0x560000A8)
#define rGSTATUS0		(*(volatile unsigned *)0x560000AC)
#define rGSTATUS1		(*(volatile unsigned *)0x560000B0)


/* RTC */
#ifdef __BIG_ENDIAN
#define rRTCCON			(*(volatile unsigned char *)0x57000043)
#define rTICNT			(*(volatile unsigned char *)0x57000047)
#define rRTCALM			(*(volatile unsigned char *)0x57000053)
#define rALMSEC			(*(volatile unsigned char *)0x57000057)
#define rALMMIN			(*(volatile unsigned char *)0x5700005B)
#define rALMHOUR		(*(volatile unsigned char *)0x5700005F)
#define rALMDATE		(*(volatile unsigned char *)0x57000063)
#define rALMMON			(*(volatile unsigned char *)0x57000067)
#define rALMYEAR		(*(volatile unsigned char *)0x5700006B)
#define rRTCRST			(*(volatile unsigned char *)0x5700006F)
#define rBCDSEC			(*(volatile unsigned char *)0x57000073)
#define rBCDMIN			(*(volatile unsigned char *)0x57000077)
#define rBCDHOUR		(*(volatile unsigned char *)0x5700007B)
#define rBCDDATE		(*(volatile unsigned char *)0x5700007F)
#define rBCDDAY			(*(volatile unsigned char *)0x57000083)
#define rBCDMON			(*(volatile unsigned char *)0x57000087)
#define rBCDYEAR		(*(volatile unsigned char *)0x5700008B)
#else /*  little endian */
#define rRTCCON			(*(volatile unsigned char *)0x57000040)
#define rTICNT			(*(volatile unsigned char *)0x57000044)
#define rRTCALM			(*(volatile unsigned char *)0x57000050)
#define rALMSEC			(*(volatile unsigned char *)0x57000054)
#define rALMMIN			(*(volatile unsigned char *)0x57000058)
#define rALMHOUR		(*(volatile unsigned char *)0x5700005C)
#define rALMDATE		(*(volatile unsigned char *)0x57000060)
#define rALMMON			(*(volatile unsigned char *)0x57000064)
#define rALMYEAR		(*(volatile unsigned char *)0x57000068)
#define rRTCRST			(*(volatile unsigned char *)0x5700006C)
#define rBCDSEC			(*(volatile unsigned char *)0x57000070)
#define rBCDMIN			(*(volatile unsigned char *)0x57000074)
#define rBCDHOUR		(*(volatile unsigned char *)0x57000078)
#define rBCDDATE		(*(volatile unsigned char *)0x5700007C)
#define rBCDDAY			(*(volatile unsigned char *)0x57000080)
#define rBCDMON			(*(volatile unsigned char *)0x57000084)
#define rBCDYEAR		(*(volatile unsigned char *)0x57000088)
#endif


/* ADC */
#define rADCCON			(*(volatile unsigned *)0x58000000)
#define rADCTSC			(*(volatile unsigned *)0x58000004)
#define rADCDLY			(*(volatile unsigned *)0x58000008)
#define rADCDAT0		(*(volatile unsigned *)0x5800000C)
#define rADCDAT1		(*(volatile unsigned *)0x58000010)


/* SPI */
#define rSPCON0			(*(volatile unsigned *)0x59000000)
#define rSPSTA0			(*(volatile unsigned *)0x59000004)
#define rSPPIN0			(*(volatile unsigned *)0x59000008)
#define rSPPRE0			(*(volatile unsigned *)0x5900000C)
#define rSPTDAT0		(*(volatile unsigned *)0x59000010)
#define rSPRDAT0		(*(volatile unsigned *)0x59000014)
#define rSPCON1			(*(volatile unsigned *)0x59000020)
#define rSPSTA1			(*(volatile unsigned *)0x59000024)
#define rSPPIN1			(*(volatile unsigned *)0x59000028)
#define rSPPRE1			(*(volatile unsigned *)0x5900002C)
#define rSPTDAT1		(*(volatile unsigned *)0x59000030)
#define rSPRDAT1		(*(volatile unsigned *)0x59000034)


/* SD INTERFACE */
#define rSDICON			(*(volatile unsigned *)0x5A000000)
#define rSDIPRE			(*(volatile unsigned *)0x5A000004)
#define rSDICmdArg		(*(volatile unsigned *)0x5A000008)
#define rSDICmdCon		(*(volatile unsigned *)0x5A00000C)
#define rSDICmdSta		(*(volatile unsigned *)0x5A000010)
#define rSDIRSP0		(*(volatile unsigned *)0x5A000014)
#define rSDIRSP1		(*(volatile unsigned *)0x5A000018)
#define rSDIRSP2		(*(volatile unsigned *)0x5A00001C)
#define rSDIRSP3		(*(volatile unsigned *)0x5A000020)
#define rSDIDTimer		(*(volatile unsigned *)0x5A000024)
#define rSDIBSize		(*(volatile unsigned *)0x5A000028)
#define rSDIDatCon		(*(volatile unsigned *)0x5A00002C)
#define rSDIDatCnt		(*(volatile unsigned *)0x5A000030)
#define rSDIDatSta		(*(volatile unsigned *)0x5A000034)
#define rSDIFSTA		(*(volatile unsigned *)0x5A000038)
#ifdef __BIG_ENDIAN
#define rSDIDAT			(*(volatile unsigned char *)0x5A00003F)
#else
#define rSDIDAT			(*(volatile unsigned char *)0x5A00003C)
#endif
#define rSDIIntMsk		(*(volatile unsigned *)0x5A000040)

#endif

#endif /*__S3C24X0_H__*/
