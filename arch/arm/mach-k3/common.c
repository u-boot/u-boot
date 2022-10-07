// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: Common Architecture initialization
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include "common.h"
#include <dm.h>
#include <remoteproc.h>
#include <asm/cache.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <fdt_support.h>
#include <asm/arch/sys_proto.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <fs_loader.h>
#include <fs.h>
#include <env.h>
#include <elf.h>
#include <soc.h>

#if IS_ENABLED(CONFIG_SYS_K3_SPL_ATF)
enum {
	IMAGE_ID_ATF,
	IMAGE_ID_OPTEE,
	IMAGE_ID_SPL,
	IMAGE_ID_DM_FW,
	IMAGE_AMT,
};

#if CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS)
static const char *image_os_match[IMAGE_AMT] = {
	"arm-trusted-firmware",
	"tee",
	"U-Boot",
	"DM",
};
#endif

static struct image_info fit_image_info[IMAGE_AMT];
#endif

struct ti_sci_handle *get_ti_sci_handle(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_FIRMWARE,
					  DM_DRIVER_GET(ti_sci), &dev);
	if (ret)
		panic("Failed to get SYSFW (%d)\n", ret);

	return (struct ti_sci_handle *)ti_sci_get_handle_from_sysfw(dev);
}

void k3_sysfw_print_ver(void)
{
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	char fw_desc[sizeof(ti_sci->version.firmware_description) + 1];

	/*
	 * Output System Firmware version info. Note that since the
	 * 'firmware_description' field is not guaranteed to be zero-
	 * terminated we manually add a \0 terminator if needed. Further
	 * note that we intentionally no longer rely on the extended
	 * printf() formatter '%.*s' to not having to require a more
	 * full-featured printf() implementation.
	 */
	strncpy(fw_desc, ti_sci->version.firmware_description,
		sizeof(ti_sci->version.firmware_description));
	fw_desc[sizeof(fw_desc) - 1] = '\0';

	printf("SYSFW ABI: %d.%d (firmware rev 0x%04x '%s')\n",
	       ti_sci->version.abi_major, ti_sci->version.abi_minor,
	       ti_sci->version.firmware_revision, fw_desc);
}

void mmr_unlock(phys_addr_t base, u32 partition)
{
	/* Translate the base address */
	phys_addr_t part_base = base + partition * CTRL_MMR0_PARTITION_SIZE;

	/* Unlock the requested partition if locked using two-step sequence */
	writel(CTRLMMR_LOCK_KICK0_UNLOCK_VAL, part_base + CTRLMMR_LOCK_KICK0);
	writel(CTRLMMR_LOCK_KICK1_UNLOCK_VAL, part_base + CTRLMMR_LOCK_KICK1);
}

bool is_rom_loaded_sysfw(struct rom_extended_boot_data *data)
{
	if (strncmp(data->header, K3_ROM_BOOT_HEADER_MAGIC, 7))
		return false;

	return data->num_components > 1;
}

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_K3_EARLY_CONS
int early_console_init(void)
{
	struct udevice *dev;
	int ret;

	gd->baudrate = CONFIG_BAUDRATE;

	ret = uclass_get_device_by_seq(UCLASS_SERIAL, CONFIG_K3_EARLY_CONS_IDX,
				       &dev);
	if (ret) {
		printf("Error getting serial dev for early console! (%d)\n",
		       ret);
		return ret;
	}

	gd->cur_serial_dev = dev;
	gd->flags |= GD_FLG_SERIAL_READY;
	gd->have_console = 1;

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_SYS_K3_SPL_ATF)

void init_env(void)
{
#ifdef CONFIG_SPL_ENV_SUPPORT
	char *part;

	env_init();
	env_relocate();
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC2:
		part = env_get("bootpart");
		env_set("storage_interface", "mmc");
		env_set("fw_dev_part", part);
		break;
	case BOOT_DEVICE_SPI:
		env_set("storage_interface", "ubi");
		env_set("fw_ubi_mtdpart", "UBI");
		env_set("fw_ubi_volume", "UBI0");
		break;
	default:
		printf("%s from device %u not supported!\n",
		       __func__, spl_boot_device());
		return;
	}
#endif
}

int load_firmware(char *name_fw, char *name_loadaddr, u32 *loadaddr)
{
	struct udevice *fsdev;
	char *name = NULL;
	int size = 0;

	if (!IS_ENABLED(CONFIG_FS_LOADER))
		return 0;

	*loadaddr = 0;
#ifdef CONFIG_SPL_ENV_SUPPORT
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC2:
		name = env_get(name_fw);
		*loadaddr = env_get_hex(name_loadaddr, *loadaddr);
		break;
	default:
		printf("Loading rproc fw image from device %u not supported!\n",
		       spl_boot_device());
		return 0;
	}
