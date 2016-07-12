/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <environment.h>
#include <fsl_mmdc.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

static void set_wait_for_bits_clear(void *ptr, u32 value, u32 bits)
{
	int timeout = 1000;

	out_be32(ptr, value);

	while (in_be32(ptr) & bits) {
		udelay(100);
		timeout--;
	}
	if (timeout <= 0)
		puts("Error: wait for clear timeout.\n");
}

int checkboard(void)
{
	u8 in1;

	puts("Board: LS1012ARDB ");

	/* Initialize i2c early for Serial flash bank information */
	i2c_set_bus_num(0);

	if (i2c_read(I2C_MUX_IO1_ADDR, 1, 1, &in1, 1) < 0) {
		printf("Error reading i2c boot information!\n");
		return 0; /* Don't want to hang() on this error */
	}

	puts("Version");
	if ((in1 & (~__SW_REV_MASK)) == __SW_REV_A)
		puts(": RevA");
	else if ((in1 & (~__SW_REV_MASK)) == __SW_REV_B)
		puts(": RevB");
	else
		puts(": unknown");

	printf(", boot from QSPI");
	if ((in1 & (~__SW_BOOT_MASK)) == __SW_BOOT_EMU)
		puts(": emu\n");
	else if ((in1 & (~__SW_BOOT_MASK)) == __SW_BOOT_BANK1)
		puts(": bank1\n");
	else if ((in1 & (~__SW_BOOT_MASK)) == __SW_BOOT_BANK2)
		puts(": bank2\n");
	else
		puts("unknown\n");

	return 0;
}

void mmdc_init(void)
{
	struct mmdc_p_regs *mmdc =
		(struct mmdc_p_regs *)CONFIG_SYS_FSL_DDR_ADDR;

	out_be32(&mmdc->mdscr, CONFIGURATION_REQ);

	/* configure timing parms */
	out_be32(&mmdc->mdotc,  CONFIG_SYS_MMDC_CORE_ODT_TIMING);
	out_be32(&mmdc->mdcfg0, CONFIG_SYS_MMDC_CORE_TIMING_CFG_0);
	out_be32(&mmdc->mdcfg1, CONFIG_SYS_MMDC_CORE_TIMING_CFG_1);
	out_be32(&mmdc->mdcfg2, CONFIG_SYS_MMDC_CORE_TIMING_CFG_2);

	/* other parms	*/
	out_be32(&mmdc->mdmisc,    CONFIG_SYS_MMDC_CORE_MISC);
	out_be32(&mmdc->mpmur0,    CONFIG_SYS_MMDC_PHY_MEASURE_UNIT);
	out_be32(&mmdc->mdrwd,     CONFIG_SYS_MMDC_CORE_RDWR_CMD_DELAY);
	out_be32(&mmdc->mpodtctrl, CONFIG_SYS_MMDC_PHY_ODT_CTRL);

	/* out of reset delays */
	out_be32(&mmdc->mdor,  CONFIG_SYS_MMDC_CORE_OUT_OF_RESET_DELAY);

	/* physical parms */
	out_be32(&mmdc->mdctl, CONFIG_SYS_MMDC_CORE_CONTROL_1);
	out_be32(&mmdc->mdasp, CONFIG_SYS_MMDC_CORE_ADDR_PARTITION);

       /* Enable MMDC */
	out_be32(&mmdc->mdctl, CONFIG_SYS_MMDC_CORE_CONTROL_2);

	/* dram init sequence: update MRs */
	out_be32(&mmdc->mdscr, (CMD_ADDR_LSB_MR_ADDR(0x8) | CONFIGURATION_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_2));
	out_be32(&mmdc->mdscr, (CONFIGURATION_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3));
	out_be32(&mmdc->mdscr, (CMD_ADDR_LSB_MR_ADDR(0x4) | CONFIGURATION_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1));
	out_be32(&mmdc->mdscr, (CMD_ADDR_MSB_MR_OP(0x19) |
				CMD_ADDR_LSB_MR_ADDR(0x30) | CONFIGURATION_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_0));

       /* dram init sequence: ZQCL */
	out_be32(&mmdc->mdscr, (CMD_ADDR_MSB_MR_OP(0x4) | CONFIGURATION_REQ |
				CMD_ZQ_CALIBRATION | CMD_BANK_ADDR_0));
	set_wait_for_bits_clear(&mmdc->mpzqhwctrl,
				CONFIG_SYS_MMDC_PHY_ZQ_HW_CTRL,
				FORCE_ZQ_AUTO_CALIBRATION);

       /* Calibrations now: wr lvl */
	out_be32(&mmdc->mdscr, (CMD_ADDR_LSB_MR_ADDR(0x84) |
				CONFIGURATION_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_1));
	out_be32(&mmdc->mdscr, (CONFIGURATION_REQ | WL_EN | CMD_NORMAL));
	set_wait_for_bits_clear(&mmdc->mpwlgcr, WR_LVL_HW_EN, WR_LVL_HW_EN);

	mdelay(1);

	out_be32(&mmdc->mdscr, (CMD_ADDR_LSB_MR_ADDR(0x4) | CONFIGURATION_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1));
	out_be32(&mmdc->mdscr, CONFIGURATION_REQ);

	mdelay(1);

       /* Calibrations now: Read DQS gating calibration */
	out_be32(&mmdc->mdscr, (CMD_ADDR_MSB_MR_OP(0x4) | CONFIGURATION_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0));
	out_be32(&mmdc->mdscr, (CMD_ADDR_LSB_MR_ADDR(0x4) | CONFIGURATION_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3));
	out_be32(&mmdc->mppdcmpr2, MPR_COMPARE_EN);
	out_be32(&mmdc->mprddlctl, CONFIG_SYS_MMDC_PHY_RD_DLY_LINES_CFG);
	set_wait_for_bits_clear(&mmdc->mpdgctrl0,
				AUTO_RD_DQS_GATING_CALIBRATION_EN,
				AUTO_RD_DQS_GATING_CALIBRATION_EN);

	out_be32(&mmdc->mdscr, (CONFIGURATION_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3));

       /* Calibrations now: Read calibration */
	out_be32(&mmdc->mdscr, (CMD_ADDR_MSB_MR_OP(0x4) | CONFIGURATION_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0));
	out_be32(&mmdc->mdscr, (CMD_ADDR_LSB_MR_ADDR(0x4) | CONFIGURATION_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3));
	out_be32(&mmdc->mppdcmpr2,  MPR_COMPARE_EN);
	set_wait_for_bits_clear(&mmdc->mprddlhwctl,
				AUTO_RD_CALIBRATION_EN,
				AUTO_RD_CALIBRATION_EN);

	out_be32(&mmdc->mdscr, (CONFIGURATION_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3));

       /* PD, SR */
	out_be32(&mmdc->mdpdc, CONFIG_SYS_MMDC_CORE_PWR_DOWN_CTRL);
	out_be32(&mmdc->mapsr, CONFIG_SYS_MMDC_CORE_PWR_SAV_CTRL_STAT);

       /* refresh scheme */
	set_wait_for_bits_clear(&mmdc->mdref,
				CONFIG_SYS_MMDC_CORE_REFRESH_CTL,
				START_REFRESH);

       /* disable CON_REQ */
	out_be32(&mmdc->mdscr, DISABLE_CFG_REQ);
}

int dram_init(void)
{
	mmdc_init();

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)CONFIG_SYS_CCI400_ADDR;
	/*
	 * Set CCI-400 control override register to enable barrier
	 * transaction
	 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	return 0;
}
