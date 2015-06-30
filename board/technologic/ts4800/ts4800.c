/*
 * (C) Copyright 2015 Savoir-faire Linux Inc.
 *
 * Derived from MX51EVK code by
 *   Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx51.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/imx-common/mx5_video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <mc13892.h>

#include "ts4800.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

u32 get_board_rev(void)
{
	u32 rev = get_cpu_rev();
	if (!gpio_get_value(IMX_GPIO_NR(1, 22)))
		rev |= BOARD_REV_2_0 << BOARD_VER_OFFSET;
	return rev;
}

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_DSE_HIGH)

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		MX51_PAD_UART1_RXD__UART1_RXD,
		MX51_PAD_UART1_TXD__UART1_TXD,
		NEW_PAD_CTRL(MX51_PAD_UART1_RTS__UART1_RTS, UART_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_UART1_CTS__UART1_CTS, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_0__GPIO1_0,
						NO_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(1, 0));
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_6__GPIO1_6,
						NO_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(1, 6));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(1, 0));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(1, 6));

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_SD1_CMD__SD1_CMD, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_CLK__SD1_CLK, PAD_CTL_DSE_MAX |
			PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA0__SD1_DATA0, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA1__SD1_DATA1, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA2__SD1_DATA2, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA3__SD1_DATA3, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_0__SD1_CD, PAD_CTL_HYS),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_1__SD1_WP, PAD_CTL_HYS),
	};

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	imx_iomux_v3_setup_multiple_pads(sd1_pads, ARRAY_SIZE(sd1_pads));

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int checkboard(void)
{
	puts("Board: TS4800\n");

	return 0;
}

void hw_watchdog_reset(void)
{
	struct ts4800_wtd_regs *wtd = (struct ts4800_wtd_regs *) (TS4800_SYSCON_BASE + 0xE);
	/* feed the watchdog for another 10s */
	writew(0x2, &wtd->feed);
}

void hw_watchdog_init(void)
{
	return;
}
