// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#include <common.h>
#include <fdtdec.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>

#include "comphy_core.h"
#include "comphy_hpipe.h"
#include "sata.h"
#include "utmi_phy.h"

DECLARE_GLOBAL_DATA_PTR;

#define SD_ADDR(base, lane)			(base + 0x1000 * lane)
#define HPIPE_ADDR(base, lane)			(SD_ADDR(base, lane) + 0x800)
#define COMPHY_ADDR(base, lane)			(base + 0x28 * lane)

/* Firmware related definitions used for SMC calls */
#define MV_SIP_COMPHY_POWER_ON	0x82000001
#define MV_SIP_COMPHY_POWER_OFF	0x82000002
#define MV_SIP_COMPHY_PLL_LOCK	0x82000003

/* Used to distinguish between different possible callers (U-boot/Linux) */
#define COMPHY_CALLER_UBOOT			(0x1 << 21)

#define COMPHY_FW_MODE_FORMAT(mode)		((mode) << 12)
#define COMPHY_FW_FORMAT(mode, idx, speeds)	\
			(((mode) << 12) | ((idx) << 8) | ((speeds) << 2))

#define COMPHY_FW_PCIE_FORMAT(pcie_width, clk_src, mode, speeds)	\
			(COMPHY_CALLER_UBOOT | ((pcie_width) << 18) |	\
			((clk_src) << 17) | COMPHY_FW_FORMAT(mode, 0, speeds))

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

/* Comphy unit index macro */
#define COMPHY_UNIT_ID0		0
#define COMPHY_UNIT_ID1		1
#define COMPHY_UNIT_ID2		2
#define COMPHY_UNIT_ID3		3

struct utmi_phy_data {
	void __iomem *utmi_base_addr;
	void __iomem *usb_cfg_addr;
	void __iomem *utmi_cfg_addr;
	u32 utmi_phy_port;
};

/*
 * For CP-110 we have 2 Selector registers "PHY Selectors",
 * and "PIPE Selectors".
 * PIPE selector include USB and PCIe options.
 * PHY selector include the Ethernet and SATA options, every Ethernet
 * option has different options, for example: serdes lane2 had option
 * Eth_port_0 that include (SGMII0, RXAUI0, SFI)
 */
struct comphy_mux_data cp110_comphy_phy_mux_data[] = {
	{4, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_SGMII1, 0x1}, /* Lane 0 */
	     {PHY_TYPE_SATA1, 0x4} } },
	{4, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_SGMII2, 0x1}, /* Lane 1 */
	     {PHY_TYPE_SATA0, 0x4} } },
	{6, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_SGMII0, 0x1}, /* Lane 2 */
	     {PHY_TYPE_RXAUI0, 0x1}, {PHY_TYPE_SFI, 0x1},
	     {PHY_TYPE_SATA0, 0x4} } },
	{8, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_RXAUI1, 0x1}, /* Lane 3 */
	     {PHY_TYPE_SGMII1, 0x2}, {PHY_TYPE_SATA1, 0x4} } },
	{7, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_SGMII0, 0x2}, /* Lane 4 */
	     {PHY_TYPE_RXAUI0, 0x2}, {PHY_TYPE_SFI, 0x2},
	     {PHY_TYPE_SGMII1, 0x1} } },
	{6, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_SGMII2, 0x1}, /* Lane 5 */
	     {PHY_TYPE_RXAUI1, 0x2}, {PHY_TYPE_SATA1, 0x4} } },
};

struct comphy_mux_data cp110_comphy_pipe_mux_data[] = {
	{2, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_PEX0, 0x4} } }, /* Lane 0 */
	{4, {{PHY_TYPE_UNCONNECTED, 0x0}, /* Lane 1 */
	     {PHY_TYPE_USB3_HOST0, 0x1}, {PHY_TYPE_USB3_DEVICE, 0x2},
	     {PHY_TYPE_PEX0, 0x4} } },
	{3, {{PHY_TYPE_UNCONNECTED, 0x0}, /* Lane 2 */
	     {PHY_TYPE_USB3_HOST0, 0x1}, {PHY_TYPE_PEX0, 0x4} } },
	{3, {{PHY_TYPE_UNCONNECTED, 0x0}, /* Lane 3 */
	     {PHY_TYPE_USB3_HOST1, 0x1}, {PHY_TYPE_PEX0, 0x4} } },
	{4, {{PHY_TYPE_UNCONNECTED, 0x0}, /* Lane 4 */
	     {PHY_TYPE_USB3_HOST1, 0x1},
	     {PHY_TYPE_USB3_DEVICE, 0x2}, {PHY_TYPE_PEX1, 0x4} } },
	{2, {{PHY_TYPE_UNCONNECTED, 0x0}, {PHY_TYPE_PEX2, 0x4} } }, /* Lane 5 */
};

static u32 polling_with_timeout(void __iomem *addr, u32 val,
				u32 mask, unsigned long usec_timout)
{
	u32 data;

	do {
		udelay(1);
		data = readl(addr) & mask;
	} while (data != val  && --usec_timout > 0);

	if (usec_timout == 0)
		return data;

	return 0;
}

