/*
 * Copyright 2004 Freescale Semiconductor.
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

#ifdef CONFIG_SPD_EEPROM


#if defined(CONFIG_DDR_ECC)
extern void dma_init(void);
extern uint dma_check(void);
extern int dma_xfer(void *dest, uint count, void *src);
#endif


#ifndef	CFG_READ_SPD
#define CFG_READ_SPD	i2c_read
#endif


/*
 * Convert picoseconds into clock cycles (rounding up if needed).
 */

int
picos_to_clk(int picos)
{
	int clks;

	clks = picos / (2000000000 / (get_bus_freq(0) / 1000));
	if (picos % (2000000000 / (get_bus_freq(0) / 1000)) != 0) {
		clks++;
	}

	return clks;
}


unsigned int
banksize(unsigned char row_dens)
{
	return ((row_dens >> 2) | ((row_dens & 3) << 6)) << 24;
}


long int
spd_sdram(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_ddr_t *ddr = &immap->im_ddr;
	volatile ccsr_local_ecm_t *ecm = &immap->im_local_ecm;
	spd_eeprom_t spd;
	unsigned tmp, tmp1;
	unsigned int memsize;
	unsigned int tlb_size;
	unsigned int law_size;
	unsigned char caslat;
	unsigned int ram_tlb_index;
	unsigned int ram_tlb_address;

	CFG_READ_SPD(SPD_EEPROM_ADDRESS, 0, 1, (uchar *) & spd, sizeof (spd));

	if (spd.nrows > 2) {
		puts("DDR:Only two chip selects are supported on ADS.\n");
		return 0;
	}

	if (spd.nrow_addr < 12
	    || spd.nrow_addr > 14
	    || spd.ncol_addr < 8
	    || spd.ncol_addr > 11) {
		puts("DDR:Row or Col number unsupported.\n");
		return 0;
	}

	ddr->cs0_bnds = (banksize(spd.row_dens) >> 24) - 1;
	ddr->cs0_config = ( 1 << 31
			    | (spd.nrow_addr - 12) << 8
			    | (spd.ncol_addr - 8) );
	debug("\n");
	debug("cs0_bnds = 0x%08x\n",ddr->cs0_bnds);
	debug("cs0_config = 0x%08x\n",ddr->cs0_config);

	if (spd.nrows == 2) {
		ddr->cs1_bnds = ( (banksize(spd.row_dens) >> 8)
				  | ((banksize(spd.row_dens) >> 23) - 1) );
		ddr->cs1_config = ( 1<<31
				    | (spd.nrow_addr-12) << 8
				    | (spd.ncol_addr-8) );
		debug("cs1_bnds = 0x%08x\n",ddr->cs1_bnds);
		debug("cs1_config = 0x%08x\n",ddr->cs1_config);
	}

	if (spd.mem_type != 0x07) {
		puts("No DDR module found!\n");
		return 0;
	}

	/*
	 * Figure out memory size in Megabytes.
	 */
	memsize = spd.nrows * banksize(spd.row_dens) / 0x100000;

	/*
	 * First supported LAW size is 16M, at LAWAR_SIZE_16M == 23. Fnord.
	 */
	law_size = 19 + __ilog2(memsize);

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
	case 1024:
	case 2048:
		tlb_size = BOOKE_PAGESZ_256M;
		break;
	default:
		puts("DDR: only 16M,32M,64M,128M,256M,512M,1G and 2G DDR I are supported.\n");
		return 0;
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
		mtspr(MAS0, TLB1_MAS0(1, ram_tlb_index, 0));
		mtspr(MAS1, TLB1_MAS1(1, 1, 0, 0, tlb_size));
		mtspr(MAS2, TLB1_MAS2(E500_TLB_EPN(ram_tlb_address),
				      0, 0, 0, 0, 0, 0, 0, 0));
		mtspr(MAS3, TLB1_MAS3(E500_TLB_RPN(ram_tlb_address),
				      0, 0, 0, 0, 0, 1, 0, 1, 0, 1));
		asm volatile("isync;msync;tlbwe;isync");

		debug("DDR:MAS0=0x%08x\n", TLB1_MAS0(1, ram_tlb_index, 0));
		debug("DDR:MAS1=0x%08x\n", TLB1_MAS1(1, 1, 0, 0, tlb_size));
		debug("DDR:MAS2=0x%08x\n",
		      TLB1_MAS2(E500_TLB_EPN(ram_tlb_address),
				0, 0, 0, 0, 0, 0, 0, 0));
		debug("DDR:MAS3=0x%08x\n",
		      TLB1_MAS3(E500_TLB_RPN(ram_tlb_address),
				0, 0, 0, 0, 0, 1, 0, 1, 0, 1));

		ram_tlb_address += (0x1000 << ((tlb_size - 1) * 2));
		ram_tlb_index++;
	}

	/*
	 * Set up LAWBAR for all of DDR.
	 */
	ecm->lawbar1 = ((CFG_DDR_SDRAM_BASE>>12) & 0xfffff);
	ecm->lawar1 = (LAWAR_EN | LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & law_size));
	debug("DDR:LAWBAR1=0x%08x\n", ecm->lawbar1);
	debug("DDR:LARAR1=0x%08x\n", ecm->lawar1);

	/*
	 * find the largest CAS
	 */
	if(spd.cas_lat & 0x40) {
		caslat = 7;
	} else if (spd.cas_lat & 0x20) {
		caslat = 6;
	} else if (spd.cas_lat & 0x10) {
		caslat = 5;
	} else if (spd.cas_lat & 0x08) {
		caslat = 4;
	} else if (spd.cas_lat & 0x04) {
		caslat = 3;
	} else if (spd.cas_lat & 0x02) {
		caslat = 2;
	} else if (spd.cas_lat & 0x01) {
		caslat = 1;
	} else {
		puts("DDR:no valid CAS Latency information.\n");
		return 0;
	}

	tmp = 20000 / (((spd.clk_cycle & 0xF0) >> 4) * 10
		       + (spd.clk_cycle & 0x0f));
	debug("DDR:Module maximum data rate is: %dMhz\n", tmp);

	tmp1 = get_bus_freq(0) / 1000000;
	if (tmp1 < 230 && tmp1 >= 90 && tmp >= 230) {
		/* 90~230 range, treated as DDR 200 */
		if (spd.clk_cycle3 == 0xa0)
			caslat -= 2;
		else if(spd.clk_cycle2 == 0xa0)
			caslat--;
	} else if (tmp1 < 280 && tmp1 >= 230 && tmp >= 280) {
		/* 230-280 range, treated as DDR 266 */
		if (spd.clk_cycle3 == 0x75)
			caslat -= 2;
		else if (spd.clk_cycle2 == 0x75)
			caslat--;
	} else if (tmp1 < 350 && tmp1 >= 280 && tmp >= 350) {
		/* 280~350 range, treated as DDR 333 */
		if (spd.clk_cycle3 == 0x60)
			caslat -= 2;
		else if (spd.clk_cycle2 == 0x60)
			caslat--;
	} else if (tmp1 < 90 || tmp1 >= 350) {
		/* DDR rate out-of-range */
		puts("DDR:platform frequency is not fit for DDR rate\n");
		return 0;
	}

	/*
	 * note: caslat must also be programmed into ddr->sdram_mode
	 * register.
	 *
	 * note: WRREC(Twr) and WRTORD(Twtr) are not in SPD,
	 * use conservative value here.
	 */
	ddr->timing_cfg_1 =
	    (((picos_to_clk(spd.trp * 250) & 0x07) << 28 ) |
	     ((picos_to_clk(spd.tras * 1000) & 0x0f ) << 24 ) |
	     ((picos_to_clk(spd.trcd * 250) & 0x07) << 20 ) |
	     ((caslat & 0x07) << 16 ) |
	     (((picos_to_clk(spd.sset[6] * 1000) - 8) & 0x0f) << 12 ) |
	     ( 0x300 ) |
	     ((picos_to_clk(spd.trrd * 250) & 0x07) << 4) | 1);

	ddr->timing_cfg_2 = 0x00000800;

	debug("DDR:timing_cfg_1=0x%08x\n", ddr->timing_cfg_1);
	debug("DDR:timing_cfg_2=0x%08x\n", ddr->timing_cfg_2);

	/*
	 * Only DDR I is supported
	 * DDR I and II have different mode-register-set definition
	 */

	/* burst length is always 4 */
	switch(caslat) {
	case 2:
		ddr->sdram_mode = 0x52; /* 1.5 */
		break;
	case 3:
		ddr->sdram_mode = 0x22; /* 2.0 */
		break;
	case 4:
		ddr->sdram_mode = 0x62; /* 2.5 */
		break;
	case 5:
		ddr->sdram_mode = 0x32; /* 3.0 */
		break;
	default:
		puts("DDR:only CAS Latency 1.5, 2.0, 2.5, 3.0 is supported.\n");
		return 0;
	}
	debug("DDR:sdram_mode=0x%08x\n", ddr->sdram_mode);

	switch(spd.refresh) {
	case 0x00:
	case 0x80:
		tmp = picos_to_clk(15625000);
		break;
	case 0x01:
	case 0x81:
		tmp = picos_to_clk(3900000);
		break;
	case 0x02:
	case 0x82:
		tmp = picos_to_clk(7800000);
		break;
	case 0x03:
	case 0x83:
		tmp = picos_to_clk(31300000);
		break;
	case 0x04:
	case 0x84:
		tmp = picos_to_clk(62500000);
		break;
	case 0x05:
	case 0x85:
		tmp = picos_to_clk(125000000);
		break;
	default:
		tmp = 0x512;
		break;
	}

	/*
	 * Set BSTOPRE to 0x100 for page mode
	 * If auto-charge is used, set BSTOPRE = 0
	 */
	ddr->sdram_interval = ((tmp & 0x3fff) << 16) | 0x100;
	debug("DDR:sdram_interval=0x%08x\n", ddr->sdram_interval);

	/*
	 * Is this an ECC DDR chip?
	 */
