/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_MRCCACHE_H
#define _ASM_ARCH_MRCCACHE_H

#define MRC_DATA_ALIGN		0x1000
#define MRC_DATA_SIGNATURE	(('M' << 0) | ('R' << 8) | ('C' << 16) | \
					('D'<<24))

__packed struct mrc_data_container {
	u32	signature;	/* "MRCD" */
	u32	data_size;	/* Size of the 'data' field */
	u32	checksum;	/* IP style checksum */
	u32	reserved;	/* For header alignment */
	u8	data[0];	/* Variable size, platform/run time dependent */
};

struct fmap_entry;
struct spi_flash;

/**
 * mrccache_find_current() - find the latest MRC cache record
 *
 * This searches the MRC cache region looking for the latest record to use
 * for setting up SDRAM
 *
 * @entry:	Information about the position and size of the MRC cache
 * @return pointer to latest record, or NULL if none
 */
struct mrc_data_container *mrccache_find_current(struct fmap_entry *entry);

/**
 * mrccache_update() - update the MRC cache with a new record
 *
 * This writes a new record to the end of the MRC cache. If the new record is
 * the same as the latest record then the write is skipped
 *
 * @sf:		SPI flash to write to
 * @entry:	Position and size of MRC cache in SPI flash
 * @cur:	Record to write
 * @return 0 if updated, -EEXIST if the record is the same as the latest
 * record, other error if SPI write failed
 */
int mrccache_update(struct spi_flash *sf, struct fmap_entry *entry,
		    struct mrc_data_container *cur);

#endif
