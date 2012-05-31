/*
 * Porting to u-boot:
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * MX51 Linux framebuffer:
 *
 * (C) Copyright 2004-2010 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

/* #define DEBUG */
#include <common.h>
#include <asm/errno.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/io.h>
#include <malloc.h>
#include <lcd.h>
#include "videomodes.h"
#include "ipu.h"
#include "mxcfb.h"

DECLARE_GLOBAL_DATA_PTR;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;	/* Start of console buffer	*/

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

short console_col;
short console_row;

vidinfo_t panel_info;

static int mxcfb_map_video_memory(struct fb_info *fbi);
static int mxcfb_unmap_video_memory(struct fb_info *fbi);

void lcd_initcolregs(void)
{
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
}

void lcd_disable(void)
{
}

void lcd_panel_disable(void)
{
}

void fb_videomode_to_var(struct fb_var_screeninfo *var,
			 const struct fb_videomode *mode)
{
	var->xres = mode->xres;
	var->yres = mode->yres;
	var->xres_virtual = mode->xres;
	var->yres_virtual = mode->yres;
	var->xoffset = 0;
	var->yoffset = 0;
	var->pixclock = mode->pixclock;
	var->left_margin = mode->left_margin;
	var->right_margin = mode->right_margin;
	var->upper_margin = mode->upper_margin;
	var->lower_margin = mode->lower_margin;
	var->hsync_len = mode->hsync_len;
	var->vsync_len = mode->vsync_len;
	var->sync = mode->sync;
	var->vmode = mode->vmode & FB_VMODE_MASK;
}

/*
 * Structure containing the MXC specific framebuffer information.
 */
struct mxcfb_info {
	int blank;
	ipu_channel_t ipu_ch;
	int ipu_di;
	u32 ipu_di_pix_fmt;
	unsigned char overlay;
	unsigned char alpha_chan_en;
	dma_addr_t alpha_phy_addr0;
	dma_addr_t alpha_phy_addr1;
	void *alpha_virt_addr0;
	void *alpha_virt_addr1;
	uint32_t alpha_mem_len;
	uint32_t cur_ipu_buf;
	uint32_t cur_ipu_alpha_buf;

	u32 pseudo_palette[16];
};

enum {
	BOTH_ON,
	SRC_ON,
	TGT_ON,
	BOTH_OFF
};

static unsigned long default_bpp = 16;
static unsigned char g_dp_in_use;
static struct fb_info *mxcfb_info[3];
static int ext_clk_used;

static uint32_t bpp_to_pixfmt(struct fb_info *fbi)
{
	uint32_t pixfmt = 0;

	debug("bpp_to_pixfmt: %d\n", fbi->var.bits_per_pixel);

	if (fbi->var.nonstd)
		return fbi->var.nonstd;

	switch (fbi->var.bits_per_pixel) {
	case 24:
		pixfmt = IPU_PIX_FMT_BGR24;
		break;
	case 32:
		pixfmt = IPU_PIX_FMT_BGR32;
		break;
	case 16:
		pixfmt = IPU_PIX_FMT_RGB565;
		break;
	}
	return pixfmt;
}

/*
 * Set fixed framebuffer parameters based on variable settings.
 *
 * @param       info     framebuffer information pointer
 */
static int mxcfb_set_fix(struct fb_info *info)
{
	struct fb_fix_screeninfo *fix = &info->fix;
	struct fb_var_screeninfo *var = &info->var;

	fix->line_length = var->xres_virtual * var->bits_per_pixel / 8;

	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->accel = FB_ACCEL_NONE;
	fix->visual = FB_VISUAL_TRUECOLOR;
	fix->xpanstep = 1;
	fix->ypanstep = 1;

	return 0;
}

