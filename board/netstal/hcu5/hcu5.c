/*
 *(C) Copyright 2005-2007 Netstal Maschinen AG
 *    Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <common.h>
#include <asm/processor.h>
#include <ppc440.h>
#include <asm/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

void sysLedSet(u32 value);

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#define mtcpr0(reg, data) do { mtdcr(CPR0_CFGADDR,reg); \
				mtdcr(CPR0_CFGDATA,data); } while (0)
#define mfcpr0(reg, data) do { mtdcr(CPR0_CFGADDR,reg); \
				data = mfdcr(CPR0_CFGDATA); } while (0)

#define SDR0_CP440			0x0180

/*
 * This function is run very early, out of flash, and before devices are
 * initialized. It is called by lib_ppc/board.c:board_init_f by virtue
 * of being in the init_sequence array.
 *
 * The SDRAM has been initialized already -- start.S:start called
 * init.S:init_sdram early on -- but it is not yet being used for
 * anything, not even stack. So be careful.
 */

int board_early_init_f(void)
{
	u32 reg;

#undef BOOTSTRAP_OPTION_A_ACTIVE
#ifdef BOOTSTRAP_OPTION_A_ACTIVE
	/* Booting with Bootstrap Option A
	 * First boot, with CPR0_ICFG_RLI_MASK == 0
	 * no we setup varios boot strapping register,
	 * then we do reset the PPC440 using a chip reset
	 * Unfortunately, we cannot use this option, as Nto1 is not set
	 * with Bootstrap Option A and cannot be changed later on by SW
	 * There are no other possible boostrap options with a 8 bit ROM
	 * See Errata (Version 1.04) CHIP_9
	 */

	u32 cpr0icfg;
	u32 dbcr;
	mfcpr0(CPR0_ICFG, cpr0icfg);
	if ( ! (cpr0icfg & CPR0_ICFG_RLI_MASK ) ) {
		mtcpr0(CPR0_MALD,   0x02000000);
		mtcpr0(CPR0_OPBD,   0x02000000);
	        mtcpr0(CPR0_PERD,   0x05000000);  /* 1:5 */
		mtcpr0(CPR0_PLLC,   0x40000238);
		mtcpr0(CPR0_PLLD,   0x01010414);
		mtcpr0(CPR0_PRIMAD, 0x01000000);
		mtcpr0(CPR0_PRIMBD, 0x01000000);
		mtcpr0(CPR0_SPCID,  0x03000000);
		mtsdr(SDR0_PFC0,    0x00003E00);  /* [CTE] = 0 */
		mtsdr(SDR0_CP440,   0x0EAAEA02);  /* [Nto1] = 1*/
		mtcpr0(CPR0_ICFG,   cpr0icfg | CPR0_ICFG_RLI_MASK);

		/*
		 * Initiate system reset in debug control register DBCR
		 */
		dbcr = mfspr(dbcr0);
		#define SYSTEM_RESET 0x30000000
		#define CHIP_RESET   0x20000000
		mtspr(dbcr0, dbcr | CHIP_RESET );
	}
	mtsdr(SDR0_CP440, 0x0EAAEA02);  /* [Nto1] = 1*/
#endif
	mtdcr(ebccfga, xbcfg);
	mtdcr(ebccfgd, 0xb8400000);

	/*--------------------------------------------------------------------
	 * Setup the GPIO pins
	 *-------------------------------------------------------------------*/
	/* test-only: take GPIO init from pcs440ep ???? in config file */
	out32(GPIO0_OR, 0x00000000);
	out32(GPIO0_TCR, 0x7C2FF1CF);
	out32(GPIO0_OSRL, 0x40055000);
	out32(GPIO0_OSRH, 0x00000000);
	out32(GPIO0_TSRL, 0x40055000);
	out32(GPIO0_TSRH, 0x00000400);
	out32(GPIO0_ISR1L, 0x40000000);
	out32(GPIO0_ISR1H, 0x00000000);
	out32(GPIO0_ISR2L, 0x00000000);
	out32(GPIO0_ISR2H, 0x00000000);
	out32(GPIO0_ISR3L, 0x00000000);
	out32(GPIO0_ISR3H, 0x00000000);

	out32(GPIO1_OR, 0x00000000);
	out32(GPIO1_TCR, 0xC6007FFF);
	out32(GPIO1_OSRL, 0x00140000);
	out32(GPIO1_OSRH, 0x00000000);
	out32(GPIO1_TSRL, 0x00000000);
	out32(GPIO1_TSRH, 0x00000000);
	out32(GPIO1_ISR1L, 0x05415555);
	out32(GPIO1_ISR1H, 0x40000000);
	out32(GPIO1_ISR2L, 0x00000000);
	out32(GPIO1_ISR2H, 0x00000000);
	out32(GPIO1_ISR3L, 0x00000000);
	out32(GPIO1_ISR3H, 0x00000000);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(uic0sr, 0xffffffff);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(uic0pr, 0xfffff7ff);	/* per ref-board manual */
	mtdcr(uic0tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic0vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic0sr, 0xffffffff);	/* clear all */

	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000000);	/* all non-critical */
	mtdcr(uic1pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic1tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	mtdcr(uic2sr, 0xffffffff);	/* clear all */
	mtdcr(uic2er, 0x00000000);	/* disable all */
	mtdcr(uic2cr, 0x00000000);	/* all non-critical */
	mtdcr(uic2pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic2tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic2vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic2sr, 0xffffffff);	/* clear all */
	mtsdr(sdr_pfc0, 0x00003E00);	/* Pin function:  */
	mtsdr(sdr_pfc1, 0x00848000);	/* Pin function: UART0 has 4 pins */

	/* PCI arbiter enabled */
	mfsdr(sdr_pci0, reg);
	mtsdr(sdr_pci0, 0x80000000 | reg);

	pci_pre_init(0);

	/* setup BOOT FLASH */
	mtsdr(SDR0_CUST0, 0xC0082350);

	return 0;
}

