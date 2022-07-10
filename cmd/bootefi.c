// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <bootm.h>
#include <charset.h>
#include <command.h>
#include <dm.h>
#include <efi_loader.h>
#include <efi_selftest.h>
#include <env.h>
#include <errno.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/libfdt_env.h>
#include <mapmem.h>
#include <memalign.h>
#include <asm-generic/sections.h>
#include <linux/linkage.h>

DECLARE_GLOBAL_DATA_PTR;

static struct efi_device_path *bootefi_image_path;
static struct efi_device_path *bootefi_device_path;
static void *image_addr;
static size_t image_size;

/**
 * efi_get_image_parameters() - return image parameters
 *
 * @img_addr:		address of loaded image in memory
 * @img_size:		size of loaded image
 */
void efi_get_image_parameters(void **img_addr, size_t *img_size)
{
	*img_addr = image_addr;
	*img_size = image_size;
}

/**
 * efi_clear_bootdev() - clear boot device
 */
static void efi_clear_bootdev(void)
{
	efi_free_pool(bootefi_device_path);
	efi_free_pool(bootefi_image_path);
	bootefi_device_path = NULL;
	bootefi_image_path = NULL;
	image_addr = NULL;
	image_size = 0;
}

/**
 * efi_set_bootdev() - set boot device
 *
 * This function is called when a file is loaded, e.g. via the 'load' command.
 * We use the path to this file to inform the UEFI binary about the boot device.
 *
 * @dev:		device, e.g. "MMC"
 * @devnr:		number of the device, e.g. "1:2"
 * @path:		path to file loaded
 * @buffer:		buffer with file loaded
 * @buffer_size:	size of file loaded
 */
void efi_set_bootdev(const char *dev, const char *devnr, const char *path,
		     void *buffer, size_t buffer_size)
{
	struct efi_device_path *device, *image;
	efi_status_t ret;

	log_debug("dev=%s, devnr=%s, path=%s, buffer=%p, size=%zx\n", dev,
		  devnr, path, buffer, buffer_size);

	/* Forget overwritten image */
	if (buffer + buffer_size >= image_addr &&
	    image_addr + image_size >= buffer)
		efi_clear_bootdev();

	/* Remember only PE-COFF and FIT images */
	if (efi_check_pe(buffer, buffer_size, NULL) != EFI_SUCCESS) {
		if (IS_ENABLED(CONFIG_FIT) &&
		    !fit_check_format(buffer, IMAGE_SIZE_INVAL)) {
			/*
			 * FIT images of type EFI_OS are started via command
			 * bootm. We should not use their boot device with the
			 * bootefi command.
			 */
			buffer = 0;
			buffer_size = 0;
		} else {
			log_debug("- not remembering image\n");
			return;
		}
	}

	/* efi_set_bootdev() is typically called repeatedly, recover memory */
	efi_clear_bootdev();

	image_addr = buffer;
	image_size = buffer_size;

	ret = efi_dp_from_name(dev, devnr, path, &device, &image);
	if (ret == EFI_SUCCESS) {
		bootefi_device_path = device;
		if (image) {
			/* FIXME: image should not contain device */
			struct efi_device_path *image_tmp = image;

			efi_dp_split_file_path(image, &device, &image);
			efi_free_pool(image_tmp);
		}
		bootefi_image_path = image;
		log_debug("- boot device %pD\n", device);
		if (image)
			log_debug("- image %pD\n", image);
	} else {
		log_debug("- efi_dp_from_name() failed, err=%lx\n", ret);
		efi_clear_bootdev();
	}
}

/**
 * efi_env_set_load_options() - set load options from environment variable
 *
 * @handle:		the image handle
 * @env_var:		name of the environment variable
 * @load_options:	pointer to load options (output)
 * Return:		status code
 */
static efi_status_t efi_env_set_load_options(efi_handle_t handle,
					     const char *env_var,
					     u16 **load_options)
{
	const char *env = env_get(env_var);
	size_t size;
	u16 *pos;
	efi_status_t ret;

	*load_options = NULL;
	if (!env)
		return EFI_SUCCESS;
	size = sizeof(u16) * (utf8_utf16_strlen(env) + 1);
	pos = calloc(size, 1);
	if (!pos)
		return EFI_OUT_OF_RESOURCES;
	*load_options = pos;
	utf8_utf16_strcpy(&pos, env);
	ret = efi_set_load_options(handle, size, *load_options);
	if (ret != EFI_SUCCESS) {
		free(*load_options);
		*load_options = NULL;
	}
	return ret;
}

