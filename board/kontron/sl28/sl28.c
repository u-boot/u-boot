// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <dfu.h>
#include <malloc.h>
#include <memalign.h>
#include <efi.h>
#include <efi_loader.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <env_internal.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#include <fsl_immap.h>
#include <netdev.h>
#include <wdt.h>

#include <sl28cpld.h>
#include <fdtdec.h>
#include <miiphy.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_images fw_images[] = {
	{
		.image_type_id = KONTRON_SL28_FIT_IMAGE_GUID,
		.fw_name = u"KONTRON-SL28-FIT",
		.image_index = 1
	},
};

u8 num_image_type_guids = ARRAY_SIZE(fw_images);
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();
	return 0;
}

int board_init(void)
{
	if (CONFIG_IS_ENABLED(FSL_CAAM))
		sec_init();

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return pci_eth_init(bis);
}

static int __sl28cpld_read(uint reg)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_NOP,
					  DM_DRIVER_GET(sl28cpld), &dev);
	if (ret)
		return ret;

	return sl28cpld_read(dev, reg);
}

static void print_cpld_version(void)
{
	int version = __sl28cpld_read(SL28CPLD_VERSION);

	if (version < 0)
		printf("CPLD:  error reading version (%d)\n", version);
	else
		printf("CPLD:  v%d\n", version);
}

int checkboard(void)
{
	printf("EL:    %d\n", current_el());
	if (CONFIG_IS_ENABLED(SL28CPLD))
		print_cpld_version();

	return 0;
}

static void stop_recovery_watchdog(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_WDT,
					  DM_DRIVER_GET(sl28cpld_wdt), &dev);
	if (!ret)
		wdt_stop(dev);
}

int fsl_board_late_init(void)
{
	/*
	 * Usually, the after a board reset, the watchdog is enabled by
	 * default. This is to supervise the bootloader boot-up. Therefore,
	 * to prevent a watchdog reset if we don't actively kick it, we have
	 * to disable it.
	 *
	 * If the watchdog isn't enabled at reset (which is a configuration
	 * option) disabling it doesn't hurt either.
	 */
	if (!CONFIG_IS_ENABLED(WATCHDOG_AUTOSTART))
		stop_recovery_watchdog();

	return 0;
}

void detail_board_ddr_info(void)
{
	print_ddr_info(0);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int nbanks = CONFIG_NR_DRAM_BANKS;
	int node;
	int i;

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	for (i = 0; i < nbanks; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

	fdt_fixup_memory_banks(blob, base, size, nbanks);

	fdt_fixup_icid(blob);

	if (CONFIG_IS_ENABLED(SL28_SPL_LOADS_OPTEE_BL32)) {
		node = fdt_node_offset_by_compatible(blob, -1, "linaro,optee-tz");
		if (node)
			fdt_set_node_status(blob, node, FDT_STATUS_OKAY);
	}

	return 0;
}

#if defined(CONFIG_SET_DFU_ALT_INFO)

#define DFU_ALT_BUF_LEN		SZ_1K

void set_dfu_alt_info(char *interface, char *devstr)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (!CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT) &&
	    env_get("dfu_alt_info"))
		return;

	memset(buf, 0, sizeof(buf));

	snprintf(buf, DFU_ALT_BUF_LEN,
		 "sf 0:0=u-boot-bin raw 0x210000 0x1d0000;"
		 "u-boot-env raw 0x3e0000 0x20000");

	env_set("dfu_alt_info", buf);
}
#endif /* CONFIG_SET_DFU_ALT_INFO */
