/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <malloc.h>
#include <qemu_fw_cfg.h>
#include <asm/io.h>
#include <linux/list.h>

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

bool qemu_fwcfg_present(void)
{
	return fwcfg_present;
}

bool qemu_fwcfg_dma_present(void)
{
	return fwcfg_dma_present;
}

void qemu_fwcfg_read_entry(uint16_t entry, uint32_t length, void *address)
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

int qemu_fwcfg_read_firmware_list(void)
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

struct fw_file *qemu_fwcfg_find_file(const char *name)
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

struct fw_file *qemu_fwcfg_file_iter_init(struct fw_cfg_file_iter *iter)
{
	iter->entry = fw_list.next;
	return list_entry(iter->entry, struct fw_file, list);
}

struct fw_file *qemu_fwcfg_file_iter_next(struct fw_cfg_file_iter *iter)
{
	iter->entry = iter->entry->next;
	return list_entry(iter->entry, struct fw_file, list);
}

bool qemu_fwcfg_file_iter_end(struct fw_cfg_file_iter *iter)
{
	return iter->entry == &fw_list;
}

static int qemu_fwcfg_list_firmware(void)
{
	int ret;
	struct fw_cfg_file_iter iter;
	struct fw_file *file;

	/* make sure fw_list is loaded */
	ret = qemu_fwcfg_read_firmware_list();
	if (ret)
		return ret;


	for (file = qemu_fwcfg_file_iter_init(&iter);
	     !qemu_fwcfg_file_iter_end(&iter);
	     file = qemu_fwcfg_file_iter_next(&iter)) {
		printf("%-56s\n", file->cfg.name);
	}

	return 0;
}

void qemu_fwcfg_init(void)
{
	uint32_t qemu;
	uint32_t dma_enabled;

	fwcfg_present = false;
	fwcfg_dma_present = false;

	qemu_fwcfg_read_entry_pio(FW_CFG_SIGNATURE, 4, &qemu);
	if (be32_to_cpu(qemu) == QEMU_FW_CFG_SIGNATURE)
		fwcfg_present = true;

	if (fwcfg_present) {
		qemu_fwcfg_read_entry_pio(FW_CFG_ID, 1, &dma_enabled);
		if (dma_enabled & FW_CFG_DMA_ENABLED)
			fwcfg_dma_present = true;
	}
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

	if (!qemu_fwcfg_present()) {
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
