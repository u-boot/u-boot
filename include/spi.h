/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
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

#ifndef _SPI_H_
#define _SPI_H_

/* SPI mode flags */
#define	SPI_CPHA	0x01			/* clock phase */
#define	SPI_CPOL	0x02			/* clock polarity */
#define	SPI_MODE_0	(0|0)			/* (original MicroWire) */
#define	SPI_MODE_1	(0|SPI_CPHA)
#define	SPI_MODE_2	(SPI_CPOL|0)
#define	SPI_MODE_3	(SPI_CPOL|SPI_CPHA)
#define	SPI_CS_HIGH	0x04			/* chipselect active high? */
#define	SPI_LSB_FIRST	0x08			/* per-word bits-on-wire */
#define	SPI_3WIRE	0x10			/* SI/SO signals shared */
#define	SPI_LOOP	0x20			/* loopback mode */

/*
 * The function call pointer type used to drive the chip select.
 */
typedef void (*spi_chipsel_type)(int cs);


/*-----------------------------------------------------------------------
 * Initialization, must be called once on start up.
 */
void spi_init(void);


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
 *
 * spi_xfer() interface:
 *   chipsel: Routine to call to set/clear the chip select:
 *              if chipsel is NULL, it is not used.
 *              if(cs),  make the chip select active (typically '0').
 *              if(!cs), make the chip select inactive (typically '1').
 *   dout:    Pointer to a string of bits to send out.  The bits are
 *              held in a byte array and are sent MSB first.
 *   din:     Pointer to a string of bits that will be filled in.
 *   bitlen:  How many bits to write and read.
 *
 *   Returns: 0 on success, not 0 on failure
 */
int spi_xfer(spi_chipsel_type chipsel, int bitlen, uchar *dout, uchar *din);

int spi_select(unsigned int bus, unsigned int dev, unsigned long mode);

#endif	/* _SPI_H_ */
