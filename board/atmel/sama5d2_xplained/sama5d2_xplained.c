/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <atmel_hlcdc.h>
#include <debug_uart.h>
#include <dm.h>
#include <i2c.h>
#include <lcd.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>

DECLARE_GLOBAL_DATA_PTR;

static void board_usb_hw_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 10, 1);
}

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col = 480,
	.vl_row = 272,
	.vl_clk = 9000000,
	.vl_bpix = LCD_BPP,
	.vl_tft = 1,
	.vl_hsync_len = 41,
	.vl_left_margin = 2,
	.vl_right_margin = 2,
	.vl_vsync_len = 11,
	.vl_upper_margin = 2,
	.vl_lower_margin = 2,
	.mmio = ATMEL_BASE_LCDC,
};

/* No power up/down pin for the LCD pannel */
void lcd_enable(void)	{ /* Empty! */ }
void lcd_disable(void)	{ /* Empty! */ }

unsigned int has_lcdc(void)
{
	return 1;
}

static void board_lcd_hw_init(void)
{
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 28, 0);	/* LCDPWM */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 29, 0);	/* LCDDISP */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 30, 0);	/* LCDVSYNC */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 31, 0);	/* LCDHSYNC */
	atmel_pio4_set_a_periph(AT91_PIO_PORTD,  0, 0);	/* LCDPCK */
	atmel_pio4_set_a_periph(AT91_PIO_PORTD,  1, 0);	/* LCDDEN */

	/* LCDDAT0 */
	/* LCDDAT1 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 10, 0);	/* LCDDAT2 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 11, 0);	/* LCDDAT3 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 12, 0);	/* LCDDAT4 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 13, 0);	/* LCDDAT5 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 14, 0);	/* LCDDAT6 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 15, 0);	/* LCDDAT7 */

	/* LCDDAT8 */
	/* LCDDAT9 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 16, 0);	/* LCDDAT10 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 17, 0);	/* LCDDAT11 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 18, 0);	/* LCDDAT12 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 19, 0);	/* LCDDAT13 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 20, 0);	/* LCDDAT14 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 21, 0);	/* LCDDAT15 */

	/* LCDD16 */
	/* LCDD17 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 22, 0);	/* LCDDAT18 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 23, 0);	/* LCDDAT19 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 24, 0);	/* LCDDAT20 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 25, 0);	/* LCDDAT21 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 26, 0);	/* LCDDAT22 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTC, 27, 0);	/* LCDDAT23 */

	at91_periph_clk_enable(ATMEL_ID_LCDC);
}

#ifdef CONFIG_LCD_INFO
void lcd_show_board_info(void)
{
	ulong dram_size;
	int i;
	char temp[32];

	lcd_printf("%s\n", U_BOOT_VERSION);
	lcd_printf("2015 ATMEL Corp\n");
	lcd_printf("%s CPU at %s MHz\n", get_cpu_name(),
		   strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;

	lcd_printf("%ld MB SDRAM\n", dram_size >> 20);
}
#endif /* CONFIG_LCD_INFO */
#endif /* CONFIG_LCD */

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
static void board_uart1_hw_init(void)
{
	atmel_pio4_set_a_periph(AT91_PIO_PORTD, 2, 1);	/* URXD1 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTD, 3, 0);	/* UTXD1 */

	at91_periph_clk_enable(ATMEL_ID_UART1);
}

void board_debug_uart_init(void)
{
	board_uart1_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_LCD
	board_lcd_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	board_usb_hw_init();
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_CMD_I2C
static int set_ethaddr_from_eeprom(void)
{
	const int ETH_ADDR_LEN = 6;
	unsigned char ethaddr[ETH_ADDR_LEN];
	const char *ETHADDR_NAME = "ethaddr";
	struct udevice *bus, *dev;

	if (env_get(ETHADDR_NAME))
		return 0;

	if (uclass_get_device_by_seq(UCLASS_I2C, 1, &bus)) {
		printf("Cannot find I2C bus 1\n");
		return -1;
	}

	if (dm_i2c_probe(bus, AT24MAC_ADDR, 0, &dev)) {
		printf("Failed to probe I2C chip\n");
		return -1;
	}

	if (dm_i2c_read(dev, AT24MAC_REG, ethaddr, ETH_ADDR_LEN)) {
		printf("Failed to read ethernet address from EEPROM\n");
		return -1;
	}

	if (!is_valid_ethaddr(ethaddr)) {
		printf("The ethernet address read from EEPROM is not valid!\n");
		return -1;
	}

	return eth_env_set_enetaddr(ETHADDR_NAME, ethaddr);
}
#else
static int set_ethaddr_from_eeprom(void)
{
	return 0;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	set_ethaddr_from_eeprom();

	return 0;
}
#endif

/* SPL */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
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

	ddrc->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      9 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddrc->tpr1 = (27 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET |
		      29 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      0 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      3 << ATMEL_MPDDRC_TPR1_TXP_OFFSET);

	ddrc->tpr2 = (0 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET |
		      0 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      0 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      4 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET);
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct atmel_mpddr *mpddrc = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;
	struct atmel_mpddrc_config ddrc_config;
	u32 reg;

	ddrc_conf(&ddrc_config);

	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	writel(AT91_PMC_DDR, &pmc->scer);

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
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = AT91_PMC_PLLAR_29 |
	      AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
	      AT91_PMC_PLLXR_MUL(82) |
	      AT91_PMC_PLLXR_DIV(1);
	at91_plla_init(tmp);

	writel(0x0 << 8, &pmc->pllicpr);

	tmp = AT91_PMC_MCKR_H32MXDIV |
	      AT91_PMC_MCKR_PLLADIV_2 |
	      AT91_PMC_MCKR_MDIV_3 |
	      AT91_PMC_MCKR_CSS_PLLA;
	at91_mck_init(tmp);
}
#endif
