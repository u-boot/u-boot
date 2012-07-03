/*
 * Machine Specific Values for SMDK5250 board based on EXYNOS5
 *
 * Copyright (C) 2012 Samsung Electronics
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _SMDK5250_SETUP_H
#define _SMDK5250_SETUP_H

#include <config.h>
#include <asm/arch/dmc.h>

/* TZPC : Register Offsets */
#define TZPC0_BASE		0x10100000
#define TZPC1_BASE		0x10110000
#define TZPC2_BASE		0x10120000
#define TZPC3_BASE		0x10130000
#define TZPC4_BASE		0x10140000
#define TZPC5_BASE		0x10150000
#define TZPC6_BASE		0x10160000
#define TZPC7_BASE		0x10170000
#define TZPC8_BASE		0x10180000
#define TZPC9_BASE		0x10190000

/* APLL_CON1	*/
#define APLL_CON1_VAL	(0x00203800)

/* MPLL_CON1	*/
#define MPLL_CON1_VAL   (0x00203800)

/* CPLL_CON1	*/
#define CPLL_CON1_VAL	(0x00203800)

/* GPLL_CON1	*/
#define GPLL_CON1_VAL	(0x00203800)

/* EPLL_CON1, CON2	*/
#define EPLL_CON1_VAL	0x00000000
#define EPLL_CON2_VAL	0x00000080

/* VPLL_CON1, CON2	*/
#define VPLL_CON1_VAL	0x00000000
#define VPLL_CON2_VAL	0x00000080

/* BPLL_CON1	*/
#define BPLL_CON1_VAL	0x00203800

/* Set PLL */
#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)

/* CLK_SRC_CPU	*/
/* 0 = MOUTAPLL,  1 = SCLKMPLL	*/
#define MUX_HPM_SEL             0
#define MUX_CPU_SEL             0
#define MUX_APLL_SEL            1

#define CLK_SRC_CPU_VAL		((MUX_HPM_SEL << 20)    \
				| (MUX_CPU_SEL << 16)  \
				| (MUX_APLL_SEL))

/* MEMCONTROL register bit fields */
#define DMC_MEMCONTROL_CLK_STOP_DISABLE	(0 << 0)
#define DMC_MEMCONTROL_DPWRDN_DISABLE	(0 << 1)
#define DMC_MEMCONTROL_DPWRDN_ACTIVE_PRECHARGE	(0 << 2)
#define DMC_MEMCONTROL_TP_DISABLE	(0 << 4)
#define DMC_MEMCONTROL_DSREF_DISABLE	(0 << 5)
#define DMC_MEMCONTROL_DSREF_ENABLE	(1 << 5)
#define DMC_MEMCONTROL_ADD_LAT_PALL_CYCLE(x)    (x << 6)

#define DMC_MEMCONTROL_MEM_TYPE_LPDDR3  (7 << 8)
#define DMC_MEMCONTROL_MEM_TYPE_DDR3    (6 << 8)
#define DMC_MEMCONTROL_MEM_TYPE_LPDDR2  (5 << 8)

#define DMC_MEMCONTROL_MEM_WIDTH_32BIT  (2 << 12)

#define DMC_MEMCONTROL_NUM_CHIP_1       (0 << 16)
#define DMC_MEMCONTROL_NUM_CHIP_2       (1 << 16)

#define DMC_MEMCONTROL_BL_8             (3 << 20)
#define DMC_MEMCONTROL_BL_4             (2 << 20)

#define DMC_MEMCONTROL_PZQ_DISABLE      (0 << 24)

#define DMC_MEMCONTROL_MRR_BYTE_7_0     (0 << 25)
#define DMC_MEMCONTROL_MRR_BYTE_15_8    (1 << 25)
#define DMC_MEMCONTROL_MRR_BYTE_23_16   (2 << 25)
#define DMC_MEMCONTROL_MRR_BYTE_31_24   (3 << 25)

/* MEMCONFIG0 register bit fields */
#define DMC_MEMCONFIGx_CHIP_MAP_INTERLEAVED     (1 << 12)
#define DMC_MEMCONFIGx_CHIP_COL_10              (3 << 8)
#define DMC_MEMCONFIGx_CHIP_ROW_14              (2 << 4)
#define DMC_MEMCONFIGx_CHIP_ROW_15              (3 << 4)
#define DMC_MEMCONFIGx_CHIP_BANK_8              (3 << 0)

