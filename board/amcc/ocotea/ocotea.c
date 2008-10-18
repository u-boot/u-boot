/*
 *  Copyright (C) 2004 PaulReynolds@lhsolutions.com
 *
 * (C) Copyright 2005
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


#include <common.h>
#include "ocotea.h"
#include <asm/processor.h>
#include <spd_sdram.h>
#include <ppc4xx_enet.h>

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
	mtebc(xbcfg, EBC_CFG_LE_UNLOCK |
	      EBC_CFG_PTD_ENABLE | EBC_CFG_RTC_64PERCLK |
	      EBC_CFG_ATC_PREVIOUS | EBC_CFG_DTC_PREVIOUS |
	      EBC_CFG_CTC_PREVIOUS | EBC_CFG_EMC_NONDEFAULT |
	      EBC_CFG_PME_DISABLE | EBC_CFG_PR_32);

	/*-------------------------------------------------------------------------+
	  | FPGA. Initialize bank 7 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(pb7ap, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(7)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(pb7cr, EBC_BXCR_BAS_ENCODE(0x48300000)|
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
	mtebc(pb0ap, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(cs0_twt)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(pb0cr, EBC_BXCR_BAS_ENCODE(cs0_base)|
	      cs0_size|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | 8KB NVRAM/RTC. Initialize bank 1 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(pb1ap, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(10)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(pb1cr, EBC_BXCR_BAS_ENCODE(0x48000000)|
	      EBC_BXCR_BS_1MB|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | 4 MB FLASH. Initialize bank 2 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(pb2ap, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(cs2_twt)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(pb2cr, EBC_BXCR_BAS_ENCODE(cs2_base)|
	      cs2_size|EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | FPGA. Initialize bank 7 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(pb7ap, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(7)|
	      EBC_BXAP_BCE_DISABLE|
	      EBC_BXAP_CSN_ENCODE(1)|EBC_BXAP_OEN_ENCODE(1)|
	      EBC_BXAP_WBN_ENCODE(1)|EBC_BXAP_WBF_ENCODE(1)|
	      EBC_BXAP_TH_ENCODE(1)|EBC_BXAP_RE_DISABLED|
	      EBC_BXAP_BEM_WRITEONLY|
	      EBC_BXAP_PEN_DISABLED);
	mtebc(pb7cr, EBC_BXCR_BAS_ENCODE(0x48300000)|
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
	mtdcr (uic1sr, 0xffffffff);	/* clear all */
	mtdcr (uic1er, 0x00000000);	/* disable all */
	mtdcr (uic1cr, 0x00000009);	/* SMI & UIC1 crit are critical */
	mtdcr (uic1pr, 0xfffffe13);	/* per ref-board manual */
	mtdcr (uic1tr, 0x01c00008);	/* per ref-board manual */
	mtdcr (uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic1sr, 0xffffffff);	/* clear all */

	mtdcr (uic2sr, 0xffffffff);	/* clear all */
	mtdcr (uic2er, 0x00000000);	/* disable all */
	mtdcr (uic2cr, 0x00000000);	/* all non-critical */
	mtdcr (uic2pr, 0xffffe0ff);	/* per ref-board manual */
	mtdcr (uic2tr, 0x00ffc000);	/* per ref-board manual */
	mtdcr (uic2vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic2sr, 0xffffffff);	/* clear all */

	mtdcr (uic3sr, 0xffffffff);	/* clear all */
	mtdcr (uic3er, 0x00000000);	/* disable all */
	mtdcr (uic3cr, 0x00000000);	/* all non-critical */
	mtdcr (uic3pr, 0xffffffff);	/* per ref-board manual */
	mtdcr (uic3tr, 0x00ff8c0f);	/* per ref-board manual */
	mtdcr (uic3vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic3sr, 0xffffffff);	/* clear all */

	mtdcr (uic0sr, 0xfc000000); /* clear all */
	mtdcr (uic0er, 0x00000000); /* disable all */
	mtdcr (uic0cr, 0x00000000); /* all non-critical */
	mtdcr (uic0pr, 0xfc000000); /* */
	mtdcr (uic0tr, 0x00000000); /* */
	mtdcr (uic0vr, 0x00000001); /* */
	mfsdr (sdr_mfr, mfr);
	mfr &= ~SDR0_MFR_ECS_MASK;
/*	mtsdr(sdr_mfr, mfr); */
	fpga_init();

	return 0;
}


