/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* Video support for the ECCX daughter board                                 */


#include <common.h>
#include <config.h>

#ifdef CONFIG_VIDEO_SED13806
#include <sed13806.h>


/* Screen configurations: the initialization of the SD13806 depends on
   screen and on display mode. We handle only 8bpp and 16 bpp modes          */

/* ECCX board is supplied with a NEC NL6448BC20 screen                       */
#ifdef CONFIG_NEC_NL6448BC20
#define DISPLAY_WIDTH   640
#define DISPLAY_HEIGHT  480

#ifdef CONFIG_VIDEO_SED13806_8BPP
static const S1D_REGS init_regs [] =
{
    {0x0001,0x00},   /* Miscellaneous Register */
    {0x01FC,0x00},   /* Display Mode Register */
    {0x0004,0x1b},   /* General IO Pins Configuration Register 0 */
    {0x0005,0x00},   /* General IO Pins Configuration Register 1 */
    {0x0008,0xe5},   /* General IO Pins Control Register 0 */
    {0x0009,0x1f},   /* General IO Pins Control Register 1 */
    {0x0010,0x02},   /* Memory Clock Configuration Register */
    {0x0014,0x10},   /* LCD Pixel Clock Configuration Register */
    {0x0018,0x02},   /* CRT/TV Pixel Clock Configuration Register */
    {0x001C,0x02},   /* MediaPlug Clock Configuration Register */
    {0x001E,0x01},   /* CPU To Memory Wait State Select Register */
    {0x0021,0x04},   /* DRAM Refresh Rate Register */
    {0x002A,0x00},   /* DRAM Timings Control Register 0 */
    {0x002B,0x01},   /* DRAM Timings Control Register 1 */
    {0x0020,0x80},   /* Memory Configuration Register */
    {0x0030,0x25},   /* Panel Type Register */
    {0x0031,0x00},   /* MOD Rate Register */
    {0x0032,0x4F},   /* LCD Horizontal Display Width Register */
    {0x0034,0x13},   /* LCD Horizontal Non-Display Period Register */
    {0x0035,0x01},   /* TFT FPLINE Start Position Register */
    {0x0036,0x0B},   /* TFT FPLINE Pulse Width Register */
    {0x0038,0xDF},   /* LCD Vertical Display Height Register 0 */
    {0x0039,0x01},   /* LCD Vertical Display Height Register 1 */
    {0x003A,0x2C},   /* LCD Vertical Non-Display Period Register */
    {0x003B,0x00},   /* TFT FPFRAME Start Position Register */
    {0x003C,0x01},   /* TFT FPFRAME Pulse Width Register */
    {0x0040,0x03},   /* LCD Display Mode Register */
    {0x0041,0x02},   /* LCD Miscellaneous Register */
    {0x0042,0x00},   /* LCD Display Start Address Register 0 */
    {0x0043,0x00},   /* LCD Display Start Address Register 1 */
    {0x0044,0x00},   /* LCD Display Start Address Register 2 */
    {0x0046,0x40},   /* LCD Memory Address Offset Register 0 */
    {0x0047,0x01},   /* LCD Memory Address Offset Register 1 */
    {0x0048,0x00},   /* LCD Pixel Panning Register */
    {0x004A,0x00},   /* LCD Display FIFO High Threshold Control Register */
    {0x004B,0x00},   /* LCD Display FIFO Low Threshold Control Register */
    {0x0050,0x4F},   /* CRT/TV Horizontal Display Width Register */
    {0x0052,0x13},   /* CRT/TV Horizontal Non-Display Period Register */
    {0x0053,0x01},   /* CRT/TV HRTC Start Position Register */
    {0x0054,0x0B},   /* CRT/TV HRTC Pulse Width Register */
    {0x0056,0xDF},   /* CRT/TV Vertical Display Height Register 0 */
    {0x0057,0x01},   /* CRT/TV Vertical Display Height Register 1 */
    {0x0058,0x2B},   /* CRT/TV Vertical Non-Display Period Register */
    {0x0059,0x09},   /* CRT/TV VRTC Start Position Register */
    {0x005A,0x01},   /* CRT/TV VRTC Pulse Width Register */
    {0x005B,0x00},   /* TV Output Control Register */
    {0x0060,0x03},   /* CRT/TV Display Mode Register */
    {0x0062,0x00},   /* CRT/TV Display Start Address Register 0 */
    {0x0063,0x00},   /* CRT/TV Display Start Address Register 1 */
    {0x0064,0x00},   /* CRT/TV Display Start Address Register 2 */
    {0x0066,0x40},   /* CRT/TV Memory Address Offset Register 0 */
    {0x0067,0x01},   /* CRT/TV Memory Address Offset Register 1 */
    {0x0068,0x00},   /* CRT/TV Pixel Panning Register */
    {0x006A,0x00},   /* CRT/TV Display FIFO High Threshold Control Register */
    {0x006B,0x00},   /* CRT/TV Display FIFO Low Threshold Control Register */
    {0x0070,0x00},   /* LCD Ink/Cursor Control Register */
    {0x0071,0x00},   /* LCD Ink/Cursor Start Address Register */
    {0x0072,0x00},   /* LCD Cursor X Position Register 0 */
    {0x0073,0x00},   /* LCD Cursor X Position Register 1 */
    {0x0074,0x00},   /* LCD Cursor Y Position Register 0 */
    {0x0075,0x00},   /* LCD Cursor Y Position Register 1 */
    {0x0076,0x00},   /* LCD Ink/Cursor Blue Color 0 Register */
    {0x0077,0x00},   /* LCD Ink/Cursor Green Color 0 Register */
    {0x0078,0x00},   /* LCD Ink/Cursor Red Color 0 Register */
    {0x007A,0x1F},   /* LCD Ink/Cursor Blue Color 1 Register */
    {0x007B,0x3F},   /* LCD Ink/Cursor Green Color 1 Register */
    {0x007C,0x1F},   /* LCD Ink/Cursor Red Color 1 Register */
    {0x007E,0x00},   /* LCD Ink/Cursor FIFO Threshold Register */
    {0x0080,0x00},   /* CRT/TV Ink/Cursor Control Register */
    {0x0081,0x00},   /* CRT/TV Ink/Cursor Start Address Register */
    {0x0082,0x00},   /* CRT/TV Cursor X Position Register 0 */
    {0x0083,0x00},   /* CRT/TV Cursor X Position Register 1 */
    {0x0084,0x00},   /* CRT/TV Cursor Y Position Register 0 */
    {0x0085,0x00},   /* CRT/TV Cursor Y Position Register 1 */
    {0x0086,0x00},   /* CRT/TV Ink/Cursor Blue Color 0 Register */
    {0x0087,0x00},   /* CRT/TV Ink/Cursor Green Color 0 Register */
    {0x0088,0x00},   /* CRT/TV Ink/Cursor Red Color 0 Register */
    {0x008A,0x1F},   /* CRT/TV Ink/Cursor Blue Color 1 Register */
    {0x008B,0x3F},   /* CRT/TV Ink/Cursor Green Color 1 Register */
    {0x008C,0x1F},   /* CRT/TV Ink/Cursor Red Color 1 Register */
    {0x008E,0x00},   /* CRT/TV Ink/Cursor FIFO Threshold Register */
    {0x0100,0x00},   /* BitBlt Control Register 0 */
    {0x0101,0x00},   /* BitBlt Control Register 1 */
    {0x0102,0x00},   /* BitBlt ROP Code/Color Expansion Register */
    {0x0103,0x00},   /* BitBlt Operation Register */
    {0x0104,0x00},   /* BitBlt Source Start Address Register 0 */
    {0x0105,0x00},   /* BitBlt Source Start Address Register 1 */
    {0x0106,0x00},   /* BitBlt Source Start Address Register 2 */
    {0x0108,0x00},   /* BitBlt Destination Start Address Register 0 */
    {0x0109,0x00},   /* BitBlt Destination Start Address Register 1 */
    {0x010A,0x00},   /* BitBlt Destination Start Address Register 2 */
    {0x010C,0x00},   /* BitBlt Memory Address Offset Register 0 */
    {0x010D,0x00},   /* BitBlt Memory Address Offset Register 1 */
    {0x0110,0x00},   /* BitBlt Width Register 0 */
    {0x0111,0x00},   /* BitBlt Width Register 1 */
    {0x0112,0x00},   /* BitBlt Height Register 0 */
    {0x0113,0x00},   /* BitBlt Height Register 1 */
    {0x0114,0x00},   /* BitBlt Background Color Register 0 */
    {0x0115,0x00},   /* BitBlt Background Color Register 1 */
    {0x0118,0x00},   /* BitBlt Foreground Color Register 0 */
    {0x0119,0x00},   /* BitBlt Foreground Color Register 1 */
    {0x01E0,0x00},   /* Look-Up Table Mode Register */
    {0x01E2,0x00},   /* Look-Up Table Address Register */
    {0x01E4,0x00},   /* Look-Up Table Data Register */
    {0x01F0,0x10},   /* Power Save Configuration Register */
    {0x01F1,0x00},   /* Power Save Status Register */
    {0x01F4,0x00},   /* CPU-to-Memory Access Watchdog Timer Register */
    {0x01FC,0x01},   /* Display Mode Register */
    {0, 0}
};
#endif /* CONFIG_VIDEO_SED13806_8BPP */

