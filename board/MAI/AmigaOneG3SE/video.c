/*
 * (C) Copyright 2002
 * Hyperion Entertainment, Hans-JoergF@hyperion-entertainment.com
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

#include <common.h>
#include <stdio_dev.h>
#include "memio.h"
#include <part.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned char *cursor_position;
unsigned int cursor_row;
unsigned int cursor_col;

unsigned char current_attr;

unsigned int video_numrows = 25;
unsigned int video_numcols = 80;
unsigned int video_scrolls = 0;

#define VIDEO_BASE (unsigned char *)0xFD0B8000
#define VIDEO_ROWS video_numrows
#define VIDEO_COLS video_numcols
#define VIDEO_PITCH (2 * video_numcols)
#define VIDEO_SIZE (video_numrows * video_numcols * 2)
#define VIDEO_NAME "vga"

void video_test(void);
void video_putc(char ch);
void video_puts(char *string);
void video_scroll(int rows);
void video_banner(void);
int  video_init(void);
int  video_start(void);
int  video_rows(void);
int  video_cols(void);

char *prompt_string = "=>";
unsigned char video_get_attr(void);

void video_set_color(unsigned char attr)
{
    unsigned char *fb = (unsigned char *)VIDEO_BASE;
    int i;

    current_attr = video_get_attr();

    for (i=0; i<VIDEO_SIZE; i+=2)
    {
	*(fb+i+1) = current_attr;
    }
}

unsigned char video_get_attr(void)
{
    char *s;
    unsigned char attr;

    attr = 0x0f;

    s = getenv("vga_fg_color");
    if (s)
    {
	attr = atoi(s);
    }

    s = getenv("vga_bg_color");
    if (s)
    {
	attr |= atoi(s)<<4;
    }

    return attr;
}

int video_inited = 0;

int drv_video_init(void)
{
    int error, devices = 1 ;
    struct stdio_dev vgadev ;
    if (video_inited) return 1;
    video_inited = 1;
    video_init();
    memset (&vgadev, 0, sizeof(vgadev));

    strcpy(vgadev.name, VIDEO_NAME);
    vgadev.flags =  DEV_FLAGS_OUTPUT | DEV_FLAGS_SYSTEM;
    vgadev.putc = video_putc;
    vgadev.puts = video_puts;
    vgadev.getc = NULL;
    vgadev.tstc = NULL;
    vgadev.start = video_start;

    error = stdio_register (&vgadev);

    if (error == 0)
    {
	char *s = getenv("stdout");
	if (s && strcmp(s, VIDEO_NAME)==0)
	{
	    if (overwrite_console()) return 1;
	    error = console_assign(stdout, VIDEO_NAME);
	    if (error == 0) return 1;
	    else return error;
	}
	return 1;
    }

    return error;
}

int video_init(void)
{
    cursor_position = VIDEO_BASE; /* Color text display base */
    cursor_row = 0;
    cursor_col = 0;
    current_attr = video_get_attr(); /* Currently selected value for attribute. */
/*    video_test(); */
    video_set_color(current_attr);

    return 0;
}

void video_set_cursor(int line, int column)
{
    unsigned short offset = line*video_numcols + column;
    cursor_position = VIDEO_BASE +  line*VIDEO_PITCH + column*2;
    out_byte(0x3D4, 0x0E);
    out_byte(0x3D5, offset/256);
    out_byte(0x3D4, 0x0F);
    out_byte(0x3D5, offset%256);
}

void video_write_char(int character)
{
    *cursor_position = character;
    *(cursor_position+1) = current_attr;
}

void video_test(void)
{

}

void video_putc(char ch)
{
    switch(ch)
    {
    case '\n':
	cursor_col = 0;
	cursor_row += 1;
	break;
    case '\r':
	cursor_col = 0;
	break;
    case '\b':
	if (cursor_col) cursor_col--;
	else return;
	break;
    case '\t':
	cursor_col = (cursor_col/8+1)*8;
	break;
    default:
	video_write_char(ch);
	cursor_col++;
	if (cursor_col > VIDEO_COLS-1)
	{
	    cursor_row++;
	    cursor_col=0;
	}
    }

    if (cursor_row > VIDEO_ROWS-1)
	video_scroll(1);
    video_set_cursor(cursor_row, cursor_col);
}

