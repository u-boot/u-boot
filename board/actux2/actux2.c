/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/arch/ixp425.h>
#include <asm/io.h>

#include <miiphy.h>

#include "actux2_hw.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* CS1: IPAC-X */
	writel(0x94d10013, IXP425_EXP_CS1);
	/* CS5: Debug port */
	writel(0x9d520003, IXP425_EXP_CS5);
	/* CS6: HW release register */
	writel(0x81860001, IXP425_EXP_CS6);
	/* CS7: LEDs */
	writel(0x80900003, IXP425_EXP_CS7);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_ACTUX2;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_IORST);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_ETHRST);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_DSR);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_DCD);

	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_IORST);
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_ETHRST);

	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_DSR);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_DCD);

	/* Setup GPIOs for Interrupt inputs */
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_DBGINT);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_ETHINT);

	/* Setup GPIOs for 33MHz clock output */
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PCI_CLK);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_EXTBUS_CLK);
	writel(0x011001FF, IXP425_GPIO_GPCLKR);

	udelay(533);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_IORST);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_ETHRST);

	ACTUX2_LED1(1);
	ACTUX2_LED2(0);
	ACTUX2_LED3(0);
	ACTUX2_LED4(0);

	return 0;
}

/*
 * Check Board Identity
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	puts("Board: AcTux-2 rev.");
	putc(ACTUX2_BOARDREL + 'A' - 1);

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE, 128<<20);
	return 0;
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 * 0 = reserved
 * 1 = Rev. A
 * 2 = Rev. B
 *************************************************************************/
u32 get_board_rev(void)
{
	return ACTUX2_BOARDREL;
}

void reset_phy(void)
{
	/* init IcPlus IP175C ethernet switch to native IP175C mode */
	miiphy_write("NPE0", 29, 31, 0x175C);
}
