/*
 *(C) Copyright 2005-2008 Netstal Maschinen AG
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
#include <asm/io.h>
#include  "../common/nm.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#undef BOOTSTRAP_OPTION_A_ACTIVE

#define SDR0_CP440		0x0180

#define SYSTEM_RESET		0x30000000
#define CHIP_RESET		0x20000000

#define SDR0_ECID0		0x0080
#define SDR0_ECID1		0x0081
#define SDR0_ECID2		0x0082
#define SDR0_ECID3		0x0083

#define SYS_IO_ADDRESS			(CFG_CS_2 + 0x00e00000)
#define SYS_SLOT_ADDRESS		(CFG_CPLD + 0x00400000)
#define HCU_DIGITAL_IO_REGISTER	(CFG_CPLD + 0x0500000)
#define HCU_SW_INSTALL_REQUESTED	0x10

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

	mfcpr(CPR0_ICFG, cpr0icfg);
	if (!(cpr0icfg & CPR0_ICFG_RLI_MASK)) {
		mtcpr(CPR0_MALD,   0x02000000);
		mtcpr(CPR0_OPBD,   0x02000000);
	        mtcpr(CPR0_PERD,   0x05000000);  /* 1:5 */
		mtcpr(CPR0_PLLC,   0x40000238);
		mtcpr(CPR0_PLLD,   0x01010414);
		mtcpr(CPR0_PRIMAD, 0x01000000);
		mtcpr(CPR0_PRIMBD, 0x01000000);
		mtcpr(CPR0_SPCID,  0x03000000);
		mtsdr(SDR0_PFC0,   0x00003E00);  /* [CTE] = 0 */
		mtsdr(SDR0_CP440,  0x0EAAEA02);  /* [Nto1] = 1*/
		mtcpr(CPR0_ICFG,   cpr0icfg | CPR0_ICFG_RLI_MASK);

		/*
		 * Initiate system reset in debug control register DBCR
		 */
		dbcr = mfspr(dbcr0);
		mtspr(dbcr0, dbcr | CHIP_RESET);
	}
	mtsdr(SDR0_CP440, 0x0EAAEA02);  /* [Nto1] = 1*/
#endif
	mtdcr(ebccfga, xbcfg);
	mtdcr(ebccfgd, 0xb8400000);

	/*
	 * Setup the GPIO pins
	 */
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

	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 */
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

	/* setup BOOT FLASH */
	mtsdr(SDR0_CUST0, 0xC0082350);

	return 0;
}

#ifdef CONFIG_BOARD_PRE_INIT
int board_pre_init(void)
{
	return board_early_init_f();
}

#endif

int sys_install_requested(void)
{
	u16 *ioValuePtr = (u16 *)HCU_DIGITAL_IO_REGISTER;
	return (in_be16(ioValuePtr) & HCU_SW_INSTALL_REQUESTED) != 0;
}

int checkboard(void)
{
	u16 *hwVersReg    = (u16 *) HCU_HW_VERSION_REGISTER;
	u16 *boardVersReg = (u16 *) HCU_CPLD_VERSION_REGISTER;
	u16 generation = in_be16(boardVersReg) & 0xf0;
	u16 index      = in_be16(boardVersReg) & 0x0f;
	u32 ecid0, ecid1, ecid2, ecid3;

	nm_show_print(generation, index, in_be16(hwVersReg) & 0xff);
	mfsdr(SDR0_ECID0, ecid0);
	mfsdr(SDR0_ECID1, ecid1);
	mfsdr(SDR0_ECID2, ecid2);
	mfsdr(SDR0_ECID3, ecid3);

	printf("Chip ID 0x%x 0x%x 0x%x 0x%x\n", ecid0, ecid1, ecid2, ecid3);

	return 0;
}

u32 hcu_led_get(void)
{
	return in16(SYS_IO_ADDRESS) & 0x3f;
}

/*
 * hcu_led_set  value to be placed into the LEDs (max 6 bit)
 */
void hcu_led_set(u32 value)
{
	out16(SYS_IO_ADDRESS, value);
}

/*
 * get_serial_number
 */
u32 get_serial_number(void)
{
	u32 *serial = (u32 *)CFG_FLASH_BASE;

	if (in_be32(serial) == 0xffffffff)
		return 0;

	return in_be32(serial);
}


/*
 * hcu_get_slot
 */
u32 hcu_get_slot(void)
{
	u16 *slot = (u16 *)SYS_SLOT_ADDRESS;
	return in_be16(slot) & 0x7f;
}


