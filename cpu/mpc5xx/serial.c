/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
 * Foundation,
 */

/*
 * File:		serial.c
 *
 * Discription:		Serial interface driver for SCI1 and SCI2.
 *     			Since this code will be called from ROM use
 * 			only non-static local variables.
 *
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc5xx.h>


/*
 * Local function prototypes
 */

static int ready_to_send(void);

/*
 * Minimal global serial functions needed to use one of the SCI modules.
 */

int serial_init (void)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;

	serial_setbrg();

#if defined(CONFIG_5xx_CONS_SCI1)
	/* 10-Bit, 1 start bit, 8 data bit, no parity, 1 stop bit */
	immr->im_qsmcm.qsmcm_scc1r1 = SCI_M_10;
	immr->im_qsmcm.qsmcm_scc1r1 = SCI_TE | SCI_RE;
#else
	immr->im_qsmcm.qsmcm_scc2r1 = SCI_M_10;
	immr->im_qsmcm.qsmcm_scc2r1 = SCI_TE | SCI_RE;
#endif
	return 0;
}

void serial_putc(const char c)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;

	/* Test for completition */
	if(ready_to_send()) {
#if defined(CONFIG_5xx_CONS_SCI1)
		immr->im_qsmcm.qsmcm_sc1dr = (short)c;
#else
		immr->im_qsmcm.qsmcm_sc2dr = (short)c;
#endif
		if(c == '\n') {
			if(ready_to_send());
#if defined(CONFIG_5xx_CONS_SCI1)
			immr->im_qsmcm.qsmcm_sc1dr = (short)'\r';
#else
			immr->im_qsmcm.qsmcm_sc2dr = (short)'\r';
#endif
		}
	}
}

int serial_getc(void)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	volatile short status;
	unsigned char tmp;

	/* New data ? */
	do {
#if defined(CONFIG_5xx_CONS_SCI1)
		status = immr->im_qsmcm.qsmcm_sc1sr;
#else
		status = immr->im_qsmcm.qsmcm_sc2sr;
#endif

#if defined(CONFIG_WATCHDOG)
		reset_5xx_watchdog (immr);
#endif
	} while ((status & SCI_RDRF) == 0);

	/* Read data */
#if defined(CONFIG_5xx_CONS_SCI1)
	tmp = (unsigned char)(immr->im_qsmcm.qsmcm_sc1dr & SCI_SCXDR_MK);
#else
	tmp = (unsigned char)( immr->im_qsmcm.qsmcm_sc2dr & SCI_SCXDR_MK);
#endif
	return	tmp;
}

int serial_tstc()
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	short status;

	/* New data character ? */
#if defined(CONFIG_5xx_CONS_SCI1)
	status = immr->im_qsmcm.qsmcm_sc1sr;
#else
	status = immr->im_qsmcm.qsmcm_sc2sr;
#endif
	return (status & SCI_RDRF);
}

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	short scxbr;

	/* Set baudrate */
	scxbr = (gd->cpu_clk / (32 * gd->baudrate));
#if defined(CONFIG_5xx_CONS_SCI1)
	immr->im_qsmcm.qsmcm_scc1r0 = (scxbr & SCI_SCXBR_MK);
#else
	immr->im_qsmcm.qsmcm_scc2r0 = (scxbr & SCI_SCXBR_MK);
#endif
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc(*s);
		++s;
	}
}

int ready_to_send(void)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	volatile short status;

	do {
#if defined(CONFIG_5xx_CONS_SCI1)
		status = immr->im_qsmcm.qsmcm_sc1sr;
#else
		status = immr->im_qsmcm.qsmcm_sc2sr;
#endif

#if defined(CONFIG_WATCHDOG)
		reset_5xx_watchdog (immr);
#endif
	} while ((status & SCI_TDRE) == 0);
	return 1;

}
