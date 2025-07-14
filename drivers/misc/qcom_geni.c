// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2025, Linaro Ltd.
 */

#define pr_fmt(fmt) "GENI-SE: " fmt

#include <blk.h>
#include <part.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <elf.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <misc.h>
#include <linux/printk.h>
#include <soc/qcom/geni-se.h>
#include <soc/qcom/qup-fw-load.h>
#include <dm/device_compat.h>

struct qup_se_rsc {
	phys_addr_t base;
	phys_addr_t wrapper_base;
	struct udevice *dev;

	enum geni_se_xfer_mode mode;
	enum geni_se_protocol_type protocol;
};

struct geni_se_plat {
	bool need_firmware_load;
};

/**
 * geni_enable_interrupts() Enable interrupts.
 * @rsc: Pointer to a structure representing SE-related resources.
 *
 * Enable the required interrupts during the firmware load process.
 *
 * Return: None.
 */
static void geni_enable_interrupts(struct qup_se_rsc *rsc)
{
	u32 reg_value;

	/* Enable required interrupts. */
	writel_relaxed(M_COMMON_GENI_M_IRQ_EN, rsc->base + GENI_M_IRQ_ENABLE);

	reg_value = S_CMD_OVERRUN_EN | S_ILLEGAL_CMD_EN |
				S_CMD_CANCEL_EN | S_CMD_ABORT_EN |
				S_GP_IRQ_0_EN | S_GP_IRQ_1_EN |
				S_GP_IRQ_2_EN | S_GP_IRQ_3_EN |
				S_RX_FIFO_WR_ERR_EN | S_RX_FIFO_RD_ERR_EN;
	writel_relaxed(reg_value, rsc->base + GENI_S_IRQ_ENABLE);

	/* DMA mode configuration. */
	reg_value = DMA_TX_IRQ_EN_SET_RESET_DONE_EN_SET_BMSK |
		    DMA_TX_IRQ_EN_SET_SBE_EN_SET_BMSK |
		    DMA_TX_IRQ_EN_SET_DMA_DONE_EN_SET_BMSK;
	writel_relaxed(reg_value, rsc->base + DMA_TX_IRQ_EN_SET);
	reg_value = DMA_RX_IRQ_EN_SET_FLUSH_DONE_EN_SET_BMSK |
		    DMA_RX_IRQ_EN_SET_RESET_DONE_EN_SET_BMSK |
		    DMA_RX_IRQ_EN_SET_SBE_EN_SET_BMSK |
		    DMA_RX_IRQ_EN_SET_DMA_DONE_EN_SET_BMSK;
	writel_relaxed(reg_value, rsc->base + DMA_RX_IRQ_EN_SET);
}

/**
 * geni_flash_fw_revision() - Flash the firmware revision.
 * @rsc: Pointer to a structure representing SE-related resources.
 * @hdr: Pointer to the ELF header of the Serial Engine.
 *
 * Flash the firmware revision and protocol into the respective register.
 *
 * Return: None.
 */
static void geni_flash_fw_revision(struct qup_se_rsc *rsc, struct elf_se_hdr *hdr)
{
	u32 reg_value;

	/* Flash firmware revision register. */
	reg_value = (hdr->serial_protocol << FW_REV_PROTOCOL_SHFT) |
		    (hdr->fw_version & 0xFF << FW_REV_VERSION_SHFT);
	writel_relaxed(reg_value, rsc->base + SE_GENI_FW_REVISION);

	reg_value = (hdr->serial_protocol << FW_REV_PROTOCOL_SHFT) |
		    (hdr->fw_version & 0xFF << FW_REV_VERSION_SHFT);

	writel_relaxed(reg_value, rsc->base + SE_S_FW_REVISION);
}

/**
 * geni_configure_xfer_mode() - Set the transfer mode.
 * @rsc: Pointer to a structure representing SE-related resources.
 *
 * Set the transfer mode to either FIFO or DMA according to the mode specified by the protocol
 * driver.
 *
 * Return: 0 if successful, otherwise return an error value.
 */
