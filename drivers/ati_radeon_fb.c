/*
 * ATI Radeon Video card Framebuffer driver.
 *
 * Copyright 2007 Freescale Semiconductor, Inc.
 * Zhang Wei <wei.zhang@freescale.com>
 * Jason Jin <jason.jin@freescale.com>
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
 *
 * Some codes of this file is partly ported from Linux kernel
 * ATI video framebuffer driver.
 *
 * Now the driver is tested on below ATI chips:
 *   9200
 *   X300
 *   X700
 *
 */

#include <common.h>

#ifdef CONFIG_ATI_RADEON_FB

#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <video_fb.h>

#include <radeon.h>
#include "ati_ids.h"
#include "ati_radeon_fb.h"

#undef DEBUG

#ifdef DEBUG
#define DPRINT(x...) printf(x)
#else
#define DPRINT(x...) do{}while(0)
#endif

#ifndef min_t
#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#endif

#define MAX_MAPPED_VRAM	(2048*2048*4)
#define MIN_MAPPED_VRAM	(1024*768*1)

/*#define PCI_VENDOR_ID_ATI*/
#define PCI_CHIP_RV280_5960		0x5960
#define PCI_CHIP_RV280_5961		0x5961
#define PCI_CHIP_RV280_5962		0x5962
#define PCI_CHIP_RV280_5964		0x5964
#define PCI_CHIP_RV370_5B60		0x5B60
#define PCI_CHIP_RV380_5657		0x5657
#define PCI_CHIP_R420_554d		0x554d

static struct pci_device_id ati_radeon_pci_ids[] = {
	{PCI_VENDOR_ID_ATI, PCI_CHIP_RV280_5960},
	{PCI_VENDOR_ID_ATI, PCI_CHIP_RV280_5961},
	{PCI_VENDOR_ID_ATI, PCI_CHIP_RV280_5962},
	{PCI_VENDOR_ID_ATI, PCI_CHIP_RV280_5964},
	{PCI_VENDOR_ID_ATI, PCI_CHIP_RV370_5B60},
	{PCI_VENDOR_ID_ATI, PCI_CHIP_RV380_5657},
	{PCI_VENDOR_ID_ATI, PCI_CHIP_R420_554d},
	{0, 0}
};

static u16 ati_radeon_id_family_table[][2] = {
	{PCI_CHIP_RV280_5960, CHIP_FAMILY_RV280},
	{PCI_CHIP_RV280_5961, CHIP_FAMILY_RV280},
	{PCI_CHIP_RV280_5962, CHIP_FAMILY_RV280},
	{PCI_CHIP_RV280_5964, CHIP_FAMILY_RV280},
	{PCI_CHIP_RV370_5B60, CHIP_FAMILY_RV380},
	{PCI_CHIP_RV380_5657, CHIP_FAMILY_RV380},
	{PCI_CHIP_R420_554d,  CHIP_FAMILY_R420},
	{0, 0}
};

u16 get_radeon_id_family(u16 device)
{
	int i;
	for (i=0; ati_radeon_id_family_table[0][i]; i+=2)
		if (ati_radeon_id_family_table[0][i] == device)
			return ati_radeon_id_family_table[0][i + 1];
	return 0;
}

struct radeonfb_info *rinfo;

