/*
 * (C) Copyright 2004
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de
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
#include "../common/kup.h"


#define  SL811_ADR (0x50000000)
#define  SL811_DAT (0x50000001)


static void sl811_write_index_data (__u8 index, __u8 data)
{
	*(volatile unsigned char *) (SL811_ADR) = index;
	__asm__ ("eieio");
	*(volatile unsigned char *) (SL811_DAT) = data;
	__asm__ ("eieio");
}

static __u8 sl811_read_index_data (__u8 index)
{
	__u8 data;

	*(volatile unsigned char *) (SL811_ADR) = index;
	__asm__ ("eieio");
	data = *(volatile unsigned char *) (SL811_DAT);
	__asm__ ("eieio");
	return (data);
}

int usb_init_kup4x (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	int i;
	unsigned char tmp;

	memctl = &immap->im_memctl;
	memctl->memc_or7 = 0xFFFF8726;
	memctl->memc_br7 = 0x50000401;	/* start at 0x50000000 */
	/* BP 14 low = USB ON */
	immap->im_cpm.cp_pbdat &= ~(BP_USB_VCC);
	/* PB 14 nomal port */
	immap->im_cpm.cp_pbpar &= ~(BP_USB_VCC);
	/* output */
	immap->im_cpm.cp_pbdir |= (BP_USB_VCC);

	puts ("USB:   ");

	for (i = 0x10; i < 0xff; i++) {
		sl811_write_index_data (i, i);
		tmp = (sl811_read_index_data (i));
		if (tmp != i) {
			printf ("SL811 compare error index=0x%02x read=0x%02x\n", i, tmp);
			return (-1);
		}
	}
	printf ("SL811 ready\n");
	return (0);
}
