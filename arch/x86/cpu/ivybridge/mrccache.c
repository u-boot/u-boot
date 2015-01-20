/*
 * From Coreboot src/southbridge/intel/bd82x6x/mrccache.c
 *
 * Copyright (C) 2014 Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <net.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/mrccache.h>
#include <asm/arch/sandybridge.h>

static struct mrc_data_container *next_mrc_block(
	struct mrc_data_container *mrc_cache)
{
	/* MRC data blocks are aligned within the region */
	u32 mrc_size = sizeof(*mrc_cache) + mrc_cache->data_size;
	if (mrc_size & (MRC_DATA_ALIGN - 1UL)) {
		mrc_size &= ~(MRC_DATA_ALIGN - 1UL);
		mrc_size += MRC_DATA_ALIGN;
	}

	u8 *region_ptr = (u8 *)mrc_cache;
	region_ptr += mrc_size;
	return (struct mrc_data_container *)region_ptr;
}

static int is_mrc_cache(struct mrc_data_container *cache)
{
	return cache && (cache->signature == MRC_DATA_SIGNATURE);
}

/*
 * Find the largest index block in the MRC cache. Return NULL if none is
 * found.
 */
struct mrc_data_container *mrccache_find_current(struct fmap_entry *entry)
{
	struct mrc_data_container *cache, *next;
	ulong base_addr, end_addr;
	uint id;

	base_addr = (1ULL << 32) - CONFIG_ROM_SIZE + entry->offset;
	end_addr = base_addr + entry->length;
	cache = NULL;

	/* Search for the last filled entry in the region */
	for (id = 0, next = (struct mrc_data_container *)base_addr;
	     is_mrc_cache(next);
	     id++) {
		cache = next;
		next = next_mrc_block(next);
		if ((ulong)next >= end_addr)
			break;
	}

	if (id-- == 0) {
		debug("%s: No valid MRC cache found.\n", __func__);
		return NULL;
	}

	/* Verify checksum */
	if (cache->checksum != compute_ip_checksum(cache->data,
						   cache->data_size)) {
		printf("%s: MRC cache checksum mismatch\n", __func__);
		return NULL;
	}

	debug("%s: picked entry %u from cache block\n", __func__, id);

	return cache;
}

/**
 * find_next_mrc_cache() - get next cache entry
 *
 * @entry:	MRC cache flash area
 * @cache:	Entry to start from
 *
 * @return next cache entry if found, NULL if we got to the end
 */
static struct mrc_data_container *find_next_mrc_cache(struct fmap_entry *entry,
		struct mrc_data_container *cache)
{
	ulong base_addr, end_addr;

	base_addr = (1ULL << 32) - CONFIG_ROM_SIZE + entry->offset;
	end_addr = base_addr + entry->length;

	cache = next_mrc_block(cache);
	if ((ulong)cache >= end_addr) {
		/* Crossed the boundary */
		cache = NULL;
		debug("%s: no available entries found\n", __func__);
	} else {
		debug("%s: picked next entry from cache block at %p\n",
		      __func__, cache);
	}

	return cache;
}

int mrccache_update(struct spi_flash *sf, struct fmap_entry *entry,
		    struct mrc_data_container *cur)
{
	struct mrc_data_container *cache;
	ulong offset;
	ulong base_addr;
	int ret;

	/* Find the last used block */
	base_addr = (1ULL << 32) - CONFIG_ROM_SIZE + entry->offset;
	debug("Updating MRC cache data\n");
	cache = mrccache_find_current(entry);
	if (cache && (cache->data_size == cur->data_size) &&
	    (!memcmp(cache, cur, cache->data_size + sizeof(*cur)))) {
		debug("MRC data in flash is up to date. No update\n");
		return -EEXIST;
	}

	/* Move to the next block, which will be the first unused block */
	if (cache)
		cache = find_next_mrc_cache(entry, cache);

	/*
	 * If we have got to the end, erase the entire mrc-cache area and start
	 * again at block 0.
	 */
	if (!cache) {
		debug("Erasing the MRC cache region of %x bytes at %x\n",
		      entry->length, entry->offset);

		ret = spi_flash_erase(sf, entry->offset, entry->length);
		if (ret) {
			debug("Failed to erase flash region\n");
			return ret;
		}
		cache = (struct mrc_data_container *)base_addr;
	}

	/* Write the data out */
	offset = (ulong)cache - base_addr + entry->offset;
	debug("Write MRC cache update to flash at %lx\n", offset);
	ret = spi_flash_write(sf, offset, cur->data_size + sizeof(*cur), cur);
	if (ret) {
		debug("Failed to write to SPI flash\n");
		return ret;
	}

	return 0;
}
