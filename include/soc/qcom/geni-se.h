/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 */

#ifndef _QCOM_GENI_SE
#define _QCOM_GENI_SE

/* Protocols supported by GENI Serial Engines */
enum geni_se_protocol_type {
	GENI_SE_NONE,
	GENI_SE_SPI,
	GENI_SE_UART,
	GENI_SE_I2C,
	GENI_SE_I3C,
	GENI_SE_SPI_SLAVE,
};

#define QUP_HW_VER_REG			0x4

/* Common SE registers */
#define GENI_INIT_CFG_REVISION		0x0
#define GENI_S_INIT_CFG_REVISION	0x4
#define GENI_FORCE_DEFAULT_REG		0x20
#define GENI_OUTPUT_CTRL		0x24
#define GENI_CGC_CTRL			0x28
#define SE_GENI_STATUS			0x40
#define GENI_SER_M_CLK_CFG		0x48
#define GENI_SER_S_CLK_CFG		0x4c
#define GENI_IF_DISABLE_RO		0x64
#define GENI_FW_REVISION_RO		0x68
#define SE_GENI_CLK_SEL			0x7c
#define SE_GENI_CFG_SEQ_START		0x84
#define SE_GENI_BYTE_GRAN		0x254
#define SE_GENI_DMA_MODE_EN		0x258
#define SE_GENI_TX_PACKING_CFG0		0x260
#define SE_GENI_TX_PACKING_CFG1		0x264
#define SE_GENI_RX_PACKING_CFG0		0x284
#define SE_GENI_RX_PACKING_CFG1		0x288
#define SE_GENI_M_CMD0			0x600
#define SE_GENI_M_CMD_CTRL_REG		0x604
#define SE_GENI_M_IRQ_STATUS		0x610
#define SE_GENI_M_IRQ_EN		0x614
#define SE_GENI_M_IRQ_CLEAR		0x618
#define SE_GENI_S_CMD0			0x630
#define SE_GENI_S_CMD_CTRL_REG		0x634
#define SE_GENI_S_IRQ_STATUS		0x640
#define SE_GENI_S_IRQ_EN		0x644
#define SE_GENI_S_IRQ_CLEAR		0x648
#define SE_GENI_TX_FIFOn		0x700
#define SE_GENI_RX_FIFOn		0x780
#define SE_GENI_TX_FIFO_STATUS		0x800
#define SE_GENI_RX_FIFO_STATUS		0x804
#define SE_GENI_TX_WATERMARK_REG	0x80c
#define SE_GENI_RX_WATERMARK_REG	0x810
#define SE_GENI_RX_RFR_WATERMARK_REG	0x814
#define SE_GENI_IOS			0x908
#define SE_DMA_TX_IRQ_STAT		0xc40
#define SE_DMA_TX_IRQ_CLR		0xc44
#define SE_DMA_TX_FSM_RST		0xc58
#define SE_DMA_RX_IRQ_STAT		0xd40
#define SE_DMA_RX_IRQ_CLR		0xd44
#define SE_DMA_RX_LEN_IN		0xd54
#define SE_DMA_RX_FSM_RST		0xd58
#define SE_GSI_EVENT_EN			0xe18
#define SE_IRQ_EN			0xe1c
#define SE_HW_PARAM_0			0xe24
#define SE_HW_PARAM_1			0xe28

/* GENI_FORCE_DEFAULT_REG fields */
#define FORCE_DEFAULT	BIT(0)

/* GENI_OUTPUT_CTRL fields */
#define GENI_IO_MUX_0_EN		BIT(0)
#define DEFAULT_IO_OUTPUT_CTRL_MSK	GENMASK(6, 0)

/* GENI_CGC_CTRL fields */
#define CFG_AHB_CLK_CGC_ON		BIT(0)
#define CFG_AHB_WR_ACLK_CGC_ON		BIT(1)
#define DATA_AHB_CLK_CGC_ON		BIT(2)
#define SCLK_CGC_ON			BIT(3)
#define TX_CLK_CGC_ON			BIT(4)
#define RX_CLK_CGC_ON			BIT(5)
#define EXT_CLK_CGC_ON			BIT(6)
#define PROG_RAM_HCLK_OFF		BIT(8)
#define PROG_RAM_SCLK_OFF		BIT(9)
#define DEFAULT_CGC_EN			GENMASK(6, 0)

/* GENI_STATUS fields */
#define M_GENI_CMD_ACTIVE		BIT(0)
#define S_GENI_CMD_ACTIVE		BIT(12)

