/*
 * Copyright 2010-2011 Calxeda, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ahci.h>
#include <netdev.h>
#include <scsi.h>

#include <linux/sizes.h>
#include <asm/io.h>

#define HB_AHCI_BASE			0xffe08000

#define HB_SREG_A9_PWR_REQ		0xfff3cf00
#define HB_SREG_A9_BOOT_SRC_STAT	0xfff3cf04
#define HB_SREG_A9_PWRDOM_STAT		0xfff3cf20

#define HB_PWR_SUSPEND			0
#define HB_PWR_SOFT_RESET		1
#define HB_PWR_HARD_RESET		2
#define HB_PWR_SHUTDOWN			3

#define PWRDOM_STAT_SATA		0x80000000
#define PWRDOM_STAT_PCI			0x40000000
#define PWRDOM_STAT_EMMC		0x20000000

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	icache_enable();

	return 0;
}

/* We know all the init functions have been run now */
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_CALXEDA_XGMAC
	rc += calxedaxgmac_initialize(0, 0xfff50000);
	rc += calxedaxgmac_initialize(1, 0xfff51000);
#endif
	return rc;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char envbuffer[16];
	u32 boot_choice;
	u32 reg = readl(HB_SREG_A9_PWRDOM_STAT);

	if (reg & PWRDOM_STAT_SATA) {
		ahci_init(HB_AHCI_BASE);
		scsi_scan(1);
	}

	boot_choice = readl(HB_SREG_A9_BOOT_SRC_STAT) & 0xff;
	sprintf(envbuffer, "bootcmd%d", boot_choice);
	if (getenv(envbuffer)) {
		sprintf(envbuffer, "run bootcmd%d", boot_choice);
		setenv("bootcmd", envbuffer);
	} else
		setenv("bootcmd", "");

	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = SZ_512M;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  PHYS_SDRAM_1_SIZE;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *fdt, bd_t *bd)
{
	static const char disabled[] = "disabled";
	u32 reg = readl(HB_SREG_A9_PWRDOM_STAT);

	if (!(reg & PWRDOM_STAT_SATA))
		do_fixup_by_compat(fdt, "calxeda,hb-ahci", "status",
			disabled, sizeof(disabled), 1);

	if (!(reg & PWRDOM_STAT_EMMC))
		do_fixup_by_compat(fdt, "calxeda,hb-sdhci", "status",
			disabled, sizeof(disabled), 1);
}
#endif

void reset_cpu(ulong addr)
{
	writel(HB_PWR_HARD_RESET, HB_SREG_A9_PWR_REQ);

	wfi();
}
