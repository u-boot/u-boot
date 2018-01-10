/*
 * Board init file for Dragonboard 820C
 *
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/sysmap-apq8096.h>
#include <linux/arm-smccc.h>
#include <linux/psci.h>
#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <asm/psci.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size  = PHYS_SDRAM_2_SIZE;

	return 0;
}

static void sdhci_power_init(void)
{
	const u32 TLMM_PULL_MASK = 0x3;
	const u32 TLMM_HDRV_MASK = 0x7;

	struct tlmm_cfg {
		u32 bit;  /* bit in the register      */
		u8 mask;  /* mask clk/dat/cmd control */
		u8 val;
	};

	/* bit offsets in the sdc tlmm register */
	enum {  SDC1_DATA_HDRV = 0,
		SDC1_CMD_HDRV  = 3,
		SDC1_CLK_HDRV  = 6,
		SDC1_DATA_PULL = 9,
		SDC1_CMD_PULL  = 11,
		SDC1_CLK_PULL  = 13,
		SDC1_RCLK_PULL = 15,
	};

	enum {  TLMM_PULL_DOWN	 = 0x1,
		TLMM_PULL_UP   = 0x3,
		TLMM_NO_PULL   = 0x0,
	};

	enum {  TLMM_CUR_VAL_10MA = 0x04,
		TLMM_CUR_VAL_16MA = 0x07,
	};
	int i;

	/* drive strength configs for sdhc pins */
	const struct tlmm_cfg hdrv[] = {
	
		{ SDC1_CLK_HDRV,  TLMM_CUR_VAL_16MA, TLMM_HDRV_MASK, },
		{ SDC1_CMD_HDRV,  TLMM_CUR_VAL_10MA, TLMM_HDRV_MASK, },
		{ SDC1_DATA_HDRV, TLMM_CUR_VAL_10MA, TLMM_HDRV_MASK, },
	};

	/* pull configs for sdhc pins */
	const struct tlmm_cfg pull[] = {
	
		{ SDC1_CLK_PULL,  TLMM_NO_PULL, TLMM_PULL_MASK, },
		{ SDC1_CMD_PULL,  TLMM_PULL_UP, TLMM_PULL_MASK, },
		{ SDC1_DATA_PULL, TLMM_PULL_UP, TLMM_PULL_MASK, },
	};

	const struct tlmm_cfg rclk[] = {
	
		{ SDC1_RCLK_PULL, TLMM_PULL_DOWN, TLMM_PULL_MASK,},
	};

	for (i = 0; i < ARRAY_SIZE(hdrv); i++)
		clrsetbits_le32(SDC1_HDRV_PULL_CTL_REG,
				hdrv[i].mask << hdrv[i].bit,
			hdrv[i].val  << hdrv[i].bit);

	for (i = 0; i < ARRAY_SIZE(pull); i++)
		clrsetbits_le32(SDC1_HDRV_PULL_CTL_REG,
				pull[i].mask << pull[i].bit,
			pull[i].val  << pull[i].bit);

	for (i = 0; i < ARRAY_SIZE(rclk); i++)
		clrsetbits_le32(SDC1_HDRV_PULL_CTL_REG,
				rclk[i].mask << rclk[i].bit,
			rclk[i].val  << rclk[i].bit);
}

static void show_psci_version(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(ARM_PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	printf("PSCI:  v%ld.%ld\n",
	       PSCI_VERSION_MAJOR(res.a0),
		PSCI_VERSION_MINOR(res.a0));
}

int board_init(void)
{
	sdhci_power_init();
	show_psci_version();

	return 0;
}

void reset_cpu(ulong addr)
{
	psci_system_reset();
