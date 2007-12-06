/*
 * (C) Copyright 2004
 * Pierre Aubert, Staubli Faverges , <p.aubert@staubli.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#ifndef CFG_DEFAULT_VIDEO_MODE
#define CFG_DEFAULT_VIDEO_MODE	0x301
#endif

/* Some mode definitions */
#define FB_SYNC_HOR_HIGH_ACT	1	/* horizontal sync high active	*/
#define FB_SYNC_VERT_HIGH_ACT	2	/* vertical sync high active	*/
#define FB_SYNC_EXT		4	/* external sync		*/
#define FB_SYNC_COMP_HIGH_ACT	8	/* composite sync high active	*/
#define FB_SYNC_BROADCAST	16	/* broadcast video timings	*/
					/* vtotal = 144d/288n/576i => PAL  */
					/* vtotal = 121d/242n/484i => NTSC */
#define FB_SYNC_ON_GREEN	32	/* sync on green */
#define FB_VMODE_NONINTERLACED	0	/* non interlaced */
#define FB_VMODE_INTERLACED	1	/* interlaced	*/
#define FB_VMODE_DOUBLE		2	/* double scan */
#define FB_VMODE_MASK		255

#define FB_VMODE_YWRAP		256	/* ywrap instead of panning	*/
#define FB_VMODE_SMOOTH_XPAN	512	/* smooth xpan possible (internally used) */
#define FB_VMODE_CONUPDATE	512	/* don't update x/yoffset	*/


/******************************************************************
 * Resolution Struct
 ******************************************************************/
struct ctfb_res_modes {
	int xres;		/* visible resolution		*/
	int yres;
	/* Timing: All values in pixclocks, except pixclock (of course) */
	int pixclock;		/* pixel clock in ps (pico seconds) */
	int left_margin;	/* time from sync to picture	*/
	int right_margin;	/* time from picture to sync	*/
	int upper_margin;	/* time from sync to picture	*/
	int lower_margin;
	int hsync_len;		/* length of horizontal sync	*/
	int vsync_len;		/* length of vertical sync	*/
	int sync;		/* see FB_SYNC_*		*/
	int vmode;		/* see FB_VMODE_*		*/
};

/******************************************************************
 * Vesa Mode Struct
 ******************************************************************/
struct ctfb_vesa_modes {
	int vesanr;		/* Vesa number as in LILO (VESA Nr + 0x200} */
	int resindex;		/* index to resolution struct */
	int bits_per_pixel;	/* bpp */
};

#define RES_MODE_640x480	0
#define RES_MODE_800x600	1
#define RES_MODE_1024x768	2
#define RES_MODE_960_720	3
#define RES_MODE_1152x864	4
#define RES_MODE_1280x1024	5
#define RES_MODES_COUNT		6

#define VESA_MODES_COUNT 19

extern const struct ctfb_vesa_modes vesa_modes[];
extern const struct ctfb_res_modes res_mode_init[];

int video_get_params (struct ctfb_res_modes *pPar, char *penv);
