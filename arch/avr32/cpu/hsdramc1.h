/*
 * Register definitions for SDRAM Controller
 */
#ifndef __ASM_AVR32_HSDRAMC1_H__
#define __ASM_AVR32_HSDRAMC1_H__

/* HSDRAMC1 register offsets */
#define HSDRAMC1_MR				0x0000
#define HSDRAMC1_TR				0x0004
#define HSDRAMC1_CR				0x0008
#define HSDRAMC1_HSR				0x000c
#define HSDRAMC1_LPR				0x0010
#define HSDRAMC1_IER				0x0014
#define HSDRAMC1_IDR				0x0018
#define HSDRAMC1_IMR				0x001c
#define HSDRAMC1_ISR				0x0020
#define HSDRAMC1_MDR				0x0024
#define HSDRAMC1_VERSION			0x00fc

/* Bitfields in MR */
#define HSDRAMC1_MODE_OFFSET			0
#define HSDRAMC1_MODE_SIZE			3

/* Bitfields in TR */
#define HSDRAMC1_COUNT_OFFSET			0
#define HSDRAMC1_COUNT_SIZE			12

/* Bitfields in CR */
#define HSDRAMC1_NC_OFFSET			0
#define HSDRAMC1_NC_SIZE			2
#define HSDRAMC1_NR_OFFSET			2
#define HSDRAMC1_NR_SIZE			2
#define HSDRAMC1_NB_OFFSET			4
#define HSDRAMC1_NB_SIZE			1
#define HSDRAMC1_CAS_OFFSET			5
#define HSDRAMC1_CAS_SIZE			2
#define HSDRAMC1_DBW_OFFSET			7
#define HSDRAMC1_DBW_SIZE			1
#define HSDRAMC1_TWR_OFFSET			8
#define HSDRAMC1_TWR_SIZE			4
#define HSDRAMC1_TRC_OFFSET			12
#define HSDRAMC1_TRC_SIZE			4
#define HSDRAMC1_TRP_OFFSET			16
#define HSDRAMC1_TRP_SIZE			4
#define HSDRAMC1_TRCD_OFFSET			20
#define HSDRAMC1_TRCD_SIZE			4
#define HSDRAMC1_TRAS_OFFSET			24
#define HSDRAMC1_TRAS_SIZE			4
#define HSDRAMC1_TXSR_OFFSET			28
#define HSDRAMC1_TXSR_SIZE			4

/* Bitfields in HSR */
#define HSDRAMC1_DA_OFFSET			0
#define HSDRAMC1_DA_SIZE			1

/* Bitfields in LPR */
#define HSDRAMC1_LPCB_OFFSET			0
#define HSDRAMC1_LPCB_SIZE			2
#define HSDRAMC1_PASR_OFFSET			4
#define HSDRAMC1_PASR_SIZE			3
#define HSDRAMC1_TCSR_OFFSET			8
#define HSDRAMC1_TCSR_SIZE			2
#define HSDRAMC1_DS_OFFSET			10
#define HSDRAMC1_DS_SIZE			2
#define HSDRAMC1_TIMEOUT_OFFSET			12
#define HSDRAMC1_TIMEOUT_SIZE			2

/* Bitfields in IDR */
#define HSDRAMC1_RES_OFFSET			0
#define HSDRAMC1_RES_SIZE			1

/* Bitfields in MDR */
#define HSDRAMC1_MD_OFFSET			0
#define HSDRAMC1_MD_SIZE			2

/* Bitfields in VERSION */
#define HSDRAMC1_VERSION_OFFSET			0
#define HSDRAMC1_VERSION_SIZE			12
#define HSDRAMC1_MFN_OFFSET			16
#define HSDRAMC1_MFN_SIZE			3

/* Constants for MODE */
#define HSDRAMC1_MODE_NORMAL			0
#define HSDRAMC1_MODE_NOP			1
#define HSDRAMC1_MODE_BANKS_PRECHARGE		2
#define HSDRAMC1_MODE_LOAD_MODE			3
#define HSDRAMC1_MODE_AUTO_REFRESH		4
#define HSDRAMC1_MODE_EXT_LOAD_MODE		5
#define HSDRAMC1_MODE_POWER_DOWN		6

/* Constants for NC */
#define HSDRAMC1_NC_8_COLUMN_BITS		0
#define HSDRAMC1_NC_9_COLUMN_BITS		1
#define HSDRAMC1_NC_10_COLUMN_BITS		2
#define HSDRAMC1_NC_11_COLUMN_BITS		3

/* Constants for NR */
#define HSDRAMC1_NR_11_ROW_BITS			0
#define HSDRAMC1_NR_12_ROW_BITS			1
#define HSDRAMC1_NR_13_ROW_BITS			2

/* Constants for NB */
#define HSDRAMC1_NB_TWO_BANKS			0
#define HSDRAMC1_NB_FOUR_BANKS			1

/* Constants for CAS */
#define HSDRAMC1_CAS_ONE_CYCLE			1
#define HSDRAMC1_CAS_TWO_CYCLES			2

/* Constants for DBW */
#define HSDRAMC1_DBW_32_BITS			0
#define HSDRAMC1_DBW_16_BITS			1

/* Constants for TIMEOUT */
#define HSDRAMC1_TIMEOUT_AFTER_END		0
#define HSDRAMC1_TIMEOUT_64_CYC_AFTER_END	1
#define HSDRAMC1_TIMEOUT_128_CYC_AFTER_END	2

/* Constants for MD */
#define HSDRAMC1_MD_SDRAM			0
#define HSDRAMC1_MD_LOW_POWER_SDRAM		1

/* Bit manipulation macros */
#define HSDRAMC1_BIT(name)					\
	(1 << HSDRAMC1_##name##_OFFSET)
#define HSDRAMC1_BF(name,value)					\
	(((value) & ((1 << HSDRAMC1_##name##_SIZE) - 1))	\
	 << HSDRAMC1_##name##_OFFSET)
#define HSDRAMC1_BFEXT(name,value)				\
	(((value) >> HSDRAMC1_##name##_OFFSET)			\
	 & ((1 << HSDRAMC1_##name##_SIZE) - 1))
#define HSDRAMC1_BFINS(name,value,old)				\
	(((old) & ~(((1 << HSDRAMC1_##name##_SIZE) - 1)		\
		    << HSDRAMC1_##name##_OFFSET))		\
	 | HSDRAMC1_BF(name,value))

/* Register access macros */
#define hsdramc1_readl(reg)					\
	readl((void *)ATMEL_BASE_HSDRAMC + HSDRAMC1_##reg)
#define hsdramc1_writel(reg,value)				\
	writel((value), (void *)ATMEL_BASE_HSDRAMC + HSDRAMC1_##reg)

#endif /* __ASM_AVR32_HSDRAMC1_H__ */
