// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek MT8189 embedded DisplayPort transmitter
 *
 * Drives the vdosys0 display pipeline (OVL, RDMA, DVO) and the eDP
 * transmitter: reads the attached panel's EDID over the AUX channel, trains
 * the link and scans out the U-Boot framebuffer to it.
 *
 * Copyright (c) 2019-2022 MediaTek Inc.
 *                         Jitao Shi <jitao.shi@mediatek.com>
 *                         Bo-Chen Chen <rex-bc.chen@mediatek.com>
 * Copyright (c) 2022, 2026 BayLibre, SAS.
 *                          Markus Schneider-Pargmann <msp@baylibre.com>
 *                          David Lechner <dlechner@baylibre.com>
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <backlight.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/ofnode_graph.h>
#include <edid.h>
#include <errno.h>
#include <fdtdec.h>
#include <generic-phy.h>
#include <linux/arm-smccc.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/drm_dp_helper.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <phy-dp.h>
#include <power/regulator.h>
#include <regmap.h>
#include <syscon.h>
#include <video.h>

#include "mtk_ddp.h"
#include "mtk_disp_comp.h"
#include "mtk_disp_mutex.h"
#include "mtk_disp_ovl.h"
#include "mtk_disp_rdma.h"
#include "mtk_dvo.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Register definitions, taken from the MediaTek DisplayPort driver in Linux
 * (drivers/gpu/drm/mediatek/mtk_dp_reg.h). Only the subset this driver
 * touches is reproduced here. The names keep their legacy offsets, from a
 * time the PHY sat in the first 0x2000 bytes of the same register range,
 * while the values are relative to the transmitter's own registers.
 */
#define MTK_DP_TOP_PWR_STATE			0x0
#define DP_PWR_STATE_MASK				GENMASK(1, 0)
#define DP_PWR_STATE_BANDGAP_TPLL			BIT(1)
#define DP_PWR_STATE_BANDGAP_TPLL_LANE			GENMASK(1, 0)
#define MTK_DP_TOP_SWING_EMP			0x4
#define DP_TX0_VOLT_SWING_MASK				GENMASK(1, 0)
#define DP_TX0_VOLT_SWING_SHIFT				0
#define DP_TX0_PRE_EMPH_MASK				GENMASK(3, 2)
#define DP_TX0_PRE_EMPH_SHIFT				2
#define DP_TX1_VOLT_SWING_SHIFT				8
#define MTK_DP_TOP_RESET_AND_PROBE		0x20
#define SW_RST_B_PHYD					BIT(4)
#define RG_SW_RST_MASK					GENMASK(7, 0)
#define RG_SW_RST					0xff
#define RG_PROBE_LOW_SEL_MASK				GENMASK(18, 16)
#define RG_PROBE_LOW_SEL				BIT(16)
#define RG_PROBE_LOW_HIGH_SWAP				BIT(23)
#define MTK_DP_TOP_IRQ_MASK			0x2c
#define ENCODER_IRQ_MSK					BIT(0)
#define TRANS_IRQ_MSK					BIT(1)
#define IRQ_MASK_AUX_TOP_IRQ				BIT(2)
#define MTK_DP_TOP_MEM_PD			0x38
#define MEM_ISO_EN					BIT(0)
#define FUSE_SEL					BIT(2)
#define EDP_TX_TOP_CLKGEN_0			0x74
#define EDP_TX_TOP_CLKGEN_REST_MASK			0xf
#define EDP_TX_TOP_CLKGEN_REST_VALUE			0xf
#define MTK_DP_ENC0_P0_3000			0x1000
#define LANE_NUM_DP_ENC0_P0_MASK			GENMASK(1, 0)
#define VIDEO_MUTE_SW_DP_ENC0_P0			BIT(2)
#define VIDEO_MUTE_SEL_DP_ENC0_P0			BIT(3)
#define ENHANCED_FRAME_EN_DP_ENC0_P0			BIT(4)
#define DP_I_MODE_ENABLE				BIT(6)
#define REG_BS_SYMBOL_CNT_RESET				BIT(7)
#define MTK_DP_ENC0_P0_3004			0x1004
#define VIDEO_M_CODE_SEL_DP_ENC0_P0_MASK		BIT(8)
#define DP_TX_ENCODER_4P_RESET_SW_DP_ENC0_P0		BIT(9)
#define MTK_DP_ENC0_P0_3010			0x1010
#define HTOTAL_SW_DP_ENC0_P0_MASK			GENMASK(15, 0)
#define MTK_DP_ENC0_P0_3014			0x1014
#define VTOTAL_SW_DP_ENC0_P0_MASK			GENMASK(15, 0)
#define MTK_DP_ENC0_P0_3018			0x1018
#define HSTART_SW_DP_ENC0_P0_MASK			GENMASK(15, 0)
#define MTK_DP_ENC0_P0_301C			0x101c
#define VSTART_SW_DP_ENC0_P0_MASK			GENMASK(15, 0)
#define MTK_DP_ENC0_P0_3020			0x1020
#define HWIDTH_SW_DP_ENC0_P0_MASK			GENMASK(15, 0)
#define MTK_DP_ENC0_P0_3024			0x1024
#define VHEIGHT_SW_DP_ENC0_P0_MASK			GENMASK(15, 0)
#define MTK_DP_ENC0_P0_3028			0x1028
#define HSW_SW_DP_ENC0_P0_MASK				GENMASK(14, 0)
#define HSP_SW_DP_ENC0_P0_MASK				BIT(15)
#define MTK_DP_ENC0_P0_302C			0x102c
#define VSW_SW_DP_ENC0_P0_MASK				GENMASK(14, 0)
#define VSP_SW_DP_ENC0_P0_MASK				BIT(15)
#define MTK_DP_ENC0_P0_3030			0x1030
#define HTOTAL_SEL_DP_ENC0_P0				BIT(0)
#define VTOTAL_SEL_DP_ENC0_P0				BIT(1)
#define HSTART_SEL_DP_ENC0_P0				BIT(2)
#define VSTART_SEL_DP_ENC0_P0				BIT(3)
#define HWIDTH_SEL_DP_ENC0_P0				BIT(4)
#define VHEIGHT_SEL_DP_ENC0_P0				BIT(5)
#define HSP_SEL_DP_ENC0_P0				BIT(6)
#define HSW_SEL_DP_ENC0_P0				BIT(7)
#define VSP_SEL_DP_ENC0_P0				BIT(8)
#define VSW_SEL_DP_ENC0_P0				BIT(9)
#define MTK_DP_ENC0_P0_3034			0x1034
#define MTK_DP_ENC0_P0_3038			0x1038
#define VIDEO_SOURCE_SEL_DP_ENC0_P0_MASK		BIT(11)
#define MTK_DP_ENC0_P0_303C			0x103c
#define SRAM_START_READ_THRD_DP_ENC0_P0_MASK		GENMASK(5, 0)
#define SRAM_START_READ_THRD_DP_ENC0_P0_VALUE		BIT(3)
#define VIDEO_COLOR_DEPTH_DP_ENC0_P0_MASK		GENMASK(10, 8)
#define VIDEO_COLOR_DEPTH_DP_ENC0_P0_8BIT		(3 << 8)
#define PIXEL_ENCODE_FORMAT_DP_ENC0_P0_MASK		GENMASK(14, 12)
#define PIXEL_ENCODE_FORMAT_DP_ENC0_P0_RGB		0
#define VIDEO_MN_GEN_EN_DP_ENC0_P0			BIT(15)
#define MTK_DP_ENC0_P0_3040			0x1040
#define SDP_DOWN_CNT_DP_ENC0_P0_VAL			0x20
#define SDP_DOWN_CNT_INIT_DP_ENC0_P0_MASK		GENMASK(11, 0)
#define MTK_DP_ENC0_P0_304C			0x104c
#define VBID_VIDEO_MUTE_DP_ENC0_P0_MASK			BIT(2)
#define SDP_VSYNC_RISING_MASK_DP_ENC0_P0_MASK		BIT(8)
#define MTK_DP_ENC0_P0_3064			0x1064
#define HDE_NUM_LAST_DP_ENC0_P0_MASK			GENMASK(15, 0)
#define MTK_DP_ENC0_P0_3154			0x1154
#define PGEN_HTOTAL_DP_ENC0_P0_MASK			GENMASK(13, 0)
#define MTK_DP_ENC0_P0_3158			0x1158
#define PGEN_HSYNC_RISING_DP_ENC0_P0_MASK		GENMASK(13, 0)
#define MTK_DP_ENC0_P0_315C			0x115c
#define PGEN_HSYNC_PULSE_WIDTH_DP_ENC0_P0_MASK		GENMASK(13, 0)
#define MTK_DP_ENC0_P0_3160			0x1160
#define PGEN_HFDE_START_DP_ENC0_P0_MASK			GENMASK(13, 0)
#define MTK_DP_ENC0_P0_3164			0x1164
#define PGEN_HFDE_ACTIVE_WIDTH_DP_ENC0_P0_MASK		GENMASK(13, 0)
#define MTK_DP_ENC0_P0_3168			0x1168
#define PGEN_VTOTAL_DP_ENC0_P0_MASK			GENMASK(12, 0)
#define MTK_DP_ENC0_P0_316C			0x116c
#define PGEN_VSYNC_RISING_DP_ENC0_P0_MASK		GENMASK(12, 0)
#define MTK_DP_ENC0_P0_3170			0x1170
#define PGEN_VSYNC_PULSE_WIDTH_DP_ENC0_P0_MASK		GENMASK(12, 0)
#define MTK_DP_ENC0_P0_3174			0x1174
#define PGEN_VFDE_START_DP_ENC0_P0_MASK			GENMASK(12, 0)
#define MTK_DP_ENC0_P0_3178			0x1178
#define PGEN_VFDE_ACTIVE_WIDTH_DP_ENC0_P0_MASK		GENMASK(12, 0)
#define MTK_DP_ENC0_P0_31B0			0x11b0
#define PGEN_PATTERN_SEL_VAL				4
#define PGEN_PATTERN_SEL_MASK				GENMASK(6, 4)
#define MTK_DP_ENC0_P0_31EC			0x11ec
#define AUDIO_CH_SRC_SEL_DP_ENC0_P0			BIT(4)
#define MTK_DP_ENC1_P0_3300			0x1300
#define VIDEO_AFIFO_RDY_SEL_DP_ENC1_P0_VAL		2
#define VIDEO_AFIFO_RDY_SEL_DP_ENC1_P0_MASK		GENMASK(9, 8)
#define MTK_DP_ENC1_P0_3364			0x1364
#define SDP_DOWN_CNT_IN_HBLANK_DP_ENC1_P0_VAL		0x20
#define SDP_DOWN_CNT_INIT_IN_HBLANK_DP_ENC1_P0_MASK	GENMASK(11, 0)
#define FIFO_READ_START_POINT_DP_ENC1_P0_VAL		4
#define FIFO_READ_START_POINT_DP_ENC1_P0_MASK		GENMASK(15, 12)
#define MTK_DP_ENC1_P0_3368			0x1368
#define VIDEO_SRAM_FIFO_CNT_RESET_SEL_DP_ENC1_P0	BIT(0)
#define VIDEO_SRAM_FIFO_CNT_RESET_SEL_MASK		GENMASK(1, 0)
#define BS2BS_MODE_DP_ENC1_P0_MASK			GENMASK(13, 12)
#define BS2BS_MODE_DP_ENC1_P0_VAL			1
#define BS_FOLLOW_SEL_DP_ENC0_P0			BIT(15)
#define MTK_DP_ENC1_P0_33C0			0x13c0
#define SDP_TESTBUS_SEL_DP_ENC_MASK			GENMASK(15, 12)
#define SDP_TESTBUS_SEL_BIT4_DP_ENC_MASK		BIT(7)
#define SDP_TESTBUS_SEL_BIT4_DP_ENC			BIT(7)
#define MTK_DP_ENC1_P0_33C4			0x13c4
#define DP_TX_ENCODER_TESTBUS_SEL_DP_ENC_MASK		GENMASK(6, 5)
#define DP_TX_ENCODER_TESTBUS_SEL_DP_ENC		BIT(5)
#define MTK_DP_TRANS_P0_3400			0x1400
#define PATTERN1_EN_DP_TRANS_P0_MASK			BIT(12)
#define PATTERN2_EN_DP_TRANS_P0_MASK			BIT(13)
#define PATTERN3_EN_DP_TRANS_P0_MASK			BIT(14)
#define PATTERN4_EN_DP_TRANS_P0_MASK			BIT(15)
#define MTK_DP_TRANS_P0_3404			0x1404
#define DP_SCR_EN_DP_TRANS_P0_MASK			BIT(0)
#define MTK_DP_TRANS_P0_340C			0x140c
#define DP_TX_TRANSMITTER_4P_RESET_SW_DP_TRANS_P0	BIT(13)
#define MTK_DP_TRANS_P0_342C			0x142c
#define XTAL_FREQ_DP_TRANS_P0_DEFAULT			(BIT(0) | BIT(3) | BIT(5) | BIT(6))
#define XTAL_FREQ_DP_TRANS_P0_MASK			GENMASK(7, 0)
#define MTK_DP_TRANS_P0_34A4			0x14a4
#define LANE_NUM_DP_TRANS_P0_MASK			GENMASK(3, 2)
#define MTK_DP_TRANS_P0_3540			0x1540
#define FEC_CLOCK_EN_MODE_DP_TRANS_P0			BIT(3)
#define MTK_DP_TRANS_P0_3580			0x1580
#define POST_MISC_DATA_LANE0_OV_DP_TRANS_P0_MASK	BIT(8)
#define POST_MISC_DATA_LANE1_OV_DP_TRANS_P0_MASK	BIT(9)
#define POST_MISC_DATA_LANE2_OV_DP_TRANS_P0_MASK	BIT(10)
#define POST_MISC_DATA_LANE3_OV_DP_TRANS_P0_MASK	BIT(11)
#define MTK_DP_TRANS_P0_35F0			0x15f0
#define DP_TRANS_DUMMY_RW_0				BIT(3)
#define DP_TRANS_DUMMY_RW_0_MASK			GENMASK(3, 2)
#define MTK_DP_AUX_P0_360C			0x160c
#define AUX_TIMEOUT_THR_AUX_TX_P0_MASK			GENMASK(12, 0)
#define AUX_TIMEOUT_THR_AUX_TX_P0_VAL			0x1595
#define MTK_DP_AUX_P0_3614			0x1614
#define AUX_RX_UI_CNT_THR_AUX_TX_P0_MASK		GENMASK(6, 0)
#define AUX_RX_UI_CNT_THR_FOR_26M			13
#define MTK_DP_AUX_P0_3618			0x1618
#define AUX_RX_FIFO_FULL_AUX_TX_P0_MASK			BIT(9)
#define AUX_RX_FIFO_WRITE_POINTER_AUX_TX_P0_MASK	GENMASK(3, 0)
#define MTK_DP_AUX_P0_3620			0x1620
#define AUX_RD_MODE_AUX_TX_P0_MASK			BIT(9)
#define AUX_RX_FIFO_READ_PULSE_TX_P0			BIT(8)
#define AUX_RX_FIFO_READ_DATA_AUX_TX_P0_MASK		GENMASK(7, 0)
#define MTK_DP_AUX_P0_3624			0x1624
#define AUX_RX_REPLY_COMMAND_AUX_TX_P0_MASK		GENMASK(3, 0)
#define MTK_DP_AUX_P0_3628			0x1628
#define AUX_RX_PHY_STATE_AUX_TX_P0_MASK			GENMASK(9, 0)
#define AUX_RX_PHY_STATE_AUX_TX_P0_RX_IDLE		BIT(0)
#define MTK_DP_AUX_P0_362C			0x162c
#define AUX_NO_LENGTH_AUX_TX_P0				BIT(0)
#define AUX_TX_AUXTX_OV_EN_AUX_TX_P0_MASK		BIT(1)
#define AUX_RESERVED_RW_0_AUX_TX_P0_MASK		GENMASK(15, 2)
#define MTK_DP_AUX_P0_3630			0x1630
#define AUX_TX_REQUEST_READY_AUX_TX_P0			BIT(3)
#define MTK_DP_AUX_P0_3634			0x1634
#define AUX_TX_OVER_SAMPLE_RATE_AUX_TX_P0_MASK		GENMASK(15, 8)
#define AUX_TX_OVER_SAMPLE_RATE_FOR_26M			25
#define MTK_DP_AUX_P0_3640			0x1640
#define AUX_400US_TIMEOUT_IRQ_AUX_TX_P0			BIT(0)
#define AUX_RX_DATA_RECV_IRQ_AUX_TX_P0			BIT(1)
#define AUX_RX_ADDR_RECV_IRQ_AUX_TX_P0			BIT(2)
#define AUX_RX_CMD_RECV_IRQ_AUX_TX_P0			BIT(3)
#define AUX_RX_MCCS_RECV_COMPLETE_IRQ_AUX_TX_P0		BIT(4)
#define AUX_RX_EDID_RECV_COMPLETE_IRQ_AUX_TX_P0		BIT(5)
#define AUX_RX_AUX_RECV_COMPLETE_IRQ_AUX_TX_P0		BIT(6)
#define DP_AUX_P0_3640_VAL	(AUX_400US_TIMEOUT_IRQ_AUX_TX_P0 | \
				 AUX_RX_DATA_RECV_IRQ_AUX_TX_P0 | \
				 AUX_RX_ADDR_RECV_IRQ_AUX_TX_P0 | \
				 AUX_RX_CMD_RECV_IRQ_AUX_TX_P0 | \
				 AUX_RX_MCCS_RECV_COMPLETE_IRQ_AUX_TX_P0 | \
				 AUX_RX_EDID_RECV_COMPLETE_IRQ_AUX_TX_P0 | \
				 AUX_RX_AUX_RECV_COMPLETE_IRQ_AUX_TX_P0)
