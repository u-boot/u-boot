/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier:	GPL-2.0+	BSD-3-Clause
 */

#include <common.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <regmap.h>
#include <spl.h>
#include <syscon.h>
#include <linux/io.h>
#include <dt-bindings/clock/stm32mp1-clks.h>

#define MAX_HSI_HZ		64000000

/* RCC registers */
#define RCC_OCENSETR		0x0C
#define RCC_OCENCLRR		0x10
#define RCC_HSICFGR		0x18
#define RCC_MPCKSELR		0x20
#define RCC_ASSCKSELR		0x24
#define RCC_RCK12SELR		0x28
#define RCC_MPCKDIVR		0x2C
#define RCC_AXIDIVR		0x30
#define RCC_APB4DIVR		0x3C
#define RCC_APB5DIVR		0x40
#define RCC_RTCDIVR		0x44
#define RCC_MSSCKSELR		0x48
#define RCC_PLL1CR		0x80
#define RCC_PLL1CFGR1		0x84
#define RCC_PLL1CFGR2		0x88
#define RCC_PLL1FRACR		0x8C
#define RCC_PLL1CSGR		0x90
#define RCC_PLL2CR		0x94
#define RCC_PLL2CFGR1		0x98
#define RCC_PLL2CFGR2		0x9C
#define RCC_PLL2FRACR		0xA0
#define RCC_PLL2CSGR		0xA4
#define RCC_I2C46CKSELR		0xC0
#define RCC_CPERCKSELR		0xD0
#define RCC_STGENCKSELR		0xD4
#define RCC_DDRITFCR		0xD8
#define RCC_BDCR		0x140
#define RCC_RDLSICR		0x144
#define RCC_MP_APB4ENSETR	0x200
#define RCC_MP_APB5ENSETR	0x208
#define RCC_MP_AHB5ENSETR	0x210
#define RCC_MP_AHB6ENSETR	0x218
#define RCC_OCRDYR		0x808
#define RCC_DBGCFGR		0x80C
#define RCC_RCK3SELR		0x820
#define RCC_RCK4SELR		0x824
#define RCC_MCUDIVR		0x830
#define RCC_APB1DIVR		0x834
#define RCC_APB2DIVR		0x838
#define RCC_APB3DIVR		0x83C
#define RCC_PLL3CR		0x880
#define RCC_PLL3CFGR1		0x884
#define RCC_PLL3CFGR2		0x888
#define RCC_PLL3FRACR		0x88C
#define RCC_PLL3CSGR		0x890
#define RCC_PLL4CR		0x894
#define RCC_PLL4CFGR1		0x898
#define RCC_PLL4CFGR2		0x89C
#define RCC_PLL4FRACR		0x8A0
#define RCC_PLL4CSGR		0x8A4
#define RCC_I2C12CKSELR		0x8C0
#define RCC_I2C35CKSELR		0x8C4
#define RCC_UART6CKSELR		0x8E4
#define RCC_UART24CKSELR	0x8E8
#define RCC_UART35CKSELR	0x8EC
#define RCC_UART78CKSELR	0x8F0
#define RCC_SDMMC12CKSELR	0x8F4
#define RCC_SDMMC3CKSELR	0x8F8
#define RCC_ETHCKSELR		0x8FC
#define RCC_QSPICKSELR		0x900
#define RCC_FMCCKSELR		0x904
#define RCC_USBCKSELR		0x91C
#define RCC_MP_APB1ENSETR	0xA00
#define RCC_MP_APB2ENSETR	0XA08
#define RCC_MP_AHB2ENSETR	0xA18
#define RCC_MP_AHB4ENSETR	0xA28

/* used for most of SELR register */
#define RCC_SELR_SRC_MASK	GENMASK(2, 0)
#define RCC_SELR_SRCRDY		BIT(31)

/* Values of RCC_MPCKSELR register */
#define RCC_MPCKSELR_HSI	0
#define RCC_MPCKSELR_HSE	1
#define RCC_MPCKSELR_PLL	2
#define RCC_MPCKSELR_PLL_MPUDIV	3

/* Values of RCC_ASSCKSELR register */
#define RCC_ASSCKSELR_HSI	0
#define RCC_ASSCKSELR_HSE	1
#define RCC_ASSCKSELR_PLL	2

/* Values of RCC_MSSCKSELR register */
#define RCC_MSSCKSELR_HSI	0
#define RCC_MSSCKSELR_HSE	1
#define RCC_MSSCKSELR_CSI	2
#define RCC_MSSCKSELR_PLL	3

/* Values of RCC_CPERCKSELR register */
#define RCC_CPERCKSELR_HSI	0
#define RCC_CPERCKSELR_CSI	1
#define RCC_CPERCKSELR_HSE	2

/* used for most of DIVR register : max div for RTC */
#define RCC_DIVR_DIV_MASK	GENMASK(5, 0)
#define RCC_DIVR_DIVRDY		BIT(31)

/* Masks for specific DIVR registers */
#define RCC_APBXDIV_MASK	GENMASK(2, 0)
#define RCC_MPUDIV_MASK		GENMASK(2, 0)
#define RCC_AXIDIV_MASK		GENMASK(2, 0)
#define RCC_MCUDIV_MASK		GENMASK(3, 0)