static void radeon_identify_vram(struct radeonfb_info *rinfo)
{
	u32 tmp;

	/* framebuffer size */
	if ((rinfo->family == CHIP_FAMILY_RS100) ||
		(rinfo->family == CHIP_FAMILY_RS200) ||
		(rinfo->family == CHIP_FAMILY_RS300)) {
		u32 tom = INREG(NB_TOM);
		tmp = ((((tom >> 16) - (tom & 0xffff) + 1) << 6) * 1024);

		radeon_fifo_wait(6);
		OUTREG(MC_FB_LOCATION, tom);
		OUTREG(DISPLAY_BASE_ADDR, (tom & 0xffff) << 16);
		OUTREG(CRTC2_DISPLAY_BASE_ADDR, (tom & 0xffff) << 16);
		OUTREG(OV0_BASE_ADDR, (tom & 0xffff) << 16);

		/* This is supposed to fix the crtc2 noise problem. */
		OUTREG(GRPH2_BUFFER_CNTL, INREG(GRPH2_BUFFER_CNTL) & ~0x7f0000);

		if ((rinfo->family == CHIP_FAMILY_RS100) ||
			(rinfo->family == CHIP_FAMILY_RS200)) {
		/* This is to workaround the asic bug for RMX, some versions
		   of BIOS dosen't have this register initialized correctly.
		*/
			OUTREGP(CRTC_MORE_CNTL, CRTC_H_CUTOFF_ACTIVE_EN,
				~CRTC_H_CUTOFF_ACTIVE_EN);
		}
	} else {
		tmp = INREG(CONFIG_MEMSIZE);
        }

	/* mem size is bits [28:0], mask off the rest */
	rinfo->video_ram = tmp & CONFIG_MEMSIZE_MASK;

	/*
	 * Hack to get around some busted production M6's
	 * reporting no ram
	 */
	if (rinfo->video_ram == 0) {
		switch (rinfo->pdev.device) {
		case PCI_CHIP_RADEON_LY:
		case PCI_CHIP_RADEON_LZ:
			rinfo->video_ram = 8192 * 1024;
			break;
		default:
			break;
		}
	}

	/*
	 * Now try to identify VRAM type
	 */
	if ((rinfo->family >= CHIP_FAMILY_R300) ||
	    (INREG(MEM_SDRAM_MODE_REG) & (1<<30)))
		rinfo->vram_ddr = 1;
	else
		rinfo->vram_ddr = 0;

	tmp = INREG(MEM_CNTL);
	if (IS_R300_VARIANT(rinfo)) {
		tmp &=  R300_MEM_NUM_CHANNELS_MASK;
		switch (tmp) {
		case 0:  rinfo->vram_width = 64; break;
		case 1:  rinfo->vram_width = 128; break;
		case 2:  rinfo->vram_width = 256; break;
		default: rinfo->vram_width = 128; break;
		}
	} else if ((rinfo->family == CHIP_FAMILY_RV100) ||
		   (rinfo->family == CHIP_FAMILY_RS100) ||
		   (rinfo->family == CHIP_FAMILY_RS200)){
		if (tmp & RV100_MEM_HALF_MODE)
			rinfo->vram_width = 32;
		else
			rinfo->vram_width = 64;
	} else {
		if (tmp & MEM_NUM_CHANNELS_MASK)
			rinfo->vram_width = 128;
		else
			rinfo->vram_width = 64;
	}

	/* This may not be correct, as some cards can have half of channel disabled
	 * ToDo: identify these cases
	 */

	DPRINT("radeonfb: Found %ldk of %s %d bits wide videoram\n",
	       rinfo->video_ram / 1024,
	       rinfo->vram_ddr ? "DDR" : "SDRAM",
	       rinfo->vram_width);

}