#define MTK_DP_AUX_P0_3644			0x1644
#define MCU_REQUEST_COMMAND_AUX_TX_P0_MASK		GENMASK(3, 0)
#define MTK_DP_AUX_P0_3648			0x1648
#define MCU_REQUEST_ADDRESS_LSB_AUX_TX_P0_MASK		GENMASK(15, 0)
#define MTK_DP_AUX_P0_364C			0x164c
#define MCU_REQUEST_ADDRESS_MSB_AUX_TX_P0_MASK		GENMASK(3, 0)
#define HPD_INT_THD_FLDMASK				GENMASK(9, 4)
#define HPD_INT_THD_FLDMASK_VAL				0x32
#define HPD_STATUS_DP_AUX_TX_P0_MASK			BIT(15)
#define MTK_DP_AUX_P0_3650			0x1650
#define MCU_ACK_TRAN_COMPLETE_AUX_TX_P0			BIT(8)
#define PHY_FIFO_RST_AUX_TX_P0_MASK			BIT(9)
#define MCU_REQ_DATA_NUM_AUX_TX_P0_MASK			GENMASK(15, 12)
#define MTK_DP_AUX_P0_3658			0x1658
#define AUX_TX_OV_EN_AUX_TX_P0_MASK			BIT(0)
#define MTK_DP_AUX_P0_366C			0x166c
#define XTAL_FREQ_DP_TX_AUX_366C_MASK			GENMASK(15, 8)
#define XTAL_FREQ_DP_TX_AUX_366C_VALUE			0x68
#define MTK_DP_AUX_P0_367C			0x167c
#define HPD_CONN_THD_AUX_TX_P0_FLDMASK			GENMASK(9, 6)
#define HPD_CONN_THD_AUX_TX_P0_VAL			5
#define MTK_DP_AUX_P0_3690			0x1690
#define RX_REPLY_COMPLETE_MODE_AUX_TX_P0		BIT(8)
#define MTK_DP_AUX_P0_36A0			0x16a0
#define DP_TX_INIT_MASK_15_TO_2_MASK			GENMASK(15, 2)
#define DP_TX_INIT_MASK_15_TO_2				0xfffc
#define MTK_DP_AUX_P0_3704			0x1704
#define AUX_TX_FIFO_WDATA_NEW_MODE_T_AUX_TX_P0_MASK	BIT(1)
#define AUX_TX_FIFO_NEW_MODE_EN_AUX_TX_P0		BIT(2)
#define MTK_DP_AUX_P0_3708			0x1708
#define MTK_DP_AUX_P0_37A0			0x17a0
#define HPD_DISC_THD_AUX_TX_P0_FLDMASK			GENMASK(7, 4)
#define HPD_DISC_THD_AUX_TX_P0_VAL			5
#define MTK_DP_AUX_P0_37C8			0x17c8
#define MTK_ATOP_EN_AUX_TX_P0				BIT(0)
#define REG_3F04_DP_ENC_P0_3			0x1f04
#define FRAME_START_MARKER_0_DP_ENC_P0_3_MASK		GENMASK(15, 0)
#define REG_3F08_DP_ENC_P0_3			0x1f08
#define FRAME_START_MARKER_1_DP_ENC_P0_3_MASK		BIT(3)
#define FRAME_START_MARKER_1_DP_ENC_P0_3		BIT(3)
#define REG_3F0C_DP_ENC_P0_3			0x1f0c
#define FRAME_END_MARKER_0_DP_ENC_P0_3_MASK		BIT(1)
#define FRAME_END_MARKER_0_DP_ENC_P0_3			BIT(1)
#define REG_3F10_DP_ENC_P0_3			0x1f10
#define FRAME_END_MARKER_1_DP_ENC_P0_3_MASK		BIT(3)
#define FRAME_END_MARKER_1_DP_ENC_P0_3			BIT(3)
#define REG_3F28_DP_ENC_P0_3			0x1f28
#define DP_TX_SDP_PSR_AS_TESTBUS_MASK			GENMASK(5, 2)
#define DP_TX_SDP_PSR_AS_TESTBUS			(0xa << 2)
#define REG_3F44_DP_ENC_P0_3			0x1f44
#define PHY_PWR_STATE_OW_EN_DP_ENC_P0_3			BIT(2)
#define PHY_PWR_STATE_OW_EN_DP_ENC_P0_3_MASK		BIT(2)
#define ALL_POWER_OFF					(0x0 << 3)
#define BIAS_POWER_ON					(0x1 << 3)
#define ALL_POWER_ON					(0x3 << 3)
#define PHY_PWR_STATE_OW_VALUE_DP_ENC_P0_3_MASK		GENMASK(4, 3)
#define REG_3F80_DP_ENC_P0_3			0x1f80
#define PSR_PATGEN_AVT_EN_FLDMASK			BIT(5)
#define REG_3FF8_DP_ENC_P0_3			0x1ff8
#define PHY_STATE_RESET_ALL_MASK			GENMASK(7, 0)
#define PHY_STATE_RESET_ALL_VALUE			0xff
#define PHY_STATE_W_1_DP_ENC_P0_3			BIT(6)
#define PHY_STATE_W_1_DP_ENC_P0_3_MASK			BIT(6)
#define DVO_ON_W_1_FLDMASK				BIT(5)
#define XTAL_FREQ_FOR_PSR_DP_ENC_P0_3_MASK		GENMASK(13, 9)
#define XTAL_FREQ_FOR_PSR_DP_ENC_P0_3_VALUE		25

/* MISC0 fields, from the DP specification */
#define DP_MSA_MISC_8_BPC			BIT(5)
#define DP_TEST_COLOR_FORMAT_MASK		GENMASK(2, 1)
#define DP_COLOR_FORMAT_RGB			(0 << 1)
#define DP_TEST_BIT_DEPTH_MASK			GENMASK(7, 5)

/*
 * The transmitter's video mute bit only takes effect once the secure world
 * has been asked to unmute, so every mute change has to be mirrored there.
 */
#define MTK_SIP_DP_CONTROL		ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
							   ARM_SMCCC_SMC_64, \
							   ARM_SMCCC_OWNER_SIP, 0x523)
#define MTK_DP_SIP_ATF_EDP_SET_VID_MUTE	0x22
#define MTK_DP_SIP_REQ_VIDEO_UNMUTE	0xfefd

#define MTK_EDP_MAX_WIDTH		3840
#define MTK_EDP_MAX_HEIGHT		2160

#define MTK_EDP_4P1T			4
#define MTK_EDP_HDE			2
#define MTK_EDP_PIX_PER_ADDR		2
#define MTK_EDP_AUX_WAIT_REPLY_COUNT	20
/*
 * Seven is the I2C-over-AUX count from the DP compliance tests. Native DPCD
 * gets 32, the number drm_dp_dpcd_access() moved to after finding that seven
 * was not enough for real sinks.
 */
#define MTK_EDP_AUX_I2C_RETRY_COUNT	7
#define MTK_EDP_AUX_NATIVE_RETRY_COUNT	32
/* attempts at a corrupt EDID block, as edid_block_read() gives it in Linux */
#define MTK_EDP_EDID_READ_TRIES		4
#define MTK_EDP_TBC_BUF_READ_START_ADDR	0x8
#define MTK_EDP_TRAIN_VOLTAGE_LEVEL_RETRY 5
#define MTK_EDP_TRAIN_DOWNSCALE_RETRY	10

/* HPD is wired straight to the panel, so it asserts as soon as it powers up */
#define MTK_EDP_HPD_TIMEOUT_MS		500

/*
 * eDP panels need settling time that the EDID cannot describe, and which
 * Linux keeps in panel-edp's per-panel table rather than in the device tree.
 * Rather than carry a copy of that table, use values generous enough for the
 * panels these boards ship with:
 *
 *   HPD_RELIABLE - time after powering the panel before HPD can be trusted.
 *		    Sampling earlier can latch a glitch and leave us reading
 *		    EDID from a panel whose AUX is not up yet.
 *   ENABLE	  - time after the video stream starts until the panel shows
 *		    a valid frame, i.e. how long to wait before the backlight
 *		    can come on without revealing garbage.
 */
