/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*                   Portions copyright (C) Josh Vanderhoof
*
* Language:     ANSI C
* Environment:  Any
*
* Description:  Functions to save and restore the VGA hardware state.
*
****************************************************************************/

#include "pmapi.h"
#if defined(__WIN32_VXD__) || defined(__NT_DRIVER__)
#include "sdd/sddhelp.h"
#else
#include <string.h>
#endif

/*--------------------------- Global variables ----------------------------*/

/* VGA index register ports */
#define CRT_I   0x3D4       /* CRT Controller Index                     */
#define ATT_IW  0x3C0       /* Attribute Controller Index & Data        */
#define GRA_I   0x3CE       /* Graphics Controller Index                */
#define SEQ_I   0x3C4       /* Sequencer Index                          */

/* VGA data register ports */
#define CRT_D   0x3D5       /* CRT Controller Data Register             */
#define ATT_R   0x3C1       /* Attribute Controller Data Read Register  */
#define GRA_D   0x3CF       /* Graphics Controller Data Register        */
#define SEQ_D   0x3C5       /* Sequencer Data Register                  */
#define MIS_R   0x3CC       /* Misc Output Read Register                */
#define MIS_W   0x3C2       /* Misc Output Write Register               */
#define IS1_R   0x3DA       /* Input Status Register 1                  */
#define PEL_IW  0x3C8       /* PEL Write Index                          */
#define PEL_IR  0x3C7       /* PEL Read Index                           */
#define PEL_D   0x3C9       /* PEL Data Register                        */

/* standard VGA indexes max counts */
#define CRT_C   24          /* 24  CRT Controller Registers             */
#define ATT_C   21          /* 21  Attribute Controller Registers       */
#define GRA_C   9           /* 9   Graphics Controller Registers        */
#define SEQ_C   5           /* 5   Sequencer Registers                  */
#define MIS_C   1           /* 1   Misc Output Register                 */
#define PAL_C   768         /* 768 Palette Registers                    */
#define FONT_C  8192        /* Total size of character generator RAM    */

/* VGA registers saving indexes */
#define CRT     0           /* CRT Controller Registers start           */
#define ATT     (CRT+CRT_C) /* Attribute Controller Registers start     */
#define GRA     (ATT+ATT_C) /* Graphics Controller Registers start      */
#define SEQ     (GRA+GRA_C) /* Sequencer Registers                      */
#define MIS     (SEQ+SEQ_C) /* General Registers                        */
#define PAL     (MIS+MIS_C) /* VGA Palette Registers                    */
#define FONT    (PAL+PAL_C) /* VGA font data                            */

/* Macros for port I/O with arguments reversed */

#define _port_out(v,p)  PM_outpb(p,(uchar)(v))
#define _port_in(p)     PM_inpb(p)

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Returns the size of the VGA state buffer.
****************************************************************************/
int PMAPI PM_getVGAStateSize(void)
{
    return CRT_C + ATT_C + GRA_C + SEQ_C + MIS_C + PAL_C + FONT_C;
}

/****************************************************************************
REMARKS:
Delay for a short period of time.
****************************************************************************/
static void vga_delay(void)
{
    int i;

    /* For the loop here we program the POST register. The length of this
     * delay is dependant only on ISA bus speed, but it is enough for
     * what we need.
     */
    for (i = 0; i <= 10; i++)
	PM_outpb(0x80, 0);
}

/****************************************************************************
PARAMETERS:
port    - I/O port to read value from
index   - Port index to read

RETURNS:
Byte read from 'port' register 'index'.
****************************************************************************/
static ushort vga_rdinx(
    ushort port,
    ushort index)
{
    PM_outpb(port,(uchar)index);
    return PM_inpb(port+1);
}

/****************************************************************************
PARAMETERS:
port    - I/O port to write to
index   - Port index to write
value   - Byte to write to port

REMARKS:
Writes a byte value to the 'port' register 'index'.
****************************************************************************/
static void vga_wrinx(
    ushort port,
    ushort index,
    ushort value)
{
    PM_outpb(port,(uchar)index);
    PM_outpb(port+1,(uchar)value);
}

