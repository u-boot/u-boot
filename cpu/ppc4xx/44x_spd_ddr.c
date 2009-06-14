/*
 * cpu/ppc4xx/44x_spd_ddr.c
 * This SPD DDR detection code supports IBM/AMCC PPC44x cpu with a
 * DDR controller. Those are 440GP/GX/EP/GR.
 *
 * (C) Copyright 2001
 * Bill Hunter, Wave 7 Optics, williamhunter@attbi.com
 *
 * Based on code by:
 *
 * Kenneth Johansson ,Ericsson AB.
 * kenneth.johansson@etx.ericsson.se
 *
 * hacked up by bill hunter. fixed so we could run before
 * serial_init and console_init. previous version avoided this by
 * running out of cache memory during serial/console init, then running
 * this code later.
 *
 * (C) Copyright 2002
 * Jun Gu, Artesyn Technology, jung@artesyncp.com
 * Support for AMCC 440 based on OpenBIOS draminit.c from IBM.
 *
 * (C) Copyright 2005-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <asm/processor.h>
#include <i2c.h>
#include <ppc4xx.h>
#include <asm/mmu.h>

#include "ecc.h"

#if defined(CONFIG_SPD_EEPROM) &&					\
	(defined(CONFIG_440GP) || defined(CONFIG_440GX) ||		\
	 defined(CONFIG_440EP) || defined(CONFIG_440GR))

/*
 * Set default values
 */
#ifndef CONFIG_SYS_I2C_SPEED
#define CONFIG_SYS_I2C_SPEED	50000
#endif

#define ONE_BILLION	1000000000

/*
 * Board-specific Platform code can reimplement spd_ddr_init_hang () if needed
 */
void __spd_ddr_init_hang (void)
{
	hang ();
}
void spd_ddr_init_hang (void) __attribute__((weak, alias("__spd_ddr_init_hang")));

/*-----------------------------------------------------------------------------+
  |  General Definition
  +-----------------------------------------------------------------------------*/
#define DEFAULT_SPD_ADDR1	0x53
#define DEFAULT_SPD_ADDR2	0x52
#define MAXBANKS		4		/* at most 4 dimm banks */
#define MAX_SPD_BYTES		256
#define NUMHALFCYCLES		4
#define NUMMEMTESTS		8
#define NUMMEMWORDS		8
#define MAXBXCR			4
#define TRUE			1
#define FALSE			0

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
#ifdef CONFIG_4xx_DCACHE
#define MY_TLB_WORD2_I_ENABLE	0			/* enable caching on SDRAM */
#else
#define MY_TLB_WORD2_I_ENABLE	TLB_WORD2_I_ENABLE	/* disable caching on SDRAM */
#endif

/* bank_parms is used to sort the bank sizes by descending order */
struct bank_param {
	unsigned long cr;
	unsigned long bank_size_bytes;
};

typedef struct bank_param BANKPARMS;

#ifdef CONFIG_SYS_SIMULATE_SPD_EEPROM
extern const unsigned char cfg_simulate_spd_eeprom[128];
#endif

static unsigned char spd_read(uchar chip, uint addr);
static void get_spd_info(unsigned long *dimm_populated,
			 unsigned char *iic0_dimm_addr,
			 unsigned long num_dimm_banks);
static void check_mem_type(unsigned long *dimm_populated,
			   unsigned char *iic0_dimm_addr,
			   unsigned long num_dimm_banks);
static void check_volt_type(unsigned long *dimm_populated,
			    unsigned char *iic0_dimm_addr,
			    unsigned long num_dimm_banks);
static void program_cfg0(unsigned long *dimm_populated,
			 unsigned char *iic0_dimm_addr,
			 unsigned long  num_dimm_banks);
static void program_cfg1(unsigned long *dimm_populated,
			 unsigned char *iic0_dimm_addr,
			 unsigned long num_dimm_banks);
static void program_rtr(unsigned long *dimm_populated,
			unsigned char *iic0_dimm_addr,
			unsigned long num_dimm_banks);
static void program_tr0(unsigned long *dimm_populated,
			unsigned char *iic0_dimm_addr,
			unsigned long num_dimm_banks);
static void program_tr1(void);

static unsigned long program_bxcr(unsigned long *dimm_populated,
				  unsigned char *iic0_dimm_addr,
				  unsigned long num_dimm_banks);

/*
 * This function is reading data from the DIMM module EEPROM over the SPD bus
 * and uses that to program the sdram controller.
 *
 * This works on boards that has the same schematics that the AMCC walnut has.
 *
 * BUG: Don't handle ECC memory
 * BUG: A few values in the TR register is currently hardcoded
 */
