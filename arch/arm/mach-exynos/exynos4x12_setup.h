/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * refer to exynox4_setup.h
 * Machine Specific Values for EXYNOS4412 based board
 *
 * Copyright (C) 2018 Wang Xinlu <wangkartx@gmail.com>
 */

#ifndef _EXYNOX4x12_SETUP_H
#define _EXYNOX4x12_SETUP_H

#include <config.h>

#ifdef CONFIG_CLK_800_330_165
#define ARM_CLK_800
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_200_200
#define ARM_CLK_1000
#define DRAM_CLK_200
#endif
#ifdef CONFIG_CLK_1000_330_165
#define ARM_CLK_1000
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_400_200
#define ARM_CLK_1000
#define DRAM_CLK_400
#endif
#ifdef CONFIG_CLK_1400_400_200
#define ARM_CLK_1400
#define DRAM_CLK_400
#endif

/* Bus Configuration Register Address */
#define ASYNC_CONFIG        0x10010350

/* CLK_SRC_CPU */
#define MUX_MPLL_USER_SEL_C_FILPLL		0x0
#define MUX_MPLL_USER_SEL_C_FOUTMPLL	    0x1
#define MUX_HPM_SEL_MOUTAPLL		0x0
#define MUX_HPM_SEL_SCLKMPLL		0x1
#define MUX_CORE_SEL_MOUTAPLL		0x0
#define MUX_CORE_SEL_SCLKMPLL		0x1
#define MUX_APLL_SEL_FILPLL		    0x0
#define MUX_APLL_SEL_MOUTMPLLFOUT	0x1
#define CLK_SRC_CPU_RESET   ((MUX_MPLL_USER_SEL_C_FILPLL << 24) \
                    | (MUX_HPM_SEL_MOUTAPLL << 20) \
                    | (MUX_CORE_SEL_MOUTAPLL << 16) \
                    | (MUX_APLL_SEL_FILPLL << 0))
#define CLK_SRC_CPU_VAL     ((MUX_MPLL_USER_SEL_C_FOUTMPLL << 24) \
                    | (MUX_HPM_SEL_MOUTAPLL << 20) \
                    | (MUX_CORE_SEL_MOUTAPLL << 16) \
                    | (MUX_APLL_SEL_MOUTMPLLFOUT << 0))

/* CLK_MUX_STAT_CPU */
#define CLK_MUX_STAT_CPU_RESET  0x01110001
#define CLK_MUX_STAT_CPU_VAL    0x02110002

/* CLK_DIV_CPU0 */
#define CORE2_RATIO     0x00
#define PCLK_DBG_RATIO  0x01
#define PERIPH_RATIO    0x00
#define CORE_RATIO      0x00

/* CLK_DIV_CPU1 */
#define HPM_RATIO       0x00

#ifdef ARM_CLK_800
/* CLK_DIV_CPU0 */
#define APLL_RATIO      0x01
#define ATB_RATIO       0x03
#define COREM1_RATIO    0x04
#define COREM0_RATIO    0x02

/* CLK_DIV_CPU1 */
#define CORES_RATIO     0x03
#define COPY_RATIO      0x03

#elif defined ARM_CLK_1000
#define APLL_RATIO      0x01
#define ATB_RATIO       0x04
#define COREM1_RATIO    0x05
#define COREM0_RATIO    0x02

#define CORES_RATIO     0x04
#define COPY_RATIO      0x04
#endif

#define CLK_DIV_CPU0_VAL    ((CORE2_RATIO << 28) \
                    | (APLL_RATIO << 24) \
                    | (PCLK_DBG_RATIO << 20) \
                    | (ATB_RATIO << 16) \
                    | (PERIPH_RATIO << 12) \
                    | (COREM1_RATIO << 8) \
                    | (COREM0_RATIO << 4) \
                    | (CORE_RATIO << 0))
#define CLK_DIV_CPU1_VAL    ((CORES_RATIO << 8) \
                    | (HPM_RATIO << 4) \
                    | (COPY_RATIO << 0))

