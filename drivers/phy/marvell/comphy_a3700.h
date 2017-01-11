/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _COMPHY_A3700_H_
#define _COMPHY_A3700_H_

#include "comphy.h"
#include "comphy_hpipe.h"

#define MVEBU_REG(offs)			((uintptr_t)MVEBU_REGISTER(offs))

#define DEFAULT_REFCLK_MHZ		25
#define PLL_SET_DELAY_US		600
#define PLL_LOCK_TIMEOUT		1000
#define POLL_16B_REG			1
#define POLL_32B_REG			0

/*
 * COMPHY SB definitions
 */
#define COMPHY_SEL_ADDR			MVEBU_REG(0x0183FC)
#define rf_compy_select(lane)		(0x1 << (((lane) == 1) ? 4 : 0))

#define COMPHY_PHY_CFG1_ADDR(lane)	MVEBU_REG(0x018300 + (lane) * 0x28)
#define rb_pin_pu_iveref		BIT(1)
#define rb_pin_reset_core		BIT(11)
#define rb_pin_reset_comphy		BIT(12)
#define rb_pin_pu_pll			BIT(16)
#define rb_pin_pu_rx			BIT(17)
#define rb_pin_pu_tx			BIT(18)
#define rb_pin_tx_idle			BIT(19)
#define rf_gen_rx_sel_shift		22
#define rf_gen_rx_select		(0x0F << rf_gen_rx_sel_shift)
#define rf_gen_tx_sel_shift		26
#define rf_gen_tx_select		(0x0F << rf_gen_tx_sel_shift)
#define rb_phy_rx_init			BIT(30)

#define COMPHY_PHY_STAT1_ADDR(lane)	MVEBU_REG(0x018318 + (lane) * 0x28)
#define rb_rx_init_done			BIT(0)
#define rb_pll_ready_rx			BIT(2)
#define rb_pll_ready_tx			BIT(3)

/*
 * PCIe/USB/SGMII definitions
 */
#define PCIE_BASE			MVEBU_REG(0x070000)
#define PCIETOP_BASE			MVEBU_REG(0x080000)
#define PCIE_RAMBASE			MVEBU_REG(0x08C000)
#define PCIEPHY_BASE			MVEBU_REG(0x01F000)
#define PCIEPHY_SHFT			2

#define USB32_BASE			MVEBU_REG(0x050000) /* usb3 device */
#define USB32H_BASE			MVEBU_REG(0x058000) /* usb3 host */
#define USB3PHY_BASE			MVEBU_REG(0x05C000)
#define USB2PHY_BASE			MVEBU_REG(0x05D000)
#define USB2PHY2_BASE			MVEBU_REG(0x05F000)
#define USB32_CTRL_BASE			MVEBU_REG(0x05D800)
#define USB3PHY_SHFT			2

#define SGMIIPHY_BASE(l)	(l == 1 ? USB3PHY_BASE : PCIEPHY_BASE)
#define SGMIIPHY_ADDR(l, a)	(((a & 0x00007FF) * 2) | SGMIIPHY_BASE(l))

#define phy_read16(l, a)	read16((void __iomem *)SGMIIPHY_ADDR(l, a))
#define phy_write16(l, a, data, mask)	\
	reg_set16((void __iomem *)SGMIIPHY_ADDR(l, a), data, mask)

/* units */
#define PCIE				1
#define USB3				2

#define PHY_BASE(unit)		((unit == PCIE) ? PCIEPHY_BASE : USB3PHY_BASE)
#define PHY_SHFT(unit)		((unit == PCIE) ? PCIEPHY_SHFT : USB3PHY_SHFT)

/* bit definition for USB32_CTRL_BASE (USB32 Control Mode) */
#define usb32_ctrl_id_mode		BIT(0)
#define usb32_ctrl_soft_id		BIT(1)
#define usb32_ctrl_int_mode		BIT(4)


