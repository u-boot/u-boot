/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Copyright (C) 2009
 * Albin Tonnerre, Free-Electrons <albin.tonnerre@free-electrons.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/gpio.h>

#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
static void sbc35_a9g20_nand_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_MODE_DBW_8 |
#endif
		AT91_SMC_MODE_TDF_CYCLE(2),
		&smc->cs[3].mode);

	writel(1 << ATMEL_ID_PIOC, &pmc->pcer);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void sbc35_a9g20_macb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct at91_port *pioa = (struct at91_port *)ATMEL_BASE_PIOA;

	/* Enable EMAC clock */
	writel(1 << ATMEL_ID_EMAC0, &pmc->pcer);

	/*
	 * Disable pull-up on:
	 *	RXDV (PA17) => PHY normal mode (not Test mode)
	 *	ERX0 (PA14) => PHY ADDR0
	 *	ERX1 (PA15) => PHY ADDR1
	 *	ERX2 (PA25) => PHY ADDR2
	 *	ERX3 (PA26) => PHY ADDR3
	 *	ECRS (PA28) => PHY ADDR4  => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA17) |
	       pin_to_mask(AT91_PIN_PA25) |
	       pin_to_mask(AT91_PIN_PA26) |
	       pin_to_mask(AT91_PIN_PA28),
	       &pioa->pudr);

	at91_phy_reset();

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA17) |
	       pin_to_mask(AT91_PIN_PA25) |
	       pin_to_mask(AT91_PIN_PA26) |
	       pin_to_mask(AT91_PIN_PA28),
	       &pioa->puer);

	at91_macb_hw_init();
}
#endif

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	at91_seriald_hw_init();
	sbc35_a9g20_nand_hw_init();
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4 | 1 << 5);
#endif
#ifdef CONFIG_MACB
	sbc35_a9g20_macb_hw_init();
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size(
		(void *)CONFIG_SYS_SDRAM_BASE,
		CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC0, 0x00);
#endif
	return rc;
}
