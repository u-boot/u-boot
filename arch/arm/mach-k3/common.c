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
	/* keep ram_top in the 32-bit address space */
	if (gd->ram_top >= 0x100000000)
		gd->ram_top = (phys_addr_t)0x100000000;

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

void efi_add_known_memory(void)
{
	if (IS_ENABLED(CONFIG_EFI_LOADER))
		/*
		 * Memory over ram_top can be used by various firmware
		 * Declare to EFI only memory area below ram_top
		 */
		efi_add_memory_map(gd->ram_base, gd->ram_top - gd->ram_base,
				   EFI_CONVENTIONAL_MEMORY);
}
