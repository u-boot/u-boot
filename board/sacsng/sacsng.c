/*
 * (C) Copyright 2002
 * Custom IDEAS, Inc. <www.cideas.com>
 * Gerald Van Baren <vanbaren@cideas.com>
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

#include <asm/u-boot.h>
#include <common.h>
#include <ioports.h>
#include <mpc8260.h>
#include <i2c.h>
#include <spi.h>
#include <command.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
#include <status_led.h>
#endif

#ifdef CONFIG_ETHER_LOOPBACK_TEST
extern void eth_loopback_test(void);
#endif /* CONFIG_ETHER_LOOPBACK_TEST */

extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#include "clkinit.h"
#include "ioconfig.h" /* I/O configuration table */

/*
 * PBI Page Based Interleaving
 *   PSDMR_PBI page based interleaving
 *   0         bank based interleaving
 * External Address Multiplexing (EAMUX) adds a clock to address cycles
 *   (this can help with marginal board layouts)
 *   PSDMR_EAMUX  adds a clock
 *   0            no extra clock
 * Buffer Command (BUFCMD) adds a clock to command cycles.
 *   PSDMR_BUFCMD adds a clock
 *   0            no extra clock
 */
#define CONFIG_PBI		PSDMR_PBI
#define PESSIMISTIC_SDRAM	0
#define EAMUX			0	/* EST requires EAMUX */
#define BUFCMD			0

/*
 * ADC/DAC Defines:
 */
#define INITIAL_SAMPLE_RATE 10016     /* Initial Daq sample rate */
#define INITIAL_RIGHT_JUST  0         /* Initial DAC right justification */
#define INITIAL_MCLK_DIVIDE 0         /* Initial MCLK Divide */
#define INITIAL_SAMPLE_64X  1         /* Initial  64x clocking mode */
#define INITIAL_SAMPLE_128X 0         /* Initial 128x clocking mode */

/*
 * ADC Defines:
 */
#define I2C_ADC_1_ADDR 0x0E           /* I2C Address of the ADC #1 */
#define I2C_ADC_2_ADDR 0x0F           /* I2C Address of the ADC #2 */

#define ADC_SDATA1_MASK 0x00020000    /* PA14 - CH12SDATA_PU   */
#define ADC_SDATA2_MASK 0x00010000    /* PA15 - CH34SDATA_PU   */

#define ADC_VREF_CAP   100            /* VREF capacitor in uF */
#define ADC_INITIAL_DELAY (10 * ADC_VREF_CAP) /* 10 usec per uF, in usec */
#define ADC_SDATA_DELAY    100        /* ADC SDATA release delay in usec */
#define ADC_CAL_DELAY (1000000 / INITIAL_SAMPLE_RATE * 4500)
				      /* Wait at least 4100 LRCLK's */

#define ADC_REG1_FRAME_START    0x80  /* Frame start */
#define ADC_REG1_GROUND_CAL     0x40  /* Ground calibration enable */
#define ADC_REG1_ANA_MOD_PDOWN  0x20  /* Analog modulator section in power down */
#define ADC_REG1_DIG_MOD_PDOWN  0x10  /* Digital modulator section in power down */

#define ADC_REG2_128x           0x80  /* Oversample at 128x */
#define ADC_REG2_CAL            0x40  /* System calibration enable */
#define ADC_REG2_CHANGE_SIGN    0x20  /* Change sign enable */
#define ADC_REG2_LR_DISABLE     0x10  /* Left/Right output disable */
#define ADC_REG2_HIGH_PASS_DIS  0x08  /* High pass filter disable */
#define ADC_REG2_SLAVE_MODE     0x04  /* Slave mode */
#define ADC_REG2_DFS            0x02  /* Digital format select */
#define ADC_REG2_MUTE           0x01  /* Mute */

#define ADC_REG7_ADDR_ENABLE    0x80  /* Address enable */
#define ADC_REG7_PEAK_ENABLE    0x40  /* Peak enable */
#define ADC_REG7_PEAK_UPDATE    0x20  /* Peak update */
#define ADC_REG7_PEAK_FORMAT    0x10  /* Peak display format */
#define ADC_REG7_DIG_FILT_PDOWN 0x04  /* Digital filter power down enable */
#define ADC_REG7_FIR2_IN_EN     0x02  /* External FIR2 input enable */
#define ADC_REG7_PSYCHO_EN      0x01  /* External pyscho filter input enable */

