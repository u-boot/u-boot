#include <common.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>

int sunxi_gmac_initialize(bd_t *bis)
{
	int pin;
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* Set up clock gating */
#ifndef CONFIG_MACH_SUN6I
	setbits_le32(&ccm->ahb_gate1, 0x1 << AHB_GATE_OFFSET_GMAC);
#else
	setbits_le32(&ccm->ahb_reset0_cfg, 0x1 << AHB_RESET_OFFSET_GMAC);
	setbits_le32(&ccm->ahb_gate0, 0x1 << AHB_GATE_OFFSET_GMAC);
#endif

	/* Set MII clock */
#ifdef CONFIG_RGMII
	setbits_le32(&ccm->gmac_clk_cfg, CCM_GMAC_CTRL_TX_CLK_SRC_INT_RGMII |
		CCM_GMAC_CTRL_GPIT_RGMII);
#else
	setbits_le32(&ccm->gmac_clk_cfg, CCM_GMAC_CTRL_TX_CLK_SRC_MII |
		CCM_GMAC_CTRL_GPIT_MII);
#endif

	/*
	 * In order for the gmac nic to work reliable on the Bananapi, we
	 * need to set bits 10-12 GTXDC "GMAC Transmit Clock Delay Chain"
	 * of the GMAC clk register to 3.
	 */
#ifdef CONFIG_TARGET_BANANAPI
	setbits_le32(&ccm->gmac_clk_cfg, 0x3 << 10);
#endif

#ifndef CONFIG_MACH_SUN6I
	/* Configure pin mux settings for GMAC */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(16); pin++) {
#ifdef CONFIG_RGMII
		/* skip unused pins in RGMII mode */
		if (pin == SUNXI_GPA(9) || pin == SUNXI_GPA(14))
			continue;
#endif
		sunxi_gpio_set_cfgpin(pin, SUN7I_GPA0_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
#elif defined CONFIG_RGMII
	/* Configure sun6i RGMII mode pin mux settings */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(3); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
	for (pin = SUNXI_GPA(9); pin <= SUNXI_GPA(14); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
	for (pin = SUNXI_GPA(19); pin <= SUNXI_GPA(20); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
	for (pin = SUNXI_GPA(25); pin <= SUNXI_GPA(27); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
		sunxi_gpio_set_drv(pin, 3);
	}
#elif defined CONFIG_GMII
	/* Configure sun6i GMII mode pin mux settings */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(27); pin++) {
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
		sunxi_gpio_set_drv(pin, 2);
	}
#else
	/* Configure sun6i MII mode pin mux settings */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(3); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
	for (pin = SUNXI_GPA(8); pin <= SUNXI_GPA(9); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
	for (pin = SUNXI_GPA(11); pin <= SUNXI_GPA(14); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
	for (pin = SUNXI_GPA(19); pin <= SUNXI_GPA(24); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
	for (pin = SUNXI_GPA(26); pin <= SUNXI_GPA(27); pin++)
		sunxi_gpio_set_cfgpin(pin, SUN6I_GPA0_GMAC);
#endif

#ifdef CONFIG_RGMII
	return designware_initialize(SUNXI_GMAC_BASE, PHY_INTERFACE_MODE_RGMII);
#elif defined CONFIG_GMII
	return designware_initialize(SUNXI_GMAC_BASE, PHY_INTERFACE_MODE_GMII);
#else
	return designware_initialize(SUNXI_GMAC_BASE, PHY_INTERFACE_MODE_MII);
#endif
}
