/*
 * (C) Copyright 2011, Missing Link Electronics
 *                     Joachim Foerster <joachim@missinglinkelectronics.com>
 *
 * Taken from arch/powerpc/cpu/ppc4xx/fdt.c:
 *
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#ifdef CONFIG_OF_BOARD_SETUP
#include <libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

int __ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
int ft_board_setup(void *blob, bd_t *bd)
	__attribute__((weak, alias("__ft_board_setup")));

void ft_cpu_setup(void *blob, bd_t *bd)
{
	/*
	 * Fixup all ethernet nodes
	 * Note: aliases in the dts are required for this
	 */
	fdt_fixup_ethernet(blob);
}
#endif /* CONFIG_OF_BOARD_SETUP */
