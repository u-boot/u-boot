/*
 * (C) Copyright 2008-2009
 * BuS Elektronik GmbH & Co. KG <www.bus-elektronik.de>
 * Jens Scharsig <esw@bus-elektronik.de>
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
#include <exports.h>
#include <net.h>
#include <netdev.h>
#include <nand.h>

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_mc.h>
#include <asm/arch/at91_common.h>

#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

#ifdef CONFIG_VIDEO
#include <bus_vcxk.h>

extern unsigned long display_width;
extern unsigned long display_height;
#endif

#ifdef CONFIG_CMD_NAND
void cpux9k2_nand_hw_init(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	at91_pio_t *pio	= (at91_pio_t *) ATMEL_BASE_PIO;

	/* Correct IRDA resistor problem / Set PA23_TXD in Output */
	writel(ATMEL_PMX_AA_TXD2, &pio->pioa.oer);

	gd->bd->bi_arch_number = MACH_TYPE_EB_CPUX9K2;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_STATUS_LED
	status_led_set(STATUS_LED_BOOT, STATUS_LED_ON);
#endif
#ifdef CONFIG_CMD_NAND
	cpux9k2_nand_hw_init();
#endif
	return 0;
}

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

#ifdef CONFIG_MISC_INIT_R

int misc_init_r(void)
{
	uchar	mac[8];
	uchar	tm;
	uchar	midx;
	uchar	macn6, macn7;

	if (getenv("ethaddr") == NULL) {
		if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0x00,
				CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
				(uchar *) &mac, sizeof(mac)) != 0) {
			puts("Error reading MAC from EEPROM\n");
		} else {
			tm = 0;
			macn6 = 0;
			macn7 = 0xFF;
			for (midx = 0; midx < 6; midx++) {
				if ((mac[midx] != 0) && (mac[midx] != 0xFF))
					tm++;
				macn6 += mac[midx];
				macn7 ^= mac[midx];
			}
			if ((macn6 != mac[6]) || (macn7 != mac[7]))
				tm = 0;
			if (tm)
				eth_setenv_enetaddr("ethaddr", mac);
			 else
				puts("Error: invalid MAC at EEPROM\n");
		}
	}
	gd->jt[XF_do_reset] = (void *) do_reset;

#ifdef CONFIG_STATUS_LED
	status_led_set(STATUS_LED_BOOT, STATUS_LED_BLINKING);
#endif
	return 0;
}
#endif

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
	udelay(10000);
	eth_init(gd->bd);
}
#endif

/*
 * DRAM initialisations
 */

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

/*
 * Ethernet initialisations
 */

#ifdef CONFIG_DRIVER_AT91EMAC
int board_eth_init(bd_t *bis)
{
	int rc = 0;
	rc = at91emac_register(bis, (u32) ATMEL_BASE_EMAC);
	return rc;
}
#endif

/*
 * Disk On Chip (NAND) Millenium initialization.
 * The NAND lives in the CS2* space
 */
#if defined(CONFIG_CMD_NAND)

#define	MASK_ALE	(1 << 22)	/* our ALE is AD22 */
#define	MASK_CLE	(1 << 21)	/* our CLE is AD21 */

void cpux9k2_nand_hw_init(void)
{
	unsigned long csr;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_mc_t *mc = (at91_mc_t *) ATMEL_BASE_MC;

	/* Setup Smart Media, fitst enable the address range of CS3 */
	writel(readl(&mc->ebi.csa) | AT91_EBI_CSA_CS3A, &mc->ebi.csa);

	/* RWH = 1 | RWS = 0 | TDF = 1 | NWS = 3 */
	csr =	AT91_SMC_CSR_RWHOLD(1) | AT91_SMC_CSR_TDF(1) |
		AT91_SMC_CSR_NWS(3) |
		AT91_SMC_CSR_ACSS_STANDARD | AT91_SMC_CSR_DBW_8 |
		AT91_SMC_CSR_WSEN;
	writel(csr, &mc->smc.csr[3]);

	writel(ATMEL_PMX_CA_SMOE | ATMEL_PMX_CA_SMWE, &pio->pioc.asr);
	writel(ATMEL_PMX_CA_BFCK | ATMEL_PMX_CA_SMOE | ATMEL_PMX_CA_SMWE,
		&pio->pioc.pdr);

	/* Configure PC2 as input (signal Nand READY ) */
	writel(ATMEL_PMX_CA_BFAVD, &pio->pioc.per);
	writel(ATMEL_PMX_CA_BFAVD, &pio->pioc.odr); /* disable output */
	writel(ATMEL_PMX_CA_BFCK, &pio->pioc.codr);

	/* PIOC clock enabling */
	writel(1 << ATMEL_ID_PIOC, &pmc->pcer);
}

