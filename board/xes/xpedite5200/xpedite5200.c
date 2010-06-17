/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 * Copyright 2004, 2007 Freescale Semiconductor, Inc.
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
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <pca953x.h>

extern void ft_board_pci_setup(void *blob, bd_t *bd);

int checkboard(void)
{
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	char *s;

	printf("Board: X-ES %s PMC\n", CONFIG_SYS_BOARD_NAME);
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

	out_be32(&lbc->ltesr, 0xffffffff);	/* Clear LBC error IRQs */
	out_be32(&lbc->lteir, 0xffffffff);	/* Enable LBC error IRQs */
	out_be32(&ecm->eedr, 0xffffffff);	/* Clear ecm errors */
	out_be32(&ecm->eeer, 0xffffffff);	/* Enable ecm errors */

	return 0;
}

static void flash_cs_fixup(void)
{
	int flash_sel;

	/*
	 * Print boot dev and swap flash flash chip selects if booted from 2nd
	 * flash.  Swapping chip selects presents user with a common memory
	 * map regardless of which flash was booted from.
	 */
	flash_sel = !((pca953x_get_val(CONFIG_SYS_I2C_PCA953X_ADDR0) &
			CONFIG_SYS_PCA953X_FLASH_PASS_CS));
	printf("FLASH: Executed from FLASH%d\n", flash_sel ? 2 : 1);

	if (flash_sel) {
		set_lbc_br(0, CONFIG_SYS_BR1_PRELIM);
		set_lbc_or(0, CONFIG_SYS_OR1_PRELIM);

		set_lbc_br(1, CONFIG_SYS_BR0_PRELIM);
		set_lbc_or(1, CONFIG_SYS_OR0_PRELIM);
	}
}

int board_early_init_r(void)
{
	/* Initialize PCA9557 devices */
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR0, 0xff, 0);
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR1, 0xff, 0);

	/*
	 * Remap NOR flash region to caching-inhibited
	 * so that flash can be erased/programmed properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* Invalidate existing TLB entry for NOR flash */
	disable_tlb(0);
	set_tlb(1, (CONFIG_SYS_FLASH_BASE2 & 0xf0000000),
		(CONFIG_SYS_FLASH_BASE2 & 0xf0000000),
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, 0, BOOKE_PAGESZ_256M, 1);

	flash_cs_fixup();

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_PCI
	ft_board_pci_setup(blob, bd);
#endif
	ft_cpu_setup(blob, bd);
}
#endif
