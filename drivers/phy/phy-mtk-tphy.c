// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 - 2019 MediaTek Inc.
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 *	   Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <dt-bindings/phy/phy.h>

/* version V1 sub-banks offset base address */
/* banks shared by multiple phys */
#define SSUSB_SIFSLV_V1_SPLLC		0x000	/* shared by u3 phys */
#define SSUSB_SIFSLV_V1_U2FREQ		0x100	/* shared by u2 phys */
#define SSUSB_SIFSLV_V1_CHIP		0x300	/* shared by u3 phys */
/* u2 phy bank */
#define SSUSB_SIFSLV_V1_U2PHY_COM	0x000
/* u3/pcie/sata phy banks */
#define SSUSB_SIFSLV_V1_U3PHYD		0x000
#define SSUSB_SIFSLV_V1_U3PHYA		0x200

/* version V2 sub-banks offset base address */
/* u2 phy banks */
#define SSUSB_SIFSLV_V2_MISC		0x000
#define SSUSB_SIFSLV_V2_U2FREQ		0x100
#define SSUSB_SIFSLV_V2_U2PHY_COM	0x300
/* u3/pcie/sata phy banks */
#define SSUSB_SIFSLV_V2_SPLLC		0x000
#define SSUSB_SIFSLV_V2_CHIP		0x100
#define SSUSB_SIFSLV_V2_U3PHYD		0x200
#define SSUSB_SIFSLV_V2_U3PHYA		0x400

#define U3P_USBPHYACR0			0x000
#define PA0_RG_U2PLL_FORCE_ON		BIT(15)
#define PA0_RG_USB20_INTR_EN		BIT(5)

#define U3P_USBPHYACR5			0x014
#define PA5_RG_U2_HSTX_SRCAL_EN		BIT(15)
#define PA5_RG_U2_HSTX_SRCTRL		GENMASK(14, 12)
#define PA5_RG_U2_HSTX_SRCTRL_VAL(x)	((0x7 & (x)) << 12)
#define PA5_RG_U2_HS_100U_U3_EN		BIT(11)

#define U3P_USBPHYACR6			0x018
#define PA6_RG_U2_BC11_SW_EN		BIT(23)
#define PA6_RG_U2_OTG_VBUSCMP_EN	BIT(20)
#define PA6_RG_U2_SQTH			GENMASK(3, 0)
#define PA6_RG_U2_SQTH_VAL(x)		(0xf & (x))

#define U3P_U2PHYACR4			0x020
#define P2C_RG_USB20_GPIO_CTL		BIT(9)
#define P2C_USB20_GPIO_MODE		BIT(8)
#define P2C_U2_GPIO_CTR_MSK	\
		(P2C_RG_USB20_GPIO_CTL | P2C_USB20_GPIO_MODE)

#define U3P_U2PHYDTM0			0x068
#define P2C_FORCE_UART_EN		BIT(26)
#define P2C_FORCE_DATAIN		BIT(23)
#define P2C_FORCE_DM_PULLDOWN		BIT(21)
#define P2C_FORCE_DP_PULLDOWN		BIT(20)
#define P2C_FORCE_XCVRSEL		BIT(19)
#define P2C_FORCE_SUSPENDM		BIT(18)
#define P2C_FORCE_TERMSEL		BIT(17)
#define P2C_RG_DATAIN			GENMASK(13, 10)
#define P2C_RG_DATAIN_VAL(x)		((0xf & (x)) << 10)
#define P2C_RG_DMPULLDOWN		BIT(7)
#define P2C_RG_DPPULLDOWN		BIT(6)
#define P2C_RG_XCVRSEL			GENMASK(5, 4)
#define P2C_RG_XCVRSEL_VAL(x)		((0x3 & (x)) << 4)
#define P2C_RG_SUSPENDM			BIT(3)
#define P2C_RG_TERMSEL			BIT(2)
#define P2C_DTM0_PART_MASK	\
		(P2C_FORCE_DATAIN | P2C_FORCE_DM_PULLDOWN | \
		P2C_FORCE_DP_PULLDOWN | P2C_FORCE_XCVRSEL | \
		P2C_FORCE_TERMSEL | P2C_RG_DMPULLDOWN | \
		P2C_RG_DPPULLDOWN | P2C_RG_TERMSEL)

