#ifndef _ASM_CPU_SH7710_H_
#define _ASM_CPU_SH7710_H_

#define CACHE_OC_NUM_WAYS	4
#define CCR_CACHE_INIT	0x0000000D

/* MMU and Cache control */
#define MMUCR		0xFFFFFFE0
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
#define CMNCR		0xA4FD0000
#define CS0BCR		0xA4FD0004
#define CS2BCR		0xA4FD0008
#define CS3BCR		0xA4FD000C
#define CS4BCR		0xA4FD0010
#define CS5ABCR		0xA4FD0014
#define CS5BBCR		0xA4FD0018
#define CS6ABCR		0xA4FD001C
#define CS6BBCR		0xA4FD0020
#define CS0WCR		0xA4FD0024
#define CS2WCR		0xA4FD0028
#define CS3WCR		0xA4FD002C
#define CS4WCR		0xA4FD0030
#define CS5AWCR		0xA4FD0034
#define CS5BWCR		0xA4FD0038
#define CS6AWCR		0xA4FD003C
#define CS6BWCR		0xA4FD0040

/* SDRAM controller */
#define SDCR		0xA4FD0044
#define RTCSR		0xA4FD0048
#define RTCNT		0xA4FD004C
#define RTCOR		0xA4FD0050

/* SCIF */
#define SCSMR_0		0xA4400000
#define SCIF0_BASE	SCSMR_0
#define SCSMR_0		0xA4410000
#define SCIF1_BASE	SCSMR_1

/* Timer */
#define TMU_BASE	0xA412FE90

/* On chip oscillator circuits */
#define FRQCR		0xA415FF80
#define WTCNT		0xA415FF84
#define WTCSR		0xA415FF86

#endif	/* _ASM_CPU_SH7710_H_ */
