/*
 * Copyright 2004, 2007 Freescale Semiconductor.
 * (C) Copyright 2003 Motorola Inc.
 * Xianghua Xiao (X.Xiao@motorola.com)
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

#include <common.h>
#include <asm/processor.h>
#include <i2c.h>
#include <spd.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>


#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void dma_init(void);
extern uint dma_check(void);
extern int dma_xfer(void *dest, uint count, void *src);
#endif

#ifdef CONFIG_SPD_EEPROM

#ifndef	CFG_READ_SPD
#define CFG_READ_SPD	i2c_read
#endif

static unsigned int setup_laws_and_tlbs(unsigned int memsize);


/*
 * Convert picoseconds into clock cycles (rounding up if needed).
 */

int
picos_to_clk(int picos)
{
	int clks;

	clks = picos / (2000000000 / (get_ddr_freq(0) / 1000));
	if (picos % (2000000000 / (get_ddr_freq(0) / 1000)) != 0) {
		clks++;
	}

	return clks;
}


/*
 * Calculate the Density of each Physical Rank.
 * Returned size is in bytes.
 *
 * Study these table from Byte 31 of JEDEC SPD Spec.
 *
 *		DDR I	DDR II
 *	Bit	Size	Size
 *	---	-----	------
 *	7 high	512MB	512MB
 *	6	256MB	256MB
 *	5	128MB	128MB
 *	4	 64MB	 16GB
 *	3	 32MB	  8GB
 *	2	 16MB	  4GB
 *	1	  2GB	  2GB
 *	0 low	  1GB	  1GB
 *
 * Reorder Table to be linear by stripping the bottom
 * 2 or 5 bits off and shifting them up to the top.
 */

unsigned int
compute_banksize(unsigned int mem_type, unsigned char row_dens)
{
	unsigned int bsize;

	if (mem_type == SPD_MEMTYPE_DDR) {
		/* Bottom 2 bits up to the top. */
		bsize = ((row_dens >> 2) | ((row_dens & 3) << 6)) << 24;
		debug("DDR: DDR I rank density = 0x%08x\n", bsize);
	} else {
		/* Bottom 5 bits up to the top. */
		bsize = ((row_dens >> 5) | ((row_dens & 31) << 3)) << 27;
		debug("DDR: DDR II rank density = 0x%08x\n", bsize);
	}
	return bsize;
}


/*
 * Convert a two-nibble BCD value into a cycle time.
 * While the spec calls for nano-seconds, picos are returned.
 *
 * This implements the tables for bytes 9, 23 and 25 for both
 * DDR I and II.  No allowance for distinguishing the invalid
 * fields absent for DDR I yet present in DDR II is made.
 * (That is, cycle times of .25, .33, .66 and .75 ns are
 * allowed for both DDR II and I.)
 */

unsigned int
convert_bcd_tenths_to_cycle_time_ps(unsigned int spd_val)
{
	/*
	 * Table look up the lower nibble, allow DDR I & II.
	 */
	unsigned int tenths_ps[16] = {
		0,
		100,
		200,
		300,
		400,
		500,
		600,
		700,
		800,
		900,
		250,
		330,
		660,
		750,
		0,	/* undefined */
		0	/* undefined */
	};

	unsigned int whole_ns = (spd_val & 0xF0) >> 4;
	unsigned int tenth_ns = spd_val & 0x0F;
	unsigned int ps = whole_ns * 1000 + tenths_ps[tenth_ns];

	return ps;
}


/*
 * Determine Refresh Rate.  Ignore self refresh bit on DDR I.
 * Table from SPD Spec, Byte 12, converted to picoseconds and
 * filled in with "default" normal values.
 */
unsigned int determine_refresh_rate(unsigned int spd_refresh)
{
	unsigned int refresh_time_ns[8] = {
		15625000,	/* 0 Normal    1.00x */
		3900000,	/* 1 Reduced    .25x */
		7800000,	/* 2 Extended   .50x */
		31300000,	/* 3 Extended  2.00x */
		62500000,	/* 4 Extended  4.00x */
		125000000,	/* 5 Extended  8.00x */
		15625000,	/* 6 Normal    1.00x  filler */
		15625000,	/* 7 Normal    1.00x  filler */
	};

	return picos_to_clk(refresh_time_ns[spd_refresh & 0x7]);
}


