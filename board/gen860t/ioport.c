/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <mpc8xx.h>
#include <asm/8xx_immap.h>
#include "ioport.h"

#if 0
#define IOPORT_DEBUG
#endif

#ifdef  IOPORT_DEBUG
#define PRINTF(fmt,args...) printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/*
 * The ioport configuration table.
 */
const mpc8xx_iop_conf_t iop_conf_tab[NUM_PORTS][PORT_BITS] = {
    /*
	 * Port A configuration
	 * Pin	Signal					Type	Active	Initial state
	 * PA7	fpgaProgramLowOut		Out		Low			High
	 * PA1	fpgaCoreVoltageFailLow	In		Low			N/A
	 */
    {	/*	    conf ppar psor pdir podr pdat pint	   function		*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /* No pin			*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /* No pin			*/
	/* PA15 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA14 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA13 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA12 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA11 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA10 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA9  */ { 1,   0,   0,   1,   0,   0,   0 }, /* grn bicolor LED 1*/
	/* PA8  */ { 1,   0,   0,   1,   0,   0,   0 }, /* red bicolor LED 1*/
	/* PA7  */ { 1,   0,   0,   1,   0,   1,   0 }, /* fpgaProgramLow	*/
	/* PA6  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA5  */ { 1,   0,   0,   1,   0,   0,   0 }, /* grn bicolor LED 0*/
	/* PA4  */ { 1,   0,   0,   1,   0,   0,   0 }, /* red bicolor LED 0*/
	/* PA3  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PA2  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#if !defined(CONFIG_SC)
	/* PA1  */ { 1,   0,   0,   0,   0,   0,   0 }, /*	fpgaCoreVoltageFail*/
#else
	/* PA1  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#endif
	/* PA0  */ { 0,   0,   0,   0,   0,   0,   0 }  /*	*/
    },

	/*
	 * Port B configuration
	 * Pin		Signal			Type		Active		Initial state
	 * PB14		docBusyLowIn	In			Low			X
	 * PB15		gpio1Sig		Out			High		Low
	 * PB16		fpgaDoneBi		In			High		X
	 * PB17		swBitOkLowOut	Out			Low			High
	 * PB19		speakerVolSig	Out/Hi-Z	High/Low	High (Hi-Z)
	 * PB22		fpgaInitLowBi	In			Low			X
	 * PB23		batteryOkSig	In			High		X
	 * PB31		pulseCatcherClr	Out			High		0
	 */
	{	/*	    conf ppar psor pdir podr pdat pint	  function			*/
#if !defined(CONFIG_SC)
	/* PB31 */ { 0,	  0,   0,   0,   0,   0,   0 }, /*	*/
#else
	/* PB31 */ { 1,   0,   0,   1,   0,   0,   0 }, /* pulseCatcherClr	*/
#endif
	/* PB30 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB29 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB28 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB27 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB26 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB25 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB24 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#if !defined(CONFIG_SC)
	/* PB23 */ { 1,   0,   0,   0,   0,   0,   0 }, /* batteryOk		*/
#else
	/* PB23 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#endif
	/* PB22 */ { 1,   0,   0,   0,   0,   0,   0 }, /* fpgaInitLowBi	*/
	/* PB21 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB20 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#if !defined(CONFIG_SC)
	/* PB19 */ { 1,   0,   0,   1,   1,   1,   0 }, /* speakerVol		*/
#else
	/* PB19 */ { 0,   0,   0,   1,   1,   1,   0 }, /*	*/
#endif
	/* PB18 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PB17 */ { 1,   0,   0,   1,   0,   1,   0 }, /* swBitOkLow		*/
	/* PB16 */ { 1,   0,   0,   0,   0,   0,   0 }, /* fpgaDone			*/
	/* PB15 */ { 1,   0,   0,   1,   0,   0,   0 }, /* gpio1			*/
#if !defined(CONFIG_SC)
	/* PB14 */ { 1,   0,   0,   0,   0,   0,   0 }  /* docBusyLow		*/
#else
	/* PB14 */ { 0,   0,   0,   0,   0,   0,   0 }  /*	*/
#endif
	},

	/*
	 * Port C configuration
	 * Pin		Signal				Type	Active		Initial state
	 * PC4		i2cBus1EnSig		Out		High		High
	 * PC5		i2cBus2EnSig		Out		High		High
	 * PC6		gpio0Sig			Out		High		Low
	 * PC8		i2cBus3EnSig		Out		High		High
	 * PC10		i2cBus4EnSig		Out		High		High
	 * PC11		fpgaResetLowOut		Out		Low			High
	 * PC12		systemBitOkIn		In		High		X
	 * PC15		selfDreqLow			In		Low			X
	 */
	{	/*	    conf ppar psor pdir podr pdat pint	  function			*/
	/* N/A	*/ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A	*/ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PC15 */ { 1,   0,   0,   0,   0,   0,   0 }, /* selfDreqLowIn	*/
	/* PC14 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PC13 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#if !defined(CONFIG_SC)
	/* PC12 */ { 1,   0,   0,   0,   0,   0,   0 }, /* systemBitOkIn	*/
#else
	/* PC12 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#endif
	/* PC11 */ { 1,   0,   0,   1,   0,   1,   0 }, /* fpgaResetLowOut	*/
#if !defined(CONFIG_SC)
	/* PC10 */ { 1,   0,   0,   1,   0,   1,   0 }, /* i2cBus4EnSig		*/
#else
	/* PC10 */ { 0,   0,   0,   1,   0,   1,   0 }, /*	*/
#endif
	/* PC9  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
#if !defined(CONFIG_SC)
	/* PC8  */ { 1,   0,   0,   1,   0,   1,   0 }, /* i2cBus3EnSig		*/
#else
	/* PC8  */ { 0,   0,   0,   1,   0,   1,   0 }, /*	*/
#endif
	/* PC7  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PC6  */ { 1,   0,   0,   1,   0,   1,   0 }, /* gpio0			*/
#if !defined(CONFIG_SC)
	/* PC5  */ { 1,   0,   0,   1,   0,   1,   0 }, /* i2cBus2EnSig		*/
	/* PC4  */ { 1,   0,   0,   1,   0,   1,   0 }, /* i2cBus1EnSig		*/
#else
	/* PC5  */ { 0,   0,   0,   1,   0,   1,   0 }, /*	*/
	/* PC4  */ { 0,   0,   0,   1,   0,   1,   0 }, /*	*/
#endif
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }  /*	*/
	},

	/*
	 * Port D configuration
	 */
	{	/*	    conf ppar psor pdir podr pdat pint	   function			*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD15 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD14 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD13 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD12 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD11 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD10 */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD9  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD8  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD7  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD6  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD5  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD4  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* PD3  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }, /*	*/
	/* N/A  */ { 0,   0,   0,   0,   0,   0,   0 }  /*	*/
	}
};