#if !CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)

/**
 * copy_fdt() - Copy the device tree to a new location available to EFI
 *
 * The FDT is copied to a suitable location within the EFI memory map.
 * Additional 12 KiB are added to the space in case the device tree needs to be
 * expanded later with fdt_open_into().
 *
 * @fdtp:	On entry a pointer to the flattened device tree.
 *		On exit a pointer to the copy of the flattened device tree.
 *		FDT start
 * Return:	status code
 */
static efi_status_t copy_fdt(void **fdtp)
{
	unsigned long fdt_ram_start = -1L, fdt_pages;
	efi_status_t ret = 0;
	void *fdt, *new_fdt;
	u64 new_fdt_addr;
	uint fdt_size;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		u64 ram_start = gd->bd->bi_dram[i].start;
		u64 ram_size = gd->bd->bi_dram[i].size;

		if (!ram_size)
			continue;

		if (ram_start < fdt_ram_start)
			fdt_ram_start = ram_start;
	}

	/*
	 * Give us at least 12 KiB of breathing room in case the device tree
	 * needs to be expanded later.
	 */
	fdt = *fdtp;
	fdt_pages = efi_size_in_pages(fdt_totalsize(fdt) + 0x3000);
	fdt_size = fdt_pages << EFI_PAGE_SHIFT;

	/*
	 * Safe fdt location is at 127 MiB.
	 * On the sandbox convert from the sandbox address space.
	 */
	new_fdt_addr = (uintptr_t)map_sysmem(fdt_ram_start + 0x7f00000 +
					     fdt_size, 0);
	ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
				 EFI_ACPI_RECLAIM_MEMORY, fdt_pages,
				 &new_fdt_addr);
	if (ret != EFI_SUCCESS) {
		/* If we can't put it there, put it somewhere */
		new_fdt_addr = (ulong)memalign(EFI_PAGE_SIZE, fdt_size);
		ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
					 EFI_ACPI_RECLAIM_MEMORY, fdt_pages,
					 &new_fdt_addr);
		if (ret != EFI_SUCCESS) {
			log_err("ERROR: Failed to reserve space for FDT\n");
			goto done;
		}
	}
	new_fdt = (void *)(uintptr_t)new_fdt_addr;
	memcpy(new_fdt, fdt, fdt_totalsize(fdt));
	fdt_set_totalsize(new_fdt, fdt_size);

	*fdtp = (void *)(uintptr_t)new_fdt_addr;
done:
	return ret;
}

/**
 * get_config_table() - get configuration table
 *
 * @guid:	GUID of the configuration table
 * Return:	pointer to configuration table or NULL
 */
static void *get_config_table(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(guid, &systab.tables[i].guid))
			return systab.tables[i].table;
	}
	return NULL;
}

#endif /* !CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE) */

/**
 * efi_install_fdt() - install device tree
 *
 * If fdt is not EFI_FDT_USE_INTERNAL, the device tree located at that memory
 * address will will be installed as configuration table, otherwise the device
 * tree located at the address indicated by environment variable fdt_addr or as
 * fallback fdtcontroladdr will be used.
 *
 * On architectures using ACPI tables device trees shall not be installed as
 * configuration table.
 *
 * @fdt:	address of device tree or EFI_FDT_USE_INTERNAL to use the
 *		the hardware device tree as indicated by environment variable
 *		fdt_addr or as fallback the internal device tree as indicated by
 *		the environment variable fdtcontroladdr
 * Return:	status code
 */