static int comphy_usb3_power_up(u32 lane, void __iomem *hpipe_base,
				void __iomem *comphy_base)
{
	u32 mask, data, ret = 1;
	void __iomem *hpipe_addr = HPIPE_ADDR(hpipe_base, lane);
	void __iomem *comphy_addr = COMPHY_ADDR(comphy_base, lane);
	void __iomem *addr;

	debug_enter();
	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	mask |= COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	mask |= COMMON_PHY_PHY_MODE_MASK;
	data |= 0x1 << COMMON_PHY_PHY_MODE_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* release from hard reset */
	mask = COMMON_PHY_CFG1_PWR_ON_RESET_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET;
	mask |= COMMON_PHY_CFG1_CORE_RSTN_MASK;
	data |= 0x1 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);

	/* Start comphy Configuration */
	debug("stage: Comphy configuration\n");
	/* Set PIPE soft reset */
	mask = HPIPE_RST_CLK_CTRL_PIPE_RST_MASK;
	data = 0x1 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET;
	/* Set PHY datapath width mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_FIXED_PCLK_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_FIXED_PCLK_OFFSET;
	/* Set Data bus width USB mode for V0 */
	mask |= HPIPE_RST_CLK_CTRL_PIPE_WIDTH_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_PIPE_WIDTH_OFFSET;
	/* Set CORE_CLK output frequency for 250Mhz */
	mask |= HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_MASK;
	data |= 0x0 << HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_OFFSET;
	reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG, data, mask);
	/* Set PLL ready delay for 0x2 */
	reg_set(hpipe_addr + HPIPE_CLK_SRC_LO_REG,
		0x2 << HPIPE_CLK_SRC_LO_PLL_RDY_DL_OFFSET,
		HPIPE_CLK_SRC_LO_PLL_RDY_DL_MASK);
	/* Set reference clock to come from group 1 - 25Mhz */
	reg_set(hpipe_addr + HPIPE_MISC_REG,
		0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET,
		HPIPE_MISC_REFCLK_SEL_MASK);
	/* Set reference frequcency select - 0x2 */
	mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
	data = 0x2 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	/* Set PHY mode to USB - 0x5 */
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x5 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);
	/* Set the amount of time spent in the LoZ state - set for 0x7 */
	reg_set(hpipe_addr + HPIPE_GLOBAL_PM_CTRL,
		0x7 << HPIPE_GLOBAL_PM_RXDLOZ_WAIT_OFFSET,
		HPIPE_GLOBAL_PM_RXDLOZ_WAIT_MASK);
	/* Set max PHY generation setting - 5Gbps */
	reg_set(hpipe_addr + HPIPE_INTERFACE_REG,
		0x1 << HPIPE_INTERFACE_GEN_MAX_OFFSET,
		HPIPE_INTERFACE_GEN_MAX_MASK);
	/* Set select data width 20Bit (SEL_BITS[2:0]) */
	reg_set(hpipe_addr + HPIPE_LOOPBACK_REG,
		0x1 << HPIPE_LOOPBACK_SEL_OFFSET,
		HPIPE_LOOPBACK_SEL_MASK);
	/* select de-emphasize 3.5db */
	reg_set(hpipe_addr + HPIPE_LANE_CONFIG0_REG,
		0x1 << HPIPE_LANE_CONFIG0_TXDEEMPH0_OFFSET,
		HPIPE_LANE_CONFIG0_TXDEEMPH0_MASK);
	/* override tx margining from the MAC */
	reg_set(hpipe_addr + HPIPE_TST_MODE_CTRL_REG,
		0x1 << HPIPE_TST_MODE_CTRL_MODE_MARGIN_OFFSET,
		HPIPE_TST_MODE_CTRL_MODE_MARGIN_MASK);

	/* Start analog paramters from ETP(HW) */
	debug("stage: Analog paramters from ETP(HW)\n");
	/* Set Pin DFE_PAT_DIS -> Bit[1]: PIN_DFE_PAT_DIS = 0x0 */
	mask = HPIPE_LANE_CFG4_DFE_CTRL_MASK;
	data = 0x1 << HPIPE_LANE_CFG4_DFE_CTRL_OFFSET;
	/* Set Override PHY DFE control pins for 0x1 */
	mask |= HPIPE_LANE_CFG4_DFE_OVER_MASK;
	data |= 0x1 << HPIPE_LANE_CFG4_DFE_OVER_OFFSET;
	/* Set Spread Spectrum Clock Enable fot 0x1 */
	mask |= HPIPE_LANE_CFG4_SSC_CTRL_MASK;
	data |= 0x1 << HPIPE_LANE_CFG4_SSC_CTRL_OFFSET;
	reg_set(hpipe_addr + HPIPE_LANE_CFG4_REG, data, mask);
	/* End of analog parameters */

	debug("stage: Comphy power up\n");
	/* Release from PIPE soft reset */
	reg_set(hpipe_addr + HPIPE_RST_CLK_CTRL_REG,
		0x0 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET,
		HPIPE_RST_CLK_CTRL_PIPE_RST_MASK);

	/* wait 15ms - for comphy calibration done */
	debug("stage: Check PLL\n");
	/* Read lane status */
	addr = hpipe_addr + HPIPE_LANE_STATUS1_REG;
	data = HPIPE_LANE_STATUS1_PCLK_EN_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 15000);
	if (data != 0) {
		debug("Read from reg = %p - value = 0x%x\n",
		      hpipe_addr + HPIPE_LANE_STATUS1_REG, data);
		pr_err("HPIPE_LANE_STATUS1_PCLK_EN_MASK is 0\n");
		ret = 0;
	}

	debug_exit();
	return ret;
}

static int comphy_smc(u32 function_id, void __iomem *comphy_base_addr,
		      u32 lane, u32 mode)
{
	struct pt_regs pregs = {0};

	pregs.regs[0] = function_id;
	pregs.regs[1] = (unsigned long)comphy_base_addr;
	pregs.regs[2] = lane;
	pregs.regs[3] = mode;

	smc_call(&pregs);

	/*
	 * TODO: Firmware return 0 on success, temporary map it to u-boot
	 * convention, but after all comphy will be reworked the convention in
	 * u-boot should be change and this conversion removed
	 */
	return pregs.regs[0] ? 0 : 1;
}

