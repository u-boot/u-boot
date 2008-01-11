/*
 * (C) Copyright 2007
 * DENX Software Engineering, Anatolij Gustschin, agust@denx.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * mb862xx.c - Graphic interface for Fujitsu CoralP/Lime
 * PCI and video mode code was derived from smiLynxEM driver.
 */

#include <common.h>

#if defined(CONFIG_VIDEO_MB862xx)

#include <asm/io.h>
#include <pci.h>
#include <video_fb.h>
#include "videomodes.h"
#include <mb862xx.h>

/*
 * Graphic Device
 */
GraphicDevice mb862xx;

/*
 * 32MB external RAM - 256K Chip MMIO = 0x1FC0000 ;
 */
#define VIDEO_MEM_SIZE	0x01FC0000

#if defined(CONFIG_PCI)
#if defined(CONFIG_VIDEO_CORALP)

static struct pci_device_id supported[] = {
	{ PCI_VENDOR_ID_FUJITSU, PCI_DEVICE_ID_CORAL_P },
	{ PCI_VENDOR_ID_FUJITSU, PCI_DEVICE_ID_CORAL_PA },
	{ }
};

/* Internal clock frequency divider table, index is mode number */
unsigned int fr_div[] = { 0x00000f00, 0x00000900, 0x00000500 };
#endif
#endif

#if defined(CONFIG_VIDEO_CORALP)
#define	rd_io		in32r
#define	wr_io		out32r
#else
#define	rd_io(addr)	in_be32((volatile unsigned*)(addr))
#define	wr_io(addr,val)	out_be32((volatile unsigned*)(addr), (val))
#endif

#define HOST_RD_REG(off)	rd_io((pGD->frameAdrs + 0x01fc0000 + (off)))
#define HOST_WR_REG(off, val)	wr_io((pGD->frameAdrs + 0x01fc0000 + (off)), (val))
#define DISP_RD_REG(off)	rd_io((pGD->frameAdrs + 0x01fd0000 + (off)))
#define DISP_WR_REG(off, val)	wr_io((pGD->frameAdrs + 0x01fd0000 + (off)), (val))
#define DE_RD_REG(off)		rd_io((pGD->dprBase + (off)))
#define DE_WR_REG(off, val)	wr_io((pGD->dprBase + (off)), (val))

#if defined(CONFIG_VIDEO_CORALP)
#define DE_WR_FIFO(val)		wr_io((pGD->dprBase + (0x8400)), (val))
#else
#define DE_WR_FIFO(val)		wr_io((pGD->dprBase + (0x04a0)), (val))
#endif

#define L0PAL_RD_REG(idx, val)	rd_io((pGD->frameAdrs + 0x01fd0400 + ((idx)<<2)))
#define L0PAL_WR_REG(idx, val)	wr_io((pGD->frameAdrs + 0x01fd0400 + ((idx)<<2)), (val))
#define L1PAL_RD_REG(idx, val)	rd_io((pGD->frameAdrs + 0x01fd0800 + ((idx)<<2)))
#define L1PAL_WR_REG(idx, val)	wr_io((pGD->frameAdrs + 0x01fd0800 + ((idx)<<2)), (val))
#define L2PAL_RD_REG(idx, val)	rd_io((pGD->frameAdrs + 0x01fd1000 + ((idx)<<2)))
#define L2PAL_WR_REG(idx, val)	wr_io((pGD->frameAdrs + 0x01fd1000 + ((idx)<<2)), (val))
#define L3PAL_RD_REG(idx, val)	rd_io((pGD->frameAdrs + 0x01fd1400 + ((idx)<<2)))
#define L3PAL_WR_REG(idx, val)	wr_io((pGD->frameAdrs + 0x01fd1400 + ((idx)<<2)), (val))

static void gdc_sw_reset(void)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	HOST_WR_REG (0x002c, 0x00000001);
	udelay (500);
	video_hw_init ();
}


static void de_wait(void)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	int lc = 0x10000;

	/* Sync with software writes to framebuffer,
	   try to reset if engine locked */
	while (DE_RD_REG (0x0400) & 0x00000131)
		if (lc-- < 0) {
			gdc_sw_reset ();
			printf ("gdc reset done after drawing engine lock...\n");
			break;
		}
}

static void de_wait_slots(int slots)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	int lc = 0x10000;

	/* Wait for free fifo slots */
	while (DE_RD_REG (0x0408) < slots)
		if (lc-- < 0) {
			gdc_sw_reset ();
			printf ("gdc reset done after drawing engine lock...\n");
			break;
		}
}

#if !defined(CONFIG_VIDEO_CORALP)
static void board_disp_init(void)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	const gdc_regs *regs = board_get_regs ();

	while (regs->index) {
		DISP_WR_REG (regs->index, regs->value);
		regs++;
	}
}
#endif

/*
 * Init drawing engine
 */
