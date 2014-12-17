/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ibmpc.h>
#include <asm/pnp_def.h>
#include <netdev.h>
#include <smsc_lpc47m.h>

#define SERIAL_DEV PNP_DEV(0x2e, 4)

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	lpc47m_enable_serial(SERIAL_DEV, UART0_BASE);

	return 0;
}

void setup_pch_gpios(u16 gpiobase, const struct pch_gpio_map *gpio)
{
	return;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
