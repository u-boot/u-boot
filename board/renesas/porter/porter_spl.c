/*
 * board/renesas/porter/porter_spl.c
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>

#include <spl.h>

#define TMU0_MSTP125	BIT(25)

#define SD2CKCR		0xE615026C
#define SD_97500KHZ	0x7

void board_init_f(ulong dummy)
{
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD2 to the 97.5MHz as well.
	 */
	writel(SD_97500KHZ, SD2CKCR);
}

void spl_board_init(void)
{
	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
}

void board_boot_order(u32 *spl_boot_list)
{
	/* Boot from SPI NOR with YMODEM UART fallback. */
	spl_boot_list[0] = BOOT_DEVICE_SPI;
	spl_boot_list[1] = BOOT_DEVICE_UART;
	spl_boot_list[2] = BOOT_DEVICE_NONE;
}

void reset_cpu(ulong addr)
{
}
