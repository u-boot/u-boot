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
	setbits_le32(&ccm->ahb_gate1, 0x1 << AHB_GATE_OFFSET_GMAC);

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
#ifdef CONFIG_BANANAPI
	setbits_le32(&ccm->gmac_clk_cfg, 0x3 << 10);
#endif

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

#ifdef CONFIG_RGMII
	return designware_initialize(SUNXI_GMAC_BASE, PHY_INTERFACE_MODE_RGMII);
#else
	return designware_initialize(SUNXI_GMAC_BASE, PHY_INTERFACE_MODE_MII);
#endif
}