#endif
	if (!*loadaddr)
		return 0;

	if (!uclass_get_device(UCLASS_FS_FIRMWARE_LOADER, 0, &fsdev)) {
		size = request_firmware_into_buf(fsdev, name, (void *)*loadaddr,
						 0, 0);
	}

	return size;
}

__weak void release_resources_for_core_shutdown(void)
{
	debug("%s not implemented...\n", __func__);
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	u32 loadaddr = 0;
	int ret, size = 0, shut_cpu = 0;

	/* Release all the exclusive devices held by SPL before starting ATF */
	ti_sci->ops.dev_ops.release_exclusive_devices(ti_sci);

	ret = rproc_init();
	if (ret)
		panic("rproc failed to be initialized (%d)\n", ret);

	init_env();

	if (!fit_image_info[IMAGE_ID_DM_FW].image_start) {
		size = load_firmware("name_mcur5f0_0fw", "addr_mcur5f0_0load",
				     &loadaddr);
	}

	/*
	 * It is assumed that remoteproc device 1 is the corresponding
	 * Cortex-A core which runs ATF. Make sure DT reflects the same.
	 */
	if (!fit_image_info[IMAGE_ID_ATF].image_start)
		fit_image_info[IMAGE_ID_ATF].image_start =
			spl_image->entry_point;

	ret = rproc_load(1, fit_image_info[IMAGE_ID_ATF].image_start, 0x200);
	if (ret)
		panic("%s: ATF failed to load on rproc (%d)\n", __func__, ret);

	if (!fit_image_info[IMAGE_ID_DM_FW].image_len &&
	    !(size > 0 && valid_elf_image(loadaddr))) {
		shut_cpu = 1;
		goto start_arm64;
	}

	if (!fit_image_info[IMAGE_ID_DM_FW].image_start) {
		loadaddr = load_elf_image_phdr(loadaddr);
	} else {
		loadaddr = fit_image_info[IMAGE_ID_DM_FW].image_start;
		if (valid_elf_image(loadaddr))
			loadaddr = load_elf_image_phdr(loadaddr);
	}

	debug("%s: jumping to address %x\n", __func__, loadaddr);

start_arm64:
	/* Add an extra newline to differentiate the ATF logs from SPL */
	printf("Starting ATF on ARM64 core...\n\n");

	ret = rproc_start(1);
	if (ret)
		panic("%s: ATF failed to start on rproc (%d)\n", __func__, ret);

	if (shut_cpu) {
		debug("Shutting down...\n");
		release_resources_for_core_shutdown();

		while (1)
			asm volatile("wfe");
	}
	image_entry_noargs_t image_entry = (image_entry_noargs_t)loadaddr;

	image_entry();
}
#endif

#if CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS)
void board_fit_image_post_process(const void *fit, int node, void **p_image,
				  size_t *p_size)
{
#if IS_ENABLED(CONFIG_SYS_K3_SPL_ATF)
	int len;
	int i;
	const char *os;
	u32 addr;

	os = fdt_getprop(fit, node, "os", &len);
	addr = fdt_getprop_u32_default_node(fit, node, 0, "entry", -1);

	debug("%s: processing image: addr=%x, size=%d, os=%s\n", __func__,
	      addr, *p_size, os);

	for (i = 0; i < IMAGE_AMT; i++) {
		if (!strcmp(os, image_os_match[i])) {
			fit_image_info[i].image_start = addr;
			fit_image_info[i].image_len = *p_size;
			debug("%s: matched image for ID %d\n", __func__, i);
			break;
		}
	}
#endif

	ti_secure_image_post_process(p_image, p_size);
}
#endif