/*
 * DAC Defines:
 */

#define I2C_DAC_ADDR 0x11             /* I2C Address of the DAC */

#define DAC_RST_MASK 0x00008000       /* PA16 - DAC_RST*  */
#define DAC_RESET_DELAY    100        /* DAC reset delay in usec */
#define DAC_INITIAL_DELAY 5000        /* DAC initialization delay in usec */

#define DAC_REG1_AMUTE   0x80         /* Auto-mute */

#define DAC_REG1_LEFT_JUST_24_BIT (0 << 4) /* Fmt 0: Left justified 24 bit  */
#define DAC_REG1_I2S_24_BIT       (1 << 4) /* Fmt 1: I2S up to 24 bit       */
#define DAC_REG1_RIGHT_JUST_16BIT (2 << 4) /* Fmt 2: Right justified 16 bit */
#define DAC_REG1_RIGHT_JUST_24BIT (3 << 4) /* Fmt 3: Right justified 24 bit */
#define DAC_REG1_RIGHT_JUST_20BIT (4 << 4) /* Fmt 4: Right justified 20 bit */
#define DAC_REG1_RIGHT_JUST_18BIT (5 << 4) /* Fmt 5: Right justified 18 bit */

#define DAC_REG1_DEM_NO           (0 << 2) /* No      De-emphasis  */
#define DAC_REG1_DEM_44KHZ        (1 << 2) /* 44.1KHz De-emphasis  */
#define DAC_REG1_DEM_48KHZ        (2 << 2) /* 48KHz   De-emphasis  */
#define DAC_REG1_DEM_32KHZ        (3 << 2) /* 32KHz   De-emphasis  */

#define DAC_REG1_SINGLE 0             /*   4- 50KHz sample rate  */
#define DAC_REG1_DOUBLE 1             /*  50-100KHz sample rate  */
#define DAC_REG1_QUAD   2             /* 100-200KHz sample rate  */
#define DAC_REG1_DSD    3             /* Direct Stream Data, DSD */

#define DAC_REG5_INVERT_A   0x80      /* Invert channel A */
#define DAC_REG5_INVERT_B   0x40      /* Invert channel B */
#define DAC_REG5_I2C_MODE   0x20      /* Control port (I2C) mode */
#define DAC_REG5_POWER_DOWN 0x10      /* Power down mode */
#define DAC_REG5_MUTEC_A_B  0x08      /* Mutec A=B */
#define DAC_REG5_FREEZE     0x04      /* Freeze */
#define DAC_REG5_MCLK_DIV   0x02      /* MCLK divide by 2 */
#define DAC_REG5_RESERVED   0x01      /* Reserved */

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard(void)
{
    printf ("SACSng\n");

    return 0;
}

/* ------------------------------------------------------------------------- */

