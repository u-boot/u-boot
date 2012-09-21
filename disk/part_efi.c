/*
 * Copyright (C) 2008 RuggedCom, Inc.
 * Richard Retanubun <RichardRetanubun@RuggedCom.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Problems with CONFIG_SYS_64BIT_LBA:
 *
 * struct disk_partition.start in include/part.h is sized as ulong.
 * When CONFIG_SYS_64BIT_LBA is activated, lbaint_t changes from ulong to uint64_t.
 * For now, it is cast back to ulong at assignment.
 *
 * This limits the maximum size of addressable storage to < 2 Terra Bytes
 */
#include <common.h>
#include <command.h>
#include <ide.h>
#include <malloc.h>
#include "part_efi.h"
#include <linux/ctype.h>

#if defined(CONFIG_CMD_IDE) || \
    defined(CONFIG_CMD_SATA) || \
    defined(CONFIG_CMD_SCSI) || \
    defined(CONFIG_CMD_USB) || \
    defined(CONFIG_MMC) || \
    defined(CONFIG_SYSTEMACE)

/* Convert char[2] in little endian format to the host format integer
 */
static inline unsigned short le16_to_int(unsigned char *le16)
{
	return ((le16[1] << 8) + le16[0]);
}

/* Convert char[4] in little endian format to the host format integer
 */
static inline unsigned long le32_to_int(unsigned char *le32)
{
	return ((le32[3] << 24) + (le32[2] << 16) + (le32[1] << 8) + le32[0]);
}

/* Convert char[8] in little endian format to the host format integer
 */
static inline unsigned long long le64_to_int(unsigned char *le64)
{
	return (((unsigned long long)le64[7] << 56) +
		((unsigned long long)le64[6] << 48) +
		((unsigned long long)le64[5] << 40) +
		((unsigned long long)le64[4] << 32) +
		((unsigned long long)le64[3] << 24) +
		((unsigned long long)le64[2] << 16) +
		((unsigned long long)le64[1] << 8) +
		(unsigned long long)le64[0]);
}

/**
 * efi_crc32() - EFI version of crc32 function
 * @buf: buffer to calculate crc32 of
 * @len - length of buf
 *
 * Description: Returns EFI-style CRC32 value for @buf
 */
static inline unsigned long efi_crc32(const void *buf, unsigned long len)
{
	return crc32(0, buf, len);
}

/*
 * Private function prototypes
 */

static int pmbr_part_valid(struct partition *part);
static int is_pmbr_valid(legacy_mbr * mbr);

static int is_gpt_valid(block_dev_desc_t * dev_desc, unsigned long long lba,
				gpt_header * pgpt_head, gpt_entry ** pgpt_pte);

static gpt_entry *alloc_read_gpt_entries(block_dev_desc_t * dev_desc,
				gpt_header * pgpt_head);

static int is_pte_valid(gpt_entry * pte);

static char *print_efiname(gpt_entry *pte)
{
	static char name[PARTNAME_SZ + 1];
	int i;
	for (i = 0; i < PARTNAME_SZ; i++) {
		u8 c;
		c = pte->partition_name[i] & 0xff;
		c = (c && !isprint(c)) ? '.' : c;
		name[i] = c;
	}
	name[PARTNAME_SZ] = 0;
	return name;
}

/*
 * Public Functions (include/part.h)
 */

