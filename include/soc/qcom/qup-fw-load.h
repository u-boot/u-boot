/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef _LINUX_QCOM_QUP_FW_LOAD
#define _LINUX_QCOM_QUP_FW_LOAD

#include <linux/kernel.h>

/*Magic numbers*/
#define MAGIC_NUM_SE			0x57464553

/* Common SE registers*/
#define GENI_INIT_CFG_REVISION		0x0
#define GENI_S_INIT_CFG_REVISION	0x4
#define GENI_FORCE_DEFAULT_REG		0x20
#define GENI_CGC_CTRL			0x28
#define GENI_CFG_REG0			0x100

#define QUPV3_SE_HW_PARAM_1		0xE28
#define RX_FIFO_WIDTH_BIT		24
#define RX_FIFO_WIDTH_MASK		0x3F

/*Same registers as GENI_DMA_MODE_EN*/
#define QUPV3_SE_GENI_DMA_MODE_EN	0x258
#define GENI_M_IRQ_ENABLE		0x614
#define GENI_S_IRQ_ENABLE		0x644
#define GENI_RX_RFR_WATERMARK_REG	0x814
#define DMA_TX_IRQ_EN_SET		0xC4C
#define DMA_RX_IRQ_EN_SET		0xD4C
#define DMA_GENERAL_CFG			0xE30
#define SE_GENI_FW_REVISION		0x1000
#define SE_S_FW_REVISION		0x1004
#define SE_GENI_CFG_RAMN		0x1010
#define SE_GENI_CLK_CTRL		0x2000
#define SE_DMA_IF_EN			0x2004
#define SE_FIFO_IF_DISABLE		0x2008

#define MAX_GENI_CFG_RAMn_CNT		455

#define MI_PBT_NON_PAGED_SEGMENT	0x0
#define MI_PBT_HASH_SEGMENT		0x2
#define MI_PBT_NOTUSED_SEGMENT		0x3
#define MI_PBT_SHARED_SEGMENT		0x4
#define MI_PBT_FLAG_PAGE_MODE_MASK	0x100000
#define MI_PBT_FLAG_PAGE_MODE_SHIFT	0x14
#define MI_PBT_FLAG_SEGMENT_TYPE_MASK	0x7000000
#define MI_PBT_FLAG_SEGMENT_TYPE_SHIFT	0x18
#define MI_PBT_FLAG_ACCESS_TYPE_MASK	0xE00000
#define MI_PBT_FLAG_ACCESS_TYPE_SHIFT	0x15

#define MI_PBT_PAGE_MODE_VALUE(x) \
	(((x) & MI_PBT_FLAG_PAGE_MODE_MASK) >> \
	  MI_PBT_FLAG_PAGE_MODE_SHIFT)

#define MI_PBT_SEGMENT_TYPE_VALUE(x) \
	(((x) & MI_PBT_FLAG_SEGMENT_TYPE_MASK) >> \
		MI_PBT_FLAG_SEGMENT_TYPE_SHIFT)

#define MI_PBT_ACCESS_TYPE_VALUE(x) \
	(((x) & MI_PBT_FLAG_ACCESS_TYPE_MASK) >> \
	  MI_PBT_FLAG_ACCESS_TYPE_SHIFT)

/* GENI_FORCE_DEFAULT_REG fields */
#define FORCE_DEFAULT			BIT(0)

/* FW_REVISION_RO fields */
#define FW_REV_PROTOCOL_SHFT		8
#define FW_REV_VERSION_SHFT		0

#define GENI_FW_REVISION_RO		0x68
#define GENI_S_FW_REVISION_RO		0x6C

/* SE_GENI_DMA_MODE_EN */
#define GENI_DMA_MODE_EN		BIT(0)

/* GENI_M_IRQ_EN fields */
#define M_CMD_DONE_EN			BIT(0)
#define M_IO_DATA_DEASSERT_EN		BIT(22)
#define M_IO_DATA_ASSERT_EN		BIT(23)
#define M_RX_FIFO_RD_ERR_EN		BIT(24)
#define M_RX_FIFO_WR_ERR_EN		BIT(25)
#define M_RX_FIFO_WATERMARK_EN		BIT(26)
#define M_RX_FIFO_LAST_EN		BIT(27)
#define M_TX_FIFO_RD_ERR_EN		BIT(28)
#define M_TX_FIFO_WR_ERR_EN		BIT(29)
#define M_TX_FIFO_WATERMARK_EN		BIT(30)
#define M_COMMON_GENI_M_IRQ_EN	(GENMASK(6, 1) | \
				M_IO_DATA_DEASSERT_EN | \
				M_IO_DATA_ASSERT_EN | M_RX_FIFO_RD_ERR_EN | \
				M_RX_FIFO_WR_ERR_EN | M_TX_FIFO_RD_ERR_EN | \
				M_TX_FIFO_WR_ERR_EN)