long int initdram(int board_type)
{
    volatile immap_t *immap  = (immap_t *)CFG_IMMR;
    volatile memctl8260_t *memctl = &immap->im_memctl;
    volatile uchar c = 0;
    volatile uchar *ramaddr = (uchar *)(CFG_SDRAM_BASE + 0x8);
    uint  psdmr = CFG_PSDMR;
    int   i;
    uint   psrt = 14;					/* for no SPD */
    uint   chipselects = 1;				/* for no SPD */
    uint   sdram_size = CFG_SDRAM0_SIZE * 1024 * 1024;	/* for no SPD */
    uint   or = CFG_OR2_PRELIM;				/* for no SPD */
#ifdef SDRAM_SPD_ADDR
    uint   data_width;
    uint   rows;
    uint   banks;
    uint   cols;
    uint   caslatency;
    uint   width;
    uint   rowst;
    uint   sdam;
    uint   bsma;
    uint   sda10;
    u_char spd_size;
    u_char data;
    u_char cksum;
    int    j;
#endif

#ifdef SDRAM_SPD_ADDR
    /* Keep the compiler from complaining about potentially uninitialized vars */
    data_width = chipselects = rows = banks = cols = caslatency = psrt = 0;

    /*
     * Read the SDRAM SPD EEPROM via I2C.
     */
    i2c_read(SDRAM_SPD_ADDR, 0, 1, &data, 1);
    spd_size = data;
    cksum    = data;
    for(j = 1; j < 64; j++) {	/* read only the checksummed bytes */
	/* note: the I2C address autoincrements when alen == 0 */
	i2c_read(SDRAM_SPD_ADDR, 0, 0, &data, 1);
	     if(j ==  5) chipselects = data & 0x0F;
	else if(j ==  6) data_width  = data;
	else if(j ==  7) data_width |= data << 8;
	else if(j ==  3) rows        = data & 0x0F;
	else if(j ==  4) cols        = data & 0x0F;
	else if(j == 12) {
	    /*
	     * Refresh rate: this assumes the prescaler is set to
	     * approximately 1uSec per tick.
	     */
	    switch(data & 0x7F) {
		default:
		case 0:  psrt =  14 ; /*  15.625uS */  break;
		case 1:  psrt =   2;  /*   3.9uS   */  break;
		case 2:  psrt =   6;  /*   7.8uS   */  break;
		case 3:  psrt =  29;  /*  31.3uS   */  break;
		case 4:  psrt =  60;  /*  62.5uS   */  break;
		case 5:  psrt = 120;  /* 125uS     */  break;
	    }
	}
	else if(j == 17) banks       = data;
	else if(j == 18) {
	    caslatency = 3; /* default CL */
#if(PESSIMISTIC_SDRAM)
		 if((data & 0x04) != 0) caslatency = 3;
	    else if((data & 0x02) != 0) caslatency = 2;
	    else if((data & 0x01) != 0) caslatency = 1;
#else
		 if((data & 0x01) != 0) caslatency = 1;
	    else if((data & 0x02) != 0) caslatency = 2;
	    else if((data & 0x04) != 0) caslatency = 3;
#endif
	    else {
		printf ("WARNING: Unknown CAS latency 0x%02X, using 3\n",
			data);
	    }
	}
	else if(j == 63) {
	    if(data != cksum) {
		printf ("WARNING: Configuration data checksum failure:"
			" is 0x%02x, calculated 0x%02x\n",
			data, cksum);
	    }
	}
	cksum += data;
    }

    /* We don't trust CL less than 2 (only saw it on an old 16MByte DIMM) */
    if(caslatency < 2) {
	printf("WARNING: CL was %d, forcing to 2\n", caslatency);
	caslatency = 2;
    }
    if(rows > 14) {
	printf("WARNING: This doesn't look good, rows = %d, should be <= 14\n", rows);
	rows = 14;
    }
    if(cols > 11) {
	printf("WARNING: This doesn't look good, columns = %d, should be <= 11\n", cols);
	cols = 11;
    }

    if((data_width != 64) && (data_width != 72))
    {
	printf("WARNING: SDRAM width unsupported, is %d, expected 64 or 72.\n",
	    data_width);
    }
    width = 3;		/* 2^3 = 8 bytes = 64 bits wide */
    /*
     * Convert banks into log2(banks)
     */
    if     (banks == 2)	banks = 1;
    else if(banks == 4)	banks = 2;
    else if(banks == 8)	banks = 3;

    sdram_size = 1 << (rows + cols + banks + width);

#if(CONFIG_PBI == 0)	/* bank-based interleaving */
    rowst = ((32 - 6) - (rows + cols + width)) * 2;
#else
    rowst = 32 - (rows + banks + cols + width);
#endif

    or = ~(sdram_size - 1)    |	/* SDAM address mask	*/
	  ((banks-1) << 13)   |	/* banks per device	*/
	  (rowst << 9)        |	/* rowst		*/
	  ((rows - 9) << 6);	/* numr			*/

    memctl->memc_or2 = or;

    /*
     * SDAM specifies the number of columns that are multiplexed
     * (reference AN2165/D), defined to be (columns - 6) for page
     * interleave, (columns - 8) for bank interleave.
     *
     * BSMA is 14 - max(rows, cols).  The bank select lines come
     * into play above the highest "address" line going into the
     * the SDRAM.
     */
#if(CONFIG_PBI == 0)	/* bank-based interleaving */
    sdam = cols - 8;
    bsma = ((31 - width) - 14) - ((rows > cols) ? rows : cols);
    sda10 = sdam + 2;
#else
    sdam = cols - 6;
    bsma = ((31 - width) - 14) - ((rows > cols) ? rows : cols);
    sda10 = sdam;
#endif
#if(PESSIMISTIC_SDRAM)
    psdmr = (CONFIG_PBI              |\
	     PSDMR_RFEN              |\
	     PSDMR_RFRC_16_CLK       |\
	     PSDMR_PRETOACT_8W       |\
	     PSDMR_ACTTORW_8W        |\
	     PSDMR_WRC_4C            |\
	     PSDMR_EAMUX             |\
	     PSDMR_BUFCMD)           |\
	     caslatency              |\
	     ((caslatency - 1) << 6) |	/* LDOTOPRE is CL - 1 */ \
	     (sdam << 24)            |\
	     (bsma << 21)            |\
	     (sda10 << 18);
#else
    psdmr = (CONFIG_PBI              |\
	     PSDMR_RFEN              |\
	     PSDMR_RFRC_7_CLK        |\
	     PSDMR_PRETOACT_3W       |	/* 1 for 7E parts (fast PC-133) */ \
	     PSDMR_ACTTORW_2W        |	/* 1 for 7E parts (fast PC-133) */ \
	     PSDMR_WRC_1C            |	/* 1 clock + 7nSec */
	     EAMUX                   |\
	     BUFCMD)                 |\
	     caslatency              |\
	     ((caslatency - 1) << 6) |	/* LDOTOPRE is CL - 1 */ \
	     (sdam << 24)            |\
	     (bsma << 21)            |\
	     (sda10 << 18);
#endif
#endif

    /*
     * Quote from 8260 UM (10.4.2 SDRAM Power-On Initialization, 10-35):
     *
     * "At system reset, initialization software must set up the
     *  programmable parameters in the memory controller banks registers
     *  (ORx, BRx, P/LSDMR). After all memory parameters are configured,
     *  system software should execute the following initialization sequence
     *  for each SDRAM device.
     *
     *  1. Issue a PRECHARGE-ALL-BANKS command
     *  2. Issue eight CBR REFRESH commands
     *  3. Issue a MODE-SET command to initialize the mode register
     *
     * Quote from Micron MT48LC8M16A2 data sheet:
     *
     *  "...the SDRAM requires a 100uS delay prior to issuing any
     *  command other than a COMMAND INHIBIT or NOP.  Starting at some
     *  point during this 100uS period and continuing at least through
     *  the end of this period, COMMAND INHIBIT or NOP commands should
     *  be applied."
     *
     *  "Once the 100uS delay has been satisfied with at least one COMMAND
     *  INHIBIT or NOP command having been applied, a /PRECHARGE command/
     *  should be applied.  All banks must then be precharged, thereby
     *  placing the device in the all banks idle state."
     *
     *  "Once in the idle state, /two/ AUTO REFRESH cycles must be
     *  performed.  After the AUTO REFRESH cycles are complete, the
     *  SDRAM is ready for mode register programming."
     *
     *  (/emphasis/ mine, gvb)
     *
     *  The way I interpret this, Micron start up sequence is:
     *  1. Issue a PRECHARGE-BANK command (initial precharge)
     *  2. Issue a PRECHARGE-ALL-BANKS command ("all banks ... precharged")
     *  3. Issue two (presumably, doing eight is OK) CBR REFRESH commands
     *  4. Issue a MODE-SET command to initialize the mode register
     *
     *  --------
     *
     *  The initial commands are executed by setting P/LSDMR[OP] and
     *  accessing the SDRAM with a single-byte transaction."
     *
     * The appropriate BRx/ORx registers have already been set when we
     * get here. The SDRAM can be accessed at the address CFG_SDRAM_BASE.
     */

    memctl->memc_mptpr = CFG_MPTPR;
    memctl->memc_psrt  = psrt;

    memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
    *ramaddr = c;

    memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
    for (i = 0; i < 8; i++)
	*ramaddr = c;

    memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
    *ramaddr = c;

    memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
    *ramaddr = c;

    /*
     * Do it a second time for the second set of chips if the DIMM has
     * two chip selects (double sided).
     */
    if(chipselects > 1) {
	ramaddr += sdram_size;

	memctl->memc_br3 = CFG_BR3_PRELIM + sdram_size;
	memctl->memc_or3 = or;

	memctl->memc_psdmr = psdmr | PSDMR_OP_PREA;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR;
	for (i = 0; i < 8; i++)
	    *ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_MRW;
	*ramaddr = c;

	memctl->memc_psdmr = psdmr | PSDMR_OP_NORM | PSDMR_RFEN;
	*ramaddr = c;
    }

    /* return total ram size */
    return (sdram_size * chipselects);
}

