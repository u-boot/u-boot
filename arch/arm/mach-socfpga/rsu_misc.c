// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#include <linux/compat.h>
#include <linux/compiler.h>
#include <linux/errno.h>
#include <asm/arch/rsu.h>
#include <asm/arch/rsu_ll.h>
#include <asm/arch/rsu_misc.h>
#include <asm/types.h>
#include <u-boot/zlib.h>
#include <errno.h>
#include <exports.h>

#define LOG_BUF_SIZE	1024

static char *cb_buffer;
static int cb_buffer_togo;

static char *reserved_names[] = {
	"BOOT_INFO",
	"FACTORY_IMAGE",
	"SPT",
	"SPT0",
	"SPT1",
	"CPB",
	"CPB0",
	"CPB1",
	""
};

struct pointer_block {
	u32 num_ptrs;
	u32 RSVD0;
	u64 ptrs[4];
	u8 RSVD1[0xd4];
	u32 crc;
};

/**
 * enum rsu_block_type - enumeration for image block types
 * @SECTION_BLOCK: section block
 * @SIGNATURE_BLOCK: signature block
 * @REGULAR_BLOCK: all other block types
 */
enum rsu_block_type {
	SECTION_BLOCK = 0,
	SIGNATURE_BLOCK,
	REGULAR_BLOCK
};

/* maximum number of sections supported for an image */
#define MAX_SECTIONS 64

/**
 * struct rsu_image_state - structure for stated of image processing
 * @offset: current block offset in bytes
 * @block_type: current block type
 * @sections: identified section offsets
 * @no_sections: number of identified sections
 * @absolute: current image is an absolute image
 *
 * This structure is used to maintain the state of image parsing, both for
 * relocating images to final destination in flash, and also for verifying
 * images already stored in flash.
 */
struct rsu_image_state {
	int offset;
	enum rsu_block_type block_type;
	u64 sections[MAX_SECTIONS];
	int no_sections;
	int absolute;
};

/**
 * find_section() - search section in the current list of identified sections
 * @state: current state machine state
 * @section: section to be searched
 *
 * Return: 1 if section is found, 0 if section is not found
 */
static int find_section(struct rsu_image_state *state, u64 section)
{
	int x;

	for (x = 0; x < state->no_sections; x++)
		if (section == state->sections[x])
			return 1;

	return 0;
}

/**
 * add_section() - add section to the current list of identified sections
 * @state: current state machine state
 * @section: section to be added
 *
 * Return: 0 on success, or -ve on error
 */
static int add_section(struct rsu_image_state *state, u64 section)
{
	if (find_section(state, section))
		return 0;

	if (state->no_sections >= MAX_SECTIONS)
		return -ENOSPC;

	state->sections[state->no_sections++] = section;

	return 0;
}

/**
 * swap_bits() - swap bits
 * @data: pointer point to data
 * @len: data length
 */
void swap_bits(char *data, int len)
{
	int x, y;
	char tmp;

	for (x = 0; x < len; x++) {
		tmp = 0;
		for (y = 0; y < 8; y++) {
			tmp <<= 1;
			if (data[x] & 1)
				tmp |= 1;
			data[x] >>= 1;
		}
		data[x] = tmp;
	}
}

int rsu_pow(u32 x, u32 y)
{
	if (y == 0)
		return 1;
	else if ((y % 2) == 0)
		return rsu_pow(x, y / 2) * rsu_pow(x, y / 2);
	else
		return x * rsu_pow(x, y / 2) * rsu_pow(x, y / 2);
}

/**
 * rsu_misc_is_rsvd_name() - check if a reserved name
 *
 * @name: name to check
 *
 * Returns 1 if a reserved name, or 0 for not
 */
int rsu_misc_is_rsvd_name(char *name)
{
	int x;

	for (x = 0; reserved_names[x][0] != '\0'; x++)
		if (strcmp(name, reserved_names[x]) == 0)
			return 1;

	return 0;
}

/**
 * rsu_misc_is_slot() - check if a read only or reserved partition
 * @ll_intf: pointer to ll_intf
 * @part_num: partition number
 *
 * Return 1 if not read only or reserved, or 0 for is
 */
int rsu_misc_is_slot(struct rsu_ll_intf *ll_intf, int part_num)
{
	if (ll_intf->partition.readonly(part_num) ||
	    ll_intf->partition.reserved(part_num))
		return 0;

	if (rsu_misc_is_rsvd_name(ll_intf->partition.name(part_num)))
		return 0;

	return 1;
}

/**
 * rsu_misc_slot2part() - get partition number from the slot
 * @ll_intf: pointer to ll_intf
 * @slot: slot number
 *
 * Return: partition number on success, or -ve on error
 */
