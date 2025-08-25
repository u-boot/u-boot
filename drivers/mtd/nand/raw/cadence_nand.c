// SPDX-License-Identifier: GPL-2.0+
/*
 * Cadence NAND flash controller driver
 *
 * Copyright (C) 2019 Cadence
 *
 * Author: Piotr Sroka <piotrs@cadence.com>
 *
 */

#include <cadence-nand.h>
#include <clk.h>
#include <dm.h>
#include <hang.h>
#include <malloc.h>
#include <memalign.h>
#include <nand.h>
#include <reset.h>
#include <wait_bit.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitfield.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/printk.h>
#include <linux/sizes.h>

static inline struct
cdns_nand_chip *to_cdns_nand_chip(struct nand_chip *chip)
{
	return container_of(chip, struct cdns_nand_chip, chip);
}

static inline struct
cadence_nand_info *to_cadence_nand_info(struct nand_hw_control *controller)
{
	return container_of(controller, struct cadence_nand_info, controller);
}

static bool
cadence_nand_dma_buf_ok(struct cadence_nand_info *cadence, const void *buf,
			u32 buf_len)
{
	u8 data_dma_width = cadence->caps2.data_dma_width;

	return buf &&
		likely(IS_ALIGNED((uintptr_t)buf, data_dma_width)) &&
		likely(IS_ALIGNED(buf_len, DMA_DATA_SIZE_ALIGN));
}

static int cadence_nand_wait_for_value(struct cadence_nand_info *cadence,
				       u32 reg_offset, u32 timeout_us,
				       u32 mask, bool is_clear)
{
	u32 val;
	int ret;

	ret = readl_poll_sleep_timeout(cadence->reg + reg_offset,
				       val, !(val & mask) == is_clear,
				       10, timeout_us);

	if (ret < 0) {
		dev_err(cadence->dev,
			"Timeout while waiting for reg %x with mask %x is clear %d\n",
			reg_offset, mask, is_clear);
	}

	return ret;
}

static int cadence_nand_set_ecc_enable(struct cadence_nand_info *cadence,
				       bool enable)
{
	u32 reg;

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	reg = readl_relaxed(cadence->reg + ECC_CONFIG_0);

	if (enable)
		reg |= ECC_CONFIG_0_ECC_EN;
	else
		reg &= ~ECC_CONFIG_0_ECC_EN;

	writel_relaxed(reg, cadence->reg + ECC_CONFIG_0);

	return 0;
}

static void cadence_nand_set_ecc_strength(struct cadence_nand_info *cadence,
					  u8 corr_str_idx)
{
	u32 reg;

	if (cadence->curr_corr_str_idx == corr_str_idx)
		return;

	reg = readl_relaxed(cadence->reg + ECC_CONFIG_0);
	reg &= ~ECC_CONFIG_0_CORR_STR;
	reg |= FIELD_PREP(ECC_CONFIG_0_CORR_STR, corr_str_idx);
	writel_relaxed(reg, cadence->reg + ECC_CONFIG_0);

	cadence->curr_corr_str_idx = corr_str_idx;
}

static int cadence_nand_get_ecc_strength_idx(struct cadence_nand_info *cadence,
					     u8 strength)
{
	int i, corr_str_idx = -1;

	for (i = 0; i < BCH_MAX_NUM_CORR_CAPS; i++) {
		if (cadence->ecc_strengths[i] == strength) {
			corr_str_idx = i;
			break;
		}
	}

	return corr_str_idx;
}

static int cadence_nand_set_skip_marker_val(struct cadence_nand_info *cadence,
					    u16 marker_value)
{
	u32 reg;

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	reg = readl_relaxed(cadence->reg + SKIP_BYTES_CONF);
	reg &= ~SKIP_BYTES_MARKER_VALUE;
	reg |= FIELD_PREP(SKIP_BYTES_MARKER_VALUE,
			  marker_value);

	writel_relaxed(reg, cadence->reg + SKIP_BYTES_CONF);

	return 0;
}

static int cadence_nand_set_skip_bytes_conf(struct cadence_nand_info *cadence,
					    u8 num_of_bytes,
					    u32 offset_value,
					    int enable)
{
	u32 reg, skip_bytes_offset;

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	if (!enable) {
		num_of_bytes = 0;
		offset_value = 0;
	}

	reg = readl_relaxed(cadence->reg + SKIP_BYTES_CONF);
	reg &= ~SKIP_BYTES_NUM_OF_BYTES;
	reg |= FIELD_PREP(SKIP_BYTES_NUM_OF_BYTES,
			  num_of_bytes);
	skip_bytes_offset = FIELD_PREP(SKIP_BYTES_OFFSET_VALUE,
				       offset_value);

	writel_relaxed(reg, cadence->reg + SKIP_BYTES_CONF);
	writel_relaxed(skip_bytes_offset, cadence->reg + SKIP_BYTES_OFFSET);

	return 0;
}

/* Functions enables/disables hardware detection of erased data */
static void cadence_nand_set_erase_detection(struct cadence_nand_info *cadence,
					     bool enable,
					     u8 bitflips_threshold)
{
	u32 reg;

	reg = readl_relaxed(cadence->reg + ECC_CONFIG_0);

	if (enable)
		reg |= ECC_CONFIG_0_ERASE_DET_EN;
	else
		reg &= ~ECC_CONFIG_0_ERASE_DET_EN;

	writel_relaxed(reg, cadence->reg + ECC_CONFIG_0);
	writel_relaxed(bitflips_threshold, cadence->reg + ECC_CONFIG_1);
}

static int cadence_nand_set_access_width16(struct cadence_nand_info *cadence,
					   bool bit_bus16)
{
	u32 reg;

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	reg = readl_relaxed(cadence->reg + COMMON_SET);
	if (!bit_bus16)
		reg &= ~COMMON_SET_DEVICE_16BIT;
	else
		reg |= COMMON_SET_DEVICE_16BIT;
	writel_relaxed(reg, cadence->reg + COMMON_SET);

	return 0;
}

static void
cadence_nand_clear_interrupt(struct cadence_nand_info *cadence,
			     struct cadence_nand_irq_status *irq_status)
{
	writel_relaxed(irq_status->status, cadence->reg + INTR_STATUS);
	writel_relaxed(irq_status->trd_status,
		       cadence->reg + TRD_COMP_INT_STATUS);
	writel_relaxed(irq_status->trd_error,
		       cadence->reg + TRD_ERR_INT_STATUS);
}

static void
cadence_nand_read_int_status(struct cadence_nand_info *cadence,
			     struct cadence_nand_irq_status *irq_status)
{
	irq_status->status = readl_relaxed(cadence->reg + INTR_STATUS);
	irq_status->trd_status = readl_relaxed(cadence->reg
					       + TRD_COMP_INT_STATUS);
	irq_status->trd_error = readl_relaxed(cadence->reg
					      + TRD_ERR_INT_STATUS);
}

static u32 irq_detected(struct cadence_nand_info *cadence,
			struct cadence_nand_irq_status *irq_status)
{
	cadence_nand_read_int_status(cadence, irq_status);

	return irq_status->status || irq_status->trd_status ||
		irq_status->trd_error;
}

static void cadence_nand_reset_irq(struct cadence_nand_info *cadence)
{
	memset(&cadence->irq_status, 0, sizeof(cadence->irq_status));
	memset(&cadence->irq_mask, 0, sizeof(cadence->irq_mask));
}

/*
 * This is the interrupt service routine. It handles all interrupts
 * sent to this device.
 */
static irqreturn_t cadence_nand_isr(struct cadence_nand_info *cadence)
{
	struct cadence_nand_irq_status irq_status;
	irqreturn_t result = IRQ_NONE;

	if (irq_detected(cadence, &irq_status)) {
		/* Handle interrupt. */
		/* First acknowledge it. */
		cadence_nand_clear_interrupt(cadence, &irq_status);
		/* Status in the device context for someone to read. */
		cadence->irq_status.status |= irq_status.status;
		cadence->irq_status.trd_status |= irq_status.trd_status;
		cadence->irq_status.trd_error |= irq_status.trd_error;
		/* Tell the OS that we've handled this. */
		result = IRQ_HANDLED;
	}
	return result;
}

static void cadence_nand_set_irq_mask(struct cadence_nand_info *cadence,
				      struct cadence_nand_irq_status *irq_mask)
{
	writel_relaxed(INTR_ENABLE_INTR_EN | irq_mask->status,
		       cadence->reg + INTR_ENABLE);

	writel_relaxed(irq_mask->trd_error,
		       cadence->reg + TRD_ERR_INT_STATUS_EN);
}

static void
cadence_nand_wait_for_irq(struct cadence_nand_info *cadence,
			  struct cadence_nand_irq_status *irq_mask,
			  struct cadence_nand_irq_status *irq_status)
{
	irqreturn_t result = IRQ_NONE;
	u32 start = get_timer(0);

	while (get_timer(start) < TIMEOUT_US) {
		result = cadence_nand_isr(cadence);

		if (result == IRQ_HANDLED) {
			*irq_status = cadence->irq_status;
			break;
		}
		udelay(1);
	}

	if (!result) {
		/* Timeout error. */
		dev_err(cadence->dev, "timeout occurred:\n");
		dev_err(cadence->dev, "\tstatus = 0x%x, mask = 0x%x\n",
			irq_status->status, irq_mask->status);
		dev_err(cadence->dev,
			"\ttrd_status = 0x%x, trd_status mask = 0x%x\n",
			irq_status->trd_status, irq_mask->trd_status);
		dev_err(cadence->dev,
			"\t trd_error = 0x%x, trd_error mask = 0x%x\n",
			irq_status->trd_error, irq_mask->trd_error);
	}
}

