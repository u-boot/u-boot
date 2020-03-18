// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <console.h>
#include <dfu.h>
#include <malloc.h>
#include <mmc.h>
#include <dm/uclass.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/sizes.h>

#include "stm32prog.h"

/* Primary GPT header size for 128 entries : 17kB = 34 LBA of 512B */
#define GPT_HEADER_SZ	34

#define OPT_SELECT	BIT(0)
#define OPT_EMPTY	BIT(1)

#define IS_SELECT(part)	((part)->option & OPT_SELECT)
#define IS_EMPTY(part)	((part)->option & OPT_EMPTY)

#define ALT_BUF_LEN			SZ_1K

#define ROOTFS_MMC0_UUID \
	EFI_GUID(0xE91C4E10, 0x16E6, 0x4C0E, \
		 0xBD, 0x0E, 0x77, 0xBE, 0xCF, 0x4A, 0x35, 0x82)

#define ROOTFS_MMC1_UUID \
	EFI_GUID(0x491F6117, 0x415D, 0x4F53, \
		 0x88, 0xC9, 0x6E, 0x0D, 0xE5, 0x4D, 0xEA, 0xC6)

#define ROOTFS_MMC2_UUID \
	EFI_GUID(0xFD58F1C7, 0xBE0D, 0x4338, \
		 0x88, 0xE9, 0xAD, 0x8F, 0x05, 0x0A, 0xEB, 0x18)

/* RAW parttion (binary / bootloader) used Linux - reserved UUID */
#define LINUX_RESERVED_UUID "8DA63339-0007-60C0-C436-083AC8230908"

/*
 * unique partition guid (uuid) for partition named "rootfs"
 * on each MMC instance = SD Card or eMMC
 * allow fixed kernel bootcmd: "rootf=PARTUID=e91c4e10-..."
 */
static const efi_guid_t uuid_mmc[3] = {
	ROOTFS_MMC0_UUID,
	ROOTFS_MMC1_UUID,
	ROOTFS_MMC2_UUID
};

DECLARE_GLOBAL_DATA_PTR;

/* order of column in flash layout file */
enum stm32prog_col_t {
	COL_OPTION,
	COL_ID,
	COL_NAME,
	COL_TYPE,
	COL_IP,
	COL_OFFSET,
	COL_NB_STM32
};

char *stm32prog_get_error(struct stm32prog_data *data)
{
	static const char error_msg[] = "Unspecified";

	if (strlen(data->error) == 0)
		strcpy(data->error, error_msg);

	return data->error;
}

u8 stm32prog_header_check(struct raw_header_s *raw_header,
			  struct image_header_s *header)
{
	unsigned int i;

	header->present = 0;
	header->image_checksum = 0x0;
	header->image_length = 0x0;

	if (!raw_header || !header) {
		pr_debug("%s:no header data\n", __func__);
		return -1;
	}
	if (raw_header->magic_number !=
		(('S' << 0) | ('T' << 8) | ('M' << 16) | (0x32 << 24))) {
		pr_debug("%s:invalid magic number : 0x%x\n",
			 __func__, raw_header->magic_number);
		return -2;
	}
	/* only header v1.0 supported */
	if (raw_header->header_version != 0x00010000) {
		pr_debug("%s:invalid header version : 0x%x\n",
			 __func__, raw_header->header_version);
		return -3;
	}
	if (raw_header->reserved1 != 0x0 || raw_header->reserved2) {
		pr_debug("%s:invalid reserved field\n", __func__);
		return -4;
	}
	for (i = 0; i < (sizeof(raw_header->padding) / 4); i++) {
		if (raw_header->padding[i] != 0) {
			pr_debug("%s:invalid padding field\n", __func__);
			return -5;
		}
	}
	header->present = 1;
	header->image_checksum = le32_to_cpu(raw_header->image_checksum);
	header->image_length = le32_to_cpu(raw_header->image_length);

	return 0;
}

