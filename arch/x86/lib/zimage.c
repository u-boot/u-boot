// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

/*
 * Linux x86 zImage and bzImage loading
 *
 * based on the procdure described in
 * linux/Documentation/i386/boot.txt
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <bootm.h>
#include <command.h>
#include <env.h>
#include <init.h>
#include <irq_func.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <acpi/acpi_table.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <asm/zimage.h>
#include <asm/byteorder.h>
#include <asm/bootm.h>
#include <asm/bootparam.h>
#include <asm/efi.h>
#include <asm/global_data.h>
#ifdef CONFIG_SYS_COREBOOT
#include <asm/arch/timestamp.h>
#endif
#include <linux/compiler.h>
#include <linux/ctype.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Memory lay-out:
 *
 * relative to setup_base (which is 0x90000 currently)
 *
 *	0x0000-0x7FFF	Real mode kernel
 *	0x8000-0x8FFF	Stack and heap
 *	0x9000-0x90FF	Kernel command line
 */
#define DEFAULT_SETUP_BASE	0x90000
#define COMMAND_LINE_OFFSET	0x9000
#define HEAP_END_OFFSET		0x8e00

#define COMMAND_LINE_SIZE	2048

/* Current state of the boot */
struct zboot_state state;

static void build_command_line(char *command_line, int auto_boot)
{
	char *env_command_line;

	command_line[0] = '\0';

	env_command_line =  env_get("bootargs");

	/* set console= argument if we use a serial console */
	if (!strstr(env_command_line, "console=")) {
		if (!strcmp(env_get("stdout"), "serial")) {

			/* We seem to use serial console */
			sprintf(command_line, "console=ttyS0,%s ",
				env_get("baudrate"));
		}
	}

	if (auto_boot)
		strcat(command_line, "auto ");

	if (env_command_line)
		strcat(command_line, env_command_line);
#ifdef DEBUG
	printf("Kernel command line:");
	puts(command_line);
	printf("\n");
#endif
}

static int kernel_magic_ok(struct setup_header *hdr)
{
	if (KERNEL_MAGIC != hdr->boot_flag) {
		printf("Error: Invalid Boot Flag "
			"(found 0x%04x, expected 0x%04x)\n",
			hdr->boot_flag, KERNEL_MAGIC);
		return 0;
	} else {
		printf("Valid Boot Flag\n");
		return 1;
	}
}

static int get_boot_protocol(struct setup_header *hdr, bool verbose)
{
	if (hdr->header == KERNEL_V2_MAGIC) {
		if (verbose)
			printf("Magic signature found\n");
		return hdr->version;
	} else {
		/* Very old kernel */
		if (verbose)
			printf("Magic signature not found\n");
		return 0x0100;
	}
}

static int setup_device_tree(struct setup_header *hdr, const void *fdt_blob)
{
	int bootproto = get_boot_protocol(hdr, false);
	struct setup_data *sd;
	int size;

	if (bootproto < 0x0209)
		return -ENOTSUPP;

	if (!fdt_blob)
		return 0;

	size = fdt_totalsize(fdt_blob);
	if (size < 0)
		return -EINVAL;

	size += sizeof(struct setup_data);
	sd = (struct setup_data *)malloc(size);
	if (!sd) {
		printf("Not enough memory for DTB setup data\n");
		return -ENOMEM;
	}

	sd->next = hdr->setup_data;
	sd->type = SETUP_DTB;
	sd->len = fdt_totalsize(fdt_blob);
	memcpy(sd->data, fdt_blob, sd->len);
	hdr->setup_data = (unsigned long)sd;

	return 0;
}

const char *zimage_get_kernel_version(struct boot_params *params,
				      void *kernel_base)
{
	struct setup_header *hdr = &params->hdr;
	int bootproto;
	const char *s, *end;

	bootproto = get_boot_protocol(hdr, false);
	log_debug("bootproto %x, hdr->setup_sects %x\n", bootproto,
		  hdr->setup_sects);
	if (bootproto < 0x0200 || hdr->setup_sects < 15)
		return NULL;

	/* sanity-check the kernel version in case it is missing */
	log_debug("hdr->kernel_version %x, str at %p\n", hdr->kernel_version,
		  kernel_base + hdr->kernel_version + 0x200);
	for (s = kernel_base + hdr->kernel_version + 0x200, end = s + 0x100; *s;
	     s++) {
		if (!isprint(*s))
			return NULL;
	}

	return kernel_base + hdr->kernel_version + 0x200;
}

