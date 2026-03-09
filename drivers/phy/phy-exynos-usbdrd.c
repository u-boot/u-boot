// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Samsung Exynos SoC series USB DRD PHY driver.
 * Based on Linux kernel PHY driver: drivers/phy/samsung/phy-exynos5-usbdrd.c
 */

#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>

/* Offset of PMU register controlling USB PHY output isolation */
#define EXYNOS_USBDRD_PHY_CONTROL		0x0704
#define EXYNOS_PHY_ENABLE			BIT(0)
#define EXYNOS7870_PHY_ENABLE			BIT(1)

/* Exynos USB PHY registers */
#define EXYNOS5_FSEL_9MHZ6			0x0
#define EXYNOS5_FSEL_10MHZ			0x1
#define EXYNOS5_FSEL_12MHZ			0x2
#define EXYNOS5_FSEL_19MHZ2			0x3
#define EXYNOS5_FSEL_20MHZ			0x4
#define EXYNOS5_FSEL_24MHZ			0x5
#define EXYNOS5_FSEL_26MHZ			0x6
#define EXYNOS5_FSEL_50MHZ			0x7

/* Exynos5: USB DRD PHY registers */
#define EXYNOS5_DRD_LINKSYSTEM			0x04
#define LINKSYSTEM_XHCI_VERSION_CONTROL		BIT(27)
#define LINKSYSTEM_FORCE_VBUSVALID		BIT(8)
#define LINKSYSTEM_FORCE_BVALID			BIT(7)
#define LINKSYSTEM_FLADJ			GENMASK(6, 1)

#define EXYNOS5_DRD_PHYUTMI			0x08
#define PHYUTMI_UTMI_SUSPEND_COM_N		BIT(12)
#define PHYUTMI_UTMI_L1_SUSPEND_COM_N		BIT(11)
#define PHYUTMI_VBUSVLDEXTSEL			BIT(10)
#define PHYUTMI_VBUSVLDEXT			BIT(9)
#define PHYUTMI_TXBITSTUFFENH			BIT(8)
#define PHYUTMI_TXBITSTUFFEN			BIT(7)
#define PHYUTMI_OTGDISABLE			BIT(6)
#define PHYUTMI_IDPULLUP			BIT(5)
#define PHYUTMI_DRVVBUS				BIT(4)
#define PHYUTMI_DPPULLDOWN			BIT(3)
#define PHYUTMI_DMPULLDOWN			BIT(2)
#define PHYUTMI_FORCESUSPEND			BIT(1)
#define PHYUTMI_FORCESLEEP			BIT(0)

#define EXYNOS5_DRD_PHYCLKRST			0x10
#define PHYCLKRST_EN_UTMISUSPEND		BIT(31)
#define PHYCLKRST_SSC_REFCLKSEL			GENMASK(30, 23)
#define PHYCLKRST_SSC_RANGE			GENMASK(22, 21)
#define PHYCLKRST_SSC_EN			BIT(20)
#define PHYCLKRST_REF_SSP_EN			BIT(19)
#define PHYCLKRST_REF_CLKDIV2			BIT(18)
#define PHYCLKRST_MPLL_MULTIPLIER		GENMASK(17, 11)
#define PHYCLKRST_MPLL_MULTIPLIER_100MHZ_REF	0x19
#define PHYCLKRST_MPLL_MULTIPLIER_50M_REF	0x32
#define PHYCLKRST_MPLL_MULTIPLIER_24MHZ_REF	0x68
#define PHYCLKRST_MPLL_MULTIPLIER_20MHZ_REF	0x7d
#define PHYCLKRST_MPLL_MULTIPLIER_19200KHZ_REF	0x02
#define PHYCLKRST_FSEL_PIPE			GENMASK(10, 8)
#define PHYCLKRST_FSEL_UTMI			GENMASK(7, 5)
#define PHYCLKRST_FSEL_PAD_100MHZ		0x27
#define PHYCLKRST_FSEL_PAD_24MHZ		0x2a
#define PHYCLKRST_FSEL_PAD_20MHZ		0x31
#define PHYCLKRST_FSEL_PAD_19_2MHZ		0x38
#define PHYCLKRST_RETENABLEN			BIT(4)
#define PHYCLKRST_REFCLKSEL			GENMASK(3, 2)
#define PHYCLKRST_REFCLKSEL_PAD_REFCLK		0x2
#define PHYCLKRST_REFCLKSEL_EXT_REFCLK		0x3
#define PHYCLKRST_PORTRESET			BIT(1)
#define PHYCLKRST_COMMONONN			BIT(0)

