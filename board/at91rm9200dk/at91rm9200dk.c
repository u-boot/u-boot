/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <asm/arch/AT91RM9200.h>
#include <at91rm9200_net.h>
#include <dm9161.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* Enable Ctrlc */
	console_init_f ();

	/* Correct IRDA resistor problem */
	/* Set PA23_TXD in Output */
	((AT91PS_PIO) AT91C_BASE_PIOA)->PIO_OER = AT91C_PA23_TXD2;

	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* arch number of AT91RM9200DK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91RM9200;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_DRIVER_ETHER
#if defined(CONFIG_CMD_NET)

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

#endif
#endif	/* CONFIG_DRIVER_ETHER */

/*
 * Disk On Chip (NAND) Millenium initialization.
 * The NAND lives in the CS2* space
 */
#if defined(CONFIG_CMD_NAND)
extern ulong nand_probe (ulong physadr);

#define AT91_SMARTMEDIA_BASE 0x40000000	/* physical address to access memory on NCS3 */
void nand_init (void)
{
	/* Setup Smart Media, fitst enable the address range of CS3 */
	*AT91C_EBI_CSA |= AT91C_EBI_CS3A_SMC_SmartMedia;
	/* set the bus interface characteristics based on
	   tDS Data Set up Time 30 - ns
	   tDH Data Hold Time 20 - ns
	   tALS ALE Set up Time 20 - ns
	   16ns at 60 MHz ~= 3  */
/*memory mapping structures */
#define SM_ID_RWH	(5 << 28)
#define SM_RWH		(1 << 28)
#define SM_RWS		(0 << 24)
#define SM_TDF		(1 << 8)
#define SM_NWS		(3)
	AT91C_BASE_SMC2->SMC2_CSR[3] = (SM_RWH | SM_RWS |
		AT91C_SMC2_ACSS_STANDARD | AT91C_SMC2_DBW_8 |
		SM_TDF | AT91C_SMC2_WSEN | SM_NWS);

	/* enable the SMOE line PC0=SMCE, A21=CLE, A22=ALE */
	*AT91C_PIOC_ASR = AT91C_PC0_BFCK | AT91C_PC1_BFRDY_SMOE |
		AT91C_PC3_BFBAA_SMWE;
	*AT91C_PIOC_PDR = AT91C_PC0_BFCK | AT91C_PC1_BFRDY_SMOE |
		AT91C_PC3_BFBAA_SMWE;

	/* Configure PC2 as input (signal READY of the SmartMedia) */
	*AT91C_PIOC_PER = AT91C_PC2_BFAVD;	/* enable direct output enable */
	*AT91C_PIOC_ODR = AT91C_PC2_BFAVD;	/* disable output */

	/* Configure PB1 as input (signal Card Detect of the SmartMedia) */
	*AT91C_PIOB_PER = AT91C_PIO_PB1;	/* enable direct output enable */
	*AT91C_PIOB_ODR = AT91C_PIO_PB1;	/* disable output */

	/* PIOB and PIOC clock enabling */
	*AT91C_PMC_PCER = 1 << AT91C_ID_PIOB;
	*AT91C_PMC_PCER = 1 << AT91C_ID_PIOC;

	if (*AT91C_PIOB_PDSR & AT91C_PIO_PB1)
		printf ("  No SmartMedia card inserted\n");
#ifdef DEBUG
	printf ("  SmartMedia card inserted\n");

	printf ("Probing at 0x%.8x\n", AT91_SMARTMEDIA_BASE);
#endif
	printf ("%4lu MB\n", nand_probe(AT91_SMARTMEDIA_BASE) >> 20);
}
#endif