#ifdef CONFIG_BOARD_PRE_INIT
int board_pre_init (void)
{
	return board_early_init_f ();
}

#endif

enum {
	/* HW_GENERATION_HCU1 is no longer supported */
	HW_GENERATION_HCU2  = 0x10,
	HW_GENERATION_HCU3  = 0x10,
	HW_GENERATION_HCU4  = 0x20,
	HW_GENERATION_HCU5  = 0x30,
	HW_GENERATION_MCU   = 0x08,
	HW_GENERATION_MCU20 = 0x0a,
	HW_GENERATION_MCU25 = 0x09,
};

int checkboard (void)
{
#define SDR0_ECID0			0x0080
#define SDR0_ECID1			0x0081
#define SDR0_ECID2			0x0082
#define SDR0_ECID3			0x0083
	unsigned j;
	uint16_t *hwVersReg    = (uint16_t *) HCU_HW_VERSION_REGISTER;
	uint16_t *boardVersReg = (uint16_t *) HCU_CPLD_VERSION_REGISTER;
	uint16_t generation = *boardVersReg & 0xf0;
	uint16_t index      = *boardVersReg & 0x0f;
	ulong ecid0, ecid1, ecid2, ecid3;
	printf ("Netstal Maschinen AG: ");
	if (generation == HW_GENERATION_HCU3)
		printf ("HCU3: index %d", index);
	else if (generation == HW_GENERATION_HCU4)
		printf ("HCU4: index %d", index);
	else if (generation == HW_GENERATION_HCU5)
		printf ("HCU5: index %d", index);
	printf (" HW 0x%02x\n", *hwVersReg & 0xff);
	mfsdr(SDR0_ECID0, ecid0);
	mfsdr(SDR0_ECID1, ecid1);
	mfsdr(SDR0_ECID2, ecid2);
	mfsdr(SDR0_ECID3, ecid3);

	printf("Chip ID 0x%x 0x%x 0x%x 0x%x\n",  ecid0,  ecid1, ecid2, ecid3);
	for (j=0; j < 6;j++) {
		sysLedSet(1 << j);
		udelay(200*1000);
	}
	return 0;
}

#define SYS_IO_ADDRESS 0xcce00000

u32 sysLedGet(void)
{
	return in16(SYS_IO_ADDRESS) & 0x3f;
}

void sysLedSet(u32 value /* value to place in LEDs */)
{
	out16(SYS_IO_ADDRESS, value);
}

/*---------------------------------------------------------------------------+
 * getSerialNr
 *---------------------------------------------------------------------------*/
static u32 getSerialNr(void)
{
	u32 *serial = (u32 *)CFG_FLASH_BASE;
	if (*serial == 0xffffffff) {
		return get_ticks();
	}
	return *serial;
}


/*---------------------------------------------------------------------------+
 * misc_init_r.
 *---------------------------------------------------------------------------*/

