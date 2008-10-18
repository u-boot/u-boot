/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 *
 * Author: Scott Wood <scottwood@freescale.com>
 *         Dave Liu <daveliu@freescale.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <i2c.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <pci.h>
#include <mpc83xx.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	if (im->pmc.pmccr1 & PMCCR1_POWER_OFF)
		gd->flags |= GD_FLG_SILENT;

	return 0;
}

static u8 read_board_info(void)
{
	u8 val8;
	i2c_set_bus_num(0);

	if (i2c_read(CONFIG_SYS_I2C_PCF8574A_ADDR, 0, 0, &val8, 1) == 0)
		return val8;
	else
		return 0;
}

int checkboard(void)
{
	static const char * const rev_str[] = {
		"0.0",
		"0.1",
		"1.0",
		"1.1",
		"<unknown>",
	};
	u8 info;
	int i;

	info = read_board_info();
	i = (!info) ? 4: info & 0x03;

	printf("Board: Freescale MPC8315ERDB Rev %s\n", rev_str[i]);

	return 0;
}

static struct pci_region pci_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI_MEM_BASE,
		phys_start: CONFIG_SYS_PCI_MEM_PHYS,
		size: CONFIG_SYS_PCI_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI_MMIO_PHYS,
		size: CONFIG_SYS_PCI_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
	{
		bus_start: CONFIG_SYS_PCI_IO_BASE,
		phys_start: CONFIG_SYS_PCI_IO_PHYS,
		size: CONFIG_SYS_PCI_IO_SIZE,
		flags: PCI_REGION_IO
	}
};

void pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	struct pci_region *reg[] = { pci_regions };
	int warmboot;

	/* Enable all 3 PCI_CLK_OUTPUTs. */
	clk->occr |= 0xe0000000;

	/*
	 * Configure PCI Local Access Windows
	 */
	pci_law[0].bar = CONFIG_SYS_PCI_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_512MB;

	pci_law[1].bar = CONFIG_SYS_PCI_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	warmboot = gd->bd->bi_bootflags & BOOTFLAG_WARM;
	warmboot |= immr->pmc.pmccr1 & PMCCR1_POWER_OFF;

	mpc83xx_pci_init(1, reg, warmboot);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void fdt_tsec1_fixup(void *fdt, bd_t *bd)
{
	char *mpc8315erdb = getenv("mpc8315erdb");
	const char disabled[] = "disabled";
	const char *path;
	int ret;

	if (!mpc8315erdb)
		return;

	if (!strcmp(mpc8315erdb, "tsec1")) {
		return;
	} else if (strcmp(mpc8315erdb, "ulpi")) {
		printf("WARNING: wrong `mpc8315erdb' environment "
		       "variable specified: `%s'. Should be `ulpi' "
		       "or `tsec1'.\n", mpc8315erdb);
		return;
	}

	ret = fdt_path_offset(fdt, "/aliases");
	if (ret < 0) {
		printf("WARNING: can't find /aliases node\n");
		return;
	}

	path = fdt_getprop(fdt, ret, "ethernet0", NULL);
	if (!path) {
		printf("WARNING: can't find ethernet0 alias\n");
		return;
	}

	do_fixup_by_path(fdt, path, "status", disabled, sizeof(disabled), 1);
}

void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	fdt_fixup_dr_usb(blob, bd);
	fdt_tsec1_fixup(blob, bd);
}
#endif

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis);	/* Initialize TSECs first */
	return pci_eth_init(bis);
}