#define DMC_MEMBASECONFIGx_CHIP_BASE(x)         (x << 16)
#define DMC_MEMBASECONFIGx_CHIP_MASK(x)         (x << 0)
#define DMC_MEMBASECONFIG_VAL(x)        (       \
	DMC_MEMBASECONFIGx_CHIP_BASE(x) |       \
	DMC_MEMBASECONFIGx_CHIP_MASK(0x780)     \
)

#define DMC_MEMBASECONFIG0_VAL  DMC_MEMBASECONFIG_VAL(0x40)
#define DMC_MEMBASECONFIG1_VAL  DMC_MEMBASECONFIG_VAL(0x80)

#define DMC_PRECHCONFIG_VAL             0xFF000000
#define DMC_PWRDNCONFIG_VAL             0xFFFF00FF

#define DMC_CONCONTROL_RESET_VAL	0x0FFF0000
#define DFI_INIT_START		(1 << 28)
#define EMPTY			(1 << 8)
#define AREF_EN			(1 << 5)

#define DFI_INIT_COMPLETE_CHO	(1 << 2)
#define DFI_INIT_COMPLETE_CH1	(1 << 3)

#define RDLVL_COMPLETE_CHO	(1 << 14)
#define RDLVL_COMPLETE_CH1	(1 << 15)

#define CLK_STOP_EN	(1 << 0)
#define DPWRDN_EN	(1 << 1)
#define DSREF_EN	(1 << 5)

/* COJCONTROL register bit fields */
#define DMC_CONCONTROL_IO_PD_CON_DISABLE	(0 << 3)
#define DMC_CONCONTROL_AREF_EN_DISABLE		(0 << 5)
#define DMC_CONCONTROL_EMPTY_DISABLE		(0 << 8)
#define DMC_CONCONTROL_EMPTY_ENABLE		(1 << 8)
#define DMC_CONCONTROL_RD_FETCH_DISABLE		(0x0 << 12)
#define DMC_CONCONTROL_TIMEOUT_LEVEL0		(0xFFF << 16)
#define DMC_CONCONTROL_DFI_INIT_START_DISABLE	(0 << 28)

/* CLK_DIV_CPU0_VAL */
#define CLK_DIV_CPU0_VAL	((ARM2_RATIO << 28)             \
				| (APLL_RATIO << 24)            \
				| (PCLK_DBG_RATIO << 20)        \
				| (ATB_RATIO << 16)             \
				| (PERIPH_RATIO << 12)          \
				| (ACP_RATIO << 8)              \
				| (CPUD_RATIO << 4)             \
				| (ARM_RATIO))


/* CLK_FSYS */
#define CLK_SRC_FSYS0_VAL              0x66666
#define CLK_DIV_FSYS0_VAL	       0x0BB00000

/* CLK_DIV_CPU1	*/
#define HPM_RATIO               0x2
#define COPY_RATIO              0x0

/* CLK_DIV_CPU1 = 0x00000003 */
#define CLK_DIV_CPU1_VAL        ((HPM_RATIO << 4)		\
				| (COPY_RATIO))

/* CLK_SRC_CORE0 */
#define CLK_SRC_CORE0_VAL       0x00000000

/* CLK_SRC_CORE1 */
#define CLK_SRC_CORE1_VAL       0x100

/* CLK_DIV_CORE0 */
#define CLK_DIV_CORE0_VAL       0x00120000

/* CLK_DIV_CORE1 */
#define CLK_DIV_CORE1_VAL       0x07070700

/* CLK_DIV_SYSRGT */
#define CLK_DIV_SYSRGT_VAL      0x00000111

/* CLK_DIV_ACP */
#define CLK_DIV_ACP_VAL         0x12

/* CLK_DIV_SYSLFT */
#define CLK_DIV_SYSLFT_VAL      0x00000311

/* CLK_SRC_CDREX */
#define CLK_SRC_CDREX_VAL       0x1

/* CLK_DIV_CDREX */
#define MCLK_CDREX2_RATIO       0x0
#define ACLK_EFCON_RATIO        0x1
#define MCLK_DPHY_RATIO		0x1
#define MCLK_CDREX_RATIO	0x1
#define ACLK_C2C_200_RATIO	0x1
#define C2C_CLK_400_RATIO	0x1
#define PCLK_CDREX_RATIO	0x1
#define ACLK_CDREX_RATIO	0x1

#define CLK_DIV_CDREX_VAL	((MCLK_DPHY_RATIO << 24)        \
				| (C2C_CLK_400_RATIO << 6)	\
				| (PCLK_CDREX_RATIO << 4)	\
				| (ACLK_CDREX_RATIO))

