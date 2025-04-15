// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 - 2013 Atmel Corporation
 * Bo Shen <voice.shen@atmel.com>
 */

#include <config.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/sama5d3_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <debug_uart.h>
#include <env.h>
#include <linux/ctype.h>
#include <spl.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/at91_wdt.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_NAND_ATMEL
void sama5d3xek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_SMC);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(5),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(8) | AT91_SMC_CYCLE_NRD(8),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(3) | AT91_SMC_TIMINGS_TADL(10) |
	       AT91_SMC_TIMINGS_TAR(3)  | AT91_SMC_TIMINGS_TRR(4)   |
	       AT91_SMC_TIMINGS_TWB(5)  | AT91_SMC_TIMINGS_RBNSEL(3)|
	       AT91_SMC_TIMINGS_NFSEL(1), &smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);
}
#endif

#ifdef CONFIG_MTD_NOR_FLASH
static void sama5d3xek_nor_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_SMC);

	/* Configure SMC CS0 for NOR flash */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[0].setup);
	writel(AT91_SMC_PULSE_NWE(10) | AT91_SMC_PULSE_NCS_WR(11) |
	       AT91_SMC_PULSE_NRD(10) | AT91_SMC_PULSE_NCS_RD(11),
	       &smc->cs[0].pulse);
	writel(AT91_SMC_CYCLE_NWE(11) | AT91_SMC_CYCLE_NRD(14),
	       &smc->cs[0].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(0) | AT91_SMC_TIMINGS_TADL(0)  |
	       AT91_SMC_TIMINGS_TAR(0)  | AT91_SMC_TIMINGS_TRR(0)   |
	       AT91_SMC_TIMINGS_TWB(0)  | AT91_SMC_TIMINGS_RBNSEL(0)|
	       AT91_SMC_TIMINGS_NFSEL(0), &smc->cs[0].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_16 |
	       AT91_SMC_MODE_TDF_CYCLE(1),
	       &smc->cs[0].mode);

	/* Address pin (A1 ~ A23) configuration */
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 1, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 2, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 3, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 4, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 5, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 6, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 7, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 8, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 9, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 10, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 11, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 12, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 13, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 14, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 15, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 16, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 17, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 18, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 19, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 20, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 21, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 22, 0);
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 23, 0);
	/* CS0 pin configuration */
	at91_pio3_set_a_periph(AT91_PIO_PORTE, 26, 0);
}
#endif

#ifdef CONFIG_CMD_USB
static void sama5d3xek_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTD, 25, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 26, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 27, 0);
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
static void sama5d3xek_mci_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTB, 10, 0);	/* MCI0 Power */
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

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_NAND_ATMEL
	sama5d3xek_nand_hw_init();
#endif
#ifdef CONFIG_MTD_NOR_FLASH
	sama5d3xek_nor_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	sama5d3xek_usb_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	sama5d3xek_mci_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	const int MAX_STR_LEN = 32;
	char name[MAX_STR_LEN], *p;
	int i;

	strncpy(name, get_cpu_name(), MAX_STR_LEN);
	for (i = 0, p = name; (*p) && (i < MAX_STR_LEN); p++, i++)
		*p = tolower(*p);

	strcat(name, "ek.dtb");
	env_set("dtb_name", name);
#endif
#ifdef CONFIG_VIDEO
	at91_video_show_board_info();
#endif
	return 0;
}
#endif

/* SPL */
#ifdef CONFIG_XPL_BUILD
void spl_board_init(void)
{
#if CONFIG_NAND_BOOT
	sama5d3xek_nand_hw_init();
#endif
}

static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_32_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_14 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
		    ATMEL_MPDDRC_CR_ENRDM_ON |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_NDQS_DISABLED |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED |
		    ATMEL_MPDDRC_CR_UNAL_SUPPORTED);
	/*
	 * As the DDR2-SDRAm device requires a refresh time is 7.8125us
	 * when DDR run at 133MHz, so it needs (7.8125us * 133MHz / 10^9) clocks
	 */
	ddr2->rtr = 0x411;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddr2->tpr1 = (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET |
		      200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      28 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      26 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      8 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void at91_mem_init(void)
{
	struct atmel_mpddrc_config ddr2;

	ddr2_conf(&ddr2);

	/* Enable MPDDR clock */
	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	at91_system_clk_enable(AT91_PMC_DDR);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddr2);
}

void at91_pmc_init(void)
{
	u32 tmp;

	tmp = AT91_PMC_PLLAR_29 |
	      AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
	      AT91_PMC_PLLXR_MUL(43) |
	      AT91_PMC_PLLXR_DIV(1);
	at91_plla_init(tmp);

	at91_pllicpr_init(AT91_PMC_IPLL_PLLA(0x3));

	tmp = AT91_PMC_MCKR_MDIV_4 |
	      AT91_PMC_MCKR_CSS_PLLA;
	at91_mck_init(tmp);
}
#endif
