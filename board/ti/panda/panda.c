/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 * Steve Sakoman  <steve@sakoman.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/clocks.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>

#include "panda_mux_data.h"

#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/arch/ehci.h>
#include <asm/ehci-omap.h>
#endif

#define PANDA_ULPI_PHY_TYPE_GPIO       182

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	"Board: OMAP4 Panda\n"
};

struct omap4_scrm_regs *const scrm = (struct omap4_scrm_regs *)0x4a30a000;

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	gpmc_init();

	gd->bd->bi_arch_number = MACH_TYPE_OMAP4_PANDA;
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return 0;
}

/**
 * @brief misc_init_r - Configure Panda board specific configurations
 * such as power configurations, ethernet initialization as phase2 of
 * boot sequence
 *
 * @return 0
 */
int misc_init_r(void)
{
	int phy_type;
	u32 auxclk, altclksrc;

	/* EHCI is not supported on ES1.0 */
	if (omap_revision() == OMAP4430_ES1_0)
		return 0;

	gpio_direction_input(PANDA_ULPI_PHY_TYPE_GPIO);
	phy_type = gpio_get_value(PANDA_ULPI_PHY_TYPE_GPIO);

	if (phy_type == 1) {
		/* ULPI PHY supplied by auxclk3 derived from sys_clk */
		debug("ULPI PHY supplied by auxclk3\n");

		auxclk = readl(&scrm->auxclk3);
		/* Select sys_clk */
		auxclk &= ~AUXCLK_SRCSELECT_MASK;
		auxclk |=  AUXCLK_SRCSELECT_SYS_CLK << AUXCLK_SRCSELECT_SHIFT;
		/* Set the divisor to 2 */
		auxclk &= ~AUXCLK_CLKDIV_MASK;
		auxclk |= AUXCLK_CLKDIV_2 << AUXCLK_CLKDIV_SHIFT;
		/* Request auxilary clock #3 */
		auxclk |= AUXCLK_ENABLE_MASK;

		writel(auxclk, &scrm->auxclk3);
       } else {
		/* ULPI PHY supplied by auxclk1 derived from PER dpll */
		debug("ULPI PHY supplied by auxclk1\n");

		auxclk = readl(&scrm->auxclk1);
		/* Select per DPLL */
		auxclk &= ~AUXCLK_SRCSELECT_MASK;
		auxclk |=  AUXCLK_SRCSELECT_PER_DPLL << AUXCLK_SRCSELECT_SHIFT;
		/* Set the divisor to 16 */
		auxclk &= ~AUXCLK_CLKDIV_MASK;
		auxclk |= AUXCLK_CLKDIV_16 << AUXCLK_CLKDIV_SHIFT;
		/* Request auxilary clock #3 */
		auxclk |= AUXCLK_ENABLE_MASK;

		writel(auxclk, &scrm->auxclk1);
	}

	altclksrc = readl(&scrm->altclksrc);

	/* Activate alternate system clock supplier */
	altclksrc &= ~ALTCLKSRC_MODE_MASK;
	altclksrc |= ALTCLKSRC_MODE_ACTIVE;

	/* enable clocks */
	altclksrc |= ALTCLKSRC_ENABLE_INT_MASK | ALTCLKSRC_ENABLE_EXT_MASK;

	writel(altclksrc, &scrm->altclksrc);

	return 0;
}

void set_muxconf_regs_essential(void)
{
	do_set_mux(CONTROL_PADCONF_CORE, core_padconf_array_essential,
		   sizeof(core_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));

	do_set_mux(CONTROL_PADCONF_WKUP, wkup_padconf_array_essential,
		   sizeof(wkup_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));

	if (omap_revision() >= OMAP4460_ES1_0)
		do_set_mux(CONTROL_PADCONF_WKUP,
				 wkup_padconf_array_essential_4460,
				 sizeof(wkup_padconf_array_essential_4460) /
				 sizeof(struct pad_conf_entry));
}

void set_muxconf_regs_non_essential(void)
{
	do_set_mux(CONTROL_PADCONF_CORE, core_padconf_array_non_essential,
		   sizeof(core_padconf_array_non_essential) /
		   sizeof(struct pad_conf_entry));

	if (omap_revision() < OMAP4460_ES1_0)
		do_set_mux(CONTROL_PADCONF_CORE,
				core_padconf_array_non_essential_4430,
				sizeof(core_padconf_array_non_essential_4430) /
				sizeof(struct pad_conf_entry));
	else
		do_set_mux(CONTROL_PADCONF_CORE,
				core_padconf_array_non_essential_4460,
				sizeof(core_padconf_array_non_essential_4460) /
				sizeof(struct pad_conf_entry));

	do_set_mux(CONTROL_PADCONF_WKUP, wkup_padconf_array_non_essential,
		   sizeof(wkup_padconf_array_non_essential) /
		   sizeof(struct pad_conf_entry));

	if (omap_revision() < OMAP4460_ES1_0)
		do_set_mux(CONTROL_PADCONF_WKUP,
				wkup_padconf_array_non_essential_4430,
				sizeof(wkup_padconf_array_non_essential_4430) /
				sizeof(struct pad_conf_entry));
}

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_GENERIC_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0);
	return 0;
}
#endif

#ifdef CONFIG_USB_EHCI

static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

int ehci_hcd_init(void)
{
	int ret;
	unsigned int utmi_clk;

	/* Now we can enable our port clocks */
	utmi_clk = readl((void *)CM_L3INIT_HSUSBHOST_CLKCTRL);
	utmi_clk |= HSUSBHOST_CLKCTRL_CLKSEL_UTMI_P1_MASK;
	sr32((void *)CM_L3INIT_HSUSBHOST_CLKCTRL, 0, 32, utmi_clk);

	ret = omap_ehci_hcd_init(&usbhs_bdata);
	if (ret < 0)
		return ret;

	return 0;
}

int ehci_hcd_stop(void)
{
	return omap_ehci_hcd_stop();
}
#endif

/*
 * get_board_rev() - get board revision
 */
u32 get_board_rev(void)
{
	return 0x20;
}