#define MTK_EDP_PANEL_HPD_RELIABLE_MS	220
#define MTK_EDP_PANEL_ENABLE_MS		50

/* time the sink needs to settle after the video is muted, as in mtk_dp */
#define MTK_EDP_VIDEO_MUTE_SETTLE_MS	20

/* the encoder is configured for 8 bit RGB and nothing else */
#define MTK_EDP_BITS_PER_PIXEL		24

struct mtk_edp_train_info {
	bool sink_ssc;
	/* in multiples of 0.27Gbps */
	u8 link_rate;
	u8 lane_count;
	unsigned int channel_eq_pattern;
};

/**
 * struct mtk_edp_data - per-SoC eDP data
 * @mmsys_compatible: compatible of the mmsys block holding the display muxes
 * @ddp_connect: route the mmsys muxes from the overlay to the DVO
 */
struct mtk_edp_data {
	const char *mmsys_compatible;
	void (*ddp_connect)(void __iomem *mmsys_base);
};

struct mtk_edp_priv {
	struct udevice *dev;
	const struct mtk_edp_data *data;
	void __iomem *regs;
	void __iomem *mmsys_base;
	struct phy phy;

	struct udevice *dvo;
	struct udevice *ovl;
	struct udevice *rdma;
	struct udevice *mutex;
	struct udevice *backlight;
	struct udevice *panel_supply;

	u8 max_lanes;
	u8 max_linkrate;
	u8 rx_cap[DP_RECEIVER_CAP_SIZE];

	struct mtk_edp_train_info train_info;
	struct display_timing timing;
	u8 edid[EDID_EXT_SIZE];
};

static u32 mtk_edp_read(struct mtk_edp_priv *priv, u32 offset)
{
	return readl(priv->regs + offset);
}

static void mtk_edp_write(struct mtk_edp_priv *priv, u32 offset, u32 val)
{
	writel(val, priv->regs + offset);
}

static void mtk_edp_mask(struct mtk_edp_priv *priv, u32 offset, u32 val,
			 u32 mask)
{
	clrsetbits_le32(priv->regs + offset, mask, val & mask);
}

/* The AUX write FIFO takes two bytes per 32-bit register. */
static void mtk_edp_bulk_16bit_write(struct mtk_edp_priv *priv, u32 offset,
				     const u8 *buf, size_t length)
{
	int i;

	for (i = 0; i < length; i += 2) {
		u32 val = buf[i] | (i + 1 < length ? buf[i + 1] << 8 : 0);

		mtk_edp_write(priv, offset + i * 2, val);
	}
}

static void mtk_edp_aux_set_cmd(struct mtk_edp_priv *priv, u8 cmd, u32 addr)
{
	mtk_edp_mask(priv, MTK_DP_AUX_P0_3644, cmd,
		     MCU_REQUEST_COMMAND_AUX_TX_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_AUX_P0_3648, addr,
		     MCU_REQUEST_ADDRESS_LSB_AUX_TX_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_AUX_P0_364C, addr >> 16,
		     MCU_REQUEST_ADDRESS_MSB_AUX_TX_P0_MASK);
}

static void mtk_edp_aux_read_rx_fifo(struct mtk_edp_priv *priv, u8 *buf,
				     size_t length, int read_delay)
{
	int read_pos;

	mtk_edp_mask(priv, MTK_DP_AUX_P0_3620, 0, AUX_RD_MODE_AUX_TX_P0_MASK);

	for (read_pos = 0; read_pos < length; read_pos++) {
		mtk_edp_mask(priv, MTK_DP_AUX_P0_3620,
			     AUX_RX_FIFO_READ_PULSE_TX_P0,
			     AUX_RX_FIFO_READ_PULSE_TX_P0);

		/* Hardware needs time to update the data */
		udelay(read_delay);
		buf[read_pos] = mtk_edp_read(priv, MTK_DP_AUX_P0_3620) &
				AUX_RX_FIFO_READ_DATA_AUX_TX_P0_MASK;
	}
}

static void mtk_edp_aux_set_length(struct mtk_edp_priv *priv, size_t length)
{
	if (length > 0) {
		mtk_edp_mask(priv, MTK_DP_AUX_P0_3650,
			     FIELD_PREP(MCU_REQ_DATA_NUM_AUX_TX_P0_MASK,
					length - 1),
			     MCU_REQ_DATA_NUM_AUX_TX_P0_MASK);
		mtk_edp_mask(priv, MTK_DP_AUX_P0_362C, 0,
			     AUX_NO_LENGTH_AUX_TX_P0 |
			     AUX_TX_AUXTX_OV_EN_AUX_TX_P0_MASK |
			     AUX_RESERVED_RW_0_AUX_TX_P0_MASK);
	} else {
		mtk_edp_mask(priv, MTK_DP_AUX_P0_362C,
			     AUX_NO_LENGTH_AUX_TX_P0,
			     AUX_NO_LENGTH_AUX_TX_P0 |
			     AUX_TX_AUXTX_OV_EN_AUX_TX_P0_MASK |
			     AUX_RESERVED_RW_0_AUX_TX_P0_MASK);
	}
}

static int mtk_edp_aux_wait_for_completion(struct mtk_edp_priv *priv,
					   bool is_read)
{
	int wait_reply = MTK_EDP_AUX_WAIT_REPLY_COUNT;

	while (--wait_reply) {
		u32 aux_irq_status;

		if (is_read) {
			u32 fifo_status = mtk_edp_read(priv, MTK_DP_AUX_P0_3618);

			if (fifo_status &
			    (AUX_RX_FIFO_WRITE_POINTER_AUX_TX_P0_MASK |
			     AUX_RX_FIFO_FULL_AUX_TX_P0_MASK))
				return 0;
		}

		aux_irq_status = mtk_edp_read(priv, MTK_DP_AUX_P0_3640);
		if (aux_irq_status & AUX_RX_AUX_RECV_COMPLETE_IRQ_AUX_TX_P0)
			return 0;

		if (aux_irq_status & AUX_400US_TIMEOUT_IRQ_AUX_TX_P0)
			return -ETIMEDOUT;

		/* Give the hardware a chance to complete before retrying */
		udelay(100);
	}

	return -ETIMEDOUT;
}

static int mtk_edp_aux_do_transfer(struct mtk_edp_priv *priv, bool is_read,
				   u8 cmd, u32 addr, u8 *buf, size_t length,
				   u8 *reply_cmd)
{
	int ret;

	if (length > DP_AUX_MAX_PAYLOAD_BYTES)
		return -EINVAL;

	if (is_read && cmd == DP_AUX_NATIVE_READ && !length)
		return -EINVAL;

	if (!is_read)
		mtk_edp_mask(priv, MTK_DP_AUX_P0_3704,
			     AUX_TX_FIFO_NEW_MODE_EN_AUX_TX_P0,
			     AUX_TX_FIFO_NEW_MODE_EN_AUX_TX_P0);

	/* Clear the FIFO and the IRQ status before talking to the sink */
	mtk_edp_mask(priv, MTK_DP_AUX_P0_3650,
		     MCU_ACK_TRAN_COMPLETE_AUX_TX_P0,
		     MCU_ACK_TRAN_COMPLETE_AUX_TX_P0 |
		     PHY_FIFO_RST_AUX_TX_P0_MASK |
		     MCU_REQ_DATA_NUM_AUX_TX_P0_MASK);
	mtk_edp_write(priv, MTK_DP_AUX_P0_3640, DP_AUX_P0_3640_VAL);

	mtk_edp_aux_set_cmd(priv, cmd, addr);
	mtk_edp_aux_set_length(priv, length);

	if (!is_read) {
		if (length)
			mtk_edp_bulk_16bit_write(priv, MTK_DP_AUX_P0_3708, buf,
						 length);

		mtk_edp_mask(priv, MTK_DP_AUX_P0_3704,
			     AUX_TX_FIFO_WDATA_NEW_MODE_T_AUX_TX_P0_MASK,
			     AUX_TX_FIFO_WDATA_NEW_MODE_T_AUX_TX_P0_MASK);
	}

	mtk_edp_mask(priv, MTK_DP_AUX_P0_3630,
		     AUX_TX_REQUEST_READY_AUX_TX_P0,
		     AUX_TX_REQUEST_READY_AUX_TX_P0);

	ret = mtk_edp_aux_wait_for_completion(priv, is_read);

	*reply_cmd = mtk_edp_read(priv, MTK_DP_AUX_P0_3624) &
		     AUX_RX_REPLY_COMMAND_AUX_TX_P0_MASK;

	if (ret) {
		u32 phy_status = mtk_edp_read(priv, MTK_DP_AUX_P0_3628) &
				 AUX_RX_PHY_STATE_AUX_TX_P0_MASK;

		if (phy_status != AUX_RX_PHY_STATE_AUX_TX_P0_RX_IDLE) {
			dev_err(priv->dev, "AUX Rx hang, need SW reset\n");
			return -EIO;
		}

		return -ETIMEDOUT;
	}

	if (!length) {
		mtk_edp_mask(priv, MTK_DP_AUX_P0_362C, 0,
			     AUX_NO_LENGTH_AUX_TX_P0 |
			     AUX_TX_AUXTX_OV_EN_AUX_TX_P0_MASK |
			     AUX_RESERVED_RW_0_AUX_TX_P0_MASK);
	} else if (is_read) {
		int read_delay;

		if (cmd == (DP_AUX_I2C_READ | DP_AUX_I2C_MOT) ||
		    cmd == DP_AUX_I2C_READ)
			read_delay = 500;
		else
			read_delay = 100;

		mtk_edp_aux_read_rx_fifo(priv, buf, length, read_delay);
	}

	return 0;
}

/*
 * Run one AUX transaction, retrying while the sink defers. The hardware FIFO
 * caps a transfer at DP_AUX_MAX_PAYLOAD_BYTES, so callers wanting more have
 * to split the request themselves.
 */
static int mtk_edp_aux_transfer(struct mtk_edp_priv *priv, bool is_read,
				u8 cmd, u32 addr, u8 *buf, size_t length)
{
	bool is_i2c = !(cmd & DP_AUX_NATIVE_WRITE);
	int retries = is_i2c ? MTK_EDP_AUX_I2C_RETRY_COUNT :
			       MTK_EDP_AUX_NATIVE_RETRY_COUNT;
	u8 reply = 0;
	int retry;
	int ret = 0;

	for (retry = 0; retry < retries; retry++) {
		ret = mtk_edp_aux_do_transfer(priv, is_read, cmd, addr, buf,
					      length, &reply);
		if (ret && ret != -ETIMEDOUT)
			return ret;

		if (!ret) {
			/*
			 * An I2C-over-AUX reply carries the native result in
			 * the low bits and the I2C result above it, and the
			 * I2C field only means anything once the native half
			 * says the sink accepted the request at all. Checking
			 * it first would read a native defer, whose I2C field
			 * is zero, as an I2C acknowledgment.
			 */
			switch (reply & DP_AUX_NATIVE_REPLY_MASK) {
			case DP_AUX_NATIVE_REPLY_ACK:
				if (!is_i2c)
					return 0;
				break;
			case DP_AUX_NATIVE_REPLY_DEFER:
				goto defer;
			default:
				/* the sink refused; asking again will not help */
				dev_err(priv->dev,
					"AUX transfer to 0x%05x rejected, reply 0x%02x\n",
					addr, reply);
				return -EIO;
			}

			switch (reply & DP_AUX_I2C_REPLY_MASK) {
			case DP_AUX_I2C_REPLY_ACK:
				return 0;
			case DP_AUX_I2C_REPLY_DEFER:
				break;
			default:
				dev_err(priv->dev,
					"I2C transfer to 0x%05x rejected, reply 0x%02x\n",
					addr, reply);
				return -EIO;
			}
		}

defer:
		udelay(500);
	}

	dev_err(priv->dev, "AUX transfer to 0x%05x failed, reply 0x%02x\n",
		addr, reply);

	return ret ? ret : -EIO;
}

static int mtk_edp_dpcd_read(struct mtk_edp_priv *priv, u32 addr, u8 *buf,
			     size_t length)
{
	size_t done = 0;

	while (done < length) {
		size_t chunk = min_t(size_t, length - done,
				     DP_AUX_MAX_PAYLOAD_BYTES);
		int ret;

		ret = mtk_edp_aux_transfer(priv, true, DP_AUX_NATIVE_READ,
					   addr + done, buf + done, chunk);
		if (ret)
			return ret;

		done += chunk;
	}

	return 0;
}

static int mtk_edp_dpcd_writeb(struct mtk_edp_priv *priv, u32 addr, u8 val)
{
	return mtk_edp_aux_transfer(priv, false, DP_AUX_NATIVE_WRITE, addr,
				    &val, 1);
}

/*
 * Read @length bytes from the EDID EEPROM using the AUX channel's
 * I2C-over-AUX mode: a short write sets the sink's address counter, then the
 * data is read back in FIFO-sized chunks. The last read drops MOT to release
 * the bus.
 */
