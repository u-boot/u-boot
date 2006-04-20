/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

/* sdram_init.c - automatic memory sizing */

#include <common.h>
#include <74xx_7xx.h>
#include <galileo/memory.h>
#include <galileo/pci.h>
#include <galileo/gt64260R.h>
#include <net.h>

#include "eth.h"
#include "mpsc.h"
#include "i2c.h"
#include "64260.h"

DECLARE_GLOBAL_DATA_PTR;

/* #define	DEBUG */
#define	MAP_PCI

#ifdef DEBUG
#define DP(x) x
#else
#define DP(x)
#endif

#define GB         (1 << 30)

/* structure to store the relevant information about an sdram bank */
typedef struct sdram_info {
	uchar drb_size;
	uchar registered, ecc;
	uchar tpar;
	uchar tras_clocks;
	uchar burst_len;
	uchar banks, slot;
	int size;		/* detected size, not from I2C but from dram_size() */
} sdram_info_t;

#ifdef DEBUG
void dump_dimm_info (struct sdram_info *d)
{
	static const char *ecc_legend[] = { "", " Parity", " ECC" };

	printf ("dimm%s %sDRAM: %dMibytes:\n",
		ecc_legend[d->ecc],
		d->registered ? "R" : "", (d->size >> 20));
	printf ("  drb=%d tpar=%d tras=%d burstlen=%d banks=%d slot=%d\n",
		d->drb_size, d->tpar, d->tras_clocks, d->burst_len,
		d->banks, d->slot);
}
#endif

static int
memory_map_bank (unsigned int bankNo,
		 unsigned int bankBase, unsigned int bankLength)
{
#ifdef DEBUG
	if (bankLength > 0) {
		printf ("mapping bank %d at %08x - %08x\n",
			bankNo, bankBase, bankBase + bankLength - 1);
	} else {
		printf ("unmapping bank %d\n", bankNo);
	}
#endif

	memoryMapBank (bankNo, bankBase, bankLength);

	return 0;
}

#ifdef MAP_PCI
static int
memory_map_bank_pci (unsigned int bankNo,
		     unsigned int bankBase, unsigned int bankLength)
{
	PCI_HOST host;

	for (host = PCI_HOST0; host <= PCI_HOST1; host++) {
		const int features =
			PREFETCH_ENABLE |
			DELAYED_READ_ENABLE |
			AGGRESSIVE_PREFETCH |
			READ_LINE_AGGRESSIVE_PREFETCH |
			READ_MULTI_AGGRESSIVE_PREFETCH |
			MAX_BURST_4 | PCI_NO_SWAP;

		pciMapMemoryBank (host, bankNo, bankBase, bankLength);

		pciSetRegionSnoopMode (host, bankNo, PCI_SNOOP_WB, bankBase,
				       bankLength);

		pciSetRegionFeatures (host, bankNo, features, bankBase,
				      bankLength);
	}
	return 0;
}
#endif

/* ------------------------------------------------------------------------- */

/* much of this code is based on (or is) the code in the pip405 port */
/* thanks go to the authors of said port - Josh */


/*
 * translate ns.ns/10 coding of SPD timing values
 * into 10 ps unit values
 */
static inline unsigned short NS10to10PS (unsigned char spd_byte)
{
	unsigned short ns, ns10;

	/* isolate upper nibble */
	ns = (spd_byte >> 4) & 0x0F;
	/* isolate lower nibble */
	ns10 = (spd_byte & 0x0F);

	return (ns * 100 + ns10 * 10);
}

/*
 * translate ns coding of SPD timing values
 * into 10 ps unit values
 */
static inline unsigned short NSto10PS (unsigned char spd_byte)
{
	return (spd_byte * 100);
}

