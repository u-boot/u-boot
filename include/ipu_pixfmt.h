/*
 * (C) Copyright 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * Based on Linux IPU driver for MX51 (ipu.h):
 *
 * (C) Copyright 2005-2010 Freescale Semiconductor, Inc.
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

#ifndef __IPU_PIXFMT_H__
#define __IPU_PIXFMT_H__

#include <linux/list.h>
#include <linux/fb.h>

/*  IPU Pixel format definitions */
#define fourcc(a, b, c, d)\
	(((__u32)(a)<<0)|((__u32)(b)<<8)|((__u32)(c)<<16)|((__u32)(d)<<24))

/*
 * Pixel formats are defined with ASCII FOURCC code. The pixel format codes are
 * the same used by V4L2 API.
 */

#define IPU_PIX_FMT_GENERIC fourcc('I', 'P', 'U', '0')
#define IPU_PIX_FMT_GENERIC_32 fourcc('I', 'P', 'U', '1')
#define IPU_PIX_FMT_LVDS666 fourcc('L', 'V', 'D', '6')
#define IPU_PIX_FMT_LVDS888 fourcc('L', 'V', 'D', '8')

#define IPU_PIX_FMT_RGB332  fourcc('R', 'G', 'B', '1')	/*<  8  RGB-3-3-2    */
#define IPU_PIX_FMT_RGB555  fourcc('R', 'G', 'B', 'O')	/*< 16  RGB-5-5-5    */
#define IPU_PIX_FMT_RGB565  fourcc('R', 'G', 'B', 'P')	/*< 1 6  RGB-5-6-5   */
#define IPU_PIX_FMT_RGB666  fourcc('R', 'G', 'B', '6')	/*< 18  RGB-6-6-6    */
#define IPU_PIX_FMT_BGR666  fourcc('B', 'G', 'R', '6')	/*< 18  BGR-6-6-6    */
#define IPU_PIX_FMT_BGR24   fourcc('B', 'G', 'R', '3')	/*< 24  BGR-8-8-8    */
#define IPU_PIX_FMT_RGB24   fourcc('R', 'G', 'B', '3')	/*< 24  RGB-8-8-8    */
#define IPU_PIX_FMT_BGR32   fourcc('B', 'G', 'R', '4')	/*< 32  BGR-8-8-8-8  */
#define IPU_PIX_FMT_BGRA32  fourcc('B', 'G', 'R', 'A')	/*< 32  BGR-8-8-8-8  */
#define IPU_PIX_FMT_RGB32   fourcc('R', 'G', 'B', '4')	/*< 32  RGB-8-8-8-8  */
#define IPU_PIX_FMT_RGBA32  fourcc('R', 'G', 'B', 'A')	/*< 32  RGB-8-8-8-8  */
#define IPU_PIX_FMT_ABGR32  fourcc('A', 'B', 'G', 'R')	/*< 32  ABGR-8-8-8-8 */

/* YUV Interleaved Formats */
#define IPU_PIX_FMT_YUYV    fourcc('Y', 'U', 'Y', 'V')	/*< 16 YUV 4:2:2 */
#define IPU_PIX_FMT_UYVY    fourcc('U', 'Y', 'V', 'Y')	/*< 16 YUV 4:2:2 */
#define IPU_PIX_FMT_Y41P    fourcc('Y', '4', '1', 'P')	/*< 12 YUV 4:1:1 */
#define IPU_PIX_FMT_YUV444  fourcc('Y', '4', '4', '4')	/*< 24 YUV 4:4:4 */

/* two planes -- one Y, one Cb + Cr interleaved  */
#define IPU_PIX_FMT_NV12    fourcc('N', 'V', '1', '2') /* 12  Y/CbCr 4:2:0  */

#define IPU_PIX_FMT_GREY    fourcc('G', 'R', 'E', 'Y')	/*< 8  Greyscale */
#define IPU_PIX_FMT_YVU410P fourcc('Y', 'V', 'U', '9')	/*< 9  YVU 4:1:0 */
#define IPU_PIX_FMT_YUV410P fourcc('Y', 'U', 'V', '9')	/*< 9  YUV 4:1:0 */
#define IPU_PIX_FMT_YVU420P fourcc('Y', 'V', '1', '2')	/*< 12 YVU 4:2:0 */
#define IPU_PIX_FMT_YUV420P fourcc('I', '4', '2', '0')	/*< 12 YUV 4:2:0 */
#define IPU_PIX_FMT_YUV420P2 fourcc('Y', 'U', '1', '2')	/*< 12 YUV 4:2:0 */
#define IPU_PIX_FMT_YVU422P fourcc('Y', 'V', '1', '6')	/*< 16 YVU 4:2:2 */
#define IPU_PIX_FMT_YUV422P fourcc('4', '2', '2', 'P')	/*< 16 YUV 4:2:2 */

int ipuv3_fb_init(struct fb_videomode const *mode,
		  uint8_t disp,
		  uint32_t pixfmt);
void ipuv3_fb_shutdown(void);

#endif