/*  offset between RCC_MP_xxxENSETR and RCC_MP_xxxENCLRR registers */
#define RCC_MP_ENCLRR_OFFSET	4

/* Fields of RCC_BDCR register */
#define RCC_BDCR_LSEON		BIT(0)
#define RCC_BDCR_LSEBYP		BIT(1)
#define RCC_BDCR_LSERDY		BIT(2)
#define RCC_BDCR_LSEDRV_MASK	GENMASK(5, 4)
#define RCC_BDCR_LSEDRV_SHIFT	4
#define RCC_BDCR_LSECSSON	BIT(8)
#define RCC_BDCR_RTCCKEN	BIT(20)
#define RCC_BDCR_RTCSRC_MASK	GENMASK(17, 16)
#define RCC_BDCR_RTCSRC_SHIFT	16

/* Fields of RCC_RDLSICR register */
#define RCC_RDLSICR_LSION	BIT(0)
#define RCC_RDLSICR_LSIRDY	BIT(1)

/* used for ALL PLLNCR registers */
#define RCC_PLLNCR_PLLON	BIT(0)
#define RCC_PLLNCR_PLLRDY	BIT(1)
#define RCC_PLLNCR_DIVPEN	BIT(4)
#define RCC_PLLNCR_DIVQEN	BIT(5)
#define RCC_PLLNCR_DIVREN	BIT(6)
#define RCC_PLLNCR_DIVEN_SHIFT	4

/* used for ALL PLLNCFGR1 registers */
#define RCC_PLLNCFGR1_DIVM_SHIFT	16
#define RCC_PLLNCFGR1_DIVM_MASK		GENMASK(21, 16)
#define RCC_PLLNCFGR1_DIVN_SHIFT	0
#define RCC_PLLNCFGR1_DIVN_MASK		GENMASK(8, 0)
/* only for PLL3 and PLL4 */
#define RCC_PLLNCFGR1_IFRGE_SHIFT	24
#define RCC_PLLNCFGR1_IFRGE_MASK	GENMASK(25, 24)

/* used for ALL PLLNCFGR2 registers */
#define RCC_PLLNCFGR2_DIVX_MASK		GENMASK(6, 0)
#define RCC_PLLNCFGR2_DIVP_SHIFT	0
#define RCC_PLLNCFGR2_DIVP_MASK		GENMASK(6, 0)
#define RCC_PLLNCFGR2_DIVQ_SHIFT	8
#define RCC_PLLNCFGR2_DIVQ_MASK		GENMASK(14, 8)
#define RCC_PLLNCFGR2_DIVR_SHIFT	16
#define RCC_PLLNCFGR2_DIVR_MASK		GENMASK(22, 16)

/* used for ALL PLLNFRACR registers */
#define RCC_PLLNFRACR_FRACV_SHIFT	3
#define RCC_PLLNFRACR_FRACV_MASK	GENMASK(15, 3)
#define RCC_PLLNFRACR_FRACLE		BIT(16)

/* used for ALL PLLNCSGR registers */
#define RCC_PLLNCSGR_INC_STEP_SHIFT	16
#define RCC_PLLNCSGR_INC_STEP_MASK	GENMASK(30, 16)
#define RCC_PLLNCSGR_MOD_PER_SHIFT	0
#define RCC_PLLNCSGR_MOD_PER_MASK	GENMASK(12, 0)
#define RCC_PLLNCSGR_SSCG_MODE_SHIFT	15
#define RCC_PLLNCSGR_SSCG_MODE_MASK	BIT(15)

/* used for RCC_OCENSETR and RCC_OCENCLRR registers */
#define RCC_OCENR_HSION			BIT(0)
#define RCC_OCENR_CSION			BIT(4)
#define RCC_OCENR_HSEON			BIT(8)
#define RCC_OCENR_HSEBYP		BIT(10)
#define RCC_OCENR_HSECSSON		BIT(11)

/* Fields of RCC_OCRDYR register */
#define RCC_OCRDYR_HSIRDY		BIT(0)
#define RCC_OCRDYR_HSIDIVRDY		BIT(2)
#define RCC_OCRDYR_CSIRDY		BIT(4)
#define RCC_OCRDYR_HSERDY		BIT(8)

/* Fields of DDRITFCR register */
#define RCC_DDRITFCR_DDRCKMOD_MASK	GENMASK(22, 20)
#define RCC_DDRITFCR_DDRCKMOD_SHIFT	20
#define RCC_DDRITFCR_DDRCKMOD_SSR	0

/* Fields of RCC_HSICFGR register */
#define RCC_HSICFGR_HSIDIV_MASK		GENMASK(1, 0)

/* used for MCO related operations */
#define RCC_MCOCFG_MCOON		BIT(12)
#define RCC_MCOCFG_MCODIV_MASK		GENMASK(7, 4)
#define RCC_MCOCFG_MCODIV_SHIFT		4
#define RCC_MCOCFG_MCOSRC_MASK		GENMASK(2, 0)

