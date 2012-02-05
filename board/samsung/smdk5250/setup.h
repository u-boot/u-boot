/*
 * Machine Specific Values for SMDK5250 board based on S5PC520
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
#include <version.h>
#include <asm/arch/cpu.h>

/* GPIO Offsets for UART: GPIO Contol Register */
#define EXYNOS5_GPIO_A0_CON_OFFSET	0x0
#define EXYNOS5_GPIO_A1_CON_OFFSET	0x20

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

/* CLK_SRC_CPU */
/* 0 = MOUTAPLL, 1 = SCLKMPLL */
#define MUX_HPM_SEL		0
#define MUX_CPU_SEL		0
#define MUX_APLL_SEL		1
#define CLK_SRC_CPU_VAL		((MUX_HPM_SEL << 20) \
				| (MUX_CPU_SEL << 16) \
				| (MUX_APLL_SEL))

/* CLK_DIV_CPU0 */
#define ARM2_RATIO		0x0
#define APLL_RATIO		0x1
#define PCLK_DBG_RATIO		0x1
#define ATB_RATIO		0x4
#define PERIPH_RATIO		0x7
#define ACP_RATIO		0x7
#define CPUD_RATIO		0x2
#define ARM_RATIO		0x0
#define CLK_DIV_CPU0_VAL	((ARM2_RATIO << 28) \
				| (APLL_RATIO << 24) \
				| (PCLK_DBG_RATIO << 20) \
				| (ATB_RATIO << 16) \
				| (PERIPH_RATIO << 12) \
				| (ACP_RATIO << 8) \
				| (CPUD_RATIO << 4) \
				| (ARM_RATIO))

/* CLK_DIV_CPU1 */
#define HPM_RATIO		0x4
#define COPY_RATIO		0x0
#define CLK_DIV_CPU1_VAL	((HPM_RATIO << 4) \
				| (COPY_RATIO))

#define APLL_MDIV		0x7D
#define APLL_PDIV		0x3
#define APLL_SDIV		0x0

#define MPLL_MDIV		0x64
#define MPLL_PDIV		0x3
#define MPLL_SDIV		0x0

#define CPLL_MDIV		0x96
#define CPLL_PDIV		0x4
#define CPLL_SDIV		0x0

/* APLL_CON1 */
#define APLL_CON1_VAL	(0x00203800)

/* MPLL_CON1 */
#define MPLL_CON1_VAL	(0x00203800)

/* CPLL_CON1 */
#define CPLL_CON1_VAL	(0x00203800)

#define EPLL_MDIV	0x60
#define EPLL_PDIV	0x3
#define EPLL_SDIV	0x3

#define EPLL_CON1_VAL	0x00000000
#define EPLL_CON2_VAL	0x00000080

#define VPLL_MDIV	0x96
#define VPLL_PDIV	0x3
#define VPLL_SDIV	0x2

#define VPLL_CON1_VAL	0x00000000
#define VPLL_CON2_VAL	0x00000080

#define BPLL_MDIV	0x215
#define BPLL_PDIV	0xC
#define BPLL_SDIV	0x1

#define BPLL_CON1_VAL	0x00203800

/* Set PLL */
#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)

#define APLL_CON0_VAL	set_pll(APLL_MDIV, APLL_PDIV, APLL_SDIV)
#define MPLL_CON0_VAL	set_pll(MPLL_MDIV, MPLL_PDIV, MPLL_SDIV)
#define CPLL_CON0_VAL	set_pll(CPLL_MDIV, CPLL_PDIV, CPLL_SDIV)
#define EPLL_CON0_VAL	set_pll(EPLL_MDIV, EPLL_PDIV, EPLL_SDIV)
#define VPLL_CON0_VAL	set_pll(VPLL_MDIV, VPLL_PDIV, VPLL_SDIV)
#define BPLL_CON0_VAL	set_pll(BPLL_MDIV, BPLL_PDIV, BPLL_SDIV)

/* CLK_SRC_CORE0 */
#define CLK_SRC_CORE0_VAL	0x00060000

/* CLK_SRC_CORE1 */
#define CLK_SRC_CORE1_VAL	0x100

/* CLK_DIV_CORE0 */
#define CLK_DIV_CORE0_VAL	0x120000

/* CLK_DIV_CORE1 */
#define CLK_DIV_CORE1_VAL	0x07070700

/* CLK_SRC_CDREX */
#define CLK_SRC_CDREX_INIT_VAL	0x1
#define CLK_SRC_CDREX_VAL	0x111

/* CLK_DIV_CDREX */
#define CLK_DIV_CDREX_INIT_VAL	0x71771111

