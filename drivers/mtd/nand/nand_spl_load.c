/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
#include <nand.h>

/*
 * The main entry for NAND booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from NAND into SDRAM and starts it from there.
 */
void nand_boot(void)
{
	__attribute__((noreturn)) void (*uboot)(void);

	/*
	 * Load U-Boot image from NAND into RAM
	 */
	nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
			CONFIG_SYS_NAND_U_BOOT_SIZE,
			(void *)CONFIG_SYS_NAND_U_BOOT_DST);

#ifdef CONFIG_NAND_ENV_DST
	nand_spl_load_image(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE,
			(void *)CONFIG_NAND_ENV_DST);

#ifdef CONFIG_ENV_OFFSET_REDUND
	nand_spl_load_image(CONFIG_ENV_OFFSET_REDUND, CONFIG_ENV_SIZE,
			(void *)CONFIG_NAND_ENV_DST + CONFIG_ENV_SIZE);
#endif
#endif

	/*
	 * Jump to U-Boot image
	 */
	uboot = (void *)CONFIG_SYS_NAND_U_BOOT_START;
	(*uboot)();
}
