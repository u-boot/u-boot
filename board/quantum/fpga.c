/*
 * (C) Copyright 2001-2003
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
/* The DEBUG define must be before common to enable debugging */
#undef DEBUG
#include <common.h>
#include <asm/processor.h>
#include <command.h>
#include "fpga.h"
/* ------------------------------------------------------------------------- */

#define MAX_ONES               226

/* MPC850 port D */
#define PD(bit) (1 << (15 - (bit)))
# define FPGA_INIT             PD(11)	/* FPGA init pin (ppc input)     */
# define FPGA_PRG              PD(12)	/* FPGA program pin (ppc output) */
# define FPGA_CLK              PD(13)	/* FPGA clk pin (ppc output)     */
# define FPGA_DATA             PD(14)	/* FPGA data pin (ppc output)    */
# define FPGA_DONE             PD(15)	/* FPGA done pin (ppc input)     */


/* DDR 0 - input, 1 - output */
#define FPGA_INIT_PDDIR          FPGA_PRG | FPGA_CLK | FPGA_DATA	/* just set outputs */


#define SET_FPGA(data)         immr->im_ioport.iop_pddat = (data)
#define GET_FPGA               immr->im_ioport.iop_pddat

#define FPGA_WRITE_1 {                                                    \
	SET_FPGA(FPGA_PRG |            FPGA_DATA);  /* set clock to 0 */  \
	SET_FPGA(FPGA_PRG |            FPGA_DATA);  /* set data to 1  */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);  /* set clock to 1 */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);}	/* set data to 1  */

#define FPGA_WRITE_0 {                                                    \
	SET_FPGA(FPGA_PRG |            FPGA_DATA);  /* set clock to 0 */  \
	SET_FPGA(FPGA_PRG);                         /* set data to 0  */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK);              /* set clock to 1 */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);}	/* set data to 1  */


int fpga_boot (unsigned char *fpgadata, int size)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	int i, index, len;
	int count;

#ifdef CONFIG_SYS_FPGA_SPARTAN2
	int j;
	unsigned char data;
#else
	unsigned char b;
	int bit;
#endif

	debug ("fpga_boot: fpgadata = %p, size = %d\n", fpgadata, size);

	/* display infos on fpgaimage */
	printf ("FPGA:");
	index = 15;
	for (i = 0; i < 4; i++) {
		len = fpgadata[index];
		printf (" %s", &(fpgadata[index + 1]));
		index += len + 3;
	}
	printf ("\n");


	index = 0;

#ifdef CONFIG_SYS_FPGA_SPARTAN2
	/* search for preamble 0xFFFFFFFF */
	while (1) {
		if ((fpgadata[index] == 0xff) && (fpgadata[index + 1] == 0xff)
		    && (fpgadata[index + 2] == 0xff)
		    && (fpgadata[index + 3] == 0xff))
			break;	/* preamble found */
		else
			index++;
	}
#else
	/* search for preamble 0xFF2X */
	for (index = 0; index < size - 1; index++) {
		if ((fpgadata[index] == 0xff)
		    && ((fpgadata[index + 1] & 0xf0) == 0x30))
			break;
	}
	index += 2;
