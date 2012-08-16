/*
 * (C) Copyright 2009-2012
 * Jens Scharsig  <esw@bus-elekronik.de>
 * BuS Elektronik GmbH & Co. KG
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

#include <config.h>
#include <common.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clk.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91sam9263.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91_common.h>
#include <lcd.h>
#include <i2c.h>
#include <atmel_lcdc.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_NAND
static void vl_ma2sc_nand_hw_init(void)
{
	unsigned long csa;
	at91_smc_t	*smc	= (at91_smc_t *) ATMEL_BASE_SMC0;
	at91_matrix_t	*matrix = (at91_matrix_t *) ATMEL_BASE_MATRIX;
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_pio_output(AT91_PIO_PORTA, 13, 1);	/* CAN_TX -> H */
	at91_set_pio_output(AT91_PIO_PORTA, 12, 1);	/* CAN_STB -> H */
	at91_set_pio_output(AT91_PIO_PORTA, 11, 1);	/* CAN_EN -> H */

	/* Enable CS3 */
	csa = readl(&matrix->csa[0]) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa[0]);

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
		AT91_SMC_MODE_DBW_8 |
		AT91_SMC_MODE_TDF_CYCLE(2),
		&smc->cs[3].mode);
	writel((1 << ATMEL_ID_PIOB) | (1 << ATMEL_ID_PIOCDE),
		&pmc->pcer);

	/* Configure RDY/BSY */
#ifdef CONFIG_SYS_NAND_READY_PIN
	at91_set_pio_input(CONFIG_SYS_NAND_READY_PIN, 1);
#endif
	/* Enable NandFlash */
	at91_set_pio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void vl_ma2sc_macb_hw_init(void)
{
	unsigned long	erstl;
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_rstc_t	*rstc	= (at91_rstc_t *) ATMEL_BASE_RSTC;
	/* Enable clock */
	writel(1 << ATMEL_ID_EMAC, &pmc->pcer);

	erstl = readl(&rstc->mr) & AT91_RSTC_MR_ERSTL_MASK;

	/* Need to reset PHY -> 500ms reset */
	writel(AT91_RSTC_KEY | AT91_RSTC_MR_ERSTL(0x0D) |
		AT91_RSTC_MR_URSTEN, &rstc->mr);

	writel(AT91_RSTC_KEY | AT91_RSTC_CR_EXTRST, &rstc->cr);
	/* Wait for end hardware reset */
	while (!(readl(&rstc->sr) & AT91_RSTC_SR_NRSTL))
		;

	/* Restore NRST value */
	writel(AT91_RSTC_KEY | erstl | AT91_RSTC_MR_URSTEN, &rstc->mr);

	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col =		320,
	.vl_row =		240,
	.vl_clk =		6500000,
	.vl_sync =		ATMEL_LCDC_INVDVAL_INVERTED |
				ATMEL_LCDC_INVLINE_INVERTED |
				ATMEL_LCDC_INVVD_INVERTED   |
				ATMEL_LCDC_INVFRAME_INVERTED,
	.vl_bpix =		(ATMEL_LCDC_PIXELSIZE_8 >> 5),
	.vl_tft =		1,
	.vl_hsync_len =		5,	/* Horiz Sync Pulse Width */
	.vl_left_margin =	68,	/* horiz back porch */
	.vl_right_margin =	20,	/* horiz front porch */
	.vl_vsync_len =		2,	/* vert Sync Pulse Width */
	.vl_upper_margin =	18,	/* vert back porch */
	.vl_lower_margin =	4,	/* vert front porch */
	.mmio =			ATMEL_BASE_LCDC,
};

void lcd_enable(void)
{
}

void lcd_disable(void)
{
}