#define U3P_U2PHYDTM1			0x06C
#define P2C_RG_UART_EN			BIT(16)
#define P2C_FORCE_IDDIG			BIT(9)
#define P2C_RG_VBUSVALID		BIT(5)
#define P2C_RG_SESSEND			BIT(4)
#define P2C_RG_AVALID			BIT(2)
#define P2C_RG_IDDIG			BIT(1)

#define U3P_U3_CHIP_GPIO_CTLD		0x0c
#define P3C_REG_IP_SW_RST		BIT(31)
#define P3C_MCU_BUS_CK_GATE_EN		BIT(30)
#define P3C_FORCE_IP_SW_RST		BIT(29)

#define U3P_U3_CHIP_GPIO_CTLE		0x10
#define P3C_RG_SWRST_U3_PHYD		BIT(25)
#define P3C_RG_SWRST_U3_PHYD_FORCE_EN	BIT(24)

#define U3P_U3_PHYA_REG0		0x000
#define P3A_RG_CLKDRV_OFF		GENMASK(3, 2)
#define P3A_RG_CLKDRV_OFF_VAL(x)	((0x3 & (x)) << 2)

#define U3P_U3_PHYA_REG1		0x004
#define P3A_RG_CLKDRV_AMP		GENMASK(31, 29)
#define P3A_RG_CLKDRV_AMP_VAL(x)	((0x7 & (x)) << 29)

#define U3P_U3_PHYA_REG6		0x018
#define P3A_RG_TX_EIDLE_CM		GENMASK(31, 28)
#define P3A_RG_TX_EIDLE_CM_VAL(x)	((0xf & (x)) << 28)

#define U3P_U3_PHYA_REG9		0x024
#define P3A_RG_RX_DAC_MUX		GENMASK(5, 1)
#define P3A_RG_RX_DAC_MUX_VAL(x)	((0x1f & (x)) << 1)

#define U3P_U3_PHYA_DA_REG0		0x100
#define P3A_RG_XTAL_EXT_PE2H		GENMASK(17, 16)
#define P3A_RG_XTAL_EXT_PE2H_VAL(x)	((0x3 & (x)) << 16)
#define P3A_RG_XTAL_EXT_PE1H		GENMASK(13, 12)
#define P3A_RG_XTAL_EXT_PE1H_VAL(x)	((0x3 & (x)) << 12)
#define P3A_RG_XTAL_EXT_EN_U3		GENMASK(11, 10)
#define P3A_RG_XTAL_EXT_EN_U3_VAL(x)	((0x3 & (x)) << 10)

#define U3P_U3_PHYA_DA_REG4		0x108
#define P3A_RG_PLL_DIVEN_PE2H		GENMASK(21, 19)
#define P3A_RG_PLL_BC_PE2H		GENMASK(7, 6)
#define P3A_RG_PLL_BC_PE2H_VAL(x)	((0x3 & (x)) << 6)

#define U3P_U3_PHYA_DA_REG5		0x10c
#define P3A_RG_PLL_BR_PE2H		GENMASK(29, 28)
#define P3A_RG_PLL_BR_PE2H_VAL(x)	((0x3 & (x)) << 28)
#define P3A_RG_PLL_IC_PE2H		GENMASK(15, 12)
#define P3A_RG_PLL_IC_PE2H_VAL(x)	((0xf & (x)) << 12)

#define U3P_U3_PHYA_DA_REG6		0x110
#define P3A_RG_PLL_IR_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_IR_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG7		0x114
#define P3A_RG_PLL_BP_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_BP_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG20		0x13c
#define P3A_RG_PLL_DELTA1_PE2H		GENMASK(31, 16)
#define P3A_RG_PLL_DELTA1_PE2H_VAL(x)	((0xffff & (x)) << 16)

#define U3P_U3_PHYA_DA_REG25		0x148
#define P3A_RG_PLL_DELTA_PE2H		GENMASK(15, 0)
#define P3A_RG_PLL_DELTA_PE2H_VAL(x)	(0xffff & (x))

