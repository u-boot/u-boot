/*
 * Copyright 2013 Broadcom Corporation.
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <mmc.h>
#include <asm/kona-common/kona_sdhci.h>
#include <asm/kona-common/clk.h>
#include <asm/arch/sysmap.h>

#define SECWATCHDOG_SDOGCR_OFFSET	0x00000000
#define SECWATCHDOG_SDOGCR_EN_SHIFT	27
#define SECWATCHDOG_SDOGCR_SRSTEN_SHIFT	26
#define SECWATCHDOG_SDOGCR_CLKS_SHIFT	20
#define SECWATCHDOG_SDOGCR_LD_SHIFT	0

DECLARE_GLOBAL_DATA_PTR;

/*
 * board_init - early hardware init
 */
int board_init(void)
{
	printf("Relocation Offset is: %08lx\n", gd->reloc_off);

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	clk_init();

	return 0;
}

/*
 * misc_init_r - miscellaneous platform dependent initializations
 */
int misc_init_r(void)
{
	/* Disable watchdog reset - watchdog unused */
	writel((0 << SECWATCHDOG_SDOGCR_EN_SHIFT) |
	       (0 << SECWATCHDOG_SDOGCR_SRSTEN_SHIFT) |
	       (4 << SECWATCHDOG_SDOGCR_CLKS_SHIFT) |
	       (0x5a0 << SECWATCHDOG_SDOGCR_LD_SHIFT),
	       (SECWD_BASE_ADDR + SECWATCHDOG_SDOGCR_OFFSET));

	return 0;
}

/*
 * dram_init - sets uboots idea of sdram size
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

/* This is called after dram_init() so use get_ram_size result */
void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;
}

#ifdef CONFIG_KONA_SDHCI
/*
 * mmc_init - Initializes mmc
 */
int board_mmc_init(bd_t *bis)
{
	int ret = 0;

	/* Register eMMC - SDIO2 */
	ret = kona_sdhci_init(1, 400000, 0);
	if (ret)
		return ret;

	/* Register SD Card - SDIO4 kona_mmc_init assumes 0 based index */
	ret = kona_sdhci_init(3, 400000, 0);
	return ret;
}
#endif
