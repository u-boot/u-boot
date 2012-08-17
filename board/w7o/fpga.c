/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com
 *  and
 * Bill Hunter, Wave 7 Optics, william.hunter@mediaone.net
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#include <common.h>
#include "w7o.h"
#include <asm/processor.h>
#include <linux/compiler.h>
#include "errors.h"

static void
fpga_img_write(unsigned long *src, unsigned long len, unsigned short *daddr)
{
	unsigned long i;
	volatile unsigned long val;
	volatile unsigned short *dest = daddr;	/* volatile-bypass optimizer */

	for (i = 0; i < len; i++, src++) {
		val = *src;
		*dest = (unsigned short) ((val & 0xff000000L) >> 16);
		*dest = (unsigned short) ((val & 0x00ff0000L) >> 8);
		*dest = (unsigned short) (val & 0x0000ff00L);
		*dest = (unsigned short) ((val & 0x000000ffL) << 8);
	}

	/* Terminate programming with 4 C clocks */
	dest = daddr;
	val = *(unsigned short *) dest;
	val = *(unsigned short *) dest;
	val = *(unsigned short *) dest;
	val = *(unsigned short *) dest;

}


int
fpgaDownload(unsigned char *saddr, unsigned long size, unsigned short *daddr)
{
	int i;			/* index, intr disable flag */
	int start;		/* timer */
	unsigned long greg, grego;	/* GPIO & output register */
	unsigned long length;	/* image size in words */
	unsigned long *source;	/* image source addr */
	unsigned short *dest;	/* destination FPGA addr */
	volatile unsigned short *ndest;	/* temp dest FPGA addr */
	unsigned long cnfg = GPIO_XCV_CNFG;	/* FPGA CNFG */
	unsigned long eirq = GPIO_XCV_IRQ;
	int retval = -1;	/* Function return value */
	__maybe_unused volatile unsigned short val;	/* temp val */

	/* Setup some basic values */
	length = (size / 4) + 1;	/* size in words, rounding UP
					   is OK */
	source = (unsigned long *) saddr;
	dest = (unsigned short *) daddr;

	/* Get DCR output register */
	grego = in32(PPC405GP_GPIO0_OR);

	/* Reset FPGA */
	grego &= ~GPIO_XCV_PROG;	/* PROG line low */
	out32(PPC405GP_GPIO0_OR, grego);

	/* Setup timeout timer */
	start = get_timer(0);

	/* Wait for FPGA init line to go low */
	while (in32(PPC405GP_GPIO0_IR) & GPIO_XCV_INIT) {
		/* Check for timeout - 100us max, so use 3ms */
		if (get_timer(start) > 3) {
			printf("     failed to start init.\n");
			log_warn(ERR_XINIT0);	/* Don't halt */

			/* Reset line stays low */
			goto done;	/* I like gotos... */
		}
	}

	/* Unreset FPGA */
	grego |= GPIO_XCV_PROG;	/* PROG line high */
	out32(PPC405GP_GPIO0_OR, grego);

	/* Wait for FPGA end of init period = init line go hi  */
	while (!(in32(PPC405GP_GPIO0_IR) & GPIO_XCV_INIT)) {

		/* Check for timeout */
		if (get_timer(start) > 3) {
			printf("     failed to exit init.\n");
			log_warn(ERR_XINIT1);

			/* Reset FPGA */
			grego &= ~GPIO_XCV_PROG;	/* PROG line low */
			out32(PPC405GP_GPIO0_OR, grego);

			goto done;
		}
	}

	/* Now program FPGA ... */
	ndest = dest;
	for (i = 0; i < CONFIG_NUM_FPGAS; i++) {
		/* Toggle IRQ/GPIO */
		greg = mfdcr(CPC0_CR0);	/* get chip ctrl register */
		greg |= eirq;	/* toggle irq/gpio */
		mtdcr(CPC0_CR0, greg);	/*  ... just do it */

		/* turn on open drain for CNFG */
		greg = in32(PPC405GP_GPIO0_ODR); /* get open drain register */
		greg |= cnfg;	/* CNFG open drain */
		out32(PPC405GP_GPIO0_ODR, greg); /*  .. just do it */

		/* Turn output enable on for CNFG */
		greg = in32(PPC405GP_GPIO0_TCR); /* get tristate register */
		greg |= cnfg;	/* CNFG tristate inactive */
		out32(PPC405GP_GPIO0_TCR, greg); /*  ... just do it */

		/* Setup FPGA for programming */
		grego &= ~cnfg;	/* CONFIG line low */
		out32(PPC405GP_GPIO0_OR, grego);

		/*
		 * Program the FPGA
		 */
		printf("\n       destination: 0x%lx ", (unsigned long) ndest);

		fpga_img_write(source, length, (unsigned short *) ndest);

		/* Done programming */
		grego |= cnfg;	/* CONFIG line high */
		out32(PPC405GP_GPIO0_OR, grego);

		/* Turn output enable OFF for CNFG */
		greg = in32(PPC405GP_GPIO0_TCR); /* get tristate register */
		greg &= ~cnfg;	/* CNFG tristate inactive */
		out32(PPC405GP_GPIO0_TCR, greg); /*  ... just do it */

		/* Toggle IRQ/GPIO */
		greg = mfdcr(CPC0_CR0);	/* get chip ctrl register */
		greg &= ~eirq;	/* toggle irq/gpio */
		mtdcr(CPC0_CR0, greg);	/*  ... just do it */

		/* XXX - Next FPGA addr */
		ndest = (unsigned short *) ((char *) ndest + 0x00100000L);
		cnfg >>= 1;	/* XXX - Next  */
		eirq >>= 1;
	}

	/* Terminate programming with 4 C clocks */
	ndest = dest;
	for (i = 0; i < CONFIG_NUM_FPGAS; i++) {
		val = *ndest;
		val = *ndest;
		val = *ndest;
		val = *ndest;
		ndest = (unsigned short *) ((char *) ndest + 0x00100000L);
	}

	/* Setup timer */
	start = get_timer(0);

	/* Wait for FPGA end of programming period = Test DONE low  */
	while (!(in32(PPC405GP_GPIO0_IR) & GPIO_XCV_DONE)) {

		/* Check for timeout */
		if (get_timer(start) > 3) {
			printf("     done failed to come high.\n");
			log_warn(ERR_XDONE1);

			/* Reset FPGA */
			grego &= ~GPIO_XCV_PROG;	/* PROG line low */
			out32(PPC405GP_GPIO0_OR, grego);

			goto done;
		}
	}

	printf("\n       FPGA load succeeded\n");
	retval = 0;		/* Program OK */

done:
	return retval;
}

