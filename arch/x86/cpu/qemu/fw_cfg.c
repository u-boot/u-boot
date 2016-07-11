/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/fw_cfg.h>
#include <asm/tables.h>
#include <asm/e820.h>
#include <linux/list.h>
#include <memalign.h>

static bool fwcfg_present;
static bool fwcfg_dma_present;

static LIST_HEAD(fw_list);

/* Read configuration item using fw_cfg PIO interface */
static void qemu_fwcfg_read_entry_pio(uint16_t entry,
		uint32_t size, void *address)
{
	uint32_t i = 0;
	uint8_t *data = address;

	/*
	 * writting FW_CFG_INVALID will cause read operation to resume at
	 * last offset, otherwise read will start at offset 0
	 */
	if (entry != FW_CFG_INVALID)
		outw(entry, FW_CONTROL_PORT);
	while (size--)
		data[i++] = inb(FW_DATA_PORT);
}

/* Read configuration item using fw_cfg DMA interface */
static void qemu_fwcfg_read_entry_dma(uint16_t entry,
		uint32_t size, void *address)
{
	struct fw_cfg_dma_access dma;

	dma.length = cpu_to_be32(size);
	dma.address = cpu_to_be64((uintptr_t)address);
	dma.control = cpu_to_be32(FW_CFG_DMA_READ);

	/*
	 * writting FW_CFG_INVALID will cause read operation to resume at
	 * last offset, otherwise read will start at offset 0
	 */
	if (entry != FW_CFG_INVALID)
		dma.control |= cpu_to_be32(FW_CFG_DMA_SELECT | (entry << 16));

	barrier();

	debug("qemu_fwcfg_dma_read_entry: addr %p, length %u control 0x%x\n",
	      address, size, be32_to_cpu(dma.control));

	outl(cpu_to_be32((uint32_t)&dma), FW_DMA_PORT_HIGH);

	while (be32_to_cpu(dma.control) & ~FW_CFG_DMA_ERROR)
		__asm__ __volatile__ ("pause");
}

static bool qemu_fwcfg_present(void)
{
	uint32_t qemu;

	qemu_fwcfg_read_entry_pio(FW_CFG_SIGNATURE, 4, &qemu);
	return be32_to_cpu(qemu) == QEMU_FW_CFG_SIGNATURE;
}

static bool qemu_fwcfg_dma_present(void)
{
	uint8_t dma_enabled;

	qemu_fwcfg_read_entry_pio(FW_CFG_ID, 1, &dma_enabled);
	if (dma_enabled & FW_CFG_DMA_ENABLED)
		return true;

	return false;
}

static void qemu_fwcfg_read_entry(uint16_t entry,
		uint32_t length, void *address)
{
	if (fwcfg_dma_present)
		qemu_fwcfg_read_entry_dma(entry, length, address);
	else
		qemu_fwcfg_read_entry_pio(entry, length, address);
}

int qemu_fwcfg_online_cpus(void)
{
	uint16_t nb_cpus;

	if (!fwcfg_present)
		return -ENODEV;

	qemu_fwcfg_read_entry(FW_CFG_NB_CPUS, 2, &nb_cpus);

	return le16_to_cpu(nb_cpus);
}

/*
 * This function prepares kernel for zboot. It loads kernel data
 * to 'load_addr', initrd to 'initrd_addr' and kernel command
 * line using qemu fw_cfg interface.
 */