long int
spd_sdram(void)
{
	volatile ccsr_ddr_t *ddr = (void *)(CFG_MPC85xx_DDR_ADDR);
	spd_eeprom_t spd;
	unsigned int n_ranks;
	unsigned int rank_density;
	unsigned int odt_rd_cfg, odt_wr_cfg, ba_bits;
	unsigned int odt_cfg, mode_odt_enable;
	unsigned int refresh_clk;
#ifdef MPC85xx_DDR_SDRAM_CLK_CNTL
	unsigned char clk_adjust;
#endif
	unsigned int dqs_cfg;
	unsigned char twr_clk, twtr_clk, twr_auto_clk;
	unsigned int tCKmin_ps, tCKmax_ps;
	unsigned int max_data_rate, effective_data_rate;
	unsigned int busfreq;
	unsigned sdram_cfg;
	unsigned int memsize = 0;
	unsigned char caslat, caslat_ctrl;
	unsigned int trfc, trfc_clk, trfc_low, trfc_high;
	unsigned int trcd_clk;
	unsigned int trtp_clk;
	unsigned char cke_min_clk;
	unsigned char add_lat;
	unsigned char wr_lat;
	unsigned char wr_data_delay;
	unsigned char four_act;
	unsigned char cpo;
	unsigned char burst_len;
	unsigned int mode_caslat;
	unsigned char sdram_type;
	unsigned char d_init;
	unsigned int bnds;

	/*
	 * Skip configuration if already configured.
	 * memsize is determined from last configured chip select.
	 */
	if (ddr->cs0_config & 0x80000000) {
		debug(" cs0 already configured, bnds=%x\n",ddr->cs0_bnds);
		bnds = 0xfff & ddr->cs0_bnds;
		if (bnds < 0xff) { /* do not add if at top of 4G */
			memsize = (bnds + 1) << 4;
		}
	}
	if (ddr->cs1_config & 0x80000000) {
		debug(" cs1 already configured, bnds=%x\n",ddr->cs1_bnds);
		bnds = 0xfff & ddr->cs1_bnds;
		if (bnds < 0xff) { /* do not add if at top of 4G */
			memsize = (bnds + 1) << 4; /* assume ordered bnds */
		}
	}
	if (ddr->cs2_config & 0x80000000) {
		debug(" cs2 already configured, bnds=%x\n",ddr->cs2_bnds);
		bnds = 0xfff & ddr->cs2_bnds;
		if (bnds < 0xff) { /* do not add if at top of 4G */
			memsize = (bnds + 1) << 4;
		}
	}
	if (ddr->cs3_config & 0x80000000) {
		debug(" cs3 already configured, bnds=%x\n",ddr->cs3_bnds);
		bnds = 0xfff & ddr->cs3_bnds;
		if (bnds < 0xff) { /* do not add if at top of 4G */
			memsize = (bnds + 1) << 4;
		}
	}

	if (memsize) {
		printf("       Reusing current %dMB configuration\n",memsize);
		memsize = setup_laws_and_tlbs(memsize);
		return memsize << 20;
	}

	/*
	 * Read SPD information.
	 */
	CFG_READ_SPD(SPD_EEPROM_ADDRESS, 0, 1, (uchar *) &spd, sizeof(spd));

	/*
	 * Check for supported memory module types.
	 */
	if (spd.mem_type != SPD_MEMTYPE_DDR &&
	    spd.mem_type != SPD_MEMTYPE_DDR2) {
		printf("Unable to locate DDR I or DDR II module.\n"
		       "    Fundamental memory type is 0x%0x\n",
		       spd.mem_type);
		return 0;
	}

	/*
	 * These test gloss over DDR I and II differences in interpretation
	 * of bytes 3 and 4, but irrelevantly.  Multiple asymmetric banks
	 * are not supported on DDR I; and not encoded on DDR II.
	 *
	 * Also note that the 8548 controller can support:
	 *    12 <= nrow <= 16
	 * and
	 *     8 <= ncol <= 11 (still, for DDR)
	 *     6 <= ncol <=  9 (for FCRAM)
	 */
	if (spd.nrow_addr < 12 || spd.nrow_addr > 14) {
		printf("DDR: Unsupported number of Row Addr lines: %d.\n",
		       spd.nrow_addr);
		return 0;
	}
	if (spd.ncol_addr < 8 || spd.ncol_addr > 11) {
		printf("DDR: Unsupported number of Column Addr lines: %d.\n",
		       spd.ncol_addr);
		return 0;
	}

	/*
	 * Determine the number of physical banks controlled by
	 * different Chip Select signals.  This is not quite the
	 * same as the number of DIMM modules on the board.  Feh.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		n_ranks = spd.nrows;
	} else {
		n_ranks = (spd.nrows & 0x7) + 1;
	}

	debug("DDR: number of ranks = %d\n", n_ranks);

	if (n_ranks > 2) {
		printf("DDR: Only 2 chip selects are supported: %d\n",
		       n_ranks);
		return 0;
	}

#ifdef CONFIG_MPC8548
	/*
	 * Adjust DDR II IO voltage biasing.
	 * Only 8548 rev 1 needs the fix
	 */
	if ((SVR_SOC_VER(get_svr()) == SVR_8548_E) &&
			(SVR_MJREV(get_svr()) == 1) &&
			(spd.mem_type == SPD_MEMTYPE_DDR2)) {
		volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
		gur->ddrioovcr = (0x80000000	/* Enable */
				  | 0x10000000);/* VSEL to 1.8V */
	}
#endif

	/*
	 * Determine the size of each Rank in bytes.
	 */
	rank_density = compute_banksize(spd.mem_type, spd.row_dens);


	/*
	 * Eg: Bounds: 0x0000_0000 to 0x0f000_0000	first 256 Meg
	 */
	ddr->cs0_bnds = (rank_density >> 24) - 1;

	/*
	 * ODT configuration recommendation from DDR Controller Chapter.
	 */
	odt_rd_cfg = 0;			/* Never assert ODT */
	odt_wr_cfg = 0;			/* Never assert ODT */
	if (spd.mem_type == SPD_MEMTYPE_DDR2) {
		odt_wr_cfg = 1;		/* Assert ODT on writes to CS0 */
#if 0
		/* FIXME: How to determine the number of dimm modules? */
		if (n_dimm_modules == 2) {
			odt_rd_cfg = 1;	/* Assert ODT on reads to CS0 */
		}
#endif
	}

	ba_bits = 0;
	if (spd.nbanks == 0x8)
		ba_bits = 1;

	ddr->cs0_config = ( 1 << 31
			    | (odt_rd_cfg << 20)
			    | (odt_wr_cfg << 16)
			    | (ba_bits << 14)
			    | (spd.nrow_addr - 12) << 8
			    | (spd.ncol_addr - 8) );
	debug("\n");
	debug("DDR: cs0_bnds   = 0x%08x\n", ddr->cs0_bnds);
	debug("DDR: cs0_config = 0x%08x\n", ddr->cs0_config);

	if (n_ranks == 2) {
		/*
		 * Eg: Bounds: 0x0f00_0000 to 0x1e0000_0000, second 256 Meg
		 */
		ddr->cs1_bnds = ( (rank_density >> 8)
				  | ((rank_density >> (24 - 1)) - 1) );
		ddr->cs1_config = ( 1<<31
				    | (odt_rd_cfg << 20)
				    | (odt_wr_cfg << 16)
				    | (spd.nrow_addr - 12) << 8
				    | (spd.ncol_addr - 8) );
		debug("DDR: cs1_bnds   = 0x%08x\n", ddr->cs1_bnds);
		debug("DDR: cs1_config = 0x%08x\n", ddr->cs1_config);
	}


	/*
	 * Find the largest CAS by locating the highest 1 bit
	 * in the spd.cas_lat field.  Translate it to a DDR
	 * controller field value:
	 *
	 *	CAS Lat	DDR I	DDR II	Ctrl
	 *	Clocks	SPD Bit	SPD Bit	Value
	 *	-------	-------	-------	-----
	 *	1.0	0		0001
	 *	1.5	1		0010
	 *	2.0	2	2	0011
	 *	2.5	3		0100
	 *	3.0	4	3	0101
	 *	3.5	5		0110
	 *	4.0		4	0111
	 *	4.5			1000
	 *	5.0		5	1001
	 */
	caslat = __ilog2(spd.cas_lat);
	if ((spd.mem_type == SPD_MEMTYPE_DDR)
	    && (caslat > 5)) {
		printf("DDR I: Invalid SPD CAS Latency: 0x%x.\n", spd.cas_lat);
		return 0;

	} else if (spd.mem_type == SPD_MEMTYPE_DDR2
		   && (caslat < 2 || caslat > 5)) {
		printf("DDR II: Invalid SPD CAS Latency: 0x%x.\n",
		       spd.cas_lat);
		return 0;
	}
	debug("DDR: caslat SPD bit is %d\n", caslat);

	/*
	 * Calculate the Maximum Data Rate based on the Minimum Cycle time.
	 * The SPD clk_cycle field (tCKmin) is measured in tenths of
	 * nanoseconds and represented as BCD.
	 */
	tCKmin_ps = convert_bcd_tenths_to_cycle_time_ps(spd.clk_cycle);
	debug("DDR: tCKmin = %d ps\n", tCKmin_ps);

	/*
	 * Double-data rate, scaled 1000 to picoseconds, and back down to MHz.
	 */
	max_data_rate = 2 * 1000 * 1000 / tCKmin_ps;
	debug("DDR: Module max data rate = %d Mhz\n", max_data_rate);


	/*
	 * Adjust the CAS Latency to allow for bus speeds that
	 * are slower than the DDR module.
	 */
	busfreq = get_ddr_freq(0) / 1000000;	/* MHz */

	effective_data_rate = max_data_rate;
	if (busfreq < 90) {
		/* DDR rate out-of-range */
		puts("DDR: platform frequency is not fit for DDR rate\n");
		return 0;

	} else if (90 <= busfreq && busfreq < 230 && max_data_rate >= 230) {
		/*
		 * busfreq 90~230 range, treated as DDR 200.
		 */
		effective_data_rate = 200;
		if (spd.clk_cycle3 == 0xa0)	/* 10 ns */
			caslat -= 2;
		else if (spd.clk_cycle2 == 0xa0)
			caslat--;

	} else if (230 <= busfreq && busfreq < 280 && max_data_rate >= 280) {
		/*
		 * busfreq 230~280 range, treated as DDR 266.
		 */
		effective_data_rate = 266;
		if (spd.clk_cycle3 == 0x75)	/* 7.5 ns */
			caslat -= 2;
		else if (spd.clk_cycle2 == 0x75)
			caslat--;

	} else if (280 <= busfreq && busfreq < 350 && max_data_rate >= 350) {
		/*
		 * busfreq 280~350 range, treated as DDR 333.
		 */
		effective_data_rate = 333;
		if (spd.clk_cycle3 == 0x60)	/* 6.0 ns */
			caslat -= 2;
		else if (spd.clk_cycle2 == 0x60)
			caslat--;

	} else if (350 <= busfreq && busfreq < 460 && max_data_rate >= 460) {
		/*
		 * busfreq 350~460 range, treated as DDR 400.
		 */
		effective_data_rate = 400;
		if (spd.clk_cycle3 == 0x50)	/* 5.0 ns */
			caslat -= 2;
		else if (spd.clk_cycle2 == 0x50)
			caslat--;

	} else if (460 <= busfreq && busfreq < 560 && max_data_rate >= 560) {
		/*
		 * busfreq 460~560 range, treated as DDR 533.
		 */
		effective_data_rate = 533;
		if (spd.clk_cycle3 == 0x3D)	/* 3.75 ns */
			caslat -= 2;
		else if (spd.clk_cycle2 == 0x3D)
			caslat--;

	} else if (560 <= busfreq && busfreq < 700 && max_data_rate >= 700) {
		/*
		 * busfreq 560~700 range, treated as DDR 667.
		 */
		effective_data_rate = 667;
		if (spd.clk_cycle3 == 0x30)	/* 3.0 ns */
			caslat -= 2;
		else if (spd.clk_cycle2 == 0x30)
			caslat--;

	} else if (700 <= busfreq) {
		/*
		 * DDR rate out-of-range
		 */
		printf("DDR: Bus freq %d MHz is not fit for DDR rate %d MHz\n",
		     busfreq, max_data_rate);
		return 0;
	}


	/*
	 * Convert caslat clocks to DDR controller value.
	 * Force caslat_ctrl to be DDR Controller field-sized.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		caslat_ctrl = (caslat + 1) & 0x07;
	} else {
		caslat_ctrl =  (2 * caslat - 1) & 0x0f;
	}

	debug("DDR: effective data rate is %d MHz\n", effective_data_rate);
	debug("DDR: caslat SPD bit is %d, controller field is 0x%x\n",
	      caslat, caslat_ctrl);

	/*
	 * Timing Config 0.
	 * Avoid writing for DDR I.  The new PQ38 DDR controller
	 * dreams up non-zero default values to be backwards compatible.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR2) {
		unsigned char taxpd_clk = 8;		/* By the book. */
		unsigned char tmrd_clk = 2;		/* By the book. */
		unsigned char act_pd_exit = 2;		/* Empirical? */
		unsigned char pre_pd_exit = 6;		/* Empirical? */

		ddr->timing_cfg_0 = (0
			| ((act_pd_exit & 0x7) << 20)	/* ACT_PD_EXIT */
			| ((pre_pd_exit & 0x7) << 16)	/* PRE_PD_EXIT */
			| ((taxpd_clk & 0xf) << 8)	/* ODT_PD_EXIT */
			| ((tmrd_clk & 0xf) << 0)	/* MRS_CYC */
			);
#if 0
		ddr->timing_cfg_0 |= 0xaa000000;	/* extra cycles */
#endif
		debug("DDR: timing_cfg_0 = 0x%08x\n", ddr->timing_cfg_0);

	} else {
#if 0
		/*
		 * Force extra cycles with 0xaa bits.
		 * Incidentally supply the dreamt-up backwards compat value!
		 */
		ddr->timing_cfg_0 = 0x00110105;	/* backwards compat value */
		ddr->timing_cfg_0 |= 0xaa000000;	/* extra cycles */
		debug("DDR: HACK timing_cfg_0 = 0x%08x\n", ddr->timing_cfg_0);
#endif
	}


	/*
	 * Some Timing Config 1 values now.
	 * Sneak Extended Refresh Recovery in here too.
	 */

	/*
	 * For DDR I, WRREC(Twr) and WRTORD(Twtr) are not in SPD,
	 * use conservative value.
	 * For DDR II, they are bytes 36 and 37, in quarter nanos.
	 */

	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		twr_clk = 3;	/* Clocks */
		twtr_clk = 1;	/* Clocks */
	} else {
		twr_clk = picos_to_clk(spd.twr * 250);
		twtr_clk = picos_to_clk(spd.twtr * 250);
	}

	/*
	 * Calculate Trfc, in picos.
	 * DDR I:  Byte 42 straight up in ns.
	 * DDR II: Byte 40 and 42 swizzled some, in ns.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		trfc = spd.trfc * 1000;		/* up to ps */
	} else {
		unsigned int byte40_table_ps[8] = {
			0,
			250,
			330,
			500,
			660,
			750,
			0,
			0
		};

		trfc = (((spd.trctrfc_ext & 0x1) * 256) + spd.trfc) * 1000
			+ byte40_table_ps[(spd.trctrfc_ext >> 1) & 0x7];
	}
	trfc_clk = picos_to_clk(trfc);

	/*
	 * Trcd, Byte 29, from quarter nanos to ps and clocks.
	 */
	trcd_clk = picos_to_clk(spd.trcd * 250) & 0x7;

	/*
	 * Convert trfc_clk to DDR controller fields.  DDR I should
	 * fit in the REFREC field (16-19) of TIMING_CFG_1, but the
	 * 8548 controller has an extended REFREC field of three bits.
	 * The controller automatically adds 8 clocks to this value,
	 * so preadjust it down 8 first before splitting it up.
	 */
	trfc_low = (trfc_clk - 8) & 0xf;
	trfc_high = ((trfc_clk - 8) >> 4) & 0x3;

	/*
	 * Sneak in some Extended Refresh Recovery.
	 */
	ddr->timing_cfg_3 = (trfc_high << 16);
	debug("DDR: timing_cfg_3 = 0x%08x\n", ddr->timing_cfg_3);

	ddr->timing_cfg_1 =
	    (0
	     | ((picos_to_clk(spd.trp * 250) & 0x07) << 28)	/* PRETOACT */
	     | ((picos_to_clk(spd.tras * 1000) & 0x0f ) << 24)	/* ACTTOPRE */
	     | (trcd_clk << 20)					/* ACTTORW */
	     | (caslat_ctrl << 16)				/* CASLAT */
	     | (trfc_low << 12)					/* REFEC */
	     | ((twr_clk & 0x07) << 8)				/* WRRREC */
	     | ((picos_to_clk(spd.trrd * 250) & 0x07) << 4)	/* ACTTOACT */
	     | ((twtr_clk & 0x07) << 0)				/* WRTORD */
	     );

	debug("DDR: timing_cfg_1  = 0x%08x\n", ddr->timing_cfg_1);


	/*
	 * Timing_Config_2
	 * Was: 0x00000800;
	 */

	/*
	 * Additive Latency
	 * For DDR I, 0.
	 * For DDR II, with ODT enabled, use "a value" less than ACTTORW,
	 * which comes from Trcd, and also note that:
	 *	add_lat + caslat must be >= 4
	 */
	add_lat = 0;
	if (spd.mem_type == SPD_MEMTYPE_DDR2
	    && (odt_wr_cfg || odt_rd_cfg)
	    && (caslat < 4)) {
		add_lat = 4 - caslat;
		if (add_lat > trcd_clk) {
			add_lat = trcd_clk - 1;
		}
	}

	/*
	 * Write Data Delay
	 * Historically 0x2 == 4/8 clock delay.
	 * Empirically, 0x3 == 6/8 clock delay is suggested for DDR I 266.
	 */
	wr_data_delay = 3;

	/*
	 * Write Latency
	 * Read to Precharge
	 * Minimum CKE Pulse Width.
	 * Four Activate Window
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		/*
		 * This is a lie.  It should really be 1, but if it is
		 * set to 1, bits overlap into the old controller's
		 * otherwise unused ACSM field.  If we leave it 0, then
		 * the HW will magically treat it as 1 for DDR 1.  Oh Yea.
		 */
		wr_lat = 0;

		trtp_clk = 2;		/* By the book. */
		cke_min_clk = 1;	/* By the book. */
		four_act = 1;		/* By the book. */

	} else {
		wr_lat = caslat - 1;

		/* Convert SPD value from quarter nanos to picos. */
		trtp_clk = picos_to_clk(spd.trtp * 250);

		cke_min_clk = 3;	/* By the book. */
		four_act = picos_to_clk(37500);	/* By the book. 1k pages? */
	}

	/*
	 * Empirically set ~MCAS-to-preamble override for DDR 2.
	 * Your milage will vary.
	 */
	cpo = 0;
	if (spd.mem_type == SPD_MEMTYPE_DDR2) {
		if (effective_data_rate <= 333) {
			cpo = 0x7;		/* READ_LAT + 5/4 */
		} else {
			cpo = 0x9;		/* READ_LAT + 7/4 */
		}
	}

	ddr->timing_cfg_2 = (0
		| ((add_lat & 0x7) << 28)		/* ADD_LAT */
		| ((cpo & 0x1f) << 23)			/* CPO */
		| ((wr_lat & 0x7) << 19)		/* WR_LAT */
		| ((trtp_clk & 0x7) << 13)		/* RD_TO_PRE */
		| ((wr_data_delay & 0x7) << 10)		/* WR_DATA_DELAY */
		| ((cke_min_clk & 0x7) << 6)		/* CKE_PLS */
		| ((four_act & 0x1f) << 0)		/* FOUR_ACT */
		);

	debug("DDR: timing_cfg_2 = 0x%08x\n", ddr->timing_cfg_2);


	/*
	 * Determine the Mode Register Set.
	 *
	 * This is nominally part specific, but it appears to be
	 * consistent for all DDR I devices, and for all DDR II devices.
	 *
	 *     caslat must be programmed
	 *     burst length is always 4
	 *     burst type is sequential
	 *
	 * For DDR I:
	 *     operating mode is "normal"
	 *
	 * For DDR II:
	 *     other stuff
	 */

	mode_caslat = 0;

	/*
	 * Table lookup from DDR I or II Device Operation Specs.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		if (1 <= caslat && caslat <= 4) {
			unsigned char mode_caslat_table[4] = {
				0x5,	/* 1.5 clocks */
				0x2,	/* 2.0 clocks */
				0x6,	/* 2.5 clocks */
				0x3	/* 3.0 clocks */
			};
			mode_caslat = mode_caslat_table[caslat - 1];
		} else {
			puts("DDR I: Only CAS Latencies of 1.5, 2.0, "
			     "2.5 and 3.0 clocks are supported.\n");
			return 0;
		}

	} else {
		if (2 <= caslat && caslat <= 5) {
			mode_caslat = caslat;
		} else {
			puts("DDR II: Only CAS Latencies of 2.0, 3.0, "
			     "4.0 and 5.0 clocks are supported.\n");
			return 0;
		}
	}

	/*
	 * Encoded Burst Lenght of 4.
	 */
	burst_len = 2;			/* Fiat. */

	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		twr_auto_clk = 0;	/* Historical */
	} else {
		/*
		 * Determine tCK max in picos.  Grab tWR and convert to picos.
		 * Auto-precharge write recovery is:
		 *	WR = roundup(tWR_ns/tCKmax_ns).
		 *
		 * Ponder: Is twr_auto_clk different than twr_clk?
		 */
		tCKmax_ps = convert_bcd_tenths_to_cycle_time_ps(spd.tckmax);
		twr_auto_clk = (spd.twr * 250 + tCKmax_ps - 1) / tCKmax_ps;
	}


	/*
	 * Mode Reg in bits 16 ~ 31,
	 * Extended Mode Reg 1 in bits 0 ~ 15.
	 */
	mode_odt_enable = 0x0;			/* Default disabled */
	if (odt_wr_cfg || odt_rd_cfg) {
		/*
		 * Bits 6 and 2 in Extended MRS(1)
		 * Bit 2 == 0x04 == 75 Ohm, with 2 DIMM modules.
		 * Bit 6 == 0x40 == 150 Ohm, with 1 DIMM module.
		 */
		mode_odt_enable = 0x40;		/* 150 Ohm */
	}

	ddr->sdram_mode =
		(0
		 | (add_lat << (16 + 3))	/* Additive Latency in EMRS1 */
		 | (mode_odt_enable << 16)	/* ODT Enable in EMRS1 */
		 | (twr_auto_clk << 9)		/* Write Recovery Autopre */
		 | (mode_caslat << 4)		/* caslat */
		 | (burst_len << 0)		/* Burst length */
		 );

	debug("DDR: sdram_mode   = 0x%08x\n", ddr->sdram_mode);


	/*
	 * Clear EMRS2 and EMRS3.
	 */
	ddr->sdram_mode_2 = 0;
	debug("DDR: sdram_mode_2 = 0x%08x\n", ddr->sdram_mode_2);

	/*
	 * Determine Refresh Rate.
	 */
	refresh_clk = determine_refresh_rate(spd.refresh & 0x7);

	/*
	 * Set BSTOPRE to 0x100 for page mode
	 * If auto-charge is used, set BSTOPRE = 0
	 */
	ddr->sdram_interval =
	    (0
	     | (refresh_clk & 0x3fff) << 16
	     | 0x100
	     );
	debug("DDR: sdram_interval = 0x%08x\n", ddr->sdram_interval);

	/*
	 * Is this an ECC DDR chip?
	 * But don't mess with it if the DDR controller will init mem.
	 */
