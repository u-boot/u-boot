/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _COMPHY_H_
#define _COMPHY_H_

#include <dt-bindings/comphy/comphy_data.h>
#include <fdtdec.h>

#if defined(DEBUG)
#define debug_enter()	printf("----> Enter %s\n", __func__);
#define debug_exit()	printf("<---- Exit  %s\n", __func__);
#else
#define debug_enter()
#define debug_exit()
#endif

/* COMPHY registers */
#define COMMON_PHY_CFG1_REG			0x0
#define COMMON_PHY_CFG1_PWR_UP_OFFSET		1
#define COMMON_PHY_CFG1_PWR_UP_MASK		\
	(0x1 << COMMON_PHY_CFG1_PWR_UP_OFFSET)
#define COMMON_PHY_CFG1_PIPE_SELECT_OFFSET	2
#define COMMON_PHY_CFG1_PIPE_SELECT_MASK	\
	(0x1 << COMMON_PHY_CFG1_PIPE_SELECT_OFFSET)
#define COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET	13
#define COMMON_PHY_CFG1_PWR_ON_RESET_MASK	\
	(0x1 << COMMON_PHY_CFG1_PWR_ON_RESET_OFFSET)
#define COMMON_PHY_CFG1_CORE_RSTN_OFFSET	14
#define COMMON_PHY_CFG1_CORE_RSTN_MASK		\
	(0x1 << COMMON_PHY_CFG1_CORE_RSTN_OFFSET)
#define COMMON_PHY_PHY_MODE_OFFSET		15
#define COMMON_PHY_PHY_MODE_MASK		\
	(0x1 << COMMON_PHY_PHY_MODE_OFFSET)

#define COMMON_PHY_CFG6_REG			0x14
#define COMMON_PHY_CFG6_IF_40_SEL_OFFSET	18
#define COMMON_PHY_CFG6_IF_40_SEL_MASK		\
	(0x1 << COMMON_PHY_CFG6_IF_40_SEL_OFFSET)

#define COMMON_SELECTOR_PHY_OFFSET		0x140
#define COMMON_SELECTOR_PIPE_OFFSET		0x144

#define COMMON_PHY_SD_CTRL1			0x148
#define COMMON_PHY_SD_CTRL1_COMPHY_0_4_PORT_OFFSET	0
#define COMMON_PHY_SD_CTRL1_COMPHY_0_4_PORT_MASK	0xFFFF
#define COMMON_PHY_SD_CTRL1_PCIE_X4_EN_OFFSET	24
#define COMMON_PHY_SD_CTRL1_PCIE_X4_EN_MASK	\
	(0x1 << COMMON_PHY_SD_CTRL1_PCIE_X4_EN_OFFSET)
#define COMMON_PHY_SD_CTRL1_PCIE_X2_EN_OFFSET	25
#define COMMON_PHY_SD_CTRL1_PCIE_X2_EN_MASK	\
	(0x1 << COMMON_PHY_SD_CTRL1_PCIE_X2_EN_OFFSET)
#define COMMON_PHY_SD_CTRL1_RXAUI1_OFFSET	26
#define COMMON_PHY_SD_CTRL1_RXAUI1_MASK		\
	(0x1 << COMMON_PHY_SD_CTRL1_RXAUI1_OFFSET)
#define COMMON_PHY_SD_CTRL1_RXAUI0_OFFSET	27
#define COMMON_PHY_SD_CTRL1_RXAUI0_MASK		\
	(0x1 << COMMON_PHY_SD_CTRL1_RXAUI0_OFFSET)

/* ToDo: Get this address via DT */
#define MVEBU_CP0_REGS_BASE			0xF2000000UL

#define DFX_DEV_GEN_CTRL12			(MVEBU_CP0_REGS_BASE + 0x400280)
#define DFX_DEV_GEN_PCIE_CLK_SRC_OFFSET		7
#define DFX_DEV_GEN_PCIE_CLK_SRC_MASK		\
	(0x3 << DFX_DEV_GEN_PCIE_CLK_SRC_OFFSET)

#define MAX_LANE_OPTIONS			10
#define MAX_UTMI_PHY_COUNT			3

struct comphy_mux_options {
	u32 type;
	u32 mux_value;
};

struct comphy_mux_data {
	u32 max_lane_values;
	struct comphy_mux_options mux_values[MAX_LANE_OPTIONS];
};

struct comphy_map {
	u32 type;
	u32 speed;
	u32 invert;
	bool clk_src;
};

struct chip_serdes_phy_config {
	struct comphy_mux_data *mux_data;
	int (*ptr_comphy_chip_init)(struct chip_serdes_phy_config *,
				    struct comphy_map *);
	void __iomem *comphy_base_addr;
	void __iomem *hpipe3_base_addr;
	u32 comphy_lanes_count;
	u32 comphy_mux_bitcount;
	u32 comphy_index;
};

/* Register helper functions */
void reg_set(void __iomem *addr, u32 data, u32 mask);
void reg_set_silent(void __iomem *addr, u32 data, u32 mask);
void reg_set16(void __iomem *addr, u16 data, u16 mask);
void reg_set_silent16(void __iomem *addr, u16 data, u16 mask);

/* SoC specific init functions */
#ifdef CONFIG_ARMADA_3700
int comphy_a3700_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map);
#else
static inline int comphy_a3700_init(struct chip_serdes_phy_config *ptr_chip_cfg,
				    struct comphy_map *serdes_map)
{
	/*
	 * This function should never be called in this configuration, so
	 * lets return an error here.
	 */
	return -1;
}
#endif

#ifdef CONFIG_ARMADA_8K
int comphy_cp110_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map);
#else
static inline int comphy_cp110_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map)
{
	/*
	 * This function should never be called in this configuration, so
	 * lets return an error here.
	 */
	return -1;
}
#endif

void comphy_dedicated_phys_init(void);

/* MUX function */
void comphy_mux_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		     struct comphy_map *comphy_map_data,
		     void __iomem *selector_base);

void comphy_pcie_config_set(u32 comphy_max_count,
			    struct comphy_map *serdes_map);
void comphy_pcie_config_detect(u32 comphy_max_count,
			       struct comphy_map *serdes_map);
void comphy_pcie_unit_general_config(u32 pex_index);

#endif /* _COMPHY_H_ */

