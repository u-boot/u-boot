/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <errno.h>
#include <asm/io.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_dpmng.h>

DECLARE_GLOBAL_DATA_PTR;
static int mc_boot_status;

/**
 * Copying MC firmware or DPL image to DDR
 */
static int mc_copy_image(const char *title,
			 u64 image_addr, u32 image_size, u64 mc_ram_addr)
{
	debug("%s copied to address %p\n", title, (void *)mc_ram_addr);
	memcpy((void *)mc_ram_addr, (void *)image_addr, image_size);
	return 0;
}

/**
 * MC firmware FIT image parser checks if the image is in FIT
 * format, verifies integrity of the image and calculates
 * raw image address and size values.
 * Returns 0 on success and a negative errno on error.
 * task fail.
 **/
int parse_mc_firmware_fit_image(const void **raw_image_addr,
				size_t *raw_image_size)
{
	int format;
	void *fit_hdr;
	int node_offset;
	const void *data;
	size_t size;
	const char *uname = "firmware";

	/* Check if the image is in NOR flash */
#ifdef CONFIG_SYS_LS_MC_FW_IN_NOR
	fit_hdr = (void *)CONFIG_SYS_LS_MC_FW_ADDR;
#else
#error "No CONFIG_SYS_LS_MC_FW_IN_xxx defined"
#endif

	/* Check if Image is in FIT format */
	format = genimg_get_format(fit_hdr);

	if (format != IMAGE_FORMAT_FIT) {
		printf("fsl-mc: ERROR: Bad firmware image (not a FIT image)\n");
		return -EINVAL;
	}

	if (!fit_check_format(fit_hdr)) {
		printf("fsl-mc: ERROR: Bad firmware image (bad FIT header)\n");
		return -EINVAL;
	}

	node_offset = fit_image_get_node(fit_hdr, uname);

	if (node_offset < 0) {
		printf("fsl-mc: ERROR: Bad firmware image (missing subimage)\n");
		return -ENOENT;
	}

	/* Verify MC firmware image */
	if (!(fit_image_verify(fit_hdr, node_offset))) {
		printf("fsl-mc: ERROR: Bad firmware image (bad CRC)\n");
		return -EINVAL;
	}

	/* Get address and size of raw image */
	fit_image_get_data(fit_hdr, node_offset, &data, &size);

	*raw_image_addr = data;
	*raw_image_size = size;

	return 0;
}

