// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <efi_loader.h>
#include <env.h>
#include <init.h>
#include <mapmem.h>
#include <net.h>
#include <usb.h>
#include <asm/io.h>
#include "fw.h"
#include "pmic.h"

/* OTP Controller base address and register offsets */
#define EXYNOS850_OTP_BASE		0x10000000
#define OTP_CHIPID0			0x4
#define OTP_CHIPID1			0x8

/* ACPM and PMIC definitions */
#define EXYNOS850_MBOX_APM2AP_BASE	0x11900000
#define EXYNOS850_APM_SRAM_BASE		0x02039000	/* in iRAM */
#define EXYNOS850_APM_SHMEM_OFFSET	0x3200
#define EXYNOS850_IPC_AP_I3C		10

/* LDFW firmware definitions */
#define LDFW_NWD_ADDR			0x88000000
#define EMMC_IFNAME			"mmc"
#define EMMC_DEV_NUM			0
#define EMMC_ESP_PART			1

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = E850_96_FWBL1_IMAGE_GUID,
		.fw_name = u"E850-96-FWBL1",
		.image_index = 1,
	},
	{
		.image_type_id = E850_96_EPBL_IMAGE_GUID,
		.fw_name = u"E850-96-EPBL",
		.image_index = 2,
	},
	{
		.image_type_id = E850_96_BL2_IMAGE_GUID,
		.fw_name = u"E850-96-BL2",
		.image_index = 3,
	},
	{
		.image_type_id = E850_96_BOOTLOADER_IMAGE_GUID,
		.fw_name = u"E850-96-BOOTLOADER",
		.image_index = 4,
	},
	{
		.image_type_id = E850_96_EL3_MON_IMAGE_GUID,
		.fw_name = u"E850-96-EL3-MON",
		.image_index = 5,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "mmc 0="
			"fwbl1.img raw 0x0 0x18 mmcpart 1;"
			"epbl.img raw 0x18 0x98 mmcpart 1;"
			"bl2.img raw 0xb0 0x200 mmcpart 1;"
			"bootloader.img raw 0x438 0x1000 mmcpart 1;"
			"el3_mon.img raw 0x1438 0x200 mmcpart 1",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

static struct acpm acpm = {
	.mbox_base	= (void __iomem *)EXYNOS850_MBOX_APM2AP_BASE,
	.sram_base	= (void __iomem *)(EXYNOS850_APM_SRAM_BASE +
					   EXYNOS850_APM_SHMEM_OFFSET),
	.ipc_ch		= EXYNOS850_IPC_AP_I3C,
};

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

/* Read the unique SoC ID from OTP registers */
static u64 get_chip_id(void)
{
	void __iomem *otp_base;
	u64 val;

	otp_base = map_sysmem(EXYNOS850_OTP_BASE, 12);
	val = readl(otp_base + OTP_CHIPID0);
	val |= (u64)readl(otp_base + OTP_CHIPID1) << 32UL;
	unmap_sysmem(otp_base);

	return val;
}

static void setup_serial(void)
{
	char serial_str[17] = { 0 };
	u64 serial_num;

	if (env_get("serial#"))
		return;

	serial_num = get_chip_id();
	snprintf(serial_str, sizeof(serial_str), "%016llx", serial_num);
	env_set("serial#", serial_str);
}

static void setup_ethaddr(void)
{
	u64 serial_num;
	u32 mac_hi, mac_lo;
	u8 mac_addr[6];

	if (env_get("ethaddr"))
		return;

	serial_num = get_chip_id();
	mac_lo = (u32)serial_num;		/* OTP_CHIPID0 */
	mac_hi = (u32)(serial_num >> 32UL);	/* OTP_CHIPID1 */
	mac_addr[0] = (mac_hi >> 8) & 0xff;
	mac_addr[1] = mac_hi & 0xff;
	mac_addr[2] = (mac_lo >> 24) & 0xff;
	mac_addr[3] = (mac_lo >> 16) & 0xff;
	mac_addr[4] = (mac_lo >> 8) & 0xff;
	mac_addr[5] = mac_lo & 0xff;
	mac_addr[0] &= ~0x1; /* make sure it's not a multicast address */
	if (is_valid_ethaddr(mac_addr))
		eth_env_set_enetaddr("ethaddr", mac_addr);
}

/*
 * Call this in board_late_init() to avoid probing block devices before
 * efi_init_early().
 */
void load_firmware(void)
{
	const char *ifname;
	ulong dev, part;
	int err;

	ifname = env_get("bootdev");
	if (!ifname)
		ifname = EMMC_IFNAME;
	dev = env_get_ulong("bootdevnum", 10, EMMC_DEV_NUM);
	part = env_get_ulong("bootdevpart", 10, EMMC_ESP_PART);

	if (!strcmp(ifname, "usb")) {
		printf("Starting USB (bootdev=usb)...\n");
		err = usb_init();
		if (err)
			return;
	}

	printf("Loading LDFW firmware (from %s %ld)...\n", ifname, dev);
	err = load_ldfw(ifname, dev, part, LDFW_NWD_ADDR);
	if (err)
		printf("ERROR: LDFW loading failed (%d)\n", err);
}

int board_late_init(void)
{
	setup_serial();
	setup_ethaddr();
	load_firmware();

	return 0;
}

int power_init_board(void)
{
	int err;

	err = pmic_init(&acpm);
	if (err)
		printf("ERROR: Failed to configure PMIC (%d)\n", err);

	return 0;
}