/* FPGA image is stored in flash */
extern flash_info_t flash_info[];

int init_fpga(void)
{
	unsigned int i, j, ptr;	/* General purpose */
	unsigned char bufchar;	/* General purpose character */
	unsigned char *buf;	/* Start of image pointer */
	unsigned long len;	/* Length of image */
	unsigned char *fn_buf;	/* Start of filename string */
	unsigned int fn_len;	/* Length of filename string */
	unsigned char *xcv_buf;	/* Pointer to start of image */
	unsigned long xcv_len;	/* Length of image */
	unsigned long crc;	/* 30bit crc in image */
	unsigned long calc_crc;	/* Calc'd 30bit crc */
	int retval = -1;

	/* Tell the world what we are doing */
	printf("FPGA:  ");

	/*
	 * Get address of first sector where the FPGA
	 * image is stored.
	 */
	buf = (unsigned char *) flash_info[1].start[0];

	/*
	 * Get the stored image's CRC & length.
	 */
	crc = *(unsigned long *) (buf + 4);	/* CRC is first long word */
	len = *(unsigned long *) (buf + 8);	/* Image len is next long */

	/* Pedantic */
	if ((len < 0x133A4) || (len > 0x80000))
		goto bad_image;

	/*
	 * Get the file name pointer and length.
	 * filename length is next short
	 */
	fn_len = (*(unsigned short *) (buf + 12) & 0xff);
	fn_buf = buf + 14;

	/*
	 * Get the FPGA image pointer and length length.
	 */
	xcv_buf = fn_buf + fn_len;	/* pointer to fpga image */
	xcv_len = len - 14 - fn_len;	/* fpga image length */

	/* Check for uninitialized FLASH */
	if ((strncmp((char *) buf, "w7o", 3) != 0) || (len > 0x0007ffffL)
	    || (len == 0))
		goto bad_image;

	/*
	 * Calculate and Check the image's CRC.
	 */
	calc_crc = crc32(0, xcv_buf, xcv_len);
	if (crc != calc_crc) {
		printf("\nfailed - bad CRC\n");
		goto done;
	}

	/* Output the file name */
	printf("file name  : ");
	for (i = 0; i < fn_len; i++) {
		bufchar = fn_buf[+i];
		if (bufchar < ' ' || bufchar > '~')
			bufchar = '.';
		putc(bufchar);
	}

	/*
	 * find rest of display data
	 */
	ptr = 15;		/* Offset to ncd filename
				   length in fpga image */
	j = xcv_buf[ptr];	/* Get len of ncd filename */
	if (j > 32)
		goto bad_image;
	ptr = ptr + j + 3;	/* skip ncd filename string +
				   3 bytes more bytes */

	/*
	 * output target device string
	 */
	j = xcv_buf[ptr++] - 1;	/* len of targ str less term */
	if (j > 32)
		goto bad_image;
	printf("\n       target     : ");
	for (i = 0; i < j; i++) {
		bufchar = (xcv_buf[ptr++]);
		if (bufchar < ' ' || bufchar > '~')
			bufchar = '.';
		putc(bufchar);
	}

	/*
	 * output compilation date string and time string
	 */
	ptr += 3;		/* skip 2 bytes */
	printf("\n       synth time : ");
	j = (xcv_buf[ptr++] - 1);	/* len of date str less term */
	if (j > 32)
		goto bad_image;
	for (i = 0; i < j; i++) {
		bufchar = (xcv_buf[ptr++]);
		if (bufchar < ' ' || bufchar > '~')
			bufchar = '.';
		putc(bufchar);
	}

	ptr += 3;		/* Skip 2 bytes */
	printf(" - ");
	j = (xcv_buf[ptr++] - 1);	/* slen = targ dev str len */
	if (j > 32)
		goto bad_image;
	for (i = 0; i < j; i++) {
		bufchar = (xcv_buf[ptr++]);
		if (bufchar < ' ' || bufchar > '~')
			bufchar = '.';
		putc(bufchar);
	}

	/*
	 * output crc and length strings
	 */
	printf("\n       len & crc  : 0x%lx  0x%lx", len, crc);

	/*
	 * Program the FPGA.
	 */
	retval = fpgaDownload((unsigned char *) xcv_buf, xcv_len,
			      (unsigned short *) 0xfd000000L);
	return retval;

bad_image:
	printf("\n       BAD FPGA image format @ %lx\n",
	       flash_info[1].start[0]);
	log_warn(ERR_XIMAGE);
done:
	return retval;
}

