/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
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
 * Note: Parts of these software are imported from
 *       - UBL, The Universal Talkware Boot Loader
 *         Copyright (C) 2000 Universal Talkware Inc.
 *       - Linux
 *
 *
 */

#include <common.h>

#ifdef CONFIG_VIDEO

#include <command.h>
#include <asm/processor.h>
#include <devices.h>
#include "video.h"
#include <pci.h>
#include "vga_table.h"



#ifdef CONFIG_VIDEO_CT69000
#define VIDEO_VEND_ID 0x102C
#define VIDEO_DEV_ID  0x00C0
#else
#error  CONFIG_VIDEO_CT69000 must be defined
#endif

/*
 * Routine for resent board info to video
 * resides in pip405.c
 */
extern void video_write_board_info(void);

#undef VGA_DEBUG

#ifdef VGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#define VGA_MAXROWS	25
#define VGA_MAXCOLS 80

#define CRTC_CURSH	14 /* cursor high pos */
#define CRTC_CURSL	15 /* cursor low pos */

/* description of the hardware layout */

#define ATTRI_INDEX		CFG_ISA_IO_BASE_ADDRESS | 0x3c0				/* Index and Data write port of the attribute Registers */
#define ATTRI_DATA		CFG_ISA_IO_BASE_ADDRESS | 0x3c1				/* Data port of the attribute Registers */
#define STATUS_REG0		CFG_ISA_IO_BASE_ADDRESS | 0x3c2				/* Status Register 0 (read only) */
#define MSR_REG_W			CFG_ISA_IO_BASE_ADDRESS | 0x3c2				/* Misc. Output Register (write only) */
#define SEQ_INDEX			CFG_ISA_IO_BASE_ADDRESS | 0x3c4				/* Index port of the Sequencer Controller */
#define SEQ_DATA			CFG_ISA_IO_BASE_ADDRESS | 0x3c5				/* Data port of the Sequencer Controller */
#define COL_PAL_MASK	CFG_ISA_IO_BASE_ADDRESS | 0x3c6				/* Color Palette Mask */
#define COL_PAL_STAT	CFG_ISA_IO_BASE_ADDRESS | 0x3c7				/* Color Palette Status (read only) */
#define COL_PAL_IND_R	CFG_ISA_IO_BASE_ADDRESS | 0x3c7				/* Color Palette Read Mode Index (write only) */
#define COL_PAL_IND_W	CFG_ISA_IO_BASE_ADDRESS | 0x3c8				/* Color Palette Write Mode Index */
#define COL_PAL_DATA	CFG_ISA_IO_BASE_ADDRESS | 0x3c9				/* Color Palette Data Port */
#define FCR_REG_R			CFG_ISA_IO_BASE_ADDRESS | 0x3ca				/* Feature Control Register (read only) */
#define MSR_REG_R			CFG_ISA_IO_BASE_ADDRESS | 0x3cc				/* Misc. Output Register (read only) */
#define GR_INDEX			CFG_ISA_IO_BASE_ADDRESS | 0x3ce				/* Index port of the Graphic Controller Registers */
#define GR_DATA				CFG_ISA_IO_BASE_ADDRESS | 0x3cf				/* Data port of the Graphic Controller Registers */
#define FP_INDEX			CFG_ISA_IO_BASE_ADDRESS | 0x3d0				/* Index port of the Flat panel Registers */
#define FP_DATA				CFG_ISA_IO_BASE_ADDRESS | 0x3d1				/* Data port of the Flat panel Registers */
#define MR_INDEX			CFG_ISA_IO_BASE_ADDRESS | 0x3d2				/* Index Port of the Multimedia Extension */
#define MR_DATA				CFG_ISA_IO_BASE_ADDRESS | 0x3d3				/* Data Port of the Multimedia Extension */
#define CRT_INDEX			CFG_ISA_IO_BASE_ADDRESS | 0x3d4				/* Index port of the CRT Controller */
#define CRT_DATA			CFG_ISA_IO_BASE_ADDRESS | 0x3d5				/* Data port of the CRT Controller */
#define XREG_INDEX		CFG_ISA_IO_BASE_ADDRESS | 0x3d6				/* Extended Register index */
#define XREG_DATA			CFG_ISA_IO_BASE_ADDRESS | 0x3d7				/* Extended Register data */
#define STATUS_REG1		CFG_ISA_IO_BASE_ADDRESS | 0x3da				/* Input Status Register 1 (read only) */
#define FCR_REG_W			CFG_ISA_IO_BASE_ADDRESS | 0x3da				/* Feature Control Register (write only) */