static int mtk_edp_read_edid_block(struct mtk_edp_priv *priv, u8 *buf,
				   u8 offset, size_t length)
{
	size_t done = 0;
	int ret;

	ret = mtk_edp_aux_transfer(priv, false,
				   DP_AUX_I2C_WRITE | DP_AUX_I2C_MOT,
				   EDID_ADDR, &offset, 1);
	if (ret)
		return ret;

	while (done < length) {
		size_t chunk = min_t(size_t, length - done,
				     DP_AUX_MAX_PAYLOAD_BYTES);
		u8 cmd = DP_AUX_I2C_READ;

		if (done + chunk < length)
			cmd |= DP_AUX_I2C_MOT;

		ret = mtk_edp_aux_transfer(priv, true, cmd, EDID_ADDR,
					   buf + done, chunk);
		if (ret)
			goto err_stop;

		done += chunk;
	}

	return 0;

err_stop:
	/*
	 * Abandoning the burst leaves the sink mid transaction, because the
	 * stop is what dropping MOT on the final read would have generated.
	 * Close it with an empty write so the bus is released.
	 */
	mtk_edp_aux_transfer(priv, false, DP_AUX_I2C_WRITE, EDID_ADDR, NULL, 0);

	return ret;
}

/*
 * Read one block and check it, retrying a block that arrives corrupted. This
 * mirrors edid_block_read() in Linux, which gives each block four attempts:
 * a marginal AUX link corrupts a byte now and then, and one bad byte in the
 * base block otherwise costs the panel the whole boot.
 *
 * A transfer that fails outright is not retried here, because
 * mtk_edp_aux_transfer() has already retried it; only a block that arrives
 * intact but does not check out is worth asking for again.
 */
static int mtk_edp_read_edid_block_checked(struct mtk_edp_priv *priv, u8 *buf,
					   u8 offset, bool is_base_block)
{
	int try, ret;

	for (try = 0; try < MTK_EDP_EDID_READ_TRIES; try++) {
		ret = mtk_edp_read_edid_block(priv, buf, offset, EDID_SIZE);
		if (ret)
			return ret;

		if (is_base_block && edid_check_info((struct edid1_info *)buf)) {
			/*
			 * An all zero base block means nothing answered rather
			 * than that the data was damaged, so do not keep asking.
			 */
			if (!memchr_inv(buf, 0, EDID_SIZE)) {
				dev_err(priv->dev, "the panel returned an empty EDID\n");
				return -ENODEV;
			}

			continue;
		}

		if (!edid_check_checksum(buf))
			return 0;
	}

	dev_err(priv->dev, "EDID block at 0x%x is corrupt after %d tries\n",
		offset, MTK_EDP_EDID_READ_TRIES);

	return -EIO;
}

/*
 * Read the base EDID block, and the first extension block as well if the
 * panel says it has one. Returns the number of bytes read.
 */
static int mtk_edp_read_edid(struct mtk_edp_priv *priv)
{
	const struct edid1_info *info = (struct edid1_info *)priv->edid;
	int ret;

	ret = mtk_edp_read_edid_block_checked(priv, priv->edid, 0, true);
	if (ret)
		return ret;

	if (!info->extension_flag)
		return EDID_SIZE;

	ret = mtk_edp_read_edid_block_checked(priv, priv->edid + EDID_SIZE,
					      EDID_SIZE, false);
	if (ret) {
		/* the base block is enough to pick a mode */
		dev_warn(priv->dev, "cannot read the EDID extension block: %d\n",
			 ret);
		return EDID_SIZE;
	}

	return EDID_EXT_SIZE;
}

static int mtk_edp_phy_configure(struct mtk_edp_priv *priv, u8 link_rate,
				 u8 lane_count)
{
	/*
	 * Spread the link clock only for a panel that says it can track a
	 * spread one, matching what the sink is told through
	 * DP_DOWNSPREAD_CTRL when the link is trained.
	 */
	struct phy_configure_opts_dp opts = {
		.link_rate = link_rate * 270,
		.lanes = lane_count,
		.ssc = priv->train_info.sink_ssc,
		.set_rate = 1,
		.set_lanes = 1,
	};
	int ret;

	ret = generic_phy_configure(&priv->phy, &opts);
	if (ret)
		return ret;

	/* Turn the PHY power on only once it is configured */
	mtk_edp_mask(priv, REG_3FF8_DP_ENC_P0_3, PHY_STATE_W_1_DP_ENC_P0_3,
		     PHY_STATE_W_1_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, MTK_DP_TOP_PWR_STATE, DP_PWR_STATE_BANDGAP_TPLL_LANE,
		     DP_PWR_STATE_MASK);

	return 0;
}

static void mtk_edp_set_swing_pre_emphasis(struct mtk_edp_priv *priv, int lane,
					   int swing, int preemphasis)
{
	u32 shift = lane * DP_TX1_VOLT_SWING_SHIFT;

	dev_dbg(priv->dev, "lane %d: swing 0x%x, pre-emphasis 0x%x\n", lane,
		swing, preemphasis);

	mtk_edp_mask(priv, MTK_DP_TOP_SWING_EMP,
		     swing << (DP_TX0_VOLT_SWING_SHIFT + shift),
		     DP_TX0_VOLT_SWING_MASK << shift);
	mtk_edp_mask(priv, MTK_DP_TOP_SWING_EMP,
		     preemphasis << (DP_TX0_PRE_EMPH_SHIFT + shift),
		     DP_TX0_PRE_EMPH_MASK << shift);
}

static void mtk_edp_reset_swing_pre_emphasis(struct mtk_edp_priv *priv)
{
	u32 mask = 0;
	int lane;

	for (lane = 0; lane < 4; lane++)
		mask |= (DP_TX0_VOLT_SWING_MASK | DP_TX0_PRE_EMPH_MASK) <<
			(lane * DP_TX1_VOLT_SWING_SHIFT);

	mtk_edp_mask(priv, MTK_DP_TOP_SWING_EMP, 0, mask);
}

static void mtk_edp_initialize_settings(struct mtk_edp_priv *priv)
{
	mtk_edp_mask(priv, REG_3F04_DP_ENC_P0_3, 0,
		     FRAME_START_MARKER_0_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3F08_DP_ENC_P0_3, FRAME_START_MARKER_1_DP_ENC_P0_3,
		     FRAME_START_MARKER_1_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3F0C_DP_ENC_P0_3, FRAME_END_MARKER_0_DP_ENC_P0_3,
		     FRAME_END_MARKER_0_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3F10_DP_ENC_P0_3, FRAME_END_MARKER_1_DP_ENC_P0_3,
		     FRAME_END_MARKER_1_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_33C0, 0, SDP_TESTBUS_SEL_DP_ENC_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_33C0, SDP_TESTBUS_SEL_BIT4_DP_ENC,
		     SDP_TESTBUS_SEL_BIT4_DP_ENC_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_33C4, DP_TX_ENCODER_TESTBUS_SEL_DP_ENC,
		     DP_TX_ENCODER_TESTBUS_SEL_DP_ENC_MASK);
	mtk_edp_mask(priv, REG_3F28_DP_ENC_P0_3, DP_TX_SDP_PSR_AS_TESTBUS,
		     DP_TX_SDP_PSR_AS_TESTBUS_MASK);
	mtk_edp_mask(priv, MTK_DP_TOP_RESET_AND_PROBE, RG_SW_RST, RG_SW_RST_MASK);
	mtk_edp_mask(priv, MTK_DP_TOP_RESET_AND_PROBE, RG_PROBE_LOW_SEL,
		     RG_PROBE_LOW_SEL_MASK);
	mtk_edp_mask(priv, MTK_DP_TOP_RESET_AND_PROBE, RG_PROBE_LOW_HIGH_SWAP,
		     RG_PROBE_LOW_HIGH_SWAP);
	mtk_edp_mask(priv, MTK_DP_TOP_IRQ_MASK, ENCODER_IRQ_MSK | TRANS_IRQ_MSK,
		     ENCODER_IRQ_MSK | TRANS_IRQ_MSK);

	mtk_edp_mask(priv, MTK_DP_TRANS_P0_342C, XTAL_FREQ_DP_TRANS_P0_DEFAULT,
		     XTAL_FREQ_DP_TRANS_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_TRANS_P0_3540, FEC_CLOCK_EN_MODE_DP_TRANS_P0,
		     FEC_CLOCK_EN_MODE_DP_TRANS_P0);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_31EC, AUDIO_CH_SRC_SEL_DP_ENC0_P0,
		     AUDIO_CH_SRC_SEL_DP_ENC0_P0);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_304C, 0,
		     SDP_VSYNC_RISING_MASK_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_TOP_IRQ_MASK, IRQ_MASK_AUX_TOP_IRQ,
		     IRQ_MASK_AUX_TOP_IRQ);
}

static void mtk_edp_initialize_aux_settings(struct mtk_edp_priv *priv)
{
	mtk_edp_mask(priv, MTK_DP_AUX_P0_360C, AUX_TIMEOUT_THR_AUX_TX_P0_VAL,
		     AUX_TIMEOUT_THR_AUX_TX_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_AUX_P0_3658, 0, AUX_TX_OV_EN_AUX_TX_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_AUX_P0_36A0, DP_TX_INIT_MASK_15_TO_2,
		     DP_TX_INIT_MASK_15_TO_2_MASK);
	/* 25 for 26M */
	mtk_edp_mask(priv, MTK_DP_AUX_P0_3634,
		     FIELD_PREP(AUX_TX_OVER_SAMPLE_RATE_AUX_TX_P0_MASK,
				AUX_TX_OVER_SAMPLE_RATE_FOR_26M),
		     AUX_TX_OVER_SAMPLE_RATE_AUX_TX_P0_MASK);
	/* 13 for 26M */
	mtk_edp_mask(priv, MTK_DP_AUX_P0_3614, AUX_RX_UI_CNT_THR_FOR_26M,
		     AUX_RX_UI_CNT_THR_AUX_TX_P0_MASK);
	/* the analog top has to be running before any transaction is started */
	mtk_edp_mask(priv, MTK_DP_AUX_P0_37C8, MTK_ATOP_EN_AUX_TX_P0,
		     MTK_ATOP_EN_AUX_TX_P0);
	/* Set complete reply mode for AUX */
	mtk_edp_mask(priv, MTK_DP_AUX_P0_3690,
		     RX_REPLY_COMPLETE_MODE_AUX_TX_P0,
		     RX_REPLY_COMPLETE_MODE_AUX_TX_P0);

	/* connect threshold 1.5ms + 5 x 0.1ms = 2ms */
	mtk_edp_mask(priv, MTK_DP_AUX_P0_367C,
		     FIELD_PREP(HPD_CONN_THD_AUX_TX_P0_FLDMASK,
				HPD_CONN_THD_AUX_TX_P0_VAL),
		     HPD_CONN_THD_AUX_TX_P0_FLDMASK);
	/* disconnect threshold 1.5ms + 5 x 0.1ms = 2ms */
	mtk_edp_mask(priv, MTK_DP_AUX_P0_37A0,
		     FIELD_PREP(HPD_DISC_THD_AUX_TX_P0_FLDMASK,
				HPD_DISC_THD_AUX_TX_P0_VAL),
		     HPD_DISC_THD_AUX_TX_P0_FLDMASK);
	mtk_edp_mask(priv, REG_3FF8_DP_ENC_P0_3,
		     FIELD_PREP(XTAL_FREQ_FOR_PSR_DP_ENC_P0_3_MASK,
				XTAL_FREQ_FOR_PSR_DP_ENC_P0_3_VALUE),
		     XTAL_FREQ_FOR_PSR_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, MTK_DP_AUX_P0_366C,
		     FIELD_PREP(XTAL_FREQ_DP_TX_AUX_366C_MASK,
				XTAL_FREQ_DP_TX_AUX_366C_VALUE),
		     XTAL_FREQ_DP_TX_AUX_366C_MASK);
}

