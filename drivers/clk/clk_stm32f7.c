/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/rcc.h>
#include <asm/arch/stm32.h>
#include <asm/arch/stm32_periph.h>

#include <dt-bindings/mfd/stm32f7-rcc.h>

#define RCC_CR_HSION			BIT(0)
#define RCC_CR_HSEON			BIT(16)
#define RCC_CR_HSERDY			BIT(17)
#define RCC_CR_HSEBYP			BIT(18)
#define RCC_CR_CSSON			BIT(19)
#define RCC_CR_PLLON			BIT(24)
#define RCC_CR_PLLRDY			BIT(25)

#define RCC_PLLCFGR_PLLM_MASK		GENMASK(5, 0)
#define RCC_PLLCFGR_PLLN_MASK		GENMASK(14, 6)
#define RCC_PLLCFGR_PLLP_MASK		GENMASK(17, 16)
#define RCC_PLLCFGR_PLLQ_MASK		GENMASK(27, 24)
#define RCC_PLLCFGR_PLLSRC		BIT(22)
#define RCC_PLLCFGR_PLLM_SHIFT		0
#define RCC_PLLCFGR_PLLN_SHIFT		6
#define RCC_PLLCFGR_PLLP_SHIFT		16
#define RCC_PLLCFGR_PLLQ_SHIFT		24

#define RCC_CFGR_AHB_PSC_MASK		GENMASK(7, 4)
#define RCC_CFGR_APB1_PSC_MASK		GENMASK(12, 10)
#define RCC_CFGR_APB2_PSC_MASK		GENMASK(15, 13)
#define RCC_CFGR_SW0			BIT(0)
#define RCC_CFGR_SW1			BIT(1)
#define RCC_CFGR_SW_MASK		GENMASK(1, 0)
#define RCC_CFGR_SW_HSI			0
#define RCC_CFGR_SW_HSE			RCC_CFGR_SW0
#define RCC_CFGR_SW_PLL			RCC_CFGR_SW1
#define RCC_CFGR_SWS0			BIT(2)
#define RCC_CFGR_SWS1			BIT(3)
#define RCC_CFGR_SWS_MASK		GENMASK(3, 2)
#define RCC_CFGR_SWS_HSI		0
#define RCC_CFGR_SWS_HSE		RCC_CFGR_SWS0
#define RCC_CFGR_SWS_PLL		RCC_CFGR_SWS1
#define RCC_CFGR_HPRE_SHIFT		4
#define RCC_CFGR_PPRE1_SHIFT		10
#define RCC_CFGR_PPRE2_SHIFT		13

/*
 * Offsets of some PWR registers
 */
#define PWR_CR1_ODEN			BIT(16)
#define PWR_CR1_ODSWEN			BIT(17)
#define PWR_CSR1_ODRDY			BIT(16)
#define PWR_CSR1_ODSWRDY		BIT(17)

struct pll_psc {
	u8	pll_m;
	u16	pll_n;
	u8	pll_p;
	u8	pll_q;
	u8	ahb_psc;
	u8	apb1_psc;
	u8	apb2_psc;
};

#define AHB_PSC_1			0
#define AHB_PSC_2			0x8
#define AHB_PSC_4			0x9
#define AHB_PSC_8			0xA
#define AHB_PSC_16			0xB
#define AHB_PSC_64			0xC
#define AHB_PSC_128			0xD
#define AHB_PSC_256			0xE
#define AHB_PSC_512			0xF

#define APB_PSC_1			0
#define APB_PSC_2			0x4
#define APB_PSC_4			0x5
#define APB_PSC_8			0x6
#define APB_PSC_16			0x7

struct stm32_clk {
	struct stm32_rcc_regs *base;
};

#if !defined(CONFIG_STM32_HSE_HZ)
#error "CONFIG_STM32_HSE_HZ not defined!"
#else
#if (CONFIG_STM32_HSE_HZ == 25000000)
#if (CONFIG_SYS_CLK_FREQ == 200000000)
/* 200 MHz */
struct pll_psc sys_pll_psc = {
	.pll_m = 25,
	.pll_n = 400,
	.pll_p = 2,
	.pll_q = 8,
	.ahb_psc = AHB_PSC_1,
	.apb1_psc = APB_PSC_4,
	.apb2_psc = APB_PSC_2
};
#endif
#else
#error "No PLL/Prescaler configuration for given CONFIG_STM32_HSE_HZ exists"
#endif
#endif

