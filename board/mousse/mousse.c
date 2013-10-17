/*
 * MOUSSE Board Support
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001
 * James Dougherty, jfd@cs.stanford.edu
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc824x.h>
#include <netdev.h>
#include <asm/processor.h>
#include <timestamp.h>

#include "mousse.h"
#include "m48t59y.h"
#include <pci.h>


int checkboard (void)
{
	ulong busfreq = get_bus_freq (0);
	char buf[32];

	puts ("Board: MOUSSE MPC8240/KAHLUA - CHRP (MAP B)\n");
	printf ("Built: %s at %s\n", U_BOOT_DATE, U_BOOT_TIME);
	printf ("MPLD:  Revision %d\n", SYS_REVID_GET ());
	printf ("Local Bus:  %s MHz\n", strmhz (buf, busfreq));

	return 0;
}

int checkflash (void)
{
	printf ("checkflash\n");
	flash_init ();
	return 0;
}

phys_size_t initdram (int board_type)
{
	return CONFIG_SYS_RAM_SIZE;
}


void get_tod (void)
{
	int year, month, day, hour, minute, second;

	m48_tod_get (&year, &month, &day, &hour, &minute, &second);

	printf ("  Current date/time: %d/%d/%d %d:%d:%d \n",
		month, day, year, hour, minute, second);

}

/*
 * EPIC, PCI, and I/O devices.
 * Initialize Mousse Platform, probe for PCI devices,
 * Query configuration parameters if not set.
 */
int misc_init_f (void)
{
	m48_tod_init ();	/* Init SGS M48T59Y TOD/NVRAM */
	printf ("RTC:   M48T589 TOD/NVRAM (%d) bytes\n", TOD_NVRAM_SIZE);
	get_tod ();
	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