#define PHY_PWR_PLL_CTRL_ADDR	0x01	/* for phy_read16 and phy_write16 */
#define PWR_PLL_CTRL_ADDR(unit)		\
	(PHY_PWR_PLL_CTRL_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define rf_phy_mode_shift		5
#define rf_phy_mode_mask		(0x7 << rf_phy_mode_shift)
#define rf_ref_freq_sel_shift		0
#define rf_ref_freq_sel_mask		(0x1F << rf_ref_freq_sel_shift)
#define PHY_MODE_SGMII			0x4

/* for phy_read16 and phy_write16 */
#define PHY_REG_KVCO_CAL_CTRL_ADDR	0x02
#define KVCO_CAL_CTRL_ADDR(unit)	\
	(PHY_REG_KVCO_CAL_CTRL_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define rb_use_max_pll_rate		BIT(12)
#define rb_force_calibration_done	BIT(9)

/* for phy_read16 and phy_write16 */
#define PHY_DIG_LB_EN_ADDR		0x23
#define DIG_LB_EN_ADDR(unit)		\
	(PHY_DIG_LB_EN_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define rf_data_width_shift		10
#define rf_data_width_mask		(0x3 << rf_data_width_shift)

/* for phy_read16 and phy_write16 */
#define PHY_SYNC_PATTERN_ADDR		0x24
#define SYNC_PATTERN_ADDR(unit)		\
	(PHY_SYNC_PATTERN_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define phy_txd_inv			BIT(10)
#define phy_rxd_inv			BIT(11)

/* for phy_read16 and phy_write16 */
#define PHY_REG_UNIT_CTRL_ADDR		0x48
#define UNIT_CTRL_ADDR(unit)		\
	(PHY_REG_UNIT_CTRL_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define rb_idle_sync_en			BIT(12)

/* for phy_read16 and phy_write16 */
#define PHY_REG_GEN2_SETTINGS_2		0x3e
#define GEN2_SETTING_2_ADDR(unit)	\
	(PHY_REG_GEN2_SETTINGS_2 * PHY_SHFT(unit) + PHY_BASE(unit))
#define g2_tx_ssc_amp			BIT(14)

/* for phy_read16 and phy_write16 */
#define PHY_REG_GEN2_SETTINGS_3		0x3f
#define GEN2_SETTING_3_ADDR(unit)	\
	(PHY_REG_GEN2_SETTINGS_3 * PHY_SHFT(unit) + PHY_BASE(unit))

/* for phy_read16 and phy_write16 */
#define PHY_MISC_REG0_ADDR		0x4f
#define MISC_REG0_ADDR(unit)		\
	(PHY_MISC_REG0_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define rb_clk100m_125m_en		BIT(4)
#define rb_clk500m_en			BIT(7)
#define rb_ref_clk_sel			BIT(10)

/* for phy_read16 and phy_write16 */
#define PHY_REG_IFACE_REF_CLK_CTRL_ADDR		0x51
#define UNIT_IFACE_REF_CLK_CTRL_ADDR(unit)	\
	(PHY_REG_IFACE_REF_CLK_CTRL_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define rb_ref1m_gen_div_force		BIT(8)
#define rf_ref1m_gen_div_value_shift	0
#define rf_ref1m_gen_div_value_mask	(0xFF << rf_ref1m_gen_div_value_shift)

/* for phy_read16 and phy_write16 */
#define PHY_REG_ERR_CNT_CONST_CTRL_ADDR	0x6A
#define UNIT_ERR_CNT_CONST_CTRL_ADDR(unit) \
	(PHY_REG_ERR_CNT_CONST_CTRL_ADDR * PHY_SHFT(unit) + PHY_BASE(unit))
#define rb_fast_dfe_enable		BIT(13)

#define MISC_REG1_ADDR(u)		(0x73 * PHY_SHFT(u) + PHY_BASE(u))
#define bf_sel_bits_pcie_force		BIT(15)

#define LANE_CFG0_ADDR(u)		(0x180 * PHY_SHFT(u) + PHY_BASE(u))
#define bf_use_max_pll_rate		BIT(9)
#define LANE_CFG1_ADDR(u)		(0x181 * PHY_SHFT(u) + PHY_BASE(u))
#define bf_use_max_pll_rate		BIT(9)
/* 0x5c310 = 0x93 (set BIT7) */
#define LANE_CFG4_ADDR(u)		(0x188 * PHY_SHFT(u) + PHY_BASE(u))
#define bf_spread_spectrum_clock_en	BIT(7)

#define LANE_STAT1_ADDR(u)		(0x183 * PHY_SHFT(u) + PHY_BASE(u))
#define rb_txdclk_pclk_en		BIT(0)

#define GLOB_PHY_CTRL0_ADDR(u)		(0x1c1 * PHY_SHFT(u) + PHY_BASE(u))
#define bf_soft_rst			BIT(0)
#define bf_mode_refdiv			0x30
#define rb_mode_core_clk_freq_sel	BIT(9)
#define rb_mode_pipe_width_32		BIT(3)

#define TEST_MODE_CTRL_ADDR(u)		(0x1c2 * PHY_SHFT(u) + PHY_BASE(u))
#define rb_mode_margin_override		BIT(2)

#define GLOB_CLK_SRC_LO_ADDR(u)		(0x1c3 * PHY_SHFT(u) + PHY_BASE(u))
#define bf_cfg_sel_20b			BIT(15)

#define PWR_MGM_TIM1_ADDR(u)		(0x1d0 * PHY_SHFT(u) + PHY_BASE(u))

#define PHY_REF_CLK_ADDR		(0x4814 + PCIE_BASE)

#define USB3_CTRPUL_VAL_REG		(0x20 + USB32_BASE)
#define USB3H_CTRPUL_VAL_REG		(0x3454 + USB32H_BASE)
#define rb_usb3_ctr_100ns		0xff000000

#define USB2_OTG_PHY_CTRL_ADDR		(0x820 + USB2PHY_BASE)
#define rb_usb2phy_suspm		BIT(14)
#define rb_usb2phy_pu			BIT(0)

#define USB2_PHY_OTG_CTRL_ADDR		(0x34 + USB2PHY_BASE)
#define rb_pu_otg			BIT(4)

#define USB2_PHY_CHRGR_DET_ADDR		(0x38 + USB2PHY_BASE)
#define rb_cdp_en			BIT(2)
#define rb_dcp_en			BIT(3)
#define rb_pd_en			BIT(4)
#define rb_pu_chrg_dtc			BIT(5)
#define rb_cdp_dm_auto			BIT(7)
#define rb_enswitch_dp			BIT(12)
#define rb_enswitch_dm			BIT(13)

#define USB2_CAL_CTRL_ADDR		(0x8 + USB2PHY_BASE)
#define rb_usb2phy_pllcal_done		BIT(31)
#define rb_usb2phy_impcal_done		BIT(23)

#define USB2_PLL_CTRL0_ADDR		(0x0 + USB2PHY_BASE)
#define rb_usb2phy_pll_ready		BIT(31)

#define USB2_RX_CHAN_CTRL1_ADDR		(0x18 + USB2PHY_BASE)
#define rb_usb2phy_sqcal_done		BIT(31)

#define USB2_PHY2_CTRL_ADDR		(0x804 + USB2PHY2_BASE)
#define rb_usb2phy2_suspm		BIT(7)
#define rb_usb2phy2_pu			BIT(0)
#define USB2_PHY2_CAL_CTRL_ADDR		(0x8 + USB2PHY2_BASE)
#define USB2_PHY2_PLL_CTRL0_ADDR	(0x0 + USB2PHY2_BASE)
#define USB2_PHY2_RX_CHAN_CTRL1_ADDR	(0x18 + USB2PHY2_BASE)

#define USB2_PHY_BASE(usb32)	(usb32 == 0 ? USB2PHY2_BASE : USB2PHY_BASE)
#define USB2_PHY_CTRL_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_CTRL_ADDR : USB2_OTG_PHY_CTRL_ADDR)
#define RB_USB2PHY_SUSPM(usb32) \
	(usb32 == 0 ? rb_usb2phy2_suspm : rb_usb2phy_suspm)
#define RB_USB2PHY_PU(usb32) \
	(usb32 == 0 ? rb_usb2phy2_pu : rb_usb2phy_pu)
#define USB2_PHY_CAL_CTRL_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_CAL_CTRL_ADDR : USB2_CAL_CTRL_ADDR)
#define USB2_PHY_RX_CHAN_CTRL1_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_RX_CHAN_CTRL1_ADDR : USB2_RX_CHAN_CTRL1_ADDR)
#define USB2_PHY_PLL_CTRL0_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_PLL_CTRL0_ADDR : USB2_PLL_CTRL0_ADDR)

/*
 * SATA definitions
 */
#define AHCI_BASE			MVEBU_REG(0xE0000)

#define rh_vsreg_addr			(AHCI_BASE + 0x178)
#define rh_vsreg_data			(AHCI_BASE + 0x17C)
#define rh_vs0_a			(AHCI_BASE + 0xA0)
#define rh_vs0_d			(AHCI_BASE + 0xA4)

#define vphy_sync_pattern_reg		0x224
#define bs_txd_inv			BIT(10)
#define bs_rxd_inv			BIT(11)

#define vphy_loopback_reg0		0x223
#define bs_phyintf_40bit		0x0C00
#define bs_pll_ready_tx			0x10

#define vphy_power_reg0			0x201

#define vphy_calctl_reg			0x202
#define bs_max_pll_rate			BIT(12)

#define vphy_reserve_reg		0x0e
#define bs_phyctrl_frm_pin		BIT(13)

#define vsata_ctrl_reg			0x00
#define bs_phy_pu_pll			BIT(6)

/*
 * SDIO/eMMC definitions
 */
#define SDIO_BASE			MVEBU_REG(0xD8000)

#define SDIO_HOST_CTRL1_ADDR		(SDIO_BASE + 0x28)
#define SDIO_SDHC_FIFO_ADDR		(SDIO_BASE + 0x12C)
#define SDIO_CAP_12_ADDR		(SDIO_BASE + 0x40)
#define SDIO_ENDIAN_ADDR		(SDIO_BASE + 0x1A4)
#define SDIO_PHY_TIMING_ADDR		(SDIO_BASE + 0x170)
#define SDIO_PHY_PAD_CTRL0_ADDR		(SDIO_BASE + 0x178)
#define SDIO_DLL_RST_ADDR		(SDIO_BASE + 0x148)

#endif /* _COMPHY_A3700_H_ */
