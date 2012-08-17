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
	u32	bwscon;
	u32	bankcon[8];
	u32	refresh;
	u32	banksize;
	u32	mrsrb6;
	u32	mrsrb7;
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
	u32	srcpnd;
	u32	intmod;
	u32	intmsk;
	u32	priority;
	u32	intpnd;
	u32	intoffset;
#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
	u32	subsrcpnd;
	u32	intsubmsk;
#endif
};


/* DMAS (see manual chapter 8) */
struct s3c24x0_dma {
	u32	disrc;
#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
	u32	disrcc;
#endif
	u32	didst;
#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
	u32	didstc;
#endif
	u32	dcon;
	u32	dstat;
	u32	dcsrc;
	u32	dcdst;
	u32	dmasktrig;
#if defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410) \
		|| defined(CONFIG_S3C2440)
	u32	res[1];
#endif
};

struct s3c24x0_dmas {
	struct s3c24x0_dma	dma[4];
};


/* CLOCK & POWER MANAGEMENT (see S3C2400 manual chapter 6) */
/*                          (see S3C2410 manual chapter 7) */
struct s3c24x0_clock_power {
	u32	locktime;
	u32	mpllcon;
	u32	upllcon;
	u32	clkcon;
	u32	clkslow;
	u32	clkdivn;
#if defined(CONFIG_S3C2440)
	u32	camdivn;
#endif
};


/* LCD CONTROLLER (see manual chapter 15) */
struct s3c24x0_lcd {
	u32	lcdcon1;
	u32	lcdcon2;
	u32	lcdcon3;
	u32	lcdcon4;
	u32	lcdcon5;
	u32	lcdsaddr1;
	u32	lcdsaddr2;
	u32	lcdsaddr3;
	u32	redlut;
	u32	greenlut;
	u32	bluelut;
	u32	res[8];
	u32	dithmode;
	u32	tpal;
#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
	u32	lcdintpnd;
	u32	lcdsrcpnd;
	u32	lcdintmsk;
	u32	lpcsel;
#endif
};


#ifdef CONFIG_S3C2410
/* NAND FLASH (see S3C2410 manual chapter 6) */
struct s3c2410_nand {
	u32	nfconf;
	u32	nfcmd;
	u32	nfaddr;
	u32	nfdata;
	u32	nfstat;
	u32	nfecc;
};
#endif
#ifdef CONFIG_S3C2440
/* NAND FLASH (see S3C2440 manual chapter 6) */
struct s3c2440_nand {
	u32	nfconf;
	u32	nfcont;
	u32	nfcmd;
	u32	nfaddr;
	u32	nfdata;
	u32	nfeccd0;
	u32	nfeccd1;
	u32	nfeccd;
	u32	nfstat;
	u32	nfstat0;
	u32	nfstat1;
};
#endif


/* UART (see manual chapter 11) */
struct s3c24x0_uart {
	u32	ulcon;
	u32	ucon;
	u32	ufcon;
	u32	umcon;
	u32	utrstat;
	u32	uerstat;
	u32	ufstat;
	u32	umstat;
#ifdef __BIG_ENDIAN
	u8	res1[3];
	u8	utxh;
	u8	res2[3];
	u8	urxh;
#else /* Little Endian */
	u8	utxh;
	u8	res1[3];
	u8	urxh;
	u8	res2[3];
#endif
	u32	ubrdiv;
};


/* PWM TIMER (see manual chapter 10) */
struct s3c24x0_timer {
	u32	tcntb;
	u32	tcmpb;
	u32	tcnto;
};

struct s3c24x0_timers {
	u32	tcfg0;
	u32	tcfg1;
	u32	tcon;
	struct s3c24x0_timer	ch[4];
	u32	tcntb4;
	u32	tcnto4;
};


/* USB DEVICE (see manual chapter 13) */
struct s3c24x0_usb_dev_fifos {
#ifdef __BIG_ENDIAN
	u8	res[3];
	u8	ep_fifo_reg;
#else /*  little endian */
	u8	ep_fifo_reg;
	u8	res[3];
#endif
};