#define MCLK_CDREX2_RATIO	0x0
#define ACLK_EFCON_RATIO	0x1
#define MCLK_DPHY_RATIO		0x0
#define MCLK_CDREX_RATIO	0x0
#define ACLK_C2C_200_RATIO	0x1
#define C2C_CLK_400_RATIO	0x1
#define PCLK_CDREX_RATIO	0x3
#define ACLK_CDREX_RATIO	0x1
#define CLK_DIV_CDREX_VAL	((MCLK_DPHY_RATIO << 20) \
				| (MCLK_CDREX_RATIO << 16) \
				| (ACLK_C2C_200_RATIO << 12) \
				| (C2C_CLK_400_RATIO << 8) \
				| (PCLK_CDREX_RATIO << 4) \
				| (ACLK_CDREX_RATIO))

#define MCLK_EFPHY_RATIO	0x4
#define CLK_DIV_CDREX2_VAL	MCLK_EFPHY_RATIO

/* CLK_DIV_ACP */
#define CLK_DIV_ACP_VAL	0x12

/* CLK_SRC_TOP0 */
#define MUX_ACLK_300_GSCL_SEL		0x1
#define MUX_ACLK_300_GSCL_MID_SEL	0x0
#define MUX_ACLK_400_SEL		0x0
#define MUX_ACLK_333_SEL		0x0
#define MUX_ACLK_300_DISP1_SEL		0x1
#define MUX_ACLK_300_DISP1_MID_SEL	0x0
#define MUX_ACLK_200_SEL		0x0
#define MUX_ACLK_166_SEL		0x0
#define CLK_SRC_TOP0_VAL	((MUX_ACLK_300_GSCL_SEL << 25) \
				| (MUX_ACLK_300_GSCL_MID_SEL << 24) \
				| (MUX_ACLK_400_SEL << 20) \
				| (MUX_ACLK_333_SEL << 16) \
				| (MUX_ACLK_300_DISP1_SEL << 15) \
				| (MUX_ACLK_300_DISP1_MID_SEL << 14)	\
				| (MUX_ACLK_200_SEL << 12) \
				| (MUX_ACLK_166_SEL << 8))

/* CLK_SRC_TOP1 */
#define MUX_ACLK_400_ISP_SEL		0x0
#define MUX_ACLK_400_IOP_SEL		0x0
#define MUX_ACLK_MIPI_HSI_TXBASE_SEL	0x0
#define CLK_SRC_TOP1_VAL		((MUX_ACLK_400_ISP_SEL << 24) \
					|(MUX_ACLK_400_IOP_SEL << 20) \
					|(MUX_ACLK_MIPI_HSI_TXBASE_SEL << 16))

/* CLK_SRC_TOP2 */
#define MUX_BPLL_USER_SEL	0x1
#define MUX_MPLL_USER_SEL	0x1
#define MUX_VPLL_SEL		0x0
#define MUX_EPLL_SEL		0x0
#define MUX_CPLL_SEL		0x0
#define VPLLSRC_SEL		0x0
#define CLK_SRC_TOP2_VAL	((MUX_BPLL_USER_SEL << 24) \
				| (MUX_MPLL_USER_SEL << 20) \
				| (MUX_VPLL_SEL << 16) \
				| (MUX_EPLL_SEL << 12) \
				| (MUX_CPLL_SEL << 8) \
				| (VPLLSRC_SEL))
/* CLK_SRC_TOP3 */
#define MUX_ACLK_333_SUB_SEL		0x1
#define MUX_ACLK_400_SUB_SEL		0x1
#define MUX_ACLK_266_ISP_SUB_SEL	0x1
#define MUX_ACLK_266_GPS_SUB_SEL	0x1
#define MUX_ACLK_300_GSCL_SUB_SEL	0x1
#define MUX_ACLK_266_GSCL_SUB_SEL	0x1
#define MUX_ACLK_300_DISP1_SUB_SEL	0x1
#define MUX_ACLK_200_DISP1_SUB_SEL	0x1
#define CLK_SRC_TOP3_VAL		((MUX_ACLK_333_SUB_SEL << 24) \
					| (MUX_ACLK_400_SUB_SEL << 20) \
					| (MUX_ACLK_266_ISP_SUB_SEL << 16) \
					| (MUX_ACLK_266_GPS_SUB_SEL << 12) \
					| (MUX_ACLK_300_GSCL_SUB_SEL << 10) \
					| (MUX_ACLK_266_GSCL_SUB_SEL << 8) \
					| (MUX_ACLK_300_DISP1_SUB_SEL << 6) \
					| (MUX_ACLK_200_DISP1_SUB_SEL << 4))

