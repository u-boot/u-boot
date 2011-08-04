/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
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
/* Video support for Epson SED13806 chipset                                  */

#include <common.h>

#include <video_fb.h>
#include <sed13806.h>

#define readByte(ptrReg)                \
    *(volatile unsigned char *)(sed13806.isaBase + ptrReg)

#define writeByte(ptrReg,value) \
    *(volatile unsigned char *)(sed13806.isaBase + ptrReg) = value

#ifdef CONFIG_TOTAL5200
#define writeWord(ptrReg,value) \
    (*(volatile unsigned short *)(sed13806.isaBase + ptrReg) = value)
#else
#define writeWord(ptrReg,value) \
    (*(volatile unsigned short *)(sed13806.isaBase + ptrReg) = ((value >> 8 ) & 0xff) | ((value << 8) & 0xff00))
#endif

GraphicDevice sed13806;

/*-----------------------------------------------------------------------------
 * EpsonSetRegs --
 *-----------------------------------------------------------------------------
 */
static void EpsonSetRegs (void)
{
    /* the content of the chipset register depends on the board (clocks, ...)*/
    const S1D_REGS *preg = board_get_regs ();
    while (preg -> Index) {
	writeByte (preg -> Index, preg -> Value);
	preg ++;
    }
}

/*-----------------------------------------------------------------------------
 * video_hw_init --
 *-----------------------------------------------------------------------------
 */
void *video_hw_init (void)
{
    unsigned int *vm, i;

    memset (&sed13806, 0, sizeof (GraphicDevice));

    /* Initialization of the access to the graphic chipset
       Retreive base address of the chipset
       (see board/RPXClassic/eccx.c)                                         */
    if ((sed13806.isaBase = board_video_init ()) == 0) {
	return (NULL);
    }

    sed13806.frameAdrs = sed13806.isaBase + FRAME_BUFFER_OFFSET;
    sed13806.winSizeX = board_get_width ();
    sed13806.winSizeY = board_get_height ();

#if defined(CONFIG_VIDEO_SED13806_8BPP)
    sed13806.gdfIndex = GDF__8BIT_INDEX;
    sed13806.gdfBytesPP = 1;

#elif defined(CONFIG_VIDEO_SED13806_16BPP)
    sed13806.gdfIndex = GDF_16BIT_565RGB;
    sed13806.gdfBytesPP = 2;

#else
#error Unsupported SED13806 BPP
#endif

    sed13806.memSize = sed13806.winSizeX * sed13806.winSizeY * sed13806.gdfBytesPP;

    /* Load SED registers                                                    */
    EpsonSetRegs ();

    /* (see board/RPXClassic/RPXClassic.c)                                   */
    board_validate_screen (sed13806.isaBase);

    /* Clear video memory */
    i = sed13806.memSize/4;
    vm = (unsigned int *)sed13806.frameAdrs;
    while(i--)
	*vm++ = 0;


    return (&sed13806);
}
/*-----------------------------------------------------------------------------
 * Epson_wait_idle -- Wait for hardware to become idle
 *-----------------------------------------------------------------------------
 */
static void Epson_wait_idle (void)
{
    while (readByte (BLT_CTRL0) & 0x80);

    /* Read a word in the BitBLT memory area to shutdown the BitBLT engine   */
    *(volatile unsigned short *)(sed13806.isaBase + BLT_REG);
}

/*-----------------------------------------------------------------------------
 * video_hw_bitblt --
 *-----------------------------------------------------------------------------
 */
void video_hw_bitblt (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int src_x,           /* source pos x */
    unsigned int src_y,           /* source pos y */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y            /* frame height */
    )
{
    register GraphicDevice *pGD = (GraphicDevice *)&sed13806;
    unsigned long	srcAddr, dstAddr;
    unsigned int stride = bpp * pGD -> winSizeX;

    srcAddr = (src_y * stride) + (src_x * bpp);
    dstAddr = (dst_y * stride) + (dst_x * bpp);

    Epson_wait_idle ();

    writeByte(BLT_ROP,0x0C);	/* source */
    writeByte(BLT_OP,0x02);/* move blit in positive direction with ROP */
    writeWord(BLT_MEM_OFF0, stride / 2);
    if (pGD -> gdfIndex == GDF__8BIT_INDEX) {
	writeByte(BLT_CTRL1,0x00);
    }
    else {
	writeByte(BLT_CTRL1,0x01);
    }

    writeWord(BLT_WIDTH0,(dim_x - 1));
    writeWord(BLT_HEIGHT0,(dim_y - 1));

    /* set up blit registers                                                 */
    writeByte(BLT_SRC_ADDR0,srcAddr);
    writeByte(BLT_SRC_ADDR1,srcAddr>>8);
    writeByte(BLT_SRC_ADDR2,srcAddr>>16);

    writeByte(BLT_DST_ADDR0,dstAddr);
    writeByte(BLT_DST_ADDR1,dstAddr>>8);
    writeByte(BLT_DST_ADDR2,dstAddr>>16);

    /* Engage the blt engine                                                 */
    /* rectangular region for src and dst                                    */
    writeByte(BLT_CTRL0,0x80);

    /* wait untill current blits finished                                    */
    Epson_wait_idle ();
}
/*-----------------------------------------------------------------------------
 * video_hw_rectfill --
 *-----------------------------------------------------------------------------
 */