/* CLK_SRC_TOP0	*/
#define MUX_ACLK_300_GSCL_SEL           0x0
#define MUX_ACLK_300_GSCL_MID_SEL       0x0
#define MUX_ACLK_400_G3D_MID_SEL        0x0
#define MUX_ACLK_333_SEL	        0x0
#define MUX_ACLK_300_DISP1_SEL	        0x0
#define MUX_ACLK_300_DISP1_MID_SEL      0x0
#define MUX_ACLK_200_SEL	        0x0
#define MUX_ACLK_166_SEL	        0x0
#define CLK_SRC_TOP0_VAL	((MUX_ACLK_300_GSCL_SEL  << 25)		\
				| (MUX_ACLK_300_GSCL_MID_SEL << 24)	\
				| (MUX_ACLK_400_G3D_MID_SEL << 20)	\
				| (MUX_ACLK_333_SEL << 16)		\
				| (MUX_ACLK_300_DISP1_SEL << 15)	\
				| (MUX_ACLK_300_DISP1_MID_SEL << 14)	\
				| (MUX_ACLK_200_SEL << 12)		\
				| (MUX_ACLK_166_SEL << 8))

/* CLK_SRC_TOP1	*/
#define MUX_ACLK_400_G3D_SEL            0x1
#define MUX_ACLK_400_ISP_SEL            0x0
#define MUX_ACLK_400_IOP_SEL            0x0
#define MUX_ACLK_MIPI_HSI_TXBASE_SEL    0x0
#define MUX_ACLK_300_GSCL_MID1_SEL      0x0
#define MUX_ACLK_300_DISP1_MID1_SEL     0x0
#define CLK_SRC_TOP1_VAL	((MUX_ACLK_400_G3D_SEL << 28)           \
				|(MUX_ACLK_400_ISP_SEL << 24)           \
				|(MUX_ACLK_400_IOP_SEL << 20)           \
				|(MUX_ACLK_MIPI_HSI_TXBASE_SEL << 16)   \
				|(MUX_ACLK_300_GSCL_MID1_SEL << 12)     \
				|(MUX_ACLK_300_DISP1_MID1_SEL << 8))

/* CLK_SRC_TOP2 */
#define MUX_GPLL_SEL                    0x1
#define MUX_BPLL_USER_SEL               0x0
#define MUX_MPLL_USER_SEL               0x0
#define MUX_VPLL_SEL                    0x1
#define MUX_EPLL_SEL                    0x1
#define MUX_CPLL_SEL                    0x1
#define VPLLSRC_SEL                     0x0
#define CLK_SRC_TOP2_VAL	((MUX_GPLL_SEL << 28)		\
				| (MUX_BPLL_USER_SEL << 24)	\
				| (MUX_MPLL_USER_SEL << 20)	\
				| (MUX_VPLL_SEL << 16)	        \
				| (MUX_EPLL_SEL << 12)	        \
				| (MUX_CPLL_SEL << 8)           \
				| (VPLLSRC_SEL))
/* CLK_SRC_TOP3 */
#define MUX_ACLK_333_SUB_SEL            0x1
#define MUX_ACLK_400_SUB_SEL            0x1
#define MUX_ACLK_266_ISP_SUB_SEL        0x1
#define MUX_ACLK_266_GPS_SUB_SEL        0x0
#define MUX_ACLK_300_GSCL_SUB_SEL       0x1
#define MUX_ACLK_266_GSCL_SUB_SEL       0x1
#define MUX_ACLK_300_DISP1_SUB_SEL      0x1
#define MUX_ACLK_200_DISP1_SUB_SEL      0x1
#define CLK_SRC_TOP3_VAL	((MUX_ACLK_333_SUB_SEL << 24)	        \
				| (MUX_ACLK_400_SUB_SEL << 20)	        \
				| (MUX_ACLK_266_ISP_SUB_SEL << 16)	\
				| (MUX_ACLK_266_GPS_SUB_SEL << 12)      \
				| (MUX_ACLK_300_GSCL_SUB_SEL << 10)     \
				| (MUX_ACLK_266_GSCL_SUB_SEL << 8)      \
				| (MUX_ACLK_300_DISP1_SUB_SEL << 6)     \
				| (MUX_ACLK_200_DISP1_SUB_SEL << 4))

