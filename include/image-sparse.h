/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <part.h>
#include <sparse_format.h>

#define ROUNDUP(x, y)	(((x) + ((y) - 1)) & ~((y) - 1))

typedef struct sparse_storage {
	unsigned int	block_sz;
	unsigned int	start;
	unsigned int	size;
	const char	*name;

	int	(*write)(struct sparse_storage *storage, void *priv,
			 unsigned int offset, unsigned int size,
			 char *data);
} sparse_storage_t;

static inline int is_sparse_image(void *buf)
{
	sparse_header_t *s_header = (sparse_header_t *)buf;

	if ((le32_to_cpu(s_header->magic) == SPARSE_HEADER_MAGIC) &&
	    (le16_to_cpu(s_header->major_version) == 1))
		return 1;

	return 0;
}

int store_sparse_image(sparse_storage_t *storage, void *storage_priv,
		       unsigned int session_id, void *data);
