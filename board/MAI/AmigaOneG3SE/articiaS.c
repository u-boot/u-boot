/*
 * (C) Copyright 2002
 * Hyperion Entertainment, ThomasF@hyperion-entertainment.com
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
#include <pci.h>
#include <asm/processor.h>
#include "memio.h"
#include "articiaS.h"
#include "smbus.h"
#include "via686.h"

#undef DEBUG

struct dimm_bank {
	uint8 used;			/* Bank is populated */
	uint32 rows;			/* Number of row addresses */
	uint32 columns;			/* Number of column addresses */
	uint8 registered;		/* SIMM is registered */
	uint8 ecc;			/* SIMM has ecc */
	uint8 burst_len;		/* Supported burst lengths */
	uint32 cas_lat;			/* Supported CAS latencies */
	uint32 cas_used;		/* CAS to use (not set by user) */
	uint32 trcd;			/* RAS to CAS latency */
	uint32 trp;			/* Precharge latency */
	uint32 tclk_hi;			/* SDRAM cycle time (highest CAS latency) */
	uint32 tclk_2hi;		/* SDRAM second highest CAS latency */
	uint32 size;			/* Size of bank in bytes */
	uint8 auto_refresh;		/* Module supports auto refresh */
	uint32 refresh_time;		/* Refresh time (in ns) */
};


/*
** Based in part on the evb64260 code
*/

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


