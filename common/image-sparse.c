/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2014, The Linux Foundation. All rights reserved.
 * Portions Copyright 2014 Broadcom Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of The Linux Foundation nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * NOTE:
 *   Although it is very similar, this license text is not identical
 *   to the "BSD-3-Clause", therefore, DO NOT MODIFY THIS LICENSE TEXT!
 */

#include <config.h>
#include <common.h>
#include <div64.h>
#include <errno.h>
#include <image-sparse.h>
#include <malloc.h>
#include <part.h>
#include <sparse_format.h>

#include <linux/math64.h>

typedef struct sparse_buffer {
	void	*data;
	u32	length;
	u32	repeat;
	u16	type;
} sparse_buffer_t;

static uint32_t last_offset;

static unsigned int sparse_get_chunk_data_size(sparse_header_t *sparse,
					       chunk_header_t *chunk)
{
	return chunk->total_sz - sparse->chunk_hdr_sz;
}

static unsigned int sparse_block_size_to_storage(unsigned int size,
						 sparse_storage_t *storage,
						 sparse_header_t *sparse)
{
	return (unsigned int)lldiv((uint64_t)size * sparse->blk_sz,
				   storage->block_sz);
}

static bool sparse_chunk_has_buffer(chunk_header_t *chunk)
{
	switch (chunk->chunk_type) {
	case CHUNK_TYPE_RAW:
	case CHUNK_TYPE_FILL:
		return true;

	default:
		return false;
	}
}

