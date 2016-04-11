/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <efi_loader.h>
#include <errno.h>
#include <libfdt.h>
#include <libfdt_env.h>
#include <malloc.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

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

static efi_status_t bootefi_open_dp(void *handle, efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	*protocol_interface = bootefi_device_path;
	return EFI_SUCCESS;
}

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
			.open = &efi_return_handle,
		},
		{
			/*
			 * When asking for the device path interface, return
			 * bootefi_device_path
			 */
			.guid = &efi_guid_device_path,
			.open = &bootefi_open_dp,
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
			.open = &bootefi_open_dp,
		}
	},
};

static void *copy_fdt(void *fdt)
{
	u64 fdt_size = fdt_totalsize(fdt);
	void *new_fdt;

	/* Give us 64kb breathing room */
	fdt_size += 64 * 1024;

	new_fdt = malloc(fdt_size);
	memcpy(new_fdt, fdt, fdt_totalsize(fdt));
	fdt_set_totalsize(new_fdt, fdt_size);

	return new_fdt;
}

/*
 * Load an EFI payload into a newly allocated piece of memory, register all
 * EFI objects it would want to access and jump to it.
 */
static unsigned long do_bootefi_exec(void *efi)
{
	ulong (*entry)(void *image_handle, struct efi_system_table *st);
	ulong fdt_pages, fdt_size, fdt_start, fdt_end;
	bootm_headers_t img = { 0 };
	void *fdt = working_fdt;

	/*
	 * gd lives in a fixed register which may get clobbered while we execute
	 * the payload. So save it here and restore it on every callback entry
	 */
	efi_save_gd();

	/* Update system table to point to our currently loaded FDT */

	/* Fall back to included fdt if none was manually loaded */
	if (!fdt && gd->fdt_blob)
		fdt = (void *)gd->fdt_blob;

	if (fdt) {
		/* Prepare fdt for payload */
		fdt = copy_fdt(fdt);

		if (image_setup_libfdt(&img, fdt, 0, NULL)) {
			printf("ERROR: Failed to process device tree\n");
			return -EINVAL;
		}

		/* Link to it in the efi tables */
		systab.tables[0].guid = EFI_FDT_GUID;
		systab.tables[0].table = fdt;
		systab.nr_tables = 1;

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
		printf("WARNING: No device tree loaded, expect boot to fail\n");
		systab.nr_tables = 0;
	}

	/* Load the EFI payload */
	entry = efi_load_pe(efi, &loaded_image_info);
	if (!entry)
		return -ENOENT;

	/* Initialize and populate EFI object list */
	INIT_LIST_HEAD(&efi_obj_list);
	list_add_tail(&loaded_image_info_obj.link, &efi_obj_list);
	list_add_tail(&bootefi_device_obj.link, &efi_obj_list);
#ifdef CONFIG_PARTITIONS
	efi_disk_register();
#endif
#ifdef CONFIG_LCD
	efi_gop_register();
#endif

	/* Call our payload! */
#ifdef DEBUG_EFI
	printf("%s:%d Jumping to 0x%lx\n", __func__, __LINE__, (long)entry);
#endif
	return entry(&loaded_image_info, &systab);
}


/* Interpreter command to boot an arbitrary EFI image from memory */
static int do_bootefi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *saddr;
	unsigned long addr;
	int r = 0;

	if (argc < 2)
		return 1;
	saddr = argv[1];

	addr = simple_strtoul(saddr, NULL, 16);

	printf("## Starting EFI application at 0x%08lx ...\n", addr);
	r = do_bootefi_exec((void *)addr);
	printf("## Application terminated, r = %d\n", r);

	if (r != 0)
		r = 1;

	return r;
}

#ifdef CONFIG_SYS_LONGHELP
static char bootefi_help_text[] =
	"<image address>\n"
	"  - boot EFI payload stored at address <image address>\n"
	"\n"
	"Since most EFI payloads want to have a device tree provided, please\n"
	"make sure you load a device tree using the fdt addr command before\n"
	"executing bootefi.\n";
#endif

U_BOOT_CMD(
	bootefi, 2, 0, do_bootefi,
	"Boots an EFI payload from memory\n",
	bootefi_help_text
);

void efi_set_bootdev(const char *dev, const char *devnr, const char *path)
{
	__maybe_unused struct blk_desc *desc;
	char devname[32] = { 0 }; /* dp->str is u16[32] long */
	char *colon;

	/* Assemble the condensed device name we use in efi_disk.c */
	snprintf(devname, sizeof(devname), "%s%s", dev, devnr);
	colon = strchr(devname, ':');

#ifdef CONFIG_ISO_PARTITION
	/* For ISOs we create partition block devices */
	desc = blk_get_dev(dev, simple_strtol(devnr, NULL, 10));
	if (desc && (desc->type != DEV_TYPE_UNKNOWN) &&
	    (desc->part_type == PART_TYPE_ISO)) {
		if (!colon)
			snprintf(devname, sizeof(devname), "%s%s:1", dev,
				 devnr);
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
	snprintf(devname, sizeof(devname), "%s", path);
	ascii2unicode(bootefi_image_path[0].str, devname);
}
