// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2020-2022 Intel Corporation <www.intel.com>
 *
 */

#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <fs_loader.h>
#include <errno.h>
#include <fdtdec.h>
#include <hang.h>
#include <ram.h>
#include <reset.h>
#include "sdram_soc64.h"
#include <wait_bit.h>
#include <asm/arch/firewall.h>
#include <asm/arch/handoff_soc64.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>
#include <dm/ofnode.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <u-boot/crc.h>
#include <u-boot/sha512.h>
#include <u-boot/schedule.h>

DECLARE_GLOBAL_DATA_PTR;

/* PHY calibration data backup attribute */
#define SOC64_OCRAM_PHY_BACKUP_BASE		0xFFE5E400
/* "SKIP" */
#define SOC64_CRAM_PHY_BACKUP_SKIP_MAGIC	0x50494B53
#define BAC_CAL_MMC_RAW_OFFSET (CONFIG_ENV_SIZE + CONFIG_ENV_OFFSET)

/* DDR handoff SHA384 attribute */
#define DDR_HANDOFF_IMG_ADDR	0xFFE44000
#define DDR_HANDOFF_IMG_LEN	0x1A000
#define CHUNKSZ_PER_WD_RESET	(256 * 1024)

/* MPFE NOC registers */
#define FPGA2SDRAM_MGR_MAIN_SIDEBANDMGR_FLAGOUTSET0	0xF8024050

/* Memory reset manager */
#define MEM_RST_MGR_STATUS	0x8

/* Register and bit in memory reset manager */
#define MEM_RST_MGR_STATUS_RESET_COMPLETE	BIT(0)
#define MEM_RST_MGR_STATUS_PWROKIN_STATUS	BIT(1)
#define MEM_RST_MGR_STATUS_CONTROLLER_RST	BIT(2)
#define MEM_RST_MGR_STATUS_AXI_RST		BIT(3)

#define TIMEOUT_200MS     200
#define TIMEOUT_5000MS    5000

/* DDR4 umctl2 */
#define DDR4_MSTR_OFFSET		0x0
#define DDR4_FREQ_RATIO			BIT(22)

#define DDR4_STAT_OFFSET		0x4
#define DDR4_STAT_SELFREF_TYPE		GENMASK(5, 4)
#define DDR4_STAT_SELFREF_TYPE_SHIFT	4
#define DDR4_STAT_OPERATING_MODE	GENMASK(2, 0)

#define DDR4_MRCTRL0_OFFSET		0x10
#define DDR4_MRCTRL0_MR_TYPE		BIT(0)
#define DDR4_MRCTRL0_MPR_EN		BIT(1)
#define DDR4_MRCTRL0_MR_RANK		GENMASK(5, 4)
#define DDR4_MRCTRL0_MR_RANK_SHIFT	4
#define DDR4_MRCTRL0_MR_ADDR		GENMASK(15, 12)
#define DDR4_MRCTRL0_MR_ADDR_SHIFT	12
#define DDR4_MRCTRL0_MR_WR		BIT(31)

#define DDR4_MRCTRL1_OFFSET		0x14
#define DDR4_MRCTRL1_MR_DATA		0x3FFFF

#define DDR4_MRSTAT_OFFSET		0x18
#define DDR4_MRSTAT_MR_WR_BUSY		BIT(0)

#define DDR4_MRCTRL2_OFFSET		0x1C

#define DDR4_PWRCTL_OFFSET			0x30
#define DDR4_PWRCTL_SELFREF_EN			BIT(0)
#define DDR4_PWRCTL_POWERDOWN_EN		BIT(1)
#define DDR4_PWRCTL_EN_DFI_DRAM_CLK_DISABLE	BIT(3)
#define DDR4_PWRCTL_SELFREF_SW			BIT(5)

#define DDR4_PWRTMG_OFFSET		0x34
#define DDR4_HWLPCTL_OFFSET		0x38
#define DDR4_RFSHCTL0_OFFSET		0x50
#define DDR4_RFSHCTL1_OFFSET		0x54

#define DDR4_RFSHCTL3_OFFSET			0x60
#define DDR4_RFSHCTL3_DIS_AUTO_REFRESH		BIT(0)
#define DDR4_RFSHCTL3_REFRESH_MODE		GENMASK(6, 4)
#define DDR4_RFSHCTL3_REFRESH_MODE_SHIFT	4

#define DDR4_ECCCFG0_OFFSET		0x70
#define DDR4_ECC_MODE			GENMASK(2, 0)
#define DDR4_DIS_SCRUB			BIT(4)
#define LPDDR4_ECCCFG0_ECC_REGION_MAP_GRANU_SHIFT	30
#define LPDDR4_ECCCFG0_ECC_REGION_MAP_SHIFT	8

#define DDR4_ECCCFG1_OFFSET		0x74
#define LPDDR4_ECCCFG1_ECC_REGIONS_PARITY_LOCK	BIT(4)

#define DDR4_CRCPARCTL0_OFFSET			0xC0
#define DDR4_CRCPARCTL0_DFI_ALERT_ERR_INIT_CLR	BIT(1)

#define DDR4_CRCPARCTL1_OFFSET			0xC4
#define DDR4_CRCPARCTL1_CRC_PARITY_RETRY_ENABLE	BIT(8)
#define DDR4_CRCPARCTL1_ALERT_WAIT_FOR_SW	BIT(9)

#define DDR4_CRCPARSTAT_OFFSET			0xCC
#define DDR4_CRCPARSTAT_DFI_ALERT_ERR_INT	BIT(16)
#define DDR4_CRCPARSTAT_DFI_ALERT_ERR_FATL_INT	BIT(17)
#define DDR4_CRCPARSTAT_DFI_ALERT_ERR_NO_SW	BIT(19)
#define DDR4_CRCPARSTAT_CMD_IN_ERR_WINDOW	BIT(29)

#define DDR4_INIT0_OFFSET			0xD0
#define DDR4_INIT0_SKIP_RAM_INIT		GENMASK(31, 30)

#define DDR4_RANKCTL_OFFSET			0xF4
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP		GENMASK(7, 4)
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP		GENMASK(11, 8)
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP_MSB	BIT(24)
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP_MSB	BIT(26)
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP_SHIFT	4
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP_SHIFT	8
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP_MSB_SHIFT	24
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP_MSB_SHIFT	26

#define DDR4_RANKCTL1_OFFSET	0xF8
#define DDR4_RANKCTL1_WR2RD_DR	GENMASK(5, 0)

#define DDR4_DRAMTMG2_OFFSET	0x108
#define DDR4_DRAMTMG2_WR2RD	GENMASK(5, 0)
#define DDR4_DRAMTMG2_RD2WR	GENMASK(13, 8)
#define DDR4_DRAMTMG2_RD2WR_SHIFT	8

#define DDR4_DRAMTMG9_OFFSET	0x124
#define DDR4_DRAMTMG9_W2RD_S	GENMASK(5, 0)

#define DDR4_DFITMG1_OFFSET	0x194
#define DDR4_DFITMG1_DFI_T_WRDATA_DELAY	GENMASK(20, 16)
#define DDR4_DFITMG1_DFI_T_WRDATA_SHIFT	16

#define DDR4_DFIMISC_OFFSET			0x1B0
#define DDR4_DFIMISC_DFI_INIT_COMPLETE_EN	BIT(0)
#define DDR4_DFIMISC_DFI_INIT_START		BIT(5)

#define DDR4_DFISTAT_OFFSET		0x1BC
#define DDR4_DFI_INIT_COMPLETE		BIT(0)

#define DDR4_DBG0_OFFSET		0x300

#define DDR4_DBG1_OFFSET		0x304
#define DDR4_DBG1_DISDQ			BIT(0)
#define DDR4_DBG1_DIS_HIF		BIT(1)

#define DDR4_DBGCAM_OFFSET			0x308
#define DDR4_DBGCAM_DBG_RD_Q_EMPTY		BIT(25)
#define DDR4_DBGCAM_DBG_WR_Q_EMPTY		BIT(26)
#define DDR4_DBGCAM_RD_DATA_PIPELINE_EMPTY	BIT(28)
#define DDR4_DBGCAM_WR_DATA_PIPELINE_EMPTY	BIT(29)

#define DDR4_SWCTL_OFFSET		0x320
#define DDR4_SWCTL_SW_DONE		BIT(0)

#define DDR4_SWSTAT_OFFSET		0x324
#define DDR4_SWSTAT_SW_DONE_ACK		BIT(0)

#define DDR4_PSTAT_OFFSET		0x3FC
#define DDR4_PSTAT_RD_PORT_BUSY_0	BIT(0)
#define DDR4_PSTAT_WR_PORT_BUSY_0	BIT(16)

#define DDR4_PCTRL0_OFFSET		0x490
#define DDR4_PCTRL0_PORT_EN		BIT(0)

#define DDR4_SBRCTL_OFFSET		0xF24
#define DDR4_SBRCTL_SCRUB_INTERVAL	0x1FFF00
#define DDR4_SBRCTL_SCRUB_EN		BIT(0)
#define DDR4_SBRCTL_SCRUB_WRITE		BIT(2)
#define DDR4_SBRCTL_SCRUB_BURST_1	BIT(4)

#define DDR4_SBRSTAT_OFFSET		0xF28
#define DDR4_SBRSTAT_SCRUB_BUSY BIT(0)
#define DDR4_SBRSTAT_SCRUB_DONE BIT(1)

#define DDR4_SBRWDATA0_OFFSET		0xF2C
#define DDR4_SBRWDATA1_OFFSET		0xF30
#define DDR4_SBRSTART0_OFFSET		0xF38
#define DDR4_SBRSTART1_OFFSET		0xF3C
#define DDR4_SBRRANGE0_OFFSET		0xF40
#define DDR4_SBRRANGE1_OFFSET		0xF44

/* DDR PHY */
#define DDR_PHY_TXODTDRVSTREN_B0_P0		0x2009A
#define DDR_PHY_RXPBDLYTG0_R0			0x200D0
#define DDR_PHY_DBYTE0_TXDQDLYTG0_U0_P0		0x201A0

#define DDR_PHY_DBYTE0_TXDQDLYTG0_U1_P0		0x203A0
#define DDR_PHY_DBYTE1_TXDQDLYTG0_U0_P0		0x221A0
#define DDR_PHY_DBYTE1_TXDQDLYTG0_U1_P0		0x223A0
#define DDR_PHY_TXDQDLYTG0_COARSE_DELAY		GENMASK(9, 6)
#define DDR_PHY_TXDQDLYTG0_COARSE_DELAY_SHIFT	6

#define DDR_PHY_CALRATE_OFFSET			0x40110
#define DDR_PHY_CALZAP_OFFSET			0x40112
#define DDR_PHY_SEQ0BDLY0_P0_OFFSET		0x40016
#define DDR_PHY_SEQ0BDLY1_P0_OFFSET		0x40018
#define DDR_PHY_SEQ0BDLY2_P0_OFFSET		0x4001A
#define DDR_PHY_SEQ0BDLY3_P0_OFFSET		0x4001C

#define DDR_PHY_MEMRESETL_OFFSET		0x400C0
#define DDR_PHY_MEMRESETL_VALUE			BIT(0)
#define DDR_PHY_PROTECT_MEMRESET		BIT(1)

#define DDR_PHY_CALBUSY_OFFSET			0x4012E
#define DDR_PHY_CALBUSY				BIT(0)

#define DDR_PHY_TRAIN_IMEM_OFFSET		0xA0000
#define DDR_PHY_TRAIN_DMEM_OFFSET		0xA8000

#define DMEM_MB_CDD_RR_1_0_OFFSET		0xA802C
#define DMEM_MB_CDD_RR_0_1_OFFSET		0xA8030
#define DMEM_MB_CDD_WW_1_0_OFFSET		0xA8038
#define DMEM_MB_CDD_WW_0_1_OFFSET		0xA803C
#define DMEM_MB_CDD_RW_1_1_OFFSET		0xA8046
#define DMEM_MB_CDD_RW_1_0_OFFSET		0xA8048
#define DMEM_MB_CDD_RW_0_1_OFFSET		0xA804A
#define DMEM_MB_CDD_RW_0_0_OFFSET		0xA804C

#define DMEM_MB_CDD_CHA_RR_1_0_OFFSET		0xA8026
#define DMEM_MB_CDD_CHA_RR_0_1_OFFSET		0xA8026
#define DMEM_MB_CDD_CHB_RR_1_0_OFFSET		0xA8058
#define DMEM_MB_CDD_CHB_RR_0_1_OFFSET		0xA805A
#define DMEM_MB_CDD_CHA_WW_1_0_OFFSET		0xA8030
#define DMEM_MB_CDD_CHA_WW_0_1_OFFSET		0xA8030
#define DMEM_MB_CDD_CHB_WW_1_0_OFFSET		0xA8062
#define DMEM_MB_CDD_CHB_WW_0_1_OFFSET		0xA8064

#define DMEM_MB_CDD_CHA_RW_1_1_OFFSET		0xA8028
#define DMEM_MB_CDD_CHA_RW_1_0_OFFSET		0xA8028
#define DMEM_MB_CDD_CHA_RW_0_1_OFFSET		0xA802A
#define DMEM_MB_CDD_CHA_RW_0_0_OFFSET		0xA802A

#define DMEM_MB_CDD_CHB_RW_1_1_OFFSET		0xA805A
#define DMEM_MB_CDD_CHB_RW_1_0_OFFSET		0xA805C
#define DMEM_MB_CDD_CHB_RW_0_1_OFFSET		0xA805c
#define DMEM_MB_CDD_CHB_RW_0_0_OFFSET		0xA805E

#define DDR_PHY_SEQ0DISABLEFLAG0_OFFSET		0x120018
#define DDR_PHY_SEQ0DISABLEFLAG1_OFFSET		0x12001A
#define DDR_PHY_SEQ0DISABLEFLAG2_OFFSET		0x12001C
#define DDR_PHY_SEQ0DISABLEFLAG3_OFFSET		0x12001E
#define DDR_PHY_SEQ0DISABLEFLAG4_OFFSET		0x120020
#define DDR_PHY_SEQ0DISABLEFLAG5_OFFSET		0x120022
#define DDR_PHY_SEQ0DISABLEFLAG6_OFFSET		0x120024
#define DDR_PHY_SEQ0DISABLEFLAG7_OFFSET		0x120026

