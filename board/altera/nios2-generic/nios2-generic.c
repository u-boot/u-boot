/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#if defined(CONFIG_CFI_FLASH_MTD)
#include <mtd/cfi_flash.h>
#endif
#include <asm/io.h>
#include <asm/gpio.h>

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
#ifdef CONFIG_ALTERA_PIO
#ifdef LED_PIO_BASE
	altera_pio_init(LED_PIO_BASE, LED_PIO_WIDTH, 'o',
			LED_PIO_RSTVAL, (1 << LED_PIO_WIDTH) - 1,
			"led");
#endif
#endif
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
				    CONFIG_SYS_ALTERA_TSE_SGDMA_TX_BASE,
#if defined(CONFIG_SYS_ALTERA_TSE_SGDMA_DESC_BASE) && \
	(CONFIG_SYS_ALTERA_TSE_SGDMA_DESC_SIZE > 0)
				    CONFIG_SYS_ALTERA_TSE_SGDMA_DESC_BASE,
				    CONFIG_SYS_ALTERA_TSE_SGDMA_DESC_SIZE);
#else
				    0,
				    0);
#endif
#endif
#ifdef CONFIG_ETHOC
	rc += ethoc_initialize(0, CONFIG_SYS_ETHOC_BASE);
#endif
	return rc;
}
#endif
