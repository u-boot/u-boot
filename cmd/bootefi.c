// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
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
#include <mapmem.h>
#include <memalign.h>
#include <asm/global_data.h>
#include <asm-generic/sections.h>
#include <asm-generic/unaligned.h>
#include <linux/linkage.h>

#ifdef CONFIG_ARMV7_NONSEC
#include <asm/armv7.h>
#include <asm/secure.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define OBJ_LIST_NOT_INITIALIZED 1

static efi_status_t efi_obj_list_initialized = OBJ_LIST_NOT_INITIALIZED;

static struct efi_device_path *bootefi_image_path;
static struct efi_device_path *bootefi_device_path;

/* Initialize and populate EFI object list */
efi_status_t efi_init_obj_list(void)
{
	efi_status_t ret = EFI_SUCCESS;

	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	/* Initialize once only */
	if (efi_obj_list_initialized != OBJ_LIST_NOT_INITIALIZED)
		return efi_obj_list_initialized;

	/* Initialize system table */
	ret = efi_initialize_system_table();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize root node */
	ret = efi_root_node_register();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize EFI driver uclass */
	ret = efi_driver_init();
	if (ret != EFI_SUCCESS)
		goto out;

	ret = efi_console_register();
	if (ret != EFI_SUCCESS)
		goto out;
#ifdef CONFIG_PARTITIONS
	ret = efi_disk_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#if defined(CONFIG_LCD) || defined(CONFIG_DM_VIDEO)
	ret = efi_gop_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_NET
	ret = efi_net_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_GENERATE_ACPI_TABLE
	ret = efi_acpi_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
#ifdef CONFIG_GENERATE_SMBIOS_TABLE
	ret = efi_smbios_register();
	if (ret != EFI_SUCCESS)
		goto out;
#endif
	ret = efi_watchdog_register();
	if (ret != EFI_SUCCESS)
		goto out;

	/* Initialize EFI runtime services */
	ret = efi_reset_system_init();
	if (ret != EFI_SUCCESS)
		goto out;

out:
	efi_obj_list_initialized = ret;
	return ret;
}

/*
 * Allow unaligned memory access.
 *
 * This routine is overridden by architectures providing this feature.
 */
void __weak allow_unaligned(void)
{
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
	u16 *pos;

	loaded_image_info->load_options = NULL;
	loaded_image_info->load_options_size = 0;
	if (!env)
		return;
	size = utf8_utf16_strlen(env) + 1;
	loaded_image_info->load_options = calloc(size, sizeof(u16));
	if (!loaded_image_info->load_options) {
		printf("ERROR: Out of memory\n");
		return;
	}
	pos = loaded_image_info->load_options;
	utf8_utf16_strcpy(&pos, env);
	loaded_image_info->load_options_size = size * 2;
}

/**
 * copy_fdt() - Copy the device tree to a new location available to EFI
 *
 * The FDT is relocated into a suitable location within the EFI memory map.
 * An additional 12KB is added to the space in case the device tree needs to be
 * expanded later with fdt_open_into().
 *
 * @fdt_addr: On entry, address of start of FDT. On exit, address of relocated
 *	FDT start
 * @fdt_sizep: Returns new size of FDT, including
 * @return new relocated address of FDT
 */
static efi_status_t copy_fdt(ulong *fdt_addrp, ulong *fdt_sizep)
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
	 * Give us at least 4KB of breathing room in case the device tree needs
	 * to be expanded later. Round up to the nearest EFI page boundary.
	 */
	fdt = map_sysmem(*fdt_addrp, 0);
	fdt_size = fdt_totalsize(fdt);
	fdt_size += 4096 * 3;
	fdt_size = ALIGN(fdt_size + EFI_PAGE_SIZE - 1, EFI_PAGE_SIZE);
	fdt_pages = fdt_size >> EFI_PAGE_SHIFT;

	/* Safe fdt location is at 127MB */
	new_fdt_addr = fdt_ram_start + (127 * 1024 * 1024) + fdt_size;
	ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
				 EFI_RUNTIME_SERVICES_DATA, fdt_pages,
				 &new_fdt_addr);
	if (ret != EFI_SUCCESS) {
		/* If we can't put it there, put it somewhere */
		new_fdt_addr = (ulong)memalign(EFI_PAGE_SIZE, fdt_size);
		ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
					 EFI_RUNTIME_SERVICES_DATA, fdt_pages,
					 &new_fdt_addr);
		if (ret != EFI_SUCCESS) {
			printf("ERROR: Failed to reserve space for FDT\n");
			goto done;
		}
	}

	new_fdt = map_sysmem(new_fdt_addr, fdt_size);
	memcpy(new_fdt, fdt, fdt_totalsize(fdt));
	fdt_set_totalsize(new_fdt, fdt_size);

	*fdt_addrp = new_fdt_addr;
	*fdt_sizep = fdt_size;