/*
 * misc_init_r.
 */
int misc_init_r(void)
{
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;

#ifdef CFG_ENV_IS_IN_FLASH
	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CFG_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

#ifdef CFG_ENV_ADDR_REDUND
	/* Env protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    CFG_ENV_ADDR_REDUND,
			    CFG_ENV_ADDR_REDUND + 2*CFG_ENV_SECT_SIZE - 1,
			    &flash_info[0]);
#endif
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
	 *  when connecting the Device to the PHY
	 */
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
	udelay(1000);
	mtsdr(SDR0_SRST1, 0x00000000);
	udelay(1000);
	mtsdr(SDR0_SRST0, 0x00000000);
	printf("USB:   Host(int phy) Device(ext phy)\n");

	common_misc_init_r();
	set_params_for_sw_install( sys_install_requested(), "hcu5" );
	/* We cannot easily enable trace before, as there are other
	 * routines messing around with sdr0_pfc1. And I do not need it.
	 */
	if (mfspr(dbcr0) & 0x80000000) {
		/* External debugger alive
		 * enable trace facilty for Lauterback
		 * CCR0[DAPUIB]=0 	Enable broadcast of instruction data
		 *			to auxiliary processor interface
		 * CCR0[DTB]=0 		Enable broadcast of trace information
		 * SDR0_PFC0[TRE] 	Trace signals are enabled instead of
		 *			GPIO49-63
		 */
		mtspr(ccr0, mfspr(ccr0)  &~ 0x00108000);
		mtsdr(SDR0_PFC0, sdr0_pfc1 | 0x00000100);
	}
	return 0;
}
#ifdef CONFIG_PCI
int board_with_pci(void)
{
	u32 reg;

	mfsdr(sdr_pci0, reg);
	return (reg & SDR0_XCR_PAE_MASK);
}

/*
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
 */
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;

	if (!board_with_pci()) { return 0; }

	/*
	 * Set priority for all PLB3 devices to 0.
	 * Set PLB3 arbiter to fair mode.
	 */
	mfsdr(sdr_amp1, addr);
	mtsdr(sdr_amp1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb3_acr);
	mtdcr(plb3_acr, addr | 0x80000000); /* Sequoia */

	/*
	 * Set priority for all PLB4 devices to 0.
	 */
	mfsdr(sdr_amp0, addr);
	mtsdr(sdr_amp0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb4_acr) | 0xa0000000;	/* Was 0x8---- */
	mtdcr(plb4_acr, addr);  /* Sequoia */

	/*
	 * As of errata version 0.4, CHIP_8: Incorrect Write to DDR SDRAM.
	 * Workaround: Disable write pipelining to DDR SDRAM by setting
	 * PLB0_ACR[WRP] = 0.
	 */
	mtdcr(plb0_acr, 0);  /* PATCH HAB: WRITE PIPELINING OFF */

	/* Segment1 */
	mtdcr(plb1_acr, 0);  /* PATCH HAB: WRITE PIPELINING OFF */

	return board_with_pci();
}

/*
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 */
void pci_target_init(struct pci_controller *hose)
{
	if (!board_with_pci()) { return; }
	/*
	 * Set up Direct MMIO registers
	 *
	 * PowerPC440EPX PCI Master configuration.
	 * Map one 1Gig range of PLB/processor addresses to PCI memory space.
	 *   PLB address 0xA0000000-0xDFFFFFFF ==> PCI address
	 *		  0xA0000000-0xDFFFFFFF
	 *   Use byte reversed out routines to handle endianess.
	 * Make this region non-prefetchable.
	 */
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

	/*
	 * Set up Configuration registers
	 */

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

/*
 *  pci_master_init
 *
 */
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;
	if (!board_with_pci()) { return; }

	/*---------------------------------------------------------------
	 * Write the PowerPC440 EP PCI Configuration regs.
	 *   Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	 *   Enable PowerPC440 EP to act as a PCI memory target (PTM).
	 *--------------------------------------------------------------*/
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}

/*
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
 */
int is_pci_host(struct pci_controller *hose)
{
	return 1;
}
#endif	 /* defined(CONFIG_PCI) */

#if defined(CONFIG_POST)
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	return 0;	/* No hotkeys supported */
}
#endif /* CONFIG_POST */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	u32 val[4];
	int rc;

	ft_cpu_setup(blob, bd);

}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