/* Execute generic command on NAND controller. */
static int cadence_nand_generic_cmd_send(struct cadence_nand_info *cadence,
					 u8 chip_nr,
					 u64 mini_ctrl_cmd)
{
	u32 mini_ctrl_cmd_l, mini_ctrl_cmd_h, reg;

	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAY_CS, chip_nr);
	mini_ctrl_cmd_l = mini_ctrl_cmd & 0xFFFFFFFF;
	mini_ctrl_cmd_h = mini_ctrl_cmd >> 32;

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	cadence_nand_reset_irq(cadence);

	writel_relaxed(mini_ctrl_cmd_l, cadence->reg + CMD_REG2);
	writel_relaxed(mini_ctrl_cmd_h, cadence->reg + CMD_REG3);

	/* Select generic command. */
	reg = FIELD_PREP(CMD_REG0_CT, CMD_REG0_CT_GEN);
	/* Thread number. */
	reg |= FIELD_PREP(CMD_REG0_TN, 0);

	/* Issue command. */
	writel_relaxed(reg, cadence->reg + CMD_REG0);
	cadence->buf_index = 0;

	return 0;
}

/* Wait for data on slave DMA interface. */
static int cadence_nand_wait_on_sdma(struct cadence_nand_info *cadence, u8 *out_sdma_trd,
				     u32 *out_sdma_size)
{
	struct cadence_nand_irq_status irq_mask, irq_status;

	irq_mask.trd_status = 0;
	irq_mask.trd_error = 0;
	irq_mask.status = INTR_STATUS_SDMA_TRIGG
		| INTR_STATUS_SDMA_ERR
		| INTR_STATUS_UNSUPP_CMD;

	cadence_nand_set_irq_mask(cadence, &irq_mask);
	cadence_nand_wait_for_irq(cadence, &irq_mask, &irq_status);
	if (irq_status.status == 0) {
		dev_err(cadence->dev, "Timeout while waiting for SDMA\n");
		return -ETIMEDOUT;
	}

	if (irq_status.status & INTR_STATUS_SDMA_TRIGG) {
		*out_sdma_size = readl_relaxed(cadence->reg + SDMA_SIZE);
		*out_sdma_trd  = readl_relaxed(cadence->reg + SDMA_TRD_NUM);
		*out_sdma_trd =
			FIELD_GET(SDMA_TRD_NUM_SDMA_TRD, *out_sdma_trd);
	} else {
		dev_err(cadence->dev, "SDMA error - irq_status %x\n",
			irq_status.status);
		return -EIO;
	}

	return 0;
}

static void cadence_nand_get_caps(struct cadence_nand_info *cadence)
{
	u32  reg;

	reg = readl_relaxed(cadence->reg + CTRL_FEATURES);

	cadence->caps2.max_banks = 1 << FIELD_GET(CTRL_FEATURES_N_BANKS, reg);

	if (FIELD_GET(CTRL_FEATURES_DMA_DWITH64, reg))
		cadence->caps2.data_dma_width = 8;
	else
		cadence->caps2.data_dma_width = 4;

	if (reg & CTRL_FEATURES_CONTROL_DATA)
		cadence->caps2.data_control_supp = true;

	if (reg & (CTRL_FEATURES_NVDDR_2_3
		   | CTRL_FEATURES_NVDDR))
		cadence->caps2.is_phy_type_dll = true;
}

/* Prepare CDMA descriptor. */
static void
cadence_nand_cdma_desc_prepare(struct cadence_nand_info *cadence,
			       char nf_mem, u32 flash_ptr, dma_addr_t mem_ptr,
			       dma_addr_t ctrl_data_ptr, u16 ctype)
{
	struct cadence_nand_cdma_desc *cdma_desc = cadence->cdma_desc;

	memset(cdma_desc, 0, sizeof(struct cadence_nand_cdma_desc));

	/* Set fields for one descriptor. */
	cdma_desc->flash_pointer = flash_ptr;
	if (cadence->ctrl_rev >= 13)
		cdma_desc->bank = nf_mem;
	else
		cdma_desc->flash_pointer |= (nf_mem << CDMA_CFPTR_MEM_SHIFT);

	cdma_desc->command_flags |= CDMA_CF_DMA_MASTER;
	cdma_desc->command_flags  |= CDMA_CF_INT;

	cdma_desc->memory_pointer = mem_ptr;
	cdma_desc->status = 0;
	cdma_desc->sync_flag_pointer = 0;
	cdma_desc->sync_arguments = 0;

	cdma_desc->command_type = ctype;
	cdma_desc->ctrl_data_ptr = ctrl_data_ptr;

	flush_cache((dma_addr_t)cadence->cdma_desc,
		    ROUND(sizeof(struct cadence_nand_cdma_desc),
			  ARCH_DMA_MINALIGN));
}

static u8 cadence_nand_check_desc_error(struct cadence_nand_info *cadence,
					u32 desc_status)
{
	if (desc_status & CDMA_CS_ERP)
		return STAT_ERASED;

	if (desc_status & CDMA_CS_UNCE)
		return STAT_ECC_UNCORR;

	if (desc_status & CDMA_CS_ERR) {
		dev_err(cadence->dev, ":CDMA desc error flag detected.\n");
		return STAT_FAIL;
	}

	if (FIELD_GET(CDMA_CS_MAXERR, desc_status))
		return STAT_ECC_CORR;

	return STAT_FAIL;
}

static int cadence_nand_cdma_finish(struct cadence_nand_info *cadence)
{
	struct cadence_nand_cdma_desc *desc_ptr = cadence->cdma_desc;
	u8 status = STAT_BUSY;

	invalidate_dcache_range((dma_addr_t)cadence->cdma_desc,
				(dma_addr_t)cadence->cdma_desc +
				ROUND(sizeof(struct cadence_nand_cdma_desc),
				      ARCH_DMA_MINALIGN));

	if (desc_ptr->status & CDMA_CS_FAIL) {
		status = cadence_nand_check_desc_error(cadence,
						       desc_ptr->status);
		dev_err(cadence->dev, ":CDMA error %x\n", desc_ptr->status);
	} else if (desc_ptr->status & CDMA_CS_COMP) {
		/* Descriptor finished with no errors. */
		if (desc_ptr->command_flags & CDMA_CF_CONT) {
			dev_info(cadence->dev, "DMA unsupported flag is set");
			status = STAT_UNKNOWN;
		} else {
			/* Last descriptor.  */
			status = STAT_OK;
		}
	}

	return status;
}

static int cadence_nand_cdma_send(struct cadence_nand_info *cadence,
				  u8 thread)
{
	u32 reg;
	int status;

	/* Wait for thread ready. */
	status = cadence_nand_wait_for_value(cadence, TRD_STATUS,
					     TIMEOUT_US,
					     BIT(thread), true);
	if (status)
		return status;

	cadence_nand_reset_irq(cadence);

	writel_relaxed((u32)cadence->dma_cdma_desc,
		       cadence->reg + CMD_REG2);
	writel_relaxed(0, cadence->reg + CMD_REG3);

	/* Select CDMA mode. */
	reg = FIELD_PREP(CMD_REG0_CT, CMD_REG0_CT_CDMA);
	/* Thread number. */
	reg |= FIELD_PREP(CMD_REG0_TN, thread);
	/* Issue command. */
	writel_relaxed(reg, cadence->reg + CMD_REG0);

	return 0;
}

/* Send SDMA command and wait for finish. */
static u32
cadence_nand_cdma_send_and_wait(struct cadence_nand_info *cadence,
				u8 thread)
{
	struct cadence_nand_irq_status irq_mask, irq_status = {0};
	int status;
	u32 val;

	irq_mask.trd_status = BIT(thread);
	irq_mask.trd_error = BIT(thread);
	irq_mask.status = INTR_STATUS_CDMA_TERR;

	cadence_nand_set_irq_mask(cadence, &irq_mask);

	status = cadence_nand_cdma_send(cadence, thread);
	if (status)
		return status;

	/* Make sure the descriptor processing is complete */
	status = readl_poll_timeout(cadence->reg + TRD_COMP_INT_STATUS, val,
				    (val & BIT(thread)), TIMEOUT_US);
	if (status) {
		pr_err("cmd thread completion timeout!\n");
		return status;
	}

	cadence_nand_wait_for_irq(cadence, &irq_mask, &irq_status);

	if (irq_status.status == 0 && irq_status.trd_status == 0 &&
	    irq_status.trd_error == 0) {
		dev_err(cadence->dev, "CDMA command timeout\n");
		return -ETIMEDOUT;
	}
	if (irq_status.status & irq_mask.status) {
		dev_err(cadence->dev, "CDMA command failed\n");
		return -EIO;
	}

	return 0;
}

/*
 * ECC size depends on configured ECC strength and on maximum supported
 * ECC step size.
 */
static int cadence_nand_calc_ecc_bytes(int max_step_size, int strength)
{
	int nbytes = DIV_ROUND_UP(fls(8 * max_step_size) * strength, 8);

	return ALIGN(nbytes, 2);
}

#define CADENCE_NAND_CALC_ECC_BYTES(max_step_size) \
	static int \
	cadence_nand_calc_ecc_bytes_##max_step_size(int step_size, \
						    int strength)\
	{\
		return cadence_nand_calc_ecc_bytes(max_step_size, strength);\
	}

CADENCE_NAND_CALC_ECC_BYTES(256)
CADENCE_NAND_CALC_ECC_BYTES(512)
CADENCE_NAND_CALC_ECC_BYTES(1024)
CADENCE_NAND_CALC_ECC_BYTES(2048)
CADENCE_NAND_CALC_ECC_BYTES(4096)