done:
	return ret;
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

#ifdef CONFIG_ARMV7_NONSEC
static bool is_nonsec;

static efi_status_t efi_run_in_hyp(EFIAPI efi_status_t (*entry)(
			efi_handle_t image_handle, struct efi_system_table *st),
			efi_handle_t image_handle, struct efi_system_table *st)
{
	/* Enable caches again */
	dcache_enable();

	is_nonsec = true;

	return efi_do_enter(image_handle, st, entry);
}
#endif

/*
 * efi_carve_out_dt_rsv() - Carve out DT reserved memory ranges
 *
 * The mem_rsv entries of the FDT are added to the memory map. Any failures are
 * ignored because this is not critical and we would rather continue to try to
 * boot.
 *
 * @fdt: Pointer to device tree
 */
static void efi_carve_out_dt_rsv(void *fdt)
{
	int nr_rsv, i;
	uint64_t addr, size, pages;

	nr_rsv = fdt_num_mem_rsv(fdt);

	/* Look for an existing entry and add it to the efi mem map. */
	for (i = 0; i < nr_rsv; i++) {
		if (fdt_get_mem_rsv(fdt, i, &addr, &size) != 0)
			continue;

		pages = ALIGN(size, EFI_PAGE_SIZE) >> EFI_PAGE_SHIFT;
		if (!efi_add_memory_map(addr, pages, EFI_RESERVED_MEMORY_TYPE,
					false))
			printf("FDT memrsv map %d: Failed to add to map\n", i);
	}
}

static efi_status_t efi_install_fdt(ulong fdt_addr)
{
	bootm_headers_t img = { 0 };
	ulong fdt_pages, fdt_size, fdt_start;
	efi_status_t ret;
	void *fdt;

	fdt = map_sysmem(fdt_addr, 0);
	if (fdt_check_header(fdt)) {
		printf("ERROR: invalid device tree\n");
		return EFI_INVALID_PARAMETER;
	}

	/* Prepare fdt for payload */
	ret = copy_fdt(&fdt_addr, &fdt_size);
	if (ret)
		return ret;

	unmap_sysmem(fdt);
	fdt = map_sysmem(fdt_addr, 0);
	fdt_size = fdt_totalsize(fdt);
	if (image_setup_libfdt(&img, fdt, 0, NULL)) {
		printf("ERROR: failed to process device tree\n");
		return EFI_LOAD_ERROR;
	}

	efi_carve_out_dt_rsv(fdt);

	/* Link to it in the efi tables */
	ret = efi_install_configuration_table(&efi_guid_fdt, fdt);
	if (ret != EFI_SUCCESS)
		return EFI_OUT_OF_RESOURCES;

	/* And reserve the space in the memory map */
	fdt_start = fdt_addr;
	fdt_pages = fdt_size >> EFI_PAGE_SHIFT;

	ret = efi_add_memory_map(fdt_start, fdt_pages,
				 EFI_BOOT_SERVICES_DATA, true);

	return ret;
}

/**
 * do_bootefi_exec() - execute EFI binary
 *
 * @efi:		address of the binary
 * @device_path:	path of the device from which the binary was loaded
 * @image_path:		device path of the binary
 * Return:		status code
 *
 * Load the EFI binary into a newly assigned memory unwinding the relocation
 * information, install the loaded image protocol, and call the binary.
 */
static efi_status_t do_bootefi_exec(void *efi,
				    struct efi_device_path *device_path,
				    struct efi_device_path *image_path)
{
	efi_handle_t mem_handle = NULL;
	struct efi_device_path *memdp = NULL;
	efi_status_t ret;
	struct efi_loaded_image_obj *image_handle = NULL;
	struct efi_loaded_image *loaded_image_info = NULL;