static void vl_ma2sc_lcd_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* LCDVSYNC */
	at91_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* LCDHSYNC */
	at91_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* LCDDOTCK */
	at91_set_a_periph(AT91_PIO_PORTC, 3, 0);	/* LCDDEN */
	at91_set_b_periph(AT91_PIO_PORTB, 9, 0);	/* LCDCC */

	at91_set_a_periph(AT91_PIO_PORTC, 4, 0);	/* LCDD0 */
	at91_set_a_periph(AT91_PIO_PORTC, 5, 0);	/* LCDD1 */
	at91_set_a_periph(AT91_PIO_PORTC, 6, 0);	/* LCDD2 */
	at91_set_a_periph(AT91_PIO_PORTC, 7, 0);	/* LCDD3 */
	at91_set_a_periph(AT91_PIO_PORTC, 8, 0);	/* LCDD4 */
	at91_set_a_periph(AT91_PIO_PORTC, 9, 0);	/* LCDD5 */
	at91_set_a_periph(AT91_PIO_PORTC, 10, 0);	/* LCDD6 */
	at91_set_a_periph(AT91_PIO_PORTC, 11, 0);	/* LCDD7 */

	at91_set_a_periph(AT91_PIO_PORTC, 13, 0);	/* LCDD9 */
	at91_set_a_periph(AT91_PIO_PORTC, 14, 0);	/* LCDD10 */
	at91_set_a_periph(AT91_PIO_PORTC, 15, 0);	/* LCDD11 */
	at91_set_a_periph(AT91_PIO_PORTC, 16, 0);	/* LCDD12 */
	at91_set_b_periph(AT91_PIO_PORTC, 12, 0);	/* LCDD13 */
	at91_set_a_periph(AT91_PIO_PORTC, 18, 0);	/* LCDD14 */
	at91_set_a_periph(AT91_PIO_PORTC, 19, 0);	/* LCDD15 */

	at91_set_a_periph(AT91_PIO_PORTC, 20, 0);	/* LCDD26 */
	at91_set_a_periph(AT91_PIO_PORTC, 21, 0);	/* LCDD17 */
	at91_set_a_periph(AT91_PIO_PORTC, 22, 0);	/* LCDD18 */
	at91_set_a_periph(AT91_PIO_PORTC, 23, 0);	/* LCDD19 */
	at91_set_a_periph(AT91_PIO_PORTC, 24, 0);	/* LCDD20 */
	at91_set_b_periph(AT91_PIO_PORTC, 17, 0);	/* LCDD21 */
	at91_set_a_periph(AT91_PIO_PORTC, 26, 0);	/* LCDD22 */
	at91_set_a_periph(AT91_PIO_PORTC, 27, 0);	/* LCDD23 */

	at91_set_pio_output(AT91_PIO_PORTE, 0, 0);	/* LCD QXH */

	at91_set_pio_output(AT91_PIO_PORTE, 2, 0);	/* LCD SHUT */
	at91_set_pio_output(AT91_PIO_PORTE, 3, 1);	/* LCD TopBottom */
	at91_set_pio_output(AT91_PIO_PORTE, 4, 0);	/* LCD REV */
	at91_set_pio_output(AT91_PIO_PORTE, 5, 1);	/* LCD RightLeft */
	at91_set_pio_output(AT91_PIO_PORTE, 6, 0);	/* LCD Color Mode CM */
	at91_set_pio_output(AT91_PIO_PORTE, 7, 0);	/* LCD BGR */

	at91_set_pio_output(AT91_PIO_PORTB, 9, 0);	/* LCD CC */

	writel(1 << ATMEL_ID_LCDC, &pmc->pcer);
	gd->fb_base = ATMEL_BASE_SRAM0;
}
#endif /* Config LCD */

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable clocks for all PIOs */
	writel((1 << ATMEL_ID_PIOA) | (1 << ATMEL_ID_PIOB) |
		(1 << ATMEL_ID_PIOCDE),
		&pmc->pcer);

	at91_seriald_hw_init();

	return 0;
}
#endif