/* CLK_DIV_TOP0 */
#define ACLK_300_RATIO		0x0
#define ACLK_400_RATIO		0x3
#define ACLK_333_RATIO		0x2
#define ACLK_266_RATIO		0x2
#define ACLK_200_RATIO		0x3
#define ACLK_166_RATIO		0x5
#define ACLK_133_RATIO		0x1
#define ACLK_66_RATIO		0x5
#define CLK_DIV_TOP0_VAL	((ACLK_300_RATIO << 28) \
				| (ACLK_400_RATIO << 24) \
				| (ACLK_333_RATIO << 20) \
				| (ACLK_266_RATIO << 16) \
				| (ACLK_200_RATIO << 12) \
				| (ACLK_166_RATIO << 8) \
				| (ACLK_133_RATIO << 4) \
				| (ACLK_66_RATIO))

/* CLK_DIV_TOP1 */
#define ACLK_MIPI_HSI_TX_BASE_RATIO	0x3
#define ACLK_66_PRE_RATIO	0x1
#define ACLK_400_ISP_RATIO	0x1
#define ACLK_400_IOP_RATIO	0x1
#define ACLK_300_GSCL_RATIO	0x0
#define ACLK_266_GPS_RATIO	0x7

#define CLK_DIV_TOP1_VAL	((ACLK_MIPI_HSI_TX_BASE_RATIO << 28) \
				| (ACLK_66_PRE_RATIO << 24) \
				| (ACLK_400_ISP_RATIO << 20) \
				| (ACLK_400_IOP_RATIO << 16) \
				| (ACLK_300_GSCL_RATIO << 12) \
				| (ACLK_266_GPS_RATIO << 8))

/* APLL_LOCK */
#define APLL_LOCK_VAL		(0x3E8)
/* MPLL_LOCK */
#define MPLL_LOCK_VAL		(0x2F1)
/* CPLL_LOCK */
#define CPLL_LOCK_VAL		(0x3E8)
/* EPLL_LOCK */
#define EPLL_LOCK_VAL		(0x2321)
/* VPLL_LOCK */
#define VPLL_LOCK_VAL		(0x2321)
/* BPLL_LOCK */
#define BPLL_LOCK_VAL		(0x3E8)

/* CLK_SRC_PERIC0 */
/* SRC_CLOCK = SCLK_MPLL */
#define PWM_SEL			0
#define UART4_SEL		6
#define UART3_SEL		6
#define UART2_SEL		6
#define UART1_SEL		6
#define UART0_SEL		6
#define CLK_SRC_PERIC0_VAL	((PWM_SEL << 24) \
				| (UART4_SEL << 16) \
				| (UART3_SEL << 12) \
				| (UART2_SEL << 8) \
				| (UART1_SEL << 4) \
				| (UART0_SEL << 0))

#define CLK_SRC_FSYS_VAL	0x66666
#define CLK_DIV_FSYS0_VAL	0x0BB00000
#define CLK_DIV_FSYS1_VAL	0x000f000f
#define CLK_DIV_FSYS2_VAL	0x020f020f
#define CLK_DIV_FSYS3_VAL	0x000f

/* CLK_DIV_PERIC0 */
#define UART5_RATIO		8
#define UART4_RATIO		8
#define UART3_RATIO		8
#define UART2_RATIO		8
#define UART1_RATIO		8
#define UART0_RATIO		8
#define CLK_DIV_PERIC0_VAL	((UART4_RATIO << 16) \
				| (UART3_RATIO << 12) \
				| (UART2_RATIO << 8) \
				| (UART1_RATIO << 4) \
				| (UART0_RATIO << 0))

/* CLK_DIV_PERIC3 */
#define PWM_RATIO		8
#define CLK_DIV_PERIC3_VAL	(PWM_RATIO << 0)

/* CLK_SRC_LEX */
#define CLK_SRC_LEX_VAL		0x0

/* CLK_DIV_LEX */
#define CLK_DIV_LEX_VAL		0x10

/* CLK_DIV_R0X */
#define CLK_DIV_R0X_VAL		0x10

/* CLK_DIV_L0X */
#define CLK_DIV_R1X_VAL		0x10

/* SCLK_SRC_ISP */
#define SCLK_SRC_ISP_VAL	0x600
/* CLK_DIV_ISP0 */
#define CLK_DIV_ISP0_VAL	0x31

/* CLK_DIV_ISP1 */
#define CLK_DIV_ISP1_VAL	0x0

/* CLK_DIV_ISP2 */
#define CLK_DIV_ISP2_VAL	0x1

#define MPLL_DEC	(MPLL_MDIV * MPLL_MDIV / (MPLL_PDIV * 2^(MPLL_SDIV-1)))

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

/* DMC Init */
#define SET			1
#define RESET			0
/* (Memory Interleaving Size = 1 << IV_SIZE) */
#define CONFIG_IV_SIZE		0x07

#define PHY_RESET_VAL	(0 << 0)