static int comphy_sata_power_up(u32 lane, void __iomem *hpipe_base,
				void __iomem *comphy_base_addr, int cp_index,
				u32 type)
{
	u32 mask, data, i, ret = 1;
	void __iomem *sata_base = NULL;
	int sata_node = -1; /* Set to -1 in order to read the first sata node */

	debug_enter();

	/*
	 * Assumption - each CP has only one SATA controller
	 * Calling fdt_node_offset_by_compatible first time (with sata_node = -1
	 * will return the first node always.
	 * In order to parse each CPs SATA node, fdt_node_offset_by_compatible
	 * must be called again (according to the CP id)
	 */
	for (i = 0; i < (cp_index + 1); i++)
		sata_node = fdt_node_offset_by_compatible(
			gd->fdt_blob, sata_node, "marvell,armada-8k-ahci");

	if (sata_node == 0) {
		pr_err("SATA node not found in FDT\n");
		return 0;
	}

	sata_base = (void __iomem *)fdtdec_get_addr_size_auto_noparent(
		gd->fdt_blob, sata_node, "reg", 0, NULL, true);
	if (sata_base == NULL) {
		pr_err("SATA address not found in FDT\n");
		return 0;
	}

	debug("SATA address found in FDT %p\n", sata_base);

	debug("stage: MAC configuration - power down comphy\n");
	/*
	 * MAC configuration powe down comphy use indirect address for
	 * vendor spesific SATA control register
	 */
	reg_set(sata_base + SATA3_VENDOR_ADDRESS,
		SATA_CONTROL_REG << SATA3_VENDOR_ADDR_OFSSET,
		SATA3_VENDOR_ADDR_MASK);
	/* SATA 0 power down */
	mask = SATA3_CTRL_SATA0_PD_MASK;
	data = 0x1 << SATA3_CTRL_SATA0_PD_OFFSET;
	/* SATA 1 power down */
	mask |= SATA3_CTRL_SATA1_PD_MASK;
	data |= 0x1 << SATA3_CTRL_SATA1_PD_OFFSET;
	/* SATA SSU disable */
	mask |= SATA3_CTRL_SATA1_ENABLE_MASK;
	data |= 0x0 << SATA3_CTRL_SATA1_ENABLE_OFFSET;
	/* SATA port 1 disable */
	mask |= SATA3_CTRL_SATA_SSU_MASK;
	data |= 0x0 << SATA3_CTRL_SATA_SSU_OFFSET;
	reg_set(sata_base + SATA3_VENDOR_DATA, data, mask);

	ret = comphy_smc(MV_SIP_COMPHY_POWER_ON, comphy_base_addr, lane, type);

	/*
	 * MAC configuration power up comphy - power up PLL/TX/RX
	 * use indirect address for vendor spesific SATA control register
	 */
	reg_set(sata_base + SATA3_VENDOR_ADDRESS,
		SATA_CONTROL_REG << SATA3_VENDOR_ADDR_OFSSET,
		SATA3_VENDOR_ADDR_MASK);
	/* SATA 0 power up */
	mask = SATA3_CTRL_SATA0_PD_MASK;
	data = 0x0 << SATA3_CTRL_SATA0_PD_OFFSET;
	/* SATA 1 power up */
	mask |= SATA3_CTRL_SATA1_PD_MASK;
	data |= 0x0 << SATA3_CTRL_SATA1_PD_OFFSET;
	/* SATA SSU enable */
	mask |= SATA3_CTRL_SATA1_ENABLE_MASK;
	data |= 0x1 << SATA3_CTRL_SATA1_ENABLE_OFFSET;
	/* SATA port 1 enable */
	mask |= SATA3_CTRL_SATA_SSU_MASK;
	data |= 0x1 << SATA3_CTRL_SATA_SSU_OFFSET;
	reg_set(sata_base + SATA3_VENDOR_DATA, data, mask);

	/* MBUS request size and interface select register */
	reg_set(sata_base + SATA3_VENDOR_ADDRESS,
		SATA_MBUS_SIZE_SELECT_REG << SATA3_VENDOR_ADDR_OFSSET,
		SATA3_VENDOR_ADDR_MASK);
	/* Mbus regret enable */
	reg_set(sata_base + SATA3_VENDOR_DATA,
		0x1 << SATA_MBUS_REGRET_EN_OFFSET, SATA_MBUS_REGRET_EN_MASK);

	ret = comphy_smc(MV_SIP_COMPHY_PLL_LOCK, comphy_base_addr, lane, type);

	debug_exit();
	return ret;
}

