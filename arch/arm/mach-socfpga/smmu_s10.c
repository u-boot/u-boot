// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/firewall.h>
#include <asm/arch/smmu_s10.h>
#include <asm/arch/system_manager.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct smmu_stream_id dev_stream_id[] = {
	{SYSMGR_EMAC0_SID_ADDR, 0x01, EMAC_W_OFST, EMAC_R_OFST},
	{SYSMGR_EMAC1_SID_ADDR, 0x02, EMAC_W_OFST, EMAC_R_OFST},
	{SYSMGR_EMAC2_SID_ADDR, 0x03, EMAC_W_OFST, EMAC_R_OFST},
	{SYSMGR_NAND_SID_ADDR,  0x04, NAND_W_OFST, NAND_R_OFST},
	{SYSMGR_SDMMC_SID_ADDR, 0x05, SDMMC_OFST, SDMMC_OFST},
	{SYSMGR_USB0_SID_ADDR,  0x06, USB_OFST, USB_OFST},
	{SYSMGR_USB1_SID_ADDR,  0x07, USB_OFST, USB_OFST},
	{SYSMGR_DMA_SID_ADDR,   0x08, DMA_W_OFST, DMA_R_OFST},
	{SYSMGR_ETR_SID_ADDR,   0x09, ETR_W_OFST, ETR_R_OFST},
};

static void set_smmu_streamid(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dev_stream_id); i++) {
		u32 mask = SMMU_SET_STREAMID(0x3FF,
					 dev_stream_id[i].r_bit_ofst,
					 dev_stream_id[i].w_bit_ofst);
		u32 value = SMMU_SET_STREAMID(dev_stream_id[i].sid,
					 dev_stream_id[i].r_bit_ofst,
					 dev_stream_id[i].w_bit_ofst);

		clrbits_le32(dev_stream_id[i].addr, mask);
		setbits_le32(dev_stream_id[i].addr, value);
	}
}

/*
 * Need to set the Secure bit (to make it non-secure) on each peripheral
 * so that SMMU can access the peripheral
 */
static void set_smmu_accessible_reg(void)
{
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC0,
		     BIT(27) | BIT(25));
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC1,
		     BIT(27) | BIT(25));
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC2,
		     BIT(27) | BIT(25));
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_NANDGRP_L3MASTER,
		     BIT(21) | BIT(17));
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_SDMMC_L3MASTER,
		     BIT(5));
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_USB0_L3MASTER,
		     BIT(9));
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_USB1_L3MASTER,
		     BIT(9));
}

static inline void setup_smmu_firewall(void)
{
	/* Enable nonsecure SMMU accesses */
	writel(FIREWALL_L4_DISABLE_ALL, SOCFPGA_FIREWALL_TCU);
}

void socfpga_init_smmu(void)
{
	setup_smmu_firewall();
	set_smmu_streamid();
	set_smmu_accessible_reg();
}

int is_smmu_bypass(void)
{
	return readl(SOCFPGA_SMMU_ADDRESS + SMMU_SCR0) & SMMU_SCR0_CLIENTPD;
}

int is_smmu_stream_id_enabled(u32 stream_id)
{
	int i;
	u32 smrg_num;
	u32 smr, s2cr, sid_mask;
	u32 cb, cb_index, cb_num;

	if (is_smmu_bypass())
		return 0;

	/* Get number of Stream Mapping Register Groups */
	smrg_num = readl(SOCFPGA_SMMU_ADDRESS + SMMU_SIDR0) &
		   SMMU_SIDR0_NUMSMRG_MASK;

	/* Get number of Context Bank */
	cb_num = readl(SOCFPGA_SMMU_ADDRESS + SMMU_SIDR1) &
		 SMMU_SIDR1_NUMCB_MASK;

	for (i = 0; i < smrg_num; i++) {
		smr = readl(SOCFPGA_SMMU_ADDRESS + SMMU_GR0_SMR((u64)i));
		sid_mask = (smr & SMMU_SMR_MASK) >> 16;

		/* Skip if Stream ID is invalid or not matched */
		if (!(smr & SMMU_SMR_VALID) || (smr & sid_mask) != stream_id)
			continue;

		/* Get Context Bank index from valid matching Stream ID */
		s2cr = readl(SOCFPGA_SMMU_ADDRESS + SMMU_GR0_S2CR((u64)i));
		cb_index = s2cr & SMMU_S2CR_CBNDX;

		/* Skip if Context Bank is invalid or not Translation mode */
		if (cb_index >= cb_num || (s2cr & SMMU_S2CR_TYPE))
			continue;

		cb = readl(SOCFPGA_SMMU_ADDRESS + SMMU_GR0_CB((u64)cb_index,
							      SMMU_CB_SCTLR));
		/* Return MMU enable status for this Context Bank */
		return (cb & SMMU_CB_SCTLR_M);
	}

	return 0;
}
