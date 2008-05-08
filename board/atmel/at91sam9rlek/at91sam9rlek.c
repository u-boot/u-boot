/*
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
#include <asm/arch/at91sam9rl.h>
#include <asm/arch/at91sam9rl_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

static void at91sam9rlek_serial_hw_init(void)
{
#ifdef CONFIG_USART0
	at91_set_A_periph(AT91_PIN_PA6, 1);		/* TXD0 */
	at91_set_A_periph(AT91_PIN_PA7, 0);		/* RXD0 */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_US0);
#endif

#ifdef CONFIG_USART1
	at91_set_A_periph(AT91_PIN_PA11, 1);		/* TXD1 */
	at91_set_A_periph(AT91_PIN_PA12, 0);		/* RXD1 */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_US1);
#endif

#ifdef CONFIG_USART2
	at91_set_A_periph(AT91_PIN_PA13, 1);		/* TXD2 */
	at91_set_A_periph(AT91_PIN_PA14, 0);		/* RXD2 */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_US2);
#endif

#ifdef CONFIG_USART3	/* DBGU */
	at91_set_A_periph(AT91_PIN_PA21, 0);		/* DRXD */
	at91_set_A_periph(AT91_PIN_PA22, 1);		/* DTXD */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_SYS);
#endif
}

#ifdef CONFIG_CMD_NAND
static void at91sam9rlek_nand_hw_init(void)
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
		       AT91_SMC_NWEPULSE_(2) | AT91_SMC_NCS_WRPULSE_(5) |
		       AT91_SMC_NRDPULSE_(2) | AT91_SMC_NCS_RDPULSE_(5));
	at91_sys_write(AT91_SMC_CYCLE(3),
		       AT91_SMC_NWECYCLE_(7) | AT91_SMC_NRDCYCLE_(7));
	at91_sys_write(AT91_SMC_MODE(3),
		       AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		       AT91_SMC_EXNWMODE_DISABLE |
#ifdef CFG_NAND_DBW_16
		       AT91_SMC_DBW_16 |
#else /* CFG_NAND_DBW_8 */
		       AT91_SMC_DBW_8 |
#endif
		       AT91_SMC_TDF_(1));

	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9RL_ID_PIOD);

	/* Configure RDY/BSY */
	at91_set_gpio_input(AT91_PIN_PD17, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(AT91_PIN_PB6, 1);

	at91_set_A_periph(AT91_PIN_PB4, 0);		/* NANDOE */
	at91_set_A_periph(AT91_PIN_PB5, 0);		/* NANDWE */
}
#endif

#ifdef CONFIG_HAS_DATAFLASH
static void at91sam9rlek_spi_hw_init(void)
{
	at91_set_A_periph(AT91_PIN_PA28, 0);	/* SPI0_NPCS0 */

	at91_set_A_periph(AT91_PIN_PA25, 0);	/* SPI0_MISO */
	at91_set_A_periph(AT91_PIN_PA26, 0);	/* SPI0_MOSI */
	at91_set_A_periph(AT91_PIN_PA27, 0);	/* SPI0_SPCK */

	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9RL_ID_SPI);
}
#endif

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* arch number of AT91SAM9RLEK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9RLEK;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91sam9rlek_serial_hw_init();
#ifdef CONFIG_CMD_NAND
	at91sam9rlek_nand_hw_init();
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91sam9rlek_spi_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}