static void radeon_write_pll_regs(struct radeonfb_info *rinfo, struct radeon_regs *mode)
{
	int i;

	radeon_fifo_wait(20);

#if 0
	/* Workaround from XFree */
	if (rinfo->is_mobility) {
	        /* A temporal workaround for the occational blanking on certain laptop
		 * panels. This appears to related to the PLL divider registers
		 * (fail to lock?). It occurs even when all dividers are the same
		 * with their old settings. In this case we really don't need to
		 * fiddle with PLL registers. By doing this we can avoid the blanking
		 * problem with some panels.
	         */
		if ((mode->ppll_ref_div == (INPLL(PPLL_REF_DIV) & PPLL_REF_DIV_MASK)) &&
		    (mode->ppll_div_3 == (INPLL(PPLL_DIV_3) &
					  (PPLL_POST3_DIV_MASK | PPLL_FB3_DIV_MASK)))) {
			/* We still have to force a switch to selected PPLL div thanks to
			 * an XFree86 driver bug which will switch it away in some cases
			 * even when using UseFDev */
			OUTREGP(CLOCK_CNTL_INDEX,
				mode->clk_cntl_index & PPLL_DIV_SEL_MASK,
				~PPLL_DIV_SEL_MASK);
			radeon_pll_errata_after_index(rinfo);
			radeon_pll_errata_after_data(rinfo);
			return;
		}
	}
#endif
	if(rinfo->pdev.device == PCI_CHIP_RV370_5B60) return;

	/* Swich VCKL clock input to CPUCLK so it stays fed while PPLL updates*/
	OUTPLLP(VCLK_ECP_CNTL, VCLK_SRC_SEL_CPUCLK, ~VCLK_SRC_SEL_MASK);

	/* Reset PPLL & enable atomic update */
	OUTPLLP(PPLL_CNTL,
		PPLL_RESET | PPLL_ATOMIC_UPDATE_EN | PPLL_VGA_ATOMIC_UPDATE_EN,
		~(PPLL_RESET | PPLL_ATOMIC_UPDATE_EN | PPLL_VGA_ATOMIC_UPDATE_EN));

	/* Switch to selected PPLL divider */
	OUTREGP(CLOCK_CNTL_INDEX,
		mode->clk_cntl_index & PPLL_DIV_SEL_MASK,
		~PPLL_DIV_SEL_MASK);

	/* Set PPLL ref. div */
	if (rinfo->family == CHIP_FAMILY_R300 ||
	    rinfo->family == CHIP_FAMILY_RS300 ||
	    rinfo->family == CHIP_FAMILY_R350 ||
	    rinfo->family == CHIP_FAMILY_RV350) {
		if (mode->ppll_ref_div & R300_PPLL_REF_DIV_ACC_MASK) {
			/* When restoring console mode, use saved PPLL_REF_DIV
			 * setting.
			 */
			OUTPLLP(PPLL_REF_DIV, mode->ppll_ref_div, 0);
		} else {
			/* R300 uses ref_div_acc field as real ref divider */
			OUTPLLP(PPLL_REF_DIV,
				(mode->ppll_ref_div << R300_PPLL_REF_DIV_ACC_SHIFT),
				~R300_PPLL_REF_DIV_ACC_MASK);
		}
	} else
		OUTPLLP(PPLL_REF_DIV, mode->ppll_ref_div, ~PPLL_REF_DIV_MASK);

	/* Set PPLL divider 3 & post divider*/
	OUTPLLP(PPLL_DIV_3, mode->ppll_div_3, ~PPLL_FB3_DIV_MASK);
	OUTPLLP(PPLL_DIV_3, mode->ppll_div_3, ~PPLL_POST3_DIV_MASK);

	/* Write update */
	while (INPLL(PPLL_REF_DIV) & PPLL_ATOMIC_UPDATE_R)
		;
	OUTPLLP(PPLL_REF_DIV, PPLL_ATOMIC_UPDATE_W, ~PPLL_ATOMIC_UPDATE_W);

	/* Wait read update complete */
	/* FIXME: Certain revisions of R300 can't recover here.  Not sure of
	   the cause yet, but this workaround will mask the problem for now.
	   Other chips usually will pass at the very first test, so the
	   workaround shouldn't have any effect on them. */
	for (i = 0; (i < 10000 && INPLL(PPLL_REF_DIV) & PPLL_ATOMIC_UPDATE_R); i++)
		;

	OUTPLL(HTOTAL_CNTL, 0);

	/* Clear reset & atomic update */
	OUTPLLP(PPLL_CNTL, 0,
		~(PPLL_RESET | PPLL_SLEEP | PPLL_ATOMIC_UPDATE_EN | PPLL_VGA_ATOMIC_UPDATE_EN));

	/* We may want some locking ... oh well */
	udelay(5000);

	/* Switch back VCLK source to PPLL */
	OUTPLLP(VCLK_ECP_CNTL, VCLK_SRC_SEL_PPLLCLK, ~VCLK_SRC_SEL_MASK);
}

