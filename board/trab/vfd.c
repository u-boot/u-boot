/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering -- wd@denx.de
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

/************************************************************************/
/* ** DEBUG SETTINGS							*/
/************************************************************************/

/* #define DEBUG	*/

/************************************************************************/
/* ** HEADER FILES							*/
/************************************************************************/

#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <s3c2400.h>

#ifdef CONFIG_VFD

/************************************************************************/
/* ** CONFIG STUFF -- should be moved to board config file		*/
/************************************************************************/

/************************************************************************/

#ifndef PAGE_SIZE
#define	PAGE_SIZE	4096
#endif

#define ROT	0x09
#define BLAU	0x0C
#define VIOLETT	0X0D

ulong vfdbase;
ulong frame_buf_size;
#define frame_buf_offs 4

/* taken from armboot/common/vfd.c */
ulong         adr_vfd_table[112][18][2][4][2];
unsigned char bit_vfd_table[112][18][2][4][2];

/*
 * initialize the values for the VFD-grid-control in the framebuffer
 */
void init_grid_ctrl(void)
{
	ulong adr, grid_cycle;
	unsigned int bit, display;
	unsigned char temp, bit_nr;

	for (adr=vfdbase; adr<=(vfdbase+7168); adr+=4) /*clear frame buffer */
		(*(volatile ulong*)(adr))=0;

	for(display=0;display<=3;display++)
	{
		for(grid_cycle=0;grid_cycle<=55;grid_cycle++)
		{
			bit = grid_cycle*256*4+(grid_cycle+200)*4+frame_buf_offs+display;
 			/* wrap arround if offset (see manual S3C2400) */
			if (bit>=frame_buf_size*8)
				bit = bit-(frame_buf_size*8);
			adr = vfdbase+(bit/32)*4+(3-(bit%32)/8);
			bit_nr = bit%8;
			bit_nr = (bit_nr>3)?bit_nr-4:bit_nr+4;
			temp=(*(volatile unsigned char*)(adr));
			temp|=(1<<bit_nr);
			(*(volatile unsigned char*)(adr))=temp;

			if(grid_cycle<55)
				bit = grid_cycle*256*4+(grid_cycle+201)*4+frame_buf_offs+display;
			else
				bit = grid_cycle*256*4+200*4+frame_buf_offs+display-4; 	/* grid nr. 0 */
			/* wrap arround if offset (see manual S3C2400) */
			if (bit>=frame_buf_size*8)
				bit = bit-(frame_buf_size*8);
			adr = vfdbase+(bit/32)*4+(3-(bit%32)/8);
			bit_nr = bit%8;
			bit_nr = (bit_nr>3)?bit_nr-4:bit_nr+4;
			temp=(*(volatile unsigned char*)(adr));
			temp|=(1<<bit_nr);
			(*(volatile unsigned char*)(adr))=temp;
		}
	}
}

/*
 *create translation table for getting easy the right position in the
 *physical framebuffer for some x/y-coordinates of the VFDs
 */
