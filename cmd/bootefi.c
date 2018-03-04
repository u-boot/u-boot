/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <charset.h>
#include <common.h>
#include <command.h>
#include <dm.h>
#include <efi_loader.h>
#include <efi_selftest.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <linux/libfdt_env.h>
#include <memalign.h>
#include <asm/global_data.h>
#include <asm-generic/sections.h>
#include <linux/linkage.h>

DECLARE_GLOBAL_DATA_PTR;

static uint8_t efi_obj_list_initalized;

static struct efi_device_path *bootefi_image_path;
static struct efi_device_path *bootefi_device_path;

/* Initialize and populate EFI object list */
static void efi_init_obj_list(void)
{
	efi_obj_list_initalized = 1;

	/* Initialize EFI driver uclass */
	efi_driver_init();

	efi_console_register();
#ifdef CONFIG_PARTITIONS
	efi_disk_register();
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_DM_VIDEO)
	efi_gop_register();
#endif
#ifdef CONFIG_CMD_NET
	efi_net_register();
#endif
#ifdef CONFIG_GENERATE_SMBIOS_TABLE
	efi_smbios_register();
#endif
	efi_watchdog_register();

	/* Initialize EFI runtime services */
	efi_reset_system_init();
	efi_get_time_init();
}

/*
 * Set the load options of an image from an environment variable.
 *
 * @loaded_image_info:	the image
 * @env_var:		name of the environment variable
 */
