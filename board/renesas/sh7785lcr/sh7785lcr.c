/*
 * Copyright (C) 2008 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/pci.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("BOARD: Renesas Technology Corp. R0P7785LC0011RL\n");
	return 0;
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	printf("DRAM:  %dMB\n", CONFIG_SYS_SDRAM_SIZE / (1024 * 1024));
	return 0;
}

static struct pci_controller hose;
void pci_init_board(void)
{
	pci_sh7780_init(&hose);
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

#if defined(CONFIG_SH_32BIT)
int do_pmb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	/* clear ITLB */
	writel(0x00000004, 0xff000010);

	/* delete PMB for peripheral */
	writel(0, PMB_ADDR_BASE(0));
	writel(0, PMB_DATA_BASE(0));
	writel(0, PMB_ADDR_BASE(1));
	writel(0, PMB_DATA_BASE(1));
	writel(0, PMB_ADDR_BASE(2));
	writel(0, PMB_DATA_BASE(2));

	/* add PMB for SDRAM(0x40000000 - 0x47ffffff) */
	writel(mk_pmb_addr_val(0x80), PMB_ADDR_BASE(8));
	writel(mk_pmb_data_val(0x40, 0, 1, 1, 0, 1, 1), PMB_DATA_BASE(8));
	writel(mk_pmb_addr_val(0xa0), PMB_ADDR_BASE(12));
	writel(mk_pmb_data_val(0x40, 1, 1, 1, 0, 0, 1), PMB_DATA_BASE(12));

	return 0;
}

U_BOOT_CMD(
	pmb,	1,	1,	do_pmb,
	"pmb     - PMB setting\n",
	"\n"
	"    - PMB setting for all SDRAM mapping"
);
#endif