int rsu_misc_slot2part(struct rsu_ll_intf *ll_intf, int slot)
{
	int partitions;
	int cnt = 0;

	partitions = ll_intf->partition.count();

	for (int x = 0; x < partitions; x++) {
		if (rsu_misc_is_slot(ll_intf, x)) {
			if (slot == cnt)
				return x;
			cnt++;
		}
	}

	return -EINVAL;
}

/**
 * rsu_misc_writeprotected() - check if a slot is protected
 * @slot: the number of slot to be checked
 *
 * Return 1 if a slot is protected, 0 for not
 */
int rsu_misc_writeprotected(int slot)
{
	char *protected;
	int protected_slot_numb;

	/* protect works only for slot 0-31 */
	if (slot > 31)
		return 0;

	protected = env_get("rsu_protected_slot");
	if (!protected)
		return 0;

	protected_slot_numb = (int)simple_strtol(protected, NULL, 0);
	if (protected_slot_numb < 0 || protected_slot_numb > 31) {
		rsu_log(RSU_WARNING,
			"protected slot works only on the first 32 slots\n");
		return 0;
	}

	if (protected_slot_numb == slot)
		return 1;
	else
		return 0;
}

/**
 * rsu_misc_spt_checksum_enabled() - check if the SPT checksum is enabled
 *
 * Return 1 if SPT checksum mechanism is enabled, 0 for disabled
 */
int rsu_misc_spt_checksum_enabled(void)
{
	char *c_enabled;
	int checksum_enabled;

	c_enabled = env_get("rsu_spt_checksum");
	if (!c_enabled)
		return 0;

	checksum_enabled = (int)simple_strtol(c_enabled, NULL, 0);
	if (checksum_enabled)
		return 1;

	return 0;
}

/**
 * rsu_misc_safe_strcpy() - buffer copy
 * @dst: pointer to dst
 * @dsz: dst buffer size
 * @src: pointer to src
 * @ssz: src buffer size
 */
void rsu_misc_safe_strcpy(char *dst, int dsz, char *src, int ssz)
{
	int len;

	if (!dst || dsz <= 0)
		return;

	if (!src || ssz <= 0) {
		dst[0] = '\0';
		return;
	}

	len = strnlen(src, ssz);
	if (len >= dsz)
		len = dsz - 1;

	memcpy(dst, src, len);
	dst[len] = '\0';
}

/**
 * rsu_cb_buf_init() - initialize buffer parameters
 * @buf: pointer to buf
 * @size: size of buffer
 *
 * Return: 0 on success, or -ve on error
 */
int rsu_cb_buf_init(void *buf, int size)
{
	if (!buf || size <= 0)
		return -EINVAL;

	cb_buffer = (char *)buf;
	cb_buffer_togo = size;

	return 0;
}

/**
 * rsu_cb_buf_exit() - reset buffer parameters
 */
void rsu_cb_buf_exit(void)
{
	cb_buffer = NULL;
	cb_buffer_togo = -1;
}

/**
 * rsu_cb_buf() - copy data to buffer
 * @buf: pointer to data buffer
 * @len: size of data buffer
 *
 * Return the buffer data size
 */
int rsu_cb_buf(void *buf, int len)
{
	int read_len;

	if (!cb_buffer_togo)
		return 0;

	if (!cb_buffer || cb_buffer_togo < 0 || !buf || len < 0)
		return -EINVAL;

	if (cb_buffer_togo < len)
		read_len = cb_buffer_togo;
	else
		read_len = len;

	memcpy(buf, cb_buffer, read_len);

	cb_buffer += read_len;
	cb_buffer_togo -= read_len;

	if (!cb_buffer_togo)
		cb_buffer = NULL;

	return read_len;
}

/**
 * sig_block_process() - process signature block
 * @state: current state machine state
 * @block: signature block
 * @info: slot where the data will be written
 *
 * Determine if the signature block is part of an absolute image, and add its
 * section pointers to the list of identified sections.
 *
 * Return: zero value for success, or negative value on error
 */
static int sig_block_process(struct rsu_image_state *state,	void *block,
			     struct rsu_slot_info *info)
{
	char *data = (char *)block;
	struct pointer_block *ptr_blk = (struct pointer_block *)(data
					+ SIG_BLOCK_PTR_OFFS);
	int x;

	/* Determine if absolute image - only done for 2nd block in an image
	 * which is always a signature block
	 */
	if (state->offset == IMAGE_BLOCK_SZ)
		for (x = 0; x < 4; x++)
			if (ptr_blk->ptrs[x] > (__u64)info->size) {
				state->absolute = 1;
				rsu_log(RSU_DEBUG, "Found absolute image\n");
				break;
			}

