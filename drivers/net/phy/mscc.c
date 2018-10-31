// SPDX-License-Identifier: MIT
/*
 * Microsemi PHY drivers
 *
 *
 * Copyright (c) 2016 Microsemi Corporation
 *
 * Author: John Haechten
 *
 */

#include <miiphy.h>
#include <bitfield.h>
#include <time.h>
#include <linux/delay.h>

/* Microsemi PHY ID's */
#define PHY_ID_VSC8530                  0x00070560
#define PHY_ID_VSC8531                  0x00070570
#define PHY_ID_VSC8540                  0x00070760
#define PHY_ID_VSC8541                  0x00070770
#define PHY_ID_VSC8584                  0x000707c0

/* Microsemi VSC85xx PHY Register Pages */
#define MSCC_EXT_PAGE_ACCESS            31     /* Page Access Register */
#define MSCC_PHY_PAGE_STD		0x0000 /* Standard registers */
#define MSCC_PHY_PAGE_EXT1		0x0001 /* Extended registers - page 1 */
#define MSCC_PHY_PAGE_EXT2		0x0002 /* Extended registers - page 2 */
#define MSCC_PHY_PAGE_EXT3		0x0003 /* Extended registers - page 3 */
#define MSCC_PHY_PAGE_EXT4		0x0004 /* Extended registers - page 4 */
#define MSCC_PHY_PAGE_GPIO		0x0010 /* GPIO registers */
#define MSCC_PHY_PAGE_TEST		0x2A30 /* TEST Page registers */
#define MSCC_PHY_PAGE_TR		0x52B5 /* Token Ring Page registers */

/* Std Page Register 18 */
#define MSCC_PHY_BYPASS_CONTROL           18
#define PARALLEL_DET_IGNORE_ADVERTISED    BIT(3)

/* Std Page Register 22 */
#define MSCC_PHY_EXT_CNTL_STATUS          22
#define SMI_BROADCAST_WR_EN              BIT(0)

/* Std Page Register 28 - PHY AUX Control/Status */
#define MIIM_AUX_CNTRL_STAT_REG		28
#define MIIM_AUX_CNTRL_STAT_ACTIPHY_TO	(0x0004)
#define MIIM_AUX_CNTRL_STAT_F_DUPLEX	(0x0020)
#define MIIM_AUX_CNTRL_STAT_SPEED_MASK	(0x0018)
#define MIIM_AUX_CNTRL_STAT_SPEED_POS	(3)
#define MIIM_AUX_CNTRL_STAT_SPEED_10M	(0x0)
#define MIIM_AUX_CNTRL_STAT_SPEED_100M	(0x1)
#define MIIM_AUX_CNTRL_STAT_SPEED_1000M	(0x2)

/* Std Page Register 23 - Extended PHY CTRL_1 */
#define MSCC_PHY_EXT_PHY_CNTL_1_REG	23
#define MAC_IF_SELECTION_MASK		(0x1800)
#define MAC_IF_SELECTION_GMII		(0)
#define MAC_IF_SELECTION_RMII		(1)
#define MAC_IF_SELECTION_RGMII		(2)
#define MAC_IF_SELECTION_POS		(11)
#define MAC_IF_SELECTION_WIDTH		(2)
#define VSC8584_MAC_IF_SELECTION_MASK     BIT(12)
#define VSC8584_MAC_IF_SELECTION_SGMII    0
#define VSC8584_MAC_IF_SELECTION_1000BASEX 1
#define VSC8584_MAC_IF_SELECTION_POS      12
#define MEDIA_OP_MODE_MASK		  GENMASK(10, 8)
#define MEDIA_OP_MODE_COPPER		  0
#define MEDIA_OP_MODE_SERDES		  1
#define MEDIA_OP_MODE_1000BASEX		  2
#define MEDIA_OP_MODE_100BASEFX		  3
#define MEDIA_OP_MODE_AMS_COPPER_SERDES	  5
#define MEDIA_OP_MODE_AMS_COPPER_1000BASEX	6
#define MEDIA_OP_MODE_AMS_COPPER_100BASEFX	7
#define MEDIA_OP_MODE_POS		  8

/* Extended Page 1 Register 20E1 */
#define MSCC_PHY_ACTIPHY_CNTL		  20
#define PHY_ADDR_REVERSED		  BIT(9)

/* Extended Page 1 Register 23E1 */

#define MSCC_PHY_EXT_PHY_CNTL_4           23
#define PHY_CNTL_4_ADDR_POS		  11

/* Extended Page 1 Register 25E1 */
#define MSCC_PHY_VERIPHY_CNTL_2		25

/* Extended Page 1 Register 26E1 */
#define MSCC_PHY_VERIPHY_CNTL_3		26

/* Extended Page 2 Register 16E2 */
#define MSCC_PHY_CU_PMD_TX_CNTL         16