long detect_sdram (uint8 * rom, int dimmNum, struct dimm_bank *banks)
{
    DECLARE_GLOBAL_DATA_PTR;
	int dimm_address = (dimmNum == 0) ? SM_DIMM0_ADDR : SM_DIMM1_ADDR;
	uint32 busclock = gd->bus_clk;
	uint32 memclock = busclock;
	uint32 tmemclock = 1000000000 / (memclock / 100);
	uint32 datawidth;

	if (sm_get_data (rom, dimm_address) == 0) {
		/* Nothing in slot, make both banks empty */
		debug ("Slot %d: vacant\n", dimmNum);
		banks[0].used = 0;
		banks[1].used = 0;
		return 0;
	}

	if (rom[2] != 0x04) {
		debug ("Slot %d: No SDRAM\n", dimmNum);
		banks[0].used = 0;
		banks[1].used = 0;
		return 0;
	}

	/* Determine number of banks/rows */
	if (rom[5] == 1) {
		banks[0].used = 1;
		banks[1].used = 0;
	} else {
		banks[0].used = 1;
		banks[1].used = 1;
	}

	/* Determine number of row addresses */
	if (rom[3] & 0xf0) {
		/* Different banks sizes */
		banks[0].rows = rom[3] & 0x0f;
		banks[1].rows = (rom[3] & 0xf0) >> 4;
	} else {
		/* Equal sized banks */
		banks[0].rows = rom[3] & 0x0f;
		banks[1].rows = banks[0].rows;
	}

	/* Determine number of column addresses */
	if (rom[4] & 0xf0) {
		/* Different bank sizes */
		banks[0].columns = rom[4] & 0x0f;
		banks[1].columns = (rom[4] & 0xf0) >> 4;
	} else {
		banks[0].columns = rom[4] & 0x0f;
		banks[1].columns = banks[0].columns;
	}

	/* Check Jedec revision, and modify row/column accordingly */
	if (rom[62] > 0x10) {
		if (banks[0].rows <= 3)
			banks[0].rows += 15;
		if (banks[1].rows <= 3)
			banks[1].rows += 15;
		if (banks[0].columns <= 3)
			banks[0].columns += 15;
		if (banks[0].columns <= 3)
			banks[0].columns += 15;
	}

	/* Check registered/unregisterd */
	if (rom[21] & 0x12) {
		banks[0].registered = 1;
		banks[1].registered = 1;
	} else {
		banks[0].registered = 0;
		banks[1].registered = 0;
	}

#ifdef CONFIG_ECC
	/* Check parity/ECC */
	banks[0].ecc = (rom[11] == 0x02);
	banks[1].ecc = (rom[11] == 0x02);
#endif

	/* Find burst lengths supported */
	banks[0].burst_len = rom[16] & 0x8f;
	banks[1].burst_len = rom[16] & 0x8f;

	/* Find possible cas latencies */
	banks[0].cas_lat = rom[18] & 0x7F;
	banks[1].cas_lat = rom[18] & 0x7F;

	/* RAS/CAS latency */
	banks[0].trcd = (NSto10PS (rom[29]) + (tmemclock - 1)) / tmemclock;
	banks[1].trcd = (NSto10PS (rom[29]) + (tmemclock - 1)) / tmemclock;

	/* Precharge latency */
	banks[0].trp = (NSto10PS (rom[27]) + (tmemclock - 1)) / tmemclock;
	banks[1].trp = (NSto10PS (rom[27]) + (tmemclock - 1)) / tmemclock;

	/* highest CAS latency */
	banks[0].tclk_hi = NS10to10PS (rom[9]);
	banks[1].tclk_hi = NS10to10PS (rom[9]);

	/* second highest CAS latency */
	banks[0].tclk_2hi = NS10to10PS (rom[23]);
	banks[1].tclk_2hi = NS10to10PS (rom[23]);

	/* bank sizes */
	datawidth = rom[13] & 0x7f;
	banks[0].size =
			(1L << (banks[0].rows + banks[0].columns)) *
			/* FIXME datawidth */ 8 * rom[17];
	if (rom[13] & 0x80)
		banks[1].size = 2 * banks[0].size;
	else
		banks[1].size = (1L << (banks[1].rows + banks[1].columns)) *
				/* FIXME datawidth */ 8 * rom[17];

	/* Refresh */
	if (rom[12] & 0x80) {
		banks[0].auto_refresh = 1;
		banks[1].auto_refresh = 1;
	} else {
		banks[0].auto_refresh = 0;
		banks[1].auto_refresh = 0;
	}

	switch (rom[12] & 0x7f) {
	case 0:
		banks[0].refresh_time = (1562500 + (tmemclock - 1)) / tmemclock;
		banks[1].refresh_time = (1562500 + (tmemclock - 1)) / tmemclock;
		break;
	case 1:
		banks[0].refresh_time = (390600 + (tmemclock - 1)) / tmemclock;
		banks[1].refresh_time = (390600 + (tmemclock - 1)) / tmemclock;
		break;
	case 2:
		banks[0].refresh_time = (781200 + (tmemclock - 1)) / tmemclock;
		banks[1].refresh_time = (781200 + (tmemclock - 1)) / tmemclock;
		break;
	case 3:
		banks[0].refresh_time = (3125000 + (tmemclock - 1)) / tmemclock;
		banks[1].refresh_time = (3125000 + (tmemclock - 1)) / tmemclock;
		break;
	case 4:
		banks[0].refresh_time = (6250000 + (tmemclock - 1)) / tmemclock;
		banks[1].refresh_time = (6250000 + (tmemclock - 1)) / tmemclock;
		break;
	case 5:
		banks[0].refresh_time = (12500000 + (tmemclock - 1)) / tmemclock;
		banks[1].refresh_time = (12500000 + (tmemclock - 1)) / tmemclock;
		break;
	default:
		banks[0].refresh_time = 0x100;	/* Default of Articia S */
		banks[1].refresh_time = 0x100;
		break;
	}

#ifdef DEBUG
	printf ("\nInformation for SIMM bank %ld:\n", dimmNum);
	printf ("Number of banks: %ld\n", banks[0].used + banks[1].used);
	printf ("Number of row addresses: %ld\n", banks[0].rows);
	printf ("Number of coumns addresses: %ld\n", banks[0].columns);
	printf ("SIMM is %sregistered\n",
			banks[0].registered == 0 ? "not " : "");
#ifdef CONFIG_ECC
	printf ("SIMM %s ECC\n",
			banks[0].ecc == 1 ? "supports" : "doesn't support");
#endif
	printf ("Supported burst lenghts: %s %s %s %s %s\n",
			banks[0].burst_len & 0x08 ? "8" : " ",
			banks[0].burst_len & 0x04 ? "4" : " ",
			banks[0].burst_len & 0x02 ? "2" : " ",
			banks[0].burst_len & 0x01 ? "1" : " ",
			banks[0].burst_len & 0x80 ? "PAGE" : "    ");
	printf ("Supported CAS latencies: %s %s %s\n",
			banks[0].cas_lat & 0x04 ? "CAS 3" : "     ",
			banks[0].cas_lat & 0x02 ? "CAS 2" : "     ",
			banks[0].cas_lat & 0x01 ? "CAS 1" : "     ");
	printf ("RAS to CAS latency: %ld\n", banks[0].trcd);
	printf ("Precharge latency: %ld\n", banks[0].trp);
	printf ("SDRAM highest CAS latency: %ld\n", banks[0].tclk_hi);
	printf ("SDRAM 2nd highest CAS latency: %ld\n", banks[0].tclk_2hi);
	printf ("SDRAM data width: %ld\n", datawidth);
	printf ("Auto Refresh %ssupported\n",
			banks[0].auto_refresh ? "" : "not ");
	printf ("Refresh time: %ld clocks\n", banks[0].refresh_time);
	if (banks[0].used)
		printf ("Bank 0 size: %ld MB\n", banks[0].size / 1024 / 1024);
	if (banks[1].used)
		printf ("Bank 1 size: %ld MB\n", banks[1].size / 1024 / 1024);

	printf ("\n");
#endif

	sm_term ();
	return 1;
}