/* CLK_SRC_DMC */
#define MUX_G2D_ACP_SEL_MOUTG2D_ACP_0   0x0
#define MUX_G2D_ACP_SEL_MOUTG2D_ACP_1   0x1
#define MUX_G2D_ACP_1_SEL_SCLKEPLL      0x0
#define MUX_G2D_ACP_1_SEL_SCLKAPLL      0x1
#define MUX_PWI_SEL_XXTI		        0x0
#define MUX_PWI_SEL_XUSBXTI		        0x1
#define MUX_PWI_SEL_SCLK_HDMI24M	    0x2
#define MUX_PWI_SEL_SCLK_USBPHY0	    0x3
#define MUX_PWI_SEL_SCLK_USBPHY1	    0x4
#define MUX_PWI_SEL_SCLK_HDMIPHY	    0x5
#define MUX_PWI_SEL_SCLKMPLL		    0x6
#define MUX_PWI_SEL_SCLKEPLL		    0x7
#define MUX_PWI_SEL_SCLKVPLL		    0x8
#define MUX_MPLL_SEL_FILPLL             0x0
#define MUX_MPLL_SEL_MOUTMPLLFOUT       0x1
#define MUX_DPHY_SEL_SCLKMPLL		    0x0
#define MUX_DPHY_SEL_SCLKAPLL		    0x1
#define MUX_DMC_BUS_SEL_SCLKMPLL	    0x0
#define MUX_DMC_BUS_SEL_SCLKAPLL	    0x1
#define CLK_SRC_DMC_VAL		((MUX_PWI_SEL_XUSBXTI << 16) \
                    | (MUX_MPLL_SEL_MOUTMPLLFOUT << 12))

/* CLK_MUX_STAT_DMC */
#define CLK_MUX_STAT_DMC_VAL    0x11102111

/* CLK_DIV_DMC0 */
#define DMCP_RATIO		0x1
#define DMCD_RATIO		0x1
#define DMC_RATIO		0x1
#define DPHY_RATIO		0x1
#define ACP_PCLK_RATIO	0x1
#define ACP_RATIO		0x3
#define CLK_DIV_DMC0_VAL    ((DMCP_RATIO << 20) \
                    | (DMCD_RATIO << 16) \
                    | (DMC_RATIO << 12) \
                    | (DPHY_RATIO << 8) \
                    | (ACP_PCLK_RATIO << 4) \
                    | (ACP_RATIO << 0))

/* CLK_DIV_DMC1 */
#define DPM_RATIO		0x1
#define DVSEM_RATIO		0x1
#define C2C_ACLK_RATIO  0x1
#define PWI_RATIO		0xF
#define C2C_RATIO       0x1
#define G2D_ACP_RATIO   0x3
#define CLK_DIV_DMC1_VAL	((DPM_RATIO << 24) \
				| (DVSEM_RATIO << 16) \
                | (C2C_ACLK_RATIO << 12) \
				| (PWI_RATIO << 8) \
                | (C2C_RATIO << 4) \
                | (G2D_ACP_RATIO << 0))

/* CLK_SRC_TOP0 */
#define MUX_VPLL_SEL_FINPLL     0x0
#define MUX_VPLL_SEL_FOUTVPLL   0x1
#define MUX_EPLL_SEL_FINPLL     0x0
#define MUX_EPLL_SEL_FOUTEPLL   0x1
#define CLK_SRC_TOP0_RESET      0x00000000
#define CLK_SRC_TOP0_VAL    ((MUX_VPLL_SEL_FOUTVPLL << 8) \
                    | (MUX_EPLL_SEL_FOUTEPLL << 4))

/* CLK_MUX_STAT_TOP0 */
#define CLK_MUX_STAT_TOP0_RESET 0x11111111
#define CLK_MUX_STAT_TOP0_VAL   0x11111221

/* CLK_SRC_TOP1 */
#define MUX_ACLK_266_GPS_SUB_SEL_FINPLL 0x0
#define MUX_ACLK_266_GPS_SUB_SEL_DIVOUT_ACLK_266_GPS    0x1
#define MUX_MPLL_USER_SEL_T_FINPLL      0x0
#define MUX_MPLL_USER_SEL_T_SCLKMPLL    0x1
#define MUX_ACLK_400_MCUISP_SEL_SCLKMPLL_USER_T 0x0
#define MUX_ACLK_400_MCUISP_SEL_SCLKAPLL        0x1
#define CLK_SRC_TOP1_RESET      0x00000000
#define CLK_SRC_TOP1_VAL    ((MUX_ACLK_266_GPS_SUB_SEL_DIVOUT_ACLK_266_GPS << 16) \
                    | (MUX_MPLL_USER_SEL_T_SCLKMPLL << 12))