long int spd_sdram(void) {
	unsigned char iic0_dimm_addr[] = SPD_EEPROM_ADDRESS;
	unsigned long dimm_populated[sizeof(iic0_dimm_addr)];
	unsigned long total_size;
	unsigned long cfg0;
	unsigned long mcsts;
	unsigned long num_dimm_banks;		    /* on board dimm banks */

	num_dimm_banks = sizeof(iic0_dimm_addr);

	/*
	 * Make sure I2C controller is initialized
	 * before continuing.
	 */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	/*
	 * Read the SPD information using I2C interface. Check to see if the
	 * DIMM slots are populated.
	 */
	get_spd_info(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * Check the memory type for the dimms plugged.
	 */
	check_mem_type(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * Check the voltage type for the dimms plugged.
	 */
	check_volt_type(dimm_populated, iic0_dimm_addr, num_dimm_banks);

#if defined(CONFIG_440GX) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
	/*
	 * Soft-reset SDRAM controller.
	 */
	mtsdr(sdr_srst, SDR0_SRST_DMC);
	mtsdr(sdr_srst, 0x00000000);
#endif

	/*
	 * program 440GP SDRAM controller options (SDRAM0_CFG0)
	 */
	program_cfg0(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program 440GP SDRAM controller options (SDRAM0_CFG1)
	 */
	program_cfg1(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program SDRAM refresh register (SDRAM0_RTR)
	 */
	program_rtr(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program SDRAM Timing Register 0 (SDRAM0_TR0)
	 */
	program_tr0(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program the BxCR registers to find out total sdram installed
	 */
	total_size = program_bxcr(dimm_populated, iic0_dimm_addr,
				  num_dimm_banks);

#ifdef CONFIG_PROG_SDRAM_TLB /* this define should eventually be removed */
	/* and program tlb entries for this size (dynamic) */
	program_tlb(0, 0, total_size, MY_TLB_WORD2_I_ENABLE);
#endif

	/*
	 * program SDRAM Clock Timing Register (SDRAM0_CLKTR)
	 */
	mtsdram(mem_clktr, 0x40000000);

	/*
	 * delay to ensure 200 usec has elapsed
	 */
	udelay(400);

	/*
	 * enable the memory controller
	 */
	mfsdram(mem_cfg0, cfg0);
	mtsdram(mem_cfg0, cfg0 | SDRAM_CFG0_DCEN);

	/*
	 * wait for SDRAM_CFG0_DC_EN to complete
	 */
	while (1) {
		mfsdram(mem_mcsts, mcsts);
		if ((mcsts & SDRAM_MCSTS_MRSC) != 0)
			break;
	}

	/*
	 * program SDRAM Timing Register 1, adding some delays
	 */
	program_tr1();

#ifdef CONFIG_DDR_ECC
	/*
	 * If ecc is enabled, initialize the parity bits.
	 */
	ecc_init(CONFIG_SYS_SDRAM_BASE, total_size);
#endif

	return total_size;
}

static unsigned char spd_read(uchar chip, uint addr)
{
	unsigned char data[2];

#ifdef CONFIG_SYS_SIMULATE_SPD_EEPROM
	if (chip == CONFIG_SYS_SIMULATE_SPD_EEPROM) {
		/*
		 * Onboard spd eeprom requested -> simulate values
		 */
		return cfg_simulate_spd_eeprom[addr];
	}
#endif /* CONFIG_SYS_SIMULATE_SPD_EEPROM */

	if (i2c_probe(chip) == 0) {
		if (i2c_read(chip, addr, 1, data, 1) == 0) {
			return data[0];
		}
	}

	return 0;
}

static void get_spd_info(unsigned long *dimm_populated,
			 unsigned char *iic0_dimm_addr,
			 unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long dimm_found;
	unsigned char num_of_bytes;
	unsigned char total_size;

	dimm_found = FALSE;
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		num_of_bytes = 0;
		total_size = 0;

		num_of_bytes = spd_read(iic0_dimm_addr[dimm_num], 0);
		total_size = spd_read(iic0_dimm_addr[dimm_num], 1);

		if ((num_of_bytes != 0) && (total_size != 0)) {
			dimm_populated[dimm_num] = TRUE;
			dimm_found = TRUE;
			debug("DIMM slot %lu: populated\n", dimm_num);
		} else {
			dimm_populated[dimm_num] = FALSE;
			debug("DIMM slot %lu: Not populated\n", dimm_num);
		}
	}

	if (dimm_found == FALSE) {
		printf("ERROR - No memory installed. Install a DDR-SDRAM DIMM.\n\n");
		spd_ddr_init_hang ();
	}
}

static void check_mem_type(unsigned long *dimm_populated,
			   unsigned char *iic0_dimm_addr,
			   unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned char dimm_type;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			dimm_type = spd_read(iic0_dimm_addr[dimm_num], 2);
			switch (dimm_type) {
			case 7:
				debug("DIMM slot %lu: DDR SDRAM detected\n", dimm_num);
				break;
			default:
				printf("ERROR: Unsupported DIMM detected in slot %lu.\n",
				       dimm_num);
				printf("Only DDR SDRAM DIMMs are supported.\n");
				printf("Replace the DIMM module with a supported DIMM.\n\n");
				spd_ddr_init_hang ();
				break;
			}
		}
	}
}

static void check_volt_type(unsigned long *dimm_populated,
			    unsigned char *iic0_dimm_addr,
			    unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long voltage_type;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			voltage_type = spd_read(iic0_dimm_addr[dimm_num], 8);
			if (voltage_type != 0x04) {
				printf("ERROR: DIMM %lu with unsupported voltage level.\n",
				       dimm_num);
				spd_ddr_init_hang ();
			} else {
				debug("DIMM %lu voltage level supported.\n", dimm_num);
			}
			break;
		}
	}
}

static void program_cfg0(unsigned long *dimm_populated,
			 unsigned char *iic0_dimm_addr,
			 unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long cfg0;
	unsigned long ecc_enabled;
	unsigned char ecc;
	unsigned char attributes;
	unsigned long data_width;
	unsigned long dimm_32bit;
	unsigned long dimm_64bit;

	/*
	 * get Memory Controller Options 0 data
	 */
	mfsdram(mem_cfg0, cfg0);

	/*
	 * clear bits
	 */
	cfg0 &= ~(SDRAM_CFG0_DCEN | SDRAM_CFG0_MCHK_MASK |
		  SDRAM_CFG0_RDEN | SDRAM_CFG0_PMUD |
		  SDRAM_CFG0_DMWD_MASK |
		  SDRAM_CFG0_UIOS_MASK | SDRAM_CFG0_PDP);


	/*
	 * FIXME: assume the DDR SDRAMs in both banks are the same
	 */
	ecc_enabled = TRUE;
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			ecc = spd_read(iic0_dimm_addr[dimm_num], 11);
			if (ecc != 0x02) {
				ecc_enabled = FALSE;
			}

			/*
			 * program Registered DIMM Enable
			 */
			attributes = spd_read(iic0_dimm_addr[dimm_num], 21);
			if ((attributes & 0x02) != 0x00) {
				cfg0 |= SDRAM_CFG0_RDEN;
			}

			/*
			 * program DDR SDRAM Data Width
			 */
			data_width =
				(unsigned long)spd_read(iic0_dimm_addr[dimm_num],6) +
				(((unsigned long)spd_read(iic0_dimm_addr[dimm_num],7)) << 8);
			if (data_width == 64 || data_width == 72) {
				dimm_64bit = TRUE;
				cfg0 |= SDRAM_CFG0_DMWD_64;
			} else if (data_width == 32 || data_width == 40) {
				dimm_32bit = TRUE;
				cfg0 |= SDRAM_CFG0_DMWD_32;
			} else {
				printf("WARNING: DIMM with datawidth of %lu bits.\n",
				       data_width);
				printf("Only DIMMs with 32 or 64 bit datawidths supported.\n");
				spd_ddr_init_hang ();
			}
			break;
		}
	}

	/*
	 * program Memory Data Error Checking
	 */
	if (ecc_enabled == TRUE) {
		cfg0 |= SDRAM_CFG0_MCHK_GEN;
	} else {
		cfg0 |= SDRAM_CFG0_MCHK_NON;
	}

	/*
	 * program Page Management Unit (0 == enabled)
	 */
	cfg0 &= ~SDRAM_CFG0_PMUD;

	/*
	 * program Memory Controller Options 0
	 * Note: DCEN must be enabled after all DDR SDRAM controller
	 * configuration registers get initialized.
	 */
	mtsdram(mem_cfg0, cfg0);
}