static int qemu_fwcfg_setup_kernel(void *load_addr, void *initrd_addr)
{
	char *data_addr;
	uint32_t setup_size, kernel_size, cmdline_size, initrd_size;

	qemu_fwcfg_read_entry(FW_CFG_SETUP_SIZE, 4, &setup_size);
	qemu_fwcfg_read_entry(FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	if (setup_size == 0 || kernel_size == 0) {
		printf("warning: no kernel available\n");
		return -1;
	}

	data_addr = load_addr;
	qemu_fwcfg_read_entry(FW_CFG_SETUP_DATA,
			      le32_to_cpu(setup_size), data_addr);
	data_addr += le32_to_cpu(setup_size);

	qemu_fwcfg_read_entry(FW_CFG_KERNEL_DATA,
			      le32_to_cpu(kernel_size), data_addr);
	data_addr += le32_to_cpu(kernel_size);

	data_addr = initrd_addr;
	qemu_fwcfg_read_entry(FW_CFG_INITRD_SIZE, 4, &initrd_size);
	if (initrd_size == 0) {
		printf("warning: no initrd available\n");
	} else {
		qemu_fwcfg_read_entry(FW_CFG_INITRD_DATA,
				      le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
	}

	qemu_fwcfg_read_entry(FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	if (cmdline_size) {
		qemu_fwcfg_read_entry(FW_CFG_CMDLINE_DATA,
				      le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		if (*data_addr != '\0') {
			if (setenv("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %p size %x", load_addr,
	       le32_to_cpu(kernel_size));
	if (initrd_size)
		printf(" initrd %p size %x\n",
		       initrd_addr,
		       le32_to_cpu(initrd_size));
	else
		printf("\n");

	return 0;
}

static int qemu_fwcfg_read_firmware_list(void)
{
	int i;
	uint32_t count;
	struct fw_file *file;
	struct list_head *entry;

	/* don't read it twice */
	if (!list_empty(&fw_list))
		return 0;

	qemu_fwcfg_read_entry(FW_CFG_FILE_DIR, 4, &count);
	if (!count)
		return 0;

	count = be32_to_cpu(count);
	for (i = 0; i < count; i++) {
		file = malloc(sizeof(*file));
		if (!file) {
			printf("error: allocating resource\n");
			goto err;
		}
		qemu_fwcfg_read_entry(FW_CFG_INVALID,
				      sizeof(struct fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &fw_list);
	}

	return 0;

err:
	list_for_each(entry, &fw_list) {
		file = list_entry(entry, struct fw_file, list);
		free(file);
	}

	return -ENOMEM;
}

#ifdef CONFIG_QEMU_ACPI_TABLE
static struct fw_file *qemu_fwcfg_find_file(const char *name)
{
	struct list_head *entry;
	struct fw_file *file;

	list_for_each(entry, &fw_list) {
		file = list_entry(entry, struct fw_file, list);
		if (!strcmp(file->cfg.name, name))
			return file;
	}

	return NULL;
}

/*
 * This function allocates memory for ACPI tables
 *
 * @entry : BIOS linker command entry which tells where to allocate memory
 *          (either high memory or low memory)
 * @addr  : The address that should be used for low memory allcation. If the
 *          memory allocation request is 'ZONE_HIGH' then this parameter will
 *          be ignored.
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_allocate(struct bios_linker_entry *entry, u32 *addr)
{
	uint32_t size, align;
	struct fw_file *file;
	unsigned long aligned_addr;

	align = le32_to_cpu(entry->alloc.align);
	/* align must be power of 2 */
	if (align & (align - 1)) {
		printf("error: wrong alignment %u\n", align);
		return -EINVAL;
	}

	file = qemu_fwcfg_find_file(entry->alloc.file);
	if (!file) {
		printf("error: can't find file %s\n", entry->alloc.file);
		return -ENOENT;
	}

	size = be32_to_cpu(file->cfg.size);

	/*
	 * ZONE_HIGH means we need to allocate from high memory, since
	 * malloc space is already at the end of RAM, so we directly use it.
	 * If allocation zone is ZONE_FSEG, then we use the 'addr' passed
	 * in which is low memory
	 */
	if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_HIGH) {
		aligned_addr = (unsigned long)memalign(align, size);
		if (!aligned_addr) {
			printf("error: allocating resource\n");
			return -ENOMEM;
		}
	} else if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG) {
		aligned_addr = ALIGN(*addr, align);
	} else {
		printf("error: invalid allocation zone\n");
		return -EINVAL;
	}

	debug("bios_linker_allocate: allocate file %s, size %u, zone %d, align %u, addr 0x%lx\n",
	      file->cfg.name, size, entry->alloc.zone, align, aligned_addr);

	qemu_fwcfg_read_entry(be16_to_cpu(file->cfg.select),
			      size, (void *)aligned_addr);
	file->addr = aligned_addr;

	/* adjust address for low memory allocation */
	if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG)
		*addr = (aligned_addr + size);

	return 0;
}

/*
 * This function patches ACPI tables previously loaded
 * by bios_linker_allocate()
 *
 * @entry : BIOS linker command entry which tells how to patch
 *          ACPI tables
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_add_pointer(struct bios_linker_entry *entry)
{
	struct fw_file *dest, *src;
	uint32_t offset = le32_to_cpu(entry->pointer.offset);
	uint64_t pointer = 0;

	dest = qemu_fwcfg_find_file(entry->pointer.dest_file);
	if (!dest || !dest->addr)
		return -ENOENT;
	src = qemu_fwcfg_find_file(entry->pointer.src_file);
	if (!src || !src->addr)
		return -ENOENT;

	debug("bios_linker_add_pointer: dest->addr 0x%lx, src->addr 0x%lx, offset 0x%x size %u, 0x%llx\n",
	      dest->addr, src->addr, offset, entry->pointer.size, pointer);

	memcpy(&pointer, (char *)dest->addr + offset, entry->pointer.size);
	pointer	= le64_to_cpu(pointer);
	pointer += (unsigned long)src->addr;
	pointer	= cpu_to_le64(pointer);
	memcpy((char *)dest->addr + offset, &pointer, entry->pointer.size);

	return 0;
}

/*
 * This function updates checksum fields of ACPI tables previously loaded
 * by bios_linker_allocate()
 *
 * @entry : BIOS linker command entry which tells where to update ACPI table
 *          checksums
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_add_checksum(struct bios_linker_entry *entry)
{
	struct fw_file *file;
	uint8_t *data, cksum = 0;
	uint8_t *cksum_start;

	file = qemu_fwcfg_find_file(entry->cksum.file);
	if (!file || !file->addr)
		return -ENOENT;

	data = (uint8_t *)(file->addr + le32_to_cpu(entry->cksum.offset));
	cksum_start = (uint8_t *)(file->addr + le32_to_cpu(entry->cksum.start));
	cksum = table_compute_checksum(cksum_start,
				       le32_to_cpu(entry->cksum.length));
	*data = cksum;

	return 0;
}

unsigned install_e820_map(unsigned max_entries, struct e820entry *entries)
{
	entries[0].addr = 0;
	entries[0].size = ISA_START_ADDRESS;
	entries[0].type = E820_RAM;

	entries[1].addr = ISA_START_ADDRESS;
	entries[1].size = ISA_END_ADDRESS - ISA_START_ADDRESS;
	entries[1].type = E820_RESERVED;

	/*
	 * since we use memalign(malloc) to allocate high memory for
	 * storing ACPI tables, we need to reserve them in e820 tables,
	 * otherwise kernel will reclaim them and data will be corrupted
	 */
	entries[2].addr = ISA_END_ADDRESS;
	entries[2].size = gd->relocaddr - TOTAL_MALLOC_LEN - ISA_END_ADDRESS;
	entries[2].type = E820_RAM;

	/* for simplicity, reserve entire malloc space */
	entries[3].addr = gd->relocaddr - TOTAL_MALLOC_LEN;
	entries[3].size = TOTAL_MALLOC_LEN;
	entries[3].type = E820_RESERVED;

	entries[4].addr = gd->relocaddr;
	entries[4].size = gd->ram_size - gd->relocaddr;
	entries[4].type = E820_RESERVED;

	entries[5].addr = CONFIG_PCIE_ECAM_BASE;
	entries[5].size = CONFIG_PCIE_ECAM_SIZE;
	entries[5].type = E820_RESERVED;

	return 6;
}

/* This function loads and patches ACPI tables provided by QEMU */
u32 write_acpi_tables(u32 addr)
{
	int i, ret = 0;
	struct fw_file *file;
	struct bios_linker_entry *table_loader;
	struct bios_linker_entry *entry;
	uint32_t size;
	struct list_head *list;

	/* make sure fw_list is loaded */
	ret = qemu_fwcfg_read_firmware_list();
	if (ret) {
		printf("error: can't read firmware file list\n");
		return addr;
	}

	file = qemu_fwcfg_find_file("etc/table-loader");
	if (!file) {
		printf("error: can't find etc/table-loader\n");
		return addr;
	}

	size = be32_to_cpu(file->cfg.size);
	if ((size % sizeof(*entry)) != 0) {
		printf("error: table-loader maybe corrupted\n");
		return addr;
	}

	table_loader = malloc(size);
	if (!table_loader) {
		printf("error: no memory for table-loader\n");
		return addr;
	}

	qemu_fwcfg_read_entry(be16_to_cpu(file->cfg.select),
			      size, table_loader);

	for (i = 0; i < (size / sizeof(*entry)); i++) {
		entry = table_loader + i;
		switch (le32_to_cpu(entry->command)) {
		case BIOS_LINKER_LOADER_COMMAND_ALLOCATE:
			ret = bios_linker_allocate(entry, &addr);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_POINTER:
			ret = bios_linker_add_pointer(entry);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_CHECKSUM:
			ret = bios_linker_add_checksum(entry);
			if (ret)
				goto out;
			break;
		default:
			break;
		}
	}

out:
	if (ret) {
		list_for_each(list, &fw_list) {
			file = list_entry(list, struct fw_file, list);
			if (file->addr)
				free((void *)file->addr);
		}
	}

	free(table_loader);
	return addr;
}
#endif

static int qemu_fwcfg_list_firmware(void)
{
	int ret;
	struct list_head *entry;
	struct fw_file *file;

	/* make sure fw_list is loaded */
	ret = qemu_fwcfg_read_firmware_list();
	if (ret)
		return ret;

	list_for_each(entry, &fw_list) {
		file = list_entry(entry, struct fw_file, list);
		printf("%-56s\n", file->cfg.name);
	}

	return 0;
}

void qemu_fwcfg_init(void)
{
	fwcfg_present = qemu_fwcfg_present();
	if (fwcfg_present)
		fwcfg_dma_present = qemu_fwcfg_dma_present();
}

static int qemu_fwcfg_do_list(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	if (qemu_fwcfg_list_firmware() < 0)
		return CMD_RET_FAILURE;

	return 0;
}

static int qemu_fwcfg_do_cpus(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	int ret = qemu_fwcfg_online_cpus();
	if (ret < 0) {
		printf("QEMU fw_cfg interface not found\n");
		return CMD_RET_FAILURE;
	}

	printf("%d cpu(s) online\n", qemu_fwcfg_online_cpus());

	return 0;
}

static int qemu_fwcfg_do_load(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	char *env;
	void *load_addr;
	void *initrd_addr;

	env = getenv("loadaddr");
	load_addr = env ?
		(void *)simple_strtoul(env, NULL, 16) :
		(void *)CONFIG_LOADADDR;

	env = getenv("ramdiskaddr");
	initrd_addr = env ?
		(void *)simple_strtoul(env, NULL, 16) :
		(void *)CONFIG_RAMDISK_ADDR;

	if (argc == 2) {
		load_addr = (void *)simple_strtoul(argv[0], NULL, 16);
		initrd_addr = (void *)simple_strtoul(argv[1], NULL, 16);
	} else if (argc == 1) {
		load_addr = (void *)simple_strtoul(argv[0], NULL, 16);
	}

	return qemu_fwcfg_setup_kernel(load_addr, initrd_addr);
}

static cmd_tbl_t fwcfg_commands[] = {
	U_BOOT_CMD_MKENT(list, 0, 1, qemu_fwcfg_do_list, "", ""),
	U_BOOT_CMD_MKENT(cpus, 0, 1, qemu_fwcfg_do_cpus, "", ""),
	U_BOOT_CMD_MKENT(load, 2, 1, qemu_fwcfg_do_load, "", ""),
};

static int do_qemu_fw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	cmd_tbl_t *fwcfg_cmd;

	if (!fwcfg_present) {
		printf("QEMU fw_cfg interface not found\n");
		return CMD_RET_USAGE;
	}

	fwcfg_cmd = find_cmd_tbl(argv[1], fwcfg_commands,
				 ARRAY_SIZE(fwcfg_commands));
	argc -= 2;
	argv += 2;
	if (!fwcfg_cmd || argc > fwcfg_cmd->maxargs)
		return CMD_RET_USAGE;

	ret = fwcfg_cmd->cmd(fwcfg_cmd, flag, argc, argv);

	return cmd_process_error(fwcfg_cmd, ret);
}

U_BOOT_CMD(
	qfw,	4,	1,	do_qemu_fw,
	"QEMU firmware interface",
	"<command>\n"
	"    - list                             : print firmware(s) currently loaded\n"
	"    - cpus                             : print online cpu number\n"
	"    - load <kernel addr> <initrd addr> : load kernel and initrd (if any), and setup for zboot\n"
)
