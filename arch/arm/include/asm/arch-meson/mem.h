/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __MESON_MEM_H__
#define __MESON_MEM_H__

/* Configure the reserved memory zones exported by the secure registers
 * into EFI and DTB reserved memory entries.
 */
void meson_gx_init_reserved_memory(void *fdt);

#endif /* __MESON_MEM_H__ */
