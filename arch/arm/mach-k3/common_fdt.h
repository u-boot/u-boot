/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 */

#ifndef _COMMON_FDT_H
#define _COMMON_FDT_H

int fdt_fixup_msmc_ram_k3(void *blob);
int fdt_del_node_path(void *blob, const char *path);
int fdt_fixup_reserved(void *blob, const char *name,
		       unsigned int new_address, unsigned int new_size);

#endif /* _COMMON_FDT_H */