/* Extended Page 2 Register 20E2 */
#define MSCC_PHY_RGMII_CNTL_REG		20
#define VSC_FAST_LINK_FAIL2_ENA_MASK	(0x8000)
#define RX_CLK_OUT_MASK			(0x0800)
#define RX_CLK_OUT_POS			(11)
#define RX_CLK_OUT_WIDTH		(1)
#define RX_CLK_OUT_NORMAL		(0)
#define RX_CLK_OUT_DISABLE		(1)
#define RGMII_RX_CLK_DELAY_POS		(4)
#define RGMII_RX_CLK_DELAY_WIDTH	(3)
#define RGMII_RX_CLK_DELAY_MASK		(0x0070)
#define RGMII_TX_CLK_DELAY_POS		(0)
#define RGMII_TX_CLK_DELAY_WIDTH	(3)
#define RGMII_TX_CLK_DELAY_MASK		(0x0007)

/* Extended Page 2 Register 27E2 */
#define MSCC_PHY_WOL_MAC_CONTROL	27
#define EDGE_RATE_CNTL_POS		(5)
#define EDGE_RATE_CNTL_WIDTH		(3)
#define EDGE_RATE_CNTL_MASK		(0x00E0)
#define RMII_CLK_OUT_ENABLE_POS		(4)
#define RMII_CLK_OUT_ENABLE_WIDTH	(1)
#define RMII_CLK_OUT_ENABLE_MASK	(0x10)

/* Extended Page 3 Register 22E3 */
#define MSCC_PHY_SERDES_TX_CRC_ERR_CNT	22

/* Extended page GPIO register 00G */
#define MSCC_DW8051_CNTL_STATUS		0
#define MICRO_NSOFT_RESET		BIT(15)
#define RUN_FROM_INT_ROM		BIT(14)
#define AUTOINC_ADDR			BIT(13)
#define PATCH_RAM_CLK			BIT(12)
#define MICRO_PATCH_EN			BIT(7)
#define DW8051_CLK_EN			BIT(4)
#define MICRO_CLK_EN			BIT(3)
#define MICRO_CLK_DIVIDE(x)		((x) >> 1)

/* Extended page GPIO register 09G */
#define MSCC_TRAP_ROM_ADDR(x)		((x) * 2 + 1)

/* Extended page GPIO register 10G */
#define MSCC_PATCH_RAM_ADDR(x)		(((x) + 1) * 2)

/* Extended page GPIO register 11G */
#define MSCC_INT_MEM_ADDR		11

/* Extended page GPIO register 12G */
#define MSCC_INT_MEM_CNTL		12
#define READ_SFR			(BIT(14) | BIT(13))
#define READ_PRAM			BIT(14)
#define READ_ROM			BIT(13)
#define READ_RAM			(0x00 << 13)
#define INT_MEM_WRITE_EN		BIT(12)
#define EN_PATCH_RAM_TRAP_ADDR(x)	BIT((x) + 7)
#define INT_MEM_DATA_M			GENMASK(7, 0)
#define INT_MEM_DATA(x)			(INT_MEM_DATA_M & (x))

/* Extended page GPIO register 18G */
#define MSCC_PHY_PROC_CMD		  18
#define PROC_CMD_NCOMPLETED		  BIT(15)
#define PROC_CMD_FAILED			  BIT(14)
#define PROC_CMD_SGMII_PORT(x)		  ((x) << 8)
#define PROC_CMD_FIBER_PORT(x)		  BIT(8 + (x) % 4)
#define PROC_CMD_QSGMII_PORT		  (BIT(11) | BIT(10))
#define PROC_CMD_RST_CONF_PORT		  BIT(7)
#define PROC_CMD_RECONF_PORT		  (0 << 7)
#define PROC_CMD_READ_MOD_WRITE_PORT	  BIT(6)
#define PROC_CMD_WRITE			  BIT(6)
#define PROC_CMD_READ			  (0 << 6)
#define PROC_CMD_FIBER_DISABLE		  BIT(5)
#define PROC_CMD_FIBER_100BASE_FX	  BIT(4)
#define PROC_CMD_FIBER_1000BASE_X	  (0 << 4)
#define PROC_CMD_SGMII_MAC		  (BIT(5) | BIT(4))
#define PROC_CMD_QSGMII_MAC		  BIT(5)
#define PROC_CMD_NO_MAC_CONF		  (0x00 << 4)
#define PROC_CMD_NOP			  GENMASK(3, 0)
#define PROC_CMD_CRC16			  BIT(3)
#define PROC_CMD_FIBER_MEDIA_CONF	  BIT(0)
#define PROC_CMD_MCB_ACCESS_MAC_CONF	  (0x0000 << 0)
#define PROC_CMD_NCOMPLETED_TIMEOUT_MS    500

/* Extended page GPIO register 19G */
#define MSCC_PHY_MAC_CFG_FASTLINK	  19
#define MAC_CFG_MASK			  GENMASK(15, 14)
#define MAC_CFG_SGMII			  (0x00 << 14)
#define MAC_CFG_QSGMII			  BIT(14)

/* Test Registers */
#define MSCC_PHY_TEST_PAGE_5		5

#define MSCC_PHY_TEST_PAGE_8		8
#define TR_CLK_DISABLE			BIT(15)

/* Token Ring Page 0x52B5 Registers */
#define MSCC_PHY_REG_TR_ADDR_16		16
#define MSCC_PHY_REG_TR_DATA_17		17
#define MSCC_PHY_REG_TR_DATA_18		18

