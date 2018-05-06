/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _DM_OF_EXTRA_H
#define _DM_OF_EXTRA_H

#include <dm/ofnode.h>

enum fmap_compress_t {
	FMAP_COMPRESS_NONE,
	FMAP_COMPRESS_LZO,
};

enum fmap_hash_t {
	FMAP_HASH_NONE,
	FMAP_HASH_SHA1,
	FMAP_HASH_SHA256,
};

/* A flash map entry, containing an offset and length */
struct fmap_entry {
	uint32_t offset;
	uint32_t length;
	uint32_t used;			/* Number of bytes used in region */
	enum fmap_compress_t compress_algo;	/* Compression type */
	enum fmap_hash_t hash_algo;		/* Hash algorithm */
	const uint8_t *hash;			/* Hash value */
	int hash_size;				/* Hash size */
};

/**
 * Read a flash entry from the fdt
 *
 * @param node	Reference to node to read
 * @param name		Name of node being read
 * @param entry		Place to put offset and size of this node
 * @return 0 if ok, -ve on error
 */
int of_read_fmap_entry(ofnode node, const char *name,
		       struct fmap_entry *entry);

#endif
