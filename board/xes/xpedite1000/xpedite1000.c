/*
 * Copyright (C) 2003 Travis B. Sawyer <travis.sawyer@sandburst.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	unsigned long sdrreg;

	/*
	 * Enable GPIO for pins 18 - 24
	 * 18 = SEEPROM_WP
	 * 19 = #M_RST
	 * 20 = #MONARCH
	 * 21 = #LED_ALARM
	 * 22 = #LED_ACT
	 * 23 = #LED_STATUS1
	 * 24 = #LED_STATUS2
	 */
	mfsdr(sdr_pfc0, sdrreg);
	mtsdr(sdr_pfc0, (sdrreg & ~SDR0_PFC0_TRE_ENABLE) | 0x00003e00);
	out32(CONFIG_SYS_GPIO_BASE + 0x018, (USR_LED0 | USR_LED1 | USR_LED2 | USR_LED3));
	LED0_OFF();
	LED1_OFF();
	LED2_OFF();
	LED3_OFF();

	/* Setup the external bus controller/chip selects */
	mtebc(pb0ap, 0x04055200);	/* 16MB Strata FLASH */
	mtebc(pb0cr, 0xff098000);	/* BAS=0xff0 16MB R/W 8-bit */
	mtebc(pb1ap, 0x04055200);	/* 512KB Socketed AMD FLASH */
	mtebc(pb1cr, 0xfe018000);	/* BAS=0xfe0 1MB R/W 8-bit */
	mtebc(pb6ap, 0x05006400);	/* 32-64MB AMD MirrorBit FLASH */
	mtebc(pb6cr, 0xf00da000);	/* BAS=0xf00 64MB R/W i6-bit */
	mtebc(pb7ap, 0x05006400);	/* 32-64MB AMD MirrorBit FLASH */
	mtebc(pb7cr, 0xf40da000);	/* BAS=0xf40 64MB R/W 16-bit */

	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 *
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
	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000003);	/* SMI & UIC1 crit are critical */
	mtdcr(uic1pr, 0xfffffe00);	/* per ref-board manual */
	mtdcr(uic1tr, 0x01c00000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	mtdcr(uic2sr, 0xffffffff);	/* clear all */
	mtdcr(uic2er, 0x00000000);	/* disable all */
	mtdcr(uic2cr, 0x00000000);	/* all non-critical */
	mtdcr(uic2pr, 0xffffc0ff);	/* per ref-board manual */
	mtdcr(uic2tr, 0x00ff8000);	/* per ref-board manual */
	mtdcr(uic2vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic2sr, 0xffffffff);	/* clear all */

	mtdcr(uic3sr, 0xffffffff);	/* clear all */
	mtdcr(uic3er, 0x00000000);	/* disable all */
	mtdcr(uic3cr, 0x00000000);	/* all non-critical */
	mtdcr(uic3pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic3tr, 0x00ff8c0f);	/* per ref-board manual */
	mtdcr(uic3vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic3sr, 0xffffffff);	/* clear all */

	mtdcr(uic0sr, 0xfc000000);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000000);	/* all non-critical */
	mtdcr(uic0pr, 0xfc000000);	/* */
	mtdcr(uic0tr, 0x00000000);	/* */
	mtdcr(uic0vr, 0x00000001);	/* */

	LED0_ON();

	return 0;
}

int checkboard(void)
{
	char *s;

	printf("Board: X-ES %s PMC SBC\n", CONFIG_SYS_BOARD_NAME);
	printf("       ");
	s = getenv("board_rev");
	if (s)
		printf("Rev %s, ", s);
	s = getenv("serial#");
	if (s)
		printf("Serial# %s, ", s);
	s = getenv("board_cfg");
	if (s)
		printf("Cfg %s", s);
	printf("\n");

	return 0;
}

phys_size_t initdram(int board_type)
{
	return spd_sdram();
}

/*
 * This routine is called just prior to registering the hose and gives
 * the board the opportunity to check things. Returning a value of zero
 * indicates that things are bad & PCI initialization should be aborted.
 *
 * Different boards may wish to customize the pci controller structure
 * (add regions, override default access routines, etc) or perform
 * certain pre-initialization actions.
 */

#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller * hose)
{
	unsigned long strap;

	/* See if we're supposed to setup the pci */
	mfsdr(sdr_sdstp1, strap);
	if ((strap & 0x00010000) == 0)
		return 0;

#if defined(CONFIG_SYS_PCI_FORCE_PCI_CONV)
	/* Setup System Device Register PCIX0_XCR */
	mfsdr(sdr_xcr, strap);
	strap &= 0x0f000000;
	mtsdr(sdr_xcr, strap);
#endif

	return 1;
}
#endif /* defined(CONFIG_PCI) */

#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
/*
 * The bootstrap configuration provides default settings for the pci
 * inbound map (PIM). But the bootstrap config choices are limited and
 * may not be sufficient for a given board.
 */
void pci_target_init(struct pci_controller * hose)
{
	/* Disable everything */
	out32r(PCIX0_PIM0SA, 0);
	out32r(PCIX0_PIM1SA, 0);
	out32r(PCIX0_PIM2SA, 0);
	out32r(PCIX0_EROMBA, 0); /* disable expansion rom */

	/*
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440 strapping
	 * options to not support sizes such as 128/256 MB.
	 */
	out32r(PCIX0_PIM0LAL, CONFIG_SYS_SDRAM_BASE);
	out32r(PCIX0_PIM0LAH, 0);
	out32r(PCIX0_PIM0SA, ~(gd->ram_size - 1) | 1);

	out32r(PCIX0_BAR0, 0);

	/* Program the board's subsystem id/vendor id */
	out16r(PCIX0_SBSYSVID, CONFIG_SYS_PCI_SUBSYS_VENDORID);
	out16r(PCIX0_SBSYSID, CONFIG_SYS_PCI_SUBSYS_DEVICEID);

	out16r(PCIX0_CMD, in16r(PCIX0_CMD) | PCI_COMMAND_MEMORY);
}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */

#if defined(CONFIG_PCI)
/*
 * This routine is called to determine if a pci scan should be
 * performed. With various hardware environments (especially cPCI and
 * PPMC) it's insufficient to depend on the state of the arbiter enable
 * bit in the strap register, or generic host/adapter assumptions.
 *
 * Rather than hard-code a bad assumption in the general 440 code, the
 * 440 pci code requires the board to decide at runtime.
 *
 * Return 0 for adapter mode, non-zero for host (monarch) mode.
 */
int is_pci_host(struct pci_controller *hose)
{
	return ((in32(CONFIG_SYS_GPIO_BASE + 0x1C) & 0x00000800) == 0);
}
#endif /* defined(CONFIG_PCI) */

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	return ctrlc();
}

void post_word_store(ulong a)
{
	volatile ulong *save_addr =
		(volatile ulong *)(CONFIG_SYS_POST_WORD_ADDR);

	*save_addr = a;
}

ulong post_word_load(void)
{
	volatile ulong *save_addr =
		(volatile ulong *)(CONFIG_SYS_POST_WORD_ADDR);

	return *save_addr;
}
#endif
