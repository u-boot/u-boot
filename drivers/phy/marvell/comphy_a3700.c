// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "comphy_a3700.h"

DECLARE_GLOBAL_DATA_PTR;

/* Firmware related definitions used for SMC calls */
#define MV_SIP_COMPHY_POWER_ON	0x82000001

#define COMPHY_FW_MODE_FORMAT(mode, invert)	((mode) << 12 | (invert) << 0)

#define COMPHY_SATA_MODE	0x1
#define COMPHY_SGMII_MODE	0x2	/* SGMII 1G */
#define COMPHY_HS_SGMII_MODE	0x3	/* SGMII 2.5G */
#define COMPHY_USB3H_MODE	0x4
#define COMPHY_USB3D_MODE	0x5
#define COMPHY_PCIE_MODE	0x6
#define COMPHY_RXAUI_MODE	0x7
#define COMPHY_XFI_MODE		0x8
#define COMPHY_SFI_MODE		0x9
#define COMPHY_USB3_MODE	0xa
#define COMPHY_AP_MODE		0xb

#define A3700_LANE_MAX_NUM	3

static int comphy_smc(u32 function_id, u32 lane, u32 mode)
{
#ifndef CONFIG_MVEBU_PALLADIUM
	struct pt_regs pregs = {0};

	pregs.regs[0] = function_id;
	pregs.regs[1] = lane;
	pregs.regs[2] = mode;

	smc_call(&pregs);

	return pregs.regs[0];
#else
	return 1;
#endif
}

/*
 * comphy_poll_reg
 *
 * return: 1 on success, 0 on timeout
 */
static u32 comphy_poll_reg(void *addr, u32 val, u32 mask, u32 timeout,
			   u8 op_type)
{
	u32 rval = 0xDEAD;

	for (; timeout > 0; timeout--) {
		if (op_type == POLL_16B_REG)
			rval = readw(addr);	/* 16 bit */
		else
			rval = readl(addr) ;	/* 32 bit */

		if ((rval & mask) == val)
			return 1;

		udelay(10000);
	}

	debug("Time out waiting (%p = %#010x)\n", addr, rval);
	return 0;
}

/*
 * comphy_sata_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_sata_power_up(u32 invert, u32 lane)
{
	int	ret;

	debug_enter();

	/*
	 * Set vendor-specific configuration (??).
	 * It was done in the middle of comphy initialization but test shows
	 * that moving it before strict comphy init works ok. Thanks to that the
	 * comphy init can be done with pure comphy range access, not touching
	 * ahci range.
	 */
	reg_set((void __iomem *)rh_vs0_a, vsata_ctrl_reg, 0xFFFFFFFF);
	reg_set((void __iomem *)rh_vs0_d, bs_phy_pu_pll, bs_phy_pu_pll);

	ret = comphy_smc(MV_SIP_COMPHY_POWER_ON, lane,
			 COMPHY_FW_MODE_FORMAT(COMPHY_SATA_MODE, invert));

	return ret;
}

/*
 * comphy_usb3_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_usb3_power_up(u32 speed, u32 invert, u32 lane,
				bool indirect_reg_access)
{
	int	ret;

	debug_enter();

	/*
	 * 1. Power up OTG module
	 */
	reg_set((void __iomem *)USB2_PHY_OTG_CTRL_ADDR, rb_pu_otg, 0);

	/*
	 * 2. Set counter for 100us pulse in USB3 Host and Device
	 * restore default burst size limit (Reference Clock 31:24)
	 */
	reg_set((void __iomem *)USB3_CTRPUL_VAL_REG,
		0x8 << 24, rb_usb3_ctr_100ns);

	ret = comphy_smc(MV_SIP_COMPHY_POWER_ON, lane,
			 COMPHY_FW_MODE_FORMAT(COMPHY_USB3_MODE, invert));

	/* No matter host mode and device mode, it works with Hard ID detection
	 * Unset DP and DM pulldown for USB2 Device mode
	 */
	reg_set((void __iomem *)USB2_OTG_PHY_CTRL_ADDR, 0x0,
		rb_usb2_dp_pulldn_dev_mode | rb_usb2_dm_pulldn_dev_mode);

	/* Disbale VBus interrupt which will be enable again in kernel */
	reg_set((void __iomem *)USB3_TOP_INT_ENABLE_REG, 0x0, vbus_int_enable);

	/* Clear VBus interrupt to prepare a clean state for kernel */
	reg_set((void __iomem *)USB3_TOP_INT_STATUS_REG,
		vbus_int_state, vbus_int_state);

	debug_exit();

	return ret;
}