void print_part_efi(block_dev_desc_t * dev_desc)
{
	ALLOC_CACHE_ALIGN_BUFFER(gpt_header, gpt_head, 1);
	gpt_entry *gpt_pte = NULL;
	int i = 0;

	if (!dev_desc) {
		printf("%s: Invalid Argument(s)\n", __func__);
		return;
	}
	/* This function validates AND fills in the GPT header and PTE */
	if (is_gpt_valid(dev_desc, GPT_PRIMARY_PARTITION_TABLE_LBA,
			 gpt_head, &gpt_pte) != 1) {
		printf("%s: *** ERROR: Invalid GPT ***\n", __func__);
		return;
	}

	debug("%s: gpt-entry at %p\n", __func__, gpt_pte);

	printf("Part\tName\t\t\tStart LBA\tEnd LBA\n");
	for (i = 0; i < le32_to_int(gpt_head->num_partition_entries); i++) {

		if (is_pte_valid(&gpt_pte[i])) {
			printf("%3d\t%-18s\t0x%08llX\t0x%08llX\n", (i + 1),
				print_efiname(&gpt_pte[i]),
				le64_to_int(gpt_pte[i].starting_lba),
				le64_to_int(gpt_pte[i].ending_lba));
		} else {
			break;	/* Stop at the first non valid PTE */
		}
	}

	/* Remember to free pte */
	free(gpt_pte);
	return;
}

#ifdef CONFIG_PARTITION_UUIDS
static void uuid_string(unsigned char *uuid, char *str)
{
	static const u8 le[16] = {3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11,
				  12, 13, 14, 15};
	int i;

	for (i = 0; i < 16; i++) {
		sprintf(str, "%02x", uuid[le[i]]);
		str += 2;
		switch (i) {
		case 3:
		case 5:
		case 7:
		case 9:
			*str++ = '-';
			break;
		}
	}
}
#endif

int get_partition_info_efi(block_dev_desc_t * dev_desc, int part,
				disk_partition_t * info)
{
	ALLOC_CACHE_ALIGN_BUFFER(gpt_header, gpt_head, 1);
	gpt_entry *gpt_pte = NULL;

	/* "part" argument must be at least 1 */
	if (!dev_desc || !info || part < 1) {
		printf("%s: Invalid Argument(s)\n", __func__);
		return -1;
	}

	/* This function validates AND fills in the GPT header and PTE */
	if (is_gpt_valid(dev_desc, GPT_PRIMARY_PARTITION_TABLE_LBA,
			gpt_head, &gpt_pte) != 1) {
		printf("%s: *** ERROR: Invalid GPT ***\n", __func__);
		return -1;
	}

	if (part > le32_to_int(gpt_head->num_partition_entries) ||
	    !is_pte_valid(&gpt_pte[part - 1])) {
		printf("%s: *** ERROR: Invalid partition number %d ***\n",
			__func__, part);
		return -1;
	}

	/* The ulong casting limits the maximum disk size to 2 TB */
	info->start = (ulong) le64_to_int(gpt_pte[part - 1].starting_lba);
	/* The ending LBA is inclusive, to calculate size, add 1 to it */
	info->size = ((ulong)le64_to_int(gpt_pte[part - 1].ending_lba) + 1)
		     - info->start;
	info->blksz = GPT_BLOCK_SIZE;

	sprintf((char *)info->name, "%s",
			print_efiname(&gpt_pte[part - 1]));
	sprintf((char *)info->type, "U-Boot");
#ifdef CONFIG_PARTITION_UUIDS
	uuid_string(gpt_pte[part - 1].unique_partition_guid.b, info->uuid);
#endif

	debug("%s: start 0x%lX, size 0x%lX, name %s", __func__,
		info->start, info->size, info->name);

	/* Remember to free pte */
	free(gpt_pte);
	return 0;
}

int test_part_efi(block_dev_desc_t * dev_desc)
{
	ALLOC_CACHE_ALIGN_BUFFER(legacy_mbr, legacymbr, 1);

	/* Read legacy MBR from block 0 and validate it */
	if ((dev_desc->block_read(dev_desc->dev, 0, 1, (ulong *)legacymbr) != 1)
		|| (is_pmbr_valid(legacymbr) != 1)) {
		return -1;
	}
	return 0;
}

/*
 * Private functions
 */
/*
 * pmbr_part_valid(): Check for EFI partition signature
 *
 * Returns: 1 if EFI GPT partition type is found.
 */