void video_hw_rectfill (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y,           /* frame height */
    unsigned int color            /* fill color */
     )
{
    register GraphicDevice *pGD = (GraphicDevice *)&sed13806;
    unsigned long	dstAddr;
    unsigned int stride = bpp * pGD -> winSizeX;

    dstAddr = (dst_y * stride) + (dst_x * bpp);

    Epson_wait_idle ();

    /* set up blit registers                                                 */
    writeByte(BLT_DST_ADDR0,dstAddr);
    writeByte(BLT_DST_ADDR1,dstAddr>>8);
    writeByte(BLT_DST_ADDR2,dstAddr>>16);

    writeWord(BLT_WIDTH0,(dim_x - 1));
    writeWord(BLT_HEIGHT0,(dim_y - 1));
    writeWord(BLT_FGCOLOR0,color);

    writeByte(BLT_OP,0x0C);  /* solid fill                                   */
    writeWord(BLT_MEM_OFF0,stride / 2);

    if (pGD -> gdfIndex == GDF__8BIT_INDEX) {
	writeByte(BLT_CTRL1,0x00);
    }
    else {
	writeByte(BLT_CTRL1,0x01);
    }

    /* Engage the blt engine                                                 */
    /* rectangular region for src and dst                                    */
    writeByte(BLT_CTRL0,0x80);

    /* wait untill current blits finished                                    */
    Epson_wait_idle ();
}

/*-----------------------------------------------------------------------------
 * video_set_lut --
 *-----------------------------------------------------------------------------
 */
void video_set_lut (
    unsigned int index,           /* color number */
    unsigned char r,              /* red */
    unsigned char g,              /* green */
    unsigned char b               /* blue */
    )
{
    writeByte(REG_LUT_ADDR, index );
    writeByte(REG_LUT_DATA, r);
    writeByte(REG_LUT_DATA, g);
    writeByte(REG_LUT_DATA, b);
}
#ifdef CONFIG_VIDEO_HW_CURSOR
/*-----------------------------------------------------------------------------
 * video_set_hw_cursor --
 *-----------------------------------------------------------------------------
 */
void video_set_hw_cursor (int x, int y)
{
    writeByte (LCD_CURSOR_XL, (x & 0xff));
    writeByte (LCD_CURSOR_XM, (x >> 8));
    writeByte (LCD_CURSOR_YL, (y & 0xff));
    writeByte (LCD_CURSOR_YM, (y >> 8));
}

/*-----------------------------------------------------------------------------
 * video_init_hw_cursor --
 *-----------------------------------------------------------------------------
 */
void video_init_hw_cursor (int font_width, int font_height)
{
    volatile unsigned char *ptr;
    unsigned char pattern;
    int i;


    /* Init cursor content
       Cursor size is 64x64 pixels
       Start of the cursor memory depends on panel type (dual panel ...)     */
    if ((i = readByte (LCD_CURSOR_START)) == 0) {
	ptr = (unsigned char *)(sed13806.frameAdrs + DEFAULT_VIDEO_MEMORY_SIZE - HWCURSORSIZE);
    }
    else {
	ptr = (unsigned char *)(sed13806.frameAdrs + DEFAULT_VIDEO_MEMORY_SIZE - (i * 8192));
    }

    /* Fill the first line and the first empty line after cursor             */
    for (i = 0, pattern = 0; i < 64; i++) {
	if (i < font_width) {
	    /* Invert background                                             */
	    pattern |= 0x3;

	}
	else {
	    /* Background                                                    */
	    pattern |= 0x2;
	}
	if ((i & 3) == 3) {
	    *ptr = pattern;
	    *(ptr + font_height * 16) = 0xaa;
	    ptr ++;
	    pattern = 0;
	}
	pattern <<= 2;
    }

    /* Duplicate this line                                                   */
    for (i = 1; i < font_height; i++) {
	memcpy ((void *)ptr, (void *)(ptr - 16), 16);
	ptr += 16;
    }

    for (; i < 64; i++) {
	memcpy ((void *)(ptr + 16), (void *)ptr, 16);
	ptr += 16;
    }

    /* Select cursor mode                                                    */
    writeByte (LCD_CURSOR_CNTL, 1);
}
#endif
