/*
 * Bluewater Systems Snapper 9260/9G20 modules
 *
 * (C) Copyright 2011 Bluewater Systems
 *   Author: Andre Renaud <andre@bluewatersys.com>
 *   Author: Ryan Mallon <ryan@bluewatersys.com>
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
#include <asm/io.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <net.h>
#include <netdev.h>
#include <i2c.h>
#include <pca953x.h>

DECLARE_GLOBAL_DATA_PTR;

/* IO Expander pins */
#define IO_EXP_ETH_RESET	(0 << 1)
#define IO_EXP_ETH_POWER	(1 << 1)

static void macb_hw_init(void)
{
	struct at91_pmc *pmc   = (struct at91_pmc  *)ATMEL_BASE_PMC;
	struct at91_port *pioa = (struct at91_port *)ATMEL_BASE_PIOA;
	struct at91_rstc *rstc = (struct at91_rstc *)ATMEL_BASE_RSTC;
	unsigned long erstl;

	/* Enable clock */
	writel(1 << ATMEL_ID_EMAC0, &pmc->pcer);

	/* Disable pull-ups to prevent PHY going into test mode */
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA18),
	       &pioa->pudr);

	/* Power down ethernet */
	pca953x_set_dir(0x28, IO_EXP_ETH_POWER, PCA953X_DIR_OUT);
	pca953x_set_val(0x28, IO_EXP_ETH_POWER, 1);

	/* Hold ethernet in reset */
	pca953x_set_dir(0x28, IO_EXP_ETH_RESET, PCA953X_DIR_OUT);
	pca953x_set_val(0x28, IO_EXP_ETH_RESET, 0);

	/* Enable ethernet power */
	pca953x_set_val(0x28, IO_EXP_ETH_POWER, 0);

	/* Need to reset PHY -> 500ms reset */
	erstl = readl(&rstc->mr) & AT91_RSTC_MR_ERSTL_MASK;
	writel(AT91_RSTC_KEY | AT91_RSTC_MR_ERSTL(13) |
	       AT91_RSTC_MR_URSTEN, &rstc->mr);
	writel(AT91_RSTC_KEY | AT91_RSTC_CR_EXTRST, &rstc->cr);

	/* Wait for end hardware reset */
	while (!(readl(&rstc->sr) & AT91_RSTC_SR_NRSTL))
		;

	/* Restore NRST value */
	writel(AT91_RSTC_KEY | erstl | AT91_RSTC_MR_URSTEN, &rstc->mr);

	/* Bring the ethernet out of reset */
	pca953x_set_val(0x28, IO_EXP_ETH_RESET, 1);

	/* The phy internal reset take 21ms */
	udelay(21 * 1000);

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA18),
	       &pioa->puer);

	at91_macb_hw_init();
}

static void nand_hw_init(void)
{
	struct at91_smc *smc       = (struct at91_smc    *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 as NAND/SmartMedia */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(4) |
	       AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(4),
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

int board_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable PIO clocks */
	writel((1 << ATMEL_ID_PIOA) |
	       (1 << ATMEL_ID_PIOB) |
	       (1 << ATMEL_ID_PIOC), &pmc->pcer);

	/* The mach-type is the same for both Snapper 9260 and 9G20 */
	gd->bd->bi_arch_number = MACH_TYPE_SNAPPER_9260;

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* Initialise peripherals */
	at91_seriald_hw_init();
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	nand_hw_init();
	macb_hw_init();

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC0, 0x1f);
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

void reset_phy(void)
{
}
