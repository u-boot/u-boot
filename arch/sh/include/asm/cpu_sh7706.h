#ifndef _ASM_CPU_SH7706_H_
#define _ASM_CPU_SH7706_H_

#define CACHE_OC_NUM_WAYS	4
#define CCR_CACHE_INIT	0x0000000D

/* MMU and Cache control */
#define MMUCR	0xFFFFFFE0
#define CCR		0xFFFFFFEC

/* PFC */
#define PACR		0xA4050100
#define PBCR		0xA4050102
#define PCCR		0xA4050104
#define PETCR		0xA4050106

/* Port Data Registers */
#define PADR		0xA4050120
#define PBDR		0xA4050122
#define PCDR		0xA4050124

/* BSC */
#define	FRQCR	0xffffff80
#define	BCR1	0xffffff60
#define	BCR2	0xffffff62
#define	WCR1	0xffffff64
#define	WCR2	0xffffff66
#define	MCR		0xffffff68

/* SDRAM controller */
#define	DCR		0xffffff6a
#define	RTCSR	0xffffff6e
#define	RTCNT	0xffffff70
#define	RTCOR	0xffffff72
#define	RFCR	0xffffff74
#define SDMR	0xFFFFD000
#define CS3_R	0xFFFFE460

/* SCIF */
#define SCSMR_2		0xA4000150
#define SCIF0_BASE	SCSMR_2

/* Timer */
#define TSTR0		0xFFFFFE92
#define TSTR		TSTR0
#define TCNT0		0xFFFFFE98
#define TCR0		0xFFFFFE9C

/* On chip oscillator circuits */
#define	WTCNT	0xFFFFFF84
#define	WTCSR	0xFFFFFF86

#endif	/* _ASM_CPU_SH7706_H_ */
