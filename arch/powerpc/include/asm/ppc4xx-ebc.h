/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PPC4xx_EBC_H_
#define _PPC4xx_EBC_H_

/*
 * Currently there are two register layout versions for the IBM EBC core
 * used on 4xx PPC's. The following grouping lists the first layout.
 * Within this group there is a slight variation concerning the bit field
 * position of the EMPL and EMPH fields:
 */
#if defined(CONFIG_405GP) || \
    defined(CONFIG_405EP) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define CONFIG_EBC_PPC4xx_IBM_VER1
#if defined(CONFIG_405GP) || \
    defined(CONFIG_405EP)
#define EBC_CFG_EMPH_POS	8
#define EBC_CFG_EMPL_POS	6
#else
#define EBC_CFG_EMPH_POS	6
#define EBC_CFG_EMPL_POS	8
#endif
#endif

/*
 * Define the max number of EBC banks (chip selects)
 */
#if defined(CONFIG_405GP) || \
    defined(CONFIG_405EZ) || \
    defined(CONFIG_440GP) || defined(CONFIG_440GX)
#define EBC_NUM_BANKS	8
#endif

#if defined(CONFIG_405EP)
#define EBC_NUM_BANKS	5
#endif

#if defined(CONFIG_405EX) || \
    defined(CONFIG_460SX)
#define EBC_NUM_BANKS	4
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define EBC_NUM_BANKS	6
#endif

#if defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_APM821XX)
#define EBC_NUM_BANKS	3
#endif

/* Bank Configuration Register */
#define EBC_BXCR(n)		(n)
#define EBC_BXCR_BANK_SIZE(n)	(0x100000 << (((n) & EBC_BXCR_BS_MASK) >> 17))

#define	EBC_BXCR_BAS_MASK	PPC_REG_VAL(11, 0xFFF)
#define EBC_BXCR_BAS_ENCODE(n)	(((static_cast(u32, n)) & EBC_BXCR_BAS_MASK))
#define EBC_BXCR_BS_MASK	PPC_REG_VAL(14, 0x7)
#define EBC_BXCR_BS_1MB		PPC_REG_VAL(14, 0x0)
#define EBC_BXCR_BS_2MB		PPC_REG_VAL(14, 0x1)
#define EBC_BXCR_BS_4MB		PPC_REG_VAL(14, 0x2)
#define EBC_BXCR_BS_8MB		PPC_REG_VAL(14, 0x3)
#define EBC_BXCR_BS_16MB	PPC_REG_VAL(14, 0x4)
#define EBC_BXCR_BS_32MB	PPC_REG_VAL(14, 0x5)
#define EBC_BXCR_BS_64MB	PPC_REG_VAL(14, 0x6)
#define EBC_BXCR_BS_128MB	PPC_REG_VAL(14, 0x7)
#define EBC_BXCR_BU_MASK	PPC_REG_VAL(16, 0x3)
#define	EBC_BXCR_BU_NONE	PPC_REG_VAL(16, 0x0)
#define EBC_BXCR_BU_R		PPC_REG_VAL(16, 0x1)
#define EBC_BXCR_BU_W		PPC_REG_VAL(16, 0x2)
#define EBC_BXCR_BU_RW		PPC_REG_VAL(16, 0x3)
#define EBC_BXCR_BW_MASK	PPC_REG_VAL(18, 0x3)
#define EBC_BXCR_BW_8BIT	PPC_REG_VAL(18, 0x0)
#define EBC_BXCR_BW_16BIT	PPC_REG_VAL(18, 0x1)
#if defined(CONFIG_EBC_PPC4xx_IBM_VER1)
#define EBC_BXCR_BW_32BIT	PPC_REG_VAL(18, 0x2)
#else
#define EBC_BXCR_BW_32BIT	PPC_REG_VAL(18, 0x3)
#endif