static void mtk_edp_initialize_digital_settings(struct mtk_edp_priv *priv)
{
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_304C, 0,
		     VBID_VIDEO_MUTE_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_3368,
		     FIELD_PREP(BS2BS_MODE_DP_ENC1_P0_MASK,
				BS2BS_MODE_DP_ENC1_P0_VAL),
		     BS2BS_MODE_DP_ENC1_P0_MASK);

	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3000, DP_I_MODE_ENABLE,
		     DP_I_MODE_ENABLE);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3000, REG_BS_SYMBOL_CNT_RESET,
		     REG_BS_SYMBOL_CNT_RESET);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_3368,
		     VIDEO_SRAM_FIFO_CNT_RESET_SEL_DP_ENC1_P0,
		     VIDEO_SRAM_FIFO_CNT_RESET_SEL_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_3368, BS_FOLLOW_SEL_DP_ENC0_P0,
		     BS_FOLLOW_SEL_DP_ENC0_P0);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_303C,
		     SRAM_START_READ_THRD_DP_ENC0_P0_VALUE,
		     SRAM_START_READ_THRD_DP_ENC0_P0_MASK);
	/* disable the PSR pattern generator */
	mtk_edp_mask(priv, REG_3F80_DP_ENC_P0_3, 0, PSR_PATGEN_AVT_EN_FLDMASK);

	/* bring the PHY digital block up */
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, PHY_PWR_STATE_OW_EN_DP_ENC_P0_3,
		     PHY_PWR_STATE_OW_EN_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, ALL_POWER_ON,
		     PHY_PWR_STATE_OW_VALUE_DP_ENC_P0_3_MASK);
	/* wait for the AUX LDO to settle */
	mdelay(100);
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, 0,
		     PHY_PWR_STATE_OW_EN_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3FF8_DP_ENC_P0_3, PHY_STATE_W_1_DP_ENC_P0_3,
		     PHY_STATE_W_1_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3FF8_DP_ENC_P0_3, DVO_ON_W_1_FLDMASK,
		     DVO_ON_W_1_FLDMASK);

	/* reset the encoder */
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3004,
		     DP_TX_ENCODER_4P_RESET_SW_DP_ENC0_P0,
		     DP_TX_ENCODER_4P_RESET_SW_DP_ENC0_P0);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3004, 0,
		     DP_TX_ENCODER_4P_RESET_SW_DP_ENC0_P0);
	mtk_edp_mask(priv, REG_3FF8_DP_ENC_P0_3, PHY_STATE_RESET_ALL_VALUE,
		     PHY_STATE_RESET_ALL_MASK);

	/* Wait for the software reset to complete */
	mdelay(1);
}

static void mtk_edp_initialize_hpd_detect_settings(struct mtk_edp_priv *priv)
{
	mtk_edp_mask(priv, MTK_DP_AUX_P0_364C,
		     FIELD_PREP(HPD_INT_THD_FLDMASK, HPD_INT_THD_FLDMASK_VAL),
		     HPD_INT_THD_FLDMASK);
}

static void mtk_edp_digital_sw_reset(struct mtk_edp_priv *priv)
{
	mtk_edp_mask(priv, MTK_DP_TRANS_P0_340C,
		     DP_TX_TRANSMITTER_4P_RESET_SW_DP_TRANS_P0,
		     DP_TX_TRANSMITTER_4P_RESET_SW_DP_TRANS_P0);

	/* Wait for the reset to complete */
	mdelay(1);

	mtk_edp_mask(priv, MTK_DP_TRANS_P0_340C, 0,
		     DP_TX_TRANSMITTER_4P_RESET_SW_DP_TRANS_P0);
}

static void mtk_edp_set_lanes(struct mtk_edp_priv *priv, int lanes)
{
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, PHY_PWR_STATE_OW_EN_DP_ENC_P0_3,
		     PHY_PWR_STATE_OW_EN_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, BIAS_POWER_ON,
		     PHY_PWR_STATE_OW_VALUE_DP_ENC_P0_3_MASK);
	mdelay(100);
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, 0,
		     PHY_PWR_STATE_OW_EN_DP_ENC_P0_3_MASK);

	mtk_edp_mask(priv, MTK_DP_TRANS_P0_35F0,
		     lanes == 0 ? 0 : DP_TRANS_DUMMY_RW_0,
		     DP_TRANS_DUMMY_RW_0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3000, lanes,
		     LANE_NUM_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_TRANS_P0_34A4,
		     FIELD_PREP(LANE_NUM_DP_TRANS_P0_MASK, lanes),
		     LANE_NUM_DP_TRANS_P0_MASK);
}

static int mtk_edp_power_enable(struct mtk_edp_priv *priv)
{
	mtk_edp_mask(priv, MTK_DP_TOP_RESET_AND_PROBE, 0, SW_RST_B_PHYD);

	/* Wait for power enable */
	udelay(10);

	mtk_edp_mask(priv, MTK_DP_TOP_RESET_AND_PROBE, SW_RST_B_PHYD,
		     SW_RST_B_PHYD);
	mtk_edp_mask(priv, MTK_DP_TOP_PWR_STATE, DP_PWR_STATE_BANDGAP_TPLL,
		     DP_PWR_STATE_MASK);

	return generic_phy_power_on(&priv->phy);
}

static void mtk_edp_power_disable(struct mtk_edp_priv *priv)
{
	mtk_edp_write(priv, MTK_DP_TOP_PWR_STATE, 0);

	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, PHY_PWR_STATE_OW_EN_DP_ENC_P0_3,
		     PHY_PWR_STATE_OW_EN_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, ALL_POWER_OFF,
		     PHY_PWR_STATE_OW_VALUE_DP_ENC_P0_3_MASK);
	mtk_edp_mask(priv, REG_3F44_DP_ENC_P0_3, 0,
		     PHY_PWR_STATE_OW_EN_DP_ENC_P0_3_MASK);

	generic_phy_power_off(&priv->phy);

	mtk_edp_write(priv, MTK_DP_TOP_MEM_PD, 0x550 | FUSE_SEL | MEM_ISO_EN);
}

static void mtk_edp_set_idle_pattern(struct mtk_edp_priv *priv, bool enable)
{
	u32 val = POST_MISC_DATA_LANE0_OV_DP_TRANS_P0_MASK |
		  POST_MISC_DATA_LANE1_OV_DP_TRANS_P0_MASK |
		  POST_MISC_DATA_LANE2_OV_DP_TRANS_P0_MASK |
		  POST_MISC_DATA_LANE3_OV_DP_TRANS_P0_MASK;

	mtk_edp_mask(priv, MTK_DP_TRANS_P0_3580, enable ? val : 0, val);
}

static void mtk_edp_init_port(struct mtk_edp_priv *priv)
{
	mtk_edp_set_idle_pattern(priv, true);

	mtk_edp_initialize_settings(priv);
	mtk_edp_initialize_aux_settings(priv);
	mtk_edp_initialize_digital_settings(priv);
	mtk_edp_initialize_hpd_detect_settings(priv);

	mtk_edp_digital_sw_reset(priv);
	mtk_edp_mask(priv, EDP_TX_TOP_CLKGEN_0, EDP_TX_TOP_CLKGEN_REST_VALUE,
		     EDP_TX_TOP_CLKGEN_REST_MASK);
}

static int mtk_edp_wait_hpd_asserted(struct mtk_edp_priv *priv)
{
	int wait_ms = MTK_EDP_HPD_TIMEOUT_MS;

	while (wait_ms--) {
		if (mtk_edp_read(priv, MTK_DP_AUX_P0_364C) &
		    HPD_STATUS_DP_AUX_TX_P0_MASK)
			return 0;

		mdelay(1);
	}

	return -ETIMEDOUT;
}

/*
 * Move the sink between D0 and D3. The result matters: a sink that never
 * leaves D3 fails every later step, and reporting the first failure is a lot
 * more use than reporting whichever one happens to be noticed first.
 */
static int mtk_edp_aux_panel_poweron(struct mtk_edp_priv *priv, bool pwron)
{
	int ret;

	if (pwron) {
		mtk_edp_mask(priv, MTK_DP_TOP_PWR_STATE,
			     DP_PWR_STATE_BANDGAP_TPLL_LANE, DP_PWR_STATE_MASK);

		ret = mtk_edp_dpcd_writeb(priv, DP_SET_POWER, DP_SET_POWER_D0);
		mdelay(2);
	} else {
		ret = mtk_edp_dpcd_writeb(priv, DP_SET_POWER, DP_SET_POWER_D3);
		mdelay(2);

		mtk_edp_mask(priv, MTK_DP_TOP_PWR_STATE,
			     DP_PWR_STATE_BANDGAP_TPLL, DP_PWR_STATE_MASK);
	}

	return ret;
}

/* --- link training ------------------------------------------------------ */

/*
 * DPCD 0x00e gives the interval the sink wants between writing a training
 * pattern and reading the lane status back, in units of 4 ms. Zero means the
 * sink has no preference, in which case the DP spec's default applies:
 * @default_us, which differs between clock recovery and channel equalisation.
 */
static void mtk_edp_train_delay(struct mtk_edp_priv *priv, u32 default_us)
{
	u8 interval = priv->rx_cap[DP_TRAINING_AUX_RD_INTERVAL] &
		      DP_TRAINING_AUX_RD_MASK;

	if (interval)
		mdelay(interval * 4);
	else
		udelay(default_us);
}

static bool mtk_edp_clock_recovery_ok(const u8 link_status[DP_LINK_STATUS_SIZE],
				      u8 lane_count)
{
	int lane;

	for (lane = 0; lane < lane_count; lane++) {
		u8 status = link_status[lane / 2] >> ((lane % 2) * 4);

		if (!(status & DP_LANE_CR_DONE))
			return false;
	}

	return true;
}

static bool mtk_edp_channel_eq_ok(const u8 link_status[DP_LINK_STATUS_SIZE],
				  u8 lane_count)
{
	int lane;

	if (!(link_status[2] & DP_INTERLANE_ALIGN_DONE))
		return false;

	for (lane = 0; lane < lane_count; lane++) {
		u8 status = link_status[lane / 2] >> ((lane % 2) * 4);

		if ((status & DP_CHANNEL_EQ_BITS) != DP_CHANNEL_EQ_BITS)
			return false;
	}

	return true;
}

static void mtk_edp_train_set_pattern(struct mtk_edp_priv *priv, int pattern)
{
	/* TPS1 doubles as the signal to stop sending the idle pattern */
	if (pattern == 1)
		mtk_edp_set_idle_pattern(priv, false);

	mtk_edp_mask(priv, MTK_DP_TRANS_P0_3400,
		     pattern ? BIT(pattern - 1) << 12 : 0,
		     PATTERN1_EN_DP_TRANS_P0_MASK |
		     PATTERN2_EN_DP_TRANS_P0_MASK |
		     PATTERN3_EN_DP_TRANS_P0_MASK |
		     PATTERN4_EN_DP_TRANS_P0_MASK);
}

static void mtk_edp_training_set_scramble(struct mtk_edp_priv *priv, bool enable)
{
	mtk_edp_mask(priv, MTK_DP_TRANS_P0_3404,
		     enable ? DP_SCR_EN_DP_TRANS_P0_MASK : 0,
		     DP_SCR_EN_DP_TRANS_P0_MASK);
}

/*
 * Apply the drive settings the sink asked for, to the transmitter and back to
 * the sink. A failure here has to be reported: the sink would keep asking for
 * settings it never received, which looks exactly like a link that will not
 * lock and costs a full walk of the rate and lane fallback.
 */
static int mtk_edp_train_update_swing_pre(struct mtk_edp_priv *priv, int lanes,
					  const u8 dpcd_adjust_req[2])
{
	struct phy_configure_opts_dp opts = {
		.lanes = lanes,
		.ssc = priv->train_info.sink_ssc,
		.set_voltages = 1,
	};
	u8 lane_set[4];
	int lane, ret;

	for (lane = 0; lane < lanes; lane++) {
		int shift = lane % 2 ? DP_ADJUST_VOLTAGE_SWING_LANE1_SHIFT : 0;
		u8 req = dpcd_adjust_req[lane / 2] >> shift;
		u8 swing, preemphasis;

		swing = req & DP_ADJUST_VOLTAGE_SWING_LANE0_MASK;
		preemphasis = (req & DP_ADJUST_PRE_EMPHASIS_LANE0_MASK) >>
			      DP_ADJUST_PRE_EMPHASIS_LANE0_SHIFT;
		lane_set[lane] = swing << DP_TRAIN_VOLTAGE_SWING_SHIFT |
				 preemphasis << DP_TRAIN_PRE_EMPHASIS_SHIFT;

		if (swing == DP_TRAIN_VOLTAGE_SWING_LEVEL_3)
			lane_set[lane] |= DP_TRAIN_MAX_SWING_REACHED;
		if (preemphasis == 3)
			lane_set[lane] |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;

		mtk_edp_set_swing_pre_emphasis(priv, lane, swing, preemphasis);
		opts.voltage[lane] = swing;
		opts.pre[lane] = preemphasis;
	}

	ret = generic_phy_configure(&priv->phy, &opts);
	if (ret)
		return ret;

	for (lane = 0; lane < lanes; lane++) {
		ret = mtk_edp_dpcd_writeb(priv, DP_TRAINING_LANE0_SET + lane,
					  lane_set[lane]);
		if (ret)
			return ret;
	}

	return 0;
}

static int mtk_edp_pattern(struct mtk_edp_priv *priv, bool is_tps1)
{
	unsigned int aux_offset;
	int pattern;

	if (is_tps1) {
		pattern = 1;
		aux_offset = DP_LINK_SCRAMBLING_DISABLE | DP_TRAINING_PATTERN_1;
	} else {
		aux_offset = priv->train_info.channel_eq_pattern;

		switch (priv->train_info.channel_eq_pattern) {
		case DP_TRAINING_PATTERN_4:
			pattern = 4;
			break;
		case DP_TRAINING_PATTERN_3:
			pattern = 3;
			aux_offset |= DP_LINK_SCRAMBLING_DISABLE;
			break;
		case DP_TRAINING_PATTERN_2:
		default:
			pattern = 2;
			aux_offset |= DP_LINK_SCRAMBLING_DISABLE;
			break;
		}
	}

	mtk_edp_train_set_pattern(priv, pattern);

	return mtk_edp_dpcd_writeb(priv, DP_TRAINING_PATTERN_SET, aux_offset);
}

