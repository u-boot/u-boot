/*
 * (C) Copyright 2010-2011
 * Daniel Gorsulowski <daniel.gorsulowski@esd.eu>
 * esd electronic system design gmbh <www.esd.eu>
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/clk.h>
#include <netdev.h>
#ifdef CONFIG_LCD
# include <atmel_lcdc.h>
# include <lcd.h>
# ifdef CONFIG_LCD_INFO
#  include <nand.h>
#  include <version.h>
# endif
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

static int hw_rev = -1;	/* hardware revision */

int get_hw_rev(void)
{
	if (hw_rev >= 0)
		return hw_rev;

	hw_rev = at91_get_pio_value(AT91_PIO_PORTB, 19);
	hw_rev |= at91_get_pio_value(AT91_PIO_PORTB, 20) << 1;
	hw_rev |= at91_get_pio_value(AT91_PIO_PORTB, 21) << 2;
	hw_rev |= at91_get_pio_value(AT91_PIO_PORTB, 22) << 3;

	if (hw_rev == 15)
		hw_rev = 0;

	return hw_rev;
}

#ifdef CONFIG_CMD_NAND
static void otc570_nand_hw_init(void)
{
	unsigned long csa;
	at91_smc_t	*smc 	= (at91_smc_t *) ATMEL_BASE_SMC0;
	at91_matrix_t	*matrix = (at91_matrix_t *) ATMEL_BASE_MATRIX;

	/* Enable CS3 */
	csa = readl(&matrix->csa[0]) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa[0]);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(1) |
		AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(2),
		&smc->cs[3].setup);

	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
		&smc->cs[3].pulse);

	writel(AT91_SMC_CYCLE_NWE(6) | AT91_SMC_CYCLE_NRD(6),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
		AT91_SMC_MODE_DBW_8 |
		AT91_SMC_MODE_TDF_CYCLE(12),
		&smc->cs[3].mode);

	/* Configure RDY/BSY */
	gpio_direction_input(CONFIG_SYS_NAND_READY_PIN);

	/* Enable NandFlash */
	gpio_direction_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif /* CONFIG_CMD_NAND */

#ifdef CONFIG_MACB
static void otc570_macb_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;
	/* Enable clock */
	writel(1 << ATMEL_ID_EMAC, &pmc->pcer);
	at91_macb_hw_init();
}
#endif

/*
 * Static memory controller initialization to enable Beckhoff ET1100 EtherCAT
 * controller debugging
 * The ET1100 is located at physical address 0x70000000
 * Its process memory is located at physical address 0x70001000
 */
static void otc570_ethercat_hw_init(void)
{
	at91_smc_t	*smc1 	= (at91_smc_t *) ATMEL_BASE_SMC1;

	/* Configure SMC EBI1_CS0 for EtherCAT */
	writel(AT91_SMC_SETUP_NWE(0) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(0) | AT91_SMC_SETUP_NCS_RD(0),
		&smc1->cs[0].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(9) |
		AT91_SMC_PULSE_NRD(5) | AT91_SMC_PULSE_NCS_RD(9),
		&smc1->cs[0].pulse);
	writel(AT91_SMC_CYCLE_NWE(10) | AT91_SMC_CYCLE_NRD(6),
		&smc1->cs[0].cycle);
	/*
	 * Configure behavior at external wait signal, byte-select mode, 16 bit
	 * data bus width, none data float wait states and TDF optimization
	 */
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_EXNW_READY |
		AT91_SMC_MODE_DBW_16 | AT91_SMC_MODE_TDF_CYCLE(0) |
		AT91_SMC_MODE_TDF, &smc1->cs[0].mode);

	/* Configure RDY/BSY */
	at91_set_b_periph(AT91_PIO_PORTE, 20, 0);	/* EBI1_NWAIT */
}