#define EXYNOS5_DRD_PHYPARAM0			0x1c
#define PHYPARAM0_REF_USE_PAD			BIT(31)
#define PHYPARAM0_REF_LOSLEVEL			GENMASK(30, 26)
#define PHYPARAM0_REF_LOSLEVEL_VAL		0x9
#define PHYPARAM0_TXVREFTUNE			GENMASK(25, 22)
#define PHYPARAM0_TXRISETUNE			GENMASK(21, 20)
#define PHYPARAM0_TXRESTUNE			GENMASK(19, 18)
#define PHYPARAM0_TXPREEMPPULSETUNE		BIT(17)
#define PHYPARAM0_TXPREEMPAMPTUNE		GENMASK(16, 15)
#define PHYPARAM0_TXHSXVTUNE			GENMASK(14, 13)
#define PHYPARAM0_TXFSLSTUNE			GENMASK(12, 9)
#define PHYPARAM0_SQRXTUNE			GENMASK(8, 6)
#define PHYPARAM0_OTGTUNE			GENMASK(5, 3)
#define PHYPARAM0_COMPDISTUNE			GENMASK(2, 0)

#define EXYNOS5_DRD_LINKPORT			0x44
#define LINKPORT_HOST_U3_PORT_DISABLE		BIT(8)
#define LINKPORT_HOST_U2_PORT_DISABLE		BIT(7)
#define LINKPORT_HOST_PORT_OVCR_U3		BIT(5)
#define LINKPORT_HOST_PORT_OVCR_U2		BIT(4)
#define LINKPORT_HOST_PORT_OVCR_U3_SEL		BIT(3)
#define LINKPORT_HOST_PORT_OVCR_U2_SEL		BIT(2)

/* Exynos7870: USB DRD PHY registers */
#define EXYNOS7870_DRD_HSPHYCTRL		0x54
#define HSPHYCTRL_PHYSWRSTALL			BIT(31)
#define HSPHYCTRL_SIDDQ				BIT(6)
#define HSPHYCTRL_PHYSWRST			BIT(0)

#define EXYNOS7870_DRD_HSPHYPLLTUNE		0x70
#define HSPHYPLLTUNE_PLL_B_TUNE			BIT(6)
#define HSPHYPLLTUNE_PLL_I_TUNE			GENMASK(5, 4)
#define HSPHYPLLTUNE_PLL_P_TUNE			GENMASK(3, 0)

/* Exynos850: USB DRD PHY registers */
#define EXYNOS850_DRD_LINKCTRL			0x04
#define LINKCTRL_FORCE_QACT			BIT(8)
#define LINKCTRL_BUS_FILTER_BYPASS		GENMASK(7, 4)

#define EXYNOS850_DRD_CLKRST			0x20
#define CLKRST_LINK_SW_RST			BIT(0)
#define CLKRST_PORT_RST				BIT(1)
#define CLKRST_PHY_SW_RST			BIT(3)

#define EXYNOS850_DRD_SSPPLLCTL			0x30
#define SSPPLLCTL_FSEL				GENMASK(2, 0)

#define EXYNOS850_DRD_UTMI			0x50
#define UTMI_FORCE_SLEEP			BIT(0)
#define UTMI_FORCE_SUSPEND			BIT(1)
#define UTMI_DM_PULLDOWN			BIT(2)
#define UTMI_DP_PULLDOWN			BIT(3)
#define UTMI_FORCE_BVALID			BIT(4)
#define UTMI_FORCE_VBUSVALID			BIT(5)

