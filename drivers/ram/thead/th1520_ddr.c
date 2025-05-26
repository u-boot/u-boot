// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2017-2024 Alibaba Group Holding Limited
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */

#include <binman.h>
#include <binman_sym.h>
#include <dm.h>
#include <init.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <ram.h>

DECLARE_GLOBAL_DATA_PTR;

#pragma pack(push, 1)

struct th1520_ddr_fw {
	u64 magic;
	u8 type, ranknum, bitwidth, freq;
	u8 reserved[8];

	u32 cfgnum;
	union th1520_ddr_cfg {
		u32 opaddr;

		struct th1520_ddr_phy {
			u32 opaddr;
			u16 data;
		} phy;

		struct th1520_ddr_range {
			u32 opaddr;
			u32 num;
			u16 data[];
		} range;
	} cfgs[];
};

#pragma pack(pop)

/* Firmware constants */
#define TH1520_DDR_MAGIC	0x4452444445415448

#define TH1520_DDR_TYPE_LPDDR4	0
#define TH1520_DDR_TYPE_LPDDR4X	1

#define TH1520_DDR_FREQ_2133	0
#define TH1520_DDR_FREQ_3200	1
#define TH1520_DDR_FREQ_3733	2
#define TH1520_DDR_FREQ_4266	3

#define TH1520_DDR_CFG_OP	GENMASK(31, 24)
#define TH1520_DDR_CFG_ADDR	GENMASK(23, 0)

#define TH1520_DDR_CFG_PHY0	0
#define TH1520_DDR_CFG_PHY1	1
#define TH1520_DDR_CFG_PHY	2
#define TH1520_DDR_CFG_RANGE	3
#define TH1520_DDR_CFG_WAITFW0	4
#define TH1520_DDR_CFG_WAITFW1	5

/* Driver constants */
#define TH1520_SYS_PLL_TIMEOUT_US	30
#define TH1520_CTRL_INIT_TIMEOUT_US	1000000
#define TH1520_PHY_MSG_TIMEOUT_US	1000000

/* System configuration registers */
#define TH1520_SYS_DDR_CFG0			0x00
#define  TH1520_SYS_DDR_CFG0_APB_RSTN		BIT(4)
#define  TH1520_SYS_DDR_CFG0_CTRL_RSTN		BIT(5)
#define  TH1520_SYS_DDR_CFG0_PHY_PWROK_RSTN	BIT(6)
#define  TH1520_SYS_DDR_CFG0_PHY_CORE_RSTN	BIT(7)
#define  TH1520_SYS_DDR_CFG0_APB_PORT_RSTN(n)	BIT(n + 4 + 4)
#define TH1520_SYS_DDR_CFG1			0x04
#define TH1520_SYS_PLL_CFG0			0x08
#define  TH1520_SYS_PLL_CFG0_POSTDIV2		GENMASK(26, 24)
#define  TH1520_SYS_PLL_CFG0_POSTDIV1		GENMASK(22, 20)
#define  TH1520_SYS_PLL_CFG0_FBDIV		GENMASK(19, 8)
#define  TH1520_SYS_PLL_CFG0_REFDIV		GENMASK(5, 0)
#define TH1520_SYS_PLL_CFG1			0x0c
#define  TH1520_SYS_PLL_CFG1_RST		BIT(30)
#define  TH1520_SYS_PLL_CFG1_FOUTPOSTDIVPD	BIT(27)
#define  TH1520_SYS_PLL_CFG1_FOUT4PHASEPD	BIT(25)
#define  Th1520_SYS_PLL_CFG1_DACPD		BIT(24)
#define TH1520_SYS_PLL_CFG2		0x10
#define TH1520_SYS_PLL_CFG3		0x14
#define TH1520_SYS_PLL_STS		0x18
#define  TH1520_SYS_PLL_STS_EN		BIT(16)
#define  TH1520_SYS_PLL_STS_LOCKED	BIT(0)