#define U3P_U3_PHYD_LFPS1		0x00c
#define P3D_RG_FWAKE_TH			GENMASK(21, 16)
#define P3D_RG_FWAKE_TH_VAL(x)		((0x3f & (x)) << 16)

#define U3P_U3_PHYD_CDR1		0x05c
#define P3D_RG_CDR_BIR_LTD1		GENMASK(28, 24)
#define P3D_RG_CDR_BIR_LTD1_VAL(x)	((0x1f & (x)) << 24)
#define P3D_RG_CDR_BIR_LTD0		GENMASK(12, 8)
#define P3D_RG_CDR_BIR_LTD0_VAL(x)	((0x1f & (x)) << 8)

#define U3P_U3_PHYD_RXDET1		0x128
#define P3D_RG_RXDET_STB2_SET		GENMASK(17, 9)
#define P3D_RG_RXDET_STB2_SET_VAL(x)	((0x1ff & (x)) << 9)

#define U3P_U3_PHYD_RXDET2		0x12c
#define P3D_RG_RXDET_STB2_SET_P3	GENMASK(8, 0)
#define P3D_RG_RXDET_STB2_SET_P3_VAL(x)	(0x1ff & (x))

#define U3P_SPLLC_XTALCTL3		0x018
#define XC3_RG_U3_XTAL_RX_PWD		BIT(9)
#define XC3_RG_U3_FRC_XTAL_RX_PWD	BIT(8)

/* SATA register setting */
#define PHYD_CTRL_SIGNAL_MODE4		0x1c
/* CDR Charge Pump P-path current adjustment */
#define RG_CDR_BICLTD1_GEN1_MSK		GENMASK(23, 20)
#define RG_CDR_BICLTD1_GEN1_VAL(x)	((0xf & (x)) << 20)
#define RG_CDR_BICLTD0_GEN1_MSK		GENMASK(11, 8)
#define RG_CDR_BICLTD0_GEN1_VAL(x)	((0xf & (x)) << 8)

#define PHYD_DESIGN_OPTION2		0x24
/* Symbol lock count selection */
#define RG_LOCK_CNT_SEL_MSK		GENMASK(5, 4)
#define RG_LOCK_CNT_SEL_VAL(x)		((0x3 & (x)) << 4)

#define PHYD_DESIGN_OPTION9		0x40
/* COMWAK GAP width window */
#define RG_TG_MAX_MSK			GENMASK(20, 16)
#define RG_TG_MAX_VAL(x)		((0x1f & (x)) << 16)
/* COMINIT GAP width window */
#define RG_T2_MAX_MSK			GENMASK(13, 8)
#define RG_T2_MAX_VAL(x)		((0x3f & (x)) << 8)
/* COMWAK GAP width window */
#define RG_TG_MIN_MSK			GENMASK(7, 5)
#define RG_TG_MIN_VAL(x)		((0x7 & (x)) << 5)
/* COMINIT GAP width window */
#define RG_T2_MIN_MSK			GENMASK(4, 0)
#define RG_T2_MIN_VAL(x)		(0x1f & (x))

#define ANA_RG_CTRL_SIGNAL1		0x4c
/* TX driver tail current control for 0dB de-empahsis mdoe for Gen1 speed */
#define RG_IDRV_0DB_GEN1_MSK		GENMASK(13, 8)
#define RG_IDRV_0DB_GEN1_VAL(x)		((0x3f & (x)) << 8)

#define ANA_RG_CTRL_SIGNAL4		0x58
#define RG_CDR_BICLTR_GEN1_MSK		GENMASK(23, 20)
#define RG_CDR_BICLTR_GEN1_VAL(x)	((0xf & (x)) << 20)
/* Loop filter R1 resistance adjustment for Gen1 speed */
#define RG_CDR_BR_GEN2_MSK		GENMASK(10, 8)
#define RG_CDR_BR_GEN2_VAL(x)		((0x7 & (x)) << 8)