static int comphy_rxauii_power_up(u32 lane, void __iomem *hpipe_base,
				  void __iomem *comphy_base)
{
	u32 mask, data, ret = 1;
	void __iomem *hpipe_addr = HPIPE_ADDR(hpipe_base, lane);
	void __iomem *sd_ip_addr = SD_ADDR(hpipe_base, lane);
	void __iomem *comphy_addr = COMPHY_ADDR(comphy_base, lane);
	void __iomem *addr;

	debug_enter();
	debug("stage: RFU configurations - hard reset comphy\n");
	/* RFU configurations - hard reset comphy */
	mask = COMMON_PHY_CFG1_PWR_UP_MASK;
	data = 0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET;
	mask |= COMMON_PHY_CFG1_PIPE_SELECT_MASK;
	data |= 0x0 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET;
	reg_set(comphy_addr + COMMON_PHY_CFG1_REG, data, mask);

	if (lane == 2) {
		reg_set(comphy_base + COMMON_PHY_SD_CTRL1,
			0x1 << COMMON_PHY_SD_CTRL1_RXAUI0_OFFSET,
			COMMON_PHY_SD_CTRL1_RXAUI0_MASK);
	}
	if (lane == 4) {
		reg_set(comphy_base + COMMON_PHY_SD_CTRL1,
			0x1 << COMMON_PHY_SD_CTRL1_RXAUI1_OFFSET,
			COMMON_PHY_SD_CTRL1_RXAUI1_MASK);
	}

	/* Select Baud Rate of Comphy And PD_PLL/Tx/Rx */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_MASK;
	data |= 0xB << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_MASK;
	data |= 0xB << SD_EXTERNAL_CONFIG0_SD_PHY_GEN_TX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG0_HALF_BUS_MODE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_MEDIA_MODE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_MEDIA_MODE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);

	/* release from hard reset */
	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x0 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	mask = SD_EXTERNAL_CONFIG1_RESET_IN_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG1_RESET_IN_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RESET_CORE_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RESET_CORE_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	/* Wait 1ms - until band gap and ref clock ready */
	mdelay(1);

	/* Start comphy Configuration */
	debug("stage: Comphy configuration\n");
	/* set reference clock */
	reg_set(hpipe_addr + HPIPE_MISC_REG,
		0x0 << HPIPE_MISC_REFCLK_SEL_OFFSET,
		HPIPE_MISC_REFCLK_SEL_MASK);
	/* Power and PLL Control */
	mask = HPIPE_PWR_PLL_REF_FREQ_MASK;
	data = 0x1 << HPIPE_PWR_PLL_REF_FREQ_OFFSET;
	mask |= HPIPE_PWR_PLL_PHY_MODE_MASK;
	data |= 0x4 << HPIPE_PWR_PLL_PHY_MODE_OFFSET;
	reg_set(hpipe_addr + HPIPE_PWR_PLL_REG, data, mask);
	/* Loopback register */
	reg_set(hpipe_addr + HPIPE_LOOPBACK_REG,
		0x1 << HPIPE_LOOPBACK_SEL_OFFSET, HPIPE_LOOPBACK_SEL_MASK);
	/* rx control 1 */
	mask = HPIPE_RX_CONTROL_1_RXCLK2X_SEL_MASK;
	data = 0x1 << HPIPE_RX_CONTROL_1_RXCLK2X_SEL_OFFSET;
	mask |= HPIPE_RX_CONTROL_1_CLK8T_EN_MASK;
	data |= 0x1 << HPIPE_RX_CONTROL_1_CLK8T_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_RX_CONTROL_1_REG, data, mask);
	/* DTL Control */
	reg_set(hpipe_addr + HPIPE_PWR_CTR_DTL_REG,
		0x0 << HPIPE_PWR_CTR_DTL_FLOOP_EN_OFFSET,
		HPIPE_PWR_CTR_DTL_FLOOP_EN_MASK);

	/* Set analog paramters from ETP(HW) */
	debug("stage: Analog paramters from ETP(HW)\n");
	/* SERDES External Configuration 2 */
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG2_REG,
		0x1 << SD_EXTERNAL_CONFIG2_PIN_DFE_EN_OFFSET,
		SD_EXTERNAL_CONFIG2_PIN_DFE_EN_MASK);
	/* 0x7-DFE Resolution control */
	reg_set(hpipe_addr + HPIPE_DFE_REG0, 0x1 << HPIPE_DFE_RES_FORCE_OFFSET,
		HPIPE_DFE_RES_FORCE_MASK);
	/* 0xd-G1_Setting_0 */
	reg_set(hpipe_addr + HPIPE_G1_SET_0_REG,
		0xd << HPIPE_G1_SET_0_G1_TX_EMPH1_OFFSET,
		HPIPE_G1_SET_0_G1_TX_EMPH1_MASK);
	/* 0xE-G1_Setting_1 */
	mask = HPIPE_G1_SET_1_G1_RX_SELMUPI_MASK;
	data = 0x1 << HPIPE_G1_SET_1_G1_RX_SELMUPI_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_SELMUPP_MASK;
	data |= 0x1 << HPIPE_G1_SET_1_G1_RX_SELMUPP_OFFSET;
	mask |= HPIPE_G1_SET_1_G1_RX_DFE_EN_MASK;
	data |= 0x1 << HPIPE_G1_SET_1_G1_RX_DFE_EN_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SET_1_REG, data, mask);
	/* 0xA-DFE_Reg3 */
	mask = HPIPE_DFE_F3_F5_DFE_EN_MASK;
	data = 0x0 << HPIPE_DFE_F3_F5_DFE_EN_OFFSET;
	mask |= HPIPE_DFE_F3_F5_DFE_CTRL_MASK;
	data |= 0x0 << HPIPE_DFE_F3_F5_DFE_CTRL_OFFSET;
	reg_set(hpipe_addr + HPIPE_DFE_F3_F5_REG, data, mask);

	/* 0x111-G1_Setting_4 */
	mask = HPIPE_G1_SETTINGS_4_G1_DFE_RES_MASK;
	data = 0x1 << HPIPE_G1_SETTINGS_4_G1_DFE_RES_OFFSET;
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_4_REG, data, mask);

	debug("stage: RFU configurations- Power Up PLL,Tx,Rx\n");
	/* SERDES External Configuration */
	mask = SD_EXTERNAL_CONFIG0_SD_PU_PLL_MASK;
	data = 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_PLL_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_RX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_RX_OFFSET;
	mask |= SD_EXTERNAL_CONFIG0_SD_PU_TX_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG0_SD_PU_TX_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG0_REG, data, mask);


	/* check PLL rx & tx ready */
	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_PLL_RX_MASK |
		SD_EXTERNAL_STATUS0_PLL_TX_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 15000);
	if (data != 0) {
		debug("Read from reg = %p - value = 0x%x\n",
		      sd_ip_addr + SD_EXTERNAL_STATUS0_REG, data);
		pr_err("SD_EXTERNAL_STATUS0_PLL_RX is %d, SD_EXTERNAL_STATUS0_PLL_TX is %d\n",
		      (data & SD_EXTERNAL_STATUS0_PLL_RX_MASK),
		      (data & SD_EXTERNAL_STATUS0_PLL_TX_MASK));
		ret = 0;
	}

	/* RX init */
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG,
		0x1 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET,
		SD_EXTERNAL_CONFIG1_RX_INIT_MASK);

	/* check that RX init done */
	addr = sd_ip_addr + SD_EXTERNAL_STATUS0_REG;
	data = SD_EXTERNAL_STATUS0_RX_INIT_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100);
	if (data != 0) {
		debug("Read from reg = %p - value = 0x%x\n",
		      sd_ip_addr + SD_EXTERNAL_STATUS0_REG, data);
		pr_err("SD_EXTERNAL_STATUS0_RX_INIT is 0\n");
		ret = 0;
	}

	debug("stage: RF Reset\n");
	/* RF Reset */
	mask =  SD_EXTERNAL_CONFIG1_RX_INIT_MASK;
	data = 0x0 << SD_EXTERNAL_CONFIG1_RX_INIT_OFFSET;
	mask |= SD_EXTERNAL_CONFIG1_RF_RESET_IN_MASK;
	data |= 0x1 << SD_EXTERNAL_CONFIG1_RF_RESET_IN_OFFSET;
	reg_set(sd_ip_addr + SD_EXTERNAL_CONFIG1_REG, data, mask);

	debug_exit();
	return ret;
}