/* DDR Controller Registers */
#define TH1520_CTRL_MSTR			0x0000
#define TH1520_CTRL_STAT			0x0004
#define TH1520_CTRL_MRCTRL0			0x0010
#define TH1520_CTRL_MRCTRL1			0x0014
#define TH1520_CTRL_MRSTAT			0x0018
#define TH1520_CTRL_DERATEEN			0x0020
#define TH1520_CTRL_DERATEINT			0x0024
#define TH1520_CTRL_DERATECTL			0x002c
#define TH1520_CTRL_PWRCTL			0x0030
#define TH1520_CTRL_PWRTMG			0x0034
#define TH1520_CTRL_HWLPCTL			0x0038
#define TH1520_CTRL_RFSHCTL0			0x0050
#define TH1520_CTRL_RFSHCTL1			0x0054
#define TH1520_CTRL_RFSHCTL3			0x0060
#define TH1520_CTRL_RFSHTMG			0x0064
#define TH1520_CTRL_RFSHTMG1			0x0068
#define TH1520_CTRL_CRCPARCTL0			0x00c0
#define TH1520_CTRL_CRCPARSTAT			0x00cc
#define TH1520_CTRL_INIT0			0x00d0
#define TH1520_CTRL_INIT1			0x00d4
#define TH1520_CTRL_INIT2			0x00d8
#define TH1520_CTRL_INIT3			0x00dc
#define TH1520_CTRL_INIT4			0x00e0
#define TH1520_CTRL_INIT5			0x00e4
#define TH1520_CTRL_INIT6			0x00e8
#define TH1520_CTRL_INIT7			0x00ec
#define TH1520_CTRL_DIMMCTL			0x00f0
#define TH1520_CTRL_RANKCTL			0x00f4
#define TH1520_CTRL_RANKCTL1			0x00f8
#define TH1520_CTRL_DRAMTMG0			0x0100
#define TH1520_CTRL_DRAMTMG1			0x0104
#define TH1520_CTRL_DRAMTMG2			0x0108
#define TH1520_CTRL_DRAMTMG3			0x010c
#define TH1520_CTRL_DRAMTMG4			0x0110
#define TH1520_CTRL_DRAMTMG5			0x0114
#define TH1520_CTRL_DRAMTMG6			0x0118
#define TH1520_CTRL_DRAMTMG7			0x011c
#define TH1520_CTRL_DRAMTMG8			0x0120
#define TH1520_CTRL_DRAMTMG12			0x0130
#define TH1520_CTRL_DRAMTMG13			0x0134
#define TH1520_CTRL_DRAMTMG14			0x0138
#define TH1520_CTRL_DRAMTMG17			0x0144
#define TH1520_CTRL_ZQCTL0			0x0180
#define TH1520_CTRL_ZQCTL1			0x0184
#define TH1520_CTRL_ZQCTL2			0x0188
#define TH1520_CTRL_ZQSTAT			0x018c
#define TH1520_CTRL_DFITMG0			0x0190
#define TH1520_CTRL_DFITMG1			0x0194
#define TH1520_CTRL_DFILPCFG0			0x0198
#define TH1520_CTRL_DFIUPD0			0x01a0
#define TH1520_CTRL_DFIUPD1			0x01a4
#define TH1520_CTRL_DFIUPD2			0x01a8
#define TH1520_CTRL_DFIMISC			0x01b0
#define TH1520_CTRL_DFITMG2			0x01b4
#define TH1520_CTRL_DFISTAT			0x01bc
#define TH1520_CTRL_DBICTL			0x01c0
#define TH1520_CTRL_DFIPHYMSTR			0x01c4
#define TH1520_CTRL_ADDRMAP0			0x0200
#define TH1520_CTRL_ADDRMAP1			0x0204
#define TH1520_CTRL_ADDRMAP2			0x0208
#define TH1520_CTRL_ADDRMAP3			0x020c
#define TH1520_CTRL_ADDRMAP4			0x0210
#define TH1520_CTRL_ADDRMAP5			0x0214
#define TH1520_CTRL_ADDRMAP6			0x0218
#define TH1520_CTRL_ADDRMAP7			0x021c
#define TH1520_CTRL_ADDRMAP8			0x0220
#define TH1520_CTRL_ADDRMAP9			0x0224
#define TH1520_CTRL_ADDRMAP10			0x0228
#define TH1520_CTRL_ADDRMAP11			0x022c
#define TH1520_CTRL_ODTCFG			0x0240
#define TH1520_CTRL_ODTMAP			0x0244
#define TH1520_CTRL_SCHED			0x0250
#define TH1520_CTRL_SCHED1			0x0254
#define TH1520_CTRL_PERFHPR1			0x025c
#define TH1520_CTRL_PERFLPR1			0x0264
#define TH1520_CTRL_PERFWR1			0x026c
#define TH1520_CTRL_SCHED3			0x0270
#define TH1520_CTRL_SCHED4			0x0274
#define TH1520_CTRL_DBG0			0x0300
#define TH1520_CTRL_DBG1			0x0304
#define TH1520_CTRL_DBGCAM			0x0308
#define TH1520_CTRL_DBGCMD			0x030c
#define TH1520_CTRL_DBGSTAT			0x0310
#define TH1520_CTRL_SWCTL			0x0320
#define TH1520_CTRL_SWSTAT			0x0324
#define TH1520_CTRL_SWCTLSTATIC			0x0328
#define TH1520_CTRL_POISONCFG			0x036c
#define TH1520_CTRL_POISONSTAT			0x0370
#define TH1520_CTRL_DERATESTAT			0x03f0
#define TH1520_CTRL_PSTAT			0x03fc
#define TH1520_CTRL_PCCFG			0x0400
#define TH1520_CTRL_PCFGR_0			0x0404
#define TH1520_CTRL_PCFGW_0			0x0408
#define TH1520_CTRL_PCTRL_0			0x0490
#define TH1520_CTRL_PCFGQOS0_0			0x0494
#define TH1520_CTRL_PCFGQOS1_0			0x0498
#define TH1520_CTRL_PCFGWQOS0_0			0x049c
#define TH1520_CTRL_PCFGWQOS1_0			0x04a0
#define TH1520_CTRL_PCFGR_1			0x04b4
#define TH1520_CTRL_PCFGW_1			0x04b8
#define TH1520_CTRL_PCTRL_1			0x0540
#define TH1520_CTRL_PCFGQOS0_1			0x0544
#define TH1520_CTRL_PCFGQOS1_1			0x0548
#define TH1520_CTRL_PCFGWQOS0_1			0x054c
#define TH1520_CTRL_PCFGWQOS1_1			0x0550
#define TH1520_CTRL_PCFGR_2			0x0564
#define TH1520_CTRL_PCFGW_2			0x0568
#define TH1520_CTRL_PCTRL_2			0x05f0
#define TH1520_CTRL_PCFGQOS0_2			0x05f4
#define TH1520_CTRL_PCFGQOS1_2			0x05f8
#define TH1520_CTRL_PCFGWQOS0_2			0x05fc
#define TH1520_CTRL_PCFGWQOS1_2			0x0600
#define TH1520_CTRL_PCFGR_3			0x0614
#define TH1520_CTRL_PCFGW_3			0x0618
#define TH1520_CTRL_PCTRL_3			0x06a0
#define TH1520_CTRL_PCFGQOS0_3			0x06a4
#define TH1520_CTRL_PCFGQOS1_3			0x06a8
#define TH1520_CTRL_PCFGWQOS0_3			0x06ac
#define TH1520_CTRL_PCFGWQOS1_3			0x06b0
#define TH1520_CTRL_PCFGR_4			0x06c4
#define TH1520_CTRL_PCFGW_4			0x06c8
#define TH1520_CTRL_PCTRL_4			0x0750
#define TH1520_CTRL_PCFGQOS0_4			0x0754
#define TH1520_CTRL_PCFGQOS1_4			0x0758
#define TH1520_CTRL_PCFGWQOS0_4			0x075c
#define TH1520_CTRL_PCFGWQOS1_4			0x0760
#define TH1520_CTRL_UMCTL2_VER_NUMBER		0x0ff0
#define TH1520_CTRL_UMCTL2_VER_TYPE		0x0ff4
#define TH1520_CTRL_DCH1_STAT			0x1b04
#define TH1520_CTRL_DCH1_MRCTRL0		0x1b10
#define TH1520_CTRL_DCH1_MRCTRL1		0x1b14
#define TH1520_CTRL_DCH1_MRSTAT			0x1b18
#define TH1520_CTRL_DCH1_DERATECTL		0x1b2c
#define TH1520_CTRL_DCH1_PWRCTL			0x1b30
#define TH1520_CTRL_DCH1_HWLPCTL		0x1b38
#define TH1520_CTRL_DCH1_CRCPARCTL0		0x1bc0
#define TH1520_CTRL_DCH1_ZQCTL2			0x1c88
#define TH1520_CTRL_DCH1_DFISTAT		0x1cbc
#define TH1520_CTRL_DCH1_ODTMAP			0x1d44
#define TH1520_CTRL_DCH1_DBG1			0x1e04
#define TH1520_CTRL_DCH1_DBGCMD			0x1e0c
#define TH1520_CTRL_DCH1_DBGCAM			0x1e08