#if defined(CONFIG_DDR_ECC)
	if (spd.config == 0x02) {
		ddr->err_disable = 0x0000000d;
		ddr->err_sbe = 0x00ff0000;
	}
	debug("DDR:err_disable=0x%08x\n", ddr->err_disable);
	debug("DDR:err_sbe=0x%08x\n", ddr->err_sbe);
#endif
	asm("sync;isync;msync");

	udelay(500);

#ifdef MPC85xx_DDR_SDRAM_CLK_CNTL
	/* Setup the clock control (8555 and later)
	 * SDRAM_CLK_CNTL[0] = Source synchronous enable == 1
	 * SDRAM_CLK_CNTL[5-7] = Clock Adjust == 3 (3/4 cycle late)
	 */
	ddr->sdram_clk_cntl = 0x83000000;
#endif

	/*
	 * Figure out the settings for the sdram_cfg register.  Build up
	 * the entire register in 'tmp' before writing since the write into
	 * the register will actually enable the memory controller, and all
	 * settings must be done before enabling.
	 *
	 * sdram_cfg[0]   = 1 (ddr sdram logic enable)
	 * sdram_cfg[1]   = 1 (self-refresh-enable)
	 * sdram_cfg[6:7] = 2 (SDRAM type = DDR SDRAM)
	 */
	tmp = 0xc2000000;

	/*
	 * sdram_cfg[3] = RD_EN - registered DIMM enable
	 *   A value of 0x26 indicates micron registered DIMMS (micron.com)
	 */
	if (spd.mod_attr == 0x26) {
		tmp |= 0x10000000;
	}