static void de_init (void)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	int cf = (pGD->gdfBytesPP == 1) ? 0x0000 : 0x8000;

	pGD->dprBase = pGD->frameAdrs + 0x01ff0000;

	/* Setup mode and fbbase, xres, fg, bg */
	de_wait_slots (2);
	DE_WR_FIFO (0xf1010108);
	DE_WR_FIFO (cf | 0x0300);
	DE_WR_REG (0x0440, 0x0000);
	DE_WR_REG (0x0444, pGD->winSizeX);
	DE_WR_REG (0x0480, 0x0000);
	DE_WR_REG (0x0484, 0x0000);
	/* Reset clipping */
	DE_WR_REG (0x0454, 0x0000);
	DE_WR_REG (0x0458, pGD->winSizeX);
	DE_WR_REG (0x045c, 0x0000);
	DE_WR_REG (0x0460, pGD->winSizeY);

	/* Clear framebuffer using drawing engine */
	de_wait_slots (3);
	DE_WR_FIFO (0x09410000);
	DE_WR_FIFO (0x00000000);
	DE_WR_FIFO (pGD->winSizeY<<16 | pGD->winSizeX);
}

#if defined(CONFIG_VIDEO_CORALP)
unsigned int pci_video_init(void)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	pci_dev_t devbusfn;

	if ((devbusfn = pci_find_devices(supported, 0)) < 0)
	{
		printf ("PCI video controller not found!\n");
		return 0;
	}

	/* PCI setup */
	pci_write_config_dword (devbusfn, PCI_COMMAND, (PCI_COMMAND_MEMORY | PCI_COMMAND_IO));
	pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_0, &pGD->frameAdrs);
	pGD->frameAdrs = pci_mem_to_phys (devbusfn, pGD->frameAdrs);

	if (pGD->frameAdrs == 0) {
		printf ("PCI config: failed to get base address\n");
		return 0;
	}

	pGD->pciBase = pGD->frameAdrs;

	/* Setup clocks and memory mode for Coral-P Eval. Board */
	HOST_WR_REG (0x0038, 0x00090000);
	udelay (200);
	HOST_WR_REG (0xfffc, 0x11d7fa13);
	udelay (100);
	return pGD->frameAdrs;
}

unsigned int card_init (void)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	unsigned int cf, videomode, div = 0;
	unsigned long t1, hsync, vsync;
	char *penv;
	int tmp, i, bpp;
	struct ctfb_res_modes *res_mode;
	struct ctfb_res_modes var_mode;

	memset (pGD, 0, sizeof (GraphicDevice));

	if (!pci_video_init ()) {
		return 0;
	}

	printf ("CoralP\n");

	tmp = 0;
	videomode = 0x310;
	/* get video mode via environment */
	if ((penv = getenv ("videomode")) != NULL) {
		/* deceide if it is a string */
		if (penv[0] <= '9') {
			videomode = (int) simple_strtoul (penv, NULL, 16);
			tmp = 1;
		}
	} else {
		tmp = 1;
	}
	if (tmp) {
		/* parameter are vesa modes */
		/* search params */
		for (i = 0; i < VESA_MODES_COUNT; i++) {
			if (vesa_modes[i].vesanr == videomode)
				break;
		}
		if (i == VESA_MODES_COUNT) {
			printf ("\tno VESA Mode found, switching to mode 0x%x \n", videomode);
			i = 0;
		}
		res_mode =
			(struct ctfb_res_modes *) &res_mode_init[vesa_modes[i].resindex];
		if (vesa_modes[i].resindex > 2) {
			printf ("\tUnsupported resolution, switching to default\n");
			bpp = vesa_modes[1].bits_per_pixel;
			div = fr_div[1];
		}
		bpp = vesa_modes[i].bits_per_pixel;
		div = fr_div[vesa_modes[i].resindex];
	} else {

		res_mode = (struct ctfb_res_modes *) &var_mode;
		bpp = video_get_params (res_mode, penv);
	}

	/* calculate hsync and vsync freq (info only) */
	t1 = (res_mode->left_margin + res_mode->xres +
	      res_mode->right_margin + res_mode->hsync_len) / 8;
	t1 *= 8;
	t1 *= res_mode->pixclock;
	t1 /= 1000;
	hsync = 1000000000L / t1;
	t1 *= (res_mode->upper_margin + res_mode->yres +
	       res_mode->lower_margin + res_mode->vsync_len);
	t1 /= 1000;
	vsync = 1000000000L / t1;

	/* fill in Graphic device struct */
	sprintf (pGD->modeIdent, "%dx%dx%d %ldkHz %ldHz", res_mode->xres,
		 res_mode->yres, bpp, (hsync / 1000), (vsync / 1000));
	printf ("\t%s\n", pGD->modeIdent);
	pGD->winSizeX = res_mode->xres;
	pGD->winSizeY = res_mode->yres;
	pGD->memSize = VIDEO_MEM_SIZE;

	switch (bpp) {
	case 8:
		pGD->gdfIndex = GDF__8BIT_INDEX;
		pGD->gdfBytesPP = 1;
		break;
	case 15:
	case 16:
		pGD->gdfIndex = GDF_15BIT_555RGB;
		pGD->gdfBytesPP = 2;
		break;
	default:
		printf ("\t%d bpp configured, but only 8,15 and 16 supported.\n", bpp);
		printf ("\tSwitching back to 15bpp\n");
		pGD->gdfIndex = GDF_15BIT_555RGB;
		pGD->gdfBytesPP = 2;
	}

	/* Setup dot clock (internal pll, division rate) */
	DISP_WR_REG (0x0100, div);
	/* L0 init */
	cf = (pGD->gdfBytesPP == 1) ? 0x00000000 : 0x80000000;
	DISP_WR_REG (0x0020, ((pGD->winSizeX * pGD->gdfBytesPP)/64)<<16 |
			     (pGD->winSizeY-1) |
			     cf);
	DISP_WR_REG (0x0024, 0x00000000);
	DISP_WR_REG (0x0028, 0x00000000);
	DISP_WR_REG (0x002c, 0x00000000);
	DISP_WR_REG (0x0110, 0x00000000);
	DISP_WR_REG (0x0114, 0x00000000);
	DISP_WR_REG (0x0118, (pGD->winSizeY-1)<<16 | pGD->winSizeX);

	/* Display timing init */
	DISP_WR_REG (0x0004, (pGD->winSizeX+res_mode->left_margin+res_mode->right_margin+res_mode->hsync_len-1)<<16);
	DISP_WR_REG (0x0008, (pGD->winSizeX-1) << 16 | (pGD->winSizeX-1));
	DISP_WR_REG (0x000c, (res_mode->vsync_len-1)<<24|(res_mode->hsync_len-1)<<16|(pGD->winSizeX+res_mode->right_margin-1));
	DISP_WR_REG (0x0010, (pGD->winSizeY+res_mode->lower_margin+res_mode->upper_margin+res_mode->vsync_len-1)<<16);
	DISP_WR_REG (0x0014, (pGD->winSizeY-1) << 16 | (pGD->winSizeY+res_mode->lower_margin-1));
	DISP_WR_REG (0x0018, 0x00000000);
	DISP_WR_REG (0x001c, pGD->winSizeY << 16 | pGD->winSizeX);
	/* Display enable, L0 layer */
	DISP_WR_REG (0x0100, 0x80010000 | div);

	return pGD->frameAdrs;
}
#endif