/* PHY configuration registers */
#define TH1520_DDR_PHY_REG(regid)	((regid) * 2)

/* UctShadowRegs */
#define TH1520_PHY_MSG_STATUS		TH1520_DDR_PHY_REG(0xd0004)
#define  TH1520_PHY_MSG_STATUS_EMPTY	BIT(0)
/* DctWriteProt */
#define TH1520_PHY_MSG_ACK		TH1520_DDR_PHY_REG(0xd0031)
#define  TH1520_PHY_MSG_ACK_EN		BIT(0)
/* UctWriteOnlyShadow */
#define TH1520_PHY_MSG_ID		TH1520_DDR_PHY_REG(0xd0032)
#define  TH1520_PHY_MSG_ID_COMPLETION	0x7
#define  TH1520_PHY_MSG_ID_ERROR	0xff
/* UctDatWriteOnlyShadow */
#define TH1520_PHY_MSG_DATA		TH1520_DDR_PHY_REG(0xd0034)

struct th1520_ddr_priv {
	void __iomem *phy0;
	void __iomem *phy1;
	void __iomem *ctrl;
	void __iomem *sys;
};

binman_sym_declare(ulong, ddr_fw, image_pos);

static int th1520_ddr_pll_config(void __iomem *sysreg, unsigned int frequency)
{
	u32 tmp;
	int ret;

	tmp = TH1520_SYS_PLL_CFG1_RST			|
	      TH1520_SYS_PLL_CFG1_FOUTPOSTDIVPD		|
	      TH1520_SYS_PLL_CFG1_FOUT4PHASEPD		|
	      Th1520_SYS_PLL_CFG1_DACPD;
	writel(tmp, sysreg + TH1520_SYS_PLL_CFG1);

	switch (frequency) {
	case TH1520_DDR_FREQ_3733:
		writel(FIELD_PREP(TH1520_SYS_PLL_CFG0_REFDIV, 1)	|
		       FIELD_PREP(TH1520_SYS_PLL_CFG0_FBDIV, 77)	|
		       FIELD_PREP(TH1520_SYS_PLL_CFG0_POSTDIV1, 2)	|
		       FIELD_PREP(TH1520_SYS_PLL_CFG0_POSTDIV2, 1),
		       sysreg + TH1520_SYS_PLL_CFG0);
		break;
	default:
		return -EINVAL;
	}

	udelay(2);
	tmp &= ~TH1520_SYS_PLL_CFG1_RST;
	writel(tmp, sysreg + TH1520_SYS_PLL_CFG1);

	ret = readl_poll_timeout(sysreg + TH1520_SYS_PLL_STS, tmp,
				 tmp & TH1520_SYS_PLL_STS_LOCKED,
				 TH1520_SYS_PLL_TIMEOUT_US);

	writel(TH1520_SYS_PLL_STS_EN, sysreg + TH1520_SYS_PLL_STS);

	return ret;
}