#ifdef CONFIG_DDR_ECC
	if (spd.config == 0x02) {
#ifndef CONFIG_ECC_INIT_VIA_DDRCONTROLLER
		ddr->err_disable = 0x0000000d;
#endif
		ddr->err_sbe = 0x00ff0000;
	}

	debug("DDR: err_disable = 0x%08x\n", ddr->err_disable);
	debug("DDR: err_sbe = 0x%08x\n", ddr->err_sbe);
#endif /* CONFIG_DDR_ECC */

	asm("sync;isync;msync");
	udelay(500);

	/*
	 * SDRAM Cfg 2
	 */

	/*
	 * When ODT is enabled, Chap 9 suggests asserting ODT to
	 * internal IOs only during reads.
	 */
	odt_cfg = 0;
	if (odt_rd_cfg | odt_wr_cfg) {
		odt_cfg = 0x2;		/* ODT to IOs during reads */
	}

	/*
	 * Try to use differential DQS with DDR II.
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR) {
		dqs_cfg = 0;		/* No Differential DQS for DDR I */
	} else {
		dqs_cfg = 0x1;		/* Differential DQS for DDR II */
	}

#if defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Use the DDR controller to auto initialize memory.
	 */
	d_init = 1;
	ddr->sdram_data_init = CONFIG_MEM_INIT_VALUE;
	debug("DDR: ddr_data_init = 0x%08x\n", ddr->sdram_data_init);
