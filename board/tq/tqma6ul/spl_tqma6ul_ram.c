// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Max Merchel
 */

#include <config.h>
#include <hang.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6ul-ddr.h>
#include <asm/arch/sys_proto.h>
#include <linux/sizes.h>

#include "../common/tq_som.h"

static void tqma6ul_init_ddr_controller(u32 size)
{
	/* TQMa6ul DDR config */

	/* reset DDR via Chip Select 0*/
	tq_som_init_write_reg(MX6_MMDC_P0_MDCTL, 0x03180000);
	tq_som_init_write_reg(MX6_MMDC_P0_MDCTL, 0x83180000);

	debug("SPL: tqma6ul ddr iomux ....\n");

	/* DDR IO TYPE: */
	tq_som_init_write_reg(MX6_IOM_GRP_DDR_TYPE, 0x000C0000);
	tq_som_init_write_reg(MX6_IOM_GRP_DDRPKE, 0x00000000);
	/* CLOCK: */
	tq_som_init_write_reg(MX6_IOM_DRAM_SDCLK_0, 0x00000030);
	/* Control: */
	tq_som_init_write_reg(MX6_IOM_DRAM_CAS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_RAS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_ADDDS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_RESET, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDBA2, 0x00000000);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDODT0, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDODT1, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_CTLDS, 0x00000030);
	/* Data Strobes: */
	tq_som_init_write_reg(MX6_IOM_DDRMODE_CTL, 0x00020000);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS0, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_SDQS1, 0x00000030);
	/* Data: */
	tq_som_init_write_reg(MX6_IOM_GRP_DDRMODE, 0x00020000);
	tq_som_init_write_reg(MX6_IOM_GRP_B0DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_GRP_B1DS, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM0, 0x00000030);
	tq_som_init_write_reg(MX6_IOM_DRAM_DQM1, 0x00000030);

	debug("tqma6ul ddr controller registers ....\n");

	/* MMDC_MDSCR - MMDC Core Special Command Register */
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00008000);

	debug("tqma6ul ddr calibrations ....\n");

	/* DDR_PHY_P0_MPZQHWCTRL , enable both one-time & periodic HW ZQ calibration. */
	tq_som_init_write_reg(MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003);

	switch (size) {
	case SZ_512M:
		if (IS_ENABLED(CONFIG_MX6UL)) {
			debug("tqma6ul ddr calibration standard variant ....\n");

			tq_som_init_write_reg(MX6_MMDC_P0_MPWLDECTRL0, 0x00000000);
			tq_som_init_write_reg(MX6_MMDC_P0_MPDGCTRL0, 0x41580150);
			tq_som_init_write_reg(MX6_MMDC_P0_MPRDDLCTL, 0x40404E52);
			tq_som_init_write_reg(MX6_MMDC_P0_MPWRDLCTL, 0x40404E4A);

		} else if (IS_ENABLED(CONFIG_MX6ULL)) {
			if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD)) {
				debug("tqma6ull ddr calibration standard variant ....\n");

				tq_som_init_write_reg(MX6_MMDC_P0_MPWLDECTRL0, 0x00090009);
				tq_som_init_write_reg(MX6_MMDC_P0_MPDGCTRL0, 0x4140013C);
				tq_som_init_write_reg(MX6_MMDC_P0_MPRDDLCTL, 0x40403A3E);
				tq_som_init_write_reg(MX6_MMDC_P0_MPWRDLCTL, 0x40402E26);

			} else if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA)) {
				debug("tqma6ull ddr calibration lga variant ....\n");

				tq_som_init_write_reg(MX6_MMDC_P0_MPWLDECTRL0, 0x00050009);
				tq_som_init_write_reg(MX6_MMDC_P0_MPDGCTRL0, 0x41340130);
				tq_som_init_write_reg(MX6_MMDC_P0_MPRDDLCTL, 0x40403A3E);
				tq_som_init_write_reg(MX6_MMDC_P0_MPWRDLCTL, 0x40402E28);

			} else {
				pr_err("invalid/unsupported SoM variant ....\n");
				hang();
			} /* IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD) */
		} else {
			pr_err("ERROR: invalid/unsupported CPU variant ....\n");
			hang();
		} /* IS_ENABLED(CONFIG_MX6UL) */
		break;
	case SZ_256M:
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD)) {
			debug("tqma6ul ddr calibration standard variant ....\n");

			tq_som_init_write_reg(MX6_MMDC_P0_MPWLDECTRL0, 0x00000000);
			tq_som_init_write_reg(MX6_MMDC_P0_MPDGCTRL0, 0x41480144);
			tq_som_init_write_reg(MX6_MMDC_P0_MPRDDLCTL, 0x40404E54);
			tq_som_init_write_reg(MX6_MMDC_P0_MPWRDLCTL, 0x40404E48);

		} else if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA)) {
			debug("tqma6ul ddr calibration lga variant ....\n");

			tq_som_init_write_reg(MX6_MMDC_P0_MPWLDECTRL0, 0x00130003);
			tq_som_init_write_reg(MX6_MMDC_P0_MPDGCTRL0, 0x41540154);
			tq_som_init_write_reg(MX6_MMDC_P0_MPRDDLCTL, 0x40405050);
			tq_som_init_write_reg(MX6_MMDC_P0_MPWRDLCTL, 0x40404E4C);

		} else {
			pr_err("ERROR: invalid/unsupported SoM variant ....\n");
			hang();
		} /* IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD) */
		break;
	default:
		pr_err("ERROR: invalid/unsupported RAM size ....\n");
		hang();
		break;
	}

	tq_som_init_write_reg(MX6_MMDC_P0_MPRDDQBY0DL, 0x33333333);
	tq_som_init_write_reg(MX6_MMDC_P0_MPRDDQBY1DL, 0x33333333);

	tq_som_init_write_reg(0x021B082C, 0xf3333333); /* MMDC_MPWRDQBY0DL */
	tq_som_init_write_reg(0x021B0830, 0xf3333333); /* MMDC_MPWRDQBY1DL */
	tq_som_init_write_reg(0x021B08C0, 0x00921012); /* MMDC_MPDCCR */

	/*
	 * Complete calibration by forced measurement:
	 */
	tq_som_init_write_reg(MX6_MMDC_P0_MPMUR0, 0x00000800);
	tq_som_init_write_reg(MX6_MMDC_P0_MDPDC, 0x0002002D);

	debug("tqma6ul ddr mmdc ....\n");

	tq_som_init_write_reg(MX6_MMDC_P0_MDOTC, 0x00333030);
	tq_som_init_write_reg(MX6_MMDC_P0_MDCFG0, 0x676B52F3);
	tq_som_init_write_reg(MX6_MMDC_P0_MDCFG1, 0xB66D8B63);
	tq_som_init_write_reg(MX6_MMDC_P0_MDCFG2, 0x01FF00DB);
	tq_som_init_write_reg(MX6_MMDC_P0_MDMISC, 0x00201740);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00008000);
	tq_som_init_write_reg(MX6_MMDC_P0_MDRWD, 0x000026D2);
	tq_som_init_write_reg(MX6_MMDC_P0_MDOR, 0x006B1023);

	switch (size) {
	case SZ_512M:
		tq_som_init_write_reg(MX6_MMDC_P0_MDASP, 0x0000004F);
		tq_som_init_write_reg(MX6_MMDC_P0_MDCTL, 0x84180000);
		break;
	case SZ_256M:
		tq_som_init_write_reg(MX6_MMDC_P0_MDASP, 0x00000047);
		tq_som_init_write_reg(MX6_MMDC_P0_MDCTL, 0x83180000);
		break;
	default:
		hang();
		break;
	}

	debug("tqma6ul ddr cs0 ....\n");
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x02008032);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00008033);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00048031);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x15208030);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x04008040);
	tq_som_init_write_reg(MX6_MMDC_P0_MDREF, 0x00000800);
	tq_som_init_write_reg(MX6_MMDC_P0_MPODTCTRL, 0x00000227);
	tq_som_init_write_reg(MX6_MMDC_P0_MDPDC, 0x0002552D);
	tq_som_init_write_reg(MX6_MMDC_P0_MAPSR, 0x00011006);
	tq_som_init_write_reg(MX6_MMDC_P0_MDSCR, 0x00000000);
}

void tq_som_ram_init(void)
{
	int i;
	/* RAM sizes need to be in descending order */
	static const u32 ram_sizes[] = {
#if IS_ENABLED(CONFIG_TQMA6UL_RAM_512M)
		SZ_512M,
#endif
#if IS_ENABLED(CONFIG_TQMA6UL_RAM_256M)
		SZ_256M,
#endif
	};

	if (!is_mx6ul() && !is_mx6ull()) {
		pr_err("ERROR: Not running on TQMa6UL[L]\n");
		hang();
	}

	for (i = 0; i < ARRAY_SIZE(ram_sizes); i++) {
		tqma6ul_init_ddr_controller(ram_sizes[i]);
		if (tq_som_ram_check_size(ram_sizes[i]))
			break;
	}

	if (i < ARRAY_SIZE(ram_sizes)) {
		debug("SPL: tqma6ul ddr init done ...\n");
	} else {
		pr_err("ERROR: Invalid DDR RAM size\n");
		hang();
	}
}
