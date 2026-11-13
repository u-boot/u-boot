// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <asm/global_data.h>
#include <bootstage.h>
#include <bootm.h>
#include <command.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/root.h>
#include <efi.h>
#include <efi_api.h>
#include <fdt_support.h>
#include <hang.h>
#include <image.h>
#include <linux/sizes.h>
#include <log.h>
#include <memalign.h>
#include <u-boot/crc.h>

DECLARE_GLOBAL_DATA_PTR;

static const efi_guid_t efi_guid_fdt = EFI_FDT_GUID;

/* LoongArch do expect a EFI style systab */
static int generate_systab(struct bootm_headers *images)
{
	const int nr_cfgtab = 1;
	struct efi_system_table *systab;
	struct efi_configuration_table *cfgtab;
	size_t table_size = sizeof(struct efi_system_table) +
			    nr_cfgtab * sizeof(struct efi_configuration_table);

	systab = malloc(table_size);
	if (!systab) {
		log_warning("Failed to allocate memory for systab\n");
		return -ENOMEM;
	}
	memset(systab, 0, table_size);

	cfgtab = (void *)systab + sizeof(struct efi_system_table);

	systab->hdr.signature = EFI_SYSTEM_TABLE_SIGNATURE;
	systab->hdr.headersize = sizeof(struct efi_system_table);
	systab->nr_tables = nr_cfgtab;
	systab->tables = cfgtab;
	systab->hdr.crc32 = crc32(0, (const unsigned char *)systab,
				  systab->hdr.headersize);

	cfgtab[0].guid = efi_guid_fdt;
	cfgtab[0].table = images->ft_addr;

	gd->bd->bi_boot_params = (ulong)systab;

	return 0;
}

static void boot_prep_linux(struct bootm_headers *images)
{
	if (CONFIG_IS_ENABLED(OF_LIBFDT) && IS_ENABLED(CONFIG_LMB) && images->ft_len) {
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
		if (generate_systab(images)) {
			printf("Failed to generate EFI systab\n");
			hang();
		}
	} else {
		printf("Device tree not found or missing FDT support\n");
		hang();
	}
}

static void boot_jump_linux(struct bootm_headers *images, int flag)
{
	void (*kernel)(ulong efi_boot, char *cmdline, void *systab);
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);

	kernel = (void (*)(ulong efi_boot, char *cmdline, void *systab))images->ep;

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	debug("## Transferring control to kernel (at address %08lx) ...\n",
	      (ulong)kernel);

	bootm_final(fake);
	cleanup_before_linux();

	if (!fake) {
		if (CONFIG_IS_ENABLED(OF_LIBFDT) && images->ft_len) {
			kernel(0, (char *)images->cmdline_start,
			       (void *)gd->bd->bi_boot_params);
		} else {
			printf("Device tree not found or missing FDT support\n");
			hang();
		}
	}
}

int do_bootm_linux(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;

	if (flag & BOOTM_STATE_OS_BD_T || flag & BOOTM_STATE_OS_CMDLINE)
		return -1;

	if (flag & BOOTM_STATE_OS_PREP) {
		boot_prep_linux(images);
		return 0;
	}

	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		boot_jump_linux(images, flag);
		return 0;
	}

	boot_prep_linux(images);
	boot_jump_linux(images, flag);
	return 0;
}

int do_bootm_vxworks(int flag, struct bootm_info *bmi)
{
	return do_bootm_linux(flag, bmi);
}
