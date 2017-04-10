/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>
#include <asm/arch/gpio.h>
#include <dm/platdata.h>
#include <dm/platform_data/serial_stm32x7.h>
#include <asm/arch/stm32_periph.h>
#include <asm/arch/stm32_defs.h>
#include <asm/arch/syscfg.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	struct udevice *dev;
	struct ram_info ram;
	int rv;

	rv = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (rv) {
		debug("DRAM init failed: %d\n", rv);
		return rv;
	}
	rv = ram_get_info(dev, &ram);
	if (rv) {
		debug("Cannot get DRAM size: %d\n", rv);
		return rv;
	}
	debug("SDRAM base=%lx, size=%x\n", ram.base, ram.size);
	gd->ram_size = ram.size;

	/*
	 * Fill in global info with description of SRAM configuration
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_RAM_BASE;
	gd->bd->bi_dram[0].size  = ram.size;

	return rv;
}

#ifdef CONFIG_ETH_DESIGNWARE
static int stmmac_setup(void)
{
	clock_setup(SYSCFG_CLOCK_CFG);
	/* Set >RMII mode */
	STM32_SYSCFG->pmc |= SYSCFG_PMC_MII_RMII_SEL;
	clock_setup(STMMAC_CLOCK_CFG);

	return 0;
}

int board_early_init_f(void)
{
	stmmac_setup();

	return 0;
}
#endif

u32 get_board_rev(void)
{
	return 0;
}

int board_late_init(void)
{
	struct gpio_desc gpio = {};
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "st,led1");
	if (node < 0)
		return -1;

	gpio_request_by_name_nodev(gd->fdt_blob, node, "led-gpio", 0, &gpio,
				   GPIOD_IS_OUT);

	if (dm_gpio_is_valid(&gpio)) {
		dm_gpio_set_value(&gpio, 0);
		mdelay(10);
		dm_gpio_set_value(&gpio, 1);
	}

	/* read button 1*/
	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "st,button1");
	if (node < 0)
		return -1;

	gpio_request_by_name_nodev(gd->fdt_blob, node, "button-gpio", 0, &gpio,
				   GPIOD_IS_IN);

	if (dm_gpio_is_valid(&gpio)) {
		if (dm_gpio_get_value(&gpio))
			puts("usr button is at HIGH LEVEL\n");
		else
			puts("usr button is at LOW LEVEL\n");
	}

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}