static u32 stm32prog_header_checksum(u32 addr, struct image_header_s *header)
{
	u32 i, checksum;
	u8 *payload;

	/* compute checksum on payload */
	payload = (u8 *)addr;
	checksum = 0;
	for (i = header->image_length; i > 0; i--)
		checksum += *(payload++);

	return checksum;
}

/* FLASHLAYOUT PARSING *****************************************/
static int parse_option(struct stm32prog_data *data,
			int i, char *p, struct stm32prog_part_t *part)
{
	int result = 0;
	char *c = p;

	part->option = 0;
	if (!strcmp(p, "-"))
		return 0;

	while (*c) {
		switch (*c) {
		case 'P':
			part->option |= OPT_SELECT;
			break;
		case 'E':
			part->option |= OPT_EMPTY;
			break;
		default:
			result = -EINVAL;
			stm32prog_err("Layout line %d: invalid option '%c' in %s)",
				      i, *c, p);
			return -EINVAL;
		}
		c++;
	}
	if (!(part->option & OPT_SELECT)) {
		stm32prog_err("Layout line %d: missing 'P' in option %s", i, p);
		return -EINVAL;
	}

	return result;
}

static int parse_id(struct stm32prog_data *data,
		    int i, char *p, struct stm32prog_part_t *part)
{
	int result = 0;
	unsigned long value;

	result = strict_strtoul(p, 0, &value);
	part->id = value;
	if (result || value > PHASE_LAST_USER) {
		stm32prog_err("Layout line %d: invalid phase value = %s", i, p);
		result = -EINVAL;
	}

	return result;
}

static int parse_name(struct stm32prog_data *data,
		      int i, char *p, struct stm32prog_part_t *part)
{
	int result = 0;

	if (strlen(p) < sizeof(part->name)) {
		strcpy(part->name, p);
	} else {
		stm32prog_err("Layout line %d: partition name too long [%d]: %s",
			      i, strlen(p), p);
		result = -EINVAL;
	}

	return result;
}

static int parse_type(struct stm32prog_data *data,
		      int i, char *p, struct stm32prog_part_t *part)
{
	int result = 0;

	if (!strcmp(p, "Binary")) {
		part->part_type = PART_BINARY;
	} else if (!strcmp(p, "System")) {
		part->part_type = PART_SYSTEM;
	} else if (!strcmp(p, "FileSystem")) {
		part->part_type = PART_FILESYSTEM;
	} else if (!strcmp(p, "RawImage")) {
		part->part_type = RAW_IMAGE;
	} else {
		result = -EINVAL;
	}
	if (result)
		stm32prog_err("Layout line %d: type parsing error : '%s'",
			      i, p);

	return result;
}

static int parse_ip(struct stm32prog_data *data,
		    int i, char *p, struct stm32prog_part_t *part)
{
	int result = 0;
	unsigned int len = 0;

	part->dev_id = 0;
	if (!strcmp(p, "none")) {
		part->target = STM32PROG_NONE;
	} else if (!strncmp(p, "mmc", 3)) {
		part->target = STM32PROG_MMC;
		len = 3;
	} else {
		result = -EINVAL;
	}
	if (len) {
		/* only one digit allowed for device id */
		if (strlen(p) != len + 1) {
			result = -EINVAL;
		} else {
			part->dev_id = p[len] - '0';
			if (part->dev_id > 9)
				result = -EINVAL;
		}
	}
	if (result)
		stm32prog_err("Layout line %d: ip parsing error: '%s'", i, p);

	return result;
}

static int parse_offset(struct stm32prog_data *data,
			int i, char *p, struct stm32prog_part_t *part)
{
	int result = 0;
	char *tail;

	part->part_id = 0;
	part->addr = 0;
	part->size = 0;
	/* eMMC boot parttion */
	if (!strncmp(p, "boot", 4)) {
		if (strlen(p) != 5) {
			result = -EINVAL;
		} else {
			if (p[4] == '1')
				part->part_id = -1;
			else if (p[4] == '2')
				part->part_id = -2;
			else
				result = -EINVAL;
		}
		if (result)
			stm32prog_err("Layout line %d: invalid part '%s'",
				      i, p);
	} else {
		part->addr = simple_strtoull(p, &tail, 0);
		if (tail == p || *tail != '\0') {
			stm32prog_err("Layout line %d: invalid offset '%s'",
				      i, p);
			result = -EINVAL;
		}
	}

	return result;
}