/* Bank Access Parameter Register */
#define EBC_BXAP_BME_ENABLED	PPC_REG_VAL(0, 0x1)
#define EBC_BXAP_BME_DISABLED	PPC_REG_VAL(0, 0x0)
#define EBC_BXAP_TWT_ENCODE(n)	PPC_REG_VAL(8, (static_cast(u32, n)) & 0xFF)
#define	EBC_BXAP_FWT_ENCODE(n)	PPC_REG_VAL(5, (static_cast(u32, n)) & 0x1F)
#define	EBC_BXAP_BWT_ENCODE(n)	PPC_REG_VAL(8, (static_cast(u32, n)) & 0x7)
#define EBC_BXAP_BCE_DISABLE	PPC_REG_VAL(9, 0x0)
#define EBC_BXAP_BCE_ENABLE	PPC_REG_VAL(9, 0x1)
#define EBC_BXAP_BCT_MASK	PPC_REG_VAL(11, 0x3)
#define EBC_BXAP_BCT_2TRANS	PPC_REG_VAL(11, 0x0)
#define EBC_BXAP_BCT_4TRANS	PPC_REG_VAL(11, 0x1)
#define EBC_BXAP_BCT_8TRANS	PPC_REG_VAL(11, 0x2)
#define EBC_BXAP_BCT_16TRANS	PPC_REG_VAL(11, 0x3)
#define EBC_BXAP_CSN_ENCODE(n)	PPC_REG_VAL(13, (static_cast(u32, n)) & 0x3)
#define EBC_BXAP_OEN_ENCODE(n)	PPC_REG_VAL(15, (static_cast(u32, n)) & 0x3)
#define EBC_BXAP_WBN_ENCODE(n)	PPC_REG_VAL(17, (static_cast(u32, n)) & 0x3)
#define EBC_BXAP_WBF_ENCODE(n)	PPC_REG_VAL(19, (static_cast(u32, n)) & 0x3)
#define EBC_BXAP_TH_ENCODE(n)	PPC_REG_VAL(22, (static_cast(u32, n)) & 0x7)
#define EBC_BXAP_RE_ENABLED	PPC_REG_VAL(23, 0x1)
#define EBC_BXAP_RE_DISABLED	PPC_REG_VAL(23, 0x0)
#define EBC_BXAP_SOR_DELAYED	PPC_REG_VAL(24, 0x0)
#define EBC_BXAP_SOR_NONDELAYED	PPC_REG_VAL(24, 0x1)
#define EBC_BXAP_BEM_WRITEONLY	PPC_REG_VAL(25, 0x0)
#define EBC_BXAP_BEM_RW		PPC_REG_VAL(25, 0x1)
#define EBC_BXAP_PEN_DISABLED	PPC_REG_VAL(26, 0x0)
#define EBC_BXAP_PEN_ENABLED	PPC_REG_VAL(26, 0x1)

/* Common fields in EBC0_CFG register */
#define EBC_CFG_PTD_MASK	PPC_REG_VAL(1, 0x1)
#define EBC_CFG_PTD_ENABLE	PPC_REG_VAL(1, 0x0)
#define EBC_CFG_PTD_DISABLE	PPC_REG_VAL(1, 0x1)
#define EBC_CFG_RTC_MASK	PPC_REG_VAL(4, 0x7)
#define EBC_CFG_RTC_16PERCLK	PPC_REG_VAL(4, 0x0)
#define EBC_CFG_RTC_32PERCLK	PPC_REG_VAL(4, 0x1)
#define EBC_CFG_RTC_64PERCLK	PPC_REG_VAL(4, 0x2)
#define EBC_CFG_RTC_128PERCLK	PPC_REG_VAL(4, 0x3)
#define EBC_CFG_RTC_256PERCLK	PPC_REG_VAL(4, 0x4)
#define EBC_CFG_RTC_512PERCLK	PPC_REG_VAL(4, 0x5)
#define EBC_CFG_RTC_1024PERCLK	PPC_REG_VAL(4, 0x6)
#define EBC_CFG_RTC_2048PERCLK	PPC_REG_VAL(4, 0x7)
#define EBC_CFG_PME_MASK	PPC_REG_VAL(14, 0x1)
#define EBC_CFG_PME_DISABLE	PPC_REG_VAL(14, 0x0)
#define EBC_CFG_PME_ENABLE	PPC_REG_VAL(14, 0x1)
#define EBC_CFG_PMT_MASK	PPC_REG_VAL(19, 0x1F)
#define EBC_CFG_PMT_ENCODE(n)	PPC_REG_VAL(19, (static_cast(u32, n)) & 0x1F)