static unsigned char * video_fb;		/* Frame buffer */

/* current hardware state */

static int	video_row;
static int	video_col;
static unsigned char video_attr;

static unsigned int font_base_addr;
/**********************************************************************
 * some forward declerations...
 */
int video_init(int busdevfunc);
void vga_set_attrib(void);
void vga_set_crt(void);
void vga_set_dac(void);
void vga_set_gr(void);
void vga_set_seq(void);
void vga_set_xreg(void);
void vga_write_sr(unsigned char reg,unsigned char val);
void vga_write_gr(unsigned char reg,unsigned char val);
void vga_write_cr(unsigned char reg,unsigned char val);
void vga_set_font(void);

/***************************************************************************
 * Init VGA Device
 */

int drv_video_init (void)
{
	int error, devices = 1 ;
 	device_t vgadev ;
	int	busdevfunc;

	busdevfunc=pci_find_device(VIDEO_VEND_ID,VIDEO_DEV_ID,0); /* get PCI Device ID */
	if(busdevfunc==-1) {
#ifdef CONFIG_VIDEO_ONBOARD
		printf("Error VGA Controller (%04X,%04X) not found\n",VIDEO_VEND_ID,VIDEO_DEV_ID);
#endif
		return -1;
	}
	video_init(busdevfunc);
	video_write_board_info();
	memset (&vgadev, 0, sizeof(vgadev));

	strcpy(vgadev.name, "vga");
	vgadev.flags =  DEV_FLAGS_OUTPUT | DEV_FLAGS_SYSTEM;
	vgadev.putc = video_putc;
	vgadev.puts = video_puts;
	vgadev.getc = NULL;
	vgadev.tstc = NULL;
	error = device_register (&vgadev);

	return (error == 0) ? devices : error ;
}


/***********************************************************
 * VGA Initializing
 */

int video_init(int busdevfunc)
{
  pci_read_config_dword(busdevfunc, PCI_BASE_ADDRESS_0, &font_base_addr);

  video_fb = (char*)font_base_addr; /* we look into the big linaer memory area */

  /* set the extended Registers */
	vga_set_xreg();

 	/* set IO Addresses to 0x3Dx (color mode ) */
	out8(MSR_REG_W,0x01);

  /* Feature Control Register:
     Bits 7-4 Reserved = 0
     Bit  3   Vertical Sync select = 1 = Enabled
     Bits 2-0 Reserved = 010 = as read from memory.
  */
  out8(FCR_REG_W,0x02);


  /* Miscelaneous output Register:
     Bits 7-6 (num lines) = 01 = VGA 400 lines,
     Bit  5   (Odd/Even Page) =  1 = Sleect high page of memory,
     Bit  4   reserved = 0,
     Bits 3-2 (Clocl Select) = 01 = 28.322Mhz
     Bit  1 = Display Ram Enable = 1 = Enable processor access.
     Bit  0 = Io Address Select = 1 = Color Graphics Enulation.
  */
  out8(MSR_REG_W,0x67);
  /* set the palette */
  vga_set_dac();
  /* set the attributes (before we bring up the engine
     then we dont have to wait for refresh).
  */
  vga_set_attrib();

  /* set the crontroller register. */
  vga_set_crt();

	vga_write_sr(0x00,0x01); /* synchronous reset */
	vga_write_sr(0x01,0x00); /* clocking mode */
	vga_write_sr(0x02,0x03); /* write to map 0, 1 */
	vga_write_sr(0x03,0x00); /* select character map 0 */
	vga_write_sr(0x04,0x03); /* even-odd addressing */
	vga_write_sr(0x00,0x03); /* clear synchronous reset */

  vga_set_seq(); /* Set the extended sr's. */

  vga_set_gr(); /* Set the graphic registers. */

  /* load the font */
  vga_set_font();

  /* initialize the rol/col counts and the text attribute. */
  video_row=0;
  video_col=0;
  video_attr = VGA_ATTR_CLR_WHT;

  /* Clear the video ram */
  video_clear();

  return 1;
}

