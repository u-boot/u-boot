/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <efi_loader.h>
#include <errno.h>
#include <libfdt.h>
#include <libfdt_env.h>
#include <memalign.h>
#include <asm/global_data.h>
#include <asm-generic/sections.h>
#include <linux/linkage.h>

DECLARE_GLOBAL_DATA_PTR;

static uint8_t efi_obj_list_initalized;

/*
 * When booting using the "bootefi" command, we don't know which
 * physical device the file came from. So we create a pseudo-device
 * called "bootefi" with the device path /bootefi.
 *
 * In addition to the originating device we also declare the file path
 * of "bootefi" based loads to be /bootefi.
 */
static struct efi_device_path_file_path bootefi_image_path[] = {
	{
		.dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE,
		.dp.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH,
		.dp.length = sizeof(bootefi_image_path[0]),
		.str = { 'b','o','o','t','e','f','i' },
	}, {
		.dp.type = DEVICE_PATH_TYPE_END,
		.dp.sub_type = DEVICE_PATH_SUB_TYPE_END,
		.dp.length = sizeof(bootefi_image_path[0]),
	}
};

static struct efi_device_path_file_path bootefi_device_path[] = {
	{
		.dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE,
		.dp.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH,
		.dp.length = sizeof(bootefi_image_path[0]),
		.str = { 'b','o','o','t','e','f','i' },
	}, {
		.dp.type = DEVICE_PATH_TYPE_END,
		.dp.sub_type = DEVICE_PATH_SUB_TYPE_END,
		.dp.length = sizeof(bootefi_image_path[0]),
	}
};

/* The EFI loaded_image interface for the image executed via "bootefi" */
static struct efi_loaded_image loaded_image_info = {
	.device_handle = bootefi_device_path,
	.file_path = bootefi_image_path,
};

/* The EFI object struct for the image executed via "bootefi" */
static struct efi_object loaded_image_info_obj = {
	.handle = &loaded_image_info,
	.protocols = {
		{
			/*
			 * When asking for the loaded_image interface, just
			 * return handle which points to loaded_image_info
			 */
			.guid = &efi_guid_loaded_image,
			.protocol_interface = &loaded_image_info,
		},
		{
			/*
			 * When asking for the device path interface, return
			 * bootefi_device_path
			 */
			.guid = &efi_guid_device_path,
			.protocol_interface = bootefi_device_path,
		},
		{
			.guid = &efi_guid_console_control,
			.protocol_interface = (void *) &efi_console_control
		},
		{
			.guid = &efi_guid_device_path_to_text_protocol,
			.protocol_interface = (void *) &efi_device_path_to_text
		},
	},
};

/* The EFI object struct for the device the "bootefi" image was loaded from */
static struct efi_object bootefi_device_obj = {
	.handle = bootefi_device_path,
	.protocols = {
		{
			/* When asking for the device path interface, return
			 * bootefi_device_path */
			.guid = &efi_guid_device_path,
			.protocol_interface = bootefi_device_path
		}
	},
};

/* Initialize and populate EFI object list */
static void efi_init_obj_list(void)
{
	efi_obj_list_initalized = 1;

	list_add_tail(&loaded_image_info_obj.link, &efi_obj_list);
	list_add_tail(&bootefi_device_obj.link, &efi_obj_list);
	efi_console_register();
#ifdef CONFIG_PARTITIONS
	efi_disk_register();
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_DM_VIDEO)
	efi_gop_register();
#endif
#ifdef CONFIG_NET
	void *nethandle = loaded_image_info.device_handle;
	efi_net_register(&nethandle);

	if (!memcmp(bootefi_device_path[0].str, "N\0e\0t", 6))
		loaded_image_info.device_handle = nethandle;
	else
		loaded_image_info.device_handle = bootefi_device_path;
#endif
#ifdef CONFIG_GENERATE_SMBIOS_TABLE
	efi_smbios_register();
#endif

	/* Initialize EFI runtime services */
	efi_reset_system_init();
	efi_get_time_init();
}

