// SPDX-License-Identifier: GPL-2.0+
/*
 * Miscelaneous DaVinci functions.
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc Ltd, <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 * Copyright (C) 2004 Texas Instruments.
 */

#include <config.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <asm/arch/hardware.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_XPL_BUILD
int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(void *)CFG_SYS_SDRAM_BASE,
			CFG_MAX_RAM_BANK_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}
#endif

#ifdef CONFIG_DRIVER_TI_EMAC
/*
 * Set the mii mode as MII or RMII
 */
void davinci_emac_mii_mode_sel(int mode_sel)
{
	int val;

	val = readl(&davinci_syscfg_regs->cfgchip3);
	if (mode_sel == 0)
		val &= ~(1 << 8);
	else
		val |= (1 << 8);
	writel(val, &davinci_syscfg_regs->cfgchip3);
}

/*
 * If there is no MAC address in the environment, then it will be initialized
 * (silently) from the value in the EEPROM.
 */
void davinci_sync_env_enetaddr(uint8_t *rom_enetaddr)
{
	uint8_t env_enetaddr[6];
	int ret;

	ret = eth_env_get_enetaddr_by_index("eth", 0, env_enetaddr);
	if (!ret) {
		/*
		 * There is no MAC address in the environment, so we
		 * initialize it from the value in the EEPROM.
		 */
		debug("### Setting environment from EEPROM MAC address = "
			"\"%pM\"\n",
			env_enetaddr);
		ret = !eth_env_set_enetaddr("ethaddr", rom_enetaddr);
	}
	if (!ret)
		printf("Failed to set mac address from EEPROM: %d\n", ret);
}
#endif	/* CONFIG_DRIVER_TI_EMAC */

void irq_init(void)
{
	/*
	 * Mask all IRQs by clearing the global enable and setting
	 * the enable clear for all the 90 interrupts.
	 */
	writel(0, &davinci_aintc_regs->ger);

	writel(0, &davinci_aintc_regs->hier);

	writel(0xffffffff, &davinci_aintc_regs->ecr1);
	writel(0xffffffff, &davinci_aintc_regs->ecr2);
	writel(0xffffffff, &davinci_aintc_regs->ecr3);
}

/*
 * Enable PSC for various peripherals.
 */
int da8xx_configure_lpsc_items(const struct lpsc_resource *item,
				    const int n_items)
{
	int i;

	for (i = 0; i < n_items; i++)
		lpsc_on(item[i].lpsc_no);

	return 0;
}
