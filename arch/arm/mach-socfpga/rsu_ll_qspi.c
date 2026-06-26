// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#include <env.h>
#include <limits.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <stdio.h>
#include <asm/arch/mailbox_s10.h>
#if CONFIG_IS_ENABLED(DM_SPI_FLASH)
#include <dm/device.h>
#endif
#include <asm/arch/rsu_flash_if.h>
#include <asm/arch/rsu.h>
#include <asm/arch/rsu_misc.h>
#include <asm/arch/smc_api.h>
#include <linux/errno.h>
#include <linux/intel-smc.h>
#include <linux/kconfig.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/string.h>
#include <u-boot/zlib.h>

#define SPT_MAGIC_NUMBER	0x57713427
#define SPT_FLAG_RESERVED	1
#define SPT_FLAG_READONLY	2
#define SPT_OFFSET_MBOX		4

#define CPB_MAGIC_NUMBER	0x57789609
#define CPB_HEADER_SIZE		24

#define ERASED_ENTRY		((u64)-1)
#define SPENT_ENTRY		((u64)0)

#define SPT_VERSION		0
#define LIBRSU_VER		0

#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_STRATIX10)
#define DCMF_SIZE		0x040000
#define DCMF0_VERSION_OFFSET	0x000420
#define DCMF1_VERSION_OFFSET	0x040420
#define DCMF2_VERSION_OFFSET	0x080420
#define DCMF3_VERSION_OFFSET	0x0C0420
#define DCIO_MAX_RETRY_OFFSET   0x10018C
#else
#define DCMF_SIZE		0x080000
#define DCMF0_VERSION_OFFSET	0x000420
#define DCMF1_VERSION_OFFSET	0x080420
#define DCMF2_VERSION_OFFSET	0x100420
#define DCMF3_VERSION_OFFSET	0x180420
#define DCIO_MAX_RETRY_OFFSET   0x20018C
#endif

/* maximum supported QSPI from programmer */
#define QSPI_MAX_DEVICE		4
#define SPT_MAX_PARTITIONS	127
#define MIN_QSPI_ERASE_SIZE	4096

#define CPB_SIZE		SZ_4K
#define SPT_SIZE		SZ_4K
#define CPB_IMAGE_PTR_OFFSET	32
#define CPB_IMAGE_PTR_NSLOTS	508

#define SPT_CHECKSUM_OFFSET	0x0C

#define FACTORY_IMAGE_NAME	"FACTORY_IMAGE"

/**
 * struct sub_partition_table_partition - SPT partition structure
 * @name: sub-partition name
 * @offset: sub-partition start offset
 * @length: sub-partition length
 * @flags: sub-partition flags
 */
struct sub_partition_table_partition {
	char name[16];
	u64 offset;
	u32 length;
	u32 flags;
};

/**
 * struct sub_partition_table - sub partition table structure
 * @magic_number: the magic number
 * @version: version number
 * @partitions: number of entries
 * @revd: reserved
 * @sub_partition_table_partition.partition: SPT partition array
 */
struct sub_partition_table {
	u32 magic_number;
	u32 version;
	u32 partitions;
	u32 checksum;
	u32 rsvd[4];
	struct sub_partition_table_partition partition[SPT_MAX_PARTITIONS];
};

/**
 * union cmf_pointer_block - CMF pointer block
 * @header.magic_number: CMF pointer block magic number
 * @header.header_size: size of CMF pointer block header
 * @header.cpb_size: size of CMF pointer block
 * @header.cpb_reserved: reserved
 * @header.image_ptr_offset: offset of image pointers
 * @header.image_ptr_slots: number of image pointer slots
 * @data: image pointer slot array
 */
union cmf_pointer_block {
	struct {
		u32 magic_number;
		u32 header_size;
		u32 cpb_size;
		u32 cpb_reserved;
		u32 image_ptr_offset;
		u32 image_ptr_slots;
	} header;
	char data[4 * 1024];
};

/* retrieve multiple qspi info from mailbox */
struct flash_info {
	u32 size;
	u32 erasesize;
};

/**
 * struct rsu_qspi_priv - per-session QSPI RSU backend state
 *
 * Allocated in rsu_ll_qspi_init(), released in ll_exit(); SPT/CPB cached
 * copies are memset before free to shorten sensitive data lifetime in RAM.
 */
struct rsu_qspi_priv {
	union cmf_pointer_block cpb;
	struct sub_partition_table spt;
	u64 *cpb_slots;
#if CONFIG_IS_ENABLED(DM_SPI_FLASH)
	struct udevice **flashlist;
	struct udevice *flash;
#else
	struct spi_flash **flashlist;
	struct spi_flash *flash;
#endif
	u32 spt0_offset;
	u32 spt1_offset;
	int cpb0_part;
	int cpb1_part;
	int num_flash;
	bool cpb_corrupted;
	bool cpb_fixed;
	bool spt_corrupted;
};

static struct rsu_qspi_priv *qspi_ctx;

#define P (qspi_ctx)

static int load_cpb(void);
static int check_spt(void);

/**
 * get_part_offset() - get a selected partition offset
 * @part_num: the selected partition number
 * @offset: the partition offset
 *
 * Return: 0 on success, or -ve on error
 */
static int get_part_offset(int part_num, u64 *offset)
{
	if (part_num < 0 || part_num >= P->spt.partitions)
		return -EINVAL;

	*offset = P->spt.partition[part_num].offset;

	return 0;
}

/*
 * @brief Calculates the current flash offset based on the given offset.
 *
 * This function calculates the current flash offset and the current flash index
 * based on the given offset. It iterates through the flashlist array and checks
 * if the current offset is greater than the size of each flash. If it is, it
 * subtracts the size of the flash from the current offset and continues to the
 * next flash. If the current offset is not greater than the size of the flash,
 * it sets the current flash index and breaks out of the loop.
 *
 * @param offset The offset to calculate the current flash offset from.
 * @param current_offset Pointer to store the calculated current offset.
 * @param current_flash Pointer to store the index of the current flash.
 */

static int get_current_flash_offset(u64 offset, u32 *current_offset,
				    int *current_flash)
{
	u64 relative_offset = offset;

	if (!current_offset || !current_flash)
		return -EINVAL;

	for (int j = 0; j < P->num_flash && j < QSPI_MAX_DEVICE; j++) {
		u32 sz = rsu_mtd_size(P->flashlist[j]);

		if (!sz)
			return -EINVAL;
		if (relative_offset >= sz) {
			relative_offset -= sz;
			continue;
		} else {
			/* relative_offset < sz here; u32 keeps the assignment loss-free. */
			*current_flash = j;
			*current_offset = (u32)relative_offset;
			return 0;
		}
	}

	return -EINVAL;
}

/**
 * read_dev() - read data from flash
 * @offset: the offset which read from flash
 * @buf: buffer for read data
 * @len: the size of data which read from flash
 *
 * Return: 0 on success, or -ve for error
 */
static int read_dev(u64 offset, void *buf, int len)
{
	int ret, current_flash;
	u32 count, current_len, current_offset;

	if (len < 0)
		return -EINVAL;
	/*
	 * Preserve the long-standing no-op convention: callers (notably
	 * read_part) may pass len == 0 at offset == flash/partition end,
	 * which the per-flash boundary check would otherwise reject.
	 */
	if (len == 0)
		return 0;

	count = 0;

	ret = get_current_flash_offset(offset, &current_offset, &current_flash);
	if (ret)
		return ret;

	for (int i = current_flash; i < P->num_flash && i < QSPI_MAX_DEVICE;
	     i++) {
		u32 sz = rsu_mtd_size(P->flashlist[i]);

		/* break if total data length is done */
		if (count == (u32)len)
			break;

		/*
		 * Re-validate per iteration: only the first flash was checked
		 * by get_current_flash_offset(), and a later device may report
		 * sz == 0.
		 */
		if (!sz || current_offset > sz) {
			rsu_log(RSU_ERR,
				"%s: flash %d invalid size %u or offset %u\n",
				__func__, i, sz, current_offset);
			return -EINVAL;
		}

		/* Compute in u64 so (len + current_offset) cannot wrap past 4 GiB. */
		if ((u64)(u32)len + current_offset - count > sz)
			current_len = sz - current_offset;
		else
			current_len = (u32)len - count;

		ret = rsu_mtd_read(P->flashlist[i], current_offset,
				   (int)current_len, buf);
		if (ret) {
			rsu_log(RSU_ERR, "read flash error=%i\n", ret);
			return ret;
		}

		buf = (char *)buf + current_len;

		/* reset the offset to new flash */
		current_offset = 0;
		count += current_len;
	}

	return 0;
}

/**
 * write_dev() - write data to flash
 * @offset: the offset which data will written to
 * @buf: the written data
 * @len: the size of data which write to flash
 *
 * Return: 0 on success, or -ve for error
 */