void vga_set_font(void)
{
	int i,j;
	char *fontmap;
	fontmap = (char *)font_base_addr;

	vga_write_sr(0x00,0x01); /* synchronous reset */
	vga_write_sr(0x04,0x06); /* sequential addressing */
	vga_write_sr(0x02,0x04); /* write to map 2 */
	vga_write_sr(0x00,0x03); /* clear synchronous reset */

	vga_write_gr(0x04,0x02); /* select map 2 */
	vga_write_gr(0x05,0x00); /* disable odd-even addressing */
	vga_write_gr(0x06,0x00); /* map start at 0xa0000 */

	for(i=0;i<0x100;i++) {
		for(j=0;j<0x10;j++) {
			*((char *)fontmap+i*32+j)=(char)fontdata_8x16[i*16+j];
		}
	}
	vga_write_sr(0x00,0x01); /* synchronous reset */
	vga_write_sr(0x02,0x03); /* write to map 0 and 1 */
	vga_write_sr(0x04,0x03); /* odd-even addressing */
	vga_write_sr(0x03,0x00); /* Character map 0 & 1 */
	vga_write_sr(0x00,0x03); /* clear synchronous reset */

	vga_write_gr(0x04,0x00); /* select map 0 for CPU */
	vga_write_gr(0x05,0x10); /* enable odd-even addressing */
	vga_write_gr(0x06,0x0E); /* map start at 0xb8000 */
}


/* since we are BIG endian, swap attributes and char */
unsigned short vga_swap_short(unsigned short val)
{
	unsigned short swapped;
	swapped = ((val & 0xff)<<8) | ((val & 0xff00)>>8);
	return swapped;
}

/****************************************************
 * Routines usable Outside world
 */

/* scolls the text up row rows */
void video_scroll(int row)
{
  unsigned short clear = ((unsigned short)video_attr << 8) | (' ');
  unsigned short* addr16 = &((unsigned short *)video_fb)[(VGA_MAXROWS-row)*VGA_MAXCOLS];
  int i;

	clear=vga_swap_short(clear);
  memcpy(video_fb, video_fb+row*(VGA_MAXCOLS*2), (VGA_MAXROWS-row)*(VGA_MAXCOLS*2));
  for (i = 0 ; i < row * VGA_MAXCOLS ; i++)
    addr16[i] = clear;
  video_row-=row;
  video_col=0;
}


unsigned long video_cursor(int col, int row)
{
   unsigned short off = row * VGA_MAXCOLS + col ;
   unsigned long saved = (video_col << 16) | (video_row & 0xFFFF);
   video_col = col;
   video_row = row;

   vga_write_cr(CRTC_CURSH,(unsigned char)((off & 0xff00)>>8)); /* Cursor pos. high */
   vga_write_cr(CRTC_CURSL,(unsigned char)(off & 0xff));          /* Cursor pos. low */
   return saved;
}

void video_set_lxy(unsigned long lxy)
{
  int col = (lxy >> 16) & 0xFFFF;
  int row = lxy & 0xFFFF;
  video_cursor(col,row);
}

unsigned long video_get_lxy(void)
{
   return (video_col << 16) | (video_row & 0xFFFF);
}

void video_clear(void)
{
  int i;
  unsigned short clear = ((unsigned short)video_attr << 8) | (' ');
  unsigned short * addr16 = (unsigned short * )video_fb;
	clear=vga_swap_short(clear);
  video_row = video_col = 0;
  for (i = 0 ; i < 2000 ; i++) {
    addr16[i] = clear;
  }
}