/* Function reads BCH capabilities. */
static int cadence_nand_read_bch_caps(struct cadence_nand_info *cadence)
{
	struct nand_ecc_caps *ecc_caps = &cadence->ecc_caps;
	int max_step_size = 0, nstrengths, i;
	u32 reg;

	reg = readl_relaxed(cadence->reg + BCH_CFG_3);
	cadence->bch_metadata_size = FIELD_GET(BCH_CFG_3_METADATA_SIZE, reg);
	if (cadence->bch_metadata_size < 4) {
		dev_err(cadence->dev,
			"Driver needs at least 4 bytes of BCH meta data\n");
		return -EIO;
	}

	reg = readl_relaxed(cadence->reg + BCH_CFG_0);
	cadence->ecc_strengths[0] = FIELD_GET(BCH_CFG_0_CORR_CAP_0, reg);
	cadence->ecc_strengths[1] = FIELD_GET(BCH_CFG_0_CORR_CAP_1, reg);
	cadence->ecc_strengths[2] = FIELD_GET(BCH_CFG_0_CORR_CAP_2, reg);
	cadence->ecc_strengths[3] = FIELD_GET(BCH_CFG_0_CORR_CAP_3, reg);

	reg = readl_relaxed(cadence->reg + BCH_CFG_1);
	cadence->ecc_strengths[4] = FIELD_GET(BCH_CFG_1_CORR_CAP_4, reg);
	cadence->ecc_strengths[5] = FIELD_GET(BCH_CFG_1_CORR_CAP_5, reg);
	cadence->ecc_strengths[6] = FIELD_GET(BCH_CFG_1_CORR_CAP_6, reg);
	cadence->ecc_strengths[7] = FIELD_GET(BCH_CFG_1_CORR_CAP_7, reg);

	reg = readl_relaxed(cadence->reg + BCH_CFG_2);
	cadence->ecc_stepinfos[0].stepsize =
		FIELD_GET(BCH_CFG_2_SECT_0, reg);

	cadence->ecc_stepinfos[1].stepsize =
		FIELD_GET(BCH_CFG_2_SECT_1, reg);

	nstrengths = 0;
	for (i = 0; i < BCH_MAX_NUM_CORR_CAPS; i++) {
		if (cadence->ecc_strengths[i] != 0)
			nstrengths++;
	}

	ecc_caps->nstepinfos = 0;
	for (i = 0; i < BCH_MAX_NUM_SECTOR_SIZES; i++) {
		/* ECC strengths are common for all step infos. */
		cadence->ecc_stepinfos[i].nstrengths = nstrengths;
		cadence->ecc_stepinfos[i].strengths =
			cadence->ecc_strengths;

		if (cadence->ecc_stepinfos[i].stepsize != 0)
			ecc_caps->nstepinfos++;

		if (cadence->ecc_stepinfos[i].stepsize > max_step_size)
			max_step_size = cadence->ecc_stepinfos[i].stepsize;
	}
	ecc_caps->stepinfos = &cadence->ecc_stepinfos[0];

	switch (max_step_size) {
	case 256:
		ecc_caps->calc_ecc_bytes = &cadence_nand_calc_ecc_bytes_256;
		break;
	case 512:
		ecc_caps->calc_ecc_bytes = &cadence_nand_calc_ecc_bytes_512;
		break;
	case 1024:
		ecc_caps->calc_ecc_bytes = &cadence_nand_calc_ecc_bytes_1024;
		break;
	case 2048:
		ecc_caps->calc_ecc_bytes = &cadence_nand_calc_ecc_bytes_2048;
		break;
	case 4096:
		ecc_caps->calc_ecc_bytes = &cadence_nand_calc_ecc_bytes_4096;
		break;
	default:
		dev_err(cadence->dev,
			"Unsupported sector size(ecc step size) %d\n",
			max_step_size);
		return -EIO;
	}

	return 0;
}

/* Hardware initialization. */
static int cadence_nand_hw_init(struct cadence_nand_info *cadence)
{
	int status;
	u32 reg;

	status = cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					     TIMEOUT_US,
					     CTRL_STATUS_INIT_COMP, false);
	if (status)
		return status;

	reg = readl_relaxed(cadence->reg + CTRL_VERSION);
	cadence->ctrl_rev = FIELD_GET(CTRL_VERSION_REV, reg);

	dev_info(cadence->dev,
		 "%s: cadence nand controller version reg %x\n",
		 __func__, reg);

	/* Disable cache and multiplane. */
	writel_relaxed(0, cadence->reg + MULTIPLANE_CFG);
	writel_relaxed(0, cadence->reg + CACHE_CFG);

	/* Clear all interrupts. */
	writel_relaxed(0xFFFFFFFF, cadence->reg + INTR_STATUS);

	cadence_nand_get_caps(cadence);
	if (cadence_nand_read_bch_caps(cadence))
		return -EIO;

	/*
	 * Set IO width access to 8.
	 * It is because during SW device discovering width access
	 * is expected to be 8.
	 */
	status = cadence_nand_set_access_width16(cadence, false);

	return status;
}

#define TT_MAIN_OOB_AREAS		2
#define TT_RAW_PAGE			3
#define TT_BBM				4
#define TT_MAIN_OOB_AREA_EXT		5

/* Prepare size of data to transfer. */
static void
cadence_nand_prepare_data_size(struct mtd_info *mtd,
			       int transfer_type)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	u32 sec_size = 0, offset = 0, sec_cnt = 1;
	u32 last_sec_size = cdns_chip->sector_size;
	u32 data_ctrl_size = 0;
	u32 reg = 0;

	if (cadence->curr_trans_type == transfer_type)
		return;

	switch (transfer_type) {
	case TT_MAIN_OOB_AREA_EXT:
		sec_cnt = cdns_chip->sector_count;
		sec_size = cdns_chip->sector_size;
		data_ctrl_size = cdns_chip->avail_oob_size;
		break;
	case TT_MAIN_OOB_AREAS:
		sec_cnt = cdns_chip->sector_count;
		last_sec_size = cdns_chip->sector_size
			+ cdns_chip->avail_oob_size;
		sec_size = cdns_chip->sector_size;
		break;
	case TT_RAW_PAGE:
		last_sec_size = mtd->writesize + mtd->oobsize;
		break;
	case TT_BBM:
		offset = mtd->writesize + cdns_chip->bbm_offs;
		last_sec_size = 8;
		break;
	}

	reg = 0;
	reg |= FIELD_PREP(TRAN_CFG_0_OFFSET, offset);
	reg |= FIELD_PREP(TRAN_CFG_0_SEC_CNT, sec_cnt);
	writel_relaxed(reg, cadence->reg + TRAN_CFG_0);

	reg = 0;
	reg |= FIELD_PREP(TRAN_CFG_1_LAST_SEC_SIZE, last_sec_size);
	reg |= FIELD_PREP(TRAN_CFG_1_SECTOR_SIZE, sec_size);
	writel_relaxed(reg, cadence->reg + TRAN_CFG_1);

	if (cadence->caps2.data_control_supp) {
		reg = readl_relaxed(cadence->reg + CONTROL_DATA_CTRL);
		reg &= ~CONTROL_DATA_CTRL_SIZE;
		reg |= FIELD_PREP(CONTROL_DATA_CTRL_SIZE, data_ctrl_size);
		writel_relaxed(reg, cadence->reg + CONTROL_DATA_CTRL);
	}

	cadence->curr_trans_type = transfer_type;
}

static int
cadence_nand_cdma_transfer(struct cadence_nand_info *cadence, u8 chip_nr,
			   int page, void *buf, void *ctrl_dat, u32 buf_size,
			   u32 ctrl_dat_size, enum dma_data_direction dir,
			   bool with_ecc)
{
	dma_addr_t dma_buf, dma_ctrl_dat = 0;
	u8 thread_nr = chip_nr;
	int status;
	u16 ctype;

	if (dir == DMA_FROM_DEVICE)
		ctype = CDMA_CT_RD;
	else
		ctype = CDMA_CT_WR;

	cadence_nand_set_ecc_enable(cadence, with_ecc);

	dma_buf = dma_map_single(buf, buf_size, dir);
	if (dma_mapping_error(cadence->dev, dma_buf)) {
		dev_err(cadence->dev, "Failed to map DMA buffer\n");
		return -EIO;
	}

	if (ctrl_dat && ctrl_dat_size) {
		dma_ctrl_dat = dma_map_single(ctrl_dat,
					      ctrl_dat_size, dir);
		if (dma_mapping_error(cadence->dev, dma_ctrl_dat)) {
			dma_unmap_single(dma_buf,
					 buf_size, dir);
			dev_err(cadence->dev, "Failed to map DMA buffer\n");
			return -EIO;
		}
	}

	cadence_nand_cdma_desc_prepare(cadence, chip_nr, page,
				       dma_buf, dma_ctrl_dat, ctype);

	status = cadence_nand_cdma_send_and_wait(cadence, thread_nr);

	dma_unmap_single(dma_buf,
			 buf_size, dir);

	if (ctrl_dat && ctrl_dat_size)
		dma_unmap_single(dma_ctrl_dat,
				 ctrl_dat_size, dir);
	if (status)
		return status;

	return cadence_nand_cdma_finish(cadence);
}

static void cadence_nand_set_timings(struct cadence_nand_info *cadence,
				     struct cadence_nand_timings *t)
{
	writel_relaxed(t->async_toggle_timings,
		       cadence->reg + ASYNC_TOGGLE_TIMINGS);
	writel_relaxed(t->timings0, cadence->reg + TIMINGS0);
	writel_relaxed(t->timings1, cadence->reg + TIMINGS1);
	writel_relaxed(t->timings2, cadence->reg + TIMINGS2);

	if (cadence->caps2.is_phy_type_dll)
		writel_relaxed(t->dll_phy_ctrl, cadence->reg + DLL_PHY_CTRL);

	writel_relaxed(t->phy_ctrl, cadence->reg + PHY_CTRL);

	if (cadence->caps2.is_phy_type_dll) {
		writel_relaxed(0, cadence->reg + PHY_TSEL);
		writel_relaxed(2, cadence->reg + PHY_DQ_TIMING);
		writel_relaxed(t->phy_dqs_timing,
			       cadence->reg + PHY_DQS_TIMING);
		writel_relaxed(t->phy_gate_lpbk_ctrl,
			       cadence->reg + PHY_GATE_LPBK_CTRL);
		writel_relaxed(PHY_DLL_MASTER_CTRL_BYPASS_MODE,
			       cadence->reg + PHY_DLL_MASTER_CTRL);
		writel_relaxed(0, cadence->reg + PHY_DLL_SLAVE_CTRL);
	}
}