#define DDR_PHY_UCCLKHCLKENABLES_OFFSET		0x180100
#define DDR_PHY_UCCLKHCLKENABLES_UCCLKEN	BIT(0)
#define DDR_PHY_UCCLKHCLKENABLES_HCLKEN		BIT(1)

#define DDR_PHY_UCTWRITEPROT_OFFSET		0x180066
#define DDR_PHY_UCTWRITEPROT			BIT(0)

#define DDR_PHY_APBONLY0_OFFSET			0x1A0000
#define DDR_PHY_MICROCONTMUXSEL			BIT(0)

#define DDR_PHY_UCTSHADOWREGS_OFFSET			0x1A0008
#define DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW	BIT(0)

#define DDR_PHY_DCTWRITEPROT_OFFSET		0x1A0062
#define DDR_PHY_DCTWRITEPROT			BIT(0)

#define DDR_PHY_UCTWRITEONLYSHADOW_OFFSET	0x1A0064
#define DDR_PHY_UCTDATWRITEONLYSHADOW_OFFSET	0x1A0068

#define DDR_PHY_MICRORESET_OFFSET		0x1A0132
#define DDR_PHY_MICRORESET_STALL		BIT(0)
#define DDR_PHY_MICRORESET_RESET		BIT(3)

#define DDR_PHY_TXODTDRVSTREN_B0_P1		0x22009A

/* For firmware training */
#define HW_DBG_TRACE_CONTROL_OFFSET	0x18
#define FW_TRAINING_COMPLETED_STAT	0x07
#define FW_TRAINING_FAILED_STAT		0xFF
#define FW_COMPLETION_MSG_ONLY_MODE	0xFF
#define FW_STREAMING_MSG_ID		0x08
#define GET_LOWHW_DATA(x)		((x) & 0xFFFF)
#define GET_LOWB_DATA(x)		((x) & 0xFF)
#define GET_HIGHB_DATA(x)		(((x) & 0xFF00) >> 8)

/* Operating mode */
#define OPM_INIT			0x000
#define OPM_NORMAL			0x001
#define OPM_PWR_D0WN			0x010
#define OPM_SELF_SELFREF		0x011
#define OPM_DDR4_DEEP_PWR_DOWN		0x100

/* Refresh mode */
#define FIXED_1X		0
#define FIXED_2X		BIT(0)
#define FIXED_4X		BIT(4)

/* Address of mode register */
#define MR0	0x0000
#define MR1	0x0001
#define MR2	0x0010
#define MR3	0x0011
#define MR4	0x0100
#define MR5	0x0101
#define MR6	0x0110
#define MR7	0x0111

/* MR rank */
#define RANK0		0x1
#define RANK1		0x2
#define ALL_RANK	0x3

#define MR5_BIT4	BIT(4)

/* Value for ecc_region_map */
#define ALL_PROTECTED	0x7F

/* Region size for ECCCFG0.ecc_region_map */
enum region_size {
	ONE_EIGHT,
	ONE_SIXTEENTH,
	ONE_THIRTY_SECOND,
	ONE_SIXTY_FOURTH
};

enum ddr_type {
	DDRTYPE_LPDDR4_0,
	DDRTYPE_LPDDR4_1,
	DDRTYPE_DDR4,
	DDRTYPE_UNKNOWN
};

/* Reset type */
enum reset_type {
	POR_RESET,
	WARM_RESET,
	COLD_RESET,
	NCONFIG,
	JTAG_CONFIG,
	RSU_RECONFIG
};

/* DDR handoff structure */
struct ddr_handoff {
	/* Memory reset manager base */
	phys_addr_t mem_reset_base;

	/* First controller attributes */
	phys_addr_t cntlr_handoff_base;
	phys_addr_t cntlr_base;
	size_t cntlr_total_length;
	enum ddr_type cntlr_t;
	int cntlr_handoff_length;

	/* Second controller attributes*/
	phys_addr_t cntlr2_handoff_base;
	phys_addr_t cntlr2_base;
	size_t cntlr2_total_length;
	enum ddr_type cntlr2_t;
	int cntlr2_handoff_length;

	/* PHY attributes */
	phys_addr_t phy_handoff_base;
	phys_addr_t phy_base;
	size_t phy_total_length;
	int phy_handoff_length;

	/* PHY engine attributes */
	phys_addr_t phy_engine_handoff_base;
	size_t phy_engine_total_length;
	int phy_engine_handoff_length;

	/* Calibration attributes */
	phys_addr_t train_imem_base;
	phys_addr_t train_dmem_base;
	size_t train_imem_length;
	size_t train_dmem_length;
};

/* Message mode */
enum message_mode {
	MAJOR_MESSAGE,
	STREAMING_MESSAGE
};

enum data_type {
	HEADER_ONLY,
	HEADER_DATA
};

/* Header for backup calibration data */
struct cal_header_t {
	u32 header_magic;
	u32 data_len;
	u32 caldata_crc32;
	u8 ddrconfig_hash[SHA384_SUM_LEN];
} __packed;

/* Header + backup calibration data */
struct bac_cal_data_t {
	struct cal_header_t header;
	u16 data;
} __packed;

enum data_process {
	STORE,
	LOADING
};

bool is_ddr_init_hang(void)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD8);

	if (reg & ALT_SYSMGR_SCRATCH_REG_8_DDR_PROGRESS_MASK)
		return true;

	return false;
}

bool is_ddr_dbe_triggered(void)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD8);

	if (reg & ALT_SYSMGR_SCRATCH_REG_8_DDR_DBE_MASK)
		return true;

	return false;
}

void ddr_init_inprogress(bool start)
{
	if (start)
		setbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_BOOT_SCRATCH_COLD8,
			     ALT_SYSMGR_SCRATCH_REG_8_DDR_PROGRESS_MASK);
	else
		clrbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_BOOT_SCRATCH_COLD8,
			     ALT_SYSMGR_SCRATCH_REG_8_DDR_PROGRESS_MASK);
}

bool is_ddr_retention_enabled(u32 reg)
{
	if (reg & ALT_SYSMGR_SCRATCH_REG_0_DDR_RETENTION_MASK)
		return true;

	return false;
}

static enum reset_type get_reset_type(u32 reg)
{
	return (reg & ALT_SYSMGR_SCRATCH_REG_0_DDR_RESET_TYPE_MASK) >>
		ALT_SYSMGR_SCRATCH_REG_0_DDR_RESET_TYPE_SHIFT;
}

void reset_type_print(enum reset_type reset_t)
{
	switch (reset_t) {
	case POR_RESET:
		printf("%s: POR is triggered\n", __func__);
		break;
	case WARM_RESET:
		printf("%s: Warm reset is triggered\n", __func__);
		break;
	case COLD_RESET:
		printf("%s: Cold reset is triggered\n", __func__);
		break;
	case NCONFIG:
		printf("%s: NCONFIG is triggered\n", __func__);
		break;
	case JTAG_CONFIG:
		printf("%s: JTAG_CONFIG is triggered\n", __func__);
		break;
	case RSU_RECONFIG:
		printf("%s: RSU_RECONFIG is triggered\n", __func__);
		break;
	default:
		printf("%s: Invalid reset type\n", __func__);
	}
}

bool is_ddr_init_skipped(u32 reg, bool is_ddr_hang_be4_rst)
{
	enum reset_type reset_t = get_reset_type(reg);

	reset_type_print(reset_t);

	if (!is_ddr_dbe_triggered() && !is_ddr_hang_be4_rst) {
		if (reset_t == WARM_RESET) {
			debug("%s: DDR init is skipped\n", __func__);
			return true;
		}

		if (reset_t == COLD_RESET) {
			if (is_ddr_retention_enabled(reg)) {
				debug("%s: DDR retention bit is set\n",
				      __func__);
				debug("%s: DDR init is skipped\n", __func__);
				return true;
			}
		}
	}

	debug("%s: DDR init is required\n", __func__);
	return false;
}

bool is_ddr_calibration_skipped(u32 reg, bool is_ddr_hang_be4_rst)
{
	enum reset_type reset_t = get_reset_type(reg);

	if ((reset_t == NCONFIG || reset_t == JTAG_CONFIG ||
	     reset_t == RSU_RECONFIG) && is_ddr_retention_enabled(reg) &&
	     !is_ddr_dbe_triggered() && !is_ddr_hang_be4_rst) {
		debug("%s: DDR retention bit is set\n", __func__);
		return true;
	}

	debug("%s: DDR calibration is required\n", __func__);
	return false;
}

static int clr_ca_parity_error_status(phys_addr_t umctl2_base)
{
	int ret;

	debug("%s: Clear C/A parity error status in MR5[4]\n", __func__);

	/* Set mode register MRS */
	clrbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET, DDR4_MRCTRL0_MPR_EN);

	/* Set mode register to write operation */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET, DDR4_MRCTRL0_MR_TYPE);

	/* Set the address of mode rgister to 0x101(MR5) */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET,
		     (MR5 << DDR4_MRCTRL0_MR_ADDR_SHIFT) &
		     DDR4_MRCTRL0_MR_ADDR);

	/* Set MR rank to rank 1 */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET,
		     (RANK1 << DDR4_MRCTRL0_MR_RANK_SHIFT) &
		     DDR4_MRCTRL0_MR_RANK);

	/* Clear C/A parity error status in MR5[4] */
	clrbits_le32(umctl2_base + DDR4_MRCTRL1_OFFSET, MR5_BIT4);

	/* Trigger mode register read or write operation */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET, DDR4_MRCTRL0_MR_WR);

	/* Wait for retry done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_MRSTAT_OFFSET), DDR4_MRSTAT_MR_WR_BUSY,
				false, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" no outstanding MR transaction\n");
		return ret;
	}

	return 0;
}

static int ddr_retry_software_sequence(phys_addr_t umctl2_base)
{
	u32 value;
	int ret;

	/* Check software can perform MRS/MPR/PDA? */
	value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
		      DDR4_CRCPARSTAT_DFI_ALERT_ERR_NO_SW;

	if (value) {
		/* Clear interrupt bit for DFI alert error */
		setbits_le32(umctl2_base + DDR4_CRCPARCTL0_OFFSET,
			     DDR4_CRCPARCTL0_DFI_ALERT_ERR_INIT_CLR);
	}

	debug("%s: Software can perform MRS/MPR/PDA\n", __func__);

	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_MRSTAT_OFFSET),
				DDR4_MRSTAT_MR_WR_BUSY,
				false, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" no outstanding MR transaction\n");
		return ret;
	}

	ret = clr_ca_parity_error_status(umctl2_base);
	if (ret)
		return ret;

	if (!value) {
		/* Clear interrupt bit for DFI alert error */
		setbits_le32(umctl2_base + DDR4_CRCPARCTL0_OFFSET,
			     DDR4_CRCPARCTL0_DFI_ALERT_ERR_INIT_CLR);
	}

	return 0;
}

static int ensure_retry_procedure_complete(phys_addr_t umctl2_base)
{
	u32 value;
	u32 start = get_timer(0);
	int ret;

	/* Check parity/crc/error window is emptied ? */
	value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
		      DDR4_CRCPARSTAT_CMD_IN_ERR_WINDOW;

	/* Polling until parity/crc/error window is emptied */
	while (value) {
		if (get_timer(start) > TIMEOUT_200MS) {
			debug("%s: Timeout while waiting for",
			      __func__);
			debug(" parity/crc/error window empty\n");
			return -ETIMEDOUT;
		}

		/* Check software intervention is enabled? */
		value = readl(umctl2_base + DDR4_CRCPARCTL1_OFFSET) &
			      DDR4_CRCPARCTL1_ALERT_WAIT_FOR_SW;
		if (value) {
			debug("%s: Software intervention is enabled\n",
			      __func__);

			/* Check dfi alert error interrupt is set? */
			value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
				      DDR4_CRCPARSTAT_DFI_ALERT_ERR_INT;

			if (value) {
				ret = ddr_retry_software_sequence(umctl2_base);
				debug("%s: DFI alert error interrupt ",
				      __func__);
				debug("is set\n");

				if (ret)
					return ret;
			}

			/*
			 * Check fatal parity error interrupt is set?
			 */
			value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
				      DDR4_CRCPARSTAT_DFI_ALERT_ERR_FATL_INT;
			if (value) {
				printf("%s: Fatal parity error  ",
				       __func__);
				printf("interrupt is set, Hang it!!\n");
				hang();
			}
		}

		value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
			      DDR4_CRCPARSTAT_CMD_IN_ERR_WINDOW;

		udelay(1);
		schedule();
	}

	return 0;
}

static int enable_quasi_dynamic_reg_grp3(phys_addr_t umctl2_base,
					 enum ddr_type umctl2_type)
{
	u32 i, value, backup;
	int ret = 0;

	/* Disable input traffic per port */
	clrbits_le32(umctl2_base + DDR4_PCTRL0_OFFSET, DDR4_PCTRL0_PORT_EN);

	/* Polling AXI port until idle */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_PSTAT_OFFSET),
				DDR4_PSTAT_WR_PORT_BUSY_0 |
				DDR4_PSTAT_RD_PORT_BUSY_0, false,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" controller idle\n");
		return ret;
	}

	/* Backup user setting */
	backup = readl(umctl2_base + DDR4_DBG1_OFFSET);

	/* Disable input traffic to the controller */
	setbits_le32(umctl2_base + DDR4_DBG1_OFFSET, DDR4_DBG1_DIS_HIF);

	/*
	 * Ensure CAM/data pipelines are empty.
	 * Poll until CAM/data pipelines are set at least twice,
	 * timeout at 200ms
	 */
	for (i = 0; i < 2; i++) {
		ret = wait_for_bit_le32((const void *)(umctl2_base +
					DDR4_DBGCAM_OFFSET),
					DDR4_DBGCAM_WR_DATA_PIPELINE_EMPTY |
					DDR4_DBGCAM_RD_DATA_PIPELINE_EMPTY |
					DDR4_DBGCAM_DBG_WR_Q_EMPTY |
					DDR4_DBGCAM_DBG_RD_Q_EMPTY, true,
					TIMEOUT_200MS, false);
		if (ret) {
			debug("%s: loop(%u): Timeout while waiting for",
			      __func__, i + 1);
			debug(" CAM/data pipelines are empty\n");

			goto out;
		}
	}

	if (umctl2_type == DDRTYPE_DDR4) {
		/* Check DDR4 retry is enabled ? */
		value = readl(umctl2_base + DDR4_CRCPARCTL1_OFFSET) &
			      DDR4_CRCPARCTL1_CRC_PARITY_RETRY_ENABLE;

		if (value) {
			debug("%s: DDR4 retry is enabled\n", __func__);

			ret = ensure_retry_procedure_complete(umctl2_base);
			if (ret) {
				debug("%s: Timeout while waiting for",
				      __func__);
				debug(" retry procedure complete\n");

				goto out;
			}
		}
	}

	debug("%s: Quasi-dynamic group 3 registers are enabled\n", __func__);