static int geni_configure_xfer_mode(struct qup_se_rsc *rsc)
{
	/* Configure SE FIFO, DMA or GSI mode. */
	switch (rsc->mode) {
	case GENI_GPI_DMA:
		geni_setbits32(rsc->base + QUPV3_SE_GENI_DMA_MODE_EN,
			       GENI_DMA_MODE_EN_GENI_DMA_MODE_EN_BMSK);
		writel_relaxed(0x0, rsc->base + SE_IRQ_EN);
		writel_relaxed(SE_GSI_EVENT_EN_BMSK, rsc->base + SE_GSI_EVENT_EN);
		break;

	case GENI_SE_FIFO:
		geni_clrbits32(rsc->base + QUPV3_SE_GENI_DMA_MODE_EN,
			       GENI_DMA_MODE_EN_GENI_DMA_MODE_EN_BMSK);
		writel_relaxed(SE_IRQ_EN_RMSK, rsc->base + SE_IRQ_EN);
		writel_relaxed(0x0, rsc->base + SE_GSI_EVENT_EN);
		break;

	case GENI_SE_DMA:
		geni_setbits32(rsc->base + QUPV3_SE_GENI_DMA_MODE_EN,
			       GENI_DMA_MODE_EN_GENI_DMA_MODE_EN_BMSK);
		writel_relaxed(SE_IRQ_EN_RMSK, rsc->base + SE_IRQ_EN);
		writel_relaxed(0x0, rsc->base + SE_GSI_EVENT_EN);
		break;

	default:
		dev_err(rsc->dev, "invalid se mode: %d\n", rsc->mode);
		return -EINVAL;
	}
	return 0;
}

/**
 * geni_config_common_control() - Configure common CGC and disable high priority interrupt.
 * @rsc: Pointer to a structure representing SE-related resources.
 *
 * Configure the common CGC and disable high priority interrupts until the current low priority
 * interrupts are handled.
 *
 * Return: None.
 */
static void geni_config_common_control(struct qup_se_rsc *rsc)
{
	/*
	 * Disable high priority interrupt until current low priority interrupts are handled.
	 */
	geni_setbits32(rsc->wrapper_base + QUPV3_COMMON_CFG,
		       FAST_SWITCH_TO_HIGH_DISABLE_BMASK);

	/*
	 * Set AHB_M_CLK_CGC_ON to indicate hardware controls se-wrapper cgc clock.
	 */
	geni_setbits32(rsc->wrapper_base + QUPV3_SE_AHB_M_CFG,
		       AHB_M_CLK_CGC_ON_BMASK);

	/* Let hardware to control common cgc. */
	geni_setbits32(rsc->wrapper_base + QUPV3_COMMON_CGC_CTRL,
		       COMMON_CSR_SLV_CLK_CGC_ON_BMASK);
}

