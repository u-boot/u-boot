// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <console.h>
#include <dfu.h>
#include <malloc.h>
#include <dm/uclass.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/sizes.h>

#include "stm32prog.h"

#define OPT_SELECT	BIT(0)
#define OPT_EMPTY	BIT(1)

#define IS_SELECT(part)	((part)->option & OPT_SELECT)
#define IS_EMPTY(part)	((part)->option & OPT_EMPTY)

#define ALT_BUF_LEN			SZ_1K

DECLARE_GLOBAL_DATA_PTR;

char *stm32prog_get_error(struct stm32prog_data *data)
{
	static const char error_msg[] = "Unspecified";

	if (strlen(data->error) == 0)
		strcpy(data->error, error_msg);

	return data->error;
}

static int parse_flash_layout(struct stm32prog_data *data,
			      ulong addr,
			      ulong size)
{
	return -ENODEV;
}

static int __init part_cmp(void *priv, struct list_head *a, struct list_head *b)
{
	struct stm32prog_part_t *parta, *partb;

	parta = container_of(a, struct stm32prog_part_t, list);
	partb = container_of(b, struct stm32prog_part_t, list);

	return parta->addr > partb->addr ? 1 : -1;
}

static int init_device(struct stm32prog_data *data,
		       struct stm32prog_dev_t *dev)
{
	struct blk_desc *block_dev = NULL;
	int part_id;
	u64 first_addr = 0, last_addr = 0;
	struct stm32prog_part_t *part, *next_part;

	switch (dev->target) {
	default:
		stm32prog_err("unknown device type = %d", dev->target);
		return -ENODEV;
	}

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

		dfu_size = part->size;
		offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
				   "raw 0x0 0x%llx", dfu_size);
	} else {
		offset += snprintf(buf + offset,
				   ALT_BUF_LEN - offset,
				   "part");
		offset += snprintf(buf + offset, ALT_BUF_LEN - offset,
				   " %d;", part->part_id);
	}
	switch (part->target) {
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