void select_cas (struct dimm_bank *banks, uint8 fast)
{
	if (!banks[0].used) {
		banks[0].cas_used = 0;
		banks[0].cas_used = 0;
		return;
	}

	if (fast) {
		/* Search for fast CAS */
		uint32 i;
		uint32 c = 0x01;

		for (i = 1; i < 5; i++) {
			if (banks[0].cas_lat & c) {
				banks[0].cas_used = i;
				banks[1].cas_used = i;
				debug ("Using CAS %d (fast)\n", i);
				return;
			}
			c <<= 1;
		}

		/* Default to CAS 3 */
		banks[0].cas_used = 3;
		banks[1].cas_used = 3;
		debug ("Using CAS 3 (fast)\n");

		return;
	} else {
		/* Search for slow cas */
		uint32 i;
		uint32 c = 0x08;

		for (i = 4; i > 1; i--) {
			if (banks[0].cas_lat & c) {
				banks[0].cas_used = i;
				banks[1].cas_used = i;
				debug ("Using CAS %d (slow)\n", i);
				return;
			}
			c >>= 1;
		}

		/* Default to CAS 3 */
		banks[0].cas_used = 3;
		banks[1].cas_used = 3;
		debug ("Using CAS 3 (slow)\n");

		return;
	}

	banks[0].cas_used = 3;
	banks[1].cas_used = 3;
	debug ("Using CAS 3\n");

	return;
}

uint32 get_reg_setting (uint32 banks, uint32 rows, uint32 columns, uint32 size)
{
	uint32 i;

	struct RowColumnSize {
		uint32 banks;
		uint32 rows;
		uint32 columns;
		uint32 size;
		uint32 register_value;
	};

	struct RowColumnSize rcs_map[] = {
		/*  Sbk Radr Cadr   MB     Value */
		{1, 11, 8, 8, 0x00840f00},
		{1, 11, 9, 16, 0x00925f00},
		{1, 11, 10, 32, 0x00a64f00},
		{2, 12, 8, 32, 0x00c55f00},
		{2, 12, 9, 64, 0x00d66f00},
		{2, 12, 10, 128, 0x00e77f00},
		{2, 12, 11, 256, 0x00ff8f00},
		{2, 13, 11, 512, 0x00ff9f00},
		{0, 0, 0, 0, 0x00000000}
	};


	i = 0;

	while (rcs_map[i].banks != 0) {
		if (rows == rcs_map[i].rows
			&& columns == rcs_map[i].columns
			&& (size / 1024 / 1024) == rcs_map[i].size)
			return rcs_map[i].register_value;

		i++;
	}

	return 0;
}

