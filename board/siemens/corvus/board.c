/*
 * Board functions for Siemens CORVUS (AT91SAM9G45) based board
 * (C) Copyright 2013 Siemens AG
 *
 * Based on:
 * U-Boot file: board/atmel/at91sam9m10g45ek/at91sam9m10g45ek.c
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9g45_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_NAND
static void corvus_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(2),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(4),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	writel(1 << ATMEL_ID_PIOC, &pmc->pcer);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_CMD_USB
static void taurus_usb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(1 << ATMEL_ID_PIODE, &pmc->pcer);

	at91_set_gpio_output(AT91_PIN_PD1, 0);
	at91_set_gpio_output(AT91_PIN_PD3, 0);
}
#endif

#ifdef CONFIG_MACB
static void corvus_macb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable clock */
	writel(1 << ATMEL_ID_EMAC, &pmc->pcer);

	/*
	 * Disable pull-up on:
	 *      RXDV (PA15) => PHY normal mode (not Test mode)
	 *      ERX0 (PA12) => PHY ADDR0
	 *      ERX1 (PA13) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 12, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 13, 0);

	at91_phy_reset();

	/* Re-enable pull-up */
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 12, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 13, 1);

	/* And the pins. */
	at91_macb_hw_init();
}
#endif

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	corvus_nand_hw_init();
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_MACB
	corvus_macb_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	taurus_usb_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
	return rc;
}

/* SPI chip select control */
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < 2;
}

void spi_cs_activate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 1:
			at91_set_gpio_output(AT91_PIN_PB18, 0);
			break;
	case 0:
	default:
			at91_set_gpio_output(AT91_PIN_PB3, 0);
			break;
	}
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 1:
			at91_set_gpio_output(AT91_PIN_PB18, 1);
			break;
	case 0:
	default:
			at91_set_gpio_output(AT91_PIN_PB3, 1);
			break;
	}
}