static
int (* const parse[COL_NB_STM32])(struct stm32prog_data *data, int i, char *p,
				  struct stm32prog_part_t *part) = {
	[COL_OPTION] = parse_option,
	[COL_ID] = parse_id,
	[COL_NAME] =  parse_name,
	[COL_TYPE] = parse_type,
	[COL_IP] = parse_ip,
	[COL_OFFSET] = parse_offset,
};

static int parse_flash_layout(struct stm32prog_data *data,
			      ulong addr,
			      ulong size)
{
	int column = 0, part_nb = 0, ret;
	bool end_of_line, eof;
	char *p, *start, *last, *col;
	struct stm32prog_part_t *part;
	int part_list_size;
	int i;

	data->part_nb = 0;

	/* check if STM32image is detected */
	if (!stm32prog_header_check((struct raw_header_s *)addr,
				    &data->header)) {
		u32 checksum;

		addr = addr + BL_HEADER_SIZE;
		size = data->header.image_length;

		checksum = stm32prog_header_checksum(addr, &data->header);
		if (checksum != data->header.image_checksum) {
			stm32prog_err("Layout: invalid checksum : 0x%x expected 0x%x",
				      checksum, data->header.image_checksum);
			return -EIO;
		}
	}
	if (!size)
		return -EINVAL;

	start = (char *)addr;
	last = start + size;

	*last = 0x0; /* force null terminated string */
	pr_debug("flash layout =\n%s\n", start);

	/* calculate expected number of partitions */
	part_list_size = 1;
	p = start;
	while (*p && (p < last)) {
		if (*p++ == '\n') {
			part_list_size++;
			if (p < last && *p == '#')
				part_list_size--;
		}
	}
	if (part_list_size > PHASE_LAST_USER) {
		stm32prog_err("Layout: too many partition (%d)",
			      part_list_size);
		return -1;
	}
	part = calloc(sizeof(struct stm32prog_part_t), part_list_size);
	if (!part) {
		stm32prog_err("Layout: alloc failed");
		return -ENOMEM;
	}
	data->part_array = part;

	/* main parsing loop */
	i = 1;
	eof = false;
	p = start;
	col = start; /* 1st column */
	end_of_line = false;
	while (!eof) {
		switch (*p) {
		/* CR is ignored and replaced by NULL character */
		case '\r':
			*p = '\0';
			p++;
			continue;
		case '\0':
			end_of_line = true;
			eof = true;
			break;
		case '\n':
			end_of_line = true;
			break;
		case '\t':
			break;
		case '#':
			/* comment line is skipped */
			if (column == 0 && p == col) {
				while ((p < last) && *p)
					if (*p++ == '\n')
						break;
				col = p;
				i++;
				if (p >= last || !*p) {
					eof = true;
					end_of_line = true;
				}
				continue;
			}
			/* fall through */
		/* by default continue with the next character */
		default:
			p++;
			continue;
		}

		/* replace by \0: allow string parsing for each column */
		*p = '\0';
		p++;
		if (p >= last) {
			eof = true;
			end_of_line = true;
		}

		/* skip empty line and multiple TAB in tsv file */
		if (strlen(col) == 0) {
			col = p;
			/* skip empty line */
			if (column == 0 && end_of_line) {
				end_of_line = false;
				i++;
			}
			continue;
		}

		if (column < COL_NB_STM32) {
			ret = parse[column](data, i, col, part);
			if (ret)
				return ret;
		}

		/* save the beginning of the next column */
		column++;
		col = p;

		if (!end_of_line)
			continue;

		/* end of the line detected */
		end_of_line = false;

		if (column < COL_NB_STM32) {
			stm32prog_err("Layout line %d: no enought column", i);
			return -EINVAL;
		}
		column = 0;
		part_nb++;
		part++;
		i++;
		if (part_nb >= part_list_size) {
			part = NULL;
			if (!eof) {
				stm32prog_err("Layout: no enought memory for %d part",
					      part_nb);
				return -EINVAL;
			}
		}
	}
	data->part_nb = part_nb;
	if (data->part_nb == 0) {
		stm32prog_err("Layout: no partition found");
		return -ENODEV;
	}

	return 0;
}

