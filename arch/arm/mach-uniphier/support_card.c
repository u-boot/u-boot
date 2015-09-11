/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <mach/board.h>

#define MICRO_SUPPORT_CARD_RESET	\
				((CONFIG_SUPPORT_CARD_BASE) + 0x000D0034)
#define MICRO_SUPPORT_CARD_REVISION	\
				((CONFIG_SUPPORT_CARD_BASE) + 0x000D00E0)
/*
 * 0: reset deassert, 1: reset
 *
 * bit[0]: LAN, I2C, LED
 * bit[1]: UART
 */
void support_card_reset_deassert(void)
{
	writel(0, MICRO_SUPPORT_CARD_RESET);
}

void support_card_reset(void)
{
	writel(3, MICRO_SUPPORT_CARD_RESET);
}

static int support_card_show_revision(void)
{
	u32 revision;

	revision = readl(MICRO_SUPPORT_CARD_REVISION);
	printf("(CPLD version %d.%d)\n", revision >> 4, revision & 0xf);
	return 0;
}

int check_support_card(void)
{
	printf("SC:    Micro Support Card ");
	return support_card_show_revision();
}

void support_card_init(void)
{
	/*
	 * After power on, we need to keep the LAN controller in reset state
	 * for a while. (200 usec)
	 * Fortunately, enough wait time is already inserted in pll_init()
	 * function. So we do not have to wait here.
	 */
	support_card_reset_deassert();
}

#if defined(CONFIG_SMC911X)
#include <netdev.h>

int board_eth_init(bd_t *bis)
{
	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
}
#endif

#if !defined(CONFIG_SYS_NO_FLASH)

#include <mtd/cfi_flash.h>
#include <mach/sbc-regs.h>

struct memory_bank {
	phys_addr_t base;
	unsigned long size;
};

static int mem_is_flash(const struct memory_bank *mem)
{
	const int loop = 128;
	u32 *scratch_addr;
	u32 saved_value;
	int ret = 1;
	int i;

	/* just in case, use the tail of the memory bank */
	scratch_addr = map_physmem(mem->base + mem->size - sizeof(u32) * loop,
				   sizeof(u32) * loop, MAP_NOCACHE);

	for (i = 0; i < loop; i++, scratch_addr++) {
		saved_value = readl(scratch_addr);
		writel(~saved_value, scratch_addr);
		if (readl(scratch_addr) != saved_value) {
			/* We assume no memory or SRAM here. */
			writel(saved_value, scratch_addr);
			ret = 0;
			break;
		}
	}

	unmap_physmem(scratch_addr, MAP_NOCACHE);

	return ret;
}

/* {address, size} */
static const struct memory_bank memory_banks[] = {
	{0x42000000, 0x01f00000},
};

static const struct memory_bank
*flash_banks_list[CONFIG_SYS_MAX_FLASH_BANKS_DETECT];

phys_addr_t cfi_flash_bank_addr(int i)
{
	return flash_banks_list[i]->base;
}

unsigned long cfi_flash_bank_size(int i)
{
	return flash_banks_list[i]->size;
}

static void detect_num_flash_banks(void)
{
	const struct memory_bank *memory_bank, *end;

	cfi_flash_num_flash_banks = 0;

	memory_bank = memory_banks;
	end = memory_bank + ARRAY_SIZE(memory_banks);

	for (; memory_bank < end; memory_bank++) {
		if (cfi_flash_num_flash_banks >=
		    CONFIG_SYS_MAX_FLASH_BANKS_DETECT)
			break;

		if (mem_is_flash(memory_bank)) {
			flash_banks_list[cfi_flash_num_flash_banks] =
								memory_bank;

			debug("flash bank found: base = 0x%lx, size = 0x%lx\n",
			      memory_bank->base, memory_bank->size);
			cfi_flash_num_flash_banks++;
		}
	}

	debug("number of flash banks: %d\n", cfi_flash_num_flash_banks);
}
#else /* CONFIG_SYS_NO_FLASH */
void detect_num_flash_banks(void)
{
};
#endif /* CONFIG_SYS_NO_FLASH */

void support_card_late_init(void)
{
	detect_num_flash_banks();
}
