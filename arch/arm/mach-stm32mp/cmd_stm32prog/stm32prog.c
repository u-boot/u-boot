// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <command.h>
#include <console.h>
#include <dfu.h>
#include <malloc.h>
#include <misc.h>
#include <mmc.h>
#include <part.h>
#include <asm/arch/stm32mp1_smc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <jffs2/load_kernel.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#include "stm32prog.h"

/* Primary GPT header size for 128 entries : 17kB = 34 LBA of 512B */
#define GPT_HEADER_SZ	34

#define OPT_SELECT	BIT(0)
#define OPT_EMPTY	BIT(1)
#define OPT_DELETE	BIT(2)

#define IS_SELECT(part)	((part)->option & OPT_SELECT)
#define IS_EMPTY(part)	((part)->option & OPT_EMPTY)
#define IS_DELETE(part)	((part)->option & OPT_DELETE)

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

/* partition handling routines : CONFIG_CMD_MTDPARTS */
int mtdparts_init(void);
int find_dev_and_part(const char *id, struct mtd_device **dev,
		      u8 *part_num, struct part_info **part);

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
		log_debug("%s:no header data\n", __func__);
		return -1;
	}
	if (raw_header->magic_number !=
		(('S' << 0) | ('T' << 8) | ('M' << 16) | (0x32 << 24))) {
		log_debug("%s:invalid magic number : 0x%x\n",
			  __func__, raw_header->magic_number);
		return -2;
	}
	/* only header v1.0 supported */
	if (raw_header->header_version != 0x00010000) {
		log_debug("%s:invalid header version : 0x%x\n",
			  __func__, raw_header->header_version);
		return -3;
	}
	if (raw_header->reserved1 != 0x0 || raw_header->reserved2) {
		log_debug("%s:invalid reserved field\n", __func__);
		return -4;
	}
	for (i = 0; i < (sizeof(raw_header->padding) / 4); i++) {
		if (raw_header->padding[i] != 0) {
			log_debug("%s:invalid padding field\n", __func__);
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
		case 'D':
			part->option |= OPT_DELETE;
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
	int len = 0;

	part->bin_nb = 0;
	if (!strncmp(p, "Binary", 6)) {
		part->part_type = PART_BINARY;

		/* search for Binary(X) case */
		len = strlen(p);
		part->bin_nb = 1;
		if (len > 6) {
			if (len < 8 ||
			    (p[6] != '(') ||
			    (p[len - 1] != ')'))
				result = -EINVAL;
			else
				part->bin_nb =
					simple_strtoul(&p[7], NULL, 10);
		}
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
	} else if (!strncmp(p, "nor", 3)) {
		part->target = STM32PROG_NOR;
		len = 3;
	} else if (!strncmp(p, "nand", 4)) {
		part->target = STM32PROG_NAND;
		len = 4;
	} else if (!strncmp(p, "spi-nand", 8)) {
		part->target = STM32PROG_SPI_NAND;
		len = 8;
	} else if (!strncmp(p, "ram", 3)) {
		part->target = STM32PROG_RAM;
		len = 0;
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
	log_debug("flash layout =\n%s\n", start);

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

static void get_mtd_by_target(char *string, enum stm32prog_target target,
			      int dev_id)
{
	const char *dev_str;

	switch (target) {
	case STM32PROG_NOR:
		dev_str = "nor";
		break;
	case STM32PROG_NAND:
		dev_str = "nand";
		break;
	case STM32PROG_SPI_NAND:
		dev_str = "spi-nand";
		break;
	default:
		dev_str = "invalid";
		break;
	}
	sprintf(string, "%s%d", dev_str, dev_id);
}

static int init_device(struct stm32prog_data *data,
		       struct stm32prog_dev_t *dev)
{
	struct mmc *mmc = NULL;
	struct blk_desc *block_dev = NULL;
	struct mtd_info *mtd = NULL;
	char mtd_id[16];
	int part_id;
	int ret;
	u64 first_addr = 0, last_addr = 0;
	struct stm32prog_part_t *part, *next_part;
	u64 part_addr, part_size;
	bool part_found;
	const char *part_name;

	switch (dev->target) {
	case STM32PROG_MMC:
		if (!IS_ENABLED(CONFIG_MMC)) {
			stm32prog_err("unknown device type = %d", dev->target);
			return -ENODEV;
		}
		mmc = find_mmc_device(dev->dev_id);
		if (!mmc || mmc_init(mmc)) {
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
		log_debug("MMC %d: lba=%ld blksz=%ld\n", dev->dev_id,
			  block_dev->lba, block_dev->blksz);
		log_debug(" available address = 0x%llx..0x%llx\n",
			  first_addr, last_addr);
		log_debug(" full_update = %d\n", dev->full_update);
		break;
	case STM32PROG_NOR:
	case STM32PROG_NAND:
	case STM32PROG_SPI_NAND:
		if (!IS_ENABLED(CONFIG_MTD)) {
			stm32prog_err("unknown device type = %d", dev->target);
			return -ENODEV;
		}
		get_mtd_by_target(mtd_id, dev->target, dev->dev_id);
		log_debug("%s\n", mtd_id);

		mtdparts_init();
		mtd = get_mtd_device_nm(mtd_id);
		if (IS_ERR(mtd)) {
			stm32prog_err("MTD device %s not found", mtd_id);
			return -ENODEV;
		}
		first_addr = 0;
		last_addr = mtd->size;
		dev->erase_size = mtd->erasesize;
		log_debug("MTD device %s: size=%lld erasesize=%d\n",
			  mtd_id, mtd->size, mtd->erasesize);
		log_debug(" available address = 0x%llx..0x%llx\n",
			  first_addr, last_addr);
		dev->mtd = mtd;
		break;
	case STM32PROG_RAM:
		first_addr = gd->bd->bi_dram[0].start;
		last_addr = first_addr + gd->bd->bi_dram[0].size;
		dev->erase_size = 1;
		break;
	default:
		stm32prog_err("unknown device type = %d", dev->target);
		return -ENODEV;
	}
	log_debug(" erase size = 0x%x\n", dev->erase_size);
	log_debug(" full_update = %d\n", dev->full_update);

	/* order partition list in offset order */
	list_sort(NULL, &dev->part_list, &part_cmp);
	part_id = 1;
	log_debug("id : Opt Phase     Name target.n dev.n addr     size     part_off part_size\n");
	list_for_each_entry(part, &dev->part_list, list) {
		if (part->bin_nb > 1) {
			if ((dev->target != STM32PROG_NAND &&
			     dev->target != STM32PROG_SPI_NAND) ||
			    part->id >= PHASE_FIRST_USER ||
			    strncmp(part->name, "fsbl", 4)) {
				stm32prog_err("%s (0x%x): multiple binary %d not supported",
					      part->name, part->id,
					      part->bin_nb);
				return -EINVAL;
			}
		}
		if (part->part_type == RAW_IMAGE) {
			part->part_id = 0x0;
			part->addr = 0x0;
			if (block_dev)
				part->size = block_dev->lba * block_dev->blksz;
			else
				part->size = last_addr;
			log_debug("-- : %1d %02x %14s %02d.%d %02d.%02d %08llx %08llx\n",
				  part->option, part->id, part->name,
				  part->part_type, part->bin_nb, part->target,
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
		log_debug("%02d : %1d %02x %14s %02d.%d %02d.%02d %08llx %08llx",
			  part->part_id, part->option, part->id, part->name,
			  part->part_type, part->bin_nb, part->target,
			  part->dev_id, part->addr, part->size);

		part_addr = 0;
		part_size = 0;
		part_found = false;

		/* check coherency with existing partition */
		if (block_dev) {
			/*
			 * block devices with GPT: check user partition size
			 * only for partial update, the GPT partions are be
			 * created for full update
			 */
			if (dev->full_update || part->part_id < 0) {
				log_debug("\n");
				continue;
			}
			struct disk_partition partinfo;

			ret = part_get_info(block_dev, part->part_id,
					    &partinfo);

			if (ret) {
				stm32prog_err("%s (0x%x):Couldn't find part %d on device mmc %d",
					      part->name, part->id,
					      part_id, part->dev_id);
				return -ENODEV;
			}
			part_addr = (u64)partinfo.start * partinfo.blksz;
			part_size = (u64)partinfo.size * partinfo.blksz;
			part_name = (char *)partinfo.name;
			part_found = true;
		}

		if (IS_ENABLED(CONFIG_MTD) && mtd) {
			char mtd_part_id[32];
			struct part_info *mtd_part;
			struct mtd_device *mtd_dev;
			u8 part_num;

			sprintf(mtd_part_id, "%s,%d", mtd_id,
				part->part_id - 1);
			ret = find_dev_and_part(mtd_part_id, &mtd_dev,
						&part_num, &mtd_part);
			if (ret != 0) {
				stm32prog_err("%s (0x%x): Invalid MTD partition %s",
					      part->name, part->id,
					      mtd_part_id);
				return -ENODEV;
			}
			part_addr = mtd_part->offset;
			part_size = mtd_part->size;
			part_name = mtd_part->name;
			part_found = true;
		}

		/* no partition for this device */
		if (!part_found) {
			log_debug("\n");
			continue;
		}

		log_debug(" %08llx %08llx\n", part_addr, part_size);

		if (part->addr != part_addr) {
			stm32prog_err("%s (0x%x): Bad address for partition %d (%s) = 0x%llx <> 0x%llx expected",
				      part->name, part->id, part->part_id,
				      part_name, part->addr, part_addr);
			return -ENODEV;
		}
		if (part->size != part_size) {
			stm32prog_err("%s (0x%x): Bad size for partition %d (%s) at 0x%llx = 0x%llx <> 0x%llx expected",
				      part->name, part->id, part->part_id,
				      part_name, part->addr, part->size,
				      part_size);
			return -ENODEV;
		}
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

	data->tee_detected = false;
	data->fsbl_nor_detected = false;
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
				data->dev[j].full_update = true;
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
		switch (part->target)  {
		case STM32PROG_NOR:
			if (!data->fsbl_nor_detected &&
			    !strncmp(part->name, "fsbl", 4))
				data->fsbl_nor_detected = true;
			/* fallthrough */
		case STM32PROG_NAND:
		case STM32PROG_SPI_NAND:
			if (!data->tee_detected &&
			    !strncmp(part->name, "tee", 3))
				data->tee_detected = true;
			break;
		default:
			break;
		}
		part->dev = &data->dev[j];
		if (!IS_SELECT(part))
			part->dev->full_update = false;
		list_add_tail(&part->list, &data->dev[j].part_list);
	}

	return 0;
}

static int create_gpt_partitions(struct stm32prog_data *data)
{
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
		/* create gpt partition support only for full update on MMC */
		if (data->dev[i].target != STM32PROG_MMC ||
		    !data->dev[i].full_update)
			continue;

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
				log_debug("\n%s: buffer too small, %s skippped",
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
			log_debug("\ncmd: %s\n", buf);
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
		log_debug("\ncmd: %s", buf);
		if (run_command(buf, 0))
			printf("fail !\n");
		else
			printf("OK\n");

		sprintf(buf, "part list mmc %d", data->dev[i].dev_id);
		run_command(buf, 0);
#endif
	}
	puts("done\n");

#ifdef DEBUG
	run_command("mtd list", 0);
#endif
	free(buf);

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

	if (part->target == STM32PROG_RAM) {
		offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
				   "ram 0x%llx 0x%llx",
				   part->addr, part->size);
	} else if (part->part_type == RAW_IMAGE) {
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
		if (part->part_type == PART_SYSTEM &&
		    (part->target == STM32PROG_NAND ||
		     part->target == STM32PROG_NOR ||
		     part->target == STM32PROG_SPI_NAND))
			offset += snprintf(buf + offset,
					   ALT_BUF_LEN - offset,
					   "partubi");
		else
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
	ret = -ENODEV;
	switch (part->target) {
	case STM32PROG_MMC:
		if (IS_ENABLED(CONFIG_MMC)) {
			ret = 0;
			sprintf(dfustr, "mmc");
			sprintf(devstr, "%d", part->dev_id);
		}
		break;
	case STM32PROG_NAND:
	case STM32PROG_NOR:
	case STM32PROG_SPI_NAND:
		if (IS_ENABLED(CONFIG_MTD)) {
			ret = 0;
			sprintf(dfustr, "mtd");
			get_mtd_by_target(devstr, part->target, part->dev_id);
		}
		break;
	case STM32PROG_RAM:
		ret = 0;
		sprintf(dfustr, "ram");
		sprintf(devstr, "0");
		break;
	default:
		break;
	}
	if (ret) {
		stm32prog_err("invalid target: %d", part->target);
		return ret;
	}
	log_debug("dfu_alt_add(%s,%s,%s)\n", dfustr, devstr, buf);
	ret = dfu_alt_add(dfu, dfustr, devstr, buf);
	log_debug("dfu_alt_add(%s,%s,%s) result %d\n",
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
	log_debug("dfu_alt_add(virt,%s,%s) result %d\n", devstr, buf, ret);

	return ret;
}

static int dfu_init_entities(struct stm32prog_data *data)
{
	int ret = 0;
	int phase, i, alt_id;
	struct stm32prog_part_t *part;
	struct dfu_entity *dfu;
	int alt_nb;

	alt_nb = 3; /* number of virtual = CMD, OTP, PMIC*/
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
		log_debug("dfu_alt_add(ram, NULL,%s) result %d\n", buf, ret);
	}

	if (!ret)
		ret = stm32prog_alt_add_virt(dfu, "virtual", PHASE_CMD, 512);

	if (!ret)
		ret = stm32prog_alt_add_virt(dfu, "OTP", PHASE_OTP, 512);

	if (!ret && CONFIG_IS_ENABLED(DM_PMIC))
		ret = stm32prog_alt_add_virt(dfu, "PMIC", PHASE_PMIC, 8);

	if (ret)
		stm32prog_err("dfu init failed: %d", ret);
	puts("done\n");

#ifdef DEBUG
	dfu_show_entities();
#endif
	return ret;
}

int stm32prog_otp_write(struct stm32prog_data *data, u32 offset, u8 *buffer,
			long *size)
{
	log_debug("%s: %x %lx\n", __func__, offset, *size);

	if (!data->otp_part) {
		data->otp_part = memalign(CONFIG_SYS_CACHELINE_SIZE, OTP_SIZE);
		if (!data->otp_part)
			return -ENOMEM;
	}

	if (!offset)
		memset(data->otp_part, 0, OTP_SIZE);

	if (offset + *size > OTP_SIZE)
		*size = OTP_SIZE - offset;

	memcpy((void *)((u32)data->otp_part + offset), buffer, *size);

	return 0;
}

int stm32prog_otp_read(struct stm32prog_data *data, u32 offset, u8 *buffer,
		       long *size)
{
	int result = 0;

	if (!IS_ENABLED(CONFIG_ARM_SMCCC)) {
		stm32prog_err("OTP update not supported");

		return -1;
	}

	log_debug("%s: %x %lx\n", __func__, offset, *size);
	/* alway read for first packet */
	if (!offset) {
		if (!data->otp_part)
			data->otp_part =
				memalign(CONFIG_SYS_CACHELINE_SIZE, OTP_SIZE);

		if (!data->otp_part) {
			result = -ENOMEM;
			goto end_otp_read;
		}

		/* init struct with 0 */
		memset(data->otp_part, 0, OTP_SIZE);

		/* call the service */
		result = stm32_smc_exec(STM32_SMC_BSEC, STM32_SMC_READ_ALL,
					(u32)data->otp_part, 0);
		if (result)
			goto end_otp_read;
	}

	if (!data->otp_part) {
		result = -ENOMEM;
		goto end_otp_read;
	}

	if (offset + *size > OTP_SIZE)
		*size = OTP_SIZE - offset;
	memcpy(buffer, (void *)((u32)data->otp_part + offset), *size);

end_otp_read:
	log_debug("%s: result %i\n", __func__, result);

	return result;
}

int stm32prog_otp_start(struct stm32prog_data *data)
{
	int result = 0;
	struct arm_smccc_res res;

	if (!IS_ENABLED(CONFIG_ARM_SMCCC)) {
		stm32prog_err("OTP update not supported");

		return -1;
	}

	if (!data->otp_part) {
		stm32prog_err("start OTP without data");
		return -1;
	}

	arm_smccc_smc(STM32_SMC_BSEC, STM32_SMC_WRITE_ALL,
		      (u32)data->otp_part, 0, 0, 0, 0, 0, &res);

	if (!res.a0) {
		switch (res.a1) {
		case 0:
			result = 0;
			break;
		case 1:
			stm32prog_err("Provisioning");
			result = 0;
			break;
		default:
			log_err("%s: OTP incorrect value (err = %ld)\n",
				__func__, res.a1);
			result = -EINVAL;
			break;
		}
	} else {
		log_err("%s: Failed to exec svc=%x op=%x in secure mode (err = %ld)\n",
			__func__, STM32_SMC_BSEC, STM32_SMC_WRITE_ALL, res.a0);
		result = -EINVAL;
	}

	free(data->otp_part);
	data->otp_part = NULL;
	log_debug("%s: result %i\n", __func__, result);

	return result;
}

int stm32prog_pmic_write(struct stm32prog_data *data, u32 offset, u8 *buffer,
			 long *size)
{
	log_debug("%s: %x %lx\n", __func__, offset, *size);

	if (!offset)
		memset(data->pmic_part, 0, PMIC_SIZE);

	if (offset + *size > PMIC_SIZE)
		*size = PMIC_SIZE - offset;

	memcpy(&data->pmic_part[offset], buffer, *size);

	return 0;
}

int stm32prog_pmic_read(struct stm32prog_data *data, u32 offset, u8 *buffer,
			long *size)
{
	int result = 0, ret;
	struct udevice *dev;

	if (!CONFIG_IS_ENABLED(PMIC_STPMIC1)) {
		stm32prog_err("PMIC update not supported");

		return -EOPNOTSUPP;
	}

	log_debug("%s: %x %lx\n", __func__, offset, *size);
	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stpmic1_nvm),
					  &dev);
	if (ret)
		return ret;

	/* alway request PMIC for first packet */
	if (!offset) {
		/* init struct with 0 */
		memset(data->pmic_part, 0, PMIC_SIZE);

		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stpmic1_nvm),
						  &dev);
		if (ret)
			return ret;

		ret = misc_read(dev, 0xF8, data->pmic_part, PMIC_SIZE);
		if (ret < 0) {
			result = ret;
			goto end_pmic_read;
		}
		if (ret != PMIC_SIZE) {
			result = -EACCES;
			goto end_pmic_read;
		}
	}

	if (offset + *size > PMIC_SIZE)
		*size = PMIC_SIZE - offset;

	memcpy(buffer, &data->pmic_part[offset], *size);

end_pmic_read:
	log_debug("%s: result %i\n", __func__, result);
	return result;
}

int stm32prog_pmic_start(struct stm32prog_data *data)
{
	int ret;
	struct udevice *dev;

	if (!CONFIG_IS_ENABLED(PMIC_STPMIC1)) {
		stm32prog_err("PMIC update not supported");

		return -EOPNOTSUPP;
	}

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stpmic1_nvm),
					  &dev);
	if (ret)
		return ret;

	return misc_write(dev, 0xF8, data->pmic_part, PMIC_SIZE);
}