/* CLK_MUX_STAT_TOP1 */
#define CLK_MUX_STAT_TOP1_RESET 0x01111110
#define CLK_MUX_STAT_TOP1_VAL   0x01122110

/* CLK_DIV_TOP */
#define ACLK_400_MCUISP_RATIO   0x1
#define ACLK_266_GPS_RATIO      0x2
#define ONENAND_RATIO           0x1
#define ACLK_133_RATIO          0x5
#define ACLK_160_RATIO          0x4
#define ACLK_100_RATIO          0x7
#define ACLK_200_RATIO          0x4
#define CLK_DIV_TOP_VAL     ((ACLK_400_MCUISP_RATIO << 24) \
                    | (ACLK_266_GPS_RATIO << 20) \
                    | (ONENAND_RATIO << 16) \
                    | (ACLK_133_RATIO << 12) \
                    | (ACLK_160_RATIO << 8) \
                    | (ACLK_100_RATIO << 4) \
                    | (ACLK_200_RATIO << 0))

/* CLK_SRC_LEFTBUS */
#define MUX_MPLL_USER_SEL_L_FILPLL      0x0
#define MUX_MPLL_USER_SEL_L_FOUTMPLL    0x1
#define MUX_GDL_SEL_SCLKMPLL            0x0
#define MUX_GDL_SEL_SCLKAPLL            0x1
#define CLK_SRC_LEFTBUS_VAL     ((MUX_MPLL_USER_SEL_L_FOUTMPLL << 4) \
                        | (MUX_GDL_SEL_SCLKMPLL << 0))

/* CLK_MUX_STAT_LEFTBUS */
#define CLK_MUX_STAT_LEFTBUS_VAL    0x00000021

/* CLK_DIV_LEFTBUS */
#define GPL_RATIO   0x1
#define GDL_RATIO   0x3
#define CLK_DIV_LEFTBUS_VAL     ((GPL_RATIO << 4) \
                        | (GDL_RATIO << 0))

/* CLK_SRC_RIGHTBUS */
#define MUX_MPLL_USER_SEL_R_FINPLL      0x0
#define MUX_MPLL_USER_SEL_R_FOUTMPLL    0x1
#define MUX_GDR_SEL_SCLKMPLL            0x0
#define MUX_GDR_SEL_SCLKAPLL            0x1
#define CLK_SRC_RIGHTBUS_VAL    ((MUX_MPLL_USER_SEL_R_FOUTMPLL << 4) \
                        | (MUX_GDR_SEL_SCLKMPLL << 0))

/* CLK_MUX_STAT_LEFTBUS */
#define CLK_MUX_STAT_RIGHTBUS_VAL    0x00000021

/* CLK_DIV_RIGHTBUS */
#define GPR_RATIO   0x1
#define GDR_RATIO   0x3
#define CLK_DIV_RIGHTBUS_VAL    ((GPR_RATIO << 4) \
                        | (GDR_RATIO << 0))

/* APLL_CON0 */
#ifdef  ARM_CLK_800
#define APLL_MDIV  0x64
#define APLL_PDIV  0x03
#define APLL_SDIV  0x00
#elif defined ARM_CLK_1000
#define APLL_MDIV  0x7D
#define APLL_PDIV  0x03
#define APLL_SDIV  0x00
#endif

#define DISABLE 0
#define ENABLE  1
#define SET_PLL(mdiv, pdiv, sdiv)     ((ENABLE << 31) \
                        | (mdiv << 16) \
                        | (pdiv << 8) \
                        | (sdiv << 0))
#define APLL_CON0_VAL   SET_PLL(APLL_MDIV, APLL_PDIV, APLL_SDIV)

/* APLL_LOCK */
#define APLL_LOCK_VAL   (APLL_PDIV * 270)

/* APLL_CON1 */
#define APLL_DCC_ENB 0x1
#define APLL_AFC_ENB 0x0
#define APLL_LOCK_CON_IN     0x3
#define APLL_LOCK_CON_DLY    0x8
#define APLL_CON1_VAL   ((APLL_DCC_ENB << 21) \
                    | (APLL_AFC_ENB << 20) \
                    | (APLL_LOCK_CON_IN << 12) \
                    | (APLL_LOCK_CON_DLY << 8))

/* MPLL_CON0 */
#define MPLL_MDIV   0x64
#define MPLL_PDIV   0x03
#define MPLL_SDIV   0x00
#define MPLL_CON0_VAL   SET_PLL(MPLL_MDIV, MPLL_PDIV, MPLL_SDIV)

