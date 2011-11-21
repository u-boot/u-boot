/*
 * (C) Copyright 2011, Missing Link Electronics
 *                     Joachim Foerster <joachim@missinglinkelectronics.com>
 *
 * Taken from arch/powerpc/cpu/ppc4xx/fdt.c:
 *
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

void __ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
void ft_board_setup(void *blob, bd_t *bd) \
	__attribute__((weak, alias("__ft_board_setup")));

void ft_cpu_setup(void *blob, bd_t *bd)
{
	/*
	 * Fixup all ethernet nodes
	 * Note: aliases in the dts are required for this
	 */
	fdt_fixup_ethernet(blob);
}
#endif /* CONFIG_OF_LIBFDT && CONFIG_OF_BOARD_SETUP */