out:
	/* Restore user setting */
	writel(backup, umctl2_base + DDR4_DBG1_OFFSET);

	return ret;
}

static enum ddr_type get_ddr_type(phys_addr_t ddr_type_location)
{
	u32 ddr_type_magic = readl(ddr_type_location);

	if (ddr_type_magic == SOC64_HANDOFF_DDR_UMCTL2_DDR4_TYPE)
		return DDRTYPE_DDR4;

	if (ddr_type_magic == SOC64_HANDOFF_DDR_UMCTL2_LPDDR4_0_TYPE)
		return DDRTYPE_LPDDR4_0;

	if (ddr_type_magic == SOC64_HANDOFF_DDR_UMCTL2_LPDDR4_1_TYPE)
		return DDRTYPE_LPDDR4_1;

	return DDRTYPE_UNKNOWN;
}

static void use_lpddr4_interleaving(bool set)
{
	if (set) {
		printf("Starting LPDDR4 interleaving configuration ...\n");
		setbits_le32(FPGA2SDRAM_MGR_MAIN_SIDEBANDMGR_FLAGOUTSET0,
			     BIT(5));
	} else {
		printf("Starting LPDDR4 non-interleaving configuration ...\n");
		clrbits_le32(FPGA2SDRAM_MGR_MAIN_SIDEBANDMGR_FLAGOUTSET0,
			     BIT(5));
	}
}

static void use_ddr4(enum ddr_type type)
{
	if (type == DDRTYPE_DDR4) {
		printf("Starting DDR4 configuration ...\n");
		setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_DDR_MODE,
			     SYSMGR_SOC64_DDR_MODE_MSK);
	} else if (type == DDRTYPE_LPDDR4_0)  {
		printf("Starting LPDDR4 configuration ...\n");
		clrbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_DDR_MODE,
			     SYSMGR_SOC64_DDR_MODE_MSK);

		use_lpddr4_interleaving(false);
	}
}

static int scrubber_ddr_config(phys_addr_t umctl2_base,
			       enum ddr_type umctl2_type)
{
	u32 backup[9];
	int ret;

	/* Reset to default value, prevent scrubber stop due to lower power */
	writel(0, umctl2_base + DDR4_PWRCTL_OFFSET);

	/* Backup user settings */
	backup[0] = readl(umctl2_base + DDR4_SBRCTL_OFFSET);
	backup[1] = readl(umctl2_base + DDR4_SBRWDATA0_OFFSET);
	backup[2] = readl(umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDRTYPE_DDR4) {
		backup[3] = readl(umctl2_base + DDR4_SBRWDATA1_OFFSET);
		backup[4] = readl(umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	backup[5] = readl(umctl2_base + DDR4_SBRRANGE0_OFFSET);
	backup[6] = readl(umctl2_base + DDR4_SBRRANGE1_OFFSET);
	backup[7] = readl(umctl2_base + DDR4_ECCCFG0_OFFSET);
	backup[8] = readl(umctl2_base + DDR4_ECCCFG1_OFFSET);

	if (umctl2_type != DDRTYPE_DDR4) {
		/* Lock ECC region, ensure this regions is not being accessed */
		setbits_le32(umctl2_base + DDR4_ECCCFG1_OFFSET,
			     LPDDR4_ECCCFG1_ECC_REGIONS_PARITY_LOCK);
	}
	/* Disable input traffic per port */
	clrbits_le32(umctl2_base + DDR4_PCTRL0_OFFSET, DDR4_PCTRL0_PORT_EN);
	/* Disables scrubber */
	clrbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);
	/* Polling all scrub writes data have been sent */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SBRSTAT_OFFSET), DDR4_SBRSTAT_SCRUB_BUSY,
				false, TIMEOUT_5000MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" sending all scrub data\n");
		return ret;
	}

	/* LPDDR4 supports inline ECC only */
	if (umctl2_type != DDRTYPE_DDR4) {
		/*
		 * Setting all regions for protected, this is required for
		 * srubber to init whole LPDDR4 expect ECC region
		 */
		writel(((ONE_EIGHT <<
		       LPDDR4_ECCCFG0_ECC_REGION_MAP_GRANU_SHIFT) |
		       (ALL_PROTECTED << LPDDR4_ECCCFG0_ECC_REGION_MAP_SHIFT)),
		       umctl2_base + DDR4_ECCCFG0_OFFSET);
	}

	/* Scrub_burst = 1, scrub_mode = 1(performs writes) */
	writel(DDR4_SBRCTL_SCRUB_BURST_1 | DDR4_SBRCTL_SCRUB_WRITE,
	       umctl2_base + DDR4_SBRCTL_OFFSET);

	/* Zeroing whole DDR */
	writel(0, umctl2_base + DDR4_SBRWDATA0_OFFSET);
	writel(0, umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDRTYPE_DDR4) {
		writel(0, umctl2_base + DDR4_SBRWDATA1_OFFSET);
		writel(0, umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	writel(0, umctl2_base + DDR4_SBRRANGE0_OFFSET);
	writel(0, umctl2_base + DDR4_SBRRANGE1_OFFSET);

	printf("Enabling scrubber ...\n");
	/* Enables scrubber */
	setbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);
	/* Polling all scrub writes commands have been sent */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SBRSTAT_OFFSET), DDR4_SBRSTAT_SCRUB_DONE,
				true, TIMEOUT_5000MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" sending all scrub commands\n");
		return ret;
	}

	/* Polling all scrub writes data have been sent */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SBRSTAT_OFFSET), DDR4_SBRSTAT_SCRUB_BUSY,
				false, TIMEOUT_5000MS, false);
	if (ret) {
		printf("%s: Timeout while waiting for", __func__);
		printf(" sending all scrub data\n");
		return ret;
	}

	/* Disables scrubber */
	clrbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);

	/* Restore user settings */
	writel(backup[0], umctl2_base + DDR4_SBRCTL_OFFSET);
	writel(backup[1], umctl2_base + DDR4_SBRWDATA0_OFFSET);
	writel(backup[2], umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDRTYPE_DDR4) {
		writel(backup[3], umctl2_base + DDR4_SBRWDATA1_OFFSET);
		writel(backup[4], umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	writel(backup[5], umctl2_base + DDR4_SBRRANGE0_OFFSET);
	writel(backup[6], umctl2_base + DDR4_SBRRANGE1_OFFSET);
	writel(backup[7], umctl2_base + DDR4_ECCCFG0_OFFSET);
	writel(backup[8], umctl2_base + DDR4_ECCCFG1_OFFSET);

	/* Enables ECC scrub on scrubber */
	if (!(readl(umctl2_base + DDR4_SBRCTL_OFFSET) &
	    DDR4_SBRCTL_SCRUB_WRITE)) {
		/* Enables scrubber */
		setbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET,
			     DDR4_SBRCTL_SCRUB_EN);
	}

	return 0;
}

static void handoff_process(struct ddr_handoff *ddr_handoff_info,
			    phys_addr_t handoff_base, size_t length,
			    phys_addr_t base)
{
	u32 handoff_table[length];
	u32 i, value = 0;

	/* Execute configuration handoff */
	socfpga_handoff_read((void *)handoff_base, handoff_table, length);

	for (i = 0; i < length; i = i + 2) {
		debug("%s: wr = 0x%08x ",  __func__, handoff_table[i + 1]);
		if (ddr_handoff_info && base == ddr_handoff_info->phy_base) {
			/*
			 * Convert PHY odd offset to even offset that
			 * supported by ARM processor.
			 */
			value = handoff_table[i] << 1;

			writew(handoff_table[i + 1],
			       (uintptr_t)(value + base));
			debug("rd = 0x%08x ",
			      readw((uintptr_t)(value + base)));
			debug("PHY offset: 0x%08x ", handoff_table[i + 1]);
		} else {
			value = handoff_table[i];
			writel(handoff_table[i + 1], (uintptr_t)(value +
			       base));
			 debug("rd = 0x%08x ",
			       readl((uintptr_t)(value + base)));
		}

		debug("Absolute addr: 0x%08llx, APB offset: 0x%08x\n",
		      value + base, value);
	}
}

static int init_umctl2(phys_addr_t umctl2_handoff_base,
		       phys_addr_t umctl2_base, enum ddr_type umctl2_type,
		       size_t umctl2_handoff_length,
		       u32 *user_backup)
{
	int ret;

	if (umctl2_type == DDRTYPE_DDR4)
		printf("Initializing DDR4 controller ...\n");
	else if (umctl2_type == DDRTYPE_LPDDR4_0)
		printf("Initializing LPDDR4_0 controller ...\n");
	else if (umctl2_type == DDRTYPE_LPDDR4_1)
		printf("Initializing LPDDR4_1 controller ...\n");

	/* Prevent controller from issuing read/write to SDRAM */
	setbits_le32(umctl2_base + DDR4_DBG1_OFFSET, DDR4_DBG1_DISDQ);

	/* Put SDRAM into self-refresh */
	setbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET, DDR4_PWRCTL_SELFREF_EN);

	/* Enable quasi-dynamic programing of the controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Ensure the controller is in initialization mode */
	ret = wait_for_bit_le32((const void *)(umctl2_base + DDR4_STAT_OFFSET),
				DDR4_STAT_OPERATING_MODE, false, TIMEOUT_200MS,
				false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" init operating mode\n");
		return ret;
	}

	debug("%s: UMCTL2 handoff base address = 0x%p table length = 0x%08x\n",
	      __func__, (u32 *)umctl2_handoff_base,
	      (u32)umctl2_handoff_length);

	handoff_process(NULL, umctl2_handoff_base, umctl2_handoff_length,
			umctl2_base);

	/* Backup user settings, restore after DDR up running */
	*user_backup = readl(umctl2_base + DDR4_PWRCTL_OFFSET);

	/* Disable self resfresh */
	clrbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET, DDR4_PWRCTL_SELFREF_EN);

	if (umctl2_type == DDRTYPE_LPDDR4_0 ||
	    umctl2_type == DDRTYPE_LPDDR4_1) {
		/* Setting selfref_sw to 1, based on lpddr4 requirement */
		setbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET,
			     DDR4_PWRCTL_SELFREF_SW);
		/* Backup user settings, restore after DDR up running */
		user_backup++;
		*user_backup = readl(umctl2_base + DDR4_INIT0_OFFSET) &
				     DDR4_INIT0_SKIP_RAM_INIT;

		/*
		 * Setting INIT0.skip_dram_init to 0x3, based on ddr4 / lpddr4
		 * requirement
		 */
		setbits_le32(umctl2_base + DDR4_INIT0_OFFSET,
			     DDR4_INIT0_SKIP_RAM_INIT);
	}

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Enable controller from issuing read/write to SDRAM */
	clrbits_le32(umctl2_base + DDR4_DBG1_OFFSET, DDR4_DBG1_DISDQ);

	return 0;
}

static int phy_pre_handoff_config(phys_addr_t umctl2_base,
				  enum ddr_type umctl2_type)
{
	int ret;
	u32 value;

	if (umctl2_type == DDRTYPE_DDR4) {
		/* Check DDR4 retry is enabled ? */
		value = readl(umctl2_base + DDR4_CRCPARCTL1_OFFSET) &
			      DDR4_CRCPARCTL1_CRC_PARITY_RETRY_ENABLE;

		if (value) {
			debug("%s: DDR4 retry is enabled\n", __func__);
			debug("%s: Disable auto refresh is not supported\n",
			      __func__);
		} else {
			/* Disable auto refresh */
			setbits_le32(umctl2_base + DDR4_RFSHCTL3_OFFSET,
				     DDR4_RFSHCTL3_DIS_AUTO_REFRESH);
		}
	}

	/* Disable selfref_en & powerdown_en, nvr disable dfi dram clk */
	clrbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET,
		     DDR4_PWRCTL_EN_DFI_DRAM_CLK_DISABLE |
		     DDR4_PWRCTL_POWERDOWN_EN | DDR4_PWRCTL_SELFREF_EN);

	/* Enable quasi-dynamic programing of the controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Masking dfi init complete */
	clrbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_COMPLETE_EN);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET), DDR4_SWSTAT_SW_DONE_ACK,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
	}

	return ret;
}

static void phy_ocram(phys_addr_t phy_base, phys_addr_t phy_offset,
		      u16 *data, enum data_process proc)
{
	u32 abs_addr;

	abs_addr = phy_base + (phy_offset << 1);

	if (proc == STORE) {
		/* Storing PHY value */
		writew(readw((uintptr_t)abs_addr), data);
		debug("%s: PHY offset 0x%08x, data %x\n", __func__,
		      (u32)phy_offset, readw((uintptr_t)data));
	} else if (proc == LOADING) {
		/* Loading bak cal data to PHY */
		writew(readw(data), (uintptr_t)abs_addr);
		debug("%s: PHY offset 0x%08x, data %x\n", __func__,
		      (u32)phy_offset, readw((uintptr_t)abs_addr));
	}
}

