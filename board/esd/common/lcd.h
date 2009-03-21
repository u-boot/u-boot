/*
 * (C) Copyright 2003-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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

/*
 * Neutralize little endians.
 */
#define SWAP_LONG(data) ((unsigned long)                                  \
		         (((unsigned long)(data) >> 24)                 | \
			  ((unsigned long)(data)  << 24)                | \
			  (((unsigned long)(data) >> 8) & 0x0000ff00 )  | \
			   (((unsigned long)(data) << 8) & 0x00ff0000 )))
#define SWAP_SHORT(data) ((unsigned short)                                \
			  (((unsigned short)(data) >> 8 )  |              \
			   ((unsigned short)(data) << 8 )))
#define LOAD_LONG(data)   SWAP_LONG(data)
#define LOAD_SHORT(data)  SWAP_SHORT(data)

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define S1D_WRITE_PALETTE(p,i,r,g,b)					\
	{								\
		out_8(&((uchar*)(p))[palette_index], (uchar)(i));	\
		out_8(&((uchar*)(p))[palette_index], (uchar)(r));	\
		out_8(&((uchar*)(p))[palette_index], (uchar)(g));	\
		out_8(&((uchar*)(p))[palette_index], (uchar)(b));	\
	}

typedef struct
{
    ushort Index;
    uchar  Value;
} S1D_REGS;

typedef struct                       /**** BMP file info structure ****/
{
	unsigned int   biSize;           /* Size of info header */
	int            biWidth;          /* Width of image */
	int            biHeight;         /* Height of image */
	unsigned short biPlanes;         /* Number of color planes */
	unsigned short biBitCount;       /* Number of bits per pixel */
	unsigned int   biCompression;    /* Type of compression to use */
	unsigned int   biSizeImage;      /* Size of image data */
	int            biXPelsPerMeter;  /* X pixels per meter */
	int            biYPelsPerMeter;  /* Y pixels per meter */
	unsigned int   biClrUsed;        /* Number of colors used */
	unsigned int   biClrImportant;   /* Number of important colors */
} BITMAPINFOHEADER;
