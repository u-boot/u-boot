/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/board.h>

#if defined(CONFIG_PFC_MICRO_SUPPORT_CARD)

#define PFC_MICRO_SUPPORT_CARD_RESET	\
				((CONFIG_SUPPORT_CARD_BASE) + 0x000D0034)
#define PFC_MICRO_SUPPORT_CARD_REVISION	\
				((CONFIG_SUPPORT_CARD_BASE) + 0x000D00E0)
/*
 * 0: reset deassert, 1: reset
 *
 * bit[0]: LAN, I2C, LED
 * bit[1]: UART
 */
void support_card_reset_deassert(void)
{
	writel(0, PFC_MICRO_SUPPORT_CARD_RESET);
}

void support_card_reset(void)
{
	writel(3, PFC_MICRO_SUPPORT_CARD_RESET);
}

static int support_card_show_revision(void)
{
	u32 revision;

	revision = readl(PFC_MICRO_SUPPORT_CARD_REVISION);
	printf("(PFC CPLD version %d.%d)\n", revision >> 4, revision & 0xf);
	return 0;
}
#endif

#if defined(CONFIG_DCC_MICRO_SUPPORT_CARD)

#define DCC_MICRO_SUPPORT_CARD_RESET_LAN	\
				((CONFIG_SUPPORT_CARD_BASE) + 0x00401300)
#define DCC_MICRO_SUPPORT_CARD_RESET_UART	\
				((CONFIG_SUPPORT_CARD_BASE) + 0x00401304)
#define DCC_MICRO_SUPPORT_CARD_RESET_I2C	\
				((CONFIG_SUPPORT_CARD_BASE) + 0x00401308)
#define DCC_MICRO_SUPPORT_CARD_REVISION		\
				((CONFIG_SUPPORT_CARD_BASE) + 0x005000E0)

void support_card_reset_deassert(void)
{
	writel(1, DCC_MICRO_SUPPORT_CARD_RESET_LAN); /* LAN and LED */
	writel(1, DCC_MICRO_SUPPORT_CARD_RESET_UART); /* UART */
	writel(1, DCC_MICRO_SUPPORT_CARD_RESET_I2C); /* I2C */
}

void support_card_reset(void)
{
	writel(0, DCC_MICRO_SUPPORT_CARD_RESET_LAN); /* LAN and LED */
	writel(0, DCC_MICRO_SUPPORT_CARD_RESET_UART); /* UART */
	writel(0, DCC_MICRO_SUPPORT_CARD_RESET_I2C); /* I2C */
}

static int support_card_show_revision(void)
{
	u32 revision;

	revision = readl(DCC_MICRO_SUPPORT_CARD_REVISION);

	if (revision >= 0x67) {
		printf("(DCC CPLD version 3.%d.%d)\n",
		       revision >> 4, revision & 0xf);
		return 0;
	} else {
		printf("(DCC CPLD unknown version)\n");
		return -1;
	}
}
#endif

void support_card_init(void)
{
	/*
	 * After power on, we need to keep the LAN controller in reset state
	 * for a while. (200 usec)
	 * Fortunatelly, enough wait time is already inserted in pll_init()
	 * function. So we do not have to wait here.
	 */
	support_card_reset_deassert();
}

int check_support_card(void)
{
	printf("SC:    Micro Support Card ");
	return support_card_show_revision();
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

#if CONFIG_SYS_MAX_FLASH_BANKS > 1
static phys_addr_t flash_banks_list[CONFIG_SYS_MAX_FLASH_BANKS] =
					CONFIG_SYS_FLASH_BANKS_LIST;

phys_addr_t cfi_flash_bank_addr(int i)
{
	return flash_banks_list[i];
}
#endif

int mem_is_flash(phys_addr_t base)
{
	const int loop = 128;
	u32 *scratch_addr;
	u32 saved_value;
	int ret = 1;
	int i;

	scratch_addr = map_physmem(base + 0x01e00000,
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

int board_flash_wp_on(void)
{
	int i;
	int ret = 1;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		if (mem_is_flash(cfi_flash_bank_addr(i))) {
			/*
			 * We found at least one flash.
			 * We need to return 0 and call flash_init().
			 */
			ret = 0;
		}
#if CONFIG_SYS_MAX_FLASH_BANKS > 1
		else {
			/*
			 * We might have a SRAM here.
			 * To prevent SRAM data from being destroyed,
			 * we set dummy address (SDRAM).
			 */
			flash_banks_list[i] = 0x80000000 + 0x10000 * i;
		}
#endif
	}

	return ret;
}
#endif
