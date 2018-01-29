/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <stm32_rcc.h>

#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/stm32_pwr.h>

#include <dt-bindings/mfd/stm32f7-rcc.h>

#define RCC_CR_HSION			BIT(0)
#define RCC_CR_HSEON			BIT(16)
#define RCC_CR_HSERDY			BIT(17)
#define RCC_CR_HSEBYP			BIT(18)
#define RCC_CR_CSSON			BIT(19)
#define RCC_CR_PLLON			BIT(24)
#define RCC_CR_PLLRDY			BIT(25)
#define RCC_CR_PLLSAION			BIT(28)
#define RCC_CR_PLLSAIRDY		BIT(29)

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

#define RCC_PLLCFGR_PLLSAIN_MASK	GENMASK(14, 6)
#define RCC_PLLCFGR_PLLSAIP_MASK	GENMASK(17, 16)
#define RCC_PLLSAICFGR_PLLSAIN_SHIFT	6
#define RCC_PLLSAICFGR_PLLSAIP_SHIFT	16
#define RCC_PLLSAICFGR_PLLSAIP_4	BIT(16)
#define RCC_PLLSAICFGR_PLLSAIQ_4	BIT(26)
#define RCC_PLLSAICFGR_PLLSAIR_2	BIT(29)

#define RCC_DCKCFGRX_CK48MSEL		BIT(27)
#define RCC_DCKCFGRX_SDMMC1SEL		BIT(28)
#define RCC_DCKCFGR2_SDMMC2SEL		BIT(29)

/*
 * RCC AHB1ENR specific definitions
 */
#define RCC_AHB1ENR_ETHMAC_EN		BIT(25)
#define RCC_AHB1ENR_ETHMAC_TX_EN	BIT(26)
#define RCC_AHB1ENR_ETHMAC_RX_EN	BIT(27)

/*
 * RCC APB1ENR specific definitions
 */
#define RCC_APB1ENR_TIM2EN		BIT(0)
#define RCC_APB1ENR_PWREN		BIT(28)

/*
 * RCC APB2ENR specific definitions
 */
#define RCC_APB2ENR_SYSCFGEN		BIT(14)
#define RCC_APB2ENR_SAI1EN		BIT(22)

enum periph_clock {
	TIMER2_CLOCK_CFG,
};

static const struct stm32_clk_info stm32f4_clk_info = {
	/* 180 MHz */
	.sys_pll_psc = {
		.pll_n = 360,
		.pll_p = 2,
		.pll_q = 8,
		.ahb_psc = AHB_PSC_1,
		.apb1_psc = APB_PSC_4,
		.apb2_psc = APB_PSC_2,
	},
	.has_overdrive = false,
	.v2 = false,
};

static const struct stm32_clk_info stm32f7_clk_info = {
	/* 200 MHz */
	.sys_pll_psc = {
		.pll_n = 400,
		.pll_p = 2,
		.pll_q = 8,
		.ahb_psc = AHB_PSC_1,
		.apb1_psc = APB_PSC_4,
		.apb2_psc = APB_PSC_2,
	},
	.has_overdrive = true,
	.v2 = true,
};

struct stm32_clk {
	struct stm32_rcc_regs *base;
	struct stm32_pwr_regs *pwr_regs;
	struct stm32_clk_info info;
	unsigned long hse_rate;
};