static int __init part_cmp(void *priv, struct list_head *a, struct list_head *b)
{
	struct stm32prog_part_t *parta, *partb;

	parta = container_of(a, struct stm32prog_part_t, list);
	partb = container_of(b, struct stm32prog_part_t, list);

	if (parta->part_id != partb->part_id)
		return parta->part_id - partb->part_id;
	else
		return parta->addr > partb->addr ? 1 : -1;
}

static int init_device(struct stm32prog_data *data,
		       struct stm32prog_dev_t *dev)
{
	struct mmc *mmc = NULL;
	struct blk_desc *block_dev = NULL;
	int part_id;
	u64 first_addr = 0, last_addr = 0;
	struct stm32prog_part_t *part, *next_part;

	switch (dev->target) {
#ifdef CONFIG_MMC
	case STM32PROG_MMC:
		mmc = find_mmc_device(dev->dev_id);
		if (mmc_init(mmc)) {
			stm32prog_err("mmc device %d not found", dev->dev_id);
			return -ENODEV;
		}
		block_dev = mmc_get_blk_desc(mmc);
		if (!block_dev) {
			stm32prog_err("mmc device %d not probed", dev->dev_id);
			return -ENODEV;
		}
		dev->erase_size = mmc->erase_grp_size * block_dev->blksz;
		dev->mmc = mmc;

		/* reserve a full erase group for each GTP headers */
		if (mmc->erase_grp_size > GPT_HEADER_SZ) {
			first_addr = dev->erase_size;
			last_addr = (u64)(block_dev->lba -
					  mmc->erase_grp_size) *
				    block_dev->blksz;
		} else {
			first_addr = (u64)GPT_HEADER_SZ * block_dev->blksz;
			last_addr = (u64)(block_dev->lba - GPT_HEADER_SZ - 1) *
				    block_dev->blksz;
		}
		pr_debug("MMC %d: lba=%ld blksz=%ld\n", dev->dev_id,
			 block_dev->lba, block_dev->blksz);
		pr_debug(" available address = 0x%llx..0x%llx\n",
			 first_addr, last_addr);
		break;
#endif
	default:
		stm32prog_err("unknown device type = %d", dev->target);
		return -ENODEV;
	}
	pr_debug(" erase size = 0x%x\n", dev->erase_size);