#define EXYNOS850_DRD_HSP			0x54
#define HSP_COMMONONN				BIT(8)
#define HSP_EN_UTMISUSPEND			BIT(9)
#define HSP_VBUSVLDEXT				BIT(12)
#define HSP_VBUSVLDEXTSEL			BIT(13)
#define HSP_FSV_OUT_EN				BIT(24)

#define EXYNOS850_DRD_HSP_TEST			0x5c
#define HSP_TEST_SIDDQ				BIT(24)

#define KHZ					1000
#define MHZ					(KHZ * KHZ)

enum exynos_usbdrd_phy_variant {
	EXYNOS7870_USBDRD_PHY,
	EXYNOS850_USBDRD_PHY,
};

/**
 * struct exynos_usbdrd_phy - driver data for Exynos USB PHY
 * @reg_phy: USB PHY controller register memory base
 * @clk: clock for register access
 * @core_clk: core clock for phy (ref clock)
 * @reg_pmu: regmap for PMU block
 * @extrefclk: frequency select settings when using 'separate reference clocks'
 * @variant: ID to uniquely distinguish USB PHY variant
 */
struct exynos_usbdrd_phy {
	void __iomem *reg_phy;
	struct clk *clk;
	struct clk *core_clk;
	struct regmap *reg_pmu;
	u32 extrefclk;
	enum exynos_usbdrd_phy_variant variant;
};

static void exynos_usbdrd_phy_isol(struct exynos_usbdrd_phy *phy_drd,
				   bool isolate)
{
	unsigned int mask = EXYNOS_PHY_ENABLE, val;

	if (!phy_drd->reg_pmu)
		return;

	if (phy_drd->variant == EXYNOS7870_USBDRD_PHY)
		mask = EXYNOS7870_PHY_ENABLE;

	val = isolate ? 0 : mask;
	regmap_update_bits(phy_drd->reg_pmu, EXYNOS_USBDRD_PHY_CONTROL,
			   mask, val);
}

/*
 * Convert the supplied clock rate to the value that can be written to the PHY
 * register.
 */