#define ANA_RG_CTRL_SIGNAL6		0x60
/* I-path capacitance adjustment for Gen1 */
#define RG_CDR_BC_GEN1_MSK		GENMASK(28, 24)
#define RG_CDR_BC_GEN1_VAL(x)		((0x1f & (x)) << 24)
#define RG_CDR_BIRLTR_GEN1_MSK		GENMASK(4, 0)
#define RG_CDR_BIRLTR_GEN1_VAL(x)	(0x1f & (x))

#define ANA_EQ_EYE_CTRL_SIGNAL1		0x6c
/* RX Gen1 LEQ tuning step */
#define RG_EQ_DLEQ_LFI_GEN1_MSK		GENMASK(11, 8)
#define RG_EQ_DLEQ_LFI_GEN1_VAL(x)	((0xf & (x)) << 8)

#define ANA_EQ_EYE_CTRL_SIGNAL4		0xd8
#define RG_CDR_BIRLTD0_GEN1_MSK		GENMASK(20, 16)
#define RG_CDR_BIRLTD0_GEN1_VAL(x)	((0x1f & (x)) << 16)

#define ANA_EQ_EYE_CTRL_SIGNAL5		0xdc
#define RG_CDR_BIRLTD0_GEN3_MSK		GENMASK(4, 0)
#define RG_CDR_BIRLTD0_GEN3_VAL(x)	(0x1f & (x))

enum mtk_phy_version {
	MTK_TPHY_V1 = 1,
	MTK_TPHY_V2,
};

struct u2phy_banks {
	void __iomem *misc;
	void __iomem *fmreg;
	void __iomem *com;
};

struct u3phy_banks {
	void __iomem *spllc;
	void __iomem *chip;
	void __iomem *phyd; /* include u3phyd_bank2 */
	void __iomem *phya; /* include u3phya_da */
};

struct mtk_phy_instance {
	void __iomem *port_base;
	const struct device_node *np;
	union {
		struct u2phy_banks u2_banks;
		struct u3phy_banks u3_banks;
	};

	struct clk ref_clk;	/* reference clock of (digital) phy */
	struct clk da_ref_clk;	/* reference clock of analog phy */
	u32 index;
	u32 type;
};

struct mtk_tphy {
	struct udevice *dev;
	void __iomem *sif_base;
	enum mtk_phy_version version;
	struct mtk_phy_instance **phys;
	int nphys;
};