#else
	/*
	 * Memory will be initialized via DMA, or not at all.
	 */
	d_init = 0;
#endif

	ddr->sdram_cfg_2 = (0
			    | (dqs_cfg << 26)	/* Differential DQS */
			    | (odt_cfg << 21)	/* ODT */
			    | (d_init << 4)	/* D_INIT auto init DDR */
			    );

	debug("DDR: sdram_cfg_2  = 0x%08x\n", ddr->sdram_cfg_2);


#ifdef MPC85xx_DDR_SDRAM_CLK_CNTL
	/*
	 * Setup the clock control.
	 * SDRAM_CLK_CNTL[0] = Source synchronous enable == 1
	 * SDRAM_CLK_CNTL[5-7] = Clock Adjust
	 *	0110	3/4 cycle late
	 *	0111	7/8 cycle late
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR)
		clk_adjust = 0x6;
	else
#ifdef CONFIG_MPC8568
		/* Empirally setting clk_adjust */
		clk_adjust = 0x6;
#else
		clk_adjust = 0x7;
#endif

	ddr->sdram_clk_cntl = (0
			       | 0x80000000
			       | (clk_adjust << 23)
			       );
	debug("DDR: sdram_clk_cntl = 0x%08x\n", ddr->sdram_clk_cntl);
