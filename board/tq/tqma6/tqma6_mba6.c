// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * ported SabreSD to TQMa6x
 * Copyright (c) 2013-2014 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <env.h>
#include <spl.h>

#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>

#include "../common/tq_bb.h"

#define UART_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x02e0790
#define TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM		0x02e07ac

#define TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x02e0768
#define TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM	0x02e0788

/* disable on die termination for RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE	0x00000000
/* optimised drive strength for 1.0 .. 1.3 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P2V	0x00080000
/* optimised drive strength for 1.3 .. 2.5 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V	0x000C0000

static const iomux_v3_cfg_t mba6_uart2_pads[] = {
	MX6_PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void mba6_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6_uart2_pads,
					 ARRAY_SIZE(mba6_uart2_pads));
}

int tq_bb_board_early_init_f(void)
{
	/* iomux and setup of uart */
	mba6_setup_iomuxc_uart();

	if (IS_ENABLED(CONFIG_SPL_BUILD))
		preloader_console_init();

	return 0;
}

#if !IS_ENABLED(CONFIG_SPL_BUILD)

static void mba6_setup_iomuxc_enet(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* clear gpr1[ENET_CLK_SEL] for externel clock */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	if (is_mx6sdl()) {
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE,
			     (void *)TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
			     (void *)TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);
	} else if (is_mx6dq()) {
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE,
			     (void *)TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
			     (void *)TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);
	}
}

int board_mmc_get_env_dev(int devno)
{
	/*
	 * This assumes that the baseboard registered
	 * the boot device first ...
	 * Note: SDHC3 == idx2
	 */
	return (devno == 2) ? 0 : 1;
}

int tq_bb_board_init(void)
{
	mba6_setup_iomuxc_enet();

	return 0;
}

const char *tq_bb_get_boardname(void)
{
	return "MBa6x";
}

int tq_bb_board_late_init(void)
{
	board_late_mmc_env_init();
	return 0;
}

#endif /* !IS_ENABLED(CONFIG_SPL_BUILD) */
