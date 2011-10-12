/*
 * head file for Ingenic Semiconductor's JZ4740 CPU.
 */
#ifndef __JZ4740_H__
#define __JZ4740_H__

#include <asm/addrspace.h>
#include <asm/cacheops.h>

/* Boot ROM Specification  */
/* NOR Boot config */
#define JZ4740_NORBOOT_8BIT	0x00000000	/* 8-bit data bus flash */
#define JZ4740_NORBOOT_16BIT	0x10101010	/* 16-bit data bus flash */
#define JZ4740_NORBOOT_32BIT	0x20202020	/* 32-bit data bus flash */
/* NAND Boot config */
#define JZ4740_NANDBOOT_B8R3	0xffffffff	/* 8-bit bus & 3 row cycles */
#define JZ4740_NANDBOOT_B8R2	0xf0f0f0f0	/* 8-bit bus & 2 row cycles */
#define JZ4740_NANDBOOT_B16R3	0x0f0f0f0f	/* 16-bit bus & 3 row cycles */
#define JZ4740_NANDBOOT_B16R2	0x00000000	/* 16-bit bus & 2 row cycles */

/* 1st-level interrupts */
#define JZ4740_IRQ_I2C		1
#define JZ4740_IRQ_UHC		3
#define JZ4740_IRQ_UART0	9
#define JZ4740_IRQ_SADC		12
#define JZ4740_IRQ_MSC		14
#define JZ4740_IRQ_RTC		15
#define JZ4740_IRQ_SSI		16
#define JZ4740_IRQ_CIM		17
#define JZ4740_IRQ_AIC		18
#define JZ4740_IRQ_ETH		19
#define JZ4740_IRQ_DMAC		20
#define JZ4740_IRQ_TCU2		21
#define JZ4740_IRQ_TCU1		22
#define JZ4740_IRQ_TCU0		23
#define JZ4740_IRQ_UDC		24
#define JZ4740_IRQ_GPIO3	25
#define JZ4740_IRQ_GPIO2	26
#define JZ4740_IRQ_GPIO1	27
#define JZ4740_IRQ_GPIO0	28
#define JZ4740_IRQ_IPU		29
#define JZ4740_IRQ_LCD		30
/* 2nd-level interrupts */
#define JZ4740_IRQ_DMA_0	32  /* 32 to 37 for DMAC channel 0 to 5 */
#define JZ4740_IRQ_GPIO_0	48  /* 48 to 175 for GPIO pin 0 to 127 */

/* Register Definitions */
#define JZ4740_CPM_BASE		0x10000000
#define JZ4740_INTC_BASE	0x10001000
#define JZ4740_TCU_BASE		0x10002000
#define JZ4740_WDT_BASE		0x10002000
#define JZ4740_RTC_BASE		0x10003000
#define JZ4740_GPIO_BASE	0x10010000
#define JZ4740_AIC_BASE		0x10020000
#define JZ4740_ICDC_BASE	0x10020000
#define JZ4740_MSC_BASE		0x10021000
#define JZ4740_UART0_BASE	0x10030000
#define JZ4740_I2C_BASE		0x10042000
#define JZ4740_SSI_BASE		0x10043000
#define JZ4740_SADC_BASE	0x10070000
#define JZ4740_EMC_BASE		0x13010000
#define JZ4740_DMAC_BASE	0x13020000
#define JZ4740_UHC_BASE		0x13030000
#define JZ4740_UDC_BASE		0x13040000
#define JZ4740_LCD_BASE		0x13050000
#define JZ4740_SLCD_BASE	0x13050000
#define JZ4740_CIM_BASE		0x13060000
#define JZ4740_ETH_BASE		0x13100000

/* 8bit Mode Register of SDRAM bank 0 */
#define JZ4740_EMC_SDMR0	(JZ4740_EMC_BASE + 0xa000)

/* GPIO (General-Purpose I/O Ports) */
/*  = 0,1,2,3 */
#define GPIO_PXPIN(n)	\
	(JZ4740_GPIO_BASE + (0x00 + (n)*0x100)) /* PIN Level Register */
#define GPIO_PXDAT(n)	\
	(JZ4740_GPIO_BASE + (0x10 + (n)*0x100)) /* Port Data Register */
#define GPIO_PXDATS(n)	\
	(JZ4740_GPIO_BASE + (0x14 + (n)*0x100)) /* Port Data Set Register */
#define GPIO_PXDATC(n)	\
	(JZ4740_GPIO_BASE + (0x18 + (n)*0x100)) /* Port Data Clear Register */
#define GPIO_PXIM(n)	\
	(JZ4740_GPIO_BASE + (0x20 + (n)*0x100)) /* Interrupt Mask Register */
#define GPIO_PXIMS(n)	\
	(JZ4740_GPIO_BASE + (0x24 + (n)*0x100)) /* Interrupt Mask Set Reg */
#define GPIO_PXIMC(n)	\
	(JZ4740_GPIO_BASE + (0x28 + (n)*0x100)) /* Interrupt Mask Clear Reg */
#define GPIO_PXPE(n)	\
	(JZ4740_GPIO_BASE + (0x30 + (n)*0x100)) /* Pull Enable Register */
#define GPIO_PXPES(n)	\
	(JZ4740_GPIO_BASE + (0x34 + (n)*0x100)) /* Pull Enable Set Reg. */
#define GPIO_PXPEC(n)	\
	(JZ4740_GPIO_BASE + (0x38 + (n)*0x100)) /* Pull Enable Clear Reg. */
#define GPIO_PXFUN(n)	\
	(JZ4740_GPIO_BASE + (0x40 + (n)*0x100)) /* Function Register */
#define GPIO_PXFUNS(n)	\
	(JZ4740_GPIO_BASE + (0x44 + (n)*0x100)) /* Function Set Register */
#define GPIO_PXFUNC(n)	\
	(JZ4740_GPIO_BASE + (0x48 + (n)*0x100)) /* Function Clear Register */
#define GPIO_PXSEL(n)	\
	(JZ4740_GPIO_BASE + (0x50 + (n)*0x100)) /* Select Register */
#define GPIO_PXSELS(n)	\
	(JZ4740_GPIO_BASE + (0x54 + (n)*0x100)) /* Select Set Register */
#define GPIO_PXSELC(n)	\
	(JZ4740_GPIO_BASE + (0x58 + (n)*0x100)) /* Select Clear Register */
#define GPIO_PXDIR(n)	\
	(JZ4740_GPIO_BASE + (0x60 + (n)*0x100)) /* Direction Register */
#define GPIO_PXDIRS(n)	\
	(JZ4740_GPIO_BASE + (0x64 + (n)*0x100)) /* Direction Set Register */
#define GPIO_PXDIRC(n)	\
	(JZ4740_GPIO_BASE + (0x68 + (n)*0x100)) /* Direction Clear Register */
#define GPIO_PXTRG(n)	\
	(JZ4740_GPIO_BASE + (0x70 + (n)*0x100)) /* Trigger Register */
#define GPIO_PXTRGS(n)	\
	(JZ4740_GPIO_BASE + (0x74 + (n)*0x100)) /* Trigger Set Register */
#define GPIO_PXTRGC(n)	\
	(JZ4740_GPIO_BASE + (0x78 + (n)*0x100)) /* Trigger Set Register */

/* Static Memory Control Register */
#define EMC_SMCR_STRV_BIT	24
#define EMC_SMCR_STRV_MASK	(0x0f << EMC_SMCR_STRV_BIT)
#define EMC_SMCR_TAW_BIT	20
#define EMC_SMCR_TAW_MASK	(0x0f << EMC_SMCR_TAW_BIT)
#define EMC_SMCR_TBP_BIT	16
#define EMC_SMCR_TBP_MASK	(0x0f << EMC_SMCR_TBP_BIT)
#define EMC_SMCR_TAH_BIT	12
#define EMC_SMCR_TAH_MASK	(0x07 << EMC_SMCR_TAH_BIT)
#define EMC_SMCR_TAS_BIT	8
#define EMC_SMCR_TAS_MASK	(0x07 << EMC_SMCR_TAS_BIT)
#define EMC_SMCR_BW_BIT		6
#define EMC_SMCR_BW_MASK	(0x03 << EMC_SMCR_BW_BIT)
  #define EMC_SMCR_BW_8BIT	(0 << EMC_SMCR_BW_BIT)
  #define EMC_SMCR_BW_16BIT	(1 << EMC_SMCR_BW_BIT)
  #define EMC_SMCR_BW_32BIT	(2 << EMC_SMCR_BW_BIT)
