/*
 * (C) Copyright 1997-2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * smiLynxEM.h
 * Silicon Motion graphic interface for sm810/sm710/sm712 accelerator
 *
 *
 *  modification history
 *  --------------------
 *  04-18-2002 Rewritten for U-Boot <fgottschling@eltec.de>.
 */

#ifndef _SMI_LYNX_EM_H_
#define _SMI_LYNX_EM_H_

/*
 * SMI 710/712 have 4MB internal RAM; SMI 810 2MB internal + 2MB external
 */
#define VIDEO_MEM_SIZE  0x400000

/*
 * Supported video modes for SMI Lynx E/EM/EM+
 */
#define VIDEO_MODES             7
#define DUAL_800_600            0   /* SMI710:VGA1:75Hz     (pitch=1600) */
				    /*        VGA2:60/120Hz (pitch=1600) */
				    /* SMI810:VGA1:75Hz     (pitch=1600) */
				    /*        VGA2:75Hz     (pitch=1600) */
#define DUAL_1024_768           1   /* VGA1:75Hz VGA2:73Hz (pitch=2048)  */
#define SINGLE_800_600          2   /* VGA1:75Hz (pitch=800)             */
#define SINGLE_1024_768         3   /* VGA1:75Hz (pitch=1024)            */
#define SINGLE_1280_1024        4   /* VGA1:75Hz (pitch=1280)            */
#define TV_MODE_CCIR            5   /* VGA1:50Hz (h=720;v=576;pitch=720) */
#define TV_MODE_EIA             6   /* VGA1:60Hz (h=720;v=484;pitch=720) */


/*
 * ISA mapped regs
 */
#define SMI_INDX_C4             (pGD->isaBase + 0x03c4)    /* index reg */
#define SMI_DATA_C5             (pGD->isaBase + 0x03c5)    /* data reg */
#define SMI_INDX_D4             (pGD->isaBase + 0x03d4)    /* index reg */
#define SMI_DATA_D5             (pGD->isaBase + 0x03d5)    /* data reg */
#define SMI_INDX_CE             (pGD->isaBase + 0x03ce)    /* index reg */
#define SMI_DATA_CF             (pGD->isaBase + 0x03cf)    /* data reg */
#define SMI_LOCK_REG            (pGD->isaBase + 0x03c3)    /* unlock/lock ext crt reg */
#define SMI_MISC_REG            (pGD->isaBase + 0x03c2)    /* misc reg */
#define SMI_LUT_MASK            (pGD->isaBase + 0x03c6)    /* lut mask reg */
#define SMI_LUT_START           (pGD->isaBase + 0x03c8)    /* lut start index */
#define SMI_LUT_RGB             (pGD->isaBase + 0x03c9)    /* lut colors auto incr.*/


/*
 * Video processor control
 */
typedef struct {
    unsigned int   control;
    unsigned int   colorKey;
    unsigned int   colorKeyMask;
    unsigned int   start;
    unsigned short offset;
    unsigned short width;
    unsigned int   fifoPrio;
    unsigned int   fifoERL;
    unsigned int   YUVtoRGB;
} SmiVideoProc;

/*
 * Video window control
 */
typedef struct {
    unsigned short top;
    unsigned short left;
    unsigned short bottom;
    unsigned short right;
    unsigned int   srcStart;
    unsigned short width;
    unsigned short offset;
    unsigned char  hStretch;
    unsigned char  vStretch;
} SmiVideoWin;

/*
 * Capture port control
 */
typedef struct {
    unsigned int   control;
    unsigned short topClip;
    unsigned short leftClip;
    unsigned short srcHeight;
    unsigned short srcWidth;
    unsigned int   srcBufStart1;
    unsigned int   srcBufStart2;
    unsigned short srcOffset;
    unsigned short fifoControl;
} SmiCapturePort;


/******************************************************************************/
/* Export Graphic Driver Control                                              */
/******************************************************************************/

typedef struct {
    unsigned int isaBase;
    unsigned int pciBase;
    unsigned int dprBase;
    unsigned int vprBase;
    unsigned int cprBase;
    unsigned int frameAdrs;
    unsigned int memSize;
    unsigned int mode;
    unsigned int gdfIndex;
    unsigned int gdfBytesPP;
    unsigned int fg;
    unsigned int bg;
    unsigned int plnSizeX;
    unsigned int plnSizeY;
    unsigned int winSizeX;
    unsigned int winSizeY;
    char modeIdent[80];
} GraphicDevice;

extern GraphicDevice smi;


/******************************************************************************/
/* Export Graphic Functions                                                   */
/******************************************************************************/

void *video_hw_init (void);       /* returns GraphicDevice struct or NULL */

void video_hw_bitblt (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int src_x,           /* source pos x */
    unsigned int src_y,           /* source pos y */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y            /* frame height */
    );

void video_hw_rectfill (
    unsigned int bpp,             /* bytes per pixel */
    unsigned int dst_x,           /* dest pos x */
    unsigned int dst_y,           /* dest pos y */
    unsigned int dim_x,           /* frame width */
    unsigned int dim_y,           /* frame height */
    unsigned int color            /* fill color */
     );

void video_set_lut (
    unsigned int index,           /* color number */
    unsigned char r,              /* red */
    unsigned char g,              /* green */
    unsigned char b               /* blue */
    );

#endif /*_SMI_LYNX_EM_H_ */
