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
#include <stdio_dev.h>
#include <asm/arch/s3c24x0_cpu.h>

DECLARE_GLOBAL_DATA_PTR;

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

/* MAGIC */
#define	FRAME_BUF_SIZE	((256*4*56)/8)
#define frame_buf_offs 4

/* defines for starting Timer3 as CPLD-Clk */
#define START3			(1 << 16)
#define UPDATE3			(1 << 17)
#define INVERT3			(1 << 18)
#define RELOAD3			(1 << 19)

/* CPLD-Register for controlling vfd-blank-signal */
#define VFD_DISABLE	(*(volatile uchar *)0x04038000=0x0000)
#define VFD_ENABLE	(*(volatile uchar *)0x04038000=0x0001)

/* Supported VFD Types */
#define	VFD_TYPE_T119C		1	/* Noritake T119C VFD */
#define	VFD_TYPE_MN11236	2

/*#define NEW_CPLD_CLK*/

int vfd_board_id;

/* taken from armboot/common/vfd.c */
unsigned long adr_vfd_table[112][18][2][4][2];
unsigned char bit_vfd_table[112][18][2][4][2];

/*
 * initialize the values for the VFD-grid-control in the framebuffer
 */
void init_grid_ctrl(void)
{
	ulong adr, grid_cycle;
	unsigned int bit, display;
	unsigned char temp, bit_nr;

	/*
	 * clear frame buffer (logical clear => set to "black")
	 */
	memset ((void *)(gd->fb_base), 0, FRAME_BUF_SIZE);

	switch (gd->vfd_type) {
	case VFD_TYPE_T119C:
	    for (display=0; display<4; display++) {
		for(grid_cycle=0; grid_cycle<56; grid_cycle++) {
			bit = grid_cycle * 256  * 4 +
			     (grid_cycle + 200) * 4 +
			     frame_buf_offs + display;
			/* wrap arround if offset (see manual S3C2400) */
			if (bit>=FRAME_BUF_SIZE*8)
				bit = bit - (FRAME_BUF_SIZE * 8);
			adr = gd->fb_base + (bit/32) * 4 + (3 - (bit%32) / 8);
			bit_nr = bit % 8;
			bit_nr = (bit_nr > 3) ? bit_nr-4 : bit_nr+4;
			temp=(*(volatile unsigned char*)(adr));
			temp |=  (1<<bit_nr);
			(*(volatile unsigned char*)(adr))=temp;

			if(grid_cycle<55)
				bit = grid_cycle*256*4+(grid_cycle+201)*4+frame_buf_offs+display;
			else
				bit = grid_cycle*256*4+200*4+frame_buf_offs+display-4;	/* grid nr. 0 */
			/* wrap arround if offset (see manual S3C2400) */
			if (bit>=FRAME_BUF_SIZE*8)
				bit = bit-(FRAME_BUF_SIZE*8);
			adr = gd->fb_base+(bit/32)*4+(3-(bit%32)/8);
			bit_nr = bit%8;
			bit_nr = (bit_nr>3)?bit_nr-4:bit_nr+4;
			temp=(*(volatile unsigned char*)(adr));
			temp |=  (1<<bit_nr);
			(*(volatile unsigned char*)(adr))=temp;
		}
	    }
	    break;
	case VFD_TYPE_MN11236:
	    for (display=0; display<4; display++) {
		for (grid_cycle=0; grid_cycle<38; grid_cycle++) {
			bit = grid_cycle * 256  * 4 +
			     (253 - grid_cycle) * 4 +
			     frame_buf_offs + display;
			/* wrap arround if offset (see manual S3C2400) */
			if (bit>=FRAME_BUF_SIZE*8)
				bit = bit - (FRAME_BUF_SIZE * 8);
			adr = gd->fb_base + (bit/32) * 4 + (3 - (bit%32) / 8);
			bit_nr = bit % 8;
			bit_nr = (bit_nr > 3) ? bit_nr-4 : bit_nr+4;
			temp=(*(volatile unsigned char*)(adr));
			temp |=  (1<<bit_nr);
			(*(volatile unsigned char*)(adr))=temp;

			if(grid_cycle<37)
				bit = grid_cycle*256*4+(252-grid_cycle)*4+frame_buf_offs+display;

			/* wrap arround if offset (see manual S3C2400) */
			if (bit>=FRAME_BUF_SIZE*8)
				bit = bit-(FRAME_BUF_SIZE*8);
			adr = gd->fb_base+(bit/32)*4+(3-(bit%32)/8);
			bit_nr = bit%8;
			bit_nr = (bit_nr>3)?bit_nr-4:bit_nr+4;
			temp=(*(volatile unsigned char*)(adr));
			temp |=  (1<<bit_nr);
			(*(volatile unsigned char*)(adr))=temp;
		}
	    }
	    break;
	default:
	    printf ("Warning: unknown display type\n");
	    break;
	}
}