#define EMC_SMCR_BCM		(1 << 3)
#define EMC_SMCR_BL_BIT		1
#define EMC_SMCR_BL_MASK	(0x03 << EMC_SMCR_BL_BIT)
  #define EMC_SMCR_BL_4		(0 << EMC_SMCR_BL_BIT)
  #define EMC_SMCR_BL_8		(1 << EMC_SMCR_BL_BIT)
  #define EMC_SMCR_BL_16	(2 << EMC_SMCR_BL_BIT)
  #define EMC_SMCR_BL_32	(3 << EMC_SMCR_BL_BIT)
#define EMC_SMCR_SMT		(1 << 0)

/* Static Memory Bank Addr Config Reg */
#define EMC_SACR_BASE_BIT	8
#define EMC_SACR_BASE_MASK	(0xff << EMC_SACR_BASE_BIT)
#define EMC_SACR_MASK_BIT	0
#define EMC_SACR_MASK_MASK	(0xff << EMC_SACR_MASK_BIT)

/* NAND Flash Control/Status Register */
#define EMC_NFCSR_NFCE4		(1 << 7) /* NAND Flash Enable */
#define EMC_NFCSR_NFE4		(1 << 6) /* NAND Flash FCE# Assertion Enable */
#define EMC_NFCSR_NFCE3		(1 << 5)
#define EMC_NFCSR_NFE3		(1 << 4)
#define EMC_NFCSR_NFCE2		(1 << 3)
#define EMC_NFCSR_NFE2		(1 << 2)
#define EMC_NFCSR_NFCE1		(1 << 1)
#define EMC_NFCSR_NFE1		(1 << 0)

/* NAND Flash ECC Control Register */
#define EMC_NFECR_PRDY		(1 << 4) /* Parity Ready */
#define EMC_NFECR_RS_DECODING	(0 << 3) /* RS is in decoding phase */
#define EMC_NFECR_RS_ENCODING	(1 << 3) /* RS is in encoding phase */
#define EMC_NFECR_HAMMING	(0 << 2) /* Use HAMMING Correction Algorithm */
#define EMC_NFECR_RS		(1 << 2) /* Select RS Correction Algorithm */
#define EMC_NFECR_ERST		(1 << 1) /* ECC Reset */
#define EMC_NFECR_ECCE		(1 << 0) /* ECC Enable */

/* NAND Flash ECC Data Register */
#define EMC_NFECC_ECC2_BIT	16
#define EMC_NFECC_ECC2_MASK	(0xff << EMC_NFECC_ECC2_BIT)
#define EMC_NFECC_ECC1_BIT	8
#define EMC_NFECC_ECC1_MASK	(0xff << EMC_NFECC_ECC1_BIT)
#define EMC_NFECC_ECC0_BIT	0
#define EMC_NFECC_ECC0_MASK	(0xff << EMC_NFECC_ECC0_BIT)

/* NAND Flash Interrupt Status Register */
#define EMC_NFINTS_ERRCNT_BIT	29       /* Error Count */
#define EMC_NFINTS_ERRCNT_MASK	(0x7 << EMC_NFINTS_ERRCNT_BIT)
#define EMC_NFINTS_PADF		(1 << 4) /* Padding Finished */
#define EMC_NFINTS_DECF		(1 << 3) /* Decoding Finished */
#define EMC_NFINTS_ENCF		(1 << 2) /* Encoding Finished */
#define EMC_NFINTS_UNCOR	(1 << 1) /* Uncorrectable Error Occurred */
#define EMC_NFINTS_ERR		(1 << 0) /* Error Occurred */

/* NAND Flash Interrupt Enable Register */
#define EMC_NFINTE_PADFE	(1 << 4) /* Padding Finished Interrupt */
#define EMC_NFINTE_DECFE	(1 << 3) /* Decoding Finished Interrupt */
#define EMC_NFINTE_ENCFE	(1 << 2) /* Encoding Finished Interrupt */
#define EMC_NFINTE_UNCORE	(1 << 1) /* Uncorrectable Error Occurred Intr */
#define EMC_NFINTE_ERRE		(1 << 0) /* Error Occurred Interrupt */

/* NAND Flash RS Error Report Register */
#define EMC_NFERR_INDEX_BIT	16       /* Error Symbol Index */
#define EMC_NFERR_INDEX_MASK	(0x1ff << EMC_NFERR_INDEX_BIT)
#define EMC_NFERR_MASK_BIT	0        /* Error Symbol Value */
#define EMC_NFERR_MASK_MASK	(0x1ff << EMC_NFERR_MASK_BIT)

/* DRAM Control Register */
#define EMC_DMCR_BW_BIT		31
#define EMC_DMCR_BW		(1 << EMC_DMCR_BW_BIT)
#define EMC_DMCR_CA_BIT		26
#define EMC_DMCR_CA_MASK	(0x07 << EMC_DMCR_CA_BIT)
  #define EMC_DMCR_CA_8		(0 << EMC_DMCR_CA_BIT)
  #define EMC_DMCR_CA_9		(1 << EMC_DMCR_CA_BIT)
  #define EMC_DMCR_CA_10	(2 << EMC_DMCR_CA_BIT)
  #define EMC_DMCR_CA_11	(3 << EMC_DMCR_CA_BIT)
  #define EMC_DMCR_CA_12	(4 << EMC_DMCR_CA_BIT)
#define EMC_DMCR_RMODE		(1 << 25)
#define EMC_DMCR_RFSH		(1 << 24)
#define EMC_DMCR_MRSET		(1 << 23)
#define EMC_DMCR_RA_BIT		20
#define EMC_DMCR_RA_MASK	(0x03 << EMC_DMCR_RA_BIT)
  #define EMC_DMCR_RA_11	(0 << EMC_DMCR_RA_BIT)
  #define EMC_DMCR_RA_12	(1 << EMC_DMCR_RA_BIT)
  #define EMC_DMCR_RA_13	(2 << EMC_DMCR_RA_BIT)
#define EMC_DMCR_BA_BIT		19
#define EMC_DMCR_BA		(1 << EMC_DMCR_BA_BIT)
#define EMC_DMCR_PDM		(1 << 18)
#define EMC_DMCR_EPIN		(1 << 17)
#define EMC_DMCR_TRAS_BIT	13
#define EMC_DMCR_TRAS_MASK	(0x07 << EMC_DMCR_TRAS_BIT)
#define EMC_DMCR_RCD_BIT	11
#define EMC_DMCR_RCD_MASK	(0x03 << EMC_DMCR_RCD_BIT)
#define EMC_DMCR_TPC_BIT	8
#define EMC_DMCR_TPC_MASK	(0x07 << EMC_DMCR_TPC_BIT)
#define EMC_DMCR_TRWL_BIT	5
#define EMC_DMCR_TRWL_MASK	(0x03 << EMC_DMCR_TRWL_BIT)
#define EMC_DMCR_TRC_BIT	2
#define EMC_DMCR_TRC_MASK	(0x07 << EMC_DMCR_TRC_BIT)
#define EMC_DMCR_TCL_BIT	0
#define EMC_DMCR_TCL_MASK	(0x03 << EMC_DMCR_TCL_BIT)

/* Refresh Time Control/Status Register */
#define EMC_RTCSR_CMF		(1 << 7)
#define EMC_RTCSR_CKS_BIT	0
#define EMC_RTCSR_CKS_MASK	(0x07 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_DISABLE	(0 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_4	(1 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_16	(2 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_64	(3 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_256	(4 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_1024	(5 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_2048	(6 << EMC_RTCSR_CKS_BIT)
  #define EMC_RTCSR_CKS_4096	(7 << EMC_RTCSR_CKS_BIT)