static int setup_disp_channel1(struct fb_info *fbi)
{
	ipu_channel_params_t params;
	struct mxcfb_info *mxc_fbi = (struct mxcfb_info *)fbi->par;

	memset(&params, 0, sizeof(params));
	params.mem_dp_bg_sync.di = mxc_fbi->ipu_di;

	debug("%s called\n", __func__);
	/*
	 * Assuming interlaced means yuv output, below setting also
	 * valid for mem_dc_sync. FG should have the same vmode as BG.
	 */
	if (fbi->var.vmode & FB_VMODE_INTERLACED) {
		params.mem_dp_bg_sync.interlaced = 1;
		params.mem_dp_bg_sync.out_pixel_fmt =
			IPU_PIX_FMT_YUV444;
	} else {
		if (mxc_fbi->ipu_di_pix_fmt) {
			params.mem_dp_bg_sync.out_pixel_fmt =
				mxc_fbi->ipu_di_pix_fmt;
		} else {
			params.mem_dp_bg_sync.out_pixel_fmt =
				IPU_PIX_FMT_RGB666;
		}
	}
	params.mem_dp_bg_sync.in_pixel_fmt = bpp_to_pixfmt(fbi);
	if (mxc_fbi->alpha_chan_en)
		params.mem_dp_bg_sync.alpha_chan_en = 1;

	ipu_init_channel(mxc_fbi->ipu_ch, &params);

	return 0;
}

static int setup_disp_channel2(struct fb_info *fbi)
{
	int retval = 0;
	struct mxcfb_info *mxc_fbi = (struct mxcfb_info *)fbi->par;

	mxc_fbi->cur_ipu_buf = 1;
	if (mxc_fbi->alpha_chan_en)
		mxc_fbi->cur_ipu_alpha_buf = 1;

	fbi->var.xoffset = fbi->var.yoffset = 0;

	debug("%s: %x %d %d %d %lx %lx\n",
		__func__,
		mxc_fbi->ipu_ch,
		fbi->var.xres,
		fbi->var.yres,
		fbi->fix.line_length,
		fbi->fix.smem_start,
		fbi->fix.smem_start +
		(fbi->fix.line_length * fbi->var.yres));

	retval = ipu_init_channel_buffer(mxc_fbi->ipu_ch, IPU_INPUT_BUFFER,
					 bpp_to_pixfmt(fbi),
					 fbi->var.xres, fbi->var.yres,
					 fbi->fix.line_length,
					 fbi->fix.smem_start +
					 (fbi->fix.line_length * fbi->var.yres),
					 fbi->fix.smem_start,
					 0, 0);
	if (retval)
		printf("ipu_init_channel_buffer error %d\n", retval);

	return retval;
}

/*
 * Set framebuffer parameters and change the operating mode.
 *
 * @param       info     framebuffer information pointer
 */