static void board_nand_hwcontrol(struct mtd_info *mtd,
	int cmd, unsigned int ctrl)
{
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	struct nand_chip *this = mtd->priv;
	ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;

	if (ctrl & NAND_CTRL_CHANGE) {
		IO_ADDR_W &= ~(MASK_ALE | MASK_CLE);

		if (ctrl & NAND_CLE)
			IO_ADDR_W |= MASK_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= MASK_ALE;

		if ((ctrl & NAND_NCE))
			writel(1, &pio->pioc.codr);
		else
			writel(1, &pio->pioc.sodr);

		this->IO_ADDR_W = (void *) IO_ADDR_W;
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

static int board_nand_dev_ready(struct mtd_info *mtd)
{
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	return ((readl(&pio->pioc.pdsr) & (1 << 2)) != 0);
}

int board_nand_init(struct nand_chip *nand)
{
	cpux9k2_nand_hw_init();
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->cmd_ctrl = board_nand_hwcontrol;
	nand->dev_ready = board_nand_dev_ready;
	nand->chip_delay = 20;
	return 0;
}

#endif

#if defined(CONFIG_VIDEO)
/*
 * drv_video_init
 * FUNCTION: initialize VCxK device
 */

int drv_video_init(void)
{
#ifdef CONFIG_SPLASH_SCREEN
	unsigned long splash;
#endif
	char *s;
	unsigned long csr;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_mc_t *mc = (at91_mc_t *) ATMEL_BASE_MC;

	printf("Init Video as ");
	s = getenv("displaywidth");
	if (s != NULL)
		display_width = simple_strtoul(s, NULL, 10);
	else
		display_width = 256;
	s = getenv("displayheight");
	if (s != NULL)
		display_height = simple_strtoul(s, NULL, 10);
	else
		display_height = 256;
	printf("%ld x %ld pixel matrix\n", display_width, display_height);

	/* RWH = 2 | RWS =2  | TDF = 4 | NWS = 0x6 */
	csr =	AT91_SMC_CSR_RWHOLD(2) | AT91_SMC_CSR_RWSETUP(2) |
		AT91_SMC_CSR_TDF(4) | AT91_SMC_CSR_NWS(6) |
		AT91_SMC_CSR_ACSS_STANDARD | AT91_SMC_CSR_DBW_16 |
		AT91_SMC_CSR_BAT_16 | AT91_SMC_CSR_WSEN;
	writel(csr, &mc->smc.csr[2]);
	writel(1 << ATMEL_ID_PIOB, &pmc->pcer);

	vcxk_init(display_width, display_height);
#ifdef CONFIG_SPLASH_SCREEN
	s = getenv("splashimage");
	if (s != NULL) {
		splash = simple_strtoul(s, NULL, 16);
		printf("use splashimage: %lx\n", splash);
		video_display_bitmap(splash, 0, 0);
	}
#endif
	return 0;
}
#endif

#ifdef CONFIG_SOFT_I2C

void i2c_init_board(void)
{
	u32 pin;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;

	writel(1 << ATMEL_ID_PIOA, &pmc->pcer);
	pin = ATMEL_PMX_AA_TWD | ATMEL_PMX_AA_TWCK;
	writel(pin, &pio->pioa.idr);
	writel(pin, &pio->pioa.pudr);
	writel(pin, &pio->pioa.per);
	writel(pin, &pio->pioa.oer);
	writel(pin, &pio->pioa.sodr);
}

#endif

/*--------------------------------------------------------------------------*/

#ifdef CONFIG_STATUS_LED

void __led_toggle(led_id_t mask)
{
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;

	if (readl(&pio->piod.odsr) & mask)
		writel(mask, &pio->piod.codr);
	else
		writel(mask, &pio->piod.codr);
}

void __led_init(led_id_t mask, int state)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;

	writel(1 << ATMEL_ID_PIOD, &pmc->pcer);	/* Enable PIOB clock */
	/* Disable peripherals on LEDs */
	writel(STATUS_LED_BIT | STATUS_LED_BIT1, &pio->piod.per);
	/* Enable pins as outputs */
	writel(STATUS_LED_BIT | STATUS_LED_BIT1, &pio->piod.oer);
	/* Turn all LEDs OFF */
	writel(STATUS_LED_BIT | STATUS_LED_BIT1, &pio->piod.sodr);

	__led_set(mask, state);
}

void __led_set(led_id_t mask, int state)
{
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;
	if (state == STATUS_LED_ON)
		writel(mask, &pio->piod.codr);
	else
		writel(mask, &pio->piod.sodr);
}

#endif

/*---------------------------------------------------------------------------*/

int do_brightness(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rcode = 0;
	ulong side;
	ulong bright;

	switch (argc) {
	case 3:
		side = simple_strtoul(argv[1], NULL, 10);
		bright = simple_strtoul(argv[2], NULL, 10);
		if ((side >= 0) && (side <= 3) &&
				(bright >= 0) && (bright <= 1000)) {
			vcxk_setbrightness(side, bright);
			rcode = 0;
		} else {
			printf("parameters out of range\n");
			printf("Usage:\n%s\n", cmdtp->usage);
			rcode = 1;
		}
		break;
	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		rcode = 1;
		break;
	}
	return rcode;
}

/*---------------------------------------------------------------------------*/

U_BOOT_CMD(
	bright,	3,	0,	do_brightness,
	"bright  - sets the display brightness\n",
	" <side> <0..1000>\n        side: 0/3=both; 1=first; 2=second\n"
);

/* EOF cpu9k2.c */
