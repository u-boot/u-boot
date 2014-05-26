/*
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int timer_init(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
}

int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM0)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR0,
						CONFIG_ZYNQ_GEM_PHY_ADDR0, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM1)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR1,
						CONFIG_ZYNQ_GEM_PHY_ADDR1, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM2)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR2,
						CONFIG_ZYNQ_GEM_PHY_ADDR2, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM3)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR3,
						CONFIG_ZYNQ_GEM_PHY_ADDR3, 0);
# endif
#endif
	return ret;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
        int ret = 0;

#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
        ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
        ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif
        return ret;
}
#endif