static void comphy_utmi_power_down(u32 utmi_index, void __iomem *utmi_base_addr,
				   void __iomem *usb_cfg_addr,
				   void __iomem *utmi_cfg_addr,
				   u32 utmi_phy_port)
{
	u32 mask, data;

	debug_enter();
	debug("stage:  UTMI %d - Power down transceiver (power down Phy), Power down PLL, and SuspendDM\n",
	      utmi_index);
	/* Power down UTMI PHY */
	reg_set(utmi_cfg_addr, 0x0 << UTMI_PHY_CFG_PU_OFFSET,
		UTMI_PHY_CFG_PU_MASK);

	/*
	 * If UTMI connected to USB Device, configure mux prior to PHY init
	 * (Device can be connected to UTMI0 or to UTMI1)
	 */
	if (utmi_phy_port == UTMI_PHY_TO_USB3_DEVICE0) {
		debug("stage:  UTMI %d - Enable Device mode and configure UTMI mux\n",
		      utmi_index);
		/* USB3 Device UTMI enable */
		mask = UTMI_USB_CFG_DEVICE_EN_MASK;
		data = 0x1 << UTMI_USB_CFG_DEVICE_EN_OFFSET;
		/* USB3 Device UTMI MUX */
		mask |= UTMI_USB_CFG_DEVICE_MUX_MASK;
		data |= utmi_index << UTMI_USB_CFG_DEVICE_MUX_OFFSET;
		reg_set(usb_cfg_addr,  data, mask);
	}

	/* Set Test suspendm mode */
	mask = UTMI_CTRL_STATUS0_SUSPENDM_MASK;
	data = 0x1 << UTMI_CTRL_STATUS0_SUSPENDM_OFFSET;
	/* Enable Test UTMI select */
	mask |= UTMI_CTRL_STATUS0_TEST_SEL_MASK;
	data |= 0x1 << UTMI_CTRL_STATUS0_TEST_SEL_OFFSET;
	reg_set(utmi_base_addr + UTMI_CTRL_STATUS0_REG, data, mask);

	/* Wait for UTMI power down */
	mdelay(1);

	debug_exit();
	return;
}