void create_vfd_table(void)
{
	unsigned int vfd_table[112][18][2][4][2];
	ulong adr;
	unsigned int x, y, color, display, entry, pixel, bit_nr;

	/*
	 * Create translation table for Noritake-T119C-VFD-specific
	 * organized frame-buffer.
	 * Created is the number of the bit in the framebuffer (the
	 * first transferred pixel of each frame is bit 0).
	 */
	for(y=0;y<=17;y++)   /* Zeile */
	{
		for(x=0;x<=111;x++)  /* Spalten */
		{
			/*Display 0 blaue Pixel Eintrag 1 */
			vfd_table[x][y][0][0][0]=((x%4)*4+y*16+(x/4)*2048);
			/*Display 0 rote Pixel Eintrag 1 */
			vfd_table[x][y][1][0][0]=((x%4)*4+y*16+(x/4)*2048+512);
			if(x<=1)
			{
				/*Display 0 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][0][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+1024);
				/*Display 0 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][0][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+512+1024);
			}
			else
			{
				/*Display 0 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][0][1]=((x%4)*4+y*16+((x-2)/4)*2048+1024);
				/*Display 0 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][0][1]=((x%4)*4+y*16+((x-2)/4)*2048+512+1024);
			}
			/*Display 1 blaue Pixel Eintrag 1 */
			vfd_table[x][y][0][1][0]=((x%4)*4+y*16+(x/4)*2048+1);
			/*Display 1 rote Pixel Eintrag 1 */
			vfd_table[x][y][1][1][0]=((x%4)*4+y*16+(x/4)*2048+512+1);
			if(x<=1)
			{
				/*Display 1 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][1][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+1+1024);
				/*Display 1 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][1][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+512+1+1024);
			}
			else
			{
				/*Display 1 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][1][1]=((x%4)*4+y*16+((x-2)/4)*2048+1+1024);
				/*Display 1 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][1][1]=((x%4)*4+y*16+((x-2)/4)*2048+512+1+1024);
			}
			/*Display 2 blaue Pixel Eintrag 1 */
			vfd_table[x][y][0][2][0]=((x%4)*4+y*16+(x/4)*2048+2);
			/*Display 2 rote Pixel Eintrag 1 */
			vfd_table[x][y][1][2][0]=((x%4)*4+y*16+(x/4)*2048+512+2);
			if(x<=1)
			{
				/*Display 2 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][2][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+2+1024);
				/*Display 2 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][2][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+512+2+1024);
			}
			else
			{
				/*Display 2 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][2][1]=((x%4)*4+y*16+((x-2)/4)*2048+2+1024);
				/*Display 2 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][2][1]=((x%4)*4+y*16+((x-2)/4)*2048+512+2+1024);
			}
			/*Display 3 blaue Pixel Eintrag 1 */
			vfd_table[x][y][0][3][0]=((x%4)*4+y*16+(x/4)*2048+3);
			/*Display 3 rote Pixel Eintrag 1 */
			vfd_table[x][y][1][3][0]=((x%4)*4+y*16+(x/4)*2048+512+3);
			if(x<=1)
			{
				/*Display 3 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][3][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+3+1024);
				/*Display 3 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][3][1]=(((x+112)%4)*4+y*16+((x+110)/4)*2048+512+3+1024);
			}
			else
			{
				/*Display 3 blaue Pixel Eintrag 2 */
				vfd_table[x][y][0][3][1]=((x%4)*4+y*16+((x-2)/4)*2048+3+1024);
				/*Display 3 rote Pixel Eintrag 2 */
				vfd_table[x][y][1][3][1]=((x%4)*4+y*16+((x-2)/4)*2048+512+3+1024);
			}
		}
	}

	/*
	 * Create translation table for Noritake-T119C-VFD-specific
	 * organized frame-buffer
	 * Create table with entries for physical byte adresses and
	 * bit-number within the byte
	 * from table with bit-numbers within the total framebuffer
	 */
	for(y=0;y<=17;y++)
	{
		for(x=0;x<=111;x++)
		{
			for(color=0;color<=1;color++)
			{
				for(display=0;display<=3;display++)
				{
					for(entry=0;entry<=1;entry++)
					{
						pixel  = vfd_table[x][y][color][display][entry] + frame_buf_offs;
						 /*
						  * wrap arround if offset
						  * (see manual S3C2400)
						  */
						if (pixel>=frame_buf_size*8)
							pixel = pixel-(frame_buf_size*8);
						adr    = vfdbase+(pixel/32)*4+(3-(pixel%32)/8);
						bit_nr = pixel%8;
						bit_nr = (bit_nr>3)?bit_nr-4:bit_nr+4;
						adr_vfd_table[x][y][color][display][entry] = adr;
						bit_vfd_table[x][y][color][display][entry] = bit_nr;
					}
				}
			}
		}
	}
}

/*
 * Set/clear pixel of the VFDs
 */
void set_vfd_pixel(unsigned char x, unsigned char y, unsigned char color, unsigned char display, unsigned char value)
{
	ulong adr;
	unsigned char bit_nr, temp;

	if (value!=0)
	{
		/* Pixel-Eintrag Nr. 1 */
		adr = adr_vfd_table[x][y][color][display][0];
		/* Pixel-Eintrag Nr. 1 */
		bit_nr = bit_vfd_table[x][y][color][display][0];
		temp=(*(volatile unsigned char*)(adr));
		temp|=1<<bit_nr;
		(*(volatile unsigned char*)(adr))=temp;

		/* Pixel-Eintrag Nr. 2 */
		adr = adr_vfd_table[x][y][color][display][1];
		/* Pixel-Eintrag Nr. 2 */
		bit_nr = bit_vfd_table[x][y][color][display][1];
		temp=(*(volatile unsigned char*)(adr));
		temp|=1<<bit_nr;
		(*(volatile unsigned char*)(adr))=temp;
	}
	else
	{
		/* Pixel-Eintrag Nr. 1 */
		adr = adr_vfd_table[x][y][color][display][0];
		/* Pixel-Eintrag Nr. 1 */
		bit_nr = bit_vfd_table[x][y][color][display][0];
		temp=(*(volatile unsigned char*)(adr));
		temp&=~(1<<bit_nr);
		(*(volatile unsigned char*)(adr))=temp;

		/* Pixel-Eintrag Nr. 2 */
		adr = adr_vfd_table[x][y][color][display][1];
		/* Pixel-Eintrag Nr. 2 */
		bit_nr = bit_vfd_table[x][y][color][display][1];
		temp=(*(volatile unsigned char*)(adr));
		temp&=~(1<<bit_nr);
		(*(volatile unsigned char*)(adr))=temp;
	}
}

/*
 * transfer image from BMP-File
 */
void transfer_pic(int display, unsigned char *adr, int height, int width)
{
	int x, y;
	unsigned char temp;

	for (; height > 0; height -= 18)
	{
		if (height > 18)
			y = 18;
		else
			y = height;
		for (; y > 0; y--)
		{
			for (x = 0; x < width; x += 2)
			{
				temp = *adr++;
				set_vfd_pixel(x, y-1, 0, display, 0);
				set_vfd_pixel(x, y-1, 1, display, 0);
				if ((temp >> 4) == BLAU)
					set_vfd_pixel(x, y-1, 0, display, 1);
				else if ((temp >> 4) == ROT)
					set_vfd_pixel(x, y-1, 1, display, 1);
				else if ((temp >> 4) == VIOLETT)
				{
					set_vfd_pixel(x, y-1, 0, display, 1);
					set_vfd_pixel(x, y-1, 1, display, 1);
				}
				set_vfd_pixel(x+1, y-1, 0, display, 0);
				set_vfd_pixel(x+1, y-1, 1, display, 0);
				if ((temp & 0x0F) == BLAU)
					set_vfd_pixel(x+1, y-1, 0, display, 1);
				else if ((temp & 0x0F) == ROT)
					set_vfd_pixel(x+1, y-1, 1, display, 1);
				else if ((temp & 0x0F) == VIOLETT)
				{
					set_vfd_pixel(x+1, y-1, 0, display, 1);
					set_vfd_pixel(x+1, y-1, 1, display, 1);
				}
			}
		}
		display++;
		if (display > 3)
			display = 0;
	}
}

/*
 * initialize LCD-Controller of the S3C2400 for using VFDs
 */
int drv_vfd_init(void)
{
	ulong palette;
	static int vfd_init_done = 0;

	DECLARE_GLOBAL_DATA_PTR;

	if (vfd_init_done != 0)
		return (0);
	vfd_init_done = 1;

	vfdbase = gd->fb_base;
	create_vfd_table();
	init_grid_ctrl();

	/*
	 * Hinweis: Der Framebuffer ist um genau ein Nibble verschoben
	 * Das erste angezeigte Pixel wird aus dem zweiten Nibble geholt
	 * das letzte angezeigte Pixel wird aus dem ersten Nibble geholt
	 * (wrap around)
	 * see manual S3C2400
	 */
	/* frame buffer startadr */
	rLCDSADDR1 = vfdbase >> 1;
 	/* frame buffer endadr */
	rLCDSADDR2 = (vfdbase + frame_buf_size) >> 1;
	rLCDSADDR3 = ((256/4));

	/* Port-Pins als LCD-Ausgang */
	rPCCON =   (rPCCON & 0xFFFFFF00)| 0x000000AA;
	/* Port-Pins als LCD-Ausgang */
	rPDCON =   (rPDCON & 0xFFFFFF03)| 0x000000A8;
#ifdef WITH_VFRAME
	/* mit VFRAME zum Messen */
	rPDCON =   (rPDCON & 0xFFFFFF00)| 0x000000AA;
#endif

	rLCDCON2 = 0x000DC000;
	rLCDCON3 = 0x0051000A;
	rLCDCON4 = 0x00000001;
	rLCDCON5 = 0x00000440;
	rLCDCON1 = 0x00000B75;

	debug ("LCDSADDR1: %lX\n", rLCDSADDR1);
	debug ("LCDSADDR2: %lX\n", rLCDSADDR2);
	debug ("LCDSADDR3: %lX\n", rLCDSADDR3);

	for(palette=0;palette<=15;palette++)
		(*(volatile unsigned int*)(PALETTE+(palette*4)))=palette;
	for(palette=16;palette<=255;palette++)
		(*(volatile unsigned int*)(PALETTE+(palette*4)))=0x00;

	return 0;
}

/************************************************************************/
/* ** ROM capable initialization part - needed to reserve FB memory	*/
/************************************************************************/

/*
 * This is called early in the system initialization to grab memory
 * for the VFD controller.
 *
 * Note that this is running from ROM, so no write access to global data.
 */
ulong vfd_setmem (ulong addr)
{
	ulong size;

	/* MAGIC */
	frame_buf_size = (256*4*56)/8;

	/* Round up to nearest full page */
	size = (frame_buf_size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	debug ("Reserving %ldk for VFD Framebuffer at: %08lx\n", size>>10, addr);

	return (size);
}

#endif /* CONFIG_VFD */