int board_init(void)
{
	at91_smc_t	*smc	= (at91_smc_t *) ATMEL_BASE_SMC0;
	at91_pio_t	*pio = (at91_pio_t *) ATMEL_BASE_PIO;
	u32		pin;

	pin = 0x1F000001;
	writel(pin, &pio->pioa.idr);
	writel(pin, &pio->pioa.pudr);
	writel(pin, &pio->pioa.per);
	writel(pin, &pio->pioa.oer);
	writel(pin, &pio->pioa.sodr);
	writel((1 << 25), &pio->pioa.codr);

	pin = 0x1F000100;
	writel(pin, &pio->piob.idr);
	writel(pin, &pio->piob.pudr);
	writel(pin, &pio->piob.per);
	writel(pin, &pio->piob.oer);
	writel(pin, &pio->piob.codr);
	writel((1 << 24), &pio->piob.sodr);

	pin = 0x40000000;			/* Pullup DRxD enbable */
	writel(pin, &pio->pioc.puer);

	pin = 0x0000000F;			/* HWversion als Input */
	writel(pin, &pio->piod.idr);
	writel(pin, &pio->piod.puer);
	writel(pin, &pio->piod.per);
	writel(pin, &pio->piod.odr);
	writel(pin, &pio->piod.owdr);

	gd->bd->bi_arch_number = MACH_TYPE_VL_MA2SC;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	writel(CONFIG_SYS_SMC0_MODE0_VAL, &smc->cs[0].setup);
	writel(CONFIG_SYS_SMC0_CYCLE0_VAL, &smc->cs[0].cycle);
	writel(CONFIG_SYS_SMC0_PULSE0_VAL, &smc->cs[0].pulse);
	writel(CONFIG_SYS_SMC0_SETUP0_VAL, &smc->cs[0].setup);

#ifdef CONFIG_CMD_NAND
	vl_ma2sc_nand_hw_init();
#endif
#ifdef CONFIG_MACB
	vl_ma2sc_macb_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91_uhp_hw_init();
#endif
#ifdef CONFIG_LCD
	vl_ma2sc_lcd_hw_init();
#endif
	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	uchar	buffer[8];
	at91_pio_t	*pio = (at91_pio_t *) ATMEL_BASE_PIO;
	u32		pin;

	buffer[0] = 0x04;
	buffer[1] = 0x00;
	if (i2c_write(0x68, 0x0E, 1, buffer, 2) != 0)
		puts("error reseting rtc clock\n\0");

	/* read hardware version */

	pin = (readl(&pio->piod.pdsr) & 0x0F) + 0x44;
	printf("Board: revision %c\n", pin);
	buffer[0] = pin;
	buffer[1] = 0;
	setenv("revision", (char *) buffer);

	pin = 0x40000000;			/* Pullup DRxD enbable */
	writel(pin, &pio->pioc.puer);
	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *) CONFIG_SYS_SDRAM_BASE,
			CONFIG_SYS_SDRAM_SIZE);
	return 0;
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
	rc = macb_eth_initialize(0, (void *) ATMEL_BASE_EMAC, 0x01);
#endif
	return rc;
}

#ifdef CONFIG_SOFT_I2C
void i2c_init_board(void)
{
	u32 pin;

	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	u8 sda = (1<<4);
	u8 scl = (1<<5);

	writel(1 << ATMEL_ID_PIOB, &pmc->pcer);
	pin = sda | scl;
	writel(pin, &pio->piob.idr);	/* Disable Interupt */
	writel(pin, &pio->piob.pudr);
	writel(pin, &pio->piob.per);
	writel(pin, &pio->piob.oer);
	writel(pin, &pio->piob.sodr);
}
#endif

void watchdog_reset(void)
{
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	u32	pin = 0x1;	/* PA0 */

	if ((readl(&pio->pioa.odsr) & pin) > 0)
		writel(pin, &pio->pioa.codr);
	else
		writel(pin, &pio->pioa.sodr);
}

void enable_caches(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	dcache_enable();
#endif
}

/*---------------------------------------------------------------------------*/

int do_ledtest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rcode = 1;
	int row;
	int col;
	u32 pinz;
	u32 pins;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;

	at91_set_pio_output(AT91_PIO_PORTB, 8, 0);	/* LCD DIM */

	pins = 0x1F000000;
	writel(pins, &pio->pioa.idr);
	writel(pins, &pio->pioa.pudr);
	writel(pins, &pio->pioa.per);
	writel(pins, &pio->pioa.oer);
	writel(pins, &pio->pioa.sodr);

	pinz = 0x1F000000;
	writel(pinz, &pio->piob.idr);
	writel(pinz, &pio->piob.pudr);
	writel(pinz, &pio->piob.per);
	writel(pinz, &pio->piob.oer);
	writel(pinz, &pio->piob.sodr);

	for (row = 0; row < 5; row++) {
		for (col = 0; col < 5; col++) {
			writel((0x01000000 << col), &pio->piob.sodr);
			writel((0x01000000 << row), &pio->pioa.codr);
			printf("LED Test %d x %d\n", row, col);
			udelay(1000000);
			writel(pinz, &pio->piob.codr);
			writel(pins, &pio->pioa.sodr);
		}
	}
	return rcode;
}

