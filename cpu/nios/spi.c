/*
 * (C) Copyright 2004, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
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
#include <linux/ctype.h>

#if defined(CONFIG_NIOS_SPI)
#include <nios-io.h>
#include <spi.h>

#if !defined(CFG_NIOS_SPIBASE)
#error "*** CFG_NIOS_SPIBASE not defined ***"
#endif

#if !defined(CFG_NIOS_SPIBITS)
#error "*** CFG_NIOS_SPIBITS not defined ***"
#endif

#if (CFG_NIOS_SPIBITS != 8) && (CFG_NIOS_SPIBITS != 16)
#error "*** CFG_NIOS_SPIBITS should be either 8 or 16 ***"
#endif

static nios_spi_t	*spi	= (nios_spi_t *)CFG_NIOS_SPIBASE;

/* Warning:
 * You cannot enable DEBUG for early system initalization, i. e. when
 * this driver is used to read environment parameters like "baudrate"
 * from EEPROM which are used to initialize the serial port which is
 * needed to print the debug messages...
 */
#undef	DEBUG

#ifdef  DEBUG

#define	DPRINT(a)	printf a;
/* -----------------------------------------------
 * Helper functions to peek into tx and rx buffers
 * ----------------------------------------------- */
static const char * const hex_digit = "0123456789ABCDEF";

static char quickhex (int i)
{
	return hex_digit[i];
}

static void memdump (void *pv, int num)
{
	int i;
	unsigned char *pc = (unsigned char *) pv;

	for (i = 0; i < num; i++)
		printf ("%c%c ", quickhex (pc[i] >> 4), quickhex (pc[i] & 0x0f));
	printf ("\t");
	for (i = 0; i < num; i++)
		printf ("%c", isprint (pc[i]) ? pc[i] : '.');
	printf ("\n");
}
#else   /* !DEBUG */

#define	DPRINT(a)
#define	memdump(p,n)

#endif  /* DEBUG */


/*
 * SPI transfer:
 *
 * See include/spi.h and http://www.altera.com/literature/ds/ds_nios_spi.pdf
 * for more informations.
 */
int spi_xfer(spi_chipsel_type chipsel, int bitlen, uchar *dout, uchar *din)
{
	int j;

	DPRINT(("spi_xfer: chipsel %08X dout %08X din %08X bitlen %d\n",
		(int)chipsel, *(uint *)dout, *(uint *)din, bitlen));

	memdump((void*)dout, (bitlen + 7) / 8);

	if(chipsel != NULL) {
		chipsel(1);	/* select the target chip */
	}

	if (bitlen > CFG_NIOS_SPIBITS) {	/* leave chip select active */
		spi->control |= NIOS_SPI_SSO;
	}

	for (	j = 0;				/* count each byte in */
		j < ((bitlen + 7) / 8);		/* dout[] and din[] */

#if	(CFG_NIOS_SPIBITS == 8)
		j++) {

		while ((spi->status & NIOS_SPI_TRDY) == 0)
			;
		spi->txdata = (unsigned)(dout[j]);

		while ((spi->status & NIOS_SPI_RRDY) == 0)
			;
		din[j] = (unsigned char)(spi->rxdata & 0xff);

#elif	(CFG_NIOS_SPIBITS == 16)
		j++, j++) {

		while ((spi->status & NIOS_SPI_TRDY) == 0)
			;
		if ((j+1) < ((bitlen + 7) / 8))
			spi->txdata = (unsigned)((dout[j] << 8) | dout[j+1]);
		else
			spi->txdata = (unsigned)(dout[j] << 8);

		while ((spi->status & NIOS_SPI_RRDY) == 0)
			;
		din[j] = (unsigned char)((spi->rxdata >> 8) & 0xff);
		if ((j+1) < ((bitlen + 7) / 8))
			din[j+1] = (unsigned char)(spi->rxdata & 0xff);

#else
#error "*** unsupported value of CFG_NIOS_SPIBITS ***"
#endif

	}

	if (bitlen > CFG_NIOS_SPIBITS) {
		spi->control &= ~NIOS_SPI_SSO;
	}

	if(chipsel != NULL) {
		chipsel(0);	/* deselect the target chip */
	}

	memdump((void*)din, (bitlen + 7) / 8);

	return 0;
}

#endif /* CONFIG_NIOS_SPI */
