// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: Common Architecture initialization
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <config.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <linux/printk.h>
#include "common.h"
#include <dm.h>
#include <remoteproc.h>
#include <asm/cache.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <fdt_support.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <fs_loader.h>
#include <fs.h>
#include <efi_loader.h>
#include <env.h>
#include <elf.h>
#include <soc.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#define PROC_BOOT_CTRL_FLAG_R5_CORE_HALT	0x00000001
#define PROC_BOOT_STATUS_FLAG_R5_WFI		0x00000002
#define PROC_ID_MCU_R5FSS0_CORE1		0x02
#define PROC_BOOT_CFG_FLAG_R5_LOCKSTEP		0x00000100

#include <asm/arch/k3-qos.h>

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

void __maybe_unused k3_dm_print_ver(void)
{
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	struct ti_sci_firmware_ops *fw_ops = &ti_sci->ops.fw_ops;
	struct ti_sci_dm_version_info dm_info = {0};
	u64 fw_caps;
	int ret;

	ret = fw_ops->query_dm_cap(ti_sci, &fw_caps);
	if (ret) {
		printf("Failed to query DM firmware capability %d\n", ret);
		return;
	}

	if (!(fw_caps & TI_SCI_MSG_FLAG_FW_CAP_DM))
		return;

	ret = fw_ops->get_dm_version(ti_sci, &dm_info);
	if (ret) {
		printf("Failed to fetch DM firmware version %d\n", ret);
		return;
	}

	printf("DM ABI: %d.%d (firmware ver 0x%04x '%s--%s' "
	       "patch_ver: %d)\n", dm_info.abi_major, dm_info.abi_minor,
	       dm_info.dm_ver, dm_info.sci_server_version,
	       dm_info.rm_pm_hal_version, dm_info.patch_ver);
}

void mmr_unlock(uintptr_t base, u32 partition)
{
	/* Translate the base address */
	uintptr_t part_base = base + partition * CTRL_MMR0_PARTITION_SIZE;

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
	gd->flags |= GD_FLG_HAVE_CONSOLE;

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS) && !IS_ENABLED(CONFIG_SYS_K3_SPL_ATF)
void board_fit_image_post_process(const void *fit, int node, void **p_image,
				  size_t *p_size)
{
	ti_secure_image_check_binary(p_image, p_size);
	ti_secure_image_post_process(p_image, p_size);
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

__weak const char *get_reset_reason(void)
{
	return NULL;
}

int print_cpuinfo(void)
{
	struct udevice *soc;
	char name[64];
	int ret;
	const char *reset_reason;

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

	reset_reason = get_reset_reason();
	if (reset_reason)
		printf("Reset reason: %s\n", reset_reason);

	return 0;
}
#endif

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

void spl_enable_cache(void)
{
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	gd->ram_top = CFG_SYS_SDRAM_BASE;
	int ret = 0;

	dram_init();

	/* reserve TLB table */
	gd->arch.tlb_size = PGTABLE_SIZE;

	gd->ram_top += get_effective_memsize();
	gd->relocaddr = gd->ram_top;

	ret = spl_reserve_video_from_ram_top();
	if (ret)
		panic("Failed to reserve framebuffer memory (%d)\n", ret);

	gd->arch.tlb_addr = gd->relocaddr - gd->arch.tlb_size;
	gd->arch.tlb_addr &= ~(0x10000 - 1);
	debug("TLB table from %08lx to %08lx\n", gd->arch.tlb_addr,
	      gd->arch.tlb_addr + gd->arch.tlb_size);
	gd->relocaddr = gd->arch.tlb_addr;

	enable_caches();
#endif
}

static __maybe_unused void k3_dma_remove(void)
{
	struct udevice *dev;
	int rc;

	rc = uclass_find_device(UCLASS_DMA, 0, &dev);
	if (!rc && dev) {
		rc = device_remove(dev, DM_REMOVE_NORMAL);
		if (rc)
			pr_warn("Cannot remove dma device '%s' (err=%d)\n",
				dev->name, rc);
	} else
		pr_warn("DMA Device not found (err=%d)\n", rc);
}

void spl_board_prepare_for_boot(void)
{
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	dcache_disable();
#endif
#if IS_ENABLED(CONFIG_SPL_DMA) && IS_ENABLED(CONFIG_SPL_DM_DEVICE_REMOVE)
	k3_dma_remove();
#endif
}

#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
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

	if (IS_ENABLED(CONFIG_TI_ICSSG_PRUETH)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(prueth),
						  &dev);
		if (ret)
			printf("Failed to probe prueth driver\n");
	}

	/* Default FIT boot on HS-SE devices */
	if (get_device_type() == K3_DEVICE_TYPE_HS_SE) {
		env_set("boot_fit", "1");
		env_set("secure_rprocs", "1");
	}

	return 0;
}

