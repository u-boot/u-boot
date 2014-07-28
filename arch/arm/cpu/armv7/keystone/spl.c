/*
 * common spl init code
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <ns16550.h>
#include <malloc.h>
#include <spl.h>
#include <spi_flash.h>

#include <asm/u-boot.h>
#include <asm/utils.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_K2HK_EVM
static struct pll_init_data spl_pll_config[] = {
	CORE_PLL_799,
	TETRIS_PLL_500,
};
#endif

#ifdef CONFIG_K2E_EVM
static struct pll_init_data spl_pll_config[] = {
	CORE_PLL_800,
};
#endif

void spl_init_keystone_plls(void)
{
	init_plls(ARRAY_SIZE(spl_pll_config), spl_pll_config);
}

void spl_board_init(void)
{
	spl_init_keystone_plls();
	preloader_console_init();
}

u32 spl_boot_device(void)
{
#if defined(CONFIG_SPL_SPI_LOAD)
	return BOOT_DEVICE_SPI;
#else
	puts("Unknown boot device\n");
	hang();
#endif
}