efi_status_t efi_install_fdt(void *fdt)
{
	/*
	 * The EBBR spec requires that we have either an FDT or an ACPI table
	 * but not both.
	 */
#if CONFIG_IS_ENABLED(GENERATE_ACPI_TABLE)
	if (fdt) {
		log_warning("WARNING: Can't have ACPI table and device tree - ignoring DT.\n");
		return EFI_SUCCESS;
	}
#else
	bootm_headers_t img = { 0 };
	efi_status_t ret;

	if (fdt == EFI_FDT_USE_INTERNAL) {
		const char *fdt_opt;
		uintptr_t fdt_addr;

		/* Look for device tree that is already installed */
		if (get_config_table(&efi_guid_fdt))
			return EFI_SUCCESS;
		/* Check if there is a hardware device tree */
		fdt_opt = env_get("fdt_addr");
		/* Use our own device tree as fallback */
		if (!fdt_opt) {
			fdt_opt = env_get("fdtcontroladdr");
			if (!fdt_opt) {
				log_err("ERROR: need device tree\n");
				return EFI_NOT_FOUND;
			}
		}
		fdt_addr = hextoul(fdt_opt, NULL);
		if (!fdt_addr) {
			log_err("ERROR: invalid $fdt_addr or $fdtcontroladdr\n");
			return EFI_LOAD_ERROR;
		}
		fdt = map_sysmem(fdt_addr, 0);
	}

	/* Install device tree */
	if (fdt_check_header(fdt)) {
		log_err("ERROR: invalid device tree\n");
		return EFI_LOAD_ERROR;
	}

	/* Prepare device tree for payload */
	ret = copy_fdt(&fdt);
	if (ret) {
		log_err("ERROR: out of memory\n");
		return EFI_OUT_OF_RESOURCES;
	}

	if (image_setup_libfdt(&img, fdt, 0, NULL)) {
		log_err("ERROR: failed to process device tree\n");
		return EFI_LOAD_ERROR;
	}

	/* Create memory reservations as indicated by the device tree */
	efi_carve_out_dt_rsv(fdt);

	efi_try_purge_kaslr_seed(fdt);

	/* Install device tree as UEFI table */
	ret = efi_install_configuration_table(&efi_guid_fdt, fdt);
	if (ret != EFI_SUCCESS) {
		log_err("ERROR: failed to install device tree\n");
		return ret;
	}
#endif /* GENERATE_ACPI_TABLE */

	return EFI_SUCCESS;
}

/**
 * do_bootefi_exec() - execute EFI binary
 *
 * The image indicated by @handle is started. When it returns the allocated
 * memory for the @load_options is freed.
 *
 * @handle:		handle of loaded image
 * @load_options:	load options
 * Return:		status code
 *
 * Load the EFI binary into a newly assigned memory unwinding the relocation
 * information, install the loaded image protocol, and call the binary.
 */
static efi_status_t do_bootefi_exec(efi_handle_t handle, void *load_options)
{
	efi_status_t ret;
	efi_uintn_t exit_data_size = 0;
	u16 *exit_data = NULL;

	/* On ARM switch from EL3 or secure mode to EL2 or non-secure mode */
	switch_to_non_secure_mode();

	/*
	 * The UEFI standard requires that the watchdog timer is set to five
	 * minutes when invoking an EFI boot option.
	 *
	 * Unified Extensible Firmware Interface (UEFI), version 2.7 Errata A
	 * 7.5. Miscellaneous Boot Services - EFI_BOOT_SERVICES.SetWatchdogTimer
	 */
	ret = efi_set_watchdog(300);
	if (ret != EFI_SUCCESS) {
		log_err("ERROR: Failed to set watchdog timer\n");
		goto out;
	}

	/* Call our payload! */
	ret = EFI_CALL(efi_start_image(handle, &exit_data_size, &exit_data));
	if (ret != EFI_SUCCESS) {
		log_err("## Application failed, r = %lu\n",
			ret & ~EFI_ERROR_MASK);
		if (exit_data) {
			log_err("## %ls\n", exit_data);
			efi_free_pool(exit_data);
		}
	}

	efi_restore_gd();

out:
	free(load_options);

	if (IS_ENABLED(CONFIG_EFI_LOAD_FILE2_INITRD))
		efi_initrd_deregister();

	/* Control is returned to U-Boot, disable EFI watchdog */
	efi_set_watchdog(0);

	return ret;
}

/**
 * do_efibootmgr() - execute EFI boot manager
 *
 * Return:	status code
 */