void *video_hw_init (void)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;

	printf ("Video: Fujitsu ");

	memset (pGD, 0, sizeof (GraphicDevice));

#if defined(CONFIG_VIDEO_CORALP)
	if (card_init () == 0) {
		return (NULL);
	}
#else
	/* Preliminary init of the onboard graphic controller,
	   retrieve base address */
	if ((pGD->frameAdrs = board_video_init ()) == 0) {
		printf ("Controller not found!\n");
		return (NULL);
	} else
		printf("Lime\n");
#endif

	de_init ();

#if !defined(CONFIG_VIDEO_CORALP)
	board_disp_init();
#endif

#if defined(CONFIG_LWMON5)
	/* Lamp on */
	board_backlight_switch (1);
#endif

	return pGD;
}

/*
 * Set a RGB color in the LUT
 */
void video_set_lut (unsigned int index, unsigned char r, unsigned char g, unsigned char b)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;

	L0PAL_WR_REG (index, (r << 16) | (g << 8) | (b));
}

/*
 * Drawing engine Fill and BitBlt screen region
 */
void video_hw_rectfill (unsigned int bpp, unsigned int dst_x, unsigned int dst_y,
			unsigned int dim_x, unsigned int dim_y, unsigned int color)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;

	de_wait_slots (3);
	DE_WR_REG (0x0480, color);
	DE_WR_FIFO (0x09410000);
	DE_WR_FIFO ((dst_y << 16) | dst_x);
	DE_WR_FIFO ((dim_y << 16) | dim_x);
	de_wait ();
}

void video_hw_bitblt (unsigned int bpp, unsigned int src_x, unsigned int src_y,
		      unsigned int dst_x, unsigned int dst_y, unsigned int width,
		      unsigned int height)
{
	GraphicDevice *pGD = (GraphicDevice *)&mb862xx;
	unsigned int ctrl = 0x0d000000L;

	if (src_x >= dst_x && src_y >= dst_y)
		ctrl |= 0x00440000L;
	else if (src_x >= dst_x && src_y <= dst_y)
		ctrl |= 0x00460000L;
	else if (src_x <= dst_x && src_y >= dst_y)
		ctrl |= 0x00450000L;
	else
		ctrl |= 0x00470000L;

	de_wait_slots (4);
	DE_WR_FIFO (ctrl);
	DE_WR_FIFO ((src_y << 16) | src_x);
	DE_WR_FIFO ((dst_y << 16) | dst_x);
	DE_WR_FIFO ((height << 16) | width);
	de_wait (); /* sync */
}
#endif /* CONFIG_VIDEO_MB862xx */