	/* Add pointers to list of identified sections */
	for (x = 0; x < 4; x++)
		if (ptr_blk->ptrs[x]) {
			if (state->absolute)
				add_section(state, ptr_blk->ptrs[x] -
					    info->offset);
			else
				add_section(state, ptr_blk->ptrs[x]);
		}

	return 0;
}

/**
 * sig_block_adjust() - adjust signature block pointers before writing to flash
 * @state: current state machine state
 * @block: signature block
 * @info: slot where the data will be written
 *
 * This function checks that the section pointers are consistent, and for non-
 * absolute images it updates them to match the destination slot, also re-
 * computing the CRC.
 *
 * Return: zero value for success, -1 on error
 */
static int sig_block_adjust(struct rsu_image_state *state, void *block,
			    struct rsu_slot_info *info)
{
	u32 calc_crc;
	int x;
	char *data = (char *)block;
	struct pointer_block *ptr_blk = (struct pointer_block *)(data
					+ SIG_BLOCK_PTR_OFFS);

	/*
	 * Check CRC on 4kB block before proceeding.  All bytes must be
	 * bit-swapped before they can used in zlib CRC32 library function.
	 * The CRC value is stored in big endian in the bitstream.
	 */
	swap_bits(block, IMAGE_BLOCK_SZ);
	calc_crc = crc32(0, (uchar *)block, SIG_BLOCK_CRC_OFFS);
	if (be32_to_cpu(ptr_blk->crc) != calc_crc) {
		rsu_log(RSU_ERR,
			"Error: Bad CRC32. Calc = %08X / From Block = %08x\n",
			calc_crc, be32_to_cpu(ptr_blk->crc));
		return -EBADMSG;
	}
	swap_bits(block, IMAGE_BLOCK_SZ);

	/* Check pointers */
	for (x = 0; x < 4; x++) {
		u64 ptr = ptr_blk->ptrs[x];

		if (!ptr)
			continue;

		if (state->absolute)
			ptr -= info->offset;

		if (ptr > info->size) {
			rsu_log(RSU_ERR,
				"Error: A pointer not within the slot\n");
			return -EINVAL;
		}
	}

	/* Absolute images do not require pointer updates */
	if (state->absolute)
		return 0;

	/* Update pointers */
	for (x = 0; x < 4; x++) {
		if (ptr_blk->ptrs[x]) {
			u64 old =  ptr_blk->ptrs[x];

			ptr_blk->ptrs[x] += info->offset;
			rsu_log(RSU_DEBUG,
				"Adjusting pointer 0x%llx -> 0x%llx\n",
				old, ptr_blk->ptrs[x]);
		}
	}

	/* Update CRC in block */
	swap_bits(block, IMAGE_BLOCK_SZ);
	calc_crc = crc32(0, (uchar *)block, SIG_BLOCK_CRC_OFFS);
	ptr_blk->crc = be32_to_cpu(calc_crc);
	swap_bits(block, IMAGE_BLOCK_SZ);

	return 0;
}

/**
 * block_compare() - compare two image blocks
 * @state: current state machine state
 * @block: input data provided by user
 * @vblock: verification data read from flash
 *
 * Return: non-negative value for successful comparisor, or negative value on
 * failure or comparison difference found.
 */
static int block_compare(struct rsu_image_state *state, void *block,
			 void *vblock)
{
	char *buf = (char *)block;
	char *vbuf = (char *)vblock;
	int x;

	for (x = 0; x < IMAGE_BLOCK_SZ; x++)
		if (vbuf[x] != buf[x]) {
			rsu_log(RSU_ERR, "Expect %02X, got %02X @0x%08X\n",
				buf[x], vbuf[x], state->offset + x);
			return -ECMP;
		}

	return 0;
}

/**
 * sig_block_compare() - compare two signature blocks
 * @state: current state machine state
 * @ublock: input data provided by user
 * @vblock: verification data read from flash
 * @info: slot where the verification data was read from
 *
 * Absolute images are compared directly, while for non-absolute images the
 * pointers and associated CRC are re-computed to see if they match.
 *
 * Return: zero for success, or negative value on erorr or finding differences.
 */