#ifdef CONFIG_ZUMA_V2
static int check_dimm (uchar slot, sdram_info_t * info)
{
	/* assume 2 dimms, 2 banks each 256M - we dont have an
	 * dimm i2c so rely on the detection routines later */

	memset (info, 0, sizeof (*info));

	info->slot = slot;
	info->banks = 2;	/* Detect later */
	info->registered = 0;
	info->drb_size = 32;	/* 16 - 256MBit, 32 - 512MBit
				   but doesn't matter, both do same
				   thing in setup_sdram() */
	info->tpar = 3;
	info->tras_clocks = 5;
	info->burst_len = 4;
#ifdef CONFIG_ECC
	info->ecc = 0;		/* Detect later */
#endif /* CONFIG_ECC */
	return 0;
}

#elif defined(CONFIG_P3G4)

static int check_dimm (uchar slot, sdram_info_t * info)
{
	memset (info, 0, sizeof (*info));

	if (slot)
		return 0;

	info->slot = slot;
	info->banks = 1;
	info->registered = 0;
	info->drb_size = 4;
	info->tpar = 3;
	info->tras_clocks = 6;
	info->burst_len = 4;
#ifdef CONFIG_ECC
	info->ecc = 2;
#endif
	return 0;
}

#else  /* ! CONFIG_ZUMA_V2 && ! CONFIG_P3G4 */

/* This code reads the SPD chip on the sdram and populates
 * the array which is passed in with the relevant information */
static int check_dimm (uchar slot, sdram_info_t * info)
{
	uchar addr = slot == 0 ? DIMM0_I2C_ADDR : DIMM1_I2C_ADDR;
	int ret;
	uchar rows, cols, sdram_banks, supp_cal, width, cal_val;
	ulong tmemclk;
	uchar trp_clocks, trcd_clocks;
	uchar data[128];

	get_clocks ();

	tmemclk = 1000000000 / (gd->bus_clk / 100);	/* in 10 ps units */

#ifdef CONFIG_EVB64260_750CX
	if (0 != slot) {
		printf ("check_dimm: The EVB-64260-750CX only has 1 DIMM,");
		printf ("            called with slot=%d insetad!\n", slot);
		return 0;
	}
#endif
	DP (puts ("before i2c read\n"));

	ret = i2c_read (addr, 0, 128, data, 0);

	DP (puts ("after i2c read\n"));

	/* zero all the values */
	memset (info, 0, sizeof (*info));

	if (ret) {
		DP (printf ("No DIMM in slot %d [err = %x]\n", slot, ret));
		return 0;
	}

	/* first, do some sanity checks */
	if (data[2] != 0x4) {
		printf ("Not SDRAM in slot %d\n", slot);
		return 0;
	}

	/* get various information */
	rows = data[3];
	cols = data[4];
	info->banks = data[5];
	sdram_banks = data[17];
	width = data[13] & 0x7f;

	DP (printf
	    ("sdram_banks: %d, banks: %d\n", sdram_banks, info->banks));

	/* check if the memory is registered */
	if (data[21] & (BIT1 | BIT4))
		info->registered = 1;

#ifdef CONFIG_ECC
	/* check for ECC/parity [0 = none, 1 = parity, 2 = ecc] */
	info->ecc = (data[11] & 2) >> 1;
#endif

	/* bit 1 is CL2, bit 2 is CL3 */
	supp_cal = (data[18] & 0x6) >> 1;

	/* compute the relevant clock values */
	trp_clocks = (NSto10PS (data[27]) + (tmemclk - 1)) / tmemclk;
	trcd_clocks = (NSto10PS (data[29]) + (tmemclk - 1)) / tmemclk;
	info->tras_clocks = (NSto10PS (data[30]) + (tmemclk - 1)) / tmemclk;

	DP (printf ("trp = %d\ntrcd_clocks = %d\ntras_clocks = %d\n",
		    trp_clocks, trcd_clocks, info->tras_clocks));

	/* try a CAS latency of 3 first... */
	cal_val = 0;
	if (supp_cal & 3) {
		if (NS10to10PS (data[9]) <= tmemclk)
			cal_val = 3;
	}

	/* then 2... */
	if (supp_cal & 2) {
		if (NS10to10PS (data[23]) <= tmemclk)
			cal_val = 2;
	}

	DP (printf ("cal_val = %d\n", cal_val));

	/* bummer, did't work... */
	if (cal_val == 0) {
		DP (printf ("Couldn't find a good CAS latency\n"));
		return 0;
	}

	/* get the largest delay -- these values need to all be the same
	 * see Res#6 */
	info->tpar = cal_val;
	if (trp_clocks > info->tpar)
		info->tpar = trp_clocks;
	if (trcd_clocks > info->tpar)
		info->tpar = trcd_clocks;

	DP (printf ("tpar set to: %d\n", info->tpar));

#ifdef CFG_BROKEN_CL2
	if (info->tpar == 2) {
		info->tpar = 3;
		DP (printf ("tpar fixed-up to: %d\n", info->tpar));
	}
#endif
	/* compute the module DRB size */
	info->drb_size =
		(((1 << (rows + cols)) * sdram_banks) * width) / _16M;

	DP (printf ("drb_size set to: %d\n", info->drb_size));

	/* find the burst len */
	info->burst_len = data[16] & 0xf;
	if ((info->burst_len & 8) == 8) {
		info->burst_len = 1;
	} else if ((info->burst_len & 4) == 4) {
		info->burst_len = 0;
	} else {
		return 0;
	}

	info->slot = slot;
	return 0;
}
#endif /* ! CONFIG_ZUMA_V2 */

