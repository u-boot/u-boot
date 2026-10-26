/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018, Linaro Limited
 */

#ifndef	_AVB_VERIFY_H
#define _AVB_VERIFY_H

#include <../lib/libavb/libavb.h>
#include <blk.h>
#include <mapmem.h>
#include <part.h>

#define AVB_MAX_ARGS			1024
#define VERITY_TABLE_OPT_RESTART	"restart_on_corruption"
#define VERITY_TABLE_OPT_LOGGING	"ignore_corruption"
#define ALLOWED_BUF_ALIGN		8

enum avb_boot_state {
	AVB_GREEN,
	AVB_YELLOW,
	AVB_ORANGE,
	AVB_RED,
};

#define AVB_MAX_PRELOADED	4

/**
 * struct avb_preloaded - a partition already loaded into memory by the caller
 *
 * libavb hashes the preloaded bytes instead of reading the partition again,
 * so what gets verified is exactly what the bootloader goes on to boot.
 *
 * @name: partition name including the slot suffix, e.g. "boot_a"
 * @addr: start of the buffer holding the partition data
 * @size: number of bytes loaded so far
 * @max_size: capacity of the buffer (at most the partition size)
 */
struct avb_preloaded {
	char name[32];
	void *addr;
	size_t size;
	size_t max_size;
};

struct AvbOpsData {
	struct AvbOps ops;
	enum uclass_id uclass_id;
	int dev_num;
	enum avb_boot_state boot_state;
#ifdef CONFIG_OPTEE_TA_AVB
	struct udevice *tee;
	u32 session;
#endif
	struct avb_preloaded preloaded[AVB_MAX_PRELOADED];
	int num_preloaded;
};

struct avb_blk_part {
	struct blk_desc *blk;
	struct disk_partition info;
};

enum avb_io_type {
	AVB_IO_READ,
	AVB_IO_WRITE
};

AvbOps *avb_ops_alloc(enum uclass_id uclass_id, int dev_num);

/**
 * avb_ops_add_preloaded() - register a partition that is already in memory
 *
 * @ops: AvbOps from avb_ops_alloc()
 * @partition: partition name including the slot suffix, e.g. "boot_a"
 * @addr: buffer holding the first @size bytes of the partition
 * @size: bytes loaded in @addr
 * @max_size: capacity of @addr; a hash descriptor covering more than this
 *	fails verification instead of overflowing the buffer
 * Return: 0 on success, -ENOSPC when no slot is left, -EINVAL on a bad name
 */
int avb_ops_add_preloaded(AvbOps *ops, const char *partition, void *addr,
			  size_t size, size_t max_size);
#ifdef CONFIG_OPTEE_TA_AVB
AvbIOResult avb_write_lock_state(AvbOps *ops, bool locked);
#endif
void avb_ops_free(AvbOps *ops);

/**
 * avb_read_root_key_digest() - get the SHA-256 digest of the trusted AVB
 * root public key
 *
 * This is the root of trust for AVB: validate_vbmeta_public_key() hashes the
 * key embedded in the (untrusted) vbmeta and compares it against the digest
 * returned here. The provider is selected via CONFIG_AVB_ROOT_KEY_* and, for
 * CONFIG_AVB_ROOT_KEY_BOARD, may be overridden by a SoC/board specific strong
 * definition (e.g. reading a hash fused into OTP).
 *
 * @ops: AvbOps, contains AVB ops handlers (needed by the TEE provider)
 * @digest: output buffer of AVB_SHA256_DIGEST_SIZE bytes
 *
 * @return:
 *      AVB_IO_RESULT_OK on success (@digest populated)
 *      any other AvbIOResult fails verification closed (key not trusted)
 */
AvbIOResult avb_read_root_key_digest(AvbOps *ops, uint8_t *digest);

char *avb_set_state(AvbOps *ops, enum avb_boot_state boot_state);
char *avb_set_enforce_verity(const char *cmdline);
char *avb_set_ignore_corruption(const char *cmdline);

char *append_cmd_line(char *cmdline_orig, char *cmdline_new);
const char *str_avb_io_error(AvbIOResult res);
const char *str_avb_slot_error(AvbSlotVerifyResult res);
/**
 * ============================================================================
 * I/O helper inline functions
 * ============================================================================
 */
static inline uint64_t calc_offset(struct avb_blk_part *part, int64_t offset)
{
	u64 part_size = part->info.size * part->info.blksz;

	if (offset < 0)
		return part_size + offset;

	return offset;
}

static inline size_t get_sector_buf_size(void)
{
	return (size_t)CONFIG_AVB_BUF_SIZE;
}

static inline void *get_sector_buf(void)
{
	return map_sysmem(CONFIG_AVB_BUF_ADDR, CONFIG_AVB_BUF_SIZE);
}

static inline bool is_buf_unaligned(void *buffer)
{
	return (bool)((uintptr_t)buffer % ALLOWED_BUF_ALIGN);
}

static inline int get_boot_device(AvbOps *ops)
{
	struct AvbOpsData *data;

	if (ops) {
		data = ops->user_data;
		if (data)
			return data->dev_num;
	}

	return -1;
}

/* Mirrors get_boot_device(): no ops means no interface, not a default one */
static inline enum uclass_id get_boot_interface(AvbOps *ops)
{
	struct AvbOpsData *data;

	if (ops) {
		data = ops->user_data;
		if (data)
			return data->uclass_id;
	}

	return UCLASS_INVALID;
}

#endif /* _AVB_VERIFY_H */