	EFIAPI efi_status_t (*entry)(efi_handle_t image_handle,
				     struct efi_system_table *st);

	/*
	 * Special case for efi payload not loaded from disk, such as
	 * 'bootefi hello' or for example payload loaded directly into
	 * memory via jtag, etc:
	 */
	if (!device_path && !image_path) {
		printf("WARNING: using memory device/image path, this may confuse some payloads!\n");
		/* actual addresses filled in after efi_load_pe() */
		memdp = efi_dp_from_mem(0, 0, 0);
		device_path = image_path = memdp;
		/*
		 * Grub expects that the device path of the loaded image is
		 * installed on a handle.
		 */
		ret = efi_create_handle(&mem_handle);
		if (ret != EFI_SUCCESS)
			goto exit;
		ret = efi_add_protocol(mem_handle, &efi_guid_device_path,
				       device_path);
		if (ret != EFI_SUCCESS)
			goto exit;
	} else {
		assert(device_path && image_path);
	}

	ret = efi_setup_loaded_image(device_path, image_path, &image_handle,
				     &loaded_image_info);
	if (ret != EFI_SUCCESS)
		goto exit;

	/* Transfer environment variable bootargs as load options */
	set_load_options(loaded_image_info, "bootargs");
	/* Load the EFI payload */
	entry = efi_load_pe(image_handle, efi, loaded_image_info);
	if (!entry) {
		ret = EFI_LOAD_ERROR;
		goto exit;
	}

	if (memdp) {
		struct efi_device_path_memory *mdp = (void *)memdp;
		mdp->memory_type = loaded_image_info->image_code_type;
		mdp->start_address = (uintptr_t)loaded_image_info->image_base;
		mdp->end_address = mdp->start_address +
				loaded_image_info->image_size;
	}

	/* we don't support much: */
	env_set("efi_8be4df61-93ca-11d2-aa0d-00e098032b8c_OsIndicationsSupported",
		"{ro,boot}(blob)0000000000000000");

	/* Call our payload! */
	debug("%s:%d Jumping to 0x%lx\n", __func__, __LINE__, (long)entry);

	if (setjmp(&image_handle->exit_jmp)) {
		ret = image_handle->exit_status;
		goto exit;
	}

#ifdef CONFIG_ARM64
	/* On AArch64 we need to make sure we call our payload in < EL3 */
	if (current_el() == 3) {
		smp_kick_all_cpus();
		dcache_disable();	/* flush cache before switch to EL2 */

		/* Move into EL2 and keep running there */
		armv8_switch_to_el2((ulong)entry,
				    (ulong)image_handle,
				    (ulong)&systab, 0, (ulong)efi_run_in_el2,
				    ES_TO_AARCH64);

		/* Should never reach here, efi exits with longjmp */
		while (1) { }
	}
#endif

#ifdef CONFIG_ARMV7_NONSEC
	if (armv7_boot_nonsec() && !is_nonsec) {
		dcache_disable();	/* flush cache before switch to HYP */

		armv7_init_nonsec();
		secure_ram_addr(_do_nonsec_entry)(
					efi_run_in_hyp,
					(uintptr_t)entry,
					(uintptr_t)image_handle,
					(uintptr_t)&systab);

		/* Should never reach here, efi exits with longjmp */
		while (1) { }
	}
#endif

	ret = efi_do_enter(image_handle, &systab, entry);

exit:
	/* image has returned, loaded-image obj goes *poof*: */
	if (image_handle)
		efi_delete_handle(&image_handle->parent);
	if (mem_handle)
		efi_delete_handle(mem_handle);

	return ret;
}

static int do_bootefi_bootmgr_exec(void)
{
	struct efi_device_path *device_path, *file_path;
	void *addr;
	efi_status_t r;

	addr = efi_bootmgr_load(&device_path, &file_path);
	if (!addr)
		return 1;

	printf("## Starting EFI application at %p ...\n", addr);
	r = do_bootefi_exec(addr, device_path, file_path);
	printf("## Application terminated, r = %lu\n",
	       r & ~EFI_ERROR_MASK);

	if (r != EFI_SUCCESS)
		return 1;

	return 0;
}