static int cadence_nand_select_target(struct nand_chip *chip)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);

	if (chip == cadence->selected_chip)
		return 0;

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	cadence_nand_set_timings(cadence, &cdns_chip->timings);

	cadence_nand_set_ecc_strength(cadence,
				      cdns_chip->corr_str_idx);

	cadence_nand_set_erase_detection(cadence, true,
					 chip->ecc.strength);

	cadence->curr_trans_type = -1;
	cadence->selected_chip = chip;

	return 0;
}

static int cadence_nand_erase(struct mtd_info *mtd, int page)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	int status;
	u8 thread_nr = cdns_chip->cs[chip->cur_cs];

	cadence_nand_cdma_desc_prepare(cadence,
				       cdns_chip->cs[chip->cur_cs],
				       page, 0, 0,
				       CDMA_CT_ERASE);
	status = cadence_nand_cdma_send_and_wait(cadence, thread_nr);
	if (status) {
		dev_err(cadence->dev, "erase operation failed\n");
		return -EIO;
	}

	status = cadence_nand_cdma_finish(cadence);
	if (status)
		return status;

	return 0;
}

static int cadence_ecc_setup(struct mtd_info *mtd, struct nand_chip *chip, int oobavail)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	int ret;

	/*
	 * If .size and .strength are already set (usually by DT),
	 * check if they are supported by this controller.
	 */
	if (chip->ecc.size && chip->ecc.strength)
		return nand_check_ecc_caps(chip, &cadence->ecc_caps, oobavail);

	/*
	 * We want .size and .strength closest to the chip's requirement
	 * unless NAND_ECC_MAXIMIZE is requested.
	 */
	if (!(chip->ecc.options & NAND_ECC_MAXIMIZE)) {
		ret = nand_match_ecc_req(chip, &cadence->ecc_caps, oobavail);
		if (!ret)
			return 0;
	}

	/* Max ECC strength is the last thing we can do */
	return nand_maximize_ecc(chip, &cadence->ecc_caps, oobavail);
}

static int cadence_nand_read_bbm(struct mtd_info *mtd, struct nand_chip *chip, int page, u8 *buf)
{
	int status;
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);

	cadence_nand_prepare_data_size(mtd, TT_BBM);

	cadence_nand_set_skip_bytes_conf(cadence, 0, 0, 0);

	/*
	 * Read only bad block marker from offset
	 * defined by a memory manufacturer.
	 */
	status = cadence_nand_cdma_transfer(cadence,
					    cdns_chip->cs[chip->cur_cs],
					    page, cadence->buf, NULL,
					    mtd->oobsize,
					    0, DMA_FROM_DEVICE, false);
	if (status) {
		dev_err(cadence->dev, "read BBM failed\n");
		return -EIO;
	}

	memcpy(buf + cdns_chip->bbm_offs, cadence->buf, cdns_chip->bbm_len);

	return 0;
}

static int cadence_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				   const u8 *buf, int oob_required, int page)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	int status;
	u16 marker_val = 0xFFFF;

	status = cadence_nand_select_target(chip);
	if (status)
		return status;

	cadence_nand_set_skip_bytes_conf(cadence, cdns_chip->bbm_len,
					 mtd->writesize
					 + cdns_chip->bbm_offs,
					 1);

	if (oob_required) {
		marker_val = *(u16 *)(chip->oob_poi
				      + cdns_chip->bbm_offs);
	} else {
		/* Set oob data to 0xFF. */
		memset(cadence->buf + mtd->writesize, 0xFF,
		       cdns_chip->avail_oob_size);
	}

	cadence_nand_set_skip_marker_val(cadence, marker_val);

	cadence_nand_prepare_data_size(mtd, TT_MAIN_OOB_AREA_EXT);

	if (cadence_nand_dma_buf_ok(cadence, buf, mtd->writesize) &&
	    cadence->caps2.data_control_supp && !(chip->options & NAND_USE_BOUNCE_BUFFER)) {
		u8 *oob;

		if (oob_required)
			oob = chip->oob_poi;
		else
			oob = cadence->buf + mtd->writesize;

		status = cadence_nand_cdma_transfer(cadence,
						    cdns_chip->cs[chip->cur_cs],
						    page, (void *)buf, oob,
						    mtd->writesize,
						    cdns_chip->avail_oob_size,
						    DMA_TO_DEVICE, true);
		if (status) {
			dev_err(cadence->dev, "write page failed\n");
			return -EIO;
		}

		return 0;
	}

	if (oob_required) {
		/* Transfer the data to the oob area. */
		memcpy(cadence->buf + mtd->writesize, chip->oob_poi,
		       cdns_chip->avail_oob_size);
	}

	memcpy(cadence->buf, buf, mtd->writesize);

	cadence_nand_prepare_data_size(mtd, TT_MAIN_OOB_AREAS);

	return cadence_nand_cdma_transfer(cadence,
					  cdns_chip->cs[chip->cur_cs],
					  page, cadence->buf, NULL,
					  mtd->writesize
					  + cdns_chip->avail_oob_size,
					  0, DMA_TO_DEVICE, true);
}

static int cadence_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
				  int page)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);

	memset(cadence->buf, 0xFF, mtd->writesize);

	return cadence_nand_write_page(mtd, chip, cadence->buf, 1, page);
}

static int cadence_nand_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				       const u8 *buf, int oob_required, int page)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	int writesize = mtd->writesize;
	int oobsize = mtd->oobsize;
	int ecc_steps = chip->ecc.steps;
	int ecc_size = chip->ecc.size;
	int ecc_bytes = chip->ecc.bytes;
	void *tmp_buf = cadence->buf;
	int oob_skip = cdns_chip->bbm_len;
	size_t size = writesize + oobsize;
	int i, pos, len;
	int status;

	status = cadence_nand_select_target(chip);
	if (status)
		return status;

	/*
	 * Fill the buffer with 0xff first except the full page transfer.
	 * This simplifies the logic.
	 */
	if (!buf || !oob_required)
		memset(tmp_buf, 0xff, size);

	cadence_nand_set_skip_bytes_conf(cadence, 0, 0, 0);

	/* Arrange the buffer for syndrome payload/ecc layout. */
	if (buf) {
		for (i = 0; i < ecc_steps; i++) {
			pos = i * (ecc_size + ecc_bytes);
			len = ecc_size;

			if (pos >= writesize)
				pos += oob_skip;
			else if (pos + len > writesize)
				len = writesize - pos;

			memcpy(tmp_buf + pos, buf, len);
			buf += len;
			if (len < ecc_size) {
				len = ecc_size - len;
				memcpy(tmp_buf + writesize + oob_skip, buf,
				       len);
				buf += len;
			}
		}
	}

	if (oob_required) {
		const u8 *oob = chip->oob_poi;
		u32 oob_data_offset = (cdns_chip->sector_count - 1) *
			(cdns_chip->sector_size + chip->ecc.bytes)
			+ cdns_chip->sector_size + oob_skip;

		/* BBM at the beginning of the OOB area. */
		memcpy(tmp_buf + writesize, oob, oob_skip);

		/* OOB free. */
		memcpy(tmp_buf + oob_data_offset, oob,
		       cdns_chip->avail_oob_size);
		oob += cdns_chip->avail_oob_size;

		/* OOB ECC. */
		for (i = 0; i < ecc_steps; i++) {
			pos = ecc_size + i * (ecc_size + ecc_bytes);
			if (i == (ecc_steps - 1))
				pos += cdns_chip->avail_oob_size;

			len = ecc_bytes;

			if (pos >= writesize)
				pos += oob_skip;
			else if (pos + len > writesize)
				len = writesize - pos;

			memcpy(tmp_buf + pos, oob, len);
			oob += len;
			if (len < ecc_bytes) {
				len = ecc_bytes - len;
				memcpy(tmp_buf + writesize + oob_skip, oob,
				       len);
				oob += len;
			}
		}
	}

	cadence_nand_prepare_data_size(mtd, TT_RAW_PAGE);

	return cadence_nand_cdma_transfer(cadence,
					  cdns_chip->cs[chip->cur_cs],
					  page, cadence->buf, NULL,
					  mtd->writesize +
					  mtd->oobsize,
					  0, DMA_TO_DEVICE, false);
}

static int cadence_nand_write_oob_raw(struct mtd_info *mtd, struct nand_chip *chip,
				      int page)
{
	return cadence_nand_write_page_raw(mtd, chip, NULL, true, page);
}

static int cadence_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				  u8 *buf, int oob_required, int page)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	int status;
	int ecc_err_count = 0;

	status = cadence_nand_select_target(chip);
	if (status)
		return status;

	cadence_nand_set_skip_bytes_conf(cadence, cdns_chip->bbm_len,
					 mtd->writesize
					 + cdns_chip->bbm_offs, 1);

	/*
	 * If data buffer can be accessed by DMA and data_control feature
	 * is supported then transfer data and oob directly.
	 */
	if (cadence_nand_dma_buf_ok(cadence, buf, mtd->writesize) &&
	    cadence->caps2.data_control_supp && !(chip->options & NAND_USE_BOUNCE_BUFFER)) {
		u8 *oob;

		if (oob_required)
			oob = chip->oob_poi;
		else
			oob = cadence->buf + mtd->writesize;

		cadence_nand_prepare_data_size(mtd, TT_MAIN_OOB_AREA_EXT);
		status = cadence_nand_cdma_transfer(cadence,
						    cdns_chip->cs[chip->cur_cs],
						    page, buf, oob,
						    mtd->writesize,
						    cdns_chip->avail_oob_size,
						    DMA_FROM_DEVICE, true);
	/* Otherwise use bounce buffer. */
	} else {
		cadence_nand_prepare_data_size(mtd, TT_MAIN_OOB_AREAS);
		status = cadence_nand_cdma_transfer(cadence,
						    cdns_chip->cs[chip->cur_cs],
						    page, cadence->buf,
						    NULL, mtd->writesize
						    + cdns_chip->avail_oob_size,
						    0, DMA_FROM_DEVICE, true);

		memcpy(buf, cadence->buf, mtd->writesize);
		if (oob_required)
			memcpy(chip->oob_poi,
			       cadence->buf + mtd->writesize,
			       mtd->oobsize);
	}

	switch (status) {
	case STAT_ECC_UNCORR:
		mtd->ecc_stats.failed++;
		ecc_err_count++;
		break;
	case STAT_ECC_CORR:
		ecc_err_count = FIELD_GET(CDMA_CS_MAXERR,
					  cadence->cdma_desc->status);
		mtd->ecc_stats.corrected += ecc_err_count;
		break;
	case STAT_ERASED:
	case STAT_OK:
		break;
	default:
		dev_err(cadence->dev, "read page failed\n");
		return -EIO;
	}

	if (oob_required)
		if (cadence_nand_read_bbm(mtd, chip, page, chip->oob_poi))
			return -EIO;

	return ecc_err_count;
}

