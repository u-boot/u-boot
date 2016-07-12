/*
 * Copyright (C) 2016 Atmel
 *		      Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <atmel_hlcdc.h>
#include <lcd.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <spi.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_usba_udc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5_sfr.h>
#include <asm/arch/sama5d2.h>
#include <asm/arch/sama5d3_smc.h>

DECLARE_GLOBAL_DATA_PTR;

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 17, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 17, 1);
}

static void board_spi0_hw_init(void)
{
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 14, 0);
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 15, 0);
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 16, 0);

	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 17, 1);

	at91_periph_clk_enable(ATMEL_ID_SPI0);
}

static void board_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct atmel_sfr *sfr = (struct atmel_sfr *)ATMEL_BASE_SFR;

	at91_periph_clk_enable(ATMEL_ID_HSMC);

	writel(AT91_SFR_EBICFG_DRIVE0_HIGH |
	       AT91_SFR_EBICFG_PULL0_NONE |
	       AT91_SFR_EBICFG_DRIVE1_HIGH |
	       AT91_SFR_EBICFG_PULL1_NONE, &sfr->ebicfg);

	/* Configure SMC CS3 for NAND */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(2) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(2) | AT91_SMC_PULSE_NCS_RD(3),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(2) | AT91_SMC_TIMINGS_TADL(7) |
	       AT91_SMC_TIMINGS_TAR(2)  | AT91_SMC_TIMINGS_TRR(3)   |
	       AT91_SMC_TIMINGS_TWB(7)  | AT91_SMC_TIMINGS_RBNSEL(3) |
	       AT91_SMC_TIMINGS_NFSEL(1), &smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 0, 0);	/* D0 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 1, 0);	/* D1 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 2, 0);	/* D2 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 3, 0);	/* D3 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 4, 0);	/* D4 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 5, 0);	/* D5 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 6, 0);	/* D6 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 7, 0);	/* D7 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 12, 0);	/* RE */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 8, 0);	/* WE */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 9, 1);	/* NCS */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 21, 1);	/* RDY */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 10, 1);	/* ALE */
	atmel_pio4_set_f_periph(AT91_PIO_PORTA, 11, 1);	/* CLE */
}

static void board_usb_hw_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 28, 1);
}

static void board_gmac_hw_init(void)
{
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 14, 0);	/* GTXCK */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 15, 0);	/* GTXEN */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 16, 0);	/* GRXDV */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 17, 0);	/* GRXER */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 18, 0);	/* GRX0 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 19, 0);	/* GRX1 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 20, 0);	/* GTX0 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 21, 0);	/* GTX1 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 22, 0);	/* GMDC */
	atmel_pio4_set_f_periph(AT91_PIO_PORTB, 23, 0);	/* GMDIO */

	at91_periph_clk_enable(ATMEL_ID_GMAC);
}

static void board_uart0_hw_init(void)
{
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 26, 1);	/* URXD0 */
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 27, 0);	/* UTXD0 */

	at91_periph_clk_enable(CONFIG_USART_ID);
}

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOD);

	board_uart0_hw_init();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_ATMEL_SPI
	board_spi0_hw_init();
#endif
#ifdef CONFIG_NAND_ATMEL
	board_nand_hw_init();
#endif
#ifdef CONFIG_MACB
	board_gmac_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	board_usb_hw_init();
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	at91_udp_hw_init();
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
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_GMAC, 0x00);
	if (rc)
		printf("GMAC register failed\n");
#endif

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	usba_udc_probe(&pdata);
#ifdef CONFIG_USB_ETH_RNDIS
	usb_eth_initialize(bis);
#endif
#endif

	return rc;
}

/* SPL */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
#ifdef CONFIG_SYS_USE_SERIALFLASH
	board_spi0_hw_init();
#endif

#ifdef CONFIG_SYS_USE_NANDFLASH
	board_nand_hw_init();
#endif
}

static void ddrc_conf(struct atmel_mpddrc_config *ddrc)
{
	ddrc->md = (ATMEL_MPDDRC_MD_DBW_32_BITS | ATMEL_MPDDRC_MD_DDR3_SDRAM);

	ddrc->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_14 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS5 |
		    ATMEL_MPDDRC_CR_DIC_DS |
		    ATMEL_MPDDRC_CR_DIS_DLL |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED |
		    ATMEL_MPDDRC_CR_UNAL_SUPPORTED);

	ddrc->rtr = 0x511;

	ddrc->tpr0 = ((6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TWR_OFFSET) |
		      (9 << ATMEL_MPDDRC_TPR0_TRC_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR0_TRP_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET));

	ddrc->tpr1 = ((27 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET) |
		      (29 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET) |
		      (0 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR1_TXP_OFFSET));

	ddrc->tpr2 = ((0 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET) |
		      (0 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET) |
		      (0 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET) |
		      (7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET));
}

void mem_init(void)
{
	struct atmel_mpddr *mpddrc = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;
	struct atmel_mpddrc_config ddrc_config;
	u32 reg;

	ddrc_conf(&ddrc_config);

	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	at91_system_clk_enable(AT91_PMC_DDR);

	reg = readl(&mpddrc->io_calibr);
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_RDIV;
	reg |= ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_55;
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_TZQIO;
	reg |= ATMEL_MPDDRC_IO_CALIBR_TZQIO_(100);
	writel(reg, &mpddrc->io_calibr);

	writel(ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_TWO_CYCLE,
	       &mpddrc->rd_data_path);

	ddr3_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddrc_config);

	writel(0x3, &mpddrc->cal_mr4);
	writel(64, &mpddrc->tim_cal);
}

void at91_pmc_init(void)
{
	at91_plla_init(AT91_PMC_PLLAR_29 |
		       AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
		       AT91_PMC_PLLXR_MUL(82) |
		       AT91_PMC_PLLXR_DIV(1));

	at91_pllicpr_init(0);

	at91_mck_init(AT91_PMC_MCKR_H32MXDIV |
		      AT91_PMC_MCKR_PLLADIV_2 |
		      AT91_PMC_MCKR_MDIV_3 |
		      AT91_PMC_MCKR_CSS_PLLA);
}
#endif
