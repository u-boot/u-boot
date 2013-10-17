/*
 *  Copyright (C) 2004 PaulReynolds@lhsolutions.com
 *
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include "ocotea.h"
#include <asm/processor.h>
#include <spd_sdram.h>
#include <asm/ppc4xx-emac.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOOT_SMALL_FLASH	32	/* 00100000 */
#define FLASH_ONBD_N		2	/* 00000010 */
#define FLASH_SRAM_SEL		1	/* 00000001 */

long int fixed_sdram (void);
void fpga_init (void);

int board_early_init_f (void)
{
	unsigned long mfr;
	unsigned char *fpga_base = (unsigned char *) CONFIG_SYS_FPGA_BASE;
	unsigned char switch_status;
	unsigned long cs0_base;
	unsigned long cs0_size;
	unsigned long cs0_twt;
	unsigned long cs2_base;
	unsigned long cs2_size;
	unsigned long cs2_twt;

	/*-------------------------------------------------------------------------+
	  | Initialize EBC CONFIG
	  +-------------------------------------------------------------------------*/
	mtebc(EBC0_CFG, EBC_CFG_LE_UNLOCK |
	      EBC_CFG_PTD_ENABLE | EBC_CFG_RTC_64PERCLK |
	      EBC_CFG_ATC_PREVIOUS | EBC_CFG_DTC_PREVIOUS |
	      EBC_CFG_CTC_PREVIOUS | EBC_CFG_EMC_NONDEFAULT |
	      EBC_CFG_PME_DISABLE | EBC_CFG_PR_32);

	/*-------------------------------------------------------------------------+
	  | FPGA. Initialize bank 7 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB7AP, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(7)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB7CR, EBC_BXCR_BAS_ENCODE(0x48300000)|
	      EBC_BXCR_BS_1MB|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/* read FPGA base register FPGA_REG0 */
	switch_status = *fpga_base;

	if (switch_status & 0x40) {
		cs0_base = 0xFFE00000;
		cs0_size = EBC_BXCR_BS_2MB;
		cs0_twt = 8;
		cs2_base = 0xFF800000;
		cs2_size = EBC_BXCR_BS_4MB;
		cs2_twt = 10;
	} else {
		cs0_base = 0xFFC00000;
		cs0_size = EBC_BXCR_BS_4MB;
		cs0_twt = 10;
		cs2_base = 0xFF800000;
		cs2_size = EBC_BXCR_BS_2MB;
		cs2_twt = 8;
	}

	/*-------------------------------------------------------------------------+
	  | 1 MB FLASH / 1 MB SRAM. Initialize bank 0 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB0AP, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(cs0_twt)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB0CR, EBC_BXCR_BAS_ENCODE(cs0_base)|
	      cs0_size|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | 8KB NVRAM/RTC. Initialize bank 1 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB1AP, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(10)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB1CR, EBC_BXCR_BAS_ENCODE(0x48000000)|
	      EBC_BXCR_BS_1MB|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | 4 MB FLASH. Initialize bank 2 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB2AP, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(cs2_twt)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB2CR, EBC_BXCR_BAS_ENCODE(cs2_base)|
	      cs2_size|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | FPGA. Initialize bank 7 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB7AP, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(7)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB7CR, EBC_BXCR_BAS_ENCODE(0x48300000)|
	      EBC_BXCR_BS_1MB|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	/*
	 * Because of the interrupt handling rework to handle 440GX interrupts
	 * with the common code, we needed to change names of the UIC registers.
	 * Here the new relationship:
	 *
	 * U-Boot name	440GX name
	 * -----------------------
	 * UIC0		UICB0
	 * UIC1		UIC0
	 * UIC2		UIC1
	 * UIC3		UIC2
	 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */
	mtdcr (UIC1ER, 0x00000000);	/* disable all */
	mtdcr (UIC1CR, 0x00000009);	/* SMI & UIC1 crit are critical */
	mtdcr (UIC1PR, 0xfffffe13);	/* per ref-board manual */
	mtdcr (UIC1TR, 0x01c00008);	/* per ref-board manual */
	mtdcr (UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */

	mtdcr (UIC2SR, 0xffffffff);	/* clear all */
	mtdcr (UIC2ER, 0x00000000);	/* disable all */
	mtdcr (UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC2PR, 0xffffe0ff);	/* per ref-board manual */
	mtdcr (UIC2TR, 0x00ffc000);	/* per ref-board manual */
	mtdcr (UIC2VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC2SR, 0xffffffff);	/* clear all */

	mtdcr (UIC3SR, 0xffffffff);	/* clear all */
	mtdcr (UIC3ER, 0x00000000);	/* disable all */
	mtdcr (UIC3CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC3PR, 0xffffffff);	/* per ref-board manual */
	mtdcr (UIC3TR, 0x00ff8c0f);	/* per ref-board manual */
	mtdcr (UIC3VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC3SR, 0xffffffff);	/* clear all */

	mtdcr (UIC0SR, 0xfc000000); /* clear all */
	mtdcr (UIC0ER, 0x00000000); /* disable all */
	mtdcr (UIC0CR, 0x00000000); /* all non-critical */
	mtdcr (UIC0PR, 0xfc000000); /* */
	mtdcr (UIC0TR, 0x00000000); /* */
	mtdcr (UIC0VR, 0x00000001); /* */
	mfsdr (SDR0_MFR, mfr);
	mfr &= ~SDR0_MFR_ECS_MASK;
/*	mtsdr(SDR0_MFR, mfr); */
	fpga_init();

	return 0;
}


int checkboard (void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf ("Board: Ocotea - AMCC PPC440GX Evaluation Board");
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc ('\n');

	return (0);
}


phys_size_t initdram (int board_type)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = fixed_sdram ();
#endif
	return dram_size;
}


