/*
 * (C) Copyright 2001
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2000-2011
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
#include <ide.h>
#include <ata.h>
#include <asm/io.h>

#define EIEIO		__asm__ volatile ("eieio")
#define SYNC		__asm__ volatile ("sync")

void ide_input_swap_data(int dev, ulong *sect_buf, int words)
{
	uchar i;
	volatile uchar *pbuf_even =
		(uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_EVEN);
	volatile uchar *pbuf_odd =
		(uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_ODD);
	ushort *dbuf = (ushort *) sect_buf;

	while (words--) {
		for (i = 0; i < 2; i++) {
			*(((uchar *) (dbuf)) + 1) = *pbuf_even;
			*(uchar *) dbuf = *pbuf_odd;
			dbuf += 1;
		}
	}
}

void ide_input_data(int dev, ulong *sect_buf, int words)
{
	uchar *dbuf;
	volatile uchar *pbuf_even;
	volatile uchar *pbuf_odd;

	pbuf_even = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_EVEN);
	pbuf_odd = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_ODD);
	dbuf = (uchar *) sect_buf;
	while (words--) {
		*dbuf++ = *pbuf_even;
		EIEIO;
		SYNC;
		*dbuf++ = *pbuf_odd;
		EIEIO;
		SYNC;
		*dbuf++ = *pbuf_even;
		EIEIO;
		SYNC;
		*dbuf++ = *pbuf_odd;
		EIEIO;
		SYNC;
	}
}

void ide_input_data_shorts(int dev, ushort *sect_buf, int shorts)
{
	uchar *dbuf;
	volatile uchar *pbuf_even;
	volatile uchar *pbuf_odd;

	pbuf_even = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_EVEN);
	pbuf_odd = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_ODD);
	dbuf = (uchar *) sect_buf;
	while (shorts--) {
		EIEIO;
		*dbuf++ = *pbuf_even;
		EIEIO;
		*dbuf++ = *pbuf_odd;
	}
}

void ide_output_data(int dev, const ulong *sect_buf, int words)
{
	uchar *dbuf;
	volatile uchar *pbuf_even;
	volatile uchar *pbuf_odd;

	pbuf_even = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_EVEN);
	pbuf_odd = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_ODD);
	dbuf = (uchar *) sect_buf;
	while (words--) {
		EIEIO;
		*pbuf_even = *dbuf++;
		EIEIO;
		*pbuf_odd = *dbuf++;
		EIEIO;
		*pbuf_even = *dbuf++;
		EIEIO;
		*pbuf_odd = *dbuf++;
	}
}

void ide_output_data_shorts(int dev, ushort *sect_buf, int shorts)
{
	uchar *dbuf;
	volatile uchar *pbuf_even;
	volatile uchar *pbuf_odd;

	pbuf_even = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_EVEN);
	pbuf_odd = (uchar *) (ATA_CURR_BASE(dev) + ATA_DATA_ODD);
	dbuf = (uchar *) sect_buf;
	while (shorts--) {
		EIEIO;
		*pbuf_even = *dbuf++;
		EIEIO;
		*pbuf_odd = *dbuf++;
	}
}

void ide_led(uchar led, uchar status)
{
	u_char	val;
	/* We have one PCMCIA slot and use LED H4 for the IDE Interface */
	val = readb(BCSR_BASE + 0x04);
	if (status)				/* led on */
		val |= B_CTRL_LED0;
	else
		val &= ~B_CTRL_LED0;

	writeb(val, BCSR_BASE + 0x04);
}

