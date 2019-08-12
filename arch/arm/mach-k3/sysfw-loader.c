// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: System Firmware Loader
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 */

#include <common.h>
#include <spl.h>
#include <malloc.h>
#include <remoteproc.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <asm/arch/sys_proto.h>

/* Name of the FIT image nodes for SYSFW and its config data */
#define SYSFW_FIRMWARE			"sysfw.bin"
#define SYSFW_CFG_BOARD			"board-cfg.bin"
#define SYSFW_CFG_PM			"pm-cfg.bin"
#define SYSFW_CFG_RM			"rm-cfg.bin"
#define SYSFW_CFG_SEC			"sec-cfg.bin"

static bool sysfw_loaded;
static void *sysfw_load_address;

/*
 * Populate SPL hook to override the default load address used by the SPL
 * loader function with a custom address for SYSFW loading.
 */
struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	if (sysfw_loaded)
		return (struct image_header *)(CONFIG_SYS_TEXT_BASE + offset);
	else if (sysfw_load_address)
		return sysfw_load_address;
	else
		panic("SYSFW load address not defined!");
}

/*
 * Populate SPL hook to skip the default SPL loader FIT post-processing steps
 * during SYSFW loading and return to the calling function so we can perform
 * our own custom processing.
 */
bool spl_load_simple_fit_skip_processing(void)
{
	return !sysfw_loaded;
}

static int fit_get_data_by_name(const void *fit, int images, const char *name,
				const void **addr, size_t *size)
{
	int node_offset;

	node_offset = fdt_subnode_offset(fit, images, name);
	if (node_offset < 0)
		return -ENOENT;

	return fit_image_get_data(fit, node_offset, addr, size);
}

static void k3_sysfw_load_using_fit(void *fit)
{
	int images;
	const void *sysfw_addr;
	size_t sysfw_size;
	int ret;

	/* Find the node holding the images information */
	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0)
		panic("Cannot find /images node (%d)\n", images);

	/* Extract System Firmware (SYSFW) image from FIT */
	ret = fit_get_data_by_name(fit, images, SYSFW_FIRMWARE,
				   &sysfw_addr, &sysfw_size);
	if (ret < 0)
		panic("Error accessing %s node in FIT (%d)\n", SYSFW_FIRMWARE,
		      ret);

	/*
	 * Start up system controller firmware
	 *
	 * It is assumed that remoteproc device 0 is the corresponding
	 * system-controller that runs SYSFW. Make sure DT reflects the same.
	 */
	ret = rproc_dev_init(0);
	if (ret)
		panic("rproc failed to be initialized (%d)\n", ret);

	ret = rproc_load(0, (ulong)sysfw_addr, (ulong)sysfw_size);
	if (ret)
		panic("Firmware failed to start on rproc (%d)\n", ret);

	ret = rproc_start(0);
	if (ret)
		panic("Firmware init failed on rproc (%d)\n", ret);
}

static void k3_sysfw_configure_using_fit(void *fit,
					 struct ti_sci_handle *ti_sci)
{
	struct ti_sci_board_ops *board_ops = &ti_sci->ops.board_ops;
	int images;
	const void *cfg_fragment_addr;
	size_t cfg_fragment_size;
	int ret;

	/* Find the node holding the images information */
	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0)
		panic("Cannot find /images node (%d)\n", images);

	/* Extract board configuration from FIT */
	ret = fit_get_data_by_name(fit, images, SYSFW_CFG_BOARD,
				   &cfg_fragment_addr, &cfg_fragment_size);
	if (ret < 0)
		panic("Error accessing %s node in FIT (%d)\n", SYSFW_CFG_BOARD,
		      ret);

	/* Apply board configuration to SYSFW */
	ret = board_ops->board_config(ti_sci,
				      (u64)(u32)cfg_fragment_addr,
				      (u32)cfg_fragment_size);
	if (ret)
		panic("Failed to set board configuration (%d)\n", ret);

	/* Extract power/clock (PM) specific configuration from FIT */
	ret = fit_get_data_by_name(fit, images, SYSFW_CFG_PM,
				   &cfg_fragment_addr, &cfg_fragment_size);
	if (ret < 0)
		panic("Error accessing %s node in FIT (%d)\n", SYSFW_CFG_PM,
		      ret);

	/* Apply power/clock (PM) specific configuration to SYSFW */
	ret = board_ops->board_config_pm(ti_sci,
					 (u64)(u32)cfg_fragment_addr,
					 (u32)cfg_fragment_size);
	if (ret)
		panic("Failed to set board PM configuration (%d)\n", ret);

	/* Extract resource management (RM) specific configuration from FIT */
	ret = fit_get_data_by_name(fit, images, SYSFW_CFG_RM,
				   &cfg_fragment_addr, &cfg_fragment_size);
	if (ret < 0)
		panic("Error accessing %s node in FIT (%d)\n", SYSFW_CFG_RM,
		      ret);

	/* Apply resource management (RM) configuration to SYSFW */
	ret = board_ops->board_config_rm(ti_sci,
					 (u64)(u32)cfg_fragment_addr,
					 (u32)cfg_fragment_size);
	if (ret)
		panic("Failed to set board RM configuration (%d)\n", ret);

	/* Extract security specific configuration from FIT */
	ret = fit_get_data_by_name(fit, images, SYSFW_CFG_SEC,
				   &cfg_fragment_addr, &cfg_fragment_size);
	if (ret < 0)
		panic("Error accessing %s node in FIT (%d)\n", SYSFW_CFG_SEC,
		      ret);

	/* Apply security configuration to SYSFW */
	ret = board_ops->board_config_security(ti_sci,
					       (u64)(u32)cfg_fragment_addr,
					       (u32)cfg_fragment_size);
	if (ret)
		panic("Failed to set board security configuration (%d)\n",
		      ret);
}