static int configure_clocks(struct udevice *dev)
{
	struct stm32_clk *priv = dev_get_priv(dev);
	struct stm32_rcc_regs *regs = priv->base;

	/* Reset RCC configuration */
	setbits_le32(&regs->cr, RCC_CR_HSION);
	writel(0, &regs->cfgr); /* Reset CFGR */
	clrbits_le32(&regs->cr, (RCC_CR_HSEON | RCC_CR_CSSON
		| RCC_CR_PLLON));
	writel(0x24003010, &regs->pllcfgr); /* Reset value from RM */
	clrbits_le32(&regs->cr, RCC_CR_HSEBYP);
	writel(0, &regs->cir); /* Disable all interrupts */

	/* Configure for HSE+PLL operation */
	setbits_le32(&regs->cr, RCC_CR_HSEON);
	while (!(readl(&regs->cr) & RCC_CR_HSERDY))
		;

	setbits_le32(&regs->cfgr, ((
		sys_pll_psc.ahb_psc << RCC_CFGR_HPRE_SHIFT)
		| (sys_pll_psc.apb1_psc << RCC_CFGR_PPRE1_SHIFT)
		| (sys_pll_psc.apb2_psc << RCC_CFGR_PPRE2_SHIFT)));

	/* Configure the main PLL */
	uint32_t pllcfgr = 0;
	pllcfgr = RCC_PLLCFGR_PLLSRC; /* pll source HSE */
	pllcfgr |= sys_pll_psc.pll_m << RCC_PLLCFGR_PLLM_SHIFT;
	pllcfgr |= sys_pll_psc.pll_n << RCC_PLLCFGR_PLLN_SHIFT;
	pllcfgr |= ((sys_pll_psc.pll_p >> 1) - 1) << RCC_PLLCFGR_PLLP_SHIFT;
	pllcfgr |= sys_pll_psc.pll_q << RCC_PLLCFGR_PLLQ_SHIFT;
	writel(pllcfgr, &regs->pllcfgr);

	/* Enable the main PLL */
	setbits_le32(&regs->cr, RCC_CR_PLLON);
	while (!(readl(&regs->cr) & RCC_CR_PLLRDY))
		;

	/* Enable high performance mode, System frequency up to 200 MHz */
	setbits_le32(&regs->apb1enr, RCC_APB1ENR_PWREN);
	setbits_le32(&STM32_PWR->cr1, PWR_CR1_ODEN);
	/* Infinite wait! */
	while (!(readl(&STM32_PWR->csr1) & PWR_CSR1_ODRDY))
		;
	/* Enable the Over-drive switch */
	setbits_le32(&STM32_PWR->cr1, PWR_CR1_ODSWEN);
	/* Infinite wait! */
	while (!(readl(&STM32_PWR->csr1) & PWR_CSR1_ODSWRDY))
		;

	stm32_flash_latency_cfg(5);
	clrbits_le32(&regs->cfgr, (RCC_CFGR_SW0 | RCC_CFGR_SW1));
	setbits_le32(&regs->cfgr, RCC_CFGR_SW_PLL);

	while ((readl(&regs->cfgr) & RCC_CFGR_SWS_MASK) !=
			RCC_CFGR_SWS_PLL)
		;

	return 0;
}