/* Token Ring - Read Value in */
#define MSCC_PHY_TR_16_READ		(0xA000)
/* Token Ring - Write Value out */
#define MSCC_PHY_TR_16_WRITE		(0x8000)

/* Token Ring Registers */
#define MSCC_PHY_TR_LINKDETCTRL_POS	(3)
#define MSCC_PHY_TR_LINKDETCTRL_WIDTH	(2)
#define MSCC_PHY_TR_LINKDETCTRL_VAL	(3)
#define MSCC_PHY_TR_LINKDETCTRL_MASK	(0x0018)
#define MSCC_PHY_TR_LINKDETCTRL_ADDR	(0x07F8)

#define MSCC_PHY_TR_VGATHRESH100_POS	(0)
#define MSCC_PHY_TR_VGATHRESH100_WIDTH	(7)
#define MSCC_PHY_TR_VGATHRESH100_VAL	(0x0018)
#define MSCC_PHY_TR_VGATHRESH100_MASK	(0x007f)
#define MSCC_PHY_TR_VGATHRESH100_ADDR	(0x0FA4)

#define MSCC_PHY_TR_VGAGAIN10_U_POS	(0)
#define MSCC_PHY_TR_VGAGAIN10_U_WIDTH	(1)
#define MSCC_PHY_TR_VGAGAIN10_U_MASK	(0x0001)
#define MSCC_PHY_TR_VGAGAIN10_U_VAL	(0)

#define MSCC_PHY_TR_VGAGAIN10_L_POS	(12)
#define MSCC_PHY_TR_VGAGAIN10_L_WIDTH	(4)
#define MSCC_PHY_TR_VGAGAIN10_L_MASK	(0xf000)
#define MSCC_PHY_TR_VGAGAIN10_L_VAL	(0x0001)
#define MSCC_PHY_TR_VGAGAIN10_ADDR	(0x0F92)

/* General Timeout Values */
#define MSCC_PHY_RESET_TIMEOUT		(100)
#define MSCC_PHY_MICRO_TIMEOUT		(500)

#define VSC8584_REVB		0x0001
#define MSCC_DEV_REV_MASK	GENMASK(3, 0)

#define MSCC_VSC8584_REVB_INT8051_FW_START_ADDR	0xe800
#define MSCC_VSC8584_REVB_INT8051_FW_CRC	0xfb48

/* RGMII/GMII Clock Delay (Skew) Options */ enum vsc_phy_rgmii_skew {
	VSC_PHY_RGMII_DELAY_200_PS,
	VSC_PHY_RGMII_DELAY_800_PS,
	VSC_PHY_RGMII_DELAY_1100_PS,
	VSC_PHY_RGMII_DELAY_1700_PS,
	VSC_PHY_RGMII_DELAY_2000_PS,
	VSC_PHY_RGMII_DELAY_2300_PS,
	VSC_PHY_RGMII_DELAY_2600_PS,
	VSC_PHY_RGMII_DELAY_3400_PS,
};

/* MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */ enum
vsc_phy_clk_slew {
	VSC_PHY_CLK_SLEW_RATE_0,
	VSC_PHY_CLK_SLEW_RATE_1,
	VSC_PHY_CLK_SLEW_RATE_2,
	VSC_PHY_CLK_SLEW_RATE_3,
	VSC_PHY_CLK_SLEW_RATE_4,
	VSC_PHY_CLK_SLEW_RATE_5,
	VSC_PHY_CLK_SLEW_RATE_6,
	VSC_PHY_CLK_SLEW_RATE_7,
};

struct vsc85xx_priv {
	int (*config_pre)(struct phy_device *phydev);
};

static void vsc8584_csr_write(struct mii_dev *bus, int phy0, u16 addr, u32 val)
{
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18,
		   val >> 16);
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17,
		   val & GENMASK(15, 0));
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		   MSCC_PHY_TR_16_WRITE | addr);
}

static int vsc8584_cmd(struct mii_dev *bus, int phy, u16 val)
{
	unsigned long deadline;
	u16 reg_val;

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_GPIO);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_PHY_PROC_CMD,
		   PROC_CMD_NCOMPLETED | val);

	deadline = timer_get_us() + PROC_CMD_NCOMPLETED_TIMEOUT_MS * 1000;
	do {
		reg_val = bus->read(bus, phy, MDIO_DEVAD_NONE,
				    MSCC_PHY_PROC_CMD);
	} while (timer_get_us() <= deadline &&
		 (reg_val & PROC_CMD_NCOMPLETED) &&
		 !(reg_val & PROC_CMD_FAILED));

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	if (reg_val & PROC_CMD_FAILED)
		return -EIO;
	if (reg_val & PROC_CMD_NCOMPLETED)
		return -ETIMEDOUT;

	return 0;
}

