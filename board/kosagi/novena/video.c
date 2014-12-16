/*
 * Novena video output support
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/video.h>
#include <i2c.h>
#include <input.h>
#include <ipu_pixfmt.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <malloc.h>
#include <stdio_dev.h>

#include "novena.h"

static void enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

struct display_info_t const displays[] = {
	{
		/* HDMI Output */
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= detect_hdmi,
		.enable	= enable_hdmi,
		.mode	= {
			.name		= "HDMI",
			.refresh	= 60,
			.xres		= 1024,
			.yres		= 768,
			.pixclock	= 15384,
			.left_margin	= 220,
			.right_margin	= 40,
			.upper_margin	= 21,
			.lower_margin	= 7,
			.hsync_len	= 60,
			.vsync_len	= 10,
			.sync		= FB_SYNC_EXT,
			.vmode		= FB_VMODE_NONINTERLACED
		},
	},
};

size_t display_count = ARRAY_SIZE(displays);

void setup_display_clock(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB0,IPU,IPU DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK);

	/* set LDB0, LDB1 clk select to 011/011 */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK |
			MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK,
			(3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET) |
			(3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET));

	setbits_le32(&mxc_ccm->cscmr2, MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV);

	setbits_le32(&mxc_ccm->chsccdr, CHSCCDR_CLK_SEL_LDB_DI0 <<
		     MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);

	writel(IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES |
	       IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH |
	       IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW |
	       IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG |
	       IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT |
	       IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
	       IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT |
	       IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED |
	       IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0,
	       &iomux->gpr[2]);

	clrsetbits_le32(&iomux->gpr[3], IOMUXC_GPR3_LVDS0_MUX_CTL_MASK,
			IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
			IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
}