void poweroff(void)
{
	watchdog_reset();
	at91_set_pio_output(AT91_PIO_PORTA, 13, 1);	/* CAN_TX -> H */
	udelay(100);
	at91_set_pio_output(AT91_PIO_PORTA, 12, 0);	/* CAN_STB -> L */
	udelay(100);
	at91_set_pio_output(AT91_PIO_PORTA, 11, 0);	/* CAN_EN -> L */
	udelay(100);
	while (1)
		watchdog_reset();
}

int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc,  char * const argv[])
{
	int rcode = 1;
	poweroff();
	return rcode;
}

int do_beep(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	u32 freq;
	u32 durate;
	int rcode = 1;

	freq = 1000;
	durate = 2;
	switch (argc) {
	case 3:
		durate = simple_strtoul(argv[2], NULL, 10);
	case 2:
		freq = simple_strtoul(argv[1], NULL, 10);
	case 1:
		break;
	default:
		cmd_usage(cmdtp);
		rcode = 1;
		break;
	}
	durate = durate * freq;
	freq = 500000 / freq;
	for (i = 0; i < durate; i++) {
		at91_set_pio_output(AT91_PIO_PORTB, 29, 1);	/* Sound On*/
		udelay(freq);
		at91_set_pio_output(AT91_PIO_PORTB, 29, 0);	/* Sound Off*/
		udelay(freq);
	}
	at91_set_pio_output(AT91_PIO_PORTB, 29, 0);	/* Sound Off*/
	return rcode;
}

int do_keytest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rcode = 1;
	int row;
	u32 col;
	u32 pinz;
	u32 pins;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	writel((1 << ATMEL_ID_PIOA), &pmc->pcer);

	pins = 0x001F0000;
	writel(pins, &pio->pioa.idr);
	writel(pins, &pio->pioa.pudr);
	writel(pins, &pio->pioa.per);
	writel(pins, &pio->pioa.odr);

	pinz = 0x000F0000;
	writel(pinz, &pio->piob.idr);
	writel(pinz, &pio->piob.pudr);
	writel(pinz, &pio->piob.per);
	writel(pinz, &pio->piob.oer);
	writel(pinz, &pio->piob.codr);

	while (1) {
		col = 0;
		for (row = 0; row < 4; row++) {
			writel((0x00010000 << row), &pio->piob.sodr);
			udelay(10000);
			col <<= 4;
			col |= ((readl(&pio->pioa.pdsr) >> 16) & 0xF) ^ 0xF ;
			writel(pinz, &pio->piob.codr);
		}
		printf("Matix: ");
		for (row = 0; row < 16; row++) {
			printf("%1.1d", col & 1);
			col >>= 1;
		}
		printf(" SP %d\r ",
			1 ^ (1 & (readl(&pio->piob.pdsr) >> 20)));
		if ((1 & (readl(&pio->pioa.pdsr) >> 1)) == 0) {
			/* SHUTDOWN */
			row = 0;
			while (row < 1000) {
				if ((1 & (readl(&pio->pioa.pdsr) >> 1)) == 0)
					row++;
				udelay(100);
			}
			udelay(100000);
			row = 0;
			while (row < 1000) {
				if ((1 & (readl(&pio->pioa.pdsr) >> 1)) > 0) {
					row++;
					udelay(1000);
				}
			}
			poweroff();
			while (1)
				;
		}
	}
	return rcode;
}

/*****************************************************************************/

U_BOOT_CMD(
	ledtest,	1,	0,	do_ledtest,
	"test ledmatrix",
	"\n"
	);

U_BOOT_CMD(
	keytest,	1,	0,	do_keytest,
	"test keymatix and special keys, poweroff on pressing ON key",
	"\n"
	);

U_BOOT_CMD(
	poweroff,	1,	0,	do_poweroff,
	"power off",
	"\n"
	);

U_BOOT_CMD(
	beep,	3,	0,	do_beep,
	"[freq [duration]]",
	"freq frequence of beep\nduration duration of beep\n"
	);

/*****************************************************************************/