enum stm32mp1_parent_id {
/*
 * _HSI, _HSE, _CSI, _LSI, _LSE should not be moved
 * they are used as index in osc[] as entry point
 */
	_HSI,
	_HSE,
	_CSI,
	_LSI,
	_LSE,
	_I2S_CKIN,
	_USB_PHY_48,
	NB_OSC,

/* other parent source */
	_HSI_KER = NB_OSC,
	_HSE_KER,
	_HSE_KER_DIV2,
	_CSI_KER,
	_PLL1_P,
	_PLL1_Q,
	_PLL1_R,
	_PLL2_P,
	_PLL2_Q,
	_PLL2_R,
	_PLL3_P,
	_PLL3_Q,
	_PLL3_R,
	_PLL4_P,
	_PLL4_Q,
	_PLL4_R,
	_ACLK,
	_PCLK1,
	_PCLK2,
	_PCLK3,
	_PCLK4,
	_PCLK5,
	_HCLK6,
	_HCLK2,
	_CK_PER,
	_CK_MPU,
	_CK_MCU,
	_PARENT_NB,
	_UNKNOWN_ID = 0xff,
};

enum stm32mp1_parent_sel {
	_I2C12_SEL,
	_I2C35_SEL,
	_I2C46_SEL,
	_UART6_SEL,
	_UART24_SEL,
	_UART35_SEL,
	_UART78_SEL,
	_SDMMC12_SEL,
	_SDMMC3_SEL,
	_ETH_SEL,
	_QSPI_SEL,
	_FMC_SEL,
	_USBPHY_SEL,
	_USBO_SEL,
	_STGEN_SEL,
	_PARENT_SEL_NB,
	_UNKNOWN_SEL = 0xff,
};

enum stm32mp1_pll_id {
	_PLL1,
	_PLL2,
	_PLL3,
	_PLL4,
	_PLL_NB
};

enum stm32mp1_div_id {
	_DIV_P,
	_DIV_Q,
	_DIV_R,
	_DIV_NB,
};

enum stm32mp1_clksrc_id {
	CLKSRC_MPU,
	CLKSRC_AXI,
	CLKSRC_MCU,
	CLKSRC_PLL12,
	CLKSRC_PLL3,
	CLKSRC_PLL4,
	CLKSRC_RTC,
	CLKSRC_MCO1,
	CLKSRC_MCO2,
	CLKSRC_NB
};

enum stm32mp1_clkdiv_id {
	CLKDIV_MPU,
	CLKDIV_AXI,
	CLKDIV_MCU,
	CLKDIV_APB1,
	CLKDIV_APB2,
	CLKDIV_APB3,
	CLKDIV_APB4,
	CLKDIV_APB5,
	CLKDIV_RTC,
	CLKDIV_MCO1,
	CLKDIV_MCO2,
	CLKDIV_NB
};

enum stm32mp1_pllcfg {
	PLLCFG_M,
	PLLCFG_N,
	PLLCFG_P,
	PLLCFG_Q,
	PLLCFG_R,
	PLLCFG_O,
	PLLCFG_NB
};

enum stm32mp1_pllcsg {
	PLLCSG_MOD_PER,
	PLLCSG_INC_STEP,
	PLLCSG_SSCG_MODE,
	PLLCSG_NB
};

enum stm32mp1_plltype {
	PLL_800,
	PLL_1600,
	PLL_TYPE_NB
};

struct stm32mp1_pll {
	u8 refclk_min;
	u8 refclk_max;
	u8 divn_max;
};

struct stm32mp1_clk_gate {
	u16 offset;
	u8 bit;
	u8 index;
	u8 set_clr;
	u8 sel;
	u8 fixed;
};

struct stm32mp1_clk_sel {
	u16 offset;
	u8 src;
	u8 msk;
	u8 nb_parent;
	const u8 *parent;
};

#define REFCLK_SIZE 4
struct stm32mp1_clk_pll {
	enum stm32mp1_plltype plltype;
	u16 rckxselr;
	u16 pllxcfgr1;
	u16 pllxcfgr2;
	u16 pllxfracr;
	u16 pllxcr;
	u16 pllxcsgr;
	u8 refclk[REFCLK_SIZE];
};

struct stm32mp1_clk_data {
	const struct stm32mp1_clk_gate *gate;
	const struct stm32mp1_clk_sel *sel;
	const struct stm32mp1_clk_pll *pll;
	const int nb_gate;
};

struct stm32mp1_clk_priv {
	fdt_addr_t base;
	const struct stm32mp1_clk_data *data;
	ulong osc[NB_OSC];
	struct udevice *osc_dev[NB_OSC];
};

#define STM32MP1_CLK(off, b, idx, s)		\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 0,			\
		.sel = (s),			\
		.fixed = _UNKNOWN_ID,		\
	}

#define STM32MP1_CLK_F(off, b, idx, f)		\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 0,			\
		.sel = _UNKNOWN_SEL,		\
		.fixed = (f),			\
	}

#define STM32MP1_CLK_SET_CLR(off, b, idx, s)	\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 1,			\
		.sel = (s),			\
		.fixed = _UNKNOWN_ID,		\
	}

#define STM32MP1_CLK_SET_CLR_F(off, b, idx, f)	\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 1,			\
		.sel = _UNKNOWN_SEL,		\
		.fixed = (f),			\
	}

#define STM32MP1_CLK_PARENT(idx, off, s, m, p)   \
	[(idx)] = {				\
		.offset = (off),		\
		.src = (s),			\
		.msk = (m),			\
		.parent = (p),			\
		.nb_parent = ARRAY_SIZE((p))	\
	}