static int mtk_edp_train_setting(struct mtk_edp_priv *priv, u8 target_link_rate,
				 u8 target_lane_count)
{
	int ret;

	/*
	 * Configuring the PHY for a rate the sink was never told about would
	 * train against a sink still set up for the previous attempt, so give
	 * up here rather than run the loops below to exhaustion.
	 */
	ret = mtk_edp_dpcd_writeb(priv, DP_LINK_BW_SET, target_link_rate);
	if (ret)
		return ret;

	ret = mtk_edp_dpcd_writeb(priv, DP_LANE_COUNT_SET,
				  target_lane_count |
				  DP_LANE_COUNT_ENHANCED_FRAME_EN);
	if (ret)
		return ret;

	if (priv->train_info.sink_ssc) {
		ret = mtk_edp_dpcd_writeb(priv, DP_DOWNSPREAD_CTRL,
					  DP_SPREAD_AMP_0_5);
		if (ret)
			return ret;
	}

	mtk_edp_set_lanes(priv, target_lane_count / 2);
	ret = mtk_edp_phy_configure(priv, target_link_rate, target_lane_count);
	if (ret)
		return ret;

	dev_dbg(priv->dev, "training at link rate 0x%x, %u lanes\n",
		target_link_rate, target_lane_count);

	return 0;
}

static int mtk_edp_train_cr(struct mtk_edp_priv *priv, u8 target_lane_count)
{
	u8 link_status[DP_LINK_STATUS_SIZE] = { };
	u8 lane_adjust[2] = { };
	u8 prev_lane_adjust = 0xff;
	int voltage_retries = 0;
	int train_retries = 0;
	int ret;

	ret = mtk_edp_pattern(priv, true);
	if (ret)
		goto out;

	/* The DP 1.4 spec puts the clock recovery retry count at 10. */
	do {
		train_retries++;

		ret = mtk_edp_dpcd_read(priv, DP_ADJUST_REQUEST_LANE0_1,
					lane_adjust, sizeof(lane_adjust));
		if (ret)
			goto out;

		ret = mtk_edp_train_update_swing_pre(priv, target_lane_count,
						     lane_adjust);
		if (ret)
			goto out;

		mtk_edp_train_delay(priv, 100);

		/*
		 * Bail out rather than carry on with a stale link_status: it
		 * still reads as "not done", so the loop would run to
		 * exhaustion at every rate and lane count before giving up.
		 */
		ret = mtk_edp_dpcd_read(priv, DP_LANE0_1_STATUS, link_status,
					sizeof(link_status));
		if (ret)
			goto out;

		if (mtk_edp_clock_recovery_ok(link_status, target_lane_count)) {
			dev_dbg(priv->dev, "link train CR pass\n");
			return 0;
		}

		/*
		 * The spec asks for five retries at an unchanged voltage level
		 * before giving up, and there is no point retrying at all once
		 * the maximum level has been reached.
		 */
		if (prev_lane_adjust == link_status[4]) {
			voltage_retries++;
			if (voltage_retries > MTK_EDP_TRAIN_VOLTAGE_LEVEL_RETRY ||
			    (prev_lane_adjust & DP_ADJUST_VOLTAGE_SWING_LANE0_MASK) == 3) {
				dev_dbg(priv->dev, "link train CR fail\n");
				break;
			}
		} else {
			voltage_retries = 0;
		}
		prev_lane_adjust = link_status[4];
	} while (train_retries < MTK_EDP_TRAIN_DOWNSCALE_RETRY);

	/*
	 * -EAGAIN, not -ETIMEDOUT: the AUX channel reports its own timeouts
	 * that way, and the caller has to be able to tell "the link did not
	 * lock, try a slower one" from "the sink stopped answering".
	 */
	ret = -EAGAIN;

out:
	/* already failing, so the sink not taking this too changes nothing */
	mtk_edp_dpcd_writeb(priv, DP_TRAINING_PATTERN_SET,
			    DP_TRAINING_PATTERN_DISABLE);
	mtk_edp_train_set_pattern(priv, 0);

	return ret;
}

static int mtk_edp_train_eq(struct mtk_edp_priv *priv, u8 target_lane_count)
{
	u8 link_status[DP_LINK_STATUS_SIZE] = { };
	u8 lane_adjust[2] = { };
	int train_retries = 0;
	int pattern_ret;
	int ret;

	ret = mtk_edp_pattern(priv, false);
	if (ret)
		goto out;

	do {
		train_retries++;

		ret = mtk_edp_dpcd_read(priv, DP_ADJUST_REQUEST_LANE0_1,
					lane_adjust, sizeof(lane_adjust));
		if (ret)
			goto out;

		ret = mtk_edp_train_update_swing_pre(priv, target_lane_count,
						     lane_adjust);
		if (ret)
			goto out;

		mtk_edp_train_delay(priv, 400);

		ret = mtk_edp_dpcd_read(priv, DP_LANE0_1_STATUS, link_status,
					sizeof(link_status));
		if (ret)
			goto out;

		if (mtk_edp_channel_eq_ok(link_status, target_lane_count)) {
			dev_dbg(priv->dev, "link train EQ pass\n");
			ret = 0;
			goto out;
		}

		dev_dbg(priv->dev, "link train EQ fail\n");
	} while (train_retries < MTK_EDP_TRAIN_DOWNSCALE_RETRY);

	/* as in mtk_edp_train_cr(): distinct from an AUX timeout */
	ret = -EAGAIN;

out:
	/*
	 * The pattern has to come off the link either way: on success because
	 * the video stream follows, on failure so the sink is not left waiting
	 * for one. On success the result matters - a sink still in training
	 * mode shows nothing, and the backlight would come up on a blank
	 * panel - while a path that is already failing has nothing to gain.
	 */
	pattern_ret = mtk_edp_dpcd_writeb(priv, DP_TRAINING_PATTERN_SET,
					  DP_TRAINING_PATTERN_DISABLE);
	mtk_edp_train_set_pattern(priv, 0);

	return ret ? ret : pattern_ret;
}

static int mtk_edp_parse_capabilities(struct mtk_edp_priv *priv)
{
	int ret;

	ret = mtk_edp_dpcd_read(priv, DP_DPCD_REV, priv->rx_cap,
				sizeof(priv->rx_cap));
	if (ret)
		return ret;

	/*
	 * The capability bits are only meaningful from the revision that
	 * defined them; on an older sink they are reserved and may read as
	 * set, which would pick a pattern the sink cannot lock to. This
	 * mirrors drm_dp_tps4_supported() and drm_dp_tps3_supported().
	 */
	if (priv->rx_cap[DP_DPCD_REV] >= 0x14 &&
	    (priv->rx_cap[DP_MAX_DOWNSPREAD] & DP_TPS4_SUPPORTED))
		priv->train_info.channel_eq_pattern = DP_TRAINING_PATTERN_4;
	else if (priv->rx_cap[DP_DPCD_REV] >= 0x12 &&
		 (priv->rx_cap[DP_MAX_LANE_COUNT] & DP_TPS3_SUPPORTED))
		priv->train_info.channel_eq_pattern = DP_TRAINING_PATTERN_3;
	else
		priv->train_info.channel_eq_pattern = DP_TRAINING_PATTERN_2;

	/*
	 * Tolerating a spread clock is mandatory from DP 1.1 on, so the
	 * capability bit only has to be consulted for older sinks - plenty of
	 * 1.1 and later panels leave it clear and spread anyway. This mirrors
	 * drm_dp_max_downspread().
	 */
	priv->train_info.sink_ssc = priv->rx_cap[DP_DPCD_REV] >= 0x11 ||
				    (priv->rx_cap[DP_MAX_DOWNSPREAD] &
				     DP_MAX_DOWNSPREAD_0_5);

	return 0;
}

/* The highest rate and lane count both ends are willing to run at. */
static void mtk_edp_max_link(struct mtk_edp_priv *priv, u8 *link_rate,
			     u8 *lane_count)
{
	*link_rate = min_t(u8, priv->max_linkrate,
			   priv->rx_cap[DP_MAX_LINK_RATE]);
	*lane_count = min_t(u8, priv->max_lanes,
			    priv->rx_cap[DP_MAX_LANE_COUNT] &
			    DP_MAX_LANE_COUNT_MASK);
}

/*
 * Check that a link of @link_rate and @lane_count can carry the mode.
 *
 * A DP_LINK_BW_* code counts 0.27 Gbps steps, and a lane transports one byte
 * per symbol, so the code names the per lane byte rate in units of 27000 kB/s
 * directly. Nothing downstream can report that the mode does not fit:
 * mtk_edp_setup_tu() would take the impossible ratio, the encoder FIFO would
 * underrun, and the panel would come up black or torn with nothing logged.
 */
static int mtk_edp_check_bandwidth(struct mtk_edp_priv *priv, u8 link_rate,
				   u8 lane_count)
{
	u32 available = (u32)link_rate * 27000 * lane_count;
	u32 required = priv->timing.pixelclock.typ / 1000 *
		       MTK_EDP_BITS_PER_PIXEL / 8;

	if (available < required) {
		dev_err(priv->dev,
			"%ux%u needs %u kB/s, a %u lane link at 0x%02x carries %u\n",
			priv->timing.hactive.typ, priv->timing.vactive.typ,
			required, lane_count, link_rate, available);
		return -ENOSPC;
	}

	return 0;
}

static int mtk_edp_training(struct mtk_edp_priv *priv)
{
	u8 lane_count, link_rate, max_link_rate, train_limit;
	int ret;

	mtk_edp_max_link(priv, &link_rate, &lane_count);
	max_link_rate = link_rate;

	/*
	 * The training patterns come from the hardware pattern generator,
	 * which needs scrambling turned off while it drives the link.
	 */
	mtk_edp_training_set_scramble(priv, false);

	for (train_limit = 6; train_limit > 0; train_limit--) {
		ret = generic_phy_reset(&priv->phy);
		if (ret)
			return ret;

		mtk_edp_reset_swing_pre_emphasis(priv);

		ret = mtk_edp_train_setting(priv, link_rate, lane_count);
		if (ret)
			return ret;

		/*
		 * Only a link that trained and did not lock is worth retrying
		 * at a lower rate. Anything else means the AUX channel itself
		 * is gone, and walking the whole ladder would just repeat the
		 * same failure a dozen times over.
		 */
		ret = mtk_edp_train_cr(priv, lane_count);
		if (ret && ret != -EAGAIN)
			return ret;

		if (ret) {
			/* drop to the next lower link rate, then halve the lanes */
			switch (link_rate) {
			case DP_LINK_BW_1_62:
				lane_count /= 2;
				link_rate = max_link_rate;
				if (!lane_count)
					return -EIO;
				break;
			case DP_LINK_BW_2_7:
				link_rate = DP_LINK_BW_1_62;
				break;
			case DP_LINK_BW_5_4:
				link_rate = DP_LINK_BW_2_7;
				break;
			case DP_LINK_BW_8_1:
				link_rate = DP_LINK_BW_5_4;
				break;
			default:
				return -EINVAL;
			}
			continue;
		}

		ret = mtk_edp_train_eq(priv, lane_count);
		if (ret && ret != -EAGAIN)
			return ret;

		if (ret) {
			/*
			 * Halve first and give up on nothing left, rather than
			 * testing the count before halving it: training zero
			 * lanes reports success, because every per lane check
			 * passes vacuously, and the transfer unit setup then
			 * divides by the lane count.
			 */
			lane_count /= 2;
			if (!lane_count)
				return -EIO;
			continue;
		}

		/* getting here means training is done */
		break;
	}

	if (!train_limit)
		return -ETIMEDOUT;

	priv->train_info.link_rate = link_rate;
	priv->train_info.lane_count = lane_count;

	/* Normal stream output needs the scrambler back on. */
	mtk_edp_training_set_scramble(priv, true);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3000, ENHANCED_FRAME_EN_DP_ENC0_P0,
		     ENHANCED_FRAME_EN_DP_ENC0_P0);

	return 0;
}

/* --- video -------------------------------------------------------------- */

static void mtk_edp_msa_bypass_enable(struct mtk_edp_priv *priv, bool enable)
{
	u32 mask = HTOTAL_SEL_DP_ENC0_P0 | VTOTAL_SEL_DP_ENC0_P0 |
		   HSTART_SEL_DP_ENC0_P0 | VSTART_SEL_DP_ENC0_P0 |
		   HWIDTH_SEL_DP_ENC0_P0 | VHEIGHT_SEL_DP_ENC0_P0 |
		   HSP_SEL_DP_ENC0_P0 | HSW_SEL_DP_ENC0_P0 |
		   VSP_SEL_DP_ENC0_P0 | VSW_SEL_DP_ENC0_P0;

	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3030, enable ? 0 : mask, mask);
}