static unsigned long stm32_clk_get_rate(struct clk *clk)
{
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->base;
	u32 sysclk = 0;
	u32 shift = 0;
	/* Prescaler table lookups for clock computation */
	u8 ahb_psc_table[16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
	};
	u8 apb_psc_table[8] = {
		0, 0, 0, 0, 1, 2, 3, 4
	};

	if ((readl(&regs->cfgr) & RCC_CFGR_SWS_MASK) ==
			RCC_CFGR_SWS_PLL) {
		u16 pllm, plln, pllp;
		pllm = (readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLM_MASK);
		plln = ((readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLN_MASK)
			>> RCC_PLLCFGR_PLLN_SHIFT);
		pllp = ((((readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLP_MASK)
			>> RCC_PLLCFGR_PLLP_SHIFT) + 1) << 1);
		sysclk = ((CONFIG_STM32_HSE_HZ / pllm) * plln) / pllp;
	} else {
		return -EINVAL;
	}

	switch (clk->id) {
	/*
	 * AHB CLOCK: 3 x 32 bits consecutive registers are used :
	 * AHB1, AHB2 and AHB3
	 */
	case STM32F7_AHB1_CLOCK(GPIOA) ... STM32F7_AHB3_CLOCK(QSPI):
		shift = ahb_psc_table[(
			(readl(&regs->cfgr) & RCC_CFGR_AHB_PSC_MASK)
			>> RCC_CFGR_HPRE_SHIFT)];
		return sysclk >>= shift;
		break;
	/* APB1 CLOCK */
	case STM32F7_APB1_CLOCK(TIM2) ... STM32F7_APB1_CLOCK(UART8):
		shift = apb_psc_table[(
			(readl(&regs->cfgr) & RCC_CFGR_APB1_PSC_MASK)
			>> RCC_CFGR_PPRE1_SHIFT)];
		return sysclk >>= shift;
		break;
	/* APB2 CLOCK */
	case STM32F7_APB2_CLOCK(TIM1) ... STM32F7_APB2_CLOCK(LTDC):
		shift = apb_psc_table[(
			(readl(&regs->cfgr) & RCC_CFGR_APB2_PSC_MASK)
			>> RCC_CFGR_PPRE2_SHIFT)];
		return sysclk >>= shift;
		break;
	default:
		pr_err("clock index %ld out of range\n", clk->id);
		return -EINVAL;
		break;
	}
}

static int stm32_clk_enable(struct clk *clk)
{
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->base;
	u32 offset = clk->id / 32;
	u32 bit_index = clk->id % 32;

	debug("%s: clkid = %ld, offset from AHB1ENR is %d, bit_index = %d\n",
	      __func__, clk->id, offset, bit_index);
	setbits_le32(&regs->ahb1enr + offset, BIT(bit_index));

	return 0;
}

void clock_setup(int peripheral)
{
	switch (peripheral) {
	case SYSCFG_CLOCK_CFG:
		setbits_le32(&STM32_RCC->apb2enr, RCC_APB2ENR_SYSCFGEN);
		break;
	case TIMER2_CLOCK_CFG:
		setbits_le32(&STM32_RCC->apb1enr, RCC_APB1ENR_TIM2EN);
		break;
	case STMMAC_CLOCK_CFG:
		setbits_le32(&STM32_RCC->ahb1enr, RCC_AHB1ENR_ETHMAC_EN);
		setbits_le32(&STM32_RCC->ahb1enr, RCC_AHB1ENR_ETHMAC_RX_EN);
		setbits_le32(&STM32_RCC->ahb1enr, RCC_AHB1ENR_ETHMAC_TX_EN);
		break;
	default:
		break;
	}
}

static int stm32_clk_probe(struct udevice *dev)
{
	debug("%s: stm32_clk_probe\n", __func__);

	struct stm32_clk *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct stm32_rcc_regs *)addr;

	configure_clocks(dev);

	return 0;
}

static int stm32_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	debug("%s(clk=%p)\n", __func__, clk);

	if (args->args_count != 2) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = args->args[1];
	else
		clk->id = 0;

	return 0;
}

static struct clk_ops stm32_clk_ops = {
	.of_xlate	= stm32_clk_of_xlate,
	.enable		= stm32_clk_enable,
	.get_rate	= stm32_clk_get_rate,
};

static const struct udevice_id stm32_clk_ids[] = {
	{ .compatible = "st,stm32f42xx-rcc"},
	{}
};

U_BOOT_DRIVER(stm32f7_clk) = {
	.name			= "stm32f7_clk",
	.id			= UCLASS_CLK,
	.of_match		= stm32_clk_ids,
	.ops			= &stm32_clk_ops,
	.probe			= stm32_clk_probe,
	.priv_auto_alloc_size	= sizeof(struct stm32_clk),
	.flags			= DM_FLAG_PRE_RELOC,
};