/****************************************************************************
REMARKS:
Save the color palette values
****************************************************************************/
static void vga_savepalette(
    uchar *pal)
{
    int i;

    _port_out(0, PEL_IR);
    for (i = 0; i < 768; i++) {
	vga_delay();
	*pal++ = _port_in(PEL_D);
	}
}

/****************************************************************************
REMARKS:
Restore the color palette values
****************************************************************************/
static void vga_restorepalette(
    const uchar *pal)
{
    int i;

    /* restore saved palette */
    _port_out(0, PEL_IW);
    for (i = 0; i < 768; i++) {
	vga_delay();
	_port_out(*pal++, PEL_D);
	}
}

/****************************************************************************
REMARKS:
Read the font data from the VGA character generator RAM
****************************************************************************/
static void vga_saveFont(
    uchar *data)
{
    uchar   *A0000Ptr = PM_getA0000Pointer();
    uchar   save[7];

    /* Enable access to character generator RAM */
    save[0] = (uchar)vga_rdinx(SEQ_I,0x00);
    save[1] = (uchar)vga_rdinx(SEQ_I,0x02);
    save[2] = (uchar)vga_rdinx(SEQ_I,0x04);
    save[3] = (uchar)vga_rdinx(SEQ_I,0x00);
    save[4] = (uchar)vga_rdinx(GRA_I,0x04);
    save[5] = (uchar)vga_rdinx(GRA_I,0x05);
    save[6] = (uchar)vga_rdinx(GRA_I,0x06);
    vga_wrinx(SEQ_I,0x00,0x01);
    vga_wrinx(SEQ_I,0x02,0x04);
    vga_wrinx(SEQ_I,0x04,0x07);
    vga_wrinx(SEQ_I,0x00,0x03);
    vga_wrinx(GRA_I,0x04,0x02);
    vga_wrinx(GRA_I,0x05,0x00);
    vga_wrinx(GRA_I,0x06,0x00);

    /* Copy character generator RAM */
    memcpy(data,A0000Ptr,FONT_C);

    /* Restore VGA state */
    vga_wrinx(SEQ_I,0x00,save[0]);
    vga_wrinx(SEQ_I,0x02,save[1]);
    vga_wrinx(SEQ_I,0x04,save[2]);
    vga_wrinx(SEQ_I,0x00,save[3]);
    vga_wrinx(GRA_I,0x04,save[4]);
    vga_wrinx(GRA_I,0x05,save[5]);
    vga_wrinx(GRA_I,0x06,save[6]);
}

/****************************************************************************
REMARKS:
Downloads the font data to the VGA character generator RAM
****************************************************************************/
static void vga_restoreFont(
    const uchar *data)
{
    uchar   *A0000Ptr = PM_getA0000Pointer();

    /* Enable access to character generator RAM */
    vga_wrinx(SEQ_I,0x00,0x01);
    vga_wrinx(SEQ_I,0x02,0x04);
    vga_wrinx(SEQ_I,0x04,0x07);
    vga_wrinx(SEQ_I,0x00,0x03);
    vga_wrinx(GRA_I,0x04,0x02);
    vga_wrinx(GRA_I,0x05,0x00);
    vga_wrinx(GRA_I,0x06,0x00);

    /* Copy font back to character generator RAM */
    memcpy(A0000Ptr,data,FONT_C);
}