/* SDRAM Bank Address Configuration Register */
#define EMC_DMAR_BASE_BIT	8
#define EMC_DMAR_BASE_MASK	(0xff << EMC_DMAR_BASE_BIT)
#define EMC_DMAR_MASK_BIT	0
#define EMC_DMAR_MASK_MASK	(0xff << EMC_DMAR_MASK_BIT)

/* Mode Register of SDRAM bank 0 */
#define EMC_SDMR_BM		(1 << 9) /* Write Burst Mode */
#define EMC_SDMR_OM_BIT		7        /* Operating Mode */
#define EMC_SDMR_OM_MASK	(3 << EMC_SDMR_OM_BIT)
  #define EMC_SDMR_OM_NORMAL	(0 << EMC_SDMR_OM_BIT)
#define EMC_SDMR_CAS_BIT	4        /* CAS Latency */
#define EMC_SDMR_CAS_MASK	(7 << EMC_SDMR_CAS_BIT)
  #define EMC_SDMR_CAS_1	(1 << EMC_SDMR_CAS_BIT)
  #define EMC_SDMR_CAS_2	(2 << EMC_SDMR_CAS_BIT)
  #define EMC_SDMR_CAS_3	(3 << EMC_SDMR_CAS_BIT)
#define EMC_SDMR_BT_BIT		3        /* Burst Type */
#define EMC_SDMR_BT_MASK	(1 << EMC_SDMR_BT_BIT)
  #define EMC_SDMR_BT_SEQ	(0 << EMC_SDMR_BT_BIT) /* Sequential */
  #define EMC_SDMR_BT_INT	(1 << EMC_SDMR_BT_BIT) /* Interleave */
#define EMC_SDMR_BL_BIT		0        /* Burst Length */
#define EMC_SDMR_BL_MASK	(7 << EMC_SDMR_BL_BIT)
  #define EMC_SDMR_BL_1		(0 << EMC_SDMR_BL_BIT)
  #define EMC_SDMR_BL_2		(1 << EMC_SDMR_BL_BIT)
  #define EMC_SDMR_BL_4		(2 << EMC_SDMR_BL_BIT)
  #define EMC_SDMR_BL_8		(3 << EMC_SDMR_BL_BIT)

#define EMC_SDMR_CAS2_16BIT \
	(EMC_SDMR_CAS_2 | EMC_SDMR_BT_SEQ | EMC_SDMR_BL_2)
#define EMC_SDMR_CAS2_32BIT \
	(EMC_SDMR_CAS_2 | EMC_SDMR_BT_SEQ | EMC_SDMR_BL_4)
#define EMC_SDMR_CAS3_16BIT \
	(EMC_SDMR_CAS_3 | EMC_SDMR_BT_SEQ | EMC_SDMR_BL_2)
#define EMC_SDMR_CAS3_32BIT \
	(EMC_SDMR_CAS_3 | EMC_SDMR_BT_SEQ | EMC_SDMR_BL_4)

/* RTC Control Register */
#define RTC_RCR_WRDY	(1 << 7)  /* Write Ready Flag */
#define RTC_RCR_HZ	(1 << 6)  /* 1Hz Flag */
#define RTC_RCR_HZIE	(1 << 5)  /* 1Hz Interrupt Enable */
#define RTC_RCR_AF	(1 << 4)  /* Alarm Flag */
#define RTC_RCR_AIE	(1 << 3)  /* Alarm Interrupt Enable */
#define RTC_RCR_AE	(1 << 2)  /* Alarm Enable */
#define RTC_RCR_RTCE	(1 << 0)  /* RTC Enable */

/* RTC Regulator Register */
#define RTC_RGR_LOCK		(1 << 31) /* Lock Bit */
#define RTC_RGR_ADJC_BIT	16
#define RTC_RGR_ADJC_MASK	(0x3ff << RTC_RGR_ADJC_BIT)
#define RTC_RGR_NC1HZ_BIT	0
#define RTC_RGR_NC1HZ_MASK	(0xffff << RTC_RGR_NC1HZ_BIT)

/* Hibernate Control Register */
#define RTC_HCR_PD		(1 << 0)  /* Power Down */

/* Hibernate Wakeup Filter Counter Register */
#define RTC_HWFCR_BIT		5
#define RTC_HWFCR_MASK		(0x7ff << RTC_HWFCR_BIT)

/* Hibernate Reset Counter Register */
#define RTC_HRCR_BIT		5
#define RTC_HRCR_MASK		(0x7f << RTC_HRCR_BIT)

/* Hibernate Wakeup Control Register */
#define RTC_HWCR_EALM		(1 << 0)  /* RTC alarm wakeup enable */

/* Hibernate Wakeup Status Register */
#define RTC_HWRSR_HR		(1 << 5)  /* Hibernate reset */
#define RTC_HWRSR_PPR		(1 << 4)  /* PPR reset */
#define RTC_HWRSR_PIN		(1 << 1)  /* Wakeup pin status bit */
#define RTC_HWRSR_ALM		(1 << 0)  /* RTC alarm status bit */

/* Clock Control Register */
#define CPM_CPCCR_I2CS		(1 << 31)
#define CPM_CPCCR_CLKOEN	(1 << 30)
#define CPM_CPCCR_UCS		(1 << 29)
#define CPM_CPCCR_UDIV_BIT	23
#define CPM_CPCCR_UDIV_MASK	(0x3f << CPM_CPCCR_UDIV_BIT)
#define CPM_CPCCR_CE		(1 << 22)
#define CPM_CPCCR_PCS		(1 << 21)
#define CPM_CPCCR_LDIV_BIT	16
#define CPM_CPCCR_LDIV_MASK	(0x1f << CPM_CPCCR_LDIV_BIT)
#define CPM_CPCCR_MDIV_BIT	12
#define CPM_CPCCR_MDIV_MASK	(0x0f << CPM_CPCCR_MDIV_BIT)
#define CPM_CPCCR_PDIV_BIT	8
#define CPM_CPCCR_PDIV_MASK	(0x0f << CPM_CPCCR_PDIV_BIT)
#define CPM_CPCCR_HDIV_BIT	4
#define CPM_CPCCR_HDIV_MASK	(0x0f << CPM_CPCCR_HDIV_BIT)
#define CPM_CPCCR_CDIV_BIT	0
#define CPM_CPCCR_CDIV_MASK	(0x0f << CPM_CPCCR_CDIV_BIT)

/* I2S Clock Divider Register */
#define CPM_I2SCDR_I2SDIV_BIT	0
#define CPM_I2SCDR_I2SDIV_MASK	(0x1ff << CPM_I2SCDR_I2SDIV_BIT)

/* LCD Pixel Clock Divider Register */
#define CPM_LPCDR_PIXDIV_BIT	0
#define CPM_LPCDR_PIXDIV_MASK	(0x1ff << CPM_LPCDR_PIXDIV_BIT)

/* MSC Clock Divider Register */
#define CPM_MSCCDR_MSCDIV_BIT	0
#define CPM_MSCCDR_MSCDIV_MASK	(0x1f << CPM_MSCCDR_MSCDIV_BIT)

/* PLL Control Register */
#define CPM_CPPCR_PLLM_BIT	23
#define CPM_CPPCR_PLLM_MASK	(0x1ff << CPM_CPPCR_PLLM_BIT)
#define CPM_CPPCR_PLLN_BIT	18
#define CPM_CPPCR_PLLN_MASK	(0x1f << CPM_CPPCR_PLLN_BIT)
#define CPM_CPPCR_PLLOD_BIT	16
#define CPM_CPPCR_PLLOD_MASK	(0x03 << CPM_CPPCR_PLLOD_BIT)
#define CPM_CPPCR_PLLS		(1 << 10)
#define CPM_CPPCR_PLLBP		(1 << 9)
#define CPM_CPPCR_PLLEN		(1 << 8)
#define CPM_CPPCR_PLLST_BIT	0
#define CPM_CPPCR_PLLST_MASK	(0xff << CPM_CPPCR_PLLST_BIT)

