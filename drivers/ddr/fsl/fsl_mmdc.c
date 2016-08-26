/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

/*
 * Generic driver for Freescale MMDC(Multi Mode DDR Controller).
 */

#include <common.h>
#include <fsl_mmdc.h>
#include <asm/io.h>

static void set_wait_for_bits_clear(void *ptr, u32 value, u32 bits)
{
	int timeout = 1000;

	out_be32(ptr, value);

	while (in_be32(ptr) & bits) {
		udelay(100);
		timeout--;
	}
	if (timeout <= 0)
		printf("Error: %p wait for clear timeout.\n", ptr);
}

void mmdc_init(void)
{
	struct mmdc_regs *mmdc = (struct mmdc_regs *)CONFIG_SYS_FSL_DDR_ADDR;
	unsigned int tmp;

	/* 1. set configuration request */
	out_be32(&mmdc->mdscr, MDSCR_ENABLE_CON_REQ);

	/* 2. configure the desired timing parameters */
	out_be32(&mmdc->mdotc,  CONFIG_MMDC_MDOTC);
	out_be32(&mmdc->mdcfg0, CONFIG_MMDC_MDCFG0);
	out_be32(&mmdc->mdcfg1, CONFIG_MMDC_MDCFG1);
	out_be32(&mmdc->mdcfg2, CONFIG_MMDC_MDCFG2);

	/* 3. configure DDR type and other miscellaneous parameters */
	out_be32(&mmdc->mdmisc, CONFIG_MMDC_MDMISC);
	out_be32(&mmdc->mpmur0,	MMDC_MPMUR0_FRC_MSR);
	out_be32(&mmdc->mdrwd,  CONFIG_MMDC_MDRWD);
	out_be32(&mmdc->mpodtctrl, CONFIG_MMDC_MPODTCTRL);

	/* 4. configure the required delay while leaving reset */
	out_be32(&mmdc->mdor,  CONFIG_MMDC_MDOR);

	/* 5. configure DDR physical parameters */
	/* set row/column address width, burst length, data bus width */
	tmp = CONFIG_MMDC_MDCTL & ~(MDCTL_SDE0 | MDCTL_SDE1);
	out_be32(&mmdc->mdctl, tmp);
	/* configure address space partition */
	out_be32(&mmdc->mdasp, CONFIG_MMDC_MDASP);

	/* 6. perform a ZQ calibration - not needed here, doing in #8b */

	/* 7. enable MMDC with the desired chip select */
#if (CONFIG_CHIP_SELECTS_PER_CTRL == 1)
		out_be32(&mmdc->mdctl, tmp | MDCTL_SDE0);
#elif (CONFIG_CHIP_SELECTS_PER_CTRL == 2)
		out_be32(&mmdc->mdctl, tmp | MDCTL_SDE0 | MDCTL_SDE1);
#endif

	/* 8a. dram init sequence: update MRs for ZQ, ODT, PRE, etc */
	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(8) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_2);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(0) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);

	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(0x19) |
				CMD_ADDR_LSB_MR_ADDR(0x30) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_0);

	/* 8b. ZQ calibration */
	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(0x4) | MDSCR_ENABLE_CON_REQ |
				CMD_ZQ_CALIBRATION | CMD_BANK_ADDR_0);

	set_wait_for_bits_clear(&mmdc->mpzqhwctrl, CONFIG_MMDC_MPZQHWCTRL,
				MPZQHWCTRL_ZQ_HW_FORCE);

	/* 9a. calibrations now, wr lvl */
	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(0x84) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);

	out_be32(&mmdc->mdscr,  MDSCR_ENABLE_CON_REQ | MDSCR_WL_EN |
				CMD_NORMAL);

	set_wait_for_bits_clear(&mmdc->mpwlgcr, MPWLGCR_HW_WL_EN,
				MPWLGCR_HW_WL_EN);

	mdelay(1);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);
	out_be32(&mmdc->mdscr, MDSCR_ENABLE_CON_REQ);

	mdelay(1);

	/* 9b. read DQS gating calibration */
	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(4) | MDSCR_ENABLE_CON_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);

	out_be32(&mmdc->mppdcmpr2, MPPDCMPR2_MPR_COMPARE_EN);

	/* set absolute read delay offset */
#if defined(CONFIG_MMDC_MPRDDLCTL)
	out_be32(&mmdc->mprddlctl, CONFIG_MMDC_MPRDDLCTL);
#else
	out_be32(&mmdc->mprddlctl, MMDC_MPRDDLCTL_DEFAULT_DELAY);
#endif
	set_wait_for_bits_clear(&mmdc->mpdgctrl0,
				AUTO_RD_DQS_GATING_CALIBRATION_EN,
				AUTO_RD_DQS_GATING_CALIBRATION_EN);

	out_be32(&mmdc->mdscr,  MDSCR_ENABLE_CON_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3);

	/* 9c. read calibration */
	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(4) | MDSCR_ENABLE_CON_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0);
	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);
	out_be32(&mmdc->mppdcmpr2,  MPPDCMPR2_MPR_COMPARE_EN);
	set_wait_for_bits_clear(&mmdc->mprddlhwctl,
				MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN,
				MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN);

	out_be32(&mmdc->mdscr,  MDSCR_ENABLE_CON_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3);

	/* 10. configure power-down, self-refresh entry, exit parameters */
	out_be32(&mmdc->mdpdc, CONFIG_MMDC_MDPDC);
	out_be32(&mmdc->mapsr, MMDC_MAPSR_PWR_SAV_CTRL_STAT);

	/* 11. ZQ config again? do nothing here */

	/* 12. refresh scheme */
	set_wait_for_bits_clear(&mmdc->mdref, CONFIG_MMDC_MDREF,
				MDREF_START_REFRESH);

	/* 13. disable CON_REQ */
	out_be32(&mmdc->mdscr, MDSCR_DISABLE_CFG_REQ);
}
