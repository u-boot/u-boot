/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 */

#ifndef __LZ4_H
#define __LZ4_H

/**
 * ulz4fn() - Decompress LZ4 data
 *
 * @src: Source data to decompress
 * @srcn: Length of source data
 * @dst: Destination for uncompressed data
 * @dstn: Returns length of uncompressed data
 * @return 0 if OK, -EPROTONOSUPPORT if the magic number or version number are
 *	not recognised or independent blocks are used, -EINVAL if the reserved
 *	fields are non-zero, or input is overrun, -EENOBUFS if the destination
 *	buffer is overrun, -EEPROTO if the compressed data causes an error in
 *	the decompression algorithm
 */
int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn);

#endif