struct boot_params *load_zimage(char *image, unsigned long kernel_size,
				ulong *load_addressp)
{
	struct boot_params *setup_base;
	const char *version;
	int setup_size;
	int bootproto;
	int big_image;

	struct boot_params *params = (struct boot_params *)image;
	struct setup_header *hdr = &params->hdr;

	/* base address for real-mode segment */
	setup_base = (struct boot_params *)DEFAULT_SETUP_BASE;

	if (!kernel_magic_ok(hdr))
		return 0;

	/* determine size of setup */
	if (0 == hdr->setup_sects) {
		log_warning("Setup Sectors = 0 (defaulting to 4)\n");
		setup_size = 5 * 512;
	} else {
		setup_size = (hdr->setup_sects + 1) * 512;
	}

	log_debug("Setup Size = 0x%8.8lx\n", (ulong)setup_size);

	if (setup_size > SETUP_MAX_SIZE)
		printf("Error: Setup is too large (%d bytes)\n", setup_size);

	/* determine boot protocol version */
	bootproto = get_boot_protocol(hdr, true);

	log_debug("Using boot protocol version %x.%02x\n",
		  (bootproto & 0xff00) >> 8, bootproto & 0xff);

	version = zimage_get_kernel_version(params, image);
	if (version)
		printf("Linux kernel version %s\n", version);
	else
		printf("Setup Sectors < 15 - Cannot print kernel version\n");

	/* Determine image type */
	big_image = (bootproto >= 0x0200) &&
		    (hdr->loadflags & BIG_KERNEL_FLAG);

	/* Determine load address */
	if (big_image)
		*load_addressp = BZIMAGE_LOAD_ADDR;
	else
		*load_addressp = ZIMAGE_LOAD_ADDR;

	printf("Building boot_params at %lx\n", (ulong)setup_base);
	memset(setup_base, 0, sizeof(*setup_base));
	setup_base->hdr = params->hdr;

	if (bootproto >= 0x0204)
		kernel_size = hdr->syssize * 16;
	else
		kernel_size -= setup_size;

	if (bootproto == 0x0100) {
		/*
		 * A very old kernel MUST have its real-mode code
		 * loaded at 0x90000
		 */
		if ((ulong)setup_base != 0x90000) {
			/* Copy the real-mode kernel */
			memmove((void *)0x90000, setup_base, setup_size);

			/* Copy the command line */
			memmove((void *)0x99000,
				(u8 *)setup_base + COMMAND_LINE_OFFSET,
				COMMAND_LINE_SIZE);

			 /* Relocated */
			setup_base = (struct boot_params *)0x90000;
		}

		/* It is recommended to clear memory up to the 32K mark */
		memset((u8 *)0x90000 + setup_size, 0,
		       SETUP_MAX_SIZE - setup_size);
	}

	if (big_image) {
		if (kernel_size > BZIMAGE_MAX_SIZE) {
			printf("Error: bzImage kernel too big! "
				"(size: %ld, max: %d)\n",
				kernel_size, BZIMAGE_MAX_SIZE);
			return 0;
		}
	} else if ((kernel_size) > ZIMAGE_MAX_SIZE) {
		printf("Error: zImage kernel too big! (size: %ld, max: %d)\n",
		       kernel_size, ZIMAGE_MAX_SIZE);
		return 0;
	}

	printf("Loading %s at address %lx (%ld bytes)\n",
	       big_image ? "bzImage" : "zImage", *load_addressp, kernel_size);

	memmove((void *)*load_addressp, image + setup_size, kernel_size);

	return setup_base;
}

