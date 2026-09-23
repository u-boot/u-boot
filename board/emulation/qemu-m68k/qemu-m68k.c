// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#include <config.h>
#include <goldfish_rtc.h>
#include <goldfish_timer.h>
#include <goldfish_tty.h>
#include <init.h>
#include <qemu_virt_ctrl.h>
#include <serial.h>
#include <asm-generic/sections.h>
#include <asm/bootinfo.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/platdata.h>
#include <dm/root.h>
#include <linux/errno.h>
#include <linux/sizes.h>
#include <virtio_mmio.h>

DECLARE_GLOBAL_DATA_PTR;

static struct goldfish_tty_plat serial_plat;
static struct goldfish_rtc_plat rtc_plat;
static struct goldfish_timer_plat timer_plat;
static struct qemu_virt_ctrl_plat reset_plat;

#define VIRTIO_MMIO_NUM	128
#define VIRTIO_MMIO_SZ	0x200

static struct virtio_mmio_plat virtio_mmio_plat[VIRTIO_MMIO_NUM];
static char virtio_mmio_names[VIRTIO_MMIO_NUM][11];
static phys_addr_t virtio_mmio_base;

static int create_virtio_mmios(void)
{
	struct driver *drv;
	int i, ret;

	if (!virtio_mmio_base)
		return -ENODEV;

	drv = lists_driver_lookup_name("virtio-mmio");
	if (!drv)
		return -ENOENT;

	for (i = 0; i < VIRTIO_MMIO_NUM; i++) {
		virtio_mmio_plat[i].base = virtio_mmio_base + (VIRTIO_MMIO_SZ * i);
		sprintf(virtio_mmio_names[i], "virtio-%d", i);

		ret = device_bind(dm_root(), drv, virtio_mmio_names[i],
				  &virtio_mmio_plat[i], ofnode_null(), NULL);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Theoretical limit derivation:
 * Max Bootinfo Size (Standard Page) = 4096 bytes
 * Min Record Size (Tag + Size)      = 4 bytes
 * Max Records = 4096 / 4 = 1024
 */
#define MAX_BOOTINFO_RECORDS  1024

static void parse_bootinfo(void)
{
	struct bi_record *record;
	ulong addr;
	int loops = 0;

	/* QEMU places bootinfo after _end, aligned to 2 bytes */
	addr = (ulong)&_end;
	addr = ALIGN(addr, 2);

	record = (struct bi_record *)addr;

	if (record->tag != BI_MACHTYPE)
		return;

	while (record->tag != BI_LAST) {
		phys_addr_t base = record->data[0];

		if (++loops > MAX_BOOTINFO_RECORDS)
			panic("Bootinfo loop exceeded");

		switch (record->tag) {
		case BI_VIRT_GF_TTY_BASE:
			serial_plat.reg = base;
			break;
		case BI_VIRT_GF_RTC_BASE:
			rtc_plat.reg = base;
			timer_plat.reg = base;
			break;
		case BI_VIRT_CTRL_BASE:
			reset_plat.reg = base;
			break;
		case BI_VIRT_VIRTIO_BASE:
			virtio_mmio_base = base;
			break;
		case BI_MEMCHUNK:
			gd->ram_size = record->data[1];
			break;
		}
		record = (struct bi_record *)((ulong)record + record->size);
	}
}

int board_early_init_f(void)
{
	parse_bootinfo();

	return 0;
}

int board_early_init_r(void)
{
	return create_virtio_mmios();
}

int checkboard(void)
{
	puts("Board: QEMU m68k virt\n");

	return 0;
}

int dram_init(void)
{
	/* Default: 16MB */
	if (!gd->ram_size)
		gd->ram_size = SZ_16M;

	return 0;
}

U_BOOT_DRVINFO(goldfish_rtc) = {
	.name = "rtc_goldfish",
	.plat = &rtc_plat,
};

U_BOOT_DRVINFO(goldfish_timer) = {
	.name = "goldfish_timer",
	.plat = &timer_plat,
};

U_BOOT_DRVINFO(goldfish_serial) = {
	.name = "serial_goldfish",
	.plat = &serial_plat,
};

U_BOOT_DRVINFO(sysreset_qemu_virt_ctrl) = {
	.name = "sysreset_qemu_virt_ctrl",
	.plat = &reset_plat,
};