static int vsc8584_micro_deassert_reset(struct mii_dev *bus, int phy,
					bool patch_en)
{
	u32 enable, release;

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_GPIO);

	enable = RUN_FROM_INT_ROM | MICRO_CLK_EN | DW8051_CLK_EN;
	release = MICRO_NSOFT_RESET | RUN_FROM_INT_ROM | DW8051_CLK_EN |
		MICRO_CLK_EN;

	if (patch_en) {
		enable |= MICRO_PATCH_EN;
		release |= MICRO_PATCH_EN;

		/* Clear all patches */
		bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL,
			   READ_RAM);
	}

	/*
	 * Enable 8051 Micro clock; CLEAR/SET patch present; disable PRAM clock
	 * override and addr. auto-incr; operate at 125 MHz
	 */
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_DW8051_CNTL_STATUS, enable);
	/* Release 8051 Micro SW reset */
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_DW8051_CNTL_STATUS, release);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	return 0;
}

static int vsc8584_micro_assert_reset(struct mii_dev *bus, int phy)
{
	int ret;
	u16 reg;

	ret = vsc8584_cmd(bus, phy, PROC_CMD_NOP);
	if (ret)
		return ret;

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_GPIO);

	reg = bus->read(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL);
	reg &= ~EN_PATCH_RAM_TRAP_ADDR(4);
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL, reg);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_TRAP_ROM_ADDR(4), 0x005b);
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_PATCH_RAM_ADDR(4), 0x005b);

	reg = bus->read(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL);
	reg |= EN_PATCH_RAM_TRAP_ADDR(4);
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL, reg);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_PHY_PROC_CMD, PROC_CMD_NOP);

	reg = bus->read(bus, phy, MDIO_DEVAD_NONE, MSCC_DW8051_CNTL_STATUS);
	reg &= ~MICRO_NSOFT_RESET;
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_DW8051_CNTL_STATUS, reg);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_PHY_PROC_CMD,
		   PROC_CMD_MCB_ACCESS_MAC_CONF | PROC_CMD_SGMII_PORT(0) |
		   PROC_CMD_NO_MAC_CONF | PROC_CMD_READ);

	reg = bus->read(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL);
	reg &= ~EN_PATCH_RAM_TRAP_ADDR(4);
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL, reg);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	return 0;
}

static const u8 fw_patch_vsc8584[] = {
	0xe8, 0x59, 0x02, 0xe8, 0x12, 0x02, 0xe8, 0x42, 0x02, 0xe8, 0x5a, 0x02,
	0xe8, 0x5b, 0x02, 0xe8, 0x5c, 0xe5, 0x69, 0x54, 0x0f, 0x24, 0xf7, 0x60,
	0x27, 0x24, 0xfc, 0x60, 0x23, 0x24, 0x08, 0x70, 0x14, 0xe5, 0x69, 0xae,
	0x68, 0x78, 0x04, 0xce, 0xa2, 0xe7, 0x13, 0xce, 0x13, 0xd8, 0xf8, 0x7e,
	0x00, 0x54, 0x0f, 0x80, 0x00, 0x7b, 0x01, 0x7a, 0x00, 0x7d, 0xee, 0x7f,
	0x92, 0x12, 0x50, 0xee, 0x22, 0xe4, 0xf5, 0x10, 0x85, 0x10, 0xfb, 0x7d,
	0x1c, 0xe4, 0xff, 0x12, 0x59, 0xea, 0x05, 0x10, 0xe5, 0x10, 0xc3, 0x94,
	0x04, 0x40, 0xed, 0x22, 0x22, 0x22, 0x22, 0x22,
};

static int vsc8584_get_fw_crc(struct mii_dev *bus, int phy, u16 start,
			      u16 *crc, const u8 *fw_patch, int fw_size)
{
	int ret;

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_EXT1);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_PHY_VERIPHY_CNTL_2, start);
	/* Add one byte to size for the one added by the patch_fw function */
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_PHY_VERIPHY_CNTL_3,
		   fw_size + 1);

	ret = vsc8584_cmd(bus, phy, PROC_CMD_CRC16);
	if (ret)
		goto out;

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_EXT1);

	*crc = bus->read(bus, phy, MDIO_DEVAD_NONE, MSCC_PHY_VERIPHY_CNTL_2);

out:
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	return ret;
}

static int vsc8584_patch_fw(struct mii_dev *bus, int phy, const u8 *fw_patch,
			    int fw_size)
{
	int i, ret;

	ret = vsc8584_micro_assert_reset(bus, phy);
	if (ret) {
		pr_err("%s: failed to assert reset of micro\n", __func__);
		return ret;
	}

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_GPIO);

	/*
	 * Hold 8051 Micro in SW Reset, Enable auto incr address and patch clock
	 * Disable the 8051 Micro clock
	 */
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_DW8051_CNTL_STATUS,
		   RUN_FROM_INT_ROM | AUTOINC_ADDR | PATCH_RAM_CLK |
		   MICRO_CLK_EN | MICRO_CLK_DIVIDE(2));
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL, READ_PRAM |
		   INT_MEM_WRITE_EN | INT_MEM_DATA(2));
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_ADDR, 0x0000);

	for (i = 0; i < fw_size; i++)
		bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL,
			   READ_PRAM | INT_MEM_WRITE_EN | fw_patch[i]);

	/* Clear internal memory access */
	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_INT_MEM_CNTL, READ_RAM);

	bus->write(bus, phy, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	return 0;
}