static int cadence_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
				 int page)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);

	return cadence_nand_read_page(mtd, chip, cadence->buf, 1, page);
}

static int cadence_nand_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				      u8 *buf, int oob_required, int page)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	int oob_skip = cdns_chip->bbm_len;
	int writesize = mtd->writesize;
	int ecc_steps = chip->ecc.steps;
	int ecc_size = chip->ecc.size;
	int ecc_bytes = chip->ecc.bytes;
	void *tmp_buf = cadence->buf;
	int i, pos, len;
	int status;

	status = cadence_nand_select_target(chip);
	if (status)
		return status;

	cadence_nand_set_skip_bytes_conf(cadence, 0, 0, 0);

	cadence_nand_prepare_data_size(mtd, TT_RAW_PAGE);
	status = cadence_nand_cdma_transfer(cadence,
					    cdns_chip->cs[chip->cur_cs],
					    page, cadence->buf, NULL,
					    mtd->writesize
					    + mtd->oobsize,
					    0, DMA_FROM_DEVICE, false);

	switch (status) {
	case STAT_ERASED:
	case STAT_OK:
		break;
	default:
		dev_err(cadence->dev, "read raw page failed\n");
		return -EIO;
	}

	/* Arrange the buffer for syndrome payload/ecc layout. */
	if (buf) {
		for (i = 0; i < ecc_steps; i++) {
			pos = i * (ecc_size + ecc_bytes);
			len = ecc_size;

			if (pos >= writesize)
				pos += oob_skip;
			else if (pos + len > writesize)
				len = writesize - pos;

			memcpy(buf, tmp_buf + pos, len);
			buf += len;
			if (len < ecc_size) {
				len = ecc_size - len;
				memcpy(buf, tmp_buf + writesize + oob_skip,
				       len);
				buf += len;
			}
		}
	}

	if (oob_required) {
		u8 *oob = chip->oob_poi;
		u32 oob_data_offset = (cdns_chip->sector_count - 1) *
			(cdns_chip->sector_size + chip->ecc.bytes)
			+ cdns_chip->sector_size + oob_skip;

		/* OOB free. */
		memcpy(oob, tmp_buf + oob_data_offset,
		       cdns_chip->avail_oob_size);

		/* BBM at the beginning of the OOB area. */
		memcpy(oob, tmp_buf + writesize, oob_skip);

		oob += cdns_chip->avail_oob_size;

		/* OOB ECC */
		for (i = 0; i < ecc_steps; i++) {
			pos = ecc_size + i * (ecc_size + ecc_bytes);
			len = ecc_bytes;

			if (i == (ecc_steps - 1))
				pos += cdns_chip->avail_oob_size;

			if (pos >= writesize)
				pos += oob_skip;
			else if (pos + len > writesize)
				len = writesize - pos;

			memcpy(oob, tmp_buf + pos, len);
			oob += len;
			if (len < ecc_bytes) {
				len = ecc_bytes - len;
				memcpy(oob, tmp_buf + writesize + oob_skip,
				       len);
				oob += len;
			}
		}
	}
	return 0;
}

static int cadence_nand_read_oob_raw(struct mtd_info *mtd, struct nand_chip *chip,
				     int page)
{
	return cadence_nand_read_page_raw(mtd, chip, NULL, true, page);
}

static void cadence_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	u8 thread_nr = 0;
	u32 sdma_size;
	int status;
	int len_in_words = len >> 2;

	/* Wait until slave DMA interface is ready to data transfer. */
	status = cadence_nand_wait_on_sdma(cadence, &thread_nr, &sdma_size);
	if (status) {
		pr_err("Wait on sdma failed:%x\n", status);
		hang();
	}

	if (!cadence->caps1->has_dma) {
		readsq(cadence->io.virt, buf, len_in_words);

		if (sdma_size > len) {
			memcpy(cadence->buf, buf + (len_in_words << 2),
			       len - (len_in_words << 2));
			readsl(cadence->io.virt, cadence->buf,
			       sdma_size / 4 - len_in_words);
		}
	}
}

static void cadence_nand_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	u8 thread_nr = 0;
	u32 sdma_size;
	int status;
	int len_in_words = len >> 2;

	/* Wait until slave DMA interface is ready to data transfer. */
	status = cadence_nand_wait_on_sdma(cadence, &thread_nr, &sdma_size);
	if (status) {
		pr_err("Wait on sdma failed:%x\n", status);
		hang();
	}

	if (!cadence->caps1->has_dma) {
		writesq(cadence->io.virt, buf, len_in_words);

		if (sdma_size > len) {
			memcpy(cadence->buf, buf + (len_in_words << 2),
			       len - (len_in_words << 2));
			writesl(cadence->io.virt, cadence->buf,
				sdma_size / 4 - len_in_words);
		}
	}
}

static int cadence_nand_cmd_opcode(struct nand_chip *chip, unsigned int op_id)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	u64 mini_ctrl_cmd = 0;
	int ret;

	mini_ctrl_cmd |= GCMD_LAY_TWB;
	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAY_INSTR, GCMD_LAY_INSTR_CMD);
	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAY_INPUT_CMD, op_id);

	ret = cadence_nand_generic_cmd_send(cadence,
					    cdns_chip->cs[chip->cur_cs],
					    mini_ctrl_cmd);

	if (ret)
		dev_err(cadence->dev, "send cmd %x failed\n",
			op_id);

	return ret;
}

static int cadence_nand_cmd_address(struct nand_chip *chip,
				    unsigned int naddrs, const u8 *addrs)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	u64 address = 0;
	u64 mini_ctrl_cmd = 0;
	int ret;
	int i;

	mini_ctrl_cmd |= GCMD_LAY_TWB;

	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAY_INSTR,
				    GCMD_LAY_INSTR_ADDR);

	for (i = 0; i < naddrs; i++)
		address |= (u64)addrs[i] << (8 * i);

	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAY_INPUT_ADDR,
				    address);
	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAY_INPUT_ADDR_SIZE,
				    naddrs - 1);

	ret = cadence_nand_generic_cmd_send(cadence,
					    cdns_chip->cs[chip->cur_cs],
					    mini_ctrl_cmd);

	if (ret)
		pr_err("send address %llx failed\n", address);

	return ret;
}

static int cadence_nand_cmd_data(struct nand_chip *chip,
				 unsigned int len, u8 mode)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	u64 mini_ctrl_cmd = 0;
	int ret;

	mini_ctrl_cmd |= GCMD_LAY_TWB;
	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAY_INSTR,
				    GCMD_LAY_INSTR_DATA);

	if (mode)
		mini_ctrl_cmd |= FIELD_PREP(GCMD_DIR, GCMD_DIR_WRITE);

	mini_ctrl_cmd |= FIELD_PREP(GCMD_SECT_CNT, 1);
	mini_ctrl_cmd |= FIELD_PREP(GCMD_LAST_SIZE, len);

	ret = cadence_nand_generic_cmd_send(cadence,
					    cdns_chip->cs[chip->cur_cs],
					    mini_ctrl_cmd);

	if (ret) {
		pr_err("send generic data cmd failed\n");
		return ret;
	}

	return ret;
}

static int cadence_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	int status;

	status = cadence_nand_wait_for_value(cadence, RBN_SETINGS,
					     TIMEOUT_US,
					     BIT(cdns_chip->cs[chip->cur_cs]),
					     false);
	return status;
}

static int cadence_nand_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);

	if (section)
		return -ERANGE;

	oobregion->offset = cdns_chip->bbm_len;
	oobregion->length = cdns_chip->avail_oob_size
		- cdns_chip->bbm_len;

	return 0;
}

static int cadence_nand_ooblayout_ecc(struct mtd_info *mtd, int section,
				      struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);

	if (section)
		return -ERANGE;

	oobregion->offset = cdns_chip->avail_oob_size;
	oobregion->length = chip->ecc.total;

	return 0;
}

static const struct mtd_ooblayout_ops cadence_nand_ooblayout_ops = {
	.rfree = cadence_nand_ooblayout_free,
	.ecc = cadence_nand_ooblayout_ecc,
};

static int calc_cycl(u32 timing, u32 clock)
{
	if (timing == 0 || clock == 0)
		return 0;

	if ((timing % clock) > 0)
		return timing / clock;
	else
		return timing / clock - 1;
}

/* Calculate max data valid window. */
static inline u32 calc_tdvw_max(u32 trp_cnt, u32 clk_period, u32 trhoh_min,
				u32 board_delay_skew_min, u32 ext_mode)
{
	if (ext_mode == 0)
		clk_period /= 2;

	return (trp_cnt + 1) * clk_period + trhoh_min +
		board_delay_skew_min;
}

/* Calculate data valid window. */
static inline u32 calc_tdvw(u32 trp_cnt, u32 clk_period, u32 trhoh_min,
			    u32 trea_max, u32 ext_mode)
{
	if (ext_mode == 0)
		clk_period /= 2;

	return (trp_cnt + 1) * clk_period + trhoh_min - trea_max;
}

static inline int of_get_child_count(const ofnode node)
{
	return fdtdec_get_child_count(gd->fdt_blob, ofnode_to_offset(node));
}

