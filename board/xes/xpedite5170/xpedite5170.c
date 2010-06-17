/*
 * Copyright 2009 Extreme Engineering Solutions, Inc.
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
#include <asm/mmu.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <pca953x.h>

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_PCI)
extern void ft_board_pci_setup(void *blob, bd_t *bd);
#endif

int checkboard(void)
{
	char *s;

	printf("Board: X-ES %s 3U VPX SBC\n", CONFIG_SYS_BOARD_NAME);
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
/*
 * Print out which flash was booted from and if booting from the 2nd flash,
 * swap flash chip selects to maintain consistent flash numbering/addresses.
 */
static void flash_cs_fixup(void)
{
	int flash_sel;

	/*
	 * Print boot dev and swap flash flash chip selects if booted from 2nd
	 * flash.  Swapping chip selects presents user with a common memory
	 * map regardless of which flash was booted from.
	 */
	flash_sel = !((pca953x_get_val(CONFIG_SYS_I2C_PCA953X_ADDR0) &
			CONFIG_SYS_PCA953X_C0_FLASH_PASS_CS));
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
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR2, 0xff, 0);
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR3, 0xff, 0);

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

#ifdef CONFIG_MP
extern void cpu_mp_lmb_reserve(struct lmb *lmb);

void board_lmb_reserve(struct lmb *lmb)
{
	cpu_mp_lmb_reserve(lmb);
}
#endif