struct s3c24x0_usb_dev_dmas {
#ifdef __BIG_ENDIAN
	u8	res1[3];
	u8	ep_dma_con;
	u8	res2[3];
	u8	ep_dma_unit;
	u8	res3[3];
	u8	ep_dma_fifo;
	u8	res4[3];
	u8	ep_dma_ttc_l;
	u8	res5[3];
	u8	ep_dma_ttc_m;
	u8	res6[3];
	u8	ep_dma_ttc_h;
#else /*  little endian */
	u8	ep_dma_con;
	u8	res1[3];
	u8	ep_dma_unit;
	u8	res2[3];
	u8	ep_dma_fifo;
	u8	res3[3];
	u8	ep_dma_ttc_l;
	u8	res4[3];
	u8	ep_dma_ttc_m;
	u8	res5[3];
	u8	ep_dma_ttc_h;
	u8	res6[3];
#endif
};

struct s3c24x0_usb_device {
#ifdef __BIG_ENDIAN
	u8	res1[3];
	u8	func_addr_reg;
	u8	res2[3];
	u8	pwr_reg;
	u8	res3[3];
	u8	ep_int_reg;
	u8	res4[15];
	u8	usb_int_reg;
	u8	res5[3];
	u8	ep_int_en_reg;
	u8	res6[15];
	u8	usb_int_en_reg;
	u8	res7[3];
	u8	frame_num1_reg;
	u8	res8[3];
	u8	frame_num2_reg;
	u8	res9[3];
	u8	index_reg;
	u8	res10[7];
	u8	maxp_reg;
	u8	res11[3];
	u8	ep0_csr_in_csr1_reg;
	u8	res12[3];
	u8	in_csr2_reg;
	u8	res13[7];
	u8	out_csr1_reg;
	u8	res14[3];
	u8	out_csr2_reg;
	u8	res15[3];
	u8	out_fifo_cnt1_reg;
	u8	res16[3];
	u8	out_fifo_cnt2_reg;
#else /*  little endian */
	u8	func_addr_reg;
	u8	res1[3];
	u8	pwr_reg;
	u8	res2[3];
	u8	ep_int_reg;
	u8	res3[15];
	u8	usb_int_reg;
	u8	res4[3];
	u8	ep_int_en_reg;
	u8	res5[15];
	u8	usb_int_en_reg;
	u8	res6[3];
	u8	frame_num1_reg;
	u8	res7[3];
	u8	frame_num2_reg;
	u8	res8[3];
	u8	index_reg;
	u8	res9[7];
	u8	maxp_reg;
	u8	res10[7];
	u8	ep0_csr_in_csr1_reg;
	u8	res11[3];
	u8	in_csr2_reg;
	u8	res12[3];
	u8	out_csr1_reg;
	u8	res13[7];
	u8	out_csr2_reg;
	u8	res14[3];
	u8	out_fifo_cnt1_reg;
	u8	res15[3];
	u8	out_fifo_cnt2_reg;
	u8	res16[3];
#endif /*  __BIG_ENDIAN */
	struct s3c24x0_usb_dev_fifos	fifo[5];
	struct s3c24x0_usb_dev_dmas	dma[5];
};


/* WATCH DOG TIMER (see manual chapter 18) */
struct s3c24x0_watchdog {
	u32	wtcon;
	u32	wtdat;
	u32	wtcnt;
};


/* IIC (see manual chapter 20) */
struct s3c24x0_i2c {
	u32	iiccon;
	u32	iicstat;
	u32	iicadd;
	u32	iicds;
};


/* IIS (see manual chapter 21) */
struct s3c24x0_i2s {
#ifdef __BIG_ENDIAN
	u16	res1;
	u16	iiscon;
	u16	res2;
	u16	iismod;
	u16	res3;
	u16	iispsr;
	u16	res4;
	u16	iisfcon;
	u16	res5;
	u16	iisfifo;
#else /*  little endian */
	u16	iiscon;
	u16	res1;
	u16	iismod;
	u16	res2;
	u16	iispsr;
	u16	res3;
	u16	iisfcon;
	u16	res4;
	u16	iisfifo;
	u16	res5;
#endif
};