	/* order partition list in offset order */
	list_sort(NULL, &dev->part_list, &part_cmp);
	part_id = 1;
	pr_debug("id : Opt Phase     Name target.n dev.n addr     size     part_off part_size\n");
	list_for_each_entry(part, &dev->part_list, list) {
		if (part->part_type == RAW_IMAGE) {
			part->part_id = 0x0;
			part->addr = 0x0;
			if (block_dev)
				part->size = block_dev->lba * block_dev->blksz;
			else
				part->size = last_addr;
			pr_debug("-- : %1d %02x %14s %02d %02d.%02d %08llx %08llx\n",
				 part->option, part->id, part->name,
				 part->part_type, part->target,
				 part->dev_id, part->addr, part->size);
			continue;
		}
		if (part->part_id < 0) { /* boot hw partition for eMMC */
			if (mmc) {
				part->size = mmc->capacity_boot;
			} else {
				stm32prog_err("%s (0x%x): hw partition not expected : %d",
					      part->name, part->id,
					      part->part_id);
				return -ENODEV;
			}
		} else {
			part->part_id = part_id++;

			/* last partition : size to the end of the device */
			if (part->list.next != &dev->part_list) {
				next_part =
					container_of(part->list.next,
						     struct stm32prog_part_t,
						     list);
				if (part->addr < next_part->addr) {
					part->size = next_part->addr -
						     part->addr;
				} else {
					stm32prog_err("%s (0x%x): same address : 0x%llx == %s (0x%x): 0x%llx",
						      part->name, part->id,
						      part->addr,
						      next_part->name,
						      next_part->id,
						      next_part->addr);
					return -EINVAL;
				}
			} else {
				if (part->addr <= last_addr) {
					part->size = last_addr - part->addr;
				} else {
					stm32prog_err("%s (0x%x): invalid address 0x%llx (max=0x%llx)",
						      part->name, part->id,
						      part->addr, last_addr);
					return -EINVAL;
				}
			}
			if (part->addr < first_addr) {
				stm32prog_err("%s (0x%x): invalid address 0x%llx (min=0x%llx)",
					      part->name, part->id,
					      part->addr, first_addr);
				return -EINVAL;
			}
		}
		if ((part->addr & ((u64)part->dev->erase_size - 1)) != 0) {
			stm32prog_err("%s (0x%x): not aligned address : 0x%llx on erase size 0x%x",
				      part->name, part->id, part->addr,
				      part->dev->erase_size);
			return -EINVAL;
		}
		pr_debug("%02d : %1d %02x %14s %02d %02d.%02d %08llx %08llx",
			 part->part_id, part->option, part->id, part->name,
			 part->part_type, part->target,
			 part->dev_id, part->addr, part->size);
	}
	return 0;
}

static int treat_partition_list(struct stm32prog_data *data)
{
	int i, j;
	struct stm32prog_part_t *part;

	for (j = 0; j < STM32PROG_MAX_DEV; j++) {
		data->dev[j].target = STM32PROG_NONE;
		INIT_LIST_HEAD(&data->dev[j].part_list);
	}

	for (i = 0; i < data->part_nb; i++) {
		part = &data->part_array[i];
		part->alt_id = -1;

		/* skip partition with IP="none" */
		if (part->target == STM32PROG_NONE) {
			if (IS_SELECT(part)) {
				stm32prog_err("Layout: selected none phase = 0x%x",
					      part->id);
				return -EINVAL;
			}
			continue;
		}

		if (part->id == PHASE_FLASHLAYOUT ||
		    part->id > PHASE_LAST_USER) {
			stm32prog_err("Layout: invalid phase = 0x%x",
				      part->id);
			return -EINVAL;
		}
		for (j = i + 1; j < data->part_nb; j++) {
			if (part->id == data->part_array[j].id) {
				stm32prog_err("Layout: duplicated phase 0x%x at line %d and %d",
					      part->id, i, j);
				return -EINVAL;
			}
		}
		for (j = 0; j < STM32PROG_MAX_DEV; j++) {
			if (data->dev[j].target == STM32PROG_NONE) {
				/* new device found */
				data->dev[j].target = part->target;
				data->dev[j].dev_id = part->dev_id;
				data->dev_nb++;
				break;
			} else if ((part->target == data->dev[j].target) &&
				   (part->dev_id == data->dev[j].dev_id)) {
				break;
			}
		}
		if (j == STM32PROG_MAX_DEV) {
			stm32prog_err("Layout: too many device");
			return -EINVAL;
		}
		part->dev = &data->dev[j];
		list_add_tail(&part->list, &data->dev[j].part_list);
	}

	return 0;
}

