// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#include <spl.h>
#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <asm/arch-adi/sc5xx/spl.h>
#include "init/clkinit.h"
#include "init/dmcinit.h"

static bool adi_start_uboot_proper;

static int adi_sf_default_bus = CONFIG_SF_DEFAULT_BUS;
static int adi_sf_default_cs = CONFIG_SF_DEFAULT_CS;
static int adi_sf_default_speed = CONFIG_SF_DEFAULT_SPEED;

u32 bmode;

int spl_start_uboot(void)
{
	return adi_start_uboot_proper;
}

unsigned int spl_spi_get_default_speed(void)
{
	return adi_sf_default_speed;
}

unsigned int spl_spi_get_default_bus(void)
{
	return adi_sf_default_bus;
}

unsigned int spl_spi_get_default_cs(void)
{
	return adi_sf_default_cs;
}

void board_boot_order(u32 *spl_boot_list)
{
	const char *bmodestring = sc5xx_get_boot_mode(&bmode);

	printf("ADI Boot Mode: 0x%x (%s)\n", bmode, bmodestring);

	/*
	 * By default everything goes back to the bootrom, where we'll read table
	 * parameters and ask for another image to be loaded
	 */
	spl_boot_list[0] = BOOT_DEVICE_BOOTROM;

	if (bmode == 0) {
		printf("SPL execution has completed.  Please load U-Boot Proper via JTAG");
		while (1)
			;
	}
}

int32_t __weak adi_rom_boot_hook(struct ADI_ROM_BOOT_CONFIG *config, int32_t cause)
{
	return 0;
}

int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev)
{
#if CONFIG_ADI_SPL_FORCE_BMODE != 0
	// see above
	if (bmode != 0 && bmode != 3)
		bmode = CONFIG_ADI_SPL_FORCE_BMODE;
#endif

	if (bmode >= (ARRAY_SIZE(adi_rom_boot_args)))
		bmode = 0;

	adi_rom_boot((void *)adi_rom_boot_args[bmode].addr,
		     adi_rom_boot_args[bmode].flags,
		     0, &adi_rom_boot_hook,
		     adi_rom_boot_args[bmode].cmd);
	return 0;
};

void board_init_f(ulong dummy)
{
	int ret;

	clks_init();
	DMC_Config();
	sc5xx_soc_init();

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed\n");

	preloader_console_init();
}