static void program_cfg1(unsigned long *dimm_populated,
			 unsigned char *iic0_dimm_addr,
			 unsigned long num_dimm_banks)
{
	unsigned long cfg1;
	mfsdram(mem_cfg1, cfg1);

	/*
	 * Self-refresh exit, disable PM
	 */
	cfg1 &= ~(SDRAM_CFG1_SRE | SDRAM_CFG1_PMEN);

	/*
	 * program Memory Controller Options 1
	 */
	mtsdram(mem_cfg1, cfg1);
}

static void program_rtr(unsigned long *dimm_populated,
			unsigned char *iic0_dimm_addr,
			unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long bus_period_x_10;
	unsigned long refresh_rate = 0;
	unsigned char refresh_rate_type;
	unsigned long refresh_interval;
	unsigned long sdram_rtr;
	PPC4xx_SYS_INFO sys_info;

	/*
	 * get the board info
	 */
	get_sys_info(&sys_info);
	bus_period_x_10 = ONE_BILLION / (sys_info.freqPLB / 10);

	for (dimm_num = 0;  dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			refresh_rate_type = 0x7F & spd_read(iic0_dimm_addr[dimm_num], 12);
			switch (refresh_rate_type) {
			case 0x00:
				refresh_rate = 15625;
				break;
			case 0x01:
				refresh_rate = 15625/4;
				break;
			case 0x02:
				refresh_rate = 15625/2;
				break;
			case 0x03:
				refresh_rate = 15626*2;
				break;
			case 0x04:
				refresh_rate = 15625*4;
				break;
			case 0x05:
				refresh_rate = 15625*8;
				break;
			default:
				printf("ERROR: DIMM %lu, unsupported refresh rate/type.\n",
				       dimm_num);
				printf("Replace the DIMM module with a supported DIMM.\n");
				break;
			}

			break;
		}
	}

	refresh_interval = refresh_rate * 10 / bus_period_x_10;
	sdram_rtr = (refresh_interval & 0x3ff8) <<  16;

	/*
	 * program Refresh Timer Register (SDRAM0_RTR)
	 */
	mtsdram(mem_rtr, sdram_rtr);
}