static void u2_phy_instance_init(struct mtk_tphy *tphy,
				 struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;

	/* switch to USB function, and enable usb pll */
	clrsetbits_le32(u2_banks->com + U3P_U2PHYDTM0,
			P2C_FORCE_UART_EN | P2C_FORCE_SUSPENDM,
			P2C_RG_XCVRSEL_VAL(1) | P2C_RG_DATAIN_VAL(0));

	clrbits_le32(u2_banks->com + U3P_U2PHYDTM1, P2C_RG_UART_EN);
	setbits_le32(u2_banks->com + U3P_USBPHYACR0, PA0_RG_USB20_INTR_EN);

	/* disable switch 100uA current to SSUSB */
	clrbits_le32(u2_banks->com + U3P_USBPHYACR5, PA5_RG_U2_HS_100U_U3_EN);

	clrbits_le32(u2_banks->com + U3P_U2PHYACR4, P2C_U2_GPIO_CTR_MSK);

	/* DP/DM BC1.1 path Disable */
	clrsetbits_le32(u2_banks->com + U3P_USBPHYACR6,
			PA6_RG_U2_BC11_SW_EN | PA6_RG_U2_SQTH,
			PA6_RG_U2_SQTH_VAL(2));

	/* set HS slew rate */
	clrsetbits_le32(u2_banks->com + U3P_USBPHYACR5,
			PA5_RG_U2_HSTX_SRCTRL, PA5_RG_U2_HSTX_SRCTRL_VAL(4));

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void u2_phy_instance_power_on(struct mtk_tphy *tphy,
				     struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;

	clrbits_le32(u2_banks->com + U3P_U2PHYDTM0,
		     P2C_RG_XCVRSEL | P2C_RG_DATAIN | P2C_DTM0_PART_MASK);

	/* OTG Enable */
	setbits_le32(u2_banks->com + U3P_USBPHYACR6,
		     PA6_RG_U2_OTG_VBUSCMP_EN);

	clrsetbits_le32(u2_banks->com + U3P_U2PHYDTM1,
			P2C_RG_SESSEND, P2C_RG_VBUSVALID | P2C_RG_AVALID);

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void u2_phy_instance_power_off(struct mtk_tphy *tphy,
				      struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;

	clrbits_le32(u2_banks->com + U3P_U2PHYDTM0,
		     P2C_RG_XCVRSEL | P2C_RG_DATAIN);

	/* OTG Disable */
	clrbits_le32(u2_banks->com + U3P_USBPHYACR6,
		     PA6_RG_U2_OTG_VBUSCMP_EN);

	clrsetbits_le32(u2_banks->com + U3P_U2PHYDTM1,
			P2C_RG_VBUSVALID | P2C_RG_AVALID, P2C_RG_SESSEND);

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void u3_phy_instance_init(struct mtk_tphy *tphy,
				 struct mtk_phy_instance *instance)
{
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	/* gating PCIe Analog XTAL clock */
	setbits_le32(u3_banks->spllc + U3P_SPLLC_XTALCTL3,
		     XC3_RG_U3_XTAL_RX_PWD | XC3_RG_U3_FRC_XTAL_RX_PWD);

	/* gating XSQ */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG0,
			P3A_RG_XTAL_EXT_EN_U3, P3A_RG_XTAL_EXT_EN_U3_VAL(2));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG9,
			P3A_RG_RX_DAC_MUX, P3A_RG_RX_DAC_MUX_VAL(4));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG6,
			P3A_RG_TX_EIDLE_CM, P3A_RG_TX_EIDLE_CM_VAL(0xe));

	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_CDR1,
			P3D_RG_CDR_BIR_LTD0 | P3D_RG_CDR_BIR_LTD1,
			P3D_RG_CDR_BIR_LTD0_VAL(0xc) |
			P3D_RG_CDR_BIR_LTD1_VAL(0x3));

	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_LFPS1,
			P3D_RG_FWAKE_TH, P3D_RG_FWAKE_TH_VAL(0x34));

	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET1,
			P3D_RG_RXDET_STB2_SET, P3D_RG_RXDET_STB2_SET_VAL(0x10));

	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET2,
			P3D_RG_RXDET_STB2_SET_P3,
			P3D_RG_RXDET_STB2_SET_P3_VAL(0x10));

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void pcie_phy_instance_init(struct mtk_tphy *tphy,
				   struct mtk_phy_instance *instance)
{
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	if (tphy->version != MTK_TPHY_V1)
		return;

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG0,
			P3A_RG_XTAL_EXT_PE1H | P3A_RG_XTAL_EXT_PE2H,
			P3A_RG_XTAL_EXT_PE1H_VAL(0x2) |
			P3A_RG_XTAL_EXT_PE2H_VAL(0x2));

	/* ref clk drive */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG1, P3A_RG_CLKDRV_AMP,
			P3A_RG_CLKDRV_AMP_VAL(0x4));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG0, P3A_RG_CLKDRV_OFF,
			P3A_RG_CLKDRV_OFF_VAL(0x1));

	/* SSC delta -5000ppm */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG20,
			P3A_RG_PLL_DELTA1_PE2H,
			P3A_RG_PLL_DELTA1_PE2H_VAL(0x3c));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG25,
			P3A_RG_PLL_DELTA_PE2H,
			P3A_RG_PLL_DELTA_PE2H_VAL(0x36));

	/* change pll BW 0.6M */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG5,
			P3A_RG_PLL_BR_PE2H | P3A_RG_PLL_IC_PE2H,
			P3A_RG_PLL_BR_PE2H_VAL(0x1) |
			P3A_RG_PLL_IC_PE2H_VAL(0x1));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG4,
			P3A_RG_PLL_DIVEN_PE2H | P3A_RG_PLL_BC_PE2H,
			P3A_RG_PLL_BC_PE2H_VAL(0x3));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG6,
			P3A_RG_PLL_IR_PE2H, P3A_RG_PLL_IR_PE2H_VAL(0x2));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG7,
			P3A_RG_PLL_BP_PE2H, P3A_RG_PLL_BP_PE2H_VAL(0xa));

	/* Tx Detect Rx Timing: 10us -> 5us */
	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET1,
			P3D_RG_RXDET_STB2_SET,
			P3D_RG_RXDET_STB2_SET_VAL(0x10));
	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET2,
			P3D_RG_RXDET_STB2_SET_P3,
			P3D_RG_RXDET_STB2_SET_P3_VAL(0x10));

	/* wait for PCIe subsys register to active */
	udelay(3000);
}