static int mxcfb_set_par(struct fb_info *fbi)
{
	int retval = 0;
	u32 mem_len;
	ipu_di_signal_cfg_t sig_cfg;
	struct mxcfb_info *mxc_fbi = (struct mxcfb_info *)fbi->par;
	uint32_t out_pixel_fmt;

	ipu_disable_channel(mxc_fbi->ipu_ch);
	ipu_uninit_channel(mxc_fbi->ipu_ch);
	mxcfb_set_fix(fbi);

	mem_len = fbi->var.yres_virtual * fbi->fix.line_length;
	if (!fbi->fix.smem_start || (mem_len > fbi->fix.smem_len)) {
		if (fbi->fix.smem_start)
			mxcfb_unmap_video_memory(fbi);

		if (mxcfb_map_video_memory(fbi) < 0)
			return -ENOMEM;
	}

	setup_disp_channel1(fbi);

	memset(&sig_cfg, 0, sizeof(sig_cfg));
	if (fbi->var.vmode & FB_VMODE_INTERLACED) {
		sig_cfg.interlaced = 1;
		out_pixel_fmt = IPU_PIX_FMT_YUV444;
	} else {
		if (mxc_fbi->ipu_di_pix_fmt)
			out_pixel_fmt = mxc_fbi->ipu_di_pix_fmt;
		else
			out_pixel_fmt = IPU_PIX_FMT_RGB666;
	}
	if (fbi->var.vmode & FB_VMODE_ODD_FLD_FIRST) /* PAL */
		sig_cfg.odd_field_first = 1;
	if ((fbi->var.sync & FB_SYNC_EXT) || ext_clk_used)
		sig_cfg.ext_clk = 1;
	if (fbi->var.sync & FB_SYNC_HOR_HIGH_ACT)
		sig_cfg.Hsync_pol = 1;
	if (fbi->var.sync & FB_SYNC_VERT_HIGH_ACT)
		sig_cfg.Vsync_pol = 1;
	if (!(fbi->var.sync & FB_SYNC_CLK_LAT_FALL))
		sig_cfg.clk_pol = 1;
	if (fbi->var.sync & FB_SYNC_DATA_INVERT)
		sig_cfg.data_pol = 1;
	if (!(fbi->var.sync & FB_SYNC_OE_LOW_ACT))
		sig_cfg.enable_pol = 1;
	if (fbi->var.sync & FB_SYNC_CLK_IDLE_EN)
		sig_cfg.clkidle_en = 1;

	debug("pixclock = %ul Hz\n",
		(u32) (PICOS2KHZ(fbi->var.pixclock) * 1000UL));

	if (ipu_init_sync_panel(mxc_fbi->ipu_di,
				(PICOS2KHZ(fbi->var.pixclock)) * 1000UL,
				fbi->var.xres, fbi->var.yres,
				out_pixel_fmt,
				fbi->var.left_margin,
				fbi->var.hsync_len,
				fbi->var.right_margin,
				fbi->var.upper_margin,
				fbi->var.vsync_len,
				fbi->var.lower_margin,
				0, sig_cfg) != 0) {
		puts("mxcfb: Error initializing panel.\n");
		return -EINVAL;
	}

	retval = setup_disp_channel2(fbi);
	if (retval)
		return retval;

	if (mxc_fbi->blank == FB_BLANK_UNBLANK)
		ipu_enable_channel(mxc_fbi->ipu_ch);

	return retval;
}

/*
 * Check framebuffer variable parameters and adjust to valid values.
 *
 * @param       var      framebuffer variable parameters
 *
 * @param       info     framebuffer information pointer
 */
static int mxcfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	u32 vtotal;
	u32 htotal;

	if (var->xres_virtual < var->xres)
		var->xres_virtual = var->xres;
	if (var->yres_virtual < var->yres)
		var->yres_virtual = var->yres;

	if ((var->bits_per_pixel != 32) && (var->bits_per_pixel != 24) &&
	    (var->bits_per_pixel != 16) && (var->bits_per_pixel != 8))
		var->bits_per_pixel = default_bpp;

	switch (var->bits_per_pixel) {
	case 8:
		var->red.length = 3;
		var->red.offset = 5;
		var->red.msb_right = 0;

		var->green.length = 3;
		var->green.offset = 2;
		var->green.msb_right = 0;

		var->blue.length = 2;
		var->blue.offset = 0;
		var->blue.msb_right = 0;

		var->transp.length = 0;
		var->transp.offset = 0;
		var->transp.msb_right = 0;
		break;
	case 16:
		var->red.length = 5;
		var->red.offset = 11;
		var->red.msb_right = 0;

		var->green.length = 6;
		var->green.offset = 5;
		var->green.msb_right = 0;

		var->blue.length = 5;
		var->blue.offset = 0;
		var->blue.msb_right = 0;

		var->transp.length = 0;
		var->transp.offset = 0;
		var->transp.msb_right = 0;
		break;
	case 24:
		var->red.length = 8;
		var->red.offset = 16;
		var->red.msb_right = 0;

		var->green.length = 8;
		var->green.offset = 8;
		var->green.msb_right = 0;

		var->blue.length = 8;
		var->blue.offset = 0;
		var->blue.msb_right = 0;

		var->transp.length = 0;
		var->transp.offset = 0;
		var->transp.msb_right = 0;
		break;
	case 32:
		var->red.length = 8;
		var->red.offset = 16;
		var->red.msb_right = 0;

		var->green.length = 8;
		var->green.offset = 8;
		var->green.msb_right = 0;

		var->blue.length = 8;
		var->blue.offset = 0;
		var->blue.msb_right = 0;

		var->transp.length = 8;
		var->transp.offset = 24;
		var->transp.msb_right = 0;
		break;
	}

	if (var->pixclock < 1000) {
		htotal = var->xres + var->right_margin + var->hsync_len +
		    var->left_margin;
		vtotal = var->yres + var->lower_margin + var->vsync_len +
		    var->upper_margin;
		var->pixclock = (vtotal * htotal * 6UL) / 100UL;
		var->pixclock = KHZ2PICOS(var->pixclock);
		printf("pixclock set for 60Hz refresh = %u ps\n",
			var->pixclock);
	}

	var->height = -1;
	var->width = -1;
	var->grayscale = 0;

	return 0;
}