/* GENI_SER_M_CLK_CFG/GENI_SER_S_CLK_CFG */
#define SER_CLK_EN			BIT(0)
#define CLK_DIV_MSK			GENMASK(15, 4)
#define CLK_DIV_SHFT			4

/* GENI_IF_DISABLE_RO fields */
#define FIFO_IF_DISABLE			(BIT(0))

/* GENI_FW_REVISION_RO fields */
#define FW_REV_PROTOCOL_MSK		GENMASK(15, 8)
#define FW_REV_PROTOCOL_SHFT		8

/* GENI_CLK_SEL fields */
#define CLK_SEL_MSK			GENMASK(2, 0)

/* SE_GENI_CFG_SEQ_START fields */
#define START_TRIGGER			BIT(0)

/* SE_IRQ_EN fields */
#define DMA_RX_IRQ_EN			BIT(0)
#define DMA_TX_IRQ_EN			BIT(1)
#define GENI_M_IRQ_EN			BIT(2)
#define GENI_S_IRQ_EN			BIT(3)

/* SE_GENI_DMA_MODE_EN */
#define GENI_DMA_MODE_EN		BIT(0)

/* GENI_M_CMD0 fields */
#define M_OPCODE_MSK			GENMASK(31, 27)
#define M_OPCODE_SHFT			27
#define M_PARAMS_MSK			GENMASK(26, 0)

/* GENI_M_CMD_CTRL_REG */
#define M_GENI_CMD_CANCEL		BIT(2)
#define M_GENI_CMD_ABORT		BIT(1)
#define M_GENI_DISABLE			BIT(0)

/* GENI_S_CMD0 fields */
#define S_OPCODE_MSK			GENMASK(31, 27)
#define S_OPCODE_SHFT			27
#define S_PARAMS_MSK			GENMASK(26, 0)

/* GENI_S_CMD_CTRL_REG */
#define S_GENI_CMD_CANCEL		BIT(2)
#define S_GENI_CMD_ABORT		BIT(1)
#define S_GENI_DISABLE			BIT(0)

/* GENI_M_IRQ_EN fields */
#define M_CMD_DONE_EN			BIT(0)
#define M_CMD_OVERRUN_EN		BIT(1)
#define M_ILLEGAL_CMD_EN		BIT(2)
#define M_CMD_FAILURE_EN		BIT(3)
#define M_CMD_CANCEL_EN			BIT(4)
#define M_CMD_ABORT_EN			BIT(5)
#define M_TIMESTAMP_EN			BIT(6)
#define M_RX_IRQ_EN			BIT(7)
#define M_GP_SYNC_IRQ_0_EN		BIT(8)
#define M_GP_IRQ_0_EN			BIT(9)
#define M_GP_IRQ_1_EN			BIT(10)
#define M_GP_IRQ_2_EN			BIT(11)
#define M_GP_IRQ_3_EN			BIT(12)
#define M_GP_IRQ_4_EN			BIT(13)
#define M_GP_IRQ_5_EN			BIT(14)
#define M_TX_FIFO_NOT_EMPTY_EN		BIT(21)
#define M_IO_DATA_DEASSERT_EN		BIT(22)
#define M_IO_DATA_ASSERT_EN		BIT(23)
#define M_RX_FIFO_RD_ERR_EN		BIT(24)
#define M_RX_FIFO_WR_ERR_EN		BIT(25)
#define M_RX_FIFO_WATERMARK_EN		BIT(26)
#define M_RX_FIFO_LAST_EN		BIT(27)
#define M_TX_FIFO_RD_ERR_EN		BIT(28)
#define M_TX_FIFO_WR_ERR_EN		BIT(29)
#define M_TX_FIFO_WATERMARK_EN		BIT(30)
#define M_SEC_IRQ_EN			BIT(31)
#define M_COMMON_GENI_M_IRQ_EN	(GENMASK(6, 1) | \
				M_IO_DATA_DEASSERT_EN | \
				M_IO_DATA_ASSERT_EN | M_RX_FIFO_RD_ERR_EN | \
				M_RX_FIFO_WR_ERR_EN | M_TX_FIFO_RD_ERR_EN | \
				M_TX_FIFO_WR_ERR_EN)