typedef struct {
	u16 reg;
	u32 val;
} reg_val;


/* these common regs are cleared before mode setting so they do not
 * interfere with anything
 */
static reg_val common_regs[] = {
	{ OVR_CLR, 0 },
	{ OVR_WID_LEFT_RIGHT, 0 },
	{ OVR_WID_TOP_BOTTOM, 0 },
	{ OV0_SCALE_CNTL, 0 },
	{ SUBPIC_CNTL, 0 },
	{ VIPH_CONTROL, 0 },
	{ I2C_CNTL_1, 0 },
	{ GEN_INT_CNTL, 0 },
	{ CAP0_TRIG_CNTL, 0 },
	{ CAP1_TRIG_CNTL, 0 },
};


void radeon_setmode(void)
{
	int i;
	struct radeon_regs *mode = malloc(sizeof(struct radeon_regs));

	mode->crtc_gen_cntl = 0x03000200;
	mode->crtc_ext_cntl = 0x00008048;
	mode->dac_cntl = 0xff002100;
	mode->crtc_h_total_disp = 0x4f0063;
	mode->crtc_h_sync_strt_wid = 0x8c02a2;
	mode->crtc_v_total_disp = 0x01df020c;
	mode->crtc_v_sync_strt_wid = 0x8201ea;
	mode->crtc_pitch = 0x00500050;

	OUTREG(CRTC_GEN_CNTL, mode->crtc_gen_cntl);
	OUTREGP(CRTC_EXT_CNTL, mode->crtc_ext_cntl,
		~(CRTC_HSYNC_DIS | CRTC_VSYNC_DIS | CRTC_DISPLAY_DIS));
	OUTREGP(DAC_CNTL, mode->dac_cntl, DAC_RANGE_CNTL | DAC_BLANKING);
	OUTREG(CRTC_H_TOTAL_DISP, mode->crtc_h_total_disp);
	OUTREG(CRTC_H_SYNC_STRT_WID, mode->crtc_h_sync_strt_wid);
	OUTREG(CRTC_V_TOTAL_DISP, mode->crtc_v_total_disp);
	OUTREG(CRTC_V_SYNC_STRT_WID, mode->crtc_v_sync_strt_wid);
	OUTREG(CRTC_OFFSET, 0);
	OUTREG(CRTC_OFFSET_CNTL, 0);
	OUTREG(CRTC_PITCH, mode->crtc_pitch);

	mode->clk_cntl_index = 0x300;
	mode->ppll_ref_div = 0xc;
	mode->ppll_div_3 = 0x00030059;

	radeon_write_pll_regs(rinfo, mode);
}

int radeon_probe(struct radeonfb_info *rinfo)
{
	pci_dev_t pdev;
	u16 did;

	pdev = pci_find_devices(ati_radeon_pci_ids, 0);

	if (pdev != -1) {
		pci_read_config_word(pdev, PCI_DEVICE_ID, &did);
		printf("ATI Radeon video card (%04x, %04x) found @(%d:%d:%d)\n",
				PCI_VENDOR_ID_ATI, did, (pdev >> 16) & 0xff,
				(pdev >> 11) & 0x1f, (pdev >> 8) & 0x7);

		strcpy(rinfo->name, "ATI Radeon");
		rinfo->pdev.vendor = PCI_VENDOR_ID_ATI;
		rinfo->pdev.device = did;
		rinfo->family = get_radeon_id_family(rinfo->pdev.device);
		pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0,
				&rinfo->fb_base_phys);
		pci_read_config_dword(pdev, PCI_BASE_ADDRESS_2,
				&rinfo->mmio_base_phys);
		rinfo->fb_base_phys &= 0xfffff000;
		rinfo->mmio_base_phys &= ~0x04;

		rinfo->mmio_base = (void *)rinfo->mmio_base_phys;
		DPRINT("rinfo->mmio_base = 0x%x\n",rinfo->mmio_base);
		rinfo->fb_local_base = INREG(MC_FB_LOCATION) << 16;
		DPRINT("rinfo->fb_local_base = 0x%x\n",rinfo->fb_local_base);
		/* PostBIOS with x86 emulater */
		BootVideoCardBIOS(pdev, NULL, 0);

		/*
		 * Check for errata
		 * (These will be added in the future for the chipfamily
		 * R300, RV200, RS200, RV100, RS100.)
		 */

		/* Get VRAM size and type */
		radeon_identify_vram(rinfo);

		rinfo->mapped_vram = min_t(unsigned long, MAX_MAPPED_VRAM,
				rinfo->video_ram);
		rinfo->fb_base = (void *)rinfo->fb_base_phys;

		DPRINT("Radeon: framebuffer base phy address 0x%08x," \
		      "MMIO base phy address 0x%08x," \
		      "framebuffer local base 0x%08x.\n ",
		      rinfo->fb_base_phys, rinfo->mmio_base_phys,
		      rinfo->fb_local_base);

		return 0;
	}
	return -1;
}

