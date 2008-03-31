/*
 * cpu/ppc4xx/denali_spd_ddr2.c
 * This SPD SDRAM detection code supports AMCC PPC44x CPUs with a Denali-core
 * DDR2 controller, specifically the 440EPx/GRx.
 *
 * (C) Copyright 2007-2008
 * Larry Johnson, lrj@acm.org.
 *
 * Based primarily on cpu/ppc4xx/4xx_spd_ddr2.c, which is...
 *
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * COPYRIGHT   AMCC   CORPORATION 2004
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
 *
 */

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <command.h>
#include <ppc4xx.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/mmu.h>

#if defined(CONFIG_SPD_EEPROM) &&				\
	(defined(CONFIG_440EPX) || defined(CONFIG_440GRX))

/*-----------------------------------------------------------------------------+
 * Defines
 *-----------------------------------------------------------------------------*/
#ifndef	TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

#define MAXDIMMS	2
#define MAXRANKS	2

#define ONE_BILLION	1000000000

#define MULDIV64(m1, m2, d)	(u32)(((u64)(m1) * (u64)(m2)) / (u64)(d))

#define DLL_DQS_DELAY	0x19
#define DLL_DQS_BYPASS	0x0B
#define DQS_OUT_SHIFT	0x7F

/*
 * This DDR2 setup code can dynamically setup the TLB entries for the DDR2 memory
 * region. Right now the cache should still be disabled in U-Boot because of the
 * EMAC driver, that need it's buffer descriptor to be located in non cached
 * memory.
 *
 * If at some time this restriction doesn't apply anymore, just define
 * CONFIG_4xx_DCACHE in the board config file and this code should setup
 * everything correctly.
 */
#if defined(CONFIG_4xx_DCACHE)
#define MY_TLB_WORD2_I_ENABLE	0			/* enable caching on SDRAM */
#else
#define MY_TLB_WORD2_I_ENABLE	TLB_WORD2_I_ENABLE	/* disable caching on SDRAM */
#endif

/*-----------------------------------------------------------------------------+
 * Prototypes
 *-----------------------------------------------------------------------------*/
extern int denali_wait_for_dlllock(void);
extern void denali_core_search_data_eye(void);
extern void dcbz_area(u32 start_address, u32 num_bytes);
extern void dflush(void);

/*
 * Board-specific Platform code can reimplement spd_ddr_init_hang () if needed
 */
void __spd_ddr_init_hang(void)
{
	hang();
}
void spd_ddr_init_hang(void)
    __attribute__ ((weak, alias("__spd_ddr_init_hang")));

#if defined(DEBUG)
static void print_mcsr(void)
{
	printf("MCSR = 0x%08X\n", mfspr(SPRN_MCSR));
}

