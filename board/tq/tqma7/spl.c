// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Alexander Feilke
 */

#include <fsl_esdhc_imx.h>
#include <hang.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch-mx7/mx7d_pins.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>

#include "../common/tq_bb.h"
#include "../common/tq_som.h"

DECLARE_GLOBAL_DATA_PTR;

#define USDHC_PAD_CTRL		(PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_CMD_PAD_CTRL	(PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_CLK_PAD_CTRL	(PAD_CTL_DSE_3P3V_98OHM | \
	PAD_CTL_SRE_SLOW | PAD_CTL_PUS_PU47KOHM)

#define USDHC_STROBE_PAD_CTRL	(PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PD100KOHM)

/* eMMC on USDHCI3 always present */
static const iomux_v3_cfg_t tqma7_usdhc3_pads[] = {
	NEW_PAD_CTRL(MX7D_PAD_SD3_CLK__SD3_CLK,		USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_CMD__SD3_CMD,		USDHC_CMD_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA0__SD3_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA1__SD3_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA2__SD3_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA3__SD3_DATA3,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA4__SD3_DATA4,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA5__SD3_DATA5,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA6__SD3_DATA6,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_DATA7__SD3_DATA7,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD3_STROBE__SD3_STROBE,	USDHC_STROBE_PAD_CTRL),
};

static struct fsl_esdhc_cfg tqma7_usdhc3_cfg = {
	.esdhc_base = USDHC3_BASE_ADDR,
	.max_bus_width = 8,
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		/* eMMC/uSDHC3 is always present */
		ret = 1;
	else
		ret = tq_bb_board_mmc_getcd(mmc);

	return ret;
}

int board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		/* eMMC/uSDHC3 is not WP */
		ret = 0;
	else
		ret = tq_bb_board_mmc_getwp(mmc);

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(tqma7_usdhc3_pads,
					 ARRAY_SIZE(tqma7_usdhc3_pads));

	tqma7_usdhc3_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	if (fsl_esdhc_initialize(bis, &tqma7_usdhc3_cfg))
		puts("Warning: failed to initialize eMMC dev\n");

	tq_bb_board_mmc_init(bis);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return tq_bb_board_init();
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	timer_init();

	tq_bb_board_early_init_f();

	preloader_console_init();

	/* DDR initialization */
	tq_som_ram_init();
}