static int write_dev(u64 offset, void *buf, int len)
{
	int ret, current_flash;
	u32 count, current_len, current_offset;

	if (len < 0)
		return -EINVAL;
	/* See read_dev() for why len == 0 is treated as a no-op. */
	if (len == 0)
		return 0;

	count = 0;

	ret = get_current_flash_offset(offset, &current_offset, &current_flash);
	if (ret)
		return ret;

	for (int i = current_flash; i < P->num_flash && i < QSPI_MAX_DEVICE;
	     i++) {
		u32 sz = rsu_mtd_size(P->flashlist[i]);

		/* break if total data length is done */
		if (count == (u32)len)
			break;

		/* See read_dev() for why these guards are required. */
		if (!sz || current_offset > sz) {
			rsu_log(RSU_ERR,
				"%s: flash %d invalid size %u or offset %u\n",
				__func__, i, sz, current_offset);
			return -EINVAL;
		}

		if ((u64)(u32)len + current_offset - count > sz)
			current_len = sz - current_offset;
		else
			current_len = (u32)len - count;

		ret = rsu_mtd_write(P->flashlist[i], current_offset,
				    (int)current_len, buf);
		if (ret) {
			rsu_log(RSU_ERR, "write flash error=%i\n", ret);
			return ret;
		}

		buf = (char *)buf + current_len;

		/* reset the offset to new flash */
		current_offset = 0;
		count += current_len;
	}

	return 0;
}

/**
 * erase_dev() - erase data at flash
 * @offset: the offset from which data will be erased
 * @len: the size of data to be erased
 *
 * Return: 0 on success, or -ve for error
 */
static int erase_dev(u64 offset, int len)
{
	int ret, current_flash;
	u32 count, current_len, current_offset;

	if (len < 0)
		return -EINVAL;
	/* See read_dev() for why len == 0 is treated as a no-op. */
	if (len == 0)
		return 0;

	count = 0;

	ret = get_current_flash_offset(offset, &current_offset, &current_flash);
	if (ret)
		return ret;

	for (int i = current_flash; i < P->num_flash && i < QSPI_MAX_DEVICE;
	     i++) {
		u32 sz = rsu_mtd_size(P->flashlist[i]);

		if (count >= (u32)len)
			break;

		/* See read_dev() for why these guards are required. */
		if (!sz || current_offset > sz) {
			rsu_log(RSU_ERR,
				"%s: flash %d invalid size %u or offset %u\n",
				__func__, i, sz, current_offset);
			return -EINVAL;
		}

		if ((u64)(u32)len + current_offset - count > sz)
			current_len = sz - current_offset;
		else
			current_len = (u32)len - count;

		ret = rsu_mtd_erase(P->flashlist[i], current_offset,
				    (int)current_len);
		if (ret) {
			rsu_log(RSU_ERR, "erase flash error=%i\n", ret);
			return ret;
		}

		current_offset = 0;
		count += current_len;
	}

	return 0;
}

/**
 * read_part() - read a selected partition data
 * @part_num: the selected partition number
 * @offset: the offset from which data will be read
 * @buf: buffer contains the read data
 * @len: the size of data to be read
 *
 * Return: 0 on success, or -ve for error
 */
static int read_part(int part_num, u64 offset, void *buf, int len)
{
	u64 part_offset;
	int ret;

	ret = get_part_offset(part_num, &part_offset);
	if (ret)
		return ret;

	if (len < 0)
		return -EINVAL;

	if (offset > P->spt.partition[part_num].length ||
	    (u64)len > P->spt.partition[part_num].length - offset)
		return -EINVAL;

	return read_dev(part_offset + offset, buf, len);
}

/**
 * write_part() - write a selected partition data
 * @part_num: the selected partition number
 * @offset: the offset to which data will be written
 * @buf: data to be written to
 * @len: the size of data to be written
 *
 * Return: 0 on success, or -ve for error
 */
static int write_part(int part_num, u64 offset, void *buf, int len)
{
	u64 part_offset;
	int ret;

	ret = get_part_offset(part_num, &part_offset);
	if (ret)
		return ret;

	if (len < 0)
		return -EINVAL;

	if (offset > P->spt.partition[part_num].length ||
	    (u64)len > P->spt.partition[part_num].length - offset)
		return -EINVAL;

	return write_dev(part_offset + offset, buf, len);
}

/**
 * erase_part() - erase a selected partition data
 * @part_num: the selected partition number
 *
 * Return: 0 on success, or -ve for error
 */
static int erase_part(int part_num)
{
	u64 part_offset;
	int ret;

	ret = get_part_offset(part_num, &part_offset);
	if (ret)
		return ret;

	return erase_dev(part_offset, P->spt.partition[part_num].length);
}

/**
 * save_spt_to_address() - save spt to the address
 * @address: the address which spt is saved to
 *
 * Return: 0 for successful operation, or -ve on error
 */
static int save_spt_to_address(u64 address)
{
	int ret;
	char *spt_data_dst = (char *)address;
	char *spt_data_src;
	u32 calc_crc;

	if (!spt_data_dst) {
		rsu_log(RSU_ERR, "failed due to invalid address\n");
		return -EINVAL;
	}

	spt_data_src = (char *)malloc(SPT_SIZE);
	if (!spt_data_src) {
		rsu_log(RSU_ERR, "failed to allocate spt_data_src\n");
		return -ENOMEM;
	}

	ret = read_dev(P->spt0_offset, spt_data_src, SPT_SIZE);
	if (ret) {
		rsu_log(RSU_ERR, "failed to read SPT data\n");
		free(spt_data_src);
		return ret;
	}

	calc_crc = crc32(0, (void *)spt_data_src, SPT_SIZE);
	rsu_log(RSU_DEBUG, "%s - calc_crc is 0x%x\n", __func__, calc_crc);
	memcpy(spt_data_dst, spt_data_src, SPT_SIZE);
	memcpy(spt_data_dst + SPT_SIZE, &calc_crc, sizeof(calc_crc));
	rsu_log(RSU_INFO, "%ld bytes SPT data saved\n",
		SPT_SIZE + sizeof(calc_crc));
	env_set_hex("filesize", SPT_SIZE + sizeof(calc_crc));

	free(spt_data_src);
	return ret;
}

/**
 * corrupted_spt() - check if spt is corrupted
 *
 * Return: 1 for the corrupted spt , or 0 for not
 */
static int corrupted_spt(void)
{
	return P->spt_corrupted;
}

/**
 * writeback_spt() - write back SPT
 *
 * Return: 0 on success, or -ve on error
 */
static int writeback_spt(void)
{
	int x;
	int updates = 0;
	char *spt_data;
	u32 calc_crc;

	for (x = 0; x < P->spt.partitions; x++) {
		if (strcmp(P->spt.partition[x].name, "SPT0") &&
		    strcmp(P->spt.partition[x].name, "SPT1"))
			continue;

		if (erase_part(x)) {
			rsu_log(RSU_ERR, "failed to erase SPTx");
			return -EIO;
		}

		if (P->spt.version > SPT_VERSION &&
		    rsu_misc_spt_checksum_enabled()) {
			rsu_log(RSU_DEBUG, "update SPT checksum...\n");
			spt_data = (char *)malloc(SPT_SIZE);
			if (!spt_data) {
				rsu_log(RSU_ERR,
					"failed to allocate spt_data\n");
				return -ENOMEM;
			}

			P->spt.checksum = (u32)0xFFFFFFFF;
			if (write_part(x, SPT_CHECKSUM_OFFSET,
				       &P->spt.checksum,
				       sizeof(P->spt.checksum))) {
				rsu_log(RSU_ERR,
					"failed to write SPTx table");
				free(spt_data);
				return -EINVAL;
			}

			memcpy(spt_data, &P->spt, SPT_SIZE);
			memset(spt_data + SPT_CHECKSUM_OFFSET, 0,
			       sizeof(P->spt.checksum));

			swap_bits(spt_data, SPT_SIZE);
			calc_crc = crc32(0, (void *)spt_data, SPT_SIZE);
			P->spt.checksum = be32_to_cpu(calc_crc);
			swap_bits(spt_data, SPT_SIZE);
			free(spt_data);

			if (write_part(x, SPT_CHECKSUM_OFFSET,
				       &P->spt.checksum,
				       sizeof(P->spt.checksum))) {
				rsu_log(RSU_ERR,
					"failed to write SPTx table");
				return -EINVAL;
			}
		}

		P->spt.magic_number = (u32)0xFFFFFFFF;
		if (write_part(x, 0, &P->spt, sizeof(P->spt))) {
			rsu_log(RSU_ERR, "failed to write SPTx table");
			return -EIO;
		}

		P->spt.magic_number = (u32)SPT_MAGIC_NUMBER;
		if (write_part(x, 0, &P->spt.magic_number,
			       sizeof(P->spt.magic_number))) {
			rsu_log(RSU_ERR, "failed to write SPTx magic #");
			return -EIO;
		}

		updates++;
	}

	if (updates != 2) {
		rsu_log(RSU_ERR, "didn't find two SPTs");
		return -ENOENT;
	}

	return 0;
}

/**
 * restore_spt_from_address() - restore the spt from an address
 * @address: the address which spt is restored from
 *
 * Return: 0 for successful operation, or -ve on error
 */