#ifdef CONFIG_VIDEO_SED13806_16BPP

static const S1D_REGS init_regs [] =
{
    {0x0001,0x00},   /* Miscellaneous Register */
    {0x01FC,0x00},   /* Display Mode Register */
    {0x0004,0x1b},   /* General IO Pins Configuration Register 0 */
    {0x0005,0x00},   /* General IO Pins Configuration Register 1 */
    {0x0008,0xe5},   /* General IO Pins Control Register 0 */
    {0x0009,0x1f},   /* General IO Pins Control Register 1 */
    {0x0010,0x02},   /* Memory Clock Configuration Register */
    {0x0014,0x10},   /* LCD Pixel Clock Configuration Register */
    {0x0018,0x02},   /* CRT/TV Pixel Clock Configuration Register */
    {0x001C,0x02},   /* MediaPlug Clock Configuration Register */
    {0x001E,0x01},   /* CPU To Memory Wait State Select Register */
    {0x0021,0x04},   /* DRAM Refresh Rate Register */
    {0x002A,0x00},   /* DRAM Timings Control Register 0 */
    {0x002B,0x01},   /* DRAM Timings Control Register 1 */
    {0x0020,0x80},   /* Memory Configuration Register */
    {0x0030,0x25},   /* Panel Type Register */
    {0x0031,0x00},   /* MOD Rate Register */
    {0x0032,0x4F},   /* LCD Horizontal Display Width Register */
    {0x0034,0x13},   /* LCD Horizontal Non-Display Period Register */
    {0x0035,0x01},   /* TFT FPLINE Start Position Register */
    {0x0036,0x0B},   /* TFT FPLINE Pulse Width Register */
    {0x0038,0xDF},   /* LCD Vertical Display Height Register 0 */
    {0x0039,0x01},   /* LCD Vertical Display Height Register 1 */
    {0x003A,0x2C},   /* LCD Vertical Non-Display Period Register */
    {0x003B,0x00},   /* TFT FPFRAME Start Position Register */
    {0x003C,0x01},   /* TFT FPFRAME Pulse Width Register */
    {0x0040,0x05},   /* LCD Display Mode Register */
    {0x0041,0x02},   /* LCD Miscellaneous Register */
    {0x0042,0x00},   /* LCD Display Start Address Register 0 */
    {0x0043,0x00},   /* LCD Display Start Address Register 1 */
    {0x0044,0x00},   /* LCD Display Start Address Register 2 */
    {0x0046,0x80},   /* LCD Memory Address Offset Register 0 */
    {0x0047,0x02},   /* LCD Memory Address Offset Register 1 */
    {0x0048,0x00},   /* LCD Pixel Panning Register */
    {0x004A,0x00},   /* LCD Display FIFO High Threshold Control Register */
    {0x004B,0x00},   /* LCD Display FIFO Low Threshold Control Register */
    {0x0050,0x4F},   /* CRT/TV Horizontal Display Width Register */
    {0x0052,0x13},   /* CRT/TV Horizontal Non-Display Period Register */
    {0x0053,0x01},   /* CRT/TV HRTC Start Position Register */
    {0x0054,0x0B},   /* CRT/TV HRTC Pulse Width Register */
    {0x0056,0xDF},   /* CRT/TV Vertical Display Height Register 0 */
    {0x0057,0x01},   /* CRT/TV Vertical Display Height Register 1 */
    {0x0058,0x2B},   /* CRT/TV Vertical Non-Display Period Register */
    {0x0059,0x09},   /* CRT/TV VRTC Start Position Register */
    {0x005A,0x01},   /* CRT/TV VRTC Pulse Width Register */
    {0x005B,0x00},   /* TV Output Control Register */
    {0x0060,0x05},   /* CRT/TV Display Mode Register */
    {0x0062,0x00},   /* CRT/TV Display Start Address Register 0 */
    {0x0063,0x00},   /* CRT/TV Display Start Address Register 1 */
    {0x0064,0x00},   /* CRT/TV Display Start Address Register 2 */
    {0x0066,0x80},   /* CRT/TV Memory Address Offset Register 0 */
    {0x0067,0x02},   /* CRT/TV Memory Address Offset Register 1 */
    {0x0068,0x00},   /* CRT/TV Pixel Panning Register */
    {0x006A,0x00},   /* CRT/TV Display FIFO High Threshold Control Register */
    {0x006B,0x00},   /* CRT/TV Display FIFO Low Threshold Control Register */
    {0x0070,0x00},   /* LCD Ink/Cursor Control Register */
    {0x0071,0x00},   /* LCD Ink/Cursor Start Address Register */
    {0x0072,0x00},   /* LCD Cursor X Position Register 0 */
    {0x0073,0x00},   /* LCD Cursor X Position Register 1 */
    {0x0074,0x00},   /* LCD Cursor Y Position Register 0 */
    {0x0075,0x00},   /* LCD Cursor Y Position Register 1 */
    {0x0076,0x00},   /* LCD Ink/Cursor Blue Color 0 Register */
    {0x0077,0x00},   /* LCD Ink/Cursor Green Color 0 Register */
    {0x0078,0x00},   /* LCD Ink/Cursor Red Color 0 Register */
    {0x007A,0x1F},   /* LCD Ink/Cursor Blue Color 1 Register */
    {0x007B,0x3F},   /* LCD Ink/Cursor Green Color 1 Register */
    {0x007C,0x1F},   /* LCD Ink/Cursor Red Color 1 Register */
    {0x007E,0x00},   /* LCD Ink/Cursor FIFO Threshold Register */
    {0x0080,0x00},   /* CRT/TV Ink/Cursor Control Register */
    {0x0081,0x00},   /* CRT/TV Ink/Cursor Start Address Register */
    {0x0082,0x00},   /* CRT/TV Cursor X Position Register 0 */
    {0x0083,0x00},   /* CRT/TV Cursor X Position Register 1 */
    {0x0084,0x00},   /* CRT/TV Cursor Y Position Register 0 */
    {0x0085,0x00},   /* CRT/TV Cursor Y Position Register 1 */
    {0x0086,0x00},   /* CRT/TV Ink/Cursor Blue Color 0 Register */
    {0x0087,0x00},   /* CRT/TV Ink/Cursor Green Color 0 Register */
    {0x0088,0x00},   /* CRT/TV Ink/Cursor Red Color 0 Register */
    {0x008A,0x1F},   /* CRT/TV Ink/Cursor Blue Color 1 Register */
    {0x008B,0x3F},   /* CRT/TV Ink/Cursor Green Color 1 Register */
    {0x008C,0x1F},   /* CRT/TV Ink/Cursor Red Color 1 Register */
    {0x008E,0x00},   /* CRT/TV Ink/Cursor FIFO Threshold Register */
    {0x0100,0x00},   /* BitBlt Control Register 0 */
    {0x0101,0x00},   /* BitBlt Control Register 1 */
    {0x0102,0x00},   /* BitBlt ROP Code/Color Expansion Register */
    {0x0103,0x00},   /* BitBlt Operation Register */
    {0x0104,0x00},   /* BitBlt Source Start Address Register 0 */
    {0x0105,0x00},   /* BitBlt Source Start Address Register 1 */
    {0x0106,0x00},   /* BitBlt Source Start Address Register 2 */
    {0x0108,0x00},   /* BitBlt Destination Start Address Register 0 */
    {0x0109,0x00},   /* BitBlt Destination Start Address Register 1 */
    {0x010A,0x00},   /* BitBlt Destination Start Address Register 2 */
    {0x010C,0x00},   /* BitBlt Memory Address Offset Register 0 */
    {0x010D,0x00},   /* BitBlt Memory Address Offset Register 1 */
    {0x0110,0x00},   /* BitBlt Width Register 0 */
    {0x0111,0x00},   /* BitBlt Width Register 1 */
    {0x0112,0x00},   /* BitBlt Height Register 0 */
    {0x0113,0x00},   /* BitBlt Height Register 1 */
    {0x0114,0x00},   /* BitBlt Background Color Register 0 */
    {0x0115,0x00},   /* BitBlt Background Color Register 1 */
    {0x0118,0x00},   /* BitBlt Foreground Color Register 0 */
    {0x0119,0x00},   /* BitBlt Foreground Color Register 1 */
    {0x01E0,0x01},   /* Look-Up Table Mode Register */
    {0x01E2,0x00},   /* Look-Up Table Address Register */
    {0x01E4,0x00},   /* Look-Up Table Data Register */
    {0x01F0,0x10},   /* Power Save Configuration Register */
    {0x01F1,0x00},   /* Power Save Status Register */
    {0x01F4,0x00},   /* CPU-to-Memory Access Watchdog Timer Register */
    {0x01FC,0x01},   /* Display Mode Register */
    {0, 0}
};

