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

/* Memory controller (see manual chapter 5) */
struct s3c24x0_memctl {
	u32	BWSCON;
	u32	BANKCON[8];
	u32	REFRESH;
	u32	BANKSIZE;
	u32	MRSRB6;
	u32	MRSRB7;
};


/* USB HOST (see manual chapter 12) */
struct s3c24x0_usb_host {
	u32	HcRevision;
	u32	HcControl;
	u32	HcCommonStatus;
	u32	HcInterruptStatus;
	u32	HcInterruptEnable;
	u32	HcInterruptDisable;
	u32	HcHCCA;
	u32	HcPeriodCuttendED;
	u32	HcControlHeadED;
	u32	HcControlCurrentED;
	u32	HcBulkHeadED;
	u32	HcBuldCurrentED;
	u32	HcDoneHead;
	u32	HcRmInterval;
	u32	HcFmRemaining;
	u32	HcFmNumber;
	u32	HcPeriodicStart;
	u32	HcLSThreshold;
	u32	HcRhDescriptorA;
	u32	HcRhDescriptorB;
	u32	HcRhStatus;
	u32	HcRhPortStatus1;
	u32	HcRhPortStatus2;
};


/* INTERRUPT (see manual chapter 14) */
struct s3c24x0_interrupt {
	u32	SRCPND;
	u32	INTMOD;
	u32	INTMSK;
	u32	PRIORITY;
	u32	INTPND;
	u32	INTOFFSET;
#ifdef CONFIG_S3C2410
	u32	SUBSRCPND;
	u32	INTSUBMSK;
#endif
};


/* DMAS (see manual chapter 8) */
struct s3c24x0_dma {
	u32	DISRC;
#ifdef CONFIG_S3C2410
	u32	DISRCC;
#endif
	u32	DIDST;
#ifdef CONFIG_S3C2410
	u32	DIDSTC;
#endif
	u32	DCON;
	u32	DSTAT;
	u32	DCSRC;
	u32	DCDST;
	u32	DMASKTRIG;
#ifdef CONFIG_S3C2400
	u32	res[1];
#endif
#ifdef CONFIG_S3C2410
	u32	res[7];
#endif
};

struct s3c24x0_dmas {
	struct s3c24x0_dma	dma[4];
};


/* CLOCK & POWER MANAGEMENT (see S3C2400 manual chapter 6) */
/*                          (see S3C2410 manual chapter 7) */
struct s3c24x0_clock_power {
	u32	LOCKTIME;
	u32	MPLLCON;
	u32	UPLLCON;
	u32	CLKCON;
	u32	CLKSLOW;
	u32	CLKDIVN;
};


/* LCD CONTROLLER (see manual chapter 15) */
struct s3c24x0_lcd {
	u32	LCDCON1;
	u32	LCDCON2;
	u32	LCDCON3;
	u32	LCDCON4;
	u32	LCDCON5;
	u32	LCDSADDR1;
	u32	LCDSADDR2;
	u32	LCDSADDR3;
	u32	REDLUT;
	u32	GREENLUT;
	u32	BLUELUT;
	u32	res[8];
	u32	DITHMODE;
	u32	TPAL;
#ifdef CONFIG_S3C2410
	u32	LCDINTPND;
	u32	LCDSRCPND;
	u32	LCDINTMSK;
	u32	LPCSEL;
#endif
};


/* NAND FLASH (see S3C2410 manual chapter 6) */
struct s3c2410_nand {
	u32	NFCONF;
	u32	NFCMD;
	u32	NFADDR;
	u32	NFDATA;
	u32	NFSTAT;
	u32	NFECC;
};


/* UART (see manual chapter 11) */
struct s3c24x0_uart {
	u32	ULCON;
	u32	UCON;
	u32	UFCON;
	u32	UMCON;
	u32	UTRSTAT;
	u32	UERSTAT;
	u32	UFSTAT;
	u32	UMSTAT;
#ifdef __BIG_ENDIAN
	u8	res1[3];
	u8	UTXH;
	u8	res2[3];
	u8	URXH;
#else /* Little Endian */
	u8	UTXH;
	u8	res1[3];
	u8	URXH;
	u8	res2[3];
#endif
	u32	UBRDIV;
};


