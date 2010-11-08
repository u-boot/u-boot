/*
 * (C) Copyright 2009
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * Based on board/amcc/canyonlands/canyonlands.c
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <ppc440.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <i2c.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/4xx_pcie.h>
#include <asm/gpio.h>

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

DECLARE_GLOBAL_DATA_PTR;

#define CONFIG_SYS_BCSR3_PCIE		0x10

int board_early_init_f(void)
{
	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	mtdcr(UIC2SR, 0xffffffff);	/* clear all */
	mtdcr(UIC2ER, 0x00000000);	/* disable all */
	mtdcr(UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC2PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC2TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC2VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all */

	mtdcr(UIC3SR, 0xffffffff);	/* clear all */
	mtdcr(UIC3ER, 0x00000000);	/* disable all */
	mtdcr(UIC3CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC3PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC3TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC3VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC3SR, 0xffffffff);	/* clear all */

	/*
	 * Configure PFC (Pin Function Control) registers
	 * enable GPIO 49-63
	 * UART0: 4 pins
	 */
	mtsdr(SDR0_PFC0, 0x00007fff);
	mtsdr(SDR0_PFC1, 0x00040000);

	/* Enable PCI host functionality in SDR0_PCI0 */
	mtsdr(SDR0_PCI0, 0xe0000000);

	mtsdr(SDR0_SRST1, 0);	/* Pull AHB out of reset default=1 */

	/* Setup PLB4-AHB bridge based on the system address map */
	mtdcr(AHB_TOP, 0x8000004B);
	mtdcr(AHB_BOT, 0x8000004B);

	/*
	* Configure USB-STP pins as alternate and not GPIO
	* It seems to be neccessary to configure the STP pins as GPIO
	* input at powerup (perhaps while USB reset is asserted). So
	* we configure those pins to their "real" function now.
	*/
	gpio_config(16, GPIO_OUT, GPIO_ALT1, GPIO_OUT_1);
	gpio_config(19, GPIO_OUT, GPIO_ALT1, GPIO_OUT_1);

	/* Trigger board component reset */
	out_le16((void *)CONFIG_SYS_IO_BASE, 0xffff);
	out_le16((void *)CONFIG_SYS_IO_BASE + 0x100, 0xffff);
	udelay(50);
	out_le16((void *)CONFIG_SYS_IO_BASE, 0xffbf);
	out_le16((void *)CONFIG_SYS_IO_BASE + 0x100, 0xffbf);
	udelay(50);
	out_le16((void *)CONFIG_SYS_IO_BASE, 0xffff);
	out_le16((void *)CONFIG_SYS_IO_BASE + 0x100, 0xffff);

	return 0;
}

int get_cpu_num(void)
{
	int cpu = NA_OR_UNKNOWN_CPU;

	return cpu;
}

int checkboard(void)
{
	char *s = getenv("serial#");

#ifdef CONFIG_DEVCONCENTER
	printf("Board: DevCon-Center");
#else
	printf("Board: Intip");
#endif

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return 0;
}

int board_early_init_r(void)
{
	/*
	 * CompactCenter has 64MBytes, DevCon-Center 128MBytes of NOR FLASH
	 * (Spansion 29GL512), but the boot EBC mapping only supports a maximum
	 * of 16MBytes (4.ff00.0000 - 4.ffff.ffff).
	 * To solve this problem, the FLASH has to get remapped to another
	 * EBC address which accepts bigger regions:
	 *
	 * 0xfn00.0000 -> 4.cn00.0000
	 */

	u32 bxcr_bw = (CONFIG_SYS_FLASH_SIZE == 128 << 20) ?
		EBC_BXCR_BS_128MB : EBC_BXCR_BS_64MB;

	/* Remap the NOR FLASH to 0xcn00.0000 ... 0xcfff.ffff */
	mtebc(PB0CR, CONFIG_SYS_FLASH_BASE_PHYS_L
		| bxcr_bw
		| EBC_BXCR_BU_RW
		| EBC_BXCR_BW_16BIT);

	/* Remove TLB entry of boot EBC mapping */
	remove_tlb(CONFIG_SYS_BOOT_BASE_ADDR, 16 << 20);

	/* Add TLB entry for 0xfn00.0000 -> 0x4.cn00.0000 */
	program_tlb(CONFIG_SYS_FLASH_BASE_PHYS, CONFIG_SYS_FLASH_BASE,
			CONFIG_SYS_FLASH_SIZE, TLB_WORD2_I_ENABLE);

	/*
	 * Now accessing of the whole 64Mbytes of NOR FLASH at virtual address
	 * 0xfc00.0000 is possible
	 */

	/*
	 * Clear potential errors resulting from auto-calibration.
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	set_mcsr(get_mcsr());

	return 0;
}

int misc_init_r(void)
{
	u32 sdr0_srst1 = 0;
	u32 eth_cfg;

	/*
	 * Set EMAC mode/configuration (GMII, SGMII, RGMII...).
	 * This is board specific, so let's do it here.
	 */
	mfsdr(SDR0_ETH_CFG, eth_cfg);
	/* disable SGMII mode */
	eth_cfg &= ~(SDR0_ETH_CFG_SGMII2_ENABLE |
		     SDR0_ETH_CFG_SGMII1_ENABLE |
		     SDR0_ETH_CFG_SGMII0_ENABLE);
	/* Set the for 2 RGMII mode */
	/* GMC0 EMAC4_0, GMC0 EMAC4_1, RGMII Bridge 0 */
	eth_cfg &= ~SDR0_ETH_CFG_GMC0_BRIDGE_SEL;
	eth_cfg |= SDR0_ETH_CFG_GMC1_BRIDGE_SEL;
	mtsdr(SDR0_ETH_CFG, eth_cfg);

	/*
	 * The AHB Bridge core is held in reset after power-on or reset
	 * so enable it now
	 */
	mfsdr(SDR0_SRST1, sdr0_srst1);
	sdr0_srst1 &= ~SDR0_SRST1_AHB;
	mtsdr(SDR0_SRST1, sdr0_srst1);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
extern void __ft_board_setup(void *blob, bd_t *bd);

void ft_board_setup(void *blob, bd_t *bd)
{
	__ft_board_setup(blob, bd);

	fdt_find_and_setprop(blob, "/plb/pciex@d00000000", "status",
			     "disabled", sizeof("disabled"), 1);

	fdt_find_and_setprop(blob, "/plb/sata@bffd1000", "status",
			     "disabled", sizeof("disabled"), 1);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