/*ZQ Configurations */
#define PHY_CON16_RESET_VAL	0x08000304

#define ZQ_MODE_DDS_VAL		(0x5 << 24)
#define ZQ_MODE_TERM_VAL	(0x5 << 21)
#define SET_ZQ_MODE_DDS_VAL(x)	(x = (x & ~(0x7 << 24)) | ZQ_MODE_DDS_VAL)
#define SET_ZQ_MODE_TERM_VAL(x)	(x = (x & ~(0x7 << 21)) | ZQ_MODE_TERM_VAL)

#define ZQ_MODE_NOTERM		(1 << 19)
#define ZQ_CLK_DIV_EN		(1 << 18)
#define ZQ_MANUAL_STR		(1 << 1)

/* Channel and Chip Selection */
#define CONFIG_DMC_CHANNELS		2
#define CONFIG_CHIPS_PER_CHANNEL	2

#define SET_CMD_CHANNEL(x, y)	(x = (x & ~(1 << 28)) | y << 28)
#define SET_CMD_CHIP(x, y)	(x = (x & ~(1 << 20)) | y << 20)

/* Diret Command */
#define	DIRECT_CMD_NOP		0x07000000
#define DIRECT_CMD_MRS1		0x00071C00
#define DIRECT_CMD_MRS2		0x00010BFC
#define DIRECT_CMD_MRS3		0x00000708
#define DIRECT_CMD_MRS4		0x00000818
#define	DIRECT_CMD_PALL		0x01000000

/* DLL Resync */
#define FP_RSYNC		(1 << 3)

#define CONFIG_CTRL_DLL_ON(x, y)	(x = (x & ~(1 << 5)) | y << 5)
#define CONFIG_CTRL_START(x, y)		(x = (x & ~(1 << 6)) | y << 6)
#define SET_CTRL_FORCE_VAL(x, y)	(x = (x & ~(0x7F << 8)) | y << 8)

/* RDLVL */
#define PHY_CON0_RESET_VAL	0x17023240
#define DDR_MODE_LPDDR2		0x2
#define BYTE_RDLVL_EN		(1 << 13)
#define CTRL_ATGATE		(1 << 6)
#define SET_CTRL_DDR_MODE(x, y)	(x = (x & ~(0x3 << 11)) | y << 11)

#define PHY_CON1_RESET_VAL	0x9210100
#define RDLVL_RDDATAPADJ	0x1
#define SET_RDLVL_RDDATAPADJ	((PHY_CON1_RESET_VAL & ~(0xFFFF << 0))\
					| RDLVL_RDDATAPADJ << 0)

#define PHY_CON2_RESET_VAL	0x00010004
#define RDLVL_EN		(1 << 25)
#define RDDSKEW_CLEAR		(1 << 13)

#define CTRL_RDLVL_DATA_EN	(1 << 1)
#define LPDDR2_ADDR		0x00000208

#define DMC_MEMCONFIG0_VAL	0x00001323
#define DMC_MEMCONFIG1_VAL	0x00001323
#define DMC_MEMBASECONFIG0_VAL	0x00400780
#define DMC_MEMBASECONFIG1_VAL	0x00800780
#define DMC_MEMCONTROL_VAL	0x00212500
#define DMC_PRECHCONFIG_VAL		0xFF000000
#define DMC_PWRDNCONFIG_VAL		0xFFFF00FF
#define DMC_TIMINGREF_VAL		0x0000005D
#define DMC_TIMINGROW_VAL		0x2336544C
#define DMC_TIMINGDATA_VAL		0x24202408
#define DMC_TIMINGPOWER_VAL		0x38260235

#define CTRL_BSTLEN		0x04
#define CTRL_RDLAT		0x08
#define PHY_CON42_VAL		(CTRL_BSTLEN << 8 | CTRL_RDLAT << 0)

/* DQS, DQ, DEBUG offsets */
#define	SET_DQS_OFFSET_VAL	0x7F7F7F7F
#define	SET_DQ_OFFSET_VAL	0x7F7F7F7F
#define	SET_DEBUG_OFFSET_VAL	0x7F

#define	RESET_DQS_OFFSET_VAL	0x08080808
#define	RESET_DQ_OFFSET_VAL	0x08080808
#define	RESET_DEBUG_OFFSET_VAL	0x8

#define CTRL_PULLD_DQ		(0x0F << 8)
#define CTRL_PULLD_DQS		(0x0F << 0)

#define DFI_INIT_START		(1 << 28)

#define CLK_STOP_EN	(1 << 0)
#define DPWRDN_EN	(1 << 1)
#define DSREF_EN	(1 << 5)

#define AREF_EN			(1 << 5)
void sdelay(unsigned long);
void mem_ctrl_init(void);
void system_clock_init(void);
void tzpc_init(void);

#endif