static int cadence_setup_data_interface(struct mtd_info *mtd, int chipnr,
					const struct nand_data_interface *conf)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(mtd_to_nand(mtd));
	const struct nand_sdr_timings *sdr;
	struct cadence_nand_timings *t = &cdns_chip->timings;
	u32 reg;
	u32 board_delay = cadence->board_delay;
	u32 clk_period = DIV_ROUND_DOWN_ULL(1000000000000ULL,
					    cadence->nf_clk_rate);
	u32 tceh_cnt, tcs_cnt, tadl_cnt, tccs_cnt;
	u32 tfeat_cnt, trhz_cnt, tvdly_cnt;
	u32 trhw_cnt, twb_cnt, twh_cnt = 0, twhr_cnt;
	u32 twp_cnt = 0, trp_cnt = 0, trh_cnt = 0;
	u32 if_skew = cadence->caps1->if_skew;
	u32 board_delay_skew_min = board_delay - if_skew;
	u32 board_delay_skew_max = board_delay + if_skew;
	u32 dqs_sampl_res, phony_dqs_mod;
	u32 tdvw, tdvw_min, tdvw_max;
	u32 ext_rd_mode, ext_wr_mode;
	u32 dll_phy_dqs_timing = 0, phony_dqs_timing = 0, rd_del_sel = 0;
	u32 sampling_point;

	sdr = nand_get_sdr_timings(conf);
	if (IS_ERR(sdr))
		return PTR_ERR(sdr);

	memset(t, 0, sizeof(*t));
	/* Sampling point calculation. */
	if (cadence->caps2.is_phy_type_dll)
		phony_dqs_mod = 2;
	else
		phony_dqs_mod = 1;

	dqs_sampl_res = clk_period / phony_dqs_mod;

	tdvw_min = sdr->tREA_max + board_delay_skew_max;
	/*
	 * The idea of those calculation is to get the optimum value
	 * for tRP and tRH timings. If it is NOT possible to sample data
	 * with optimal tRP/tRH settings, the parameters will be extended.
	 * If clk_period is 50ns (the lowest value) this condition is met
	 * for SDR timing modes 1, 2, 3, 4 and 5.
	 * If clk_period is 20ns the condition is met only for SDR timing
	 * mode 5.
	 */
	if (sdr->tRC_min <= clk_period &&
	    sdr->tRP_min <= (clk_period / 2) &&
	    sdr->tREH_min <= (clk_period / 2)) {
		/* Performance mode. */
		ext_rd_mode = 0;
		tdvw = calc_tdvw(trp_cnt, clk_period, sdr->tRHOH_min,
				 sdr->tREA_max, ext_rd_mode);
		tdvw_max = calc_tdvw_max(trp_cnt, clk_period, sdr->tRHOH_min,
					 board_delay_skew_min,
					 ext_rd_mode);
		/*
		 * Check if data valid window and sampling point can be found
		 * and is not on the edge (ie. we have hold margin).
		 * If not extend the tRP timings.
		 */
		if (tdvw > 0) {
			if (tdvw_max <= tdvw_min ||
			    (tdvw_max % dqs_sampl_res) == 0) {
				/*
				 * No valid sampling point so the RE pulse need
				 * to be widen widening by half clock cycle.
				 */
				ext_rd_mode = 1;
			}
		} else {
			/*
			 * There is no valid window
			 * to be able to sample data the tRP need to be widen.
			 * Very safe calculations are performed here.
			 */
			trp_cnt = (sdr->tREA_max + board_delay_skew_max
				   + dqs_sampl_res) / clk_period;
			ext_rd_mode = 1;
		}

	} else {
		/* Extended read mode. */
		u32 trh;

		ext_rd_mode = 1;
		trp_cnt = calc_cycl(sdr->tRP_min, clk_period);
		trh = sdr->tRC_min - ((trp_cnt + 1) * clk_period);
		if (sdr->tREH_min >= trh)
			trh_cnt = calc_cycl(sdr->tREH_min, clk_period);
		else
			trh_cnt = calc_cycl(trh, clk_period);

		tdvw = calc_tdvw(trp_cnt, clk_period, sdr->tRHOH_min,
				 sdr->tREA_max, ext_rd_mode);
		/*
		 * Check if data valid window and sampling point can be found
		 * or if it is at the edge check if previous is valid
		 * - if not extend the tRP timings.
		 */
		if (tdvw > 0) {
			tdvw_max = calc_tdvw_max(trp_cnt, clk_period,
						 sdr->tRHOH_min,
						 board_delay_skew_min,
						 ext_rd_mode);

			if ((((tdvw_max / dqs_sampl_res)
			      * dqs_sampl_res) <= tdvw_min) ||
			    (((tdvw_max % dqs_sampl_res) == 0) &&
			     (((tdvw_max / dqs_sampl_res - 1)
			       * dqs_sampl_res) <= tdvw_min))) {
				/*
				 * Data valid window width is lower than
				 * sampling resolution and do not hit any
				 * sampling point to be sure the sampling point
				 * will be found the RE low pulse width will be
				 *  extended by one clock cycle.
				 */
				trp_cnt = trp_cnt + 1;
			}
		} else {
			/*
			 * There is no valid window to be able to sample data.
			 * The tRP need to be widen.
			 * Very safe calculations are performed here.
			 */
			trp_cnt = (sdr->tREA_max + board_delay_skew_max
				   + dqs_sampl_res) / clk_period;
		}
	}

	tdvw_max = calc_tdvw_max(trp_cnt, clk_period,
				 sdr->tRHOH_min,
				 board_delay_skew_min, ext_rd_mode);

	if (sdr->tWC_min <= clk_period &&
	    (sdr->tWP_min + if_skew) <= (clk_period / 2) &&
	    (sdr->tWH_min + if_skew) <= (clk_period / 2)) {
		ext_wr_mode = 0;
	} else {
		u32 twh;

		ext_wr_mode = 1;
		twp_cnt = calc_cycl(sdr->tWP_min + if_skew, clk_period);
		if ((twp_cnt + 1) * clk_period < (sdr->tALS_min + if_skew))
			twp_cnt = calc_cycl(sdr->tALS_min + if_skew,
					    clk_period);

		twh = (sdr->tWC_min - (twp_cnt + 1) * clk_period);
		if (sdr->tWH_min >= twh)
			twh = sdr->tWH_min;

		twh_cnt = calc_cycl(twh + if_skew, clk_period);
	}

	reg = FIELD_PREP(ASYNC_TOGGLE_TIMINGS_TRH, trh_cnt);
	reg |= FIELD_PREP(ASYNC_TOGGLE_TIMINGS_TRP, trp_cnt);
	reg |= FIELD_PREP(ASYNC_TOGGLE_TIMINGS_TWH, twh_cnt);
	reg |= FIELD_PREP(ASYNC_TOGGLE_TIMINGS_TWP, twp_cnt);
	t->async_toggle_timings = reg;
	dev_dbg(cadence->dev, "ASYNC_TOGGLE_TIMINGS_SDR\t%x\n", reg);

	tadl_cnt = calc_cycl((sdr->tADL_min + if_skew), clk_period);
	tccs_cnt = calc_cycl((sdr->tCCS_min + if_skew), clk_period);
	twhr_cnt = calc_cycl((sdr->tWHR_min + if_skew), clk_period);
	trhw_cnt = calc_cycl((sdr->tRHW_min + if_skew), clk_period);
	reg = FIELD_PREP(TIMINGS0_TADL, tadl_cnt);

	/*
	 * If timing exceeds delay field in timing register
	 * then use maximum value.
	 */
	if (FIELD_FIT(TIMINGS0_TCCS, tccs_cnt))
		reg |= FIELD_PREP(TIMINGS0_TCCS, tccs_cnt);
	else
		reg |= TIMINGS0_TCCS;

	reg |= FIELD_PREP(TIMINGS0_TWHR, twhr_cnt);
	reg |= FIELD_PREP(TIMINGS0_TRHW, trhw_cnt);
	t->timings0 = reg;
	dev_dbg(cadence->dev, "TIMINGS0_SDR\t%x\n", reg);

	/* The following is related to single signal so skew is not needed. */
	trhz_cnt = calc_cycl(sdr->tRHZ_max, clk_period);
	trhz_cnt = trhz_cnt + 1;
	twb_cnt = calc_cycl((sdr->tWB_max + board_delay), clk_period);
	/*
	 * Because of the two stage syncflop the value must be increased by 3
	 * first value is related with sync, second value is related
	 * with output if delay.
	 */
	twb_cnt = twb_cnt + 3 + 5;
	/*
	 * The following is related to the we edge of the random data input
	 * sequence so skew is not needed.
	 */
	tvdly_cnt = calc_cycl(500000 + if_skew, clk_period);
	reg = FIELD_PREP(TIMINGS1_TRHZ, trhz_cnt);
	reg |= FIELD_PREP(TIMINGS1_TWB, twb_cnt);
	reg |= FIELD_PREP(TIMINGS1_TVDLY, tvdly_cnt);
	t->timings1 = reg;
	dev_dbg(cadence->dev, "TIMINGS1_SDR\t%x\n", reg);

	tfeat_cnt = calc_cycl(sdr->tFEAT_max, clk_period);
	if (tfeat_cnt < twb_cnt)
		tfeat_cnt = twb_cnt;

	tceh_cnt = calc_cycl(sdr->tCEH_min, clk_period);
	tcs_cnt = calc_cycl((sdr->tCS_min + if_skew), clk_period);

	reg = FIELD_PREP(TIMINGS2_TFEAT, tfeat_cnt);
	reg |= FIELD_PREP(TIMINGS2_CS_HOLD_TIME, tceh_cnt);
	reg |= FIELD_PREP(TIMINGS2_CS_SETUP_TIME, tcs_cnt);
	t->timings2 = reg;
	dev_dbg(cadence->dev, "TIMINGS2_SDR\t%x\n", reg);

	if (cadence->caps2.is_phy_type_dll) {
		reg = DLL_PHY_CTRL_DLL_RST_N;
		if (ext_wr_mode)
			reg |= DLL_PHY_CTRL_EXTENDED_WR_MODE;
		if (ext_rd_mode)
			reg |= DLL_PHY_CTRL_EXTENDED_RD_MODE;

		reg |= FIELD_PREP(DLL_PHY_CTRL_RS_HIGH_WAIT_CNT, 7);
		reg |= FIELD_PREP(DLL_PHY_CTRL_RS_IDLE_CNT, 7);
		t->dll_phy_ctrl = reg;
		dev_dbg(cadence->dev, "DLL_PHY_CTRL_SDR\t%x\n", reg);
	}

	/* Sampling point calculation. */
	if ((tdvw_max % dqs_sampl_res) > 0)
		sampling_point = tdvw_max / dqs_sampl_res;
	else
		sampling_point = (tdvw_max / dqs_sampl_res - 1);

	if (sampling_point * dqs_sampl_res > tdvw_min) {
		dll_phy_dqs_timing =
			FIELD_PREP(PHY_DQS_TIMING_DQS_SEL_OE_END, 4);
		dll_phy_dqs_timing |= PHY_DQS_TIMING_USE_PHONY_DQS;
		phony_dqs_timing = sampling_point / phony_dqs_mod;

		if ((sampling_point % 2) > 0) {
			dll_phy_dqs_timing |= PHY_DQS_TIMING_PHONY_DQS_SEL;
			if ((tdvw_max % dqs_sampl_res) == 0)
				/*
				 * Calculation for sampling point at the edge
				 * of data and being odd number.
				 */
				phony_dqs_timing = (tdvw_max / dqs_sampl_res)
					/ phony_dqs_mod - 1;

			if (!cadence->caps2.is_phy_type_dll)
				phony_dqs_timing--;

		} else {
			phony_dqs_timing--;
		}
		rd_del_sel = phony_dqs_timing + 3;
	} else {
		dev_warn(cadence->dev,
			 "ERROR : cannot find valid sampling point\n");
	}

	reg = FIELD_PREP(PHY_CTRL_PHONY_DQS, phony_dqs_timing);
	if (cadence->caps2.is_phy_type_dll)
		reg  |= PHY_CTRL_SDR_DQS;
	t->phy_ctrl = reg;
	dev_dbg(cadence->dev, "PHY_CTRL_REG_SDR\t%x\n", reg);

	if (cadence->caps2.is_phy_type_dll) {
		dev_dbg(cadence->dev, "PHY_TSEL_REG_SDR\t%x\n", 0);
		dev_dbg(cadence->dev, "PHY_DQ_TIMING_REG_SDR\t%x\n", 2);
		dev_dbg(cadence->dev, "PHY_DQS_TIMING_REG_SDR\t%x\n",
			dll_phy_dqs_timing);
		t->phy_dqs_timing = dll_phy_dqs_timing;

		reg = FIELD_PREP(PHY_GATE_LPBK_CTRL_RDS, rd_del_sel);
		dev_dbg(cadence->dev, "PHY_GATE_LPBK_CTRL_REG_SDR\t%x\n",
			reg);
		t->phy_gate_lpbk_ctrl = reg;

		dev_dbg(cadence->dev, "PHY_DLL_MASTER_CTRL_REG_SDR\t%lx\n",
			PHY_DLL_MASTER_CTRL_BYPASS_MODE);
		dev_dbg(cadence->dev, "PHY_DLL_SLAVE_CTRL_REG_SDR\t%x\n", 0);
	}
	return 0;
}