static unsigned int exynos_rate_to_clk(unsigned long rate, u32 *reg)
{
	switch (rate) {
	case 9600 * KHZ:
		*reg = EXYNOS5_FSEL_9MHZ6;
		break;
	case 10 * MHZ:
		*reg = EXYNOS5_FSEL_10MHZ;
		break;
	case 12 * MHZ:
		*reg = EXYNOS5_FSEL_12MHZ;
		break;
	case 19200 * KHZ:
		*reg = EXYNOS5_FSEL_19MHZ2;
		break;
	case 20 * MHZ:
		*reg = EXYNOS5_FSEL_20MHZ;
		break;
	case 24 * MHZ:
		*reg = EXYNOS5_FSEL_24MHZ;
		break;
	case 26 * MHZ:
		*reg = EXYNOS5_FSEL_26MHZ;
		break;
	case 50 * MHZ:
		*reg = EXYNOS5_FSEL_50MHZ;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static void exynos7870_usbdrd_utmi_init(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg;

	reg = readl(regs_base + EXYNOS5_DRD_PHYCLKRST);
	/* Use PADREFCLK as ref clock */
	reg &= ~PHYCLKRST_REFCLKSEL;
	reg |= FIELD_PREP(PHYCLKRST_REFCLKSEL, PHYCLKRST_REFCLKSEL_PAD_REFCLK);
	/* Select ref clock rate */
	reg &= ~PHYCLKRST_FSEL_UTMI;
	reg &= ~PHYCLKRST_FSEL_PIPE;
	reg |= FIELD_PREP(PHYCLKRST_FSEL_UTMI, phy_drd->extrefclk);
	/* Enable suspend and reset the port */
	reg |= PHYCLKRST_EN_UTMISUSPEND;
	reg |= PHYCLKRST_COMMONONN;
	reg |= PHYCLKRST_PORTRESET;
	writel(reg, regs_base + EXYNOS5_DRD_PHYCLKRST);
	udelay(10);

	/* Clear the port reset bit */
	reg &= ~PHYCLKRST_PORTRESET;
	writel(reg, regs_base + EXYNOS5_DRD_PHYCLKRST);

	/* Change PHY PLL tune value */
	reg = readl(regs_base + EXYNOS7870_DRD_HSPHYPLLTUNE);
	if (phy_drd->extrefclk == EXYNOS5_FSEL_24MHZ)
		reg |= HSPHYPLLTUNE_PLL_B_TUNE;
	else
		reg &= ~HSPHYPLLTUNE_PLL_B_TUNE;
	reg &= ~HSPHYPLLTUNE_PLL_P_TUNE;
	reg |= FIELD_PREP(HSPHYPLLTUNE_PLL_P_TUNE, 14);
	writel(reg, regs_base + EXYNOS7870_DRD_HSPHYPLLTUNE);

	/* High-Speed PHY control */
	reg = readl(regs_base + EXYNOS7870_DRD_HSPHYCTRL);
	reg &= ~HSPHYCTRL_SIDDQ;
	reg &= ~HSPHYCTRL_PHYSWRST;
	reg &= ~HSPHYCTRL_PHYSWRSTALL;
	writel(reg, regs_base + EXYNOS7870_DRD_HSPHYCTRL);
	udelay(500);

	reg = readl(regs_base + EXYNOS5_DRD_LINKSYSTEM);
	/*
	 * Setting the Frame length Adj value[6:1] to default 0x20
	 * See xHCI 1.0 spec, 5.2.4
	 */
	reg |= LINKSYSTEM_XHCI_VERSION_CONTROL;
	reg &= ~LINKSYSTEM_FLADJ;
	reg |= FIELD_PREP(LINKSYSTEM_FLADJ, 0x20);
	/* Set VBUSVALID signal as the VBUS pad is not used */
	reg |= LINKSYSTEM_FORCE_BVALID;
	reg |= LINKSYSTEM_FORCE_VBUSVALID;
	writel(reg, regs_base + EXYNOS5_DRD_LINKSYSTEM);

	reg = readl(regs_base + EXYNOS5_DRD_PHYUTMI);
	/* Release force_sleep & force_suspend */
	reg &= ~PHYUTMI_FORCESLEEP;
	reg &= ~PHYUTMI_FORCESUSPEND;
	/* DP/DM pull down control */
	reg &= ~PHYUTMI_DMPULLDOWN;
	reg &= ~PHYUTMI_DPPULLDOWN;
	reg &= ~PHYUTMI_DRVVBUS;
	/* Set DP-pull up as the VBUS pad is not used */
	reg |= PHYUTMI_VBUSVLDEXTSEL;
	reg |= PHYUTMI_VBUSVLDEXT;
	/* Disable OTG block and VBUS valid comparator */
	reg |= PHYUTMI_OTGDISABLE;
	writel(reg, regs_base + EXYNOS5_DRD_PHYUTMI);

	/* Configure OVC IO usage */
	reg = readl(regs_base + EXYNOS5_DRD_LINKPORT);
	reg |= LINKPORT_HOST_PORT_OVCR_U3_SEL | LINKPORT_HOST_PORT_OVCR_U2_SEL;
	writel(reg, regs_base + EXYNOS5_DRD_LINKPORT);

	/* High-Speed PHY swrst */
	reg = readl(regs_base + EXYNOS7870_DRD_HSPHYCTRL);
	reg |= HSPHYCTRL_PHYSWRST;
	writel(reg, regs_base + EXYNOS7870_DRD_HSPHYCTRL);
	udelay(20);

	/* Clear the PHY swrst bit */
	reg = readl(regs_base + EXYNOS7870_DRD_HSPHYCTRL);
	reg &= ~HSPHYCTRL_PHYSWRST;
	writel(reg, regs_base + EXYNOS7870_DRD_HSPHYCTRL);

	reg = readl(regs_base + EXYNOS5_DRD_PHYPARAM0);
	reg &= ~(PHYPARAM0_TXVREFTUNE | PHYPARAM0_TXRISETUNE |
		 PHYPARAM0_TXRESTUNE | PHYPARAM0_TXPREEMPPULSETUNE |
		 PHYPARAM0_TXPREEMPAMPTUNE | PHYPARAM0_TXHSXVTUNE |
		 PHYPARAM0_TXFSLSTUNE | PHYPARAM0_SQRXTUNE |
		 PHYPARAM0_OTGTUNE | PHYPARAM0_COMPDISTUNE);
	reg |= FIELD_PREP_CONST(PHYPARAM0_TXVREFTUNE, 14) |
	       FIELD_PREP_CONST(PHYPARAM0_TXRISETUNE, 1) |
	       FIELD_PREP_CONST(PHYPARAM0_TXRESTUNE, 3) |
	       FIELD_PREP_CONST(PHYPARAM0_TXPREEMPAMPTUNE, 0) |
	       FIELD_PREP_CONST(PHYPARAM0_TXHSXVTUNE, 0) |
	       FIELD_PREP_CONST(PHYPARAM0_TXFSLSTUNE, 3) |
	       FIELD_PREP_CONST(PHYPARAM0_SQRXTUNE, 6) |
	       FIELD_PREP_CONST(PHYPARAM0_OTGTUNE, 2) |
	       FIELD_PREP_CONST(PHYPARAM0_COMPDISTUNE, 3);
	writel(reg, regs_base + EXYNOS5_DRD_PHYPARAM0);
}

static void exynos850_usbdrd_utmi_init(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg;

	/*
	 * Disable HWACG (hardware auto clock gating control). This will force
	 * QACTIVE signal in Q-Channel interface to HIGH level, to make sure
	 * the PHY clock is not gated by the hardware.
	 */
	reg = readl(regs_base + EXYNOS850_DRD_LINKCTRL);
	reg |= LINKCTRL_FORCE_QACT;
	writel(reg, regs_base + EXYNOS850_DRD_LINKCTRL);

	/* Start PHY Reset (POR=high) */
	reg = readl(regs_base + EXYNOS850_DRD_CLKRST);
	reg |= CLKRST_PHY_SW_RST;
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);

	/* Enable UTMI+ */
	reg = readl(regs_base + EXYNOS850_DRD_UTMI);
	reg &= ~(UTMI_FORCE_SUSPEND | UTMI_FORCE_SLEEP | UTMI_DP_PULLDOWN |
		 UTMI_DM_PULLDOWN);
	writel(reg, regs_base + EXYNOS850_DRD_UTMI);

	/* Set PHY clock and control HS PHY */
	reg = readl(regs_base + EXYNOS850_DRD_HSP);
	reg |= HSP_EN_UTMISUSPEND | HSP_COMMONONN;
	writel(reg, regs_base + EXYNOS850_DRD_HSP);

	/* Set VBUS Valid and D+ pull-up control by VBUS pad usage */
	reg = readl(regs_base + EXYNOS850_DRD_LINKCTRL);
	reg |= FIELD_PREP(LINKCTRL_BUS_FILTER_BYPASS, 0xf);
	writel(reg, regs_base + EXYNOS850_DRD_LINKCTRL);

	reg = readl(regs_base + EXYNOS850_DRD_UTMI);
	reg |= UTMI_FORCE_BVALID | UTMI_FORCE_VBUSVALID;
	writel(reg, regs_base + EXYNOS850_DRD_UTMI);

	reg = readl(regs_base + EXYNOS850_DRD_HSP);
	reg |= HSP_VBUSVLDEXT | HSP_VBUSVLDEXTSEL;
	writel(reg, regs_base + EXYNOS850_DRD_HSP);

	reg = readl(regs_base + EXYNOS850_DRD_SSPPLLCTL);
	reg &= ~SSPPLLCTL_FSEL;
	switch (phy_drd->extrefclk) {
	case EXYNOS5_FSEL_50MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 7);
		break;
	case EXYNOS5_FSEL_26MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 6);
		break;
	case EXYNOS5_FSEL_24MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 2);
		break;
	case EXYNOS5_FSEL_20MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 1);
		break;
	case EXYNOS5_FSEL_19MHZ2:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 0);
		break;
	default:
		dev_warn(phy->dev, "unsupported ref clk: %#.2x\n",
			 phy_drd->extrefclk);
		break;
	}
	writel(reg, regs_base + EXYNOS850_DRD_SSPPLLCTL);

	/* Power up PHY analog blocks */
	reg = readl(regs_base + EXYNOS850_DRD_HSP_TEST);
	reg &= ~HSP_TEST_SIDDQ;
	writel(reg, regs_base + EXYNOS850_DRD_HSP_TEST);

	/* Finish PHY reset (POR=low) */
	udelay(10); /* required before doing POR=low */
	reg = readl(regs_base + EXYNOS850_DRD_CLKRST);
	reg &= ~(CLKRST_PHY_SW_RST | CLKRST_PORT_RST);
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);
	udelay(75); /* required after POR=low for guaranteed PHY clock */

	/* Disable single ended signal out */
	reg = readl(regs_base + EXYNOS850_DRD_HSP);
	reg &= ~HSP_FSV_OUT_EN;
	writel(reg, regs_base + EXYNOS850_DRD_HSP);
}

