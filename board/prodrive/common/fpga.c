/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2001-2004
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

#include <common.h>
#include <asm/processor.h>
#include <command.h>

/* ------------------------------------------------------------------------- */

#ifdef FPGA_DEBUG
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif /* DEBUG */

#define FPGA_PRG		CFG_FPGA_PRG /* FPGA program pin (cpu output)*/
#define FPGA_CLK		CFG_FPGA_CLK /* FPGA clk pin (cpu output)    */
#define FPGA_DATA		CFG_FPGA_DATA /* FPGA data pin (cpu output)  */
#define FPGA_DONE		CFG_FPGA_DONE /* FPGA done pin (cpu input)   */
#define FPGA_INIT		CFG_FPGA_INIT /* FPGA init pin (cpu input)   */

#define ERROR_FPGA_PRG_INIT_LOW  -1        /* Timeout after PRG* asserted   */
#define ERROR_FPGA_PRG_INIT_HIGH -2        /* Timeout after PRG* deasserted */
#define ERROR_FPGA_PRG_DONE      -3        /* Timeout after programming     */

#ifndef OLD_VAL
# define OLD_VAL		0
#endif

#if 0 /* test-only */
#define FPGA_WRITE_1 { \
		SET_FPGA(OLD_VAL | FPGA_PRG | 0        | FPGA_DATA);  /* set clock to 0 */ \
		SET_FPGA(OLD_VAL | FPGA_PRG | 0        | FPGA_DATA);  /* set data to 1  */	\
		SET_FPGA(OLD_VAL | FPGA_PRG | FPGA_CLK | FPGA_DATA);  /* set clock to 1 */	\
		SET_FPGA(OLD_VAL | FPGA_PRG | FPGA_CLK | FPGA_DATA);} /* set data to 1  */

#define FPGA_WRITE_0 { \
		SET_FPGA(OLD_VAL | FPGA_PRG | 0        | FPGA_DATA);  /* set clock to 0 */	\
		SET_FPGA(OLD_VAL | FPGA_PRG | 0        | 0        );  /* set data to 0  */	\
		SET_FPGA(OLD_VAL | FPGA_PRG | FPGA_CLK | 0        );  /* set clock to 1 */	\
		SET_FPGA(OLD_VAL | FPGA_PRG | FPGA_CLK | FPGA_DATA);} /* set data to 1  */
#else
#define FPGA_WRITE_1 { \
		SET_FPGA(OLD_VAL | FPGA_PRG | 0        | FPGA_DATA);  /* set data to 1  */	\
		SET_FPGA(OLD_VAL | FPGA_PRG | FPGA_CLK | FPGA_DATA);} /* set data to 1  */

#define FPGA_WRITE_0 { \
		SET_FPGA(OLD_VAL | FPGA_PRG | 0        | 0        );   /* set data to 0  */	\
		SET_FPGA(OLD_VAL | FPGA_PRG | FPGA_CLK | 0        );}  /* set data to 1  */
#endif

static int fpga_boot(unsigned char *fpgadata, int size)
{
	int i,index,len;
	int count;
	int j;

	/* display infos on fpgaimage */
	index = 15;
	for (i=0; i<4; i++) {
		len = fpgadata[index];
		DBG("FPGA: %s\n", &(fpgadata[index+1]));
		index += len+3;
	}

	/* search for preamble 0xFFFFFFFF */
	while (1) {
		if ((fpgadata[index] == 0xff) && (fpgadata[index+1] == 0xff) &&
		    (fpgadata[index+2] == 0xff) && (fpgadata[index+3] == 0xff))
			break; /* preamble found */
		else
			index++;
	}

	DBG("FPGA: configdata starts at position 0x%x\n",index);
	DBG("FPGA: length of fpga-data %d\n", size-index);

	/*
	 * Setup port pins for fpga programming
	 */
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);            /* set pins to high */

	DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
	DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

	/*
	 * Init fpga by asserting and deasserting PROGRAM*
	 */
	SET_FPGA(0 | FPGA_CLK | FPGA_DATA);             /* set prog active */

	/* Wait for FPGA init line low */
	count = 0;
	while (FPGA_INIT_STATE) {
		udelay(1000); /* wait 1ms */
		/* Check for timeout - 100us max, so use 3ms */
		if (count++ > 3) {
			DBG("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_INIT_LOW;
		}
	}

	DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
	DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

	/* deassert PROGRAM* */
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);           /* set prog inactive */

	/* Wait for FPGA end of init period .  */
	count = 0;
	while (!(FPGA_INIT_STATE)) {
		udelay(1000); /* wait 1ms */
		/* Check for timeout */
		if (count++ > 3) {
			DBG("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_INIT_HIGH;
		}
	}

	DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
	DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

	DBG("write configuration data into fpga\n");
	/* write configuration-data into fpga... */

	/*
	 * Load uncompressed image into fpga
	 */
	for (i=index; i<size; i++) {
		for (j=0; j<8; j++) {
			if ((fpgadata[i] & 0x80) == 0x80) {
				FPGA_WRITE_1;
			} else {
				FPGA_WRITE_0;
			}
			fpgadata[i] <<= 1;
		}
	}

	DBG("%s, ",(FPGA_DONE_STATE == 0) ? "NOT DONE" : "DONE" );
	DBG("%s\n",(FPGA_INIT_STATE == 0) ? "NOT INIT" : "INIT" );

	/*
	 * Check if fpga's DONE signal - correctly booted ?
	 */

	/* Wait for FPGA end of programming period .  */
	count = 0;
	while (!(FPGA_DONE_STATE)) {
		udelay(1000); /* wait 1ms */
		/* Check for timeout */
		if (count++ > 3) {
			DBG("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_DONE;
		}
	}

	DBG("FPGA: Booting successful!\n");
	return 0;
}