/* I/O PORT (see manual chapter 9) */
struct s3c24x0_gpio {
#ifdef CONFIG_S3C2400
	u32	pacon;
	u32	padat;

	u32	pbcon;
	u32	pbdat;
	u32	pbup;

	u32	pccon;
	u32	pcdat;
	u32	pcup;

	u32	pdcon;
	u32	pddat;
	u32	pdup;

	u32	pecon;
	u32	pedat;
	u32	peup;

	u32	pfcon;
	u32	pfdat;
	u32	pfup;

	u32	pgcon;
	u32	pgdat;
	u32	pgup;

	u32	opencr;

	u32	misccr;
	u32	extint;
#endif
#ifdef CONFIG_S3C2410
	u32	gpacon;
	u32	gpadat;
	u32	res1[2];
	u32	gpbcon;
	u32	gpbdat;
	u32	gpbup;
	u32	res2;
	u32	gpccon;
	u32	gpcdat;
	u32	gpcup;
	u32	res3;
	u32	gpdcon;
	u32	gpddat;
	u32	gpdup;
	u32	res4;
	u32	gpecon;
	u32	gpedat;
	u32	gpeup;
	u32	res5;
	u32	gpfcon;
	u32	gpfdat;
	u32	gpfup;
	u32	res6;
	u32	gpgcon;
	u32	gpgdat;
	u32	gpgup;
	u32	res7;
	u32	gphcon;
	u32	gphdat;
	u32	gphup;
	u32	res8;

	u32	misccr;
	u32	dclkcon;
	u32	extint0;
	u32	extint1;
	u32	extint2;
	u32	eintflt0;
	u32	eintflt1;
	u32	eintflt2;
	u32	eintflt3;
	u32	eintmask;
	u32	eintpend;
	u32	gstatus0;
	u32	gstatus1;
	u32	gstatus2;
	u32	gstatus3;
	u32	gstatus4;
#endif
#if defined(CONFIG_S3C2440)
	u32	gpacon;
	u32	gpadat;
	u32	res1[2];
	u32	gpbcon;
	u32	gpbdat;
	u32	gpbup;
	u32	res2;
	u32	gpccon;
	u32	gpcdat;
	u32	gpcup;
	u32	res3;
	u32	gpdcon;
	u32	gpddat;
	u32	gpdup;
	u32	res4;
	u32	gpecon;
	u32	gpedat;
	u32	gpeup;
	u32	res5;
	u32	gpfcon;
	u32	gpfdat;
	u32	gpfup;
	u32	res6;
	u32	gpgcon;
	u32	gpgdat;
	u32	gpgup;
	u32	res7;
	u32	gphcon;
	u32	gphdat;
	u32	gphup;
	u32	res8;

	u32	misccr;
	u32	dclkcon;
	u32	extint0;
	u32	extint1;
	u32	extint2;
	u32	eintflt0;
	u32	eintflt1;
	u32	eintflt2;
	u32	eintflt3;
	u32	eintmask;
	u32	eintpend;
	u32	gstatus0;
	u32	gstatus1;
	u32	gstatus2;
	u32	gstatus3;
	u32	gstatus4;

	u32	res9;
	u32	dsc0;
	u32	dsc1;
	u32	mslcon;
	u32	gpjcon;
	u32	gpjdat;
	u32	gpjup;
	u32	res10;
#endif
};