static void sata_phy_instance_init(struct mtk_tphy *tphy,
				   struct mtk_phy_instance *instance)
{
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	clrsetbits_le32(u3_banks->phyd + ANA_RG_CTRL_SIGNAL6,
			RG_CDR_BIRLTR_GEN1_MSK | RG_CDR_BC_GEN1_MSK,
			RG_CDR_BIRLTR_GEN1_VAL(0x6) |
			RG_CDR_BC_GEN1_VAL(0x1a));
	clrsetbits_le32(u3_banks->phyd + ANA_EQ_EYE_CTRL_SIGNAL4,
			RG_CDR_BIRLTD0_GEN1_MSK,
			RG_CDR_BIRLTD0_GEN1_VAL(0x18));
	clrsetbits_le32(u3_banks->phyd + ANA_EQ_EYE_CTRL_SIGNAL5,
			RG_CDR_BIRLTD0_GEN3_MSK,
			RG_CDR_BIRLTD0_GEN3_VAL(0x06));
	clrsetbits_le32(u3_banks->phyd + ANA_RG_CTRL_SIGNAL4,
			RG_CDR_BICLTR_GEN1_MSK | RG_CDR_BR_GEN2_MSK,
			RG_CDR_BICLTR_GEN1_VAL(0x0c) |
			RG_CDR_BR_GEN2_VAL(0x07));
	clrsetbits_le32(u3_banks->phyd + PHYD_CTRL_SIGNAL_MODE4,
			RG_CDR_BICLTD0_GEN1_MSK | RG_CDR_BICLTD1_GEN1_MSK,
			RG_CDR_BICLTD0_GEN1_VAL(0x08) |
			RG_CDR_BICLTD1_GEN1_VAL(0x02));
	clrsetbits_le32(u3_banks->phyd + PHYD_DESIGN_OPTION2,
			RG_LOCK_CNT_SEL_MSK,
			RG_LOCK_CNT_SEL_VAL(0x02));
	clrsetbits_le32(u3_banks->phyd + PHYD_DESIGN_OPTION9,
			RG_T2_MIN_MSK | RG_TG_MIN_MSK |
			RG_T2_MAX_MSK | RG_TG_MAX_MSK,
			RG_T2_MIN_VAL(0x12) | RG_TG_MIN_VAL(0x04) |
			RG_T2_MAX_VAL(0x31) | RG_TG_MAX_VAL(0x0e));
	clrsetbits_le32(u3_banks->phyd + ANA_RG_CTRL_SIGNAL1,
			RG_IDRV_0DB_GEN1_MSK,
			RG_IDRV_0DB_GEN1_VAL(0x20));
	clrsetbits_le32(u3_banks->phyd + ANA_EQ_EYE_CTRL_SIGNAL1,
			RG_EQ_DLEQ_LFI_GEN1_MSK,
			RG_EQ_DLEQ_LFI_GEN1_VAL(0x03));
}

static void pcie_phy_instance_power_on(struct mtk_tphy *tphy,
				       struct mtk_phy_instance *instance)
{
	struct u3phy_banks *bank = &instance->u3_banks;

	clrbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLD,
		     P3C_FORCE_IP_SW_RST | P3C_REG_IP_SW_RST);
	clrbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLE,
		     P3C_RG_SWRST_U3_PHYD_FORCE_EN | P3C_RG_SWRST_U3_PHYD);
}

static void pcie_phy_instance_power_off(struct mtk_tphy *tphy,
					struct mtk_phy_instance *instance)