static int setup_sdram_common (sdram_info_t info[2])
{
	ulong tmp;
	int tpar = 2, tras_clocks = 5, registered = 1, ecc = 2;

	if (!info[0].banks && !info[1].banks)
		return 0;

	if (info[0].banks) {
		if (info[0].tpar > tpar)
			tpar = info[0].tpar;
		if (info[0].tras_clocks > tras_clocks)
			tras_clocks = info[0].tras_clocks;
		if (!info[0].registered)
			registered = 0;
		if (info[0].ecc != 2)
			ecc = 0;
	}

	if (info[1].banks) {
		if (info[1].tpar > tpar)
			tpar = info[1].tpar;
		if (info[1].tras_clocks > tras_clocks)
			tras_clocks = info[1].tras_clocks;
		if (!info[1].registered)
			registered = 0;
		if (info[1].ecc != 2)
			ecc = 0;
	}

	/* SDRAM configuration */
	tmp = GTREGREAD (SDRAM_CONFIGURATION);

	/* Turn on physical interleave if both DIMMs
	 * have even numbers of banks. */
	if ((info[0].banks == 0 || info[0].banks == 2) &&
	    (info[1].banks == 0 || info[1].banks == 2)) {
		/* physical interleave on */
		tmp &= ~(1 << 15);
	} else {
		/* physical interleave off */
		tmp |= (1 << 15);
	}

	tmp |= (registered << 17);

	/* Use buffer 1 to return read data to the CPU
	 * See Res #12 */
	tmp |= (1 << 26);

	GT_REG_WRITE (SDRAM_CONFIGURATION, tmp);
	DP (printf ("SDRAM config: %08x\n", GTREGREAD (SDRAM_CONFIGURATION)));

	/* SDRAM timing */
	tmp = (((tpar == 3) ? 2 : 1) |
	       (((tpar == 3) ? 2 : 1) << 2) |
	       (((tpar == 3) ? 2 : 1) << 4) | (tras_clocks << 8));

#ifdef CONFIG_ECC
	/* Setup ECC */
	if (ecc == 2)
		tmp |= 1 << 13;
#endif /* CONFIG_ECC */

	GT_REG_WRITE (SDRAM_TIMING, tmp);
	DP (printf ("SDRAM timing: %08x (%d,%d,%d,%d)\n",
		    GTREGREAD (SDRAM_TIMING), tpar, tpar, tpar, tras_clocks));

	/* SDRAM address decode register */
	/* program this with the default value */
	GT_REG_WRITE (SDRAM_ADDRESS_DECODE, 0x2);
	DP (printf ("SDRAM decode: %08x\n",
		    GTREGREAD (SDRAM_ADDRESS_DECODE)));

	return 0;
}