static int cal_data_ocram(phys_addr_t phy_base, u32 addr,
			  enum data_process proc)
{
	/*
	 * This array variable contains a list of PHY registers required for
	 * backup before DDR entering self-refresh mode
	 */
	static const u32 phybak[] = {0x1005f, 0x1015f, 0x1105f, 0x1115f, 0x1205f,
			0x1215f, 0x1305f, 0x1315f, 0x1405f, 0x1415f,
			0x1505f, 0x1515f, 0x1605f, 0x1615f, 0x1705f,
			0x1715f, 0x1805f, 0x1815f, 0x00055, 0x01055,
			0x02055, 0x03055, 0x04055, 0x05055, 0x06055,
			0x07055, 0x08055, 0x09055, 0x200c5, 0x2002e,
			0x20024, 0x2003a, 0x2007d, 0x2007c, 0x20056,
			0x1004d, 0x1014d, 0x1104d, 0x1114d, 0x1204d,
			0x1214d, 0x1304d, 0x1314d, 0x1404d, 0x1414d,
			0x1504d, 0x1514d, 0x1604d, 0x1614d, 0x1704d,
			0x1714d, 0x1804d, 0x1814d, 0x10049, 0x10149,
			0x11049, 0x11149, 0x12049, 0x12149, 0x13049,
			0x13149, 0x14049, 0x14149, 0x15049, 0x15149,
			0x16049, 0x16149, 0x17049, 0x17149, 0x18049,
			0x18149, 0x00043, 0x01043, 0x02043, 0x03043,
			0x04043, 0x05043, 0x06043, 0x07043, 0x08043,
			0x09043, 0x20018, 0x20075, 0x20050, 0x2009b,
			0x20008, 0x20088, 0x200b2, 0x10043, 0x10143,
			0x11043, 0x11143, 0x12043, 0x12143, 0x13043,
			0x13143, 0x14043, 0x14143, 0x15043, 0x15143,
			0x16043, 0x16143, 0x17043, 0x17143, 0x18043,
			0x18143, 0x2005b, 0x2005c, 0x200fa, 0x20019,
			0x200f0, 0x200f1, 0x200f2, 0x200f3, 0x200f4,
			0x200f5, 0x200f6, 0x200f7, 0x1004a, 0x1104a,
			0x1204a, 0x1304a, 0x1404a, 0x1504a, 0x1604a,
			0x1704a, 0x1804a, 0x20025, 0x2002d, 0x2002c,
			0xd0000, 0x90000, 0x90001, 0x90002, 0x90003,
			0x90004, 0x90005, 0x90029, 0x9002a, 0x9002b,
			0x9002c, 0x9002d, 0x9002e, 0x9002f, 0x90030,
			0x90031, 0x90032, 0x90033, 0x90034, 0x90035,
			0x90036, 0x90037, 0x90038, 0x90039, 0x9003a,
			0x9003b, 0x9003c, 0x9003d, 0x9003e, 0x9003f,
			0x90040, 0x90041, 0x90042, 0x90043, 0x90044,
			0x90045, 0x90046, 0x90047, 0x90048, 0x90049,
			0x9004a, 0x9004b, 0x9004c, 0x9004d, 0x9004e,
			0x9004f, 0x90050, 0x90051, 0x90052, 0x90053,
			0x90054, 0x90055, 0x90056, 0x90057, 0x90058,
			0x90059, 0x9005a, 0x9005b, 0x9005c, 0x9005d,
			0x9005e, 0x9005f, 0x90060, 0x90061, 0x90062,
			0x90063, 0x90064, 0x90065, 0x90066, 0x90067,
			0x90068, 0x90069, 0x9006a, 0x9006b, 0x9006c,
			0x9006d, 0x9006e, 0x9006f, 0x90070, 0x90071,
			0x90072, 0x90073, 0x90074, 0x90075, 0x90076,
			0x90077, 0x90078, 0x90079, 0x9007a, 0x9007b,
			0x9007c, 0x9007d, 0x9007e, 0x9007f, 0x90080,
			0x90081, 0x90082, 0x90083, 0x90084, 0x90085,
			0x90086, 0x90087, 0x90088, 0x90089, 0x9008a,
			0x9008b, 0x9008c, 0x9008d, 0x9008e, 0x9008f,
			0x90090, 0x90091, 0x90092, 0x90093, 0x90094,
			0x90095, 0x90096, 0x90097, 0x90098, 0x90099,
			0x9009a, 0x9009b, 0x9009c, 0x9009d, 0x9009e,
			0x9009f, 0x900a0, 0x900a1, 0x900a2, 0x900a3,
			0x900a4, 0x900a5, 0x900a6, 0x900a7, 0x900a8,
			0x900a9, 0x900aa, 0x900ab, 0x900ac, 0x900ad,
			0x900ae, 0x900af, 0x900b0, 0x900b1, 0x900b2,
			0x900b3, 0x900b4, 0x900b5, 0x900b6, 0x900b7,
			0x900b8, 0x900b9, 0x900ba, 0x900bb, 0x900bc,
			0x900bd, 0x900be, 0x900bf, 0x900c0, 0x900c1,
			0x900c2, 0x900c3, 0x900c4, 0x900c5, 0x900c6,
			0x900c7, 0x90006, 0x90007, 0x90008, 0x90009,
			0x9000a, 0x9000b, 0xd00e7, 0x90017, 0x90026,
			0x2000b, 0x2000c, 0x2000d, 0x2000e, 0x9000c,
			0x9000d, 0x9000e, 0x9000f, 0x90010, 0x90011,
			0x90012, 0x90013, 0x20089, 0xc0080, 0x200cb,
			0x10068, 0x10069, 0x1006a, 0x1006b, 0x10168,
			0x10169, 0x1016a, 0x1016b, 0x10268, 0x10269,
			0x1026a, 0x1026b, 0x10368, 0x10369, 0x1036a,
			0x1036b, 0x10468, 0x10469, 0x1046a, 0x1046b,
			0x10568, 0x10569, 0x1056a, 0x1056b, 0x10668,
			0x10669, 0x1066a, 0x1066b, 0x10768, 0x10769,
			0x1076a, 0x1076b, 0x10868, 0x10869, 0x1086a,
			0x1086b, 0x11068, 0x11069, 0x1106a, 0x1106b,
			0x11168, 0x11169, 0x1116a, 0x1116b, 0x11268,
			0x11269, 0x1126a, 0x1126b, 0x11368, 0x11369,
			0x1136a, 0x1136b, 0x11468, 0x11469, 0x1146a,
			0x1146b, 0x11568, 0x11569, 0x1156a, 0x1156b,
			0x11668, 0x11669, 0x1166a, 0x1166b, 0x11768,
			0x11769, 0x1176a, 0x1176b, 0x11868, 0x11869,
			0x1186a, 0x1186b, 0x12068, 0x12069, 0x1206a,
			0x1206b, 0x12168, 0x12169, 0x1216a, 0x1216b,
			0x12268, 0x12269, 0x1226a, 0x1226b, 0x12368,
			0x12369, 0x1236a, 0x1236b, 0x12468, 0x12469,
			0x1246a, 0x1246b, 0x12568, 0x12569, 0x1256a,
			0x1256b, 0x12668, 0x12669, 0x1266a, 0x1266b,
			0x12768, 0x12769, 0x1276a, 0x1276b, 0x12868,
			0x12869, 0x1286a, 0x1286b, 0x13068, 0x13069,
			0x1306a, 0x1306b, 0x13168, 0x13169, 0x1316a,
			0x1316b, 0x13268, 0x13269, 0x1326a, 0x1326b,
			0x13368, 0x13369, 0x1336a, 0x1336b, 0x13468,
			0x13469, 0x1346a, 0x1346b, 0x13568, 0x13569,
			0x1356a, 0x1356b, 0x13668, 0x13669, 0x1366a,
			0x1366b, 0x13768, 0x13769, 0x1376a, 0x1376b,
			0x13868, 0x13869, 0x1386a, 0x1386b, 0x14068,
			0x14069, 0x1406a, 0x1406b, 0x14168, 0x14169,
			0x1416a, 0x1416b, 0x14268, 0x14269, 0x1426a,
			0x1426b, 0x14368, 0x14369, 0x1436a, 0x1436b,
			0x14468, 0x14469, 0x1446a, 0x1446b, 0x14568,
			0x14569, 0x1456a, 0x1456b, 0x14668, 0x14669,
			0x1466a, 0x1466b, 0x14768, 0x14769, 0x1476a,
			0x1476b, 0x14868, 0x14869, 0x1486a, 0x1486b,
			0x15068, 0x15069, 0x1506a, 0x1506b, 0x15168,
			0x15169, 0x1516a, 0x1516b, 0x15268, 0x15269,
			0x1526a, 0x1526b, 0x15368, 0x15369, 0x1536a,
			0x1536b, 0x15468, 0x15469, 0x1546a, 0x1546b,
			0x15568, 0x15569, 0x1556a, 0x1556b, 0x15668,
			0x15669, 0x1566a, 0x1566b, 0x15768, 0x15769,
			0x1576a, 0x1576b, 0x15868, 0x15869, 0x1586a,
			0x1586b, 0x16068, 0x16069, 0x1606a, 0x1606b,
			0x16168, 0x16169, 0x1616a, 0x1616b, 0x16268,
			0x16269, 0x1626a, 0x1626b, 0x16368, 0x16369,
			0x1636a, 0x1636b, 0x16468, 0x16469, 0x1646a,
			0x1646b, 0x16568, 0x16569, 0x1656a, 0x1656b,
			0x16668, 0x16669, 0x1666a, 0x1666b, 0x16768,
			0x16769, 0x1676a, 0x1676b, 0x16868, 0x16869,
			0x1686a, 0x1686b, 0x17068, 0x17069, 0x1706a,
			0x1706b, 0x17168, 0x17169, 0x1716a, 0x1716b,
			0x17268, 0x17269, 0x1726a, 0x1726b, 0x17368,
			0x17369, 0x1736a, 0x1736b, 0x17468, 0x17469,
			0x1746a, 0x1746b, 0x17568, 0x17569, 0x1756a,
			0x1756b, 0x17668, 0x17669, 0x1766a, 0x1766b,
			0x17768, 0x17769, 0x1776a, 0x1776b, 0x17868,
			0x17869, 0x1786a, 0x1786b, 0x18068, 0x18069,
			0x1806a, 0x1806b, 0x18168, 0x18169, 0x1816a,
			0x1816b, 0x18268, 0x18269, 0x1826a, 0x1826b,
			0x18368, 0x18369, 0x1836a, 0x1836b, 0x18468,
			0x18469, 0x1846a, 0x1846b, 0x18568, 0x18569,
			0x1856a, 0x1856b, 0x18668, 0x18669, 0x1866a,
			0x1866b, 0x18768, 0x18769, 0x1876a, 0x1876b,
			0x18868, 0x18869, 0x1886a, 0x1886b, 0x00080,
			0x01080, 0x02080, 0x03080, 0x04080, 0x05080,
			0x06080, 0x07080, 0x08080, 0x09080, 0x10020,
			0x10080, 0x10081, 0x10082, 0x10083, 0x100d0,
			0x100d1, 0x100d2, 0x100d3, 0x1008c, 0x1008d,
			0x1008e, 0x1008f, 0x10180, 0x10181, 0x10182,
			0x10183, 0x101d0, 0x101d1, 0x101d2, 0x101d3,
			0x1018c, 0x1018d, 0x1018e, 0x1018f, 0x100c0,
			0x100c1, 0x100c2, 0x100c3, 0x101c0, 0x101c1,
			0x101c2, 0x101c3, 0x102c0, 0x102c1, 0x102c2,
			0x102c3, 0x103c0, 0x103c1, 0x103c2, 0x103c3,
			0x104c0, 0x104c1, 0x104c2, 0x104c3, 0x105c0,
			0x105c1, 0x105c2, 0x105c3, 0x106c0, 0x106c1,
			0x106c2, 0x106c3, 0x107c0, 0x107c1, 0x107c2,
			0x107c3, 0x108c0, 0x108c1, 0x108c2, 0x108c3,
			0x11020, 0x11080, 0x11081, 0x11082, 0x11083,
			0x110d0, 0x110d1, 0x110d2, 0x110d3, 0x1108c,
			0x1108d, 0x1108e, 0x1108f, 0x11180, 0x11181,
			0x11182, 0x11183, 0x111d0, 0x111d1, 0x111d2,
			0x111d3, 0x1118c, 0x1118d, 0x1118e, 0x1118f,
			0x110c0, 0x110c1, 0x110c2, 0x110c3, 0x111c0,
			0x111c1, 0x111c2, 0x111c3, 0x112c0, 0x112c1,
			0x112c2, 0x112c3, 0x113c0, 0x113c1, 0x113c2,
			0x113c3, 0x114c0, 0x114c1, 0x114c2, 0x114c3,
			0x115c0, 0x115c1, 0x115c2, 0x115c3, 0x116c0,
			0x116c1, 0x116c2, 0x116c3, 0x117c0, 0x117c1,
			0x117c2, 0x117c3, 0x118c0, 0x118c1, 0x118c2,
			0x118c3, 0x12020, 0x12080, 0x12081, 0x12082,
			0x12083, 0x120d0, 0x120d1, 0x120d2, 0x120d3,
			0x1208c, 0x1208d, 0x1208e, 0x1208f, 0x12180,
			0x12181, 0x12182, 0x12183, 0x121d0, 0x121d1,
			0x121d2, 0x121d3, 0x1218c, 0x1218d, 0x1218e,
			0x1218f, 0x120c0, 0x120c1, 0x120c2, 0x120c3,
			0x121c0, 0x121c1, 0x121c2, 0x121c3, 0x122c0,
			0x122c1, 0x122c2, 0x122c3, 0x123c0, 0x123c1,
			0x123c2, 0x123c3, 0x124c0, 0x124c1, 0x124c2,
			0x124c3, 0x125c0, 0x125c1, 0x125c2, 0x125c3,
			0x126c0, 0x126c1, 0x126c2, 0x126c3, 0x127c0,
			0x127c1, 0x127c2, 0x127c3, 0x128c0, 0x128c1,
			0x128c2, 0x128c3, 0x13020, 0x13080, 0x13081,
			0x13082, 0x13083, 0x130d0, 0x130d1, 0x130d2,
			0x130d3, 0x1308c, 0x1308d, 0x1308e, 0x1308f,
			0x13180, 0x13181, 0x13182, 0x13183, 0x131d0,
			0x131d1, 0x131d2, 0x131d3, 0x1318c, 0x1318d,
			0x1318e, 0x1318f, 0x130c0, 0x130c1, 0x130c2,
			0x130c3, 0x131c0, 0x131c1, 0x131c2, 0x131c3,
			0x132c0, 0x132c1, 0x132c2, 0x132c3, 0x133c0,
			0x133c1, 0x133c2, 0x133c3, 0x134c0, 0x134c1,
			0x134c2, 0x134c3, 0x135c0, 0x135c1, 0x135c2,
			0x135c3, 0x136c0, 0x136c1, 0x136c2, 0x136c3,
			0x137c0, 0x137c1, 0x137c2, 0x137c3, 0x138c0,
			0x138c1, 0x138c2, 0x138c3, 0x14020, 0x14080,
			0x14081, 0x14082, 0x14083, 0x140d0, 0x140d1,
			0x140d2, 0x140d3, 0x1408c, 0x1408d, 0x1408e,
			0x1408f, 0x14180, 0x14181, 0x14182, 0x14183,
			0x141d0, 0x141d1, 0x141d2, 0x141d3, 0x1418c,
			0x1418d, 0x1418e, 0x1418f, 0x140c0, 0x140c1,
			0x140c2, 0x140c3, 0x141c0, 0x141c1, 0x141c2,
			0x141c3, 0x142c0, 0x142c1, 0x142c2, 0x142c3,
			0x143c0, 0x143c1, 0x143c2, 0x143c3, 0x144c0,
			0x144c1, 0x144c2, 0x144c3, 0x145c0, 0x145c1,
			0x145c2, 0x145c3, 0x146c0, 0x146c1, 0x146c2,
			0x146c3, 0x147c0, 0x147c1, 0x147c2, 0x147c3,
			0x148c0, 0x148c1, 0x148c2, 0x148c3, 0x15020,
			0x15080, 0x15081, 0x15082, 0x15083, 0x150d0,
			0x150d1, 0x150d2, 0x150d3, 0x1508c, 0x1508d,
			0x1508e, 0x1508f, 0x15180, 0x15181, 0x15182,
			0x15183, 0x151d0, 0x151d1, 0x151d2, 0x151d3,
			0x1518c, 0x1518d, 0x1518e, 0x1518f, 0x150c0,
			0x150c1, 0x150c2, 0x150c3, 0x151c0, 0x151c1,
			0x151c2, 0x151c3, 0x152c0, 0x152c1, 0x152c2,
			0x152c3, 0x153c0, 0x153c1, 0x153c2, 0x153c3,
			0x154c0, 0x154c1, 0x154c2, 0x154c3, 0x155c0,
			0x155c1, 0x155c2, 0x155c3, 0x156c0, 0x156c1,
			0x156c2, 0x156c3, 0x157c0, 0x157c1, 0x157c2,
			0x157c3, 0x158c0, 0x158c1, 0x158c2, 0x158c3,
			0x16020, 0x16080, 0x16081, 0x16082, 0x16083,
			0x160d0, 0x160d1, 0x160d2, 0x160d3, 0x1608c,
			0x1608d, 0x1608e, 0x1608f, 0x16180, 0x16181,
			0x16182, 0x16183, 0x161d0, 0x161d1, 0x161d2,
			0x161d3, 0x1618c, 0x1618d, 0x1618e, 0x1618f,
			0x160c0, 0x160c1, 0x160c2, 0x160c3, 0x161c0,
			0x161c1, 0x161c2, 0x161c3, 0x162c0, 0x162c1,
			0x162c2, 0x162c3, 0x163c0, 0x163c1, 0x163c2,
			0x163c3, 0x164c0, 0x164c1, 0x164c2, 0x164c3,
			0x165c0, 0x165c1, 0x165c2, 0x165c3, 0x166c0,
			0x166c1, 0x166c2, 0x166c3, 0x167c0, 0x167c1,
			0x167c2, 0x167c3, 0x168c0, 0x168c1, 0x168c2,
			0x168c3, 0x17020, 0x17080, 0x17081, 0x17082,
			0x17083, 0x170d0, 0x170d1, 0x170d2, 0x170d3,
			0x1708c, 0x1708d, 0x1708e, 0x1708f, 0x17180,
			0x17181, 0x17182, 0x17183, 0x171d0, 0x171d1,
			0x171d2, 0x171d3, 0x1718c, 0x1718d, 0x1718e,
			0x1718f, 0x170c0, 0x170c1, 0x170c2, 0x170c3,
			0x171c0, 0x171c1, 0x171c2, 0x171c3, 0x172c0,
			0x172c1, 0x172c2, 0x172c3, 0x173c0, 0x173c1,
			0x173c2, 0x173c3, 0x174c0, 0x174c1, 0x174c2,
			0x174c3, 0x175c0, 0x175c1, 0x175c2, 0x175c3,
			0x176c0, 0x176c1, 0x176c2, 0x176c3, 0x177c0,
			0x177c1, 0x177c2, 0x177c3, 0x178c0, 0x178c1,
			0x178c2, 0x178c3, 0x18020, 0x18080, 0x18081,
			0x18082, 0x18083, 0x180d0, 0x180d1, 0x180d2,
			0x180d3, 0x1808c, 0x1808d, 0x1808e, 0x1808f,
			0x18180, 0x18181, 0x18182, 0x18183, 0x181d0,
			0x181d1, 0x181d2, 0x181d3, 0x1818c, 0x1818d,
			0x1818e, 0x1818f, 0x180c0, 0x180c1, 0x180c2,
			0x180c3, 0x181c0, 0x181c1, 0x181c2, 0x181c3,
			0x182c0, 0x182c1, 0x182c2, 0x182c3, 0x183c0,
			0x183c1, 0x183c2, 0x183c3, 0x184c0, 0x184c1,
			0x184c2, 0x184c3, 0x185c0, 0x185c1, 0x185c2,
			0x185c3, 0x186c0, 0x186c1, 0x186c2, 0x186c3,
			0x187c0, 0x187c1, 0x187c2, 0x187c3, 0x188c0,
			0x188c1, 0x188c2, 0x188c3, 0x90201, 0x90202,
			0x90203, 0x90204, 0x90205, 0x90206, 0x90207,
			0x90208, 0x20077, 0x10040, 0x10030, 0x10140,
			0x10130, 0x10240, 0x10230, 0x10340, 0x10330,
			0x10440, 0x10430, 0x10540, 0x10530, 0x10640,
			0x10630, 0x10740, 0x10730, 0x10840, 0x10830,
			0x11040, 0x11030, 0x11140, 0x11130, 0x11240,
			0x11230, 0x11340, 0x11330, 0x11440, 0x11430,
			0x11540, 0x11530, 0x11640, 0x11630, 0x11740,
			0x11730, 0x11840, 0x11830, 0x12040, 0x12030,
			0x12140, 0x12130, 0x12240, 0x12230, 0x12340,
			0x12330, 0x12440, 0x12430, 0x12540, 0x12530,
			0x12640, 0x12630, 0x12740, 0x12730, 0x12840,
			0x12830, 0x13040, 0x13030, 0x13140, 0x13130,
			0x13240, 0x13230, 0x13340, 0x13330, 0x13440,
			0x13430, 0x13540, 0x13530, 0x13640, 0x13630,
			0x13740, 0x13730, 0x13840, 0x13830, 0x14040,
			0x14030, 0x14140, 0x14130, 0x14240, 0x14230,
			0x14340, 0x14330, 0x14440, 0x14430, 0x14540,
			0x14530, 0x14640, 0x14630, 0x14740, 0x14730,
			0x14840, 0x14830, 0x15040, 0x15030, 0x15140,
			0x15130, 0x15240, 0x15230, 0x15340, 0x15330,
			0x15440, 0x15430, 0x15540, 0x15530, 0x15640,
			0x15630, 0x15740, 0x15730, 0x15840, 0x15830,
			0x16040, 0x16030, 0x16140, 0x16130, 0x16240,
			0x16230, 0x16340, 0x16330, 0x16440, 0x16430,
			0x16540, 0x16530, 0x16640, 0x16630, 0x16740,
			0x16730, 0x16840, 0x16830, 0x17040, 0x17030,
			0x17140, 0x17130, 0x17240, 0x17230, 0x17340,
			0x17330, 0x17440, 0x17430, 0x17540, 0x17530,
			0x17640, 0x17630, 0x17740, 0x17730, 0x17840,
			0x17830, 0x18040, 0x18030, 0x18140, 0x18130,
			0x18240, 0x18230, 0x18340, 0x18330, 0x18440,
			0x18430, 0x18540, 0x18530, 0x18640, 0x18630,
			0x18740, 0x18730, 0x18840, 0x18830};
	size_t phybak_num;
	const u32 *phybak_p = phybak;
	u16 *data;
	struct bac_cal_data_t *cal = (struct bac_cal_data_t *)
					((uintptr_t)addr);