{
	struct u3phy_banks *bank = &instance->u3_banks;

	setbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLD,
		     P3C_FORCE_IP_SW_RST | P3C_REG_IP_SW_RST);
	setbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLE,
		     P3C_RG_SWRST_U3_PHYD_FORCE_EN | P3C_RG_SWRST_U3_PHYD);
}

static void phy_v1_banks_init(struct mtk_tphy *tphy,
			      struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_banks->misc = NULL;
		u2_banks->fmreg = tphy->sif_base + SSUSB_SIFSLV_V1_U2FREQ;
		u2_banks->com = instance->port_base + SSUSB_SIFSLV_V1_U2PHY_COM;
		break;
	case PHY_TYPE_USB3:
	case PHY_TYPE_PCIE:
		u3_banks->spllc = tphy->sif_base + SSUSB_SIFSLV_V1_SPLLC;
		u3_banks->chip = tphy->sif_base + SSUSB_SIFSLV_V1_CHIP;
		u3_banks->phyd = instance->port_base + SSUSB_SIFSLV_V1_U3PHYD;
		u3_banks->phya = instance->port_base + SSUSB_SIFSLV_V1_U3PHYA;
		break;
	case PHY_TYPE_SATA:
		u3_banks->phyd = instance->port_base + SSUSB_SIFSLV_V1_U3PHYD;
		break;
	default:
		dev_err(tphy->dev, "incompatible PHY type\n");
		return;
	}
}

static void phy_v2_banks_init(struct mtk_tphy *tphy,
			      struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_banks->misc = instance->port_base + SSUSB_SIFSLV_V2_MISC;
		u2_banks->fmreg = instance->port_base + SSUSB_SIFSLV_V2_U2FREQ;
		u2_banks->com = instance->port_base + SSUSB_SIFSLV_V2_U2PHY_COM;
		break;
	case PHY_TYPE_USB3:
	case PHY_TYPE_PCIE:
		u3_banks->spllc = instance->port_base + SSUSB_SIFSLV_V2_SPLLC;
		u3_banks->chip = instance->port_base + SSUSB_SIFSLV_V2_CHIP;
		u3_banks->phyd = instance->port_base + SSUSB_SIFSLV_V2_U3PHYD;
		u3_banks->phya = instance->port_base + SSUSB_SIFSLV_V2_U3PHYA;
		break;
	default:
		dev_err(tphy->dev, "incompatible PHY type\n");
		return;
	}
}

static int mtk_phy_init(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];
	int ret;

	ret = clk_enable(&instance->ref_clk);
	if (ret < 0) {
		dev_err(tphy->dev, "failed to enable ref_clk\n");
		return ret;
	}

	ret = clk_enable(&instance->da_ref_clk);
	if (ret < 0) {
		dev_err(tphy->dev, "failed to enable da_ref_clk %d\n", ret);
		clk_disable(&instance->ref_clk);
		return ret;
	}

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_phy_instance_init(tphy, instance);
		break;
	case PHY_TYPE_USB3:
		u3_phy_instance_init(tphy, instance);
		break;
	case PHY_TYPE_PCIE:
		pcie_phy_instance_init(tphy, instance);
		break;
	case PHY_TYPE_SATA:
		sata_phy_instance_init(tphy, instance);
		break;
	default:
		dev_err(tphy->dev, "incompatible PHY type\n");
		return -EINVAL;
	}

	return 0;
}

static int mtk_phy_power_on(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	if (instance->type == PHY_TYPE_USB2)
		u2_phy_instance_power_on(tphy, instance);
	else if (instance->type == PHY_TYPE_PCIE)
		pcie_phy_instance_power_on(tphy, instance);

	return 0;
}

static int mtk_phy_power_off(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	if (instance->type == PHY_TYPE_USB2)
		u2_phy_instance_power_off(tphy, instance);
	else if (instance->type == PHY_TYPE_PCIE)
		pcie_phy_instance_power_off(tphy, instance);

	return 0;
}

static int mtk_phy_exit(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	clk_disable(&instance->da_ref_clk);
	clk_disable(&instance->ref_clk);

	return 0;
}