static int create_partitions(struct stm32prog_data *data)
{
#ifdef CONFIG_MMC
	int offset = 0;
	const int buflen = SZ_8K;
	char *buf;
	char uuid[UUID_STR_LEN + 1];
	unsigned char *uuid_bin;
	unsigned int mmc_id;
	int i;
	bool rootfs_found;
	struct stm32prog_part_t *part;

	buf = malloc(buflen);
	if (!buf)
		return -ENOMEM;

	puts("partitions : ");
	/* initialize the selected device */
	for (i = 0; i < data->dev_nb; i++) {
		offset = 0;
		rootfs_found = false;
		memset(buf, 0, buflen);

		list_for_each_entry(part, &data->dev[i].part_list, list) {
			/* skip eMMC boot partitions */
			if (part->part_id < 0)
				continue;
			/* skip Raw Image */
			if (part->part_type == RAW_IMAGE)
				continue;

			if (offset + 100 > buflen) {
				pr_debug("\n%s: buffer too small, %s skippped",
					 __func__, part->name);
				continue;
			}

			if (!offset)
				offset += sprintf(buf, "gpt write mmc %d \"",
						  data->dev[i].dev_id);

			offset += snprintf(buf + offset, buflen - offset,
					   "name=%s,start=0x%llx,size=0x%llx",
					   part->name,
					   part->addr,
					   part->size);

			if (part->part_type == PART_BINARY)
				offset += snprintf(buf + offset,
						   buflen - offset,
						   ",type="
						   LINUX_RESERVED_UUID);
			else
				offset += snprintf(buf + offset,
						   buflen - offset,
						   ",type=linux");

			if (part->part_type == PART_SYSTEM)
				offset += snprintf(buf + offset,
						   buflen - offset,
						   ",bootable");

			if (!rootfs_found && !strcmp(part->name, "rootfs")) {
				mmc_id = part->dev_id;
				rootfs_found = true;
				if (mmc_id < ARRAY_SIZE(uuid_mmc)) {
					uuid_bin =
					  (unsigned char *)uuid_mmc[mmc_id].b;
					uuid_bin_to_str(uuid_bin, uuid,
							UUID_STR_FORMAT_GUID);
					offset += snprintf(buf + offset,
							   buflen - offset,
							   ",uuid=%s", uuid);
				}
			}

			offset += snprintf(buf + offset, buflen - offset, ";");
		}

		if (offset) {
			offset += snprintf(buf + offset, buflen - offset, "\"");
			pr_debug("\ncmd: %s\n", buf);
			if (run_command(buf, 0)) {
				stm32prog_err("GPT partitionning fail: %s",
					      buf);
				free(buf);

				return -1;
			}
		}

		if (data->dev[i].mmc)
			part_init(mmc_get_blk_desc(data->dev[i].mmc));

#ifdef DEBUG
		sprintf(buf, "gpt verify mmc %d", data->dev[i].dev_id);
		pr_debug("\ncmd: %s", buf);
		if (run_command(buf, 0))
			printf("fail !\n");
		else
			printf("OK\n");

		sprintf(buf, "part list mmc %d", data->dev[i].dev_id);
		run_command(buf, 0);
#endif
	}
	puts("done\n");

	free(buf);
#endif

	return 0;
}

static int stm32prog_alt_add(struct stm32prog_data *data,
			     struct dfu_entity *dfu,
			     struct stm32prog_part_t *part)
{
	int ret = 0;
	int offset = 0;
	char devstr[10];
	char dfustr[10];
	char buf[ALT_BUF_LEN];
	u32 size;
	char multiplier,  type;

	/* max 3 digit for sector size */
	if (part->size > SZ_1M) {
		size = (u32)(part->size / SZ_1M);
		multiplier = 'M';
	} else if (part->size > SZ_1K) {
		size = (u32)(part->size / SZ_1K);
		multiplier = 'K';
	} else {
		size = (u32)part->size;
		multiplier = 'B';
	}
	if (IS_SELECT(part) && !IS_EMPTY(part))
		type = 'e'; /*Readable and Writeable*/
	else
		type = 'a';/*Readable*/

	memset(buf, 0, sizeof(buf));
	offset = snprintf(buf, ALT_BUF_LEN - offset,
			  "@%s/0x%02x/1*%d%c%c ",
			  part->name, part->id,
			  size, multiplier, type);

