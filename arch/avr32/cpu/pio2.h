/*
 * Register definitions for Parallel Input/Output Controller
 */
#ifndef __CPU_AT32AP_PIO2_H__
#define __CPU_AT32AP_PIO2_H__

/* PIO2 register offsets */
#define PIO2_PER				0x0000
#define PIO2_PDR				0x0004
#define PIO2_PSR				0x0008
#define PIO2_OER				0x0010
#define PIO2_ODR				0x0014
#define PIO2_OSR				0x0018
#define PIO2_IFER				0x0020
#define PIO2_IFDR				0x0024
#define PIO2_ISFR				0x0028
#define PIO2_SODR				0x0030
#define PIO2_CODR				0x0034
#define PIO2_ODSR				0x0038
#define PIO2_PDSR				0x003c
#define PIO2_IER				0x0040
#define PIO2_IDR				0x0044
#define PIO2_IMR				0x0048
#define PIO2_ISR				0x004c
#define PIO2_MDER				0x0050
#define PIO2_MDDR				0x0054
#define PIO2_MDSR				0x0058
#define PIO2_PUDR				0x0060
#define PIO2_PUER				0x0064
#define PIO2_PUSR				0x0068
#define PIO2_ASR				0x0070
#define PIO2_BSR				0x0074
#define PIO2_ABSR				0x0078
#define PIO2_OWER				0x00a0
#define PIO2_OWDR				0x00a4
#define PIO2_OWSR				0x00a8

/* Register access macros */
#define pio2_readl(base,reg)				\
	readl((void *)base + PIO2_##reg)
#define pio2_writel(base,reg,value)			\
	writel((value), (void *)base + PIO2_##reg)

#endif /* __CPU_AT32AP_PIO2_H__ */