static int restore_spt_from_address(u64 address)
{
	int ret;
	u32 calc_crc;
	u32 crc_from_saved;
	u32 magic_number;
	char *spt_data = (char *)address;

	if (!spt_data) {
		rsu_log(RSU_ERR, "failed due to invalid address\n");
		return -EINVAL;
	}

	calc_crc = crc32(0, (void *)spt_data, SPT_SIZE);
	rsu_log(RSU_DEBUG, "%s - calc_crc is 0x%x\n", __func__, calc_crc);
	memcpy(&crc_from_saved, spt_data + SPT_SIZE, sizeof(crc_from_saved));
	rsu_log(RSU_DEBUG, "%s - crc_from_saved is 0x%x\n", __func__,
		crc_from_saved);

	if (calc_crc != crc_from_saved) {
		rsu_log(RSU_ERR, "saved data is corrupted\n");
		return -EINVAL;
	}

	/*
	 * check the magic number to prevent user from accidentally
	 * restoring CPB
	 */
	memcpy(&magic_number, spt_data, sizeof(magic_number));
	if (magic_number != SPT_MAGIC_NUMBER) {
		rsu_log(RSU_ERR, "failure due to mismatch magic number\n");
		return -EINVAL;
	}

	memcpy(&P->spt, spt_data, SPT_SIZE);

	/*
	 * CRC+magic only prove self-consistency of the supplied image; a
	 * crafted SPT could still carry an out-of-range partition count or
	 * overlapping regions. Re-run the full validator before committing.
	 */
	ret = check_spt();
	if (ret) {
		rsu_log(RSU_ERR, "restored SPT failed validation\n");
		P->spt_corrupted = true;
		return ret;
	}

	ret = writeback_spt();
	if (ret) {
		rsu_log(RSU_ERR, "failed to write back spt\n");
		return ret;
	}

	P->spt_corrupted = false;

	/* try to reload CPB, as we have a new SPT */
	P->cpb_corrupted = false;
	if (load_cpb() && !P->cpb_corrupted)
		rsu_log(RSU_ERR, "Bad CPB\n");

	return 0;
}

/**
 * check_spt() - check if SPT is valid
 *
 * Return: 0 for valid SPT, or -ve on error
 */
static int check_spt(void)
{
	int x;
	int y;
	int max_len = sizeof(P->spt.partition[0].name);
	int spt0_found = 0;
	int spt1_found = 0;
	int cpb0_found = 0;
	int cpb1_found = 0;

	u32 calc_crc;
	char *spt_data;

	/*
	 * Make sure the SPT names are '\0' terminated. Truncate last byte
	 * if the name uses all available bytes.  Perform validity check on
	 * entries.
	 */

	rsu_log(RSU_DEBUG,
		"MAX length of a name = %i bytes\n", max_len - 1);

	if (P->spt.version > SPT_VERSION &&
	    rsu_misc_spt_checksum_enabled()) {
		rsu_log(RSU_DEBUG, "check SPT checksum...\n");

		spt_data = (char *)malloc(SPT_SIZE);
		if (!spt_data) {
			rsu_log(RSU_ERR, "failed to allocate spt_data\n");
			return -ENOMEM;
		}

		memcpy(spt_data, &P->spt, SPT_SIZE);
		memset(spt_data + SPT_CHECKSUM_OFFSET, 0,
		       sizeof(P->spt.checksum));

		swap_bits(spt_data, SPT_SIZE);
		calc_crc = crc32(0, (void *)spt_data, SPT_SIZE);
		if (be32_to_cpu(P->spt.checksum) != calc_crc) {
			rsu_log(RSU_ERR, "Error, bad SPT checksum\n");
			free(spt_data);
			return -EINVAL;
		}
		swap_bits(spt_data, SPT_SIZE);
		free(spt_data);
	}

	if (P->spt.partitions > SPT_MAX_PARTITIONS) {
		rsu_log(RSU_ERR, "bigger than max partition\n");
		return -EINVAL;
	}

	for (x = 0; x < P->spt.partitions; x++) {
		u64 s_start;
		u64 s_len;
		u64 s_end;

		if (strnlen(P->spt.partition[x].name, max_len) >= max_len)
			P->spt.partition[x].name[max_len - 1] = '\0';

		s_start = P->spt.partition[x].offset;
		s_len = P->spt.partition[x].length;

		/* Zero length would underflow the inclusive end below. */
		if (s_len == 0) {
			rsu_log(RSU_ERR,
				"SPT entry %d (%s): zero-length partition\n",
				x, P->spt.partition[x].name);
			return -EINVAL;
		}

		/* Reject offset+length wraparound; otherwise overlap test passes spuriously. */
		if (s_len > U64_MAX - s_start) {
			rsu_log(RSU_ERR,
				"SPT entry %d: offset+length overflows u64\n",
				x);
			return -EINVAL;
		}
		s_end = s_start + s_len;

		rsu_log(RSU_DEBUG, "RSU %-16s %016llX - %016llX (%X)\n",
			P->spt.partition[x].name, s_start, s_end - 1,
			P->spt.partition[x].flags);

		for (y = 0; y < P->spt.partitions; y++) {
			if (x == y)
				continue;

			/*
			 * don't allow the same partition name to appear
			 * more than once
			 */
			if (!(strcmp(P->spt.partition[x].name,
				     P->spt.partition[y].name))) {
				rsu_log(RSU_ERR, "partition name ");
				rsu_log(RSU_ERR, "appears more than once\n");
				return -EINVAL;
			}

			u64 d_start = P->spt.partition[y].offset;
			u64 d_len = P->spt.partition[y].length;
			u64 d_end;

			if (d_len > U64_MAX - d_start) {
				rsu_log(RSU_ERR,
					"SPT entry %d: offset+length overflows u64\n",
					y);
				return -EINVAL;
			}
			d_end = d_start + d_len;

			if (s_start < d_end && s_end > d_start) {
				rsu_log(RSU_ERR, "partition overlap\n");
				return -EINVAL;
			}
		}

		if (strcmp(P->spt.partition[x].name, "SPT0") == 0)
			spt0_found = 1;
		else if (strcmp(P->spt.partition[x].name, "SPT1") == 0)
			spt1_found = 1;
		else if (strcmp(P->spt.partition[x].name, "CPB0") == 0)
			cpb0_found = 1;
		else if (strcmp(P->spt.partition[x].name, "CPB1") == 0)
			cpb1_found = 1;
	}

	if (!spt0_found || !spt1_found || !cpb0_found || !cpb1_found) {
		rsu_log(RSU_ERR, "Missing a critical entry in the SPT\n");
		return -ENOENT;
	}

	return 0;
}

/**
 * check_both_spt() - check if both SPTs are same
 *
 * Return: 0 for the identical SPTs, or -ve on error
 */
static int check_both_spt(void)
{
	int ret;
	char *spt0_data;
	char *spt1_data;

	spt0_data = (char *)malloc(SPT_SIZE);
	if (!spt0_data) {
		rsu_log(RSU_ERR, "failed to allocate spt0_data\n");
		return -ENOMEM;
	}

	spt1_data = (char *)malloc(SPT_SIZE);
	if (!spt1_data) {
		rsu_log(RSU_ERR, "failed to allocate spt1_data\n");
		free(spt0_data);
		return -ENOMEM;
	}

	ret = read_dev(P->spt0_offset, spt0_data, SPT_SIZE);
	if (ret) {
		rsu_log(RSU_ERR, "failed to read spt0_data\n");
		goto ops_error;
	}

	ret = read_dev(P->spt1_offset, spt1_data, SPT_SIZE);
	if (ret) {
		rsu_log(RSU_ERR, "failed to read spt1_data\n");
		goto ops_error;
	}

	ret = memcmp(spt0_data, spt1_data, SPT_SIZE);

ops_error:
	free(spt1_data);
	free(spt0_data);
	return ret;
}

/**
 * load_spt() - retrieve SPT from flash
 *
 * Return: 0 on success, or -ve on error
 */
