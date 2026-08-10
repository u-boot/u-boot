// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022 - 2026, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <cpu_func.h>
#include <dfu.h>
#include <env.h>
#include <efi_loader.h>
#include <fdtdec.h>
#include <fwu.h>
#include <init.h>
#include <env_internal.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <mtd.h>
#include <time.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/sections.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <versalpl.h>
#include "../../xilinx/common/board.h"

#include <debug_uart.h>
#include <generated/dt.h>
#include <linux/ioport.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA_VERSALPL)
static xilinx_desc versalpl = {
	xilinx_versal2, csu_dma, 1, &versal_op, 0, &versal_op, NULL,
	FPGA_LEGACY
};
#endif

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

#if defined(CONFIG_FPGA_VERSALPL)
	fpga_init();
	fpga_add(fpga_xilinx, &versalpl);
#endif

	if (CONFIG_IS_ENABLED(DM_I2C) && CONFIG_IS_ENABLED(I2C_EEPROM))
		xilinx_read_eeprom();

	return 0;
}

int board_early_init_r(void)
{
	if (current_el() != 3)
		return 0;

	versal2_timer_setup();

	return 0;
}

static u32 versal2_multi_boot(void)
{
	u8 bootmode = versal2_get_bootmode();

	/* Mostly workaround for QEMU CI pipeline */
	if (bootmode == JTAG_MODE)
		return 0;

	return versal2_pmc_multi_boot();
}

static int boot_targets_setup(void)
{
	u8 bootmode;
	struct udevice *dev;
	int bootseq = -1;
	int bootseq_len = 0;
	int env_targets_len = 0;
	const char *mode = NULL;
	char *new_targets;
	char *env_targets;

	bootmode = versal2_get_bootmode();

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "usb_dfu0 usb_dfu1";
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "jtag pxe dhcp";
		break;
	case QSPI_MODE_24BIT:
		puts("QSPI_MODE_24\n");
		if (uclass_get_device_by_name(UCLASS_SPI,
					      "spi@f1030000", &dev)) {
			debug("QSPI driver for QSPI device is not present\n");
			break;
		}
		mode = "xspi";
		bootseq = dev_seq(dev);
		break;
	case QSPI_MODE_32BIT:
		puts("QSPI_MODE_32\n");
		if (uclass_get_device_by_name(UCLASS_SPI,
					      "spi@f1030000", &dev)) {
			debug("QSPI driver for QSPI device is not present\n");
			break;
		}
		mode = "xspi";
		bootseq = dev_seq(dev);
		break;
	case OSPI_MODE:
		puts("OSPI_MODE\n");
		if (uclass_get_device_by_name(UCLASS_SPI,
					      "spi@f1010000", &dev)) {
			debug("OSPI driver for OSPI device is not present\n");
			break;
		}
		mode = "xspi";
		bootseq = dev_seq(dev);
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@f1050000", &dev)) {
			debug("SD1 driver for SD1 device is not present\n");
			break;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev_seq(dev));
		mode = "mmc";
		bootseq = dev_seq(dev);
		break;
	case SELECTMAP_MODE:
		puts("SELECTMAP_MODE\n");
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@f1040000", &dev)) {
			debug("SD0 driver for SD0 device is not present\n");
			break;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		fallthrough;
	case SD_MODE1:
		puts("SD_MODE1\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@f1050000", &dev)) {
			debug("SD1 driver for SD1 device is not present\n");
			break;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		break;
	case UFS_MODE:
		puts("UFS_MODE\n");
		if (uclass_get_device(UCLASS_UFS, 0, &dev)) {
			debug("UFS driver for UFS device is not present\n");
			break;
		}
		debug("ufs device found at %p\n", dev);

		mode = "ufs";
		break;
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	if (mode) {
		if (bootseq >= 0) {
			bootseq_len = snprintf(NULL, 0, "%i", bootseq);
			debug("Bootseq len: %x\n", bootseq_len);
		}

		/*
		 * One terminating char + one byte for space between mode
		 * and default boot_targets
		 */
		env_targets = env_get("boot_targets");
		if (env_targets)
			env_targets_len = strlen(env_targets);

		new_targets = calloc(1, strlen(mode) + env_targets_len + 2 +
				     bootseq_len);
		if (!new_targets)
			return -ENOMEM;

		if (bootseq >= 0)
			sprintf(new_targets, "%s%x %s", mode, bootseq,
				env_targets ? env_targets : "");
		else
			sprintf(new_targets, "%s %s", mode,
				env_targets ? env_targets : "");

		env_set("boot_targets", new_targets);
		free(new_targets);
	}

	return 0;
}