static int vsc8584_config_pre_init(struct phy_device *phydev)
{
	struct mii_dev *bus = phydev->bus;
	u16 reg, crc, phy0, addr;
	int ret;

	if ((phydev->phy_id & MSCC_DEV_REV_MASK) != VSC8584_REVB) {
		pr_warn("VSC8584 revA not officially supported, skipping firmware patching. Use at your own risk.\n");
		return 0;
	}

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_EXT1);
	addr = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_4);
	addr >>= PHY_CNTL_4_ADDR_POS;

	reg = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_ACTIPHY_CNTL);
	if (reg & PHY_ADDR_REVERSED)
		phy0 = phydev->addr + addr;
	else
		phy0 = phydev->addr - addr;

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	/* all writes below are broadcasted to all PHYs in the same package */
	reg = bus->read(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_EXT_CNTL_STATUS);
	reg |= SMI_BROADCAST_WR_EN;
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_EXT_CNTL_STATUS, reg);

	/*
	 * The below register writes are tweaking analog and electrical
	 * configuration that were determined through characterization by PHY
	 * engineers. These don't mean anything more than "these are the best
	 * values".
	 */
	reg = bus->read(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_BYPASS_CONTROL);
	reg |= PARALLEL_DET_IGNORE_ADVERTISED;
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_BYPASS_CONTROL, reg);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_EXT3);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_SERDES_TX_CRC_ERR_CNT,
		   0x2000);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_TEST);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_TEST_PAGE_5, 0x1f20);

	reg = bus->read(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_TEST_PAGE_8);
	reg |= TR_CLK_DISABLE;
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_TEST_PAGE_8, reg);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_TR);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16, 0xafa4);

	reg = bus->read(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18);
	reg &= ~0x007f;
	reg |= 0x0019;
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18, reg);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16, 0x8fa4);

	vsc8584_csr_write(bus, phy0, 0x07fa, 0x0050100f);
	vsc8584_csr_write(bus, phy0, 0x1688, 0x00049f81);
	vsc8584_csr_write(bus, phy0, 0x0f90, 0x00688980);
	vsc8584_csr_write(bus, phy0, 0x03a4, 0x0000d8f0);
	vsc8584_csr_write(bus, phy0, 0x0fc0, 0x00000400);
	vsc8584_csr_write(bus, phy0, 0x0f82, 0x0012b002);
	vsc8584_csr_write(bus, phy0, 0x1686, 0x00000004);
	vsc8584_csr_write(bus, phy0, 0x168c, 0x00d2c46f);
	vsc8584_csr_write(bus, phy0, 0x17a2, 0x00000620);
	vsc8584_csr_write(bus, phy0, 0x16a0, 0x00eeffdd);
	vsc8584_csr_write(bus, phy0, 0x16a6, 0x00071448);
	vsc8584_csr_write(bus, phy0, 0x16a4, 0x0013132f);
	vsc8584_csr_write(bus, phy0, 0x16a8, 0x00000000);
	vsc8584_csr_write(bus, phy0, 0x0ffc, 0x00c0a028);
	vsc8584_csr_write(bus, phy0, 0x0fe8, 0x0091b06c);
	vsc8584_csr_write(bus, phy0, 0x0fea, 0x00041600);
	vsc8584_csr_write(bus, phy0, 0x0f80, 0x00fffaff);
	vsc8584_csr_write(bus, phy0, 0x0fec, 0x00901809);
	vsc8584_csr_write(bus, phy0, 0x0ffe, 0x00b01007);
	vsc8584_csr_write(bus, phy0, 0x16b0, 0x00eeff00);
	vsc8584_csr_write(bus, phy0, 0x16b2, 0x00007000);
	vsc8584_csr_write(bus, phy0, 0x16b4, 0x00000814);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_EXT2);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_CU_PMD_TX_CNTL, 0x028e);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_TR);

	vsc8584_csr_write(bus, phy0, 0x0486, 0x0008a518);
	vsc8584_csr_write(bus, phy0, 0x0488, 0x006dc696);
	vsc8584_csr_write(bus, phy0, 0x048a, 0x00000912);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_TEST);

	reg = bus->read(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_TEST_PAGE_8);
	reg &= ~TR_CLK_DISABLE;
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_TEST_PAGE_8, reg);

	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	/* end of write broadcasting */
	reg = bus->read(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_EXT_CNTL_STATUS);
	reg &= ~SMI_BROADCAST_WR_EN;
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_PHY_EXT_CNTL_STATUS, reg);

	ret = vsc8584_get_fw_crc(bus, phy0,
				 MSCC_VSC8584_REVB_INT8051_FW_START_ADDR, &crc,
				 fw_patch_vsc8584,
				 ARRAY_SIZE(fw_patch_vsc8584));
	if (ret)
		goto out;

	if (crc != MSCC_VSC8584_REVB_INT8051_FW_CRC) {
		debug("FW CRC is not the expected one, patching FW...\n");
		if (vsc8584_patch_fw(bus, phy0, fw_patch_vsc8584,
				     ARRAY_SIZE(fw_patch_vsc8584)))
			pr_warn("failed to patch FW, expect non-optimal device\n");
	}

	vsc8584_micro_deassert_reset(bus, phy0, false);

	ret = vsc8584_get_fw_crc(bus, phy0,
				 MSCC_VSC8584_REVB_INT8051_FW_START_ADDR, &crc,
				 fw_patch_vsc8584,
				 ARRAY_SIZE(fw_patch_vsc8584));
	if (ret)
		goto out;

	if (crc != MSCC_VSC8584_REVB_INT8051_FW_CRC)
		pr_warn("FW CRC after patching is not the expected one, expect non-optimal device\n");

	ret = vsc8584_micro_assert_reset(bus, phy0);
	if (ret)
		goto out;

	vsc8584_micro_deassert_reset(bus, phy0, true);