	/* Enable access to the PHY configuration registers */
	clrbits_le16(phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	phybak_num = ARRAY_SIZE(phybak);

	debug("%s: Total PHY backup %d registers\n", __func__,
	      (int)phybak_num);

	if (proc == STORE) {
		debug("%s: Storing PHY bak calibration to OCRAM ...\n",
		      __func__);
		/* Creating header */
		/* Write PHY magic number */
		cal->header.header_magic = SOC64_HANDOFF_DDR_PHY_MAGIC;
		cal->header.data_len = phybak_num * sizeof(*data);
	} else if (proc == LOADING) {
		debug("%s: Loading bak calibration to PHY from OCRAM ...\n",
		      __func__);
	}

	data = &cal->data;
	while (phybak_num) {
		phy_ocram(phy_base, *phybak_p, data, proc);
		phybak_p++;
		phybak_num--;
		data++;
		schedule();
	};

	if (proc == STORE) {
		/* Creating header */
		/* Generate HASH384 from the DDR config */
		sha384_csum_wd((u8 *)DDR_HANDOFF_IMG_ADDR,
			       DDR_HANDOFF_IMG_LEN,
			       cal->header.ddrconfig_hash,
			       CHUNKSZ_PER_WD_RESET);

		if (SOC64_HANDOFF_BASE < ((uintptr_t)(&cal->data) +
		    cal->header.data_len)) {
			debug("%s: Backup cal data overflow HPS handoff\n",
			      __func__);
			return -ENOEXEC;
		}

		crc32_wd_buf((u8 *)&cal->data, cal->header.data_len,
			     (u8 *)&cal->header.caldata_crc32,
			     CHUNKSZ_PER_WD_RESET);
	}

	/* Isolate the APB access from internal CSRs */
	setbits_le16(phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	return 0;
}

static bool is_ddrconfig_hash_match(const void *buffer)
{
	int ret;
	u8 hash[SHA384_SUM_LEN];

	/* Magic symbol in first 4 bytes of header */
	struct cal_header_t *header = (struct cal_header_t *)buffer;

	/* Generate HASH384 from the image */
	sha384_csum_wd((u8 *)DDR_HANDOFF_IMG_ADDR, DDR_HANDOFF_IMG_LEN,
		       hash, CHUNKSZ_PER_WD_RESET);

	ret = memcmp((void *)hash, (const void *)header->ddrconfig_hash,
		     SHA384_SUM_LEN);
	if (ret) {
		debug("%s: DDR config hash mismatch\n", __func__);
		return false;
	}

	return true;
}

static ofnode get_ddr_ofnode(ofnode from)
{
	return ofnode_by_compatible(from, "intel,sdr-ctl-n5x");
}

static const char *get_ddrcal_ddr_offset(void)
{
	const char *ddr_offset = NULL;

	ofnode ddr_node = get_ddr_ofnode(ofnode_null());

	if (ofnode_valid(ddr_node))
		ddr_offset = ofnode_read_string(ddr_node,
						"intel,ddrcal-ddr-offset");

	return ddr_offset;
}

static const char *get_ddrcal_qspi_offset(void)
{
	const char *qspi_offset = NULL;

	ofnode ddr_node = get_ddr_ofnode(ofnode_null());

	if (ofnode_valid(ddr_node))
		qspi_offset = ofnode_read_string(ddr_node,
						 "intel,ddrcal-qspi-offset");

	return qspi_offset;
}

static bool is_cal_bak_data_valid(void)
{
	int ret, size;
	struct udevice *dev;
	ofnode node;
	const fdt32_t *phandle_p;
	u32 phandle, crc32;
	const char *qspi_offset = get_ddrcal_qspi_offset();
	struct bac_cal_data_t *cal = (struct bac_cal_data_t *)((uintptr_t)
					SOC64_OCRAM_PHY_BACKUP_BASE);

	node = get_ddr_ofnode(ofnode_null());

	if (ofnode_valid(node)) {
		phandle_p = ofnode_get_property(node, "firmware-loader",
						&size);
		if (!phandle_p) {
			node = ofnode_path("/chosen");
			if (!ofnode_valid(node)) {
				debug("%s: /chosen node was not found.\n",
				      __func__);
				return false;
			}

			phandle_p = ofnode_get_property(node,
							"firmware-loader",
							&size);
			if (!phandle_p) {
				debug("%s: firmware-loader property ",
				      __func__);
				debug("was not found.\n");
				return false;
			}
		}
	} else {
		debug("%s: DDR node was not found.\n", __func__);
		return false;
	}

	phandle = fdt32_to_cpu(*phandle_p);
	ret = uclass_get_device_by_phandle_id(UCLASS_FS_FIRMWARE_LOADER,
					      phandle, &dev);
	if (ret)
		return false;

	/* Load DDR bak cal header into OCRAM buffer */
	ret = request_firmware_into_buf(dev,
					qspi_offset,
					(void *)SOC64_OCRAM_PHY_BACKUP_BASE,
					sizeof(struct cal_header_t), 0);
	if (ret < 0) {
		debug("%s: Failed to read bak cal header from flash.\n",
		      __func__);
		return false;
	}

	if (cal->header.header_magic != SOC64_HANDOFF_DDR_PHY_MAGIC) {
		debug("%s: No magic found in cal backup header\n", __func__);
		return false;
	}

	if (SOC64_HANDOFF_BASE < (SOC64_OCRAM_PHY_BACKUP_BASE +
	    cal->header.data_len + sizeof(struct cal_header_t))) {
		debug("%s: Backup cal data overflow HPS handoff\n", __func__);
		return false;
	}

	/* Load header + DDR bak cal into OCRAM buffer */
	ret = request_firmware_into_buf(dev,
					qspi_offset,
					(void *)SOC64_OCRAM_PHY_BACKUP_BASE,
					cal->header.data_len +
					sizeof(struct cal_header_t),
					0);
	if (ret < 0) {
		debug("FPGA: Failed to read DDR bak cal data from flash.\n");
		return false;
	}

	if (SOC64_HANDOFF_BASE < ((uintptr_t)(&cal->data) +
	    cal->header.data_len)) {
		debug("%s: Backup cal data overflow HPS handoff\n", __func__);
		return false;
	}

	crc32_wd_buf((u8 *)&cal->data, cal->header.data_len,
		     (u8 *)&crc32, CHUNKSZ_PER_WD_RESET);
	debug("%s: crc32 %x for bak calibration data from QSPI\n", __func__,
	      crc32);
	if (crc32 != cal->header.caldata_crc32) {
		debug("%s: CRC32 mismatch for calibration backup data\n",
		      __func__);
		return false;
	}

	if (!is_ddrconfig_hash_match((const void *)
		SOC64_OCRAM_PHY_BACKUP_BASE)) {
		debug("%s: HASH mismatch for DDR config data\n", __func__);
		return false;
	}

	return true;
}

static int init_phy(struct ddr_handoff *ddr_handoff_info,
		    bool *need_calibrate, bool is_ddr_hang_be4_rst)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD0);
	int ret;