#if defined(CONFIG_DDR_ECC)
	/*
	 * If the user wanted ECC (enabled via sdram_cfg[2])
	 */
	if (spd.config == 0x02) {
		tmp |= 0x20000000;
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
			tmp |= 0x8000;
#endif
		}
	}

	ddr->sdram_cfg = tmp;

	asm("sync;isync;msync");
	udelay(500);

	debug("DDR:sdram_cfg=0x%08x\n", ddr->sdram_cfg);

	return memsize * 1024 * 1024;
}

#endif /* CONFIG_SPD_EEPROM */


#if defined(CONFIG_DDR_ECC)
/*
 * Initialize all of memory for ECC, then enable errors.
 */

void
ddr_enable_ecc(unsigned int dram_size)
{
	uint *p = 0;
	uint i = 0;
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_ddr_t *ddr= &immap->im_ddr;

	dma_init();

	for (*p = 0; p < (uint *)(8 * 1024); p++) {
		if (((unsigned int)p & 0x1f) == 0) {
			ppcDcbz((unsigned long) p);
		}
		*p = (unsigned int)0xdeadbeef;
		if (((unsigned int)p & 0x1c) == 0x1c) {
			ppcDcbf((unsigned long) p);
		}
	}

	/* 8K */
	dma_xfer((uint *)0x2000, 0x2000, (uint *)0);
	/* 16K */
	dma_xfer((uint *)0x4000, 0x4000, (uint *)0);
	/* 32K */
	dma_xfer((uint *)0x8000, 0x8000, (uint *)0);
	/* 64K */
	dma_xfer((uint *)0x10000, 0x10000, (uint *)0);
	/* 128k */
	dma_xfer((uint *)0x20000, 0x20000, (uint *)0);
	/* 256k */
	dma_xfer((uint *)0x40000, 0x40000, (uint *)0);
	/* 512k */
	dma_xfer((uint *)0x80000, 0x80000, (uint *)0);
	/* 1M */
	dma_xfer((uint *)0x100000, 0x100000, (uint *)0);
	/* 2M */
	dma_xfer((uint *)0x200000, 0x200000, (uint *)0);
	/* 4M */
	dma_xfer((uint *)0x400000, 0x400000, (uint *)0);

	for (i = 1; i < dram_size / 0x800000; i++) {
		dma_xfer((uint *)(0x800000*i), 0x800000, (uint *)0);
	}

	/*
	 * Enable errors for ECC.
	 */
	ddr->err_disable = 0x00000000;
	asm("sync;isync;msync");
}

#endif	/* CONFIG_DDR_ECC */
