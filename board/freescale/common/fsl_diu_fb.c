/*
 * Copyright 2007 Freescale Semiconductor, Inc.
 * York Sun <yorksun@freescale.com>
 *
 * FSL DIU Framebuffer driver
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

#include <common.h>
#include <i2c.h>
#include <malloc.h>

#ifdef CONFIG_FSL_DIU_FB

#include "fsl_diu_fb.h"

#ifdef DEBUG
#define DPRINTF(fmt, args...) printf("%s: " fmt,__FUNCTION__,## args)
#else
#define DPRINTF(fmt, args...)
#endif

struct fb_videomode {
	const char *name;	/* optional */
	unsigned int refresh;		/* optional */
	unsigned int xres;
	unsigned int yres;
	unsigned int pixclock;
	unsigned int left_margin;
	unsigned int right_margin;
	unsigned int upper_margin;
	unsigned int lower_margin;
	unsigned int hsync_len;
	unsigned int vsync_len;
	unsigned int sync;
	unsigned int vmode;
	unsigned int flag;
};

#define FB_SYNC_VERT_HIGH_ACT	2	/* vertical sync high active	*/
#define FB_SYNC_COMP_HIGH_ACT	8	/* composite sync high active   */
#define FB_VMODE_NONINTERLACED  0	/* non interlaced */

/*
 * These parameters give default parameters
 * for video output 1024x768,
 * FIXME - change timing to proper amounts
 * hsync 31.5kHz, vsync 60Hz
 */
static struct fb_videomode fsl_diu_mode_1024 = {
	.refresh	= 60,
	.xres		= 1024,
	.yres		= 768,
	.pixclock	= 15385,
	.left_margin	= 160,
	.right_margin	= 24,
	.upper_margin	= 29,
	.lower_margin	= 3,
	.hsync_len	= 136,
	.vsync_len	= 6,
	.sync		= FB_SYNC_COMP_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode		= FB_VMODE_NONINTERLACED
};

static struct fb_videomode fsl_diu_mode_1280 = {
	.name		= "1280x1024-60",
	.refresh	= 60,
	.xres		= 1280,
	.yres		= 1024,
	.pixclock	= 9375,
	.left_margin	= 38,
	.right_margin	= 128,
	.upper_margin	= 2,
	.lower_margin	= 7,
	.hsync_len	= 216,
	.vsync_len	= 37,
	.sync		= FB_SYNC_COMP_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode		= FB_VMODE_NONINTERLACED
};

/*
 * These are the fields of area descriptor(in DDR memory) for every plane
 */
struct diu_ad {
	/* Word 0(32-bit) in DDR memory */
	unsigned int pix_fmt; /* hard coding pixel format */
	/* Word 1(32-bit) in DDR memory */
	unsigned int addr;
	/* Word 2(32-bit) in DDR memory */
	unsigned int src_size_g_alpha;
	/* Word 3(32-bit) in DDR memory */
	unsigned int aoi_size;
	/* Word 4(32-bit) in DDR memory */
	unsigned int offset_xyi;
	/* Word 5(32-bit) in DDR memory */
	unsigned int offset_xyd;
	/* Word 6(32-bit) in DDR memory */
	unsigned int ckmax_r:8;
	unsigned int ckmax_g:8;
	unsigned int ckmax_b:8;
	unsigned int res9:8;
	/* Word 7(32-bit) in DDR memory */
	unsigned int ckmin_r:8;
	unsigned int ckmin_g:8;
	unsigned int ckmin_b:8;
	unsigned int res10:8;
	/* Word 8(32-bit) in DDR memory */
	unsigned int next_ad;
	/* Word 9(32-bit) in DDR memory, just for 64-bit aligned */
	unsigned int res1;
	unsigned int res2;
	unsigned int res3;
}__attribute__ ((packed));

/*
 * DIU register map
 */
struct diu {
	unsigned int desc[3];
	unsigned int gamma;
	unsigned int pallete;
	unsigned int cursor;
	unsigned int curs_pos;
	unsigned int diu_mode;
	unsigned int bgnd;
	unsigned int bgnd_wb;
	unsigned int disp_size;
	unsigned int wb_size;
	unsigned int wb_mem_addr;
	unsigned int hsyn_para;
	unsigned int vsyn_para;
	unsigned int syn_pol;
	unsigned int thresholds;
	unsigned int int_status;
	unsigned int int_mask;
	unsigned int colorbar[8];
	unsigned int filling;
	unsigned int plut;
} __attribute__ ((packed));