static int pmbr_part_valid(struct partition *part)
{
	if (part->sys_ind == EFI_PMBR_OSTYPE_EFI_GPT &&
		le32_to_int(part->start_sect) == 1UL) {
		return 1;
	}

	return 0;
}

/*
 * is_pmbr_valid(): test Protective MBR for validity
 *
 * Returns: 1 if PMBR is valid, 0 otherwise.
 * Validity depends on two things:
 *  1) MSDOS signature is in the last two bytes of the MBR
 *  2) One partition of type 0xEE is found, checked by pmbr_part_valid()
 */
static int is_pmbr_valid(legacy_mbr * mbr)
{
	int i = 0;

	if (!mbr || le16_to_int(mbr->signature) != MSDOS_MBR_SIGNATURE) {
		return 0;
	}

	for (i = 0; i < 4; i++) {
		if (pmbr_part_valid(&mbr->partition_record[i])) {
			return 1;
		}
	}
	return 0;
}

/**
 * is_gpt_valid() - tests one GPT header and PTEs for validity
 *
 * lba is the logical block address of the GPT header to test
 * gpt is a GPT header ptr, filled on return.
 * ptes is a PTEs ptr, filled on return.
 *
 * Description: returns 1 if valid,  0 on error.
 * If valid, returns pointers to PTEs.
 */
static int is_gpt_valid(block_dev_desc_t * dev_desc, unsigned long long lba,
			gpt_header * pgpt_head, gpt_entry ** pgpt_pte)
{
	unsigned char crc32_backup[4] = { 0 };
	unsigned long calc_crc32;
	unsigned long long lastlba;

	if (!dev_desc || !pgpt_head) {
		printf("%s: Invalid Argument(s)\n", __func__);
		return 0;
	}

	/* Read GPT Header from device */
	if (dev_desc->block_read(dev_desc->dev, lba, 1, pgpt_head) != 1) {
		printf("*** ERROR: Can't read GPT header ***\n");
		return 0;
	}

	/* Check the GPT header signature */
	if (le64_to_int(pgpt_head->signature) != GPT_HEADER_SIGNATURE) {
		printf("GUID Partition Table Header signature is wrong:"
			"0x%llX != 0x%llX\n",
			(unsigned long long)le64_to_int(pgpt_head->signature),
			(unsigned long long)GPT_HEADER_SIGNATURE);
		return 0;
	}

	/* Check the GUID Partition Table CRC */
	memcpy(crc32_backup, pgpt_head->header_crc32, sizeof(crc32_backup));
	memset(pgpt_head->header_crc32, 0, sizeof(pgpt_head->header_crc32));

	calc_crc32 = efi_crc32((const unsigned char *)pgpt_head,
		le32_to_int(pgpt_head->header_size));

	memcpy(pgpt_head->header_crc32, crc32_backup, sizeof(crc32_backup));

	if (calc_crc32 != le32_to_int(crc32_backup)) {
		printf("GUID Partition Table Header CRC is wrong:"
			"0x%08lX != 0x%08lX\n",
			le32_to_int(crc32_backup), calc_crc32);
		return 0;
	}

	/* Check that the my_lba entry points to the LBA that contains the GPT */
	if (le64_to_int(pgpt_head->my_lba) != lba) {
		printf("GPT: my_lba incorrect: %llX != %llX\n",
			(unsigned long long)le64_to_int(pgpt_head->my_lba),
			(unsigned long long)lba);
		return 0;
	}

	/* Check the first_usable_lba and last_usable_lba are within the disk. */
	lastlba = (unsigned long long)dev_desc->lba;
	if (le64_to_int(pgpt_head->first_usable_lba) > lastlba) {
		printf("GPT: first_usable_lba incorrect: %llX > %llX\n",
			le64_to_int(pgpt_head->first_usable_lba), lastlba);
		return 0;
	}
	if (le64_to_int(pgpt_head->last_usable_lba) > lastlba) {
		printf("GPT: last_usable_lba incorrect: %llX > %llX\n",
			le64_to_int(pgpt_head->last_usable_lba), lastlba);
		return 0;
	}

	debug("GPT: first_usable_lba: %llX last_usable_lba %llX last lba %llX\n",
		le64_to_int(pgpt_head->first_usable_lba),
		le64_to_int(pgpt_head->last_usable_lba), lastlba);

	/* Read and allocate Partition Table Entries */
	*pgpt_pte = alloc_read_gpt_entries(dev_desc, pgpt_head);
	if (*pgpt_pte == NULL) {
		printf("GPT: Failed to allocate memory for PTE\n");
		return 0;
	}

	/* Check the GUID Partition Table Entry Array CRC */
	calc_crc32 = efi_crc32((const unsigned char *)*pgpt_pte,
		le32_to_int(pgpt_head->num_partition_entries) *
		le32_to_int(pgpt_head->sizeof_partition_entry));

	if (calc_crc32 != le32_to_int(pgpt_head->partition_entry_array_crc32)) {
		printf("GUID Partition Table Entry Array CRC is wrong:"
			"0x%08lX != 0x%08lX\n",
			le32_to_int(pgpt_head->partition_entry_array_crc32),
			calc_crc32);

		free(*pgpt_pte);
		return 0;
	}

	/* We're done, all's well */
	return 1;
}

