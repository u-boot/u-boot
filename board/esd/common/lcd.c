/*
 * (C) Copyright 2003-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#include "asm/io.h"
#include "lcd.h"


extern int video_display_bitmap (ulong, int, int);


int palette_index;
int palette_value;
int lcd_depth;
unsigned char *glob_lcd_reg;
unsigned char *glob_lcd_mem;

#if defined(CONFIG_SYS_LCD_ENDIAN)
void lcd_setup(int lcd, int config)
{
	if (lcd == 0) {
		/*
		 * Set endianess and reset lcd controller 0 (small)
		 */

		/* set reset to low */
		out_be32((void*)GPIO0_OR,
			 in_be32((void*)GPIO0_OR) & ~CONFIG_SYS_LCD0_RST);
		udelay(10); /* wait 10us */
		if (config == 1) {
			/* big-endian */
			out_be32((void*)GPIO0_OR,
				 in_be32((void*)GPIO0_OR) | CONFIG_SYS_LCD_ENDIAN);
		} else {
			/* little-endian */
			out_be32((void*)GPIO0_OR,
				 in_be32((void*)GPIO0_OR) & ~CONFIG_SYS_LCD_ENDIAN);
		}
		udelay(10); /* wait 10us */
		/* set reset to high */
		out_be32((void*)GPIO0_OR,
			 in_be32((void*)GPIO0_OR) | CONFIG_SYS_LCD0_RST);
	} else {
		/*
		 * Set endianess and reset lcd controller 1 (big)
		 */

		/* set reset to low */
		out_be32((void*)GPIO0_OR,
			 in_be32((void*)GPIO0_OR) & ~CONFIG_SYS_LCD1_RST);
		udelay(10); /* wait 10us */
		if (config == 1) {
			/* big-endian */
			out_be32((void*)GPIO0_OR,
				 in_be32((void*)GPIO0_OR) | CONFIG_SYS_LCD_ENDIAN);
		} else {
			/* little-endian */
			out_be32((void*)GPIO0_OR,
				 in_be32((void*)GPIO0_OR) & ~CONFIG_SYS_LCD_ENDIAN);
		}
		udelay(10); /* wait 10us */
		/* set reset to high */
		out_be32((void*)GPIO0_OR,
			 in_be32((void*)GPIO0_OR) | CONFIG_SYS_LCD1_RST);
	}

	/*
	 * CONFIG_SYS_LCD_ENDIAN may also be FPGA_RESET, so set inactive
	 */
	out_be32((void*)GPIO0_OR, in_be32((void*)GPIO0_OR) | CONFIG_SYS_LCD_ENDIAN);
}
#endif /* CONFIG_SYS_LCD_ENDIAN */


int lcd_bmp(uchar *logo_bmp)
{
	int i;
	uchar *ptr;
	ushort *ptr2;
	ushort val;
	unsigned char *dst = NULL;
	int x, y;
	int width, height, bpp, colors, line_size;
	int header_size;
	unsigned char *bmp;
	unsigned char r, g, b;
	BITMAPINFOHEADER *bm_info;
	ulong len;

	/*
	 * Check for bmp mark 'BM'
	 */
	if (*(ushort *)logo_bmp != 0x424d) {
		/*
		 * Decompress bmp image
		 */
		len = CONFIG_SYS_VIDEO_LOGO_MAX_SIZE;
		dst = malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
		if (dst == NULL) {
			printf("Error: malloc for gunzip failed!\n");
			return 1;
		}
		if (gunzip(dst, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE,
			   (uchar *)logo_bmp, &len) != 0) {
			free(dst);
			return 1;
		}
		if (len == CONFIG_SYS_VIDEO_LOGO_MAX_SIZE) {
			printf("Image could be truncated"
			       " (increase CONFIG_SYS_VIDEO_LOGO_MAX_SIZE)!\n");
		}

		/*
		 * Check for bmp mark 'BM'
		 */
		if (*(ushort *)dst != 0x424d) {
			printf("LCD: Unknown image format!\n");
			free(dst);
			return 1;
		}
	} else {
		/*
		 * Uncompressed BMP image, just use this pointer
		 */
		dst = (uchar *)logo_bmp;
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
		if ((dst != NULL) && (dst != (uchar *)logo_bmp))
			free(dst);
		return 1;
	}
	printf(" (%d*%d, %dbpp)\n", width, height, bpp);

	/*
	 * Write color palette
	 */
	if ((colors <= 256) && (lcd_depth <= 8)) {
		ptr = (unsigned char *)(dst + 14 + 40);
		for (i = 0; i < colors; i++) {
			b = *ptr++;
			g = *ptr++;
			r = *ptr++;
			ptr++;
			S1D_WRITE_PALETTE(glob_lcd_reg, i, r, g, b);
		}
	}

	/*
	 * Write bitmap data into framebuffer
	 */
	ptr = glob_lcd_mem;
	ptr2 = (ushort *)glob_lcd_mem;
	header_size = 14 + 40 + 4*colors;          /* skip bmp header */
	for (y = 0; y < height; y++) {
		bmp = &dst[(height-1-y)*line_size + header_size];
		if (lcd_depth == 16) {
			if (bpp == 24) {
				for (x = 0; x < width; x++) {
					/*
					 * Generate epson 16bpp fb-format
					 * from 24bpp image
					 */
					b = *bmp++ >> 3;
					g = *bmp++ >> 2;
					r = *bmp++ >> 3;
					val = ((r & 0x1f) << 11) |
						((g & 0x3f) << 5) |
						(b & 0x1f);
					*ptr2++ = val;
				}
			} else if (bpp == 8) {
				for (x = 0; x < line_size; x++) {
					/* query rgb value from palette */
					ptr = (unsigned char *)(dst + 14 + 40);
					ptr += (*bmp++) << 2;
					b = *ptr++ >> 3;
					g = *ptr++ >> 2;
					r = *ptr++ >> 3;
					val = ((r & 0x1f) << 11) |
						((g & 0x3f) << 5) |
						(b & 0x1f);
					*ptr2++ = val;
				}
			}
		} else {
			for (x = 0; x < line_size; x++)
				*ptr++ = *bmp++;
		}
	}

	if ((dst != NULL) && (dst != (uchar *)logo_bmp))
		free(dst);
	return 0;
}


