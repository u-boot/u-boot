/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 *
 * (C) Copyright 2009-2011
 * Eric Benard <eric@eukrea.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9260.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
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
static void cpu9260_nand_hw_init(void)
{
	unsigned long csa;
	at91_smc_t *smc = (at91_smc_t *) ATMEL_BASE_SMC;
	at91_matrix_t *matrix = (at91_matrix_t *) ATMEL_BASE_MATRIX;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	/* Enable CS3 */
	csa = readl(&matrix->csa) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa);

	/* Configure SMC CS3 for NAND/SmartMedia */
#if defined(CONFIG_CPU9G20)
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
#elif defined(CONFIG_CPU9260)
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
		AT91_SMC_MODE_DBW_8 |
		AT91_SMC_MODE_TDF_CYCLE(2),
		&smc->cs[3].mode);
#endif

	writel(1 << ATMEL_ID_PIOC, &pmc->pcer);

	/* Configure RDY/BSY */
	at91_set_pio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_pio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void cpu9260_macb_hw_init(void)
{
	unsigned long rstcmr;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_rstc_t *rstc = (at91_rstc_t *) ATMEL_BASE_RSTC;

	/* Enable clock */
	writel(1 << ATMEL_ID_EMAC0, &pmc->pcer);

	at91_set_pio_pullup(AT91_PIO_PORTA, 17, 1);

	rstcmr = readl(&rstc->mr) & AT91_RSTC_MR_ERSTL_MASK;

	/* Need to reset PHY -> 500ms reset */
	writel(AT91_RSTC_KEY | AT91_RSTC_MR_ERSTL(0xD) |
				AT91_RSTC_MR_URSTEN, &rstc->mr);

	writel(AT91_RSTC_KEY | AT91_RSTC_CR_EXTRST, &rstc->cr);

	/* Wait for end hardware reset */
	while (!(readl(&rstc->sr) & AT91_RSTC_SR_NRSTL))
		;

	/* Restore NRST value */
	writel(AT91_RSTC_KEY | rstcmr | AT91_RSTC_MR_URSTEN, &rstc->mr);

	at91_macb_hw_init();
}
#endif

int board_early_init_f(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	writel((1 << ATMEL_ID_PIOA) |
		(1 << ATMEL_ID_PIOB) |
		(1 << ATMEL_ID_PIOC),
		&pmc->pcer);

	at91_seriald_hw_init();

	return 0;
}


int board_init(void)
{
	/* arch number of the board */
#if defined(CONFIG_CPU9G20)
	gd->bd->bi_arch_number = MACH_TYPE_CPUAT9G20;
#elif defined(CONFIG_CPU9260)
	gd->bd->bi_arch_number = MACH_TYPE_CPUAT9260;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	cpu9260_nand_hw_init();
#endif
#ifdef CONFIG_MACB
	cpu9260_macb_hw_init();
#endif
#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set(STATUS_LED_BOOT, STATUS_LED_ON);
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC0, 0);
#endif
	return rc;
}