int setup_zimage(struct boot_params *setup_base, char *cmd_line, int auto_boot,
		 ulong initrd_addr, ulong initrd_size, ulong cmdline_force)
{
	struct setup_header *hdr = &setup_base->hdr;
	int bootproto = get_boot_protocol(hdr, false);

	log_debug("Setup E820 entries\n");
	if (IS_ENABLED(CONFIG_COREBOOT_SYSINFO)) {
		setup_base->e820_entries = cb_install_e820_map(
			ARRAY_SIZE(setup_base->e820_map), setup_base->e820_map);
	} else {
		setup_base->e820_entries = install_e820_map(
			ARRAY_SIZE(setup_base->e820_map), setup_base->e820_map);
	}

	if (bootproto == 0x0100) {
		setup_base->screen_info.cl_magic = COMMAND_LINE_MAGIC;
		setup_base->screen_info.cl_offset = COMMAND_LINE_OFFSET;
	}
	if (bootproto >= 0x0200) {
		hdr->type_of_loader = 0x80;	/* U-Boot version 0 */
		if (initrd_addr) {
			printf("Initial RAM disk at linear address "
			       "%lx, size %lx (%ld bytes)\n",
			       initrd_addr, initrd_size, initrd_size);

			hdr->ramdisk_image = initrd_addr;
			setup_base->ext_ramdisk_image = 0;
			setup_base->ext_ramdisk_size = 0;
			setup_base->ext_cmd_line_ptr = 0;
			hdr->ramdisk_size = initrd_size;
		}
	}

	if (bootproto >= 0x0201) {
		hdr->heap_end_ptr = HEAP_END_OFFSET;
		hdr->loadflags |= HEAP_FLAG;
	}

	if (cmd_line) {
		int max_size = 0xff;
		int ret;

		log_debug("Setup cmdline\n");
		if (bootproto >= 0x0206)
			max_size = hdr->cmdline_size;
		if (bootproto >= 0x0202) {
			hdr->cmd_line_ptr = (uintptr_t)cmd_line;
		} else if (bootproto >= 0x0200) {
			setup_base->screen_info.cl_magic = COMMAND_LINE_MAGIC;
			setup_base->screen_info.cl_offset =
				(uintptr_t)cmd_line - (uintptr_t)setup_base;

			hdr->setup_move_size = 0x9100;
		}

		/* build command line at COMMAND_LINE_OFFSET */
		if (cmdline_force)
			strcpy(cmd_line, (char *)cmdline_force);
		else
			build_command_line(cmd_line, auto_boot);
		if (IS_ENABLED(CONFIG_CMD_BOOTM)) {
			ret = bootm_process_cmdline(cmd_line, max_size,
						    BOOTM_CL_ALL);
			if (ret) {
				printf("Cmdline setup failed (max_size=%x, bootproto=%x, err=%d)\n",
				       max_size, bootproto, ret);
				return ret;
			}
		}
		printf("Kernel command line: \"");
		puts(cmd_line);
		printf("\"\n");
	}

	if (IS_ENABLED(CONFIG_INTEL_MID) && bootproto >= 0x0207)
		hdr->hardware_subarch = X86_SUBARCH_INTEL_MID;

	if (IS_ENABLED(CONFIG_GENERATE_ACPI_TABLE))
		setup_base->acpi_rsdp_addr = acpi_get_rsdp_addr();

	log_debug("Setup devicetree\n");
	setup_device_tree(hdr, (const void *)env_get_hex("fdtaddr", 0));
	setup_video(&setup_base->screen_info);

	if (IS_ENABLED(CONFIG_EFI_STUB))
		setup_efi_info(&setup_base->efi_info);

	return 0;
}

int zboot_load(void)
{
	struct boot_params *base_ptr;
	int ret;

	if (state.base_ptr) {
		struct boot_params *from = (struct boot_params *)state.base_ptr;

		base_ptr = (struct boot_params *)DEFAULT_SETUP_BASE;
		log_debug("Building boot_params at %lx\n", (ulong)base_ptr);
		memset(base_ptr, '\0', sizeof(*base_ptr));
		base_ptr->hdr = from->hdr;
	} else {
		base_ptr = load_zimage((void *)state.bzimage_addr, state.bzimage_size,
				       &state.load_address);
		if (!base_ptr) {
			puts("## Kernel loading failed ...\n");
			return -EINVAL;
		}
	}
	state.base_ptr = base_ptr;

	ret = env_set_hex("zbootbase", map_to_sysmem(state.base_ptr));
	if (!ret)
		ret = env_set_hex("zbootaddr", state.load_address);
	if (ret)
		return ret;

	return 0;
}