/* Low Power Control Register */
#define CPM_LCR_DOZE_DUTY_BIT	3
#define CPM_LCR_DOZE_DUTY_MASK	(0x1f << CPM_LCR_DOZE_DUTY_BIT)
#define CPM_LCR_DOZE_ON		(1 << 2)
#define CPM_LCR_LPM_BIT		0
#define CPM_LCR_LPM_MASK	(0x3 << CPM_LCR_LPM_BIT)
  #define CPM_LCR_LPM_IDLE	(0x0 << CPM_LCR_LPM_BIT)
  #define CPM_LCR_LPM_SLEEP	(0x1 << CPM_LCR_LPM_BIT)

/* Clock Gate Register */
#define CPM_CLKGR_UART1		(1 << 15)
#define CPM_CLKGR_UHC		(1 << 14)
#define CPM_CLKGR_IPU		(1 << 13)
#define CPM_CLKGR_DMAC		(1 << 12)
#define CPM_CLKGR_UDC		(1 << 11)
#define CPM_CLKGR_LCD		(1 << 10)
#define CPM_CLKGR_CIM		(1 << 9)
#define CPM_CLKGR_SADC		(1 << 8)
#define CPM_CLKGR_MSC		(1 << 7)
#define CPM_CLKGR_AIC1		(1 << 6)
#define CPM_CLKGR_AIC2		(1 << 5)
#define CPM_CLKGR_SSI		(1 << 4)
#define CPM_CLKGR_I2C		(1 << 3)
#define CPM_CLKGR_RTC		(1 << 2)
#define CPM_CLKGR_TCU		(1 << 1)
#define CPM_CLKGR_UART0		(1 << 0)

/* Sleep Control Register */
#define CPM_SCR_O1ST_BIT	8
#define CPM_SCR_O1ST_MASK	(0xff << CPM_SCR_O1ST_BIT)
#define CPM_SCR_UDCPHY_ENABLE	(1 << 6)
#define CPM_SCR_USBPHY_DISABLE	(1 << 7)
#define CPM_SCR_OSC_ENABLE	(1 << 4)

/* Hibernate Control Register */
#define CPM_HCR_PD		(1 << 0)

/* Wakeup Filter Counter Register in Hibernate Mode */
#define CPM_HWFCR_TIME_BIT	0
#define CPM_HWFCR_TIME_MASK	(0x3ff << CPM_HWFCR_TIME_BIT)

/* Reset Counter Register in Hibernate Mode */
#define CPM_HRCR_TIME_BIT	0
#define CPM_HRCR_TIME_MASK	(0x7f << CPM_HRCR_TIME_BIT)

/* Wakeup Control Register in Hibernate Mode */
#define CPM_HWCR_WLE_LOW	(0 << 2)
#define CPM_HWCR_WLE_HIGH	(1 << 2)
#define CPM_HWCR_PIN_WAKEUP	(1 << 1)
#define CPM_HWCR_RTC_WAKEUP	(1 << 0)

/* Wakeup Status Register in Hibernate Mode */
#define CPM_HWSR_WSR_PIN	(1 << 1)
#define CPM_HWSR_WSR_RTC	(1 << 0)

/* Reset Status Register */
#define CPM_RSR_HR		(1 << 2)
#define CPM_RSR_WR		(1 << 1)
#define CPM_RSR_PR		(1 << 0)

/* Register definitions */
#define TCU_TCSR_PWM_SD		(1 << 9)
#define TCU_TCSR_PWM_INITL_HIGH	(1 << 8)
#define TCU_TCSR_PWM_EN		(1 << 7)
#define TCU_TCSR_PRESCALE_BIT	3
#define TCU_TCSR_PRESCALE_MASK	(0x7 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE1	(0x0 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE4	(0x1 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE16	(0x2 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE64	(0x3 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE256	(0x4 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_PRESCALE1024	(0x5 << TCU_TCSR_PRESCALE_BIT)
#define TCU_TCSR_EXT_EN		(1 << 2)
#define TCU_TCSR_RTC_EN		(1 << 1)
#define TCU_TCSR_PCK_EN		(1 << 0)

#define TCU_TER_TCEN5		(1 << 5)
#define TCU_TER_TCEN4		(1 << 4)
#define TCU_TER_TCEN3		(1 << 3)
#define TCU_TER_TCEN2		(1 << 2)
#define TCU_TER_TCEN1		(1 << 1)
#define TCU_TER_TCEN0		(1 << 0)

#define TCU_TESR_TCST5		(1 << 5)
#define TCU_TESR_TCST4		(1 << 4)
#define TCU_TESR_TCST3		(1 << 3)
#define TCU_TESR_TCST2		(1 << 2)
#define TCU_TESR_TCST1		(1 << 1)
#define TCU_TESR_TCST0		(1 << 0)

#define TCU_TECR_TCCL5		(1 << 5)
#define TCU_TECR_TCCL4		(1 << 4)
#define TCU_TECR_TCCL3		(1 << 3)
#define TCU_TECR_TCCL2		(1 << 2)
#define TCU_TECR_TCCL1		(1 << 1)
#define TCU_TECR_TCCL0		(1 << 0)

#define TCU_TFR_HFLAG5		(1 << 21)
#define TCU_TFR_HFLAG4		(1 << 20)
#define TCU_TFR_HFLAG3		(1 << 19)
#define TCU_TFR_HFLAG2		(1 << 18)
#define TCU_TFR_HFLAG1		(1 << 17)
#define TCU_TFR_HFLAG0		(1 << 16)
#define TCU_TFR_FFLAG5		(1 << 5)
#define TCU_TFR_FFLAG4		(1 << 4)
#define TCU_TFR_FFLAG3		(1 << 3)
#define TCU_TFR_FFLAG2		(1 << 2)
#define TCU_TFR_FFLAG1		(1 << 1)
#define TCU_TFR_FFLAG0		(1 << 0)

#define TCU_TFSR_HFLAG5		(1 << 21)
#define TCU_TFSR_HFLAG4		(1 << 20)
#define TCU_TFSR_HFLAG3		(1 << 19)
#define TCU_TFSR_HFLAG2		(1 << 18)
#define TCU_TFSR_HFLAG1		(1 << 17)
#define TCU_TFSR_HFLAG0		(1 << 16)
#define TCU_TFSR_FFLAG5		(1 << 5)
#define TCU_TFSR_FFLAG4		(1 << 4)
#define TCU_TFSR_FFLAG3		(1 << 3)
#define TCU_TFSR_FFLAG2		(1 << 2)
#define TCU_TFSR_FFLAG1		(1 << 1)
#define TCU_TFSR_FFLAG0		(1 << 0)

#define TCU_TFCR_HFLAG5		(1 << 21)
#define TCU_TFCR_HFLAG4		(1 << 20)
#define TCU_TFCR_HFLAG3		(1 << 19)
#define TCU_TFCR_HFLAG2		(1 << 18)
#define TCU_TFCR_HFLAG1		(1 << 17)
#define TCU_TFCR_HFLAG0		(1 << 16)
#define TCU_TFCR_FFLAG5		(1 << 5)
#define TCU_TFCR_FFLAG4		(1 << 4)
#define TCU_TFCR_FFLAG3		(1 << 3)
#define TCU_TFCR_FFLAG2		(1 << 2)
#define TCU_TFCR_FFLAG1		(1 << 1)
#define TCU_TFCR_FFLAG0		(1 << 0)

#define TCU_TMR_HMASK5		(1 << 21)
#define TCU_TMR_HMASK4		(1 << 20)
#define TCU_TMR_HMASK3		(1 << 19)
#define TCU_TMR_HMASK2		(1 << 18)
#define TCU_TMR_HMASK1		(1 << 17)
#define TCU_TMR_HMASK0		(1 << 16)
#define TCU_TMR_FMASK5		(1 << 5)
#define TCU_TMR_FMASK4		(1 << 4)
#define TCU_TMR_FMASK3		(1 << 3)
#define TCU_TMR_FMASK2		(1 << 2)
#define TCU_TMR_FMASK1		(1 << 1)
#define TCU_TMR_FMASK0		(1 << 0)

