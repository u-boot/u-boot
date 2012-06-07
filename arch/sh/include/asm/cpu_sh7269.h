#ifndef _ASM_CPU_SH7269_H_
#define _ASM_CPU_SH7269_H_

/* Cache */
#define CCR1		0xFFFC1000
#define CCR		CCR1

/* SCIF */
#define SCSMR_0		0xE8007000
#define SCIF0_BASE	SCSMR_0
#define SCSMR_1		0xE8007800
#define SCIF1_BASE	SCSMR_1
#define SCSMR_2		0xE8008000
#define SCIF2_BASE	SCSMR_2
#define SCSMR_3		0xE8008800
#define SCIF3_BASE	SCSMR_3
#define SCSMR_7		0xE800A800
#define SCIF7_BASE	SCSMR_7

/* Timer(CMT) */
#define CMSTR		0xFFFEC000
#define CMCSR_0		0xFFFEC002
#define CMCNT_0		0xFFFEC004
#define CMCOR_0		0xFFFEC006

#endif	/* _ASM_CPU_SH7269_H_ */