#endif

	/*
	 * Figure out the settings for the sdram_cfg register.
	 * Build up the entire register in 'sdram_cfg' before writing
	 * since the write into the register will actually enable the
	 * memory controller; all settings must be done before enabling.
	 *
	 * sdram_cfg[0]   = 1 (ddr sdram logic enable)
	 * sdram_cfg[1]   = 1 (self-refresh-enable)
	 * sdram_cfg[5:7] = (SDRAM type = DDR SDRAM)
	 *			010 DDR 1 SDRAM
	 *			011 DDR 2 SDRAM
	 */
	sdram_type = (spd.mem_type == SPD_MEMTYPE_DDR) ? 2 : 3;
	sdram_cfg = (0
		     | (1 << 31)			/* Enable */
		     | (1 << 30)			/* Self refresh */
		     | (sdram_type << 24)		/* SDRAM type */
		     );

	/*
	 * sdram_cfg[3] = RD_EN - registered DIMM enable
	 *   A value of 0x26 indicates micron registered DIMMS (micron.com)
	 */
	if (spd.mem_type == SPD_MEMTYPE_DDR && spd.mod_attr == 0x26) {
		sdram_cfg |= 0x10000000;		/* RD_EN */
	}

#if defined(CONFIG_DDR_ECC)
	/*
	 * If the user wanted ECC (enabled via sdram_cfg[2])
	 */
	if (spd.config == 0x02) {
		sdram_cfg |= 0x20000000;		/* ECC_EN */
	}