/* Now the two versions of the other bits */
#if defined(CONFIG_EBC_PPC4xx_IBM_VER1)
#define EBC_CFG_EBTC_MASK	PPC_REG_VAL(0, 0x1)
#define EBC_CFG_EBTC_HI		PPC_REG_VAL(0, 0x0)
#define EBC_CFG_EBTC_DRIVEN	PPC_REG_VAL(0, 0x1)
#define EBC_CFG_EMPH_MASK	PPC_REG_VAL(EBC_CFG_EMPH_POS, 0x3)
#define EBC_CFG_EMPH_ENCODE(n)	PPC_REG_VAL(EBC_CFG_EMPH_POS, \
						(static_cast(u32, n)) & 0x3)
#define EBC_CFG_EMPL_MASK	PPC_REG_VAL(EBC_CFG_EMPL_POS, 0x3)
#define EBC_CFG_EMPL_ENCODE(n)	PPC_REG_VAL(EBC_CFG_EMPH_POS, \
						(static_cast(u32, n)) & 0x3)
#define EBC_CFG_CSTC_MASK	PPC_REG_VAL(9, 0x1)
#define EBC_CFG_CSTC_HI		PPC_REG_VAL(9, 0x0)
#define EBC_CFG_CSTC_DRIVEN	PPC_REG_VAL(9, 0x1)
#define EBC_CFG_BPR_MASK	PPC_REG_VAL(11, 0x3)
#define EBC_CFG_BPR_1DW		PPC_REG_VAL(11, 0x0)
#define EBC_CFG_BPR_2DW		PPC_REG_VAL(11, 0x1)
#define EBC_CFG_BPR_4DW		PPC_REG_VAL(11, 0x2)
#define EBC_CFG_EMS_MASK	PPC_REG_VAL(13, 0x3)
#define EBC_CFG_EMS_8BIT	PPC_REG_VAL(13, 0x0)
#define EBC_CFG_EMS_16BIT	PPC_REG_VAL(13, 0x1)
#define EBC_CFG_EMS_32BIT	PPC_REG_VAL(13, 0x2)
#else
#define EBC_CFG_LE_MASK		PPC_REG_VAL(0, 0x1)
#define EBC_CFG_LE_UNLOCK	PPC_REG_VAL(0, 0x0)
#define EBC_CFG_LE_LOCK		PPC_REG_VAL(0, 0x1)
#define EBC_CFG_ATC_MASK	PPC_REG_VAL(5, 0x1)
#define EBC_CFG_ATC_HI		PPC_REG_VAL(5, 0x0)
#define EBC_CFG_ATC_PREVIOUS	PPC_REG_VAL(5, 0x1)
#define EBC_CFG_DTC_MASK	PPC_REG_VAL(6, 0x1)
#define EBC_CFG_DTC_HI		PPC_REG_VAL(6, 0x0)
#define EBC_CFG_DTC_PREVIOUS	PPC_REG_VAL(6, 0x1)
#define EBC_CFG_CTC_MASK	PPC_REG_VAL(7, 0x1)
#define EBC_CFG_CTC_HI		PPC_REG_VAL(7, 0x0)
#define EBC_CFG_CTC_PREVIOUS	PPC_REG_VAL(7, 0x1)
#define EBC_CFG_OEO_MASK	PPC_REG_VAL(8, 0x1)
#define EBC_CFG_OEO_HI		PPC_REG_VAL(8, 0x0)
#define EBC_CFG_OEO_PREVIOUS	PPC_REG_VAL(8, 0x1)
#define EBC_CFG_EMC_MASK	PPC_REG_VAL(9, 0x1)
#define EBC_CFG_EMC_NONDEFAULT	PPC_REG_VAL(9, 0x0)
#define EBC_CFG_EMC_DEFAULT	PPC_REG_VAL(9, 0x1)
#define EBC_CFG_PR_MASK		PPC_REG_VAL(21, 0x3)
#define EBC_CFG_PR_16		PPC_REG_VAL(21, 0x0)
#define EBC_CFG_PR_32		PPC_REG_VAL(21, 0x1)
#define EBC_CFG_PR_64		PPC_REG_VAL(21, 0x2)
#define EBC_CFG_PR_128		PPC_REG_VAL(21, 0x3)
#endif

#endif /* _PPC4xx_EBC_H_ */
