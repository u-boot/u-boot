/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2024 Google LLC
 * Written by: Simon Glass <sjg@chromeium.org>
 */

#ifndef __BOARD_F
#define __BOARD_F

/**
 * struct board_f: Information used only before relocation
 *
 * This struct is set up in board_init_f() and used to deal with relocation. It
 * is not available after relocation.
 */
struct board_f {
	/**
	 * @new_fdt: relocated device tree
	 */
	void *new_fdt;
	/**
	 * @fdt_size: space reserved for relocated device space
	 */
	unsigned long fdt_size;
	/**
	 * @new_bootstage: relocated boot stage information
	 */
	struct bootstage_data *new_bootstage;
	/**
	 * @new_bloblist: relocated blob list information
	 */
	struct bloblist_hdr *new_bloblist;
};

#endif
