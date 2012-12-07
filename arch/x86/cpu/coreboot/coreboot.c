/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/u-boot-x86.h>
#include <flash.h>
#include <netdev.h>
#include <asm/msr.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch-coreboot/tables.h>
#include <asm/arch-coreboot/sysinfo.h>
#include <asm/arch/timestamp.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initializations
 */
int cpu_init_f(void)
{
	int ret = get_coreboot_info(&lib_sysinfo);
	if (ret != 0)
		printf("Failed to parse coreboot tables.\n");

	timestamp_init();

	return ret;
}

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
	/* CPU Speed to 100MHz */
	gd->cpu_clk = 100000000;

	/* Crystal is 33.000MHz */
	gd->bus_clk = 33000000;

	return 0;
}

void show_boot_progress(int val)
{
#if MIN_PORT80_KCLOCKS_DELAY
	static uint32_t prev_stamp;
	static uint32_t base;

	/*
	 * Scale the time counter reading to avoid using 64 bit arithmetics.
	 * Can't use get_timer() here becuase it could be not yet
	 * initialized or even implemented.
	 */
	if (!prev_stamp) {
		base = rdtsc() / 1000;
		prev_stamp = 0;
	} else {
		uint32_t now;

		do {
			now = rdtsc() / 1000 - base;
		} while (now < (prev_stamp + MIN_PORT80_KCLOCKS_DELAY));
		prev_stamp = now;
	}
#endif
	outb(val, 0x80);
}

int last_stage_init(void)
{
	return 0;
}

#ifndef CONFIG_SYS_NO_FLASH
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

#define MTRR_TYPE_WP          5
#define MTRRcap_MSR           0xfe
#define MTRRphysBase_MSR(reg) (0x200 + 2 * (reg))
#define MTRRphysMask_MSR(reg) (0x200 + 2 * (reg) + 1)

int board_final_cleanup(void)
{
	/* Un-cache the ROM so the kernel has one
	 * more MTRR available.
	 *
	 * Coreboot should have assigned this to the
	 * top available variable MTRR.
	 */
	u8 top_mtrr = (native_read_msr(MTRRcap_MSR) & 0xff) - 1;
	u8 top_type = native_read_msr(MTRRphysBase_MSR(top_mtrr)) & 0xff;

	/* Make sure this MTRR is the correct Write-Protected type */
	if (top_type == MTRR_TYPE_WP) {
		disable_caches();
		wrmsrl(MTRRphysBase_MSR(top_mtrr), 0);
		wrmsrl(MTRRphysMask_MSR(top_mtrr), 0);
		enable_caches();
	}

	/* Issue SMI to Coreboot to lock down ME and registers */
	printf("Finalizing Coreboot\n");
	outb(0xcb, 0xb2);

	return 0;
}
