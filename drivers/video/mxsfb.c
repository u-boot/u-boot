/*
 * Freescale i.MX23/i.MX28 LCDIF driver
 *
 * Copyright (C) 2011-2013 Marek Vasut <marex@denx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <malloc.h>
#include <video_fb.h>

#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/errno.h>
#include <asm/io.h>

#include "videomodes.h"

#define	PS2KHZ(ps)	(1000000000UL / (ps))

static GraphicDevice panel;

/*
 * DENX M28EVK:
 * setenv videomode
 * video=ctfb:x:800,y:480,depth:18,mode:0,pclk:30066,
 *       le:0,ri:256,up:0,lo:45,hs:1,vs:1,sync:100663296,vmode:0
 *
 * Freescale mx23evk/mx28evk with a Seiko 4.3'' WVGA panel:
 * setenv videomode
 * video=ctfb:x:800,y:480,depth:24,mode:0,pclk:29851,
 * 	 le:89,ri:164,up:23,lo:10,hs:10,vs:10,sync:0,vmode:0
 */

static void mxs_lcd_init(GraphicDevice *panel,
			struct ctfb_res_modes *mode, int bpp)
{
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;
	uint32_t word_len = 0, bus_width = 0;
	uint8_t valid_data = 0;

	/* Kick in the LCDIF clock */
	mxs_set_lcdclk(PS2KHZ(mode->pixclock));

	/* Restart the LCDIF block */
	mxs_reset_block(&regs->hw_lcdif_ctrl_reg);

	switch (bpp) {
	case 24:
		word_len = LCDIF_CTRL_WORD_LENGTH_24BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_24BIT;
		valid_data = 0x7;
		break;
	case 18:
		word_len = LCDIF_CTRL_WORD_LENGTH_24BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_18BIT;
		valid_data = 0x7;
		break;
	case 16:
		word_len = LCDIF_CTRL_WORD_LENGTH_16BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_16BIT;
		valid_data = 0xf;
		break;
	case 8:
		word_len = LCDIF_CTRL_WORD_LENGTH_8BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_8BIT;
		valid_data = 0xf;
		break;
	}

	writel(bus_width | word_len | LCDIF_CTRL_DOTCLK_MODE |
		LCDIF_CTRL_BYPASS_COUNT | LCDIF_CTRL_LCDIF_MASTER,
		&regs->hw_lcdif_ctrl);

	writel(valid_data << LCDIF_CTRL1_BYTE_PACKING_FORMAT_OFFSET,
		&regs->hw_lcdif_ctrl1);
	writel((mode->yres << LCDIF_TRANSFER_COUNT_V_COUNT_OFFSET) | mode->xres,
		&regs->hw_lcdif_transfer_count);

	writel(LCDIF_VDCTRL0_ENABLE_PRESENT | LCDIF_VDCTRL0_ENABLE_POL |
		LCDIF_VDCTRL0_VSYNC_PERIOD_UNIT |
		LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_UNIT |
		mode->vsync_len, &regs->hw_lcdif_vdctrl0);
	writel(mode->upper_margin + mode->lower_margin +
		mode->vsync_len + mode->yres,
		&regs->hw_lcdif_vdctrl1);
	writel((mode->hsync_len << LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH_OFFSET) |
		(mode->left_margin + mode->right_margin +
		mode->hsync_len + mode->xres),
		&regs->hw_lcdif_vdctrl2);
	writel(((mode->left_margin + mode->hsync_len) <<
		LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT_OFFSET) |
		(mode->upper_margin + mode->vsync_len),
		&regs->hw_lcdif_vdctrl3);
	writel((0 << LCDIF_VDCTRL4_DOTCLK_DLY_SEL_OFFSET) | mode->xres,
		&regs->hw_lcdif_vdctrl4);

	writel(panel->frameAdrs, &regs->hw_lcdif_cur_buf);
	writel(panel->frameAdrs, &regs->hw_lcdif_next_buf);

	/* Flush FIFO first */
	writel(LCDIF_CTRL1_FIFO_CLEAR, &regs->hw_lcdif_ctrl1_set);

	/* Sync signals ON */
	setbits_le32(&regs->hw_lcdif_vdctrl4, LCDIF_VDCTRL4_SYNC_SIGNALS_ON);

	/* FIFO cleared */
	writel(LCDIF_CTRL1_FIFO_CLEAR, &regs->hw_lcdif_ctrl1_clr);

	/* RUN! */
	writel(LCDIF_CTRL_RUN, &regs->hw_lcdif_ctrl_set);
}

void *video_hw_init(void)
{
	int bpp = -1;
	char *penv;
	void *fb;
	struct ctfb_res_modes mode;

	puts("Video: ");

	/* Suck display configuration from "videomode" variable */
	penv = getenv("videomode");
	if (!penv) {
		puts("MXSFB: 'videomode' variable not set!\n");
		return NULL;
	}

	bpp = video_get_params(&mode, penv);

	/* fill in Graphic device struct */
	sprintf(panel.modeIdent, "%dx%dx%d",
			mode.xres, mode.yres, bpp);

	panel.winSizeX = mode.xres;
	panel.winSizeY = mode.yres;
	panel.plnSizeX = mode.xres;
	panel.plnSizeY = mode.yres;

	switch (bpp) {
	case 24:
	case 18:
		panel.gdfBytesPP = 4;
		panel.gdfIndex = GDF_32BIT_X888RGB;
		break;
	case 16:
		panel.gdfBytesPP = 2;
		panel.gdfIndex = GDF_16BIT_565RGB;
		break;
	case 8:
		panel.gdfBytesPP = 1;
		panel.gdfIndex = GDF__8BIT_INDEX;
		break;
	default:
		printf("MXSFB: Invalid BPP specified! (bpp = %i)\n", bpp);
		return NULL;
	}

	panel.memSize = mode.xres * mode.yres * panel.gdfBytesPP;

	/* Allocate framebuffer */
	fb = malloc(panel.memSize);
	if (!fb) {
		printf("MXSFB: Error allocating framebuffer!\n");
		return NULL;
	}

	/* Wipe framebuffer */
	memset(fb, 0, panel.memSize);

	panel.frameAdrs = (u32)fb;

	printf("%s\n", panel.modeIdent);

	/* Start framebuffer */
	mxs_lcd_init(&panel, &mode, bpp);

	return (void *)&panel;
}