void video_copy(unsigned short *buffer)
{
  int i;
  unsigned short * addr16 = (unsigned short * )video_fb;
  for (i = 0 ; i < 2000 ; i++) {
    buffer[i] = addr16[i];
  }
}

void video_write(unsigned short *buffer)
{
  int i;
  unsigned short * addr16 = (unsigned short *)video_fb;
  for (i = 0 ; i < 2000 ; i++) {
    addr16[i] = buffer[i];
  }
}

void video_putc(char ch)
{
	char* addr;
#if 0
	char buf[48];
	char buf1[16];
	static int i=0;

	sprintf(buf1,"%02X ",ch);
	serial_puts(buf1);
	buf[i++]=((ch>=0x20)&&(ch<=0x7f)) ? ch : '.';
	if(i>=16) {
		buf[i++]='\n';
		buf[i]='\0';
		i=0;
		serial_puts("     ");
		serial_puts(buf);
	}
#endif
	switch (ch) {
		case '\n':
			video_col=0;
			video_row++;
 			break;
		case '\r':
 			video_col=0;
 			break;
		case '\t':
      video_col += 8 - video_col % 8;
      break;
    case '\a':
/*      beep(); */
      break;
    case '\b':
      if(video_col)
				video_col--;
      else
				return;
      break;
    default:
      addr = video_fb + 2 * video_row * 80 + 2 * video_col;

      *((char *)addr+1) = (char) video_attr;
      *((char *)addr) = (char) ch;
      video_col++;
      if (video_col > (VGA_MAXCOLS-1)) {
	 			video_row++;
	 			video_col=0;
      }
   }

   /* If we're on the bottom of the secreen, wrap one row */
   if (video_row > (VGA_MAXROWS-1))
     video_scroll(1);
    video_cursor(video_col, video_row);
}


unsigned char video_set_attr(unsigned char attr)
{
  unsigned char saved_attr = video_attr;
  video_attr = attr;
  return saved_attr;
}

unsigned char video_set_attr_xy(unsigned char attr, int x, int y)
{
  unsigned char *addr = video_fb + (x * 80 + y) * 2 + 1;
  unsigned char saved_attr = *addr;
  *addr = attr;
  return saved_attr;
}

/* put char at xy */
void video_putc_xy(char ch, int x, int y)
{
  video_col = x;
  video_row = y;
  video_putc(ch);
}

/* put char at xy relative to the position */
void video_putc_rxy(char ch, int x, int y)
{
  video_col += x;
  video_row += y;
  video_putc(ch);
}

/* put char with attribute at xy */
void video_putc_axy(char ch, char attr, int x, int y)
{
  unsigned char saved_attr = video_set_attr(attr);
  video_col = x;
  video_row = y;
  video_putc(ch);
  video_set_attr(saved_attr);
}

void video_puts(const char *s)
{
   while(*s) {
      video_putc(*s);
      s++;
   }
}

void video_puts_a(const char *s, char attr)
{
  unsigned char saved_attr = video_set_attr(attr);
  video_puts(s);
  video_set_attr(saved_attr);
}

void video_puts_xy(const char *s, int x, int y)
{
  video_cursor(x,y);
  video_puts(s);
}

void video_puts_axy(const char *s, char attr, int x, int y)
{
  unsigned char saved_attr = video_set_attr(attr);
  video_puts_xy(s, x, y);
  video_set_attr(saved_attr);
}

void video_wipe_ca_area(unsigned char ch, char attr, int x, int y, int w, int h)
{
  int r, c;
  /* better to do this as word writes */
  unsigned short * addr16 = (unsigned short  *)video_fb + (y * 80 + x);
  unsigned short charattr = (unsigned short)ch << 8 | attr;
	charattr=vga_swap_short(charattr);
  for (r = 0 ; r < h ; r++, addr16 += 80) {
    for (c = 0 ; c < w ; c++) {
      addr16[c] = charattr;
    }
  }
}