#endif /* CONFIG_VIDEO_SED13806_16BPP */
#endif /* CONFIG_NEC_NL6448BC20 */


#ifdef CONFIG_CONSOLE_EXTRA_INFO

/*-----------------------------------------------------------------------------
 * video_get_info_str -- setup a board string: type, speed, etc.
 * line_number= location to place info string beside logo
 * info= buffer for info string
 *-----------------------------------------------------------------------------
 */
void video_get_info_str (int line_number, char *info)
{
    if (line_number == 1) {
	strcpy (info, " RPXClassic board");
    }
    else {
	info [0] = '\0';
    }

}
#endif

/*-----------------------------------------------------------------------------
 * board_video_init -- init de l'EPSON, config du CS
 *-----------------------------------------------------------------------------
 */
unsigned int board_video_init (void)
{
    volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;

    /* Program ECCX registers                                                */
    *(ECCX_CSR12) |= ECCX_860;
    *(ECCX_CSR8) |= ECCX_BE | ECCX_CS2;
    *(ECCX_CSR8) |= ECCX_ENEPSON;

    memctl->memc_or2 = SED13806_OR;
    memctl->memc_br2 = SED13806_REG_ADDR | SED13806_ACCES;

    return (SED13806_REG_ADDR);
}

/*-----------------------------------------------------------------------------
 * board_validate_screen --
 *-----------------------------------------------------------------------------
 */
void board_validate_screen (unsigned int base)
{
    /* Activate the panel bias power                                         */
    *(volatile unsigned char *)(base + REG_GPIO_CTRL) = 0x80;
}
/*-----------------------------------------------------------------------------
 * board_get_regs --
 *-----------------------------------------------------------------------------
 */
const S1D_REGS *board_get_regs (void)
{
    return (init_regs);
}
/*-----------------------------------------------------------------------------
 * board_get_width --
 *-----------------------------------------------------------------------------
 */
int board_get_width (void)
{
    return (DISPLAY_WIDTH);
}

/*-----------------------------------------------------------------------------
 * board_get_height --
 *-----------------------------------------------------------------------------
 */
int board_get_height (void)
{
    return (DISPLAY_HEIGHT);
}

#endif /* CONFIG_VIDEO_SED13806 */