	printf("Initializing DDR PHY ...\n");

	if (ddr_handoff_info->cntlr_t == DDRTYPE_DDR4 ||
	    ddr_handoff_info->cntlr_t == DDRTYPE_LPDDR4_0) {
		ret = phy_pre_handoff_config(ddr_handoff_info->cntlr_base,
					     ddr_handoff_info->cntlr_t);
		if (ret)
			return ret;
	}

	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1) {
		ret = phy_pre_handoff_config
			(ddr_handoff_info->cntlr2_base,
			 ddr_handoff_info->cntlr2_t);
		if (ret)
			return ret;
	}

	/* Execute PHY configuration handoff */
	handoff_process(ddr_handoff_info, ddr_handoff_info->phy_handoff_base,
			ddr_handoff_info->phy_handoff_length,
			ddr_handoff_info->phy_base);

	if (is_ddr_calibration_skipped(reg, is_ddr_hang_be4_rst)) {
		if (is_cal_bak_data_valid())
			*need_calibrate = false;
		else
			*need_calibrate = true;
	}

	debug("%s: Need calibrate %d\n", __func__, *need_calibrate);

	if (*need_calibrate) {
		/* Execute PHY configuration handoff */
		handoff_process(ddr_handoff_info, ddr_handoff_info->phy_handoff_base,
				ddr_handoff_info->phy_handoff_length,
				ddr_handoff_info->phy_base);
	} else {
		ret = cal_data_ocram(ddr_handoff_info->phy_base,
				     SOC64_OCRAM_PHY_BACKUP_BASE, LOADING);

		if (ret)
			return ret;

	       /*
		* Invalidate the section used for processing the PHY
		* backup calibration data
		*/
		writel(SOC64_CRAM_PHY_BACKUP_SKIP_MAGIC,
		       SOC64_OCRAM_PHY_BACKUP_BASE);
	}

	printf("DDR PHY configuration is completed\n");

	return 0;
}

static void phy_init_engine(struct ddr_handoff *handoff)
{
	printf("Load PHY Init Engine ...\n");

	/* Execute PIE production code handoff */
	handoff_process(handoff, handoff->phy_engine_handoff_base,
			handoff->phy_engine_handoff_length, handoff->phy_base);

	printf("End of loading PHY Init Engine\n");
}

int populate_ddr_handoff(struct ddr_handoff *handoff)
{
	phys_addr_t next_section_header;

	/* DDR handoff */
	handoff->mem_reset_base = SOC64_HANDOFF_DDR_MEMRESET_BASE;
	debug("%s: DDR memory reset base = 0x%x\n", __func__,
	      (u32)handoff->mem_reset_base);
	debug("%s: DDR memory reset address = 0x%x\n", __func__,
	      readl(handoff->mem_reset_base));

	/* Beginning of DDR controller handoff */
	handoff->cntlr_handoff_base = SOC64_HANDOFF_DDR_UMCTL2_SECTION;
	debug("%s: cntlr handoff base = 0x%x\n", __func__,
	      (u32)handoff->cntlr_handoff_base);

	/* Get 1st DDR type */
	handoff->cntlr_t = get_ddr_type(handoff->cntlr_handoff_base +
					SOC64_HANDOFF_DDR_UMCTL2_TYPE_OFFSET);
	if (handoff->cntlr_t == DDRTYPE_LPDDR4_1 ||
	    handoff->cntlr_t == DDRTYPE_UNKNOWN) {
		debug("%s: Wrong DDR handoff format, the 1st DDR ", __func__);
		debug("type must be DDR4 or LPDDR4_0\n");
		return -ENOEXEC;
	}

	/* 1st cntlr base physical address */
	handoff->cntlr_base = readl(handoff->cntlr_handoff_base +
				    SOC64_HANDOFF_DDR_UMCTL2_BASE_ADDR_OFFSET);
	debug("%s: cntlr base = 0x%x\n", __func__, (u32)handoff->cntlr_base);

	/* Get the total length of DDR cntlr handoff section */
	handoff->cntlr_total_length = readl(handoff->cntlr_handoff_base +
					    SOC64_HANDOFF_OFFSET_LENGTH);
	debug("%s: Umctl2 total length in byte = 0x%x\n", __func__,
	      (u32)handoff->cntlr_total_length);

	/* Get the length of user setting data in DDR cntlr handoff section */
	handoff->cntlr_handoff_length = socfpga_get_handoff_size((void *)
						handoff->cntlr_handoff_base);
	debug("%s: Umctl2 handoff length in word(32-bit) = 0x%x\n", __func__,
	      (u32)handoff->cntlr_handoff_length);

	/* Wrong format on user setting data */
	if (handoff->cntlr_handoff_length < 0) {
		debug("%s: Wrong format on user setting data\n", __func__);
		return -ENOEXEC;
	}

	/* Get the next handoff section address */
	next_section_header = handoff->cntlr_handoff_base +
				handoff->cntlr_total_length;
	debug("%s: Next handoff section header location = 0x%llx\n", __func__,
	      next_section_header);

	/*
	 * Checking next section handoff is cntlr or PHY, and changing
	 * subsequent implementation accordingly
	 */
	if (readl(next_section_header) == SOC64_HANDOFF_DDR_UMCTL2_MAGIC) {
		/* Get the next cntlr handoff section address */
		handoff->cntlr2_handoff_base = next_section_header;
		debug("%s: umctl2 2nd handoff base = 0x%x\n", __func__,
		      (u32)handoff->cntlr2_handoff_base);

		/* Get 2nd DDR type */
		handoff->cntlr2_t = get_ddr_type(handoff->cntlr2_handoff_base +
					SOC64_HANDOFF_DDR_UMCTL2_TYPE_OFFSET);
		if (handoff->cntlr2_t == DDRTYPE_LPDDR4_0 ||
		    handoff->cntlr2_t == DDRTYPE_UNKNOWN) {
			debug("%s: Wrong DDR handoff format, the 2nd DDR ",
			      __func__);
			debug("type must be LPDDR4_1\n");
			return -ENOEXEC;
		}

		/* 2nd umctl2 base physical address */
		handoff->cntlr2_base =
			readl(handoff->cntlr2_handoff_base +
			      SOC64_HANDOFF_DDR_UMCTL2_BASE_ADDR_OFFSET);
		debug("%s: cntlr2 base = 0x%x\n", __func__,
		      (u32)handoff->cntlr2_base);

		/* Get the total length of 2nd DDR umctl2 handoff section */
		handoff->cntlr2_total_length =
			readl(handoff->cntlr2_handoff_base +
			      SOC64_HANDOFF_OFFSET_LENGTH);
		debug("%s: Umctl2_2nd total length in byte = 0x%x\n", __func__,
		      (u32)handoff->cntlr2_total_length);

		/*
		 * Get the length of user setting data in DDR umctl2 handoff
		 * section
		 */
		handoff->cntlr2_handoff_length =
			socfpga_get_handoff_size((void *)
						 handoff->cntlr2_handoff_base);
		debug("%s: cntlr2 handoff length in word(32-bit) = 0x%x\n",
		      __func__,
		     (u32)handoff->cntlr2_handoff_length);

		/* Wrong format on user setting data */
		if (handoff->cntlr2_handoff_length < 0) {
			debug("%s: Wrong format on umctl2 user setting data\n",
			      __func__);
			return -ENOEXEC;
		}

		/* Get the next handoff section address */
		next_section_header = handoff->cntlr2_handoff_base +
					handoff->cntlr2_total_length;
		debug("%s: Next handoff section header location = 0x%llx\n",
		      __func__, next_section_header);
	}

	/* Checking next section handoff is PHY ? */
	if (readl(next_section_header) == SOC64_HANDOFF_DDR_PHY_MAGIC) {
		/* DDR PHY handoff */
		handoff->phy_handoff_base = next_section_header;
		debug("%s: PHY handoff base = 0x%x\n", __func__,
		      (u32)handoff->phy_handoff_base);

		/* PHY base physical address */
		handoff->phy_base = readl(handoff->phy_handoff_base +
					SOC64_HANDOFF_DDR_PHY_BASE_OFFSET);
		debug("%s: PHY base = 0x%x\n", __func__,
		      (u32)handoff->phy_base);

		/* Get the total length of PHY handoff section */
		handoff->phy_total_length = readl(handoff->phy_handoff_base +
						SOC64_HANDOFF_OFFSET_LENGTH);
		debug("%s: PHY total length in byte = 0x%x\n", __func__,
		      (u32)handoff->phy_total_length);

		/*
		 * Get the length of user setting data in DDR PHY handoff
		 * section
		 */
		handoff->phy_handoff_length = socfpga_get_handoff_size((void *)
						handoff->phy_handoff_base);
		debug("%s: PHY handoff length in word(32-bit) = 0x%x\n",
		      __func__, (u32)handoff->phy_handoff_length);

		/* Wrong format on PHY user setting data */
		if (handoff->phy_handoff_length < 0) {
			debug("%s: Wrong format on PHY user setting data\n",
			      __func__);
			return -ENOEXEC;
		}

		/* Get the next handoff section address */
		next_section_header = handoff->phy_handoff_base +
					handoff->phy_total_length;
		debug("%s: Next handoff section header location = 0x%llx\n",
		      __func__, next_section_header);
	} else {
		debug("%s: Wrong format for DDR handoff, expect PHY",
		      __func__);
		debug(" handoff section after umctl2 handoff section\n");
		return -ENOEXEC;
	}

	/* Checking next section handoff is PHY init Engine ? */
	if (readl(next_section_header) ==
		SOC64_HANDOFF_DDR_PHY_INIT_ENGINE_MAGIC) {
		/* DDR PHY Engine handoff */
		handoff->phy_engine_handoff_base = next_section_header;
		debug("%s: PHY init engine handoff base = 0x%x\n", __func__,
		      (u32)handoff->phy_engine_handoff_base);

		/* Get the total length of PHY init engine handoff section */
		handoff->phy_engine_total_length =
				readl(handoff->phy_engine_handoff_base +
				      SOC64_HANDOFF_OFFSET_LENGTH);
		debug("%s: PHY engine total length in byte = 0x%x\n", __func__,
		      (u32)handoff->phy_engine_total_length);

		/*
		 * Get the length of user setting data in DDR PHY init engine
		 * handoff section
		 */
		handoff->phy_engine_handoff_length =
			socfpga_get_handoff_size((void *)
					handoff->phy_engine_handoff_base);
		debug("%s: PHY engine handoff length in word(32-bit) = 0x%x\n",
		      __func__, (u32)handoff->phy_engine_handoff_length);

		/* Wrong format on PHY init engine setting data */
		if (handoff->phy_engine_handoff_length < 0) {
			debug("%s: Wrong format on PHY init engine ",
			      __func__);
			debug("user setting data\n");
			return -ENOEXEC;
		}

		/* Checking DDR handoff is overflow? */
		if (SOC64_OCRAM_PHY_BACKUP_BASE <=
		    (handoff->phy_engine_handoff_base +
		    handoff->phy_engine_total_length)) {
			printf("%s: DDR handoff is overflow\n ", __func__);
			return -ENOEXEC;
		}
	} else {
		debug("%s: Wrong format for DDR handoff, expect PHY",
		      __func__);
		debug(" init engine handoff section after PHY handoff\n");
		debug(" section\n");
		return -ENOEXEC;
	}

	handoff->train_imem_base = handoff->phy_base +
						DDR_PHY_TRAIN_IMEM_OFFSET;
	debug("%s: PHY train IMEM base = 0x%x\n",
	      __func__, (u32)handoff->train_imem_base);

	handoff->train_dmem_base = handoff->phy_base +
						DDR_PHY_TRAIN_DMEM_OFFSET;
	debug("%s: PHY train DMEM base = 0x%x\n",
	      __func__, (u32)handoff->train_dmem_base);

	handoff->train_imem_length = SOC64_HANDOFF_DDR_TRAIN_IMEM_LENGTH;
	debug("%s: PHY train IMEM length = 0x%x\n",
	      __func__, (u32)handoff->train_imem_length);

	handoff->train_dmem_length = SOC64_HANDOFF_DDR_TRAIN_DMEM_LENGTH;
	debug("%s: PHY train DMEM length = 0x%x\n",
	      __func__, (u32)handoff->train_dmem_length);

	return 0;
}

int enable_ddr_clock(struct udevice *dev)
{
	struct clk *ddr_clk;
	int ret;

	/* Enable clock before init DDR */
	ddr_clk = devm_clk_get(dev, "mem_clk");
	if (!IS_ERR(ddr_clk)) {
		ret = clk_enable(ddr_clk);
		if (ret) {
			printf("%s: Failed to enable DDR clock\n", __func__);
			return ret;
		}
	} else {
		ret = PTR_ERR(ddr_clk);
		debug("%s: Failed to get DDR clock from dts\n", __func__);
		return ret;
	}

	printf("%s: DDR clock is enabled\n", __func__);

	return 0;
}

static int ddr_start_dfi_init(phys_addr_t umctl2_base,
			      enum ddr_type umctl2_type)
{
	int ret;

	debug("%s: Start DFI init\n", __func__);

	/* Enable quasi-dynamic programing of controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Start DFI init sequence */
	setbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_START);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET),
				DDR4_SWSTAT_SW_DONE_ACK, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
	}

	return ret;
}

static int ddr_check_dfi_init_complete(phys_addr_t umctl2_base,
				       enum ddr_type umctl2_type)
{
	int ret;

	/* Polling DFI init complete */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_DFISTAT_OFFSET),
				DDR4_DFI_INIT_COMPLETE, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" DFI init done\n");
		return ret;
	}

	debug("%s: DFI init completed.\n", __func__);

	/* Enable quasi-dynamic programing of controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Stop DFI init sequence */
	clrbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_START);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET),
				DDR4_SWSTAT_SW_DONE_ACK, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
		return ret;
	}

	debug("%s:DDR programming done\n", __func__);

	return ret;
}

static int ddr_trigger_sdram_init(phys_addr_t umctl2_base,
				  enum ddr_type umctl2_type)
{
	int ret;

	/* Enable quasi-dynamic programing of controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Unmasking dfi init complete */
	setbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_COMPLETE_EN);

	/* Software exit from self-refresh */
	clrbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET, DDR4_PWRCTL_SELFREF_SW);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET),
				DDR4_SWSTAT_SW_DONE_ACK, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
		return ret;
	}

	debug("%s:DDR programming done\n", __func__);
	return ret;
}