static void mtk_edp_set_msa(struct mtk_edp_priv *priv)
{
	const struct display_timing *t = &priv->timing;
	u32 htotal = t->hactive.typ + t->hfront_porch.typ + t->hsync_len.typ +
		     t->hback_porch.typ;
	u32 vtotal = t->vactive.typ + t->vfront_porch.typ + t->vsync_len.typ +
		     t->vback_porch.typ;

	/* horizontal */
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3010, htotal, HTOTAL_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3018,
		     t->hsync_len.typ + t->hback_porch.typ,
		     HSTART_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3028, t->hsync_len.typ,
		     HSW_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3028, 0, HSP_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3020, t->hactive.typ,
		     HWIDTH_SW_DP_ENC0_P0_MASK);

	/* vertical */
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3014, vtotal, VTOTAL_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_301C,
		     t->vsync_len.typ + t->vback_porch.typ,
		     VSTART_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_302C, t->vsync_len.typ,
		     VSW_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_302C, 0, VSP_SW_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3024, t->vactive.typ,
		     VHEIGHT_SW_DP_ENC0_P0_MASK);

	/* pattern generator, horizontal */
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3064, t->hactive.typ,
		     HDE_NUM_LAST_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3154, htotal,
		     PGEN_HTOTAL_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3158, t->hfront_porch.typ,
		     PGEN_HSYNC_RISING_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_315C, t->hsync_len.typ,
		     PGEN_HSYNC_PULSE_WIDTH_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3160,
		     t->hback_porch.typ + t->hsync_len.typ + t->hfront_porch.typ,
		     PGEN_HFDE_START_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3164, t->hactive.typ,
		     PGEN_HFDE_ACTIVE_WIDTH_DP_ENC0_P0_MASK);

	/* pattern generator, vertical */
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3168, vtotal,
		     PGEN_VTOTAL_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_316C, t->vfront_porch.typ,
		     PGEN_VSYNC_RISING_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3170, t->vsync_len.typ,
		     PGEN_VSYNC_PULSE_WIDTH_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3174,
		     t->vback_porch.typ + t->vsync_len.typ + t->vfront_porch.typ,
		     PGEN_VFDE_START_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3178, t->vactive.typ,
		     PGEN_VFDE_ACTIVE_WIDTH_DP_ENC0_P0_MASK);
}

static void mtk_edp_sdp_set_down_cnt_init(struct mtk_edp_priv *priv,
					  u32 sram_read_start)
{
	const struct display_timing *t = &priv->timing;
	u32 vtotal = t->vactive.typ + t->vfront_porch.typ + t->vsync_len.typ +
		     t->vback_porch.typ;
	u32 sdp_down_cnt_init = 0;
	u32 clock_khz = t->pixelclock.typ / 1000;

	if (clock_khz)
		sdp_down_cnt_init = sram_read_start *
				    priv->train_info.link_rate * 2700 * 8 /
				    (clock_khz * 4);

	switch (priv->train_info.lane_count) {
	case 1:
		sdp_down_cnt_init = max_t(u32, sdp_down_cnt_init, 0x1a);
		break;
	case 2:
		/* for a low resolution with a high audio sample rate */
		sdp_down_cnt_init = max_t(u32, sdp_down_cnt_init, 0x10);
		sdp_down_cnt_init += vtotal <= 525 ? 4 : 0;
		break;
	case 4:
	default:
		sdp_down_cnt_init = max_t(u32, sdp_down_cnt_init, 6);
		break;
	}

	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3040, sdp_down_cnt_init,
		     SDP_DOWN_CNT_INIT_DP_ENC0_P0_MASK);
}

static void mtk_edp_sdp_set_down_cnt_init_in_hblank(struct mtk_edp_priv *priv)
{
	const struct display_timing *t = &priv->timing;
	u32 vtotal = t->vactive.typ + t->vfront_porch.typ + t->vsync_len.typ +
		     t->vback_porch.typ;
	u32 pix_clk_mhz = t->pixelclock.typ / 1000000;
	u32 sdp_down_cnt_init;
	u32 dc_offset;

	switch (priv->train_info.lane_count) {
	case 1:
		sdp_down_cnt_init = 0x20;
		break;
	case 2:
		dc_offset = vtotal <= 525 ? 0x14 : 0x00;
		sdp_down_cnt_init = 0x18 + dc_offset;
		break;
	case 4:
	default:
		dc_offset = vtotal <= 525 ? 0x08 : 0x00;
		if (pix_clk_mhz > priv->train_info.link_rate * 27)
			sdp_down_cnt_init = 0x8;
		else
			sdp_down_cnt_init = 0x10 + dc_offset;
		break;
	}

	mtk_edp_mask(priv, MTK_DP_ENC1_P0_3364, sdp_down_cnt_init,
		     SDP_DOWN_CNT_INIT_IN_HBLANK_DP_ENC1_P0_MASK);
}

static void mtk_edp_setup_encoder(struct mtk_edp_priv *priv)
{
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_303C, VIDEO_MN_GEN_EN_DP_ENC0_P0,
		     VIDEO_MN_GEN_EN_DP_ENC0_P0);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3040, SDP_DOWN_CNT_DP_ENC0_P0_VAL,
		     SDP_DOWN_CNT_INIT_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_3364,
		     SDP_DOWN_CNT_IN_HBLANK_DP_ENC1_P0_VAL,
		     SDP_DOWN_CNT_INIT_IN_HBLANK_DP_ENC1_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_3300,
		     FIELD_PREP(VIDEO_AFIFO_RDY_SEL_DP_ENC1_P0_MASK,
				VIDEO_AFIFO_RDY_SEL_DP_ENC1_P0_VAL),
		     VIDEO_AFIFO_RDY_SEL_DP_ENC1_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC1_P0_3364,
		     FIELD_PREP(FIFO_READ_START_POINT_DP_ENC1_P0_MASK,
				FIFO_READ_START_POINT_DP_ENC1_P0_VAL),
		     FIFO_READ_START_POINT_DP_ENC1_P0_MASK);
}

static void mtk_edp_setup_tu(struct mtk_edp_priv *priv)
{
	u32 sram_read_start = min_t(u32, MTK_EDP_TBC_BUF_READ_START_ADDR,
				    priv->timing.hactive.typ /
				    priv->train_info.lane_count /
				    MTK_EDP_4P1T / MTK_EDP_HDE /
				    MTK_EDP_PIX_PER_ADDR);

	mtk_edp_mask(priv, MTK_DP_ENC0_P0_303C, sram_read_start,
		     SRAM_START_READ_THRD_DP_ENC0_P0_MASK);
	mtk_edp_setup_encoder(priv);
	mtk_edp_sdp_set_down_cnt_init_in_hblank(priv);
	mtk_edp_sdp_set_down_cnt_init(priv, sram_read_start);
}

static void mtk_edp_video_mute(struct mtk_edp_priv *priv, bool enable)
{
	struct arm_smccc_res res;
	u32 val = VIDEO_MUTE_SEL_DP_ENC0_P0 |
		  (enable ? VIDEO_MUTE_SW_DP_ENC0_P0 : 0);

	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3000, val,
		     VIDEO_MUTE_SEL_DP_ENC0_P0 | VIDEO_MUTE_SW_DP_ENC0_P0);

	arm_smccc_smc(MTK_SIP_DP_CONTROL, MTK_DP_SIP_ATF_EDP_SET_VID_MUTE, enable,
		      (MTK_DP_SIP_ATF_EDP_SET_VID_MUTE << 16) | enable,
		      MTK_DP_SIP_REQ_VIDEO_UNMUTE,
		      0, 0, 0, &res);

	/*
	 * Firmware that does not implement the call leaves the video muted in
	 * the secure world, which looks exactly like a working boot with a
	 * blank panel, so say something rather than fail.
	 */
	if (!enable && res.a0)
		dev_warn(priv->dev, "secure unmute refused: 0x%lx-0x%lx\n",
			 res.a0, res.a1);
	else
		dev_dbg(priv->dev, "video %s: smc ret 0x%lx-0x%lx\n",
			enable ? "mute" : "unmute", res.a0, res.a1);
}

static void mtk_edp_pg_enable(struct mtk_edp_priv *priv, bool enable)
{
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3038,
		     enable ? VIDEO_SOURCE_SEL_DP_ENC0_P0_MASK : 0,
		     VIDEO_SOURCE_SEL_DP_ENC0_P0_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_31B0,
		     FIELD_PREP(PGEN_PATTERN_SEL_MASK, PGEN_PATTERN_SEL_VAL),
		     PGEN_PATTERN_SEL_MASK);
}

static void mtk_edp_video_config(struct mtk_edp_priv *priv)
{
	/* 0: hardware mode, 1: software mode */
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3004, 0,
		     VIDEO_M_CODE_SEL_DP_ENC0_P0_MASK);

	mtk_edp_set_msa(priv);

	/* Only 8 bits per colour is supported */
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3034, DP_MSA_MISC_8_BPC,
		     DP_TEST_BIT_DEPTH_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_303C,
		     VIDEO_COLOR_DEPTH_DP_ENC0_P0_8BIT,
		     VIDEO_COLOR_DEPTH_DP_ENC0_P0_MASK);

	mtk_edp_mask(priv, MTK_DP_ENC0_P0_3034, DP_COLOR_FORMAT_RGB,
		     DP_TEST_COLOR_FORMAT_MASK);
	mtk_edp_mask(priv, MTK_DP_ENC0_P0_303C,
		     PIXEL_ENCODE_FORMAT_DP_ENC0_P0_RGB,
		     PIXEL_ENCODE_FORMAT_DP_ENC0_P0_MASK);
}

static void mtk_edp_video_enable(struct mtk_edp_priv *priv, bool enable)
{
	/* the mute sequence differs between enable and disable */
	if (enable) {
		mtk_edp_msa_bypass_enable(priv, false);
		mtk_edp_pg_enable(priv, false);
		mtk_edp_setup_tu(priv);
		mtk_edp_video_mute(priv, false);
	} else {
		mtk_edp_video_mute(priv, true);
		mtk_edp_pg_enable(priv, true);
		mtk_edp_msa_bypass_enable(priv, true);
	}
}

/* --- driver model ------------------------------------------------------- */

/*
 * The display path muxes live in the mmsys syscon block. It is bound as a
 * clock provider rather than a pipeline component, so reach it by compatible
 * and take its register base directly, the way mtk_hdmi does for vdosys1.
 */
static int mtk_edp_get_mmsys_base(struct mtk_edp_priv *priv)
{
	struct udevice *mmsys;
	struct regmap *regmap;
	ofnode node;
	int ret;

	node = ofnode_by_compatible(ofnode_null(), priv->data->mmsys_compatible);
	if (!ofnode_valid(node)) {
		dev_err(priv->dev, "cannot find mmsys node\n");
		return -ENODEV;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_CLK, node, &mmsys);
	if (ret) {
		dev_err(priv->dev, "cannot get mmsys syscon: %d\n", ret);
		return ret;
	}

	regmap = syscon_node_to_regmap(dev_ofnode(mmsys));
	if (IS_ERR(regmap)) {
		ret = PTR_ERR(regmap);
		dev_err(priv->dev, "cannot get mmsys regmap: %d\n", ret);
		return ret;
	}

	priv->mmsys_base = regmap_get_range(regmap, 0);
	if (!priv->mmsys_base) {
		dev_err(priv->dev, "cannot map the mmsys registers\n");
		return -ENOENT;
	}

	return 0;
}

/*
 * Look up a display pipeline component by its device tree alias. Only the
 * lookup happens here; each component is ungated later with
 * mtk_disp_comp_enable(), which also enables the SMI larb of the ones that
 * read DRAM.
 */
static int mtk_edp_get_comp_by_alias(struct udevice *dev, const char *alias,
				     struct udevice **compp)
{
	int ret;

	ret = uclass_get_device_by_of_path(UCLASS_MISC, alias, compp);
	if (ret)
		dev_err(dev, "cannot get %s: %d\n", alias, ret);

	return ret;
}

/*
 * The panel hangs off the transmitter's AUX bus. U-Boot has no eDP panel
 * driver, and everything this needs from the panel comes from the EDID, so
 * only its supply and backlight are picked up from the device tree.
 */