#if defined(CONFIG_OF_LIBFDT)
int fdt_fixup_msmc_ram(void *blob, char *parent_path, char *node_name)
{
	u64 msmc_start = 0, msmc_end = 0, msmc_size, reg[2];
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	int ret, node, subnode, len, prev_node;
	u32 range[4], addr, size;
	const fdt32_t *sub_reg;

	ti_sci->ops.core_ops.query_msmc(ti_sci, &msmc_start, &msmc_end);
	msmc_size = msmc_end - msmc_start + 1;
	debug("%s: msmc_start = 0x%llx, msmc_size = 0x%llx\n", __func__,
	      msmc_start, msmc_size);

	/* find or create "msmc_sram node */
	ret = fdt_path_offset(blob, parent_path);
	if (ret < 0)
		return ret;

	node = fdt_find_or_add_subnode(blob, ret, node_name);
	if (node < 0)
		return node;

	ret = fdt_setprop_string(blob, node, "compatible", "mmio-sram");
	if (ret < 0)
		return ret;

	reg[0] = cpu_to_fdt64(msmc_start);
	reg[1] = cpu_to_fdt64(msmc_size);
	ret = fdt_setprop(blob, node, "reg", reg, sizeof(reg));
	if (ret < 0)
		return ret;

	fdt_setprop_cell(blob, node, "#address-cells", 1);
	fdt_setprop_cell(blob, node, "#size-cells", 1);

	range[0] = 0;
	range[1] = cpu_to_fdt32(msmc_start >> 32);
	range[2] = cpu_to_fdt32(msmc_start & 0xffffffff);
	range[3] = cpu_to_fdt32(msmc_size);
	ret = fdt_setprop(blob, node, "ranges", range, sizeof(range));
	if (ret < 0)
		return ret;

	subnode = fdt_first_subnode(blob, node);
	prev_node = 0;

	/* Look for invalid subnodes and delete them */
	while (subnode >= 0) {
		sub_reg = fdt_getprop(blob, subnode, "reg", &len);
		addr = fdt_read_number(sub_reg, 1);
		sub_reg++;
		size = fdt_read_number(sub_reg, 1);
		debug("%s: subnode = %d, addr = 0x%x. size = 0x%x\n", __func__,
		      subnode, addr, size);
		if (addr + size > msmc_size ||
		    !strncmp(fdt_get_name(blob, subnode, &len), "sysfw", 5) ||
		    !strncmp(fdt_get_name(blob, subnode, &len), "l3cache", 7)) {
			fdt_del_node(blob, subnode);
			debug("%s: deleting subnode %d\n", __func__, subnode);
			if (!prev_node)
				subnode = fdt_first_subnode(blob, node);
			else
				subnode = fdt_next_subnode(blob, prev_node);
		} else {
			prev_node = subnode;
			subnode = fdt_next_subnode(blob, prev_node);
		}
	}

	return 0;
}

int fdt_disable_node(void *blob, char *node_path)
{
	int offs;
	int ret;

	offs = fdt_path_offset(blob, node_path);
	if (offs < 0) {
		printf("Node %s not found.\n", node_path);
		return offs;
	}
	ret = fdt_setprop_string(blob, offs, "status", "disabled");
	if (ret < 0) {
		printf("Could not add status property to node %s: %s\n",
		       node_path, fdt_strerror(ret));
		return ret;
	}
	return 0;
}

#endif

#ifndef CONFIG_SYSRESET
void reset_cpu(void)
{
}
#endif

enum k3_device_type get_device_type(void)
{
	u32 sys_status = readl(K3_SEC_MGR_SYS_STATUS);

	u32 sys_dev_type = (sys_status & SYS_STATUS_DEV_TYPE_MASK) >>
			SYS_STATUS_DEV_TYPE_SHIFT;

	u32 sys_sub_type = (sys_status & SYS_STATUS_SUB_TYPE_MASK) >>
			SYS_STATUS_SUB_TYPE_SHIFT;

	switch (sys_dev_type) {
	case SYS_STATUS_DEV_TYPE_GP:
		return K3_DEVICE_TYPE_GP;
	case SYS_STATUS_DEV_TYPE_TEST:
		return K3_DEVICE_TYPE_TEST;
	case SYS_STATUS_DEV_TYPE_EMU:
		return K3_DEVICE_TYPE_EMU;
	case SYS_STATUS_DEV_TYPE_HS:
		if (sys_sub_type == SYS_STATUS_SUB_TYPE_VAL_FS)
			return K3_DEVICE_TYPE_HS_FS;
		else
			return K3_DEVICE_TYPE_HS_SE;
	default:
		return K3_DEVICE_TYPE_BAD;
	}
}

#if defined(CONFIG_DISPLAY_CPUINFO)
static const char *get_device_type_name(void)
{
	enum k3_device_type type = get_device_type();

	switch (type) {
	case K3_DEVICE_TYPE_GP:
		return "GP";
	case K3_DEVICE_TYPE_TEST:
		return "TEST";
	case K3_DEVICE_TYPE_EMU:
		return "EMU";
	case K3_DEVICE_TYPE_HS_FS:
		return "HS-FS";
	case K3_DEVICE_TYPE_HS_SE:
		return "HS-SE";
	default:
		return "BAD";
	}
}

