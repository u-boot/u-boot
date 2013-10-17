/*
 * Copyright (C) 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#ifdef CONFIG_FAVR32_EZKIT_EXT_FLASH
#include <asm/arch/cacheflush.h>
#include <asm/io.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

flash_info_t flash_info[1];

static void flash_identify(uint16_t *flash, flash_info_t *info)
{
	unsigned long flags;

	flags = disable_interrupts();

	dcache_flush_unlocked();

	writew(0xaa, flash + 0x555);
	writew(0x55, flash + 0xaaa);
	writew(0x90, flash + 0x555);
	info->flash_id = readl(flash);
	writew(0xff, flash);

	readw(flash);

	if (flags)
		enable_interrupts();
}

unsigned long flash_init(void)
{
	unsigned long addr;
	unsigned int i;

	flash_info[0].size = CONFIG_SYS_FLASH_SIZE;
	flash_info[0].sector_count = 135;

	flash_identify(uncached((void *)CONFIG_SYS_FLASH_BASE), &flash_info[0]);

	for (i = 0, addr = 0; i < 8; i++, addr += 0x2000)
		flash_info[0].start[i] = addr;
	for (; i < flash_info[0].sector_count; i++, addr += 0x10000)
		flash_info[0].start[i] = addr;

	return CONFIG_SYS_FLASH_SIZE;
}

void flash_print_info(flash_info_t *info)
{
	printf("Flash: Vendor ID: 0x%02lx, Product ID: 0x%02lx\n",
	       info->flash_id >> 16, info->flash_id & 0xffff);
	printf("Size: %ld MB in %d sectors\n",
	       info->size >> 10, info->sector_count);
}

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	unsigned long flags;
	unsigned long start_time;
	uint16_t *fb, *sb;
	unsigned int i;
	int ret;
	uint16_t status;

	if ((s_first < 0) || (s_first > s_last)
	    || (s_last >= info->sector_count)) {
		puts("Error: first and/or last sector out of range\n");
		return ERR_INVAL;
	}

	for (i = s_first; i < s_last; i++)
		if (info->protect[i]) {
			printf("Error: sector %d is protected\n", i);
			return ERR_PROTECTED;
		}

	fb = (uint16_t *)uncached(info->start[0]);

	dcache_flush_unlocked();

	for (i = s_first; (i <= s_last) && !ctrlc(); i++) {
		printf("Erasing sector %3d...", i);

		sb = (uint16_t *)uncached(info->start[i]);

		flags = disable_interrupts();

		start_time = get_timer(0);

		/* Unlock sector */
		writew(0xaa, fb + 0x555);
		writew(0x70, sb);

		/* Erase sector */
		writew(0xaa, fb + 0x555);
		writew(0x55, fb + 0xaaa);
		writew(0x80, fb + 0x555);
		writew(0xaa, fb + 0x555);
		writew(0x55, fb + 0xaaa);
		writew(0x30, sb);

		/* Wait for completion */
		ret = ERR_OK;
		do {
			/* TODO: Timeout */
			status = readw(sb);
		} while ((status != 0xffff) && !(status & 0x28));

		writew(0xf0, fb);

		/*
		 * Make sure the command actually makes it to the bus
		 * before we re-enable interrupts.
		 */
		readw(fb);

		if (flags)
			enable_interrupts();

		if (status != 0xffff) {
			printf("Flash erase error at address 0x%p: 0x%02x\n",
			       sb, status);
			ret = ERR_PROG_ERROR;
			break;
		}
	}

	if (ctrlc())
		printf("User interrupt!\n");

	return ERR_OK;
}

int write_buff(flash_info_t *info, uchar *src,
			   ulong addr, ulong count)
{
	unsigned long flags;
	uint16_t *base, *p, *s, *end;
	uint16_t word, status, status1;
	int ret = ERR_OK;

	if (addr < info->start[0]
	    || (addr + count) > (info->start[0] + info->size)
	    || (addr + count) < addr) {
		puts("Error: invalid address range\n");
		return ERR_INVAL;
	}

	if (addr & 1 || count & 1 || (unsigned int)src & 1) {
		puts("Error: misaligned source, destination or count\n");
		return ERR_ALIGN;
	}

	base = (uint16_t *)uncached(info->start[0]);
	end = (uint16_t *)uncached(addr + count);

	flags = disable_interrupts();

	dcache_flush_unlocked();
	sync_write_buffer();

	for (p = (uint16_t *)uncached(addr), s = (uint16_t *)src;
	     p < end && !ctrlc(); p++, s++) {
		word = *s;

		writew(0xaa, base + 0x555);
		writew(0x55, base + 0xaaa);
		writew(0xa0, base + 0x555);
		writew(word, p);

		sync_write_buffer();

		/* Wait for completion */
		status1 = readw(p);
		do {
			/* TODO: Timeout */
			status = status1;
			status1 = readw(p);
		} while (((status ^ status1) & 0x40)	/* toggled */
			 && !(status1 & 0x28));		/* error bits */

		/*
		 * We'll need to check once again for toggle bit
		 * because the toggle bit may stop toggling as I/O5
		 * changes to "1" (ref at49bv642.pdf p9)
		 */
		status1 = readw(p);
		status = readw(p);
		if ((status ^ status1) & 0x40) {
			printf("Flash write error at address 0x%p: "
			       "0x%02x != 0x%02x\n",
			       p, status,word);
			ret = ERR_PROG_ERROR;
			writew(0xf0, base);
			readw(base);
			break;
		}

		writew(0xf0, base);
		readw(base);
	}

	if (flags)
		enable_interrupts();

	return ret;
}

#endif /* CONFIG_FAVR32_EZKIT_EXT_FLASH */
