/*
 * Copyright (c) Orbacom Systems, Inc <www.orbacom.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */

#include <common.h>
#include <asm/processor.h>
#include <image.h>
#include <net.h>

#include <lynxkdi.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_MPC8260) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
void lynxkdi_boot (image_header_t *hdr)
{
	void (*lynxkdi)(void) = (void(*)(void))image_get_ep (hdr);
	lynxos_bootparms_t *parms = (lynxos_bootparms_t *)0x0020;
	bd_t *kbd;
	u32 *psz = (u32 *)(image_get_load (hdr) + 0x0204);

	memset (parms, 0, sizeof(*parms));
	kbd = gd->bd;
	parms->clock_ref = kbd->bi_busfreq;
	parms->dramsz = kbd->bi_memsize;
	eth_getenv_enetaddr("ethaddr", parms->ethaddr);
	mtspr (SPRN_SPRG2, 0x0020);

	/* Do a simple check for Bluecat so we can pass the
	 * kernel command line parameters.
	 */
	if (le32_to_cpu (*psz) == image_get_data_size (hdr)) {	/* FIXME: NOT SURE HERE ! */
		char *args;
		char *cmdline = (char *)(image_get_load (hdr) + 0x020c);
		int len;

		printf ("Booting Bluecat KDI ...\n");
		udelay (200*1000); /* Allow serial port to flush */
		if ((args = getenv ("bootargs")) == NULL)
			args = "";
		/* Prepend the cmdline */
		len = strlen (args);
		if (len && (len + strlen (cmdline) + 2 < (0x0400 - 0x020c))) {
			memmove (cmdline + strlen (args) + 1, cmdline, strlen (cmdline));
			strcpy (cmdline, args);
			cmdline[len] = ' ';
		}
	}
	else {
		printf ("Booting LynxOS KDI ...\n");
	}

	lynxkdi ();
}
#else
#error "Lynx KDI support not implemented for configured CPU"
#endif
