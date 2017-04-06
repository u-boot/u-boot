/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Needed to align size SPL image to a 4-byte length
 */
u32 end_align __attribute__ ((section(".end_align")));

/*
 * Return selected boot device. On MPC5200 its only NOR flash right now.
 */
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NOR;
}

/*
 * SPL version of board_init_f()
 */
void board_init_f(ulong bootflag)
{
	end_align = (u32)__spl_flash_end;

	/*
	 * On MPC5200, the initial RAM (and gd) is located in the internal
	 * SRAM. So we can actually call the preloader console init code
	 * before calling dram_init(). This makes serial output (printf)
	 * available very early, even before SDRAM init, which has been
	 * an U-Boot priciple from day 1.
	 */

	/*
	 * Init global_data pointer. Has to be done before calling
	 * get_clocks(), as it stores some clock values into gd needed
	 * later on in the serial driver.
	 */
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *)(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);
	/* Clear initial global data */
	memset((void *)gd, 0, sizeof(gd_t));

	/*
	 * get_clocks() needs to be called so that the serial driver
	 * works correctly
	 */
	get_clocks();

	/*
	 * Do rudimental console / serial setup
	 */
	preloader_console_init();

	/*
	 * First we need to initialize the SDRAM, so that the real
	 * U-Boot or the OS (Linux) can be loaded
	 */
	dram_init();

	/* Clear bss */
	memset(__bss_start, '\0', __bss_end - __bss_start);

	/*
	 * Call board_init_r() (SPL framework version) to load and boot
	 * real U-Boot or OS
	 */
	board_init_r(NULL, 0);
	/* Does not return!!! */
}