#endif

	debug ("FPGA: configdata starts at position 0x%x\n", index);
	debug ("FPGA: length of fpga-data %d\n", size - index);

	/*
	 * Setup port pins for fpga programming
	 */
	immr->im_ioport.iop_pddir = FPGA_INIT_PDDIR;

	debug ("%s, ", ((GET_FPGA & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	debug ("%s\n", ((GET_FPGA & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	/*
	 * Init fpga by asserting and deasserting PROGRAM*
	 */
	SET_FPGA (FPGA_CLK | FPGA_DATA);

	/* Wait for FPGA init line low */
	count = 0;
	while (GET_FPGA & FPGA_INIT) {
		udelay (1000);	/* wait 1ms */
		/* Check for timeout - 100us max, so use 3ms */
		if (count++ > 3) {
			debug ("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_INIT_LOW;
		}
	}

	debug ("%s, ", ((GET_FPGA & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	debug ("%s\n", ((GET_FPGA & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	/* deassert PROGRAM* */
	SET_FPGA (FPGA_PRG | FPGA_CLK | FPGA_DATA);

	/* Wait for FPGA end of init period .  */
	count = 0;
	while (!(GET_FPGA & FPGA_INIT)) {
		udelay (1000);	/* wait 1ms */
		/* Check for timeout */
		if (count++ > 3) {
			debug ("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_INIT_HIGH;
		}
	}

	debug ("%s, ", ((GET_FPGA & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	debug ("%s\n", ((GET_FPGA & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	debug ("write configuration data into fpga\n");
	/* write configuration-data into fpga... */

#ifdef CONFIG_SYS_FPGA_SPARTAN2
	/*
	 * Load uncompressed image into fpga
	 */
	for (i = index; i < size; i++) {
#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		if ((i % 1024) == 0)
			printf ("%6d out of %6d\r", i, size);	/* let them know we are alive */
#endif

		data = fpgadata[i];
		for (j = 0; j < 8; j++) {
			if ((data & 0x80) == 0x80) {
				FPGA_WRITE_1;
			} else {
				FPGA_WRITE_0;
			}
			data <<= 1;
		}
	}
	/* add some 0xff to the end of the file */
	for (i = 0; i < 8; i++) {
		data = 0xff;
		for (j = 0; j < 8; j++) {
			if ((data & 0x80) == 0x80) {
				FPGA_WRITE_1;
			} else {
				FPGA_WRITE_0;
			}
			data <<= 1;
		}
	}
#else
	/* send 0xff 0x20 */
	FPGA_WRITE_1;
	FPGA_WRITE_1;
	FPGA_WRITE_1;
	FPGA_WRITE_1;
	FPGA_WRITE_1;
	FPGA_WRITE_1;
	FPGA_WRITE_1;
	FPGA_WRITE_1;
	FPGA_WRITE_0;
	FPGA_WRITE_0;
	FPGA_WRITE_1;
	FPGA_WRITE_0;
	FPGA_WRITE_0;
	FPGA_WRITE_0;
	FPGA_WRITE_0;
	FPGA_WRITE_0;

	/*
	 ** Bit_DeCompression
	 **   Code 1           .. maxOnes     : n                 '1's followed by '0'
	 **        maxOnes + 1 .. maxOnes + 1 : n - 1             '1's no '0'
	 **        maxOnes + 2 .. 254         : n - (maxOnes + 2) '0's followed by '1'
	 **        255                        :                   '1'
	 */

	for (i = index; i < size; i++) {
		b = fpgadata[i];
		if ((b >= 1) && (b <= MAX_ONES)) {
			for (bit = 0; bit < b; bit++) {
				FPGA_WRITE_1;
			}
			FPGA_WRITE_0;
		} else if (b == (MAX_ONES + 1)) {
			for (bit = 1; bit < b; bit++) {
				FPGA_WRITE_1;
			}
		} else if ((b >= (MAX_ONES + 2)) && (b <= 254)) {
			for (bit = 0; bit < (b - (MAX_ONES + 2)); bit++) {
				FPGA_WRITE_0;
			}
			FPGA_WRITE_1;
		} else if (b == 255) {
			FPGA_WRITE_1;
		}
	}
#endif
	debug ("\n\n");
	debug ("%s, ", ((GET_FPGA & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	debug ("%s\n", ((GET_FPGA & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	/*
	 * Check if fpga's DONE signal - correctly booted ?
	 */

	/* Wait for FPGA end of programming period .  */
	count = 0;
	while (!(GET_FPGA & FPGA_DONE)) {
		udelay (1000);	/* wait 1ms */
		/* Check for timeout */
		if (count++ > 3) {
			debug ("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_DONE;
		}
	}

	debug ("FPGA: Booting successful!\n");
	return 0;
}
