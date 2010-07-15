/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 * (C) Copyright 2008 Sergey Lapin <slapin@ossfans.org>
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
#include <asm/arch/at91sam9260.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>
#include <asm/arch/hardware.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <netdev.h>
#include <net.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

static void afeb9260_nand_hw_init(void)
{
	unsigned long csa;

	/* Enable CS3 */
	csa = at91_sys_read(AT91_MATRIX_EBICSA);
	at91_sys_write(AT91_MATRIX_EBICSA,
		       csa | AT91_MATRIX_CS3A_SMC_SMARTMEDIA);

	/* Configure SMC CS3 for NAND/SmartMedia */
	at91_sys_write(AT91_SMC_SETUP(3),
		       AT91_SMC_NWESETUP_(0) | AT91_SMC_NCS_WRSETUP_(0) |
		       AT91_SMC_NRDSETUP_(0) | AT91_SMC_NCS_RDSETUP_(0));
	at91_sys_write(AT91_SMC_PULSE(3),
		       AT91_SMC_NWEPULSE_(3) | AT91_SMC_NCS_WRPULSE_(3) |
		       AT91_SMC_NRDPULSE_(3) | AT91_SMC_NCS_RDPULSE_(3));
	at91_sys_write(AT91_SMC_CYCLE(3),
		       AT91_SMC_NWECYCLE_(5) | AT91_SMC_NRDCYCLE_(5));
	at91_sys_write(AT91_SMC_MODE(3),
		       AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		       AT91_SMC_EXNWMODE_DISABLE |
		       AT91_SMC_DBW_8 |
		       AT91_SMC_TDF_(2));

	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9260_ID_PIOC);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}

#ifdef CONFIG_MACB
static void afeb9260_macb_hw_init(void)
{
	unsigned long rstc;

	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9260_ID_EMAC);

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
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA17) |
	       pin_to_mask(AT91_PIN_PA25) |
	       pin_to_mask(AT91_PIN_PA26) |
	       pin_to_mask(AT91_PIN_PA28),
	       pin_to_controller(AT91_PIN_PA0) + PIO_PUDR);

	rstc = at91_sys_read(AT91_RSTC_MR) & AT91_RSTC_ERSTL;

	/* Need to reset PHY -> 500ms reset */
	at91_sys_write(AT91_RSTC_MR, AT91_RSTC_KEY |
				     AT91_RSTC_ERSTL | (0x0D << 8) |
				     AT91_RSTC_URSTEN);

	at91_sys_write(AT91_RSTC_CR, AT91_RSTC_KEY | AT91_RSTC_EXTRST);

	/* Wait for end hardware reset */
	while (!(at91_sys_read(AT91_RSTC_SR) & AT91_RSTC_NRSTL));

	/* Restore NRST value */
	at91_sys_write(AT91_RSTC_MR, AT91_RSTC_KEY |
				     (rstc) |
				     AT91_RSTC_URSTEN);

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA17) |
	       pin_to_mask(AT91_PIN_PA25) |
	       pin_to_mask(AT91_PIN_PA26) |
	       pin_to_mask(AT91_PIN_PA28),
	       pin_to_controller(AT91_PIN_PA0) + PIO_PUER);

	at91_macb_hw_init();
}
#endif

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* arch number of AT91SAM9260EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AFEB9260;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91_serial_hw_init();
#ifdef CONFIG_CMD_NAND
	afeb9260_nand_hw_init();
#endif
	at91_spi0_hw_init((1 << 0) | (1 << 1));
#ifdef CONFIG_MACB
	afeb9260_macb_hw_init();
#endif

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)AT91SAM9260_BASE_EMAC, 0x01);
#endif
	return rc;
}