struct diu_hw {
	struct diu *diu_reg;
	volatile unsigned int mode;		/* DIU operation mode */
};

struct diu_addr {
	unsigned char  *  paddr;	/* Virtual address */
	unsigned int 	   offset;
};

#define FSL_DIU_BASE_OFFSET	0x2C000	/* Offset of Display Interface Unit */

/*
 * Modes of operation of DIU
 */
#define MFB_MODE0	0	/* DIU off */
#define MFB_MODE1	1	/* All three planes output to display */
#define MFB_MODE2	2	/* Plane 1 to display,
				 * planes 2+3 written back to memory */
#define MFB_MODE3	3	/* All three planes written back to memory */
#define MFB_MODE4	4	/* Color bar generation */

#define MAX_CURS		32

static struct fb_info fsl_fb_info;
static struct diu_addr gamma, cursor;
static struct diu_ad fsl_diu_fb_ad __attribute__ ((aligned(32)));
static struct diu_ad dummy_ad __attribute__ ((aligned(32)));
static unsigned char *dummy_fb;
static struct diu_hw dr = {
	.mode = MFB_MODE1,
};

int fb_enabled = 0;
int fb_initialized = 0;
const int default_xres = 1280;
const int default_pixel_format = 0x88882317;

static int map_video_memory(struct fb_info *info, unsigned long bytes_align);
static void enable_lcdc(void);
static void disable_lcdc(void);
static int fsl_diu_enable_panel(struct fb_info *info);
static int fsl_diu_disable_panel(struct fb_info *info);
static int allocate_buf(struct diu_addr *buf, u32 size, u32 bytes_align);
static u32 get_busfreq(void);