static void exynos7870_usbdrd_utmi_exit(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg;

	/*
	 * Disable the VBUS signal and the ID pull-up resistor.
	 * Enable force-suspend and force-sleep modes.
	 */
	reg = readl(regs_base + EXYNOS5_DRD_PHYUTMI);
	reg &= ~(PHYUTMI_DRVVBUS | PHYUTMI_VBUSVLDEXT | PHYUTMI_VBUSVLDEXTSEL);
	reg &= ~PHYUTMI_IDPULLUP;
	reg |= PHYUTMI_FORCESUSPEND | PHYUTMI_FORCESLEEP;
	writel(reg, regs_base + EXYNOS5_DRD_PHYUTMI);

	/* Power down PHY analog blocks */
	reg = readl(regs_base + EXYNOS7870_DRD_HSPHYCTRL);
	reg |= HSPHYCTRL_SIDDQ;
	writel(reg, regs_base + EXYNOS7870_DRD_HSPHYCTRL);

	/* Clear VBUSVALID signal as the VBUS pad is not used */
	reg = readl(regs_base + EXYNOS5_DRD_LINKSYSTEM);
	reg &= ~(LINKSYSTEM_FORCE_BVALID | LINKSYSTEM_FORCE_VBUSVALID);
	writel(reg, regs_base + EXYNOS5_DRD_LINKSYSTEM);
}