static int do_efibootmgr(void)
{
	efi_handle_t handle;
	efi_status_t ret;
	void *load_options;

	ret = efi_bootmgr_load(&handle, &load_options);
	if (ret != EFI_SUCCESS) {
		log_notice("EFI boot manager: Cannot load any image\n");
		return CMD_RET_FAILURE;
	}

	ret = do_bootefi_exec(handle, load_options);

	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

/**
 * do_bootefi_image() - execute EFI binary
 *
 * Set up memory image for the binary to be loaded, prepare device path, and
 * then call do_bootefi_exec() to execute it.
 *
 * @image_opt:	string with image start address
 * @size_opt:	string with image size or NULL
 * Return:	status code
 */
static int do_bootefi_image(const char *image_opt, const char *size_opt)
{
	void *image_buf;
	unsigned long addr, size;
	efi_status_t ret;

#ifdef CONFIG_CMD_BOOTEFI_HELLO
	if (!strcmp(image_opt, "hello")) {
		image_buf = __efi_helloworld_begin;
		size = __efi_helloworld_end - __efi_helloworld_begin;
		efi_clear_bootdev();
	} else
#endif
	{
		addr = strtoul(image_opt, NULL, 16);
		/* Check that a numeric value was passed */
		if (!addr)
			return CMD_RET_USAGE;
		image_buf = map_sysmem(addr, 0);

		if (size_opt) {
			size = strtoul(size_opt, NULL, 16);
			if (!size)
				return CMD_RET_USAGE;
			efi_clear_bootdev();
		} else {
			if (image_buf != image_addr) {
				log_err("No UEFI binary known at %s\n",
					image_opt);
				return CMD_RET_FAILURE;
			}
			size = image_size;
		}
	}
	ret = efi_run_image(image_buf, size);

	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

/**
 * efi_run_image() - run loaded UEFI image
 *
 * @source_buffer:	memory address of the UEFI image
 * @source_size:	size of the UEFI image
 * Return:		status code
 */
efi_status_t efi_run_image(void *source_buffer, efi_uintn_t source_size)
{
	efi_handle_t mem_handle = NULL, handle;
	struct efi_device_path *file_path = NULL;
	struct efi_device_path *msg_path;
	efi_status_t ret;
	u16 *load_options;

	if (!bootefi_device_path || !bootefi_image_path) {
		log_debug("Not loaded from disk\n");
		/*
		 * Special case for efi payload not loaded from disk,
		 * such as 'bootefi hello' or for example payload
		 * loaded directly into memory via JTAG, etc:
		 */
		file_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
					    (uintptr_t)source_buffer,
					    source_size);
		/*
		 * Make sure that device for device_path exist
		 * in load_image(). Otherwise, shell and grub will fail.
		 */
		ret = efi_create_handle(&mem_handle);
		if (ret != EFI_SUCCESS)
			goto out;

		ret = efi_add_protocol(mem_handle, &efi_guid_device_path,
				       file_path);
		if (ret != EFI_SUCCESS)
			goto out;
		msg_path = file_path;
	} else {
		file_path = efi_dp_append(bootefi_device_path,
					  bootefi_image_path);
		msg_path = bootefi_image_path;
		log_debug("Loaded from disk\n");
	}

	log_info("Booting %pD\n", msg_path);

	ret = EFI_CALL(efi_load_image(false, efi_root, file_path, source_buffer,
				      source_size, &handle));
	if (ret != EFI_SUCCESS) {
		log_err("Loading image failed\n");
		goto out;
	}

	/* Transfer environment variable as load options */
	ret = efi_env_set_load_options(handle, "bootargs", &load_options);
	if (ret != EFI_SUCCESS)
		goto out;

	ret = do_bootefi_exec(handle, load_options);

out:
	efi_delete_handle(mem_handle);
	efi_free_pool(file_path);
	return ret;
}

#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
static efi_status_t bootefi_run_prepare(const char *load_options_path,
		struct efi_device_path *device_path,
		struct efi_device_path *image_path,
		struct efi_loaded_image_obj **image_objp,
		struct efi_loaded_image **loaded_image_infop)
{
	efi_status_t ret;
	u16 *load_options;

	ret = efi_setup_loaded_image(device_path, image_path, image_objp,
				     loaded_image_infop);
	if (ret != EFI_SUCCESS)
		return ret;

	/* Transfer environment variable as load options */
	return efi_env_set_load_options((efi_handle_t)*image_objp,
					load_options_path,
					&load_options);
}

/**
 * bootefi_test_prepare() - prepare to run an EFI test
 *
 * Prepare to run a test as if it were provided by a loaded image.
 *
 * @image_objp:		pointer to be set to the loaded image handle
 * @loaded_image_infop:	pointer to be set to the loaded image protocol
 * @path:		dummy file path used to construct the device path
 *			set in the loaded image protocol
 * @load_options_path:	name of a U-Boot environment variable. Its value is
 *			set as load options in the loaded image protocol.
 * Return:		status code
 */
static efi_status_t bootefi_test_prepare
		(struct efi_loaded_image_obj **image_objp,
		 struct efi_loaded_image **loaded_image_infop, const char *path,
		 const char *load_options_path)
{
	efi_status_t ret;

	/* Construct a dummy device path */
	bootefi_device_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE, 0, 0);
	if (!bootefi_device_path)
		return EFI_OUT_OF_RESOURCES;

	bootefi_image_path = efi_dp_from_file(NULL, 0, path);
	if (!bootefi_image_path) {
		ret = EFI_OUT_OF_RESOURCES;
		goto failure;
	}

	ret = bootefi_run_prepare(load_options_path, bootefi_device_path,
				  bootefi_image_path, image_objp,
				  loaded_image_infop);
	if (ret == EFI_SUCCESS)
		return ret;

failure:
	efi_clear_bootdev();
	return ret;
}