/*
 *create translation table for getting easy the right position in the
 *physical framebuffer for some x/y-coordinates of the VFDs
 */
void create_vfd_table(void)
{
	unsigned long vfd_table[112][18][2][4][2];
	unsigned int x, y, color, display, entry, pixel;
	unsigned int x_abcdef = 0;

	switch (gd->vfd_type) {
	case VFD_TYPE_T119C:
	    for(y=0; y<=17; y++) {	/* Line */
		for(x=0; x<=111; x++) {	/* Column */
		    for(display=0; display <=3; display++) {

			    /* Display 0 blue pixels */
			    vfd_table[x][y][0][display][0] =
				(x==0) ? y*16+display
				       : (x%4)*4+y*16+((x-1)/2)*1024+display;
			    /* Display 0 red pixels */
			    vfd_table[x][y][1][display][0] =
				(x==0) ? y*16+512+display
				       : (x%4)*4+y*16+((x-1)/2)*1024+512+display;
		    }
		}
	    }
	    break;
	case VFD_TYPE_MN11236:
	    for(y=0; y<=17; y++) {	/* Line */
		for(x=0; x<=111; x++) {	/* Column */
		    for(display=0; display <=3; display++) {

			    vfd_table[x][y][0][display][0]=0;
			    vfd_table[x][y][0][display][1]=0;
			    vfd_table[x][y][1][display][0]=0;
			    vfd_table[x][y][1][display][1]=0;

			    switch (x%6) {
			    case 0: x_abcdef=0; break; /* a -> a */
			    case 1: x_abcdef=2; break; /* b -> c */
			    case 2: x_abcdef=4; break; /* c -> e */
			    case 3: x_abcdef=5; break; /* d -> f */
			    case 4: x_abcdef=3; break; /* e -> d */
			    case 5: x_abcdef=1; break; /* f -> b */
			    }

			    /* blue pixels */
			    vfd_table[x][y][0][display][0] =
				(x>1) ? x_abcdef*4+((x-1)/3)*1024+y*48+display
				      : x_abcdef*4+             0+y*48+display;
			    /* blue pixels */
			    if (x>1 && (x-1)%3)
				    vfd_table[x][y][0][display][1] = x_abcdef*4+((x-1)/3+1)*1024+y*48+display;

			    /* red pixels */
			    vfd_table[x][y][1][display][0] =
				(x>1) ? x_abcdef*4+24+((x-1)/3)*1024+y*48+display
				      : x_abcdef*4+24+             0+y*48+display;
			    /* red pixels */
			    if (x>1 && (x-1)%3)
				    vfd_table[x][y][1][display][1] = x_abcdef*4+24+((x-1)/3+1)*1024+y*48+display;
		    }
		}
	    }
	    break;
	default:
	    /* do nothing */
	    return;
	}

	/*
	 * Create table with entries for physical byte adresses and
	 * bit-number within the byte
	 * from table with bit-numbers within the total framebuffer
	 */
	for(y=0;y<18;y++) {
	    for(x=0;x<112;x++) {
		for(color=0;color<2;color++) {
		    for(display=0;display<4;display++) {
			for(entry=0;entry<2;entry++) {
			    unsigned long adr  = gd->fb_base;
			    unsigned int bit_nr = 0;

			    pixel  = vfd_table[x][y][color][display][entry] + frame_buf_offs;
			    /*
			     * wrap arround if offset
			     * (see manual S3C2400)
			     */
			    if (pixel>=FRAME_BUF_SIZE*8)
				    pixel = pixel-(FRAME_BUF_SIZE*8);
			    adr    = gd->fb_base+(pixel/32)*4+(3-(pixel%32)/8);
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
void set_vfd_pixel(unsigned char x, unsigned char y,
		   unsigned char color, unsigned char display,
		   unsigned char value)
{
	ulong adr;
	unsigned char bit_nr, temp;

	if (! gd->vfd_type) {
		/* Unknown type. */
		return;
	}

	/* Pixel-Eintrag Nr. 1 */
	adr = adr_vfd_table[x][y][color][display][0];
	/* Pixel-Eintrag Nr. 1 */
	bit_nr = bit_vfd_table[x][y][color][display][0];
	temp=(*(volatile unsigned char*)(adr));

	if (value)
		temp |=  (1<<bit_nr);
	else
		temp &= ~(1<<bit_nr);

	(*(volatile unsigned char*)(adr))=temp;
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
		if (display > 0)
			display--;
		else
			display = 3;
	}
}

/*
 * This function initializes VFD clock that is needed for the CPLD that
 * manages the keyboard.
 */
int vfd_init_clocks (void)
{
	int i;

	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();
	struct s3c24x0_timers * const timers = s3c24x0_get_base_timers();
	struct s3c24x0_lcd * const lcd = s3c24x0_get_base_lcd();

	/* try to determine display type from the value
	 * defined by pull-ups
	 */
	gpio->PCUP = (gpio->PCUP & 0xFFF0);	/* activate  GPC0...GPC3 pullups */
	gpio->PCCON = (gpio->PCCON & 0xFFFFFF00);	/* configure GPC0...GPC3 as inputs */
	/* allow signals to settle */
	for (i=0; i<10000; i++)	/* udelay isn't working yet at this point! */
		__asm__("NOP");
	vfd_board_id = (~gpio->PCDAT) & 0x000F;	/* read GPC0...GPC3 port pins */

	VFD_DISABLE;				/* activate blank for the vfd */

#define	NEW_CPLD_CLK

#ifdef NEW_CPLD_CLK
	if (vfd_board_id) {
		/* If new board revision, then use PWM 3 as cpld-clock */
		/* Enable 500 Hz timer for fill level sensor to operate properly */
		/* Configure TOUT3 as functional pin, disable pull-up */
		gpio->PDCON &= ~0x30000;
		gpio->PDCON |= 0x20000;
		gpio->PDUP |= (1 << 8);

		/* Configure the prescaler */
		timers->TCFG0 &= ~0xff00;
		timers->TCFG0 |= 0x0f00;

		/* Select MUX input (divider) for timer3 (1/16) */
		timers->TCFG1 &= ~0xf000;
		timers->TCFG1 |= 0x3000;

		/* Enable autoreload and set the counter and compare
		 * registers to values for the 500 Hz clock
		 * (for a given  prescaler (15) and divider (16)):
		 * counter = (66000000 / 500) >> 9;
		 */
		timers->ch[3].TCNTB = 0x101;
		timers->ch[3].TCMPB = 0x101 / 2;

		/* Start timer */
		timers->TCON = (timers->TCON | UPDATE3 | RELOAD3) & ~INVERT3;
		timers->TCON = (timers->TCON | START3) & ~UPDATE3;
	}
#endif
	/* If old board revision, then use vm-signal as cpld-clock */
	lcd->LCDCON2 = 0x00FFC000;
	lcd->LCDCON3 = 0x0007FF00;
	lcd->LCDCON4 = 0x00000000;
	lcd->LCDCON5 = 0x00000400;
	lcd->LCDCON1 = 0x00000B75;
	/* VM (GPD1) is used as clock for the CPLD */
	gpio->PDCON = (gpio->PDCON & 0xFFFFFFF3) | 0x00000008;

	return 0;
}

/*
 * initialize LCD-Controller of the S3C2400 for using VFDs
 *
 * VFD detection depends on the board revision:
 * starting from Rev. 200 a type code can be read from the data pins,
 * driven by some pull-up resistors; all earlier systems must be
 * manually configured. The type is set in the "vfd_type" environment
 * variable.
 */
int drv_vfd_init(void)
{
	struct s3c24x0_lcd * const lcd = s3c24x0_get_base_lcd();
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();
	char *tmp;
	ulong palette;
	static int vfd_init_done = 0;
	int vfd_inv_data = 0;

	if (vfd_init_done != 0)
		return (0);
	vfd_init_done = 1;

	debug("Detecting Revison of WA4-VFD: ID=0x%X\n", vfd_board_id);

	switch (vfd_board_id) {
	case 0:			/* board revision < Rev.200 */
		if ((tmp = getenv ("vfd_type")) == NULL) {
			break;
		}
		if (strcmp(tmp, "T119C") == 0) {
			gd->vfd_type = VFD_TYPE_T119C;
		} else if (strcmp(tmp, "MN11236") == 0) {
			gd->vfd_type = VFD_TYPE_MN11236;
		} else {
			/* cannot use printf for a warning here */
			gd->vfd_type = 0;	/* unknown */
		}

		break;
	default:		/* default to MN11236, data inverted */
		gd->vfd_type = VFD_TYPE_MN11236;
		vfd_inv_data = 1;
		setenv ("vfd_type", "MN11236");
	}
	debug ("VFD type: %s%s\n",
		(gd->vfd_type == VFD_TYPE_T119C)   ? "T119C" :
		(gd->vfd_type == VFD_TYPE_MN11236) ? "MN11236" :
		"unknown",
		vfd_inv_data ? ", inverted data" : "");

	gd->fb_base = gd->fb_base;
	create_vfd_table();
	init_grid_ctrl();

	for (palette=0; palette < 16; palette++)
		(*(volatile unsigned int*)(PALETTE+(palette*4)))=palette;
	for (palette=16; palette < 256; palette++)
		(*(volatile unsigned int*)(PALETTE+(palette*4)))=0x00;

	/*
	 * Hinweis: Der Framebuffer ist um genau ein Nibble verschoben
	 * Das erste angezeigte Pixel wird aus dem zweiten Nibble geholt
	 * das letzte angezeigte Pixel wird aus dem ersten Nibble geholt
	 * (wrap around)
	 * see manual S3C2400
	 */
	/* Stopp LCD-Controller */
	lcd->LCDCON1 = 0x00000000;
	/* frame buffer startadr */
	lcd->LCDSADDR1 = gd->fb_base >> 1;
	/* frame buffer endadr */
	lcd->LCDSADDR2 = (gd->fb_base + FRAME_BUF_SIZE) >> 1;
	lcd->LCDSADDR3 = ((256/4));
	lcd->LCDCON2 = 0x000DC000;
	if(gd->vfd_type == VFD_TYPE_MN11236)
		lcd->LCDCON2 = 37 << 14;	/* MN11236: 38 lines */
	else
		lcd->LCDCON2 = 55 << 14;	/* T119C:   56 lines */
	lcd->LCDCON3 = 0x0051000A;
	lcd->LCDCON4 = 0x00000001;
	if (gd->vfd_type && vfd_inv_data)
		lcd->LCDCON5 = 0x000004C0;
	else
		lcd->LCDCON5 = 0x00000440;

	/* Port pins as LCD output */
	gpio->PCCON =   (gpio->PCCON & 0xFFFFFF00)| 0x000000AA;
	gpio->PDCON =   (gpio->PDCON & 0xFFFFFF03)| 0x000000A8;

	/* Synchronize VFD enable with LCD controller to avoid flicker	*/
	lcd->LCDCON1 = 0x00000B75;			/* Start LCD-Controller	*/
	while((lcd->LCDCON5 & 0x180000)!=0x100000);	/* Wait for end of VSYNC */
	while((lcd->LCDCON5 & 0x060000)!=0x040000);	/* Wait for next HSYNC	*/
	while((lcd->LCDCON5 & 0x060000)==0x040000);
	while((lcd->LCDCON5 & 0x060000)!=0x000000);
	if(gd->vfd_type)
		VFD_ENABLE;

	debug ("LCDSADDR1: %lX\n", lcd->LCDSADDR1);
	debug ("LCDSADDR2: %lX\n", lcd->LCDSADDR2);
	debug ("LCDSADDR3: %lX\n", lcd->LCDSADDR3);

	return 0;
}

/*
 * Disable VFD: should be run before resetting the system:
 * disable VM, enable pull-up
 */
void disable_vfd (void)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	VFD_DISABLE;
	gpio->PDCON &= ~0xC;
	gpio->PDUP  &= ~0x2;
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

	/* Round up to nearest full page */
	size = (FRAME_BUF_SIZE + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	debug ("Reserving %ldk for VFD Framebuffer at: %08lx\n", size>>10, addr);

	return (size);
}

/*
 * Calculate fb size for VIDEOLFB_ATAG. Size returned contains fb,
 * descriptors and palette areas.
 */
ulong calc_fbsize (void)
{
	return FRAME_BUF_SIZE;
}

#endif /* CONFIG_VFD */