static int cadence_nand_attach_chip(struct nand_chip *chip)
{
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	struct cdns_nand_chip *cdns_chip = to_cdns_nand_chip(chip);
	static struct nand_ecclayout nand_oob;
	u32 ecc_size;
	struct mtd_info *mtd = nand_to_mtd(chip);
	int ret;

	if (chip->options & NAND_BUSWIDTH_16) {
		ret = cadence_nand_set_access_width16(cadence, true);
		if (ret)
			return ret;
	}

	chip->options |= NAND_USE_BOUNCE_BUFFER;
	chip->bbt_options |= NAND_BBT_USE_FLASH;
	chip->bbt_options |= NAND_BBT_NO_OOB;
	chip->ecc.mode = NAND_ECC_HW_SYNDROME;

	chip->options |= NAND_NO_SUBPAGE_WRITE;

	cdns_chip->bbm_offs = chip->badblockpos;
	cdns_chip->bbm_offs &= ~0x01;
	/* this value should be even number */
	cdns_chip->bbm_len = 2;

	ret = cadence_ecc_setup(mtd, chip, mtd->oobsize - cdns_chip->bbm_len);
	if (ret) {
		dev_err(cadence->dev, "ECC configuration failed\n");
		return ret;
	}

	dev_dbg(cadence->dev,
		"chosen ECC settings: step=%d, strength=%d, bytes=%d\n",
		chip->ecc.size, chip->ecc.strength, chip->ecc.bytes);

	/* Error correction configuration. */
	cdns_chip->sector_size = chip->ecc.size;
	cdns_chip->sector_count = mtd->writesize / cdns_chip->sector_size;
	ecc_size = cdns_chip->sector_count * chip->ecc.bytes;

	cdns_chip->avail_oob_size = mtd->oobsize - ecc_size;

	if (cdns_chip->avail_oob_size > cadence->bch_metadata_size)
		cdns_chip->avail_oob_size = cadence->bch_metadata_size;

	if ((cdns_chip->avail_oob_size + cdns_chip->bbm_len + ecc_size)
	    > mtd->oobsize)
		cdns_chip->avail_oob_size -= 4;

	ret = cadence_nand_get_ecc_strength_idx(cadence, chip->ecc.strength);
	if (ret < 0)
		return -EINVAL;

	cdns_chip->corr_str_idx = (u8)ret;

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	cadence_nand_set_ecc_strength(cadence,
				      cdns_chip->corr_str_idx);

	cadence_nand_set_erase_detection(cadence, true,
					 chip->ecc.strength);

	dev_dbg(cadence->dev,
		"chosen ECC settings: step=%d, strength=%d, bytes=%d\n",
		chip->ecc.size, chip->ecc.strength, chip->ecc.bytes);

	/* Override the default read operations. */
	chip->ecc.options |= NAND_ECC_CUSTOM_PAGE_ACCESS;
	chip->ecc.read_page = cadence_nand_read_page;
	chip->ecc.read_page_raw = cadence_nand_read_page_raw;
	chip->ecc.write_page = cadence_nand_write_page;
	chip->ecc.write_page_raw = cadence_nand_write_page_raw;
	chip->ecc.read_oob = cadence_nand_read_oob;
	chip->ecc.write_oob = cadence_nand_write_oob;
	chip->ecc.read_oob_raw = cadence_nand_read_oob_raw;
	chip->ecc.write_oob_raw = cadence_nand_write_oob_raw;
	chip->erase = cadence_nand_erase;

	if ((mtd->writesize + mtd->oobsize) > cadence->buf_size)
		cadence->buf_size = mtd->writesize + mtd->oobsize;

	mtd_set_ooblayout(mtd, &cadence_nand_ooblayout_ops);

	nand_oob.eccbytes = cdns_chip->chip.ecc.bytes;
	cdns_chip->chip.ecc.layout = &nand_oob;

	return 0;
}

/* Dummy implementation: we don't support multiple chips */
static void cadence_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	switch (chipnr) {
	case -1:
	case 0:
		break;

	default:
		WARN_ON(chipnr);
	}
}

static int cadence_nand_status(struct mtd_info *mtd, unsigned int command)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	int ret = 0;

	ret = cadence_nand_cmd_opcode(chip, command);
	if (ret)
		return ret;

	ret = cadence_nand_cmd_data(chip, 1, GCMD_DIR_READ);
	if (ret)
		return ret;

	return 0;
}

static int cadence_nand_readid(struct mtd_info *mtd, int offset_in_page, unsigned int command)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	u8 addrs = (u8)offset_in_page;
	int ret = 0;

	ret = cadence_nand_cmd_opcode(chip, command);
	if (ret)
		return ret;

	ret = cadence_nand_cmd_address(chip, ONE_CYCLE, &addrs);
	if (ret)
		return ret;

	ret = cadence_nand_cmd_data(chip, 8, GCMD_DIR_READ);
	if (ret)
		return ret;

	return 0;
}

static int cadence_nand_param(struct mtd_info *mtd, u8 offset_in_page, unsigned int command)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	int ret = 0;

	ret = cadence_nand_cmd_opcode(chip, command);
	if (ret)
		return ret;

	ret = cadence_nand_cmd_address(chip, ONE_CYCLE, &offset_in_page);
	if (ret)
		return ret;

	ret = cadence_nand_waitfunc(mtd, chip);
	if (ret)
		return ret;

	ret = cadence_nand_cmd_data(chip, sizeof(struct nand_jedec_params), GCMD_DIR_READ);
	if (ret)
		return ret;

	return 0;
}

static int cadence_nand_reset(struct mtd_info *mtd, unsigned int command)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	int ret = 0;

	ret = cadence_nand_cmd_opcode(chip, command);
	if (ret)
		return ret;

	ret = cadence_nand_waitfunc(mtd, chip);
	if (ret)
		return ret;

	return 0;
}

static int cadence_nand_features(struct mtd_info *mtd, u8 offset_in_page, u32 command)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	int ret = 0;

	ret = cadence_nand_cmd_opcode(chip, command);
	if (ret)
		return ret;

	ret = cadence_nand_cmd_address(chip, ONE_CYCLE, &offset_in_page);
	if (ret)
		return ret;

	if (command == NAND_CMD_GET_FEATURES)
		ret = cadence_nand_cmd_data(chip, ONFI_SUBFEATURE_PARAM_LEN,
					    GCMD_DIR_READ);
	else
		ret = cadence_nand_cmd_data(chip, ONFI_SUBFEATURE_PARAM_LEN,
					    GCMD_DIR_WRITE);

	return ret;
}