static int ddr_post_handoff_config(phys_addr_t umctl2_base,
				   enum ddr_type umctl2_type,
				   bool *need_calibrate)
{
	int ret = 0;
	u32 value;
	u32 start = get_timer(0);

	do {
		if (get_timer(start) > TIMEOUT_200MS) {
			debug("%s: Timeout while waiting for",
			      __func__);
			debug(" DDR enters normal operating mode\n");
			return -ETIMEDOUT;
		}

		udelay(1);
		schedule();

		/* Polling until SDRAM entered normal operating mode */
		value = readl(umctl2_base + DDR4_STAT_OFFSET) &
			      DDR4_STAT_OPERATING_MODE;
	} while (value != OPM_NORMAL);

	printf("DDR entered normal operating mode\n");

	/* Enabling auto refresh */
	clrbits_le32(umctl2_base + DDR4_RFSHCTL3_OFFSET,
		     DDR4_RFSHCTL3_DIS_AUTO_REFRESH);

	/* Checking ECC is enabled? */
	value = readl(umctl2_base + DDR4_ECCCFG0_OFFSET) & DDR4_ECC_MODE;
	if (value)
		printf("ECC is enabled\n");

	if (value && *need_calibrate) {
		ret = scrubber_ddr_config(umctl2_base, umctl2_type);
		if (ret)
			printf("Failed to enable scrubber\n");
	}

	return ret;
}

static int configure_training_firmware(struct ddr_handoff *ddr_handoff_info,
				       const void *train_imem,
				       const void *train_dmem)
{
	int ret = 0;

	printf("Configuring training firmware ...\n");

	/* Reset SDRAM */
	writew(DDR_PHY_PROTECT_MEMRESET,
	       (uintptr_t)(ddr_handoff_info->phy_base +
	       DDR_PHY_MEMRESETL_OFFSET));

	/* Enable access to the PHY configuration registers */
	clrbits_le16(ddr_handoff_info->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	/* Copy train IMEM bin */
	memcpy((void *)ddr_handoff_info->train_imem_base, train_imem,
	       ddr_handoff_info->train_imem_length);

	ret = memcmp((void *)ddr_handoff_info->train_imem_base, train_imem,
		     ddr_handoff_info->train_imem_length);
	if (ret) {
		debug("%s: Failed to copy train IMEM binary\n", __func__);
		/* Isolate the APB access from internal CSRs */
		setbits_le16(ddr_handoff_info->phy_base +
			     DDR_PHY_APBONLY0_OFFSET, DDR_PHY_MICROCONTMUXSEL);
		return ret;
	}

	memcpy((void *)ddr_handoff_info->train_dmem_base, train_dmem,
	       ddr_handoff_info->train_dmem_length);

	ret = memcmp((void *)ddr_handoff_info->train_dmem_base, train_dmem,
		     ddr_handoff_info->train_dmem_length);
	if (ret)
		debug("%s: Failed to copy train DMEM binary\n", __func__);

	/* Isolate the APB access from internal CSRs */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	return ret;
}

static void calibrating_sdram(struct ddr_handoff *ddr_handoff_info)
{
	/* Init mailbox protocol - set 1 to DCTWRITEPROT[0] */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	/* Init mailbox protocol - set 1 to UCTWRITEPROT[0] */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_UCTWRITEPROT_OFFSET,
		     DDR_PHY_UCTWRITEPROT);

	/* Reset and stalling ARC processor */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_MICRORESET_OFFSET,
		     DDR_PHY_MICRORESET_RESET | DDR_PHY_MICRORESET_STALL);

	/* Release ARC processor */
	clrbits_le16(ddr_handoff_info->phy_base + DDR_PHY_MICRORESET_OFFSET,
		     DDR_PHY_MICRORESET_RESET);

	/* Starting PHY firmware execution */
	clrbits_le16(ddr_handoff_info->phy_base + DDR_PHY_MICRORESET_OFFSET,
		     DDR_PHY_MICRORESET_STALL);
}

static int get_mail(struct ddr_handoff *handoff, enum message_mode mode,
		    u32 *message_id)
{
	int ret;

	/* Polling major messages from PMU */
	ret = wait_for_bit_le16((const void *)(handoff->phy_base +
				DDR_PHY_UCTSHADOWREGS_OFFSET),
				DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW,
				false, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for",
		      __func__);
		debug(" major messages from PMU\n");
		return ret;
	}

	*message_id = readw((uintptr_t)(handoff->phy_base +
			    DDR_PHY_UCTWRITEONLYSHADOW_OFFSET));

	if (mode == STREAMING_MESSAGE)
		*message_id |= readw((uintptr_t)((handoff->phy_base +
				     DDR_PHY_UCTDATWRITEONLYSHADOW_OFFSET))) <<
				     SZ_16;

	/* Ack the receipt of the major message */
	clrbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	ret = wait_for_bit_le16((const void *)(handoff->phy_base +
				DDR_PHY_UCTSHADOWREGS_OFFSET),
				DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for",
		      __func__);
		debug(" ack the receipt of the major message completed\n");
		return ret;
	}

	/* Complete protocol */
	setbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	return ret;
}

static int get_mail_streaming(struct ddr_handoff *handoff,
			      enum message_mode mode, u32 *index)
{
	int ret;

	*index = readw((uintptr_t)(handoff->phy_base +
		       DDR_PHY_UCTWRITEONLYSHADOW_OFFSET));

	if (mode == STREAMING_MESSAGE)
		*index |= readw((uintptr_t)((handoff->phy_base +
				DDR_PHY_UCTDATWRITEONLYSHADOW_OFFSET))) <<
				SZ_16;

	/* Ack the receipt of the major message */
	clrbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	ret = wait_for_bit_le16((const void *)(handoff->phy_base +
				DDR_PHY_UCTSHADOWREGS_OFFSET),
				DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for",
		      __func__);
		debug(" ack the receipt of the major message completed\n");
		return ret;
	}

	/* Complete protocol */
	setbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	return 0;
}

static int decode_streaming_message(struct ddr_handoff *ddr_handoff_info,
				    u32 *streaming_index)
{
	int i = 0, ret;
	u32 temp;

	temp = *streaming_index;

	while (i < GET_LOWHW_DATA(temp)) {
		ret = get_mail(ddr_handoff_info, STREAMING_MESSAGE,
			       streaming_index);
		if (ret)
			return ret;

		printf("args[%d]: 0x%x ", i, *streaming_index);
		i++;
	}

	return 0;
}

static int poll_for_training_complete(struct ddr_handoff *ddr_handoff_info)
{
	int ret;
	u32 message_id = 0;
	u32 streaming_index = 0;

	do {
		ret = get_mail(ddr_handoff_info, MAJOR_MESSAGE, &message_id);
		if (ret)
			return ret;

		 printf("Major message id = 0%x\n", message_id);

		if (message_id == FW_STREAMING_MSG_ID) {
			ret = get_mail_streaming(ddr_handoff_info,
						 STREAMING_MESSAGE,
						 &streaming_index);
			if (ret)
				return ret;

			printf("streaming index 0%x : ", streaming_index);

			decode_streaming_message(ddr_handoff_info,
						 &streaming_index);

			printf("\n");
		}
	} while ((message_id != FW_TRAINING_COMPLETED_STAT) &&
	       (message_id != FW_TRAINING_FAILED_STAT));

	if (message_id == FW_TRAINING_COMPLETED_STAT) {
		printf("DDR firmware training completed\n");
	} else if (message_id == FW_TRAINING_FAILED_STAT) {
		printf("DDR firmware training failed\n");
		hang();
	}

	return 0;
}

static void enable_phy_clk_for_csr_access(struct ddr_handoff *handoff,
					  bool enable)
{
	if (enable) {
		/* Enable PHY clk */
		setbits_le16((uintptr_t)(handoff->phy_base +
			     DDR_PHY_UCCLKHCLKENABLES_OFFSET),
			     DDR_PHY_UCCLKHCLKENABLES_UCCLKEN |
			     DDR_PHY_UCCLKHCLKENABLES_HCLKEN);
	} else {
		/* Disable PHY clk */
		clrbits_le16((uintptr_t)(handoff->phy_base +
			     DDR_PHY_UCCLKHCLKENABLES_OFFSET),
			     DDR_PHY_UCCLKHCLKENABLES_UCCLKEN |
			     DDR_PHY_UCCLKHCLKENABLES_HCLKEN);
	}
}

/* helper function for updating train result to umctl2 RANKCTL register */
static void set_cal_res_to_rankctrl(u32 reg_addr, u16 update_value,
				    u32 mask, u32 msb_mask, u32 shift)
{
	u32 reg, value;

	reg = readl((uintptr_t)reg_addr);

	debug("max value divided by 2 is 0x%x\n", update_value);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("updating with train result\n");

	value = (reg & mask) >> shift;

	value += update_value + 3;

	/* reg value greater than 0xF, set one to diff_rank_wr_gap_msb */
	if (value > 0xF)
		setbits_le32((u32 *)(uintptr_t)reg_addr, msb_mask);
	else
		clrbits_le32((u32 *)(uintptr_t)reg_addr, msb_mask);

	reg = readl((uintptr_t)reg_addr);

	value = (value << shift) & mask;

	/* update register */
	writel((reg & (~mask)) | value, (uintptr_t)reg_addr);

	reg = readl((uintptr_t)reg_addr);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("updating with train result\n");
}

/* helper function for updating train result to register */
static void set_cal_res_to_reg(u32 reg_addr, u16 update_value, u32 mask,
			       u32 shift)
{
	u32 reg, value;

	reg = readl((uintptr_t)reg_addr);

	debug("max value divided by 2 is 0x%x\n", update_value);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("updating with train result\n");

	value = (reg & mask) >> shift;

	value = ((value + update_value + 3) << shift) & mask;

	/* update register */
	writel((reg & (~mask)) | value, (uintptr_t)reg_addr);

	reg = readl((uintptr_t)reg_addr);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("updating with train result\n");
}

static u16 get_max_txdqsdlytg0_ux_p0(struct ddr_handoff *handoff, u32 reg,
				     u8 numdbyte, u16 upd_val)
{
	u32 b_addr;
	u16 val;
	u8 byte;

	/* Getting max value from DBYTEx TxDqsDlyTg0_ux_p0 */
	for (byte = 0; byte < numdbyte; byte++) {
		b_addr = byte << 13;

		/* TxDqsDlyTg0[9:6] is the coarse delay */
		val = (readw((uintptr_t)(handoff->phy_base +
			     reg + b_addr)) &
			     DDR_PHY_TXDQDLYTG0_COARSE_DELAY) >>
			     DDR_PHY_TXDQDLYTG0_COARSE_DELAY_SHIFT;

		upd_val = max(val, upd_val);
	}

	return upd_val;
}

static int set_cal_res_to_umctl2(struct ddr_handoff *handoff,
				 phys_addr_t umctl2_base,
				 enum ddr_type umctl2_type)
{
	int ret;
	u8 numdbyte = 0x8;
	u16 upd_val, val;
	u32 dramtmg2_reg_addr, rankctl_reg_addr, reg_addr;

	/* Enable quasi-dynamic programing of the controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Enable access to the PHY configuration registers */
	clrbits_le16(handoff->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	if (umctl2_type == DDRTYPE_DDR4) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_WW_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_WW_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_0) {
		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_WW_1_0_OFFSET)));

		upd_val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
					 DMEM_MB_CDD_CHA_WW_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_1) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_WW_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_CHB_WW_0_1_OFFSET)));
	} else {
		debug("%s: Invalid DDR controller type\n", __func__);
		return -ENXIO;
	}

	upd_val = max(val, upd_val);
	debug("max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	debug("Update train value to umctl2 RANKCTL.diff_rank_wr_gap\n");
	rankctl_reg_addr = umctl2_base + DDR4_RANKCTL_OFFSET;
	/* Update train value to umctl2 RANKCTL.diff_rank_wr_gap */
	set_cal_res_to_rankctrl(rankctl_reg_addr, upd_val,
				DDR4_RANKCTL_DIFF_RANK_WR_GAP,
				DDR4_RANKCTL_DIFF_RANK_WR_GAP_MSB,
				DDR4_RANKCTL_DIFF_RANK_WR_GAP_SHIFT);

	debug("Update train value to umctl2 DRAMTMG2.W2RD\n");
	dramtmg2_reg_addr = umctl2_base + DDR4_DRAMTMG2_OFFSET;
	/* Update train value to umctl2 dramtmg2.wr2rd */
	set_cal_res_to_reg(dramtmg2_reg_addr, upd_val, DDR4_DRAMTMG2_WR2RD, 0);

	if (umctl2_type == DDRTYPE_DDR4) {
		debug("Update train value to umctl2 DRAMTMG9.W2RD_S\n");
		reg_addr = umctl2_base + DDR4_DRAMTMG9_OFFSET;
		/* Update train value to umctl2 dramtmg9.wr2rd_s */
		set_cal_res_to_reg(reg_addr, upd_val, DDR4_DRAMTMG9_W2RD_S, 0);
	}

	if (umctl2_type == DDRTYPE_DDR4) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_RR_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_RR_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_0) {
		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_RR_1_0_OFFSET)));

		upd_val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
					 DMEM_MB_CDD_CHA_RR_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_1) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_RR_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_CHB_RR_0_1_OFFSET)));
	}

	upd_val = max(val, upd_val);
	debug("max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	debug("Update train value to umctl2 RANKCTL.diff_rank_rd_gap\n");
	/* Update train value to umctl2 RANKCTL.diff_rank_rd_gap */
	set_cal_res_to_rankctrl(rankctl_reg_addr, upd_val,
				DDR4_RANKCTL_DIFF_RANK_RD_GAP,
				DDR4_RANKCTL_DIFF_RANK_RD_GAP_MSB,
				DDR4_RANKCTL_DIFF_RANK_RD_GAP_SHIFT);

	if (umctl2_type == DDRTYPE_DDR4) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_RW_1_1_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_RW_1_0_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_RW_0_1_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_RW_0_0_OFFSET)));

		upd_val = max(val, upd_val);
	} else if (umctl2_type == DDRTYPE_LPDDR4_0) {
		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_RW_1_1_OFFSET)));

		upd_val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
					 DMEM_MB_CDD_CHA_RW_1_0_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_RW_0_1_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHA_RW_0_0_OFFSET)));

		upd_val = max(val, upd_val);
	} else if (umctl2_type == DDRTYPE_LPDDR4_1) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_RW_1_1_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_CHB_RW_1_0_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_RW_0_1_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHB_RW_0_0_OFFSET)));

		upd_val = max(val, upd_val);
	}

	debug("max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	debug("Update train value to umctl2 dramtmg2.rd2wr\n");
	/* Update train value to umctl2 dramtmg2.rd2wr */
	set_cal_res_to_reg(dramtmg2_reg_addr, upd_val, DDR4_DRAMTMG2_RD2WR,
			   DDR4_DRAMTMG2_RD2WR_SHIFT);

	/* Checking ECC is enabled?, lpddr4 using inline ECC */
	val = readl(umctl2_base + DDR4_ECCCFG0_OFFSET) & DDR4_ECC_MODE;
	if (val && umctl2_type == DDRTYPE_DDR4)
		numdbyte = 0x9;

	upd_val = 0;

	/* Getting max value from DBYTEx TxDqsDlyTg0_u0_p0 */
	upd_val = get_max_txdqsdlytg0_ux_p0(handoff,
					    DDR_PHY_DBYTE0_TXDQDLYTG0_U0_P0,
					    numdbyte, upd_val);

	/* Getting max value from DBYTEx TxDqsDlyTg0_u1_p0 */
	upd_val = get_max_txdqsdlytg0_ux_p0(handoff,
					    DDR_PHY_DBYTE0_TXDQDLYTG0_U1_P0,
					    numdbyte, upd_val);

	debug("TxDqsDlyTg0 max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	reg_addr = umctl2_base + DDR4_DFITMG1_OFFSET;
	/* Update train value to umctl2 dfitmg1.dfi_wrdata_delay */
	set_cal_res_to_reg(reg_addr, upd_val, DDR4_DFITMG1_DFI_T_WRDATA_DELAY,
			   DDR4_DFITMG1_DFI_T_WRDATA_SHIFT);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET), DDR4_SWSTAT_SW_DONE_ACK,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
	}

	/* Isolate the APB access from internal CSRs */
	setbits_le16(handoff->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	return ret;
}