static int configure_clocks(struct udevice *dev)
{
	struct stm32_clk *priv = dev_get_priv(dev);
	struct stm32_rcc_regs *regs = priv->base;
	struct stm32_pwr_regs *pwr = priv->pwr_regs;
	struct pll_psc *sys_pll_psc = &priv->info.sys_pll_psc;
	u32 pllsaicfgr = 0;

	/* Reset RCC configuration */
	setbits_le32(&regs->cr, RCC_CR_HSION);
	writel(0, &regs->cfgr); /* Reset CFGR */
	clrbits_le32(&regs->cr, (RCC_CR_HSEON | RCC_CR_CSSON
		| RCC_CR_PLLON | RCC_CR_PLLSAION));
	writel(0x24003010, &regs->pllcfgr); /* Reset value from RM */
	clrbits_le32(&regs->cr, RCC_CR_HSEBYP);
	writel(0, &regs->cir); /* Disable all interrupts */

	/* Configure for HSE+PLL operation */
	setbits_le32(&regs->cr, RCC_CR_HSEON);
	while (!(readl(&regs->cr) & RCC_CR_HSERDY))
		;

	setbits_le32(&regs->cfgr, ((
		sys_pll_psc->ahb_psc << RCC_CFGR_HPRE_SHIFT)
		| (sys_pll_psc->apb1_psc << RCC_CFGR_PPRE1_SHIFT)
		| (sys_pll_psc->apb2_psc << RCC_CFGR_PPRE2_SHIFT)));

	/* Configure the main PLL */
	setbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLSRC); /* pll source HSE */
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLM_MASK,
			sys_pll_psc->pll_m << RCC_PLLCFGR_PLLM_SHIFT);
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLN_MASK,
			sys_pll_psc->pll_n << RCC_PLLCFGR_PLLN_SHIFT);
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLP_MASK,
			((sys_pll_psc->pll_p >> 1) - 1) << RCC_PLLCFGR_PLLP_SHIFT);
	clrsetbits_le32(&regs->pllcfgr, RCC_PLLCFGR_PLLQ_MASK,
			sys_pll_psc->pll_q << RCC_PLLCFGR_PLLQ_SHIFT);

	/* Configure the SAI PLL to get a 48 MHz source */
	pllsaicfgr = RCC_PLLSAICFGR_PLLSAIR_2 | RCC_PLLSAICFGR_PLLSAIQ_4 |
		     RCC_PLLSAICFGR_PLLSAIP_4;
	pllsaicfgr |= 192 << RCC_PLLSAICFGR_PLLSAIN_SHIFT;
	writel(pllsaicfgr, &regs->pllsaicfgr);

	/* Enable the main PLL */
	setbits_le32(&regs->cr, RCC_CR_PLLON);
	while (!(readl(&regs->cr) & RCC_CR_PLLRDY))
		;

	if (priv->info.v2) { /*stm32f7 case */
		/* select PLLSAI as 48MHz clock source */
		setbits_le32(&regs->dckcfgr2, RCC_DCKCFGRX_CK48MSEL);

		/* select 48MHz as SDMMC1 clock source */
		clrbits_le32(&regs->dckcfgr2, RCC_DCKCFGRX_SDMMC1SEL);

		/* select 48MHz as SDMMC2 clock source */
		clrbits_le32(&regs->dckcfgr2, RCC_DCKCFGR2_SDMMC2SEL);
	} else  { /* stm32f4 case */
		/* select PLLSAI as 48MHz clock source */
		setbits_le32(&regs->dckcfgr, RCC_DCKCFGRX_CK48MSEL);

		/* select 48MHz as SDMMC1 clock source */
		clrbits_le32(&regs->dckcfgr, RCC_DCKCFGRX_SDMMC1SEL);
	}

	/* Enable the SAI PLL */
	setbits_le32(&regs->cr, RCC_CR_PLLSAION);
	while (!(readl(&regs->cr) & RCC_CR_PLLSAIRDY))
		;

	setbits_le32(&regs->apb1enr, RCC_APB1ENR_PWREN);

	if (priv->info.has_overdrive) {
		/*
		 * Enable high performance mode
		 * System frequency up to 200 MHz
		 */
		setbits_le32(&pwr->cr1, PWR_CR1_ODEN);
		/* Infinite wait! */
		while (!(readl(&pwr->csr1) & PWR_CSR1_ODRDY))
			;
		/* Enable the Over-drive switch */
		setbits_le32(&pwr->cr1, PWR_CR1_ODSWEN);
		/* Infinite wait! */
		while (!(readl(&pwr->csr1) & PWR_CSR1_ODSWRDY))
			;
	}

	stm32_flash_latency_cfg(5);
	clrbits_le32(&regs->cfgr, (RCC_CFGR_SW0 | RCC_CFGR_SW1));
	setbits_le32(&regs->cfgr, RCC_CFGR_SW_PLL);

	while ((readl(&regs->cfgr) & RCC_CFGR_SWS_MASK) !=
			RCC_CFGR_SWS_PLL)
		;
	/* gate the SAI clock, needed for MMC 1&2 clocks */
	setbits_le32(&regs->apb2enr, RCC_APB2ENR_SAI1EN);