static int load_se_firmware(struct qup_se_rsc *rsc, struct elf_se_hdr *hdr)
{
	const u32 *fw_val_arr, *cfg_val_arr;
	const u8 *cfg_idx_arr;
	u32 i, reg_value, mask, ramn_cnt;
	int ret;

	fw_val_arr = (const u32 *)((u8 *)hdr + hdr->fw_offset);
	cfg_idx_arr = (const u8 *)hdr + hdr->cfg_idx_offset;
	cfg_val_arr = (const u32 *)((u8 *)hdr + hdr->cfg_val_offset);

	geni_config_common_control(rsc);

	/* Allows to drive corresponding data according to hardware value. */
	writel_relaxed(0x0, rsc->base + GENI_OUTPUT_CTRL);

	/* Set SCLK and HCLK to program RAM */
	geni_setbits32(rsc->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_SCLK_OFF_BMSK |
		       GENI_CGC_CTRL_PROG_RAM_HCLK_OFF_BMSK);
	writel_relaxed(0x0, rsc->base + SE_GENI_CLK_CTRL);
	geni_clrbits32(rsc->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_SCLK_OFF_BMSK |
		       GENI_CGC_CTRL_PROG_RAM_HCLK_OFF_BMSK);

	/* Enable required clocks for DMA CSR, TX and RX. */
	reg_value = DMA_GENERAL_CFG_AHB_SEC_SLV_CLK_CGC_ON_BMSK |
		DMA_GENERAL_CFG_DMA_AHB_SLV_CLK_CGC_ON_BMSK |
		DMA_GENERAL_CFG_DMA_TX_CLK_CGC_ON_BMSK |
		DMA_GENERAL_CFG_DMA_RX_CLK_CGC_ON_BMSK;

	geni_setbits32(rsc->base + DMA_GENERAL_CFG, reg_value);

	/* Let hardware control CGC by default. */
	writel_relaxed(DEFAULT_CGC_EN, rsc->base + GENI_CGC_CTRL);

	/* Set version of the configuration register part of firmware. */
	writel_relaxed(hdr->cfg_version, rsc->base + GENI_INIT_CFG_REVISION);
	writel_relaxed(hdr->cfg_version, rsc->base + GENI_S_INIT_CFG_REVISION);

	/* Configure GENI primitive table. */
	for (i = 0; i < hdr->cfg_size_in_items; i++)
		writel_relaxed(cfg_val_arr[i],
			       rsc->base + GENI_CFG_REG0 + (cfg_idx_arr[i] * sizeof(u32)));

	/* Configure condition for assertion of RX_RFR_WATERMARK condition. */
	reg_value = readl_relaxed(rsc->base + QUPV3_SE_HW_PARAM_1);
	mask = (reg_value >> RX_FIFO_WIDTH_BIT) & RX_FIFO_WIDTH_MASK;
	writel_relaxed(mask - 2, rsc->base + GENI_RX_RFR_WATERMARK_REG);

	/* Let hardware control CGC */
	geni_setbits32(rsc->base + GENI_OUTPUT_CTRL, DEFAULT_IO_OUTPUT_CTRL_MSK);

	ret = geni_configure_xfer_mode(rsc);
	if (ret) {
		dev_err(rsc->dev, "failed to configure xfer mode: %d\n", ret);
		return ret;
	}

	geni_enable_interrupts(rsc);

	geni_flash_fw_revision(rsc, hdr);

	ramn_cnt = hdr->fw_size_in_items;
	if (hdr->fw_size_in_items % 2 != 0)
		ramn_cnt++;

	if (ramn_cnt >= MAX_GENI_CFG_RAMn_CNT) {
		dev_err(rsc->dev, "firmware size is too large\n");
		return -EINVAL;
	}

	/* Program RAM address space. */
	for (i = 0; i < hdr->fw_size_in_items; i++)
		writel_relaxed(fw_val_arr[i], rsc->base + SE_GENI_CFG_RAMN + i * sizeof(u32));

	/* Put default values on GENI's output pads. */
	writel_relaxed(0x1, rsc->base + GENI_FORCE_DEFAULT_REG);

	/* High to low SCLK and HCLK to finish RAM. */
	geni_setbits32(rsc->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_SCLK_OFF_BMSK |
			GENI_CGC_CTRL_PROG_RAM_HCLK_OFF_BMSK);
	geni_setbits32(rsc->base + SE_GENI_CLK_CTRL, GENI_CLK_CTRL_SER_CLK_SEL_BMSK);
	geni_clrbits32(rsc->base + GENI_CGC_CTRL, GENI_CGC_CTRL_PROG_RAM_SCLK_OFF_BMSK |
			GENI_CGC_CTRL_PROG_RAM_HCLK_OFF_BMSK);

	/* Serial engine DMA interface is enabled. */
	geni_setbits32(rsc->base + SE_DMA_IF_EN, DMA_IF_EN_DMA_IF_EN_BMSK);

	/* Enable or disable FIFO interface of the serial engine. */
	if (rsc->mode == GENI_SE_FIFO)
		geni_clrbits32(rsc->base + SE_FIFO_IF_DISABLE, FIFO_IF_DISABLE);
	else
		geni_setbits32(rsc->base + SE_FIFO_IF_DISABLE, FIFO_IF_DISABLE);

	return 0;
}

/**
 * elf_phdr_valid() - Validate an ELF header.
 * @phdr: Pointer to the ELF header.
 *
 * Validate the ELF header by comparing the fields stored in p_flags and the payload type.
 *
 * Return: true if the validation is successful, false otherwise.
 */
static bool elf_phdr_valid(const Elf32_Phdr *phdr)
{
	if (phdr->p_type != PT_LOAD || !phdr->p_memsz)
		return false;

	if (MI_PBT_PAGE_MODE_VALUE(phdr->p_flags) == MI_PBT_NON_PAGED_SEGMENT &&
	    MI_PBT_SEGMENT_TYPE_VALUE(phdr->p_flags) != MI_PBT_HASH_SEGMENT &&
	    MI_PBT_ACCESS_TYPE_VALUE(phdr->p_flags) != MI_PBT_NOTUSED_SEGMENT &&
	    MI_PBT_ACCESS_TYPE_VALUE(phdr->p_flags) != MI_PBT_SHARED_SEGMENT)
		return true;

	return false;
}

/**
 * valid_seg_size() - Validate the segment size.
 * @pelfseg: Pointer to the ELF header.
 * @p_filesz: Pointer to the file size.
 *
 * Validate the ELF segment size by comparing the file size.
 *
 * Return: true if the segment is valid, false if the segment is invalid.
 */
static bool valid_seg_size(struct elf_se_hdr *pelfseg, Elf32_Word p_filesz)
{
	if (p_filesz >= pelfseg->fw_offset + pelfseg->fw_size_in_items * sizeof(u32) &&
	    p_filesz >= pelfseg->cfg_idx_offset + pelfseg->cfg_size_in_items * sizeof(u8) &&
	    p_filesz >= pelfseg->cfg_val_offset + pelfseg->cfg_size_in_items * sizeof(u32))
		return true;
	return false;
}