int fsl_diu_init(int xres,
		 unsigned int pixel_format,
		 int gamma_fix,
		 unsigned char *splash_bmp)
{
	struct fb_videomode *fsl_diu_mode_db;
	struct diu_ad *ad = &fsl_diu_fb_ad;
	struct diu *hw;
	struct fb_info *info = &fsl_fb_info;
	struct fb_var_screeninfo *var = &info->var;
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	volatile unsigned int *guts_clkdvdr = &gur->clkdvdr;
	unsigned char *gamma_table_base;
	unsigned int i, j;
	unsigned long speed_ccb, temp, pixval;

	DPRINTF("Enter fsl_diu_init\n");
	dr.diu_reg = (struct diu *) (CFG_IMMR + FSL_DIU_BASE_OFFSET);
	hw = (struct diu *) dr.diu_reg;

	disable_lcdc();

	if (xres == 1280) {
		fsl_diu_mode_db = &fsl_diu_mode_1280;
	} else {
		fsl_diu_mode_db = &fsl_diu_mode_1024;
	}

	if (0 == fb_initialized) {
		allocate_buf(&gamma, 768, 32);
		DPRINTF("gamma is allocated @ 0x%x\n",
			(unsigned int)gamma.paddr);
		allocate_buf(&cursor, MAX_CURS * MAX_CURS * 2, 32);
		DPRINTF("curosr is allocated @ 0x%x\n",
			(unsigned int)cursor.paddr);

		/* create a dummy fb and dummy ad */
		dummy_fb = malloc(64);
		if (NULL == dummy_fb) {
			printf("Cannot allocate dummy fb\n");
			return -1;
		}
		dummy_ad.addr = cpu_to_le32((unsigned int)dummy_fb);
		dummy_ad.pix_fmt = 0x88882317;
		dummy_ad.src_size_g_alpha = 0x04400000;	/* alpha = 0 */
		dummy_ad.aoi_size = 0x02000400;
		dummy_ad.offset_xyi = 0;
		dummy_ad.offset_xyd = 0;
		dummy_ad.next_ad = 0;
		/* Memory allocation for framebuffer */
		if (map_video_memory(info, 32)) {
			printf("Unable to allocate fb memory 1\n");
			return -1;
		}
	} else {
		memset(info->screen_base, 0, info->smem_len);
	}

	dr.diu_reg->desc[0] = (unsigned int) &dummy_ad;
	dr.diu_reg->desc[1] = (unsigned int) &dummy_ad;
	dr.diu_reg->desc[2] = (unsigned int) &dummy_ad;
	DPRINTF("dummy dr.diu_reg->desc[0] = 0x%x\n", dr.diu_reg->desc[0]);
	DPRINTF("dummy desc[0] = 0x%x\n", hw->desc[0]);

	/* read mode info */
	var->xres = fsl_diu_mode_db->xres;
	var->yres = fsl_diu_mode_db->yres;
	var->bits_per_pixel = 32;
	var->pixclock = fsl_diu_mode_db->pixclock;
	var->left_margin = fsl_diu_mode_db->left_margin;
	var->right_margin = fsl_diu_mode_db->right_margin;
	var->upper_margin = fsl_diu_mode_db->upper_margin;
	var->lower_margin = fsl_diu_mode_db->lower_margin;
	var->hsync_len = fsl_diu_mode_db->hsync_len;
	var->vsync_len = fsl_diu_mode_db->vsync_len;
	var->sync = fsl_diu_mode_db->sync;
	var->vmode = fsl_diu_mode_db->vmode;
	info->line_length = var->xres * var->bits_per_pixel / 8;
	info->logo_size = 0;
	info->logo_height = 0;

	ad->pix_fmt = pixel_format;
	ad->addr    = cpu_to_le32((unsigned int)info->screen_base);
	ad->src_size_g_alpha
			= cpu_to_le32((var->yres << 12) | var->xres);
	/* fix me. AOI should not be greater than display size */
	ad->aoi_size 	= cpu_to_le32(( var->yres << 16) |  var->xres);
	ad->offset_xyi = 0;
	ad->offset_xyd = 0;

	/* Disable chroma keying function */
	ad->ckmax_r = 0;
	ad->ckmax_g = 0;
	ad->ckmax_b = 0;

	ad->ckmin_r = 255;
	ad->ckmin_g = 255;
	ad->ckmin_b = 255;

	gamma_table_base = gamma.paddr;
	DPRINTF("gamma_table_base is allocated @ 0x%x\n",
		(unsigned int)gamma_table_base);

	/* Prep for DIU init  - gamma table */

	for (i = 0; i <= 2; i++)
		for (j = 0; j <= 255; j++)
			*gamma_table_base++ = j;

	if (gamma_fix == 1) {	/* fix the gamma */
		DPRINTF("Fix gamma table\n");
		gamma_table_base = gamma.paddr;
		for (i = 0; i < 256*3; i++) {
			gamma_table_base[i] = (gamma_table_base[i] << 2)
				| ((gamma_table_base[i] >> 6) & 0x03);
		}
	}

	DPRINTF("update-lcdc: HW - %p\n Disabling DIU\n", hw);

	/* Program DIU registers */

	hw->gamma = (unsigned int) gamma.paddr;
	hw->cursor= (unsigned int) cursor.paddr;
	hw->bgnd = 0x007F7F7F;				/* BGND */
	hw->bgnd_wb = 0; 				/* BGND_WB */
	hw->disp_size = var->yres << 16 | var->xres;	/* DISP SIZE */
	hw->wb_size = 0;				/* WB SIZE */
	hw->wb_mem_addr = 0;				/* WB MEM ADDR */
	hw->hsyn_para = var->left_margin << 22 |	/* BP_H */
			var->hsync_len << 11   |	/* PW_H */
			var->right_margin;		/* FP_H */
	hw->vsyn_para = var->upper_margin << 22 |	/* BP_V */
			var->vsync_len << 11    |	/* PW_V  */
			var->lower_margin;		/* FP_V  */

	/* Pixel Clock configuration */
	DPRINTF("DIU: Bus Frequency = %d\n", get_busfreq());
	speed_ccb = get_busfreq();

	DPRINTF("DIU pixclock in ps - %d\n", var->pixclock);
	temp = 1;
	temp *= 1000000000;
	temp /= var->pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;
	DPRINTF("DIU pixval = %lu\n", pixval);

	hw->syn_pol = 0;			/* SYNC SIGNALS POLARITY */
	hw->thresholds = 0x00037800;		/* The Thresholds */
	hw->int_status = 0;			/* INTERRUPT STATUS */
	hw->int_mask = 0;			/* INT MASK */
	hw->plut = 0x01F5F666;

	/* Modify PXCLK in GUTS CLKDVDR */
	DPRINTF("DIU: Current value of CLKDVDR = 0x%08x\n", *guts_clkdvdr);
	temp = *guts_clkdvdr & 0x2000FFFF;
	*guts_clkdvdr = temp;				/* turn off clock */
	*guts_clkdvdr = temp | 0x80000000 | ((pixval & 0x1F) << 16);
	DPRINTF("DIU: Modified value of CLKDVDR = 0x%08x\n", *guts_clkdvdr);

	fb_initialized = 1;

	if (splash_bmp) {
		info->logo_height = fsl_diu_display_bmp(splash_bmp, 0, 0, 0);
		info->logo_size = info->logo_height * info->line_length;
		DPRINTF("logo height %d, logo_size 0x%x\n",
			info->logo_height,info->logo_size);
	}

	/* Enable the DIU */
	fsl_diu_enable_panel(info);
	enable_lcdc();

	return 0;
}

