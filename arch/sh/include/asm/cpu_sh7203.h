#ifndef _ASM_CPU_SH7203_H_
#define _ASM_CPU_SH7203_H_

/* Cache */
#define CCR1		0xFFFC1000
#define CCR			CCR1

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

/* SDRAM controller */

/* SCIF */
#define SCSMR_0		0xFFFE8000
#define SCIF0_BASE	SCSMR_0

/* Timer(CMT) */
#define CMSTR 	0xFFFEC000
#define CMCSR_0 0xFFFEC002
#define CMCNT_0 0xFFFEC004
#define CMCOR_0 0xFFFEC006
#define CMCSR_1 0xFFFEC008
#define CMCNT_1 0xFFFEC00A
#define CMCOR_1	0xFFFEC00C

/* On chip oscillator circuits */
#define FRQCR		0xA415FF80
#define WTCNT		0xA415FF84
#define WTCSR		0xA415FF86

#endif	/* _ASM_CPU_SH7203_H_ */