#endif

	/*
	 * REV1 uses 1T timing.
	 * REV2 may use 1T or 2T as configured by the user.
	 */
	{
		uint pvr = get_pvr();

		if (pvr != PVR_85xx_REV1) {
#if defined(CONFIG_DDR_2T_TIMING)
			/*
			 * Enable 2T timing by setting sdram_cfg[16].
			 */
			sdram_cfg |= 0x8000;		/* 2T_EN */
#endif
		}
	}

	/*
	 * 200 painful micro-seconds must elapse between
	 * the DDR clock setup and the DDR config enable.
	 */
	udelay(200);

	/*
	 * Go!
	 */
	ddr->sdram_cfg = sdram_cfg;

	asm("sync;isync;msync");
	udelay(500);

	debug("DDR: sdram_cfg   = 0x%08x\n", ddr->sdram_cfg);


#if defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Poll until memory is initialized.
	 * 512 Meg at 400 might hit this 200 times or so.
	 */
	while ((ddr->sdram_cfg_2 & (d_init << 4)) != 0) {
		udelay(1000);
	}
#endif


	/*
	 * Figure out memory size in Megabytes.
	 */
	memsize = n_ranks * rank_density / 0x100000;

	/*
	 * Establish Local Access Window and TLB mappings for DDR memory.
	 */
	memsize = setup_laws_and_tlbs(memsize);
	if (memsize == 0) {
		return 0;
	}

	return memsize * 1024 * 1024;
}