void video_scroll(int rows)
{
    unsigned short clear = ((unsigned short)current_attr) | (' '<<8);
    unsigned short* addr16 = &((unsigned short *)VIDEO_BASE)[(VIDEO_ROWS-rows)*VIDEO_COLS];
    int i;
    char *s;

    s = getenv("vga_askscroll");
    video_scrolls += rows;

    if (video_scrolls >= video_numrows)
    {
	if (s && strcmp(s, "yes"))
	{
	    while (-1 == tstc());
	}

	video_scrolls = 0;
    }


    memcpy(VIDEO_BASE, VIDEO_BASE+rows*(VIDEO_COLS*2), (VIDEO_ROWS-rows)*(VIDEO_COLS*2));
    for (i = 0 ; i < rows * VIDEO_COLS ; i++)
	addr16[i] = clear;
    cursor_row-=rows;
    cursor_col=0;
}

void video_puts(char *string)
{
    while (*string)
    {
	video_putc(*string);
	string++;
    }
}

int video_start(void)
{
    return 0;
}

unsigned char video_single_box[] =
{
    218, 196, 191,
    179,      179,
    192, 196, 217
};

unsigned char video_double_box[] =
{
    201, 205, 187,
    186,      186,
    200, 205, 188
};

unsigned char video_single_title[] =
{
    195, 196, 180, 180, 195
};

unsigned char video_double_title[] =
{
    204, 205, 185, 181, 198
};

#define SINGLE_BOX 0
#define DOUBLE_BOX 1

unsigned char *video_addr(int x, int y)
{
    return VIDEO_BASE + 2*(VIDEO_COLS*y) + 2*x;
}

void video_bios_print_string(char *s, int x, int y, int attr, int count)
{
    int cattr = current_attr;
    if (attr != -1) current_attr = attr;
    video_set_cursor(x,y);
    while (count)
    {
	char c = *s++;
	if (attr == -1) current_attr = *s++;
	video_putc(c);
	count--;
    }
}

void video_draw_box(int style, int attr, char *title, int separate, int x, int y, int w, int h)
{
    unsigned char *fb, *fb2;
    unsigned char *st = (style == SINGLE_BOX)?video_single_box : video_double_box;
    unsigned char *ti = (style == SINGLE_BOX)?video_single_title : video_double_title;
    int i;

    fb = video_addr(x,y);
    *(fb) = st[0];
    *(fb+1) = attr;
    fb += 2;

    fb2 = video_addr(x,y+h-1);
    *(fb2) = st[5];
    *(fb2+1) = attr;
    fb2 += 2;

    for (i=0; i<w-2;i++)
    {
	*fb = st[1];
	fb++;
	*fb = attr;
	fb++;

	*fb2 = st[6];
	fb2++;
	*fb2 = attr;
	fb2++;

    }
    *fb = st[2];
    *(fb+1) = attr;

    *fb2 = st[7];
    *(fb2+1) = attr;

    fb  = video_addr(x, y+1);
    fb2 = video_addr(x+w-1, y+1);
    for (i=0; i<h-2; i++)
    {
	*fb = st[3];
	*(fb+1) = attr; fb += 2*VIDEO_COLS;

	*fb2 = st[4];
	*(fb2+1) = attr; fb2 += 2*VIDEO_COLS;
    }

    /* Draw title */
    if (title)
    {
	if (separate == 0)
	{
	    fb = video_addr(x+1, y);
	    *fb = ti[3];
	    fb += 2;
	    *fb = ' ';
	    fb += 2;
	    while (*title)
	    {
		*fb = *title;
		fb ++;
		*fb = attr;
		fb++; title++;
	    }
	    *fb = ' ';
	    fb += 2;
	    *fb = ti[4];
	}
	else
	{
	    fb = video_addr(x, y+2);
	    *fb = ti[0];
	    fb += 2;
	    for (i=0; i<w-2; i++)
	    {
		*fb = ti[1];
		*(fb+1) = attr;
		fb += 2;
	    }
	    *fb = ti[2];
	    *(fb+1) = attr;
	    fb = video_addr(x+1, y+1);
	    for (i=0; i<w-2; i++)
	    {
		*fb = ' ';
		*(fb+1) = attr;
		fb += 2;
	    }
	    fb = video_addr(x+2, y+1);

	    while (*title)
	    {
		*fb = *title;
		*(fb+1) = attr;
		fb += 2;
		title++;
	    }
	}
    }

}