static int sig_block_compare(struct rsu_image_state *state, void *ublock,
			     void *vblock, struct rsu_slot_info *info)
{
	u32 calc_crc;
	int x;
	char block[IMAGE_BLOCK_SZ];
	struct pointer_block *ptr_blk = (struct pointer_block *)(block +
		SIG_BLOCK_PTR_OFFS);

	rsu_log(RSU_DEBUG, "Compare signature block @0x%08x\n", state->offset);

	/* Make a copy of the data provided by the user */
	memcpy(block, ublock, IMAGE_BLOCK_SZ);

	/* Update signature block to match what we expect in flash */
	if (!state->absolute) {
		/* Update pointers */
		for (x = 0; x < 4; x++)
			if (ptr_blk->ptrs[x])
				ptr_blk->ptrs[x] += info->offset;

		/* Update CRC in block */
		swap_bits(block, IMAGE_BLOCK_SZ);
		calc_crc = crc32(0, (uchar *)block, SIG_BLOCK_CRC_OFFS);
		ptr_blk->crc = be32_to_cpu(calc_crc);
		swap_bits(block, IMAGE_BLOCK_SZ);
	}

	return block_compare(state, block, vblock);
}

/**
 * rsu_misc_image_block_init() - initialize state machine for processing blocks
 * @state: current state machine state
 *
 * Function is called before processing images either for writing to flash or
 * for comparison with verification data.
 *
 * Returns 0 on success, or -1 on error
 */
static int rsu_misc_image_block_init(struct rsu_image_state *state)
{
	rsu_log(RSU_DEBUG, "Resetting image block state machine\n");

	state->no_sections = 1;
	add_section(state, 0);
	state->block_type = REGULAR_BLOCK;
	state->absolute = 0;
	state->offset = -IMAGE_BLOCK_SZ;

	return 0;
}

/**
 * rsu_misc_image_block_process() - process image blocks
 *
 * @state: current state machine state
 * @block: pointer to current 4KB image block
 * @vblock: pointer to current 4KB image verification block
 * @info: rsu_slot_info structure for target slot
 *
 * Image blocks are processed either for updating before writing to flash
 * (when vblock==NULL) or for comparison with verification data
 * (when vblock!=NULL)
 *
 * Returns: 0 on success, or -ve on error
 */
static int rsu_misc_image_block_process(struct rsu_image_state *state,
					void *block, void *vblock,
					struct rsu_slot_info *info)
{
	u32 magic;
	int ret;

	state->offset += IMAGE_BLOCK_SZ;

	if (find_section(state, state->offset))
		state->block_type = SECTION_BLOCK;

	switch (state->block_type) {
	case SECTION_BLOCK:
		magic = *(__u32 *)block;
		if (magic == CMF_MAGIC) {
			rsu_log(RSU_DEBUG, "Found CMF sect @0x%08x\n",
				state->offset);
			state->block_type = SIGNATURE_BLOCK;
		} else {
			state->block_type = REGULAR_BLOCK;
		}

		if (vblock)
			return block_compare(state, block, vblock);
		break;

	case SIGNATURE_BLOCK:
		rsu_log(RSU_DEBUG, "Found signature block @0x%08x\n",
			state->offset);

		ret = sig_block_process(state, block, info);
		if (ret)
			return ret;

		state->block_type = REGULAR_BLOCK;

		if (vblock)
			return sig_block_compare(state, block, vblock, info);

		ret = sig_block_adjust(state, block, info);
		if (ret)
			return ret;

		break;

	case REGULAR_BLOCK:
		break;
	}

	if (vblock)
		return block_compare(state, block, vblock);

	return 0;
}

/**
 * rsu_cb_program_common - callback to program flash
 * @ll_intf: pointer to ll_intf
 * @slot: slot number
 * @callback: callback function pointer
 * @rawdata: flag (raw data or not)
 *
 * Return 0 if success, or error code
 */