/*
 * Setup Local Access Window and TLB1 mappings for the requested
 * amount of memory.  Returns the amount of memory actually mapped
 * (usually the original request size), or 0 on error.
 */

static unsigned int
setup_laws_and_tlbs(unsigned int memsize)
{
	unsigned int tlb_size;
	unsigned int law_size;
	unsigned int ram_tlb_index;
	unsigned int ram_tlb_address;

	/*
	 * Determine size of each TLB1 entry.
	 */
	switch (memsize) {
	case 16:
	case 32:
		tlb_size = BOOKE_PAGESZ_16M;
		break;
	case 64:
	case 128:
		tlb_size = BOOKE_PAGESZ_64M;
		break;
	case 256:
	case 512:
		tlb_size = BOOKE_PAGESZ_256M;
		break;
	case 1024:
	case 2048:
		if (PVR_VER(get_pvr()) > PVR_VER(PVR_85xx))
			tlb_size = BOOKE_PAGESZ_1G;
		else
			tlb_size = BOOKE_PAGESZ_256M;
		break;
	default:
		puts("DDR: only 16M,32M,64M,128M,256M,512M,1G and 2G are supported.\n");

		/*
		 * The memory was not able to be mapped.
		 * Default to a small size.
		 */
		tlb_size = BOOKE_PAGESZ_64M;
		memsize=64;
		break;
	}

	/*
	 * Configure DDR TLB1 entries.
	 * Starting at TLB1 8, use no more than 8 TLB1 entries.
	 */
	ram_tlb_index = 8;
	ram_tlb_address = (unsigned int)CFG_DDR_SDRAM_BASE;
	while (ram_tlb_address < (memsize * 1024 * 1024)
	      && ram_tlb_index < 16) {
		set_tlb(1, ram_tlb_address, ram_tlb_address,
			MAS3_SX|MAS3_SW|MAS3_SR, 0,
			0, ram_tlb_index, tlb_size, 1);

		ram_tlb_address += (0x1000 << ((tlb_size - 1) * 2));
		ram_tlb_index++;
	}


	/*
	 * First supported LAW size is 16M, at LAWAR_SIZE_16M == 23.  Fnord.
	 */
	law_size = 19 + __ilog2(memsize);

	/*
	 * Set up LAWBAR for all of DDR.
	 */

#ifdef CONFIG_FSL_LAW
	set_law(1, CFG_DDR_SDRAM_BASE, law_size, LAW_TRGT_IF_DDR);
#endif

	/*
	 * Confirm that the requested amount of memory was mapped.
	 */
	return memsize;
}

