/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/system.h>

#include <fsl-mc/fsl_mc.h>
#include <fsl_debug_server.h>

DECLARE_GLOBAL_DATA_PTR;
static int debug_server_ver_info_maj, debug_server_ver_info_min;

/**
 * Copying Debug Server firmware to DDR
 */
static int debug_server_copy_image(const char *title, u64 image_addr,
				   u32 image_size, u64 debug_server_ram_addr)
{
	debug("%s copied to address %p\n", title,
	      (void *)debug_server_ram_addr);
	memcpy((void *)debug_server_ram_addr, (void *)image_addr, image_size);

	return 0;
}

/**
 * Debug Server FIT image parser checks if the image is in FIT
 * format, verifies integrity of the image and calculates
 * raw image address and size values.
 *
 * Returns 0 if success and -1 if any of the above mentioned
 * task fail.
 **/
int debug_server_parse_firmware_fit_image(const void **raw_image_addr,
					  size_t *raw_image_size)
{
	int format;
	void *fit_hdr;
	int node_offset;
	const void *data;
	size_t size;
	const char *uname = "firmware";
	char *desc;
	char *debug_server_ver_info;
	char *debug_server_ver_info_major, *debug_server_ver_info_minor;

	/* Check if the image is in NOR flash */
#ifdef CONFIG_SYS_DEBUG_SERVER_FW_IN_NOR
	fit_hdr = (void *)CONFIG_SYS_DEBUG_SERVER_FW_ADDR;
#else
#error "CONFIG_SYS_DEBUG_SERVER_FW_IN_NOR not defined"
#endif

	/* Check if Image is in FIT format */
	format = genimg_get_format(fit_hdr);
	if (format != IMAGE_FORMAT_FIT) {
		printf("Debug Server FW: Not a FIT image\n");
		goto out_error;
	}

	if (!fit_check_format(fit_hdr)) {
		printf("Debug Server FW: Bad FIT image format\n");
		goto out_error;
	}

	node_offset = fit_image_get_node(fit_hdr, uname);
	if (node_offset < 0) {
		printf("Debug Server FW:Can not find %s subimage\n", uname);
		goto out_error;
	}

	/* Verify Debug Server firmware image */
	if (!fit_image_verify(fit_hdr, node_offset)) {
		printf("Debug Server FW: Bad Debug Server firmware hash");
		goto out_error;
	}

	if (fit_get_desc(fit_hdr, node_offset, &desc) < 0) {
		printf("Debug Server FW: Failed to get FW description");
		goto out_error;
	}

	debug_server_ver_info = strstr(desc, "Version");
	debug_server_ver_info_major = strtok(debug_server_ver_info, ".");
	debug_server_ver_info_minor = strtok(NULL, ".");

	debug_server_ver_info_maj =
			simple_strtoul(debug_server_ver_info_major, NULL, 10);
	debug_server_ver_info_min =
			simple_strtoul(debug_server_ver_info_minor, NULL, 10);

	/* Debug server version checking */
	if ((debug_server_ver_info_maj < DEBUG_SERVER_VER_MAJOR) ||
	    (debug_server_ver_info_min < DEBUG_SERVER_VER_MINOR)) {
		printf("Debug server FW mismatches the min version required\n");
		printf("Expected:%d.%d, Got %d.%d\n",
		       DEBUG_SERVER_VER_MAJOR, DEBUG_SERVER_VER_MINOR,
		       debug_server_ver_info_maj,
		       debug_server_ver_info_min);
		goto out_error;
	}

	/* Get address and size of raw image */
	fit_image_get_data(fit_hdr, node_offset, &data, &size);

	*raw_image_addr = data;
	*raw_image_size = size;

	return 0;

out_error:
	return -1;
}

/**
 * Return the actual size of the Debug Server private DRAM block.
 *
 * NOTE: For now this function always returns the minimum required size,
 * However, in the future, the actual size may be obtained from an environment
 * variable.
 */
unsigned long debug_server_get_dram_block_size(void)
{
	return CONFIG_SYS_DEBUG_SERVER_DRAM_BLOCK_MIN_SIZE;
}