/* RTC (see manual chapter 17) */
struct s3c24x0_rtc {
#ifdef __BIG_ENDIAN
	u8	res1[67];
	u8	rtccon;
	u8	res2[3];
	u8	ticnt;
	u8	res3[11];
	u8	rtcalm;
	u8	res4[3];
	u8	almsec;
	u8	res5[3];
	u8	almmin;
	u8	res6[3];
	u8	almhour;
	u8	res7[3];
	u8	almdate;
	u8	res8[3];
	u8	almmon;
	u8	res9[3];
	u8	almyear;
	u8	res10[3];
	u8	rtcrst;
	u8	res11[3];
	u8	bcdsec;
	u8	res12[3];
	u8	bcdmin;
	u8	res13[3];
	u8	bcdhour;
	u8	res14[3];
	u8	bcddate;
	u8	res15[3];
	u8	bcdday;
	u8	res16[3];
	u8	bcdmon;
	u8	res17[3];
	u8	bcdyear;
#else /*  little endian */
	u8	res0[64];
	u8	rtccon;
	u8	res1[3];
	u8	ticnt;
	u8	res2[11];
	u8	rtcalm;
	u8	res3[3];
	u8	almsec;
	u8	res4[3];
	u8	almmin;
	u8	res5[3];
	u8	almhour;
	u8	res6[3];
	u8	almdate;
	u8	res7[3];
	u8	almmon;
	u8	res8[3];
	u8	almyear;
	u8	res9[3];
	u8	rtcrst;
	u8	res10[3];
	u8	bcdsec;
	u8	res11[3];
	u8	bcdmin;
	u8	res12[3];
	u8	bcdhour;
	u8	res13[3];
	u8	bcddate;
	u8	res14[3];
	u8	bcdday;
	u8	res15[3];
	u8	bcdmon;
	u8	res16[3];
	u8	bcdyear;
	u8	res17[3];
#endif
};


/* ADC (see manual chapter 16) */
struct s3c2400_adc {
	u32	adccon;
	u32	adcdat;
};


/* ADC (see manual chapter 16) */
struct s3c2410_adc {
	u32	adccon;
	u32	adctsc;
	u32	adcdly;
	u32	adcdat0;
	u32	adcdat1;
};


/* SPI (see manual chapter 22) */
struct s3c24x0_spi_channel {
	u8	spcon;
	u8	res1[3];
	u8	spsta;
	u8	res2[3];
	u8	sppin;
	u8	res3[3];
	u8	sppre;
	u8	res4[3];
	u8	sptdat;
	u8	res5[3];
	u8	sprdat;
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
	u8	mmcon;
	u8	res2[3];
	u8	mmcrr;
	u8	res3[3];
	u8	mmfcon;
	u8	res4[3];
	u8	mmsta;
	u16	res5;
	u16	mmfsta;
	u8	res6[3];
	u8	mmpre;
	u16	res7;
	u16	mmlen;
	u8	res8[3];
	u8	mmcr7;
	u32	mmrsp[4];
	u8	res9[3];
	u8	mmcmd0;
	u32	mmcmd1;
	u16	res10;
	u16	mmcr16;
	u8	res11[3];
	u8	mmdat;
#else
	u8	mmcon;
	u8	res1[3];
	u8	mmcrr;
	u8	res2[3];
	u8	mmfcon;
	u8	res3[3];
	u8	mmsta;
	u8	res4[3];
	u16	mmfsta;
	u16	res5;
	u8	mmpre;
	u8	res6[3];
	u16	mmlen;
	u16	res7;
	u8	mmcr7;
	u8	res8[3];
	u32	mmrsp[4];
	u8	mmcmd0;
	u8	res9[3];
	u32	mmcmd1;
	u16	mmcr16;
	u16	res10;
	u8	mmdat;
	u8	res11[3];
#endif
};


/* SD INTERFACE (see S3C2410 manual chapter 19) */
struct s3c2410_sdi {
	u32	sdicon;
	u32	sdipre;
	u32	sdicarg;
	u32	sdiccon;
	u32	sdicsta;
	u32	sdirsp0;
	u32	sdirsp1;
	u32	sdirsp2;
	u32	sdirsp3;
	u32	sdidtimer;
	u32	sdibsize;
	u32	sdidcon;
	u32	sdidcnt;
	u32	sdidsta;
	u32	sdifsta;
#ifdef __BIG_ENDIAN
	u8	res[3];
	u8	sdidat;
#else
	u8	sdidat;
	u8	res[3];
#endif
	u32	sdiimsk;
};

#endif /*__S3C24X0_H__*/