/* Interpreter command to boot an arbitrary EFI image from memory */
static int do_bootefi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;
	char *saddr;
	efi_status_t r;
	unsigned long fdt_addr;

	/* Allow unaligned memory access */
	allow_unaligned();

	/* Initialize EFI drivers */
	r = efi_init_obj_list();
	if (r != EFI_SUCCESS) {
		printf("Error: Cannot set up EFI drivers, r = %lu\n",
		       r & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2) {
		fdt_addr = simple_strtoul(argv[2], NULL, 16);
		if (!fdt_addr && *argv[2] != '0')
			return CMD_RET_USAGE;
		/* Install device tree */
		r = efi_install_fdt(fdt_addr);
		if (r != EFI_SUCCESS) {
			printf("ERROR: failed to install device tree\n");
			return CMD_RET_FAILURE;
		}
	} else {
		/* Remove device tree. EFI_NOT_FOUND can be ignored here */
		efi_install_configuration_table(&efi_guid_fdt, NULL);
		printf("WARNING: booting without device tree\n");
	}
#ifdef CONFIG_CMD_BOOTEFI_HELLO
	if (!strcmp(argv[1], "hello")) {
		ulong size = __efi_helloworld_end - __efi_helloworld_begin;

		saddr = env_get("loadaddr");
		if (saddr)
			addr = simple_strtoul(saddr, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;
		memcpy(map_sysmem(addr, size), __efi_helloworld_begin, size);
	} else
#endif
#ifdef CONFIG_CMD_BOOTEFI_SELFTEST
	if (!strcmp(argv[1], "selftest")) {
		struct efi_loaded_image_obj *image_handle;
		struct efi_loaded_image *loaded_image_info;

		/* Construct a dummy device path. */
		bootefi_device_path = efi_dp_from_mem(EFI_RESERVED_MEMORY_TYPE,
						      (uintptr_t)&efi_selftest,
						      (uintptr_t)&efi_selftest);
		bootefi_image_path = efi_dp_from_file(NULL, 0, "\\selftest");

		r = efi_setup_loaded_image(bootefi_device_path,
					   bootefi_image_path, &image_handle,
					   &loaded_image_info);
		if (r != EFI_SUCCESS)
			return CMD_RET_FAILURE;

		efi_save_gd();
		/* Transfer environment variable efi_selftest as load options */
		set_load_options(loaded_image_info, "efi_selftest");
		/* Execute the test */
		r = efi_selftest(image_handle, &systab);
		efi_restore_gd();
		free(loaded_image_info->load_options);
		efi_delete_handle(&image_handle->parent);
		return r != EFI_SUCCESS;
	} else
#endif
	if (!strcmp(argv[1], "bootmgr")) {
		return do_bootefi_bootmgr_exec();
	} else {
		saddr = argv[1];

		addr = simple_strtoul(saddr, NULL, 16);
		/* Check that a numeric value was passed */
		if (!addr && *saddr != '0')
			return CMD_RET_USAGE;

	}

	printf("## Starting EFI application at %08lx ...\n", addr);
	r = do_bootefi_exec(map_sysmem(addr, 0), bootefi_device_path,
			    bootefi_image_path);
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
	"bootefi selftest [fdt address]\n"
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

void efi_set_bootdev(const char *dev, const char *devnr, const char *path)
{
	char filename[32] = { 0 }; /* dp->str is u16[32] long */
	char *s;

	/* efi_set_bootdev is typically called repeatedly, recover memory */
	efi_free_pool(bootefi_device_path);
	efi_free_pool(bootefi_image_path);
	/* If blk_get_device_part_str fails, avoid duplicate free. */
	bootefi_device_path = NULL;
	bootefi_image_path = NULL;

	if (strcmp(dev, "Net")) {
		struct blk_desc *desc;
		disk_partition_t fs_partition;
		int part;

		part = blk_get_device_part_str(dev, devnr, &desc, &fs_partition,
					       1);
		if (part < 0)
			return;

		bootefi_device_path = efi_dp_from_part(desc, part);
	} else {
#ifdef CONFIG_NET
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