/* GENI_S_IRQ_EN fields */
#define S_CMD_OVERRUN_EN		BIT(1)
#define S_ILLEGAL_CMD_EN		BIT(2)
#define S_CMD_CANCEL_EN			BIT(4)
#define S_CMD_ABORT_EN			BIT(5)
#define S_GP_IRQ_0_EN			BIT(9)
#define S_GP_IRQ_1_EN			BIT(10)
#define S_GP_IRQ_2_EN			BIT(11)
#define S_GP_IRQ_3_EN			BIT(12)
#define S_RX_FIFO_RD_ERR_EN		BIT(24)
#define S_RX_FIFO_WR_ERR_EN		BIT(25)
#define S_COMMON_GENI_S_IRQ_EN	(GENMASK(5, 1) | GENMASK(13, 9) | \
				 S_RX_FIFO_RD_ERR_EN | S_RX_FIFO_WR_ERR_EN)

#define GENI_CGC_CTRL_PROG_RAM_SCLK_OFF_BMSK		0x00000200
#define GENI_CGC_CTRL_PROG_RAM_HCLK_OFF_BMSK		0x00000100

#define GENI_DMA_MODE_EN_GENI_DMA_MODE_EN_BMSK		0x00000001

#define DMA_TX_IRQ_EN_SET_RESET_DONE_EN_SET_BMSK	0x00000008
#define DMA_TX_IRQ_EN_SET_SBE_EN_SET_BMSK		0x00000004
#define DMA_TX_IRQ_EN_SET_DMA_DONE_EN_SET_BMSK		0x00000001

#define DMA_RX_IRQ_EN_SET_FLUSH_DONE_EN_SET_BMSK	0x00000010
#define DMA_RX_IRQ_EN_SET_RESET_DONE_EN_SET_BMSK	0x00000008
#define DMA_RX_IRQ_EN_SET_SBE_EN_SET_BMSK		0x00000004
#define DMA_RX_IRQ_EN_SET_DMA_DONE_EN_SET_BMSK		0x00000001

#define DMA_GENERAL_CFG_AHB_SEC_SLV_CLK_CGC_ON_BMSK	0x00000008
#define DMA_GENERAL_CFG_DMA_AHB_SLV_CLK_CGC_ON_BMSK	0x00000004
#define DMA_GENERAL_CFG_DMA_TX_CLK_CGC_ON_BMSK		0x00000002
#define DMA_GENERAL_CFG_DMA_RX_CLK_CGC_ON_BMSK		0x00000001

#define GENI_CLK_CTRL_SER_CLK_SEL_BMSK			0x00000001
#define DMA_IF_EN_DMA_IF_EN_BMSK			0x00000001
#define SE_GSI_EVENT_EN_BMSK				0x0000000f
#define SE_IRQ_EN_RMSK					0x0000000f

#define QUPV3_COMMON_CFG				0x0120
#define FAST_SWITCH_TO_HIGH_DISABLE_BMASK		0x00000001

#define QUPV3_SE_AHB_M_CFG				0x0118
#define AHB_M_CLK_CGC_ON_BMASK				0x00000001

#define QUPV3_COMMON_CGC_CTRL				0x021C
#define COMMON_CSR_SLV_CLK_CGC_ON_BMASK			0x00000001

/* access ports */
#define geni_setbits32(_addr, _v) writel_relaxed(readl_relaxed(_addr) |  (_v), (_addr))
#define geni_clrbits32(_addr, _v) writel_relaxed(readl_relaxed(_addr) & ~(_v), (_addr))

/**
 * struct elf_se_hdr - firmware configurations
 *
 * @magic: set to 'SEFW'
 * @version: A 32-bit value indicating the structure’s version number
 * @core_version: QUPV3_HW_VERSION
 * @serial_protocol: Programmed into GENI_FW_REVISION
 * @fw_version: Programmed into GENI_FW_REVISION
 * @cfg_version: Programmed into GENI_INIT_CFG_REVISION
 * @fw_size_in_items: Number of (uint32_t) GENI_FW_RAM words
 * @fw_offset: Byte offset of GENI_FW_RAM array
 * @cfg_size_in_items: Number of GENI_FW_CFG index/value pairs
 * @cfg_idx_offset: Byte offset of GENI_FW_CFG index array
 * @cfg_val_offset: Byte offset of GENI_FW_CFG values array
 */
struct elf_se_hdr {
	u32 magic;
	u32 version;
	u32 core_version;
	u16 serial_protocol;
	u16 fw_version;
	u16 cfg_version;
	u16 fw_size_in_items;
	u16 fw_offset;
	u16 cfg_size_in_items;
	u16 cfg_idx_offset;
	u16 cfg_val_offset;
};

struct udevice;

int qcom_geni_load_firmware(phys_addr_t qup_base, struct udevice *dev);

#endif /* _LINUX_QCOM_QUP_FW_LOAD */
