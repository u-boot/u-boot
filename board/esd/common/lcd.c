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

#include "lcd.h"


int palette_index;
int palette_value;


#ifdef CFG_LCD_ENDIAN
void lcd_setup(int lcd, int config)
{
	if (lcd == 0) {
		/*
		 * Set endianess and reset lcd controller 0 (small)
		 */
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_LCD0_RST); /* set reset to low */
		udelay(10); /* wait 10us */
		if (config == 1) {
			out32(GPIO0_OR, in32(GPIO0_OR) | CFG_LCD_ENDIAN); /* big-endian */
		} else {
			out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_LCD_ENDIAN); /* little-endian */
		}
		udelay(10); /* wait 10us */
		out32(GPIO0_OR, in32(GPIO0_OR) | CFG_LCD0_RST); /* set reset to high */
	} else {
		/*
		 * Set endianess and reset lcd controller 1 (big)
		 */
		out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_LCD1_RST); /* set reset to low */
		udelay(10); /* wait 10us */
		if (config == 1) {
			out32(GPIO0_OR, in32(GPIO0_OR) | CFG_LCD_ENDIAN); /* big-endian */
		} else {
			out32(GPIO0_OR, in32(GPIO0_OR) & ~CFG_LCD_ENDIAN); /* little-endian */
		}
		udelay(10); /* wait 10us */
		out32(GPIO0_OR, in32(GPIO0_OR) | CFG_LCD1_RST); /* set reset to high */
	}

	/*
	 * CFG_LCD_ENDIAN may also be FPGA_RESET, so set inactive
	 */
	out32(GPIO0_OR, in32(GPIO0_OR) | CFG_LCD_ENDIAN); /* set reset high again */
}
#endif /* #ifdef CFG_LCD_ENDIAN */


void lcd_init(uchar *lcd_reg, uchar *lcd_mem, S1D_REGS *regs, int reg_count,
	      uchar *logo_bmp, ulong len)
{
	int i;
	ushort s1dReg;
	uchar s1dValue;
	uchar *ptr;
	ushort *ptr2;
	ushort val;
	unsigned char *dst;
	int x, y;
	int width, height, bpp, colors, line_size;
	int header_size;
	unsigned char *bmp;
	unsigned char r, g, b;
	BITMAPINFOHEADER *bm_info;
	int reg_byte_swap;

	/*
	 * Detect epson
	 */
	if (lcd_reg[0] == 0x1c) {
		/*
		 * Big epson detected
		 */
		reg_byte_swap = FALSE;
		palette_index = 0x1e2;
		palette_value = 0x1e4;
		puts("LCD:   S1D13806");
	} else if (lcd_reg[1] == 0x1c) {
		/*
		 * Big epson detected (with register swap bug)
		 */
		reg_byte_swap = TRUE;
		palette_index = 0x1e3;
		palette_value = 0x1e5;
		puts("LCD:   S1D13806S");
	} else if (lcd_reg[0] == 0x18) {
		/*
		 * Small epson detected (704)
		 */
		reg_byte_swap = FALSE;
		palette_index = 0x15;
		palette_value = 0x17;
		puts("LCD:   S1D13704");
	} else if (lcd_reg[0x10000] == 0x24) {
		/*
		 * Small epson detected (705)
		 */
		reg_byte_swap = FALSE;
		palette_index = 0x15;
		palette_value = 0x17;
		lcd_reg += 0x10000; /* add offset for 705 regs */
		puts("LCD:   S1D13705");
	} else {
		puts("LCD:   No controller detected!\n");
		return;
	}

	for (i = 0; i<reg_count; i++) {
		s1dReg = regs[i].Index;
		if (reg_byte_swap) {
			if ((s1dReg & 0x0001) == 0)
				s1dReg |= 0x0001;
			else
				s1dReg &= ~0x0001;
		}
		s1dValue = regs[i].Value;
		lcd_reg[s1dReg] = s1dValue;
	}

	/*
	 * Decompress bmp image
	 */
	dst = malloc(CFG_LCD_LOGO_MAX_SIZE);
	if (gunzip(dst, CFG_LCD_LOGO_MAX_SIZE, (uchar *)logo_bmp, &len) != 0) {
		return;
	}

	/*
	 * Check for bmp mark 'BM'
	 */
	if (*(ushort *)dst != 0x424d) {
		printf("LCD: Unknown image format!\n");
		free(dst);
		return;
	}

	/*
	 * Get image info from bmp-header
	 */
	bm_info = (BITMAPINFOHEADER *)(dst + 14);
	bpp = LOAD_SHORT(bm_info->biBitCount);
	width = LOAD_LONG(bm_info->biWidth);
	height = LOAD_LONG(bm_info->biHeight);
	switch (bpp) {
	case 1:
		colors = 1;
		line_size = width >> 3;
		break;
	case 4:
		colors = 16;
		line_size = width >> 1;
		break;
	case 8:
		colors = 256;
		line_size = width;
		break;
	case 24:
		colors = 0;
		line_size = width * 3;
		break;
	default:
		printf("LCD: Unknown bpp (%d) im image!\n", bpp);
		free(dst);
		return;
	}
	printf(" (%d*%d, %dbpp)\n", width, height, bpp);

	/*
	 * Write color palette
	 */
	if (colors <= 256) {
		ptr = (unsigned char *)(dst + 14 + 40);
		for (i=0; i<colors; i++) {
			b = *ptr++;
			g = *ptr++;
			r = *ptr++;
			ptr++;
			S1D_WRITE_PALETTE(lcd_reg, i, r, g, b);
		}
	}

	/*
	 * Write bitmap data into framebuffer
	 */
	ptr = lcd_mem;
	ptr2 = (ushort *)lcd_mem;
	header_size = 14 + 40 + 4*colors;          /* skip bmp header */
	for (y=0; y<height; y++) {
		bmp = &dst[(height-1-y)*line_size + header_size];
		if (bpp == 24) {
			for (x=0; x<width; x++) {
				/*
				 * Generate epson 16bpp fb-format from 24bpp image
				 */
				b = *bmp++ >> 3;
				g = *bmp++ >> 2;
				r = *bmp++ >> 3;
				val = ((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f);
				*ptr2++ = val;
			}
		} else {
			for (x=0; x<line_size; x++) {
				*ptr++ = *bmp++;
			}
		}
	}

	free(dst);
}