int board_late_init(void)
{
	int ret;
	u32 multiboot;

	if (IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT) &&
	    !IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE))
		configure_capsule_updates();

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	if (!IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG))
		return 0;

	multiboot = versal2_multi_boot();
	env_set_hex("multiboot", multiboot);

	if (IS_ENABLED(CONFIG_DISTRO_DEFAULTS)) {
		ret = boot_targets_setup();
		if (ret)
			return ret;
	}

	return board_late_init_xilinx();
}

int dram_init_banksize(void)
{
	fill_bd_mem_info();
	return 0;
}

int dram_init(void)
{
	struct mm_region bank_info[CONFIG_NR_DRAM_BANKS];
	ofnode mem = ofnode_null();
	struct resource res;
	int ret, i, reg = 0;
	u32 num_banks, reloc_use = 0;
	u64 text = (u64)_start;

	gd->ram_base = (unsigned long)~0;

	mem = fdtdec_get_next_memory_node(mem);
	if (!ofnode_valid(mem)) {
		printf("%s: Missing /memory node\n", __func__);
		return -EINVAL;
	}

	debug("%s: Text base = 0x%llx\n", __func__, text);

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		reloc_use = 0;
		ret = ofnode_read_resource(mem, reg++, &res);
		if (ret < 0) {
			reg = 0;
			mem = fdtdec_get_next_memory_node(mem);
			if (!ofnode_valid(mem))
				break;

			ret = ofnode_read_resource(mem, reg++, &res);
			if (ret < 0)
				break;
		}

		if (ret != 0)
			return -EINVAL;

		bank_info[i].phys = (phys_addr_t)res.start;
		bank_info[i].size = (phys_size_t)(res.end - res.start + 1);

		if (bank_info[i].size == 0)
			break;

		if (text >= bank_info[i].phys &&
		    text < (bank_info[i].phys + bank_info[i].size)) {
			gd->ram_base = bank_info[i].phys;
			gd->ram_size = bank_info[i].size;
			reloc_use = 1;
		}

		debug("%s: DRAM Bank #%d: start = 0x%llx, size = 0x%llx %s",
		      __func__, i, (unsigned long long)bank_info[i].phys,
		      (unsigned long long)bank_info[i].size,
		      (reloc_use ? " - USED for RELOCATION\n" : "\n"));

		num_banks++;
	}

	mem_map_fill(bank_info, num_banks);

	debug("%s: Initial DRAM: start = 0x%lx, size = 0x%lx\n", __func__,
	      gd->ram_base, (unsigned long)gd->ram_size);

	return 0;
}

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
}
#endif

#if defined(CONFIG_ENV_IS_NOWHERE)
enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = versal2_get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode) {
	case EMMC_MODE:
	case SD_MODE:
	case SD1_LSHFT_MODE:
	case SD_MODE1:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_FAT))
			return ENVL_FAT;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		return ENVL_NOWHERE;
	case OSPI_MODE:
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		return ENVL_NOWHERE;
	case JTAG_MODE:
	case SELECTMAP_MODE:
	default:
		return ENVL_NOWHERE;
	}
}
#endif