/* CLK_DIV_TOP0	*/
#define ACLK_300_DISP1_RATIO	0x2
#define ACLK_400_G3D_RATIO	0x0
#define ACLK_333_RATIO		0x0
#define ACLK_266_RATIO		0x2
#define ACLK_200_RATIO		0x3
#define ACLK_166_RATIO		0x1
#define ACLK_133_RATIO		0x1
#define ACLK_66_RATIO		0x5

#define CLK_DIV_TOP0_VAL	((ACLK_300_DISP1_RATIO << 28)	\
				| (ACLK_400_G3D_RATIO << 24)	\
				| (ACLK_333_RATIO  << 20)	\
				| (ACLK_266_RATIO << 16)	\
				| (ACLK_200_RATIO << 12)	\
				| (ACLK_166_RATIO << 8)		\
				| (ACLK_133_RATIO << 4)		\
				| (ACLK_66_RATIO))

/* CLK_DIV_TOP1	*/
#define ACLK_MIPI_HSI_TX_BASE_RATIO     0x3
#define ACLK_66_PRE_RATIO               0x1
#define ACLK_400_ISP_RATIO              0x1
#define ACLK_400_IOP_RATIO              0x1
#define ACLK_300_GSCL_RATIO             0x2

#define CLK_DIV_TOP1_VAL	((ACLK_MIPI_HSI_TX_BASE_RATIO << 28)	\
				| (ACLK_66_PRE_RATIO << 24)		\
				| (ACLK_400_ISP_RATIO  << 20)		\
				| (ACLK_400_IOP_RATIO << 16)		\
				| (ACLK_300_GSCL_RATIO << 12))

/* APLL_LOCK	*/
#define APLL_LOCK_VAL	(0x546)
/* MPLL_LOCK	*/
#define MPLL_LOCK_VAL	(0x546)
/* CPLL_LOCK	*/
#define CPLL_LOCK_VAL	(0x546)
/* GPLL_LOCK	*/
#define GPLL_LOCK_VAL	(0x546)
/* EPLL_LOCK	*/
#define EPLL_LOCK_VAL	(0x3A98)
/* VPLL_LOCK	*/
#define VPLL_LOCK_VAL	(0x3A98)
/* BPLL_LOCK	*/
#define BPLL_LOCK_VAL	(0x546)

#define MUX_APLL_SEL_MASK	(1 << 0)
#define MUX_MPLL_SEL_MASK	(1 << 8)
#define MPLL_SEL_MOUT_MPLLFOUT	(2 << 8)
#define MUX_CPLL_SEL_MASK	(1 << 8)
#define MUX_EPLL_SEL_MASK	(1 << 12)
#define MUX_VPLL_SEL_MASK	(1 << 16)
#define MUX_GPLL_SEL_MASK	(1 << 28)
#define MUX_BPLL_SEL_MASK	(1 << 0)
#define MUX_HPM_SEL_MASK	(1 << 20)
#define HPM_SEL_SCLK_MPLL	(1 << 21)
#define APLL_CON0_LOCKED	(1 << 29)
#define MPLL_CON0_LOCKED	(1 << 29)
#define BPLL_CON0_LOCKED	(1 << 29)
#define CPLL_CON0_LOCKED	(1 << 29)
#define EPLL_CON0_LOCKED	(1 << 29)
#define GPLL_CON0_LOCKED	(1 << 29)
#define VPLL_CON0_LOCKED	(1 << 29)
#define CLK_REG_DISABLE		0x0
#define TOP2_VAL		0x0110000

/* CLK_SRC_PERIC0 */
#define PWM_SEL		0
#define UART3_SEL	6
#define UART2_SEL	6
#define UART1_SEL	6
#define UART0_SEL	6
/* SRC_CLOCK = SCLK_MPLL */
#define CLK_SRC_PERIC0_VAL	((PWM_SEL << 24)        \
				| (UART3_SEL << 12)     \
				| (UART2_SEL << 8)       \
				| (UART1_SEL << 4)      \
				| (UART0_SEL))

/* CLK_SRC_PERIC1 */
/* SRC_CLOCK = SCLK_MPLL */
#define SPI0_SEL		6
#define SPI1_SEL		6
#define SPI2_SEL		6
#define CLK_SRC_PERIC1_VAL	((SPI2_SEL << 24) \
				| (SPI1_SEL << 20) \
				| (SPI0_SEL << 16))

/* SCLK_SRC_ISP - set SPI0/1 to 6 = SCLK_MPLL_USER */
#define SPI0_ISP_SEL		6
#define SPI1_ISP_SEL		6
#define SCLK_SRC_ISP_VAL	(SPI1_ISP_SEL << 4) \
				| (SPI0_ISP_SEL << 0)

