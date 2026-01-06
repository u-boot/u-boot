// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Max Merchel
 */

#include <config.h>
#include <hang.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6dl-ddr.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>

#include "../common/tq_som.h"

static void tqma6dl_init_ddr_controller(void)
{
	debug("SPL: tqma6dl ddr iom ....\n");
	/* TQMa6DL DDR config Rev. 0300D */
	/* IOMUX configuration */
	tq_som_init_write_reg(MX6_IOM_GRP_DDR_TYPE, 0x000C0000);
	tq_som_init_write_reg(MX6_IOM_GRP_DDRPKE, 0x00000000);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDCLK_0, 0x00008030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDCLK_1, 0x00008030);
	tq_som_init_write_reg(MX6_IOM_DRAM_CAS, 0x00008030);
	tq_som_init_write_reg(MX6_IOM_DRAM_RAS, 0x00008030);
	tq_som_init_write_reg(MX6_IOM_GRP_ADDDS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_RESET, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDCKE0, 0x00003000);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDCKE1, 0x00000000);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDBA2, 0x00000000);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDODT0, 0x00003030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDODT1, 0x00003030);
	tq_som_init_write_reg(MX6_IOM_GRP_CTLDS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DDRMODE_CTL, 0x00020000);

	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS0, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS1, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS2, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS3, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS4, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS5, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS6, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS7, 0x00000030);

	tq_som_init_write_reg(MX6_IOM_GRP_DDRMODE, 0x00020000);

	tq_som_init_write_reg(MX6_IOM_GRP_B0DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B1DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B2DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B3DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B4DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B5DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B6DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B7DS, 0x00000030);

	tq_som_init_write_reg(MX6_IOM_DRAM_DQM0, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM1, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM2, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM3, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM4, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM5, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM6, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM7, 0x00000030);

	debug("SPL: tqma6dl ddr calibration ....\n");
	/* memory interface calibration values */
	tq_som_init_write_reg(MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003);
	tq_som_init_write_reg(MX6_MMDC_P1_MPZQHWCTRL, 0xA1390003);
	tq_som_init_write_reg(MX6_MMDC_P0_MPWLDECTRL0, 0x00440048);
	tq_som_init_write_reg(MX6_MMDC_P0_MPWLDECTRL1, 0x003D003F);
	tq_som_init_write_reg(MX6_MMDC_P1_MPWLDECTRL0, 0x0029002D);
	tq_som_init_write_reg(MX6_MMDC_P1_MPWLDECTRL1, 0x002B0043);

	tq_som_init_write_reg(MX6_MMDC_P0_MPDGCTRL0, 0x424C0250);
	tq_som_init_write_reg(MX6_MMDC_P0_MPDGCTRL1, 0x02300234);
	tq_som_init_write_reg(MX6_MMDC_P1_MPDGCTRL0, 0x4234023C);
	tq_som_init_write_reg(MX6_MMDC_P1_MPDGCTRL1, 0x0224022C);

	tq_som_init_write_reg(MX6_MMDC_P0_MPRDDLCTL, 0x48484C4C);
	tq_som_init_write_reg(MX6_MMDC_P1_MPRDDLCTL, 0x4C4E4E4C);
	tq_som_init_write_reg(MX6_MMDC_P0_MPWRDLCTL, 0x36382C36);
	tq_som_init_write_reg(MX6_MMDC_P1_MPWRDLCTL, 0x34343630);
	tq_som_init_write_reg(MX6_MMDC_P0_MPRDDQBY0DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P0_MPRDDQBY1DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P0_MPRDDQBY2DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P0_MPRDDQBY3DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P1_MPRDDQBY0DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P1_MPRDDQBY1DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P1_MPRDDQBY2DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P1_MPRDDQBY3DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P0_MPMUR0, 0x00000800);
	tq_som_init_write_reg(MX6_MMDC_P1_MPMUR0, 0x00000800);

	debug("SPL: tqma6dl ddr config ....\n");
	/* configure memory interface */
	tq_som_init_write_reg(MX6_MMDC_P0_MDPDC, 0x0002002D);
	tq_som_init_write_reg(MX6_MMDC_P0_MDOTC, 0x00333030);
	tq_som_init_write_reg(MX6_MMDC_P0_MDCFG0, 0x3F435333);
	tq_som_init_write_reg(MX6_MMDC_P0_MDCFG1, 0xB68E8B63);
	tq_som_init_write_reg(MX6_MMDC_P0_MDCFG2, 0x01FF00DB);
	tq_som_init_write_reg(MX6_MMDC_P0_MDMISC, 0x00011740);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00008000);

	debug("SPL: tqma6dl MX6_MMDC_P0_MDSCR %x ....\n", __raw_readl(MX6_MMDC_P0_MDSCR));

	tq_som_init_write_reg(MX6_MMDC_P0_MDRWD, 0x000026D2);
	tq_som_init_write_reg(MX6_MMDC_P0_MDOR, 0x00431023);
	tq_som_init_write_reg(MX6_MMDC_P0_MDASP, 0x00000027);

	debug("SPL: tqma6dl ddr mdctl - leave reset\n");
	tq_som_init_write_reg(MX6_MMDC_P0_MDCTL, 0x831A0000);

	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00408032);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00008033);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00048031);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x15208030);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x04008040);
	tq_som_init_write_reg(MX6_MMDC_P0_MDREF, 0x00007800);
	tq_som_init_write_reg(MX6_MMDC_P0_MPODTCTRL, 0x00022222);
	tq_som_init_write_reg(MX6_MMDC_P1_MPODTCTRL, 0x00022222);
	tq_som_init_write_reg(MX6_MMDC_P0_MDPDC, 0x0002552D);
	tq_som_init_write_reg(MX6_MMDC_P0_MAPSR, 0x00001006);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00000000);

	/* wait for auto-ZQ calibration to complete */
	mdelay(1);
	debug("SPL: tqma6dl ddr init done ...\n");
}

void tq_som_ram_init(void)
{
	if (!is_mx6dl()) {
		printf("Not running on TQMa6DL\n");
		hang();
	}

	tqma6dl_init_ddr_controller();

	if (tq_som_ram_check_size(SZ_1G)) {
		debug("SPL: TQMa6DL ddr init done ...\n");
	} else {
		pr_err("ERROR: Invalid DDR RAM size\n");
		hang();
	}
}