out:
	bus->write(bus, phy0, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		   MSCC_PHY_PAGE_STD);

	return ret;
}

static int mscc_vsc8531_vsc8541_init_scripts(struct phy_device *phydev)
{
	u16	reg_val;

	/* Set to Access Token Ring Registers */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_TR);

	/* Update LinkDetectCtrl default to optimized values */
	/* Determined during Silicon Validation Testing */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_LINKDETCTRL_ADDR | MSCC_PHY_TR_16_READ));
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_LINKDETCTRL_POS,
				   MSCC_PHY_TR_LINKDETCTRL_WIDTH,
				   MSCC_PHY_TR_LINKDETCTRL_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_LINKDETCTRL_ADDR | MSCC_PHY_TR_16_WRITE));

	/* Update VgaThresh100 defaults to optimized values */
	/* Determined during Silicon Validation Testing */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGATHRESH100_ADDR | MSCC_PHY_TR_16_READ));

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_VGATHRESH100_POS,
				   MSCC_PHY_TR_VGATHRESH100_WIDTH,
				   MSCC_PHY_TR_VGATHRESH100_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGATHRESH100_ADDR | MSCC_PHY_TR_16_WRITE));

	/* Update VgaGain10 defaults to optimized values */
	/* Determined during Silicon Validation Testing */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGAGAIN10_ADDR | MSCC_PHY_TR_16_READ));

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_VGAGAIN10_U_POS,
				   MSCC_PHY_TR_VGAGAIN10_U_WIDTH,
				   MSCC_PHY_TR_VGAGAIN10_U_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18, reg_val);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_VGAGAIN10_L_POS,
				   MSCC_PHY_TR_VGAGAIN10_L_WIDTH,
				   MSCC_PHY_TR_VGAGAIN10_L_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGAGAIN10_ADDR | MSCC_PHY_TR_16_WRITE));

	/* Set back to Access Standard Page Registers */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return 0;
}

static int mscc_parse_status(struct phy_device *phydev)
{
	u16 speed;
	u16 mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_AUX_CNTRL_STAT_REG);

	if (mii_reg & MIIM_AUX_CNTRL_STAT_F_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_AUX_CNTRL_STAT_SPEED_MASK;
	speed = speed >> MIIM_AUX_CNTRL_STAT_SPEED_POS;

	switch (speed) {
	case MIIM_AUX_CNTRL_STAT_SPEED_1000M:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_AUX_CNTRL_STAT_SPEED_100M:
		phydev->speed = SPEED_100;
		break;
	case MIIM_AUX_CNTRL_STAT_SPEED_10M:
		phydev->speed = SPEED_10;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int mscc_startup(struct phy_device *phydev)
{
	int retval;

	retval = genphy_update_link(phydev);

	if (retval)
		return retval;

	return mscc_parse_status(phydev);
}

static int mscc_phy_soft_reset(struct phy_device *phydev)
{
	int     retval = 0;
	u16     timeout = MSCC_PHY_RESET_TIMEOUT;
	u16     reg_val = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, (reg_val | BMCR_RESET));

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

	while ((reg_val & BMCR_RESET) && (timeout > 0)) {
		reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
		timeout--;
		udelay(1000);   /* 1 ms */
	}

	if (timeout == 0) {
		printf("MSCC PHY Soft_Reset Error: mac i/f = 0x%x\n",
		       phydev->interface);
		retval = -ETIME;
	}

	return retval;
}

static int vsc8531_vsc8541_mac_config(struct phy_device *phydev)
{
	u16	reg_val = 0;
	u16	mac_if = 0;
	u16	rx_clk_out = 0;

	/* For VSC8530/31 the only MAC modes are RMII/RGMII. */
	/* For VSC8540/41 the only MAC modes are (G)MII and RMII/RGMII. */
	/* Setup MAC Configuration */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
		/* Set Reg23.12:11=0 */
		mac_if = MAC_IF_SELECTION_GMII;
		/* Set Reg20E2.11=1 */
		rx_clk_out = RX_CLK_OUT_DISABLE;
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set Reg23.12:11=1 */
		mac_if = MAC_IF_SELECTION_RMII;
		/* Set Reg20E2.11=0 */
		rx_clk_out = RX_CLK_OUT_NORMAL;
		break;

	case PHY_INTERFACE_MODE_RGMII:
		/* Set Reg23.12:11=2 */
		mac_if = MAC_IF_SELECTION_RGMII;
		/* Set Reg20E2.11=0 */
		rx_clk_out = RX_CLK_OUT_NORMAL;
		break;

	default:
		printf("MSCC PHY - INVALID MAC i/f Config: mac i/f = 0x%x\n",
		       phydev->interface);
		return -EINVAL;
	}

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE,
			   MSCC_PHY_EXT_PHY_CNTL_1_REG);
	/* Set MAC i/f bits Reg23.12:11 */
	reg_val = bitfield_replace(reg_val, MAC_IF_SELECTION_POS,
				   MAC_IF_SELECTION_WIDTH, mac_if);
	/* Update Reg23.12:11 */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_PHY_EXT_PHY_CNTL_1_REG, reg_val);
	/* Setup ExtPg_2 Register Access */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_EXT2);
	/* Read Reg20E2 */
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE,
			   MSCC_PHY_RGMII_CNTL_REG);
	reg_val = bitfield_replace(reg_val, RX_CLK_OUT_POS,
				   RX_CLK_OUT_WIDTH, rx_clk_out);
	/* Update Reg20E2.11 */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_PHY_RGMII_CNTL_REG, reg_val);
	/* Before leaving - Change back to Std Page Register Access */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return 0;
}

