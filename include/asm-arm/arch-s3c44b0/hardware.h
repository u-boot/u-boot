/********************************************************/
/*							*/
/* Samsung S3C44B0					*/
/* tpu <tapu@371.net>					*/
/*							*/
/********************************************************/
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#define REGBASE		0x01c00000
#define REGL(addr)	(*(volatile unsigned int *)(REGBASE+addr))
#define REGW(addr)	(*(volatile unsigned short *)(REGBASE+addr))
#define REGB(addr)	(*(volatile unsigned char *)(REGBASE+addr))


/*****************************/
/* CPU Wrapper Registers     */
/*****************************/

#define SYSCFG		REGL(0x000000)
#define NCACHBE0	REGL(0x000004)
#define NCACHBE1	REGL(0x000008)
#define SBUSCON		REGL(0x040000)

/************************************/
/* Memory Controller Registers      */
/************************************/

#define BWSCON		REGL(0x080000)
#define BANKCON0	REGL(0x080004)
#define BANKCON1	REGL(0x080008)
#define BANKCON2	REGL(0x08000c)
#define BANKCON3	REGL(0x080010)
#define BANKCON4	REGL(0x080014)
#define BANKCON5	REGL(0x080018)
#define BANKCON6	REGL(0x08001c)
#define BANKCON7	REGL(0x080020)
#define REFRESH		REGL(0x080024)
#define BANKSIZE	REGL(0x080028)
#define MRSRB6		REGL(0x08002c)
#define MRSRB7		REGL(0x080030)

/*********************/
/* UART Registers    */
/*********************/

#define ULCON0		REGL(0x100000)
#define ULCON1		REGL(0x104000)
#define UCON0		REGL(0x100004)
#define UCON1		REGL(0x104004)
#define UFCON0		REGL(0x100008)
#define UFCON1		REGL(0x104008)
#define UMCON0		REGL(0x10000c)
#define UMCON1		REGL(0x10400c)
#define UTRSTAT0	REGL(0x100010)
#define UTRSTAT1	REGL(0x104010)
#define UERSTAT0	REGL(0x100014)
#define UERSTAT1	REGL(0x104014)
#define UFSTAT0		REGL(0x100018)
#define UFSTAT1		REGL(0x104018)
#define UMSTAT0		REGL(0x10001c)
#define UMSTAT1		REGL(0x10401c)
#define UTXH0		REGB(0x100020)
#define UTXH1		REGB(0x104020)
#define URXH0		REGB(0x100024)
#define URXH1		REGB(0x104024)
#define UBRDIV0		REGL(0x100028)
#define UBRDIV1		REGL(0x104028)

/*******************/
/* SIO Registers   */
/*******************/

#define SIOCON		REGL(0x114000)
#define SIODAT		REGL(0x114004)
#define SBRDR		REGL(0x114008)
#define ITVCNT		REGL(0x11400c)
#define DCNTZ		REGL(0x114010)

/********************/
/* IIS Registers    */
/********************/

#define IISCON		REGL(0x118000)
#define IISMOD		REGL(0x118004)
#define IISPSR		REGL(0x118008)
#define IISFIFCON	REGL(0x11800c)
#define IISFIF		REGW(0x118010)

/**************************/
/* I/O Ports Registers    */
/**************************/

#define PCONA		REGL(0x120000)
#define PDATA		REGL(0x120004)
#define PCONB		REGL(0x120008)
#define PDATB		REGL(0x12000c)
#define PCONC		REGL(0x120010)
#define PDATC		REGL(0x120014)
#define PUPC		REGL(0x120018)
#define PCOND		REGL(0x12001c)
#define PDATD		REGL(0x120020)
#define PUPD		REGL(0x120024)
#define PCONE		REGL(0x120028)
#define PDATE		REGL(0x12002c)
#define PUPE		REGL(0x120030)
#define PCONF		REGL(0x120034)
#define PDATF		REGL(0x120038)
#define PUPF		REGL(0x12003c)
#define PCONG		REGL(0x120040)
#define PDATG		REGL(0x120044)
#define PUPG		REGL(0x120048)
#define SPUCR		REGL(0x12004c)
#define EXTINT		REGL(0x120050)
#define EXTINTPND	REGL(0x120054)

/*********************************/
/* WatchDog Timers Registers     */
/*********************************/

#define WTCON		REGL(0x130000)
#define WTDAT		REGL(0x130004)
#define WTCNT		REGL(0x130008)

/*********************************/
/* A/D Converter Registers       */
/*********************************/

#define ADCCON		REGL(0x140000)
#define ADCPSR		REGL(0x140004)
#define ADCDAT		REGL(0x140008)

/***************************/
/* PWM Timer Registers     */
/***************************/

#define TCFG0		REGL(0x150000)
#define TCFG1		REGL(0x150004)
#define TCON		REGL(0x150008)
#define TCNTB0		REGL(0x15000c)
#define TCMPB0		REGL(0x150010)
#define TCNTO0		REGL(0x150014)
#define TCNTB1		REGL(0x150018)
#define TCMPB1		REGL(0x15001c)
#define TCNTO1		REGL(0x150020)
#define TCNTB2		REGL(0x150024)
#define TCMPB2		REGL(0x150028)
#define TCNTO2		REGL(0x15002c)
#define TCNTB3		REGL(0x150030)
#define TCMPB3		REGL(0x150034)
#define TCNTO3		REGL(0x150038)
#define TCNTB4		REGL(0x15003c)
#define TCMPB4		REGL(0x150040)
#define TCNTO4		REGL(0x150044)
#define TCNTB5		REGL(0x150048)
#define TCNTO5		REGL(0x15004c)