static void exynos850_usbdrd_utmi_exit(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg;

	/* Set PHY clock and control HS PHY */
	reg = readl(regs_base + EXYNOS850_DRD_UTMI);
	reg &= ~(UTMI_DP_PULLDOWN | UTMI_DM_PULLDOWN);
	reg |= UTMI_FORCE_SUSPEND | UTMI_FORCE_SLEEP;
	writel(reg, regs_base + EXYNOS850_DRD_UTMI);

	/* Power down PHY analog blocks */
	reg = readl(regs_base + EXYNOS850_DRD_HSP_TEST);
	reg |= HSP_TEST_SIDDQ;
	writel(reg, regs_base + EXYNOS850_DRD_HSP_TEST);

	/* Link reset */
	reg = readl(regs_base + EXYNOS850_DRD_CLKRST);
	reg |= CLKRST_LINK_SW_RST;
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);
	udelay(10); /* required before doing POR=low */
	reg &= ~CLKRST_LINK_SW_RST;
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);
}

static int exynos_usbdrd_phy_init(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	int ret;

	ret = clk_prepare_enable(phy_drd->clk);
	if (ret)
		return ret;

	switch (phy_drd->variant) {
	case EXYNOS7870_USBDRD_PHY:
		exynos7870_usbdrd_utmi_init(phy);
		break;
	case EXYNOS850_USBDRD_PHY:
		exynos850_usbdrd_utmi_init(phy);
		break;
	default:
		dev_err(phy->dev, "Failed to recognize phy variant\n");
	}

	clk_disable_unprepare(phy_drd->clk);

	return 0;
}