static int th1520_ddr_ctrl_init(void __iomem *ctrlreg, struct th1520_ddr_fw *fw)
{
	int ret;
	u32 tmp;

	writel(0x00000001, ctrlreg + TH1520_CTRL_DBG1);
	writel(0x00000001, ctrlreg + TH1520_CTRL_PWRCTL);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_STAT, tmp,
				 tmp == 0x00000000,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	if (fw->ranknum == 2)
		writel(0x03080020, ctrlreg + TH1520_CTRL_MSTR);
	else
		return -EINVAL;

	writel(0x00003030, ctrlreg + TH1520_CTRL_MRCTRL0);
	writel(0x0002d90f, ctrlreg + TH1520_CTRL_MRCTRL1);

	switch (fw->freq) {
	case TH1520_DDR_FREQ_3733:
		writel(0x000013f3, ctrlreg + TH1520_CTRL_DERATEEN);
		writel(0x40000000, ctrlreg + TH1520_CTRL_DERATEINT);
		writel(0x00000001, ctrlreg + TH1520_CTRL_DERATECTL);
		writel(0x00000020, ctrlreg + TH1520_CTRL_PWRCTL);
		writel(0x0040ae04, ctrlreg + TH1520_CTRL_PWRTMG);
		writel(0x00430000, ctrlreg + TH1520_CTRL_HWLPCTL);
		writel(0x00210004, ctrlreg + TH1520_CTRL_RFSHCTL0);
		writel(0x000d0021, ctrlreg + TH1520_CTRL_RFSHCTL1);
		writel(0x00000001, ctrlreg + TH1520_CTRL_RFSHCTL3);
		writel(0x81c00084, ctrlreg + TH1520_CTRL_RFSHTMG);
		writel(0x00540000, ctrlreg + TH1520_CTRL_RFSHTMG1);
		writel(0x00000000, ctrlreg + TH1520_CTRL_CRCPARCTL0);
		writel(0xc0020002, ctrlreg + TH1520_CTRL_INIT0);
		writel(0x00010002, ctrlreg + TH1520_CTRL_INIT1);
		writel(0x00001f00, ctrlreg + TH1520_CTRL_INIT2);
		writel(0x00640036, ctrlreg + TH1520_CTRL_INIT3);
		writel(0x00f20008, ctrlreg + TH1520_CTRL_INIT4);
		writel(0x0004000b, ctrlreg + TH1520_CTRL_INIT5);
		writel(0x00440012, ctrlreg + TH1520_CTRL_INIT6);
		writel(0x0004001a, ctrlreg + TH1520_CTRL_INIT7);
		writel(0x00000000, ctrlreg + TH1520_CTRL_DIMMCTL);
		writel(0x0000ab9f, ctrlreg + TH1520_CTRL_RANKCTL);
		writel(0x00000017, ctrlreg + TH1520_CTRL_RANKCTL1);
		writel(0x1f263f28, ctrlreg + TH1520_CTRL_DRAMTMG0);
		writel(0x00080839, ctrlreg + TH1520_CTRL_DRAMTMG1);
		writel(0x08121d17, ctrlreg + TH1520_CTRL_DRAMTMG2);
		writel(0x00d0e000, ctrlreg + TH1520_CTRL_DRAMTMG3);
		writel(0x11040a12, ctrlreg + TH1520_CTRL_DRAMTMG4);
		writel(0x02050e0e, ctrlreg + TH1520_CTRL_DRAMTMG5);
		writel(0x01010008, ctrlreg + TH1520_CTRL_DRAMTMG6);
		writel(0x00000502, ctrlreg + TH1520_CTRL_DRAMTMG7);
		writel(0x00000101, ctrlreg + TH1520_CTRL_DRAMTMG8);
		writel(0x00020000, ctrlreg + TH1520_CTRL_DRAMTMG12);
		writel(0x0d100002, ctrlreg + TH1520_CTRL_DRAMTMG13);
		writel(0x0000010c, ctrlreg + TH1520_CTRL_DRAMTMG14);
		writel(0x03a50021, ctrlreg + TH1520_CTRL_ZQCTL0);
		writel(0x02f00800, ctrlreg + TH1520_CTRL_ZQCTL1);
		writel(0x00000000, ctrlreg + TH1520_CTRL_ZQCTL2);
		writel(0x059f820c, ctrlreg + TH1520_CTRL_DFITMG0);
		writel(0x000c0303, ctrlreg + TH1520_CTRL_DFITMG1);
		writel(0x0351a101, ctrlreg + TH1520_CTRL_DFILPCFG0);
		writel(0x00000011, ctrlreg + TH1520_CTRL_DFIMISC);
		writel(0x00001f0c, ctrlreg + TH1520_CTRL_DFITMG2);
		writel(0x00000007, ctrlreg + TH1520_CTRL_DBICTL);
		writel(0x14000001, ctrlreg + TH1520_CTRL_DFIPHYMSTR);
		writel(0x06090b40, ctrlreg + TH1520_CTRL_ODTCFG);
		break;
	default:
		return -EINVAL;
	}

	writel(0x00400018, ctrlreg + TH1520_CTRL_DFIUPD0);
	writel(0x00280032, ctrlreg + TH1520_CTRL_DFIUPD1);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DFIUPD2);
	writel(0x00000000, ctrlreg + TH1520_CTRL_ODTMAP);
	writel(0x1f829b1c, ctrlreg + TH1520_CTRL_SCHED);
	writel(0x4400b00f, ctrlreg + TH1520_CTRL_SCHED1);
	writel(0x0f000001, ctrlreg + TH1520_CTRL_PERFHPR1);
	writel(0x0f00007f, ctrlreg + TH1520_CTRL_PERFLPR1);
	writel(0x0f00007f, ctrlreg + TH1520_CTRL_PERFWR1);
	writel(0x00000208, ctrlreg + TH1520_CTRL_SCHED3);
	writel(0x08400810, ctrlreg + TH1520_CTRL_SCHED4);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DBG0);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DBG1);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DBGCMD);
	writel(0x00000001, ctrlreg + TH1520_CTRL_SWCTL);
	writel(0x00000000, ctrlreg + TH1520_CTRL_SWCTLSTATIC);
	writel(0x00000001, ctrlreg + TH1520_CTRL_POISONCFG);
	writel(0x00000001, ctrlreg + TH1520_CTRL_PCTRL_0);
	writel(0x00000001, ctrlreg + TH1520_CTRL_PCTRL_1);
	writel(0x00000001, ctrlreg + TH1520_CTRL_PCTRL_2);
	writel(0x00000001, ctrlreg + TH1520_CTRL_PCTRL_3);
	writel(0x00000001, ctrlreg + TH1520_CTRL_PCTRL_4);
	writel(0x00003030, ctrlreg + TH1520_CTRL_DCH1_MRCTRL0);
	writel(0x0002d90f, ctrlreg + TH1520_CTRL_DCH1_MRCTRL1);
	writel(0x00000001, ctrlreg + TH1520_CTRL_DCH1_DERATECTL);
	writel(0x00000020, ctrlreg + TH1520_CTRL_DCH1_PWRCTL);
	writel(0x00430002, ctrlreg + TH1520_CTRL_DCH1_HWLPCTL);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DCH1_CRCPARCTL0);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DCH1_ZQCTL2);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DCH1_ODTMAP);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DCH1_DBG1);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DCH1_DBGCMD);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_RFSHCTL3, tmp,
				 tmp == 0x00000001,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000010, ctrlreg + TH1520_CTRL_PCCFG);
	writel(0x0000500f, ctrlreg + TH1520_CTRL_PCFGR_0);
	writel(0x0000500f, ctrlreg + TH1520_CTRL_PCFGW_0);
	writel(0x00005020, ctrlreg + TH1520_CTRL_PCFGR_1);
	writel(0x0000501f, ctrlreg + TH1520_CTRL_PCFGW_1);
	writel(0x0000501f, ctrlreg + TH1520_CTRL_PCFGR_2);
	writel(0x0000503f, ctrlreg + TH1520_CTRL_PCFGW_2);
	writel(0x000051ff, ctrlreg + TH1520_CTRL_PCFGR_3);
	writel(0x000051ff, ctrlreg + TH1520_CTRL_PCFGW_3);
	writel(0x0000503f, ctrlreg + TH1520_CTRL_PCFGR_4);
	writel(0x0000503f, ctrlreg + TH1520_CTRL_PCFGW_4);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_PWRCTL, tmp,
				 tmp == 0x00000020,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000020, ctrlreg + TH1520_CTRL_PWRCTL);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_DCH1_PWRCTL, tmp,
				 tmp == 0x00000020,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000020, ctrlreg + TH1520_CTRL_DCH1_PWRCTL);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DBG1);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_PWRCTL, tmp,
				 tmp == 0x00000020,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000020, ctrlreg + TH1520_CTRL_PWRCTL);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_PWRCTL, tmp,
				 tmp == 0x00000020,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000020, ctrlreg + TH1520_CTRL_PWRCTL);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DCH1_DBG1);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_DCH1_PWRCTL, tmp,
				 tmp == 0x00000020,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000020, ctrlreg + TH1520_CTRL_DCH1_PWRCTL);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_DCH1_PWRCTL, tmp,
				 tmp == 0x00000020,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000020, ctrlreg + TH1520_CTRL_DCH1_PWRCTL);
	writel(0x14000001, ctrlreg + TH1520_CTRL_DFIPHYMSTR);
	writel(0x00000000, ctrlreg + TH1520_CTRL_SWCTL);
	writel(0x00000010, ctrlreg + TH1520_CTRL_DFIMISC);
	writel(0x00000010, ctrlreg + TH1520_CTRL_DFIMISC);
	writel(0x00000002, ctrlreg + TH1520_CTRL_DBG1);
	writel(0x00000002, ctrlreg + TH1520_CTRL_DCH1_DBG1);

	switch (fw->bitwidth) {
	case 64:
		writel(0x00040018, ctrlreg + TH1520_CTRL_ADDRMAP0);
		writel(0x00090909, ctrlreg + TH1520_CTRL_ADDRMAP1);
		writel(0x00000000, ctrlreg + TH1520_CTRL_ADDRMAP2);
		writel(0x01010101, ctrlreg + TH1520_CTRL_ADDRMAP3);
		writel(0x00001f1f, ctrlreg + TH1520_CTRL_ADDRMAP4);
		writel(0x080f0808, ctrlreg + TH1520_CTRL_ADDRMAP5);
		writel(0x08080808, ctrlreg + TH1520_CTRL_ADDRMAP6);
		writel(0x00000f0f, ctrlreg + TH1520_CTRL_ADDRMAP7);
		writel(0x08080808, ctrlreg + TH1520_CTRL_ADDRMAP9);
		writel(0x08080808, ctrlreg + TH1520_CTRL_ADDRMAP10);
		writel(0x00000008, ctrlreg + TH1520_CTRL_ADDRMAP11);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int th1520_ddr_read_msg(void __iomem *phyreg, u16 *id, u16 *data)
{
	u32 tmp;
	int ret;

	ret = readw_poll_timeout(phyreg + TH1520_PHY_MSG_STATUS, tmp,
				 !(tmp & TH1520_PHY_MSG_STATUS_EMPTY),
				 TH1520_PHY_MSG_TIMEOUT_US);
	if (ret)
		return ret;

	*id   = readw(phyreg + TH1520_PHY_MSG_ID);
	*data = readw(phyreg + TH1520_PHY_MSG_DATA);

	writew(0, phyreg + TH1520_PHY_MSG_ACK);

	ret = readw_poll_timeout(phyreg + TH1520_PHY_MSG_STATUS, tmp,
				 tmp & TH1520_PHY_MSG_STATUS_EMPTY,
				 TH1520_PHY_MSG_TIMEOUT_US);
	if (ret)
		return ret;

	writew(TH1520_PHY_MSG_ACK_EN, phyreg + TH1520_PHY_MSG_ACK);

	return 0;
}

static int th1520_phy_wait_pmu_completion(void __iomem *phyreg)
{
	u16 id, data;
	int ret;

	do {
		ret = th1520_ddr_read_msg(phyreg, &id, &data);

		if (ret)
			return ret;
	} while (id != TH1520_PHY_MSG_ID_COMPLETION	&&
		 id != TH1520_PHY_MSG_ID_ERROR		&&
		 !ret);

	return id == TH1520_PHY_MSG_ID_COMPLETION ? ret : -EIO;
}

static int lpddr4_load_firmware(struct th1520_ddr_priv *priv,
				struct th1520_ddr_fw *fw)
{
	union th1520_ddr_cfg *cfg;
	size_t i, j;
	int ret;

	for (cfg = fw->cfgs, i = 0; i < fw->cfgnum; i++) {
		u32 addr = FIELD_GET(TH1520_DDR_CFG_ADDR, cfg->opaddr) * 2;
		u32 op = FIELD_GET(TH1520_DDR_CFG_OP, cfg->opaddr);

		switch (op) {
		case TH1520_DDR_CFG_PHY0:
			writew(cfg->phy.data, priv->phy0 + addr);
			break;
		case TH1520_DDR_CFG_PHY1:
			writew(cfg->phy.data, priv->phy1 + addr);
			break;
		case TH1520_DDR_CFG_PHY:
			writew(cfg->phy.data, priv->phy0 + addr);
			writew(cfg->phy.data, priv->phy1 + addr);
			break;
		case TH1520_DDR_CFG_RANGE:
			for (j = 0; j < cfg->range.num; j++) {
				writew(cfg->range.data[j],
				       priv->phy0 + addr + j * 2);
				writew(cfg->range.data[j],
				       priv->phy1 + addr + j * 2);
			}
			break;
		case TH1520_DDR_CFG_WAITFW0:
			ret = th1520_phy_wait_pmu_completion(priv->phy0);

			if (ret) {
				pr_err("phy 0 training failed: %d\n", ret);
				return ret;
			}

			break;
		case TH1520_DDR_CFG_WAITFW1:
			ret = th1520_phy_wait_pmu_completion(priv->phy1);

			if (ret) {
				pr_err("phy 1 training failed: %d\n", ret);
				return ret;
			}

			break;
		default:
			pr_err("Unknown DRAM configuration %d\n", op);

			return -EOPNOTSUPP;
		}

		if (op == TH1520_DDR_CFG_RANGE)
			cfg = (void *)cfg + sizeof(cfg->range) +
				      cfg->range.num * sizeof(u16);
		else
			cfg = (union th1520_ddr_cfg *)(&cfg->phy + 1);
	}

	return 0;
}

static int th1520_ddr_ctrl_enable(void __iomem *ctrlreg,
				  struct th1520_ddr_fw *fw)
{
	u32 tmp;
	int ret;

	writel(0x00000030, ctrlreg + TH1520_CTRL_DFIMISC);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_DFISTAT, tmp,
				 tmp == 0x00000001,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_DCH1_DFISTAT, tmp,
				 tmp == 0x00000001,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x00000010, ctrlreg + TH1520_CTRL_DFIMISC);
	writel(0x00000011, ctrlreg + TH1520_CTRL_DFIMISC);
	writel(0x0000000a, ctrlreg + TH1520_CTRL_PWRCTL);
	writel(0x0000000a, ctrlreg + TH1520_CTRL_DCH1_PWRCTL);
	writel(0x00000001, ctrlreg + TH1520_CTRL_SWCTL);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_SWSTAT, tmp,
				 tmp == 0x00000001,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_STAT, tmp,
				 tmp == 0x00000001,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_DCH1_STAT, tmp,
				 tmp == 0x00000001,
				 TH1520_CTRL_INIT_TIMEOUT_US);
	if (ret)
		return ret;

	writel(0x14000001, ctrlreg + TH1520_CTRL_DFIPHYMSTR);
	writel(0x00000000, ctrlreg + TH1520_CTRL_SWCTL);
	writel(0x00020002, ctrlreg + TH1520_CTRL_INIT0);
	writel(0x00000001, ctrlreg + TH1520_CTRL_SWCTL);

	ret = readl_poll_timeout(ctrlreg + TH1520_CTRL_SWSTAT, tmp,
				 tmp == 0x00000001,
				 TH1520_CTRL_INIT_TIMEOUT_US);

	if (ret)
		return ret;

	writel(0x00000000, ctrlreg + TH1520_CTRL_DBG1);
	writel(0x00000000, ctrlreg + TH1520_CTRL_DCH1_DBG1);

	return 0;
}

