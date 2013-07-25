/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* Video support for Epson SED13806 chipset                                  */


#ifndef _SED13806_H_
#define _SED13806_H_


/* General definitions                                                       */
#define FRAME_BUFFER_OFFSET        0x200000     /* Frame buffer offset */
#define TOTAL_SPACE_SIZE           0x400000

#define DEFAULT_VIDEO_MEMORY_SIZE  0x140000     /* Video Memory Size */

#define HWCURSORSIZE		   1024     /* Size of memory reserved
						   for HW cursor*/

/* Offset of chipset registers                                               */
#define	BLT_CTRL0	(0x0100)
#define	BLT_CTRL1	(0x0101)
#define BLT_ROP		(0x0102)
#define	BLT_OP		(0x0103)
#define BLT_SRC_ADDR0	(0x0104)
#define	BLT_SRC_ADDR1	(0x0105)
#define	BLT_SRC_ADDR2	(0x0106)
#define	BLT_DST_ADDR0	(0x0108)
#define BLT_DST_ADDR1	(0x0109)
#define	BLT_DST_ADDR2	(0x010A)
#define BLT_MEM_OFF0	(0x010C)
#define BLT_MEM_OFF1	(0x010D)
#define BLT_WIDTH0	(0x0110)
#define BLT_WIDTH1	(0x0111)
#define BLT_HEIGHT0	(0x0112)
#define BLT_HEIGHT1	(0x0113)
#define	BLT_BGCOLOR0	(0x0114)
#define	BLT_BGCOLOR1	(0x0115)
#define	BLT_FGCOLOR0	(0x0118)
#define BLT_FGCOLOR1	(0x0119)

#define BLT_REG         (0x100000)

/* Lookup table registers                                                    */
#define REG_LUT_ADDR 0x1e2
#define REG_LUT_DATA 0x1e4

/* Cursor/Ink registers                                                      */
#define LCD_CURSOR_CNTL         (0x0070)
#define LCD_CURSOR_START        (0x0071)
#define LCD_CURSOR_XL           (0x0072)
#define LCD_CURSOR_XM           (0x0073)
#define LCD_CURSOR_YL           (0x0074)
#define LCD_CURSOR_YM           (0x0075)
#define LCD_CURSOR_COL0_B       (0x0076)
#define LCD_CURSOR_COL0_G       (0x0077)
#define LCD_CURSOR_COL0_R       (0x0078)
#define LCD_CURSOR_COL1_B       (0x007A)
#define LCD_CURSOR_COL1_G       (0x007B)
#define LCD_CURSOR_COL1_R       (0x007C)
#define LCD_CURSOR_FIFO         (0x007E)

typedef struct
{
    unsigned short      Index;
    unsigned char       Value;
} S1D_REGS;


/* Board specific functions                                                  */
unsigned int board_video_init (void);
void board_validate_screen (unsigned int base);
const S1D_REGS *board_get_regs (void);
int board_get_width (void);
int board_get_height (void);

#endif /* _SED13806_H_ */