static int mxcfb_map_video_memory(struct fb_info *fbi)
{
	if (fbi->fix.smem_len < fbi->var.yres_virtual * fbi->fix.line_length) {
		fbi->fix.smem_len = fbi->var.yres_virtual *
				    fbi->fix.line_length;
	}

	fbi->screen_base = (char *)lcd_base;
	fbi->fix.smem_start = (unsigned long)lcd_base;
	if (fbi->screen_base == 0) {
		puts("Unable to allocate framebuffer memory\n");
		fbi->fix.smem_len = 0;
		fbi->fix.smem_start = 0;
		return -EBUSY;
	}

	debug("allocated fb @ paddr=0x%08X, size=%d.\n",
		(uint32_t) fbi->fix.smem_start, fbi->fix.smem_len);

	fbi->screen_size = fbi->fix.smem_len;

	/* Clear the screen */
	memset((char *)fbi->screen_base, 0, fbi->fix.smem_len);

	return 0;
}

static int mxcfb_unmap_video_memory(struct fb_info *fbi)
{
	fbi->screen_base = 0;
	fbi->fix.smem_start = 0;
	fbi->fix.smem_len = 0;
	return 0;
}

/*
 * Initializes the framebuffer information pointer. After allocating
 * sufficient memory for the framebuffer structure, the fields are
 * filled with custom information passed in from the configurable
 * structures.  This includes information such as bits per pixel,
 * color maps, screen width/height and RGBA offsets.
 *
 * @return      Framebuffer structure initialized with our information
 */
static struct fb_info *mxcfb_init_fbinfo(void)
{
#define BYTES_PER_LONG 4
#define PADDING (BYTES_PER_LONG - (sizeof(struct fb_info) % BYTES_PER_LONG))
	struct fb_info *fbi;
	struct mxcfb_info *mxcfbi;
	char *p;
	int size = sizeof(struct mxcfb_info) + PADDING +
		sizeof(struct fb_info);

	debug("%s: %d %d %d %d\n",
		__func__,
		PADDING,
		size,
		sizeof(struct mxcfb_info),
		sizeof(struct fb_info));
	/*
	 * Allocate sufficient memory for the fb structure
	 */

	p = malloc(size);
	if (!p)
		return NULL;

	memset(p, 0, size);

	fbi = (struct fb_info *)p;
	fbi->par = p + sizeof(struct fb_info) + PADDING;

	mxcfbi = (struct mxcfb_info *)fbi->par;
	debug("Framebuffer structures at: fbi=0x%x mxcfbi=0x%x\n",
		(unsigned int)fbi, (unsigned int)mxcfbi);

	fbi->var.activate = FB_ACTIVATE_NOW;

