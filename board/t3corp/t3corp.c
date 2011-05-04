/*
 * (C) Copyright 2010
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
#include <asm/ppc440.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <i2c.h>
#include <mtd/cfi_flash.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/4xx_pcie.h>
#include <asm/ppc4xx-gpio.h>

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
	mtdcr(UIC1PR, 0x7fffffff);	/* per ref-board manual */
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

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: T3CORP");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

int board_early_init_r(void)
{
	/*
	 * T3CORP has 64MBytes of NOR flash (Spansion 29GL512), but the
	 * boot EBC mapping only supports a maximum of 16MBytes
	 * (4.ff00.0000 - 4.ffff.ffff).
	 * To solve this problem, the flash has to get remapped to another
	 * EBC address which accepts bigger regions:
	 *
	 * 0xfn00.0000 -> 4.cn00.0000
	 */

	/* Remap the NOR flash to 0xcn00.0000 ... 0xcfff.ffff */
	mtebc(PB0CR, CONFIG_SYS_FLASH_BASE_PHYS_L | EBC_BXCR_BS_64MB |
	      EBC_BXCR_BU_RW | EBC_BXCR_BW_16BIT);

	/* Remove TLB entry of boot EBC mapping */
	remove_tlb(CONFIG_SYS_BOOT_BASE_ADDR, 16 << 20);

	/* Add TLB entry for 0xfn00.0000 -> 0x4.cn00.0000 */
	program_tlb(CONFIG_SYS_FLASH_BASE_PHYS, CONFIG_SYS_FLASH_BASE,
		    CONFIG_SYS_FLASH_SIZE, TLB_WORD2_I_ENABLE);

	/*
	 * Now accessing of the whole 64Mbytes of NOR flash at virtual address
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
	eth_cfg &= ~SDR0_ETH_CFG_GMC1_BRIDGE_SEL;
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

int board_pcie_last(void)
{
	/*
	 * Only PCIe0 for now, PCIe1 hangs on this board
	 */
	return 0;
}

/*
 * Board specific WRDTR and CLKTR values used by the auto-
 * calibration code (4xx_ibm_ddr2_autocalib.c).
 */
static struct sdram_timing board_scan_options[] = {
	{1, 2},
	{-1, -1}
};

struct sdram_timing *ddr_scan_option(struct sdram_timing *default_val)
{
	return board_scan_options;
}

/*
 * Accessor functions replacing the "weak" functions in
 * drivers/mtd/cfi_flash.c
 *
 * The NOR flash devices "behind" the FPGA's (Xilinx DS617)
 * can only be read correctly in 16bit mode. We need to emulate
 * 8bit and 32bit reads here in the board specific code.
 */
u8 flash_read8(void *addr)
{
	u16 val = __raw_readw((void *)((u32)addr & ~1));

	if ((u32)addr & 1)
		return val;

	return val >> 8;
}

u32 flash_read32(void *addr)
{
	return (__raw_readw(addr) << 16) | __raw_readw((void *)((u32)addr + 2));
}

void flash_cmd_reset(flash_info_t *info)
{
	/*
	 * FLASH at address CONFIG_SYS_FLASH_BASE is a Spansion chip and
	 * needs the Spansion type reset commands. The other flash chip
	 * is located behind a FPGA (Xilinx DS617) and needs the Intel type
	 * reset command.
	 */
	if (info->start[0] == CONFIG_SYS_FLASH_BASE)
		flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	else
		flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
}
