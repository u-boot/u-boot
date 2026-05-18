// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Alexander Feilke
 */

#include <config.h>
#include <hang.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/iomux-v3.h>
#include <linux/sizes.h>

#include "../common/tq_som.h"

#define DDRC_RFSHTMG_512M 0x0020002B
#define DDRC_RFSHTMG_1G   0x00200045
#define DDRC_RFSHTMG_2G   0x0020005D

#define DDRC_ADDRMAP1_512M 0x00161616
#define DDRC_ADDRMAP1_1G   0x00171717
#define DDRC_ADDRMAP1_2G   0x00181818

#define DDRC_ADDRMAP6_512M 0x0F0F0404
#define DDRC_ADDRMAP6_1G   0x0F040404
#define DDRC_ADDRMAP6_2G   0x04040404

#define DDR_PHY_OFFSET_RD_CON0_512M 0x0B0B0B0B
#define DDR_PHY_OFFSET_RD_CON0_1G   0x0B0B0B0B
#define DDR_PHY_OFFSET_RD_CON0_2G   0x0A0A0A0A

#define DDR_PHY_OFFSET_WR_CON0_512M 0x06060606
#define DDR_PHY_OFFSET_WR_CON0_1G   0x06060606
#define DDR_PHY_OFFSET_WR_CON0_2G   0x04040404

static void tqma7_ddr_exit_retention(void)
{
	/* Clear then set bit30 to ensure exit from DDR retention */
	tq_som_init_write_reg(0x30360388, 0x40000000);
	tq_som_init_write_reg(0x30360384, 0x40000000);
}

static void gpr_init(void)
{
	/* reset default and enable GPR OCRAM EPDC */
	tq_som_init_write_reg(0x30340004, 0x4F400005);
}

static void tqma7_ccgr_init(void)
{
	tq_som_init_write_reg(0x30384130, 0x00000000); /* CCM_CCGR19 */
	tq_som_init_write_reg(0x30340020, 0x00000178); /* IOMUXC_GPR_GPR8 */
	tq_som_init_write_reg(0x30384130, 0x00000002); /* CCM_CCGR19 */
	tq_som_init_write_reg(0x30790018, 0x0000000f); /* DDR_PHY_LP_CON0 */

	/* wait for auto-ZQ calibration to complete */
	tq_som_check_bits_set(0x307a0004, 0x1); /* DDRC_STAT */
}

static void ddr_init_error(const char *msg)
{
	pr_err("%s", msg);
	hang();
}

#define TQMA7_SELECT_DDR_VALUE(SIZE, NAME) \
	((SIZE) == SZ_512M ? NAME ## _512M : \
	((SIZE) == SZ_1G ? NAME ## _1G :     \
	((SIZE) == SZ_2G ? NAME ## _2G :     \
	(ddr_init_error("Invalid DDR RAM size detected"), 0))))