/**
 * do_board_detect() - Detect board description
 *
 * Function to detect board description. This is expected to be
 * overridden in the SoC family board file where desired.
 */
void __weak do_board_detect(void)
{
}

#if (IS_ENABLED(CONFIG_K3_QOS))
void setup_qos(void)
{
	u32 i;

	for (i = 0; i < qos_count; i++)
		writel(qos_data[i].val, (uintptr_t)qos_data[i].reg);
}
#endif

int __maybe_unused shutdown_mcu_r5_core1(void)
{
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	struct ti_sci_dev_ops *dev_ops = &ti_sci->ops.dev_ops;
	struct ti_sci_proc_ops *proc_ops = &ti_sci->ops.proc_ops;
	u32 dev_id_mcu_r5_core1 = put_core_ids[0];
	u64 boot_vector;
	u32 cfg, ctrl, sts, halted;
	int cluster_mode_lockstep, ret;
	bool r_state = false, c_state = false;

	ret = proc_ops->proc_request(ti_sci, PROC_ID_MCU_R5FSS0_CORE1);
	if (ret) {
		printf("Unable to request processor control for MCU1_1 core, %d\n",
		       ret);
		return ret;
	}

	ret = dev_ops->is_on(ti_sci, dev_id_mcu_r5_core1, &r_state, &c_state);
	if (ret) {
		printf("Unable to get device status for MCU1_1 core, %d\n", ret);
		return ret;
	}

	ret = proc_ops->get_proc_boot_status(ti_sci, PROC_ID_MCU_R5FSS0_CORE1,
					     &boot_vector, &cfg, &ctrl, &sts);
	if (ret) {
		printf("Unable to get Processor boot status for MCU1_1 core, %d\n",
		       ret);
		goto release_proc_ctrl;
	}

	halted = !!(sts & PROC_BOOT_STATUS_FLAG_R5_WFI);
	cluster_mode_lockstep = !!(cfg & PROC_BOOT_CFG_FLAG_R5_LOCKSTEP);

	/*
	 * Shutdown MCU R5F Core 1 only if:
	 *	- cluster is booted in SplitMode
	 *	- core is powered on
	 *	- core is in WFI (halted)
	 */
	if (cluster_mode_lockstep || !c_state || !halted) {
		ret = -EINVAL;
		goto release_proc_ctrl;
	}

	ret = proc_ops->set_proc_boot_ctrl(ti_sci, PROC_ID_MCU_R5FSS0_CORE1,
					   PROC_BOOT_CTRL_FLAG_R5_CORE_HALT, 0);
	if (ret) {
		printf("Unable to Halt MCU1_1 core, %d\n", ret);
		goto release_proc_ctrl;
	}

	ret = dev_ops->put_device(ti_sci, dev_id_mcu_r5_core1);
	if (ret) {
		printf("Unable to assert reset on MCU1_1 core, %d\n", ret);
		return ret;
	}

release_proc_ctrl:
	proc_ops->proc_release(ti_sci, PROC_ID_MCU_R5FSS0_CORE1);
	return ret;
}