void test_fpga(unsigned short *daddr)
{
	int i;
	volatile unsigned short *ndest = daddr;

	for (i = 0; i < CONFIG_NUM_FPGAS; i++) {
#if defined(CONFIG_W7OLMG)
		ndest[0x7e] = 0x55aa;
		if (ndest[0x7e] != 0x55aa)
			log_warn(ERR_XRW1 + i);
		ndest[0x7e] = 0xaa55;
		if (ndest[0x7e] != 0xaa55)
			log_warn(ERR_XRW1 + i);
		ndest[0x7e] = 0xc318;
		if (ndest[0x7e] != 0xc318)
			log_warn(ERR_XRW1 + i);

#elif defined(CONFIG_W7OLMC)
		ndest[0x800] = 0x55aa;
		ndest[0x801] = 0xaa55;
		ndest[0x802] = 0xc318;
		ndest[0x4800] = 0x55aa;
		ndest[0x4801] = 0xaa55;
		ndest[0x4802] = 0xc318;
		if ((ndest[0x800] != 0x55aa) ||
		    (ndest[0x801] != 0xaa55) || (ndest[0x802] != 0xc318))
			log_warn(ERR_XRW1 + (2 * i)); /* Auto gen error code */
		if ((ndest[0x4800] != 0x55aa) ||
		    (ndest[0x4801] != 0xaa55) || (ndest[0x4802] != 0xc318))
			log_warn(ERR_XRW2 + (2 * i)); /* Auto gen error code */

#else
#error "Unknown W7O board configuration"
#endif
	}

	printf("       FPGA ready\n");
	return;
}