static void comphy_utmi_phy_config(u32 utmi_index, void __iomem *utmi_base_addr,
				   void __iomem *usb_cfg_addr,
				   void __iomem *utmi_cfg_addr,
				   u32 utmi_phy_port)
{
	u32 mask, data;

	debug_exit();
	debug("stage: Configure UTMI PHY %d registers\n", utmi_index);
	/* Reference Clock Divider Select */
	mask = UTMI_PLL_CTRL_REFDIV_MASK;
	data = 0x5 << UTMI_PLL_CTRL_REFDIV_OFFSET;
	/* Feedback Clock Divider Select - 90 for 25Mhz*/
	mask |= UTMI_PLL_CTRL_FBDIV_MASK;
	data |= 0x60 << UTMI_PLL_CTRL_FBDIV_OFFSET;
	/* Select LPFR - 0x0 for 25Mhz/5=5Mhz*/
	mask |= UTMI_PLL_CTRL_SEL_LPFR_MASK;
	data |= 0x0 << UTMI_PLL_CTRL_SEL_LPFR_OFFSET;
	reg_set(utmi_base_addr + UTMI_PLL_CTRL_REG, data, mask);

	/* Impedance Calibration Threshold Setting */
	reg_set(utmi_base_addr + UTMI_CALIB_CTRL_REG,
		0x6 << UTMI_CALIB_CTRL_IMPCAL_VTH_OFFSET,
		UTMI_CALIB_CTRL_IMPCAL_VTH_MASK);

	/* Set LS TX driver strength coarse control */
	mask = UTMI_TX_CH_CTRL_DRV_EN_LS_MASK;
	data = 0x3 << UTMI_TX_CH_CTRL_DRV_EN_LS_OFFSET;
	/* Set LS TX driver fine adjustment */
	mask |= UTMI_TX_CH_CTRL_IMP_SEL_LS_MASK;
	data |= 0x3 << UTMI_TX_CH_CTRL_IMP_SEL_LS_OFFSET;
	reg_set(utmi_base_addr + UTMI_TX_CH_CTRL_REG, data, mask);

	/* Enable SQ */
	mask = UTMI_RX_CH_CTRL0_SQ_DET_MASK;
	data = 0x0 << UTMI_RX_CH_CTRL0_SQ_DET_OFFSET;
	/* Enable analog squelch detect */
	mask |= UTMI_RX_CH_CTRL0_SQ_ANA_DTC_MASK;
	data |= 0x1 << UTMI_RX_CH_CTRL0_SQ_ANA_DTC_OFFSET;
	reg_set(utmi_base_addr + UTMI_RX_CH_CTRL0_REG, data, mask);

	/* Set External squelch calibration number */
	mask = UTMI_RX_CH_CTRL1_SQ_AMP_CAL_MASK;
	data = 0x1 << UTMI_RX_CH_CTRL1_SQ_AMP_CAL_OFFSET;
	/* Enable the External squelch calibration */
	mask |= UTMI_RX_CH_CTRL1_SQ_AMP_CAL_EN_MASK;
	data |= 0x1 << UTMI_RX_CH_CTRL1_SQ_AMP_CAL_EN_OFFSET;
	reg_set(utmi_base_addr + UTMI_RX_CH_CTRL1_REG, data, mask);

	/* Set Control VDAT Reference Voltage - 0.325V */
	mask = UTMI_CHGDTC_CTRL_VDAT_MASK;
	data = 0x1 << UTMI_CHGDTC_CTRL_VDAT_OFFSET;
	/* Set Control VSRC Reference Voltage - 0.6V */
	mask |= UTMI_CHGDTC_CTRL_VSRC_MASK;
	data |= 0x1 << UTMI_CHGDTC_CTRL_VSRC_OFFSET;
	reg_set(utmi_base_addr + UTMI_CHGDTC_CTRL_REG, data, mask);

	debug_exit();
	return;
}

static int comphy_utmi_power_up(u32 utmi_index, void __iomem *utmi_base_addr,
				void __iomem *usb_cfg_addr,
				void __iomem *utmi_cfg_addr, u32 utmi_phy_port)
{
	u32 data, mask, ret = 1;
	void __iomem *addr;

	debug_enter();
	debug("stage: UTMI %d - Power up transceiver(Power up Phy), and exit SuspendDM\n",
	      utmi_index);
	/* Power UP UTMI PHY */
	reg_set(utmi_cfg_addr, 0x1 << UTMI_PHY_CFG_PU_OFFSET,
		UTMI_PHY_CFG_PU_MASK);
	/* Disable Test UTMI select */
	reg_set(utmi_base_addr + UTMI_CTRL_STATUS0_REG,
		0x0 << UTMI_CTRL_STATUS0_TEST_SEL_OFFSET,
		UTMI_CTRL_STATUS0_TEST_SEL_MASK);

	debug("stage: Polling for PLL and impedance calibration done, and PLL ready done\n");
	addr = utmi_base_addr + UTMI_CALIB_CTRL_REG;
	data = UTMI_CALIB_CTRL_IMPCAL_DONE_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100);
	if (data != 0) {
		pr_err("Impedance calibration is not done\n");
		debug("Read from reg = %p - value = 0x%x\n", addr, data);
		ret = 0;
	}

	data = UTMI_CALIB_CTRL_PLLCAL_DONE_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100);
	if (data != 0) {
		pr_err("PLL calibration is not done\n");
		debug("Read from reg = %p - value = 0x%x\n", addr, data);
		ret = 0;
	}

	addr = utmi_base_addr + UTMI_PLL_CTRL_REG;
	data = UTMI_PLL_CTRL_PLL_RDY_MASK;
	mask = data;
	data = polling_with_timeout(addr, data, mask, 100);
	if (data != 0) {
		pr_err("PLL is not ready\n");
		debug("Read from reg = %p - value = 0x%x\n", addr, data);
		ret = 0;
	}

	if (ret)
		debug("Passed\n");
	else
		debug("\n");

	debug_exit();
	return ret;
}

/*
 * comphy_utmi_phy_init initialize the UTMI PHY
 * the init split in 3 parts:
 * 1. Power down transceiver and PLL
 * 2. UTMI PHY configure
 * 3. Powe up transceiver and PLL
 * Note: - Power down/up should be once for both UTMI PHYs
 *       - comphy_dedicated_phys_init call this function if at least there is
 *         one UTMI PHY exists in FDT blob. access to cp110_utmi_data[0] is
 *         legal
 */
static void comphy_utmi_phy_init(u32 utmi_phy_count,
				 struct utmi_phy_data *cp110_utmi_data)
{
	u32 i;

