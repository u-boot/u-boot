/*
 * (C) Copyright 2004
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>
#include "kup.h"
#include <asm/io.h>


int misc_init_f(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile sysconf8xx_t *siu = &immap->im_siu_conf;

	while (in_be32(&siu->sc_sipend) & 0x20000000) {
		debug("waiting for 5V VCC\n");
	}

	/* RS232 / RS485 default is RS232 */
	clrbits_be16(&immap->im_ioport.iop_padat, PA_RS485);
	clrbits_be16(&immap->im_ioport.iop_papar, PA_RS485);
	clrbits_be16(&immap->im_ioport.iop_paodr, PA_RS485);
	setbits_be16(&immap->im_ioport.iop_padir, PA_RS485);

	/* IO Reset min 1 msec */
	setbits_be16(&immap->im_ioport.iop_padat,
				 (PA_RESET_IO_01 | PA_RESET_IO_02));
	clrbits_be16(&immap->im_ioport.iop_papar,
				 (PA_RESET_IO_01 | PA_RESET_IO_02));
	clrbits_be16(&immap->im_ioport.iop_paodr,
				 (PA_RESET_IO_01 | PA_RESET_IO_02));
	setbits_be16(&immap->im_ioport.iop_padir,
				 (PA_RESET_IO_01 | PA_RESET_IO_02));
	udelay(1000);
	clrbits_be16(&immap->im_ioport.iop_padat,
				 (PA_RESET_IO_01 | PA_RESET_IO_02));
	return (0);
}

#ifdef CONFIG_IDE_LED
void ide_led(uchar led, uchar status)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/* We have one led for both pcmcia slots */
	if (status)
		clrbits_be16(&immap->im_ioport.iop_padat, PA_LED_YELLOW);
	else
		setbits_be16(&immap->im_ioport.iop_padat, PA_LED_YELLOW);
}
#endif

void poweron_key(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	clrbits_be16(&immap->im_ioport.iop_pcpar, PC_SWITCH1);
	clrbits_be16(&immap->im_ioport.iop_pcdir, PC_SWITCH1);

	if (in_be16(&immap->im_ioport.iop_pcdat) & (PC_SWITCH1))
		setenv("key1", "off");
	else
		setenv("key1", "on");
}