#define DEFAULT_ETH_ADDR  "ethaddr"
/* ethaddr  for first or etha1ddr  for second ethernet */

int misc_init_r(void)
{
	char *s = getenv(DEFAULT_ETH_ADDR);
	char *e;
	int i;
	u32 serial =  getSerialNr();
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;

	for (i = 0; i < 6; ++i) {
		gd->bd->bi_enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	if (gd->bd->bi_enetaddr[3] == 0 &&
	    gd->bd->bi_enetaddr[4] == 0 &&
	    gd->bd->bi_enetaddr[5] == 0) {
		char ethaddr[22];
		/* Must be in sync with CONFIG_ETHADDR */
		gd->bd->bi_enetaddr[0] = 0x00;
		gd->bd->bi_enetaddr[1] = 0x60;
		gd->bd->bi_enetaddr[2] = 0x13;
		gd->bd->bi_enetaddr[3] = (serial          >> 16) & 0xff;
		gd->bd->bi_enetaddr[4] = (serial          >>  8) & 0xff;
		/* byte[5].bit 0 must be zero */
		gd->bd->bi_enetaddr[5] = (serial          >>  0) & 0xfe;
		sprintf (ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X\0",
			 gd->bd->bi_enetaddr[0], gd->bd->bi_enetaddr[1],
			 gd->bd->bi_enetaddr[2], gd->bd->bi_enetaddr[3],
			 gd->bd->bi_enetaddr[4], gd->bd->bi_enetaddr[5]) ;
		printf("%s: Setting eth %s serial 0x%x\n",  __FUNCTION__,
		       ethaddr, serial);
		setenv (DEFAULT_ETH_ADDR, ethaddr);
	}
#ifdef CFG_ENV_IS_IN_FLASH
	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CFG_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

	/* Env protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    CFG_ENV_ADDR_REDUND,
			    CFG_ENV_ADDR_REDUND + 2*CFG_ENV_SECT_SIZE - 1,
			    &flash_info[0]);
#endif

	/*
	 * USB stuff...
	 */

	/* SDR Setting */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	mfsdr(SDR0_USB2D0CR, usb2d0cr);
	mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mfsdr(SDR0_USB2H0CR, usb2h0cr);

	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;	/*0*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ;	/*1*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;		/*0*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;		/*1*/
	usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
	usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;		/*1*/

	/* An 8-bit/60MHz interface is the only possible alternative
	   when connecting the Device to the PHY */
	usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
	usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_16BIT_30MHZ;	/*1*/

	/* To enable the USB 2.0 Device function through the UTMI interface */
	usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
	usb2d0cr = usb2d0cr | SDR0_USB2D0CR_USB2DEV_SELECTION;		/*1*/

	sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
	sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_USB2D_SEL;		/*0*/

	mtsdr(SDR0_PFC1, sdr0_pfc1);
	mtsdr(SDR0_USB2D0CR, usb2d0cr);
	mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
	mtsdr(SDR0_USB2H0CR, usb2h0cr);

	/*clear resets*/
	udelay (1000);
	mtsdr(SDR0_SRST1, 0x00000000);
	udelay (1000);
	mtsdr(SDR0_SRST0, 0x00000000);

	printf("USB:   Host(int phy) Device(ext phy)\n");

	return 0;
}

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
#if defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;

	/*-------------------------------------------------------------------+
	 * As of errata version 0.4, CHIP_8: Incorrect Write to DDR SDRAM.
	 * Workaround: Disable write pipelining to DDR SDRAM by setting
	 * PLB0_ACR[WRP] = 0.
	 *-------------------------------------------------------------------*/

	/*-------------------------------------------------------------------+
	  | Set priority for all PLB3 devices to 0.
	  | Set PLB3 arbiter to fair mode.
	  +-------------------------------------------------------------------*/
	mfsdr(sdr_amp1, addr);
	mtsdr(sdr_amp1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb3_acr);
	// mtdcr(plb3_acr, addr & ~plb1_acr_wrp_mask);  /* ngngng */
	mtdcr(plb3_acr, addr | 0x80000000); /* Sequoia */

	/*-------------------------------------------------------------------+
	  | Set priority for all PLB4 devices to 0.
	  +-------------------------------------------------------------------*/
	mfsdr(sdr_amp0, addr);
	mtsdr(sdr_amp0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb4_acr) | 0xa0000000;	/* Was 0x8---- */
//	mtdcr(plb4_acr, addr & ~plb1_acr_wrp_mask);  /* ngngng */
	mtdcr(plb4_acr, addr);  /* Sequoia */

	/*-------------------------------------------------------------------+
	  | Set Nebula PLB4 arbiter to fair mode.
	  +-------------------------------------------------------------------*/
	/* Segment0 */
	addr = (mfdcr(plb0_acr) & ~plb0_acr_ppm_mask) | plb0_acr_ppm_fair;
	addr = (addr & ~plb0_acr_hbu_mask) | plb0_acr_hbu_enabled;
	addr = (addr & ~plb0_acr_rdp_mask) | plb0_acr_rdp_4deep;
//	addr = (addr & ~plb0_acr_wrp_mask) ;  /* ngngng */
	addr = (addr & ~plb0_acr_wrp_mask) | plb0_acr_wrp_2deep; /* Sequoia */

	// mtdcr(plb0_acr, addr); /* Sequoia */
	mtdcr(plb0_acr, 0);  // PATCH HAB: WRITE PIPELINING OFF


	/* Segment1 */
	addr = (mfdcr(plb1_acr) & ~plb1_acr_ppm_mask) | plb1_acr_ppm_fair;
	addr = (addr & ~plb1_acr_hbu_mask) | plb1_acr_hbu_enabled;
	addr = (addr & ~plb1_acr_rdp_mask) | plb1_acr_rdp_4deep;
	addr = (addr & ~plb1_acr_wrp_mask) ;
	// mtdcr(plb1_acr, addr); /* Sequoia */
	mtdcr(plb1_acr, 0);  // PATCH HAB: WRITE PIPELINING OFF

	return 1;
}
#endif	/* defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT) */

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
	/*-------------------------------------------------------------+
	 * Set up Direct MMIO registers
	 *-------------------------------------------------------------*/
	/*-------------------------------------------------------------+
	  | PowerPC440EPX PCI Master configuration.
	  | Map one 1Gig range of PLB/processor addresses to PCI memory space.
	  |   PLB address 0xA0000000-0xDFFFFFFF ==> PCI address
	  |		  0xA0000000-0xDFFFFFFF
	  |   Use byte reversed out routines to handle endianess.
	  | Make this region non-prefetchable.
	  +-------------------------------------------------------------*/
	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM0MA, 0x00000000);
	out32r(PCIX0_PMM0LA, CFG_PCI_MEMBASE);	/* PMM0 Local Address */
	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM0PCILA, CFG_PCI_MEMBASE);
	out32r(PCIX0_PMM0PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	/* 512M + No prefetching, and enable region */
	out32r(PCIX0_PMM0MA, 0xE0000001);

	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM1MA, 0x00000000);
	out32r(PCIX0_PMM1LA, CFG_PCI_MEMBASE2);	/* PMM0 Local Address */
	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM1PCILA, CFG_PCI_MEMBASE2);
	out32r(PCIX0_PMM1PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	/* 512M + No prefetching, and enable region */
	out32r(PCIX0_PMM1MA, 0xE0000001);

	out32r(PCIX0_PTM1MS, 0x00000001);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM1LA, 0);	/* Local Addr. Reg */
	out32r(PCIX0_PTM2MS, 0);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM2LA, 0);	/* Local Addr. Reg */

	/*------------------------------------------------------------------+
	 * Set up Configuration registers
	 *------------------------------------------------------------------*/

	/* Program the board's subsystem id/vendor id */
	pci_write_config_word(0, PCI_SUBSYSTEM_VENDOR_ID,
			      CFG_PCI_SUBSYS_VENDORID);
	pci_write_config_word(0, PCI_SUBSYSTEM_ID, CFG_PCI_SUBSYS_ID);

	/* Configure command register as bus master */
	pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER);

	/* 240nS PCI clock */
	pci_write_config_word(0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_write_config_word(0, PCI_ERREN, 0);

	pci_write_config_dword(0, PCI_BRDGOPT2, 0x00000101);

}
#endif	/* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */

/*************************************************************************
 *  pci_master_init
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;

	/*---------------------------------------------------------------+
	  | Write the PowerPC440 EP PCI Configuration regs.
	  |   Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	  |   Enable PowerPC440 EP to act as a PCI memory target (PTM).
	  +--------------------------------------------------------------*/
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}
#endif
/* defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT) */

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
	return 1;
}
#endif				/* defined(CONFIG_PCI) */

