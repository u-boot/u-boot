/*
 * Board functions for Siemens TAURUS (AT91SAM9G20) based boards
 * (C) Copyright Siemens AG
 *
 * Based on:
 * U-Boot file: board/atmel/at91sam9260ek/at91sam9260ek.c
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91sam9_sdramc.h>
#include <asm/arch/clk.h>
#include <linux/mtd/nand.h>
#include <atmel_mci.h>
#include <asm/arch/at91_spi.h>
#include <spi.h>

#include <net.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

static void taurus_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Assign CS3 to NAND/SmartMedia Interface */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(3),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(7),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>
#include <spi_flash.h>

void matrix_init(void)
{
	struct at91_matrix *mat = (struct at91_matrix *)ATMEL_BASE_MATRIX;

	writel((readl(&mat->scfg[3]) & (~AT91_MATRIX_SLOT_CYCLE))
			| AT91_MATRIX_SLOT_CYCLE_(0x40),
			&mat->scfg[3]);
}

void at91_spl_board_init(void)
{
	taurus_nand_hw_init();
	at91_spi0_hw_init(TAURUS_SPI_MASK);

	/* Configure recovery button PINs */
	at91_set_gpio_input(AT91_PIN_PA31, 1);

	/* check if button is pressed */
	if (at91_get_gpio_value(AT91_PIN_PA31) == 0) {
		struct spi_flash *flash;

		debug("Recovery button pressed\n");
		nand_init();
		spl_nand_erase_one(0, 0);
		flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
					0,
					CONFIG_SF_DEFAULT_SPEED,
					SPI_MODE_3);
		if (!flash) {
			puts("no flash\n");
		} else {
			puts("erase spi flash sector 0\n");
			spi_flash_erase(flash, 0,
					CONFIG_SYS_NAND_U_BOOT_SIZE);
		}
	}
}

void mem_init(void)
{
	struct at91_matrix *ma = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct sdramc_reg setting;

	at91_sdram_hw_init();
	setting.cr = (AT91_SDRAMC_NC_9 |
		      AT91_SDRAMC_NR_13 |
		      AT91_SDRAMC_CAS_3 |
		      AT91_SDRAMC_NB_4 |
		      AT91_SDRAMC_DBW_32 |
		      AT91_SDRAMC_TWR_VAL(3) |
		      AT91_SDRAMC_TRC_VAL(9) |
		      AT91_SDRAMC_TRP_VAL(3) |
		      AT91_SDRAMC_TRCD_VAL(3) |
		      AT91_SDRAMC_TRAS_VAL(6) |
		      AT91_SDRAMC_TXSR_VAL(10));
	setting.mdr = AT91_SDRAMC_MD_SDRAM;
	setting.tr = (CONFIG_SYS_MASTER_CLOCK * 7) / 1000000;


	writel(readl(&ma->ebicsa) | AT91_MATRIX_CS1A_SDRAMC |
		AT91_MATRIX_VDDIOMSEL_3_3V | AT91_MATRIX_EBI_IOSR_SEL,
		&ma->ebicsa);
	sdramc_initialize(ATMEL_BASE_CS1, &setting);
}
#endif

#ifdef CONFIG_MACB
static void taurus_macb_hw_init(void)
{
	/* Enable EMAC clock */
	at91_periph_clk_enable(ATMEL_ID_EMAC0);

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
	at91_set_pio_pullup(AT91_PIO_PORTA, 14, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 17, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 25, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 26, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 28, 0);

	at91_phy_reset();

	at91_set_gpio_input(AT91_PIN_PA25, 1);   /* ERST tri-state */

	/* Re-enable pull-up */
	at91_set_pio_pullup(AT91_PIO_PORTA, 14, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 17, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 25, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 26, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 28, 1);

	/* Initialize EMAC=MACB hardware */
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bd)
{
	at91_mci_hw_init();

	return atmel_mci_init((void *)ATMEL_BASE_MCI);
}
#endif

int board_early_init_f(void)
{
	/* Enable clocks for all PIOs */
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);

	at91_seriald_hw_init();

	return 0;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	at91_set_gpio_value(TAURUS_SPI_CS_PIN, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	at91_set_gpio_value(TAURUS_SPI_CS_PIN, 1);
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	taurus_nand_hw_init();
#endif
#ifdef CONFIG_MACB
	taurus_macb_hw_init();
#endif
	at91_spi0_hw_init(TAURUS_SPI_MASK);

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
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC0, 0x00);
#endif
	return rc;
}