int print_cpuinfo(void)
{
	struct udevice *soc;
	char name[64];
	int ret;

	printf("SoC:   ");

	ret = soc_get(&soc);
	if (ret) {
		printf("UNKNOWN\n");
		return 0;
	}

	ret = soc_get_family(soc, name, 64);
	if (!ret) {
		printf("%s ", name);
	}

	ret = soc_get_revision(soc, name, 64);
	if (!ret) {
		printf("%s ", name);
	}

	printf("%s\n", get_device_type_name());

	return 0;
}
#endif

bool soc_is_j721e(void)
{
	u32 soc;

	soc = (readl(CTRLMMR_WKUP_JTAG_ID) &
		JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT;

	return soc == J721E;
}

bool soc_is_j7200(void)
{
	u32 soc;

	soc = (readl(CTRLMMR_WKUP_JTAG_ID) &
		JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT;

	return soc == J7200;
}

#ifdef CONFIG_ARM64
void board_prep_linux(struct bootm_headers *images)
{
	debug("Linux kernel Image start = 0x%lx end = 0x%lx\n",
	      images->os.start, images->os.end);
	__asm_flush_dcache_range(images->os.start,
				 ROUND(images->os.end,
				       CONFIG_SYS_CACHELINE_SIZE));
}
#endif

#ifdef CONFIG_CPU_V7R
void disable_linefill_optimization(void)
{
	u32 actlr;

	/*
	 * On K3 devices there are 2 conditions where R5F can deadlock:
	 * 1.When software is performing series of store operations to
	 *   cacheable write back/write allocate memory region and later
	 *   on software execute barrier operation (DSB or DMB). R5F may
	 *   hang at the barrier instruction.
	 * 2.When software is performing a mix of load and store operations
	 *   within a tight loop and store operations are all writing to
	 *   cacheable write back/write allocates memory regions, R5F may
	 *   hang at one of the load instruction.
	 *
	 * To avoid the above two conditions disable linefill optimization
	 * inside Cortex R5F.
	 */
	asm("mrc p15, 0, %0, c1, c0, 1" : "=r" (actlr));
	actlr |= (1 << 13); /* Set DLFO bit  */
	asm("mcr p15, 0, %0, c1, c0, 1" : : "r" (actlr));
}
#endif

void remove_fwl_configs(struct fwl_data *fwl_data, size_t fwl_data_size)
{
	struct ti_sci_msg_fwl_region region;
	struct ti_sci_fwl_ops *fwl_ops;
	struct ti_sci_handle *ti_sci;
	size_t i, j;

	ti_sci = get_ti_sci_handle();
	fwl_ops = &ti_sci->ops.fwl_ops;
	for (i = 0; i < fwl_data_size; i++) {
		for (j = 0; j <  fwl_data[i].regions; j++) {
			region.fwl_id = fwl_data[i].fwl_id;
			region.region = j;
			region.n_permission_regs = 3;

			fwl_ops->get_fwl_region(ti_sci, &region);

			if (region.control != 0) {
				pr_debug("Attempting to disable firewall %5d (%25s)\n",
					 region.fwl_id, fwl_data[i].name);
				region.control = 0;

				if (fwl_ops->set_fwl_region(ti_sci, &region))
					pr_err("Could not disable firewall %5d (%25s)\n",
					       region.fwl_id, fwl_data[i].name);
			}
		}
	}
}

void spl_enable_dcache(void)
{
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	phys_addr_t ram_top = CONFIG_SYS_SDRAM_BASE;

	dram_init();

	/* reserve TLB table */
	gd->arch.tlb_size = PGTABLE_SIZE;

	ram_top += get_effective_memsize();
	/* keep ram_top in the 32-bit address space */
	if (ram_top >= 0x100000000)
		ram_top = (phys_addr_t) 0x100000000;

	gd->arch.tlb_addr = ram_top - gd->arch.tlb_size;
	debug("TLB table from %08lx to %08lx\n", gd->arch.tlb_addr,
	      gd->arch.tlb_addr + gd->arch.tlb_size);

	dcache_enable();
#endif
}

#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
void spl_board_prepare_for_boot(void)
{
	dcache_disable();
}

void spl_board_prepare_for_linux(void)
{
	dcache_disable();
}
#endif

int misc_init_r(void)
{
	if (IS_ENABLED(CONFIG_TI_AM65_CPSW_NUSS)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(am65_cpsw_nuss),
						  &dev);
		if (ret)
			printf("Failed to probe am65_cpsw_nuss driver\n");
	}

	/* Default FIT boot on non-GP devices */
	if (get_device_type() != K3_DEVICE_TYPE_GP)
		env_set("boot_fit", "1");

	return 0;
}