int debug_server_init(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int error, timeout = CONFIG_SYS_DEBUG_SERVER_TIMEOUT;
	int debug_server_boot_status;
	u64 debug_server_ram_addr, debug_server_ram_size;
	const void *raw_image_addr;
	size_t raw_image_size = 0;

	debug("debug_server_init called\n");
	/*
	 * The Debug Server private DRAM block was already carved at the end of
	 * DRAM by board_init_f() using CONFIG_SYS_MEM_TOP_HIDE:
	 */
	debug_server_ram_size = debug_server_get_dram_block_size();
	if (gd->bd->bi_dram[1].start)
		debug_server_ram_addr =
			gd->bd->bi_dram[1].start + gd->bd->bi_dram[1].size;
	else
		debug_server_ram_addr =
			gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size;

#ifdef CONFIG_FSL_MC_ENET
	debug_server_ram_addr += mc_get_dram_block_size();
#endif

	error = debug_server_parse_firmware_fit_image(&raw_image_addr,
							&raw_image_size);
	if (error != 0)
		goto out;

	debug("debug server (ram addr = 0x%llx, ram size = 0x%llx)\n",
	      debug_server_ram_addr, debug_server_ram_size);
	/*
	 * Load the Debug Server FW at the beginning of the Debug Server
	 * private DRAM block:
	 */
	debug_server_copy_image("Debug Server Firmware",
				(u64)raw_image_addr, raw_image_size,
				debug_server_ram_addr);

	/* flush dcache */
	flush_dcache_range((unsigned long)debug_server_ram_addr,
			   (unsigned long)debug_server_ram_addr +
			   (unsigned long)debug_server_ram_size);

	/*
	 * Tell SP that the Debug Server FW is about to be launched. Before that
	 * populate the following:
	 * 1. Write the size allocated to SP Memory region into Bits {31:16} of
	 *    SCRATCHRW5.
	 * 2. Write the start address of the SP memory regions into
	 *    SCRATCHRW5 (Bits {15:0}, contain most significant bits, Bits
	 *    {47:32} of the SP Memory Region physical start address
	 *    (SoC address)) and SCRATCHRW6 (Bits {31:0}).
	 * 3. To know the Debug Server FW boot status, set bit 0 of SCRATCHRW11
	 *    to 1. The Debug Server sets this to 0 to indicate a
	 *    successul boot.
	 * 4. Wakeup SP by writing 0x1F to VSG GIC reg VIGR2.
	 */

	/* 512 MB */
	out_le32(&gur->scratchrw[5 - 1],
		 (u32)((u64)debug_server_ram_addr >> 32) | (0x000D << 16));
	out_le32(&gur->scratchrw[6 - 1],
		 ((u32)debug_server_ram_addr) & 0xFFFFFFFF);

	out_le32(&gur->scratchrw[11 - 1], DEBUG_SERVER_INIT_STATUS);
	/* Allow the changes to reflect in GUR block */
	mb();

	/*
	 * Program VGIC to raise an interrupt to SP
	 */
	out_le32(CONFIG_SYS_FSL_SP_VSG_GIC_VIGR2, 0x1F);
	/* Allow the changes to reflect in VIGR2 */
	mb();

	dmb();
	debug("Polling for Debug server to launch ...\n");

	while (1) {
		debug_server_boot_status = in_le32(&gur->scratchrw[11 - 1]);
		if (!(debug_server_boot_status & DEBUG_SERVER_INIT_STATUS_MASK))
			break;

		udelay(1);	/* throttle polling */
		if (timeout-- <= 0)
			break;
	}

	if (timeout <= 0) {
		printf("Debug Server FW timed out (boot status: 0x%x)\n",
		       debug_server_boot_status);
		error = -ETIMEDOUT;
		goto out;
	}

	if (debug_server_boot_status & DEBUG_SERVER_INIT_STATUS_MASK) {
		printf("Debug server FW error'ed out (boot status: 0x%x)\n",
		       debug_server_boot_status);
		error = -ENODEV;
		goto out;
	}

	printf("Debug server booted\n");
	printf("Detected firmware %d.%d, (boot status: 0x0%x)\n",
	       debug_server_ver_info_maj, debug_server_ver_info_min,
	       debug_server_boot_status);

out:
	if (error != 0)
		debug_server_boot_status = -error;
	else
		debug_server_boot_status = 0;

	return debug_server_boot_status;
}