void video_draw_text(int x, int y, int attr, char *text)
{
    unsigned char *fb = video_addr(x,y);
    while (*text)
    {
	*fb++ = *text++;
	*fb++ = attr;
    }
}

void video_save_rect(int x, int y, int w, int h, void *save_area, int clearchar, int clearattr)
{
    unsigned char *save = (unsigned char *)save_area;
    unsigned char *fb = video_addr(x,y);
    int i,j;
    for (i=0; i<h; i++)
    {
	unsigned char *fbb = fb;
	for (j=0; j<w; j++)
	{
	    *save ++ = *fb;
	    if (clearchar > 0) *fb = clearchar;
	    fb ++;
	    *save ++ = *fb;
	    if (clearattr > 0) *fb = clearattr;
	}
	fb = fbb + 2*VIDEO_COLS;
    }
}

void video_restore_rect(int x, int y, int w, int h, void *save_area)
{
    unsigned char *save = (unsigned char *)save_area;
    unsigned char *fb = video_addr(x,y);
    int i,j;
    for (i=0; i<h; i++)
    {
	unsigned char *fbb = fb;
	for (j=0; j<w; j++)
	{
	    *fb ++ = *save ++;
	    *fb ++ = *save ++;
	}
	fb = fbb + 2*VIDEO_COLS;
    }

}

int video_rows(void)
{
    return VIDEO_ROWS;
}

int video_cols(void)
{
    return VIDEO_COLS;
}

void video_size(int cols, int rows)
{
    video_numrows = rows;
    video_numcols = cols;
}

void video_clear(void)
{
    unsigned short *fbb = (unsigned short *)0xFD0B8000;
    int i,j;
    unsigned short val = 0x2000 | current_attr;

    for (i=0; i<video_rows(); i++)
    {
	for (j=0; j<video_cols(); j++)
	{
	    *fbb++ = val;
	}
    }
    video_set_cursor(0,0);
    cursor_row = 0;
    cursor_col = 0;
}

#ifdef EASTEREGG
int video_easteregg_active = 0;

void video_easteregg(void)
{
    video_easteregg_active = 1;
}
#endif

extern block_dev_desc_t * ide_get_dev(int dev);
extern char version_string[];

void video_banner(void)
{
    block_dev_desc_t *ide;
    int i;
    char *s;
    int maxdev;


    if (video_inited == 0) return;
#ifdef EASTEREGG
    if (video_easteregg_active)
    {
	prompt_string="";
	video_clear();
	printf("\n");
	printf("    **** COMMODORE 64 BASIC X2 ****\n\n");
	printf(" 64K RAM SYSTEM  38911 BASIC BYTES FREE\n\n");
	printf("READY\n");
    }
    else
    {
#endif
	s = getenv("ide_maxbus");
	if (s)
	    maxdev = atoi(s) * 2;
	else
	    maxdev = 4;

	s = getenv("stdout");
	if (s && strcmp(s, "serial") == 0)
	    return;

	video_clear();
	printf("%s\n\nCPU: ", version_string);
	checkcpu();
	printf("DRAM: %ld MB\n", gd->bd->bi_memsize/(1024*1024));
	printf("FSB: %ld MHz\n", gd->bd->bi_busfreq/1000000);

	printf("\n---- Disk summary ----\n");
	for (i = 0; i < maxdev; i++)
	{
	    ide = ide_get_dev(i);
	    printf("Device %d: ", i);
	    dev_print(ide);
	}

/*
    video_draw_box(SINGLE_BOX, 0x0F, "Test 1", 0, 0,18, 72, 4);
    video_draw_box(DOUBLE_BOX, 0x0F, "Test 2", 1, 4,10, 50, 6);
    video_draw_box(DOUBLE_BOX, 0x0F, "Test 3", 0, 40, 3, 20, 5);

    video_draw_text(1, 4, 0x2F, "Highlighted options");
    video_draw_text(1, 5, 0x0F, "Non-selected option");
    video_draw_text(1, 6, 0x07, "disabled option");
*/
#ifdef EASTEREGG
    }
#endif
}
