/*
 *  Copyright (C) 2003 Travis B. Sawyer	 <travis.sawyer@sandburst.com>
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
#include <spd_sdram.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOOT_SMALL_FLASH	32	/* 00100000 */
#define FLASH_ONBD_N		2	/* 00000010 */
#define FLASH_SRAM_SEL		1	/* 00000001 */

long int fixed_sdram (void);

int board_early_init_f(void)
{
	unsigned long sdrreg;
	/* TBS:	 Setup the GPIO access for the user LEDs */
	mfsdr(sdr_pfc0, sdrreg);
	mtsdr(sdr_pfc0, (sdrreg & ~0x00000100) | 0x00000E00);
	out32(CFG_GPIO_BASE + 0x018, (USR_LED0 | USR_LED1 | USR_LED2 | USR_LED3));
	LED0_OFF();
	LED1_OFF();
	LED2_OFF();
	LED3_OFF();

	/*--------------------------------------------------------------------
	 * Setup the external bus controller/chip selects
	 *-------------------------------------------------------------------*/

	/* set the bus controller */
	mtebc (pb0ap, 0x04055200);	/* FLASH/SRAM */
	mtebc (pb0cr, 0xfff18000);	/* BAS=0xfff 1MB R/W 8-bit */
	mtebc (pb1ap, 0x04055200);	/* FLASH/SRAM */
	mtebc (pb1cr, 0xfe098000);	/* BAS=0xff8 16MB R/W 8-bit */

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr (uic0sr, 0xffffffff);	/* clear all */
	mtdcr (uic0er, 0x00000000);	/* disable all */
	mtdcr (uic0cr, 0x00000003);	/* SMI & UIC1 crit are critical */
	mtdcr (uic0pr, 0xfffffe00);	/* per ref-board manual */
	mtdcr (uic0tr, 0x01c00000);	/* per ref-board manual */
	mtdcr (uic0vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic0sr, 0xffffffff);	/* clear all */

	mtdcr (uic1sr, 0xffffffff);	/* clear all */
	mtdcr (uic1er, 0x00000000);	/* disable all */
	mtdcr (uic1cr, 0x00000000);	/* all non-critical */
	mtdcr (uic1pr, 0xffffc0ff);	/* per ref-board manual */
	mtdcr (uic1tr, 0x00ff8000);	/* per ref-board manual */
	mtdcr (uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic1sr, 0xffffffff);	/* clear all */

	mtdcr (uic2sr, 0xffffffff);	/* clear all */
	mtdcr (uic2er, 0x00000000);	/* disable all */
	mtdcr (uic2cr, 0x00000000);	/* all non-critical */
	mtdcr (uic2pr, 0xffffffff);	/* per ref-board manual */
	mtdcr (uic2tr, 0x00ff8c0f);	/* per ref-board manual */
	mtdcr (uic2vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic2sr, 0xffffffff);	/* clear all */

	mtdcr (uicb0sr, 0xfc000000); /* clear all */
	mtdcr (uicb0er, 0x00000000); /* disable all */
	mtdcr (uicb0cr, 0x00000000); /* all non-critical */
	mtdcr (uicb0pr, 0xfc000000); /* */
	mtdcr (uicb0tr, 0x00000000); /* */
	mtdcr (uicb0vr, 0x00000001); /* */

	LED0_ON();


	return 0;
}

int checkboard (void)
{
	printf ("Board: XES XPedite1000 440GX\n");

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


#if defined(CFG_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) 0x00000000;
	uint *pend = (uint *) 0x08000000;
	uint *p;

	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}
	return 0;
}
#endif

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 *
 *  Assumes:	128 MB, non-ECC, non-registered
 *		PLB @ 133 MHz
 *
 ************************************************************************/