uint32 burst_to_len (uint32 support)
{
	if (support & 0x80)
		return 0x7;
	else if (support & 0x8)
		return 0x3;
	else if (support & 0x4)
		return 0x2;
	else if (support & 0x2)
		return 0x1;
	else if (support & 0x1)
		return 0x0;

	return 0;
}

long articiaS_ram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	register uint32 i;
	register uint32 value1;
	register uint32 value2;
	uint8 rom[128];
	uint32 burst_len;
	uint32 burst_support;
	uint32 total_ram = 0;

	struct dimm_bank banks[4];	/* FIXME: Move to initram */
	uint32 busclock = gd->bus_clk;
	uint32 memclock = busclock;
	uint32 reg32;
	uint32 refresh_clocks;
	uint8 auto_refresh;

	memset (banks, 0, sizeof (struct dimm_bank) * 4);

	detect_sdram (rom, 0, &banks[0]);
	detect_sdram (rom, 1, &banks[2]);

	for (i = 0; i < 4; i++) {
		total_ram = total_ram + (banks[i].used * banks[i].size);
	}

	pci_write_cfg_long (0, 0, GLOBALINFO0, 0x117430c0);
	pci_write_cfg_long (0, 0, HBUSACR0, 0x1f0100b0);
	pci_write_cfg_long (0, 0, SRAM_CR, 0x00f12000);	/* Note: Might also try 0x00f10000 (original: 0x00f12000) */
	pci_write_cfg_byte (0, 0, DRAM_RAS_CTL0, 0x3f);
	pci_write_cfg_byte (0, 0, DRAM_RAS_CTL1, 0x00);	/*  was: 0x04); */
	pci_write_cfg_word (0, 0, DRAM_ECC0, 0x2020);	/*  was: 0x2400);  No ECC yet */

	/* FIXME: Move this stuff to seperate function, like setup_dimm_bank */
	if (banks[0].used) {
		value1 = get_reg_setting (banks[0].used + banks[1].used,
					  banks[0].rows, banks[0].columns,
					  banks[0].size);
	} else {
		value1 = 0;
	}

	if (banks[1].used) {
		value2 = get_reg_setting (banks[0].used + banks[1].used,
					  banks[1].rows, banks[1].columns,
					  banks[1].size);
	} else {
		value2 = 0;
	}

	pci_write_cfg_long (0, 0, DIMM0_B0_SCR0, value1);
	pci_write_cfg_long (0, 0, DIMM0_B1_SCR0, value2);

	debug ("DIMM0_B0_SCR0 = 0x%08x\n", value1);
	debug ("DIMM0_B1_SCR0 = 0x%08x\n", value2);

	if (banks[2].used) {
		value1 = get_reg_setting (banks[2].used + banks[3].used,
					  banks[2].rows, banks[2].columns,
					  banks[2].size);
	} else {
		value1 = 0;
	}

	if (banks[3].used) {
		value2 = get_reg_setting (banks[2].used + banks[3].used,
					  banks[3].rows, banks[3].columns,
					  banks[3].size);
	} else {
		value2 = 0;
	}

	pci_write_cfg_long (0, 0, DIMM1_B2_SCR0, value1);
	pci_write_cfg_long (0, 0, DIMM1_B3_SCR0, value2);

	debug ("DIMM0_B2_SCR0 = 0x%08x\n", value1);
	debug ("DIMM0_B3_SCR0 = 0x%08x\n", value2);

	pci_write_cfg_long (0, 0, DIMM2_B4_SCR0, 0);
	pci_write_cfg_long (0, 0, DIMM2_B5_SCR0, 0);
	pci_write_cfg_long (0, 0, DIMM3_B6_SCR0, 0);
	pci_write_cfg_long (0, 0, DIMM3_B7_SCR0, 0);

	/* Determine timing */
	select_cas (&banks[0], 0);
	select_cas (&banks[2], 0);

	/* FIXME: What about write recovery */
	/*                    Auto refresh    Precharge */