int lcd_init(uchar *lcd_reg, uchar *lcd_mem, S1D_REGS *regs, int reg_count,
	     uchar *logo_bmp, ulong len)
{
	int i;
	ushort s1dReg;
	uchar s1dValue;
	int reg_byte_swap;

	/*
	 * Detect epson
	 */
	out_8(&lcd_reg[0], 0x00);
	out_8(&lcd_reg[1], 0x00);

	if (in_8(&lcd_reg[0]) == 0x1c) {
		/*
		 * Big epson detected
		 */
		reg_byte_swap = FALSE;
		palette_index = 0x1e2;
		palette_value = 0x1e4;
		lcd_depth = 16;
		puts("LCD:   S1D13806");
	} else if (in_8(&lcd_reg[1]) == 0x1c) {
		/*
		 * Big epson detected (with register swap bug)
		 */
		reg_byte_swap = TRUE;
		palette_index = 0x1e3;
		palette_value = 0x1e5;
		lcd_depth = 16;
		puts("LCD:   S1D13806S");
	} else if (in_8(&lcd_reg[0]) == 0x18) {
		/*
		 * Small epson detected (704)
		 */
		reg_byte_swap = FALSE;
		palette_index = 0x15;
		palette_value = 0x17;
		lcd_depth = 8;
		puts("LCD:   S1D13704");
	} else if (in_8(&lcd_reg[0x10000]) == 0x24) {
		/*
		 * Small epson detected (705)
		 */
		reg_byte_swap = FALSE;
		palette_index = 0x15;
		palette_value = 0x17;
		lcd_depth = 8;
		lcd_reg += 0x10000; /* add offset for 705 regs */
		puts("LCD:   S1D13705");
	} else {
		out_8(&lcd_reg[0x1a], 0x00);
		udelay(1000);
		if (in_8(&lcd_reg[1]) == 0x0c) {
			/*
			 * S1D13505 detected
			 */
			reg_byte_swap = TRUE;
			palette_index = 0x25;
			palette_value = 0x27;
			lcd_depth = 16;

			puts("LCD:   S1D13505");
		} else {
			puts("LCD:   No controller detected!\n");
			return 1;
		}
	}

	/*
	 * Setup lcd controller regs
	 */
	for (i = 0; i < reg_count; i++) {
		s1dReg = regs[i].Index;
		if (reg_byte_swap) {
			if ((s1dReg & 0x0001) == 0)
				s1dReg |= 0x0001;
			else
				s1dReg &= ~0x0001;
		}
		s1dValue = regs[i].Value;
		out_8(&lcd_reg[s1dReg], s1dValue);
	}

	/*
	 * Save reg & mem pointer for later usage (e.g. bmp command)
	 */
	glob_lcd_reg = lcd_reg;
	glob_lcd_mem = lcd_mem;

	/*
	 * Display bmp image
	 */
	return lcd_bmp(logo_bmp);
}

int do_esdbmp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong addr;
#ifdef CONFIG_VIDEO_SM501
	char *str;
#endif
	if (argc != 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

#ifdef CONFIG_VIDEO_SM501
	str = getenv("bd_type");
	if ((strcmp(str, "ppc221") == 0) || (strcmp(str, "ppc231") == 0)) {
		/*
		 * SM501 available, use standard bmp command
		 */
		return video_display_bitmap(addr, 0, 0);
	} else {
		/*
		 * No SM501 available, use esd epson bmp command
		 */
		return lcd_bmp((uchar *)addr);
	}
#else
	return lcd_bmp((uchar *)addr);
#endif
}

U_BOOT_CMD(
	esdbmp,	2,	1,	do_esdbmp,
	"display BMP image",
	"<imageAddr> - display image"
);