long int fixed_sdram (void)
{
	uint reg;

	/*--------------------------------------------------------------------
	 * Setup some default
	 *------------------------------------------------------------------*/
	mtsdram (mem_uabba, 0x00000000);	/* ubba=0 (default)		*/
	mtsdram (mem_slio, 0x00000000);		/* rdre=0 wrre=0 rarw=0		*/
	mtsdram (mem_devopt, 0x00000000);	/* dll=0 ds=0 (normal)		*/
	mtsdram (mem_wddctr, 0x00000000);	/* wrcp=0 dcd=0			*/
	mtsdram (mem_clktr, 0x40000000);	/* clkp=1 (90 deg wr) dcdt=0	*/

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram (mem_b0cr, 0x000a4001); /* SDBA=0x000 128MB, Mode 3, enabled */
	mtsdram (mem_tr0, 0x410a4012);	/* WR=2	 WD=1 CL=2.5 PA=3 CP=4 LD=2 */
	/* RA=10 RD=3			    */
	mtsdram (mem_tr1, 0x8080082f);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x02f   */
	mtsdram (mem_rtr, 0x08200000);	/* Rate 15.625 ns @ 133 MHz PLB	    */
	mtsdram (mem_cfg1, 0x00000000); /* Self-refresh exit, disable PM    */
	udelay (400);			/* Delay 200 usecs (min)	    */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram (mem_cfg0, 0x86000000); /* DCEN=1, PMUD=1, 64-bit	    */
	for (;;) {
		mfsdram (mem_mcsts, reg);
		if (reg & 0x80000000)
			break;
	}

	return (128 * 1024 * 1024);	/* 128 MB			    */
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
	/* See if we're supposed to setup the pci */
	mfsdr(sdr_sdstp1, strap);
	if ((strap & 0x00010000) == 0) {
		return (0);
	}

#if defined(CFG_PCI_FORCE_PCI_CONV)
	/* Setup System Device Register PCIX0_XCR */
	mfsdr(sdr_xcr, strap);
	strap &= 0x0f000000;
	mtsdr(sdr_xcr, strap);
#endif
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
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
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
	out32r( PCIX0_PIM0LAL, CFG_SDRAM_BASE );
	out32r( PCIX0_PIM0LAH, 0 );
	out32r( PCIX0_PIM0SA, ~(gd->ram_size - 1) | 1 );

	out32r( PCIX0_BAR0, 0 );

	/*--------------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *--------------------------------------------------------------------------*/
	out16r( PCIX0_SBSYSVID, CFG_PCI_SUBSYS_VENDORID );
	out16r( PCIX0_SBSYSID, CFG_PCI_SUBSYS_DEVICEID );

	out16r( PCIX0_CMD, in16r(PCIX0_CMD) | PCI_COMMAND_MEMORY );
}
#endif /* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */


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
	return ((in32(CFG_GPIO_BASE + 0x1C) & 0x00000800) == 0);
}
#endif /* defined(CONFIG_PCI) */

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{

	return (ctrlc());
}

void post_word_store (ulong a)
{
	volatile ulong *save_addr =
		(volatile ulong *)(CFG_POST_WORD_ADDR);

	*save_addr = a;
}

ulong post_word_load (void)
{
	volatile ulong *save_addr =
		(volatile ulong *)(CFG_POST_WORD_ADDR);

	return *save_addr;
}
#endif

/*-----------------------------------------------------------------------------
 * board_get_enetaddr -- Read the MAC Addresses in the I2C EEPROM
 *-----------------------------------------------------------------------------
 */
static int enetaddr_num = 0;
void board_get_enetaddr (uchar * enet)
{
	int i;
	unsigned char buff[0x100], *cp;

	/* Initialize I2C					*/
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);

	/* Read 256 bytes in EEPROM				*/
	i2c_read (0x50, 0, 1, buff, 0x100);

	if (enetaddr_num == 0) {
		cp = &buff[0xF4];
		enetaddr_num = 1;
	}
	else
		cp = &buff[0xFA];

	for (i = 0; i < 6; i++,cp++)
		enet[i] = *cp;

	printf ("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n",
		enet[0], enet[1], enet[2], enet[3], enet[4], enet[5]);

}