#ifdef CONFIG_LCD
/* Number of columns and rows, pixel clock in Hz and hsync/vsync polarity */
vidinfo_t panel_info = {
	.vl_col =		640,
	.vl_row =		480,
	.vl_clk =		25175000,
	.vl_sync =		ATMEL_LCDC_INVLINE_INVERTED |
				ATMEL_LCDC_INVFRAME_INVERTED,

	.vl_bpix =		LCD_BPP,/* Bits per pixel, 0 = 1bit, 3 = 8bit */
	.vl_tft =		1,	/* 0 = passive, 1 = TFT */
	.vl_vsync_len =		1,	/* Length of vertical sync in NOL */
	.vl_upper_margin =	35,	/* Idle lines at the frame start */
	.vl_lower_margin =	5,	/* Idle lines at the end of the frame */
	.vl_hsync_len =		5,	/* Width of the LCDHSYNC pulse */
	.vl_left_margin =	112,	/* Idle cycles at the line beginning */
	.vl_right_margin =	1,	/* Idle cycles at the end of the line */

	.mmio =			ATMEL_BASE_LCDC,
};

void lcd_enable(void)
{
	at91_set_pio_value(AT91_PIO_PORTA, 30, 0); /* power up */
}

void lcd_disable(void)
{
	at91_set_pio_value(AT91_PIO_PORTA, 30, 1); /* power down */
}

static void otc570_lcd_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* LCDVSYNC */
	at91_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* LCDHSYNC */
	at91_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* LCDDOTCK */
	at91_set_a_periph(AT91_PIO_PORTC, 3, 0);	/* LCDDEN */
	at91_set_b_periph(AT91_PIO_PORTB, 9, 0);	/* LCDCC */
	at91_set_a_periph(AT91_PIO_PORTC, 6, 0);	/* LCDD2 */
	at91_set_a_periph(AT91_PIO_PORTC, 7, 0);	/* LCDD3 */
	at91_set_a_periph(AT91_PIO_PORTC, 8, 0);	/* LCDD4 */
	at91_set_a_periph(AT91_PIO_PORTC, 9, 0);	/* LCDD5 */
	at91_set_a_periph(AT91_PIO_PORTC, 10, 0);	/* LCDD6 */
	at91_set_a_periph(AT91_PIO_PORTC, 11, 0);	/* LCDD7 */
	at91_set_a_periph(AT91_PIO_PORTC, 14, 0);	/* LCDD10 */
	at91_set_a_periph(AT91_PIO_PORTC, 15, 0);	/* LCDD11 */
	at91_set_a_periph(AT91_PIO_PORTC, 16, 0);	/* LCDD12 */
	at91_set_b_periph(AT91_PIO_PORTC, 12, 0);	/* LCDD13 */
	at91_set_a_periph(AT91_PIO_PORTC, 18, 0);	/* LCDD14 */
	at91_set_a_periph(AT91_PIO_PORTC, 19, 0);	/* LCDD15 */
	at91_set_a_periph(AT91_PIO_PORTC, 22, 0);	/* LCDD18 */
	at91_set_a_periph(AT91_PIO_PORTC, 23, 0);	/* LCDD19 */
	at91_set_a_periph(AT91_PIO_PORTC, 24, 0);	/* LCDD20 */
	at91_set_b_periph(AT91_PIO_PORTC, 17, 0);	/* LCDD21 */
	at91_set_a_periph(AT91_PIO_PORTC, 26, 0);	/* LCDD22 */
	at91_set_a_periph(AT91_PIO_PORTC, 27, 0);	/* LCDD23 */
	at91_set_pio_output(AT91_PIO_PORTA, 30, 1);	/* PCI */

	writel(1 << ATMEL_ID_LCDC, &pmc->pcer);
}

#ifdef CONFIG_LCD_INFO
void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i].size;

	lcd_printf("\n%s\n", U_BOOT_VERSION);
	lcd_printf("CPU at %s MHz\n", strmhz(temp, get_cpu_clk_rate()));
	lcd_printf("  %ld MB SDRAM, %ld MB NAND\n",
		dram_size >> 20,
		nand_size >> 20 );
	lcd_printf("  Board            : esd ARM9 HMI Panel - OTC570\n");
	lcd_printf("  Hardware-revision: 1.%d\n", get_hw_rev());
	lcd_printf("  Mach-type        : %lu\n", gd->bd->bi_arch_number);
}
#endif /* CONFIG_LCD_INFO */
#endif /* CONFIG_LCD */

