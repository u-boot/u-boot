/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>

int spi_set_wordlen(struct spi_slave *slave, unsigned int wordlen)
{
	if (wordlen == 0 || wordlen > 32) {
		printf("spi: invalid wordlen %d\n", wordlen);
		return -1;
	}

	slave->wordlen = wordlen;

	return 0;
}

void *spi_do_alloc_slave(int offset, int size, unsigned int bus,
			 unsigned int cs)
{
	struct spi_slave *slave;
	void *ptr;

	ptr = malloc(size);
	if (ptr) {
		memset(ptr, '\0', size);
		slave = (struct spi_slave *)(ptr + offset);
		slave->bus = bus;
		slave->cs = cs;
		slave->wordlen = SPI_DEFAULT_WORDLEN;
	}

	return ptr;
}