static int mtk_phy_xlate(struct phy *phy,
			 struct ofnode_phandle_args *args)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = NULL;
	const struct device_node *phy_np = ofnode_to_np(args->node);
	u32 index;

	if (!phy_np) {
		dev_err(phy->dev, "null pointer phy node\n");
		return -EINVAL;
	}

	if (args->args_count < 1) {
		dev_err(phy->dev, "invalid number of cells in 'phy' property\n");
		return -EINVAL;
	}

	for (index = 0; index < tphy->nphys; index++)
		if (phy_np == tphy->phys[index]->np) {
			instance = tphy->phys[index];
			break;
		}

	if (!instance) {
		dev_err(phy->dev, "failed to find appropriate phy\n");
		return -EINVAL;
	}

	phy->id = index;
	instance->type = args->args[1];
	if (!(instance->type == PHY_TYPE_USB2 ||
	      instance->type == PHY_TYPE_USB3 ||
	      instance->type == PHY_TYPE_SATA ||
	      instance->type == PHY_TYPE_PCIE)) {
		dev_err(phy->dev, "unsupported device type\n");
		return -EINVAL;
	}

	if (tphy->version == MTK_TPHY_V1) {
		phy_v1_banks_init(tphy, instance);
	} else if (tphy->version == MTK_TPHY_V2) {
		phy_v2_banks_init(tphy, instance);
	} else {
		dev_err(phy->dev, "phy version is not supported\n");
		return -EINVAL;
	}

	return 0;
}

static const struct phy_ops mtk_tphy_ops = {
	.init		= mtk_phy_init,
	.exit		= mtk_phy_exit,
	.power_on	= mtk_phy_power_on,
	.power_off	= mtk_phy_power_off,
	.of_xlate	= mtk_phy_xlate,
};

static int mtk_tphy_probe(struct udevice *dev)
{
	struct mtk_tphy *tphy = dev_get_priv(dev);
	ofnode subnode;
	int index = 0;

	tphy->nphys = dev_get_child_count(dev);

	tphy->phys = devm_kcalloc(dev, tphy->nphys, sizeof(*tphy->phys),
				  GFP_KERNEL);
	if (!tphy->phys)
		return -ENOMEM;

	tphy->dev = dev;
	tphy->version = dev_get_driver_data(dev);

	/* v1 has shared banks for usb/pcie mode, */
	/* but not for sata mode */
	if (tphy->version == MTK_TPHY_V1) {
		tphy->sif_base = dev_read_addr_ptr(dev);
	}

	dev_for_each_subnode(subnode, dev) {
		struct mtk_phy_instance *instance;
		fdt_addr_t addr;
		int err;

		instance = devm_kzalloc(dev, sizeof(*instance), GFP_KERNEL);
		if (!instance)
			return -ENOMEM;

		addr = ofnode_get_addr(subnode);
		if (addr == FDT_ADDR_T_NONE)
			return -ENOMEM;

		instance->port_base = map_sysmem(addr, 0);
		instance->index = index;
		instance->np = ofnode_to_np(subnode);
		tphy->phys[index] = instance;
		index++;

		err = clk_get_by_name_nodev_optional(subnode, "ref",
						     &instance->ref_clk);
		if (err)
			return err;

		err = clk_get_by_name_nodev_optional(subnode, "da_ref",
						     &instance->da_ref_clk);
		if (err)
			return err;
	}

	return 0;
}

static const struct udevice_id mtk_tphy_id_table[] = {
	{ .compatible = "mediatek,generic-tphy-v1", .data = MTK_TPHY_V1, },
	{ .compatible = "mediatek,generic-tphy-v2", .data = MTK_TPHY_V2, },
	{ }
};

U_BOOT_DRIVER(mtk_tphy) = {
	.name		= "mtk-tphy",
	.id		= UCLASS_PHY,
	.of_match	= mtk_tphy_id_table,
	.ops		= &mtk_tphy_ops,
	.probe		= mtk_tphy_probe,
	.priv_auto	= sizeof(struct mtk_tphy),
};