static void program_tr0(unsigned long *dimm_populated,
			 unsigned char *iic0_dimm_addr,
			 unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long tr0;
	unsigned char wcsbc;
	unsigned char t_rp_ns;
	unsigned char t_rcd_ns;
	unsigned char t_ras_ns;
	unsigned long t_rp_clk;
	unsigned long t_ras_rcd_clk;
	unsigned long t_rcd_clk;
	unsigned long t_rfc_clk;
	unsigned long plb_check;
	unsigned char cas_bit;
	unsigned long cas_index;
	unsigned char cas_2_0_available;
	unsigned char cas_2_5_available;
	unsigned char cas_3_0_available;
	unsigned long cycle_time_ns_x_10[3];
	unsigned long tcyc_3_0_ns_x_10;
	unsigned long tcyc_2_5_ns_x_10;
	unsigned long tcyc_2_0_ns_x_10;
	unsigned long tcyc_reg;
	unsigned long bus_period_x_10;
	PPC4xx_SYS_INFO sys_info;
	unsigned long residue;

	/*
	 * get the board info
	 */
	get_sys_info(&sys_info);
	bus_period_x_10 = ONE_BILLION / (sys_info.freqPLB / 10);

	/*
	 * get SDRAM Timing Register 0 (SDRAM_TR0) and clear bits
	 */
	mfsdram(mem_tr0, tr0);
	tr0 &= ~(SDRAM_TR0_SDWR_MASK | SDRAM_TR0_SDWD_MASK |
		 SDRAM_TR0_SDCL_MASK | SDRAM_TR0_SDPA_MASK |
		 SDRAM_TR0_SDCP_MASK | SDRAM_TR0_SDLD_MASK |
		 SDRAM_TR0_SDRA_MASK | SDRAM_TR0_SDRD_MASK);

	/*
	 * initialization
	 */
	wcsbc = 0;
	t_rp_ns = 0;
	t_rcd_ns = 0;
	t_ras_ns = 0;
	cas_2_0_available = TRUE;
	cas_2_5_available = TRUE;
	cas_3_0_available = TRUE;
	tcyc_2_0_ns_x_10 = 0;
	tcyc_2_5_ns_x_10 = 0;
	tcyc_3_0_ns_x_10 = 0;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			wcsbc = spd_read(iic0_dimm_addr[dimm_num], 15);
			t_rp_ns	 = spd_read(iic0_dimm_addr[dimm_num], 27) >> 2;
			t_rcd_ns = spd_read(iic0_dimm_addr[dimm_num], 29) >> 2;
			t_ras_ns = spd_read(iic0_dimm_addr[dimm_num], 30);
			cas_bit = spd_read(iic0_dimm_addr[dimm_num], 18);

			for (cas_index = 0; cas_index < 3; cas_index++) {
				switch (cas_index) {
				case 0:
					tcyc_reg = spd_read(iic0_dimm_addr[dimm_num], 9);
					break;
				case 1:
					tcyc_reg = spd_read(iic0_dimm_addr[dimm_num], 23);
					break;
				default:
					tcyc_reg = spd_read(iic0_dimm_addr[dimm_num], 25);
					break;
				}

				if ((tcyc_reg & 0x0F) >= 10) {
					printf("ERROR: Tcyc incorrect for DIMM in slot %lu\n",
					       dimm_num);
					spd_ddr_init_hang ();
				}

				cycle_time_ns_x_10[cas_index] =
					(((tcyc_reg & 0xF0) >> 4) * 10) + (tcyc_reg & 0x0F);
			}

			cas_index = 0;

			if ((cas_bit & 0x80) != 0) {
				cas_index += 3;
			} else if ((cas_bit & 0x40) != 0) {
				cas_index += 2;
			} else if ((cas_bit & 0x20) != 0) {
				cas_index += 1;
			}

			if (((cas_bit & 0x10) != 0) && (cas_index < 3)) {
				tcyc_3_0_ns_x_10 = cycle_time_ns_x_10[cas_index];
				cas_index++;
			} else {
				if (cas_index != 0) {
					cas_index++;
				}
				cas_3_0_available = FALSE;
			}

			if (((cas_bit & 0x08) != 0) || (cas_index < 3)) {
				tcyc_2_5_ns_x_10 = cycle_time_ns_x_10[cas_index];
				cas_index++;
			} else {
				if (cas_index != 0) {
					cas_index++;
				}
				cas_2_5_available = FALSE;
			}

			if (((cas_bit & 0x04) != 0) || (cas_index < 3)) {
				tcyc_2_0_ns_x_10 = cycle_time_ns_x_10[cas_index];
				cas_index++;
			} else {
				if (cas_index != 0) {
					cas_index++;
				}
				cas_2_0_available = FALSE;
			}

			break;
		}
	}

	/*
	 * Program SD_WR and SD_WCSBC fields
	 */
	tr0 |= SDRAM_TR0_SDWR_2_CLK;		    /* Write Recovery: 2 CLK */
	switch (wcsbc) {
	case 0:
		tr0 |= SDRAM_TR0_SDWD_0_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDWD_1_CLK;
		break;
	}

	/*
	 * Program SD_CASL field
	 */
	if ((cas_2_0_available == TRUE) &&
	    (bus_period_x_10 >= tcyc_2_0_ns_x_10)) {
		tr0 |= SDRAM_TR0_SDCL_2_0_CLK;
	} else if ((cas_2_5_available == TRUE) &&
		 (bus_period_x_10 >= tcyc_2_5_ns_x_10)) {
		tr0 |= SDRAM_TR0_SDCL_2_5_CLK;
	} else if ((cas_3_0_available == TRUE) &&
		 (bus_period_x_10 >= tcyc_3_0_ns_x_10)) {
		tr0 |= SDRAM_TR0_SDCL_3_0_CLK;
	} else {
		printf("ERROR: No supported CAS latency with the installed DIMMs.\n");
		printf("Only CAS latencies of 2.0, 2.5, and 3.0 are supported.\n");
		printf("Make sure the PLB speed is within the supported range.\n");
		spd_ddr_init_hang ();
	}

	/*
	 * Calculate Trp in clock cycles and round up if necessary
	 * Program SD_PTA field
	 */
	t_rp_clk = sys_info.freqPLB * t_rp_ns / ONE_BILLION;
	plb_check = ONE_BILLION * t_rp_clk / t_rp_ns;
	if (sys_info.freqPLB != plb_check) {
		t_rp_clk++;
	}
	switch ((unsigned long)t_rp_clk) {
	case 0:
	case 1:
	case 2:
		tr0 |= SDRAM_TR0_SDPA_2_CLK;
		break;
	case 3:
		tr0 |= SDRAM_TR0_SDPA_3_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDPA_4_CLK;
		break;
	}

	/*
	 * Program SD_CTP field
	 */
	t_ras_rcd_clk = sys_info.freqPLB * (t_ras_ns - t_rcd_ns) / ONE_BILLION;
	plb_check = ONE_BILLION * t_ras_rcd_clk / (t_ras_ns - t_rcd_ns);
	if (sys_info.freqPLB != plb_check) {
		t_ras_rcd_clk++;
	}
	switch (t_ras_rcd_clk) {
	case 0:
	case 1:
	case 2:
		tr0 |= SDRAM_TR0_SDCP_2_CLK;
		break;
	case 3:
		tr0 |= SDRAM_TR0_SDCP_3_CLK;
		break;
	case 4:
		tr0 |= SDRAM_TR0_SDCP_4_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDCP_5_CLK;
		break;
	}

	/*
	 * Program SD_LDF field
	 */
	tr0 |= SDRAM_TR0_SDLD_2_CLK;

	/*
	 * Program SD_RFTA field
	 * FIXME tRFC hardcoded as 75 nanoseconds
	 */
	t_rfc_clk = sys_info.freqPLB / (ONE_BILLION / 75);
	residue = sys_info.freqPLB % (ONE_BILLION / 75);
	if (residue >= (ONE_BILLION / 150)) {
		t_rfc_clk++;
	}
	switch (t_rfc_clk) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		tr0 |= SDRAM_TR0_SDRA_6_CLK;
		break;
	case 7:
		tr0 |= SDRAM_TR0_SDRA_7_CLK;
		break;
	case 8:
		tr0 |= SDRAM_TR0_SDRA_8_CLK;
		break;
	case 9:
		tr0 |= SDRAM_TR0_SDRA_9_CLK;
		break;
	case 10:
		tr0 |= SDRAM_TR0_SDRA_10_CLK;
		break;
	case 11:
		tr0 |= SDRAM_TR0_SDRA_11_CLK;
		break;
	case 12:
		tr0 |= SDRAM_TR0_SDRA_12_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDRA_13_CLK;
		break;
	}

	/*
	 * Program SD_RCD field
	 */
	t_rcd_clk = sys_info.freqPLB * t_rcd_ns / ONE_BILLION;
	plb_check = ONE_BILLION * t_rcd_clk / t_rcd_ns;
	if (sys_info.freqPLB != plb_check) {
		t_rcd_clk++;
	}
	switch (t_rcd_clk) {
	case 0:
	case 1:
	case 2:
		tr0 |= SDRAM_TR0_SDRD_2_CLK;
		break;
	case 3:
		tr0 |= SDRAM_TR0_SDRD_3_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDRD_4_CLK;
		break;
	}

	debug("tr0: %x\n", tr0);
	mtsdram(mem_tr0, tr0);
}