static int vsc8531_config(struct phy_device *phydev)
{
	int  retval = -EINVAL;
	u16  reg_val;
	u16  rmii_clk_out;
	enum vsc_phy_rgmii_skew  rx_clk_skew = VSC_PHY_RGMII_DELAY_1700_PS;
	enum vsc_phy_rgmii_skew  tx_clk_skew = VSC_PHY_RGMII_DELAY_800_PS;
	enum vsc_phy_clk_slew    edge_rate = VSC_PHY_CLK_SLEW_RATE_4;

	/* For VSC8530/31 and VSC8540/41 the init scripts are the same */
	mscc_vsc8531_vsc8541_init_scripts(phydev);

	/* For VSC8530/31 the only MAC modes are RMII/RGMII. */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RMII:
	case PHY_INTERFACE_MODE_RGMII:
		retval = vsc8531_vsc8541_mac_config(phydev);
		if (retval != 0)
			return retval;

		retval = mscc_phy_soft_reset(phydev);
		if (retval != 0)
			return retval;
		break;
	default:
		printf("PHY 8530/31 MAC i/f Config Error: mac i/f = 0x%x\n",
		       phydev->interface);
		return -EINVAL;
	}
	/* Default RMII Clk Output to 0=OFF/1=ON  */
	rmii_clk_out = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_EXT2);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG);

	/* Reg20E2 - Update RGMII RX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_RX_CLK_DELAY_POS,
				   RGMII_RX_CLK_DELAY_WIDTH, rx_clk_skew);
	/* Reg20E2 - Update RGMII TX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_TX_CLK_DELAY_POS,
				   RGMII_TX_CLK_DELAY_WIDTH, tx_clk_skew);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG, reg_val);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL);
	/* Reg27E2 - Update Clk Slew Rate. */
	reg_val = bitfield_replace(reg_val, EDGE_RATE_CNTL_POS,
				   EDGE_RATE_CNTL_WIDTH, edge_rate);
	/* Reg27E2 - Update RMII Clk Out. */
	reg_val = bitfield_replace(reg_val, RMII_CLK_OUT_ENABLE_POS,
				   RMII_CLK_OUT_ENABLE_WIDTH, rmii_clk_out);
	/* Update Reg27E2 */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return genphy_config_aneg(phydev);
}

static int vsc8541_config(struct phy_device *phydev)
{
	int  retval = -EINVAL;
	u16  reg_val;
	u16  rmii_clk_out;
	enum vsc_phy_rgmii_skew  rx_clk_skew = VSC_PHY_RGMII_DELAY_1700_PS;
	enum vsc_phy_rgmii_skew  tx_clk_skew = VSC_PHY_RGMII_DELAY_800_PS;
	enum vsc_phy_clk_slew    edge_rate = VSC_PHY_CLK_SLEW_RATE_4;

	/* For VSC8530/31 and VSC8540/41 the init scripts are the same */
	mscc_vsc8531_vsc8541_init_scripts(phydev);

	/* For VSC8540/41 the only MAC modes are (G)MII and RMII/RGMII. */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_RMII:
	case PHY_INTERFACE_MODE_RGMII:
		retval = vsc8531_vsc8541_mac_config(phydev);
		if (retval != 0)
			return retval;

		retval = mscc_phy_soft_reset(phydev);
		if (retval != 0)
			return retval;
		break;
	default:
		printf("PHY 8541 MAC i/f config Error: mac i/f = 0x%x\n",
		       phydev->interface);
		return -EINVAL;
	}
	/* Default RMII Clk Output to 0=OFF/1=ON  */
	rmii_clk_out = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_EXT2);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG);
	/* Reg20E2 - Update RGMII RX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_RX_CLK_DELAY_POS,
				   RGMII_RX_CLK_DELAY_WIDTH, rx_clk_skew);
	/* Reg20E2 - Update RGMII TX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_TX_CLK_DELAY_POS,
				   RGMII_TX_CLK_DELAY_WIDTH, tx_clk_skew);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG, reg_val);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL);
	/* Reg27E2 - Update Clk Slew Rate. */
	reg_val = bitfield_replace(reg_val, EDGE_RATE_CNTL_POS,
				   EDGE_RATE_CNTL_WIDTH, edge_rate);
	/* Reg27E2 - Update RMII Clk Out. */
	reg_val = bitfield_replace(reg_val, RMII_CLK_OUT_ENABLE_POS,
				   RMII_CLK_OUT_ENABLE_WIDTH, rmii_clk_out);
	/* Update Reg27E2 */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return genphy_config_aneg(phydev);
}

