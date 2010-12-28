/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/nand_defs.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DM6467_EVM;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	lpsc_on(DAVINCI_DM646X_LPSC_TIMER0);
	lpsc_on(DAVINCI_DM646X_LPSC_UART0);
	lpsc_on(DAVINCI_DM646X_LPSC_I2C);
	lpsc_on(DAVINCI_DM646X_LPSC_EMAC);

	/* Enable GIO3.3V cells used for EMAC */
	REG(VDD3P3V_PWDN) = 0x80000c0;

	/* Select UART function on UART0 */
	REG(PINMUX0) &= ~(0x0000003f << 18);
	REG(PINMUX1) &= ~(0x00000003);

	return 0;
}

#if defined(CONFIG_DRIVER_TI_EMAC)

int board_eth_init(bd_t *bis)
{
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */

#ifdef CONFIG_NAND_DAVINCI
int board_nand_init(struct nand_chip *nand)
{
	davinci_nand_init(nand);

	return 0;
}
#endif