static void denali_sdram_register_dump(void)
{
	unsigned int sdram_data;

	printf("\n  Register Dump:\n");
	mfsdram(DDR0_00, sdram_data);
	printf("        DDR0_00 = 0x%08X", sdram_data);
	mfsdram(DDR0_01, sdram_data);
	printf("        DDR0_01 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_02, sdram_data);
	printf("        DDR0_02 = 0x%08X", sdram_data);
	mfsdram(DDR0_03, sdram_data);
	printf("        DDR0_03 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_04, sdram_data);
	printf("        DDR0_04 = 0x%08X", sdram_data);
	mfsdram(DDR0_05, sdram_data);
	printf("        DDR0_05 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_06, sdram_data);
	printf("        DDR0_06 = 0x%08X", sdram_data);
	mfsdram(DDR0_07, sdram_data);
	printf("        DDR0_07 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_08, sdram_data);
	printf("        DDR0_08 = 0x%08X", sdram_data);
	mfsdram(DDR0_09, sdram_data);
	printf("        DDR0_09 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_10, sdram_data);
	printf("        DDR0_10 = 0x%08X", sdram_data);
	mfsdram(DDR0_11, sdram_data);
	printf("        DDR0_11 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_12, sdram_data);
	printf("        DDR0_12 = 0x%08X", sdram_data);
	mfsdram(DDR0_14, sdram_data);
	printf("        DDR0_14 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_17, sdram_data);
	printf("        DDR0_17 = 0x%08X", sdram_data);
	mfsdram(DDR0_18, sdram_data);
	printf("        DDR0_18 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_19, sdram_data);
	printf("        DDR0_19 = 0x%08X", sdram_data);
	mfsdram(DDR0_20, sdram_data);
	printf("        DDR0_20 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_21, sdram_data);
	printf("        DDR0_21 = 0x%08X", sdram_data);
	mfsdram(DDR0_22, sdram_data);
	printf("        DDR0_22 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_23, sdram_data);
	printf("        DDR0_23 = 0x%08X", sdram_data);
	mfsdram(DDR0_24, sdram_data);
	printf("        DDR0_24 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_25, sdram_data);
	printf("        DDR0_25 = 0x%08X", sdram_data);
	mfsdram(DDR0_26, sdram_data);
	printf("        DDR0_26 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_27, sdram_data);
	printf("        DDR0_27 = 0x%08X", sdram_data);
	mfsdram(DDR0_28, sdram_data);
	printf("        DDR0_28 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_31, sdram_data);
	printf("        DDR0_31 = 0x%08X", sdram_data);
	mfsdram(DDR0_32, sdram_data);
	printf("        DDR0_32 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_33, sdram_data);
	printf("        DDR0_33 = 0x%08X", sdram_data);
	mfsdram(DDR0_34, sdram_data);
	printf("        DDR0_34 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_35, sdram_data);
	printf("        DDR0_35 = 0x%08X", sdram_data);
	mfsdram(DDR0_36, sdram_data);
	printf("        DDR0_36 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_37, sdram_data);
	printf("        DDR0_37 = 0x%08X", sdram_data);
	mfsdram(DDR0_38, sdram_data);
	printf("        DDR0_38 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_39, sdram_data);
	printf("        DDR0_39 = 0x%08X", sdram_data);
	mfsdram(DDR0_40, sdram_data);
	printf("        DDR0_40 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_41, sdram_data);
	printf("        DDR0_41 = 0x%08X", sdram_data);
	mfsdram(DDR0_42, sdram_data);
	printf("        DDR0_42 = 0x%08X\n", sdram_data);
	mfsdram(DDR0_43, sdram_data);
	printf("        DDR0_43 = 0x%08X", sdram_data);
	mfsdram(DDR0_44, sdram_data);
	printf("        DDR0_44 = 0x%08X\n", sdram_data);
}
#else
static inline void denali_sdram_register_dump(void)
{
}

inline static void print_mcsr(void)
{
}
#endif /* defined(DEBUG) */

static int is_ecc_enabled(void)
{
	u32 val;

	mfsdram(DDR0_22, val);
	return 0x3 == DDR0_22_CTRL_RAW_DECODE(val);
}

static unsigned char spd_read(u8 chip, unsigned int addr)
{
	u8 data[2];

	if (0 != i2c_probe(chip) || 0 != i2c_read(chip, addr, 1, data, 1)) {
		debug("spd_read(0x%02X, 0x%02X) failed\n", chip, addr);
		return 0;
	}
	debug("spd_read(0x%02X, 0x%02X) returned 0x%02X\n",
	      chip, addr, data[0]);
	return data[0];
}

static unsigned long get_tcyc(unsigned char reg)
{
	/*
	 * Byte 9, et al: Cycle time for CAS Latency=X, is split into two
	 * nibbles: the higher order nibble (bits 4-7) designates the cycle time
	 * to a granularity of 1ns; the value presented by the lower order
	 * nibble (bits 0-3) has a granularity of .1ns and is added to the value
	 * designated by the higher nibble. In addition, four lines of the lower
	 * order nibble are assigned to support +.25, +.33, +.66, and +.75.
	 */

	unsigned char subfield_b = reg & 0x0F;

	switch (subfield_b & 0x0F) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
	case 0x8:
	case 0x9:
		return 1000 * (reg >> 4) + 100 * subfield_b;
	case 0xA:
		return 1000 * (reg >> 4) + 250;
	case 0xB:
		return 1000 * (reg >> 4) + 333;
	case 0xC:
		return 1000 * (reg >> 4) + 667;
	case 0xD:
		return 1000 * (reg >> 4) + 750;
	}
	return 0;
}

/*------------------------------------------------------------------
 * Find the installed DIMMs, make sure that the are DDR2, and fill
 * in the dimm_ranks array.  Then dimm_ranks[dimm_num] > 0 iff the
 * DIMM and dimm_num is present.
 * Note: Because there are only two chip-select lines, it is assumed
 * that a board with a single socket can support two ranks on that
 * socket, while a board with two sockets can support only one rank
 * on each socket.
 *-----------------------------------------------------------------*/
static void get_spd_info(unsigned long dimm_ranks[],
			 unsigned long *ranks,
			 unsigned char const iic0_dimm_addr[],
			 unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long dimm_found = FALSE;
	unsigned long const max_ranks_per_dimm = (1 == num_dimm_banks) ? 2 : 1;
	unsigned char num_of_bytes;
	unsigned char total_size;

	*ranks = 0;
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		num_of_bytes = 0;
		total_size = 0;

		num_of_bytes = spd_read(iic0_dimm_addr[dimm_num], 0);
		total_size = spd_read(iic0_dimm_addr[dimm_num], 1);
		if ((num_of_bytes != 0) && (total_size != 0)) {
			unsigned char const dimm_type =
			    spd_read(iic0_dimm_addr[dimm_num], 2);

			unsigned long ranks_on_dimm =
			    (spd_read(iic0_dimm_addr[dimm_num], 5) & 0x07) + 1;

			if (8 != dimm_type) {
				switch (dimm_type) {
				case 1:
					printf("ERROR: Standard Fast Page Mode "
					       "DRAM DIMM");
					break;
				case 2:
					printf("ERROR: EDO DIMM");
					break;
				case 3:
					printf("ERROR: Pipelined Nibble DIMM");
					break;
				case 4:
					printf("ERROR: SDRAM DIMM");
					break;
				case 5:
					printf("ERROR: Multiplexed ROM DIMM");
					break;
				case 6:
					printf("ERROR: SGRAM DIMM");
					break;
				case 7:
					printf("ERROR: DDR1 DIMM");
					break;
				default:
					printf("ERROR: Unknown DIMM (type %d)",
					       (unsigned int)dimm_type);
					break;
				}
				printf(" detected in slot %lu.\n", dimm_num);
				printf("Only DDR2 SDRAM DIMMs are supported."
				       "\n");
				printf("Replace the module with a DDR2 DIMM."
				       "\n\n");
				spd_ddr_init_hang();
			}
			dimm_found = TRUE;
			debug("DIMM slot %lu: populated with %lu-rank DDR2 DIMM"
			      "\n", dimm_num, ranks_on_dimm);
			if (ranks_on_dimm > max_ranks_per_dimm) {
				printf("WARNING: DRAM DIMM in slot %lu has %lu "
				       "ranks.\n");
				if (1 == max_ranks_per_dimm) {
					printf("Only one rank will be used.\n");
				} else {
					printf
					    ("Only two ranks will be used.\n");
				}
				ranks_on_dimm = max_ranks_per_dimm;
			}
			dimm_ranks[dimm_num] = ranks_on_dimm;
			*ranks += ranks_on_dimm;
		} else {
			dimm_ranks[dimm_num] = 0;
			debug("DIMM slot %lu: Not populated\n", dimm_num);
		}
	}
	if (dimm_found == FALSE) {
		printf("ERROR: No memory installed.\n");
		printf("Install at least one DDR2 DIMM.\n\n");
		spd_ddr_init_hang();
	}
	debug("Total number of ranks = %d\n", *ranks);
}

/*------------------------------------------------------------------
 * For the memory DIMMs installed, this routine verifies that
 * frequency previously calculated is supported.
 *-----------------------------------------------------------------*/
static void check_frequency(unsigned long *dimm_ranks,
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks,
			    unsigned long sdram_freq)
{
	unsigned long dimm_num;
	unsigned long cycle_time;
	unsigned long calc_cycle_time;

	/*
	 * calc_cycle_time is calculated from DDR frequency set by board/chip
	 * and is expressed in picoseconds to match the way DIMM cycle time is
	 * calculated below.
	 */
	calc_cycle_time = MULDIV64(ONE_BILLION, 1000, sdram_freq);

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_ranks[dimm_num]) {
			cycle_time =
			    get_tcyc(spd_read(iic0_dimm_addr[dimm_num], 9));
			debug("cycle_time=%d ps\n", cycle_time);

			if (cycle_time > (calc_cycle_time + 10)) {
				/*
				 * the provided sdram cycle_time is too small
				 * for the available DIMM cycle_time. The
				 * additionnal 10ps is here to accept a small
				 * incertainty.
				 */
				printf
				    ("ERROR: DRAM DIMM detected with cycle_time %d ps in "
				     "slot %d \n while calculated cycle time is %d ps.\n",
				     (unsigned int)cycle_time,
				     (unsigned int)dimm_num,
				     (unsigned int)calc_cycle_time);
				printf
				    ("Replace the DIMM, or change DDR frequency via "
				     "strapping bits.\n\n");
				spd_ddr_init_hang();
			}
		}
	}
}

/*------------------------------------------------------------------
 * This routine gets size information for the installed memory
 * DIMMs.
 *-----------------------------------------------------------------*/
static void get_dimm_size(unsigned long dimm_ranks[],
			  unsigned char const iic0_dimm_addr[],
			  unsigned long num_dimm_banks,
			  unsigned long *const rows,
			  unsigned long *const banks,
			  unsigned long *const cols, unsigned long *const width)
{
	unsigned long dimm_num;

	*rows = 0;
	*banks = 0;
	*cols = 0;
	*width = 0;
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_ranks[dimm_num]) {
			unsigned long t;

			/* Rows */
			t = spd_read(iic0_dimm_addr[dimm_num], 3);
			if (0 == *rows) {
				*rows = t;
			} else if (t != *rows) {
				printf("ERROR: DRAM DIMM modules do not all "
				       "have the same number of rows.\n\n");
				spd_ddr_init_hang();
			}
			/* Banks */
			t = spd_read(iic0_dimm_addr[dimm_num], 17);
			if (0 == *banks) {
				*banks = t;
			} else if (t != *banks) {
				printf("ERROR: DRAM DIMM modules do not all "
				       "have the same number of banks.\n\n");
				spd_ddr_init_hang();
			}
			/* Columns */
			t = spd_read(iic0_dimm_addr[dimm_num], 4);
			if (0 == *cols) {
				*cols = t;
			} else if (t != *cols) {
				printf("ERROR: DRAM DIMM modules do not all "
				       "have the same number of columns.\n\n");
				spd_ddr_init_hang();
			}
			/* Data width */
			t = spd_read(iic0_dimm_addr[dimm_num], 6);
			if (0 == *width) {
				*width = t;
			} else if (t != *width) {
				printf("ERROR: DRAM DIMM modules do not all "
				       "have the same data width.\n\n");
				spd_ddr_init_hang();
			}
		}
	}
	debug("Number of rows = %d\n", *rows);
	debug("Number of columns = %d\n", *cols);
	debug("Number of banks = %d\n", *banks);
	debug("Data width = %d\n", *width);
	if (*rows > 14) {
		printf("ERROR: DRAM DIMM modules have %lu address rows.\n",
		       *rows);
		printf("Only modules with 14 or fewer rows are supported.\n\n");
		spd_ddr_init_hang();
	}
	if (4 != *banks && 8 != *banks) {
		printf("ERROR: DRAM DIMM modules have %lu banks.\n", *banks);
		printf("Only modules with 4 or 8 banks are supported.\n\n");
		spd_ddr_init_hang();
	}
	if (*cols > 12) {
		printf("ERROR: DRAM DIMM modules have %lu address columns.\n",
		       *cols);
		printf("Only modules with 12 or fewer columns are "
		       "supported.\n\n");
		spd_ddr_init_hang();
	}
	if (32 != *width && 40 != *width && 64 != *width && 72 != *width) {
		printf("ERROR: DRAM DIMM modules have a width of %lu bit.\n",
		       *width);
		printf("Only modules with widths of 32, 40, 64, and 72 bits "
		       "are supported.\n\n");
		spd_ddr_init_hang();
	}
}

/*------------------------------------------------------------------
 * Only 1.8V modules are supported.  This routine verifies this.
 *-----------------------------------------------------------------*/
static void check_voltage_type(unsigned long dimm_ranks[],
			       unsigned char const iic0_dimm_addr[],
			       unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long voltage_type;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_ranks[dimm_num]) {
			voltage_type = spd_read(iic0_dimm_addr[dimm_num], 8);
			if (0x05 != voltage_type) {	/* 1.8V for DDR2 */
				printf("ERROR: Slot %lu provides 1.8V for DDR2 "
				       "DIMMs.\n", dimm_num);
				switch (voltage_type) {
				case 0x00:
					printf("This DIMM is 5.0 Volt/TTL.\n");
					break;
				case 0x01:
					printf("This DIMM is LVTTL.\n");
					break;
				case 0x02:
					printf("This DIMM is 1.5 Volt.\n");
					break;
				case 0x03:
					printf("This DIMM is 3.3 Volt/TTL.\n");
					break;
				case 0x04:
					printf("This DIMM is 2.5 Volt.\n");
					break;
				default:
					printf("This DIMM is an unknown "
					       "voltage.\n");
					break;
				}
				printf("Replace it with a 1.8V DDR2 DIMM.\n\n");
				spd_ddr_init_hang();
			}
		}
	}
}

static void program_ddr0_03(unsigned long dimm_ranks[],
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks,
			    unsigned long sdram_freq,
			    unsigned long rows, unsigned long *cas_latency)
{
	unsigned long dimm_num;
	unsigned long cas_index;
	unsigned long cycle_2_0_clk;
	unsigned long cycle_3_0_clk;
	unsigned long cycle_4_0_clk;
	unsigned long cycle_5_0_clk;
	unsigned long max_2_0_tcyc_ps = 100;
	unsigned long max_3_0_tcyc_ps = 100;
	unsigned long max_4_0_tcyc_ps = 100;
	unsigned long max_5_0_tcyc_ps = 100;
	unsigned char cas_available = 0x3C;	/* value for DDR2 */
	u32 ddr0_03 = DDR0_03_BSTLEN_ENCODE(0x2) | DDR0_03_INITAREF_ENCODE(0x2);
	unsigned int const tcyc_addr[3] = { 9, 23, 25 };

	/*------------------------------------------------------------------
	 * Get the board configuration info.
	 *-----------------------------------------------------------------*/
	debug("sdram_freq = %d\n", sdram_freq);

	/*------------------------------------------------------------------
	 * Handle the timing.  We need to find the worst case timing of all
	 * the dimm modules installed.
	 *-----------------------------------------------------------------*/
	/* loop through all the DIMM slots on the board */
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		/* If a dimm is installed in a particular slot ... */
		if (dimm_ranks[dimm_num]) {
			unsigned char const cas_bit =
			    spd_read(iic0_dimm_addr[dimm_num], 18);
			unsigned char cas_mask;

			cas_available &= cas_bit;
			for (cas_mask = 0x80; cas_mask; cas_mask >>= 1) {
				if (cas_bit & cas_mask)
					break;
			}
			debug("cas_bit (SPD byte 18) = %02X, cas_mask = %02X\n",
			      cas_bit, cas_mask);

			for (cas_index = 0; cas_index < 3;
			     cas_mask >>= 1, cas_index++) {
				unsigned long cycle_time_ps;

				if (!(cas_available & cas_mask)) {
					continue;
				}
				cycle_time_ps =
				    get_tcyc(spd_read(iic0_dimm_addr[dimm_num],
						      tcyc_addr[cas_index]));

				debug("cas_index = %d: cycle_time_ps = %d\n",
				      cas_index, cycle_time_ps);
				/*
				 * DDR2 devices use the following bitmask for CAS latency:
				 *  Bit   7    6    5    4    3    2    1    0
				 *       TBD  6.0  5.0  4.0  3.0  2.0  TBD  TBD
				 */
				switch (cas_mask) {
				case 0x20:
					max_5_0_tcyc_ps =
					    max(max_5_0_tcyc_ps, cycle_time_ps);
					break;
				case 0x10:
					max_4_0_tcyc_ps =
					    max(max_4_0_tcyc_ps, cycle_time_ps);
					break;
				case 0x08:
					max_3_0_tcyc_ps =
					    max(max_3_0_tcyc_ps, cycle_time_ps);
					break;
				case 0x04:
					max_2_0_tcyc_ps =
					    max(max_2_0_tcyc_ps, cycle_time_ps);
					break;
				}
			}
		}
	}
	debug("cas_available (bit map) = 0x%02X\n", cas_available);

	/*------------------------------------------------------------------
	 * Set the SDRAM mode, SDRAM_MMODE
	 *-----------------------------------------------------------------*/

	/* add 10 here because of rounding problems */
	cycle_2_0_clk = MULDIV64(ONE_BILLION, 1000, max_2_0_tcyc_ps) + 10;
	cycle_3_0_clk = MULDIV64(ONE_BILLION, 1000, max_3_0_tcyc_ps) + 10;
	cycle_4_0_clk = MULDIV64(ONE_BILLION, 1000, max_4_0_tcyc_ps) + 10;
	cycle_5_0_clk = MULDIV64(ONE_BILLION, 1000, max_5_0_tcyc_ps) + 10;
	debug("cycle_2_0_clk = %d\n", cycle_2_0_clk);
	debug("cycle_3_0_clk = %d\n", cycle_3_0_clk);
	debug("cycle_4_0_clk = %d\n", cycle_4_0_clk);
	debug("cycle_5_0_clk = %d\n", cycle_5_0_clk);

	if ((cas_available & 0x04) && (sdram_freq <= cycle_2_0_clk)) {
		*cas_latency = 2;
		ddr0_03 |= DDR0_03_CASLAT_ENCODE(0x2) |
		    DDR0_03_CASLAT_LIN_ENCODE(0x4);
	} else if ((cas_available & 0x08) && (sdram_freq <= cycle_3_0_clk)) {
		*cas_latency = 3;
		ddr0_03 |= DDR0_03_CASLAT_ENCODE(0x3) |
		    DDR0_03_CASLAT_LIN_ENCODE(0x6);
	} else if ((cas_available & 0x10) && (sdram_freq <= cycle_4_0_clk)) {
		*cas_latency = 4;
		ddr0_03 |= DDR0_03_CASLAT_ENCODE(0x4) |
		    DDR0_03_CASLAT_LIN_ENCODE(0x8);
	} else if ((cas_available & 0x20) && (sdram_freq <= cycle_5_0_clk)) {
		*cas_latency = 5;
		ddr0_03 |= DDR0_03_CASLAT_ENCODE(0x5) |
		    DDR0_03_CASLAT_LIN_ENCODE(0xA);
	} else {
		printf("ERROR: Cannot find a supported CAS latency with the "
		       "installed DIMMs.\n");
		printf("Only DDR2 DIMMs with CAS latencies of 2.0, 3.0, 4.0, "
		       "and 5.0 are supported.\n");
		printf("Make sure the PLB speed is within the supported range "
		       "of the DIMMs.\n");
		printf("sdram_freq=%d cycle2=%d cycle3=%d cycle4=%d "
		       "cycle5=%d\n\n", sdram_freq, cycle_2_0_clk,
		       cycle_3_0_clk, cycle_4_0_clk, cycle_5_0_clk);
		spd_ddr_init_hang();
	}
	debug("CAS latency = %d\n", *cas_latency);
	mtsdram(DDR0_03, ddr0_03);
}

static void program_ddr0_04(unsigned long dimm_ranks[],
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks,
			    unsigned long sdram_freq)
{
	unsigned long dimm_num;
	unsigned long t_rc_ps = 0;
	unsigned long t_rrd_ps = 0;
	unsigned long t_rtp_ps = 0;
	unsigned long t_rc_clk;
	unsigned long t_rrd_clk;
	unsigned long t_rtp_clk;

	/*------------------------------------------------------------------
	 * Handle the timing.  We need to find the worst case timing of all
	 * the dimm modules installed.
	 *-----------------------------------------------------------------*/
	/* loop through all the DIMM slots on the board */
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		/* If a dimm is installed in a particular slot ... */
		if (dimm_ranks[dimm_num]) {
			unsigned long ps;

			/* tRC */
			ps = 1000 * spd_read(iic0_dimm_addr[dimm_num], 41);
			switch (spd_read(iic0_dimm_addr[dimm_num], 40) >> 4) {
			case 0x1:
				ps += 250;
				break;
			case 0x2:
				ps += 333;
				break;
			case 0x3:
				ps += 500;
				break;
			case 0x4:
				ps += 667;
				break;
			case 0x5:
				ps += 750;
				break;
			}
			t_rc_ps = max(t_rc_ps, ps);
			/* tRRD */
			ps = 250 * spd_read(iic0_dimm_addr[dimm_num], 28);
			t_rrd_ps = max(t_rrd_ps, ps);
			/* tRTP */
			ps = 250 * spd_read(iic0_dimm_addr[dimm_num], 38);
			t_rtp_ps = max(t_rtp_ps, ps);
		}
	}
	debug("t_rc_ps  = %d\n", t_rc_ps);
	t_rc_clk = (MULDIV64(sdram_freq, t_rc_ps, ONE_BILLION) + 999) / 1000;
	debug("t_rrd_ps = %d\n", t_rrd_ps);
	t_rrd_clk = (MULDIV64(sdram_freq, t_rrd_ps, ONE_BILLION) + 999) / 1000;
	debug("t_rtp_ps = %d\n", t_rtp_ps);
	t_rtp_clk = (MULDIV64(sdram_freq, t_rtp_ps, ONE_BILLION) + 999) / 1000;
	mtsdram(DDR0_04, DDR0_04_TRC_ENCODE(t_rc_clk) |
		DDR0_04_TRRD_ENCODE(t_rrd_clk) |
		DDR0_04_TRTP_ENCODE(t_rtp_clk));
}

static void program_ddr0_05(unsigned long dimm_ranks[],
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks,
			    unsigned long sdram_freq)
{
	unsigned long dimm_num;
	unsigned long t_rp_ps = 0;
	unsigned long t_ras_ps = 0;
	unsigned long t_rp_clk;
	unsigned long t_ras_clk;
	u32 ddr0_05 = DDR0_05_TMRD_ENCODE(0x2) | DDR0_05_TEMRS_ENCODE(0x2);

	/*------------------------------------------------------------------
	 * Handle the timing.  We need to find the worst case timing of all
	 * the dimm modules installed.
	 *-----------------------------------------------------------------*/
	/* loop through all the DIMM slots on the board */
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		/* If a dimm is installed in a particular slot ... */
		if (dimm_ranks[dimm_num]) {
			unsigned long ps;

			/* tRP */
			ps = 250 * spd_read(iic0_dimm_addr[dimm_num], 27);
			t_rp_ps = max(t_rp_ps, ps);
			/* tRAS */
			ps = 1000 * spd_read(iic0_dimm_addr[dimm_num], 30);
			t_ras_ps = max(t_ras_ps, ps);
		}
	}
	debug("t_rp_ps  = %d\n", t_rp_ps);
	t_rp_clk = (MULDIV64(sdram_freq, t_rp_ps, ONE_BILLION) + 999) / 1000;
	debug("t_ras_ps = %d\n", t_ras_ps);
	t_ras_clk = (MULDIV64(sdram_freq, t_ras_ps, ONE_BILLION) + 999) / 1000;
	mtsdram(DDR0_05, ddr0_05 | DDR0_05_TRP_ENCODE(t_rp_clk) |
		DDR0_05_TRAS_MIN_ENCODE(t_ras_clk));
}

static void program_ddr0_06(unsigned long dimm_ranks[],
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks,
			    unsigned long sdram_freq)
{
	unsigned long dimm_num;
	unsigned char spd_40;
	unsigned long t_wtr_ps = 0;
	unsigned long t_rfc_ps = 0;
	unsigned long t_wtr_clk;
	unsigned long t_rfc_clk;
	u32 ddr0_06 =
	    DDR0_06_WRITEINTERP_ENCODE(0x1) | DDR0_06_TDLL_ENCODE(200);

	/*------------------------------------------------------------------
	 * Handle the timing.  We need to find the worst case timing of all
	 * the dimm modules installed.
	 *-----------------------------------------------------------------*/
	/* loop through all the DIMM slots on the board */
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		/* If a dimm is installed in a particular slot ... */
		if (dimm_ranks[dimm_num]) {
			unsigned long ps;

			/* tWTR */
			ps = 250 * spd_read(iic0_dimm_addr[dimm_num], 37);
			t_wtr_ps = max(t_wtr_ps, ps);
			/* tRFC */
			ps = 1000 * spd_read(iic0_dimm_addr[dimm_num], 42);
			spd_40 = spd_read(iic0_dimm_addr[dimm_num], 40);
			ps += 256000 * (spd_40 & 0x01);
			switch ((spd_40 & 0x0E) >> 1) {
			case 0x1:
				ps += 250;
				break;
			case 0x2:
				ps += 333;
				break;
			case 0x3:
				ps += 500;
				break;
			case 0x4:
				ps += 667;
				break;
			case 0x5:
				ps += 750;
				break;
			}
			t_rfc_ps = max(t_rfc_ps, ps);
		}
	}
	debug("t_wtr_ps = %d\n", t_wtr_ps);
	t_wtr_clk = (MULDIV64(sdram_freq, t_wtr_ps, ONE_BILLION) + 999) / 1000;
	debug("t_rfc_ps = %d\n", t_rfc_ps);
	t_rfc_clk = (MULDIV64(sdram_freq, t_rfc_ps, ONE_BILLION) + 999) / 1000;
	mtsdram(DDR0_06, ddr0_06 | DDR0_06_TWTR_ENCODE(t_wtr_clk) |
		DDR0_06_TRFC_ENCODE(t_rfc_clk));
}

static void program_ddr0_10(unsigned long dimm_ranks[], unsigned long ranks)
{
	unsigned long csmap;

	if (2 == ranks) {
		/* Both chip selects in use */
		csmap = 0x03;
	} else {
		/* One chip select in use */
		csmap = (1 == dimm_ranks[0]) ? 0x1 : 0x2;
	}
	mtsdram(DDR0_10, DDR0_10_WRITE_MODEREG_ENCODE(0x0) |
		DDR0_10_CS_MAP_ENCODE(csmap) |
		DDR0_10_OCD_ADJUST_PUP_CS_0_ENCODE(0));
}

static void program_ddr0_11(unsigned long sdram_freq)
{
	unsigned long const t_xsnr_ps = 200000;	/* 200 ns */
	unsigned long t_xsnr_clk;

	debug("t_xsnr_ps = %d\n", t_xsnr_ps);
	t_xsnr_clk =
	    (MULDIV64(sdram_freq, t_xsnr_ps, ONE_BILLION) + 999) / 1000;
	mtsdram(DDR0_11, DDR0_11_SREFRESH_ENCODE(0) |
		DDR0_11_TXSNR_ENCODE(t_xsnr_clk) | DDR0_11_TXSR_ENCODE(200));
}

static void program_ddr0_22(unsigned long dimm_ranks[],
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks, unsigned long width)
{
#if defined(CONFIG_DDR_ECC)
	unsigned long dimm_num;
	unsigned long ecc_available = width >= 64;
	u32 ddr0_22 = DDR0_22_DQS_OUT_SHIFT_BYPASS_ENCODE(0x26) |
	    DDR0_22_DQS_OUT_SHIFT_ENCODE(DQS_OUT_SHIFT) |
	    DDR0_22_DLL_DQS_BYPASS_8_ENCODE(DLL_DQS_BYPASS);

	/* loop through all the DIMM slots on the board */
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		/* If a dimm is installed in a particular slot ... */
		if (dimm_ranks[dimm_num]) {
			/* Check for ECC */
			if (0 == (spd_read(iic0_dimm_addr[dimm_num], 11) &
				  0x02)) {
				ecc_available = FALSE;
			}
		}
	}
	if (ecc_available) {
		debug("ECC found on all DIMMs present\n");
		mtsdram(DDR0_22, ddr0_22 | DDR0_22_CTRL_RAW_ENCODE(0x3));
	} else {
		debug("ECC not found on some or all DIMMs present\n");
		mtsdram(DDR0_22, ddr0_22 | DDR0_22_CTRL_RAW_ENCODE(0x0));
	}
#else
	mtsdram(DDR0_22, DDR0_22_CTRL_RAW_ENCODE(0x0) |
		DDR0_22_DQS_OUT_SHIFT_BYPASS_ENCODE(0x26) |
		DDR0_22_DQS_OUT_SHIFT_ENCODE(DQS_OUT_SHIFT) |
		DDR0_22_DLL_DQS_BYPASS_8_ENCODE(DLL_DQS_BYPASS));
#endif /* defined(CONFIG_DDR_ECC) */
}

static void program_ddr0_24(unsigned long ranks)
{
	u32 ddr0_24 = DDR0_24_RTT_PAD_TERMINATION_ENCODE(0x1) |	/* 75 ohm */
	    DDR0_24_ODT_RD_MAP_CS1_ENCODE(0x0);

	if (2 == ranks) {
		/* Both chip selects in use */
		ddr0_24 |= DDR0_24_ODT_WR_MAP_CS1_ENCODE(0x1) |
		    DDR0_24_ODT_WR_MAP_CS0_ENCODE(0x2);
	} else {
		/* One chip select in use */
		/* One of the two fields added to ddr0_24 is a "don't care" */
		ddr0_24 |= DDR0_24_ODT_WR_MAP_CS1_ENCODE(0x2) |
		    DDR0_24_ODT_WR_MAP_CS0_ENCODE(0x1);
	}
	mtsdram(DDR0_24, ddr0_24);
}

static void program_ddr0_26(unsigned long sdram_freq)
{
	unsigned long const t_ref_ps = 7800000;	/* 7.8 us. refresh */
	/* TODO: check definition of tRAS_MAX */
	unsigned long const t_ras_max_ps = 9 * t_ref_ps;
	unsigned long t_ras_max_clk;
	unsigned long t_ref_clk;

	/* Round down t_ras_max_clk and t_ref_clk */
	debug("t_ras_max_ps = %d\n", t_ras_max_ps);
	t_ras_max_clk = MULDIV64(sdram_freq, t_ras_max_ps, ONE_BILLION) / 1000;
	debug("t_ref_ps     = %d\n", t_ref_ps);
	t_ref_clk = MULDIV64(sdram_freq, t_ref_ps, ONE_BILLION) / 1000;
	mtsdram(DDR0_26, DDR0_26_TRAS_MAX_ENCODE(t_ras_max_clk) |
		DDR0_26_TREF_ENCODE(t_ref_clk));
}

static void program_ddr0_27(unsigned long sdram_freq)
{
	unsigned long const t_init_ps = 200000000;	/* 200 us. init */
	unsigned long t_init_clk;

	debug("t_init_ps = %d\n", t_init_ps);
	t_init_clk =
	    (MULDIV64(sdram_freq, t_init_ps, ONE_BILLION) + 999) / 1000;
	mtsdram(DDR0_27, DDR0_27_EMRS_DATA_ENCODE(0x0000) |
		DDR0_27_TINIT_ENCODE(t_init_clk));
}

static void program_ddr0_43(unsigned long dimm_ranks[],
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks,
			    unsigned long sdram_freq,
			    unsigned long cols, unsigned long banks)
{
	unsigned long dimm_num;
	unsigned long t_wr_ps = 0;
	unsigned long t_wr_clk;
	u32 ddr0_43 = DDR0_43_APREBIT_ENCODE(10) |
	    DDR0_43_COLUMN_SIZE_ENCODE(12 - cols) |
	    DDR0_43_EIGHT_BANK_MODE_ENCODE(8 == banks ? 1 : 0);

	/*------------------------------------------------------------------
	 * Handle the timing.  We need to find the worst case timing of all
	 * the dimm modules installed.
	 *-----------------------------------------------------------------*/
	/* loop through all the DIMM slots on the board */
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		/* If a dimm is installed in a particular slot ... */
		if (dimm_ranks[dimm_num]) {
			unsigned long ps;

			ps = 250 * spd_read(iic0_dimm_addr[dimm_num], 36);
			t_wr_ps = max(t_wr_ps, ps);
		}
	}
	debug("t_wr_ps = %d\n", t_wr_ps);
	t_wr_clk = (MULDIV64(sdram_freq, t_wr_ps, ONE_BILLION) + 999) / 1000;
	mtsdram(DDR0_43, ddr0_43 | DDR0_43_TWR_ENCODE(t_wr_clk));
}

static void program_ddr0_44(unsigned long dimm_ranks[],
			    unsigned char const iic0_dimm_addr[],
			    unsigned long num_dimm_banks,
			    unsigned long sdram_freq)
{
	unsigned long dimm_num;
	unsigned long t_rcd_ps = 0;
	unsigned long t_rcd_clk;

	/*------------------------------------------------------------------
	 * Handle the timing.  We need to find the worst case timing of all
	 * the dimm modules installed.
	 *-----------------------------------------------------------------*/
	/* loop through all the DIMM slots on the board */
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		/* If a dimm is installed in a particular slot ... */
		if (dimm_ranks[dimm_num]) {
			unsigned long ps;

			ps = 250 * spd_read(iic0_dimm_addr[dimm_num], 29);
			t_rcd_ps = max(t_rcd_ps, ps);
		}
	}
	debug("t_rcd_ps = %d\n", t_rcd_ps);
	t_rcd_clk = (MULDIV64(sdram_freq, t_rcd_ps, ONE_BILLION) + 999) / 1000;
	mtsdram(DDR0_44, DDR0_44_TRCD_ENCODE(t_rcd_clk));
}

/*-----------------------------------------------------------------------------+
 * initdram.  Initializes the 440EPx/GPx DDR SDRAM controller.
 * Note: This routine runs from flash with a stack set up in the chip's
 * sram space.  It is important that the routine does not require .sbss, .bss or
 * .data sections.  It also cannot call routines that require these sections.
 *-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Function:	 initdram
 * Description:  Configures SDRAM memory banks for DDR operation.
 *		 Auto Memory Configuration option reads the DDR SDRAM EEPROMs
 *		 via the IIC bus and then configures the DDR SDRAM memory
 *		 banks appropriately. If Auto Memory Configuration is
 *		 not used, it is assumed that no DIMM is plugged
 *-----------------------------------------------------------------------------*/
long int initdram(int board_type)
{
	unsigned char const iic0_dimm_addr[] = SPD_EEPROM_ADDRESS;
	unsigned long dimm_ranks[MAXDIMMS];
	unsigned long ranks;
	unsigned long rows;
	unsigned long banks;
	unsigned long cols;
	unsigned long width;
	unsigned long const sdram_freq = get_bus_freq(0);
	unsigned long const num_dimm_banks = sizeof(iic0_dimm_addr);	/* on board dimm banks */
	unsigned long cas_latency = 0;	/* to quiet initialization warning */
	unsigned long dram_size;

	debug("\nEntering initdram()\n");

	/*------------------------------------------------------------------
	 * Stop the DDR-SDRAM controller.
	 *-----------------------------------------------------------------*/
	mtsdram(DDR0_02, DDR0_02_START_ENCODE(0));

	/*
	 * Make sure I2C controller is initialized
	 * before continuing.
	 */
	/* switch to correct I2C bus */
	I2C_SET_BUS(CFG_SPD_BUS_NUM);
	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);

	/*------------------------------------------------------------------
	 * Clear out the serial presence detect buffers.
	 * Perform IIC reads from the dimm.  Fill in the spds.
	 * Check to see if the dimm slots are populated
	 *-----------------------------------------------------------------*/
	get_spd_info(dimm_ranks, &ranks, iic0_dimm_addr, num_dimm_banks);

	/*------------------------------------------------------------------
	 * Check the frequency supported for the dimms plugged.
	 *-----------------------------------------------------------------*/
	check_frequency(dimm_ranks, iic0_dimm_addr, num_dimm_banks, sdram_freq);

	/*------------------------------------------------------------------
	 * Check and get size information.
	 *-----------------------------------------------------------------*/
	get_dimm_size(dimm_ranks, iic0_dimm_addr, num_dimm_banks, &rows, &banks,
		      &cols, &width);

	/*------------------------------------------------------------------
	 * Check the voltage type for the dimms plugged.
	 *-----------------------------------------------------------------*/
	check_voltage_type(dimm_ranks, iic0_dimm_addr, num_dimm_banks);

	/*------------------------------------------------------------------
	 * Program registers for SDRAM controller.
	 *-----------------------------------------------------------------*/
	mtsdram(DDR0_00, DDR0_00_DLL_INCREMENT_ENCODE(0x19) |
		DDR0_00_DLL_START_POINT_DECODE(0x0A));

	mtsdram(DDR0_01, DDR0_01_PLB0_DB_CS_LOWER_ENCODE(0x01) |
		DDR0_01_PLB0_DB_CS_UPPER_ENCODE(0x00) |
		DDR0_01_INT_MASK_ENCODE(0xFF));

	program_ddr0_03(dimm_ranks, iic0_dimm_addr, num_dimm_banks, sdram_freq,
			rows, &cas_latency);

	program_ddr0_04(dimm_ranks, iic0_dimm_addr, num_dimm_banks, sdram_freq);

	program_ddr0_05(dimm_ranks, iic0_dimm_addr, num_dimm_banks, sdram_freq);

	program_ddr0_06(dimm_ranks, iic0_dimm_addr, num_dimm_banks, sdram_freq);

	/*
	 * TODO: tFAW not found in SPD.  Value of 13 taken from Sequoia
	 * board SDRAM, but may be overly conservative.
	 */
	mtsdram(DDR0_07, DDR0_07_NO_CMD_INIT_ENCODE(0) |
		DDR0_07_TFAW_ENCODE(13) |
		DDR0_07_AUTO_REFRESH_MODE_ENCODE(1) |
		DDR0_07_AREFRESH_ENCODE(0));

	mtsdram(DDR0_08, DDR0_08_WRLAT_ENCODE(cas_latency - 1) |
		DDR0_08_TCPD_ENCODE(200) | DDR0_08_DQS_N_EN_ENCODE(0) |
		DDR0_08_DDRII_ENCODE(1));

	mtsdram(DDR0_09, DDR0_09_OCD_ADJUST_PDN_CS_0_ENCODE(0x00) |
		DDR0_09_RTT_0_ENCODE(0x1) |
		DDR0_09_WR_DQS_SHIFT_BYPASS_ENCODE(0x1D) |
		DDR0_09_WR_DQS_SHIFT_ENCODE(DQS_OUT_SHIFT - 0x20));

	program_ddr0_10(dimm_ranks, ranks);

	program_ddr0_11(sdram_freq);

	mtsdram(DDR0_12, DDR0_12_TCKE_ENCODE(3));

	mtsdram(DDR0_14, DDR0_14_DLL_BYPASS_MODE_ENCODE(0) |
		DDR0_14_REDUC_ENCODE(width <= 40 ? 1 : 0) |
		DDR0_14_REG_DIMM_ENABLE_ENCODE(0));

	mtsdram(DDR0_17, DDR0_17_DLL_DQS_DELAY_0_ENCODE(DLL_DQS_DELAY));

	mtsdram(DDR0_18, DDR0_18_DLL_DQS_DELAY_4_ENCODE(DLL_DQS_DELAY) |
		DDR0_18_DLL_DQS_DELAY_3_ENCODE(DLL_DQS_DELAY) |
		DDR0_18_DLL_DQS_DELAY_2_ENCODE(DLL_DQS_DELAY) |
		DDR0_18_DLL_DQS_DELAY_1_ENCODE(DLL_DQS_DELAY));

	mtsdram(DDR0_19, DDR0_19_DLL_DQS_DELAY_8_ENCODE(DLL_DQS_DELAY) |
		DDR0_19_DLL_DQS_DELAY_7_ENCODE(DLL_DQS_DELAY) |
		DDR0_19_DLL_DQS_DELAY_6_ENCODE(DLL_DQS_DELAY) |
		DDR0_19_DLL_DQS_DELAY_5_ENCODE(DLL_DQS_DELAY));

	mtsdram(DDR0_20, DDR0_20_DLL_DQS_BYPASS_3_ENCODE(DLL_DQS_BYPASS) |
		DDR0_20_DLL_DQS_BYPASS_2_ENCODE(DLL_DQS_BYPASS) |
		DDR0_20_DLL_DQS_BYPASS_1_ENCODE(DLL_DQS_BYPASS) |
		DDR0_20_DLL_DQS_BYPASS_0_ENCODE(DLL_DQS_BYPASS));

	mtsdram(DDR0_21, DDR0_21_DLL_DQS_BYPASS_7_ENCODE(DLL_DQS_BYPASS) |
		DDR0_21_DLL_DQS_BYPASS_6_ENCODE(DLL_DQS_BYPASS) |
		DDR0_21_DLL_DQS_BYPASS_5_ENCODE(DLL_DQS_BYPASS) |
		DDR0_21_DLL_DQS_BYPASS_4_ENCODE(DLL_DQS_BYPASS));

	program_ddr0_22(dimm_ranks, iic0_dimm_addr, num_dimm_banks, width);

	mtsdram(DDR0_23, DDR0_23_ODT_RD_MAP_CS0_ENCODE(0x0) |
		DDR0_23_FWC_ENCODE(0));

	program_ddr0_24(ranks);

	program_ddr0_26(sdram_freq);

	program_ddr0_27(sdram_freq);

	mtsdram(DDR0_28, DDR0_28_EMRS3_DATA_ENCODE(0x0000) |
		DDR0_28_EMRS2_DATA_ENCODE(0x0000));

	mtsdram(DDR0_31, DDR0_31_XOR_CHECK_BITS_ENCODE(0x0000));

	mtsdram(DDR0_42, DDR0_42_ADDR_PINS_DECODE(14 - rows) |
		DDR0_42_CASLAT_LIN_GATE_ENCODE(2 * cas_latency));

	program_ddr0_43(dimm_ranks, iic0_dimm_addr, num_dimm_banks, sdram_freq,
			cols, banks);

	program_ddr0_44(dimm_ranks, iic0_dimm_addr, num_dimm_banks, sdram_freq);

	denali_sdram_register_dump();

	dram_size = (width >= 64) ? 8 : 4;
	dram_size *= 1 << cols;
	dram_size *= banks;
	dram_size *= 1 << rows;
	dram_size *= ranks;
	debug("dram_size = %lu\n", dram_size);

	/* Start the SDRAM controler */
	mtsdram(DDR0_02, DDR0_02_START_ENCODE(1));
	denali_wait_for_dlllock();

#if defined(CONFIG_DDR_DATA_EYE)
	/*
	 * Map the first 1 MiB of memory in the TLB, and perform the data eye
	 * search.
	 */
	program_tlb(0, CFG_SDRAM_BASE, TLB_1MB_SIZE, TLB_WORD2_I_ENABLE);
	denali_core_search_data_eye();
	denali_sdram_register_dump();
	remove_tlb(CFG_SDRAM_BASE, TLB_1MB_SIZE);
#endif

#if defined(CONFIG_ZERO_SDRAM) || defined(CONFIG_DDR_ECC)
	program_tlb(0, CFG_SDRAM_BASE, dram_size, 0);
	sync();
	/* Zero the memory */
	debug("Zeroing SDRAM...");
#if defined(CFG_MEM_TOP_HIDE)
	dcbz_area(CFG_SDRAM_BASE, dram_size - CFG_MEM_TOP_HIDE);
#else
#error Please define CFG_MEM_TOP_HIDE (see README) in your board config file
#endif
	dflush();
	debug("Completed\n");
	sync();
	remove_tlb(CFG_SDRAM_BASE, dram_size);

#if defined(CONFIG_DDR_ECC)
	/*
	 * If ECC is enabled, clear and enable interrupts
	 */
	if (is_ecc_enabled()) {
		u32 val;

		sync();
		/* Clear error status */
		mfsdram(DDR0_00, val);
		mtsdram(DDR0_00, val | DDR0_00_INT_ACK_ALL);
		/* Set 'int_mask' parameter to functionnal value */
		mfsdram(DDR0_01, val);
		mtsdram(DDR0_01, (val & ~DDR0_01_INT_MASK_MASK) |
			DDR0_01_INT_MASK_ALL_OFF);
#if defined(CONFIG_DDR_DATA_EYE)
		/*
		 * Running denali_core_search_data_eye() when ECC is enabled
		 * causes non-ECC machine checks.  This clears them.
		 */
		print_mcsr();
		mtspr(SPRN_MCSR, mfspr(SPRN_MCSR));
		print_mcsr();
#endif
		sync();
	}
#endif /* defined(CONFIG_DDR_ECC) */
#endif /* defined(CONFIG_ZERO_SDRAM) || defined(CONFIG_DDR_ECC) */

	program_tlb(0, CFG_SDRAM_BASE, dram_size, MY_TLB_WORD2_I_ENABLE);
	return dram_size;
}

void board_add_ram_info(int use_default)
{
	u32 val;

	printf(" (ECC");
	if (!is_ecc_enabled()) {
		printf(" not");
	}
	printf(" enabled, %d MHz", (2 * get_bus_freq(0)) / 1000000);

	mfsdram(DDR0_03, val);
	printf(", CL%d)", DDR0_03_CASLAT_LIN_DECODE(val) >> 1);
}
#endif /* CONFIG_SPD_EEPROM */