#define STM32MP1_CLK_PLL(idx, type, off1, off2, off3, off4, off5, off6,\
			p1, p2, p3, p4) \
	[(idx)] = {				\
		.plltype = (type),			\
		.rckxselr = (off1),		\
		.pllxcfgr1 = (off2),		\
		.pllxcfgr2 = (off3),		\
		.pllxfracr = (off4),		\
		.pllxcr = (off5),		\
		.pllxcsgr = (off6),		\
		.refclk[0] = (p1),		\
		.refclk[1] = (p2),		\
		.refclk[2] = (p3),		\
		.refclk[3] = (p4),		\
	}

static const u8 stm32mp1_clks[][2] = {
	{CK_PER, _CK_PER},
	{CK_MPU, _CK_MPU},
	{CK_AXI, _ACLK},
	{CK_MCU, _CK_MCU},
	{CK_HSE, _HSE},
	{CK_CSI, _CSI},
	{CK_LSI, _LSI},
	{CK_LSE, _LSE},
	{CK_HSI, _HSI},
	{CK_HSE_DIV2, _HSE_KER_DIV2},
};

static const struct stm32mp1_clk_gate stm32mp1_clk_gate[] = {
	STM32MP1_CLK(RCC_DDRITFCR, 0, DDRC1, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 1, DDRC1LP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 2, DDRC2, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 3, DDRC2LP, _UNKNOWN_SEL),
	STM32MP1_CLK_F(RCC_DDRITFCR, 4, DDRPHYC, _PLL2_R),
	STM32MP1_CLK(RCC_DDRITFCR, 5, DDRPHYCLP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 6, DDRCAPB, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 7, DDRCAPBLP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 8, AXIDCG, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 9, DDRPHYCAPB, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 10, DDRPHYCAPBLP, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 14, USART2_K, _UART24_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 15, USART3_K, _UART35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 16, UART4_K, _UART24_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 17, UART5_K, _UART35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 18, UART7_K, _UART78_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 19, UART8_K, _UART78_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 21, I2C1_K, _I2C12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 22, I2C2_K, _I2C12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 23, I2C3_K, _I2C35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 24, I2C5_K, _I2C35_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB2ENSETR, 13, USART6_K, _UART6_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 8, DDRPERFM, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 15, IWDG2, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 16, USBPHY_K, _USBPHY_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 2, I2C4_K, _I2C46_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 20, STGEN_K, _STGEN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB2ENSETR, 8, USBO_K, _USBO_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB2ENSETR, 16, SDMMC3_K, _SDMMC3_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 0, GPIOA, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 1, GPIOB, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 2, GPIOC, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 3, GPIOD, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 4, GPIOE, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 5, GPIOF, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 6, GPIOG, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 7, GPIOH, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 8, GPIOI, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 9, GPIOJ, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 10, GPIOK, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB5ENSETR, 0, GPIOZ, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 7, ETHCK, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 8, ETHTX, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 9, ETHRX, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 10, ETHMAC_K, _ETH_SEL),
	STM32MP1_CLK_SET_CLR_F(RCC_MP_AHB6ENSETR, 10, ETHMAC, _ACLK),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 12, FMC_K, _FMC_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 14, QSPI_K, _QSPI_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 16, SDMMC1_K, _SDMMC12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 17, SDMMC2_K, _SDMMC12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 24, USBH, _UNKNOWN_SEL),

	STM32MP1_CLK(RCC_DBGCFGR, 8, CK_DBG, _UNKNOWN_SEL),
};

static const u8 i2c12_parents[] = {_PCLK1, _PLL4_R, _HSI_KER, _CSI_KER};
static const u8 i2c35_parents[] = {_PCLK1, _PLL4_R, _HSI_KER, _CSI_KER};
static const u8 i2c46_parents[] = {_PCLK5, _PLL3_Q, _HSI_KER, _CSI_KER};
static const u8 uart6_parents[] = {_PCLK2, _PLL4_Q, _HSI_KER, _CSI_KER,
					_HSE_KER};
static const u8 uart24_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const u8 uart35_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const u8 uart78_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const u8 sdmmc12_parents[] = {_HCLK6, _PLL3_R, _PLL4_P, _HSI_KER};
static const u8 sdmmc3_parents[] = {_HCLK2, _PLL3_R, _PLL4_P, _HSI_KER};
static const u8 eth_parents[] = {_PLL4_P, _PLL3_Q};
static const u8 qspi_parents[] = {_ACLK, _PLL3_R, _PLL4_P, _CK_PER};
static const u8 fmc_parents[] = {_ACLK, _PLL3_R, _PLL4_P, _CK_PER};
static const u8 usbphy_parents[] = {_HSE_KER, _PLL4_R, _HSE_KER_DIV2};
static const u8 usbo_parents[] = {_PLL4_R, _USB_PHY_48};
static const u8 stgen_parents[] = {_HSI_KER, _HSE_KER};

