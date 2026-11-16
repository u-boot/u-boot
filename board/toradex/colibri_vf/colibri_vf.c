// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2019 Toradex, Inc.
 *
 * Based on vf610twr.c:
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <init.h>
#include <asm/global_data.h>

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <env.h>
#include <fdt_support.h>
#include <g_dnl.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <usb.h>

#include "../common/tdx-common.h"

DECLARE_GLOBAL_DATA_PTR;

#define PTC0_GPIO_45		45

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

#ifdef CONFIG_VYBRID_GPIO
static void setup_iomux_gpio(void)
{
	static const iomux_v3_cfg_t gpio_pads[] = {
		VF610_PAD_PTA17__GPIO_7,
		VF610_PAD_PTA20__GPIO_10,
		VF610_PAD_PTA21__GPIO_11,
		VF610_PAD_PTA30__GPIO_20,
		VF610_PAD_PTA31__GPIO_21,
		VF610_PAD_PTB0__GPIO_22,
		VF610_PAD_PTB1__GPIO_23,
		VF610_PAD_PTB6__GPIO_28,
		VF610_PAD_PTB7__GPIO_29,
		VF610_PAD_PTB8__GPIO_30,
		VF610_PAD_PTB9__GPIO_31,
		VF610_PAD_PTB12__GPIO_34,
		VF610_PAD_PTB13__GPIO_35,
		VF610_PAD_PTB16__GPIO_38,
		VF610_PAD_PTB17__GPIO_39,
		VF610_PAD_PTB18__GPIO_40,
		VF610_PAD_PTB21__GPIO_43,
		VF610_PAD_PTB22__GPIO_44,
		VF610_PAD_PTC0__GPIO_45,
		VF610_PAD_PTC1__GPIO_46,
		VF610_PAD_PTC2__GPIO_47,
		VF610_PAD_PTC3__GPIO_48,
		VF610_PAD_PTC4__GPIO_49,
		VF610_PAD_PTC5__GPIO_50,
		VF610_PAD_PTC6__GPIO_51,
		VF610_PAD_PTC7__GPIO_52,
		VF610_PAD_PTC8__GPIO_53,
		VF610_PAD_PTD31__GPIO_63,
		VF610_PAD_PTD30__GPIO_64,
		VF610_PAD_PTD29__GPIO_65,
		VF610_PAD_PTD28__GPIO_66,
		VF610_PAD_PTD27__GPIO_67,
		VF610_PAD_PTD26__GPIO_68,
		VF610_PAD_PTD25__GPIO_69,
		VF610_PAD_PTD24__GPIO_70,
		VF610_PAD_PTD9__GPIO_88,
		VF610_PAD_PTD10__GPIO_89,
		VF610_PAD_PTD11__GPIO_90,
		VF610_PAD_PTD12__GPIO_91,
		VF610_PAD_PTD13__GPIO_92,
		VF610_PAD_PTB23__GPIO_93,
		VF610_PAD_PTB26__GPIO_96,
		VF610_PAD_PTB28__GPIO_98,
		VF610_PAD_PTC30__GPIO_103,
		VF610_PAD_PTA7__GPIO_134,
	};

	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}
#endif

static inline int is_colibri_vf61(void)
{
	struct mscm *mscm = (struct mscm *)MSCM_BASE_ADDR;

	/*
	 * Detect board type by Level 2 Cache: VF50 don't have any
	 * Level 2 Cache.
	 */
	return !!mscm->cpxcfg1;
}

static void clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;
	u32 pfd_clk_sel, ddr_clk_sel;

	clrsetbits_le32(&ccm->ccgr0, CCM_REG_CTRL_MASK,
			CCM_CCGR0_UART0_CTRL_MASK);
#ifdef CONFIG_FSL_DSPI
	setbits_le32(&ccm->ccgr0, CCM_CCGR0_DSPI1_CTRL_MASK);
#endif
	clrsetbits_le32(&ccm->ccgr1, CCM_REG_CTRL_MASK,
			CCM_CCGR1_PIT_CTRL_MASK | CCM_CCGR1_WDOGA5_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr2, CCM_REG_CTRL_MASK,
			CCM_CCGR2_IOMUXC_CTRL_MASK | CCM_CCGR2_PORTA_CTRL_MASK |
			CCM_CCGR2_PORTB_CTRL_MASK | CCM_CCGR2_PORTC_CTRL_MASK |
			CCM_CCGR2_PORTD_CTRL_MASK | CCM_CCGR2_PORTE_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr3, CCM_REG_CTRL_MASK,
			CCM_CCGR3_ANADIG_CTRL_MASK | CCM_CCGR3_SCSC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr4, CCM_REG_CTRL_MASK,
			CCM_CCGR4_WKUP_CTRL_MASK | CCM_CCGR4_CCM_CTRL_MASK |
			CCM_CCGR4_GPC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr6, CCM_REG_CTRL_MASK,
			CCM_CCGR6_OCOTP_CTRL_MASK | CCM_CCGR6_DDRMC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr7, CCM_REG_CTRL_MASK,
			CCM_CCGR7_SDHC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr9, CCM_REG_CTRL_MASK,
			CCM_CCGR9_FEC0_CTRL_MASK | CCM_CCGR9_FEC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr10, CCM_REG_CTRL_MASK,
			CCM_CCGR10_NFC_CTRL_MASK);