/* MPLL_LOCK */
#define MPLL_LOCK_VAL   (MPLL_PDIV * 2700)

/* MPLL_CON1 */
#define MPLL_DCC_ENB 0x1
#define MPLL_AFC_ENB 0x0
#define MPLL_LOCK_CON_IN     0x3
#define MPLL_LOCK_CON_DLY    0x8
#define MPLL_CON1_VAL   ((MPLL_DCC_ENB << 21) \
                    | (MPLL_AFC_ENB << 20) \
                    | (MPLL_LOCK_CON_IN << 12) \
                    | (MPLL_LOCK_CON_DLY << 8))

/* EPLL_CON0 */
#define EPLL_MDIV   0x40
#define EPLL_PDIV   0x02
#define EPLL_SDIV   0x03
#define EPLL_CON0_VAL   SET_PLL(EPLL_MDIV, EPLL_PDIV, EPLL_SDIV)

/* EPLL_LOCK */
#define EPLL_LOCK_VAL   (EPLL_PDIV * 3000)

/* EPLL_CON1 */
#define EPLL_SEL_PF  0x3
#define EPLL_MRR     0x6
#define EPLL_MFR     0x1
#define EPLL_CON1_VAL   ((EPLL_SEL_PF << 29) \
                    | (EPLL_MRR << 24) \
                    | (EPLL_MFR << 16))

/* EPLL_CON2 */
#define EPLL_DCC_ENB 0x1
#define EPLL_AFC_ENB 0x0
#define EPLL_FVCO_EN 0x1
#define EPLL_LOCK_CON_IN     0x3
#define EPLL_LOCK_OUT_DLY    0x8
#define EPLL_CON2_VAL   (EPLL_DCC_ENB << 7)

/* VPLL_CON0 */
#define VPLL_MDIV   0xAF
#define VPLL_PDIV   0x03
#define VPLL_SDIV   0x02
#define VPLL_CON0_VAL   SET_PLL(VPLL_MDIV, VPLL_PDIV, VPLL_SDIV)

/* VPLL_LOCK */
#define VPLL_LOCK_VAL   (VPLL_PDIV * 3000)

/* VPLL_CON1 */
#define VPLL_SEL_PF  0x3
#define VPLL_MRR     0x6
#define VPLL_MFR     0x1
#define VPLL_CON1_VAL   ((VPLL_SEL_PF << 29) \
                    | (VPLL_MRR << 24) \
                    | (VPLL_MFR << 16))

/* VPLL_CON2 */
#define VPLL_DCC_ENB 0x1
#define VPLL_AFC_ENB 0x0
#define VPLL_FVCO_EN 0x1
#define VPLL_LOCK_CON_IN     0x3
#define VPLL_LOCK_OUT_DLY    0x8
#define VPLL_CON2_VAL   (VPLL_DCC_ENB << 7)

/* CLK_SRC_PERIL0 */
#define UART4_SEL_SCLKMPLL_USER_T   6
#define UART3_SEL_SCLKMPLL_USER_T   6
#define UART2_SEL_SCLKMPLL_USER_T   6
#define UART1_SEL_SCLKMPLL_USER_T   6
#define UART0_SEL_SCLKMPLL_USER_T   6
#define CLK_SRC_PERIL0_VAL  ((UART4_SEL_SCLKMPLL_USER_T << 16) \
                    | (UART3_SEL_SCLKMPLL_USER_T << 12) \
                    | (UART2_SEL_SCLKMPLL_USER_T << 8) \
                    | (UART1_SEL_SCLKMPLL_USER_T << 4) \
                    | (UART0_SEL_SCLKMPLL_USER_T << 0))

/* CLK_DIV_PERIL0 */
#define UART4_RATIO     7
#define UART3_RATIO     7
#define UART2_RATIO     7
#define UART1_RATIO     7
#define UART0_RATIO     7
#define CLK_DIV_PERIL0_VAL  ((UART4_RATIO << 16) \
                    | (UART3_RATIO << 12) \
                    | (UART2_RATIO << 8) \
                    | (UART1_RATIO << 4) \
                    | (UART0_RATIO << 0))

/* CLK_SRC_FSYS */
#define MMC4_SEL        6
#define MMC2_SEL        6
#define CLK_SRC_FSYS_VAL    ((MMC4_SEL << 16) \
                    | (MMC2_SEL << 8))