/**
 * read_elf() - Read an ELF file.
 * @rsc: Pointer to the SE resources structure.
 * @fw: Pointer to the firmware buffer.
 * @pelfseg: Pointer to the SE-specific ELF header.
 * @phdr: Pointer to one of the valid headers from the list in the firmware buffer.
 *
 * Read the ELF file and output a pointer to the header data, which contains the firmware data and
 * any other details.
 *
 * Return: 0 if successful, otherwise return an error value.
 */
static int read_elf(struct qup_se_rsc *rsc, const void *fw,
		    struct elf_se_hdr **pelfseg)
{
	Elf32_Phdr *phdr;
	const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)fw;
	Elf32_Phdr *phdrs = (Elf32_Phdr *)(ehdr + 1);
	const u8 *addr;
	int i;

	ehdr = (Elf32_Ehdr *)fw;

	if (ehdr->e_phnum < 2)
		return -EINVAL;

	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = &phdrs[i];
		if (!elf_phdr_valid(phdr))
			continue;

		if (phdr->p_filesz >= sizeof(struct elf_se_hdr)) {
			addr =  fw + phdr->p_offset;
			*pelfseg = (struct elf_se_hdr *)addr;

			if ((*pelfseg)->magic == MAGIC_NUM_SE &&
			    (*pelfseg)->version == 1 &&
			    valid_seg_size(*pelfseg, phdr->p_filesz) &&
			    (*pelfseg)->serial_protocol == rsc->protocol &&
			    (*pelfseg)->serial_protocol != GENI_SE_NONE)
				return 0;
		}
	}
	return -EINVAL;
}

int qcom_geni_load_firmware(phys_addr_t qup_base,
			    struct udevice *dev)
{
	struct qup_se_rsc rsc;
	struct elf_se_hdr *hdr;
	int ret;
	void *fw;

	rsc.dev = dev;
	rsc.base = qup_base;
	rsc.wrapper_base = dev_read_addr(dev->parent);

	/* FIXME: GSI DMA mode if device has property qcom,gsi-dma-allowed */
	rsc.mode = GENI_SE_FIFO;

	switch (device_get_uclass_id(dev)) {
	case UCLASS_I2C:
		rsc.protocol = GENI_SE_I2C;
		break;
	case UCLASS_SPI:
		rsc.protocol = GENI_SE_SPI;
		break;
	case UCLASS_SERIAL:
		rsc.protocol = GENI_SE_UART;
		break;
	default:
		return -EINVAL;
	}

	/* The firmware blob is the private data of the GENI wrapper (parent) */
	fw = dev_get_priv(dev->parent);

	ret = read_elf(&rsc, fw, &hdr);
	if (ret) {
		dev_err(dev, "Failed to read ELF: %d\n", ret);
		return ret;
	}

	dev_info(dev, "Loading QUP firmware...\n");

	return load_se_firmware(&rsc, hdr);
}

/*
 * We need to determine if firmware loading is necessary. Best way to do that is to check the FW
 * revision of each QUP and see if it has already been loaded.
 */
static int geni_se_of_to_plat(struct udevice *dev)
{
	ofnode child;
	struct resource res;
	u32 proto;
	struct geni_se_plat *plat = dev_get_plat(dev);

	plat->need_firmware_load = false;

	dev_for_each_subnode(child, dev) {
		if (!ofnode_is_enabled(child))
			continue;

		if (ofnode_read_resource(child, 0, &res))
			continue;

		proto = readl(res.start + GENI_FW_REVISION_RO);
		proto &= FW_REV_PROTOCOL_MSK;
		proto >>= FW_REV_PROTOCOL_SHFT;

		if (proto == GENI_SE_INVALID_PROTO)
			plat->need_firmware_load = true;
	}

	return 0;
}

#define QUPFW_PART_TYPE_GUID "21d1219f-2ed1-4ab4-930a-41a16ae75f7f"

static int find_qupfw_part(struct udevice **blk_dev, struct disk_partition *part_info)
{
	struct blk_desc *desc;
	int ret, partnum;

	uclass_foreach_dev_probe(UCLASS_BLK, *blk_dev) {
		if (device_get_uclass_id(*blk_dev) != UCLASS_BLK)
			continue;

		desc = dev_get_uclass_plat(*blk_dev);
		if (!desc || desc->part_type == PART_TYPE_UNKNOWN)
			continue;
		for (partnum = 1;; partnum++) {
			ret = part_get_info(desc, partnum, part_info);
			if (ret)
				break;
			if (!strcmp(part_info->type_guid, QUPFW_PART_TYPE_GUID))
				return 0;
		}
	}

	return -ENOENT;
}

