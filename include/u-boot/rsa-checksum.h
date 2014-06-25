/*
 * Copyright (c) 2013, Andreas Oetken.
 *
 * SPDX-License-Identifier:    GPL-2.0+
*/

#ifndef _RSA_CHECKSUM_H
#define _RSA_CHECKSUM_H

#include <errno.h>
#include <image.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>

extern const uint8_t padding_sha256_rsa4096[];
extern const uint8_t padding_sha256_rsa2048[];
extern const uint8_t padding_sha1_rsa2048[];

void sha256_calculate(const struct image_region region[], int region_count,
		      uint8_t *checksum);
void sha1_calculate(const struct image_region region[], int region_count,
		    uint8_t *checksum);

#endif