#ifdef CONFIG_ETH_DESIGNWARE
	/* gate the SYSCFG clock, needed to set RMII ethernet interface */
	setbits_le32(&regs->apb2enr, RCC_APB2ENR_SYSCFGEN);
#endif

	return 0;
}

static unsigned long stm32_clk_pll48clk_rate(struct stm32_clk *priv,
					     u32 sysclk)
{
	struct stm32_rcc_regs *regs = priv->base;
	u16 pllq, pllm, pllsain, pllsaip;
	bool pllsai;

	pllq = (readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLQ_MASK)
	       >> RCC_PLLCFGR_PLLQ_SHIFT;

	if (priv->info.v2) /*stm32f7 case */
		pllsai = readl(&regs->dckcfgr2) & RCC_DCKCFGRX_CK48MSEL;
	else
		pllsai = readl(&regs->dckcfgr) & RCC_DCKCFGRX_CK48MSEL;

	if (pllsai) {
		/* PLL48CLK is selected from PLLSAI, get PLLSAI value */
		pllm = (readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLM_MASK);
		pllsain = ((readl(&regs->pllsaicfgr) & RCC_PLLCFGR_PLLSAIN_MASK)
			>> RCC_PLLSAICFGR_PLLSAIN_SHIFT);
		pllsaip = ((((readl(&regs->pllsaicfgr) & RCC_PLLCFGR_PLLSAIP_MASK)
			>> RCC_PLLSAICFGR_PLLSAIP_SHIFT) + 1) << 1);
		return ((priv->hse_rate / pllm) * pllsain) / pllsaip;
	}
	/* PLL48CLK is selected from PLLQ */
	return sysclk / pllq;
}

static unsigned long stm32_clk_get_rate(struct clk *clk)
{
	struct stm32_clk *priv = dev_get_priv(clk->dev);
	struct stm32_rcc_regs *regs = priv->base;
	u32 sysclk = 0;
	u32 shift = 0;
	u16 pllm, plln, pllp;
	/* Prescaler table lookups for clock computation */
	u8 ahb_psc_table[16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
	};
	u8 apb_psc_table[8] = {
		0, 0, 0, 0, 1, 2, 3, 4
	};

	if ((readl(&regs->cfgr) & RCC_CFGR_SWS_MASK) ==
			RCC_CFGR_SWS_PLL) {
		pllm = (readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLM_MASK);
		plln = ((readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLN_MASK)
			>> RCC_PLLCFGR_PLLN_SHIFT);
		pllp = ((((readl(&regs->pllcfgr) & RCC_PLLCFGR_PLLP_MASK)
			>> RCC_PLLCFGR_PLLP_SHIFT) + 1) << 1);
		sysclk = ((priv->hse_rate / pllm) * plln) / pllp;
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
	/* APB1 CLOCK */
	case STM32F7_APB1_CLOCK(TIM2) ... STM32F7_APB1_CLOCK(UART8):
		shift = apb_psc_table[(
			(readl(&regs->cfgr) & RCC_CFGR_APB1_PSC_MASK)
			>> RCC_CFGR_PPRE1_SHIFT)];
		return sysclk >>= shift;
	/* APB2 CLOCK */
	case STM32F7_APB2_CLOCK(TIM1) ... STM32F7_APB2_CLOCK(LTDC):
		/*
		 * particular case for SDMMC1 and SDMMC2 :
		 * 48Mhz source clock can be from main PLL or from
		 * SAI PLL
		 */
		switch (clk->id) {
		case STM32F7_APB2_CLOCK(SDMMC1):
			if (readl(&regs->dckcfgr2) & RCC_DCKCFGRX_SDMMC1SEL)
				/* System clock is selected as SDMMC1 clock */
				return sysclk;
			else
				return stm32_clk_pll48clk_rate(priv, sysclk);
			break;
		case STM32F7_APB2_CLOCK(SDMMC2):
			if (readl(&regs->dckcfgr2) & RCC_DCKCFGR2_SDMMC2SEL)
				/* System clock is selected as SDMMC2 clock */
				return sysclk;
			else
				return stm32_clk_pll48clk_rate(priv, sysclk);
			break;
		}

		shift = apb_psc_table[(
			(readl(&regs->cfgr) & RCC_CFGR_APB2_PSC_MASK)
			>> RCC_CFGR_PPRE2_SHIFT)];
		return sysclk >>= shift;
	default:
		pr_err("clock index %ld out of range\n", clk->id);
		return -EINVAL;
	}
}