#define DFU_ALT_BUF_LEN		SZ_1K

#if defined(CONFIG_EFI_HAVE_CAPSULE_SUPPORT) && \
	!defined(CONFIG_FWU_MULTI_BANK_UPDATE)
static void mtd_found_part(u32 *base, u32 *size)
{
	struct mtd_info *part, *mtd;

	mtd_probe_devices();

	mtd = get_mtd_device_nm("nor0");
	if (!IS_ERR_OR_NULL(mtd)) {
		list_for_each_entry(part, &mtd->partitions, node) {
			debug("0x%012llx-0x%012llx : \"%s\"\n",
			      part->offset, part->offset + part->size,
			      part->name);

			if (*base >= part->offset &&
			    *base < part->offset + part->size) {
				debug("Found my partition: %d/%s\n",
				      part->index, part->name);
				*base = part->offset;
				*size = part->size;
				break;
			}
		}
	}
}

void configure_capsule_updates(void)
{
	int bootseq = 0, len = 0;
	u32 multiboot = versal2_multi_boot();
	u32 bootmode = versal2_get_bootmode();

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	memset(buf, 0, DFU_ALT_BUF_LEN);

	multiboot = env_get_hex("multiboot", multiboot);

	switch (bootmode) {
	case EMMC_MODE:
	case SD_MODE:
	case SD1_LSHFT_MODE:
	case SD_MODE1:
		bootseq = mmc_get_env_dev();

		len += snprintf(buf + len, DFU_ALT_BUF_LEN, "mmc %d=boot",
			       bootseq);

		if (multiboot)
			len += snprintf(buf + len, DFU_ALT_BUF_LEN,
					"%04d", multiboot);

		len += snprintf(buf + len, DFU_ALT_BUF_LEN, ".bin fat %d 1",
			       bootseq);
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
	case OSPI_MODE:
		{
			u32 base = multiboot * SZ_32K;
			u32 size = 0x1500000;
			u32 limit = size;

			mtd_found_part(&base, &limit);

			len += snprintf(buf + len, DFU_ALT_BUF_LEN,
					"sf 0:0=boot.bin raw 0x%x 0x%x",
					base, limit);
		}
		break;
	default:
		return;
	}

	update_info.dfu_string = strdup(buf);
	debug("Capsule DFU: %s\n", update_info.dfu_string);
}
#endif

#if defined(CONFIG_FWU_MULTI_BANK_UPDATE)

/* Generate dfu_alt_info from partitions */
void set_dfu_alt_info(char *interface, char *devstr)
{
	int ret;
	struct mtd_info *mtd;

	/*
	 * It is called multiple times for every image
	 * per bank that's why enough to set it up once.
	 */
	if (env_get("dfu_alt_info"))
		return;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);
	memset(buf, 0, DFU_ALT_BUF_LEN);

	mtd_probe_devices();

	mtd = get_mtd_device_nm("nor0");
	if (IS_ERR_OR_NULL(mtd))
		return;

	ret = fwu_gen_alt_info_from_mtd(buf, DFU_ALT_BUF_LEN, mtd);
	if (ret < 0) {
		log_err("Error: Failed to generate dfu_alt_info. (%d)\n", ret);
		return;
	}
	log_debug("Make dfu_alt_info: '%s'\n", buf);

	env_set("dfu_alt_info", buf);
}
#endif

int spi_get_env_dev(void)
{
	struct udevice *dev;
	const char *name;
	int bootseq;

	switch (versal2_get_bootmode()) {
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		name = "spi@f1030000";
		break;
	case OSPI_MODE:
		name = "spi@f1010000";
		break;
	default:
		return -1;
	}

	if (uclass_get_device_by_name(UCLASS_SPI, name, &dev)) {
		debug("SPI driver for %s is not present\n", name);
		return -1;
	}

	bootseq = dev_seq(dev);
	debug("bootseq %d\n", bootseq);
	return bootseq;
}