	debug_enter();
	/* UTMI Power down */
	for (i = 0; i < utmi_phy_count; i++) {
		comphy_utmi_power_down(i, cp110_utmi_data[i].utmi_base_addr,
				       cp110_utmi_data[i].usb_cfg_addr,
				       cp110_utmi_data[i].utmi_cfg_addr,
				       cp110_utmi_data[i].utmi_phy_port);
	}
	/* PLL Power down */
	debug("stage: UTMI PHY power down PLL\n");
	for (i = 0; i < utmi_phy_count; i++) {
		reg_set(cp110_utmi_data[i].usb_cfg_addr,
			0x0 << UTMI_USB_CFG_PLL_OFFSET, UTMI_USB_CFG_PLL_MASK);
	}
	/* UTMI configure */
	for (i = 0; i < utmi_phy_count; i++) {
		comphy_utmi_phy_config(i, cp110_utmi_data[i].utmi_base_addr,
				       cp110_utmi_data[i].usb_cfg_addr,
				       cp110_utmi_data[i].utmi_cfg_addr,
				       cp110_utmi_data[i].utmi_phy_port);
	}
	/* UTMI Power up */
	for (i = 0; i < utmi_phy_count; i++) {
		if (!comphy_utmi_power_up(i, cp110_utmi_data[i].utmi_base_addr,
					  cp110_utmi_data[i].usb_cfg_addr,
					  cp110_utmi_data[i].utmi_cfg_addr,
					  cp110_utmi_data[i].utmi_phy_port)) {
			pr_err("Failed to initialize UTMI PHY %d\n", i);
			continue;
		}
		printf("UTMI PHY %d initialized to ", i);
		if (cp110_utmi_data[i].utmi_phy_port ==
		    UTMI_PHY_TO_USB3_DEVICE0)
			printf("USB Device\n");
		else
			printf("USB Host%d\n",
			       cp110_utmi_data[i].utmi_phy_port);
	}
	/* PLL Power up */
	debug("stage: UTMI PHY power up PLL\n");
	for (i = 0; i < utmi_phy_count; i++) {
		reg_set(cp110_utmi_data[i].usb_cfg_addr,
			0x1 << UTMI_USB_CFG_PLL_OFFSET, UTMI_USB_CFG_PLL_MASK);
	}

	debug_exit();
	return;
}

/*
 * comphy_dedicated_phys_init initialize the dedicated PHYs
 * - not muxed SerDes lanes e.g. UTMI PHY
 */
void comphy_dedicated_phys_init(void)
{
	struct utmi_phy_data cp110_utmi_data[MAX_UTMI_PHY_COUNT];
	int node;
	int i;

	debug_enter();
	debug("Initialize USB UTMI PHYs\n");

	/* Find the UTMI phy node in device tree and go over them */
	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
					     "marvell,mvebu-utmi-2.6.0");

	i = 0;
	while (node > 0) {
		/* get base address of UTMI phy */
		cp110_utmi_data[i].utmi_base_addr =
			(void __iomem *)fdtdec_get_addr_size_auto_noparent(
				gd->fdt_blob, node, "reg", 0, NULL, true);
		if (cp110_utmi_data[i].utmi_base_addr == NULL) {
			pr_err("UTMI PHY base address is invalid\n");
			i++;
			continue;
		}

		/* get usb config address */
		cp110_utmi_data[i].usb_cfg_addr =
			(void __iomem *)fdtdec_get_addr_size_auto_noparent(
				gd->fdt_blob, node, "reg", 1, NULL, true);
		if (cp110_utmi_data[i].usb_cfg_addr == NULL) {
			pr_err("UTMI PHY base address is invalid\n");
			i++;
			continue;
		}

		/* get UTMI config address */
		cp110_utmi_data[i].utmi_cfg_addr =
			(void __iomem *)fdtdec_get_addr_size_auto_noparent(
				gd->fdt_blob, node, "reg", 2, NULL, true);
		if (cp110_utmi_data[i].utmi_cfg_addr == NULL) {
			pr_err("UTMI PHY base address is invalid\n");
			i++;
			continue;
		}

		/*
		 * get the port number (to check if the utmi connected to
		 * host/device)
		 */
		cp110_utmi_data[i].utmi_phy_port = fdtdec_get_int(
			gd->fdt_blob, node, "utmi-port", UTMI_PHY_INVALID);
		if (cp110_utmi_data[i].utmi_phy_port == UTMI_PHY_INVALID) {
			pr_err("UTMI PHY port type is invalid\n");
			i++;
			continue;
		}

		node = fdt_node_offset_by_compatible(
			gd->fdt_blob, node, "marvell,mvebu-utmi-2.6.0");
		i++;
	}

	if (i > 0)
		comphy_utmi_phy_init(i, cp110_utmi_data);

	debug_exit();
}

static void comphy_mux_cp110_init(struct chip_serdes_phy_config *ptr_chip_cfg,
				  struct comphy_map *serdes_map)
{
	void __iomem *comphy_base_addr;
	struct comphy_map comphy_map_pipe_data[MAX_LANE_OPTIONS];
	struct comphy_map comphy_map_phy_data[MAX_LANE_OPTIONS];
	u32 lane, comphy_max_count;

	comphy_max_count = ptr_chip_cfg->comphy_lanes_count;
	comphy_base_addr = ptr_chip_cfg->comphy_base_addr;

	/*
	 * Copy the SerDes map configuration for PIPE map and PHY map
	 * the comphy_mux_init modify the type of the lane if the type
	 * is not valid because we have 2 selectores run the
	 * comphy_mux_init twice and after that update the original
	 * serdes_map
	 */
	for (lane = 0; lane < comphy_max_count; lane++) {
		comphy_map_pipe_data[lane].type = serdes_map[lane].type;
		comphy_map_pipe_data[lane].speed = serdes_map[lane].speed;
		comphy_map_phy_data[lane].type = serdes_map[lane].type;
		comphy_map_phy_data[lane].speed = serdes_map[lane].speed;
	}
	ptr_chip_cfg->mux_data = cp110_comphy_phy_mux_data;
	comphy_mux_init(ptr_chip_cfg, comphy_map_phy_data,
			comphy_base_addr + COMMON_SELECTOR_PHY_OFFSET);