/* SCLK_DIV_ISP - set SPI0/1 to 0xf = divide by 16 */
#define SPI0_ISP_RATIO		0xf
#define SPI1_ISP_RATIO		0xf
#define SCLK_DIV_ISP_VAL	(SPI1_ISP_RATIO << 12) \
				| (SPI0_ISP_RATIO << 0)

/* CLK_DIV_PERIL0	*/
#define UART5_RATIO	7
#define UART4_RATIO	7
#define UART3_RATIO	7
#define UART2_RATIO	7
#define UART1_RATIO	7
#define UART0_RATIO	7

#define CLK_DIV_PERIC0_VAL	((UART3_RATIO << 12)    \
				| (UART2_RATIO << 8)    \
				| (UART1_RATIO << 4)    \
				| (UART0_RATIO))
/* CLK_DIV_PERIC1 */
#define SPI1_RATIO		0x7
#define SPI0_RATIO		0xf
#define SPI1_SUB_RATIO		0x0
#define SPI0_SUB_RATIO		0x0
#define CLK_DIV_PERIC1_VAL	((SPI1_SUB_RATIO << 24) \
				| ((SPI1_RATIO << 16) \
				| (SPI0_SUB_RATIO << 8) \
				| (SPI0_RATIO << 0)))

/* CLK_DIV_PERIC2 */
#define SPI2_RATIO		0xf
#define SPI2_SUB_RATIO		0x0
#define CLK_DIV_PERIC2_VAL	((SPI2_SUB_RATIO << 8) \
				| (SPI2_RATIO << 0))

/* CLK_DIV_PERIC3 */
#define PWM_RATIO		8
#define CLK_DIV_PERIC3_VAL	(PWM_RATIO << 0)

/* CLK_DIV_FSYS2 */
#define MMC2_RATIO_MASK		0xf
#define MMC2_RATIO_VAL		0x3
#define MMC2_RATIO_OFFSET	0

#define MMC2_PRE_RATIO_MASK	0xff
#define MMC2_PRE_RATIO_VAL	0x9
#define MMC2_PRE_RATIO_OFFSET	8

#define MMC3_RATIO_MASK		0xf
#define MMC3_RATIO_VAL		0x1
#define MMC3_RATIO_OFFSET	16

#define MMC3_PRE_RATIO_MASK	0xff
#define MMC3_PRE_RATIO_VAL	0x0
#define MMC3_PRE_RATIO_OFFSET	24

/* CLK_SRC_LEX */
#define CLK_SRC_LEX_VAL         0x0

/* CLK_DIV_LEX */
#define CLK_DIV_LEX_VAL         0x10

/* CLK_DIV_R0X */
#define CLK_DIV_R0X_VAL         0x10

/* CLK_DIV_L0X */
#define CLK_DIV_R1X_VAL         0x10

/* CLK_DIV_ISP0 */
#define CLK_DIV_ISP0_VAL        0x31

/* CLK_DIV_ISP1 */
#define CLK_DIV_ISP1_VAL        0x0

/* CLK_DIV_ISP2 */
#define CLK_DIV_ISP2_VAL        0x1

/* CLK_SRC_DISP1_0 */
#define CLK_SRC_DISP1_0_VAL	0x6

/*
 * DIV_DISP1_0
 * For DP, divisor should be 2
 */
#define CLK_DIV_DISP1_0_FIMD1	(2 << 0)

/* CLK_GATE_IP_DISP1 */
#define CLK_GATE_DP1_ALLOW	(1 << 4)

/*
 * TZPC Register Value :
 * R0SIZE: 0x0 : Size of secured ram
 */
#define R0SIZE			0x0

/*
 * TZPC Decode Protection Register Value :
 * DECPROTXSET: 0xFF : Set Decode region to non-secure
 */
#define DECPROTXSET		0xFF

#define DDR3PHY_CTRL_PHY_RESET	(1 << 0)
#define DDR3PHY_CTRL_PHY_RESET_OFF	(0 << 0)

#define PHY_CON0_RESET_VAL	0x17020a40
#define P0_CMD_EN		(1 << 14)
#define BYTE_RDLVL_EN		(1 << 13)
#define CTRL_SHGATE		(1 << 8)

#define PHY_CON1_RESET_VAL	0x09210100
#define CTRL_GATEDURADJ_MASK	(0xf << 20)