/*
 * comphy_usb2_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_usb2_power_up(u8 usb32)
{
	int	ret;

	debug_enter();

	if (usb32 != 0 && usb32 != 1) {
		printf("invalid usb32 value: (%d), should be either 0 or 1\n",
		       usb32);
		debug_exit();
		return 0;
	}

	/*
	 * 0. Setup PLL. 40MHz clock uses defaults.
	 *    See "PLL Settings for Typical REFCLK" table
	 */
	if (get_ref_clk() == 25) {
		reg_set((void __iomem *)USB2_PHY_BASE(usb32),
			5 | (96 << 16), 0x3F | (0xFF << 16) | (0x3 << 28));
	}

	/*
	 * 1. PHY pull up and disable USB2 suspend
	 */
	reg_set((void __iomem *)USB2_PHY_CTRL_ADDR(usb32),
		RB_USB2PHY_SUSPM(usb32) | RB_USB2PHY_PU(usb32), 0);

	if (usb32 != 0) {
		/*
		 * 2. Power up OTG module
		 */
		reg_set((void __iomem *)USB2_PHY_OTG_CTRL_ADDR, rb_pu_otg, 0);

		/*
		 * 3. Configure PHY charger detection
		 */
		reg_set((void __iomem *)USB2_PHY_CHRGR_DET_ADDR, 0,
			rb_cdp_en | rb_dcp_en | rb_pd_en | rb_cdp_dm_auto |
			rb_enswitch_dp | rb_enswitch_dm | rb_pu_chrg_dtc);
	}

	/* Assert PLL calibration done */
	ret = comphy_poll_reg((void *)USB2_PHY_CAL_CTRL_ADDR(usb32),
			      rb_usb2phy_pllcal_done,	/* value */
			      rb_usb2phy_pllcal_done,	/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */
	if (ret == 0)
		printf("Failed to end USB2 PLL calibration\n");

	/* Assert impedance calibration done */
	ret = comphy_poll_reg((void *)USB2_PHY_CAL_CTRL_ADDR(usb32),
			      rb_usb2phy_impcal_done,	/* value */
			      rb_usb2phy_impcal_done,	/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */
	if (ret == 0)
		printf("Failed to end USB2 impedance calibration\n");

	/* Assert squetch calibration done */
	ret = comphy_poll_reg((void *)USB2_PHY_RX_CHAN_CTRL1_ADDR(usb32),
			      rb_usb2phy_sqcal_done,	/* value */
			      rb_usb2phy_sqcal_done,	/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */
	if (ret == 0)
		printf("Failed to end USB2 unknown calibration\n");

	/* Assert PLL is ready */
	ret = comphy_poll_reg((void *)USB2_PHY_PLL_CTRL0_ADDR(usb32),
			      rb_usb2phy_pll_ready,		/* value */
			      rb_usb2phy_pll_ready,		/* mask */
			      PLL_LOCK_TIMEOUT,		/* timeout */
			      POLL_32B_REG);		/* 32bit */

	if (ret == 0)
		printf("Failed to lock USB2 PLL\n");

	if (usb32) {
		/*
		 * Disbale VBus interrupt which will be
		 * enable again in kernel
		 */
		reg_set((void __iomem *)USB3_TOP_INT_ENABLE_REG, 0x0,
			vbus_int_enable);
		/* Clear VBus interrupt to prepare a clean state for kernel */
		reg_set((void __iomem *)USB3_TOP_INT_STATUS_REG,
			vbus_int_state, vbus_int_state);
	}

	debug_exit();

	return ret;
}

/*
 * comphy_emmc_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_emmc_power_up(void)
{
	debug_enter();

	/*
	 * 1. Bus power ON, Bus voltage 1.8V
	 */
	reg_set((void __iomem *)SDIO_HOST_CTRL1_ADDR, 0xB00, 0xF00);

	/*
	 * 2. Set FIFO parameters
	 */
	reg_set((void __iomem *)SDIO_SDHC_FIFO_ADDR, 0x315, 0xFFFFFFFF);

	/*
	 * 3. Set Capabilities 1_2
	 */
	reg_set((void __iomem *)SDIO_CAP_12_ADDR, 0x25FAC8B2, 0xFFFFFFFF);

	/*
	 * 4. Set Endian
	 */
	reg_set((void __iomem *)SDIO_ENDIAN_ADDR, 0x00c00000, 0);

	/*
	 * 4. Init PHY
	 */
	reg_set((void __iomem *)SDIO_PHY_TIMING_ADDR, 0x80000000, 0x80000000);
	reg_set((void __iomem *)SDIO_PHY_PAD_CTRL0_ADDR, 0x50000000,
		0xF0000000);

	/*
	 * 5. DLL reset
	 */
	reg_set((void __iomem *)SDIO_DLL_RST_ADDR, 0xFFFEFFFF, 0);
	reg_set((void __iomem *)SDIO_DLL_RST_ADDR, 0x00010000, 0);

	debug_exit();

	return 1;
}

/*
 * comphy_dedicated_phys_init initialize the dedicated PHYs
 * - not muxed SerDes lanes e.g. UTMI PHY
 */
void comphy_dedicated_phys_init(void)
{
	int node, usb32, ret = 1;
	const void *blob = gd->fdt_blob;

	debug_enter();

	for (usb32 = 0; usb32 <= 1; usb32++) {
		/*
		 * There are 2 UTMI PHYs in this SOC.
		 * One is independendent and one is paired with USB3 port (OTG)
		 */
		if (usb32 == 0) {
			node = fdt_node_offset_by_compatible(
				blob, -1, "marvell,armada3700-ehci");
		} else {
			node = fdt_node_offset_by_compatible(
				blob, -1, "marvell,armada3700-xhci");
		}

		if (node > 0) {
			if (fdtdec_get_is_enabled(blob, node)) {
				ret = comphy_usb2_power_up(usb32);
				if (ret == 0)
					printf("Failed to initialize UTMI PHY\n");
				else
					debug("UTMI PHY init succeed\n");
			} else {
				debug("USB%d node is disabled\n",
				      usb32 == 0 ? 2 : 3);
			}
		} else {
			debug("No USB%d node in DT\n", usb32 == 0 ? 2 : 3);
		}
	}

	node = fdt_node_offset_by_compatible(blob, -1,
					     "marvell,armada-8k-sdhci");
	if (node <= 0) {
		node = fdt_node_offset_by_compatible(
			blob, -1, "marvell,armada-3700-sdhci");
	}

	if (node > 0) {
		if (fdtdec_get_is_enabled(blob, node)) {
			ret = comphy_emmc_power_up();
			if (ret == 0)
				printf("Failed to initialize SDIO/eMMC PHY\n");
			else
				debug("SDIO/eMMC PHY init succeed\n");
		} else {
			debug("SDIO/eMMC node is disabled\n");
		}
	}  else {
		debug("No SDIO/eMMC node in DT\n");
	}

	debug_exit();
}

int comphy_a3700_init(struct chip_serdes_phy_config *chip_cfg,
		      struct comphy_map *serdes_map)
{
	struct comphy_map *comphy_map;
	u32 comphy_max_count = chip_cfg->comphy_lanes_count;
	u32 lane, ret = 0;
	u32 mode = 0, fw_format = 0;

	debug_enter();

	if (comphy_max_count > A3700_LANE_MAX_NUM) {
		printf("Comphy number %d is too large\n", comphy_max_count);
		return 1;
	}

	for (lane = 0, comphy_map = serdes_map; lane < comphy_max_count;
	     lane++, comphy_map++) {
		debug("Initialize serdes number %d\n", lane);
		debug("Serdes type = 0x%x invert=%d\n",
		      comphy_map->type, comphy_map->invert);

		switch (comphy_map->type) {
		case COMPHY_TYPE_UNCONNECTED:
			continue;
			break;

		case COMPHY_TYPE_PEX0:
			fw_format = COMPHY_FW_MODE_FORMAT(COMPHY_PCIE_MODE,
							  comphy_map->invert);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON, lane,
					 fw_format);
			break;

		case COMPHY_TYPE_USB3:
		case COMPHY_TYPE_USB3_HOST0:
		case COMPHY_TYPE_USB3_DEVICE:

			ret = comphy_usb3_power_up(comphy_map->speed,
						   comphy_map->invert,
						   lane,
						   (lane == 2) ? true : false);
			break;

		case COMPHY_TYPE_SGMII0:
		case COMPHY_TYPE_SGMII1:
			if (comphy_map->speed == COMPHY_SPEED_1_25G)
				mode = COMPHY_SGMII_MODE;
			else if (comphy_map->speed == COMPHY_SPEED_3_125G)
				mode = COMPHY_HS_SGMII_MODE;
			else
				printf("Unsupported COMPHY speed!\n");

			fw_format = COMPHY_FW_MODE_FORMAT(mode,
							  comphy_map->invert);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON, lane,
					 fw_format);

			break;

		case COMPHY_TYPE_SATA0:
			ret = comphy_sata_power_up(comphy_map->invert, lane);

			break;

		default:
			debug("Unknown SerDes type, skip initialize SerDes %d\n",
			      lane);
			ret = 1;
			break;
		}
		if (ret != 0)
			printf("PLL is not locked - Failed to initialize lane %d\n",
			       lane);
	}

	debug_exit();
	return ret;
}