static void *copy_fdt(void *fdt)
{
	u64 fdt_size = fdt_totalsize(fdt);
	unsigned long fdt_ram_start = -1L, fdt_pages;
	u64 new_fdt_addr;
	void *new_fdt;
	int i;

        for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
                u64 ram_start = gd->bd->bi_dram[i].start;
                u64 ram_size = gd->bd->bi_dram[i].size;

		if (!ram_size)
			continue;

		if (ram_start < fdt_ram_start)
			fdt_ram_start = ram_start;
	}

	/* Give us at least 4kb breathing room */
	fdt_size = ALIGN(fdt_size + 4096, EFI_PAGE_SIZE);
	fdt_pages = fdt_size >> EFI_PAGE_SHIFT;

	/* Safe fdt location is at 128MB */
	new_fdt_addr = fdt_ram_start + (128 * 1024 * 1024) + fdt_size;
	if (efi_allocate_pages(1, EFI_BOOT_SERVICES_DATA, fdt_pages,
			       &new_fdt_addr) != EFI_SUCCESS) {
		/* If we can't put it there, put it somewhere */
		new_fdt_addr = (ulong)memalign(EFI_PAGE_SIZE, fdt_size);
		if (efi_allocate_pages(1, EFI_BOOT_SERVICES_DATA, fdt_pages,
				       &new_fdt_addr) != EFI_SUCCESS) {
			printf("ERROR: Failed to reserve space for FDT\n");
			return NULL;
		}
	}

	new_fdt = (void*)(ulong)new_fdt_addr;
	memcpy(new_fdt, fdt, fdt_totalsize(fdt));
	fdt_set_totalsize(new_fdt, fdt_size);

	return new_fdt;
}

static ulong efi_do_enter(void *image_handle,
			  struct efi_system_table *st,
			  asmlinkage ulong (*entry)(void *image_handle,
				struct efi_system_table *st))
{
	efi_status_t ret = EFI_LOAD_ERROR;

	if (entry)
		ret = entry(image_handle, st);
	st->boottime->exit(image_handle, ret, 0, NULL);
	return ret;
}

#ifdef CONFIG_ARM64
static unsigned long efi_run_in_el2(asmlinkage ulong (*entry)(
			void *image_handle, struct efi_system_table *st),
			void *image_handle, struct efi_system_table *st)
{
	/* Enable caches again */
	dcache_enable();

	return efi_do_enter(image_handle, st, entry);
}
#endif

/*
 * Load an EFI payload into a newly allocated piece of memory, register all
 * EFI objects it would want to access and jump to it.
 */
static unsigned long do_bootefi_exec(void *efi, void *fdt)
{
	ulong (*entry)(void *image_handle, struct efi_system_table *st)
		asmlinkage;
	ulong fdt_pages, fdt_size, fdt_start, fdt_end;
	const efi_guid_t fdt_guid = EFI_FDT_GUID;
	bootm_headers_t img = { 0 };

	/*
	 * gd lives in a fixed register which may get clobbered while we execute
	 * the payload. So save it here and restore it on every callback entry
	 */
	efi_save_gd();

	if (fdt && !fdt_check_header(fdt)) {
		/* Prepare fdt for payload */
		fdt = copy_fdt(fdt);

		if (image_setup_libfdt(&img, fdt, 0, NULL)) {
			printf("ERROR: Failed to process device tree\n");
			return -EINVAL;
		}

		/* Link to it in the efi tables */
		efi_install_configuration_table(&fdt_guid, fdt);

		/* And reserve the space in the memory map */
		fdt_start = ((ulong)fdt) & ~EFI_PAGE_MASK;
		fdt_end = ((ulong)fdt) + fdt_totalsize(fdt);
		fdt_size = (fdt_end - fdt_start) + EFI_PAGE_MASK;
		fdt_pages = fdt_size >> EFI_PAGE_SHIFT;
		/* Give a bootloader the chance to modify the device tree */
		fdt_pages += 2;
		efi_add_memory_map(fdt_start, fdt_pages,
				   EFI_BOOT_SERVICES_DATA, true);
	} else {
		printf("WARNING: Invalid device tree, expect boot to fail\n");
		efi_install_configuration_table(&fdt_guid, NULL);
	}

	/* Load the EFI payload */
	entry = efi_load_pe(efi, &loaded_image_info);
	if (!entry)
		return -ENOENT;

	/* Initialize and populate EFI object list */
	if (!efi_obj_list_initalized)
		efi_init_obj_list();

	/* Call our payload! */
	debug("%s:%d Jumping to 0x%lx\n", __func__, __LINE__, (long)entry);

	if (setjmp(&loaded_image_info.exit_jmp)) {
		return loaded_image_info.exit_status;
	}

#ifdef CONFIG_ARM64
	/* On AArch64 we need to make sure we call our payload in < EL3 */
	if (current_el() == 3) {
		smp_kick_all_cpus();
		dcache_disable();	/* flush cache before switch to EL2 */

		/* Move into EL2 and keep running there */
		armv8_switch_to_el2((ulong)entry, (ulong)&loaded_image_info,
				    (ulong)&systab, 0, (ulong)efi_run_in_el2,
				    ES_TO_AARCH64);

		/* Should never reach here, efi exits with longjmp */
		while (1) { }
	}
#endif

	return efi_do_enter(&loaded_image_info, &systab, entry);
}