int rsu_cb_program_common(struct rsu_ll_intf *ll_intf, int slot,
			  rsu_data_callback callback, int rawdata)
{
	int part_num;
	int offset;
	unsigned char buf[IMAGE_BLOCK_SZ];
	unsigned char vbuf[IMAGE_BLOCK_SZ];
	int cnt, c, done;
	int x;
	struct rsu_slot_info info;
	struct rsu_image_state state;

	if (!ll_intf)
		return -EINTF;

	if (slot < 0)
		return -ESLOTNUM;

	if (rsu_misc_writeprotected(slot)) {
		rsu_log(RSU_ERR,
			"Trying to program a write protected slot\n");
		return -EWRPROT;
	}

	if (rsu_slot_get_info(slot, &info)) {
		rsu_log(RSU_ERR, "Unable to read slot info\n");
		return -ESLOTNUM;
	}

	part_num = rsu_misc_slot2part(ll_intf, slot);
	if (part_num < 0)
		return -ESLOTNUM;

	if (ll_intf->priority.get(part_num) > 0) {
		rsu_log(RSU_ERR,
			"Trying to program a slot already in use\n");
		return -EPROGRAM;
	}

	if (!callback)
		return -EARGS;

	offset = 0;
	done = 0;

	if (rsu_misc_image_block_init(&state))
		return -EPROGRAM;

	while (!done) {
		cnt = 0;
		while (cnt < IMAGE_BLOCK_SZ) {
			c = callback(buf + cnt, IMAGE_BLOCK_SZ - cnt);
			if (c == 0) {
				done = 1;
				break;
			} else if (c < 0) {
				return -ECALLBACK;
			}
			cnt += c;
		}

		if (cnt == 0)
			break;

		if (!rawdata)
			if (rsu_misc_image_block_process(&state, buf, NULL,
							 &info))
				return -EPROGRAM;

		if ((offset + cnt) > ll_intf->partition.size(part_num)) {
			rsu_log(RSU_ERR,
				"Trying to program too much data into slot\n");
			return -ESIZE;
		}

		if (ll_intf->data.write(part_num, offset, cnt, buf))
			return -ELOWLEVEL;

		if (ll_intf->data.read(part_num, offset, cnt, vbuf))
			return -ELOWLEVEL;

		for (x = 0; x < cnt; x++)
			if (vbuf[x] != buf[x]) {
				rsu_log(RSU_DEBUG,
					"Expect %02X, got %02X @ 0x%08X\n",
					buf[x], vbuf[x], offset + x);
				return -ECMP;
			}

		offset += cnt;
	}

	if (!rawdata && ll_intf->priority.add(part_num))
		return -ELOWLEVEL;

	return 0;
}

/**
 * rsu_cb_verify_common() - callback for data verification
 * @ll_intf: pointer to ll_intf
 * @slot: slot number
 * @callback: callback function pointer
 * @rawdata: flag (raw data or not)
 *
 * Return 0 if success, or error code
 */
int rsu_cb_verify_common(struct rsu_ll_intf *ll_intf, int slot,
			 rsu_data_callback callback, int rawdata)
{
	int part_num;
	int offset;
	unsigned char buf[IMAGE_BLOCK_SZ];
	unsigned char vbuf[IMAGE_BLOCK_SZ];
	int cnt, c, done;
	int x;
	struct rsu_slot_info info;
	struct rsu_image_state state;

	if (!ll_intf)
		return -EINTF;

	if (rsu_slot_get_info(slot, &info)) {
		rsu_log(RSU_ERR, "Unable to read slot info\n");
		return -ESLOTNUM;
	}

	part_num = rsu_misc_slot2part(ll_intf, slot);
	if (part_num < 0)
		return -ESLOTNUM;

	if (!rawdata && ll_intf->priority.get(part_num) <= 0) {
		rsu_log(RSU_ERR, "Trying to verify a slot not in use\n");
		return -EERASE;
	}

	if (!callback)
		return -EARGS;

	offset = 0;
	done = 0;

	if (rsu_misc_image_block_init(&state))
		return -ECMP;

	while (!done) {
		cnt = 0;
		while (cnt < IMAGE_BLOCK_SZ) {
			c = callback(buf + cnt, IMAGE_BLOCK_SZ - cnt);
			if (c == 0) {
				done = 1;
				break;
			} else if (c < 0) {
				return -ECALLBACK;
			}

			cnt += c;
		}

		if (cnt == 0)
			break;

		if (ll_intf->data.read(part_num, offset, cnt, vbuf))
			return -ELOWLEVEL;

		if (!rawdata) {
			if (rsu_misc_image_block_process(&state, buf, vbuf,
							 &info))
				return -ECMP;

			offset += cnt;
			continue;
		}

		for (x = 0; x < cnt; x++)
			if (vbuf[x] != buf[x]) {
				rsu_log(RSU_ERR,
					"Expect %02X, got %02X @ 0x%08X\n",
					buf[x], vbuf[x], offset + x);
				return -ECMP;
			}

		offset += cnt;
	}

	return 0;
}

/*
 * rsu_log() - display rsu log message
 * @level: log level
 * @format: log message format
 */
void rsu_log(const enum rsu_log_level level, const char *format, ...)
{
	va_list args;
	char *log_level_env;
	int log_level;
	char printbuffer[LOG_BUF_SIZE];

	log_level_env = env_get("rsu_log_level");

	if (log_level_env) {
		log_level = (int)simple_strtol(log_level_env, NULL, 0);

		if (level >= log_level)
			return;

		va_start(args, format);
		vscnprintf(printbuffer, sizeof(printbuffer), format, args);
		va_end(args);
		puts(printbuffer);
	}
}
