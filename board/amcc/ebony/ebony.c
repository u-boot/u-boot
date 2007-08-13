/*
 *  Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
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

#define BOOT_SMALL_FLASH	32	/* 00100000 */
#define FLASH_ONBD_N		2	/* 00000010 */
#define FLASH_SRAM_SEL		1	/* 00000001 */

DECLARE_GLOBAL_DATA_PTR;

long int fixed_sdram(void);

int board_early_init_f(void)
{
	uint reg;
	unsigned char *fpga_base = (unsigned char *)CFG_FPGA_BASE;
	unsigned char status;

	/*--------------------------------------------------------------------
	 * Setup the external bus controller/chip selects
	 *-------------------------------------------------------------------*/
	mtdcr(ebccfga, xbcfg);
	reg = mfdcr(ebccfgd);
	mtdcr(ebccfgd, reg | 0x04000000);	/* Set ATC */

	mtebc(pb1ap, 0x02815480);	/* NVRAM/RTC */
	mtebc(pb1cr, 0x48018000);	/* BA=0x480 1MB R/W 8-bit */
	mtebc(pb7ap, 0x01015280);	/* FPGA registers */
	mtebc(pb7cr, 0x48318000);	/* BA=0x483 1MB R/W 8-bit */

	/* read FPGA_REG0  and set the bus controller */
	status = *fpga_base;
	if ((status & BOOT_SMALL_FLASH) && !(status & FLASH_ONBD_N)) {
		mtebc(pb0ap, 0x9b015480);	/* FLASH/SRAM */
		mtebc(pb0cr, 0xfff18000);	/* BAS=0xfff 1MB R/W 8-bit */
		mtebc(pb2ap, 0x9b015480);	/* 4MB FLASH */
		mtebc(pb2cr, 0xff858000);	/* BAS=0xff8 4MB R/W 8-bit */
	} else {
		mtebc(pb0ap, 0x9b015480);	/* 4MB FLASH */
		mtebc(pb0cr, 0xffc58000);	/* BAS=0xffc 4MB R/W 8-bit */

		/* set CS2 if FLASH_ONBD_N == 0 */
		if (!(status & FLASH_ONBD_N)) {
			mtebc(pb2ap, 0x9b015480);	/* FLASH/SRAM */
			mtebc(pb2cr, 0xff818000);	/* BAS=0xff8 4MB R/W 8-bit */
		}
	}

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(uic0sr, 0xffffffff);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000009);	/* SMI & UIC1 crit are critical */
	mtdcr(uic0pr, 0xfffffe13);	/* per ref-board manual */
	mtdcr(uic0tr, 0x01c00008);	/* per ref-board manual */
	mtdcr(uic0vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic0sr, 0xffffffff);	/* clear all */

	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000000);	/* all non-critical */
	mtdcr(uic1pr, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(uic1tr, 0x00ffc000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: Ebony - AMCC PPC440GP Evaluation Board");
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

long int initdram(int board_type)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram();
#else
	dram_size = fixed_sdram();
#endif
	return dram_size;
}

#if defined(CFG_DRAM_TEST)
int testdram(void)
{
	uint *pstart = (uint *) 0x00000000;
	uint *pend = (uint *) 0x08000000;
	uint *p;

	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf("SDRAM test fails at: %08x\n", (uint) p);
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
 *  Assumes:    128 MB, non-ECC, non-registered
 *              PLB @ 133 MHz
 *
 ************************************************************************/
long int fixed_sdram(void)
{
	uint reg;

	/*--------------------------------------------------------------------
	 * Setup some default
	 *------------------------------------------------------------------*/
	mtsdram(mem_uabba, 0x00000000);	/* ubba=0 (default)             */
	mtsdram(mem_slio, 0x00000000);	/* rdre=0 wrre=0 rarw=0         */
	mtsdram(mem_devopt, 0x00000000);	/* dll=0 ds=0 (normal)          */
	mtsdram(mem_wddctr, 0x00000000);	/* wrcp=0 dcd=0                 */
	mtsdram(mem_clktr, 0x40000000);	/* clkp=1 (90 deg wr) dcdt=0    */

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram(mem_b0cr, 0x000a4001);	/* SDBA=0x000 128MB, Mode 3, enabled */
	mtsdram(mem_tr0, 0x410a4012);	/* WR=2  WD=1 CL=2.5 PA=3 CP=4 LD=2 */
	/* RA=10 RD=3                       */
	mtsdram(mem_tr1, 0x8080082f);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x02f   */
	mtsdram(mem_rtr, 0x08200000);	/* Rate 15.625 ns @ 133 MHz PLB     */
	mtsdram(mem_cfg1, 0x00000000);	/* Self-refresh exit, disable PM    */
	udelay(400);		/* Delay 200 usecs (min)            */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram(mem_cfg0, 0x86000000);	/* DCEN=1, PMUD=1, 64-bit           */
	for (;;) {
		mfsdram(mem_mcsts, reg);
		if (reg & 0x80000000)
			break;
	}

	return (128 * 1024 * 1024);	/* 128 MB                           */
}
#endif				/* !defined(CONFIG_SPD_EEPROM) */

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
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long strap;

	/*--------------------------------------------------------------------------+
	 * The ebony board is always configured as the host & requires the
	 * PCI arbiter to be enabled.
	 *--------------------------------------------------------------------------*/
	strap = mfdcr(cpc0_strp1);
	if ((strap & 0x00100000) == 0) {
		printf("PCI: CPC0_STRP1[PAE] not set.\n");
		return 0;
	}

	return 1;
}
#endif	/* defined(CONFIG_PCI) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Disable everything
	 *--------------------------------------------------------------------------*/
	out32r(PCIX0_PIM0SA, 0);	/* disable */
	out32r(PCIX0_PIM1SA, 0);	/* disable */
	out32r(PCIX0_PIM2SA, 0);	/* disable */
	out32r(PCIX0_EROMBA, 0);	/* disable expansion rom */

	/*--------------------------------------------------------------------------+
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440 strapping
     * options to not support sizes such as 128/256 MB.
	 *--------------------------------------------------------------------------*/
	out32r(PCIX0_PIM0LAL, CFG_SDRAM_BASE);
	out32r(PCIX0_PIM0LAH, 0);
	out32r(PCIX0_PIM0SA, ~(gd->ram_size - 1) | 1);

	out32r(PCIX0_BAR0, 0);

	/*--------------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *--------------------------------------------------------------------------*/
	out16r(PCIX0_SBSYSVID, CFG_PCI_SUBSYS_VENDORID);
	out16r(PCIX0_SBSYSID, CFG_PCI_SUBSYS_DEVICEID);

	out16r(PCIX0_CMD, in16r(PCIX0_CMD) | PCI_COMMAND_MEMORY);
}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */

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
	/* The ebony board is always configured as host. */
	return (1);
}
#endif				/* defined(CONFIG_PCI) */