int zboot_setup(void)
{
	struct boot_params *base_ptr = state.base_ptr;
	int ret;

	ret = setup_zimage(base_ptr, (char *)base_ptr + COMMAND_LINE_OFFSET,
			   0, state.initrd_addr, state.initrd_size,
			   (ulong)state.cmdline);
	if (ret)
		return -EINVAL;

	return 0;
}

int zboot_go(void)
{
	struct boot_params *params = state.base_ptr;
	struct setup_header *hdr = &params->hdr;
	bool image_64bit;
	ulong entry;
	int ret;

	disable_interrupts();

	entry = state.load_address;
	image_64bit = false;
	if (IS_ENABLED(CONFIG_X86_64) &&
	    (hdr->xloadflags & XLF_KERNEL_64)) {
		image_64bit = true;
	}

	/* we assume that the kernel is in place */
	ret = boot_linux_kernel((ulong)state.base_ptr, entry, image_64bit);

	return ret;
}

int zboot_run(ulong addr, ulong size, ulong initrd, ulong initrd_size,
	      ulong base, char *cmdline)
{
	int ret;

	zboot_start(addr, size, initrd, initrd_size, base, cmdline);
	ret = zboot_load();
	if (ret)
		return log_msg_ret("ld", ret);
	ret = zboot_setup();
	if (ret)
		return log_msg_ret("set", ret);
	ret = zboot_go();
	if (ret)
		return log_msg_ret("go", ret);

	return -EFAULT;
}

static void print_num(const char *name, ulong value)
{
	printf("%-20s: %lx\n", name, value);
}

static void print_num64(const char *name, u64 value)
{
	printf("%-20s: %llx\n", name, value);
}

static const char *const bootloader_id[] = {
	"LILO",
	"Loadlin",
	"bootsect-loader",
	"Syslinux",
	"Etherboot/gPXE/iPXE",
	"ELILO",
	"undefined",
	"GRUB",
	"U-Boot",
	"Xen",
	"Gujin",
	"Qemu",
	"Arcturus Networks uCbootloader",
	"kexec-tools",
	"Extended",
	"Special",
	"Reserved",
	"Minimal Linux Bootloader",
	"OVMF UEFI virtualization stack",
};

struct flag_info {
	uint bit;
	const char *name;
};

static struct flag_info load_flags[] = {
	{ LOADED_HIGH, "loaded-high" },
	{ QUIET_FLAG, "quiet" },
	{ KEEP_SEGMENTS, "keep-segments" },
	{ CAN_USE_HEAP, "can-use-heap" },
};

static struct flag_info xload_flags[] = {
	{ XLF_KERNEL_64, "64-bit-entry" },
	{ XLF_CAN_BE_LOADED_ABOVE_4G, "can-load-above-4gb" },
	{ XLF_EFI_HANDOVER_32, "32-efi-handoff" },
	{ XLF_EFI_HANDOVER_64, "64-efi-handoff" },
	{ XLF_EFI_KEXEC, "kexec-efi-runtime" },
};

static void print_flags(struct flag_info *flags, int count, uint value)
{
	int i;

	printf("%-20s:", "");
	for (i = 0; i < count; i++) {
		uint mask = flags[i].bit;

		if (value & mask)
			printf(" %s", flags[i].name);
	}
	printf("\n");
}

static void show_loader(struct setup_header *hdr)
{
	bool version_valid = false;
	int type, version;
	const char *name;

	type = hdr->type_of_loader >> 4;
	version = hdr->type_of_loader & 0xf;
	if (type == 0xe)
		type = 0x10 + hdr->ext_loader_type;
	version |= hdr->ext_loader_ver << 4;
	if (!hdr->type_of_loader) {
		name = "pre-2.00 bootloader";
	} else if (hdr->type_of_loader == 0xff) {
		name = "unknown";
	} else if (type < ARRAY_SIZE(bootloader_id)) {
		name = bootloader_id[type];
		version_valid = true;
	} else {
		name = "undefined";
	}
	printf("%20s  %s", "", name);
	if (version_valid)
		printf(", version %x", version);
	printf("\n");
}

