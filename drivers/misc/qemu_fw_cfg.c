/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <malloc.h>
#include <qemu_fw_cfg.h>
#include <asm/io.h>
#include <linux/list.h>

static bool fwcfg_present;
static bool fwcfg_dma_present;

static LIST_HEAD(fw_list);

/* Read configuration item using fw_cfg PIO interface */
static void qemu_fwcfg_read_entry_pio(uint16_t entry,
		uint32_t size, void *address)
{
	uint32_t i = 0;
	uint8_t *data = address;

	/*
	 * writting FW_CFG_INVALID will cause read operation to resume at
	 * last offset, otherwise read will start at offset 0
	 */
	if (entry != FW_CFG_INVALID)
		outw(entry, FW_CONTROL_PORT);
	while (size--)
		data[i++] = inb(FW_DATA_PORT);
}

/* Read configuration item using fw_cfg DMA interface */
static void qemu_fwcfg_read_entry_dma(uint16_t entry,
		uint32_t size, void *address)
{
	struct fw_cfg_dma_access dma;

	dma.length = cpu_to_be32(size);
	dma.address = cpu_to_be64((uintptr_t)address);
	dma.control = cpu_to_be32(FW_CFG_DMA_READ);

	/*
	 * writting FW_CFG_INVALID will cause read operation to resume at
	 * last offset, otherwise read will start at offset 0
	 */
	if (entry != FW_CFG_INVALID)
		dma.control |= cpu_to_be32(FW_CFG_DMA_SELECT | (entry << 16));

	barrier();

	debug("qemu_fwcfg_dma_read_entry: addr %p, length %u control 0x%x\n",
	      address, size, be32_to_cpu(dma.control));

	outl(cpu_to_be32((uint32_t)&dma), FW_DMA_PORT_HIGH);

	while (be32_to_cpu(dma.control) & ~FW_CFG_DMA_ERROR)
		__asm__ __volatile__ ("pause");
}

bool qemu_fwcfg_present(void)
{
	return fwcfg_present;
}

bool qemu_fwcfg_dma_present(void)
{
	return fwcfg_dma_present;
}

void qemu_fwcfg_read_entry(uint16_t entry, uint32_t length, void *address)
{
	if (fwcfg_dma_present)
		qemu_fwcfg_read_entry_dma(entry, length, address);
	else
		qemu_fwcfg_read_entry_pio(entry, length, address);
}

int qemu_fwcfg_online_cpus(void)
{
	uint16_t nb_cpus;

	if (!fwcfg_present)
		return -ENODEV;

	qemu_fwcfg_read_entry(FW_CFG_NB_CPUS, 2, &nb_cpus);

	return le16_to_cpu(nb_cpus);
}

int qemu_fwcfg_read_firmware_list(void)
{
	int i;
	uint32_t count;
	struct fw_file *file;
	struct list_head *entry;

	/* don't read it twice */
	if (!list_empty(&fw_list))
		return 0;

	qemu_fwcfg_read_entry(FW_CFG_FILE_DIR, 4, &count);
	if (!count)
		return 0;

	count = be32_to_cpu(count);
	for (i = 0; i < count; i++) {
		file = malloc(sizeof(*file));
		if (!file) {
			printf("error: allocating resource\n");
			goto err;
		}
		qemu_fwcfg_read_entry(FW_CFG_INVALID,
				      sizeof(struct fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &fw_list);
	}

	return 0;

err:
	list_for_each(entry, &fw_list) {
		file = list_entry(entry, struct fw_file, list);
		free(file);
	}

	return -ENOMEM;
}

struct fw_file *qemu_fwcfg_find_file(const char *name)
{
	struct list_head *entry;
	struct fw_file *file;

	list_for_each(entry, &fw_list) {
		file = list_entry(entry, struct fw_file, list);
		if (!strcmp(file->cfg.name, name))
			return file;
	}

	return NULL;
}

struct fw_file *qemu_fwcfg_file_iter_init(struct fw_cfg_file_iter *iter)
{
	iter->entry = fw_list.next;
	return list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

struct fw_file *qemu_fwcfg_file_iter_next(struct fw_cfg_file_iter *iter)
{
	iter->entry = ((struct list_head *)iter->entry)->next;
	return list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

bool qemu_fwcfg_file_iter_end(struct fw_cfg_file_iter *iter)
{
	return iter->entry == &fw_list;
}

void qemu_fwcfg_init(void)
{
	uint32_t qemu;
	uint32_t dma_enabled;

	fwcfg_present = false;
	fwcfg_dma_present = false;

	qemu_fwcfg_read_entry_pio(FW_CFG_SIGNATURE, 4, &qemu);
	if (be32_to_cpu(qemu) == QEMU_FW_CFG_SIGNATURE)
		fwcfg_present = true;

	if (fwcfg_present) {
		qemu_fwcfg_read_entry_pio(FW_CFG_ID, 1, &dma_enabled);
		if (dma_enabled & FW_CFG_DMA_ENABLED)
			fwcfg_dma_present = true;
	}
}