/* PWM TIMER (see manual chapter 10) */
struct s3c24x0_timer {
	u32	TCNTB;
	u32	TCMPB;
	u32	TCNTO;
};

struct s3c24x0_timers {
	u32	TCFG0;
	u32	TCFG1;
	u32	TCON;
	struct s3c24x0_timer	ch[4];
	u32	TCNTB4;
	u32	TCNTO4;
};


/* USB DEVICE (see manual chapter 13) */
struct s3c24x0_usb_dev_fifos {
#ifdef __BIG_ENDIAN
	u8	res[3];
	u8	EP_FIFO_REG;
#else /*  little endian */
	u8	EP_FIFO_REG;
	u8	res[3];
#endif
};

struct s3c24x0_usb_dev_dmas {
#ifdef __BIG_ENDIAN
	u8	res1[3];
	u8	EP_DMA_CON;
	u8	res2[3];
	u8	EP_DMA_UNIT;
	u8	res3[3];
	u8	EP_DMA_FIFO;
	u8	res4[3];
	u8	EP_DMA_TTC_L;
	u8	res5[3];
	u8	EP_DMA_TTC_M;
	u8	res6[3];
	u8	EP_DMA_TTC_H;
#else /*  little endian */
	u8	EP_DMA_CON;
	u8	res1[3];
	u8	EP_DMA_UNIT;
	u8	res2[3];
	u8	EP_DMA_FIFO;
	u8	res3[3];
	u8	EP_DMA_TTC_L;
	u8	res4[3];
	u8	EP_DMA_TTC_M;
	u8	res5[3];
	u8	EP_DMA_TTC_H;
	u8	res6[3];
#endif
};

struct s3c24x0_usb_device {
#ifdef __BIG_ENDIAN
	u8	res1[3];
	u8	FUNC_ADDR_REG;
	u8	res2[3];
	u8	PWR_REG;
	u8	res3[3];
	u8	EP_INT_REG;
	u8	res4[15];
	u8	USB_INT_REG;
	u8	res5[3];
	u8	EP_INT_EN_REG;
	u8	res6[15];
	u8	USB_INT_EN_REG;
	u8	res7[3];
	u8	FRAME_NUM1_REG;
	u8	res8[3];
	u8	FRAME_NUM2_REG;
	u8	res9[3];
	u8	INDEX_REG;
	u8	res10[7];
	u8	MAXP_REG;
	u8	res11[3];
	u8	EP0_CSR_IN_CSR1_REG;
	u8	res12[3];
	u8	IN_CSR2_REG;
	u8	res13[7];
	u8	OUT_CSR1_REG;
	u8	res14[3];
	u8	OUT_CSR2_REG;
	u8	res15[3];
	u8	OUT_FIFO_CNT1_REG;
	u8	res16[3];
	u8	OUT_FIFO_CNT2_REG;
#else /*  little endian */
	u8	FUNC_ADDR_REG;
	u8	res1[3];
	u8	PWR_REG;
	u8	res2[3];
	u8	EP_INT_REG;
	u8	res3[15];
	u8	USB_INT_REG;
	u8	res4[3];
	u8	EP_INT_EN_REG;
	u8	res5[15];
	u8	USB_INT_EN_REG;
	u8	res6[3];
	u8	FRAME_NUM1_REG;
	u8	res7[3];
	u8	FRAME_NUM2_REG;
	u8	res8[3];
	u8	INDEX_REG;
	u8	res9[7];
	u8	MAXP_REG;
	u8	res10[7];
	u8	EP0_CSR_IN_CSR1_REG;
	u8	res11[3];
	u8	IN_CSR2_REG;
	u8	res12[3];
	u8	OUT_CSR1_REG;
	u8	res13[7];
	u8	OUT_CSR2_REG;
	u8	res14[3];
	u8	OUT_FIFO_CNT1_REG;
	u8	res15[3];
	u8	OUT_FIFO_CNT2_REG;
	u8	res16[3];
#endif /*  __BIG_ENDIAN */
	struct s3c24x0_usb_dev_fifos	fifo[5];
	struct s3c24x0_usb_dev_dmas	dma[5];
};