static const struct stm32mp1_clk_sel stm32mp1_clk_sel[_PARENT_SEL_NB] = {
	STM32MP1_CLK_PARENT(_I2C12_SEL, RCC_I2C12CKSELR, 0, 0x7, i2c12_parents),
	STM32MP1_CLK_PARENT(_I2C35_SEL, RCC_I2C35CKSELR, 0, 0x7, i2c35_parents),
	STM32MP1_CLK_PARENT(_I2C46_SEL, RCC_I2C46CKSELR, 0, 0x7, i2c46_parents),
	STM32MP1_CLK_PARENT(_UART6_SEL, RCC_UART6CKSELR, 0, 0x7, uart6_parents),
	STM32MP1_CLK_PARENT(_UART24_SEL, RCC_UART24CKSELR, 0, 0x7,
			    uart24_parents),
	STM32MP1_CLK_PARENT(_UART35_SEL, RCC_UART35CKSELR, 0, 0x7,
			    uart35_parents),
	STM32MP1_CLK_PARENT(_UART78_SEL, RCC_UART78CKSELR, 0, 0x7,
			    uart78_parents),
	STM32MP1_CLK_PARENT(_SDMMC12_SEL, RCC_SDMMC12CKSELR, 0, 0x7,
			    sdmmc12_parents),
	STM32MP1_CLK_PARENT(_SDMMC3_SEL, RCC_SDMMC3CKSELR, 0, 0x7,
			    sdmmc3_parents),
	STM32MP1_CLK_PARENT(_ETH_SEL, RCC_ETHCKSELR, 0, 0x3, eth_parents),
	STM32MP1_CLK_PARENT(_QSPI_SEL, RCC_QSPICKSELR, 0, 0xf, qspi_parents),
	STM32MP1_CLK_PARENT(_FMC_SEL, RCC_FMCCKSELR, 0, 0xf, fmc_parents),
	STM32MP1_CLK_PARENT(_USBPHY_SEL, RCC_USBCKSELR, 0, 0x3, usbphy_parents),
	STM32MP1_CLK_PARENT(_USBO_SEL, RCC_USBCKSELR, 4, 0x1, usbo_parents),
	STM32MP1_CLK_PARENT(_STGEN_SEL, RCC_STGENCKSELR, 0, 0x3, stgen_parents),
};

#ifdef STM32MP1_CLOCK_TREE_INIT
/* define characteristic of PLL according type */
#define DIVN_MIN	24
static const struct stm32mp1_pll stm32mp1_pll[PLL_TYPE_NB] = {
	[PLL_800] = {
		.refclk_min = 4,
		.refclk_max = 16,
		.divn_max = 99,
		},
	[PLL_1600] = {
		.refclk_min = 8,
		.refclk_max = 16,
		.divn_max = 199,
		},
};
#endif /* STM32MP1_CLOCK_TREE_INIT */

static const struct stm32mp1_clk_pll stm32mp1_clk_pll[_PLL_NB] = {
	STM32MP1_CLK_PLL(_PLL1, PLL_1600,
			 RCC_RCK12SELR, RCC_PLL1CFGR1, RCC_PLL1CFGR2,
			 RCC_PLL1FRACR, RCC_PLL1CR, RCC_PLL1CSGR,
			 _HSI, _HSE, _UNKNOWN_ID, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL2, PLL_1600,
			 RCC_RCK12SELR, RCC_PLL2CFGR1, RCC_PLL2CFGR2,
			 RCC_PLL2FRACR, RCC_PLL2CR, RCC_PLL2CSGR,
			 _HSI, _HSE, _UNKNOWN_ID, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL3, PLL_800,
			 RCC_RCK3SELR, RCC_PLL3CFGR1, RCC_PLL3CFGR2,
			 RCC_PLL3FRACR, RCC_PLL3CR, RCC_PLL3CSGR,
			 _HSI, _HSE, _CSI, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL4, PLL_800,
			 RCC_RCK4SELR, RCC_PLL4CFGR1, RCC_PLL4CFGR2,
			 RCC_PLL4FRACR, RCC_PLL4CR, RCC_PLL4CSGR,
			 _HSI, _HSE, _CSI, _I2S_CKIN),
};

/* Prescaler table lookups for clock computation */
/* div = /1 /2 /4 /8 / 16 /64 /128 /512 */
static const u8 stm32mp1_mcu_div[16] = {
	0, 1, 2, 3, 4, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9
};

/* div = /1 /2 /4 /8 /16 : same divider for pmu and apbx*/
#define stm32mp1_mpu_div stm32mp1_mpu_apbx_div
#define stm32mp1_apbx_div stm32mp1_mpu_apbx_div
static const u8 stm32mp1_mpu_apbx_div[8] = {
	0, 1, 2, 3, 4, 4, 4, 4
};

/* div = /1 /2 /3 /4 */
static const u8 stm32mp1_axi_div[8] = {
	1, 2, 3, 4, 4, 4, 4, 4
};

