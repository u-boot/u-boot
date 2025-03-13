// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <config.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <miiphy.h>
#include <phy_interface.h>
#include <ram.h>
#include <serial.h>
#include <spl.h>
#include <splash.h>
#include <video.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>
#include <asm/arch/syscfg.h>
#include <asm/gpio.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
#ifndef CONFIG_XPL_BUILD
	int rv;
	struct udevice *dev;
	rv = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (rv) {
		debug("DRAM init failed: %d\n", rv);
		return rv;
	}

#endif
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#ifdef CONFIG_XPL_BUILD
#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	debug("SPL: booting kernel\n");
	/* break into full u-boot on 'c' */
	return serial_tstc() && serial_getc() == 'c';
}
#endif

int spl_dram_init(void)
{
	struct udevice *dev;
	int rv;
	rv = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (rv)
		debug("DRAM init failed: %d\n", rv);
	return rv;
}
void spl_board_init(void)
{
	preloader_console_init();
	spl_dram_init();
	arch_cpu_init(); /* to configure mpu for sdram rw permissions */
}
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_XIP;
}
#endif

int board_init(void)
{
#ifdef CONFIG_ETH_DESIGNWARE
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "st,stm32-dwmac");
	if (!ofnode_valid(node))
		return -1;

	switch (ofnode_read_phy_mode(node)) {
	case PHY_INTERFACE_MODE_RMII:
		STM32_SYSCFG->pmc |= SYSCFG_PMC_MII_RMII_SEL;
		break;
	case PHY_INTERFACE_MODE_MII:
		STM32_SYSCFG->pmc &= ~SYSCFG_PMC_MII_RMII_SEL;
		break;
	default:
		printf("Unsupported PHY interface!\n");
	}
#endif

	return 0;
}
