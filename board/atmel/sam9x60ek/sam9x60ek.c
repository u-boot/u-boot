// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Sandeep Sheriker M <sandeep.sheriker@microchip.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_sfr.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <debug_uart.h>
#include <asm/mach-types.h>

extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

void at91_prepare_cpu_var(void);

#ifdef CONFIG_CMD_NAND
static void sam9x60ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct atmel_sfr *sfr = (struct atmel_sfr *)ATMEL_BASE_SFR;
	unsigned int csa;

	at91_pio3_set_a_periph(AT91_PIO_PORTD, 0, 1);	/* NAND OE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 1, 1);	/* NAND WE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 2, 0);	/* NAND ALE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 3, 0);	/* NAND CLE */
	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 6, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 7, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 8, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 9, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 10, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 11, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 12, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 13, 1);

	at91_periph_clk_enable(ATMEL_ID_PIOD);

	/* Enable CS3 */
	csa = readl(&sfr->ebicsa);
	csa |= AT91_SFR_CCFG_EBI_CSA(3, 1) | AT91_SFR_CCFG_NFD0_ON_D16;

	/* Configure IO drive */
	csa &= ~AT91_SFR_CCFG_EBI_DRIVE_SAM9X60;

	writel(csa, &sfr->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(4), &smc->cs[3].setup);

	writel(AT91_SMC_PULSE_NWE(10) | AT91_SMC_PULSE_NCS_WR(20) |
	       AT91_SMC_PULSE_NRD(10) | AT91_SMC_PULSE_NCS_RD(20),
	       &smc->cs[3].pulse);

	writel(AT91_SMC_CYCLE_NWE(20) | AT91_SMC_CYCLE_NRD(20),
	       &smc->cs[3].cycle);

	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF | AT91_SMC_MODE_TDF_CYCLE(15),
	       &smc->cs[3].mode);
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	at91_prepare_cpu_var();

	at91_pda_detect();

	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	at91_seriald_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	return 0;
}
#endif

#define MAC24AA_MAC_OFFSET     0xfa

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_I2C_EEPROM
	at91_set_ethaddr(MAC24AA_MAC_OFFSET);
#endif
	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	sam9x60ek_nand_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}