/**
 * alloc_read_gpt_entries(): reads partition entries from disk
 * @dev_desc
 * @gpt - GPT header
 *
 * Description: Returns ptes on success,  NULL on error.
 * Allocates space for PTEs based on information found in @gpt.
 * Notes: remember to free pte when you're done!
 */
static gpt_entry *alloc_read_gpt_entries(block_dev_desc_t * dev_desc,
					 gpt_header * pgpt_head)
{
	size_t count = 0;
	gpt_entry *pte = NULL;

	if (!dev_desc || !pgpt_head) {
		printf("%s: Invalid Argument(s)\n", __func__);
		return NULL;
	}

	count = le32_to_int(pgpt_head->num_partition_entries) *
		le32_to_int(pgpt_head->sizeof_partition_entry);

	debug("%s: count = %lu * %lu = %u\n", __func__,
		le32_to_int(pgpt_head->num_partition_entries),
		le32_to_int(pgpt_head->sizeof_partition_entry), count);

	/* Allocate memory for PTE, remember to FREE */
	if (count != 0) {
		pte = memalign(ARCH_DMA_MINALIGN, count);
	}

	if (count == 0 || pte == NULL) {
		printf("%s: ERROR: Can't allocate 0x%X bytes for GPT Entries\n",
			__func__, count);
		return NULL;
	}

	/* Read GPT Entries from device */
	if (dev_desc->block_read (dev_desc->dev,
		(unsigned long)le64_to_int(pgpt_head->partition_entry_lba),
		(lbaint_t) (count / GPT_BLOCK_SIZE), pte)
		!= (count / GPT_BLOCK_SIZE)) {

		printf("*** ERROR: Can't read GPT Entries ***\n");
		free(pte);
		return NULL;
	}
	return pte;
}

/**
 * is_pte_valid(): validates a single Partition Table Entry
 * @gpt_entry - Pointer to a single Partition Table Entry
 *
 * Description: returns 1 if valid,  0 on error.
 */
static int is_pte_valid(gpt_entry * pte)
{
	efi_guid_t unused_guid;

	if (!pte) {
		printf("%s: Invalid Argument(s)\n", __func__);
		return 0;
	}

	/* Only one validation for now:
	 * The GUID Partition Type != Unused Entry (ALL-ZERO)
	 */
	memset(unused_guid.b, 0, sizeof(unused_guid.b));

	if (memcmp(pte->partition_type_guid.b, unused_guid.b,
		sizeof(unused_guid.b)) == 0) {

		debug("%s: Found an unused PTE GUID at 0x%08X\n", __func__,
		(unsigned int)pte);

		return 0;
	} else {
		return 1;
	}
}
#endif