/* sets up the GT properly with information passed in */
static int setup_sdram (sdram_info_t * info)
{
	ulong tmp, check;
	ulong *addr = 0;
	int i;

	/* sanity checking */
	if (!info->banks)
		return 0;

	/* ---------------------------- */
	/* Program the GT with the discovered data */

	/* bank parameters */
	tmp = (0xf << 16);	/* leave all virt bank pages open */

	DP (printf ("drb_size: %d\n", info->drb_size));
	switch (info->drb_size) {
	case 1:
		tmp |= (1 << 14);
		break;
	case 4:
	case 8:
		tmp |= (2 << 14);
		break;
	case 16:
	case 32:
		tmp |= (3 << 14);
		break;
	default:
		printf ("Error in dram size calculation\n");
		return 1;
	}

	/* SDRAM bank parameters */
	/* the param registers for slot 1 (banks 2+3) are offset by 0x8 */
	GT_REG_WRITE (SDRAM_BANK0PARAMETERS + (info->slot * 0x8), tmp);
	GT_REG_WRITE (SDRAM_BANK1PARAMETERS + (info->slot * 0x8), tmp);
	DP (printf
	    ("SDRAM bankparam slot %d (bank %d+%d): %08lx\n", info->slot,
	     info->slot * 2, (info->slot * 2) + 1, tmp));

	/* set the SDRAM configuration for each bank */
	for (i = info->slot * 2; i < ((info->slot * 2) + info->banks); i++) {
		DP (printf ("*** Running a MRS cycle for bank %d ***\n", i));

		/* map the bank */
		memory_map_bank (i, 0, GB / 4);

		/* set SDRAM mode */
		GT_REG_WRITE (SDRAM_OPERATION_MODE, 0x3);
		check = GTREGREAD (SDRAM_OPERATION_MODE);

		/* dummy write */
		*addr = 0;

		/* wait for the command to complete */
		while ((GTREGREAD (SDRAM_OPERATION_MODE) & (1 << 31)) == 0);

		/* switch back to normal operation mode */
		GT_REG_WRITE (SDRAM_OPERATION_MODE, 0);
		check = GTREGREAD (SDRAM_OPERATION_MODE);

		/* unmap the bank */
		memory_map_bank (i, 0, 0);
		DP (printf ("*** MRS cycle for bank %d done ***\n", i));
	}

	return 0;
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */
static long int dram_size (long int *base, long int maxsize)
{
	volatile long int *addr, *b = base;
	long int cnt, val, save1, save2;

#define STARTVAL (1<<20)	/* start test at 1M */
	for (cnt = STARTVAL / sizeof (long); cnt < maxsize / sizeof (long);
	     cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */

		save1 = *addr;	/* save contents of addr */
		save2 = *b;	/* save contents of base */

		*addr = cnt;	/* write cnt to addr */
		*b = 0;		/* put null at base */

		/* check at base address */
		if ((*b) != 0) {
			*addr = save1;	/* restore *addr */
			*b = save2;	/* restore *b */
			return (0);
		}
		val = *addr;	/* read *addr */

		*addr = save1;
		*b = save2;

		if (val != cnt) {
			/* fix boundary condition.. STARTVAL means zero */
			if (cnt == STARTVAL / sizeof (long))
				cnt = 0;
			return (cnt * sizeof (long));
		}
	}
	return maxsize;
}

/* ------------------------------------------------------------------------- */

/* U-Boot interface function to SDRAM init - this is where all the
 * controlling logic happens */
long int initdram (int board_type)
{
	ulong checkbank[4] = {[0 ... 3] = 0 };
	int bank_no;
	ulong total;
	int nhr;
	sdram_info_t dimm_info[2];


	/* first, use the SPD to get info about the SDRAM */

	/* check the NHR bit and skip mem init if it's already done */
	nhr = get_hid0 () & (1 << 16);

	if (nhr) {
		printf ("Skipping SDRAM setup due to NHR bit being set\n");
	} else {
		/* DIMM0 */
		check_dimm (0, &dimm_info[0]);

		/* DIMM1 */
#ifndef CONFIG_EVB64260_750CX	/* EVB64260_750CX has only 1 DIMM */
		check_dimm (1, &dimm_info[1]);
#else  /* CONFIG_EVB64260_750CX */
		memset (&dimm_info[1], 0, sizeof (sdram_info_t));
#endif

		/* unmap all banks */
		memory_map_bank (0, 0, 0);
		memory_map_bank (1, 0, 0);
		memory_map_bank (2, 0, 0);
		memory_map_bank (3, 0, 0);

		/* Now, program the GT with the correct values */
		if (setup_sdram_common (dimm_info)) {
			printf ("Setup common failed.\n");
		}

		if (setup_sdram (&dimm_info[0])) {
			printf ("Setup for DIMM1 failed.\n");
		}

		if (setup_sdram (&dimm_info[1])) {
			printf ("Setup for DIMM2 failed.\n");
		}

		/* set the NHR bit */
		set_hid0 (get_hid0 () | (1 << 16));
	}
	/* next, size the SDRAM banks */

	total = 0;
	if (dimm_info[0].banks > 0)
		checkbank[0] = 1;
	if (dimm_info[0].banks > 1)
		checkbank[1] = 1;
	if (dimm_info[0].banks > 2)
		printf ("Error, SPD claims DIMM1 has >2 banks\n");

	if (dimm_info[1].banks > 0)
		checkbank[2] = 1;
	if (dimm_info[1].banks > 1)
		checkbank[3] = 1;
	if (dimm_info[1].banks > 2)
		printf ("Error, SPD claims DIMM2 has >2 banks\n");

	/* Generic dram sizer: works even if we don't have i2c DIMMs,
	 * as long as the timing settings are more or less correct */

	/*
	 * pass 1: size all the banks, using first bat (0-256M)
	 *         limitation: we only support 256M per bank due to
	 *         us only having 1 BAT for all DRAM
	 */
	for (bank_no = 0; bank_no < CFG_DRAM_BANKS; bank_no++) {
		/* skip over banks that are not populated */
		if (!checkbank[bank_no])
			continue;

		DP (printf ("checking bank %d\n", bank_no));

		memory_map_bank (bank_no, 0, GB / 4);
		checkbank[bank_no] = dram_size (NULL, GB / 4);
		memory_map_bank (bank_no, 0, 0);

		DP (printf ("bank %d %08lx\n", bank_no, checkbank[bank_no]));
	}

	/*
	 * pass 2: contiguously map each bank into physical address
	 *         space.
	 */
	dimm_info[0].banks = dimm_info[1].banks = 0;
	for (bank_no = 0; bank_no < CFG_DRAM_BANKS; bank_no++) {
		if (!checkbank[bank_no])
			continue;

		dimm_info[bank_no / 2].banks++;
		dimm_info[bank_no / 2].size += checkbank[bank_no];

		memory_map_bank (bank_no, total, checkbank[bank_no]);
#ifdef MAP_PCI
		memory_map_bank_pci (bank_no, total, checkbank[bank_no]);
#endif
		total += checkbank[bank_no];
	}

#ifdef CONFIG_ECC
#ifdef CONFIG_ZUMA_V2
	/*
	 * We always enable ECC when bank 2 and 3 are unpopulated
	 * If we 2 or 3 are populated, we CAN'T support ECC.
	 * (Zuma boards only support ECC in banks 0 and 1; assume that
	 * in that configuration, ECC chips are mounted, even for stacked
	 * chips)
	 */
	if (checkbank[2] == 0 && checkbank[3] == 0) {
		dimm_info[0].ecc = 2;
		GT_REG_WRITE (SDRAM_TIMING,
			      GTREGREAD (SDRAM_TIMING) | (1 << 13));
		/* TODO: do we have to run MRS cycles again? */
	}
#endif /* CONFIG_ZUMA_V2 */

	if (GTREGREAD (SDRAM_TIMING) & (1 << 13)) {
		puts ("[ECC] ");
	}
#endif /* CONFIG_ECC */

#ifdef DEBUG
	dump_dimm_info (&dimm_info[0]);
	dump_dimm_info (&dimm_info[1]);
#endif
	/* TODO: return at MOST 256M? */
	/* return total > GB/4 ? GB/4 : total; */
	return total;
}