static int vsc8584_config_init(struct phy_device *phydev)
{
	struct vsc85xx_priv *priv = phydev->priv;
	int ret;
	u16 addr;
	u16 reg_val;
	u16 val;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_EXT1);
	addr = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_4);
	addr >>= PHY_CNTL_4_ADDR_POS;

	ret = priv->config_pre(phydev);
	if (ret)
		return ret;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_GPIO);

	if (phydev->interface == PHY_INTERFACE_MODE_QSGMII)
		val = MAC_CFG_QSGMII;
	else
		val = MAC_CFG_SGMII;

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_MAC_CFG_FASTLINK);
	reg_val &= ~MAC_CFG_MASK;
	reg_val |= val;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_MAC_CFG_FASTLINK,
			reg_val);
	if (ret)
		return ret;

	reg_val = PROC_CMD_MCB_ACCESS_MAC_CONF | PROC_CMD_RST_CONF_PORT |
		PROC_CMD_READ_MOD_WRITE_PORT;
	if (phydev->interface == PHY_INTERFACE_MODE_QSGMII)
		reg_val |= PROC_CMD_QSGMII_MAC;
	else
		reg_val |= PROC_CMD_SGMII_MAC;

	ret = vsc8584_cmd(phydev->bus, phydev->addr, reg_val);
	if (ret)
		return ret;

	mdelay(10);

	/* Disable SerDes for 100Base-FX */
	ret = vsc8584_cmd(phydev->bus, phydev->addr, PROC_CMD_FIBER_MEDIA_CONF |
			  PROC_CMD_FIBER_PORT(addr) | PROC_CMD_FIBER_DISABLE |
			  PROC_CMD_READ_MOD_WRITE_PORT |
			  PROC_CMD_RST_CONF_PORT | PROC_CMD_FIBER_100BASE_FX);
	if (ret)
		return ret;

	/* Disable SerDes for 1000Base-X */
	ret = vsc8584_cmd(phydev->bus, phydev->addr, PROC_CMD_FIBER_MEDIA_CONF |
			  PROC_CMD_FIBER_PORT(addr) | PROC_CMD_FIBER_DISABLE |
			  PROC_CMD_READ_MOD_WRITE_PORT |
			  PROC_CMD_RST_CONF_PORT | PROC_CMD_FIBER_1000BASE_X);
	if (ret)
		return ret;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE,
			   MSCC_PHY_EXT_PHY_CNTL_1_REG);
	reg_val &= ~(MEDIA_OP_MODE_MASK | VSC8584_MAC_IF_SELECTION_MASK);
	reg_val |= MEDIA_OP_MODE_COPPER |
		(VSC8584_MAC_IF_SELECTION_SGMII <<
		 VSC8584_MAC_IF_SELECTION_POS);
	ret = phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_1_REG,
			reg_val);

	ret = mscc_phy_soft_reset(phydev);
	if (ret != 0)
		return ret;

	return genphy_config(phydev);
}

static struct vsc85xx_priv vsc8584_priv = {
	.config_pre = vsc8584_config_pre_init,
};

static int vsc8584_config(struct phy_device *phydev)
{
	phydev->priv = &vsc8584_priv;

	return vsc8584_config_init(phydev);
}

static struct phy_driver VSC8530_driver = {
	.name = "Microsemi VSC8530",
	.uid = PHY_ID_VSC8530,
	.mask = 0x000ffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &vsc8531_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8531_driver = {
	.name = "Microsemi VSC8531",
	.uid = PHY_ID_VSC8531,
	.mask = 0x000ffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8531_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8540_driver = {
	.name = "Microsemi VSC8540",
	.uid = PHY_ID_VSC8540,
	.mask = 0x000ffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &vsc8541_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8541_driver = {
	.name = "Microsemi VSC8541",
	.uid = PHY_ID_VSC8541,
	.mask = 0x000ffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8541_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8584_driver = {
	.name = "Microsemi VSC8584",
	.uid = PHY_ID_VSC8584,
	.mask = 0x000ffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8584_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

int phy_mscc_init(void)
{
	phy_register(&VSC8530_driver);
	phy_register(&VSC8531_driver);
	phy_register(&VSC8540_driver);
	phy_register(&VSC8541_driver);
	phy_register(&VSC8584_driver);

	return 0;
}