#define TCU_TMSR_HMST5		(1 << 21)
#define TCU_TMSR_HMST4		(1 << 20)
#define TCU_TMSR_HMST3		(1 << 19)
#define TCU_TMSR_HMST2		(1 << 18)
#define TCU_TMSR_HMST1		(1 << 17)
#define TCU_TMSR_HMST0		(1 << 16)
#define TCU_TMSR_FMST5		(1 << 5)
#define TCU_TMSR_FMST4		(1 << 4)
#define TCU_TMSR_FMST3		(1 << 3)
#define TCU_TMSR_FMST2		(1 << 2)
#define TCU_TMSR_FMST1		(1 << 1)
#define TCU_TMSR_FMST0		(1 << 0)

#define TCU_TMCR_HMCL5		(1 << 21)
#define TCU_TMCR_HMCL4		(1 << 20)
#define TCU_TMCR_HMCL3		(1 << 19)
#define TCU_TMCR_HMCL2		(1 << 18)
#define TCU_TMCR_HMCL1		(1 << 17)
#define TCU_TMCR_HMCL0		(1 << 16)
#define TCU_TMCR_FMCL5		(1 << 5)
#define TCU_TMCR_FMCL4		(1 << 4)
#define TCU_TMCR_FMCL3		(1 << 3)
#define TCU_TMCR_FMCL2		(1 << 2)
#define TCU_TMCR_FMCL1		(1 << 1)
#define TCU_TMCR_FMCL0		(1 << 0)

#define TCU_TSR_WDTS		(1 << 16)
#define TCU_TSR_STOP5		(1 << 5)
#define TCU_TSR_STOP4		(1 << 4)
#define TCU_TSR_STOP3		(1 << 3)
#define TCU_TSR_STOP2		(1 << 2)
#define TCU_TSR_STOP1		(1 << 1)
#define TCU_TSR_STOP0		(1 << 0)

#define TCU_TSSR_WDTSS		(1 << 16)
#define TCU_TSSR_STPS5		(1 << 5)
#define TCU_TSSR_STPS4		(1 << 4)
#define TCU_TSSR_STPS3		(1 << 3)
#define TCU_TSSR_STPS2		(1 << 2)
#define TCU_TSSR_STPS1		(1 << 1)
#define TCU_TSSR_STPS0		(1 << 0)

#define TCU_TSSR_WDTSC		(1 << 16)
#define TCU_TSSR_STPC5		(1 << 5)
#define TCU_TSSR_STPC4		(1 << 4)
#define TCU_TSSR_STPC3		(1 << 3)
#define TCU_TSSR_STPC2		(1 << 2)
#define TCU_TSSR_STPC1		(1 << 1)
#define TCU_TSSR_STPC0		(1 << 0)

/* Register definition */
#define WDT_TCSR_PRESCALE_BIT	3
#define WDT_TCSR_PRESCALE_MASK	(0x7 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE1	(0x0 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE4	(0x1 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE16	(0x2 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE64	(0x3 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE256	(0x4 << WDT_TCSR_PRESCALE_BIT)
  #define WDT_TCSR_PRESCALE1024	(0x5 << WDT_TCSR_PRESCALE_BIT)
#define WDT_TCSR_EXT_EN		(1 << 2)
#define WDT_TCSR_RTC_EN		(1 << 1)
#define WDT_TCSR_PCK_EN		(1 << 0)
#define WDT_TCER_TCEN		(1 << 0)

/*
 * Define macros for UART_IER
 * UART Interrupt Enable Register
 */
#define UART_IER_RIE	(1 << 0) /* 0: receive fifo full interrupt disable */
#define UART_IER_TIE	(1 << 1) /* 0: transmit fifo empty interrupt disable */
#define UART_IER_RLIE	(1 << 2) /* 0: receive line status interrupt disable */
#define UART_IER_MIE	(1 << 3) /* 0: modem status interrupt disable */
#define UART_IER_RTIE	(1 << 4) /* 0: receive timeout interrupt disable */

/*
 * Define macros for UART_ISR
 * UART Interrupt Status Register
 */
#define UART_ISR_IP	(1 << 0) /* 0: interrupt is pending 1: no interrupt */
#define UART_ISR_IID	(7 << 1) /* Source of Interrupt */
#define UART_ISR_IID_MSI  (0 << 1) /* Modem status interrupt */
#define UART_ISR_IID_THRI (1 << 1) /* Transmitter holding register empty */
#define UART_ISR_IID_RDI  (2 << 1) /* Receiver data interrupt */
#define UART_ISR_IID_RLSI (3 << 1) /* Receiver line status interrupt */
/* FIFO mode select, set when UART_FCR.FE is set to 1 */
#define UART_ISR_FFMS	(3 << 6)
#define UART_ISR_FFMS_NO_FIFO	(0 << 6)
#define UART_ISR_FFMS_FIFO_MODE	(3 << 6)

/*
 * Define macros for UART_FCR
 * UART FIFO Control Register
 */
#define UART_FCR_FE	(1 << 0)	/* 0: non-FIFO mode  1: FIFO mode */
#define UART_FCR_RFLS	(1 << 1)	/* write 1 to flush receive FIFO */
#define UART_FCR_TFLS	(1 << 2)	/* write 1 to flush transmit FIFO */
#define UART_FCR_DMS	(1 << 3)	/* 0: disable DMA mode */
#define UART_FCR_UUE	(1 << 4)	/* 0: disable UART */
#define UART_FCR_RTRG	(3 << 6)	/* Receive FIFO Data Trigger */
#define UART_FCR_RTRG_1	(0 << 6)
#define UART_FCR_RTRG_4	(1 << 6)
#define UART_FCR_RTRG_8	(2 << 6)
#define UART_FCR_RTRG_15	(3 << 6)

/*
 * Define macros for UART_LCR
 * UART Line Control Register
 */
#define UART_LCR_WLEN	(3 << 0)	/* word length */
#define UART_LCR_WLEN_5	(0 << 0)
#define UART_LCR_WLEN_6	(1 << 0)
#define UART_LCR_WLEN_7	(2 << 0)
#define UART_LCR_WLEN_8	(3 << 0)
#define UART_LCR_STOP	(1 << 2)
	/* 0: 1 stop bit when word length is 5,6,7,8
	   1: 1.5 stop bits when 5; 2 stop bits when 6,7,8 */
#define UART_LCR_STOP_1	(0 << 2)
	/* 0: 1 stop bit when word length is 5,6,7,8
	   1: 1.5 stop bits when 5; 2 stop bits when 6,7,8 */
#define UART_LCR_STOP_2	(1 << 2)
	/* 0: 1 stop bit when word length is 5,6,7,8
	   1: 1.5 stop bits when 5; 2 stop bits when 6,7,8 */

#define UART_LCR_PE	(1 << 3) /* 0: parity disable */
#define UART_LCR_PROE	(1 << 4) /* 0: even parity  1: odd parity */
#define UART_LCR_SPAR	(1 << 5) /* 0: sticky parity disable */
#define UART_LCR_SBRK	(1 << 6) /* write 0 normal, write 1 send break */
/* 0: access UART_RDR/TDR/IER  1: access UART_DLLR/DLHR */
#define UART_LCR_DLAB	(1 << 7)

/*
 * Define macros for UART_LSR
 * UART Line Status Register
 */
/* 0: receive FIFO is empty  1: receive data is ready */
#define UART_LSR_DR	(1 << 0)
/* 0: no overrun error */
#define UART_LSR_ORER	(1 << 1)
/* 0: no parity error */
#define UART_LSR_PER	(1 << 2)
/* 0; no framing error */
#define UART_LSR_FER	(1 << 3)
/* 0: no break detected  1: receive a break signal */
#define UART_LSR_BRK	(1 << 4)
/* 1: transmit FIFO half "empty" */
#define UART_LSR_TDRQ	(1 << 5)
/* 1: transmit FIFO and shift registers empty */
#define UART_LSR_TEMT	(1 << 6)
/* 0: no receive error  1: receive error in FIFO mode */
#define UART_LSR_RFER	(1 << 7)