#if 0
	reg32 = (0x3 << 13) | (0x7 << 10) | ((banks[0].trp - 2) << 8) |
	/*    Write recovery  CAS Latency */
		(0x1 << 6) | (banks[0].cas_used << 4) |
	/*      RAS/CAS latency */
		((banks[0].trcd - 1) << 0);

	reg32 |= ((0x3 << 13) | (0x7 << 10) | ((banks[2].trp - 2) << 8) |
		  (0x1 << 6) | (banks[2].cas_used << 4) |
		  ((banks[2].trcd - 1) << 0)) << 16;
#else
	if (100000000 == gd->bus_clk)
		reg32 = 0x71737173;
	else
		reg32 = 0x69736973;
#endif
	pci_write_cfg_long (0, 0, DIMM0_TCR0, reg32);
	debug ("DIMM0_TCR0 = 0x%08x\n", reg32);

	/* Write default in DIMM2/3 (not used on A1) */
	pci_write_cfg_long (0, 0, DIMM2_TCR0, 0x7d737d73);


	/* Determine buffered/unbuffered mode for each SIMM. Uses first bank as reference (second, if present, uses the same) */
	reg32 = pci_read_cfg_long (0, 0, DRAM_GCR0);
	reg32 &= 0xFF00FFFF;

#if 0
	if (banks[0].used && banks[0].registered)
		reg32 |= 0x1 << 16;

	if (banks[2].used && banks[2].registered)
		reg32 |= 0x1 << 18;
#else
	if (banks[0].registered || banks[2].registered)
		reg32 |= 0x55 << 16;
#endif
	pci_write_cfg_long (0, 0, DRAM_GCR0, reg32);
	debug ("DRAM_GCR0 = 0x%08x\n", reg32);

	/* Determine refresh */
	refresh_clocks = 0xffffffff;
	auto_refresh = 1;

	for (i = 0; i < 4; i++) {
		if (banks[i].used) {
			if (banks[i].auto_refresh == 0)
				auto_refresh = 0;
			if (banks[i].refresh_time < refresh_clocks)
				refresh_clocks = banks[i].refresh_time;
		}
	}


#if 1
	/*  It seems this is suggested by the ArticiaS data book */
	if (100000000 == gd->bus_clk)
		refresh_clocks = 1561;
	else
		refresh_clocks = 2083;
#endif


	debug ("Refresh set to %ld clocks, auto refresh %s\n",
		   refresh_clocks, auto_refresh ? "on" : "off");

	pci_write_cfg_long (0, 0, DRAM_REFRESH0,
			(1 << 16) | (1 << 15) | (auto_refresh << 12) |
			(refresh_clocks));
	debug ("DRAM_REFRESH0 = 0x%08x\n",
			(1 << 16) | (1 << 15) | (auto_refresh << 12) |
			(refresh_clocks));

