/*
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * Copyright 2015 ATS Advanced Telematics Systems GmbH
 * Copyright 2015 Konsulko Group, Matt Porter <mporter@konsulko.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>

#define STM32_FLASH_KEY1	0x45670123
#define STM32_FLASH_KEY2	0xcdef89ab

#define STM32_NUM_BANKS	2
#define STM32_MAX_BANK	0x200

flash_info_t flash_info[STM32_NUM_BANKS];
static struct stm32_flash_bank_regs *flash_bank[STM32_NUM_BANKS];

static void stm32f1_flash_lock(u8 bank, u8 lock)
{
	if (lock) {
		setbits_le32(&flash_bank[bank]->cr, STM32_FLASH_CR_LOCK);
	} else {
		writel(STM32_FLASH_KEY1, &flash_bank[bank]->keyr);
		writel(STM32_FLASH_KEY2, &flash_bank[bank]->keyr);
	}
}

/* Only XL devices are supported (2 KiB sector size) */
unsigned long flash_init(void)
{
	u8 i, banks;
	u16 j, size;

	/* Set up accessors for XL devices with wonky register layout */
	flash_bank[0] = (struct stm32_flash_bank_regs *)&STM32_FLASH->keyr;
	flash_bank[1] = (struct stm32_flash_bank_regs *)&STM32_FLASH->keyr2;

	/*
	 * Get total flash size (in KiB) and configure number of banks
	 * present and sector count per bank.
	 */
	size = readw(&STM32_DES->flash_size);
	if (size <= STM32_MAX_BANK) {
		banks = 1;
		flash_info[0].sector_count = size >> 1;
	} else if (size > STM32_MAX_BANK) {
		banks = 2;
		flash_info[0].sector_count = STM32_MAX_BANK >> 1;
		flash_info[1].sector_count = (size - STM32_MAX_BANK) >> 1;
	}

	/* Configure start/size for all sectors */
	for (i = 0; i < banks; i++) {
		flash_info[i].flash_id = FLASH_STM32F1;
		flash_info[i].start[0] = CONFIG_SYS_FLASH_BASE + (i << 19);
		flash_info[i].size = 2048;
		for (j = 1; (j < flash_info[i].sector_count); j++) {
			flash_info[i].start[j] = flash_info[i].start[j - 1]
				+ 2048;
			flash_info[i].size += 2048;
		}
	}

	return size << 10;
}

void flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("Missing or unknown FLASH type\n");
		return;
	} else if (info->flash_id == FLASH_STM32F1) {
		printf("STM32F1 Embedded Flash\n");
	}

	printf("  Size: %ld MB in %d Sectors\n",
	       info->size >> 10, info->sector_count);

	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
		       info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
	return;
}

int flash_erase(flash_info_t *info, int first, int last)
{
	u8 bank = 0xff;
	int i;

	for (i = 0; i < STM32_NUM_BANKS; i++) {
		if (info == &flash_info[i]) {
			bank = i;
			break;
		}
	}
	if (bank == 0xff)
		return -1;

	stm32f1_flash_lock(bank, 0);

	for (i = first; i <= last; i++) {
		while (readl(&flash_bank[bank]->sr) & STM32_FLASH_SR_BSY)
			;

		setbits_le32(&flash_bank[bank]->cr, STM32_FLASH_CR_PER);

		writel(info->start[i], &flash_bank[bank]->ar);

		setbits_le32(&flash_bank[bank]->cr, STM32_FLASH_CR_STRT);

		while (readl(&flash_bank[bank]->sr) & STM32_FLASH_SR_BSY)
			;
	}

	clrbits_le32(&flash_bank[bank]->cr, STM32_FLASH_CR_PER);

	stm32f1_flash_lock(bank, 1);

	return 0;
}

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong i;
	u8 bank = 0xff;

	if (addr & 1) {
		printf("Flash address must be half word aligned\n");
		return -1;
	}

	if (cnt & 1) {
		printf("Flash length must be half word aligned\n");
		return -1;
	}

	for (i = 0; i < 2; i++) {
		if (info == &flash_info[i]) {
			bank = i;
			break;
		}
	}

	if (bank == 0xff)
		return -1;

	while (readl(&flash_bank[bank]->sr) & STM32_FLASH_SR_BSY)
		;

	stm32f1_flash_lock(bank, 0);

	setbits_le32(&flash_bank[bank]->cr, STM32_FLASH_CR_PG);

	/* STM32F1 requires half word writes */
	for (i = 0; i < cnt >> 1; i++) {
		*(u16 *)(addr + i * 2) = ((u16 *)src)[i];
		while (readl(&flash_bank[bank]->sr) & STM32_FLASH_SR_BSY)
			;
	}

	clrbits_le32(&flash_bank[bank]->cr, STM32_FLASH_CR_PG);

	stm32f1_flash_lock(bank, 1);

	return 0;
}
