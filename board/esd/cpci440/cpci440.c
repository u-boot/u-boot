/*
 * (C) Copyright 2002
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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


extern void lxt971_no_sleep(void);


long int fixed_sdram( void );

int board_early_init_f (void)
{
	uint reg;

	/*--------------------------------------------------------------------
	 * Setup the external bus controller/chip selects
	 *-------------------------------------------------------------------*/
	mtdcr( ebccfga, xbcfg );
	reg = mfdcr( ebccfgd );
	mtdcr( ebccfgd, reg | 0x04000000 );	/* Set ATC */

	mtebc( pb0ap, 0x92015480 );	/* FLASH/SRAM */
	mtebc( pb0cr, 0xFF87A000 ); /* BAS=0xff8 8MB R/W 16-bit */
	/* test-only: other regs still missing... */

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr( uic0sr, 0xffffffff );    /* clear all */
	mtdcr( uic0er, 0x00000000 );    /* disable all */
	mtdcr( uic0cr, 0x00000009 );    /* SMI & UIC1 crit are critical */
	mtdcr( uic0pr, 0xfffffe13 );    /* per ref-board manual */
	mtdcr( uic0tr, 0x01c00008 );    /* per ref-board manual */
	mtdcr( uic0vr, 0x00000001 );    /* int31 highest, base=0x000 */
	mtdcr( uic0sr, 0xffffffff );    /* clear all */

	mtdcr( uic1sr, 0xffffffff );    /* clear all */
	mtdcr( uic1er, 0x00000000 );    /* disable all */
	mtdcr( uic1cr, 0x00000000 );    /* all non-critical */
	mtdcr( uic1pr, 0xffffe0ff );    /* per ref-board manual */
	mtdcr( uic1tr, 0x00ffc000 );    /* per ref-board manual */
	mtdcr( uic1vr, 0x00000001 );    /* int31 highest, base=0x000 */
	mtdcr( uic1sr, 0xffffffff );    /* clear all */

	return 0;
}


int checkboard (void)
{
	sys_info_t sysinfo;
	get_sys_info(&sysinfo);

	printf("Board: esd CPCI-440\n");
	printf("\tVCO: %lu MHz\n", sysinfo.freqVCOMhz/1000000);
	printf("\tCPU: %lu MHz\n", sysinfo.freqProcessor/1000000);
	printf("\tPLB: %lu MHz\n", sysinfo.freqPLB/1000000);
	printf("\tOPB: %lu MHz\n", sysinfo.freqOPB/1000000);
	printf("\tEPB: %lu MHz\n", sysinfo.freqEPB/1000000);

	/*
	 * Disable sleep mode in LXT971
	 */
	lxt971_no_sleep();

	return (0);
}


long int initdram (int board_type)
{
	long    dram_size = 0;

	dram_size = fixed_sdram();
	return dram_size;
}


/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 *
 *  Assumes:    64 MB, non-ECC, non-registered
 *              PLB @ 133 MHz
 *
 ************************************************************************/
long int fixed_sdram( void )
{
	uint    reg;

#if 1 /* test-only */
	/*--------------------------------------------------------------------
	 * Setup some default
	 *------------------------------------------------------------------*/
	mtsdram( mem_uabba, 0x00000000 );   /* ubba=0 (default)             */
	mtsdram( mem_slio,  0x00000000 );   /* rdre=0 wrre=0 rarw=0         */
	mtsdram( mem_devopt,0x00000000 );   /* dll=0 ds=0 (normal)          */
	mtsdram( mem_wddctr,0x40000000 );   /* wrcp=0 dcd=0                 */
	mtsdram( mem_clktr, 0x40000000 );   /* clkp=1 (90 deg wr) dcdt=0    */

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram( mem_b0cr, 0x00082001 );/* SDBA=0x000, 64MB, Mode 2, enabled*/
	mtsdram( mem_tr0,  0x410a4012 );/* WR=2  WD=1 CL=2.5 PA=3 CP=4 LD=2 */
	/* RA=10 RD=3                       */
	mtsdram( mem_tr1,  0x8080082f );/* SS=T2 SL=STAGE 3 CD=1 CT=0x02f   */
	mtsdram( mem_rtr,  0x08200000 );/* Rate 15.625 ns @ 133 MHz PLB     */
	mtsdram( mem_cfg1, 0x00000000 );/* Self-refresh exit, disable PM    */
	udelay( 400 );                  /* Delay 200 usecs (min)            */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram( mem_cfg0, 0x86000000 );/* DCEN=1, PMUD=1, 64-bit           */
	for(;;)
	{
		mfsdram( mem_mcsts, reg );
		if( reg & 0x80000000 )
			break;
	}

	return( 64 * 1024 * 1024 );      /* 64 MB                           */
#else
	return( 32 * 1024 * 1024 );      /* 64 MB                           */
#endif
}