#define PHY_CON2_RESET_VAL	0x00010004
#define INIT_DESKEW_EN		(1 << 6)
#define RDLVL_GATE_EN		(1 << 24)

/*ZQ Configurations */
#define PHY_CON16_RESET_VAL	0x08000304

#define ZQ_CLK_DIV_EN		(1 << 18)
#define ZQ_MANUAL_STR		(1 << 1)
#define ZQ_DONE			(1 << 0)

#define CTRL_RDLVL_GATE_ENABLE	1
#define CTRL_RDLVL_GATE_DISABLE	1

/* Direct Command */
#define DIRECT_CMD_NOP			0x07000000
#define DIRECT_CMD_PALL			0x01000000
#define DIRECT_CMD_ZQINIT		0x0a000000
#define DIRECT_CMD_CHANNEL_SHIFT	28
#define DIRECT_CMD_CHIP_SHIFT		20

/* DMC PHY Control0 register */
#define PHY_CONTROL0_RESET_VAL	0x0
#define MEM_TERM_EN	(1 << 31)	/* Termination enable for memory */
#define PHY_TERM_EN	(1 << 30)	/* Termination enable for PHY */
#define DMC_CTRL_SHGATE	(1 << 29)	/* Duration of DQS gating signal */
#define FP_RSYNC	(1 << 3)	/* Force DLL resyncronization */

/* Driver strength for CK, CKE, CS & CA */
#define IMP_OUTPUT_DRV_40_OHM	0x5
#define IMP_OUTPUT_DRV_30_OHM	0x7
#define CA_CK_DRVR_DS_OFFSET	9
#define CA_CKE_DRVR_DS_OFFSET	6
#define CA_CS_DRVR_DS_OFFSET	3
#define CA_ADR_DRVR_DS_OFFSET	0

#define PHY_CON42_CTRL_BSTLEN_SHIFT	8
#define PHY_CON42_CTRL_RDLAT_SHIFT	0

struct mem_timings;

/* Errors that we can encourter in low-level setup */
enum {
	SETUP_ERR_OK,
	SETUP_ERR_RDLV_COMPLETE_TIMEOUT = -1,
	SETUP_ERR_ZQ_CALIBRATION_FAILURE = -2,
};

/*
 * Memory variant specific initialization code
 *
 * @param mem		Memory timings for this memory type.
 * @param mem_iv_size	Memory interleaving size is a configurable parameter
 *			which the DMC uses to decide how to split a memory
 *			chunk into smaller chunks to support concurrent
 *			accesses; may vary across boards.
 * @return 0 if ok, SETUP_ERR_... if there is a problem
 */
int ddr3_mem_ctrl_init(struct mem_timings *mem, unsigned long mem_iv_size);

/*
 * Configure ZQ I/O interface
 *
 * @param mem		Memory timings for this memory type.
 * @param phy0_ctrl	Pointer to struct containing PHY0 control reg
 * @param phy1_ctrl	Pointer to struct containing PHY1 control reg
 * @return 0 if ok, -1 on error
 */
int dmc_config_zq(struct mem_timings *mem,
		  struct exynos5_phy_control *phy0_ctrl,
		  struct exynos5_phy_control *phy1_ctrl);

/*
 * Send NOP and MRS/EMRS Direct commands
 *
 * @param mem		Memory timings for this memory type.
 * @param dmc		Pointer to struct of DMC registers
 */
void dmc_config_mrs(struct mem_timings *mem, struct exynos5_dmc *dmc);

/*
 * Send PALL Direct commands
 *
 * @param mem		Memory timings for this memory type.
 * @param dmc		Pointer to struct of DMC registers
 */
void dmc_config_prech(struct mem_timings *mem, struct exynos5_dmc *dmc);

/*
 * Configure the memconfig and membaseconfig registers
 *
 * @param mem		Memory timings for this memory type.
 * @param exynos5_dmc	Pointer to struct of DMC registers
 */
void dmc_config_memory(struct mem_timings *mem, struct exynos5_dmc *dmc);

/*
 * Reset the DLL. This function is common between DDR3 and LPDDR2.
 * However, the reset value is different. So we are passing a flag
 * ddr_mode to distinguish between LPDDR2 and DDR3.
 *
 * @param exynos5_dmc	Pointer to struct of DMC registers
 * @param ddr_mode	Type of DDR memory
 */
void update_reset_dll(struct exynos5_dmc *, enum ddr_mode);

void sdelay(unsigned long);
void mem_ctrl_init(void);
void system_clock_init(void);
void tzpc_init(void);
#endif