/*
 * The Graphic Device
 */
GraphicDevice ctfb;

#define CURSOR_SIZE	0x1000	/* in KByte for HW Cursor */
#define PATTERN_ADR	(pGD->dprBase + CURSOR_SIZE)	/* pattern Memory after Cursor Memory */
#define PATTERN_SIZE	8*8*4	/* 4 Bytes per Pixel 8 x 8 Pixel */
#define ACCELMEMORY	(CURSOR_SIZE + PATTERN_SIZE)	/* reserved Memory for BITBlt and hw cursor */

void *video_hw_init(void)
{
	GraphicDevice *pGD = (GraphicDevice *) & ctfb;
	int i;
	u32 *vm;

	rinfo = malloc(sizeof(struct radeonfb_info));

	if(radeon_probe(rinfo)) {
		printf("No radeon video card found!\n");
		return NULL;
	}

	/* fill in Graphic device struct */
	sprintf (pGD->modeIdent, "%dx%dx%d %ldkHz %ldHz", 640,
		 480, 16, (1000 / 1000),
		 (2000 / 1000));
	printf ("%s\n", pGD->modeIdent);

	pGD->winSizeX = 640;
	pGD->winSizeY = 480;
	pGD->plnSizeX = 640;
	pGD->plnSizeY = 480;

	pGD->gdfBytesPP = 1;
	pGD->gdfIndex = GDF__8BIT_INDEX;

	pGD->isaBase = CFG_ISA_IO_BASE_ADDRESS;
	pGD->pciBase = rinfo->fb_base_phys;
	pGD->frameAdrs = rinfo->fb_base_phys;
	pGD->memSize = 64 * 1024 * 1024;

	/* Cursor Start Address */
	pGD->dprBase =
	    (pGD->winSizeX * pGD->winSizeY * pGD->gdfBytesPP) + rinfo->fb_base_phys;
	if ((pGD->dprBase & 0x0fff) != 0) {
		/* allign it */
		pGD->dprBase &= 0xfffff000;
		pGD->dprBase += 0x00001000;
	}
	DPRINT ("Cursor Start %x Pattern Start %x\n", pGD->dprBase,
		PATTERN_ADR);
	pGD->vprBase = rinfo->fb_base_phys;	/* Dummy */
	pGD->cprBase = rinfo->fb_base_phys;	/* Dummy */
	/* set up Hardware */

	/* Clear video memory */
	i = pGD->memSize / 4;
	vm = (unsigned int *) pGD->pciBase;
	while (i--)
		*vm++ = 0;
	/*SetDrawingEngine (bits_per_pixel);*/

	radeon_setmode();

	return ((void *) pGD);
}

void video_set_lut (unsigned int index,	/* color number */
	       unsigned char r,	/* red */
	       unsigned char g,	/* green */
	       unsigned char b	/* blue */
	       )
{
	OUTREG(PALETTE_INDEX, index);
	OUTREG(PALETTE_DATA, (r << 16) | (g << 8) | b);
}
#endif