static int short_mem_test(void)
{
	unsigned long i, j;
	unsigned long bxcr_num;
	unsigned long *membase;
	const unsigned long test[NUMMEMTESTS][NUMMEMWORDS] = {
		{0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF},
		{0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000},
		{0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		 0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555},
		{0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		 0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA},
		{0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		 0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A},
		{0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		 0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5},
		{0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		 0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA},
		{0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
		 0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55}};

	for (bxcr_num = 0; bxcr_num < MAXBXCR; bxcr_num++) {
		mtdcr(memcfga, mem_b0cr + (bxcr_num << 2));
		if ((mfdcr(memcfgd) & SDRAM_BXCR_SDBE) == SDRAM_BXCR_SDBE) {
			/* Bank is enabled */
			membase = (unsigned long*)
				(mfdcr(memcfgd) & SDRAM_BXCR_SDBA_MASK);

			/*
			 * Run the short memory test
			 */
			for (i = 0; i < NUMMEMTESTS; i++) {
				for (j = 0; j < NUMMEMWORDS; j++) {
					/* printf("bank enabled base:%x\n", &membase[j]); */
					membase[j] = test[i][j];
					ppcDcbf((unsigned long)&(membase[j]));
				}

				for (j = 0; j < NUMMEMWORDS; j++) {
					if (membase[j] != test[i][j]) {
						ppcDcbf((unsigned long)&(membase[j]));
						return 0;
					}
					ppcDcbf((unsigned long)&(membase[j]));
				}

				if (j < NUMMEMWORDS)
					return 0;
			}

			/*
			 * see if the rdclt value passed
			 */
			if (i < NUMMEMTESTS)
				return 0;
		}
	}

	return 1;
}

