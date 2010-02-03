/*
 * (C) Copyright 2006 Eukrea Electromatique <www.eukrea.com>
 * Eric Benard <eric@eukrea.com>
 * based on at91rm9200dk.c which is :
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
#include <netdev.h>
#include <asm/arch/AT91RM9200.h>
#include <asm/io.h>

#if defined(CONFIG_DRIVER_ETHER)
#include <at91rm9200_net.h>
#include <ks8721.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();
	/* arch number of CPUAT91-Board */
	gd->bd->bi_arch_number = MACH_TYPE_CPUAT91;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

#if defined(CONFIG_DRIVER_ETHER)
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
	p_phyops->Init = ks8721_initphy;
	p_phyops->IsPhyConnected = ks8721_isphyconnected;
	p_phyops->GetLinkSpeed = ks8721_getlinkspeed;
	p_phyops->AutoNegotiate = ks8721_autonegotiate;
}

#endif	/* CONFIG_CMD_NET */
#endif	/* CONFIG_DRIVER_ETHER */
#ifdef CONFIG_DRIVER_AT91EMAC

int board_eth_init(bd_t *bis)
{
	int rc = 0;
	rc = at91emac_register(bis, 0);
	return rc;
}
#endif
