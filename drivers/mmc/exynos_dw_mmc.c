/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,  MA 02111-1307 USA
 *
 */

#include <common.h>
#include <malloc.h>
#include <dwmmc.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/clk.h>

static char *EXYNOS_NAME = "EXYNOS DWMMC";

static void exynos_dwmci_clksel(struct dwmci_host *host)
{
	u32 val;
	val = DWMCI_SET_SAMPLE_CLK(DWMCI_SHIFT_0) |
		DWMCI_SET_DRV_CLK(DWMCI_SHIFT_0) | DWMCI_SET_DIV_RATIO(0);

	dwmci_writel(host, DWMCI_CLKSEL, val);
}

int exynos_dwmci_init(u32 regbase, int bus_width, int index)
{
	struct dwmci_host *host = NULL;
	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		printf("dwmci_host malloc fail!\n");
		return 1;
	}

	host->name = EXYNOS_NAME;
	host->ioaddr = (void *)regbase;
	host->buswidth = bus_width;
	host->clksel = exynos_dwmci_clksel;
	host->dev_index = index;

	add_dwmci(host, 52000000, 400000);

	return 0;
}

