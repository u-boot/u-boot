/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 *
 * Influenced by code from:
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
#include <spi.h>

#if defined(CONFIG_SOFT_SPI)

/*-----------------------------------------------------------------------
 * Definitions
 */

#ifdef DEBUG_SPI
#define PRINTD(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif


/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
void spi_init (void)
{
#ifdef	SPI_INIT
	volatile immap_t *immr = (immap_t *)CFG_IMMR;

	SPI_INIT;
#endif
}


/*-----------------------------------------------------------------------
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 *
 * If the chipsel() function is not NULL, it is called with a parameter
 * of '1' (chip select active) at the start of the transfer and again with
 * a parameter of '0' at the end of the transfer.
 *
 * If the chipsel() function _is_ NULL, it the responsibility of the
 * caller to make the appropriate chip select active before calling
 * spi_xfer() and making it inactive after spi_xfer() returns.
 */
int  spi_xfer(spi_chipsel_type chipsel, int bitlen, uchar *dout, uchar *din)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	uchar tmpdin  = 0;
	uchar tmpdout = 0;
	int   j;

	PRINTD("spi_xfer: chipsel %08X dout %08X din %08X bitlen %d\n",
		(int)chipsel, *(uint *)dout, *(uint *)din, bitlen);

	if(chipsel != NULL) {
		(*chipsel)(1);	/* select the target chip */
	}

	for(j = 0; j < bitlen; j++) {
		/*
		 * Check if it is time to work on a new byte.
		 */
		if((j % 8) == 0) {
			tmpdout = *dout++;
			if(j != 0) {
				*din++ = tmpdin;
			}
			tmpdin  = 0;
		}
		SPI_SCL(0);
		SPI_SDA(tmpdout & 0x80);
		SPI_DELAY;
		SPI_SCL(1);
		SPI_DELAY;
		tmpdin  <<= 1;
		tmpdin   |= SPI_READ;
		tmpdout <<= 1;
	}
	/*
	 * If the number of bits isn't a multiple of 8, shift the last
	 * bits over to left-justify them.  Then store the last byte
	 * read in.
	 */
	if((bitlen % 8) != 0)
		tmpdin <<= 8 - (bitlen % 8);
	*din++ = tmpdin;

	SPI_SCL(0);		/* SPI wants the clock left low for idle */

	if(chipsel != NULL) {
		(*chipsel)(0);	/* deselect the target chip */

	}

	return(0);
}

#endif	/* CONFIG_SOFT_SPI */