#ifdef CONFIG_USB_EHCI_VF
	setbits_le32(&ccm->ccgr1, CCM_CCGR1_USBC0_CTRL_MASK);
	setbits_le32(&ccm->ccgr7, CCM_CCGR7_USBC1_CTRL_MASK);

	clrsetbits_le32(&anadig->pll3_ctrl, ANADIG_PLL3_CTRL_BYPASS |
			ANADIG_PLL3_CTRL_POWERDOWN |
			ANADIG_PLL3_CTRL_DIV_SELECT,
			ANADIG_PLL3_CTRL_ENABLE);
	clrsetbits_le32(&anadig->pll7_ctrl, ANADIG_PLL7_CTRL_BYPASS |
			ANADIG_PLL7_CTRL_POWERDOWN |
			ANADIG_PLL7_CTRL_DIV_SELECT,
			ANADIG_PLL7_CTRL_ENABLE);
#endif

	clrsetbits_le32(&anadig->pll5_ctrl, ANADIG_PLL5_CTRL_BYPASS |
			ANADIG_PLL5_CTRL_POWERDOWN, ANADIG_PLL5_CTRL_ENABLE |
			ANADIG_PLL5_CTRL_DIV_SELECT);

	clrsetbits_le32(&anadig->pll2_ctrl, ANADIG_PLL5_CTRL_BYPASS |
			ANADIG_PLL2_CTRL_POWERDOWN,
			ANADIG_PLL2_CTRL_ENABLE |
			ANADIG_PLL2_CTRL_DIV_SELECT);

	clrsetbits_le32(&anadig->pll1_ctrl, ANADIG_PLL1_CTRL_POWERDOWN,
			ANADIG_PLL1_CTRL_ENABLE | ANADIG_PLL1_CTRL_DIV_SELECT);

	clrsetbits_le32(&ccm->ccr, CCM_CCR_OSCNT_MASK,
			CCM_CCR_FIRC_EN | CCM_CCR_OSCNT(5));

	/* See "Typical PLL Configuration" */
	if (is_colibri_vf61())
		pfd_clk_sel = CCM_CCSR_PLL1_PFD_CLK_SEL(1);
	else
		pfd_clk_sel = CCM_CCSR_PLL1_PFD_CLK_SEL(3);
	ddr_clk_sel = CCM_CCSR_DDRC_CLK_SEL(0);

	clrsetbits_le32(&ccm->ccsr, CCM_REG_CTRL_MASK, pfd_clk_sel |
			CCM_CCSR_PLL2_PFD4_EN | CCM_CCSR_PLL2_PFD3_EN |
			CCM_CCSR_PLL2_PFD2_EN | CCM_CCSR_PLL2_PFD1_EN |
			CCM_CCSR_PLL1_PFD4_EN | CCM_CCSR_PLL1_PFD3_EN |
			CCM_CCSR_PLL1_PFD2_EN | CCM_CCSR_PLL1_PFD1_EN |
			ddr_clk_sel | CCM_CCSR_FAST_CLK_SEL(1) |
			CCM_CCSR_SYS_CLK_SEL(4));

	clrsetbits_le32(&ccm->cacrr, CCM_REG_CTRL_MASK,
			CCM_CACRR_IPG_CLK_DIV(1) | CCM_CACRR_BUS_CLK_DIV(2) |
			CCM_CACRR_ARM_CLK_DIV(0));
	clrsetbits_le32(&ccm->cscmr1, CCM_REG_CTRL_MASK,
			CCM_CSCMR1_ESDHC1_CLK_SEL(3) |
			CCM_CSCMR1_NFC_CLK_SEL(0));
	clrsetbits_le32(&ccm->cscdr1, CCM_REG_CTRL_MASK,
			CCM_CSCDR1_RMII_CLK_EN);
	clrsetbits_le32(&ccm->cscdr2, CCM_REG_CTRL_MASK,
			CCM_CSCDR2_ESDHC1_EN | CCM_CSCDR2_ESDHC1_CLK_DIV(0) |
			CCM_CSCDR2_NFC_EN);
	clrsetbits_le32(&ccm->cscdr3, CCM_REG_CTRL_MASK,
			CCM_CSCDR3_NFC_PRE_DIV(3));
	clrsetbits_le32(&ccm->cscmr2, CCM_REG_CTRL_MASK,
			CCM_CSCMR2_RMII_CLK_SEL(2));
}

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_IR_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CP0_EN, &mscmir->irsprc[i]);
}

int board_early_init_f(void)
{
	clock_init();
	mscm_init();

#ifdef CONFIG_VYBRID_GPIO
	setup_iomux_gpio();
#endif

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct src *src = (struct src *)SRC_BASE_ADDR;

	if (((src->sbmr2 & SRC_SBMR2_BMOD_MASK) >> SRC_SBMR2_BMOD_SHIFT)
			== SRC_SBMR2_BMOD_SERIAL) {
		printf("Serial Downloader recovery mode, disable autoboot\n");
		env_set("bootdelay", "-1");
	}

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

int board_init(void)
{
	struct scsc_reg *scsc = (struct scsc_reg *)SCSC_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/*
	 * Enable external 32K Oscillator
	 *
	 * The internal clock experiences significant drift
	 * so we must use the external oscillator in order
	 * to maintain correct time in the hwclock
	 */
	setbits_le32(&scsc->sosc_ctr, SCSC_SOSC_CTR_SOSC_EN);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	static const struct node_info nodes[] = {
		{ "fsl,vf610-nfc", MTD_DEV_TYPE_NAND, }, /* NAND flash */
	};

	/* Update partition nodes using info from mtdparts env var */
	puts("   Updating MTD partitions...\n");
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif

	return ft_common_board_setup(blob, bd);
}
#endif

/*
 * Backlight off before OS handover
 */
void board_preboot_os(void)
{
	gpio_request(PTC0_GPIO_45, "BL_ON");
	gpio_direction_output(PTC0_GPIO_45, 0);
}