	fbi->flags = FBINFO_FLAG_DEFAULT;
	fbi->pseudo_palette = mxcfbi->pseudo_palette;

	return fbi;
}

/*
 * Probe routine for the framebuffer driver. It is called during the
 * driver binding process.      The following functions are performed in
 * this routine: Framebuffer initialization, Memory allocation and
 * mapping, Framebuffer registration, IPU initialization.
 *
 * @return      Appropriate error code to the kernel common code
 */
static int mxcfb_probe(u32 interface_pix_fmt, struct fb_videomode *mode)
{
	struct fb_info *fbi;
	struct mxcfb_info *mxcfbi;
	int ret = 0;

	/*
	 * Initialize FB structures
	 */
	fbi = mxcfb_init_fbinfo();
	if (!fbi) {
		ret = -ENOMEM;
		goto err0;
	}
	mxcfbi = (struct mxcfb_info *)fbi->par;

	if (!g_dp_in_use) {
		mxcfbi->ipu_ch = MEM_BG_SYNC;
		mxcfbi->blank = FB_BLANK_UNBLANK;
	} else {
		mxcfbi->ipu_ch = MEM_DC_SYNC;
		mxcfbi->blank = FB_BLANK_POWERDOWN;
	}

	mxcfbi->ipu_di = 0;

	ipu_disp_set_global_alpha(mxcfbi->ipu_ch, 1, 0x80);
	ipu_disp_set_color_key(mxcfbi->ipu_ch, 0, 0);
	strcpy(fbi->fix.id, "DISP3 BG");

	g_dp_in_use = 1;

	mxcfb_info[mxcfbi->ipu_di] = fbi;

	/* Need dummy values until real panel is configured */
	fbi->var.xres = 640;
	fbi->var.yres = 480;
	fbi->var.bits_per_pixel = 16;

	mxcfbi->ipu_di_pix_fmt = interface_pix_fmt;
	fb_videomode_to_var(&fbi->var, mode);

	mxcfb_check_var(&fbi->var, fbi);

	/* Default Y virtual size is 2x panel size */
	fbi->var.yres_virtual = fbi->var.yres * 2;

	mxcfb_set_fix(fbi);

	/* alocate fb first */
	if (mxcfb_map_video_memory(fbi) < 0)
		return -ENOMEM;

	mxcfb_set_par(fbi);

	/* Setting panel_info for lcd */
	panel_info.cmap = NULL;
	panel_info.vl_col = fbi->var.xres;
	panel_info.vl_row = fbi->var.yres;
	panel_info.vl_bpix = LCD_BPP;

	lcd_line_length = (panel_info.vl_col * NBITS(panel_info.vl_bpix)) / 8;

	debug("MXC IPUV3 configured\n"
		"XRES = %d YRES = %d BitsXpixel = %d\n",
		panel_info.vl_col,
		panel_info.vl_row,
		panel_info.vl_bpix);

	ipu_dump_registers();

	return 0;

err0:
	return ret;
}

int overwrite_console(void)
{
	/* Keep stdout / stderr on serial, our LCD is for splashscreen only */
	return 1;
}

void lcd_ctrl_init(void *lcdbase)
{
	u32 mem_len = panel_info.vl_col *
		panel_info.vl_row *
		NBITS(panel_info.vl_bpix) / 8;

	/*
	 * We rely on lcdbase being a physical address, i.e., either MMU off,
	 * or 1-to-1 mapping. Might want to add some virt2phys here.
	 */
	if (!lcdbase)
		return;

	memset(lcdbase, 0, mem_len);
}

int mx51_fb_init(struct fb_videomode *mode)
{
	int ret;

	ret = ipu_probe();
	if (ret)
		puts("Error initializing IPU\n");

	lcd_base += 56;

	debug("Framebuffer at 0x%x\n", (unsigned int)lcd_base);
	ret = mxcfb_probe(IPU_PIX_FMT_RGB666, mode);

	return ret;
}