static void th1520_ddr_enable_self_refresh(void __iomem *ctrlreg,
					   void __iomem *sysreg)
{
	writel(0x00000000, ctrlreg + TH1520_CTRL_RFSHCTL3);

	writel(0x000a0000, sysreg + TH1520_SYS_DDR_CFG1);

	writel(0x00000000, ctrlreg + TH1520_CTRL_SWCTL);
	writel(0x00000001, ctrlreg + TH1520_CTRL_SWCTLSTATIC);
	writel(0x0040ae04, ctrlreg + TH1520_CTRL_PWRTMG);
	writel(0x00430003, ctrlreg + TH1520_CTRL_HWLPCTL);
	writel(0x00430003, ctrlreg + TH1520_CTRL_DCH1_HWLPCTL);
	writel(0x00000001, ctrlreg + TH1520_CTRL_SWCTL);
	writel(0x00000000, ctrlreg + TH1520_CTRL_SWCTLSTATIC);
	writel(0x0000000b, ctrlreg + TH1520_CTRL_PWRCTL);
	writel(0x0000000b, ctrlreg + TH1520_CTRL_DCH1_PWRCTL);
}

static int th1520_ddr_init(struct th1520_ddr_priv *priv)
{
	struct th1520_ddr_fw *fw = (void *)binman_sym(ulong, ddr_fw, image_pos);
	u32 reset;
	int ret;

	ret = th1520_ddr_pll_config(priv->sys, fw->freq);
	if (ret) {
		pr_err("failed to configure PLL: %d\n", ret);
		return ret;
	}

	reset = TH1520_SYS_DDR_CFG0_PHY_PWROK_RSTN;
	writel(reset, priv->sys + TH1520_SYS_DDR_CFG0);
	reset |= TH1520_SYS_DDR_CFG0_PHY_CORE_RSTN;
	writel(reset, priv->sys + TH1520_SYS_DDR_CFG0);
	reset |= TH1520_SYS_DDR_CFG0_APB_RSTN;
	writel(reset, priv->sys + TH1520_SYS_DDR_CFG0);

	ret = th1520_ddr_ctrl_init(priv->ctrl, fw);
	if (ret) {
		pr_err("failed to initialize DDR controller: %d\n", ret);
		return ret;
	}

	reset |= TH1520_SYS_DDR_CFG0_APB_PORT_RSTN(0) |
		 TH1520_SYS_DDR_CFG0_APB_PORT_RSTN(1) |
		 TH1520_SYS_DDR_CFG0_APB_PORT_RSTN(2) |
		 TH1520_SYS_DDR_CFG0_APB_PORT_RSTN(3) |
		 TH1520_SYS_DDR_CFG0_APB_PORT_RSTN(4) |
		 TH1520_SYS_DDR_CFG0_CTRL_RSTN;
	writel(reset, priv->sys + TH1520_SYS_DDR_CFG0);

	lpddr4_load_firmware(priv, fw);

	ret = th1520_ddr_ctrl_enable(priv->ctrl, fw);
	if (ret) {
		pr_err("failed to enable DDR controller: %d\n", ret);
		return ret;
	}

	th1520_ddr_enable_self_refresh(priv->ctrl, priv->sys);

	return 0;
}

static int th1520_ddr_probe(struct udevice *dev)
{
	struct th1520_ddr_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr_name(dev, "phy-0");
	priv->phy0 = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	addr = dev_read_addr_name(dev, "phy-1");
	priv->phy1 = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	addr = dev_read_addr_name(dev, "ctrl");
	priv->ctrl = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	addr = dev_read_addr_name(dev, "sys");
	priv->sys = (void __iomem *)addr;
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	return th1520_ddr_init(priv);
}

static int th1520_ddr_get_info(struct udevice *dev, struct ram_info *info)
{
	info->base = gd->ram_base;
	info->size = gd->ram_size;

	return 0;
}

static struct ram_ops th1520_ddr_ops = {
	.get_info = th1520_ddr_get_info,
};

static const struct udevice_id th1520_ddr_ids[] = {
	{ .compatible = "thead,th1520-ddrc" },
	{ }
};

U_BOOT_DRIVER(th1520_ddr) = {
	.name = "th1520_ddr",
	.id = UCLASS_RAM,
	.ops = &th1520_ddr_ops,
	.of_match = th1520_ddr_ids,
	.probe = th1520_ddr_probe,
	.priv_auto = sizeof(struct th1520_ddr_priv),
};