#endif /* CONFIG_SPD_EEPROM */


#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)

/*
 * Initialize all of memory for ECC, then enable errors.
 */

void
ddr_enable_ecc(unsigned int dram_size)
{
	uint *p = 0;
	uint i = 0;
	volatile ccsr_ddr_t *ddr= (void *)(CFG_MPC85xx_DDR_ADDR);

	dma_init();

	for (*p = 0; p < (uint *)(8 * 1024); p++) {
		if (((unsigned int)p & 0x1f) == 0) {
			ppcDcbz((unsigned long) p);
		}
		*p = (unsigned int)CONFIG_MEM_INIT_VALUE;
		if (((unsigned int)p & 0x1c) == 0x1c) {
			ppcDcbf((unsigned long) p);
		}
	}

	dma_xfer((uint *)0x002000, 0x002000, (uint *)0); /* 8K */
	dma_xfer((uint *)0x004000, 0x004000, (uint *)0); /* 16K */
	dma_xfer((uint *)0x008000, 0x008000, (uint *)0); /* 32K */
	dma_xfer((uint *)0x010000, 0x010000, (uint *)0); /* 64K */
	dma_xfer((uint *)0x020000, 0x020000, (uint *)0); /* 128k */
	dma_xfer((uint *)0x040000, 0x040000, (uint *)0); /* 256k */
	dma_xfer((uint *)0x080000, 0x080000, (uint *)0); /* 512k */
	dma_xfer((uint *)0x100000, 0x100000, (uint *)0); /* 1M */
	dma_xfer((uint *)0x200000, 0x200000, (uint *)0); /* 2M */
	dma_xfer((uint *)0x400000, 0x400000, (uint *)0); /* 4M */

	for (i = 1; i < dram_size / 0x800000; i++) {
		dma_xfer((uint *)(0x800000*i), 0x800000, (uint *)0);
	}

	/*
	 * Enable errors for ECC.
	 */
	debug("DMA DDR: err_disable = 0x%08x\n", ddr->err_disable);
	ddr->err_disable = 0x00000000;
	asm("sync;isync;msync");
	debug("DMA DDR: err_disable = 0x%08x\n", ddr->err_disable);
}

#endif	/* CONFIG_DDR_ECC  && ! CONFIG_ECC_INIT_VIA_DDRCONTROLLER */