	if (part->part_type == RAW_IMAGE) {
		u64 dfu_size;

		if (part->dev->target == STM32PROG_MMC)
			dfu_size = part->size / part->dev->mmc->read_bl_len;
		else
			dfu_size = part->size;
		offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
				   "raw 0x0 0x%llx", dfu_size);
	} else if (part->part_id < 0) {
		u64 nb_blk = part->size / part->dev->mmc->read_bl_len;

		offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
				   "raw 0x%llx 0x%llx",
				   part->addr, nb_blk);
		offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
				   " mmcpart %d;", -(part->part_id));
	} else {
		offset += snprintf(buf + offset,
				   ALT_BUF_LEN - offset,
				   "part");
		/* dev_id requested by DFU MMC */
		if (part->target == STM32PROG_MMC)
			offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
					   " %d", part->dev_id);
		offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
				   " %d;", part->part_id);
	}
	switch (part->target) {
#ifdef CONFIG_MMC
	case STM32PROG_MMC:
		sprintf(dfustr, "mmc");
		sprintf(devstr, "%d", part->dev_id);
		break;
#endif
	default:
		stm32prog_err("invalid target: %d", part->target);
		return -ENODEV;
	}
	pr_debug("dfu_alt_add(%s,%s,%s)\n", dfustr, devstr, buf);
	ret = dfu_alt_add(dfu, dfustr, devstr, buf);
	pr_debug("dfu_alt_add(%s,%s,%s) result %d\n",
		 dfustr, devstr, buf, ret);

	return ret;
}

static int stm32prog_alt_add_virt(struct dfu_entity *dfu,
				  char *name, int phase, int size)
{
	int ret = 0;
	char devstr[4];
	char buf[ALT_BUF_LEN];

	sprintf(devstr, "%d", phase);
	sprintf(buf, "@%s/0x%02x/1*%dBe", name, phase, size);
	ret = dfu_alt_add(dfu, "virt", devstr, buf);
	pr_debug("dfu_alt_add(virt,%s,%s) result %d\n", devstr, buf, ret);

	return ret;
}

static int dfu_init_entities(struct stm32prog_data *data)
{
	int ret = 0;
	int phase, i, alt_id;
	struct stm32prog_part_t *part;
	struct dfu_entity *dfu;
	int alt_nb;

	alt_nb = 1; /* number of virtual = CMD */
	if (data->part_nb == 0)
		alt_nb++;  /* +1 for FlashLayout */
	else
		for (i = 0; i < data->part_nb; i++) {
			if (data->part_array[i].target != STM32PROG_NONE)
				alt_nb++;
		}

	if (dfu_alt_init(alt_nb, &dfu))
		return -ENODEV;

	puts("DFU alt info setting: ");
	if (data->part_nb) {
		alt_id = 0;
		for (phase = 1;
		     (phase <= PHASE_LAST_USER) &&
		     (alt_id < alt_nb) && !ret;
		     phase++) {
			/* ordering alt setting by phase id */
			part = NULL;
			for (i = 0; i < data->part_nb; i++) {
				if (phase == data->part_array[i].id) {
					part = &data->part_array[i];
					break;
				}
			}
			if (!part)
				continue;
			if (part->target == STM32PROG_NONE)
				continue;
			part->alt_id = alt_id;
			alt_id++;

			ret = stm32prog_alt_add(data, dfu, part);
		}
	} else {
		char buf[ALT_BUF_LEN];

		sprintf(buf, "@FlashLayout/0x%02x/1*256Ke ram %x 40000",
			PHASE_FLASHLAYOUT, STM32_DDR_BASE);
		ret = dfu_alt_add(dfu, "ram", NULL, buf);
		pr_debug("dfu_alt_add(ram, NULL,%s) result %d\n", buf, ret);
	}

	if (!ret)
		ret = stm32prog_alt_add_virt(dfu, "virtual", PHASE_CMD, 512);

	if (ret)
		stm32prog_err("dfu init failed: %d", ret);
	puts("done\n");

#ifdef DEBUG
	dfu_show_entities();
#endif
	return ret;
}