int mc_init(bd_t *bis)
{
	int error = 0;
	int timeout = 200000;
	struct mc_ccsr_registers __iomem *mc_ccsr_regs = MC_CCSR_BASE_ADDR;
	u64 mc_ram_addr;
	u64 mc_dpl_offset;
	u32 reg_gsr;
	u32 mc_fw_boot_status;
	void *dpl_fdt_hdr;
	int dpl_size;
	const void *raw_image_addr;
	size_t raw_image_size = 0;
	struct fsl_mc_io mc_io;
	int portal_id;
	struct mc_version mc_ver_info;

	/*
	 * The MC private DRAM block was already carved at the end of DRAM
	 * by board_init_f() using CONFIG_SYS_MEM_TOP_HIDE:
	 */
	if (gd->bd->bi_dram[1].start) {
		mc_ram_addr =
			gd->bd->bi_dram[1].start + gd->bd->bi_dram[1].size;
	} else {
		mc_ram_addr =
			gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size;
	}

	/*
	 * Management Complex cores should be held at reset out of POR.
	 * U-boot should be the first software to touch MC. To be safe,
	 * we reset all cores again by setting GCR1 to 0. It doesn't do
	 * anything if they are held at reset. After we setup the firmware
	 * we kick off MC by deasserting the reset bit for core 0, and
	 * deasserting the reset bits for Command Portal Managers.
	 * The stop bits are not touched here. They are used to stop the
	 * cores when they are active. Setting stop bits doesn't stop the
	 * cores from fetching instructions when they are released from
	 * reset.
	 */
	out_le32(&mc_ccsr_regs->reg_gcr1, 0);
	dmb();

	error = parse_mc_firmware_fit_image(&raw_image_addr, &raw_image_size);
	if (error != 0)
		goto out;
	/*
	 * Load the MC FW at the beginning of the MC private DRAM block:
	 */
	mc_copy_image("MC Firmware",
		      (u64)raw_image_addr, raw_image_size, mc_ram_addr);

	/*
	 * Get address and size of the DPL blob stored in flash:
	 */
#ifdef CONFIG_SYS_LS_MC_DPL_IN_NOR
	dpl_fdt_hdr = (void *)CONFIG_SYS_LS_MC_DPL_ADDR;
#else
#error "No CONFIG_SYS_LS_MC_DPL_IN_xxx defined"
#endif

	error = fdt_check_header(dpl_fdt_hdr);
	if (error != 0) {
		printf("fsl-mc: ERROR: Bad DPL image (bad header)\n");
		goto out;
	}

	dpl_size = fdt_totalsize(dpl_fdt_hdr);
	if (dpl_size > CONFIG_SYS_LS_MC_DPL_MAX_LENGTH) {
		printf("fsl-mc: ERROR: Bad DPL image (too large: %d)\n",
		       dpl_size);
		error = -EINVAL;
		goto out;
	}

	/*
	 * Calculate offset in the MC private DRAM block at which the MC DPL
	 * blob is to be placed:
	 */
#ifdef CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET
	BUILD_BUG_ON((CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET & 0x3) != 0 ||
		     CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET > 0xffffffff);

	mc_dpl_offset = CONFIG_SYS_LS_MC_DRAM_DPL_OFFSET;
#else
	mc_dpl_offset = mc_get_dram_block_size() -
			roundup(CONFIG_SYS_LS_MC_DPL_MAX_LENGTH, 4096);

	if ((mc_dpl_offset & 0x3) != 0 || mc_dpl_offset > 0xffffffff) {
		printf("%s: Invalid MC DPL offset: %llu\n",
		       __func__, mc_dpl_offset);
		error = -EINVAL;
		goto out;
	}
#endif

	/*
	 * Load the MC DPL blob at the far end of the MC private DRAM block:
	 *
	 * TODO: Should we place the DPL at a different location to match
	 * assumptions of MC firmware about its memory layout?
	 */
	mc_copy_image("MC DPL blob",
		      (u64)dpl_fdt_hdr, dpl_size, mc_ram_addr + mc_dpl_offset);

	debug("mc_ccsr_regs %p\n", mc_ccsr_regs);

	/*
	 * Tell MC where the MC Firmware image was loaded in DDR:
	 */
	out_le32(&mc_ccsr_regs->reg_mcfbalr, (u32)mc_ram_addr);
	out_le32(&mc_ccsr_regs->reg_mcfbahr, (u32)((u64)mc_ram_addr >> 32));
	out_le32(&mc_ccsr_regs->reg_mcfapr, MCFAPR_BYPASS_ICID_MASK);

	/*
	 * Tell MC where the DPL blob was loaded in DDR, by indicating
	 * its offset relative to the beginning of the DDR block
	 * allocated to the MC firmware. The MC firmware is responsible
	 * for checking that there is no overlap between the DPL blob
	 * and the runtime heap and stack of the MC firmware itself.
	 *
	 * NOTE: bits [31:2] of this offset need to be stored in bits [29:0] of
	 * the GSR MC CCSR register. So, this offset is assumed to be 4-byte
	 * aligned.
	 * Care must be taken not to write 1s into bits 31 and 30 of the GSR in
	 * this case as the SoC COP or PIC will be signaled.
	 */
	out_le32(&mc_ccsr_regs->reg_gsr, (u32)(mc_dpl_offset >> 2));

	printf("\nfsl-mc: Booting Management Complex ...\n");

	/*
	 * Deassert reset and release MC core 0 to run
	 */
	out_le32(&mc_ccsr_regs->reg_gcr1, GCR1_P1_DE_RST | GCR1_M_ALL_DE_RST);
	dmb();
	debug("Polling mc_ccsr_regs->reg_gsr ...\n");

	for (;;) {
		reg_gsr = in_le32(&mc_ccsr_regs->reg_gsr);
		mc_fw_boot_status = (reg_gsr & GSR_FS_MASK);
		if (mc_fw_boot_status & 0x1)
			break;

		udelay(1000);	/* throttle polling */
		if (timeout-- <= 0)
			break;
	}

	if (timeout <= 0) {
		printf("fsl-mc: timeout booting management complex firmware\n");

		/* TODO: Get an error status from an MC CCSR register */
		error = -ETIMEDOUT;
		goto out;
	}

	if (mc_fw_boot_status != 0x1) {
		/*
		 * TODO: Identify critical errors from the GSR register's FS
		 * field and for those errors, set error to -ENODEV or other
		 * appropriate errno, so that the status property is set to
		 * failure in the fsl,dprc device tree node.
		 */
		printf("fsl-mc: WARNING: Firmware booted with error (GSR: %#x)\n",
		       reg_gsr);
	}

	/*
	 * TODO: need to obtain the portal_id for the root container from the
	 * DPL
	 */
	portal_id = 0;

	/*
	 * Check that the MC firmware is responding portal commands:
	 */
	mc_io.mmio_regs = SOC_MC_PORTAL_ADDR(portal_id);
	debug("Checking access to MC portal of root DPRC container (portal_id %d, portal physical addr %p)\n",
	      portal_id, mc_io.mmio_regs);

	error = mc_get_version(&mc_io, &mc_ver_info);
	if (error != 0) {
		printf("fsl-mc: ERROR: Firmware version check failed (error: %d)\n",
		       error);
		goto out;
	}

	if (MC_VER_MAJOR != mc_ver_info.major)
		printf("fsl-mc: ERROR: Firmware major version mismatch (found: %d, expected: %d)\n",
		       mc_ver_info.major, MC_VER_MAJOR);

	if (MC_VER_MINOR != mc_ver_info.minor)
		printf("fsl-mc: WARNING: Firmware minor version mismatch (found: %d, expected: %d)\n",
		       mc_ver_info.minor, MC_VER_MINOR);

	printf("fsl-mc: Management Complex booted (version: %d.%d.%d, boot status: %#x)\n",
	       mc_ver_info.major, mc_ver_info.minor, mc_ver_info.revision,
	       mc_fw_boot_status);
out:
	if (error != 0)
		mc_boot_status = -error;
	else
		mc_boot_status = 0;

	return error;
}

int get_mc_boot_status(void)
{
	return mc_boot_status;
}

/**
 * Return the actual size of the MC private DRAM block.
 *
 * NOTE: For now this function always returns the minimum required size,
 * However, in the future, the actual size may be obtained from an environment
 * variable.
 */
unsigned long mc_get_dram_block_size(void)
{
	return CONFIG_SYS_LS_MC_DRAM_BLOCK_MIN_SIZE;
}