static int mtk_edp_get_panel_resources(struct mtk_edp_priv *priv)
{
	ofnode aux_bus, panel, node;
	int ret;

	aux_bus = dev_read_subnode(priv->dev, "aux-bus");
	if (!ofnode_valid(aux_bus))
		return 0;

	panel = ofnode_find_subnode(aux_bus, "panel");
	if (!ofnode_valid(panel))
		return 0;

	node = ofnode_parse_phandle(panel, "power-supply", 0);
	if (ofnode_valid(node)) {
		ret = uclass_get_device_by_ofnode(UCLASS_REGULATOR, node,
						  &priv->panel_supply);
		if (ret) {
			dev_err(priv->dev, "cannot get panel supply: %d\n", ret);
			return ret;
		}
	}

	node = ofnode_parse_phandle(panel, "backlight", 0);
	if (ofnode_valid(node)) {
		ret = uclass_get_device_by_ofnode(UCLASS_PANEL_BACKLIGHT, node,
						  &priv->backlight);
		if (ret) {
			dev_err(priv->dev, "cannot get backlight: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static int mtk_edp_probe(struct udevice *dev)
{
	struct mtk_edp_priv *priv = dev_get_priv(dev);
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	ofnode endpoint, dvo_node;
	u8 link_rate, lane_count;
	u32 linkrate;
	int len, bpc, ret;

	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	priv->dev = dev;
	priv->data = (const struct mtk_edp_data *)dev_get_driver_data(dev);

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs) {
		dev_err(dev, "cannot map the transmitter registers\n");
		return -EINVAL;
	}

	ret = generic_phy_get_by_index(dev, 0, &priv->phy);
	if (ret) {
		dev_err(dev, "cannot get the PHY: %d\n", ret);
		return ret;
	}

	/* The lane count comes from the output endpoint, as the binding says. */
	endpoint = ofnode_graph_get_endpoint_by_regs(dev_ofnode(dev), 1, -1);
	if (!ofnode_valid(endpoint)) {
		dev_err(dev, "no output endpoint\n");
		return -EINVAL;
	}

	len = ofnode_read_size(endpoint, "data-lanes");
	if (len < 0) {
		dev_err(dev, "no data-lanes property\n");
		return -EINVAL;
	}

	len /= sizeof(u32);
	if (len < 1 || len > 4 || len == 3) {
		dev_err(dev, "invalid data lane count: %d\n", len);
		return -EINVAL;
	}
	priv->max_lanes = len;

	ret = dev_read_u32(dev, "max-linkrate-mhz", &linkrate);
	if (ret) {
		dev_err(dev, "failed to read max linkrate: %d\n", ret);
		return ret;
	}

	switch (linkrate) {
	case 1620:
		priv->max_linkrate = DP_LINK_BW_1_62;
		break;
	case 2700:
		priv->max_linkrate = DP_LINK_BW_2_7;
		break;
	case 5400:
		priv->max_linkrate = DP_LINK_BW_5_4;
		break;
	case 8100:
		priv->max_linkrate = DP_LINK_BW_8_1;
		break;
	default:
		dev_err(dev, "invalid max linkrate %u\n", linkrate);
		return -EINVAL;
	}

	/* The DVO feeding us is the far side of the input endpoint. */
	dvo_node = ofnode_graph_get_remote_node(dev_ofnode(dev), 0, -1);
	if (!ofnode_valid(dvo_node)) {
		dev_err(dev, "cannot find the DVO node\n");
		return -ENODEV;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MISC, dvo_node, &priv->dvo);
	if (ret) {
		dev_err(dev, "cannot get DVO device: %d\n", ret);
		return ret;
	}

	/*
	 * Probing the DVO only claims its clocks; they still have to be
	 * ungated, the same way the components fetched by alias below are.
	 */
	ret = mtk_disp_comp_enable(priv->dvo);
	if (ret) {
		dev_err(dev, "cannot enable DVO clocks: %d\n", ret);
		return ret;
	}

	ret = mtk_edp_get_mmsys_base(priv);
	if (ret)
		goto err_dvo_disable;

	ret = mtk_edp_get_comp_by_alias(dev, "ovl0", &priv->ovl);
	if (ret)
		goto err_dvo_disable;

	ret = mtk_edp_get_comp_by_alias(dev, "rdma0", &priv->rdma);
	if (ret)
		goto err_dvo_disable;

	ret = mtk_edp_get_comp_by_alias(dev, "mutex0", &priv->mutex);
	if (ret)
		goto err_dvo_disable;

	ret = mtk_edp_get_panel_resources(priv);
	if (ret)
		goto err_dvo_disable;

	if (priv->panel_supply) {
		ret = regulator_set_enable_if_allowed(priv->panel_supply, true);
		if (ret) {
			dev_err(dev, "cannot enable panel supply: %d\n", ret);
			goto err_dvo_disable;
		}

		mdelay(MTK_EDP_PANEL_HPD_RELIABLE_MS);
	}

	/* the order matters: the AUX channel is not usable until all three */
	ret = generic_phy_init(&priv->phy);
	if (ret) {
		dev_err(dev, "cannot initialize the PHY: %d\n", ret);
		goto err_supply_disable;
	}

	mtk_edp_init_port(priv);

	ret = mtk_edp_power_enable(priv);
	if (ret) {
		dev_err(dev, "cannot power on the PHY: %d\n", ret);
		goto err_power_off;
	}

	ret = mtk_edp_wait_hpd_asserted(priv);
	if (ret) {
		dev_err(dev, "no panel detected on the eDP link\n");
		goto err_power_off;
	}

	ret = mtk_edp_aux_panel_poweron(priv, true);
	if (ret) {
		dev_err(dev, "cannot bring the panel out of D3: %d\n", ret);
		goto err_power_off;
	}

	ret = mtk_edp_parse_capabilities(priv);
	if (ret) {
		dev_err(dev, "cannot read the sink capabilities: %d\n", ret);
		goto err_power_off;
	}

	ret = mtk_edp_read_edid(priv);
	if (ret < 0) {
		dev_err(dev, "cannot read the panel EDID: %d\n", ret);
		goto err_power_off;
	}

	ret = edid_get_timing(priv->edid, ret, &priv->timing, &bpc);
	if (ret) {
		dev_err(dev, "cannot parse the panel EDID: %d\n", ret);
		goto err_power_off;
	}

	if (priv->timing.hactive.typ > MTK_EDP_MAX_WIDTH ||
	    priv->timing.vactive.typ > MTK_EDP_MAX_HEIGHT) {
		dev_err(dev, "panel mode %ux%u is too large\n",
			priv->timing.hactive.typ, priv->timing.vactive.typ);
		ret = -EINVAL;
		goto err_power_off;
	}

	/*
	 * Check the mode against the best link both ends advertise before
	 * spending several seconds training one that could never carry it.
	 */
	mtk_edp_max_link(priv, &link_rate, &lane_count);
	ret = mtk_edp_check_bandwidth(priv, link_rate, lane_count);
	if (ret)
		goto err_power_off;

	ret = mtk_edp_training(priv);
	if (ret) {
		dev_err(dev, "link training failed: %d\n", ret);
		goto err_power_off;
	}

	/*
	 * Training may have fallen back to a lower rate or fewer lanes, so the
	 * mode has to be checked again against what the link actually reached.
	 */
	ret = mtk_edp_check_bandwidth(priv, priv->train_info.link_rate,
				      priv->train_info.lane_count);
	if (ret)
		goto err_power_off;

	dev_dbg(dev, "%ux%u, pixel clock %u Hz, link rate 0x%02x, %u lanes\n",
		priv->timing.hactive.typ, priv->timing.vactive.typ,
		priv->timing.pixelclock.typ, priv->train_info.link_rate,
		priv->train_info.lane_count);

	uc_priv->bpix = VIDEO_BPP32;
	uc_priv->xsize = priv->timing.hactive.typ;
	uc_priv->ysize = priv->timing.vactive.typ;
	uc_priv->line_length = uc_priv->xsize * VNBYTES(VIDEO_BPP32);

	/*
	 * The mutex holds the pipeline's own clock, so it has to be ungated
	 * before its registers are written.
	 */
	ret = mtk_disp_comp_enable(priv->mutex);
	if (ret) {
		dev_err(dev, "failed to enable mutex: %d\n", ret);
		goto err_power_off;
	}

	ret = mtk_disp_mutex_ovl_dvo_enable(priv->mutex);
	if (ret) {
		dev_err(dev, "failed to set up mutex: %d\n", ret);
		goto err_mutex_disable;
	}

	priv->data->ddp_connect(priv->mmsys_base);

	/*
	 * Ungate the overlay and RDMA engines and open their path to DRAM
	 * before programming them: their registers need the engine clock, and
	 * mtk_ovl_config() resets the block. Enabling the component is also
	 * what enables its SMI larb, which issues the ATF call that takes
	 * these ports out of IOMMU translation, so scanout cannot reach the
	 * framebuffer without it.
	 */
	ret = mtk_disp_comp_enable(priv->ovl);
	if (ret) {
		dev_err(dev, "failed to enable ovl: %d\n", ret);
		goto err_mutex_disable;
	}

	ret = mtk_disp_comp_enable(priv->rdma);
	if (ret) {
		dev_err(dev, "failed to enable rdma: %d\n", ret);
		goto err_ovl_disable;
	}

	mtk_ovl_config(priv->ovl, &priv->timing);
	mtk_ovl_start(priv->ovl);

	mtk_rdma_config(priv->rdma, &priv->timing);
	mtk_rdma_start(priv->rdma);

	ret = mtk_ovl_layer(priv->ovl, plat, &priv->timing);
	if (ret) {
		dev_err(dev, "failed to set up the overlay layer: %d\n", ret);
		goto err_rdma_disable;
	}

	/*
	 * Bring the DVO up before the transmitter it feeds, matching the order
	 * drm_atomic_bridge_chain_enable() walks the chain in: the DVO
	 * attaches the transmitter as its next bridge, so the pixel stream is
	 * running by the time the encoder unmutes.
	 */
	ret = mtk_dvo_config(priv->dvo, &priv->timing);
	if (ret)
		goto err_rdma_disable;

	mtk_dvo_hw_enable(priv->dvo);

	mtk_edp_video_config(priv);
	mtk_edp_video_enable(priv, true);

	video_set_flush_dcache(dev, true);

	if (priv->backlight) {
		/* let the panel latch a valid frame before lighting it up */
		mdelay(MTK_EDP_PANEL_ENABLE_MS);

		ret = backlight_enable(priv->backlight);
		if (ret) {
			dev_err(dev, "failed to enable backlight: %d\n", ret);
			goto err_rdma_disable;
		}
	}

	return 0;

err_rdma_disable:
	mtk_disp_comp_disable(priv->rdma);

err_ovl_disable:
	mtk_disp_comp_disable(priv->ovl);

err_mutex_disable:
	mtk_disp_comp_disable(priv->mutex);

err_power_off:
	mtk_edp_power_disable(priv);
	generic_phy_exit(&priv->phy);

err_supply_disable:
	if (priv->panel_supply)
		regulator_set_enable_if_allowed(priv->panel_supply, false);

err_dvo_disable:
	mtk_disp_comp_disable(priv->dvo);

	return ret;
}

static int mtk_edp_remove(struct udevice *dev)
{
	struct mtk_edp_priv *priv = dev_get_priv(dev);

	/* Nothing to undo if we never made it past the pre-relocation stub. */
	if (!priv->regs)
		return 0;

	/*
	 * Hand the link back to the OS in a quiescent state: the kernel's
	 * mtk_dp driver retrains from scratch and expects to find the
	 * transmitter powered down rather than actively scanning out.
	 *
	 * Turn the backlight off first, the order the panel's disable runs in
	 * ahead of the bridge's in DRM: muting the video switches the pattern
	 * generator in as the source, so anything still lit would show it.
	 */
	if (priv->backlight)
		backlight_set_brightness(priv->backlight, BACKLIGHT_OFF);
	mtk_edp_video_enable(priv, false);

	/* let the sink settle on a muted link before the transmitter goes */
	mdelay(MTK_EDP_VIDEO_MUTE_SETTLE_MS);

	mtk_edp_aux_panel_poweron(priv, false);
	mtk_edp_power_disable(priv);
	generic_phy_exit(&priv->phy);

	/*
	 * The panel has just been put into D3 over AUX. Leaving its supply on
	 * would only get the OS driver's regulator_enable() refcounted onto an
	 * already enabled rail rather than power cycling the panel, so nothing
	 * would take it back out of D3.
	 */
	if (priv->panel_supply)
		regulator_set_enable_if_allowed(priv->panel_supply, false);

	return 0;
}

static int mtk_edp_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = MTK_EDP_MAX_WIDTH * MTK_EDP_MAX_HEIGHT *
		     VNBYTES(VIDEO_BPP32);

	return 0;
}

/* the MT8189 mmsys muxes are the MT8366 ones */
static const struct mtk_edp_data mt8189_edp_data = {
	.mmsys_compatible = "mediatek,mt8189-mmsys",
	.ddp_connect = mt8366_ddp_ovl_to_dvo,
};

static const struct udevice_id mtk_edp_ids[] = {
	{
		.compatible = "mediatek,mt8189-edp-tx",
		.data = (ulong)&mt8189_edp_data,
	},
	{ }
};

U_BOOT_DRIVER(mtk_edp) = {
	.name = "mtk_edp",
	.id = UCLASS_VIDEO,
	.of_match = mtk_edp_ids,
	.probe = mtk_edp_probe,
	.bind = mtk_edp_bind,
	.remove = mtk_edp_remove,
	.priv_auto = sizeof(struct mtk_edp_priv),
	.flags = DM_FLAG_PRE_RELOC | DM_FLAG_OS_PREPARE,
};