/* WATCH DOG TIMER (see manual chapter 18) */
struct s3c24x0_watchdog {
	u32	WTCON;
	u32	WTDAT;
	u32	WTCNT;
};


/* IIC (see manual chapter 20) */
struct s3c24x0_i2c {
	u32	IICCON;
	u32	IICSTAT;
	u32	IICADD;
	u32	IICDS;
};


/* IIS (see manual chapter 21) */
struct s3c24x0_i2s {
#ifdef __BIG_ENDIAN
	u16	res1;
	u16	IISCON;
	u16	res2;
	u16	IISMOD;
	u16	res3;
	u16	IISPSR;
	u16	res4;
	u16	IISFCON;
	u16	res5;
	u16	IISFIFO;
#else /*  little endian */
	u16	IISCON;
	u16	res1;
	u16	IISMOD;
	u16	res2;
	u16	IISPSR;
	u16	res3;
	u16	IISFCON;
	u16	res4;
	u16	IISFIFO;
	u16	res5;
#endif
};


/* I/O PORT (see manual chapter 9) */
struct s3c24x0_gpio {
#ifdef CONFIG_S3C2400
	u32	PACON;
	u32	PADAT;

	u32	PBCON;
	u32	PBDAT;
	u32	PBUP;

	u32	PCCON;
	u32	PCDAT;
	u32	PCUP;

	u32	PDCON;
	u32	PDDAT;
	u32	PDUP;

	u32	PECON;
	u32	PEDAT;
	u32	PEUP;

	u32	PFCON;
	u32	PFDAT;
	u32	PFUP;

	u32	PGCON;
	u32	PGDAT;
	u32	PGUP;

	u32	OPENCR;

	u32	MISCCR;
	u32	EXTINT;
#endif
#ifdef CONFIG_S3C2410
	u32	GPACON;
	u32	GPADAT;
	u32	res1[2];
	u32	GPBCON;
	u32	GPBDAT;
	u32	GPBUP;
	u32	res2;
	u32	GPCCON;
	u32	GPCDAT;
	u32	GPCUP;
	u32	res3;
	u32	GPDCON;
	u32	GPDDAT;
	u32	GPDUP;
	u32	res4;
	u32	GPECON;
	u32	GPEDAT;
	u32	GPEUP;
	u32	res5;
	u32	GPFCON;
	u32	GPFDAT;
	u32	GPFUP;
	u32	res6;
	u32	GPGCON;
	u32	GPGDAT;
	u32	GPGUP;
	u32	res7;
	u32	GPHCON;
	u32	GPHDAT;
	u32	GPHUP;
	u32	res8;

	u32	MISCCR;
	u32	DCLKCON;
	u32	EXTINT0;
	u32	EXTINT1;
	u32	EXTINT2;
	u32	EINTFLT0;
	u32	EINTFLT1;
	u32	EINTFLT2;
	u32	EINTFLT3;
	u32	EINTMASK;
	u32	EINTPEND;
	u32	GSTATUS0;
	u32	GSTATUS1;
	u32	GSTATUS2;
	u32	GSTATUS3;
	u32	GSTATUS4;
#endif
};


/* RTC (see manual chapter 17) */
struct s3c24x0_rtc {
#ifdef __BIG_ENDIAN
	u8	res1[67];
	u8	RTCCON;
	u8	res2[3];
	u8	TICNT;
	u8	res3[11];
	u8	RTCALM;
	u8	res4[3];
	u8	ALMSEC;
	u8	res5[3];
	u8	ALMMIN;
	u8	res6[3];
	u8	ALMHOUR;
	u8	res7[3];
	u8	ALMDATE;
	u8	res8[3];
	u8	ALMMON;
	u8	res9[3];
	u8	ALMYEAR;
	u8	res10[3];
	u8	RTCRST;
	u8	res11[3];
	u8	BCDSEC;
	u8	res12[3];
	u8	BCDMIN;
	u8	res13[3];
	u8	BCDHOUR;
	u8	res14[3];
	u8	BCDDATE;
	u8	res15[3];
	u8	BCDDAY;
	u8	res16[3];
	u8	BCDMON;
	u8	res17[3];
	u8	BCDYEAR;
#else /*  little endian */
	u8	res0[64];
	u8	RTCCON;
	u8	res1[3];
	u8	TICNT;
	u8	res2[11];
	u8	RTCALM;
	u8	res3[3];
	u8	ALMSEC;
	u8	res4[3];
	u8	ALMMIN;
	u8	res5[3];
	u8	ALMHOUR;
	u8	res6[3];
	u8	ALMDATE;
	u8	res7[3];
	u8	ALMMON;
	u8	res8[3];
	u8	ALMYEAR;
	u8	res9[3];
	u8	RTCRST;
	u8	res10[3];
	u8	BCDSEC;
	u8	res11[3];
	u8	BCDMIN;
	u8	res12[3];
	u8	BCDHOUR;
	u8	res13[3];
	u8	BCDDATE;
	u8	res14[3];
	u8	BCDDAY;
	u8	res15[3];
	u8	BCDMON;
	u8	res16[3];
	u8	BCDYEAR;
	u8	res17[3];
#endif
};