/*
 * Define macros for UART_MCR
 * UART Modem Control Register
 */
#define UART_MCR_DTR	(1 << 0) /* 0: DTR_ ouput high */
#define UART_MCR_RTS	(1 << 1) /* 0: RTS_ output high */
/* 0: UART_MSR.RI is set to 0 and RI_ input high */
#define UART_MCR_OUT1	(1 << 2)
/* 0: UART_MSR.DCD is set to 0 and DCD_ input high */
#define UART_MCR_OUT2	(1 << 3)
#define UART_MCR_LOOP	(1 << 4) /* 0: normal  1: loopback mode */
#define UART_MCR_MCE	(1 << 7) /* 0: modem function is disable */

/*
 * Define macros for UART_MSR
 * UART Modem Status Register
 */
#define UART_MSR_DCTS	(1 << 0) /* 0: no change on CTS_ since last read */
#define UART_MSR_DDSR	(1 << 1) /* 0: no change on DSR_ since last read */
#define UART_MSR_DRI	(1 << 2) /* 0: no change on RI_  since last read */
#define UART_MSR_DDCD	(1 << 3) /* 0: no change on DCD_ since last read */
#define UART_MSR_CTS	(1 << 4) /* 0: CTS_ pin is high */
#define UART_MSR_DSR	(1 << 5) /* 0: DSR_ pin is high */
#define UART_MSR_RI	(1 << 6) /* 0: RI_ pin is high */
#define UART_MSR_DCD	(1 << 7) /* 0: DCD_ pin is high */

/*
 * Define macros for SIRCR
 * Slow IrDA Control Register
 */
#define SIRCR_TSIRE (1 << 0) /* 0: TX is in UART mode 1: IrDA mode */
#define SIRCR_RSIRE (1 << 1) /* 0: RX is in UART mode 1: IrDA mode */
#define SIRCR_TPWS  (1 << 2) /* 0: transmit 0 pulse width is 3/16 of bit length
				1: 0 pulse width is 1.6us for 115.2Kbps */
#define SIRCR_TXPL  (1 << 3) /* 0: encoder generates a positive pulse for 0 */
#define SIRCR_RXPL  (1 << 4) /* 0: decoder interprets positive pulse as 0 */

/* MSC Clock and Control Register (MSC_STRPCL) */
#define MSC_STRPCL_EXIT_MULTIPLE	(1 << 7)
#define MSC_STRPCL_EXIT_TRANSFER	(1 << 6)
#define MSC_STRPCL_START_READWAIT	(1 << 5)
#define MSC_STRPCL_STOP_READWAIT	(1 << 4)
#define MSC_STRPCL_RESET		(1 << 3)
#define MSC_STRPCL_START_OP		(1 << 2)
#define MSC_STRPCL_CLOCK_CONTROL_BIT	0
#define MSC_STRPCL_CLOCK_CONTROL_MASK	(0x3 << MSC_STRPCL_CLOCK_CONTROL_BIT)
#define MSC_STRPCL_CLOCK_CONTROL_STOP	(0x1 << MSC_STRPCL_CLOCK_CONTROL_BIT)
#define MSC_STRPCL_CLOCK_CONTROL_START	(0x2 << MSC_STRPCL_CLOCK_CONTROL_BIT)

/* MSC Status Register (MSC_STAT) */
#define MSC_STAT_IS_RESETTING		(1 << 15)
#define MSC_STAT_SDIO_INT_ACTIVE	(1 << 14)
#define MSC_STAT_PRG_DONE		(1 << 13)
#define MSC_STAT_DATA_TRAN_DONE		(1 << 12)
#define MSC_STAT_END_CMD_RES		(1 << 11)
#define MSC_STAT_DATA_FIFO_AFULL	(1 << 10)
#define MSC_STAT_IS_READWAIT		(1 << 9)
#define MSC_STAT_CLK_EN			(1 << 8)
#define MSC_STAT_DATA_FIFO_FULL		(1 << 7)
#define MSC_STAT_DATA_FIFO_EMPTY	(1 << 6)
#define MSC_STAT_CRC_RES_ERR		(1 << 5)
#define MSC_STAT_CRC_READ_ERROR		(1 << 4)
#define MSC_STAT_CRC_WRITE_ERROR_BIT	2
#define MSC_STAT_CRC_WRITE_ERROR_MASK	(0x3 << MSC_STAT_CRC_WRITE_ERROR_BIT)
/* No error on transmission of data */
  #define MSC_STAT_CRC_WRITE_ERROR_NO	(0 << MSC_STAT_CRC_WRITE_ERROR_BIT)
/* Card observed erroneous transmission of data */
  #define MSC_STAT_CRC_WRITE_ERROR	(1 << MSC_STAT_CRC_WRITE_ERROR_BIT)
/* No CRC status is sent back */
  #define MSC_STAT_CRC_WRITE_ERROR_NOSTS (2 << MSC_STAT_CRC_WRITE_ERROR_BIT)
#define MSC_STAT_TIME_OUT_RES		(1 << 1)
#define MSC_STAT_TIME_OUT_READ		(1 << 0)

/* MSC Bus Clock Control Register (MSC_CLKRT) */
#define MSC_CLKRT_CLK_RATE_BIT		0
#define MSC_CLKRT_CLK_RATE_MASK		(0x7 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_1  (0x0 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_2  (0x1 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_4  (0x2 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_8  (0x3 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_16  (0x4 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_32  (0x5 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_64  (0x6 << MSC_CLKRT_CLK_RATE_BIT)
  #define MSC_CLKRT_CLK_RATE_DIV_128 (0x7 << MSC_CLKRT_CLK_RATE_BIT)

/* MSC Command Sequence Control Register (MSC_CMDAT) */
#define MSC_CMDAT_IO_ABORT	(1 << 11)
#define MSC_CMDAT_BUS_WIDTH_BIT	9
#define MSC_CMDAT_BUS_WIDTH_MASK (0x3 << MSC_CMDAT_BUS_WIDTH_BIT)
#define MSC_CMDAT_BUS_WIDTH_1BIT (0x0 << MSC_CMDAT_BUS_WIDTH_BIT)
#define MSC_CMDAT_BUS_WIDTH_4BIT (0x2 << MSC_CMDAT_BUS_WIDTH_BIT)
#define MSC_CMDAT_DMA_EN	(1 << 8)
#define MSC_CMDAT_INIT		(1 << 7)
#define MSC_CMDAT_BUSY		(1 << 6)
#define MSC_CMDAT_STREAM_BLOCK	(1 << 5)
#define MSC_CMDAT_WRITE		(1 << 4)
#define MSC_CMDAT_READ		(0 << 4)
#define MSC_CMDAT_DATA_EN	(1 << 3)
#define MSC_CMDAT_RESPONSE_BIT	0
#define MSC_CMDAT_RESPONSE_MASK	(0x7 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_NONE	(0x0 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_R1	(0x1 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_R2	(0x2 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_R3	(0x3 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_R4	(0x4 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_R5	(0x5 << MSC_CMDAT_RESPONSE_BIT)
#define MSC_CMDAT_RESPONSE_R6	(0x6 << MSC_CMDAT_RESPONSE_BIT)

/* MSC Interrupts Mask Register (MSC_IMASK) */
#define MSC_IMASK_SDIO			(1 << 7)
#define MSC_IMASK_TXFIFO_WR_REQ		(1 << 6)
#define MSC_IMASK_RXFIFO_RD_REQ		(1 << 5)
#define MSC_IMASK_END_CMD_RES		(1 << 2)
#define MSC_IMASK_PRG_DONE		(1 << 1)
#define MSC_IMASK_DATA_TRAN_DONE	(1 << 0)