/* CLK_DIV_FSYS2 */
#define MMC2_RATIO      0xf
#define CLK_DIV_FSYS2_VAL   (MMC2_RATIO << 0)

/* CLK_DIV_FSYS3 */
#define MMC4_RATIO      0xf
#define CLK_DIV_FSYS3_VAL   (MMC4_RATIO << 0)

/* DMC */
#define DIRECT_CMD_NOP	0x07000000
#define DIRECT_CMD_ZQ	0x0a000000
#define DIRECT_CMD_CHIP1_SHIFT	(1 << 20)
#define MEM_TIMINGS_MSR_COUNT	4
#define CTRL_START	(1 << 0)
#define CTRL_DLL_ON	(1 << 1)
#define AREF_EN		(1 << 5)
#define DRV_TYPE	(1 << 6)

struct mem_timings {
	unsigned direct_cmd_msr[MEM_TIMINGS_MSR_COUNT];
	unsigned timingref;
	unsigned timingrow;
	unsigned timingdata;
	unsigned timingpower;
	unsigned zqcontrol;
	unsigned control0;
	unsigned control1;
	unsigned control2;
	unsigned concontrol;
	unsigned prechconfig;
	unsigned memcontrol;
	unsigned memconfig0;
	unsigned memconfig1;
	unsigned dll_resync;
	unsigned dll_on;
};

/* MIU */
/* MIU Config Register Offsets*/
#define APB_SFR_INTERLEAVE_CONF_OFFSET	0x400
#define APB_SFR_ARBRITATION_CONF_OFFSET 0xC00

/* Interleave: 2Bit, Interleave_bit1: 0x15, Interleave_bit0: 0x7 */
#define APB_SFR_INTERLEAVE_CONF_VAL	0x20001507
#define APB_SFR_ARBRITATION_CONF_VAL	0x00000001

#define FORCE_DLL_RESYNC	3
#define DLL_CONTROL_ON		1

#define DIRECT_CMD1	0x00020000
#define DIRECT_CMD2	0x00030000
#define DIRECT_CMD3	0x00010002
#define DIRECT_CMD4	0x00000328

#define CTRL_ZQ_MODE_NOTERM	(0x1 << 0)
#define CTRL_ZQ_START		(0x1 << 1)
#define CTRL_ZQ_DIV		(0 << 4)
#ifdef CONFIG_X4412
#define CTRL_ZQ_MODE_DDS	(0x5 << 8)
#define CTRL_ZQ_MODE_TERM	(0x2 << 11)
#define CTRL_ZQ_FORCE_IMPN	(0x5 << 14)
#define CTRL_ZQ_FORCE_IMPP	(0x2 << 17)
#endif
#ifdef CONFIG_ITOP4412
#define CTRL_ZQ_MODE_DDS     (0x7 << 8)
#define CTRL_ZQ_MODE_TERM	(0x2 << 11)
#define CTRL_ZQ_FORCE_IMPN	(0x5 << 14)
#define CTRL_ZQ_FORCE_IMPP	(0x6 << 17)
#endif
#define CTRL_DCC		(0xE38 << 20)
#define ZQ_CONTROL_VAL		(CTRL_ZQ_MODE_NOTERM | CTRL_ZQ_START\
				| CTRL_ZQ_DIV | CTRL_ZQ_MODE_DDS\
				| CTRL_ZQ_MODE_TERM | CTRL_ZQ_FORCE_IMPN\
				| CTRL_ZQ_FORCE_IMPP | CTRL_DCC)

#define CLK_RATIO		(1 << 1)
#define DIV_PIPE		(1 << 3)
#define AWR_ON			(1 << 4)
#define AREF_DISABLE		(0 << 5)
#ifdef CONFIG_X4412
#define DRV_TYPE_DISABLE	(3 << 6)
#endif
#ifdef CONFIG_ITOP4412
#define DRV_TYPE_DISABLE	(0 << 6)
#endif
#define CHIP0_NOT_EMPTY		(0 << 8)
#define CHIP1_NOT_EMPTY		(0 << 9)
#define DQ_SWAP_DISABLE		(0 << 10)
#define QOS_FAST_DISABLE	(0 << 11)
#define RD_FETCH		(0x3 << 12)
#define TIMEOUT_LEVEL0		(0xFFF << 16)
#define CONCONTROL_VAL		(CLK_RATIO | DIV_PIPE | AWR_ON\
				| AREF_DISABLE | DRV_TYPE_DISABLE\
				| CHIP0_NOT_EMPTY | CHIP1_NOT_EMPTY\
				| DQ_SWAP_DISABLE | QOS_FAST_DISABLE\
				| RD_FETCH | TIMEOUT_LEVEL0)