static void stm32prog_end_phase(struct stm32prog_data *data)
{
	if (data->phase == PHASE_FLASHLAYOUT) {
		if (parse_flash_layout(data, STM32_DDR_BASE, 0))
			stm32prog_err("Layout: invalid FlashLayout");
		return;
	}

	if (!data->cur_part)
		return;

	if (CONFIG_IS_ENABLED(MMC) &&
	    data->cur_part->part_id < 0) {
		char cmdbuf[60];

		sprintf(cmdbuf, "mmc bootbus %d 0 0 0; mmc partconf %d 1 %d 0",
			data->cur_part->dev_id, data->cur_part->dev_id,
			-(data->cur_part->part_id));
		if (run_command(cmdbuf, 0)) {
			stm32prog_err("commands '%s' failed", cmdbuf);
			return;
		}
	}
}

void stm32prog_do_reset(struct stm32prog_data *data)
{
	if (data->phase == PHASE_RESET) {
		data->phase = PHASE_DO_RESET;
		puts("Reset requested\n");
	}
}

void stm32prog_next_phase(struct stm32prog_data *data)
{
	int phase, i;
	struct stm32prog_part_t *part;
	bool found;

	phase = data->phase;
	switch (phase) {
	case PHASE_RESET:
	case PHASE_END:
	case PHASE_DO_RESET:
		return;
	}

	/* found next selected partition */
	data->cur_part = NULL;
	data->phase = PHASE_END;
	found = false;
	do {
		phase++;
		if (phase > PHASE_LAST_USER)
			break;
		for (i = 0; i < data->part_nb; i++) {
			part = &data->part_array[i];
			if (part->id == phase) {
				if (IS_SELECT(part) && !IS_EMPTY(part)) {
					data->cur_part = part;
					data->phase = phase;
					found = true;
				}
				break;
			}
		}
	} while (!found);

	if (data->phase == PHASE_END)
		puts("Phase=END\n");
}

static void stm32prog_devices_init(struct stm32prog_data *data)
{
	int i;
	int ret;

	ret = treat_partition_list(data);
	if (ret)
		goto error;

	/* initialize the selected device */
	for (i = 0; i < data->dev_nb; i++) {
		ret = init_device(data, &data->dev[i]);
		if (ret)
			goto error;
	}

	ret = create_partitions(data);
	if (ret)
		goto error;

	return;

error:
	data->part_nb = 0;
}

int stm32prog_dfu_init(struct stm32prog_data *data)
{
	/* init device if no error */
	if (data->part_nb)
		stm32prog_devices_init(data);

	if (data->part_nb)
		stm32prog_next_phase(data);

	/* prepare DFU for device read/write */
	dfu_free_entities();
	return dfu_init_entities(data);
}

int stm32prog_init(struct stm32prog_data *data, ulong addr, ulong size)
{
	memset(data, 0x0, sizeof(*data));
	data->phase = PHASE_FLASHLAYOUT;

	return parse_flash_layout(data, addr, size);
}

void stm32prog_clean(struct stm32prog_data *data)
{
	/* clean */
	dfu_free_entities();
	free(data->part_array);
	free(data->header_data);
}

/* DFU callback: used after serial and direct DFU USB access */
void dfu_flush_callback(struct dfu_entity *dfu)
{
	if (!stm32prog_data)
		return;

	if (dfu->dev_type == DFU_DEV_RAM) {
		if (dfu->alt == 0 &&
		    stm32prog_data->phase == PHASE_FLASHLAYOUT) {
			stm32prog_end_phase(stm32prog_data);
			/* waiting DFU DETACH for reenumeration */
		}
	}

	if (!stm32prog_data->cur_part)
		return;

	if (dfu->alt == stm32prog_data->cur_part->alt_id) {
		stm32prog_end_phase(stm32prog_data);
		stm32prog_next_phase(stm32prog_data);
	}
}

void dfu_initiated_callback(struct dfu_entity *dfu)
{
	if (!stm32prog_data)
		return;

	if (!stm32prog_data->cur_part)
		return;

	/* force the saved offset for the current partition */
	if (dfu->alt == stm32prog_data->cur_part->alt_id) {
		dfu->offset = stm32prog_data->offset;
		pr_debug("dfu offset = 0x%llx\n", dfu->offset);
	}
}