#ifdef DEBUG
static const char * const stm32mp1_clk_parent_name[_PARENT_NB] = {
	[_HSI] = "HSI",
	[_HSE] = "HSE",
	[_CSI] = "CSI",
	[_LSI] = "LSI",
	[_LSE] = "LSE",
	[_I2S_CKIN] = "I2S_CKIN",
	[_HSI_KER] = "HSI_KER",
	[_HSE_KER] = "HSE_KER",
	[_HSE_KER_DIV2] = "HSE_KER_DIV2",
	[_CSI_KER] = "CSI_KER",
	[_PLL1_P] = "PLL1_P",
	[_PLL1_Q] = "PLL1_Q",
	[_PLL1_R] = "PLL1_R",
	[_PLL2_P] = "PLL2_P",
	[_PLL2_Q] = "PLL2_Q",
	[_PLL2_R] = "PLL2_R",
	[_PLL3_P] = "PLL3_P",
	[_PLL3_Q] = "PLL3_Q",
	[_PLL3_R] = "PLL3_R",
	[_PLL4_P] = "PLL4_P",
	[_PLL4_Q] = "PLL4_Q",
	[_PLL4_R] = "PLL4_R",
	[_ACLK] = "ACLK",
	[_PCLK1] = "PCLK1",
	[_PCLK2] = "PCLK2",
	[_PCLK3] = "PCLK3",
	[_PCLK4] = "PCLK4",
	[_PCLK5] = "PCLK5",
	[_HCLK6] = "KCLK6",
	[_HCLK2] = "HCLK2",
	[_CK_PER] = "CK_PER",
	[_CK_MPU] = "CK_MPU",
	[_CK_MCU] = "CK_MCU",
	[_USB_PHY_48] = "USB_PHY_48"
};

static const char * const stm32mp1_clk_parent_sel_name[_PARENT_SEL_NB] = {
	[_I2C12_SEL] = "I2C12",
	[_I2C35_SEL] = "I2C35",
	[_I2C46_SEL] = "I2C46",
	[_UART6_SEL] = "UART6",
	[_UART24_SEL] = "UART24",
	[_UART35_SEL] = "UART35",
	[_UART78_SEL] = "UART78",
	[_SDMMC12_SEL] = "SDMMC12",
	[_SDMMC3_SEL] = "SDMMC3",
	[_ETH_SEL] = "ETH",
	[_QSPI_SEL] = "QSPI",
	[_FMC_SEL] = "FMC",
	[_USBPHY_SEL] = "USBPHY",
	[_USBO_SEL] = "USBO",
	[_STGEN_SEL] = "STGEN"
};
#endif

static const struct stm32mp1_clk_data stm32mp1_data = {
	.gate = stm32mp1_clk_gate,
	.sel = stm32mp1_clk_sel,
	.pll = stm32mp1_clk_pll,
	.nb_gate = ARRAY_SIZE(stm32mp1_clk_gate),
};

static ulong stm32mp1_clk_get_fixed(struct stm32mp1_clk_priv *priv, int idx)
{
	if (idx >= NB_OSC) {
		debug("%s: clk id %d not found\n", __func__, idx);
		return 0;
	}

	debug("%s: clk id %d = %x : %ld kHz\n", __func__, idx,
	      (u32)priv->osc[idx], priv->osc[idx] / 1000);

	return priv->osc[idx];
}

static int stm32mp1_clk_get_id(struct stm32mp1_clk_priv *priv, unsigned long id)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i, nb_clks = priv->data->nb_gate;

	for (i = 0; i < nb_clks; i++) {
		if (gate[i].index == id)
			break;
	}

	if (i == nb_clks) {
		printf("%s: clk id %d not found\n", __func__, (u32)id);
		return -EINVAL;
	}

	return i;
}

static int stm32mp1_clk_get_sel(struct stm32mp1_clk_priv *priv,
				int i)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;

	if (gate[i].sel > _PARENT_SEL_NB) {
		printf("%s: parents for clk id %d not found\n",
		       __func__, i);
		return -EINVAL;
	}

	return gate[i].sel;
}

static int stm32mp1_clk_get_fixed_parent(struct stm32mp1_clk_priv *priv,
					 int i)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;

	if (gate[i].fixed == _UNKNOWN_ID)
		return -ENOENT;

	return gate[i].fixed;
}

static int stm32mp1_clk_get_parent(struct stm32mp1_clk_priv *priv,
				   unsigned long id)
{
	const struct stm32mp1_clk_sel *sel = priv->data->sel;
	int i;
	int s, p;

	for (i = 0; i < ARRAY_SIZE(stm32mp1_clks); i++)
		if (stm32mp1_clks[i][0] == id)
			return stm32mp1_clks[i][1];

	i = stm32mp1_clk_get_id(priv, id);
	if (i < 0)
		return i;

	p = stm32mp1_clk_get_fixed_parent(priv, i);
	if (p >= 0 && p < _PARENT_NB)
		return p;

	s = stm32mp1_clk_get_sel(priv, i);
	if (s < 0)
		return s;

	p = (readl(priv->base + sel[s].offset) >> sel[s].src) & sel[s].msk;

	if (p < sel[s].nb_parent) {
#ifdef DEBUG
		debug("%s: %s clock is the parent %s of clk id %d\n", __func__,
		      stm32mp1_clk_parent_name[sel[s].parent[p]],
		      stm32mp1_clk_parent_sel_name[s],
		      (u32)id);
#endif
		return sel[s].parent[p];
	}

	pr_err("%s: no parents defined for clk id %d\n",
	       __func__, (u32)id);

	return -EINVAL;
}