static void tqma7_init_ddr_controller(u32 size)
{
	gpr_init();

	/* TQMa7 DDR config */
	/* TQMa7x DRAM Timing REV0201A */
	/* DCD Code i.MX7D/S 528 MHz 512 MByte Samsung K4B2G1646F */
	tq_som_init_write_reg(0x30360070, 0x0070302C);   /*CCM_ANALOG_PLL_DDRx*/
	tq_som_init_write_reg(0x30360090, 0x00000000);   /*CCM_ANALOG_PLL_NUM*/
	tq_som_init_write_reg(0x30360070, 0x0060302C);   /*CCM_ANALOG_PLL_DDRx*/

	tq_som_check_bits_set(0x30360070, 0x80000000);

	tq_som_init_write_reg(0x30391000, 0x00000002);   /*SRC_DDRC_RCR*/
	tq_som_init_write_reg(0x307a0000, 0x01040001);   /*DDRC_MSTR*/
	tq_som_init_write_reg(0x307a01a0, 0x80400003);   /*DDRC_DFIUPD0*/
	tq_som_init_write_reg(0x307a01a4, 0x00100020);   /*DDRC_DFIUPD1*/
	tq_som_init_write_reg(0x307a01a8, 0x80100004);   /*DDRC_DFIUPD2*/
	tq_som_init_write_reg(0x307a0064, TQMA7_SELECT_DDR_VALUE(size, DDRC_RFSHTMG));
	tq_som_init_write_reg(0x307a0490, 0x00000001);   /*DDRC_MP_PCTRL_0*/
	tq_som_init_write_reg(0x307a00d0, 0x00020081);   /*DDRC_INIT0*/
	tq_som_init_write_reg(0x307a00d4, 0x00680000);   /*DDRC_INIT1*/
	tq_som_init_write_reg(0x307a00dc, 0x09300004);   /*DDRC_INIT3*/
	tq_som_init_write_reg(0x307a00e0, 0x00480000);   /*DDRC_INIT4*/
	tq_som_init_write_reg(0x307a00e4, 0x00100004);   /*DDRC_INIT5*/
	tq_som_init_write_reg(0x307a00f4, 0x0000033F);   /*DDRC_RANKCTL*/
	tq_som_init_write_reg(0x307a0100, 0x090E0809);   /*DDRC_DRAMTMG0*/
	tq_som_init_write_reg(0x307a0104, 0x0007020E);   /*DDRC_DRAMTMG1*/
	tq_som_init_write_reg(0x307a0108, 0x03040407);   /*DDRC_DRAMTMG2*/
	tq_som_init_write_reg(0x307a010c, 0x00002006);   /*DDRC_DRAMTMG3*/
	tq_som_init_write_reg(0x307a0110, 0x04020304);   /*DDRC_DRAMTMG4*/
	tq_som_init_write_reg(0x307a0114, 0x03030202);   /*DDRC_DRAMTMG5*/
	tq_som_init_write_reg(0x307a0120, 0x00000803);   /*DDRC_DRAMTMG8*/
	tq_som_init_write_reg(0x307a0180, 0x00800020);   /*DDRC_ZQCTL0*/
	tq_som_init_write_reg(0x307a0190, 0x02098204);   /*DDRC_DFITMG0*/
	tq_som_init_write_reg(0x307a0194, 0x00030303);   /*DDRC_DFITMG1*/
	tq_som_init_write_reg(0x307a0200, 0x0000001F);   /*DDRC_ADDRMAP0*/
	tq_som_init_write_reg(0x307a0204, TQMA7_SELECT_DDR_VALUE(size, DDRC_ADDRMAP1));
	tq_som_init_write_reg(0x307a020C, 0x00000000);   /*DDRC_ADDRMAP3*/
	tq_som_init_write_reg(0x307a0210, 0x00000F0F);   /*DDRC_ADDRMAP4*/
	tq_som_init_write_reg(0x307a0214, 0x04040404);   /*DDRC_ADDRMAP5*/
	tq_som_init_write_reg(0x307a0218, TQMA7_SELECT_DDR_VALUE(size, DDRC_ADDRMAP6));
	tq_som_init_write_reg(0x307a0240, 0x06000604);   /*DDRC_ODTCFG*/
	tq_som_init_write_reg(0x307a0244, 0x00000001);   /*DDRC_ODTMAP*/
	tq_som_init_write_reg(0x30391000, 0x00000000);   /*SRC_DDRC_RCR*/
	tq_som_init_write_reg(0x30790000, 0x17420F40);   /*DDR_PHY_PHY_CON0*/
	tq_som_init_write_reg(0x30790004, 0x10210100);   /*DDR_PHY_PHY_CON1*/
	tq_som_init_write_reg(0x30790010, 0x00060807);   /*DDR_PHY_PHY_CON4*/
	tq_som_init_write_reg(0x307900b0, 0x1010007E);   /*DDR_PHY_MDLL_CON0*/
	tq_som_init_write_reg(0x3079009c, 0x00000924);   /*DDR_PHY_DRVDS_CON0*/

	tq_som_init_write_reg(0x30790020, TQMA7_SELECT_DDR_VALUE(size, DDR_PHY_OFFSET_RD_CON0));
	tq_som_init_write_reg(0x30790030, TQMA7_SELECT_DDR_VALUE(size, DDR_PHY_OFFSET_WR_CON0));
	tq_som_init_write_reg(0x30790050, 0x01000010);   /*DDR_PHY_CMD_SDLL_CON0*/
	tq_som_init_write_reg(0x30790050, 0x00000010);   /*DDR_PHY_CMD_SDLL_CON0*/

	tq_som_init_write_reg(0x307900c0, 0x0C407304);   /*DDR_PHY_ZQ_CON0*/
	tq_som_init_write_reg(0x307900c0, 0x0C447304);   /*DDR_PHY_ZQ_CON0*/
	tq_som_init_write_reg(0x307900c0, 0x0C447306);   /*DDR_PHY_ZQ_CON0*/

	tq_som_check_bits_set(0x307900c4, 0x1);      /*ZQ Calibration is finished*/

	tq_som_init_write_reg(0x307900c0, 0x0C447304);   /*DDR_PHY_ZQ_CON0*/
	tq_som_init_write_reg(0x307900c0, 0x0C407304);   /*DDR_PHY_ZQ_CON0*/

	tqma7_ccgr_init();
}

void tq_som_ram_init(void)
{
	/* RAM sizes need to be in descending order */
	static const u32 ram_sizes[] = {
#if IS_ENABLED(CONFIG_TQMA7_RAM_2G)
		SZ_2G,
#endif
#if IS_ENABLED(CONFIG_TQMA7_RAM_1G)
		SZ_1G,
#endif
#if IS_ENABLED(CONFIG_TQMA7_RAM_512M)
		SZ_512M,
#endif
	};
	int i;

	debug("SPL: tqma7 iomux ....\n");
	tqma7_ddr_exit_retention();

	for (i = 0; i < ARRAY_SIZE(ram_sizes); i++) {
		tqma7_init_ddr_controller(ram_sizes[i]);
		if (tq_som_ram_check_size(ram_sizes[i]))
			break;
	}

	if (i < ARRAY_SIZE(ram_sizes)) {
		debug("SPL: tqma7 ddr init done ...\n");
	} else {
		pr_err("Error: Invalid DDR RAM size\n");
		hang();
	}
}
