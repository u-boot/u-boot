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

#define ns2clk(ns) ((ns) / (2000000000 /get_bus_freq(0) + 1) + 1)

long int spd_sdram(void) {
    	volatile immap_t *immap = (immap_t *)CFG_IMMR;
    	volatile ccsr_ddr_t *ddr = &immap->im_ddr;
	volatile ccsr_local_ecm_t *ecm = &immap->im_local_ecm;
    	spd_eeprom_t spd;
    	unsigned int memsize,tmp,tmp1,tmp2;
	unsigned char caslat;

    	i2c_read (SPD_EEPROM_ADDRESS, 0, 1, (uchar *) & spd, sizeof (spd));

	if ( spd.nrows > 2 ) {
		printf("DDR:Only two chip selects are supported on ADS.\n");
		return 0;
	}

	if ( spd.nrow_addr < 12 || spd.nrow_addr > 14 || spd.ncol_addr < 8 || spd.ncol_addr > 11) {
		printf("DDR:Row or Col number unsupported.\n");
		return 0;
	}

	ddr->cs0_bnds = ((spd.row_dens>>2) - 1);
	ddr->cs0_config = ( 1<<31 | (spd.nrow_addr-12)<<8 | (spd.ncol_addr-8) );
	debug ("\n");
	debug ("cs0_bnds = 0x%08x\n",ddr->cs0_bnds);
	debug ("cs0_config = 0x%08x\n",ddr->cs0_config);
	if ( spd.nrows == 2 ) {
		ddr->cs1_bnds = ((spd.row_dens<<14) | ((spd.row_dens>>1) - 1));
		ddr->cs1_config = ( 1<<31 | (spd.nrow_addr-12)<<8 | (spd.ncol_addr-8) );
		debug ("cs1_bnds = 0x%08x\n",ddr->cs1_bnds);
		debug ("cs1_config = 0x%08x\n",ddr->cs1_config);
	}

	memsize = spd.nrows * (4 * spd.row_dens);
	if( spd.mem_type != 0x07 ) {
		printf("No DDR module found!\n");
		return 0;
	}

	switch (memsize) {
	case 16:
		tmp = 7;		/* TLB size */
		tmp1 = 1;		/* TLB entry number */
		tmp2 = 23;		/* Local Access Window size */
		break;
	case 32:
		tmp = 7;
		tmp1 = 2;
		tmp2 = 24;
		break;
	case 64:
		tmp = 8;
		tmp1 = 1;
		tmp2 = 25;
		break;
	case 128:
		tmp = 8;
		tmp1 = 2;
		tmp2 = 26;
		break;
	case 256:
		tmp = 9;
		tmp1 = 1;
		tmp2 = 27;
		break;
	case 512:
		tmp = 9;
		tmp1 = 2;
		tmp2 = 28;
		break;
	case 1024:
		tmp = 10;
		tmp1 = 1;
		tmp2 = 29;
		break;
	default:
		printf ("DDR:we only added support 16M,32M,64M,128M,256M,512M and 1G DDR I.\n");
		return 0;
		break;
	}

	/* configure DDR TLB to TLB1 Entry 4,5 */
	mtspr(MAS0, TLB1_MAS0(1,4,0));
	mtspr(MAS1, TLB1_MAS1(1,1,0,0,tmp));
	mtspr(MAS2, TLB1_MAS2(((CFG_DDR_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,0,0,0));
	mtspr(MAS3, TLB1_MAS3(((CFG_DDR_SDRAM_BASE>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1));
	asm volatile("isync;msync;tlbwe;isync");
	debug ("DDR:MAS0=0x%08x\n",TLB1_MAS0(1,4,0));
	debug ("DDR:MAS1=0x%08x\n",TLB1_MAS1(1,1,0,0,tmp));
	debug ("DDR:MAS2=0x%08x\n",TLB1_MAS2(((CFG_DDR_SDRAM_BASE>>12) \
		& 0xfffff),0,0,0,0,0,0,0,0));
	debug ("DDR:MAS3=0x%08x\n",TLB1_MAS3(((CFG_DDR_SDRAM_BASE>>12) \
		& 0xfffff),0,0,0,0,0,1,0,1,0,1));

	if(tmp1 == 2) {
		mtspr(MAS0, TLB1_MAS0(1,5,0));
		mtspr(MAS1, TLB1_MAS1(1,1,0,0,tmp));
		mtspr(MAS2, TLB1_MAS2((((CFG_DDR_SDRAM_BASE+(memsize*1024*1024)/2)>>12) \
			& 0xfffff),0,0,0,0,0,0,0,0));
		mtspr(MAS3, TLB1_MAS3((((CFG_DDR_SDRAM_BASE+(memsize*1024*1024)/2)>>12) \
			& 0xfffff),0,0,0,0,0,1,0,1,0,1));
		asm volatile("isync;msync;tlbwe;isync");
		debug ("DDR:MAS0=0x%08x\n",TLB1_MAS0(1,5,0));
		debug ("DDR:MAS1=0x%08x\n",TLB1_MAS1(1,1,0,0,tmp));
		debug ("DDR:MAS2=0x%08x\n",TLB1_MAS2((((CFG_DDR_SDRAM_BASE \
			+(memsize*1024*1024)/2)>>12) & 0xfffff),0,0,0,0,0,0,0,0));
		debug ("DDR:MAS3=0x%08x\n",TLB1_MAS3((((CFG_DDR_SDRAM_BASE \
			+(memsize*1024*1024)/2)>>12) & 0xfffff),0,0,0,0,0,1,0,1,0,1));
	}

#if defined(CONFIG_RAM_AS_FLASH)
	ecm->lawbar2 = ((CFG_DDR_SDRAM_BASE>>12) & 0xfffff);
	ecm->lawar2 = (LAWAR_EN | LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & tmp2));
	debug ("DDR:LAWBAR2=0x%08x\n",ecm->lawbar2);
	debug ("DDR:LARAR2=0x%08x\n",ecm->lawar2);
#else
	ecm->lawbar1 = ((CFG_DDR_SDRAM_BASE>>12) & 0xfffff);
	ecm->lawar1 = (LAWAR_EN | LAWAR_TRGT_IF_DDR | (LAWAR_SIZE & tmp2));
	debug ("DDR:LAWBAR1=0x%08x\n",ecm->lawbar1);
	debug ("DDR:LARAR1=0x%08x\n",ecm->lawar1);
#endif

	tmp = 20000/(((spd.clk_cycle & 0xF0) >> 4) * 10 + (spd.clk_cycle & 0x0f));
	debug ("DDR:Module maximum data rate is: %dMhz\n",tmp);

	/* find the largest CAS */
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
		printf("DDR:no valid CAS Latency information.\n");
		return 0;
	}

	tmp1 = get_bus_freq(0)/1000000;
	if(tmp1<230 && tmp1>=90 && tmp>=230) {
		/* 90~230 range, treated as DDR 200 */
		if(spd.clk_cycle3 == 0xa0) caslat -= 2;
		else if(spd.clk_cycle2 == 0xa0) caslat--;
	} else if(tmp1<280 && tmp1>=230 && tmp>=280) {
		/* 230-280 range, treated as DDR 266 */
		if(spd.clk_cycle3 == 0x75) caslat -= 2;
		else if(spd.clk_cycle2 == 0x75) caslat--;
	} else if(tmp1<350 && tmp1>=280 && tmp>=350) {
		/* 280~350 range, treated as DDR 333 */
		if(spd.clk_cycle3 == 0x60) caslat -= 2;
		else if(spd.clk_cycle2 == 0x60) caslat--;
	} else if(tmp1<90 || tmp1 >=350) { /* DDR rate out-of-range */
		printf("DDR:platform frequency is not fit for DDR rate\n");
		return 0;
	}

	/* note: caslat must also be programmed into ddr->sdram_mode
	   register */
	/* note: WRREC(Twr) and WRTORD(Twtr) are not in SPD,use
	   conservative value here */
	ddr->timing_cfg_1 = 	(((ns2clk(spd.trp/4) & 0x07) << 28 ) | \
				((ns2clk(spd.tras) & 0x0f ) << 24 ) | \
				((ns2clk(spd.trcd/4) & 0x07) << 20 ) | \
				((caslat & 0x07)<< 16 ) | \
				(((ns2clk(spd.sset[6]) - 8) & 0x0f) << 12 ) | \
				( 0x300 ) | \
				((ns2clk(spd.trrd/4) & 0x07) << 4) | 1);

	debug ("DDR:timing_cfg_1=0x%08x\n",ddr->timing_cfg_1);

	ddr->timing_cfg_2 = 0x00000800;
	debug ("DDR:timing_cfg_2=0x%08x\n",ddr->timing_cfg_2);

	/* only DDR I is supported, DDR I and II have different mode-register-set definition */
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
		printf("DDR:only CAS Latency 1.5,2.0,2.5,3.0 is supported.\n");
		return 0;
	}
	debug ("DDR:sdram_mode=0x%08x\n",ddr->sdram_mode);

	switch(spd.refresh) {
	case 0x00:
	case 0x80:
		tmp = ns2clk(15625);
		break;
	case 0x01:
	case 0x81:
		tmp = ns2clk(3900);
		break;
	case 0x02:
	case 0x82:
		tmp = ns2clk(7800);
		break;
	case 0x03:
	case 0x83:
		tmp = ns2clk(31300);
		break;
	case 0x04:
	case 0x84:
		tmp = ns2clk(62500);
		break;
	case 0x05:
	case 0x85:
		tmp = ns2clk(125000);
		break;
	default:
		tmp = 0x512;
		break;
	}

	/* set BSTOPRE to 0x100 for page mode, if auto-charge is used, set BSTOPRE = 0 */
	ddr->sdram_interval = ((tmp & 0x3fff) << 16) | 0x100;
	debug ("DDR:sdram_interval=0x%08x\n",ddr->sdram_interval);

	/* is this an ECC DDR chip? */
#if defined(CONFIG_DDR_ECC)
	if(spd.config == 0x02) {
		ddr->err_disable = 0x0000000d;
		ddr->err_sbe = 0x00ff0000;
	}
	debug ("DDR:err_disable=0x%08x\n",ddr->err_disable);
	debug ("DDR:err_sbe=0x%08x\n",ddr->err_sbe);
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

	/* Figure out the settings for the sdram_cfg register.  Build up
	 * the entire register in 'tmp' before writing since the write into
	 * the register will actually enable the memory controller, and all
	 * settings must be done before enabling.
	 *
	 * sdram_cfg[0]   = 1 (ddr sdram logic enable)
	 * sdram_cfg[1]   = 1 (self-refresh-enable)
	 * sdram_cfg[6:7] = 2 (SDRAM type = DDR SDRAM)
	 */
	tmp = 0xc2000000;

	/* sdram_cfg[3] = RD_EN - registered DIMM enable
	 *   A value of 0x26 indicates micron registered DIMMS (micron.com)
	 */
	if (spd.mod_attr == 0x26) {
		tmp |= 0x10000000;
	}

#if defined(CONFIG_DDR_ECC)
	/* If the user wanted ECC (enabled via sdram_cfg[2]) */
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

	debug ("DDR:sdram_cfg=0x%08x\n",ddr->sdram_cfg);

    	return (memsize*1024*1024);
}

#endif /* CONFIG_SPD_EEPROM */