int checkboard (void)
{
	char *s = getenv ("serial#");

	printf ("Board: Ocotea - AMCC PPC440GX Evaluation Board");
	if (s != NULL) {
		puts (", serial# ");
		puts (s);
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
	mtsdram (mem_uabba, 0x00000000);	/* ubba=0 (default)             */
	mtsdram (mem_slio, 0x00000000);		/* rdre=0 wrre=0 rarw=0         */
	mtsdram (mem_devopt, 0x00000000);	/* dll=0 ds=0 (normal)          */
	mtsdram (mem_wddctr, 0x00000000);	/* wrcp=0 dcd=0                 */
	mtsdram (mem_clktr, 0x40000000);	/* clkp=1 (90 deg wr) dcdt=0    */

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram (mem_b0cr, 0x000a4001);	/* SDBA=0x000 128MB, Mode 3, enabled */
	mtsdram (mem_tr0, 0x410a4012);	/* WR=2  WD=1 CL=2.5 PA=3 CP=4 LD=2 */
	/* RA=10 RD=3                       */
	mtsdram (mem_tr1, 0x8080082f);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x02f   */
	mtsdram (mem_rtr, 0x08200000);	/* Rate 15.625 ns @ 133 MHz PLB     */
	mtsdram (mem_cfg1, 0x00000000);	/* Self-refresh exit, disable PM    */
	udelay (400);			/* Delay 200 usecs (min)            */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram (mem_cfg0, 0x86000000);	/* DCEN=1, PMUD=1, 64-bit           */
	for (;;) {
		mfsdram (mem_mcsts, reg);
		if (reg & 0x80000000)
			break;
	}

	return (128 * 1024 * 1024);	/* 128 MB                           */
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */


/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller * hose )
{
	unsigned long strap;

	/*--------------------------------------------------------------------------+
	 *	The ocotea board is always configured as the host & requires the
	 *	PCI arbiter to be enabled.
	 *--------------------------------------------------------------------------*/
	mfsdr(sdr_sdstp1, strap);
	if( (strap & SDR0_SDSTP1_PAE_MASK) == 0 ){
		printf("PCI: SDR0_STRP1[%08lX] - PCI Arbiter disabled.\n",strap);
		return 0;
	}

	return 1;
}
#endif /* defined(CONFIG_PCI) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller * hose )
{
	/*--------------------------------------------------------------------------+
	 * Disable everything
	 *--------------------------------------------------------------------------*/
	out32r( PCIX0_PIM0SA, 0 ); /* disable */
	out32r( PCIX0_PIM1SA, 0 ); /* disable */
	out32r( PCIX0_PIM2SA, 0 ); /* disable */
	out32r( PCIX0_EROMBA, 0 ); /* disable expansion rom */

	/*--------------------------------------------------------------------------+
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440 strapping
	 * options to not support sizes such as 128/256 MB.
	 *--------------------------------------------------------------------------*/
	out32r( PCIX0_PIM0LAL, CONFIG_SYS_SDRAM_BASE );
	out32r( PCIX0_PIM0LAH, 0 );
	out32r( PCIX0_PIM0SA, ~(gd->ram_size - 1) | 1 );

	out32r( PCIX0_BAR0, 0 );

	/*--------------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *--------------------------------------------------------------------------*/
	out16r( PCIX0_SBSYSVID, CONFIG_SYS_PCI_SUBSYS_VENDORID );
	out16r( PCIX0_SBSYSID, CONFIG_SYS_PCI_SUBSYS_DEVICEID );

	out16r( PCIX0_CMD, in16r(PCIX0_CMD) | PCI_COMMAND_MEMORY );
}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */


/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
    /* The ocotea board is always configured as host. */
    return(1);
}
#endif /* defined(CONFIG_PCI) */


void fpga_init(void)
{
	unsigned long group;
	unsigned long sdr0_pfc0;
	unsigned long sdr0_pfc1;
	unsigned long sdr0_cust0;
	unsigned long pvr;

	mfsdr (sdr_pfc0, sdr0_pfc0);
	mfsdr (sdr_pfc1, sdr0_pfc1);
	group = SDR0_PFC1_EPS_DECODE(sdr0_pfc1);
	pvr = get_pvr ();

	sdr0_pfc0 = (sdr0_pfc0 & ~SDR0_PFC0_GEIE_MASK) | SDR0_PFC0_GEIE_TRE;
	if ( ((pvr == PVR_440GX_RA) || (pvr == PVR_440GX_RB)) && ((group == 4) || (group == 5))) {
		sdr0_pfc0 = (sdr0_pfc0 & ~SDR0_PFC0_TRE_MASK) | SDR0_PFC0_TRE_DISABLE;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_CTEMS_MASK) | SDR0_PFC1_CTEMS_EMS;
		out8(FPGA_REG2, (in8(FPGA_REG2) & ~FPGA_REG2_EXT_INTFACE_MASK) |
		     FPGA_REG2_EXT_INTFACE_ENABLE);
		mtsdr (sdr_pfc0, sdr0_pfc0);
		mtsdr (sdr_pfc1, sdr0_pfc1);
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
			mtsdr (sdr_pfc0, sdr0_pfc0);
			mtsdr (sdr_pfc1, sdr0_pfc1);
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			/* CPU trace B - Over EBMI */
			sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_CTEMS_MASK) | SDR0_PFC1_CTEMS_CPUTRACE;
			mtsdr (sdr_pfc0, sdr0_pfc0);
			mtsdr (sdr_pfc1, sdr0_pfc1);
			out8(FPGA_REG2, (in8(FPGA_REG2) & ~FPGA_REG2_EXT_INTFACE_MASK) |
			     FPGA_REG2_EXT_INTFACE_DISABLE);
			break;
		}
	}

	/* Initialize the ethernet specific functions in the fpga */
	mfsdr(sdr_pfc1, sdr0_pfc1);
	mfsdr(sdr_cust0, sdr0_cust0);
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

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{

	return (ctrlc());
}
#endif