static int probe_children_load_firmware(struct udevice *dev)
{
	struct geni_se_plat *plat;
	ofnode child;
	struct udevice *child_dev;
	struct resource res;
	u32 proto;
	int ret;

	plat = dev_get_plat(dev);

	dev_for_each_subnode(child, dev) {
		if (!ofnode_is_enabled(child))
			continue;

		if (ofnode_read_resource(child, 0, &res))
			continue;

		proto = readl(res.start + GENI_FW_REVISION_RO);
		proto &= FW_REV_PROTOCOL_MSK;
		proto >>= FW_REV_PROTOCOL_SHFT;

		if (proto != GENI_SE_INVALID_PROTO)
			continue;

		ret = 0;
		/* Find the device for this ofnode, or bind it */
		if (device_find_global_by_ofnode(child, &child_dev))
			ret = lists_bind_fdt(dev, child, &child_dev, NULL, false);	
		if (ret) {
			/* Skip nodes that don't have drivers */
			debug("Failed to probe child %s: %d\n", ofnode_get_name(child), ret);
			continue;
		}
		debug("Probing child %s for fw loading\n", child_dev->name);
		device_probe(child_dev);
	}

	return 0;
}

#define MAX_FW_BUF_SIZE (128 * 1024)

/*
 * Load firmware for QCOM GENI peripherals from the dedicated partition on storage and bind/probe
 * all the peripheral devices that need firmware to be loaded.
 */
static int qcom_geni_fw_initialise(void)
{
	debug("Loading firmware for QCOM GENI SE\n");
	struct udevice *geni_wrapper, *blk_dev;
	struct disk_partition part_info;
	int ret;
	void *fw_buf;
	size_t fw_size = MAX_FW_BUF_SIZE;
	struct geni_se_plat *plat;

	/* Find the first GENI SE wrapper that needs fw loading */
	for (uclass_first_device(UCLASS_MISC, &geni_wrapper);
	     geni_wrapper;
	     uclass_next_device(&geni_wrapper)) {
		if (device_get_uclass_id(geni_wrapper) == UCLASS_MISC &&
		    !strcmp(geni_wrapper->driver->name, "geni-se-qup")) {
			plat = dev_get_plat(geni_wrapper);
			if (plat->need_firmware_load)
				break;
		}
	}
	if (!geni_wrapper) {
		pr_err("GENI SE wrapper not found\n");
		return 0;
	}

	ret = find_qupfw_part(&blk_dev, &part_info);
	if (ret) {
		pr_err("QUP firmware partition not found\n");
		return 0;
	}

	if (part_info.size * part_info.blksz > MAX_FW_BUF_SIZE) {
		pr_err("Firmware partition too large\n");
		return -EINVAL;
	}
	fw_size = part_info.size * part_info.blksz;

	fw_buf = malloc(fw_size);
	if (!fw_buf) {
		pr_err("Failed to allocate buffer for firmware\n");
		return -ENOMEM;
	}
	memset(fw_buf, 0, fw_size);

	ret = blk_read(blk_dev, part_info.start, part_info.size, fw_buf);
	if (ret < 0) {
		pr_err("Failed to read firmware from partition\n");
		free(fw_buf);
		return 0;
	}

	/*
	 * OK! Firmware is loaded, now bind and probe remaining children. They will attempt to load
	 * firmware during probe. Do this for each GENI SE wrapper that needs firmware loading.
	 */
	for (; geni_wrapper;
	     uclass_next_device(&geni_wrapper)) {
		if (device_get_uclass_id(geni_wrapper) == UCLASS_MISC &&
		    !strcmp(geni_wrapper->driver->name, "geni-se-qup")) {
			plat = dev_get_plat(geni_wrapper);
			if (plat->need_firmware_load) {
				dev_set_priv(geni_wrapper, fw_buf);
				probe_children_load_firmware(geni_wrapper);
			}
		}
	}

	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, qcom_geni_fw_initialise);

static const struct udevice_id geni_ids[] = {
	{ .compatible = "qcom,geni-se-qup" },
	{}
};

U_BOOT_DRIVER(geni_se_qup) = {
	.name = "geni-se-qup",
	.id = UCLASS_MISC,
	.of_match = geni_ids,
	.of_to_plat = geni_se_of_to_plat,
	.plat_auto = sizeof(struct geni_se_plat),
	.flags = DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
