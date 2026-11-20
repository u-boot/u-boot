// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Max Merchel
 */

#include <fsl_esdhc_imx.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/spi.h>
#include <asm-generic/sections.h>

#include "tqma6.h"
#include "tqma6_emmc.h"
#include "../common/tq_bb.h"
#include "../common/tq_som.h"

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* set the default clock gate to save power */
	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

/* eMMC on USDHC3 always present */
static const iomux_v3_cfg_t tqma6_usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT4__SD3_DATA4 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT5__SD3_DATA5 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT6__SD3_DATA6 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT7__SD3_DATA7 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	/* eMMC reset */
	MX6_PAD_SD3_RST__SD3_RESET |	MUX_PAD_CTRL(GPIO_OUT_PAD_CTRL),
};

/*
 * According to board_mmc_init() the following map is done:
 * (U-Boot device node)    (Physical Port)
 * mmc0                    eMMC (SD3) on TQMa6
 * mmc1 .. n               optional slots used on baseboard
 */
static struct fsl_esdhc_cfg tqma6_usdhc_cfg = {
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
		/* eMMC/uSDHC3 is never WP */
		ret = 0;
	else
		ret = tq_bb_board_mmc_getwp(mmc);

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(tqma6_usdhc3_pads,
					 ARRAY_SIZE(tqma6_usdhc3_pads));
	tqma6_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	if (fsl_esdhc_initialize(bis, &tqma6_usdhc_cfg)) {
		puts("Warning: failed to initialize eMMC dev\n");
	} else {
		struct mmc *mmc = find_mmc_device(0);

		if (mmc)
			mmc_set_dsr(mmc, TQMA6_EMMC_DSR);
	}

	tq_bb_board_mmc_init(bis);

	return 0;
}

static const iomux_v3_cfg_t tqma6_ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	/* SS1 */
	MX6_PAD_EIM_D19__GPIO3_IO19 | (MUX_PAD_CTRL(SPI_PAD_CTRL) |
		MUX_MODE_SION),
};

#define TQMA6_SF_CS_GPIO IMX_GPIO_NR(3, 19)

void tqma6_iomuxc_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(tqma6_ecspi1_pads,
					 ARRAY_SIZE(tqma6_ecspi1_pads));

	gpio_direction_output(TQMA6_SF_CS_GPIO, 1);
}

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	return ((bus == CONFIG_SF_DEFAULT_BUS) &&
		(cs == CONFIG_SF_DEFAULT_CS)) ? TQMA6_SF_CS_GPIO : -1;
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/* setup clock gating */
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* setup AXI */
	gpr_init();

	timer_init();

	tqma6_iomuxc_spi();

	/* iomux and setup of uart */
	board_early_init_f();

	/* DDR initialization */
	tq_som_ram_init();
}