char *fsl_fb_open(struct fb_info **info)
{
	*info = &fsl_fb_info;
	return (char *) ((unsigned int)(*info)->screen_base
			 + (*info)->logo_size);
}

void fsl_diu_close(void)
{
	struct fb_info *info = &fsl_fb_info;
	fsl_diu_disable_panel(info);
}

static int fsl_diu_enable_panel(struct fb_info *info)
{
	struct diu *hw = dr.diu_reg;
	struct diu_ad *ad = &fsl_diu_fb_ad;

	DPRINTF("Entered: enable_panel\n");
	if (hw->desc[0] != (unsigned int)ad)
		hw->desc[0] = (unsigned int)ad;
	DPRINTF("desc[0] = 0x%x\n", hw->desc[0]);
	return 0;
}

static int fsl_diu_disable_panel(struct fb_info *info)
{
	struct diu *hw = dr.diu_reg;

	DPRINTF("Entered: disable_panel\n");
	if (hw->desc[0] != (unsigned int)&dummy_ad)
		hw->desc[0] = (unsigned int)&dummy_ad;
	return 0;
}

static int map_video_memory(struct fb_info *info, unsigned long bytes_align)
{
	unsigned long offset;
	unsigned long mask;

	DPRINTF("Entered: map_video_memory\n");
	/* allocate maximum 1280*1024 with 32bpp */
	info->smem_len = 1280 * 4 *1024 + bytes_align;
	DPRINTF("MAP_VIDEO_MEMORY: smem_len = %d\n", info->smem_len);
	info->screen_base = malloc(info->smem_len);
	if (info->screen_base == NULL) {
		printf("Unable to allocate fb memory\n");
		return -1;
	}
	info->smem_start = (unsigned int) info->screen_base;
	mask = bytes_align - 1;
	offset = (unsigned long)info->screen_base & mask;
	if (offset) {
		info->screen_base += offset;
		info->smem_len = info->smem_len - (bytes_align - offset);
	} else
		info->smem_len = info->smem_len - bytes_align;

	info->screen_size = info->smem_len;

	DPRINTF("Allocated fb @ 0x%08lx, size=%d.\n",
		info->smem_start, info->smem_len);

	return 0;
}

static void enable_lcdc(void)
{
	struct diu *hw = dr.diu_reg;

	DPRINTF("Entered: enable_lcdc, fb_enabled = %d\n", fb_enabled);
	if (!fb_enabled) {
		hw->diu_mode = dr.mode;
		fb_enabled++;
	}
	DPRINTF("diu_mode = %d\n", hw->diu_mode);
}

static void disable_lcdc(void)
{
	struct diu *hw = dr.diu_reg;

	DPRINTF("Entered: disable_lcdc, fb_enabled = %d\n", fb_enabled);
	if (fb_enabled) {
		hw->diu_mode = 0;
		fb_enabled = 0;
	}
}

static u32 get_busfreq(void)
{
	u32 fs_busfreq = 0;

	fs_busfreq = get_bus_freq(0);
	return fs_busfreq;
}

/*
 * Align to 64-bit(8-byte), 32-byte, etc.
 */
static int allocate_buf(struct diu_addr *buf, u32 size, u32 bytes_align)
{
	u32 offset, ssize;
	u32 mask;

	DPRINTF("Entered: allocate_buf\n");
	ssize = size + bytes_align;
	buf->paddr = malloc(ssize);
	if (!buf->paddr)
		return -1;

	memset(buf->paddr, 0, ssize);
	mask = bytes_align - 1;
	offset = (u32)buf->paddr & mask;
	if (offset) {
		buf->offset = bytes_align - offset;
		buf->paddr = (unsigned char *) ((u32)buf->paddr + offset);
	} else
		buf->offset = 0;
	return 0;
}

