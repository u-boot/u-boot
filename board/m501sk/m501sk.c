/*
 * (C) Copyright 2008
 * Based on modifications by Alan Lu / Artila
 * Author : Timo Tuunainen / Sysart
			Kimmo Leppala / Sysart
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <at91rm9200_net.h>
#include <dm9161.h>
#include "m501sk.h"
#include "net.h"

#ifdef CONFIG_M501SK

void m501sk_gpio_init(void)
{
	AT91C_BASE_PIOD->PIO_PER = 1 << (M501SK_DEBUG_LED1 - 96) |
		1 << (M501SK_DEBUG_LED2 - 96) | 1 << (M501SK_DEBUG_LED3 - 96) |
		1 << (M501SK_DEBUG_LED4 - 96) | 1 << (M501SK_READY_LED - 96);

	AT91C_BASE_PIOD->PIO_OER = 1 << (M501SK_DEBUG_LED1 - 96) |
		1 << (M501SK_DEBUG_LED2 - 96) | 1 << (M501SK_DEBUG_LED3 - 96) |
		1 << (M501SK_DEBUG_LED4 - 96) | 1 << (M501SK_READY_LED - 96);

	AT91C_BASE_PIOD->PIO_SODR = 1 << (M501SK_READY_LED - 96);
	AT91C_BASE_PIOD->PIO_CODR = 1 << (M501SK_DEBUG_LED3 - 96);
	AT91C_BASE_PIOB->PIO_PER = 1 << (M501SK_BUZZER - 32);
	AT91C_BASE_PIOB->PIO_OER = 1 << (M501SK_BUZZER - 32);
	AT91C_BASE_PIOC->PIO_PDR = (1 << 7) | (1 << 8);

	/* Power OFF all USART's LEDs */
	AT91C_BASE_PIOA->PIO_PER = AT91C_PA5_TXD3 | AT91C_PA6_RXD3 |
		AT91C_PA17_TXD0 | AT91C_PA18_RXD0 | AT91C_PA22_RXD2 | \
		AT91C_PA23_TXD2;

	AT91C_BASE_PIOA->PIO_OER = AT91C_PA5_TXD3 | AT91C_PA6_RXD3 |
		AT91C_PA17_TXD0 | AT91C_PA18_RXD0 | AT91C_PA22_RXD2 | \
		AT91C_PA23_TXD2;

	AT91C_BASE_PIOA->PIO_SODR = AT91C_PA5_TXD3 | AT91C_PA6_RXD3 |
		AT91C_PA17_TXD0 | AT91C_PA18_RXD0 | AT91C_PA22_RXD2 | \
		AT91C_PA23_TXD2;

	AT91C_BASE_PIOB->PIO_PER = AT91C_PB20_RXD1 | AT91C_PB21_TXD1;
	AT91C_BASE_PIOB->PIO_OER = AT91C_PB20_RXD1 | AT91C_PB21_TXD1;
	AT91C_BASE_PIOB->PIO_SODR = AT91C_PB20_RXD1 | AT91C_PB21_TXD1;
}

uchar m501sk_gpio_set(M501SK_PIO io)
{
	uchar status = 0xff;
	switch (io) {
	case M501SK_DEBUG_LED1:
	case M501SK_DEBUG_LED2:
	case M501SK_DEBUG_LED3:
	case M501SK_DEBUG_LED4:
	case M501SK_READY_LED:
		AT91C_BASE_PIOD->PIO_SODR = 1 << (io - 96);
		status = AT91C_BASE_PIOD->PIO_ODSR & (1 << (io - 96));
		break;
	case M501SK_BUZZER:
		AT91C_BASE_PIOB->PIO_SODR = 1 << (io - 32);
		status = AT91C_BASE_PIOB->PIO_ODSR & (1 << (io - 32));
		break;
	}
	return status;
}

uchar m501sk_gpio_clear(M501SK_PIO io)
{
	uchar status = 0xff;
	switch (io) {
	case M501SK_DEBUG_LED1:
	case M501SK_DEBUG_LED2:
	case M501SK_DEBUG_LED3:
	case M501SK_DEBUG_LED4:
	case M501SK_READY_LED:
		AT91C_BASE_PIOD->PIO_CODR = 1 << (io - 96);
		status = AT91C_BASE_PIOD->PIO_ODSR & (1 << (io - 96));
		break;
	case M501SK_BUZZER:
		AT91C_BASE_PIOB->PIO_CODR = 1 << (io - 32);
		status = AT91C_BASE_PIOB->PIO_ODSR & (1 << (io - 32));
		break;
	}
	return status;
}

void load_sernum_ethaddr(void)
{
	return;
}

/*
 * Miscelaneous platform dependent initialisations
 */
DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* Correct IRDA resistor problem */
	/* Set PA23_TXD in Output */
	((AT91PS_PIO)AT91C_BASE_PIOA)->PIO_OER = AT91C_PA23_TXD2;

	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */
	gd->bd->bi_arch_number = MACH_TYPE_M501;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	m501sk_gpio_init();

	/* Do interrupt init here, because flash needs timers */
	interrupt_init();
	flash_init();

	return 0;
}

int dram_init(void)
{
	int i = 0;
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;

	for (i = 0; i < 500; i++) {
		m501sk_gpio_clear(M501SK_DEBUG_LED3);
		m501sk_gpio_clear(M501SK_BUZZER);
		udelay(250);
		m501sk_gpio_set(M501SK_DEBUG_LED3);
		m501sk_gpio_set(M501SK_BUZZER);
		udelay(80);
	}
	m501sk_gpio_clear(M501SK_BUZZER);
	m501sk_gpio_clear(M501SK_DEBUG_LED3);

	return 0;
}

int board_late_init(void)
{
#if defined(CONFIG_CMD_NET)
	eth_init(gd->bd);
	eth_halt();
#endif

	/* Protect U-Boot, kernel & ramdisk memory addresses */
	run_command("protect on 10000000 1041ffff", 0);
	return 0;
}

#ifdef CONFIG_DRIVER_ETHER
#if defined(CONFIG_CMD_NET)
/*
 * Name:
 *     at91rm9200_GetPhyInterface
 * Description:
 *     Initialise the interface functions to the PHY
 * Arguments:
 *     None
 * Return value:
 *     None
 */
void at91rm9200_GetPhyInterface(AT91PS_PhyOps p_phyops)
{
	p_phyops->Init = dm9161_InitPhy;
	p_phyops->IsPhyConnected = dm9161_IsPhyConnected;
	p_phyops->GetLinkSpeed = dm9161_GetLinkSpeed;
	p_phyops->AutoNegotiate = dm9161_AutoNegotiate;
}
#endif /* CONFIG_CMD_NET */
#endif /* CONFIG_DRIVER_ETHER */
#endif /* CONFIG_M501SK */
