/*
 * (C) Copyright 2001-2003
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#define MAX_ONES               226

#ifdef CONFIG_SYS_FPGA_PRG
# define FPGA_PRG              CONFIG_SYS_FPGA_PRG /* FPGA program pin (ppc output)*/
# define FPGA_CLK              CONFIG_SYS_FPGA_CLK /* FPGA clk pin (ppc output)    */
# define FPGA_DATA             CONFIG_SYS_FPGA_DATA /* FPGA data pin (ppc output)  */
# define FPGA_DONE             CONFIG_SYS_FPGA_DONE /* FPGA done pin (ppc input)   */
# define FPGA_INIT             CONFIG_SYS_FPGA_INIT /* FPGA init pin (ppc input)   */
#else
# define FPGA_PRG              0x04000000  /* FPGA program pin (ppc output) */
# define FPGA_CLK              0x02000000  /* FPGA clk pin (ppc output)     */
# define FPGA_DATA             0x01000000  /* FPGA data pin (ppc output)    */
# define FPGA_DONE             0x00800000  /* FPGA done pin (ppc input)     */
# define FPGA_INIT             0x00400000  /* FPGA init pin (ppc input)     */
#endif

#define ERROR_FPGA_PRG_INIT_LOW  -1        /* Timeout after PRG* asserted   */
#define ERROR_FPGA_PRG_INIT_HIGH -2        /* Timeout after PRG* deasserted */
#define ERROR_FPGA_PRG_DONE      -3        /* Timeout after programming     */

#define SET_FPGA(data)         out32(GPIO0_OR, data)

#define FPGA_WRITE_1 {                                                    \
	SET_FPGA(FPGA_PRG |            FPGA_DATA);  /* set clock to 0 */  \
	SET_FPGA(FPGA_PRG |            FPGA_DATA);  /* set data to 1  */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);  /* set clock to 1 */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);} /* set data to 1  */

#define FPGA_WRITE_0 {                                                    \
	SET_FPGA(FPGA_PRG |            FPGA_DATA);  /* set clock to 0 */  \
	SET_FPGA(FPGA_PRG);                         /* set data to 0  */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK);              /* set clock to 1 */  \
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);} /* set data to 1  */

#if 0
static int fpga_boot (unsigned char *fpgadata, int size)
{
	int i, index, len;
	int count;

#ifdef CONFIG_SYS_FPGA_SPARTAN2
	int j;
#else
	unsigned char b;
	int bit;
#endif

	/* display infos on fpgaimage */
	index = 15;
	for (i = 0; i < 4; i++) {
		len = fpgadata[index];
		DBG ("FPGA: %s\n", &(fpgadata[index + 1]));
		index += len + 3;
	}

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

	DBG ("FPGA: configdata starts at position 0x%x\n", index);
	DBG ("FPGA: length of fpga-data %d\n", size - index);

	/*
	 * Setup port pins for fpga programming
	 */
	out32 (GPIO0_ODR, 0x00000000);	/* no open drain pins */
	out32 (GPIO0_TCR, in32 (GPIO0_TCR) | FPGA_PRG | FPGA_CLK | FPGA_DATA);	/* setup for output */
	out32 (GPIO0_OR, in32 (GPIO0_OR) | FPGA_PRG | FPGA_CLK | FPGA_DATA);	/* set pins to high */

	DBG ("%s, ",
	     ((in32 (GPIO0_IR) & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	DBG ("%s\n",
	     ((in32 (GPIO0_IR) & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	/*
	 * Init fpga by asserting and deasserting PROGRAM*
	 */
	SET_FPGA (FPGA_CLK | FPGA_DATA);

	/* Wait for FPGA init line low */
	count = 0;
	while (in32 (GPIO0_IR) & FPGA_INIT) {
		udelay (1000);	/* wait 1ms */
		/* Check for timeout - 100us max, so use 3ms */
		if (count++ > 3) {
			DBG ("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_INIT_LOW;
		}
	}

	DBG ("%s, ",
	     ((in32 (GPIO0_IR) & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	DBG ("%s\n",
	     ((in32 (GPIO0_IR) & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	/* deassert PROGRAM* */
	SET_FPGA (FPGA_PRG | FPGA_CLK | FPGA_DATA);

	/* Wait for FPGA end of init period .  */
	count = 0;
	while (!(in32 (GPIO0_IR) & FPGA_INIT)) {
		udelay (1000);	/* wait 1ms */
		/* Check for timeout */
		if (count++ > 3) {
			DBG ("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_INIT_HIGH;
		}
	}

	DBG ("%s, ",
	     ((in32 (GPIO0_IR) & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	DBG ("%s\n",
	     ((in32 (GPIO0_IR) & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	DBG ("write configuration data into fpga\n");
	/* write configuration-data into fpga... */

#ifdef CONFIG_SYS_FPGA_SPARTAN2
	/*
	 * Load uncompressed image into fpga
	 */
	for (i = index; i < size; i++) {
		for (j = 0; j < 8; j++) {
			if ((fpgadata[i] & 0x80) == 0x80) {
				FPGA_WRITE_1;
			} else {
				FPGA_WRITE_0;
			}
			fpgadata[i] <<= 1;
		}
	}
#else	/* ! CONFIG_SYS_FPGA_SPARTAN2 */
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
#endif	/* CONFIG_SYS_FPGA_SPARTAN2 */

	DBG ("%s, ",
	     ((in32 (GPIO0_IR) & FPGA_DONE) == 0) ? "NOT DONE" : "DONE");
	DBG ("%s\n",
	     ((in32 (GPIO0_IR) & FPGA_INIT) == 0) ? "NOT INIT" : "INIT");

	/*
	 * Check if fpga's DONE signal - correctly booted ?
	 */

	/* Wait for FPGA end of programming period .  */
	count = 0;
	while (!(in32 (GPIO0_IR) & FPGA_DONE)) {
		udelay (1000);	/* wait 1ms */
		/* Check for timeout */
		if (count++ > 3) {
			DBG ("FPGA: Booting failed!\n");
			return ERROR_FPGA_PRG_DONE;
		}
	}

	DBG ("FPGA: Booting successful!\n");
	return 0;
}
#endif	/* 0 */
