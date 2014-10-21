/*
 * Olimex MX23 Olinuxino board
 *
 * Copyright (C) 2013 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/iomux-mx23.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * Functions
 */
int board_early_init_f(void)
{
	/* IO0 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK0, 480000);

	/* SSP0 clock at 96MHz */
	mxs_set_sspclk(MXC_SSPCLK0, 96000, 0);

	return 0;
}

#ifdef CONFIG_CMD_USB
int board_ehci_hcd_init(int port)
{
	/* Enable LAN9512 (Maxi) or GL850G (Mini) USB HUB power. */
	gpio_direction_output(MX23_PAD_GPMI_ALE__GPIO_0_17, 1);
	udelay(100);
	return 0;
}

int board_ehci_hcd_exit(int port)
{
	/* Enable LAN9512 (Maxi) or GL850G (Mini) USB HUB power. */
	gpio_direction_output(MX23_PAD_GPMI_ALE__GPIO_0_17, 0);
	return 0;
}
#endif

int dram_init(void)
{
	return mxs_dram_init();
}

#ifdef	CONFIG_CMD_MMC
static int mx23_olx_mmc_cd(int id)
{
	return 1;	/* Card always present */
}

int board_mmc_init(bd_t *bis)
{
	return mxsmmc_initialize(bis, 0, NULL, mx23_olx_mmc_cd);
}
#endif

int board_init(void)
{
	/* Adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set(STATUS_LED_BOOT, STATUS_LED_STATE);
#endif

	return 0;
}

/* Fine-tune the DRAM configuration. */
void mxs_adjust_memory_params(uint32_t *dram_vals)
{
	/* Enable Auto Precharge. */
	dram_vals[3] |= 1 << 8;
	/* Enable Fast Writes. */
	dram_vals[5] |= 1 << 8;
	/* tEMRS = 3*tCK */
	dram_vals[10] &= ~(0x3 << 8);
	dram_vals[10] |= (0x3 << 8);
	/* CASLAT = 3*tCK */
	dram_vals[11] &= ~(0x3 << 0);
	dram_vals[11] |= (0x3 << 0);
	/* tCKE = 1*tCK */
	dram_vals[12] &= ~(0x7 << 0);
	dram_vals[12] |= (0x1 << 0);
	/* CASLAT_LIN_GATE = 3*tCK , CASLAT_LIN = 3*tCK, tWTR=2*tCK */
	dram_vals[13] &= ~((0xf << 16) | (0xf << 24) | (0xf << 0));
	dram_vals[13] |= (0x6 << 16) | (0x6 << 24) | (0x2 << 0);
	/* tDAL = 6*tCK */
	dram_vals[15] &= ~(0xf << 16);
	dram_vals[15] |= (0x6 << 16);
	/* tREF = 1040*tCK */
	dram_vals[26] &= ~0xffff;
	dram_vals[26] |= 0x0410;
	/* tRAS_MAX = 9334*tCK */
	dram_vals[32] &= ~0xffff;
	dram_vals[32] |= 0x2475;
}