/*-----------------------------------------------------------------------
 * Board Control Functions
 */
void board_poweroff (void)
{
    while (1);		/* hang forever */
}


#ifdef CONFIG_MISC_INIT_R
/* ------------------------------------------------------------------------- */
int misc_init_r(void)
{
    /*
     * Note: iop is used by the I2C macros, and iopa by the ADC/DAC initialization.
     */
    volatile ioport_t *iopa = ioport_addr((immap_t *)CFG_IMMR, 0 /* port A */);
    volatile ioport_t *iop  = ioport_addr((immap_t *)CFG_IMMR, I2C_PORT);

    int  reg;          /* I2C register value */
    char *ep;          /* Environment pointer */
    char str_buf[12] ; /* sprintf output buffer */
    int  sample_rate;  /* ADC/DAC sample rate */
    int  sample_64x;   /* Use  64/4 clocking for the ADC/DAC */
    int  sample_128x;  /* Use 128/4 clocking for the ADC/DAC */
    int  right_just;   /* Is the data to the DAC right justified? */
    int  mclk_divide;  /* MCLK Divide */
    int  quiet;        /* Quiet or minimal output mode */

    quiet = 0;
    if ((ep = getenv("quiet")) != NULL) {
	quiet = simple_strtol(ep, NULL, 10);
    }
    else {
	setenv("quiet", "0");
    }

    /*
     * SACSng custom initialization:
     *    Start the ADC and DAC clocks, since the Crystal parts do not
     *    work on the I2C bus until the clocks are running.
     */

    sample_rate = INITIAL_SAMPLE_RATE;
    if ((ep = getenv("DaqSampleRate")) != NULL) {
	sample_rate = simple_strtol(ep, NULL, 10);
    }

    sample_64x  = INITIAL_SAMPLE_64X;
    sample_128x = INITIAL_SAMPLE_128X;
    if ((ep = getenv("Daq64xSampling")) != NULL) {
	sample_64x = simple_strtol(ep, NULL, 10);
	if (sample_64x) {
	    sample_128x = 0;
	}
	else {
	    sample_128x = 1;
	}
    }
    else {
	if ((ep = getenv("Daq128xSampling")) != NULL) {
	    sample_128x = simple_strtol(ep, NULL, 10);
	    if (sample_128x) {
		sample_64x = 0;
	    }
	    else {
		sample_64x = 1;
	    }
	}
    }

    /*
     * Stop the clocks and wait for at least 1 LRCLK period
     * to make sure the clocking has really stopped.
     */
    Daq_Stop_Clocks();
    udelay((1000000 / sample_rate) * NUM_LRCLKS_TO_STABILIZE);

    /*
     * Initialize the clocks with the new rates
     */
    Daq_Init_Clocks(sample_rate, sample_64x);
    sample_rate = Daq_Get_SampleRate();

    /*
     * Start the clocks and wait for at least 1 LRCLK period
     * to make sure the clocking has become stable.
     */
    Daq_Start_Clocks(sample_rate);
    udelay((1000000 / sample_rate) * NUM_LRCLKS_TO_STABILIZE);

    sprintf(str_buf, "%d", sample_rate);
    setenv("DaqSampleRate", str_buf);

    if (sample_64x) {
	setenv("Daq64xSampling",  "1");
	setenv("Daq128xSampling", NULL);
    }
    else {
	setenv("Daq64xSampling",  NULL);
	setenv("Daq128xSampling", "1");
    }

    /*
     * Display the ADC/DAC clocking information
     */
    if (!quiet) {
	Daq_Display_Clocks();
    }

    /*
     * Determine the DAC data justification
     */

    right_just = INITIAL_RIGHT_JUST;
    if ((ep = getenv("DaqDACRightJustified")) != NULL) {
	right_just = simple_strtol(ep, NULL, 10);
    }

    sprintf(str_buf, "%d", right_just);
    setenv("DaqDACRightJustified", str_buf);

    /*
     * Determine the DAC MCLK Divide
     */

    mclk_divide = INITIAL_MCLK_DIVIDE;
    if ((ep = getenv("DaqDACMClockDivide")) != NULL) {
	mclk_divide = simple_strtol(ep, NULL, 10);
    }

    sprintf(str_buf, "%d", mclk_divide);
    setenv("DaqDACMClockDivide", str_buf);

    /*
     * Initializing the I2C address in the Crystal A/Ds:
     *
     * 1) Wait for VREF cap to settle (10uSec per uF)
     * 2) Release pullup on SDATA
     * 3) Write the I2C address to register 6
     * 4) Enable address matching by setting the MSB in register 7
     */

    if (!quiet) {
	printf("Initializing the ADC...\n");
    }
    udelay(ADC_INITIAL_DELAY);		/* 10uSec per uF of VREF cap */

    iopa->pdat &= ~ADC_SDATA1_MASK;     /* release SDATA1 */
    udelay(ADC_SDATA_DELAY);		/* arbitrary settling time */

    i2c_reg_write(0x00, 0x06, I2C_ADC_1_ADDR);	/* set address */
    i2c_reg_write(I2C_ADC_1_ADDR, 0x07,         /* turn on ADDREN */
		  ADC_REG7_ADDR_ENABLE);

    i2c_reg_write(I2C_ADC_1_ADDR, 0x02, /* 128x, slave mode, !HPEN */
		  (sample_64x ? 0 : ADC_REG2_128x) |
		  ADC_REG2_HIGH_PASS_DIS |
		  ADC_REG2_SLAVE_MODE);

    reg = i2c_reg_read(I2C_ADC_1_ADDR, 0x06) & 0x7F;
    if(reg != I2C_ADC_1_ADDR)
	printf("Init of ADC U10 failed: address is 0x%02X should be 0x%02X\n",
	       reg, I2C_ADC_1_ADDR);

    iopa->pdat &= ~ADC_SDATA2_MASK;	/* release SDATA2 */
    udelay(ADC_SDATA_DELAY);		/* arbitrary settling time */

    i2c_reg_write(0x00, 0x06, I2C_ADC_2_ADDR);	/* set address (do not set ADDREN yet) */

    i2c_reg_write(I2C_ADC_2_ADDR, 0x02, /* 64x, slave mode, !HPEN */
		  (sample_64x ? 0 : ADC_REG2_128x) |
		  ADC_REG2_HIGH_PASS_DIS |
		  ADC_REG2_SLAVE_MODE);

    reg = i2c_reg_read(I2C_ADC_2_ADDR, 0x06) & 0x7F;
    if(reg != I2C_ADC_2_ADDR)
	printf("Init of ADC U15 failed: address is 0x%02X should be 0x%02X\n",
	       reg, I2C_ADC_2_ADDR);

    i2c_reg_write(I2C_ADC_1_ADDR, 0x01, /* set FSTART and GNDCAL */
		  ADC_REG1_FRAME_START |
		  ADC_REG1_GROUND_CAL);

    i2c_reg_write(I2C_ADC_1_ADDR, 0x02, /* Start calibration */
		  (sample_64x ? 0 : ADC_REG2_128x) |
		  ADC_REG2_CAL |
		  ADC_REG2_HIGH_PASS_DIS |
		  ADC_REG2_SLAVE_MODE);

    udelay(ADC_CAL_DELAY);		/* a minimum of 4100 LRCLKs */
    i2c_reg_write(I2C_ADC_1_ADDR, 0x01, 0x00);	/* remove GNDCAL */

    /*
     * Now that we have synchronized the ADC's, enable address
     * selection on the second ADC as well as the first.
     */
    i2c_reg_write(I2C_ADC_2_ADDR, 0x07, ADC_REG7_ADDR_ENABLE);

    /*
     * Initialize the Crystal DAC
     *
     * Two of the config lines are used for I2C so we have to set them
     * to the proper initialization state without inadvertantly
     * sending an I2C "start" sequence.  When we bring the I2C back to
     * the normal state, we send an I2C "stop" sequence.
     */
    if (!quiet) {
	printf("Initializing the DAC...\n");
    }

    /*
     * Bring the I2C clock and data lines low for initialization
     */
    I2C_SCL(0);
    I2C_DELAY;
    I2C_SDA(0);
    I2C_ACTIVE;
    I2C_DELAY;

    /* Reset the DAC */
    iopa->pdat &= ~DAC_RST_MASK;
    udelay(DAC_RESET_DELAY);

    /* Release the DAC reset */
    iopa->pdat |=  DAC_RST_MASK;
    udelay(DAC_INITIAL_DELAY);

    /*
     * Cause the DAC to:
     *     Enable control port (I2C mode)
     *     Going into power down
     */
    i2c_reg_write(I2C_DAC_ADDR, 0x05,
		  DAC_REG5_I2C_MODE |
		  DAC_REG5_POWER_DOWN);

    /*
     * Cause the DAC to:
     *     Enable control port (I2C mode)
     *     Going into power down
     *         . MCLK divide by 1
     *         . MCLK divide by 2
     */
    i2c_reg_write(I2C_DAC_ADDR, 0x05,
		  DAC_REG5_I2C_MODE |
		  DAC_REG5_POWER_DOWN |
		  (mclk_divide ? DAC_REG5_MCLK_DIV : 0));

    /*
     * Cause the DAC to:
     *     Auto-mute disabled
     *         . Format 0, left  justified 24 bits
     *         . Format 3, right justified 24 bits
     *     No de-emphasis
     *         . Single speed mode
     *         . Double speed mode
     */
    i2c_reg_write(I2C_DAC_ADDR, 0x01,
		  (right_just ? DAC_REG1_RIGHT_JUST_24BIT :
				DAC_REG1_LEFT_JUST_24_BIT) |
		  DAC_REG1_DEM_NO |
		  (sample_rate >= 50000 ? DAC_REG1_DOUBLE : DAC_REG1_SINGLE));

    sprintf(str_buf, "%d",
	    sample_rate >= 50000 ? DAC_REG1_DOUBLE : DAC_REG1_SINGLE);
    setenv("DaqDACFunctionalMode", str_buf);

    /*
     * Cause the DAC to:
     *     Enable control port (I2C mode)
     *     Remove power down
     *         . MCLK divide by 1
     *         . MCLK divide by 2
     */
    i2c_reg_write(I2C_DAC_ADDR, 0x05,
		  DAC_REG5_I2C_MODE |
		  (mclk_divide ? DAC_REG5_MCLK_DIV : 0));

    /*
     * Create a I2C stop condition:
     *     low->high on data while clock is high.
     */
    I2C_SCL(1);
    I2C_DELAY;
    I2C_SDA(1);
    I2C_DELAY;
    I2C_TRISTATE;

    if (!quiet) {
	printf("\n");
    }

#ifdef CONFIG_ETHER_LOOPBACK_TEST
    /*
     * Run the Ethernet loopback test
     */
    eth_loopback_test ();
#endif /* CONFIG_ETHER_LOOPBACK_TEST */

#ifdef CONFIG_SHOW_BOOT_PROGRESS
    /*
     * Turn off the RED fail LED now that we are up and running.
     */
    status_led_set(STATUS_LED_RED, STATUS_LED_OFF);
#endif

    return 0;
}

