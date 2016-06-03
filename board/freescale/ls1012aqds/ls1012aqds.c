/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/fdt.h>
#include <asm/arch/soc.h>
#include <ahci.h>
#include <hwconfig.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_mmdc.h>
#include <spl.h>
#include <netdev.h>

#include "../common/qixis.h"
#include "ls1012aqds_qixis.h"

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
	char buf[64];
	u8 sw;

	sw = QIXIS_READ(arch);
	printf("Board Arch: V%d, ", sw >> 4);
	printf("Board version: %c, boot from ", (sw & 0xf) + 'A' - 1);

	sw = QIXIS_READ(brdcfg[QIXIS_LBMAP_BRDCFG_REG]);

	if (sw & QIXIS_LBMAP_ALTBANK)
		printf("flash: 2\n");
	else
		printf("flash: 1\n");

	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());

	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));
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

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	u8 mux_sdhc_cd = 0x80;

	i2c_set_bus_num(0);

	i2c_write(CONFIG_SYS_I2C_FPGA_ADDR, 0x5a, 1, &mux_sdhc_cd, 1);
	return 0;
}
#endif

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)
				   CONFIG_SYS_CCI400_ADDR;

	/* Set CCI-400 control override register to enable barrier
	 * transaction */
	out_le32(&cci->ctrl_ord,
		 CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif
	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	return 0;
}
#endif