/* GENI_S_IRQ_EN fields */
#define S_CMD_DONE_EN			BIT(0)
#define S_CMD_OVERRUN_EN		BIT(1)
#define S_ILLEGAL_CMD_EN		BIT(2)
#define S_CMD_FAILURE_EN		BIT(3)
#define S_CMD_CANCEL_EN			BIT(4)
#define S_CMD_ABORT_EN			BIT(5)
#define S_GP_SYNC_IRQ_0_EN		BIT(8)
#define S_GP_IRQ_0_EN			BIT(9)
#define S_GP_IRQ_1_EN			BIT(10)
#define S_GP_IRQ_2_EN			BIT(11)
#define S_GP_IRQ_3_EN			BIT(12)
#define S_GP_IRQ_4_EN			BIT(13)
#define S_GP_IRQ_5_EN			BIT(14)
#define S_IO_DATA_DEASSERT_EN		BIT(22)
#define S_IO_DATA_ASSERT_EN		BIT(23)
#define S_RX_FIFO_RD_ERR_EN		BIT(24)
#define S_RX_FIFO_WR_ERR_EN		BIT(25)
#define S_RX_FIFO_WATERMARK_EN		BIT(26)
#define S_RX_FIFO_LAST_EN		BIT(27)
#define S_COMMON_GENI_S_IRQ_EN	(GENMASK(5, 1) | GENMASK(13, 9) | \
				 S_RX_FIFO_RD_ERR_EN | S_RX_FIFO_WR_ERR_EN)

/*  GENI_/TX/RX/RX_RFR/_WATERMARK_REG fields */
#define WATERMARK_MSK			GENMASK(5, 0)

/* GENI_TX_FIFO_STATUS fields */
#define TX_FIFO_WC			GENMASK(27, 0)

/*  GENI_RX_FIFO_STATUS fields */
#define RX_LAST				BIT(31)
#define RX_LAST_BYTE_VALID_MSK		GENMASK(30, 28)
#define RX_LAST_BYTE_VALID_SHFT		28
#define RX_FIFO_WC_MSK			GENMASK(24, 0)

/* SE_GENI_IOS fields */
#define IO2_DATA_IN			BIT(1)
#define RX_DATA_IN			BIT(0)

/* SE_DMA_TX_IRQ_STAT Register fields */
#define TX_DMA_DONE			BIT(0)
#define TX_EOT				BIT(1)
#define TX_SBE				BIT(2)
#define TX_RESET_DONE			BIT(3)

/* SE_DMA_RX_IRQ_STAT Register fields */
#define RX_DMA_DONE			BIT(0)
#define RX_EOT				BIT(1)
#define RX_SBE				BIT(2)
#define RX_RESET_DONE			BIT(3)
#define RX_FLUSH_DONE			BIT(4)
#define RX_DMA_PARITY_ERR		BIT(5)
#define RX_DMA_BREAK			GENMASK(8, 7)
#define RX_GENI_GP_IRQ			GENMASK(10, 5)
#define RX_GENI_CANCEL_IRQ		BIT(11)
#define RX_GENI_GP_IRQ_EXT		GENMASK(13, 12)

/* SE_HW_PARAM_0 fields */
#define TX_FIFO_WIDTH_MSK		GENMASK(29, 24)
#define TX_FIFO_WIDTH_SHFT		24
/*
 * For QUP HW Version >= 3.10 Tx fifo depth support is increased
 * to 256bytes and corresponding bits are 16 to 23
 */
#define TX_FIFO_DEPTH_MSK_256_BYTES	GENMASK(23, 16)
#define TX_FIFO_DEPTH_MSK		GENMASK(21, 16)
#define TX_FIFO_DEPTH_SHFT		16

/* SE_HW_PARAM_1 fields */
#define RX_FIFO_WIDTH_MSK		GENMASK(29, 24)
#define RX_FIFO_WIDTH_SHFT		24
/*
 * For QUP HW Version >= 3.10 Rx fifo depth support is increased
 * to 256bytes and corresponding bits are 16 to 23
 */
#define RX_FIFO_DEPTH_MSK_256_BYTES	GENMASK(23, 16)
#define RX_FIFO_DEPTH_MSK		GENMASK(21, 16)
#define RX_FIFO_DEPTH_SHFT		16

#define HW_VER_MAJOR_MASK		GENMASK(31, 28)
#define HW_VER_MAJOR_SHFT		28
#define HW_VER_MINOR_MASK		GENMASK(27, 16)
#define HW_VER_MINOR_SHFT		16
#define HW_VER_STEP_MASK		GENMASK(15, 0)

#define GENI_SE_VERSION_MAJOR(ver) ((ver & HW_VER_MAJOR_MASK) >> HW_VER_MAJOR_SHFT)
#define GENI_SE_VERSION_MINOR(ver) ((ver & HW_VER_MINOR_MASK) >> HW_VER_MINOR_SHFT)
#define GENI_SE_VERSION_STEP(ver) (ver & HW_VER_STEP_MASK)

/* QUP SE VERSION value for major number 2 and minor number 5 */
#define QUP_SE_VERSION_2_5                  0x20050000

#endif