static void cadence_nand_cmdfunc(struct mtd_info *mtd, unsigned int command,
				 int offset_in_page, int page)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	int ret = 0;

	cadence->cmd = command;
	switch (command) {
	case NAND_CMD_STATUS:
		ret = cadence_nand_status(mtd, command);
		break;

	case NAND_CMD_READID:
		ret = cadence_nand_readid(mtd, offset_in_page, command);
		break;

	case NAND_CMD_PARAM:
		ret = cadence_nand_param(mtd, offset_in_page, command);
		break;

	case NAND_CMD_RESET:
		ret = cadence_nand_reset(mtd, command);
		break;

	case NAND_CMD_SET_FEATURES:
	case NAND_CMD_GET_FEATURES:
		ret = cadence_nand_features(mtd, offset_in_page, command);
		break;
	/*
	 * ecc will override other command for read, write and erase
	 */
	default:
		break;
	}

	if (cadence->cmd == NAND_CMD_RESET) {
		ret = cadence_nand_select_target(chip);
		if (ret)
			dev_err(cadence->dev, "Chip select failure after reset\n");
	}

	if (ret != 0)
		printf("ERROR:%s:command:0x%x\n", __func__, cadence->cmd);
}

static int cadence_nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);

	if (cadence_nand_wait_for_value(cadence, CTRL_STATUS,
					TIMEOUT_US,
					CTRL_STATUS_CTRL_BUSY, true))
		return -ETIMEDOUT;

	return 0;
}

static u8 cadence_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct cadence_nand_info *cadence = to_cadence_nand_info(chip->controller);
	u32 size = 1;
	u8 val;

	if (cadence->buf_index == 0) {
		if (cadence->cmd == NAND_CMD_READID)
			size = 8;
		else if (cadence->cmd == NAND_CMD_PARAM)
			size = sizeof(struct nand_jedec_params);
		else if (cadence->cmd == NAND_CMD_GET_FEATURES)
			size = ONFI_SUBFEATURE_PARAM_LEN;

		cadence_nand_read_buf(mtd, &cadence->buf[0], size);
	}

	val = *(&cadence->buf[0] + cadence->buf_index);
	cadence->buf_index++;

	return val;
}

static void cadence_nand_write_byte(struct mtd_info *mtd, u8 byte)
{
	cadence_nand_write_buf(mtd, &byte, 1);
}

static int cadence_nand_chip_init(struct cadence_nand_info *cadence, ofnode node)
{
	struct cdns_nand_chip *cdns_chip;
	struct nand_chip *chip;
	struct mtd_info *mtd;
	int ret, i;
	int nsels;
	u32 cs;

	if (!ofnode_get_property(node, "reg", &nsels))
		return -ENODEV;

	nsels /= sizeof(u32);
	if (nsels <= 0) {
		dev_err(cadence->dev, "invalid reg property size %d\n", nsels);
		return -EINVAL;
	}

	cdns_chip = devm_kzalloc(cadence->dev, sizeof(*cdns_chip) +
				 (nsels * sizeof(u8)), GFP_KERNEL);
	if (!cdns_chip)
		return -ENODEV;

	cdns_chip->nsels = nsels;
	for (i = 0; i < nsels; i++) {
		/* Retrieve CS id. */
		ret = ofnode_read_u32_index(node, "reg", i, &cs);
		if (ret) {
			dev_err(cadence->dev,
				"could not retrieve reg property: %d\n",
				ret);
			goto free_buf;
		}

		if (cs >= cadence->caps2.max_banks) {
			dev_err(cadence->dev,
				"invalid reg value: %u (max CS = %d)\n",
				cs, cadence->caps2.max_banks);
			ret = -EINVAL;
			goto free_buf;
		}

		if (test_and_set_bit(cs, &cadence->assigned_cs)) {
			dev_err(cadence->dev,
				"CS %d already assigned\n", cs);
			ret = -EINVAL;
			goto free_buf;
		}

		cdns_chip->cs[i] = cs;
	}

	chip = &cdns_chip->chip;
	chip->controller = &cadence->controller;
	nand_set_flash_node(chip, node);
	mtd = nand_to_mtd(chip);
	mtd->dev = cadence->dev;

	chip->options |= NAND_BUSWIDTH_AUTO;
	chip->select_chip = cadence_nand_select_chip;
	chip->cmdfunc = cadence_nand_cmdfunc;
	chip->dev_ready = cadence_nand_dev_ready;
	chip->read_byte = cadence_nand_read_byte;
	chip->write_byte = cadence_nand_write_byte;
	chip->waitfunc = cadence_nand_waitfunc;
	chip->read_buf = cadence_nand_read_buf;
	chip->write_buf = cadence_nand_write_buf;
	chip->setup_data_interface = cadence_setup_data_interface;

	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret) {
		dev_err(cadence->dev, "Chip identification failure\n");
		goto free_buf;
	}

	ret = cadence_nand_attach_chip(chip);
	if (ret) {
		dev_err(cadence->dev, "Chip not able to attached\n");
		goto free_buf;
	}

	ret = nand_scan_tail(mtd);
	if (ret) {
		dev_err(cadence->dev, "could not scan the nand chip\n");
		goto free_buf;
	}

	ret = nand_register(0, mtd);
	if (ret) {
		dev_err(cadence->dev, "Failed to register MTD: %d\n", ret);
		goto free_buf;
	}

	return 0;

free_buf:
	devm_kfree(cadence->dev, cdns_chip);
	return ret;
}

static int cadence_nand_chips_init(struct cadence_nand_info *cadence)
{
	struct udevice *dev = cadence->dev;
	ofnode node = dev_ofnode(dev);
	ofnode nand_node;
	int max_cs = cadence->caps2.max_banks;
	int nchips, ret;

	nchips = of_get_child_count(node);

	if (nchips > max_cs) {
		dev_err(cadence->dev,
			"too many NAND chips: %d (max = %d CS)\n",
			nchips, max_cs);
		return -EINVAL;
	}

	ofnode_for_each_subnode(nand_node, node) {
		ret = cadence_nand_chip_init(cadence, nand_node);
		if (ret)
			return ret;
	}

	return 0;
}

static int cadence_nand_init(struct cadence_nand_info *cadence)
{
	int ret;

	cadence->cdma_desc = dma_alloc_coherent(sizeof(*cadence->cdma_desc),
						(unsigned long *)&cadence->dma_cdma_desc);
	if (!cadence->cdma_desc)
		return -ENOMEM;

	cadence->buf_size = SZ_16K;
	cadence->buf = kmalloc(cadence->buf_size, GFP_KERNEL);
	if (!cadence->buf) {
		ret = -ENOMEM;
		goto free_buf_desc;
	}

	//Hardware initialization
	ret = cadence_nand_hw_init(cadence);
	if (ret)
		goto free_buf;

	cadence->curr_corr_str_idx = 0xFF;

	ret = cadence_nand_chips_init(cadence);
	if (ret) {
		dev_err(cadence->dev, "Failed to register MTD: %d\n",
			ret);
		goto free_buf;
	}

	kfree(cadence->buf);
	cadence->buf = kzalloc(cadence->buf_size, GFP_KERNEL);
	if (!cadence->buf) {
		ret = -ENOMEM;
		goto free_buf_desc;
	}

	return 0;

free_buf:
	kfree(cadence->buf);

free_buf_desc:
	dma_free_coherent(cadence->cdma_desc);

	return ret;
}

static const struct cadence_nand_dt_devdata cadence_nand_default = {
	.if_skew = 0,
	.has_dma = 0,
};

static const struct udevice_id cadence_nand_dt_ids[] = {
	{
		.compatible = "cdns,nand",
		.data = (unsigned long)&cadence_nand_default
	}, {}
};

static int cadence_nand_dt_probe(struct udevice *dev)
{
	struct cadence_nand_info *cadence = dev_get_priv(dev);
	const struct udevice_id *of_id;
	const struct cadence_nand_dt_devdata *devdata;
	struct resource res;
	int ret;
	u32 val;

	if (!dev) {
		dev_warn(dev, "Device ptr null\n");
		return -EINVAL;
	}

	of_id = &cadence_nand_dt_ids[0];
	devdata = (struct cadence_nand_dt_devdata *)of_id->data;

	cadence->caps1 = devdata;
	cadence->dev = dev;

	ret = clk_get_by_index(dev, 0, &cadence->clk);
	if (ret)
		return ret;

	ret = clk_enable(&cadence->clk);
	if (ret && ret != -ENOSYS && ret != -ENOMEM) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}
	cadence->nf_clk_rate = clk_get_rate(&cadence->clk);

	ret = reset_get_by_index(dev, 1, &cadence->softphy_reset);
	if (ret) {
		if (ret != -ENOMEM)
			dev_warn(dev, "Can't get softphy_reset: %d\n", ret);
	} else {
		reset_deassert(&cadence->softphy_reset);
	}

	ret = reset_get_by_index(dev, 0, &cadence->nand_reset);
	if (ret) {
		if (ret != -ENOMEM)
			dev_warn(dev, "Can't get nand_reset: %d\n", ret);
	} else {
		reset_deassert(&cadence->nand_reset);
	}

	ret = dev_read_resource_byname(dev, "reg", &res);
	if (ret)
		return ret;
	cadence->reg = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "sdma", &res);
	if (ret)
		return ret;
	cadence->io.dma = res.start;
	cadence->io.virt = devm_ioremap(dev, res.start, resource_size(&res));

	ret = ofnode_read_u32(dev_ofnode(dev->parent),
			      "cdns,board-delay-ps", &val);
	if (ret) {
		val = 4830;
		dev_info(cadence->dev,
			 "missing cdns,board-delay-ps property, %d was set\n",
			 val);
	}
	cadence->board_delay = val;

	ret = cadence_nand_init(cadence);
	if (ret)
		return ret;

	return 0;
}

U_BOOT_DRIVER(cadence_nand_dt) = {
	.name = "cadence-nand-dt",
	.id = UCLASS_MTD,
	.of_match = cadence_nand_dt_ids,
	.probe = cadence_nand_dt_probe,
	.priv_auto = sizeof(struct cadence_nand_info),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(cadence_nand_dt),
					  &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize Cadence NAND controller. (error %d)\n",
		       ret);
}
