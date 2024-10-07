/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#ifndef __ASM_AST2700_SDRAM_H__
#define __ASM_AST2700_SDRAM_H__

struct sdrammc_regs {
	u32 prot_key;
	u32 intr_status;
	u32 intr_clear;
	u32 intr_mask;
	u32 mcfg;
	u32 mctl;
	u32 msts;
	u32 error_status;
	u32 actime1;
	u32 actime2;
	u32 actime3;
	u32 actime4;
	u32 actime5;
	u32 actime6;
	u32 actime7;
	u32 dfi_timing;
	u32 dcfg;
	u32 dctl;
	u32 mrctl;
	u32 mrwr;
	u32 mrrd;
	u32 mr01;
	u32 mr23;
	u32 mr45;
	u32 mr67;
	u32 refctl;
	u32 refmng_ctl;
	u32 refsts;
	u32 zqctl;
	u32 ecc_addr_range;
	u32 ecc_failure_status;
	u32 ecc_failure_addr;
	u32 ecc_test_control;
	u32 ecc_test_status;
	u32 arbctl;
	u32 enccfg;
	u32 protect_lock_set;
	u32 protect_lock_status;
	u32 protect_lock_reset;
	u32 enc_min_addr;
	u32 enc_max_addr;
	u32 enc_key[4];
	u32 enc_iv[3];
	u32 bistcfg;
	u32 bist_addr;
	u32 bist_size;
	u32 bist_patt;
	u32 bist_res;
	u32 bist_fail_addr;
	u32 bist_fail_data[4];
	u32 reserved2[2];
	u32 debug_control;
	u32 debug_status;
	u32 phy_intf_status;
	u32 testcfg;
	u32 gfmcfg;
	u32 gfm0ctl;
	u32 gfm1ctl;
	u32 reserved3[0xf8];
};

#define DRAMC_UNLK_KEY	0x1688a8a8

/* offset 0x04 */
#define DRAMC_IRQSTA_PWRCTL_ERR			BIT(16)
#define DRAMC_IRQSTA_PHY_ERR			BIT(15)
#define DRAMC_IRQSTA_LOWPOWER_DONE		BIT(12)
#define DRAMC_IRQSTA_FREQ_CHG_DONE		BIT(11)
#define DRAMC_IRQSTA_REF_DONE			BIT(10)
#define DRAMC_IRQSTA_ZQ_DONE			BIT(9)
#define DRAMC_IRQSTA_BIST_DONE			BIT(8)
#define DRAMC_IRQSTA_ECC_RCVY_ERR		BIT(5)
#define DRAMC_IRQSTA_ECC_ERR			BIT(4)
#define DRAMC_IRQSTA_PROT_ERR			BIT(3)
#define DRAMC_IRQSTA_OVERSZ_ERR			BIT(2)
#define DRAMC_IRQSTA_MR_DONE			BIT(1)
#define DRAMC_IRQSTA_PHY_INIT_DONE		BIT(0)

/* offset 0x14 */
#define DRAMC_MCTL_WB_SOFT_RESET		BIT(24)
#define DRAMC_MCTL_PHY_CLK_DIS			BIT(18)
#define DRAMC_MCTL_PHY_RESET			BIT(17)
#define DRAMC_MCTL_PHY_POWER_ON			BIT(16)
#define DRAMC_MCTL_FREQ_CHG_START		BIT(3)
#define DRAMC_MCTL_PHY_LOWPOWER_START		BIT(2)
#define DRAMC_MCTL_SELF_REF_START		BIT(1)
#define DRAMC_MCTL_PHY_INIT_START		BIT(0)

/* offset 0x40 */
#define DRAMC_DFICFG_WD_POL			BIT(18)
#define DRAMC_DFICFG_CKE_OUT			BIT(17)
#define DRAMC_DFICFG_RESET			BIT(16)

/* offset 0x48 */
#define DRAMC_MRCTL_ERR_STATUS			BIT(31)
#define DRAMC_MRCTL_READY_STATUS		BIT(30)
#define DRAMC_MRCTL_MR_ADDR			BIT(8)
#define DRAMC_MRCTL_CMD_DLL_RST			BIT(7)
#define DRAMC_MRCTL_CMD_DQ_SEL			BIT(6)
#define DRAMC_MRCTL_CMD_TYPE			BIT(2)
#define DRAMC_MRCTL_CMD_WR_CTL			BIT(1)
#define DRAMC_MRCTL_CMD_START			BIT(0)

/* offset 0xC0 */
#define DRAMC_BISTRES_RUNNING			BIT(10)
#define DRAMC_BISTRES_FAIL			BIT(9)
#define DRAMC_BISTRES_DONE			BIT(8)
#define DRAMC_BISTCFG_INIT_MODE			BIT(7)
#define DRAMC_BISTCFG_PMODE			GENMASK(6, 4)
#define DRAMC_BISTCFG_BMODE			GENMASK(3, 2)
#define DRAMC_BISTCFG_ENABLE			BIT(1)
#define DRAMC_BISTCFG_START			BIT(0)
#define BIST_PMODE_CRC				(3)
#define BIST_BMODE_RW_SWITCH			(3)

/* DRAMC048 MR Control Register */
#define MR_TYPE_SHIFT				2
#define MR_RW					(0 << MR_TYPE_SHIFT)
#define MR_MPC					BIT(2)
#define MR_VREFCS				(2 << MR_TYPE_SHIFT)
#define MR_VREFCA				(3 << MR_TYPE_SHIFT)
#define MR_ADDRESS_SHIFT			8
#define MR_ADDR(n)				(((n) << MR_ADDRESS_SHIFT) | DRAMC_MRCTL_CMD_WR_CTL)
#define MR_NUM_SHIFT				4
#define MR_NUM(n)				((n) << MR_NUM_SHIFT)
#define MR_DLL_RESET				BIT(7)
#define MR_1T_MODE				BIT(16)

#endif