#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 *
 *  Assumes:    128 MB, non-ECC, non-registered
 *              PLB @ 133 MHz
 *
 ************************************************************************/
long int fixed_sdram (void)
{
	uint reg;

	/*--------------------------------------------------------------------
	 * Setup some default
	 *------------------------------------------------------------------*/
	mtsdram (SDRAM0_UABBA, 0x00000000);	/* ubba=0 (default)             */
	mtsdram (SDRAM0_SLIO, 0x00000000);		/* rdre=0 wrre=0 rarw=0         */
	mtsdram (SDRAM0_DEVOPT, 0x00000000);	/* dll=0 ds=0 (normal)          */
	mtsdram (SDRAM0_WDDCTR, 0x00000000);	/* wrcp=0 dcd=0                 */
	mtsdram (SDRAM0_CLKTR, 0x40000000);	/* clkp=1 (90 deg wr) dcdt=0    */

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram (SDRAM0_B0CR, 0x000a4001);	/* SDBA=0x000 128MB, Mode 3, enabled */
	mtsdram (SDRAM0_TR0, 0x410a4012);	/* WR=2  WD=1 CL=2.5 PA=3 CP=4 LD=2 */
	/* RA=10 RD=3                       */
	mtsdram (SDRAM0_TR1, 0x8080082f);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x02f   */
	mtsdram (SDRAM0_RTR, 0x08200000);	/* Rate 15.625 ns @ 133 MHz PLB     */
	mtsdram (SDRAM0_CFG1, 0x00000000);	/* Self-refresh exit, disable PM    */
	udelay (400);			/* Delay 200 usecs (min)            */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram (SDRAM0_CFG0, 0x86000000);	/* DCEN=1, PMUD=1, 64-bit           */
	for (;;) {
		mfsdram (SDRAM0_MCSTS, reg);
		if (reg & 0x80000000)
			break;
	}

	return (128 * 1024 * 1024);	/* 128 MB                           */
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */

void fpga_init(void)
{
	unsigned long group;
	unsigned long sdr0_pfc0;
	unsigned long sdr0_pfc1;
	unsigned long sdr0_cust0;
	unsigned long pvr;

	mfsdr (SDR0_PFC0, sdr0_pfc0);
	mfsdr (SDR0_PFC1, sdr0_pfc1);
	group = SDR0_PFC1_EPS_DECODE(sdr0_pfc1);
	pvr = get_pvr ();

	sdr0_pfc0 = (sdr0_pfc0 & ~SDR0_PFC0_GEIE_MASK) | SDR0_PFC0_GEIE_TRE;
	if ( ((pvr == PVR_440GX_RA) || (pvr == PVR_440GX_RB)) && ((group == 4) || (group == 5))) {
		sdr0_pfc0 = (sdr0_pfc0 & ~SDR0_PFC0_TRE_MASK) | SDR0_PFC0_TRE_DISABLE;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_CTEMS_MASK) | SDR0_PFC1_CTEMS_EMS;
		out8(FPGA_REG2, (in8(FPGA_REG2) & ~FPGA_REG2_EXT_INTFACE_MASK) |
		     FPGA_REG2_EXT_INTFACE_ENABLE);
		mtsdr (SDR0_PFC0, sdr0_pfc0);
		mtsdr (SDR0_PFC1, sdr0_pfc1);
	} else {
		sdr0_pfc0 = (sdr0_pfc0 & ~SDR0_PFC0_TRE_MASK) | SDR0_PFC0_TRE_ENABLE;
		switch (group)
		{
		case 0:
		case 1:
		case 2:
			/* CPU trace A */
			out8(FPGA_REG2, (in8(FPGA_REG2) & ~FPGA_REG2_EXT_INTFACE_MASK) |
			     FPGA_REG2_EXT_INTFACE_ENABLE);
			sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_CTEMS_MASK) | SDR0_PFC1_CTEMS_EMS;
			mtsdr (SDR0_PFC0, sdr0_pfc0);
			mtsdr (SDR0_PFC1, sdr0_pfc1);
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			/* CPU trace B - Over EBMI */
			sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_CTEMS_MASK) | SDR0_PFC1_CTEMS_CPUTRACE;
			mtsdr (SDR0_PFC0, sdr0_pfc0);
			mtsdr (SDR0_PFC1, sdr0_pfc1);
			out8(FPGA_REG2, (in8(FPGA_REG2) & ~FPGA_REG2_EXT_INTFACE_MASK) |
			     FPGA_REG2_EXT_INTFACE_DISABLE);
			break;
		}
	}

	/* Initialize the ethernet specific functions in the fpga */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	mfsdr(SDR0_CUST0, sdr0_cust0);
	if ( (SDR0_PFC1_EPS_DECODE(sdr0_pfc1) == 4) &&
	    ((SDR0_CUST0_RGMII2_DECODE(sdr0_cust0) == RGMII_FER_GMII) ||
	     (SDR0_CUST0_RGMII2_DECODE(sdr0_cust0) == RGMII_FER_TBI)))
	{
		if ((in8(FPGA_REG0) & FPGA_REG0_ECLS_MASK) == FPGA_REG0_ECLS_VER1)
		{
			out8(FPGA_REG3, (in8(FPGA_REG3) & ~FPGA_REG3_ENET_MASK1) |
			     FPGA_REG3_ENET_GROUP7);
		}
		else
		{
			if (SDR0_CUST0_RGMII2_DECODE(sdr0_cust0) == RGMII_FER_GMII)
			{
				out8(FPGA_REG3, (in8(FPGA_REG3) & ~FPGA_REG3_ENET_MASK2) |
				     FPGA_REG3_ENET_GROUP7);
			}
			else
			{
				out8(FPGA_REG3, (in8(FPGA_REG3) & ~FPGA_REG3_ENET_MASK2) |
				     FPGA_REG3_ENET_GROUP8);
			}
		}
	}
	else
	{
		if ((in8(FPGA_REG0) & FPGA_REG0_ECLS_MASK) == FPGA_REG0_ECLS_VER1)
		{
			out8(FPGA_REG3, (in8(FPGA_REG3) & ~FPGA_REG3_ENET_MASK1) |
			     FPGA_REG3_ENET_ENCODE1(SDR0_PFC1_EPS_DECODE(sdr0_pfc1)));
		}
		else
		{
			out8(FPGA_REG3, (in8(FPGA_REG3) & ~FPGA_REG3_ENET_MASK2) |
			     FPGA_REG3_ENET_ENCODE2(SDR0_PFC1_EPS_DECODE(sdr0_pfc1)));
		}
	}
	out8(FPGA_REG4, FPGA_REG4_GPHY_MODE10 |
	     FPGA_REG4_GPHY_MODE100 | FPGA_REG4_GPHY_MODE1000 |
	     FPGA_REG4_GPHY_FRC_DPLX | FPGA_REG4_CONNECT_PHYS);

	/* reset the gigabyte phy if necessary */
	if (SDR0_PFC1_EPS_DECODE(sdr0_pfc1) >= 3)
	{
		if ((in8(FPGA_REG0) & FPGA_REG0_ECLS_MASK) == FPGA_REG0_ECLS_VER1)
		{
			out8(FPGA_REG3, in8(FPGA_REG3) & ~FPGA_REG3_GIGABIT_RESET_DISABLE);
			udelay(10000);
			out8(FPGA_REG3, in8(FPGA_REG3) | FPGA_REG3_GIGABIT_RESET_DISABLE);
		}
		else
		{
			out8(FPGA_REG2, in8(FPGA_REG2) & ~FPGA_REG2_GIGABIT_RESET_DISABLE);
			udelay(10000);
			out8(FPGA_REG2, in8(FPGA_REG2) | FPGA_REG2_GIGABIT_RESET_DISABLE);
		}
	}

	/*
	 * new Ocotea with Rev. F (pass 3) chips has SMII PHY reset
	 */
	if ((in8(FPGA_REG0) & FPGA_REG0_ECLS_MASK) == FPGA_REG0_ECLS_VER2) {
		out8(FPGA_REG2, in8(FPGA_REG2) & ~FPGA_REG2_SMII_RESET_DISABLE);
		udelay(10000);
		out8(FPGA_REG2, in8(FPGA_REG2) | FPGA_REG2_SMII_RESET_DISABLE);
	}

	/* Turn off the LED's */
	out8(FPGA_REG3, (in8(FPGA_REG3) & ~FPGA_REG3_STAT_MASK) |
	     FPGA_REG3_STAT_LED8_DISAB | FPGA_REG3_STAT_LED4_DISAB |
	     FPGA_REG3_STAT_LED2_DISAB | FPGA_REG3_STAT_LED1_DISAB);

	return;
}
