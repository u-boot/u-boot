/*
 * Register definitions for Static Memory Controller
 */
#ifndef __CPU_AT32AP_HSMC3_H__
#define __CPU_AT32AP_HSMC3_H__

/* HSMC3 register offsets */
#define HSMC3_SETUP0				0x0000
#define HSMC3_PULSE0				0x0004
#define HSMC3_CYCLE0				0x0008
#define HSMC3_MODE0				0x000c
#define HSMC3_SETUP1				0x0010
#define HSMC3_PULSE1				0x0014
#define HSMC3_CYCLE1				0x0018
#define HSMC3_MODE1				0x001c
#define HSMC3_SETUP2				0x0020
#define HSMC3_PULSE2				0x0024
#define HSMC3_CYCLE2				0x0028
#define HSMC3_MODE2				0x002c
#define HSMC3_SETUP3				0x0030
#define HSMC3_PULSE3				0x0034
#define HSMC3_CYCLE3				0x0038
#define HSMC3_MODE3				0x003c
#define HSMC3_SETUP4				0x0040
#define HSMC3_PULSE4				0x0044
#define HSMC3_CYCLE4				0x0048
#define HSMC3_MODE4				0x004c
#define HSMC3_SETUP5				0x0050
#define HSMC3_PULSE5				0x0054
#define HSMC3_CYCLE5				0x0058
#define HSMC3_MODE5				0x005c

/* Bitfields in SETUP0 */
#define HSMC3_NWE_SETUP_OFFSET			0
#define HSMC3_NWE_SETUP_SIZE			6
#define HSMC3_NCS_WR_SETUP_OFFSET		8
#define HSMC3_NCS_WR_SETUP_SIZE			6
#define HSMC3_NRD_SETUP_OFFSET			16
#define HSMC3_NRD_SETUP_SIZE			6
#define HSMC3_NCS_RD_SETUP_OFFSET		24
#define HSMC3_NCS_RD_SETUP_SIZE			6

/* Bitfields in PULSE0 */
#define HSMC3_NWE_PULSE_OFFSET			0
#define HSMC3_NWE_PULSE_SIZE			7
#define HSMC3_NCS_WR_PULSE_OFFSET		8
#define HSMC3_NCS_WR_PULSE_SIZE			7
#define HSMC3_NRD_PULSE_OFFSET			16
#define HSMC3_NRD_PULSE_SIZE			7
#define HSMC3_NCS_RD_PULSE_OFFSET		24
#define HSMC3_NCS_RD_PULSE_SIZE			7

/* Bitfields in CYCLE0 */
#define HSMC3_NWE_CYCLE_OFFSET			0
#define HSMC3_NWE_CYCLE_SIZE			9
#define HSMC3_NRD_CYCLE_OFFSET			16
#define HSMC3_NRD_CYCLE_SIZE			9

/* Bitfields in MODE0 */
#define HSMC3_READ_MODE_OFFSET			0
#define HSMC3_READ_MODE_SIZE			1
#define HSMC3_WRITE_MODE_OFFSET			1
#define HSMC3_WRITE_MODE_SIZE			1
#define HSMC3_EXNW_MODE_OFFSET			4
#define HSMC3_EXNW_MODE_SIZE			2
#define HSMC3_BAT_OFFSET			8
#define HSMC3_BAT_SIZE				1
#define HSMC3_DBW_OFFSET			12
#define HSMC3_DBW_SIZE				2
#define HSMC3_TDF_CYCLES_OFFSET			16
#define HSMC3_TDF_CYCLES_SIZE			4
#define HSMC3_TDF_MODE_OFFSET			20
#define HSMC3_TDF_MODE_SIZE			1
#define HSMC3_PMEN_OFFSET			24
#define HSMC3_PMEN_SIZE				1
#define HSMC3_PS_OFFSET				28
#define HSMC3_PS_SIZE				2

/* Bitfields in MODE1 */
#define HSMC3_PD_OFFSET				28
#define HSMC3_PD_SIZE				2

/* Constants for READ_MODE */
#define HSMC3_READ_MODE_NCS_CONTROLLED		0
#define HSMC3_READ_MODE_NRD_CONTROLLED		1

/* Constants for WRITE_MODE */
#define HSMC3_WRITE_MODE_NCS_CONTROLLED		0
#define HSMC3_WRITE_MODE_NWE_CONTROLLED		1

/* Constants for EXNW_MODE */
#define HSMC3_EXNW_MODE_DISABLED		0
#define HSMC3_EXNW_MODE_RESERVED		1
#define HSMC3_EXNW_MODE_FROZEN			2
#define HSMC3_EXNW_MODE_READY			3

/* Constants for BAT */
#define HSMC3_BAT_BYTE_SELECT			0
#define HSMC3_BAT_BYTE_WRITE			1

/* Constants for DBW */
#define HSMC3_DBW_8_BITS			0
#define HSMC3_DBW_16_BITS			1
#define HSMC3_DBW_32_BITS			2

/* Bit manipulation macros */
#define HSMC3_BIT(name)						\
	(1 << HSMC3_##name##_OFFSET)
#define HSMC3_BF(name,value)					\
	(((value) & ((1 << HSMC3_##name##_SIZE) - 1))		\
	 << HSMC3_##name##_OFFSET)
#define HSMC3_BFEXT(name,value)					\
	(((value) >> HSMC3_##name##_OFFSET)			\
	 & ((1 << HSMC3_##name##_SIZE) - 1))
#define HSMC3_BFINS(name,value,old)\
	(((old) & ~(((1 << HSMC3_##name##_SIZE) - 1)		\
		    << HSMC3_##name##_OFFSET))			\
	 | HSMC3_BF(name,value))

/* Register access macros */
#define hsmc3_readl(port,reg)					\
	readl((port)->regs + HSMC3_##reg)
#define hsmc3_writel(port,reg,value)				\
	writel((value), (port)->regs + HSMC3_##reg)

#endif /* __CPU_AT32AP_HSMC3_H__ */