#ifndef __ASSEMBLY__
/* INTC (Interrupt Controller) */
struct jz4740_intc {
	uint32_t isr;		/* interrupt source register */
	uint32_t imr;		/* interrupt mask register */
	uint32_t imsr;		/* interrupt mask set register */
	uint32_t imcr;		/* interrupt mask clear register */
	uint32_t ipr;		/* interrupt pending register */
};

/* RTC */
struct jz4740_rtc {
	uint32_t rcr;		/* rtc control register */
	uint32_t rsr;		/* rtc second register */
	uint32_t rsar;		/* rtc second alarm register */
	uint32_t rgr;		/* rtc regulator register */
	uint32_t hcr;		/* hibernate control register */
	uint32_t hwfcr;		/* hibernate wakeup filter counter reg */
	uint32_t hrcr;		/* hibernate reset counter reg */
	uint32_t hwcr;		/* hibernate wakeup control register */
	uint32_t hwrsr;		/* hibernate wakeup status reg */
	uint32_t hspr;		/* scratch pattern register */
};

/* CPM (Clock reset and Power control Management) */
struct jz4740_cpm {
	uint32_t cpccr; /* 0x00 clock control reg */
	uint32_t lcr;	/* 0x04 low power control reg */
	uint32_t rsr;	/* 0x08 reset status reg */
	uint32_t pad00;
	uint32_t cppcr; /* 0x10 pll control reg */
	uint32_t pad01[3];
	uint32_t clkgr;	/* 0x20 clock gate reg */
	uint32_t scr;	/* 0x24 sleep control reg */
	uint32_t pad02[14];
	uint32_t i2scd; /* 0x60 I2S device clock divider reg */
	uint32_t lpcdr; /* 0x64 LCD pix clock divider reg */
	uint32_t msccdr; /* 0x68 MSC device clock divider reg */
	uint32_t uhccdr; /* 0x6C UHC 48M clock divider reg */
	uint32_t uhcts; /* 0x70 UHC PHY test point reg */
	uint32_t ssicd; /* 0x74 SSI clock divider reg */
};

/* TCU (Timer Counter Unit) */
struct jz4740_tcu {
	uint32_t pad00[4];
	uint32_t ter;	/* 0x10  Timer Counter Enable Register */
	uint32_t tesr;	/* 0x14  Timer Counter Enable Set Register */
	uint32_t tecr;	/* 0x18  Timer Counter Enable Clear Register */
	uint32_t tsr;	/* 0x1C  Timer Stop Register */
	uint32_t tfr;	/* 0x20  Timer Flag Register */
	uint32_t tfsr;	/* 0x24  Timer Flag Set Register */
	uint32_t tfcr;	/* 0x28  Timer Flag Clear Register */
	uint32_t tssr;	/* 0x2C  Timer Stop Set Register */
	uint32_t tmr;	/* 0x30  Timer Mask Register */
	uint32_t tmsr;	/* 0x34  Timer Mask Set Register */
	uint32_t tmcr;	/* 0x38  Timer Mask Clear Register */
	uint32_t tscr;	/* 0x3C  Timer Stop Clear Register */
	uint32_t tdfr0;	/* 0x40  Timer Data Full Register */
	uint32_t tdhr0;	/* 0x44  Timer Data Half Register */
	uint32_t tcnt0;	/* 0x48  Timer Counter Register */
	uint32_t tcsr0;	/* 0x4C  Timer Control Register */
	uint32_t tdfr1;	/* 0x50 */
	uint32_t tdhr1;	/* 0x54 */
	uint32_t tcnt1;	/* 0x58 */
	uint32_t tcsr1;	/* 0x5C */
	uint32_t tdfr2;	/* 0x60 */
	uint32_t tdhr2;	/* 0x64 */
	uint32_t tcnt2;	/* 0x68 */
	uint32_t tcsr2;	/* 0x6C */
	uint32_t tdfr3;	/* 0x70 */
	uint32_t tdhr3;	/* 0x74 */
	uint32_t tcnt3;	/* 0x78 */
	uint32_t tcsr3;	/* 0x7C */
	uint32_t tdfr4;	/* 0x80 */
	uint32_t tdhr4;	/* 0x84 */
	uint32_t tcnt4;	/* 0x88 */
	uint32_t tcsr4;	/* 0x8C */
	uint32_t tdfr5;	/* 0x90 */
	uint32_t tdhr5;	/* 0x94 */
	uint32_t tcnt5;	/* 0x98 */
	uint32_t tcsr5;	/* 0x9C */
};

/* WDT (WatchDog Timer) */
struct jz4740_wdt {
	uint16_t tdr; /* 0x00 watchdog timer data reg*/
	uint16_t pad00;
	uint8_t tcer; /* 0x04 watchdog counter enable reg*/
	uint8_t pad01[3];
	uint16_t tcnt; /* 0x08 watchdog timer counter*/
	uint16_t pad02;
	uint16_t tcsr; /* 0x0C watchdog timer control reg*/
	uint16_t pad03;
};

struct jz4740_uart {
	uint8_t rbr_thr_dllr;
		/* 0x00 R  8b receive buffer reg */
		/* 0x00 W  8b transmit hold reg */
		/* 0x00 RW 8b divisor latch low reg */
	uint8_t pad00[3];
	uint8_t dlhr_ier;
		/* 0x04 RW 8b divisor latch high reg */
		/* 0x04 RW 8b interrupt enable reg */
	uint8_t pad01[3];
	uint8_t iir_fcr;
		/* 0x08 R  8b interrupt identification reg */
		/* 0x08 W  8b FIFO control reg */
	uint8_t pad02[3];
	uint8_t lcr;	/* 0x0C RW 8b Line control reg */
	uint8_t pad03[3];
	uint8_t mcr;	/* 0x10 RW 8b modem control reg */
	uint8_t pad04[3];
	uint8_t lsr;	/* 0x14 R  8b line status reg */
	uint8_t pad05[3];
	uint8_t msr;	/* 0x18 R  8b modem status reg */
	uint8_t pad06[3];
	uint8_t spr;	/* 0x1C RW 8b scratch pad reg */
	uint8_t pad07[3];
	uint8_t isr;	/* 0x20 RW 8b infrared selection reg */
	uint8_t pad08[3];
	uint8_t umr;	/* 0x24 RW 8b */
};

/* MSC */
struct jz4740_msc {
	uint16_t strpcl;/* 0x00 */
	uint32_t stat;	/* 0x04 */
	uint16_t clkrt;	/* 0x08 */
	uint32_t cmdat;	/* 0x0C */
	uint16_t resto;	/* 0x10 */
	uint16_t rdto;	/* 0x14 */
	uint16_t blklen;/* 0x18 */
	uint16_t nob;	/* 0x1C */
	uint16_t snob;	/* 0x20 */
	uint16_t imask;	/* 0x24 */
	uint16_t ireg;	/* 0x28 */
	uint8_t  cmd;	/* 0x2C */
	uint32_t arg;	/* 0x30 */
	uint16_t res;	/* 0x34 */
	uint32_t rxfifo;/* 0x38 */
	uint32_t txfifo;/* 0x3C */
};

/* External Memory Controller */
struct jz4740_emc {
	uint32_t bcr; /* 0x00 BCR */
	uint32_t pad00[3];
	uint32_t smcr[5];
		/* x10 Static Memory Control Register 0 */
		/* x14 Static Memory Control Register 1 */
		/* x18 Static Memory Control Register 2 */
		/* x1c Static Memory Control Register 3 */
		/* x20 Static Memory Control Register 4 */
	uint32_t pad01[3];
	uint32_t sacr[5];
		/* x30 Static Memory Bank 0 Addr Config Reg */
		/* x34 Static Memory Bank 1 Addr Config Reg */
		/* x38 Static Memory Bank 2 Addr Config Reg */
		/* x3c Static Memory Bank 3 Addr Config Reg */
		/* x40 Static Memory Bank 4 Addr Config Reg */
	uint32_t pad02[3];
	uint32_t nfcsr; /* x050 NAND Flash Control/Status Register */