/* ADC (see manual chapter 16) */
struct s3c2400_adc {
	u32	ADCCON;
	u32	ADCDAT;
};


/* ADC (see manual chapter 16) */
struct s3c2410_adc {
	u32	ADCCON;
	u32	ADCTSC;
	u32	ADCDLY;
	u32	ADCDAT0;
	u32	ADCDAT1;
};


/* SPI (see manual chapter 22) */
struct s3c24x0_spi_channel {
	u8	SPCON;
	u8	res1[3];
	u8	SPSTA;
	u8	res2[3];
	u8	SPPIN;
	u8	res3[3];
	u8	SPPRE;
	u8	res4[3];
	u8	SPTDAT;
	u8	res5[3];
	u8	SPRDAT;
	u8	res6[3];
	u8	res7[16];
};

struct s3c24x0_spi {
	struct s3c24x0_spi_channel	ch[S3C24X0_SPI_CHANNELS];
};


/* MMC INTERFACE (see S3C2400 manual chapter 19) */
struct s3c2400_mmc {
#ifdef __BIG_ENDIAN
	u8	res1[3];
	u8	MMCON;
	u8	res2[3];
	u8	MMCRR;
	u8	res3[3];
	u8	MMFCON;
	u8	res4[3];
	u8	MMSTA;
	u16	res5;
	u16	MMFSTA;
	u8	res6[3];
	u8	MMPRE;
	u16	res7;
	u16	MMLEN;
	u8	res8[3];
	u8	MMCR7;
	u32	MMRSP[4];
	u8	res9[3];
	u8	MMCMD0;
	u32	MMCMD1;
	u16	res10;
	u16	MMCR16;
	u8	res11[3];
	u8	MMDAT;
#else
	u8	MMCON;
	u8	res1[3];
	u8	MMCRR;
	u8	res2[3];
	u8	MMFCON;
	u8	res3[3];
	u8	MMSTA;
	u8	res4[3];
	u16	MMFSTA;
	u16	res5;
	u8	MMPRE;
	u8	res6[3];
	u16	MMLEN;
	u16	res7;
	u8	MMCR7;
	u8	res8[3];
	u32	MMRSP[4];
	u8	MMCMD0;
	u8	res9[3];
	u32	MMCMD1;
	u16	MMCR16;
	u16	res10;
	u8	MMDAT;
	u8	res11[3];
#endif
};


/* SD INTERFACE (see S3C2410 manual chapter 19) */
struct s3c2410_sdi {
	u32	SDICON;
	u32	SDIPRE;
	u32	SDICARG;
	u32	SDICCON;
	u32	SDICSTA;
	u32	SDIRSP0;
	u32	SDIRSP1;
	u32	SDIRSP2;
	u32	SDIRSP3;
	u32	SDIDTIMER;
	u32	SDIBSIZE;
	u32	SDIDCON;
	u32	SDIDCNT;
	u32	SDIDSTA;
	u32	SDIFSTA;
#ifdef __BIG_ENDIAN
	u8	res[3];
	u8	SDIDAT;
#else
	u8	SDIDAT;
	u8	res[3];
#endif
	u32	SDIIMSK;
};

#endif /*__S3C24X0_H__*/
