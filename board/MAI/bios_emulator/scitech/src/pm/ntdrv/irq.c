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
* Language:     ANSI C
* Environment:  32-bit Windows NT device drivers.
*
* Description:  Implementation for the NT driver IRQ management functions
*               for the PM library.
*
****************************************************************************/

#include "pmapi.h"
#include "pmint.h"
#include "drvlib/os/os.h"
#include "sdd/sddhelp.h"
#include "mtrr.h"
#include "oshdr.h"

/*--------------------------- Global variables ----------------------------*/

static int          globalDataStart;
static uchar        _PM_oldCMOSRegA;
static uchar        _PM_oldCMOSRegB;
static uchar        _PM_oldRTCPIC2;
static ulong        RTC_idtEntry;
PM_intHandler       _PM_rtcHandler = NULL;
PMFARPTR    _VARAPI _PM_prevRTC = PMNULL;

/*----------------------------- Implementation ----------------------------*/

/* Functions to read and write CMOS registers */

uchar   _ASMAPI _PM_readCMOS(int index);
void    _ASMAPI _PM_writeCMOS(int index,uchar value);
void    _ASMAPI _PM_rtcISR(void);
void    _ASMAPI _PM_getISR(int irq,PMFARPTR *handler);
void    _ASMAPI _PM_setISR(int irq,void *handler);
void    _ASMAPI _PM_restoreISR(int irq,PMFARPTR *handler);
void    _ASMAPI _PM_irqCodeStart(void);
void    _ASMAPI _PM_irqCodeEnd(void);

/****************************************************************************
REMARKS:
Set the real time clock frequency (for stereo modes).
****************************************************************************/
void PMAPI PM_setRealTimeClockFrequency(
    int frequency)
{
    static short convert[] = {
	8192,
	4096,
	2048,
	1024,
	512,
	256,
	128,
	64,
	32,
	16,
	8,
	4,
	2,
	-1,
	};
    int i;

    /* First clear any pending RTC timeout if not cleared */
    _PM_readCMOS(0x0C);
    if (frequency == 0) {
	/* Disable RTC timout */
	_PM_writeCMOS(0x0A,(uchar)_PM_oldCMOSRegA);
	_PM_writeCMOS(0x0B,(uchar)(_PM_oldCMOSRegB & 0x0F));
	}
    else {
	/* Convert frequency value to RTC clock indexes */
	for (i = 0; convert[i] != -1; i++) {
	    if (convert[i] == frequency)
		break;
	    }

	/* Set RTC timout value and enable timeout */
	_PM_writeCMOS(0x0A,(uchar)(0x20 | (i+3)));
	_PM_writeCMOS(0x0B,(uchar)((_PM_oldCMOSRegB & 0x0F) | 0x40));
	}
}

ibool PMAPI PM_setRealTimeClockHandler(PM_intHandler th,int frequency)
{
    static ibool    locked = false;

    /* Save the old CMOS real time clock values */
    _PM_oldCMOSRegA = _PM_readCMOS(0x0A);
    _PM_oldCMOSRegB = _PM_readCMOS(0x0B);

    /* Install the interrupt handler */
    RTC_idtEntry = 0x38;
    _PM_getISR(RTC_idtEntry, &_PM_prevRTC);
    _PM_rtcHandler = th;
    _PM_setISR(RTC_idtEntry, _PM_rtcISR);

    /* Program the real time clock default frequency */
    PM_setRealTimeClockFrequency(frequency);

    /* Unmask IRQ8 in the PIC2 */
    _PM_oldRTCPIC2 = PM_inpb(0xA1);
    PM_outpb(0xA1,(uchar)(_PM_oldRTCPIC2 & 0xFE));
    return true;
}

void PMAPI PM_restoreRealTimeClockHandler(void)
{
    if (_PM_rtcHandler) {
	/* Restore CMOS registers and mask RTC clock */
	_PM_writeCMOS(0x0A,_PM_oldCMOSRegA);
	_PM_writeCMOS(0x0B,_PM_oldCMOSRegB);
	PM_outpb(0xA1,(uchar)((PM_inpb(0xA1) & 0xFE) | (_PM_oldRTCPIC2 & ~0xFE)));

	/* Restore the interrupt vector */
	_PM_restoreISR(RTC_idtEntry, &_PM_prevRTC);
	_PM_rtcHandler = NULL;
	}
}