static ulong stm32_set_rate(struct clk *clk, ulong rate)
{
	return 0;
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
	case TIMER2_CLOCK_CFG:
		setbits_le32(&STM32_RCC->apb1enr, RCC_APB1ENR_TIM2EN);
		break;
	default:
		break;
	}
}

static int stm32_clk_probe(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	struct udevice *fixed_clock_dev = NULL;
	struct clk clk;
	int err;

	debug("%s\n", __func__);

	struct stm32_clk *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct stm32_rcc_regs *)addr;

	switch (dev_get_driver_data(dev)) {
	case STM32F4:
		memcpy(&priv->info, &stm32f4_clk_info,
		       sizeof(struct stm32_clk_info));
		break;
	case STM32F7:
		memcpy(&priv->info, &stm32f7_clk_info,
		       sizeof(struct stm32_clk_info));
		break;
	default:
		return -EINVAL;
	}

	/* retrieve HSE frequency (external oscillator) */
	err = uclass_get_device_by_name(UCLASS_CLK, "clk-hse",
					&fixed_clock_dev);

	if (err) {
		pr_err("Can't find fixed clock (%d)", err);
		return err;
	}

	err = clk_request(fixed_clock_dev, &clk);
	if (err) {
		pr_err("Can't request %s clk (%d)", fixed_clock_dev->name,
		       err);
		return err;
	}

	/*
	 * set pllm factor accordingly to the external oscillator
	 * frequency (HSE). For STM32F4 and STM32F7, we want VCO
	 * freq at 1MHz
	 * if input PLL frequency is 25Mhz, divide it by 25
	 */
	clk.id = 0;
	priv->hse_rate = clk_get_rate(&clk);

	if (priv->hse_rate < 1000000) {
		pr_err("%s: unexpected HSE clock rate = %ld \"n", __func__,
		       priv->hse_rate);
		return -EINVAL;
	}

	priv->info.sys_pll_psc.pll_m = priv->hse_rate / 1000000;

	if (priv->info.has_overdrive) {
		err = dev_read_phandle_with_args(dev, "st,syscfg", NULL, 0, 0,
						 &args);
		if (err) {
			debug("%s: can't find syscon device (%d)\n", __func__,
			      err);
			return err;
		}

		priv->pwr_regs = (struct stm32_pwr_regs *)ofnode_get_addr(args.node);
	}

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
	.set_rate	= stm32_set_rate,
};

U_BOOT_DRIVER(stm32fx_clk) = {
	.name			= "stm32fx_rcc_clock",
	.id			= UCLASS_CLK,
	.ops			= &stm32_clk_ops,
	.probe			= stm32_clk_probe,
	.priv_auto_alloc_size	= sizeof(struct stm32_clk),
	.flags			= DM_FLAG_PRE_RELOC,
};