#define CLK_STOP_DISABLE	(0 << 0)
#define DPWRDN_DISABLE		(0 << 1)
#define DPWRDN_TYPE		(0 << 2)
#define TP_DISABLE		(0 << 4)
#define DSREF_DIABLE		(0 << 5)
#ifdef CONFIG_X4412
#define ADD_LAT_PALL		(0 << 6)
#endif
#ifdef CONFIG_ITOP4412
#define ADD_LAT_PALL		(1 << 6)
#endif
#define MEM_TYPE_DDR3		(0x6 << 8)
#define MEM_WIDTH_32		(0x2 << 12)
#define NUM_CHIP_2		(0 << 16)
#define BL_8			(0x3 << 20)
#define MEMCONTROL_VAL		(CLK_STOP_DISABLE | DPWRDN_DISABLE\
				| DPWRDN_TYPE | TP_DISABLE | DSREF_DIABLE\
				| ADD_LAT_PALL | MEM_TYPE_DDR3 | MEM_WIDTH_32\
				| NUM_CHIP_2 | BL_8)


#define CHIP_BANK_8		(0x3 << 0)
#ifdef CONFIG_X4412
#define CHIP_ROW    	(0x2 << 4)  /* 14 bits */
#endif
#ifdef CONFIG_ITOP4412
#define CHIP_ROW        (0x3 << 4) /* 15 bits */
#endif
#define CHIP_COL_10		(0x3 << 8)
#define CHIP_MAP_INTERLEAVED	(1 << 12)
#define CHIP_MASK		(0xC0 << 16)
#define CHIP0_BASE		(0x40 << 24)
#define CHIP1_BASE		(0x60 << 24)
#define MEMCONFIG0_VAL		(CHIP_BANK_8 | CHIP_ROW | CHIP_COL_10\
				| CHIP_MAP_INTERLEAVED | CHIP_MASK | CHIP0_BASE)
#define MEMCONFIG1_VAL		(CHIP_BANK_8 | CHIP_ROW | CHIP_COL_10\
				| CHIP_MAP_INTERLEAVED | CHIP_MASK | CHIP1_BASE)

#ifdef CONFIG_X4412
#define TP_CNT          (0x64 << 24)
#endif
#ifdef CONFIG_ITOP4412
#define TP_CNT			(0xff << 24)
#endif
#define PRECHCONFIG		TP_CNT

#define CTRL_OFF		(0 << 0)
#define CTRL_DLL_OFF		(0 << 1)
#define CTRL_HALF		(0 << 2)
#define CTRL_DFDQS		(1 << 3)
#define DQS_DELAY		(0 << 4)
#define CTRL_START_POINT	(0x10 << 8)
#define CTRL_INC		(0x10 << 16)
#define CTRL_FORCE		(0x71 << 24)
#define CONTROL0_VAL		(CTRL_OFF | CTRL_DLL_OFF | CTRL_HALF\
				| CTRL_DFDQS | DQS_DELAY | CTRL_START_POINT\
				| CTRL_INC | CTRL_FORCE)

#define CTRL_SHIFTC		(0x6 << 0)
#define CTRL_REF		(8 << 4)
#define CTRL_SHGATE		(1 << 29)
#ifdef CONFIG_X4412
#define TERM_READ_EN		(0 << 30)
#define TERM_WRITE_EN		(0 << 31)
#endif
#ifdef CONFIG_ITOP4412
#define TERM_READ_EN		(1 << 30)
#define TERM_WRITE_EN		(1 << 31)
#endif
#define CONTROL1_VAL		(CTRL_SHIFTC | CTRL_REF | CTRL_SHGATE\
				| TERM_READ_EN | TERM_WRITE_EN)

#define CONTROL2_VAL		0x00000000

#ifdef CONFIG_EXYNOS4x12
#define TIMINGREF_VAL		0x000000BB
#define	TIMINGDATA_VAL		0x46400506
#define	TIMINGPOWER_VAL		0x52000A3C
#ifdef CONFIG_X4412
#define TIMINGROW_VAL		0x7846654f
#endif
#ifdef CONFIG_ITOP4412
#define TIMINGROW_VAL		0x4046654f
#endif
#endif

#endif