static sparse_header_t *sparse_parse_header(void **data)
{
	/* Read and skip over sparse image header */
	sparse_header_t *sparse_header = (sparse_header_t *) *data;

	*data += sparse_header->file_hdr_sz;

	debug("=== Sparse Image Header ===\n");
	debug("magic: 0x%x\n", sparse_header->magic);
	debug("major_version: 0x%x\n", sparse_header->major_version);
	debug("minor_version: 0x%x\n", sparse_header->minor_version);
	debug("file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
	debug("chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
	debug("blk_sz: %d\n", sparse_header->blk_sz);
	debug("total_blks: %d\n", sparse_header->total_blks);
	debug("total_chunks: %d\n", sparse_header->total_chunks);

	return sparse_header;
}

static int sparse_parse_fill_chunk(sparse_header_t *sparse,
				   chunk_header_t *chunk)
{
	unsigned int chunk_data_sz = sparse_get_chunk_data_size(sparse, chunk);

	if (chunk_data_sz != sizeof(uint32_t))
		return -EINVAL;

	return 0;
}

static int sparse_parse_raw_chunk(sparse_header_t *sparse,
				  chunk_header_t *chunk)
{
	unsigned int chunk_data_sz = sparse_get_chunk_data_size(sparse, chunk);

	/* Check if the data size is a multiple of the main block size */
	if (chunk_data_sz % sparse->blk_sz)
		return -EINVAL;

	/* Check that the chunk size is consistent */
	if ((chunk_data_sz / sparse->blk_sz) != chunk->chunk_sz)
		return -EINVAL;

	return 0;
}

static chunk_header_t *sparse_parse_chunk(sparse_header_t *sparse,
					  void **image)
{
	chunk_header_t *chunk = (chunk_header_t *) *image;
	int ret;

	debug("=== Chunk Header ===\n");
	debug("chunk_type: 0x%x\n", chunk->chunk_type);
	debug("chunk_data_sz: 0x%x\n", chunk->chunk_sz);
	debug("total_size: 0x%x\n", chunk->total_sz);

	switch (chunk->chunk_type) {
	case CHUNK_TYPE_RAW:
		ret = sparse_parse_raw_chunk(sparse, chunk);
		if (ret)
			return NULL;
		break;

	case CHUNK_TYPE_FILL:
		ret = sparse_parse_fill_chunk(sparse, chunk);
		if (ret)
			return NULL;
		break;

	case CHUNK_TYPE_DONT_CARE:
	case CHUNK_TYPE_CRC32:
		debug("Ignoring chunk\n");
		break;

	default:
		printf("%s: Unknown chunk type: %x\n", __func__,
		       chunk->chunk_type);
		return NULL;
	}

	*image += sparse->chunk_hdr_sz;

	return chunk;
}

static int sparse_get_fill_buffer(sparse_header_t *sparse,
				  chunk_header_t *chunk,
				  sparse_buffer_t *buffer,
				  unsigned int blk_sz,
				  void *data)
{
	int i;

	buffer->type = CHUNK_TYPE_FILL;

	/*
	 * We create a buffer of one block, and ask it to be
	 * repeated as many times as needed.
	 */
	buffer->length = blk_sz;
	buffer->repeat = (chunk->chunk_sz * sparse->blk_sz) / blk_sz;

	buffer->data = memalign(ARCH_DMA_MINALIGN,
				ROUNDUP(blk_sz,
					ARCH_DMA_MINALIGN));
	if (!buffer->data)
		return -ENOMEM;

	for (i = 0; i < (buffer->length / sizeof(uint32_t)); i++)
		((uint32_t *)buffer->data)[i] = *(uint32_t *)(data);

	return 0;
}

static int sparse_get_raw_buffer(sparse_header_t *sparse,
				 chunk_header_t *chunk,
				 sparse_buffer_t *buffer,
				 unsigned int blk_sz,
				 void *data)
{
	unsigned int chunk_data_sz = sparse_get_chunk_data_size(sparse, chunk);

	buffer->type = CHUNK_TYPE_RAW;
	buffer->length = chunk_data_sz;
	buffer->data = data;
	buffer->repeat = 1;

	return 0;
}

static sparse_buffer_t *sparse_get_data_buffer(sparse_header_t *sparse,
					       chunk_header_t *chunk,
					       unsigned int blk_sz,
					       void **image)
{
	unsigned int chunk_data_sz = sparse_get_chunk_data_size(sparse, chunk);
	sparse_buffer_t *buffer;
	void *data = *image;
	int ret;

	*image += chunk_data_sz;

	if (!sparse_chunk_has_buffer(chunk))
		return NULL;

	buffer = calloc(sizeof(sparse_buffer_t), 1);
	if (!buffer)
		return NULL;

	switch (chunk->chunk_type) {
	case CHUNK_TYPE_RAW:
		ret = sparse_get_raw_buffer(sparse, chunk, buffer, blk_sz,
					    data);
		if (ret)
			return NULL;
		break;

	case CHUNK_TYPE_FILL:
		ret = sparse_get_fill_buffer(sparse, chunk, buffer, blk_sz,
					     data);
		if (ret)
			return NULL;
		break;

	default:
		return NULL;
	}

	debug("=== Buffer ===\n");
	debug("length: 0x%x\n", buffer->length);
	debug("repeat: 0x%x\n", buffer->repeat);
	debug("type: 0x%x\n", buffer->type);
	debug("data: 0x%p\n", buffer->data);

	return buffer;
}

static void sparse_put_data_buffer(sparse_buffer_t *buffer)
{
	if (buffer->type == CHUNK_TYPE_FILL)
		free(buffer->data);

	free(buffer);
}

int store_sparse_image(sparse_storage_t *storage, void *storage_priv,
		       unsigned int session_id, void *data)
{
	unsigned int chunk, offset;
	sparse_header_t *sparse_header;
	chunk_header_t *chunk_header;
	sparse_buffer_t *buffer;
	uint32_t start;
	uint32_t total_blocks = 0;
	int i;

	debug("=== Storage ===\n");
	debug("name: %s\n", storage->name);
	debug("block_size: 0x%x\n", storage->block_sz);
	debug("start: 0x%x\n", storage->start);
	debug("size: 0x%x\n", storage->size);
	debug("write: 0x%p\n", storage->write);
	debug("priv: 0x%p\n", storage_priv);

	sparse_header = sparse_parse_header(&data);
	if (!sparse_header) {
		printf("sparse header issue\n");
		return -EINVAL;
	}

	/*
	 * Verify that the sparse block size is a multiple of our
	 * storage backend block size
	 */
	div_u64_rem(sparse_header->blk_sz, storage->block_sz, &offset);
	if (offset) {
		printf("%s: Sparse image block size issue [%u]\n",
		       __func__, sparse_header->blk_sz);
		return -EINVAL;
	}

	/*
	 * If it's a new flashing session, start at the beginning of
	 * the partition. If not, then simply resume where we were.
	 */
	if (session_id > 0)
		start = last_offset;
	else
		start = storage->start;

	printf("Flashing sparse image on partition %s at offset 0x%x (ID: %d)\n",
	       storage->name, start * storage->block_sz, session_id);

	/* Start processing chunks */
	for (chunk = 0; chunk < sparse_header->total_chunks; chunk++) {
		uint32_t blkcnt;

		chunk_header = sparse_parse_chunk(sparse_header, &data);
		if (!chunk_header) {
			printf("Unknown chunk type");
			return -EINVAL;
		}

		/*
		 * If we have a DONT_CARE type, just skip the blocks
		 * and go on parsing the rest of the chunks
		 */
		if (chunk_header->chunk_type == CHUNK_TYPE_DONT_CARE) {
			blkcnt = sparse_block_size_to_storage(chunk_header->chunk_sz,
							      storage,
							      sparse_header);
#ifdef CONFIG_FASTBOOT_FLASH_MMC_DEV
			total_blocks += blkcnt;
#endif
			continue;
		}

		/* Retrieve the buffer we're going to write */
		buffer = sparse_get_data_buffer(sparse_header, chunk_header,
						storage->block_sz, &data);
		if (!buffer)
			continue;

		blkcnt = (buffer->length / storage->block_sz) * buffer->repeat;

		if ((start + total_blocks + blkcnt) >
		    (storage->start + storage->size)) {
			printf("%s: Request would exceed partition size!\n",
			       __func__);
			return -EINVAL;
		}

		for (i = 0; i < buffer->repeat; i++) {
			unsigned long buffer_blk_cnt;
			int ret;

			buffer_blk_cnt = buffer->length / storage->block_sz;

			ret = storage->write(storage, storage_priv,
					     start + total_blocks,
					     buffer_blk_cnt,
					     buffer->data);
			if (ret < 0) {
				printf("%s: Write %d failed %d\n",
				       __func__, i, ret);
				return ret;
			}

			total_blocks += ret;
		}

		sparse_put_data_buffer(buffer);
	}

	debug("Wrote %d blocks, expected to write %d blocks\n",
	      total_blocks,
	      sparse_block_size_to_storage(sparse_header->total_blks,
					   storage, sparse_header));
	printf("........ wrote %d blocks to '%s'\n", total_blocks,
	       storage->name);

	if (total_blocks !=
	    sparse_block_size_to_storage(sparse_header->total_blks,
					 storage, sparse_header)) {
		printf("sparse image write failure\n");
		return -EIO;
	}

	last_offset = start + total_blocks;

	return 0;
}