static void program_tr1(void)
{
	unsigned long tr0;
	unsigned long tr1;
	unsigned long cfg0;
	unsigned long ecc_temp;
	unsigned long dlycal;
	unsigned long dly_val;
	unsigned long k;
	unsigned long max_pass_length;
	unsigned long current_pass_length;
	unsigned long current_fail_length;
	unsigned long current_start;
	unsigned long rdclt;
	unsigned long rdclt_offset;
	long max_start;
	long max_end;
	long rdclt_average;
	unsigned char window_found;
	unsigned char fail_found;
	unsigned char pass_found;
	PPC4xx_SYS_INFO sys_info;

	/*
	 * get the board info
	 */
	get_sys_info(&sys_info);

	/*
	 * get SDRAM Timing Register 0 (SDRAM_TR0) and clear bits
	 */
	mfsdram(mem_tr1, tr1);
	tr1 &= ~(SDRAM_TR1_RDSS_MASK | SDRAM_TR1_RDSL_MASK |
		 SDRAM_TR1_RDCD_MASK | SDRAM_TR1_RDCT_MASK);

	mfsdram(mem_tr0, tr0);
	if (((tr0 & SDRAM_TR0_SDCL_MASK) == SDRAM_TR0_SDCL_2_5_CLK) &&
	    (sys_info.freqPLB > 100000000)) {
		tr1 |= SDRAM_TR1_RDSS_TR2;
		tr1 |= SDRAM_TR1_RDSL_STAGE3;
		tr1 |= SDRAM_TR1_RDCD_RCD_1_2;
	} else {
		tr1 |= SDRAM_TR1_RDSS_TR1;
		tr1 |= SDRAM_TR1_RDSL_STAGE2;
		tr1 |= SDRAM_TR1_RDCD_RCD_0_0;
	}

	/*
	 * save CFG0 ECC setting to a temporary variable and turn ECC off
	 */
	mfsdram(mem_cfg0, cfg0);
	ecc_temp = cfg0 & SDRAM_CFG0_MCHK_MASK;
	mtsdram(mem_cfg0, (cfg0 & ~SDRAM_CFG0_MCHK_MASK) | SDRAM_CFG0_MCHK_NON);

	/*
	 * get the delay line calibration register value
	 */
	mfsdram(mem_dlycal, dlycal);
	dly_val = SDRAM_DLYCAL_DLCV_DECODE(dlycal) << 2;

	max_pass_length = 0;
	max_start = 0;
	max_end = 0;
	current_pass_length = 0;
	current_fail_length = 0;
	current_start = 0;
	rdclt_offset = 0;
	window_found = FALSE;
	fail_found = FALSE;
	pass_found = FALSE;
	debug("Starting memory test ");

	for (k = 0; k < NUMHALFCYCLES; k++) {
		for (rdclt = 0; rdclt < dly_val; rdclt++) {
			/*
			 * Set the timing reg for the test.
			 */
			mtsdram(mem_tr1, (tr1 | SDRAM_TR1_RDCT_ENCODE(rdclt)));

			if (short_mem_test()) {
				if (fail_found == TRUE) {
					pass_found = TRUE;
					if (current_pass_length == 0) {
						current_start = rdclt_offset + rdclt;
					}

					current_fail_length = 0;
					current_pass_length++;

					if (current_pass_length > max_pass_length) {
						max_pass_length = current_pass_length;
						max_start = current_start;
						max_end = rdclt_offset + rdclt;
					}
				}
			} else {
				current_pass_length = 0;
				current_fail_length++;

				if (current_fail_length >= (dly_val>>2)) {
					if (fail_found == FALSE) {
						fail_found = TRUE;
					} else if (pass_found == TRUE) {
						window_found = TRUE;
						break;
					}
				}
			}
		}
		debug(".");

		if (window_found == TRUE) {
			break;
		}

		tr1 = tr1 ^ SDRAM_TR1_RDCD_MASK;
		rdclt_offset += dly_val;
	}
	debug("\n");

	/*
	 * make sure we find the window
	 */
	if (window_found == FALSE) {
		printf("ERROR: Cannot determine a common read delay.\n");
		spd_ddr_init_hang ();
	}

	/*
	 * restore the orignal ECC setting
	 */
	mtsdram(mem_cfg0, (cfg0 & ~SDRAM_CFG0_MCHK_MASK) | ecc_temp);

	/*
	 * set the SDRAM TR1 RDCD value
	 */
	tr1 &= ~SDRAM_TR1_RDCD_MASK;
	if ((tr0 & SDRAM_TR0_SDCL_MASK) == SDRAM_TR0_SDCL_2_5_CLK) {
		tr1 |= SDRAM_TR1_RDCD_RCD_1_2;
	} else {
		tr1 |= SDRAM_TR1_RDCD_RCD_0_0;
	}

	/*
	 * set the SDRAM TR1 RDCLT value
	 */
	tr1 &= ~SDRAM_TR1_RDCT_MASK;
	while (max_end >= (dly_val << 1)) {
		max_end -= (dly_val << 1);
		max_start -= (dly_val << 1);
	}

	rdclt_average = ((max_start + max_end) >> 1);

	if (rdclt_average < 0) {
		rdclt_average = 0;
	}

	if (rdclt_average >= dly_val) {
		rdclt_average -= dly_val;
		tr1 = tr1 ^ SDRAM_TR1_RDCD_MASK;
	}
	tr1 |= SDRAM_TR1_RDCT_ENCODE(rdclt_average);

	debug("tr1: %x\n", tr1);

	/*
	 * program SDRAM Timing Register 1 TR1
	 */
	mtsdram(mem_tr1, tr1);
}

