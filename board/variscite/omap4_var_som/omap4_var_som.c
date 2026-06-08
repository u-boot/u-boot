// SPDX-License-Identifier: GPL-2.0+
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/emif.h>
#include <asm/global_data.h>
#include <asm/mach-types.h>
#include <asm-generic/gpio.h>
#include <env.h>
#include <init.h>
#include <linux/delay.h>
#include <log.h>
#include <serial.h>

#include "omap4_var_som_mux.h"

#define VAR_SOM_REV_GPIO		   52

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	"Board: OMAP4 VAR-SOM-OM44\n"
};

struct omap4_scrm_regs *const scrm = (struct omap4_scrm_regs *)0x4a30a000;

/**
 * @brief board_init
 *
 * Return: 0
 */
int board_init(void)
{
	gpmc_init();

	gd->bd->bi_arch_number = MACH_TYPE_OMAP4_VAR_SOM;
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	return 0;
}

static const struct emif_regs emif_regs_hynix_kdpm_400_mhz_1cs = {
	.sdram_config_init		= 0x80000eb2,
	.sdram_config			= 0x80001ab2,
	.ref_ctrl			= 0x000005cd,
	.sdram_tim1			= 0x10cb0622,
	.sdram_tim2			= 0x20350d52,
	.sdram_tim3			= 0x00b1431f,
	.read_idle_ctrl			= 0x000501ff,
	.zq_config			= 0x500b3214,
	.temp_alert_config		= 0x58016893,
	.emif_ddr_phy_ctlr_1_init	= 0x049ffff5,
	.emif_ddr_phy_ctlr_1		= 0x049ff418
};

const struct emif_regs emif_regs_hynix_kdpm_400_mhz_2cs = {
	.sdram_config_init		= 0x80000eb9,
	.sdram_config			= 0x80001ab9,
	.ref_ctrl			= 0x00000618,
	.sdram_tim1			= 0x10eb0662,
	.sdram_tim2			= 0x20370dd2,
	.sdram_tim3			= 0x00b1c33f,
	.read_idle_ctrl			= 0x000501ff,
	.zq_config			= 0xd00b3214,
	.temp_alert_config		= 0xd8016893,
	.emif_ddr_phy_ctlr_1_init	= 0x049ffff5,
	.emif_ddr_phy_ctlr_1		= 0x049ff418
};

/*
 * emif_get_reg_dump() - emif_get_reg_dump strong function
 *
 * @emif_nr - emif base
 * @regs - reg dump of timing values
 *
 * Strong function to override emif_get_reg_dump weak function in sdram_elpida.c
 */
void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	u32 rev;

	gpio_direction_input(VAR_SOM_REV_GPIO);
	rev = gpio_get_value(VAR_SOM_REV_GPIO);

	if (rev == 1)
		*regs = &emif_regs_hynix_kdpm_400_mhz_1cs;
	else
		*regs = &emif_regs_hynix_kdpm_400_mhz_2cs;
}

void emif_get_dmm_regs(const struct dmm_lisa_map_regs
						**dmm_lisa_regs)
{
	u32 omap_rev = omap_revision();

	if (omap_rev == OMAP4430_ES1_0)
		*dmm_lisa_regs = &lisa_map_2G_x_1_x_2;
	else if (omap_rev == OMAP4430_ES2_3)
		*dmm_lisa_regs = &lisa_map_2G_x_2_x_2;
	else if (omap_rev < OMAP4460_ES1_0)
		*dmm_lisa_regs = &lisa_map_2G_x_2_x_2;
	else
		*dmm_lisa_regs = &ma_lisa_map_2G_x_2_x_2;
}

void emif_get_device_timings(u32 emif_nr,
			     const struct lpddr2_device_timings **cs0_device_timings,
			     const struct lpddr2_device_timings **cs1_device_timings)
{
	/* Identical devices on EMIF1 & EMIF2 */
	*cs0_device_timings = &elpida_2G_S4_timings;
	*cs1_device_timings = NULL;
}

/**
 * @brief misc_init_r() - VAR-SOM configuration
 *
 * Configure VAR-SOM board specific configurations such as power configurations.
 *
 * Return: 0
 */
int misc_init_r(void)
{
	u32 auxclk, altclksrc;

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

	altclksrc = readl(&scrm->altclksrc);

	/* Activate alternate system clock supplier */
	altclksrc &= ~ALTCLKSRC_MODE_MASK;
	altclksrc |= ALTCLKSRC_MODE_ACTIVE;

	/* enable clocks */
	altclksrc |= ALTCLKSRC_ENABLE_INT_MASK | ALTCLKSRC_ENABLE_EXT_MASK;

	writel(altclksrc, &scrm->altclksrc);

	return 0;
}

void set_muxconf_regs(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		do_set_mux((*ctrl)->control_padconf_core_base,
			   core_padconf_array_essential,
			   sizeof(core_padconf_array_essential) /
			   sizeof(struct pad_conf_entry));

		do_set_mux((*ctrl)->control_padconf_wkup_base,
			   wkup_padconf_array_essential,
			   sizeof(wkup_padconf_array_essential) /
			   sizeof(struct pad_conf_entry));
	}
}

int board_mmc_init(struct bd_info *bis)
{
	if (IS_ENABLED(CONFIG_MMC))
		return omap_mmc_init(0, 0, 0, -1, -1);
}