static ulong stm32mp1_read_pll_freq(struct stm32mp1_clk_priv *priv,
				    int pll_id, int div_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	int divm, divn, divy, src;
	ulong refclk, dfout;
	u32 selr, cfgr1, cfgr2, fracr;
	const u8 shift[_DIV_NB] = {
		[_DIV_P] = RCC_PLLNCFGR2_DIVP_SHIFT,
		[_DIV_Q] = RCC_PLLNCFGR2_DIVQ_SHIFT,
		[_DIV_R] = RCC_PLLNCFGR2_DIVR_SHIFT };

	debug("%s(%d, %d)\n", __func__, pll_id, div_id);
	if (div_id > _DIV_NB)
		return 0;

	selr = readl(priv->base + pll[pll_id].rckxselr);
	cfgr1 = readl(priv->base + pll[pll_id].pllxcfgr1);
	cfgr2 = readl(priv->base + pll[pll_id].pllxcfgr2);
	fracr = readl(priv->base + pll[pll_id].pllxfracr);

	debug("PLL%d : selr=%x cfgr1=%x cfgr2=%x fracr=%x\n",
	      pll_id, selr, cfgr1, cfgr2, fracr);

	divm = (cfgr1 & (RCC_PLLNCFGR1_DIVM_MASK)) >> RCC_PLLNCFGR1_DIVM_SHIFT;
	divn = cfgr1 & RCC_PLLNCFGR1_DIVN_MASK;
	divy = (cfgr2 >> shift[div_id]) & RCC_PLLNCFGR2_DIVX_MASK;

	debug("        DIVN=%d DIVM=%d DIVY=%d\n", divn, divm, divy);

	src = selr & RCC_SELR_SRC_MASK;
	refclk = stm32mp1_clk_get_fixed(priv, pll[pll_id].refclk[src]);

	debug("        refclk = %d kHz\n", (u32)(refclk / 1000));

	/*
	 * For: PLL1 & PLL2 => VCO is * 2 but ck_pll_y is also / 2
	 * So same final result than PLL2 et 4
	 * with FRACV :
	 *   Fck_pll_y = Fck_ref * ((DIVN + 1) + FRACV / 2^13)
	 *               / (DIVM + 1) * (DIVy + 1)
	 * without FRACV
	 *   Fck_pll_y = Fck_ref * ((DIVN + 1) / (DIVM + 1) *(DIVy + 1)
	 */
	if (fracr & RCC_PLLNFRACR_FRACLE) {
		u32 fracv = (fracr & RCC_PLLNFRACR_FRACV_MASK)
			    >> RCC_PLLNFRACR_FRACV_SHIFT;
		dfout = (ulong)lldiv((unsigned long long)refclk *
				     (((divn + 1) << 13) + fracv),
				     ((unsigned long long)(divm + 1) *
				      (divy + 1)) << 13);
	} else {
		dfout = (ulong)(refclk * (divn + 1) / (divm + 1) * (divy + 1));
	}
	debug("        => dfout = %d kHz\n", (u32)(dfout / 1000));

	return dfout;
}

static ulong stm32mp1_clk_get(struct stm32mp1_clk_priv *priv, int p)
{
	u32 reg;
	ulong clock = 0;

	switch (p) {
	case _CK_MPU:
	/* MPU sub system */
		reg = readl(priv->base + RCC_MPCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_MPCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_MPCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_MPCKSELR_PLL:
		case RCC_MPCKSELR_PLL_MPUDIV:
			clock = stm32mp1_read_pll_freq(priv, _PLL1, _DIV_P);
			if (p == RCC_MPCKSELR_PLL_MPUDIV) {
				reg = readl(priv->base + RCC_MPCKDIVR);
				clock /= stm32mp1_mpu_div[reg &
							  RCC_MPUDIV_MASK];
			}
			break;
		}
		break;
	/* AXI sub system */
	case _ACLK:
	case _HCLK2:
	case _HCLK6:
	case _PCLK4:
	case _PCLK5:
		reg = readl(priv->base + RCC_ASSCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_ASSCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_ASSCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_ASSCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(priv, _PLL2, _DIV_P);
			break;
		}

		/* System clock divider */
		reg = readl(priv->base + RCC_AXIDIVR);
		clock /= stm32mp1_axi_div[reg & RCC_AXIDIV_MASK];

		switch (p) {
		case _PCLK4:
			reg = readl(priv->base + RCC_APB4DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK5:
			reg = readl(priv->base + RCC_APB5DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		default:
			break;
		}
		break;
	/* MCU sub system */
	case _CK_MCU:
	case _PCLK1:
	case _PCLK2:
	case _PCLK3:
		reg = readl(priv->base + RCC_MSSCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_MSSCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_MSSCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_MSSCKSELR_CSI:
			clock = stm32mp1_clk_get_fixed(priv, _CSI);
			break;
		case RCC_MSSCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(priv, _PLL3, _DIV_P);
			break;
		}

		/* MCU clock divider */
		reg = readl(priv->base + RCC_MCUDIVR);
		clock >>= stm32mp1_mcu_div[reg & RCC_MCUDIV_MASK];

		switch (p) {
		case _PCLK1:
			reg = readl(priv->base + RCC_APB1DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK2:
			reg = readl(priv->base + RCC_APB2DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK3:
			reg = readl(priv->base + RCC_APB3DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _CK_MCU:
		default:
			break;
		}
		break;
	case _CK_PER:
		reg = readl(priv->base + RCC_CPERCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_CPERCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_CPERCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_CPERCKSELR_CSI:
			clock = stm32mp1_clk_get_fixed(priv, _CSI);
			break;
		}
		break;
	case _HSI:
	case _HSI_KER:
		clock = stm32mp1_clk_get_fixed(priv, _HSI);
		break;
	case _CSI:
	case _CSI_KER:
		clock = stm32mp1_clk_get_fixed(priv, _CSI);
		break;
	case _HSE:
	case _HSE_KER:
	case _HSE_KER_DIV2:
		clock = stm32mp1_clk_get_fixed(priv, _HSE);
		if (p == _HSE_KER_DIV2)
			clock >>= 1;
		break;
	case _LSI:
		clock = stm32mp1_clk_get_fixed(priv, _LSI);
		break;
	case _LSE:
		clock = stm32mp1_clk_get_fixed(priv, _LSE);
		break;
	/* PLL */
	case _PLL1_P:
	case _PLL1_Q:
	case _PLL1_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL1, p - _PLL1_P);
		break;
	case _PLL2_P:
	case _PLL2_Q:
	case _PLL2_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL2, p - _PLL2_P);
		break;
	case _PLL3_P:
	case _PLL3_Q:
	case _PLL3_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL3, p - _PLL3_P);
		break;
	case _PLL4_P:
	case _PLL4_Q:
	case _PLL4_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL4, p - _PLL4_P);
		break;
	/* other */
	case _USB_PHY_48:
		clock = stm32mp1_clk_get_fixed(priv, _USB_PHY_48);
		break;

	default:
		break;
	}

	debug("%s(%d) clock = %lx : %ld kHz\n",
	      __func__, p, clock, clock / 1000);

	return clock;
}

static int stm32mp1_clk_enable(struct clk *clk)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(clk->dev);
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i = stm32mp1_clk_get_id(priv, clk->id);

	if (i < 0)
		return i;

	if (gate[i].set_clr)
		writel(BIT(gate[i].bit), priv->base + gate[i].offset);
	else
		setbits_le32(priv->base + gate[i].offset, BIT(gate[i].bit));

	debug("%s: id clock %d has been enabled\n", __func__, (u32)clk->id);

	return 0;
}