	uint32_t pad03[11];
	uint32_t dmcr; /* x80 DRAM Control Register */
	uint16_t rtcsr; /* x84 Refresh Time Control/Status Register */
	uint16_t pad04;
	uint16_t rtcnt; /* x88 Refresh Timer Counter */
	uint16_t pad05;
	uint16_t rtcor; /* x8c Refresh Time Constant Register */
	uint16_t pad06;
	uint32_t dmar0; /* x90 SDRAM Bank 0 Addr Config Register */
	uint32_t pad07[27];
	uint32_t nfecr; /* x100 NAND Flash ECC Control Register */
	uint32_t nfecc; /* x104 NAND Flash ECC Data Register */
	uint8_t nfpar[12];
		/* x108 NAND Flash RS Parity 0 Register */
		/* x10c NAND Flash RS Parity 1 Register */
		/* x110 NAND Flash RS Parity 2 Register */
	uint32_t nfints; /* x114 NAND Flash Interrupt Status Register */
	uint32_t nfinte; /* x118 NAND Flash Interrupt Enable Register */
	uint32_t nferr[4];
		/* x11c NAND Flash RS Error Report 0 Register */
		/* x120 NAND Flash RS Error Report 1 Register */
		/* x124 NAND Flash RS Error Report 2 Register */
		/* x128 NAND Flash RS Error Report 3 Register */
};

#define __gpio_as_nand()			\
do {						\
	writel(0x02018000, GPIO_PXFUNS(1));	\
	writel(0x02018000, GPIO_PXSELC(1));	\
	writel(0x02018000, GPIO_PXPES(1));	\
	writel(0x30000000, GPIO_PXFUNS(2));	\
	writel(0x30000000, GPIO_PXSELC(2));	\
	writel(0x30000000, GPIO_PXPES(2));	\
	writel(0x40000000, GPIO_PXFUNC(2));	\
	writel(0x40000000, GPIO_PXSELC(2));	\
	writel(0x40000000, GPIO_PXDIRC(2));	\
	writel(0x40000000, GPIO_PXPES(2));	\
	writel(0x00400000, GPIO_PXFUNS(1));	\
	writel(0x00400000, GPIO_PXSELC(1));	\
} while (0)

#define __gpio_as_sdram_16bit_4720()		\
do {						\
	writel(0x5442bfaa, GPIO_PXFUNS(0));	\
	writel(0x5442bfaa, GPIO_PXSELC(0));	\
	writel(0x5442bfaa, GPIO_PXPES(0));	\
	writel(0x81f9ffff, GPIO_PXFUNS(1));	\
	writel(0x81f9ffff, GPIO_PXSELC(1));	\
	writel(0x81f9ffff, GPIO_PXPES(1));	\
	writel(0x01000000, GPIO_PXFUNS(2));	\
	writel(0x01000000, GPIO_PXSELC(2));	\
	writel(0x01000000, GPIO_PXPES(2));	\
} while (0)

#define __gpio_as_lcd_18bit()			\
do {						\
	writel(0x003fffff, GPIO_PXFUNS(2));	\
	writel(0x003fffff, GPIO_PXSELC(2));	\
	writel(0x003fffff, GPIO_PXPES(2));	\
} while (0)

/* MSC_CMD, MSC_CLK, MSC_D0 ~ MSC_D3 */
#define __gpio_as_msc()				\
do {						\
	writel(0x00003f00, GPIO_PXFUNS(3));	\
	writel(0x00003f00, GPIO_PXSELC(3));	\
	writel(0x00003f00, GPIO_PXPES(3));	\
} while (0)

#define __gpio_get_port(p)	(readl(GPIO_PXPIN(p)))

#define __gpio_disable_pull(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	writel((1 << o), GPIO_PXPES(p));	\
} while (0)

#define __gpio_enable_pull(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	writel(1 << (o), GPIO_PXPEC(p));	\
} while (0)

#define __gpio_port_as_output(p, o)		\
do {						\
	writel(1 << (o), GPIO_PXFUNC(p));	\
	writel(1 << (o), GPIO_PXSELC(p));	\
	writel(1 << (o), GPIO_PXDIRS(p));	\
} while (0)

#define __gpio_port_as_input(p, o)		\
do {						\
	writel(1 << (o), GPIO_PXFUNC(p));	\
	writel(1 << (o), GPIO_PXSELC(p));	\
	writel(1 << (o), GPIO_PXDIRC(p));	\
} while (0)

#define __gpio_as_output(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_as_output(p, o);		\
} while (0)

#define __gpio_as_input(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_as_input(p, o);		\
} while (0)

#define __gpio_set_pin(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	writel((1 << o), GPIO_PXDATS(p));	\
} while (0)

#define __gpio_clear_pin(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	writel((1 << o), GPIO_PXDATC(p));	\
} while (0)

#define __gpio_get_pin(n)			\
({						\
	unsigned int p, o, v;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	if (__gpio_get_port(p) & (1 << o))	\
		v = 1;				\
	else					\
		v = 0;				\
	v;					\
})

#define __gpio_as_uart0()			\
do {						\
	writel(0x06000000, GPIO_PXFUNS(3));	\
	writel(0x06000000, GPIO_PXSELS(3));	\
	writel(0x06000000, GPIO_PXPES(3));	\
} while (0)

#define __gpio_jtag_to_uart0()			\
do {						\
	writel(0x80000000, GPIO_PXSELS(2));	\
} while (0)

/* Clock Control Register */
#define __cpm_get_pllm()					\
	((readl(JZ4740_CPM_BASE + 0x10) & CPM_CPPCR_PLLM_MASK)	\
	 >> CPM_CPPCR_PLLM_BIT)
#define __cpm_get_plln()					\
	((readl(JZ4740_CPM_BASE + 0x10) & CPM_CPPCR_PLLN_MASK)	\
	 >> CPM_CPPCR_PLLN_BIT)
#define __cpm_get_pllod()					\
	((readl(JZ4740_CPM_BASE + 0x10) & CPM_CPPCR_PLLOD_MASK)	\
	 >> CPM_CPPCR_PLLOD_BIT)
#define __cpm_get_hdiv()					\
	((readl(JZ4740_CPM_BASE + 0x00) & CPM_CPCCR_HDIV_MASK)	\
	 >> CPM_CPCCR_HDIV_BIT)
#define __cpm_get_pdiv()					\
	((readl(JZ4740_CPM_BASE + 0x00) & CPM_CPCCR_PDIV_MASK)	\
	 >> CPM_CPCCR_PDIV_BIT)
#define __cpm_get_cdiv()					\
	((readl(JZ4740_CPM_BASE + 0x00) & CPM_CPCCR_CDIV_MASK)	\
	 >> CPM_CPCCR_CDIV_BIT)
#define __cpm_get_mdiv()					\
	((readl(JZ4740_CPM_BASE + 0x00) & CPM_CPCCR_MDIV_MASK)	\
	 >> CPM_CPCCR_MDIV_BIT)

static inline unsigned int __cpm_get_pllout(void)
{
	uint32_t m, n, no, pllout;
	uint32_t od[4] = {1, 2, 2, 4};

	struct jz4740_cpm *cpm = (struct jz4740_cpm *)JZ4740_CPM_BASE;
	uint32_t cppcr = readl(&cpm->cppcr);

	if ((cppcr & CPM_CPPCR_PLLEN) && !(cppcr & CPM_CPPCR_PLLBP)) {
		m = __cpm_get_pllm() + 2;
		n = __cpm_get_plln() + 2;
		no = od[__cpm_get_pllod()];
		pllout = (CONFIG_SYS_EXTAL / (n * no)) * m;
	} else
		pllout = CONFIG_SYS_EXTAL;

	return pllout;
}

extern void pll_init(void);
extern void sdram_init(void);
extern void calc_clocks(void);
extern void rtc_init(void);

#endif	/* !__ASSEMBLY__ */
#endif	/* __JZ4740_H__ */