/*
 * Configure the MPC8XX I/O ports per the ioport configuration table
 * (taken from ./cpu/mpc8260/cpu_init.c)
 */
void config_mpc8xx_ioports (volatile immap_t * immr)
{
	int portnum;

	for (portnum = 0; portnum < NUM_PORTS; portnum++) {
		uint pmsk = 0, ppar = 0, psor = 0, pdir = 0;
		uint podr = 0, pdat = 0, pint = 0;
		uint msk = 1;
		mpc8xx_iop_conf_t *iopc =
			(mpc8xx_iop_conf_t *) & iop_conf_tab[portnum][0];
		mpc8xx_iop_conf_t *eiopc = iopc + PORT_BITS;

		/*
		 * For all ports except port B, ignore the two don't care entries
		 * in the configuration tables.
		 */
		if (portnum != 1) {
			iopc = (mpc8xx_iop_conf_t *) &
				iop_conf_tab[portnum][2];
		}

		/*
		 * NOTE: index 0 refers to pin 17, index 17 refers to pin 0
		 */
		while (iopc < eiopc) {
			if (iopc->conf) {
				pmsk |= msk;
				if (iopc->ppar)
					ppar |= msk;
				if (iopc->psor)
					psor |= msk;
				if (iopc->pdir)
					pdir |= msk;
				if (iopc->podr)
					podr |= msk;
				if (iopc->pdat)
					pdat |= msk;
				if (iopc->pint)
					pint |= msk;
			}
			msk <<= 1;
			iopc++;
		}

		PRINTF ("%s:%d:\n  portnum=%d ", __FUNCTION__, __LINE__,
			portnum);
#ifdef IOPORT_DEBUG
		switch (portnum) {
		case 0:
			printf ("(A)\n");
			break;
		case 1:
			printf ("(B)\n");
			break;
		case 2:
			printf ("(C)\n");
			break;
		case 3:
			printf ("(D)\n");
			break;
		default:
			printf ("(?)\n");
			break;
		}
#endif
		PRINTF ("  ppar=0x%.8x  pdir=0x%.8x  podr=0x%.8x\n"
			"  pdat=0x%.8x  psor=0x%.8x  pint=0x%.8x  pmsk=0x%.8x\n",
			ppar, pdir, podr, pdat, psor, pint, pmsk);

		/*
		 * Have to handle the ioports on a port-by-port basis since there
		 * are three different flavors.
		 */
		if (pmsk != 0) {
			uint tpmsk = ~pmsk;

			if (0 == portnum) {	/* port A */
				immr->im_ioport.iop_papar &= tpmsk;
				immr->im_ioport.iop_padat =
					(immr->im_ioport.
					 iop_padat & tpmsk) | pdat;
				immr->im_ioport.iop_padir =
					(immr->im_ioport.
					 iop_padir & tpmsk) | pdir;
				immr->im_ioport.iop_paodr =
					(immr->im_ioport.
					 iop_paodr & tpmsk) | podr;
				immr->im_ioport.iop_papar |= ppar;
			} else if (1 == portnum) {	/* port B */
				immr->im_cpm.cp_pbpar &= tpmsk;
				immr->im_cpm.cp_pbdat =
					(immr->im_cpm.
					 cp_pbdat & tpmsk) | pdat;
				immr->im_cpm.cp_pbdir =
					(immr->im_cpm.
					 cp_pbdir & tpmsk) | pdir;
				immr->im_cpm.cp_pbodr =
					(immr->im_cpm.
					 cp_pbodr & tpmsk) | podr;
				immr->im_cpm.cp_pbpar |= ppar;
			} else if (2 == portnum) {	/* port C */
				immr->im_ioport.iop_pcpar &= tpmsk;
				immr->im_ioport.iop_pcdat =
					(immr->im_ioport.
					 iop_pcdat & tpmsk) | pdat;
				immr->im_ioport.iop_pcdir =
					(immr->im_ioport.
					 iop_pcdir & tpmsk) | pdir;
				immr->im_ioport.iop_pcint =
					(immr->im_ioport.
					 iop_pcint & tpmsk) | pint;
				immr->im_ioport.iop_pcso =
					(immr->im_ioport.
					 iop_pcso & tpmsk) | psor;
				immr->im_ioport.iop_pcpar |= ppar;
			} else if (3 == portnum) {	/* port D */
				immr->im_ioport.iop_pdpar &= tpmsk;
				immr->im_ioport.iop_pddat =
					(immr->im_ioport.
					 iop_pddat & tpmsk) | pdat;
				immr->im_ioport.iop_pddir =
					(immr->im_ioport.
					 iop_pddir & tpmsk) | pdir;
				immr->im_ioport.iop_pdpar |= ppar;
			}
		}
	}

	PRINTF ("%s:%d: Port A:\n  papar=0x%.4x  padir=0x%.4x"
		"  paodr=0x%.4x\n  padat=0x%.4x\n", __FUNCTION__, __LINE__,
		immr->im_ioport.iop_papar, immr->im_ioport.iop_padir,
		immr->im_ioport.iop_paodr, immr->im_ioport.iop_padat);
	PRINTF ("%s:%d: Port B:\n  pbpar=0x%.8x  pbdir=0x%.8x"
		"  pbodr=0x%.8x\n  pbdat=0x%.8x\n", __FUNCTION__, __LINE__,
		immr->im_cpm.cp_pbpar, immr->im_cpm.cp_pbdir,
		immr->im_cpm.cp_pbodr, immr->im_cpm.cp_pbdat);
	PRINTF ("%s:%d: Port C:\n  pcpar=0x%.4x  pcdir=0x%.4x"
		"  pcdat=0x%.4x\n  pcso=0x%.4x  pcint=0x%.4x\n  ",
		__FUNCTION__, __LINE__, immr->im_ioport.iop_pcpar,
		immr->im_ioport.iop_pcdir, immr->im_ioport.iop_pcdat,
		immr->im_ioport.iop_pcso, immr->im_ioport.iop_pcint);
	PRINTF ("%s:%d: Port D:\n  pdpar=0x%.4x  pddir=0x%.4x"
		"  pddat=0x%.4x\n", __FUNCTION__, __LINE__,
		immr->im_ioport.iop_pdpar, immr->im_ioport.iop_pddir,
		immr->im_ioport.iop_pddat);
}
