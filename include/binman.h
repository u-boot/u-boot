/* SPDX-License-Identifier: Intel */
/*
 * Access to binman information at runtime
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _BINMAN_H_
#define _BINMAN_H_

/**
 *struct binman_entry - information about a binman entry
 *
 * @image_pos: Position of entry in the image
 * @size: Size of entry
 */
struct binman_entry {
	u32 image_pos;
	u32 size;
};

/**
 * binman_entry_find() - Find a binman symbol
 *
 * This searches the binman information in the device tree for a symbol of the
 * given name
 *
 * @name: Path to entry to examine (e.g. "/read-only/u-boot")
 * @entry: Returns information about the entry
 * @return 0 if OK, -ENOENT if the path is not found, other -ve value if the
 *	binman information is invalid (missing image-pos or size)
 */
int binman_entry_find(const char *name, struct binman_entry *entry);

/**
 * binman_init() - Set up the binman symbol information
 *
 * This locates the binary symbol information in the device tree ready for use
 *
 * @return 0 if OK, -ENOMEM if out of memory, -EINVAL if there is no binman node
 */
int binman_init(void);

#endif
