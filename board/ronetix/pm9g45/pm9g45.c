/*
 * (C) Copyright 2010
 * Ilko Iliev <iliev@ronetix.at>
 * Asen Dimov <dimov@ronetix.at>
 * Ronetix GmbH <www.ronetix.at>
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
static void pm9g45_nand_hw_init(void)
{
	unsigned long csa;
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable CS3 */
	csa = readl(&matrix->ccr[6]) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->ccr[6]);

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
		AT91_SMC_MODE_DBW_8 |
		AT91_SMC_MODE_TDF_CYCLE(3),
		&smc->cs[3].mode);

	writel(1 << ATMEL_ID_PIOC, &pmc->pcer);

#ifdef CONFIG_SYS_NAND_READY_PIN
	/* Configure RDY/BSY */
	at91_set_pio_input(CONFIG_SYS_NAND_READY_PIN, 1);
#endif

	/* Enable NandFlash */
	at91_set_pio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void pm9g45_macb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/*
	 * PD2 enables the 50MHz oscillator for Ethernet PHY
	 * 1 - enable
	 * 0 - disable
	 */
	at91_set_pio_output(AT91_PIO_PORTD, 2, 1);
	at91_set_pio_value(AT91_PIO_PORTD, 2, 1); /* 1- enable, 0 - disable */

	/* Enable clock */
	writel(1 << ATMEL_ID_EMAC, &pmc->pcer);

	/*
	 * Disable pull-up on:
	 *	RXDV (PA15) => PHY normal mode (not Test mode)
	 *	ERX0 (PA12) => PHY ADDR0
	 *	ERX1 (PA13) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 12, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 13, 0);

	/* Re-enable pull-up */
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 12, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 13, 1);

	at91_macb_hw_init();
}
#endif

int board_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable Ctrlc */
	console_init_f();

	writel((1 << ATMEL_ID_PIOA) |
		(1 << ATMEL_ID_PIOB) |
		(1 << ATMEL_ID_PIOC) |
		(1 << ATMEL_ID_PIODE), &pmc->pcer);

	/* arch number of AT91SAM9M10G45EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_PM9G45;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91_seriald_hw_init();
#ifdef CONFIG_CMD_NAND
	pm9g45_nand_hw_init();
#endif

#ifdef CONFIG_MACB
	pm9g45_macb_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM,
				PHYS_SDRAM_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
#ifdef CONFIG_MACB
	/*
	 * Initialize ethernet HW addr prior to starting Linux,
	 * needed for nfsroot
	 */
	eth_init(gd->bd);
#endif
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x01);
#endif
	return rc;
}