static int load_spt(void)
{
	int spt0_good = 0;
	int spt1_good = 0;
	int spt_size = P->spt1_offset - P->spt0_offset;

	rsu_log(RSU_DEBUG, "reading SPT1\n");
	if (read_dev(P->spt1_offset, &P->spt, sizeof(P->spt)) == 0 &&
	    P->spt.magic_number == SPT_MAGIC_NUMBER) {
		if (check_spt() == 0)
			spt1_good = 1;
		else
			rsu_log(RSU_ERR, "SPT1 validity check failed\n");
	} else {
		rsu_log(RSU_ERR, "Bad SPT1 magic number 0x%08X\n",
			P->spt.magic_number);
	}

	rsu_log(RSU_DEBUG, "reading SPT0\n");
	if (read_dev(P->spt0_offset, &P->spt, sizeof(P->spt)) == 0 &&
	    P->spt.magic_number == SPT_MAGIC_NUMBER) {
		if (check_spt() == 0)
			spt0_good = 1;
		else
			rsu_log(RSU_ERR, "SPT0 validity check failed\n");
	} else {
		rsu_log(RSU_ERR, "Bad SPT0 magic number 0x%08X\n",
			P->spt.magic_number);
	}

	if (spt0_good && spt1_good) {
		if (check_both_spt()) {
			rsu_log(RSU_ERR, "unmatched SPT0/1 data");
			P->spt_corrupted = true;
			return -EINVAL;
		}
		rsu_log(RSU_INFO, "SPTs are GOOD!!!\n");
		return 0;
	}

	if (spt0_good) {
		rsu_log(RSU_WARNING, "warning: Restoring SPT1\n");
		if (erase_dev(P->spt1_offset, spt_size)) {
			rsu_log(RSU_ERR, "Erase SPT1 region failed\n");
			return -EIO;
		}

		P->spt.magic_number = (u32)0xFFFFFFFF;
		if (write_dev(P->spt1_offset, &P->spt, sizeof(P->spt))) {
			rsu_log(RSU_ERR, "Unable to write SPT1 table\n");
			return -EIO;
		}

		P->spt.magic_number = (u32)SPT_MAGIC_NUMBER;
		if (write_dev(P->spt1_offset, &P->spt.magic_number,
			      sizeof(P->spt.magic_number))) {
			rsu_log(RSU_ERR, "Unable to wr SPT1 magic #\n");
			return -EIO;
		}

		return 0;
	}

	if (spt1_good) {
		if (read_dev(P->spt1_offset, &P->spt, sizeof(P->spt)) ||
		    P->spt.magic_number != SPT_MAGIC_NUMBER || check_spt()) {
			rsu_log(RSU_ERR, "Failed to load SPT1\n");
			return -EUCLEAN;
		}

		rsu_log(RSU_WARNING, "Restoring SPT0");

		if (erase_dev(P->spt0_offset, spt_size)) {
			rsu_log(RSU_ERR, "Erase SPT0 region failed\n");
			return -EIO;
		}

		P->spt.magic_number = (u32)0xFFFFFFFF;
		if (write_dev(P->spt0_offset, &P->spt, sizeof(P->spt))) {
			rsu_log(RSU_ERR, "Unable to write SPT0 table\n");
			return -EIO;
		}

		P->spt.magic_number = (u32)SPT_MAGIC_NUMBER;
		if (write_dev(P->spt0_offset, &P->spt.magic_number,
			      sizeof(P->spt.magic_number))) {
			rsu_log(RSU_ERR, "Unable to wr SPT0 magic #\n");
			return -EIO;
		}

		return 0;
	}

	P->spt_corrupted = true;
	rsu_log(RSU_ERR, "no valid SPT0 and SPT1 found\n");
	return -EUCLEAN;
}

/**
 * cpb_header_access_ok() - validate CPB header fields used to index
 * cpb_slots[].
 *
 * Return: 0 if the header is safe to use, -EINVAL otherwise.
 */
static int cpb_header_access_ok(void)
{
	u32 ip_off = P->cpb.header.image_ptr_offset;
	u32 ip_slots = P->cpb.header.image_ptr_slots;
	u32 max_by_buf;

	if (P->cpb.header.header_size > CPB_HEADER_SIZE) {
		rsu_log(RSU_WARNING,
			"CPB header is larger than expected\n");
		return -EINVAL;
	}
	if (ip_off >= CPB_SIZE) {
		rsu_log(RSU_ERR, "CPB image_ptr_offset out of range\n");
		return -EINVAL;
	}
	/* cpb_slots[] is dereferenced as u64; reject misaligned image_ptr_offset. */
	if (ip_off % sizeof(u64)) {
		rsu_log(RSU_ERR, "CPB image_ptr_offset not 8-byte aligned\n");
		return -EINVAL;
	}
	max_by_buf = (CPB_SIZE - ip_off) / sizeof(u64);
	/* Reject ip_slots == 0 too: zero-iteration loops would silently succeed. */
	if (!ip_slots || !max_by_buf || ip_slots > max_by_buf ||
	    ip_slots > CPB_IMAGE_PTR_NSLOTS) {
		rsu_log(RSU_ERR, "CPB image_ptr_slots out of range\n");
		return -EINVAL;
	}
	return 0;
}

/**
 * check_cpb() - check if CPB is valid
 *
 * Return: 0 for the valid CPB, or -ve on error
 */