void video_wipe_a_area(unsigned char attr, int x, int y, int w, int h)
{
  int r, c;
  /* better to do this as word writes */
  unsigned short * addr16 = (unsigned short *)video_fb + (y * 80 + x);
  for (r = 0 ; r < h ; r++, addr16 += 80) {
    for (c = 0 ; c < w ; c++) {
      ((char*)addr16)[c*2+1] = attr;
    }
  }
}

void video_wipe_c_area(unsigned char ch, int x, int y, int w, int h)
{
  int r, c;
  /* better to do this as word writes */
  unsigned short * addr16 = (unsigned short  *)video_fb + (y * 80 + x);
  for (r = 0 ; r < h ; r++, addr16 += 80) {
    for (c = 0 ; c < w ; c++) {
      ((char*)addr16)[c*2] = ch;
    }
  }
}


/*
tl t tr
l    l
bl b br
*/
typedef struct {
  unsigned char tl; /* top left corner */
  unsigned char t;  /* top edge */
  unsigned char tr; /* top right corner */
  unsigned char l;  /* left edge */
  unsigned char r;  /* right edge */
  unsigned char bl; /* bottom left corner */
  unsigned char b;  /* bottom edge */
  unsigned char br; /* bottom right corner */
} box_chars_t;

box_chars_t sbox_chars = {
  0xDA, 0xC4, 0xBF,
  0xB3,       0xB3,
  0xC0, 0xC4, 0xD9
};

box_chars_t dbox_chars = {
  0xC9, 0xCD, 0xBB,
  0xBA,       0xBA,
  0xC8, 0xCD, 0xBC
};

static char cmap[] = "0123456789ABCDEF";
void video_putchex(char c)
{
  video_putc(cmap[(c >> 4 ) & 0xF]);
  video_putc(cmap[c & 0xF]);
}

void video_putchexl(char c)
{
  video_putc(cmap[c & 0xF]);
}

void video_putchexh(char c)
{
  video_putc(cmap[(c >> 4) & 0xF]);
}

#define VGA_CELL_CA(a,c) (((unsigned short)c<<8)|a) /* for BIG endians */

void video_gbox_area(box_chars_t *box_chars_p, int x, int y, int w, int h)
{
  int r, c;
  /* better to do this as word writes */
  unsigned short* addr16 = (unsigned short *)video_fb + (y * VGA_MAXCOLS + x);
  for (r = 0 ; r < h ; r++, addr16 += VGA_MAXCOLS) {
    if (r == 0) {
      addr16[0]   = VGA_CELL_CA(video_attr, box_chars_p->tl);
      addr16[w-1] = VGA_CELL_CA(video_attr, box_chars_p->tr);
      for (c = 1 ; c < w - 1 ; c++)
				addr16[c] = VGA_CELL_CA(video_attr, box_chars_p->t);
    } else if (r == h - 1) {
      addr16[0] = VGA_CELL_CA(video_attr, box_chars_p->bl);
      addr16[w-1] = VGA_CELL_CA(video_attr, box_chars_p->br);
      for (c = 1 ; c < w - 1 ; c++)
				addr16[c] = VGA_CELL_CA(video_attr, box_chars_p->b);
    } else {
      addr16[0] = VGA_CELL_CA(video_attr, box_chars_p->l);
      addr16[w-1] = VGA_CELL_CA(video_attr, box_chars_p->r);
    }
  }
}

/* Writes a box on the screen */
void video_box_area(int x, int y, int w, int h) {
  video_gbox_area(&sbox_chars, x, y, w, h);
}
/*writes a box with double lines on the screen */
void video_dbox_area(int x, int y, int w, int h) {
  video_gbox_area(&dbox_chars, x, y, w, h);
}

/* routines to set the VGA registers */

/* set attributes */
void vga_set_attrib(void)
{
	int i;
	unsigned char status;

	status=in8(STATUS_REG1);
	i=0;

	while(attr[i].reg!=0xFF) {
		out8(ATTRI_INDEX,attr[i].reg);
		out8(ATTRI_INDEX,attr[i].val); /* Attribute uses index for index and data */
		i++;
	}
	out8(ATTRI_INDEX,0x20); /* unblank the screen */
}