static int exynos_usbdrd_phy_exit(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	int ret;

	ret = clk_prepare_enable(phy_drd->clk);
	if (ret)
		return ret;

	switch (phy_drd->variant) {
	case EXYNOS7870_USBDRD_PHY:
		exynos7870_usbdrd_utmi_exit(phy);
		break;
	case EXYNOS850_USBDRD_PHY:
		exynos850_usbdrd_utmi_exit(phy);
		break;
	default:
		dev_err(phy->dev, "Failed to recognize phy variant\n");
	}

	clk_disable_unprepare(phy_drd->clk);

	return 0;
}

static int exynos_usbdrd_phy_power_on(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	int ret;

	dev_dbg(phy->dev, "Request to power_on usbdrd_phy phy\n");

	ret = clk_prepare_enable(phy_drd->core_clk);
	if (ret)
		return ret;

	/* Power-on PHY */
	exynos_usbdrd_phy_isol(phy_drd, false);

	return 0;
}

static int exynos_usbdrd_phy_power_off(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);

	dev_dbg(phy->dev, "Request to power_off usbdrd_phy phy\n");

	/* Power-off the PHY */
	exynos_usbdrd_phy_isol(phy_drd, true);

	clk_disable_unprepare(phy_drd->core_clk);

	return 0;
}

static int exynos_usbdrd_phy_init_clk(struct udevice *dev)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(dev);
	unsigned long ref_rate;
	int err;

	phy_drd->clk = devm_clk_get(dev, "phy");
	if (IS_ERR(phy_drd->clk)) {
		err = PTR_ERR(phy_drd->clk);
		dev_err(dev, "Failed to get phy clock (err=%d)\n", err);
		return err;
	}

	phy_drd->core_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(phy_drd->core_clk)) {
		err = PTR_ERR(phy_drd->core_clk);
		dev_err(dev, "Failed to get ref clock (err=%d)\n", err);
		return err;
	}

	ref_rate = clk_get_rate(phy_drd->core_clk);
	err = exynos_rate_to_clk(ref_rate, &phy_drd->extrefclk);
	if (err) {
		dev_err(dev, "Clock rate %lu not supported\n", ref_rate);
		return err;
	}

	return 0;
}

static int exynos_usbdrd_phy_probe(struct udevice *dev)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(dev);
	int err;

	phy_drd->reg_phy = dev_read_addr_ptr(dev);
	if (!phy_drd->reg_phy)
		return -EINVAL;

	err = exynos_usbdrd_phy_init_clk(dev);
	if (err)
		return err;

	phy_drd->reg_pmu = syscon_regmap_lookup_by_phandle(dev,
							  "samsung,pmu-syscon");
	if (IS_ERR(phy_drd->reg_pmu)) {
		err = PTR_ERR(phy_drd->reg_pmu);
		dev_err(dev, "Failed to lookup PMU regmap\n");
		return err;
	}

	phy_drd->variant = dev_get_driver_data(dev);

	return 0;
}

static const struct phy_ops exynos_usbdrd_phy_ops = {
	.init		= exynos_usbdrd_phy_init,
	.exit		= exynos_usbdrd_phy_exit,
	.power_on	= exynos_usbdrd_phy_power_on,
	.power_off	= exynos_usbdrd_phy_power_off,
};

static const struct udevice_id exynos_usbdrd_phy_of_match[] = {
	{
		.compatible = "samsung,exynos7870-usbdrd-phy",
		.data = EXYNOS7870_USBDRD_PHY,
	},
	{
		.compatible = "samsung,exynos850-usbdrd-phy",
		.data = EXYNOS850_USBDRD_PHY,
	},
	{ }
};

U_BOOT_DRIVER(exynos_usbdrd_phy) = {
	.name		= "exynos-usbdrd-phy",
	.id		= UCLASS_PHY,
	.of_match	= exynos_usbdrd_phy_of_match,
	.probe		= exynos_usbdrd_phy_probe,
	.ops		= &exynos_usbdrd_phy_ops,
	.priv_auto	= sizeof(struct exynos_usbdrd_phy),
};