#ifdef CONFIG_SHOW_BOOT_PROGRESS
/*
 * Show boot status: flash the LED if something goes wrong, indicating
 * that last thing that worked and thus, by implication, what is broken.
 *
 * This stores the last OK value in RAM so this will not work properly
 * before RAM is initialized.  Since it is being used for indicating
 * boot status (i.e. after RAM is initialized), that is OK.
 */
static void flash_code(uchar number, uchar modulo, uchar digits)
{
    int   j;

    /*
     * Recursively do upper digits.
     */
    if(digits > 1) {
	flash_code(number / modulo, modulo, digits - 1);
    }

    number = number % modulo;

    /*
     * Zero is indicated by one long flash (dash).
     */
    if(number == 0) {
	status_led_set(STATUS_LED_BOOT, STATUS_LED_ON);
	udelay(1000000);
	status_led_set(STATUS_LED_BOOT, STATUS_LED_OFF);
	udelay(200000);
    } else {
	/*
	 * Non-zero is indicated by short flashes, one per count.
	 */
	for(j = 0; j < number; j++) {
	    status_led_set(STATUS_LED_BOOT, STATUS_LED_ON);
	    udelay(100000);
	    status_led_set(STATUS_LED_BOOT, STATUS_LED_OFF);
	    udelay(200000);
	}
    }
    /*
     * Inter-digit pause: we've already waited 200 mSec, wait 1 sec total
     */
    udelay(700000);
}