static int check_cpb(void)
{
	int x, y;
	int ret;

	ret = cpb_header_access_ok();
	if (ret)
		return ret;

	for (x = 0; x < P->cpb.header.image_ptr_slots; x++) {
		if (P->cpb_slots[x] == ERASED_ENTRY ||
		    P->cpb_slots[x] == SPENT_ENTRY)
			continue;

		for (y = 0; y < P->spt.partitions; y++) {
			if (P->cpb_slots[x] == P->spt.partition[y].offset) {
				rsu_log(RSU_DEBUG, "cpb_slots[%i] = %s\n",
					x, P->spt.partition[y].name);
				break;
			}
		}

		if (y >= P->spt.partitions) {
			rsu_log(RSU_ERR, "CPB is not included in SPT\n");
			rsu_log(RSU_DEBUG, "cpb_slots[%i] = %016llX ???",
				x, P->cpb_slots[x]);
			return -EINVAL;
		}

		if (P->spt.partition[y].flags & SPT_FLAG_RESERVED) {
			rsu_log(RSU_ERR, "CPB is included in SPT ");
			rsu_log(RSU_ERR, "but it is reserved\n");
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * check_both_cpb() - check if both CPBs are same
 *
 * Return: 0 for the identical CPBs, or -ve on error
 */
static int check_both_cpb(void)
{
	int ret;
	char *cpb0_data;
	char *cpb1_data;

	cpb0_data = (char *)malloc(CPB_SIZE);
	if (!cpb0_data) {
		rsu_log(RSU_ERR, "failed to allocate cpb0_data\n");
		return -ENOMEM;
	}

	cpb1_data = (char *)malloc(CPB_SIZE);
	if (!cpb1_data) {
		rsu_log(RSU_ERR, "failed to allocate cpb1_data\n");
		free(cpb0_data);
		return -ENOMEM;
	}

	ret = read_part(P->cpb0_part, 0, cpb0_data, CPB_SIZE);
	if (ret) {
		rsu_log(RSU_ERR, "failed to read cpb0_data\n");
		goto ops_error;
	}

	ret = read_part(P->cpb1_part, 0, cpb1_data, CPB_SIZE);
	if (ret) {
		rsu_log(RSU_ERR, "failed to read cpb1_data\n");
		goto ops_error;
	}

	ret = memcmp(cpb0_data, cpb1_data, CPB_SIZE);

ops_error:
	free(cpb1_data);
	free(cpb0_data);
	return ret;
}

/**
 * save_cpb_to_address() - save cpb to the address
 * @address: the address which cpb is saved to
 *
 * Return: 0 for successful operation, or -ve on error
 */
static int save_cpb_to_address(u64 address)
{
	int ret;
	char *cpb_data_dst = (char *)address;
	char *cpb_data_src;
	u32 calc_crc;

	if (!cpb_data_dst) {
		rsu_log(RSU_ERR, "failed due to invalid address");
		return -EINVAL;
	}

	cpb_data_src = (char *)malloc(CPB_SIZE);
	if (!cpb_data_src) {
		rsu_log(RSU_ERR, "failed to allocate cpb_data_src\n");
		return -ENOMEM;
	}

	ret = read_part(P->cpb0_part, 0, cpb_data_src, CPB_SIZE);
	if (ret) {
		rsu_log(RSU_ERR, "failed to read CPB data\n");
		free(cpb_data_src);
		return ret;
	}

	calc_crc = crc32(0, (void *)cpb_data_src, CPB_SIZE);
	rsu_log(RSU_DEBUG, "%s - calc_crc is 0x%x\n", __func__, calc_crc);
	memcpy(cpb_data_dst, cpb_data_src, CPB_SIZE);
	memcpy(cpb_data_dst + CPB_SIZE, &calc_crc, sizeof(calc_crc));
	rsu_log(RSU_INFO, "%ld bytes CPB data saved\n",
		CPB_SIZE + sizeof(calc_crc));
	env_set_hex("filesize", CPB_SIZE + sizeof(calc_crc));

	free(cpb_data_src);
	return ret;
}

/**
 * corrupted_cpb() - check if cpb is corrupted
 *
 * Return: 1 for the corrupted cpb , or 0 for not
 */
static int corrupted_cpb(void)
{
	return P->cpb_corrupted;
}

/**
 * load_cpb() - retrieve CPB from flash
 *
 * Return: 0 on success, or -ve on error
 */
static int load_cpb(void)
{
	int x;
	int cpb0_good = 0;
	int cpb1_good = 0;
	struct rsu_status_info status_info;
	int cpb0_corrupted = 0;

	if (mbox_rsu_status((u32 *)&status_info,
			    sizeof(status_info) / 4)) {
		rsu_log(RSU_ERR, "FW doesn't support RSU\n");
		return -EINVAL;
	}

	if (!P->cpb_fixed && status_info.state == STATE_CPB0_CPB1_CORRUPTED) {
		rsu_log(RSU_ERR, "FW detects both CPBs corrupted\n");
		P->cpb_corrupted = true;
		return -EINVAL;
	}

	if (!P->cpb_fixed && status_info.state == STATE_CPB0_CORRUPTED) {
		rsu_log(RSU_ERR,
			"FW detects corrupted CPB0 but CPB1 is fine\n");
		cpb0_corrupted = 1;
	}

	for (x = 0; x < P->spt.partitions; x++) {
		if (strcmp(P->spt.partition[x].name, "CPB0") == 0)
			P->cpb0_part = x;
		else if (strcmp(P->spt.partition[x].name, "CPB1") == 0)
			P->cpb1_part = x;

		if (P->cpb0_part >= 0 && P->cpb1_part >= 0)
			break;
	}

	if (P->cpb0_part < 0 || P->cpb1_part < 0) {
		rsu_log(RSU_ERR, "Missing CPB0/1 partition\n");
		return -ENOENT;
	}

	rsu_log(RSU_DEBUG, "Reading CPB1\n");
	if (read_part(P->cpb1_part, 0, &P->cpb, sizeof(P->cpb)) == 0 &&
	    P->cpb.header.magic_number == CPB_MAGIC_NUMBER &&
	    cpb_header_access_ok() == 0) {
		P->cpb_slots = (u64 *)
			     &P->cpb.data[P->cpb.header.image_ptr_offset];
		if (check_cpb() == 0)
			cpb1_good = 1;
	} else {
		rsu_log(RSU_ERR, "Bad CPB1 is bad\n");
	}

	if (!cpb0_corrupted) {
		rsu_log(RSU_DEBUG, "Reading CPB0\n");
		if (read_part(P->cpb0_part, 0, &P->cpb, sizeof(P->cpb)) == 0 &&
		    P->cpb.header.magic_number == CPB_MAGIC_NUMBER &&
		    cpb_header_access_ok() == 0) {
			P->cpb_slots = (u64 *)
				     &P->cpb.data[P->cpb.header.image_ptr_offset];
			if (check_cpb() == 0)
				cpb0_good = 1;
		} else {
			rsu_log(RSU_ERR, "Bad CPB0 is bad\n");
		}
	}

	if (cpb0_good && cpb1_good) {
		if (check_both_cpb()) {
			rsu_log(RSU_ERR, "unmatched CPB0/1 data");
			P->cpb_corrupted = true;
			return -EINVAL;
		}
		rsu_log(RSU_INFO, "CPBs are GOOD!!!\n");
		P->cpb_slots = (u64 *)
			     &P->cpb.data[P->cpb.header.image_ptr_offset];
		return 0;
	}

	if (cpb0_good) {
		rsu_log(RSU_WARNING, "Restoring CPB1\n");
		if (erase_part(P->cpb1_part)) {
			rsu_log(RSU_ERR, "Failed erase CPB1\n");
			return -EIO;
		}

		P->cpb.header.magic_number = (u32)0xFFFFFFFF;
		if (write_part(P->cpb1_part, 0, &P->cpb, sizeof(P->cpb))) {
			rsu_log(RSU_ERR, "Unable to write CPB1 table\n");
			return -EIO;
		}

		P->cpb.header.magic_number = (u32)CPB_MAGIC_NUMBER;
		if (write_part(P->cpb1_part, 0, &P->cpb.header.magic_number,
			       sizeof(P->cpb.header.magic_number))) {
			rsu_log(RSU_ERR, "Unable to write CPB1 magic number\n");
			return -EIO;
		}

		P->cpb_slots = (u64 *)&P->cpb.data[P->cpb.header.image_ptr_offset];
		return 0;
	}

	if (cpb1_good) {
		if (read_part(P->cpb1_part, 0, &P->cpb, sizeof(P->cpb)) ||
		    P->cpb.header.magic_number != CPB_MAGIC_NUMBER) {
			rsu_log(RSU_ERR, "Unable to load CPB1\n");
			return -EUCLEAN;
		}

		rsu_log(RSU_WARNING, "Restoring CPB0\n");
		if (erase_part(P->cpb0_part)) {
			rsu_log(RSU_ERR, "Failed erase CPB0\n");
			return -EIO;
		}

		P->cpb.header.magic_number = (u32)0xFFFFFFFF;
		if (write_part(P->cpb0_part, 0, &P->cpb, sizeof(P->cpb))) {
			rsu_log(RSU_ERR, "Unable to write CPB0 table\n");
			return -EIO;
		}

		P->cpb.header.magic_number = (u32)CPB_MAGIC_NUMBER;
		if (write_part(P->cpb0_part, 0, &P->cpb.header.magic_number,
			       sizeof(P->cpb.header.magic_number))) {
			rsu_log(RSU_ERR, "Unable to write CPB0 magic number\n");
			return -EIO;
		}

		P->cpb_slots = (u64 *)&P->cpb.data[P->cpb.header.image_ptr_offset];
		return 0;
	}

	P->cpb_corrupted = true;
	rsu_log(RSU_ERR, "No valid CPB0 or CPB1 found\n");
	return -EUCLEAN;
}

/**
 * update_cpb() - update a CPB slot in flash (best-effort).
 * @slot: index into P->cpb_slots
 * @ptr:  new pointer value (NAND-style 1->0 transitions only)
 *
 * Writes CPB0 then CPB1; on a mid-write failure (one updated, the other
 * stale) returns -ve with no rollback. Callers MUST then call load_cpb(),
 * which reconciles the two flash copies; do not roll back P->cpb locally.
 *
 * Return: 0 on success, or -ve on error (caller MUST load_cpb()).
 */
static int update_cpb(int slot, u64 ptr)
{
	int x;
	int updates = 0;

	if (slot < 0 || slot >= P->cpb.header.image_ptr_slots)
		return -EINVAL;

	if ((P->cpb_slots[slot] & ptr) != ptr)
		return -EINVAL;

	P->cpb_slots[slot] = ptr;

	for (x = 0; x < P->spt.partitions; x++) {
		if (strcmp(P->spt.partition[x].name, "CPB0") &&
		    strcmp(P->spt.partition[x].name, "CPB1"))
			continue;

		if (write_part(x, 0, &P->cpb, sizeof(P->cpb)))
			return -EIO;

		updates++;
	}

	if (updates != 2) {
		rsu_log(RSU_ERR, "Did not find two CPBs\n");
		return -ENOENT;
	}

	return 0;
}

/**
 * writeback_cpb() - write CPB back to flash
 *
 * Return: 0 on success, or -ve on error
 */
static int writeback_cpb(void)
{
	int x;
	int updates = 0;

	for (x = 0; x < P->spt.partitions; x++) {
		if (strcmp(P->spt.partition[x].name, "CPB0") &&
		    strcmp(P->spt.partition[x].name, "CPB1"))
			continue;

		if (erase_part(x)) {
			rsu_log(RSU_ERR, "Unable to ease CPBx\n");
			return -EIO;
		}

		P->cpb.header.magic_number = (u32)0xFFFFFFFF;
		if (write_part(x, 0, &P->cpb, sizeof(P->cpb))) {
			rsu_log(RSU_ERR, "Unable to write CPBx table\n");
			return -EIO;
		}

		P->cpb.header.magic_number = (u32)CPB_MAGIC_NUMBER;
		if (write_part(x, 0, &P->cpb.header.magic_number,
			       sizeof(P->cpb.header.magic_number))) {
			rsu_log(RSU_ERR,
				"Unable to write CPBx magic number\n");
			return -EIO;
		}

		updates++;
	}

	if (updates != 2) {
		rsu_log(RSU_ERR, "Did not find two CPBs\n");
		return -ENOENT;
	}

	return 0;
}

/**
 * empty_cpb() - create a cpb with header field only
 *
 * Return: 0 for successful operation, or -ve on error
 */
static int empty_cpb(void)
{
	int ret;
	struct cpb_header {
		u32 magic_number;
		u32 header_size;
		u32 cpb_size;
		u32 cpb_reserved;
		u32 image_ptr_offset;
		u32 image_ptr_slots;
	} *c_header;

	if (P->spt_corrupted) {
		rsu_log(RSU_ERR, "corrupted SPT ---");
		rsu_log(RSU_ERR, "run rsu restore_spt <address> first\n");
		return -EINVAL;
	}

	c_header = (struct cpb_header *)malloc(sizeof(struct cpb_header));
	if (!c_header) {
		rsu_log(RSU_ERR, "failed to allocate cpb_header\n");
		return -ENOMEM;
	}

	c_header->magic_number = CPB_MAGIC_NUMBER;
	c_header->header_size = CPB_HEADER_SIZE;
	c_header->cpb_size = CPB_SIZE;
	c_header->cpb_reserved = 0;
	c_header->image_ptr_offset = CPB_IMAGE_PTR_OFFSET;
	c_header->image_ptr_slots = CPB_IMAGE_PTR_NSLOTS;

	memset(&P->cpb, -1, CPB_SIZE);
	memcpy(&P->cpb, c_header, sizeof(*c_header));

	ret = writeback_cpb();
	if (ret) {
		rsu_log(RSU_ERR, "failed to write back cpb\n");
		goto ops_error;
	}

	P->cpb_slots = (u64 *)&P->cpb.data[P->cpb.header.image_ptr_offset];
	P->cpb_corrupted = false;
	P->cpb_fixed = true;

ops_error:
	free(c_header);
	return ret;
}

/**
 * restore_cpb_from_address() - restore the cpb from an address
 * @address: the address which cpb is restored from
 *
 * Return: 0 for successful operation, or -ve on error
 */
static int restore_cpb_from_address(u64 address)
{
	int ret;
	u32 calc_crc;
	u32 crc_from_saved;
	u32 magic_number;
	char *cpb_data = (char *)address;

	if (P->spt_corrupted) {
		rsu_log(RSU_ERR, "corrupted SPT --");
		rsu_log(RSU_ERR, "run rsu restore_spt <address> first\n");
		return -EINVAL;
	}

	if (!cpb_data) {
		rsu_log(RSU_ERR, "failed due to invalid address\n");
		return -EINVAL;
	}

	calc_crc = crc32(0, (void *)cpb_data, CPB_SIZE);
	rsu_log(RSU_DEBUG, "%s - calc_crc is 0x%x\n", __func__, calc_crc);
	memcpy(&crc_from_saved, cpb_data + CPB_SIZE, sizeof(crc_from_saved));
	rsu_log(RSU_DEBUG, "%s - crc_from_saved is 0x%x\n", __func__,
		crc_from_saved);

	if (calc_crc != crc_from_saved) {
		rsu_log(RSU_ERR, "saved data is corrupted\n");
		return -EINVAL;
	}

	/*
	 * check the magic number to prevent user from accidentally
	 * restoring SPB
	 */
	memcpy(&magic_number, cpb_data, sizeof(magic_number));
	if (magic_number != CPB_MAGIC_NUMBER) {
		rsu_log(RSU_ERR, "failure due to mismatch magic number\n");
		return -EINVAL;
	}

	memcpy(&P->cpb, cpb_data, CPB_SIZE);

	/*
	 * CRC+magic only prove self-consistency; a crafted header with an
	 * out-of-range or misaligned image_ptr_offset would otherwise flow
	 * into cpb_slots[] accesses. Validate before writing back to flash.
	 */
	ret = cpb_header_access_ok();
	if (ret) {
		rsu_log(RSU_ERR, "restored CPB has invalid header\n");
		P->cpb_slots = NULL;
		P->cpb_corrupted = true;
		return ret;
	}

	P->cpb_slots = (u64 *)&P->cpb.data[P->cpb.header.image_ptr_offset];
	ret = check_cpb();
	if (ret) {
		rsu_log(RSU_ERR, "restored CPB failed validation\n");
		P->cpb_slots = NULL;
		P->cpb_corrupted = true;
		return ret;
	}

	ret = writeback_cpb();
	if (ret) {
		rsu_log(RSU_ERR, "failed to write back cpb\n");
		return ret;
	}

	P->cpb_corrupted = false;
	P->cpb_fixed = true;
	return 0;
}

/**
 * partition_count() - get the partition count
 *
 * Return: the number of partition at flash
 */
static int partition_count(void)
{
	return P->spt.partitions;
}

/**
 * partition_name() - get a selected partition name
 * @part_num: the selected partition number
 *
 * Return: partition name on success, or "BAD" on error
 */
static char *partition_name(int part_num)
{
	if (part_num < 0 || part_num >= P->spt.partitions)
		return "BAD";

	return P->spt.partition[part_num].name;
}

/**
 * partition_offset() - get a selected partition offset
 * @part_num: the selected partition number
 *
 * Return: offset on success, or -1 on error
 */
static u64 partition_offset(int part_num)
{
	if (part_num < 0 || part_num >= P->spt.partitions)
		return -1;

	return P->spt.partition[part_num].offset;
}

/**
 * factory_offset() - get the offset of the factory image
 *
 * Return: offset on success, or -ENOENT if factory image not found
 */
static s64 factory_offset(void)
{
	int x;

	for (x = 0; x < P->spt.partitions; x++)
		if (strncmp(P->spt.partition[x].name, FACTORY_IMAGE_NAME,
			    sizeof(P->spt.partition[0].name) - 1) == 0)
			return P->spt.partition[x].offset;

	return -ENOENT;
}

/**
 * partition_size() - get a selected partition size
 * @part_num: the selected partition number
 *
 * Return: the partition size for success, or -1 for error
 */
static u32 partition_size(int part_num)
{
	if (part_num < 0 || part_num >= P->spt.partitions)
		return -1;

	return P->spt.partition[part_num].length;
}

/**
 * partition_reserved() - check if a selected partition is reserved
 * @part_num: the selected partition number
 *
 * Return: 1 for reserved partition, or 0 for not
 */
static int partition_reserved(int part_num)
{
	if (part_num < 0 || part_num >= P->spt.partitions)
		return 0;

	return (P->spt.partition[part_num].flags & SPT_FLAG_RESERVED) ? 1 : 0;
}

/**
 * partition_readonly() - check if a selected partition is read only
 * @part_num: the selected partition number
 *
 * Return: 1 for read only partition, or 0 for not
 */
static int partition_readonly(int part_num)
{
	if (part_num < 0 || part_num >= P->spt.partitions)
		return 0;

	return (P->spt.partition[part_num].flags & SPT_FLAG_READONLY) ? 1 : 0;
}

/**
 * partition_rename() - rename the selected partition name
 * @part_num: the selected partition
 * @name: the new name
 *
 * Return: 0 for success, or -ve on error
 */
static int partition_rename(int part_num, char *name)
{
	int x;
	int ret;

	if (part_num < 0 || part_num >= P->spt.partitions)
		return -EINVAL;

	if (strnlen(name, sizeof(P->spt.partition[0].name)) >=
	    sizeof(P->spt.partition[0].name)) {
		rsu_log(RSU_ERR,
			"Partition name is too long - limited to %li",
			sizeof(P->spt.partition[0].name) - 1);
		return -EINVAL;
	}

	for (x = 0; x < P->spt.partitions; x++) {
		if (strncmp(P->spt.partition[x].name, name,
			    sizeof(P->spt.partition[0].name) - 1) == 0) {
			rsu_log(RSU_ERR,
				"Partition rename already in use\n");
			return -EEXIST;
		}
	}

	rsu_misc_safe_strcpy(P->spt.partition[part_num].name,
			     sizeof(P->spt.partition[0].name),
			     name, sizeof(P->spt.partition[0].name));

	ret = writeback_spt();
	if (ret)
		return ret;

	return load_spt();
}

/**
 * partition_delete() - delete a partition
 * @part_num: the selected partition
 *
 * Return: 0 for success, or -ve on error
 */
static int partition_delete(int part_num)
{
	int x;
	int ret;

	if (part_num < 0 || part_num >= P->spt.partitions) {
		rsu_log(RSU_ERR, "Invalid partition number\n");
		return -EINVAL;
	}

	for (x = part_num; x < P->spt.partitions - 1; x++)
		P->spt.partition[x] = P->spt.partition[x + 1];

	P->spt.partitions--;

	ret = writeback_spt();
	if (ret)
		return ret;

	return load_spt();
}

/**
 * partition_create() - create a partition
 * @name: partition name
 * @start: partition start address
 * @size: partition size
 *
 * Return: 0 for success, or -ve on error
 */
static int partition_create(char *name, u64 start, unsigned int size)
{
	int x;
	int ret;
	u64 end;

	/* Reject overflow before computing end; a wrapped end defeats overlap checks. */
	if ((u64)size > U64_MAX - start) {
		rsu_log(RSU_ERR, "Partition end overflows u64\n");
		return -EINVAL;
	}
	end = start + size;

	if (size % MIN_QSPI_ERASE_SIZE) {
		rsu_log(RSU_ERR, "Invalid partition size\n");
		return -EINVAL;
	}

	if (start % MIN_QSPI_ERASE_SIZE) {
		rsu_log(RSU_ERR, "Invalid partition address\n");
		return -EINVAL;
	}

	if (strnlen(name, sizeof(P->spt.partition[0].name)) >=
	    sizeof(P->spt.partition[0].name)) {
		rsu_log(RSU_ERR, "Partition name is too long - limited to %i\n",
			sizeof(P->spt.partition[0].name) - 1);
		return -EINVAL;
	}

	for (x = 0; x < P->spt.partitions; x++) {
		if (strncmp(P->spt.partition[x].name, name,
			    sizeof(P->spt.partition[0].name) - 1) == 0) {
			rsu_log(RSU_ERR, "Partition name already in use\n");
			return -EEXIST;
		}
	}

	if (P->spt.partitions == SPT_MAX_PARTITIONS) {
		rsu_log(RSU_ERR, "Partition table is full\n");
		return -ENOSPC;
	}

	for (x = 0; x < P->spt.partitions; x++) {
		u64 pstart = P->spt.partition[x].offset;
		u64 plen = P->spt.partition[x].length;
		u64 pend;

		/* Skip a corrupt entry rather than wrap; check_spt() validates on load. */
		if (plen > U64_MAX - pstart)
			continue;
		pend = pstart + plen;

		if (start < pend && end > pstart) {
			rsu_log(RSU_ERR, "Partition overlap\n");
			return -EEXIST;
		}
	}

	rsu_misc_safe_strcpy(P->spt.partition[P->spt.partitions].name,
			     sizeof(P->spt.partition[0].name), name,
			     sizeof(P->spt.partition[0].name));
	P->spt.partition[P->spt.partitions].offset = start;
	P->spt.partition[P->spt.partitions].length = size;
	P->spt.partition[P->spt.partitions].flags = 0;

	P->spt.partitions++;

	ret = writeback_spt();
	if (ret)
		return ret;

	return load_spt();
}

/* Reject corrupt CPB header before indexing cpb_slots[]. */
static int cpb_ptr_slots_access_ok(void)
{
	if (!P->cpb_slots)
		return -EUCLEAN;
	return cpb_header_access_ok();
}

/**
 * priority_get() - get the selected partition's priority
 * @part_num: the selected partition number
 *
 * Return: 0 for success, or -ve on error
 */
static int priority_get(int part_num)
{
	int x;
	int priority = 0;
	int ret;

	if (part_num < 0 || part_num >= P->spt.partitions)
		return -EINVAL;
	ret = cpb_ptr_slots_access_ok();
	if (ret)
		return ret;

	for (x = P->cpb.header.image_ptr_slots; x > 0; x--) {
		if (P->cpb_slots[x - 1] != ERASED_ENTRY &&
		    P->cpb_slots[x - 1] != SPENT_ENTRY) {
			priority++;
			if (P->cpb_slots[x - 1] ==
			    P->spt.partition[part_num].offset)
				return priority;
		}
	}

	return 0;
}

/**
 * priority_add() - enable the selected partition's priority
 * @part_num: the selected partition number
 *
 * Return: 0 for success, or -ve on error
 */
static int priority_add(int part_num)
{
	int x;
	int y;
	int ret;

	if (part_num < 0 || part_num >= P->spt.partitions)
		return -EINVAL;
	ret = cpb_ptr_slots_access_ok();
	if (ret)
		return ret;

	for (x = 0; x < P->cpb.header.image_ptr_slots; x++) {
		if (P->cpb_slots[x] == ERASED_ENTRY) {
			if (update_cpb(x,
				       P->spt.partition[part_num].offset)) {
				/*
				 * update_cpb() may have written one CPB but
				 * not the other; force a reload so load_cpb()
				 * resyncs the two flash copies before we
				 * return failure.
				 */
				load_cpb();
				return -EIO;
			}
			return load_cpb();
		}
	}

	rsu_log(RSU_DEBUG, "Compressing CPB\n");

	for (x = 0, y = 0; x < P->cpb.header.image_ptr_slots; x++) {
		if (P->cpb_slots[x] != ERASED_ENTRY &&
		    P->cpb_slots[x] != SPENT_ENTRY) {
			P->cpb_slots[y++] = P->cpb_slots[x];
		}
	}

	if (y < P->cpb.header.image_ptr_slots)
		P->cpb_slots[y++] = P->spt.partition[part_num].offset;
	else
		return -ENOSPC;

	while (y < P->cpb.header.image_ptr_slots)
		P->cpb_slots[y++] = ERASED_ENTRY;

	ret = writeback_cpb();
	if (ret)
		return ret;

	return load_cpb();
}

/**
 * priority_remove() - remove the selected partition's priority
 * @part_num: the selected partition number
 *
 * Return: 0 for success, or -ve on error
 */
static int priority_remove(int part_num)
{
	int x;
	int ret;

	if (part_num < 0 || part_num >= P->spt.partitions)
		return -EINVAL;
	ret = cpb_ptr_slots_access_ok();
	if (ret)
		return ret;

	for (x = 0; x < P->cpb.header.image_ptr_slots; x++) {
		if (P->cpb_slots[x] == P->spt.partition[part_num].offset)
			if (update_cpb(x, SPENT_ENTRY)) {
				/* See priority_add(): same recovery contract. */
				load_cpb();
				return -EIO;
			}
	}

	return load_cpb();
}

/**
 * data_read() - read data from flash
 * @part_num: partition number
 * @offset: offset which data will be read from
 * @bytes: data size in byte which will be read
 * @buf: pointer to buffer contains to be read data
 *
 * Return: 0 for success, or error code
 */
static int data_read(int part_num, int offset, int bytes, void *buf)
{
	return read_part(part_num, offset, buf, bytes);
}

/**
 * data_write() - write data to flash
 * @part_num: partition number
 * @part_num: offset which data will be written to
 * @bytes: data size in bytes which will be written
 * @buf: pointer to buffer contains to be written data
 *
 * Return: 0 for success, or error code
 */
static int data_write(int part_num, int offset, int bytes, void *buf)
{
	return write_part(part_num, offset, buf, bytes);
}

/**
 * data_erase() - erase flash data
 * @part_num: partition number
 *
 * Return: 0 for success, or error code
 */
static int data_erase(int part_num)
{
	return erase_part(part_num);
}

/**
 * image_load() - load production or factory image
 * @offset: the image offset
 *
 * Return: 0 for success, or error code
 */
static int image_load(u64 offset)
{
	u32 flash_offset[2];

	flash_offset[0] = lower_32_bits(offset);
	flash_offset[1] = upper_32_bits(offset);

	rsu_log(RSU_DEBUG, "RSU_DEBUG: RSU updated to 0x%08x%08x\n",
		flash_offset[1], flash_offset[0]);

	if (mbox_rsu_update(flash_offset))
		return -ELOWLEVEL;

	return 0;
}

/**
 * status_log() - get firmware status info
 * @info: pointer to rsu_status_info
 *
 * Return: 0 for success, or error code
 */
static int status_log(struct rsu_status_info *info)
{
	if (mbox_rsu_status((u32 *)info,
			    sizeof(struct rsu_status_info) / 4)) {
		rsu_log(RSU_ERR,
			"RSU: Firmware or flash content not supporting RSU\n");
		return -ENOTSUPP;
	}

	return 0;
}

/**
 * notify_fw() - call the firmware notify command
 * @value: the notification value
 *
 * Return: 0 for success, or error code
 */
static int notify_fw(u32 value)
{
	rsu_log(RSU_DEBUG, "RSU_DEBUG: notified with 0x%08x.\n", value);

	if (mbox_hps_stage_notify(value))
		return -ELOWLEVEL;

	return 0;
}

/**
 * dcmf_version() - retrieve the decision firmware version
 * @versions: pointer to where the four DCMF versions will be stored
 *
 * This function is used to retrieve the version of each of the four DCMF copies
 * in flash.
 *
 * Returns: 0 on success, or error code
 */
static int dcmf_version(__u32 *versions)
{
	int ret;

	if (!versions)
		return -EINVAL;

	/* get the first flash since DCMF always located at first flash */
	P->flash = P->flashlist[0];

	ret = rsu_mtd_read(P->flash, DCMF0_VERSION_OFFSET, 4, &versions[0]);
	if (ret) {
		rsu_log(RSU_ERR, "read flash error=%i\n", ret);
		return ret;
	}

	ret = rsu_mtd_read(P->flash, DCMF1_VERSION_OFFSET, 4, &versions[1]);
	if (ret) {
		rsu_log(RSU_ERR, "read flash error=%i\n", ret);
		return ret;
	}

	ret = rsu_mtd_read(P->flash, DCMF2_VERSION_OFFSET, 4, &versions[2]);
	if (ret) {
		rsu_log(RSU_ERR, "read flash error=%i\n", ret);
		return ret;
	}

	ret = rsu_mtd_read(P->flash, DCMF3_VERSION_OFFSET, 4, &versions[3]);
	if (ret) {
		rsu_log(RSU_ERR, "read flash error=%i\n", ret);
		return ret;
	}

	return 0;
}

/**
 * dcmf_status() - determine if decision dcmfs are corrupted
 * @status: pointer to where the status will be stored
 *
 * This function is used to determine whether decision firmware copies are
 * corrupted in flash, with the currently used decision firmware being used as
 * reference. The status is an array of 4 values, one for each decision
 * firmware copy. A 0 means the copy is fine, anything else means the copy is
 * corrupted.
 *
 * Returns: 0 on success, or error code
 */
static int dcmf_status(u16 *status)
{
	int ret;
	struct rsu_status_info rsu_status;
	char *buffa = NULL;
	char *buffb = NULL;
	int crt_dcmf;
	int idx;

	/* get the first flash since DCMF always located at first flash */
	P->flash = P->flashlist[0];

	ret = status_log(&rsu_status);
	if (ret) {
		rsu_log(RSU_ERR, "status_log error");
		return ret;
	}
	crt_dcmf = RSU_VERSION_CRT_DCMF_IDX(rsu_status.version);

	buffa = (char *)malloc(DCMF_SIZE);
	if (!buffa) {
		rsu_log(RSU_ERR, "malloc error");
		return -ENOMEM;
	}

	buffb = (char *)malloc(DCMF_SIZE);
	if (!buffb) {
		rsu_log(RSU_ERR, "malloc error");
		ret = -ENOMEM;
		goto ret_val;
	}

	ret = rsu_mtd_read(P->flash, crt_dcmf * DCMF_SIZE, DCMF_SIZE, buffa);
	if (ret) {
		rsu_log(RSU_ERR, "read flash error=%i\n", ret);
		goto ret_val;
	}

	for (idx = 0; idx < 4; idx++) {
		int i;

		status[idx] = 0;

		if (idx == crt_dcmf)
			continue;

		ret = rsu_mtd_read(P->flash, idx * DCMF_SIZE, DCMF_SIZE, buffb);
		if (ret) {
			rsu_log(RSU_ERR, "read flash error=%i\n", ret);
			goto ret_val;
		}

		for (i = 0; i < DCMF_SIZE; i++)
			if (buffa[i] != buffb[i]) {
				status[idx] = 1;
				break;
			}
	}

ret_val:
	if (buffa)
		free(buffa);
	if (buffb)
		free(buffb);
	return ret;
}

/**
 * max_retry() - retrieve the max_retry parameter
 * @value: pointer to where the max_retry will be stored
 *
 * This function is used to retrieve the max_retry parameter from the decision
 * firmware data section.
 *
 * Returns: 0 on success, or error code
 */
static int max_retry(__u8 *value)
{
	int ret;
	__u8 tmp;

	if (!value)
		return -EINVAL;

	/* get the first flash since DCMF always located at first flash */
	P->flash = P->flashlist[0];

	ret = rsu_mtd_read(P->flash, DCIO_MAX_RETRY_OFFSET, 1, &tmp);
	if (ret) {
		rsu_log(RSU_ERR, "read flash error=%i\n", ret);
		return ret;
	}

	/* Add one, to make value consistent with Quartus view */
	*value = tmp + 1;

	return ret;
}

/* Forward decl so qspi_ll_intf can reference .exit before the body. */
static void ll_exit(void);

static struct rsu_ll_intf qspi_ll_intf = {
	.exit = ll_exit,
	.priv = NULL,

	.partition.count = partition_count,
	.partition.name = partition_name,
	.partition.offset = partition_offset,
	.partition.factory_offset = factory_offset,
	.partition.size = partition_size,
	.partition.reserved = partition_reserved,
	.partition.readonly = partition_readonly,
	.partition.rename = partition_rename,
	.partition.delete = partition_delete,
	.partition.create = partition_create,

	.priority.get = priority_get,
	.priority.add = priority_add,
	.priority.remove = priority_remove,

	.data.read = data_read,
	.data.write = data_write,
	.data.erase = data_erase,

	.fw_ops.load = image_load,
	.fw_ops.status = status_log,
	.fw_ops.notify = notify_fw,
	.fw_ops.dcmf_version = dcmf_version,
	.fw_ops.dcmf_status = dcmf_status,
	.fw_ops.max_retry = max_retry,

	.spt_ops.restore = restore_spt_from_address,
	.spt_ops.save = save_spt_to_address,
	.spt_ops.corrupted = corrupted_spt,

	.cpb_ops.empty = empty_cpb,
	.cpb_ops.restore = restore_cpb_from_address,
	.cpb_ops.save = save_cpb_to_address,
	.cpb_ops.corrupted = corrupted_cpb
};

static void ll_exit(void)
{
	struct rsu_qspi_priv *ctx = qspi_ctx;

	if (!ctx)
		return;

	ctx->cpb0_part = -1;
	ctx->cpb1_part = -1;
	ctx->cpb_corrupted = false;
	ctx->cpb_fixed = false;
	ctx->spt_corrupted = false;

	for (int i = 0; i < QSPI_MAX_DEVICE; i++) {
		if (ctx->flashlist && ctx->flashlist[i]) {
			rsu_mtd_unclaim(ctx->flashlist[i]);
			ctx->flashlist[i] = NULL;
		}
	}

	free(ctx->flashlist);
	ctx->flashlist = NULL;
	ctx->flash = NULL;
	ctx->cpb_slots = NULL;

	memset(ctx, 0, sizeof(*ctx));
	free(ctx);
	qspi_ctx = NULL;

	/* Clear so a stale .priv cannot dereference freed memory. */
	qspi_ll_intf.priv = NULL;
}

#if CONFIG_IS_ENABLED(SOCFPGA_RSU_MULTIFLASH)
int get_num_flash(u32 *flash_enabled)
{
	int flash_count = 0;
	struct flash_info mbox_flash_info[QSPI_MAX_DEVICE];

	/* retrieve qspi info from mailbox */
	if (mbox_qspi_get_device_info((u32 *)mbox_flash_info, 8)) {
		rsu_log(RSU_ERR,
			"%s: RSU:Firmware or flash content not supporting RSU\n", __func__);
		return -EOPNOTSUPP;
	}

	for (int i = 0; i < QSPI_MAX_DEVICE; i++) {
		debug("QSPI Device INFO\nflash_info[%d]: 0x%08x\nlen: 0x%08x\n",
		      i, mbox_flash_info[i].size, QSPI_GET_DEVICE_INFO_RESP_LEN);
	}

	for (int i = 0; i < QSPI_MAX_DEVICE; i++) {
		flash_enabled[i] = 0;

		/* Calculate the size return by mailbox; */
		if (mbox_flash_info[i].size > SZ_2G) {
			/* >2Gb, power the bit 0-30 */
			debug("RSU: QSPI %d larger than 2Gbits capacity.", i);
			flash_enabled[i] = pow(2, mbox_flash_info[i].size & GENMASK(30, 0));
			flash_enabled[i] = flash_enabled[i] / SZ_8;
		} else {
			/* <= 2Gb, total the bit 0-30 */
			debug("RSU: QSPI %d lower than 2Gbits capacity.", i);
			flash_enabled[i] = mbox_flash_info[i].size & GENMASK(30, 0);
			flash_enabled[i] = flash_enabled[i] / SZ_8;
		}

		debug("Calculated flash size[%d]: %d\n", i, flash_enabled[i]);
		if (flash_enabled[i] > 0)
			flash_count++;
	}

	debug("RSU: Total flash #: %d\n", flash_count);

	return flash_count;
}
#endif

int rsu_ll_qspi_init(struct rsu_ll_intf **intf)
{
	u32 spt_offset[SPT_OFFSET_MBOX];

	qspi_ctx = malloc(sizeof(*qspi_ctx));
	if (!qspi_ctx)
		return -ENOMEM;
	memset(qspi_ctx, 0, sizeof(*qspi_ctx));
	qspi_ctx->cpb0_part = -1;
	qspi_ctx->cpb1_part = -1;
	qspi_ctx->num_flash = -1;
	/* Set for future per-session use; current backend reads qspi_ctx directly. */
	qspi_ll_intf.priv = qspi_ctx;

	/* get the offset from firmware */
	if (mbox_rsu_get_spt_offset(spt_offset, 4)) {
		rsu_log(RSU_ERR,
			"RSU: Firmware or flash content not supporting RSU\n");
		ll_exit();
		return -ECOMM;
	}

#if CONFIG_IS_ENABLED(SOCFPGA_RSU_MULTIFLASH)
	/* Zero-init so a partial fill cannot leak into the probe loop. */
	u32 flash_enabled[QSPI_MAX_DEVICE] = {0};

	/* retrieve qspi info from mailbox */
	P->num_flash = get_num_flash(flash_enabled);
	printf("%s: MULTIFLASH_ENABLED: num_flash #%d\n", __func__, P->num_flash);
	if (P->num_flash < 0) {
		rsu_log(RSU_ERR, "get_num_flash failed, err=%d\n",
			P->num_flash);
		ll_exit();
		return P->num_flash;
	}
#else
	P->num_flash = 1;
	printf("%s: MULTIFLASH_DISABLED: num_flash #%d\n", __func__, P->num_flash);
#endif

	P->flashlist = calloc(QSPI_MAX_DEVICE, sizeof(*P->flashlist));
	if (!P->flashlist) {
		rsu_log(RSU_ERR,
			"RSU: Failed to allocate memory for flash list. Exiting.\n");
		ll_exit();
		return -ENOMEM;
	}

#if CONFIG_IS_ENABLED(SOCFPGA_RSU_MULTIFLASH)
	{
		int found = 0;

		/*
		 * Firmware may enable CSes sparsely; compact the probed
		 * handles into a contiguous prefix because consumers iterate
		 * 0..num_flash-1 unconditionally.
		 */
		for (int i = 0; i < QSPI_MAX_DEVICE; i++) {
			debug("%s: probe flash #%d\n", __func__, i);
			if (flash_enabled[i] > 0) {
				int err;

				err = rsu_mtd_probe(CONFIG_SF_DEFAULT_BUS, i,
						    &P->flash);
				if (err) {
					rsu_log(RSU_ERR, "SPI probe failed.\n");
					ll_exit();
					return err;
				}

				P->flashlist[found++] = P->flash;
			}
		}

		if (!found) {
			rsu_log(RSU_ERR,
				"RSU: no QSPI flash enabled by firmware\n");
			ll_exit();
			return -ENODEV;
		}

		/* Resync num_flash with the compacted flashlist[]. */
		P->num_flash = found;
	}
#else
	{
		int err;

		err = rsu_mtd_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SOCFPGA_RSU_SF_CS,
				    &P->flash);
		if (err) {
			ll_exit();
			rsu_log(RSU_ERR, "SPI probe failed.\n");
			return err;
		}

		P->flashlist[0] = P->flash;
	}
#endif
	P->spt0_offset = spt_offset[1];
	P->spt1_offset = spt_offset[3];
	rsu_log(RSU_DEBUG, "SPT0 offset 0x%08x\n", P->spt0_offset);
	rsu_log(RSU_DEBUG, "SPT1 offset 0x%08x\n", P->spt1_offset);

	if (load_spt() && !P->spt_corrupted) {
		ll_exit();
		rsu_log(RSU_ERR, "Bad SPT\n");
		return -EUCLEAN;
	}

	if (P->spt_corrupted) {
		P->cpb_corrupted = true;
	} else if (load_cpb() && !P->cpb_corrupted) {
		ll_exit();
		rsu_log(RSU_ERR, "Bad CPB\n");
		return -EUCLEAN;
	}

	*intf = &qspi_ll_intf;

	return 0;
}