int fsl_diu_display_bmp(unsigned char *bmp,
			int xoffset,
			int yoffset,
			int transpar)
{
	struct fb_info *info = &fsl_fb_info;
	unsigned char r, g, b;
	unsigned int *fb_t, val;
	unsigned char *bitmap;
	unsigned int palette[256];
	int width, height, bpp, ncolors, raster, offset, x, y, i, k, cpp;

	if (!bmp) {
		printf("Must supply a bitmap address\n");
		return 0;
	}

	raster = bmp[10] + (bmp[11] << 8) + (bmp[12] << 16) + (bmp[13] << 24);
	width  = (bmp[21] << 24) | (bmp[20] << 16) | (bmp[19] << 8) | bmp[18];
	height = (bmp[25] << 24) | (bmp[24] << 16) | (bmp[23] << 8) | bmp[22];
	bpp  = (bmp[29] <<  8) | (bmp[28]);
	ncolors = bmp[46] + (bmp[47] << 8) + (bmp[48] << 16) + (bmp[49] << 24);
	bitmap   = bmp + raster;
	cpp = info->var.bits_per_pixel / 8;

	DPRINTF("bmp = 0x%08x\n", (unsigned int)bmp);
	DPRINTF("bitmap = 0x%08x\n", (unsigned int)bitmap);
	DPRINTF("width = %d\n", width);
	DPRINTF("height = %d\n", height);
	DPRINTF("bpp = %d\n", bpp);
	DPRINTF("ncolors = %d\n", ncolors);

	DPRINTF("xres = %d\n", info->var.xres);
	DPRINTF("yres = %d\n", info->var.yres);
	DPRINTF("Screen_base = 0x%x\n", (unsigned int)info->screen_base);

	if (((width+xoffset) > info->var.xres) ||
	    ((height+yoffset) > info->var.yres)) {
		printf("bitmap is out of range, image too large or too much offset\n");
		return 0;
	}
	if (bpp < 24) {
		for (i = 0, offset = 54; i < ncolors; i++, offset += 4)
			palette[i] = (bmp[offset+2] << 16)
				+ (bmp[offset+1] << 8) + bmp[offset];
	}

	switch (bpp) {
	case 1:
		for (y = height - 1; y >= 0; y--) {
			fb_t = (unsigned int *) ((unsigned int)info->screen_base + (((y+yoffset) * info->var.xres) + xoffset)*cpp);
			for (x = 0; x < width; x += 8) {
				b = *bitmap++;
				for (k = 0; k < 8; k++) {
					if (b & 0x80)
						*fb_t = palette[1];
					else
						*fb_t = palette[0];
					b = b << 1;
				}
			}
			for (i = (width / 2) % 4; i > 0; i--)
				bitmap++;
		}
		break;
	case 4:
		for (y = height - 1; y >= 0; y--) {
			fb_t = (unsigned int *) ((unsigned int)info->screen_base + (((y+yoffset) * info->var.xres) + xoffset)*cpp);
			for (x = 0; x < width; x += 2) {
				b = *bitmap++;
				r = (b >> 4) & 0x0F;
				g =  b & 0x0F;
				*fb_t++ = palette[r];
				*fb_t++ = palette[g];
			}
			for (i = (width / 2) % 4; i > 0; i--)
				bitmap++;
		}
		break;
	case 8:
		for (y = height - 1; y >= 0; y--) {
			fb_t = (unsigned int *) ((unsigned int)info->screen_base + (((y+yoffset) * info->var.xres) + xoffset)*cpp);
			for (x = 0; x < width; x++) {
				*fb_t++ = palette[ *bitmap++ ];
			}
			for (i = (width / 2) % 4; i > 0; i--)
				bitmap++;
		}
		break;
	case 24:
		for (y = height - 1; y >= 0; y--) {
			fb_t = (unsigned int *) ((unsigned int)info->screen_base + (((y+yoffset) * info->var.xres) + xoffset)*cpp);
			for (x = 0; x < width; x++) {
				b = *bitmap++;
				g = *bitmap++;
				r = *bitmap++;
				val = (r << 16) + (g << 8) + b;
				*fb_t++ = val;
			}
			for (; (x % 4) != 0; x++)	/* 4-byte alignment */
				bitmap++;
		}
		break;
	}

	return height;
}

void fsl_diu_clear_screen(void)
{
	struct fb_info *info = &fsl_fb_info;

	memset(info->screen_base, 0, info->smem_len);
}
#endif /* CONFIG_FSL_DIU_FB */
