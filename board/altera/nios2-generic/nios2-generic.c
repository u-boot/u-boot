/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <netdev.h>
#include <mtd/cfi_flash.h>
#include <asm/io.h>

void text_base_hook(void); /* nop hook for text_base.S */

#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
static void __early_flash_cmd_reset(void)
{
	/* reset flash before we read env */
	writeb(AMD_CMD_RESET, CONFIG_ENV_ADDR);
	writeb(FLASH_CMD_RESET, CONFIG_ENV_ADDR);
}
void early_flash_cmd_reset(void)
	__attribute__((weak,alias("__early_flash_cmd_reset")));
#endif

int board_early_init_f(void)
{
	text_base_hook();
#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
	early_flash_cmd_reset();
#endif
	return 0;
}

int checkboard(void)
{
	printf("BOARD : %s\n", CONFIG_BOARD_NAME);
	return 0;
}

phys_size_t initdram(int board_type)
{
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc += smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
#ifdef CONFIG_DRIVER_DM9000
	rc += dm9000_initialize(bis);
#endif
#ifdef CONFIG_ALTERA_TSE
	rc += altera_tse_initialize(0,
				    CONFIG_SYS_ALTERA_TSE_MAC_BASE,
				    CONFIG_SYS_ALTERA_TSE_SGDMA_RX_BASE,
				    CONFIG_SYS_ALTERA_TSE_SGDMA_TX_BASE);
#endif
#ifdef CONFIG_ETHOC
	rc += ethoc_initialize(0, CONFIG_SYS_ETHOC_BASE);
#endif
	return rc;
}
#endif
