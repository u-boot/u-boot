/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Modified for CMC_PU2 (removed Smart Media support) by Gary Jennejohn
 * (2004) garyj@denx.de
 *
 * Modified for CMC_BASIC by Martin Krause (2005), TQ-Systems GmbH
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
#include <asm/mach-types.h>
#include <asm/arch/AT91RM9200.h>
#include <at91rm9200_net.h>
#include <dm9161.h>

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */
#define CMC_HP_BASIC	1
#define CMC_PU2		2
#define CMC_BASIC	4

int hw_detect (void);

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	AT91PS_PIO piob = AT91C_BASE_PIOB;
	AT91PS_PIO pioc = AT91C_BASE_PIOC;

	/* Enable Ctrlc */
	console_init_f ();

	/* Correct IRDA resistor problem */
	/* Set PA23_TXD in Output */
	/* (AT91PS_PIO) AT91C_BASE_PIOA->PIO_OER = AT91C_PA23_TXD2; */

	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* PIOB and PIOC clock enabling */
	*AT91C_PMC_PCER = 1 << AT91C_ID_PIOB;
	*AT91C_PMC_PCER = 1 << AT91C_ID_PIOC;

	/*
	 * configure PC0-PC3 as input without pull ups, so RS485 driver enable
	 * (CMC-PU2) and digital outputs (CMC-BASIC) are deactivated.
	 */
	pioc->PIO_ODR = AT91C_PIO_PC0 | AT91C_PIO_PC1 |
			AT91C_PIO_PC2 | AT91C_PIO_PC3;
	pioc->PIO_PPUDR = AT91C_PIO_PC0 | AT91C_PIO_PC1 |
			AT91C_PIO_PC2 | AT91C_PIO_PC3;
	pioc->PIO_PER = AT91C_PIO_PC0 | AT91C_PIO_PC1 |
	        	AT91C_PIO_PC2 | AT91C_PIO_PC3;

	/*
	 * On CMC-PU2 board configure PB3-PB6 to input without pull ups to
	 * clear the duo LEDs (the external pull downs assure a proper
	 * signal). On CMC-BASIC and CMC-HP-BASIC set PB3-PB6 to output and
	 * drive it high, to configure current measurement on AINx.
	 */
	if (hw_detect() & CMC_PU2) {
		piob->PIO_ODR = AT91C_PIO_PB3 | AT91C_PIO_PB4 |
				AT91C_PIO_PB5 | AT91C_PIO_PB6;
	}
	else if ((hw_detect() & CMC_BASIC) || (hw_detect() & CMC_HP_BASIC)) {
		piob->PIO_SODR = AT91C_PIO_PB3 | AT91C_PIO_PB4 |
				AT91C_PIO_PB5 | AT91C_PIO_PB6;
		piob->PIO_OER = AT91C_PIO_PB3 | AT91C_PIO_PB4 |
				AT91C_PIO_PB5 | AT91C_PIO_PB6;
	}
	piob->PIO_PPUDR = AT91C_PIO_PB3 | AT91C_PIO_PB4 |
			AT91C_PIO_PB5 | AT91C_PIO_PB6;
	piob->PIO_PER = AT91C_PIO_PB3 | AT91C_PIO_PB4 |
			AT91C_PIO_PB5 | AT91C_PIO_PB6;

	/*
	 * arch number of CMC_PU2-Board. MACH_TYPE_CMC_PU2 is not supported in
	 * the linuxarm kernel, yet.
	 */
	/* gd->bd->bi_arch_number = MACH_TYPE_CMC_PU2; */
	gd->bd->bi_arch_number = 251;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

int checkboard (void)
{
	if (hw_detect() & CMC_PU2)
		puts ("Board: CMC-PU2 (Rittal GmbH)\n");
	else if (hw_detect() & CMC_BASIC)
		puts ("Board: CMC-BASIC (Rittal GmbH)\n");
	else if (hw_detect() & CMC_HP_BASIC)
		puts ("Board: CMC-HP-BASIC (Rittal GmbH)\n");
	else
		puts ("Board: unknown\n");
	return 0;
}

int hw_detect (void)
{
	AT91PS_PIO pio = AT91C_BASE_PIOB;

	/* PIOB clock enabling */
	*AT91C_PMC_PCER = 1 << AT91C_ID_PIOB;

	/* configure PB12 as input without pull up */
	pio->PIO_ODR = AT91C_PIO_PB12;
	pio->PIO_PPUDR = AT91C_PIO_PB12;
	pio->PIO_PER = AT91C_PIO_PB12;

	/* configure PB13 as input without pull up */
	pio->PIO_ODR = AT91C_PIO_PB13;
	pio->PIO_PPUDR = AT91C_PIO_PB13;
	pio->PIO_PER = AT91C_PIO_PB13;

	/* read board identification pin */
	if (pio->PIO_PDSR & AT91C_PIO_PB12)
		return ((pio->PIO_PDSR & AT91C_PIO_PB13)
			? CMC_PU2 : 0);
	else
		return ((pio->PIO_PDSR & AT91C_PIO_PB13)
			? CMC_HP_BASIC : CMC_BASIC);
}

#ifdef CONFIG_DRIVER_ETHER
#if (CONFIG_COMMANDS & CFG_CMD_NET)

/*
 * Name:
 *	at91rm9200_GetPhyInterface
 * Description:
 *	Initialise the interface functions to the PHY
 * Arguments:
 *	None
 * Return value:
 *	None
 */
void at91rm9200_GetPhyInterface(AT91PS_PhyOps p_phyops)
{
	p_phyops->Init = dm9161_InitPhy;
	p_phyops->IsPhyConnected = dm9161_IsPhyConnected;
	p_phyops->GetLinkSpeed = dm9161_GetLinkSpeed;
	p_phyops->AutoNegotiate = dm9161_AutoNegotiate;
}

#endif	/* CONFIG_COMMANDS & CFG_CMD_NET */
#endif	/* CONFIG_DRIVER_ETHER */