/****************************************************************************
REMARKS:
Save the state of all VGA compatible registers
****************************************************************************/
void PMAPI PM_saveVGAState(
    void *stateBuf)
{
    uchar   *regs = stateBuf;
    int     i;

    /* Save state of VGA registers */
    for (i = 0; i < CRT_C; i++) {
	_port_out(i, CRT_I);
	regs[CRT + i] = _port_in(CRT_D);
	}
    for (i = 0; i < ATT_C; i++) {
	_port_in(IS1_R);
	vga_delay();
	_port_out(i, ATT_IW);
	vga_delay();
	regs[ATT + i] = _port_in(ATT_R);
	vga_delay();
	}
    for (i = 0; i < GRA_C; i++) {
	_port_out(i, GRA_I);
	regs[GRA + i] = _port_in(GRA_D);
	}
    for (i = 0; i < SEQ_C; i++) {
	_port_out(i, SEQ_I);
	regs[SEQ + i] = _port_in(SEQ_D);
	}
    regs[MIS] = _port_in(MIS_R);

    /* Save the VGA palette values */
    vga_savepalette(&regs[PAL]);

    /* Save the VGA character generator RAM */
    vga_saveFont(&regs[FONT]);

    /* Turn the VGA display back on */
    PM_vgaUnblankDisplay();
}

/****************************************************************************
REMARKS:
Retore the state of all VGA compatible registers
****************************************************************************/
void PMAPI PM_restoreVGAState(
    const void *stateBuf)
{
    const uchar *regs = stateBuf;
    int         i;

    /* Blank the display before we start the restore */
    PM_vgaBlankDisplay();

    /* Restore the VGA character generator RAM */
    vga_restoreFont(&regs[FONT]);

    /* Restore the VGA palette values */
    vga_restorepalette(&regs[PAL]);

    /* Restore the state of the VGA compatible registers */
    _port_out(regs[MIS], MIS_W);

    /* Delay to allow clock change to settle */
    for (i = 0; i < 10; i++)
	vga_delay();

    /* Synchronous reset on */
    _port_out(0x00,SEQ_I);
    _port_out(0x01,SEQ_D);

    /* Write seqeuencer registers */
    _port_out(1, SEQ_I);
    _port_out(regs[SEQ + 1] | 0x20, SEQ_D);
    for (i = 2; i < SEQ_C; i++) {
	_port_out(i, SEQ_I);
	_port_out(regs[SEQ + i], SEQ_D);
	}

    /* Synchronous reset off */
    _port_out(0x00,SEQ_I);
    _port_out(0x03,SEQ_D);

    /* Deprotect CRT registers 0-7 and write CRTC */
    _port_out(0x11, CRT_I);
    _port_out(_port_in(CRT_D) & 0x7F, CRT_D);
    for (i = 0; i < CRT_C; i++) {
	_port_out(i, CRT_I);
	_port_out(regs[CRT + i], CRT_D);
	}
    for (i = 0; i < GRA_C; i++) {
	_port_out(i, GRA_I);
	_port_out(regs[GRA + i], GRA_D);
	}
    for (i = 0; i < ATT_C; i++) {
	_port_in(IS1_R);        /* reset flip-flop */
	vga_delay();
	_port_out(i, ATT_IW);
	vga_delay();
	_port_out(regs[ATT + i], ATT_IW);
	vga_delay();
	}

    /* Ensure the VGA screen is turned on */
    PM_vgaUnblankDisplay();
}

/****************************************************************************
REMARKS:
Disables the VGA display for screen output making it blank.
****************************************************************************/
void PMAPI PM_vgaBlankDisplay(void)
{
    /* Turn screen off */
    _port_out(0x01, SEQ_I);
    _port_out(_port_in(SEQ_D) | 0x20, SEQ_D);

    /* Disable video output */
    _port_in(IS1_R);
    vga_delay();
     _port_out(0x00, ATT_IW);
}

/****************************************************************************
REMARKS:
Enables the VGA display for screen output.
****************************************************************************/
void PMAPI PM_vgaUnblankDisplay(void)
{
    /* Turn screen back on */
    _port_out(0x01, SEQ_I);
    _port_out(_port_in(SEQ_D) & 0xDF, SEQ_D);

    /* Enable video output */
    _port_in(IS1_R);
    vga_delay();
    _port_out(0x20, ATT_IW);
}