void zimage_dump(struct boot_params *base_ptr, bool show_cmdline)
{
	struct setup_header *hdr;
	const char *version;

	printf("Setup located at %p:\n\n", base_ptr);
	print_num64("ACPI RSDP addr", base_ptr->acpi_rsdp_addr);

	printf("E820: %d entries\n", base_ptr->e820_entries);
	if (base_ptr->e820_entries)
		e820_dump(base_ptr->e820_map, base_ptr->e820_entries);

	hdr = &base_ptr->hdr;
	print_num("Setup sectors", hdr->setup_sects);
	print_num("Root flags", hdr->root_flags);
	print_num("Sys size", hdr->syssize);
	print_num("RAM size", hdr->ram_size);
	print_num("Video mode", hdr->vid_mode);
	print_num("Root dev", hdr->root_dev);
	print_num("Boot flag", hdr->boot_flag);
	print_num("Jump", hdr->jump);
	print_num("Header", hdr->header);
	if (hdr->header == KERNEL_V2_MAGIC)
		printf("%-20s  %s\n", "", "Kernel V2");
	else
		printf("%-20s  %s\n", "", "Ancient kernel, using version 100");
	print_num("Version", hdr->version);
	print_num("Real mode switch", hdr->realmode_swtch);
	print_num("Start sys seg", hdr->start_sys_seg);
	print_num("Kernel version", hdr->kernel_version);
	version = zimage_get_kernel_version(base_ptr,
					    (void *)state.bzimage_addr);
	if (version)
		printf("   @%p: %s\n", version, version);
	print_num("Type of loader", hdr->type_of_loader);
	show_loader(hdr);
	print_num("Load flags", hdr->loadflags);
	print_flags(load_flags, ARRAY_SIZE(load_flags), hdr->loadflags);
	print_num("Setup move size", hdr->setup_move_size);
	print_num("Code32 start", hdr->code32_start);
	print_num("Ramdisk image", hdr->ramdisk_image);
	print_num("Ramdisk size", hdr->ramdisk_size);
	print_num("Bootsect kludge", hdr->bootsect_kludge);
	print_num("Heap end ptr", hdr->heap_end_ptr);
	print_num("Ext loader ver", hdr->ext_loader_ver);
	print_num("Ext loader type", hdr->ext_loader_type);
	print_num("Command line ptr", hdr->cmd_line_ptr);
	if (show_cmdline && hdr->cmd_line_ptr) {
		printf("   ");
		/* Use puts() to avoid limits from CONFIG_SYS_PBSIZE */
		puts((char *)(ulong)hdr->cmd_line_ptr);
		printf("\n");
	}
	print_num("Initrd addr max", hdr->initrd_addr_max);
	print_num("Kernel alignment", hdr->kernel_alignment);
	print_num("Relocatable kernel", hdr->relocatable_kernel);
	print_num("Min alignment", hdr->min_alignment);
	if (hdr->min_alignment)
		printf("%-20s: %x\n", "", 1 << hdr->min_alignment);
	print_num("Xload flags", hdr->xloadflags);
	print_flags(xload_flags, ARRAY_SIZE(xload_flags), hdr->xloadflags);
	print_num("Cmdline size", hdr->cmdline_size);
	print_num("Hardware subarch", hdr->hardware_subarch);
	print_num64("HW subarch data", hdr->hardware_subarch_data);
	print_num("Payload offset", hdr->payload_offset);
	print_num("Payload length", hdr->payload_length);
	print_num64("Setup data", hdr->setup_data);
	print_num64("Pref address", hdr->pref_address);
	print_num("Init size", hdr->init_size);
	print_num("Handover offset", hdr->handover_offset);
	if (get_boot_protocol(hdr, false) >= 0x215)
		print_num("Kernel info offset", hdr->kernel_info_offset);
}

void zboot_start(ulong bzimage_addr, ulong bzimage_size, ulong initrd_addr,
		 ulong initrd_size, ulong base_addr, const char *cmdline)
{
	memset(&state, '\0', sizeof(state));

	state.bzimage_size = bzimage_size;
	state.initrd_addr = initrd_addr;
	state.initrd_size = initrd_size;
	if (base_addr) {
		state.base_ptr = map_sysmem(base_addr, 0);
		state.load_address = bzimage_addr;
	} else {
		state.bzimage_addr = bzimage_addr;
	}
	state.cmdline = cmdline;
}

void zboot_info(void)
{
	printf("Kernel loaded at %08lx, setup_base=%p\n",
	       state.load_address, state.base_ptr);
}