static int last_boot_progress;

void show_boot_progress (int status)
{
    int i,j;
    if(status > 0) {
	last_boot_progress = status;
    } else {
	/*
	 * If a specific failure code is given, flash this code
	 * else just use the last success code we've seen
	 */
	if(status < -1)
	    last_boot_progress = -status;

	/*
	 * Flash this code 5 times
	 */
	for(j=0; j<5; j++) {
	    /*
	     * Houston, we have a problem.
	     * Blink the last OK status which indicates where things failed.
	     */
	    status_led_set(STATUS_LED_RED, STATUS_LED_ON);
	    flash_code(last_boot_progress, 5, 3);

	    /*
	     * Delay 5 seconds between repetitions,
	     * with the fault LED blinking
	     */
	    for(i=0; i<5; i++) {
		status_led_set(STATUS_LED_RED, STATUS_LED_OFF);
		udelay(500000);
		status_led_set(STATUS_LED_RED, STATUS_LED_ON);
		udelay(500000);
	    }
	}

	/*
	 * Reset the board to retry initialization.
	 */
	do_reset (NULL, 0, 0, NULL);
    }
}
#endif /* CONFIG_SHOW_BOOT_PROGRESS */


/*
 * The following are used to control the SPI chip selects for the SPI command.
 */
#if (CONFIG_COMMANDS & CFG_CMD_SPI) || defined(CONFIG_CMD_SPI)