static int update_training_result(struct ddr_handoff *ddr_handoff_info)
{
	int ret = 0;

	/* Updating training result to first DDR controller */
	if (ddr_handoff_info->cntlr_t == DDRTYPE_DDR4 ||
	    ddr_handoff_info->cntlr_t == DDRTYPE_LPDDR4_0) {
		ret = set_cal_res_to_umctl2(ddr_handoff_info,
					    ddr_handoff_info->cntlr_base,
					    ddr_handoff_info->cntlr_t);
		if (ret) {
			debug("%s: Failed to update train result to ",
			      __func__);
			debug("first DDR controller\n");
			return ret;
		}
	}

	/* Updating training result to 2nd DDR controller */
	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1) {
		ret = set_cal_res_to_umctl2(ddr_handoff_info,
					    ddr_handoff_info->cntlr2_base,
					    ddr_handoff_info->cntlr2_t);
		if (ret) {
			debug("%s: Failed to update train result to ",
			      __func__);
			debug("2nd DDR controller\n");
		}
	}

	return ret;
}

static int start_ddr_calibration(struct ddr_handoff *ddr_handoff_info)
{
	int ret;

	/* Implement 1D training firmware */
	ret = configure_training_firmware(ddr_handoff_info,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_IMEM_1D_SECTION,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_DMEM_1D_SECTION);
	if (ret) {
		debug("%s: Failed to configure 1D training firmware\n",
		      __func__);
		return ret;
	}

	calibrating_sdram(ddr_handoff_info);

	ret = poll_for_training_complete(ddr_handoff_info);
	if (ret) {
		debug("%s: Failed to get FW training completed\n",
		      __func__);
		return ret;
	}

	/* Updating training result to DDR controller */
	ret = update_training_result(ddr_handoff_info);
	if (ret)
		return ret;

	/* Implement 2D training firmware */
	ret = configure_training_firmware(ddr_handoff_info,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_IMEM_2D_SECTION,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_DMEM_2D_SECTION);
	if (ret) {
		debug("%s: Failed to update train result to ", __func__);
		debug("DDR controller\n");
		return ret;
	}

	calibrating_sdram(ddr_handoff_info);

	ret = poll_for_training_complete(ddr_handoff_info);
	if (ret)
		debug("%s: Failed to get FW training completed\n",
		      __func__);

	return ret;
}

static int init_controller(struct ddr_handoff *ddr_handoff_info,
			   u32 *user_backup, u32 *user_backup_2nd)
{
	int ret = 0;

	if (ddr_handoff_info->cntlr_t == DDRTYPE_DDR4  ||
	    ddr_handoff_info->cntlr_t == DDRTYPE_LPDDR4_0) {
		/* Initialize 1st DDR controller */
		ret = init_umctl2(ddr_handoff_info->cntlr_handoff_base,
				  ddr_handoff_info->cntlr_base,
				  ddr_handoff_info->cntlr_t,
				  ddr_handoff_info->cntlr_handoff_length,
				  user_backup);
		if (ret) {
			debug("%s: Failed to inilialize first controller\n",
			      __func__);
			return ret;
		}
	}

	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1) {
		/* Initialize 2nd DDR controller */
		ret = init_umctl2(ddr_handoff_info->cntlr2_handoff_base,
				  ddr_handoff_info->cntlr2_base,
				  ddr_handoff_info->cntlr2_t,
				  ddr_handoff_info->cntlr2_handoff_length,
				  user_backup_2nd);
		if (ret)
			debug("%s: Failed to inilialize 2nd controller\n",
			      __func__);
	}

	return ret;
}

static int dfi_init(struct ddr_handoff *ddr_handoff_info)
{
	int ret;

	ret = ddr_start_dfi_init(ddr_handoff_info->cntlr_base,
				 ddr_handoff_info->cntlr_t);
	if (ret)
		return ret;

	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_start_dfi_init(ddr_handoff_info->cntlr2_base,
					 ddr_handoff_info->cntlr2_t);

	return ret;
}

static int check_dfi_init(struct ddr_handoff *handoff)
{
	int ret;

	ret = ddr_check_dfi_init_complete(handoff->cntlr_base,
					  handoff->cntlr_t);
	if (ret)
		return ret;

	if (handoff->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_check_dfi_init_complete(handoff->cntlr2_base,
						  handoff->cntlr2_t);

	return ret;
}

static int trigger_sdram_init(struct ddr_handoff *handoff)
{
	int ret;

	ret = ddr_trigger_sdram_init(handoff->cntlr_base,
				     handoff->cntlr_t);
	if (ret)
		return ret;

	if (handoff->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_trigger_sdram_init(handoff->cntlr2_base,
					     handoff->cntlr2_t);

	return ret;
}

static int ddr_post_config(struct ddr_handoff *handoff, bool *need_calibrate)
{
	int ret;

	ret = ddr_post_handoff_config(handoff->cntlr_base, handoff->cntlr_t,
				      need_calibrate);
	if (ret)
		return ret;

	if (handoff->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_post_handoff_config(handoff->cntlr2_base,
					      handoff->cntlr2_t,
					      need_calibrate);

	return ret;
}

void reset_type_debug_print(u32 boot_scratch_cold0_reg)
{
	switch (get_reset_type(boot_scratch_cold0_reg)) {
	case POR_RESET:
		debug("%s: POR is triggered\n", __func__);
		break;
	case WARM_RESET:
		debug("%s: Warm reset is triggered\n", __func__);
		break;
	case COLD_RESET:
		debug("%s: Cold reset is triggered\n", __func__);
		break;
	default:
		debug("%s: Invalid reset type\n", __func__);
	}
}

int sdram_mmr_init_full(struct udevice *dev)
{
	u32 user_backup[2], user_backup_2nd[2];
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD0);
	int ret;
	bool is_ddr_hang_be4_rst = is_ddr_init_hang();
	bool need_calibrate = true;
	ulong ddr_offset;
	char *endptr;
	const char *offset = get_ddrcal_ddr_offset();
	struct bd_info bd;
	struct ddr_handoff ddr_handoff_info;
	struct altera_sdram_priv *priv = dev_get_priv(dev);

	printf("Checking SDRAM configuration in progress ...\n");
	ret = populate_ddr_handoff(&ddr_handoff_info);
	if (ret) {
		debug("%s: Failed to populate DDR handoff\n",
		      __func__);
		return ret;
	}

	/* Set the MPFE NoC mux to correct DDR controller type */
	use_ddr4(ddr_handoff_info.cntlr_t);

	/*
	 * Invalidate the section used for processing the PHY backup
	 * calibration data
	 */
	writel(SOC64_CRAM_PHY_BACKUP_SKIP_MAGIC, SOC64_OCRAM_PHY_BACKUP_BASE);

	if (!is_ddr_init_skipped(reg, is_ddr_hang_be4_rst)) {
		printf("SDRAM init in progress ...\n");
		ddr_init_inprogress(true);

		/*
		 * Polling reset complete, must be high to ensure DDR
		 * subsystem in complete reset state before init DDR clock
		 * and DDR controller
		 */
		ret = wait_for_bit_le32((const void *)((uintptr_t)readl
					(ddr_handoff_info.mem_reset_base) +
					MEM_RST_MGR_STATUS),
					MEM_RST_MGR_STATUS_RESET_COMPLETE,
					true, TIMEOUT_200MS, false);
		if (ret) {
			debug("%s: Timeout while waiting for", __func__);
			debug(" reset complete done\n");
			return ret;
		}

		ret = enable_ddr_clock(dev);
		if (ret)
			return ret;

		ret = init_controller(&ddr_handoff_info, user_backup,
				      user_backup_2nd);
		if (ret) {
			debug("%s: Failed to inilialize DDR controller\n",
			      __func__);
			return ret;
		}

		/* Release the controller from reset */
		setbits_le32((uintptr_t)
			     readl(ddr_handoff_info.mem_reset_base) +
			     MEM_RST_MGR_STATUS, MEM_RST_MGR_STATUS_AXI_RST |
			     MEM_RST_MGR_STATUS_CONTROLLER_RST |
			     MEM_RST_MGR_STATUS_RESET_COMPLETE);

		printf("DDR controller configuration is completed\n");

		/* Initialize DDR PHY */
		ret = init_phy(&ddr_handoff_info, &need_calibrate,
			       is_ddr_hang_be4_rst);
		if (ret) {
			debug("%s: Failed to inilialize DDR PHY\n", __func__);
			return ret;
		}

		enable_phy_clk_for_csr_access(&ddr_handoff_info, true);

		if (need_calibrate) {
			ret = start_ddr_calibration(&ddr_handoff_info);
			if (ret) {
				debug("%s: Failed to calibrate DDR\n",
				      __func__);
				return ret;
			}

			/* DDR freq set to support DDR4-3200 */
			phy_init_engine(&ddr_handoff_info);

			/*
			 * Backup calibration data to OCRAM first, these data
			 * might be permanant stored to flash in later
			 */
			if (is_ddr_retention_enabled(reg)) {
				ret = cal_data_ocram(ddr_handoff_info.phy_base,
						     SOC64_OCRAM_PHY_BACKUP_BASE,
						     STORE);

				if (ret)
					return ret;
			}

		} else {
			/* Updating training result to DDR controller */
			ret = update_training_result(&ddr_handoff_info);
			if (ret)
				return ret;
		}

		/*
		 * Reset ARC processor when no using for security
		 * purpose
		 */
		setbits_le16(ddr_handoff_info.phy_base +
			     DDR_PHY_MICRORESET_OFFSET,
			     DDR_PHY_MICRORESET_RESET);

		enable_phy_clk_for_csr_access(&ddr_handoff_info, false);

		ret = dfi_init(&ddr_handoff_info);
		if (ret)
			return ret;

		ret = check_dfi_init(&ddr_handoff_info);
		if (ret)
			return ret;

		ret = trigger_sdram_init(&ddr_handoff_info);
		if (ret)
			return ret;

		ret = ddr_post_config(&ddr_handoff_info, &need_calibrate);
		if (ret)
			return ret;

		/* Restore user settings */
		writel(user_backup[0], ddr_handoff_info.cntlr_base +
		       DDR4_PWRCTL_OFFSET);

		if (ddr_handoff_info.cntlr2_t == DDRTYPE_LPDDR4_0)
			setbits_le32(ddr_handoff_info.cntlr_base +
				     DDR4_INIT0_OFFSET, user_backup[1]);

		if (ddr_handoff_info.cntlr2_t == DDRTYPE_LPDDR4_1) {
			/* Restore user settings */
			writel(user_backup_2nd[0],
			       ddr_handoff_info.cntlr2_base +
			       DDR4_PWRCTL_OFFSET);

			setbits_le32(ddr_handoff_info.cntlr2_base +
				     DDR4_INIT0_OFFSET, user_backup_2nd[1]);
		}

		/* Enable input traffic per port */
		setbits_le32(ddr_handoff_info.cntlr_base + DDR4_PCTRL0_OFFSET,
			     DDR4_PCTRL0_PORT_EN);

		if (ddr_handoff_info.cntlr2_t == DDRTYPE_LPDDR4_1) {
			/* Enable input traffic per port */
			setbits_le32(ddr_handoff_info.cntlr2_base +
				     DDR4_PCTRL0_OFFSET, DDR4_PCTRL0_PORT_EN);
		}

		printf("DDR init success\n");
	}

	/* Get bank configuration from devicetree */
	ret = fdtdec_decode_ram_size(gd->fdt_blob, NULL, 0, NULL,
				     (phys_size_t *)&gd->ram_size, &bd);
	if (ret) {
		debug("%s: Failed to decode memory node\n",  __func__);
		return -1;
	}

	printf("DDR: %lld MiB\n", gd->ram_size >> 20);

	priv->info.base = bd.bi_dram[0].start;
	priv->info.size = gd->ram_size;

	sdram_size_check(&bd);

	/* Marking end of ddr init with passing basic memory test */
	ddr_init_inprogress(false);

	sdram_set_firewall(&bd);

	if (is_ddr_retention_enabled(reg)) {
		ddr_offset = simple_strtoul(offset, &endptr, 16);
		if (!(offset == endptr || *endptr != '\0'))
			memcpy((void *)ddr_offset,
			       (const void *)SOC64_OCRAM_PHY_BACKUP_BASE,
			       SZ_4K);
	}

	return 0;
}