/*********************/
/* IIC Registers     */
/*********************/

#define IICCON		REGL(0x160000)
#define IICSTAT		REGL(0x160004)
#define IICADD		REGL(0x160008)
#define IICDS		REGL(0x16000c)

/*********************/
/* RTC Registers     */
/*********************/

#define RTCCON		REGB(0x170040)
#define RTCALM		REGB(0x170050)
#define ALMSEC		REGB(0x170054)
#define ALMMIN		REGB(0x170058)
#define ALMHOUR		REGB(0x17005c)
#define ALMDAY		REGB(0x170060)
#define ALMMON		REGB(0x170064)
#define ALMYEAR		REGB(0x170068)
#define RTCRST		REGB(0x17006c)
#define BCDSEC		REGB(0x170070)
#define BCDMIN		REGB(0x170074)
#define BCDHOUR		REGB(0x170078)
#define BCDDAY		REGB(0x17007c)
#define BCDDATE		REGB(0x170080)
#define BCDMON		REGB(0x170084)
#define BCDYEAR		REGB(0x170088)
#define TICINT		REGB(0x17008c)

/*********************************/
/* Clock & Power Registers       */
/*********************************/

#define PLLCON		REGL(0x180000)
#define CLKCON		REGL(0x180004)
#define CLKSLOW		REGL(0x180008)
#define LOCKTIME	REGL(0x18000c)

/**************************************/
/* Interrupt Controller Registers     */
/**************************************/

#define INTCON		REGL(0x200000)
#define INTPND		REGL(0x200004)
#define INTMOD		REGL(0x200008)
#define INTMSK		REGL(0x20000c)
#define I_PSLV		REGL(0x200010)
#define I_PMST		REGL(0x200014)
#define I_CSLV		REGL(0x200018)
#define I_CMST		REGL(0x20001c)
#define I_ISPR		REGL(0x200020)
#define I_ISPC		REGL(0x200024)
#define F_ISPR		REGL(0x200038)
#define F_ISPC		REGL(0x20003c)

/********************************/
/* LCD Controller Registers     */
/********************************/

#define LCDCON1		REGL(0x300000)
#define LCDCON2		REGL(0x300004)
#define LCDSADDR1	REGL(0x300008)
#define LCDSADDR2	REGL(0x30000c)
#define LCDSADDR3	REGL(0x300010)
#define REDLUT		REGL(0x300014)
#define GREENLUT	REGL(0x300018)
#define BLUELUT		REGL(0x30001c)
#define DP1_2		REGL(0x300020)
#define DP4_7		REGL(0x300024)
#define DP3_5		REGL(0x300028)
#define DP2_3		REGL(0x30002c)
#define DP5_7		REGL(0x300030)
#define DP3_4		REGL(0x300034)
#define DP4_5		REGL(0x300038)
#define DP6_7		REGL(0x30003c)
#define LCDCON3		REGL(0x300040)
#define DITHMODE	REGL(0x300044)

/*********************/
/* DMA Registers     */
/*********************/

#define ZDCON0		REGL(0x280000)
#define ZDISRC0		REGL(0x280004)
#define ZDIDES0		REGL(0x280008)
#define ZDICNT0		REGL(0x28000c)
#define ZDCSRC0		REGL(0x280010)
#define ZDCDES0		REGL(0x280014)
#define ZDCCNT0		REGL(0x280018)

#define ZDCON1		REGL(0x280020)
#define ZDISRC1		REGL(0x280024)
#define ZDIDES1		REGL(0x280028)
#define ZDICNT1		REGL(0x28002c)
#define ZDCSRC1		REGL(0x280030)
#define ZDCDES1		REGL(0x280034)
#define ZDCCNT1		REGL(0x280038)

#define BDCON0		REGL(0x380000)
#define BDISRC0		REGL(0x380004)
#define BDIDES0		REGL(0x380008)
#define BDICNT0		REGL(0x38000c)
#define BDCSRC0		REGL(0x380010)
#define BDCDES0		REGL(0x380014)
#define BDCCNT0		REGL(0x380018)

#define BDCON1		REGL(0x380020)
#define BDISRC1		REGL(0x380024)
#define BDIDES1		REGL(0x380028)
#define BDICNT1		REGL(0x38002c)
#define BDCSRC1		REGL(0x380030)
#define BDCDES1		REGL(0x380034)
#define BDCCNT1		REGL(0x380038)


#define CLEAR_PEND_INT(n)       I_ISPC = (1<<(n))
#define INT_ENABLE(n)		INTMSK &= ~(1<<(n))
#define INT_DISABLE(n)		INTMSK |= (1<<(n))

#define HARD_RESET_NOW()

#endif /* __ASM_ARCH_HARDWARE_H */