/* set CRT Controller Registers */
void vga_set_crt(void)
{
	int i;
	i=0;
	while(crtc[i].reg!=0xFF) {
		out8(CRT_INDEX,crtc[i].reg);
		out8(CRT_DATA,crtc[i].val);
		i++;
	}
}
/* Set Palette Registers (DAC) */
void vga_set_dac(void)
{
	int i;
	for(i=0;i<256;i++) {
		out8(COL_PAL_IND_W,(unsigned char)i);
		out8(COL_PAL_DATA,dac[i][0]); /* red */
		out8(COL_PAL_DATA,dac[i][1]); /* green */
		out8(COL_PAL_DATA,dac[i][2]); /* blue */
	}
	out8(COL_PAL_MASK,0xff); /* set mask */
}
/* set Graphic Controller Register */
void vga_set_gr(void)
{
	int i;
	i=0;
	while(grmr[i].reg!=0xFF) {
		out8(GR_INDEX,grmr[i].reg);
		out8(GR_DATA,grmr[i].val);
		i++;
	}
}

/* Set Sequencer Registers */
void vga_set_seq(void)
{
	int i;
	i=0;
	while(seq[i].reg!=0xFF) {
		out8(SEQ_INDEX,seq[i].reg);
		out8(SEQ_DATA,seq[i].val);
		i++;
	}
}


/* Set Extension Registers */
void vga_set_xreg(void)
{
	int i;
	i=0;
	while(xreg[i].reg!=0xFF) {
		out8(XREG_INDEX,xreg[i].reg);
		out8(XREG_DATA,xreg[i].val);
		i++;
	}
}

/************************************************************
 * some helping routines
 */

void vga_write_sr(unsigned char reg,unsigned char val)
{
	out8(SEQ_INDEX,reg);
	out8(SEQ_DATA,val);
}


void vga_write_gr(unsigned char reg,unsigned char val)
{
	out8(GR_INDEX,reg);
	out8(GR_DATA,val);
}

void vga_write_cr(unsigned char reg,unsigned char val)
{
	out8(CRT_INDEX,reg);
	out8(CRT_DATA,val);
}


#if 0
void video_dump_reg(void)
{
	/* first dump attributes */
	int i;
	unsigned char status;


	printf("Extended Regs:\n");
	i=0;
	while(xreg[i].reg!=0xFF) {
		out8(XREG_INDEX,xreg[i].reg);
		status=in8(XREG_DATA);
		printf("XR%02X is %02X, should be %02X\n",xreg[i].reg,status,xreg[i].val);
		i++;
	}
	printf("Sequencer Regs:\n");
	i=0;
	while(seq[i].reg!=0xFF) {
		out8(SEQ_INDEX,seq[i].reg);
		status=in8(SEQ_DATA);
		printf("SR%02X is %02X, should be %02X\n",seq[i].reg,status,seq[i].val);
		i++;
	}
	printf("Graphic Regs:\n");
	i=0;
	while(grmr[i].reg!=0xFF) {
		out8(GR_INDEX,grmr[i].reg);
		status=in8(GR_DATA);
		printf("GR%02X is %02X, should be %02X\n",grmr[i].reg,status,grmr[i].val);
		i++;
	}
	printf("CRT Regs:\n");
	i=0;
	while(crtc[i].reg!=0xFF) {
		out8(CRT_INDEX,crtc[i].reg);
		status=in8(CRT_DATA);
		printf("CR%02X is %02X, should be %02X\n",crtc[i].reg,status,crtc[i].val);
		i++;
	}
	printf("Attributes:\n");
	status=in8(STATUS_REG1);
	i=0;
	while(attr[i].reg!=0xFF) {
		out8(ATTRI_INDEX,attr[i].reg);
		status=in8(ATTRI_DATA);
		out8(ATTRI_INDEX,attr[i].val); /* Attribute uses index for index and data */
		printf("AR%02X is %02X, should be %02X\n",attr[i].reg,status,attr[i].val);
		i++;
	}
}
#endif

#endif /* CONFIG_VIDEO */




















