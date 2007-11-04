/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
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

#ifndef __FDT_SUPPORT_H
#define __FDT_SUPPORT_H

#ifdef CONFIG_OF_LIBFDT

#include <fdt.h>

int fdt_chosen(void *fdt, ulong initrd_start, ulong initrd_end, int force);
void do_fixup_by_path(void *fdt, const char *path, const char *prop,
		      const void *val, int len, int create);
void do_fixup_by_path_u32(void *fdt, const char *path, const char *prop,
			  u32 val, int create);

#ifdef CONFIG_OF_HAS_UBOOT_ENV
int fdt_env(void *fdt);
#endif

#ifdef CONFIG_OF_HAS_BD_T
int fdt_bd_t(void *fdt);
#endif

#ifdef CONFIG_OF_BOARD_SETUP
void ft_board_setup(void *blob, bd_t *bd);
void ft_cpu_setup(void *blob, bd_t *bd);
void ft_pci_setup(void *blob, bd_t *bd);
#endif

#endif /* ifdef CONFIG_OF_LIBFDT */
#endif /* ifndef __FDT_SUPPORT_H */