/**
 * bootefi_run_finish() - finish up after running an EFI test
 *
 * @loaded_image_info: Pointer to a struct which holds the loaded image info
 * @image_obj: Pointer to a struct which holds the loaded image object
 */
static void bootefi_run_finish(struct efi_loaded_image_obj *image_obj,
			       struct efi_loaded_image *loaded_image_info)
{
	efi_restore_gd();
	free(loaded_image_info->load_options);
	efi_delete_handle(&image_obj->header);
}

/**
 * do_efi_selftest() - execute EFI selftest
 *
 * Return:	status code
 */
static int do_efi_selftest(void)
{
	struct efi_loaded_image_obj *image_obj;
	struct efi_loaded_image *loaded_image_info;
	efi_status_t ret;

	ret = bootefi_test_prepare(&image_obj, &loaded_image_info,
				   "\\selftest", "efi_selftest");
	if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	/* Execute the test */
	ret = EFI_CALL(efi_selftest(&image_obj->header, &systab));
	bootefi_run_finish(image_obj, loaded_image_info);

	return ret != EFI_SUCCESS;
}
#endif /* CONFIG_CMD_BOOTEFI_SELFTEST */

/**
 * do_bootefi() - execute `bootefi` command
 *
 * @cmdtp:	table entry describing command
 * @flag:	bitmap indicating how the command was invoked
 * @argc:	number of arguments
 * @argv:	command line arguments
 * Return:	status code
 */
static int do_bootefi(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	efi_status_t ret;
	char *img_addr, *img_size, *str_copy, *pos;
	void *fdt;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		log_err("Error: Cannot initialize UEFI sub-system, r = %lu\n",
			ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	if (argc > 2) {
		uintptr_t fdt_addr;

		fdt_addr = hextoul(argv[2], NULL);
		fdt = map_sysmem(fdt_addr, 0);
	} else {
		fdt = EFI_FDT_USE_INTERNAL;
	}
	ret = efi_install_fdt(fdt);
	if (ret == EFI_INVALID_PARAMETER)
		return CMD_RET_USAGE;
	else if (ret != EFI_SUCCESS)
		return CMD_RET_FAILURE;

	if (IS_ENABLED(CONFIG_CMD_BOOTEFI_BOOTMGR)) {
		if (!strcmp(argv[1], "bootmgr"))
			return do_efibootmgr();
	}
#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
	if (!strcmp(argv[1], "selftest"))
		return do_efi_selftest();
#endif
	str_copy = strdup(argv[1]);
	if (!str_copy) {
		log_err("Out of memory\n");
		return CMD_RET_FAILURE;
	}
	pos = str_copy;
	img_addr = strsep(&pos, ":");
	img_size = strsep(&pos, ":");
	ret = do_bootefi_image(img_addr, img_size);
	free(str_copy);

	return ret;
}

#ifdef CONFIG_SYS_LONGHELP
static char bootefi_help_text[] =
	"<image address>[:<image size>] [<fdt address>]\n"
	"  - boot EFI payload\n"
#ifdef CONFIG_CMD_BOOTEFI_HELLO
	"bootefi hello\n"
	"  - boot a sample Hello World application stored within U-Boot\n"
#endif
#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
	"bootefi selftest [fdt address]\n"
	"  - boot an EFI selftest application stored within U-Boot\n"
	"    Use environment variable efi_selftest to select a single test.\n"
	"    Use 'setenv efi_selftest list' to enumerate all tests.\n"
#endif
#ifdef CONFIG_CMD_BOOTEFI_BOOTMGR
	"bootefi bootmgr [fdt address]\n"
	"  - load and boot EFI payload based on BootOrder/BootXXXX variables.\n"
	"\n"
	"    If specified, the device tree located at <fdt address> gets\n"
	"    exposed as EFI configuration table.\n"
#endif
	;
#endif

U_BOOT_CMD(
	bootefi, 4, 0, do_bootefi,
	"Boots an EFI payload from memory",
	bootefi_help_text
);