void k3_sysfw_loader(void (*config_pm_done_callback)(void))
{
	struct spl_image_info spl_image = { 0 };
	struct spl_boot_device bootdev = { 0 };
	struct ti_sci_handle *ti_sci;
	int ret;

	/* Reserve a block of aligned memory for loading the SYSFW image */
	sysfw_load_address = memalign(ARCH_DMA_MINALIGN,
				      CONFIG_K3_SYSFW_IMAGE_SIZE_MAX);
	if (!sysfw_load_address)
		panic("Error allocating %u bytes of memory for SYSFW image\n",
		      CONFIG_K3_SYSFW_IMAGE_SIZE_MAX);

	debug("%s: allocated %u bytes at 0x%p\n", __func__,
	      CONFIG_K3_SYSFW_IMAGE_SIZE_MAX, sysfw_load_address);

	/* Set load address for legacy modes that bypass spl_get_load_buffer */
	spl_image.load_addr = (uintptr_t)sysfw_load_address;

	bootdev.boot_device = spl_boot_device();

	/* Load combined System Controller firmware and config data image */
	switch (bootdev.boot_device) {
#if CONFIG_IS_ENABLED(MMC_SUPPORT)
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		ret = spl_mmc_load(&spl_image, &bootdev,
#ifdef CONFIG_K3_SYSFW_IMAGE_NAME
				   CONFIG_K3_SYSFW_IMAGE_NAME,
#else
				   NULL,
#endif
#ifdef CONFIG_K3_SYSFW_IMAGE_MMCSD_RAW_MODE_PART
				   CONFIG_K3_SYSFW_IMAGE_MMCSD_RAW_MODE_PART,
#else
				   0,
#endif
#ifdef CONFIG_K3_SYSFW_IMAGE_MMCSD_RAW_MODE_SECT
				   CONFIG_K3_SYSFW_IMAGE_MMCSD_RAW_MODE_SECT);
#else
				   0);
#endif
		break;
#endif
	default:
		panic("Loading SYSFW image from device %u not supported!\n",
		      bootdev.boot_device);
	}

	if (ret)
		panic("Error %d occurred during loading SYSFW image!\n", ret);

	/*
	 * Now that SYSFW got loaded set helper flag to restore regular SPL
	 * loader behavior so we can later boot into the next stage as expected.
	 */
	sysfw_loaded = true;

	/* Ensure the SYSFW image is in FIT format */
	if (image_get_magic((const image_header_t *)sysfw_load_address) !=
	    FDT_MAGIC)
		panic("SYSFW image not in FIT format!\n");

	/* Extract and start SYSFW */
	k3_sysfw_load_using_fit(sysfw_load_address);

	/* Get handle for accessing SYSFW services */
	ti_sci = get_ti_sci_handle();

	/* Parse and apply the different SYSFW configuration fragments */
	k3_sysfw_configure_using_fit(sysfw_load_address, ti_sci);

	/*
	 * Now that all clocks and PM aspects are setup, invoke a user-
	 * provided callback function. Usually this callback would be used
	 * to setup or re-configure the U-Boot console UART.
	 */
	if (config_pm_done_callback)
		config_pm_done_callback();

	/*
	 * Output System Firmware version info. Note that since the
	 * 'firmware_description' field is not guaranteed to be zero-
	 * terminated we manually add a \0 terminator if needed. Further
	 * note that we intentionally no longer rely on the extended
	 * printf() formatter '%.*s' to not having to require a more
	 * full-featured printf() implementation.
	 */
	char fw_desc[sizeof(ti_sci->version.firmware_description) + 1];

	strncpy(fw_desc, ti_sci->version.firmware_description,
		sizeof(ti_sci->version.firmware_description));
	fw_desc[sizeof(fw_desc) - 1] = '\0';

	printf("SYSFW ABI: %d.%d (firmware rev 0x%04x '%s')\n",
	       ti_sci->version.abi_major, ti_sci->version.abi_minor,
	       ti_sci->version.firmware_revision, fw_desc);
}