/*     pci_write_cfg_long(0, 0, DRAM_REFRESH0,   0x00019400);  */

	/* Set mode registers */
	/* FIXME: For now, set same burst len for all modules. Dunno if that's necessary */
	/* Find a common burst len */
	burst_support = 0xff;

	if (banks[0].used)
		burst_support = banks[0].burst_len;
	if (banks[1].used)
		burst_support = banks[1].burst_len;
	if (banks[2].used)
		burst_support = banks[2].burst_len;
	if (banks[3].used)
		burst_support = banks[3].burst_len;

	/*
	   ** Mode register:
	   ** Bits         Use
	   ** 0-2          Burst len
	   ** 3            Burst type (0 = sequential, 1 = interleave)
	   ** 4-6          CAS latency
	   ** 7-8          Operation mode (0 = default, all others invalid)
	   ** 9            Write burst
	   ** 10-11        Reserved
	   **
	   ** Mode register burst table:
	   ** A2 A1 A0     lenght
	   ** 0  0  0      1
	   ** 0  0  1      2
	   ** 0  1  0      4
	   ** 0  1  1      8
	   ** 1  0  0      invalid
	   ** 1  0  1      invalid
	   ** 1  1  0      invalid
	   ** 1  1  1      page (only valid for non-interleaved)
	 */

	burst_len = burst_to_len (burst_support);
	burst_len = 2;				/* FIXME */

	if (banks[0].used) {
		pci_write_cfg_word (0, 0, DRAM_PCR0,
			0x8000 | burst_len | (banks[0].cas_used << 4));
		debug ("Mode bank 0: 0x%08x\n",
			0x8000 | burst_len | (banks[0].cas_used << 4));
	} else {
		/*  Seems to be needed to disable the bank */
		pci_write_cfg_word (0, 0, DRAM_PCR0, 0x0000 | 0x032);
	}

	if (banks[1].used) {
		pci_write_cfg_word (0, 0, DRAM_PCR0,
			0x9000 | burst_len | (banks[1].cas_used << 4));
		debug ("Mode bank 1: 0x%08x\n",
			0x8000 | burst_len | (banks[1].cas_used << 4));
	} else {
		/*  Seems to be needed to disable the bank */
		pci_write_cfg_word (0, 0, DRAM_PCR0, 0x1000 | 0x032);
	}


	if (banks[2].used) {
		pci_write_cfg_word (0, 0, DRAM_PCR0,
			0xa000 | burst_len | (banks[2].cas_used << 4));
		debug ("Mode bank 2: 0x%08x\n",
			0x8000 | burst_len | (banks[2].cas_used << 4));
	} else {
		/*  Seems to be needed to disable the bank */
		pci_write_cfg_word (0, 0, DRAM_PCR0, 0x2000 | 0x032);
	}


	if (banks[3].used) {
		pci_write_cfg_word (0, 0, DRAM_PCR0,
			0xb000 | burst_len | (banks[3].cas_used << 4));
		debug ("Mode bank 3: 0x%08x\n",
			0x8000 | burst_len | (banks[3].cas_used << 4));
	} else {
		/*  Seems to be needed to disable the bank */
		pci_write_cfg_word (0, 0, DRAM_PCR0, 0x3000 | 0x032);
	}


	pci_write_cfg_word (0, 0, 0xba, 0x00);

	return total_ram;
}

extern int drv_isa_kbd_init (void);

int last_stage_init (void)
{
	drv_isa_kbd_init ();
	return 0;
}

int overwrite_console (void)
{
	return (0);
}

#define in_8 read_byte
#define out_8 write_byte

static __inline__ unsigned long get_msr (void)
{
	unsigned long msr;

	asm volatile ("mfmsr %0":"=r" (msr):);

	return msr;
}

static __inline__ void set_msr (unsigned long msr)
{
	asm volatile ("mtmsr %0"::"r" (msr));
}

int board_early_init_f (void)
{
	unsigned char c_value = 0;
	unsigned long msr;

	/* Basic init of PS/2 keyboard (needed for some reason)... */
	/* Ripped from John's code */
	while ((in_8 ((unsigned char *) 0xfe000064) & 0x02) != 0);
	out_8 ((unsigned char *) 0xfe000064, 0xaa);
	while ((in_8 ((unsigned char *) 0xfe000064) & 0x01) == 0);
	c_value = in_8 ((unsigned char *) 0xfe000060);
	while ((in_8 ((unsigned char *) 0xfe000064) & 0x02) != 0);
	out_8 ((unsigned char *) 0xfe000064, 0xab);
	while ((in_8 ((unsigned char *) 0xfe000064) & 0x01) == 0);
	c_value = in_8 ((unsigned char *) 0xfe000060);
	while ((in_8 ((unsigned char *) 0xfe000064) & 0x02) != 0);
	out_8 ((unsigned char *) 0xfe000064, 0xae);
/*     while ((in_8((unsigned char *)0xfe000064) & 0x01) == 0); */
/*     c_value = in_8((unsigned char *)0xfe000060); */

	/*  Enable FPU */
	msr = get_msr ();
	set_msr (msr | MSR_FP);

	via_calibrate_bus_freq ();

	return 0;
}