/* Interpreter command to boot an arbitrary EFI image from memory */
static int do_bootefi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *saddr, *sfdt;
	unsigned long addr, fdt_addr = 0;
	unsigned long r;

	if (argc < 2)
		return CMD_RET_USAGE;
#ifdef CONFIG_CMD_BOOTEFI_HELLO
	if (!strcmp(argv[1], "hello")) {
		ulong size = __efi_hello_world_end - __efi_hello_world_begin;

		saddr = env_get("loadaddr");
		if (saddr)
			addr = simple_strtoul(saddr, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;
		memcpy((char *)addr, __efi_hello_world_begin, size);
	} else
#endif
	{
		saddr = argv[1];

		addr = simple_strtoul(saddr, NULL, 16);

		if (argc > 2) {
			sfdt = argv[2];
			fdt_addr = simple_strtoul(sfdt, NULL, 16);
		}
	}

	printf("## Starting EFI application at %08lx ...\n", addr);
	r = do_bootefi_exec((void *)addr, (void*)fdt_addr);
	printf("## Application terminated, r = %lu\n",
	       r & ~EFI_ERROR_MASK);

	if (r != EFI_SUCCESS)
		return 1;
	else
		return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char bootefi_help_text[] =
	"<image address> [fdt address]\n"
	"  - boot EFI payload stored at address <image address>.\n"
	"    If specified, the device tree located at <fdt address> gets\n"
	"    exposed as EFI configuration table.\n"
#ifdef CONFIG_CMD_BOOTEFI_HELLO
	"hello\n"
	"  - boot a sample Hello World application stored within U-Boot"
#endif
	;
#endif

U_BOOT_CMD(
	bootefi, 3, 0, do_bootefi,
	"Boots an EFI payload from memory",
	bootefi_help_text
);

void efi_set_bootdev(const char *dev, const char *devnr, const char *path)
{
	__maybe_unused struct blk_desc *desc;
	char devname[32] = { 0 }; /* dp->str is u16[32] long */
	char *colon, *s;

#if defined(CONFIG_BLK) || CONFIG_IS_ENABLED(ISO_PARTITION)
	desc = blk_get_dev(dev, simple_strtol(devnr, NULL, 10));
#endif

#ifdef CONFIG_BLK
	if (desc) {
		snprintf(devname, sizeof(devname), "%s", desc->bdev->name);
	} else
#endif

	{
		/* Assemble the condensed device name we use in efi_disk.c */
		snprintf(devname, sizeof(devname), "%s%s", dev, devnr);
	}

	colon = strchr(devname, ':');

#if CONFIG_IS_ENABLED(ISO_PARTITION)
	/* For ISOs we create partition block devices */
	if (desc && (desc->type != DEV_TYPE_UNKNOWN) &&
	    (desc->part_type == PART_TYPE_ISO)) {
		if (!colon)
			snprintf(devname, sizeof(devname), "%s:1", devname);

		colon = NULL;
	}
#endif

	if (colon)
		*colon = '\0';

	/* Patch bootefi_device_path to the target device */
	memset(bootefi_device_path[0].str, 0, sizeof(bootefi_device_path[0].str));
	ascii2unicode(bootefi_device_path[0].str, devname);

	/* Patch bootefi_image_path to the target file path */
	memset(bootefi_image_path[0].str, 0, sizeof(bootefi_image_path[0].str));
	if (strcmp(dev, "Net")) {
		/* Add leading / to fs paths, because they're absolute */
		snprintf(devname, sizeof(devname), "/%s", path);
	} else {
		snprintf(devname, sizeof(devname), "%s", path);
	}
	/* DOS style file path: */
	s = devname;
	while ((s = strchr(s, '/')))
		*s++ = '\\';
	ascii2unicode(bootefi_image_path[0].str, devname);
}