static void set_load_options(struct efi_loaded_image *loaded_image_info,
			     const char *env_var)
{
	size_t size;
	const char *env = env_get(env_var);

	loaded_image_info->load_options = NULL;
	loaded_image_info->load_options_size = 0;
	if (!env)
		return;
	size = strlen(env) + 1;
	loaded_image_info->load_options = calloc(size, sizeof(u16));
	if (!loaded_image_info->load_options) {
		printf("ERROR: Out of memory\n");
		return;
	}
	utf8_to_utf16(loaded_image_info->load_options, (u8 *)env, size);
	loaded_image_info->load_options_size = size * 2;
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
	if (efi_allocate_pages(1, EFI_RUNTIME_SERVICES_DATA, fdt_pages,
			       &new_fdt_addr) != EFI_SUCCESS) {
		/* If we can't put it there, put it somewhere */
		new_fdt_addr = (ulong)memalign(EFI_PAGE_SIZE, fdt_size);
		if (efi_allocate_pages(1, EFI_RUNTIME_SERVICES_DATA, fdt_pages,
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

static efi_status_t efi_do_enter(
			efi_handle_t image_handle, struct efi_system_table *st,
			EFIAPI efi_status_t (*entry)(
				efi_handle_t image_handle,
				struct efi_system_table *st))
{
	efi_status_t ret = EFI_LOAD_ERROR;

	if (entry)
		ret = entry(image_handle, st);
	st->boottime->exit(image_handle, ret, 0, NULL);
	return ret;
}

#ifdef CONFIG_ARM64
static efi_status_t efi_run_in_el2(EFIAPI efi_status_t (*entry)(
			efi_handle_t image_handle, struct efi_system_table *st),
			efi_handle_t image_handle, struct efi_system_table *st)
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
static efi_status_t do_bootefi_exec(void *efi, void *fdt,
				    struct efi_device_path *device_path,
				    struct efi_device_path *image_path)
{
	struct efi_loaded_image loaded_image_info = {};
	struct efi_object loaded_image_info_obj = {};
	struct efi_device_path *memdp = NULL;
	ulong ret;

	EFIAPI efi_status_t (*entry)(efi_handle_t image_handle,
				     struct efi_system_table *st);
	ulong fdt_pages, fdt_size, fdt_start, fdt_end;
	const efi_guid_t fdt_guid = EFI_FDT_GUID;
	bootm_headers_t img = { 0 };

	/*
	 * Special case for efi payload not loaded from disk, such as
	 * 'bootefi hello' or for example payload loaded directly into
	 * memory via jtag/etc:
	 */
	if (!device_path && !image_path) {
		printf("WARNING: using memory device/image path, this may confuse some payloads!\n");
		/* actual addresses filled in after efi_load_pe() */
		memdp = efi_dp_from_mem(0, 0, 0);
		device_path = image_path = memdp;
	} else {
		assert(device_path && image_path);
	}

	/* Initialize and populate EFI object list */
	if (!efi_obj_list_initalized)
		efi_init_obj_list();

	efi_setup_loaded_image(&loaded_image_info, &loaded_image_info_obj,
			       device_path, image_path);

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

	/* Transfer environment variable bootargs as load options */
	set_load_options(&loaded_image_info, "bootargs");
	/* Load the EFI payload */
	entry = efi_load_pe(efi, &loaded_image_info);
	if (!entry) {
		ret = -ENOENT;
		goto exit;
	}

	if (memdp) {
		struct efi_device_path_memory *mdp = (void *)memdp;
		mdp->memory_type = loaded_image_info.image_code_type;
		mdp->start_address = (uintptr_t)loaded_image_info.image_base;
		mdp->end_address = mdp->start_address +
				loaded_image_info.image_size;
	}

	/* we don't support much: */
	env_set("efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_OsIndicationsSupported",
		"{ro,boot}(blob)0000000000000000");

	/* Call our payload! */
	debug("%s:%d Jumping to 0x%lx\n", __func__, __LINE__, (long)entry);

	if (setjmp(&loaded_image_info.exit_jmp)) {
		ret = loaded_image_info.exit_status;
		goto exit;
	}

#ifdef CONFIG_ARM64
	/* On AArch64 we need to make sure we call our payload in < EL3 */
	if (current_el() == 3) {
		smp_kick_all_cpus();
		dcache_disable();	/* flush cache before switch to EL2 */

		/* Move into EL2 and keep running there */
		armv8_switch_to_el2((ulong)entry,
				    (ulong)&loaded_image_info_obj.handle,
				    (ulong)&systab, 0, (ulong)efi_run_in_el2,
				    ES_TO_AARCH64);

		/* Should never reach here, efi exits with longjmp */
		while (1) { }
	}
#endif

	ret = efi_do_enter(loaded_image_info_obj.handle, &systab, entry);

exit:
	/* image has returned, loaded-image obj goes *poof*: */
	list_del(&loaded_image_info_obj.link);

	return ret;
}

static int do_bootefi_bootmgr_exec(unsigned long fdt_addr)
{
	struct efi_device_path *device_path, *file_path;
	void *addr;
	efi_status_t r;

	/* Initialize and populate EFI object list */
	if (!efi_obj_list_initalized)
		efi_init_obj_list();

	/*
	 * gd lives in a fixed register which may get clobbered while we execute
	 * the payload. So save it here and restore it on every callback entry
	 */
	efi_save_gd();

	addr = efi_bootmgr_load(&device_path, &file_path);
	if (!addr)
		return 1;

	printf("## Starting EFI application at %p ...\n", addr);
	r = do_bootefi_exec(addr, (void *)fdt_addr, device_path, file_path);
	printf("## Application terminated, r = %lu\n",
	       r & ~EFI_ERROR_MASK);

	if (r != EFI_SUCCESS)
		return 1;

	return 0;
}

/* Interpreter command to boot an arbitrary EFI image from memory */
static int do_bootefi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *saddr, *sfdt;
	unsigned long addr, fdt_addr = 0;
	efi_status_t r;

	if (argc < 2)
		return CMD_RET_USAGE;
#ifdef CONFIG_CMD_BOOTEFI_HELLO
	if (!strcmp(argv[1], "hello")) {
		ulong size = __efi_helloworld_end - __efi_helloworld_begin;

		saddr = env_get("loadaddr");
		if (saddr)
			addr = simple_strtoul(saddr, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;
		memcpy((char *)addr, __efi_helloworld_begin, size);
	} else
#endif
#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
	if (!strcmp(argv[1], "selftest")) {
		struct efi_loaded_image loaded_image_info = {};
		struct efi_object loaded_image_info_obj = {};

		/* Construct a dummy device path. */
		bootefi_device_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
						      (uintptr_t)&efi_selftest,
						      (uintptr_t)&efi_selftest);
		bootefi_image_path = efi_dp_from_file(NULL, 0, "\\selftest");

		efi_setup_loaded_image(&loaded_image_info,
				       &loaded_image_info_obj,
				       bootefi_device_path, bootefi_image_path);
		/*
		 * gd lives in a fixed register which may get clobbered while we
		 * execute the payload. So save it here and restore it on every
		 * callback entry
		 */
		efi_save_gd();
		/* Initialize and populate EFI object list */
		if (!efi_obj_list_initalized)
			efi_init_obj_list();
		/* Transfer environment variable efi_selftest as load options */
		set_load_options(&loaded_image_info, "efi_selftest");
		/* Execute the test */
		r = efi_selftest(loaded_image_info_obj.handle, &systab);
		efi_restore_gd();
		free(loaded_image_info.load_options);
		list_del(&loaded_image_info_obj.link);
		return r != EFI_SUCCESS;
	} else
#endif
	if (!strcmp(argv[1], "bootmgr")) {
		unsigned long fdt_addr = 0;

		if (argc > 2)
			fdt_addr = simple_strtoul(argv[2], NULL, 16);

		return do_bootefi_bootmgr_exec(fdt_addr);
	} else {
		saddr = argv[1];

		addr = simple_strtoul(saddr, NULL, 16);
		/* Check that a numeric value was passed */
		if (!addr && *saddr != '0')
			return CMD_RET_USAGE;

		if (argc > 2) {
			sfdt = argv[2];
			fdt_addr = simple_strtoul(sfdt, NULL, 16);
		}
	}

	printf("## Starting EFI application at %08lx ...\n", addr);
	r = do_bootefi_exec((void *)addr, (void *)fdt_addr,
			    bootefi_device_path, bootefi_image_path);
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
	"bootefi hello\n"
	"  - boot a sample Hello World application stored within U-Boot\n"
#endif
#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
	"bootefi selftest\n"
	"  - boot an EFI selftest application stored within U-Boot\n"
	"    Use environment variable efi_selftest to select a single test.\n"
	"    Use 'setenv efi_selftest list' to enumerate all tests.\n"
#endif
	"bootefi bootmgr [fdt addr]\n"
	"  - load and boot EFI payload based on BootOrder/BootXXXX variables.\n"
	"\n"
	"    If specified, the device tree located at <fdt address> gets\n"
	"    exposed as EFI configuration table.\n";
#endif

U_BOOT_CMD(
	bootefi, 3, 0, do_bootefi,
	"Boots an EFI payload from memory",
	bootefi_help_text
);

static int parse_partnum(const char *devnr)
{
	const char *str = strchr(devnr, ':');
	if (str) {
		str++;
		return simple_strtoul(str, NULL, 16);
	}
	return 0;
}

void efi_set_bootdev(const char *dev, const char *devnr, const char *path)
{
	char filename[32] = { 0 }; /* dp->str is u16[32] long */
	char *s;

	if (strcmp(dev, "Net")) {
		struct blk_desc *desc;
		int part;

		desc = blk_get_dev(dev, simple_strtol(devnr, NULL, 10));
		if (!desc)
			return;
		part = parse_partnum(devnr);

		bootefi_device_path = efi_dp_from_part(desc, part);
	} else {
#ifdef CONFIG_CMD_NET
		bootefi_device_path = efi_dp_from_eth();
#endif
	}

	if (!path)
		return;

	if (strcmp(dev, "Net")) {
		/* Add leading / to fs paths, because they're absolute */
		snprintf(filename, sizeof(filename), "/%s", path);
	} else {
		snprintf(filename, sizeof(filename), "%s", path);
	}
	/* DOS style file path: */
	s = filename;
	while ((s = strchr(s, '/')))
		*s++ = '\\';
	bootefi_image_path = efi_dp_from_file(NULL, 0, filename);
}