int dram_init(void)
{
	gd->ram_size = get_ram_size(
		(void *)CONFIG_SYS_SDRAM_BASE,
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

int checkboard(void)
{
	char str[32];

	puts("Board            : esd ARM9 HMI Panel - OTC570");
	if (getenv_f("serial#", str, sizeof(str)) > 0) {
		puts(", serial# ");
		puts(str);
	}
	printf("\n");
	printf("Hardware-revision: 1.%d\n", get_hw_rev());
	printf("Mach-type        : %lu\n", gd->bd->bi_arch_number);
	return 0;
}

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *str;

	char *serial = getenv("serial#");
	if (serial) {
		str = strchr(serial, '_');
		if (str && (strlen(str) >= 4)) {
			serialnr->high = (*(str + 1) << 8) | *(str + 2);
			serialnr->low = simple_strtoul(str + 3, NULL, 16);
		}
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}
#endif

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
{
	return hw_rev | 0x100;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char		str[64];
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_pio_output(AT91_PIO_PORTA, 29, 1);
	at91_set_a_periph(AT91_PIO_PORTA, 26, 1);	/* TXD0 */
	at91_set_a_periph(AT91_PIO_PORTA, 27, 0);	/* RXD0 */
	writel(1 << ATMEL_ID_USART0, &pmc->pcer);
	/* Set USART_MODE = 1 (RS485) */
	writel(1, 0xFFF8C004);

	printf("USART0: ");

	if (getenv_f("usart0", str, sizeof(str)) == -1) {
		printf("No entry - assuming 1-wire\n");
		/* CTS pin, works as mode select pin (0 = 1-wire; 1 = RS485) */
		at91_set_pio_output(AT91_PIO_PORTA, 29, 0);
	} else {
		if (strcmp(str, "1-wire") == 0) {
			printf("%s\n", str);
			at91_set_pio_output(AT91_PIO_PORTA, 29, 0);
		} else if (strcmp(str, "rs485") == 0) {
			printf("%s\n", str);
			at91_set_pio_output(AT91_PIO_PORTA, 29, 1);
		} else {
			printf("Wrong entry - assuming 1-wire ");
			printf("(valid values are '1-wire' or 'rs485')\n");
			at91_set_pio_output(AT91_PIO_PORTA, 29, 0);
		}
	}
#ifdef CONFIG_LCD
	printf("Display memory address: 0x%08lX\n", gd->fb_base);
#endif

	return 0;
}
#endif /* CONFIG_MISC_INIT_R */

int board_early_init_f(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	/* enable all clocks */
	writel((1 << ATMEL_ID_PIOA) |
		(1 << ATMEL_ID_PIOB) |
		(1 << ATMEL_ID_PIOCDE) |
		(1 << ATMEL_ID_TWI) |
		(1 << ATMEL_ID_SPI0) |
#ifdef CONFIG_LCD
		(1 << ATMEL_ID_LCDC) |
#endif
		(1 << ATMEL_ID_UHP),
		&pmc->pcer);

	at91_seriald_hw_init();

	/* arch number of OTC570-Board */
	gd->bd->bi_arch_number = MACH_TYPE_OTC570;

	return 0;
}

int board_init(void)
{
	/* initialize ET1100 Controller */
	otc570_ethercat_hw_init();

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	otc570_nand_hw_init();
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_MACB
	otc570_macb_hw_init();
#endif
#ifdef CONFIG_AT91_CAN
	at91_can_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91_uhp_hw_init();
#endif
#ifdef CONFIG_LCD
	otc570_lcd_hw_init();
#endif
	return 0;
}
