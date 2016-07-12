/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>

static struct list_head mmc_devices;
static int cur_dev_num = -1;

struct mmc *find_mmc_device(int dev_num)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->block_dev.devnum == dev_num)
			return m;
	}

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
	printf("MMC Device %d not found\n", dev_num);
#endif

	return NULL;
}

int mmc_get_next_devnum(void)
{
	return cur_dev_num++;
}

struct blk_desc *mmc_get_blk_desc(struct mmc *mmc)
{
	return &mmc->block_dev;
}

int get_mmc_num(void)
{
	return cur_dev_num;
}

void mmc_do_preinit(void)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
		mmc_set_preinit(m, 1);
#endif
		if (m->preinit)
			mmc_start_init(m);
	}
}

void mmc_list_init(void)
{
	INIT_LIST_HEAD(&mmc_devices);
	cur_dev_num = 0;
}

void mmc_list_add(struct mmc *mmc)
{
	INIT_LIST_HEAD(&mmc->link);

	list_add_tail(&mmc->link, &mmc_devices);
}

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
void print_mmc_devices(char separator)
{
	struct mmc *m;
	struct list_head *entry;
	char *mmc_type;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->has_init)
			mmc_type = IS_SD(m) ? "SD" : "eMMC";
		else
			mmc_type = NULL;

		printf("%s: %d", m->cfg->name, m->block_dev.devnum);
		if (mmc_type)
			printf(" (%s)", mmc_type);

		if (entry->next != &mmc_devices) {
			printf("%c", separator);
			if (separator != '\n')
				puts(" ");
		}
	}

	printf("\n");
}

#else
void print_mmc_devices(char separator) { }
#endif