static unsigned long program_bxcr(unsigned long *dimm_populated,
				  unsigned char *iic0_dimm_addr,
				  unsigned long num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long bank_base_addr;
	unsigned long cr;
	unsigned long i;
	unsigned long j;
	unsigned long temp;
	unsigned char num_row_addr;
	unsigned char num_col_addr;
	unsigned char num_banks;
	unsigned char bank_size_id;
	unsigned long ctrl_bank_num[MAXBANKS];
	unsigned long bx_cr_num;
	unsigned long largest_size_index;
	unsigned long largest_size;
	unsigned long current_size_index;
	BANKPARMS bank_parms[MAXBXCR];
	unsigned long sorted_bank_num[MAXBXCR]; /* DDR Controller bank number table (sorted by size) */
	unsigned long sorted_bank_size[MAXBXCR]; /* DDR Controller bank size table (sorted by size)*/

	/*
	 * Set the BxCR regs.  First, wipe out the bank config registers.
	 */
	for (bx_cr_num = 0; bx_cr_num < MAXBXCR; bx_cr_num++) {
		mtdcr(memcfga, mem_b0cr + (bx_cr_num << 2));
		mtdcr(memcfgd, 0x00000000);
		bank_parms[bx_cr_num].bank_size_bytes = 0;
	}

#ifdef CONFIG_BAMBOO
	/*
	 * This next section is hardware dependent and must be programmed
	 * to match the hardware.  For bamboo, the following holds...
	 * 1. SDRAM0_B0CR: Bank 0 of dimm 0 ctrl_bank_num : 0 (soldered onboard)
	 * 2. SDRAM0_B1CR: Bank 0 of dimm 1 ctrl_bank_num : 1
	 * 3. SDRAM0_B2CR: Bank 1 of dimm 1 ctrl_bank_num : 1
	 * 4. SDRAM0_B3CR: Bank 0 of dimm 2 ctrl_bank_num : 3
	 * ctrl_bank_num corresponds to the first usable DDR controller bank number by DIMM
	 */
	ctrl_bank_num[0] = 0;
	ctrl_bank_num[1] = 1;
	ctrl_bank_num[2] = 3;
#else
	/*
	 * Ocotea, Ebony and the other IBM/AMCC eval boards have
	 * 2 DIMM slots with each max 2 banks
	 */
	ctrl_bank_num[0] = 0;
	ctrl_bank_num[1] = 2;
#endif

	/*
	 * reset the bank_base address
	 */
	bank_base_addr = CONFIG_SYS_SDRAM_BASE;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			num_row_addr = spd_read(iic0_dimm_addr[dimm_num], 3);
			num_col_addr = spd_read(iic0_dimm_addr[dimm_num], 4);
			num_banks    = spd_read(iic0_dimm_addr[dimm_num], 5);
			bank_size_id = spd_read(iic0_dimm_addr[dimm_num], 31);
			debug("DIMM%d: row=%d col=%d banks=%d\n", dimm_num,
			      num_row_addr, num_col_addr, num_banks);

			/*
			 * Set the SDRAM0_BxCR regs
			 */
			cr = 0;
			switch (bank_size_id) {
			case 0x02:
				cr |= SDRAM_BXCR_SDSZ_8;
				break;
			case 0x04:
				cr |= SDRAM_BXCR_SDSZ_16;
				break;
			case 0x08:
				cr |= SDRAM_BXCR_SDSZ_32;
				break;
			case 0x10:
				cr |= SDRAM_BXCR_SDSZ_64;
				break;
			case 0x20:
				cr |= SDRAM_BXCR_SDSZ_128;
				break;
			case 0x40:
				cr |= SDRAM_BXCR_SDSZ_256;
				break;
			case 0x80:
				cr |= SDRAM_BXCR_SDSZ_512;
				break;
			default:
				printf("DDR-SDRAM: DIMM %lu BxCR configuration.\n",
				       dimm_num);
				printf("ERROR: Unsupported value for the banksize: %d.\n",
				       bank_size_id);
				printf("Replace the DIMM module with a supported DIMM.\n\n");
				spd_ddr_init_hang ();
			}

			switch (num_col_addr) {
			case 0x08:
				cr |= SDRAM_BXCR_SDAM_1;
				break;
			case 0x09:
				cr |= SDRAM_BXCR_SDAM_2;
				break;
			case 0x0A:
				cr |= SDRAM_BXCR_SDAM_3;
				break;
			case 0x0B:
				cr |= SDRAM_BXCR_SDAM_4;
				break;
			default:
				printf("DDR-SDRAM: DIMM %lu BxCR configuration.\n",
				       dimm_num);
				printf("ERROR: Unsupported value for number of "
				       "column addresses: %d.\n", num_col_addr);
				printf("Replace the DIMM module with a supported DIMM.\n\n");
				spd_ddr_init_hang ();
			}

			/*
			 * enable the bank
			 */
			cr |= SDRAM_BXCR_SDBE;

			for (i = 0; i < num_banks; i++) {
				bank_parms[ctrl_bank_num[dimm_num]+i].bank_size_bytes =
					(4 << 20) * bank_size_id;
				bank_parms[ctrl_bank_num[dimm_num]+i].cr = cr;
				debug("DIMM%d-bank %d (SDRAM0_B%dCR): bank_size_bytes=%d\n",
				      dimm_num, i, ctrl_bank_num[dimm_num]+i,
				      bank_parms[ctrl_bank_num[dimm_num]+i].bank_size_bytes);
			}
		}
	}

	/* Initialize sort tables */
	for (i = 0; i < MAXBXCR; i++) {
		sorted_bank_num[i] = i;
		sorted_bank_size[i] = bank_parms[i].bank_size_bytes;
	}

	for (i = 0; i < MAXBXCR-1; i++) {
		largest_size = sorted_bank_size[i];
		largest_size_index = 255;

		/* Find the largest remaining value */
		for (j = i + 1; j < MAXBXCR; j++) {
			if (sorted_bank_size[j] > largest_size) {
				/* Save largest remaining value and its index */
				largest_size = sorted_bank_size[j];
				largest_size_index = j;
			}
		}

		if (largest_size_index != 255) {
			/* Swap the current and largest values */
			current_size_index = sorted_bank_num[largest_size_index];
			sorted_bank_size[largest_size_index] = sorted_bank_size[i];
			sorted_bank_size[i] = largest_size;
			sorted_bank_num[largest_size_index] = sorted_bank_num[i];
			sorted_bank_num[i] = current_size_index;
		}
	}

	/* Set the SDRAM0_BxCR regs thanks to sort tables */
	for (bx_cr_num = 0, bank_base_addr = 0; bx_cr_num < MAXBXCR; bx_cr_num++) {
		if (bank_parms[sorted_bank_num[bx_cr_num]].bank_size_bytes) {
			mtdcr(memcfga, mem_b0cr + (sorted_bank_num[bx_cr_num] << 2));
			temp = mfdcr(memcfgd) & ~(SDRAM_BXCR_SDBA_MASK | SDRAM_BXCR_SDSZ_MASK |
						  SDRAM_BXCR_SDAM_MASK | SDRAM_BXCR_SDBE);
			temp = temp | (bank_base_addr & SDRAM_BXCR_SDBA_MASK) |
				bank_parms[sorted_bank_num[bx_cr_num]].cr;
			mtdcr(memcfgd, temp);
			bank_base_addr += bank_parms[sorted_bank_num[bx_cr_num]].bank_size_bytes;
			debug("SDRAM0_B%dCR=0x%08lx\n", sorted_bank_num[bx_cr_num], temp);
		}
	}

	return(bank_base_addr);
}
#endif /* CONFIG_SPD_EEPROM */