	ptr_chip_cfg->mux_data = cp110_comphy_pipe_mux_data;
	comphy_mux_init(ptr_chip_cfg, comphy_map_pipe_data,
			comphy_base_addr + COMMON_SELECTOR_PIPE_OFFSET);
	/* Fix the type after check the PHY and PIPE configuration */
	for (lane = 0; lane < comphy_max_count; lane++) {
		if ((comphy_map_pipe_data[lane].type == PHY_TYPE_UNCONNECTED) &&
		    (comphy_map_phy_data[lane].type == PHY_TYPE_UNCONNECTED))
			serdes_map[lane].type = PHY_TYPE_UNCONNECTED;
	}
}

int comphy_cp110_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map)
{
	struct comphy_map *ptr_comphy_map;
	void __iomem *comphy_base_addr, *hpipe_base_addr;
	u32 comphy_max_count, lane, ret = 0;
	u32 pcie_width = 0;
	u32 mode;

	debug_enter();

	comphy_max_count = ptr_chip_cfg->comphy_lanes_count;
	comphy_base_addr = ptr_chip_cfg->comphy_base_addr;
	hpipe_base_addr = ptr_chip_cfg->hpipe3_base_addr;

	/* Config Comphy mux configuration */
	comphy_mux_cp110_init(ptr_chip_cfg, serdes_map);

	/* Check if the first 4 lanes configured as By-4 */
	for (lane = 0, ptr_comphy_map = serdes_map; lane < 4;
	     lane++, ptr_comphy_map++) {
		if (ptr_comphy_map->type != PHY_TYPE_PEX0)
			break;
		pcie_width++;
	}

	for (lane = 0, ptr_comphy_map = serdes_map; lane < comphy_max_count;
	     lane++, ptr_comphy_map++) {
		debug("Initialize serdes number %d\n", lane);
		debug("Serdes type = 0x%x\n", ptr_comphy_map->type);
		if (lane == 4) {
			/*
			 * PCIe lanes above the first 4 lanes, can be only
			 * by1
			 */
			pcie_width = 1;
		}
		switch (ptr_comphy_map->type) {
		case PHY_TYPE_UNCONNECTED:
		case PHY_TYPE_IGNORE:
			continue;
			break;
		case PHY_TYPE_PEX0:
		case PHY_TYPE_PEX1:
		case PHY_TYPE_PEX2:
		case PHY_TYPE_PEX3:
			mode = COMPHY_FW_PCIE_FORMAT(pcie_width,
						     ptr_comphy_map->clk_src,
						     COMPHY_PCIE_MODE,
						     ptr_comphy_map->speed);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case PHY_TYPE_SATA0:
		case PHY_TYPE_SATA1:
		case PHY_TYPE_SATA2:
		case PHY_TYPE_SATA3:
			mode =  COMPHY_FW_MODE_FORMAT(COMPHY_SATA_MODE);
			ret = comphy_sata_power_up(lane, hpipe_base_addr,
						   comphy_base_addr,
						   ptr_chip_cfg->cp_index,
						   mode);
			break;
		case PHY_TYPE_USB3_HOST0:
		case PHY_TYPE_USB3_HOST1:
		case PHY_TYPE_USB3_DEVICE:
			ret = comphy_usb3_power_up(lane, hpipe_base_addr,
						   comphy_base_addr);
			break;
		case PHY_TYPE_SGMII0:
		case PHY_TYPE_SGMII1:
			if (ptr_comphy_map->speed == PHY_SPEED_INVALID) {
				debug("Warning: ");
				debug("SGMII PHY speed in lane %d is invalid,",
				      lane);
				debug(" set PHY speed to 1.25G\n");
				ptr_comphy_map->speed = PHY_SPEED_1_25G;
			}

			/*
			 * UINIT_ID not relevant for SGMII0 and SGMII1 - will be
			 * ignored by firmware
			 */
			mode = COMPHY_FW_FORMAT(COMPHY_SGMII_MODE,
						COMPHY_UNIT_ID0,
						ptr_comphy_map->speed);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case PHY_TYPE_SGMII2:
		case PHY_TYPE_SGMII3:
			if (ptr_comphy_map->speed == PHY_SPEED_INVALID) {
				debug("Warning: SGMII PHY speed in lane %d is invalid, set PHY speed to 1.25G\n",
				      lane);
				ptr_comphy_map->speed = PHY_SPEED_1_25G;
			}

			mode = COMPHY_FW_FORMAT(COMPHY_SGMII_MODE,
						COMPHY_UNIT_ID2,
						ptr_comphy_map->speed);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case PHY_TYPE_SFI:
			mode = COMPHY_FW_FORMAT(COMPHY_SFI_MODE,
						COMPHY_UNIT_ID0,
						ptr_comphy_map->speed);
			ret = comphy_smc(MV_SIP_COMPHY_POWER_ON,
					 ptr_chip_cfg->comphy_base_addr, lane,
					 mode);
			break;
		case PHY_TYPE_RXAUI0:
		case PHY_TYPE_RXAUI1:
			ret = comphy_rxauii_power_up(lane, hpipe_base_addr,
						     comphy_base_addr);
			break;
		default:
			debug("Unknown SerDes type, skip initialize SerDes %d\n",
			      lane);
			break;
		}
		if (ret == 0) {
			/*
			 * If interface wans't initialized, set the lane to
			 * PHY_TYPE_UNCONNECTED state.
			 */
			ptr_comphy_map->type = PHY_TYPE_UNCONNECTED;
			pr_err("PLL is not locked - Failed to initialize lane %d\n",
			      lane);
		}
	}

	debug_exit();
	return 0;
}