/* copy FSBL on NAND to improve reliability on NAND */
static int stm32prog_copy_fsbl(struct stm32prog_part_t *part)
{
	int ret, i;
	void *fsbl;
	struct image_header_s header;
	struct raw_header_s raw_header;
	struct dfu_entity *dfu;
	long size, offset;

	if (part->target != STM32PROG_NAND &&
	    part->target != STM32PROG_SPI_NAND)
		return -1;

	dfu = dfu_get_entity(part->alt_id);

	/* read header */
	dfu_transaction_cleanup(dfu);
	size = BL_HEADER_SIZE;
	ret = dfu->read_medium(dfu, 0, (void *)&raw_header, &size);
	if (ret)
		return ret;
	if (stm32prog_header_check(&raw_header, &header))
		return -1;

	/* read header + payload */
	size = header.image_length + BL_HEADER_SIZE;
	size = round_up(size, part->dev->mtd->erasesize);
	fsbl = calloc(1, size);
	if (!fsbl)
		return -ENOMEM;
	ret = dfu->read_medium(dfu, 0, fsbl, &size);
	log_debug("%s read size=%lx ret=%d\n", __func__, size, ret);
	if (ret)
		goto error;

	dfu_transaction_cleanup(dfu);
	offset = 0;
	for (i = part->bin_nb - 1; i > 0; i--) {
		offset += size;
		/* write to the next erase block */
		ret = dfu->write_medium(dfu, offset, fsbl, &size);
		log_debug("%s copy at ofset=%lx size=%lx ret=%d",
			  __func__, offset, size, ret);
		if (ret)
			goto error;
	}

error:
	free(fsbl);
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

	if (data->cur_part->target == STM32PROG_RAM) {
		if (data->cur_part->part_type == PART_SYSTEM)
			data->uimage = data->cur_part->addr;
		if (data->cur_part->part_type == PART_FILESYSTEM)
			data->dtb = data->cur_part->addr;
	}

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

	if (CONFIG_IS_ENABLED(MTD) &&
	    data->cur_part->bin_nb > 1) {
		if (stm32prog_copy_fsbl(data->cur_part)) {
			stm32prog_err("%s (0x%x): copy of fsbl failed",
				      data->cur_part->name, data->cur_part->id);
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
	data->dfu_seq = 0;
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

static int part_delete(struct stm32prog_data *data,
		       struct stm32prog_part_t *part)
{
	int ret = 0;
	unsigned long blks, blks_offset, blks_size;
	struct blk_desc *block_dev = NULL;
	char cmdbuf[40];
	char devstr[10];

	printf("Erasing %s ", part->name);
	switch (part->target) {
	case STM32PROG_MMC:
		if (!IS_ENABLED(CONFIG_MMC)) {
			ret = -1;
			stm32prog_err("%s (0x%x): erase invalid",
				      part->name, part->id);
			break;
		}
		printf("on mmc %d: ", part->dev->dev_id);
		block_dev = mmc_get_blk_desc(part->dev->mmc);
		blks_offset = lldiv(part->addr, part->dev->mmc->read_bl_len);
		blks_size = lldiv(part->size, part->dev->mmc->read_bl_len);
		/* -1 or -2 : delete boot partition of MMC
		 * need to switch to associated hwpart 1 or 2
		 */
		if (part->part_id < 0)
			if (blk_select_hwpart_devnum(IF_TYPE_MMC,
						     part->dev->dev_id,
						     -part->part_id))
				return -1;

		blks = blk_derase(block_dev, blks_offset, blks_size);

		/* return to user partition */
		if (part->part_id < 0)
			blk_select_hwpart_devnum(IF_TYPE_MMC,
						 part->dev->dev_id, 0);
		if (blks != blks_size) {
			ret = -1;
			stm32prog_err("%s (0x%x): MMC erase failed",
				      part->name, part->id);
		}
		break;
	case STM32PROG_NOR:
	case STM32PROG_NAND:
	case STM32PROG_SPI_NAND:
		if (!IS_ENABLED(CONFIG_MTD)) {
			ret = -1;
			stm32prog_err("%s (0x%x): erase invalid",
				      part->name, part->id);
			break;
		}
		get_mtd_by_target(devstr, part->target, part->dev->dev_id);
		printf("on %s: ", devstr);
		sprintf(cmdbuf, "mtd erase %s 0x%llx 0x%llx",
			devstr, part->addr, part->size);
		if (run_command(cmdbuf, 0)) {
			ret = -1;
			stm32prog_err("%s (0x%x): MTD erase commands failed (%s)",
				      part->name, part->id, cmdbuf);
		}
		break;
	case STM32PROG_RAM:
		printf("on ram: ");
		memset((void *)(uintptr_t)part->addr, 0, (size_t)part->size);
		break;
	default:
		ret = -1;
		stm32prog_err("%s (0x%x): erase invalid", part->name, part->id);
		break;
	}
	if (!ret)
		printf("done\n");

	return ret;
}

static void stm32prog_devices_init(struct stm32prog_data *data)
{
	int i;
	int ret;
	struct stm32prog_part_t *part;

	ret = treat_partition_list(data);
	if (ret)
		goto error;

	/* initialize the selected device */
	for (i = 0; i < data->dev_nb; i++) {
		ret = init_device(data, &data->dev[i]);
		if (ret)
			goto error;
	}

	/* delete RAW partition before create partition */
	for (i = 0; i < data->part_nb; i++) {
		part = &data->part_array[i];

		if (part->part_type != RAW_IMAGE)
			continue;

		if (!IS_SELECT(part) || !IS_DELETE(part))
			continue;

		ret = part_delete(data, part);
		if (ret)
			goto error;
	}

	if (IS_ENABLED(CONFIG_MMC)) {
		ret = create_gpt_partitions(data);
		if (ret)
			goto error;
	}

	/* delete partition GPT or MTD */
	for (i = 0; i < data->part_nb; i++) {
		part = &data->part_array[i];

		if (part->part_type == RAW_IMAGE)
			continue;

		if (!IS_SELECT(part) || !IS_DELETE(part))
			continue;

		ret = part_delete(data, part);
		if (ret)
			goto error;
	}

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
	data->read_phase = PHASE_RESET;
	data->phase = PHASE_FLASHLAYOUT;

	return parse_flash_layout(data, addr, size);
}

void stm32prog_clean(struct stm32prog_data *data)
{
	/* clean */
	dfu_free_entities();
	free(data->part_array);
	free(data->otp_part);
	free(data->buffer);
	free(data->header_data);
}

/* DFU callback: used after serial and direct DFU USB access */
void dfu_flush_callback(struct dfu_entity *dfu)
{
	if (!stm32prog_data)
		return;

	if (dfu->dev_type == DFU_DEV_VIRT) {
		if (dfu->data.virt.dev_num == PHASE_OTP)
			stm32prog_otp_start(stm32prog_data);
		else if (dfu->data.virt.dev_num == PHASE_PMIC)
			stm32prog_pmic_start(stm32prog_data);
		return;
	}

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
		stm32prog_data->dfu_seq = 0;
		log_debug("dfu offset = 0x%llx\n", dfu->offset);
	}
}