#define SPI_ADC_CS_MASK	0x00000800
#define SPI_DAC_CS_MASK	0x00001000

void spi_adc_chipsel(int cs)
{
    volatile ioport_t *iopd = ioport_addr((immap_t *)CFG_IMMR, 3 /* port D */);

    if(cs)
	iopd->pdat &= ~SPI_ADC_CS_MASK;	/* activate the chip select */
    else
	iopd->pdat |=  SPI_ADC_CS_MASK;	/* deactivate the chip select */
}

void spi_dac_chipsel(int cs)
{
    volatile ioport_t *iopd = ioport_addr((immap_t *)CFG_IMMR, 3 /* port D */);

    if(cs)
	iopd->pdat &= ~SPI_DAC_CS_MASK;	/* activate the chip select */
    else
	iopd->pdat |=  SPI_DAC_CS_MASK;	/* deactivate the chip select */
}

/*
 * The SPI command uses this table of functions for controlling the SPI
 * chip selects: it calls the appropriate function to control the SPI
 * chip selects.
 */
spi_chipsel_type spi_chipsel[] = {
	spi_adc_chipsel,
	spi_dac_chipsel
};
int spi_chipsel_cnt = sizeof(spi_chipsel) / sizeof(spi_chipsel[0]);

#endif /* CFG_CMD_SPI */

#endif /* CONFIG_MISC_INIT_R */

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	return 0;	/* No hotkeys supported */
}

#endif
