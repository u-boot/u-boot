/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/nand_defs.h>

DECLARE_GLOBAL_DATA_PTR;

#define REV_DM6467EVM		0
#define REV_DM6467TEVM		1
/*
 * get_board_rev() - setup to pass kernel board revision information
 * Returns:
 * bit[0-3]	System clock frequency
 * 0000b	- 27 MHz
 * 0001b	- 33 MHz
 */
u32 get_board_rev(void)
{

#ifdef CONFIG_DAVINCI_DM6467TEVM
	return REV_DM6467TEVM;
#else
	return REV_DM6467EVM;
#endif

}

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