static int stm32mp1_clk_disable(struct clk *clk)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(clk->dev);
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i = stm32mp1_clk_get_id(priv, clk->id);

	if (i < 0)
		return i;

	if (gate[i].set_clr)
		writel(BIT(gate[i].bit),
		       priv->base + gate[i].offset
		       + RCC_MP_ENCLRR_OFFSET);
	else
		clrbits_le32(priv->base + gate[i].offset, BIT(gate[i].bit));

	debug("%s: id clock %d has been disabled\n", __func__, (u32)clk->id);

	return 0;
}

static ulong stm32mp1_clk_get_rate(struct clk *clk)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(clk->dev);
	int p = stm32mp1_clk_get_parent(priv, clk->id);
	ulong rate;

	if (p < 0)
		return 0;

	rate = stm32mp1_clk_get(priv, p);

#ifdef DEBUG
	debug("%s: computed rate for id clock %d is %d (parent is %s)\n",
	      __func__, (u32)clk->id, (u32)rate, stm32mp1_clk_parent_name[p]);
#endif
	return rate;
}

static void stm32mp1_osc_clk_init(const char *name,
				  struct stm32mp1_clk_priv *priv,
				  int index)
{
	struct clk clk;
	struct udevice *dev = NULL;

	priv->osc[index] = 0;
	clk.id = 0;
	if (!uclass_get_device_by_name(UCLASS_CLK, name, &dev)) {
		if (clk_request(dev, &clk))
			pr_err("%s request", name);
		else
			priv->osc[index] = clk_get_rate(&clk);
	}
	priv->osc_dev[index] = dev;
}

static void stm32mp1_osc_init(struct udevice *dev)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(dev);
	int i;
	const char *name[NB_OSC] = {
		[_LSI] = "clk-lsi",
		[_LSE] = "clk-lse",
		[_HSI] = "clk-hsi",
		[_HSE] = "clk-hse",
		[_CSI] = "clk-csi",
		[_I2S_CKIN] = "i2s_ckin",
		[_USB_PHY_48] = "ck_usbo_48m"};

	for (i = 0; i < NB_OSC; i++) {
		stm32mp1_osc_clk_init(name[i], priv, i);
		debug("%d: %s => %x\n", i, name[i], (u32)priv->osc[i]);
	}
}

static int stm32mp1_clk_probe(struct udevice *dev)
{
	int result = 0;
	struct stm32mp1_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev->parent);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = (void *)&stm32mp1_data;

	if (!priv->data->gate || !priv->data->sel ||
	    !priv->data->pll)
		return -EINVAL;

	stm32mp1_osc_init(dev);

	return result;
}

static const struct clk_ops stm32mp1_clk_ops = {
	.enable = stm32mp1_clk_enable,
	.disable = stm32mp1_clk_disable,
	.get_rate = stm32mp1_clk_get_rate,
};

static const struct udevice_id stm32mp1_clk_ids[] = {
	{ .compatible = "st,stm32mp1-rcc-clk" },
	{ }
};

U_BOOT_DRIVER(stm32mp1_clock) = {
	.name = "stm32mp1_clk",
	.id = UCLASS_CLK,
	.of_match = stm32mp1_clk_ids,
	.ops = &stm32mp1_clk_ops,
	.priv_auto_alloc_size = sizeof(struct stm32mp1_clk_priv),
	.probe = stm32mp1_clk_probe,
};
