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
* Environment:  32-bit SMX embedded systems development
*
* Description:  Implementation for the OS Portability Manager Library, which
*               contains functions to implement OS specific services in a
*               generic, cross platform API. Porting the OS Portability
*               Manager library is the first step to porting any SciTech
*               products to a new platform.
*
****************************************************************************/

#include "pmapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "smx/ps2mouse.h"

/*--------------------------- Global variables ----------------------------*/

static int  globalDataStart;

PM_criticalHandler  _VARAPI _PM_critHandler = NULL;
PM_breakHandler     _VARAPI _PM_breakHandler = NULL;
PM_intHandler       _VARAPI _PM_timerHandler = NULL;
PM_intHandler       _VARAPI _PM_rtcHandler = NULL;
PM_intHandler       _VARAPI _PM_keyHandler = NULL;
PM_key15Handler     _VARAPI _PM_key15Handler = NULL;
PM_mouseHandler     _VARAPI _PM_mouseHandler = NULL;
PM_intHandler       _VARAPI _PM_int10Handler = NULL;
int                 _VARAPI _PM_mouseMask;

uchar *     _VARAPI _PM_ctrlCPtr;               /* Location of Ctrl-C flag      */
uchar *     _VARAPI _PM_ctrlBPtr;               /* Location of Ctrl-Break flag  */
uchar *     _VARAPI _PM_critPtr;                /* Location of Critical error Bf*/
PMFARPTR    _VARAPI _PM_prevTimer = PMNULL;     /* Previous timer handler       */
PMFARPTR    _VARAPI _PM_prevRTC = PMNULL;       /* Previous RTC handler         */
PMFARPTR    _VARAPI _PM_prevKey = PMNULL;       /* Previous key handler         */
PMFARPTR    _VARAPI _PM_prevKey15 = PMNULL;     /* Previous key15 handler       */
PMFARPTR    _VARAPI _PM_prevBreak = PMNULL;     /* Previous break handler       */
PMFARPTR    _VARAPI _PM_prevCtrlC = PMNULL;     /* Previous CtrlC handler       */
PMFARPTR    _VARAPI _PM_prevCritical = PMNULL;  /* Previous critical handler    */
long        _VARAPI _PM_prevRealTimer;          /* Previous real mode timer     */
long        _VARAPI _PM_prevRealRTC;            /* Previous real mode RTC       */
long        _VARAPI _PM_prevRealKey;            /* Previous real mode key       */
long        _VARAPI _PM_prevRealKey15;          /* Previous real mode key15     */
long        _VARAPI _PM_prevRealInt10;          /* Previous real mode int 10h   */
static uchar        _PM_oldCMOSRegA;            /* CMOS register A contents     */
static uchar        _PM_oldCMOSRegB;            /* CMOS register B contents     */
static uchar        _PM_oldRTCPIC2;             /* Mask value for RTC IRQ8      */

/*----------------------------- Implementation ----------------------------*/

/* Globals for locking interrupt handlers in _pmsmx.asm */

extern int  _ASMAPI _PM_pmsmxDataStart;
extern int  _ASMAPI _PM_pmsmxDataEnd;
void _ASMAPI _PM_pmsmxCodeStart(void);
void _ASMAPI _PM_pmsmxCodeEnd(void);

/* Protected mode interrupt handlers, also called by PM callbacks below */

void _ASMAPI _PM_timerISR(void);
void _ASMAPI _PM_rtcISR(void);
void _ASMAPI _PM_keyISR(void);
void _ASMAPI _PM_key15ISR(void);
void _ASMAPI _PM_breakISR(void);
void _ASMAPI _PM_ctrlCISR(void);
void _ASMAPI _PM_criticalISR(void);
void _ASMAPI _PM_mouseISR(void);
void _ASMAPI _PM_int10PMCB(void);

/* Protected mode DPMI callback handlers */

void _ASMAPI _PM_mousePMCB(void);

/* Routine to install a mouse handler function */

void _ASMAPI _PM_setMouseHandler(int mask);

/* Routine to allocate DPMI real mode callback routines */

void _ASMAPI _DPMI_allocateCallback(void (_ASMAPI *pmcode)(),void *rmregs,long *RMCB);
void _ASMAPI _DPMI_freeCallback(long RMCB);

/* DPMI helper functions in PMLITE.C */

ulong   PMAPI DPMI_mapPhysicalToLinear(ulong physAddr,ulong limit);
int     PMAPI DPMI_setSelectorBase(ushort sel,ulong linAddr);
ulong   PMAPI DPMI_getSelectorBase(ushort sel);
int     PMAPI DPMI_setSelectorLimit(ushort sel,ulong limit);
uint    PMAPI DPMI_createSelector(ulong base,ulong limit);
void    PMAPI DPMI_freeSelector(uint sel);
int     PMAPI DPMI_lockLinearPages(ulong linear,ulong len);
int     PMAPI DPMI_unlockLinearPages(ulong linear,ulong len);

/* Functions to read and write CMOS registers */

uchar   PMAPI _PM_readCMOS(int index);
void    PMAPI _PM_writeCMOS(int index,uchar value);

/*-------------------------------------------------------------------------*/
/* Generic routines common to all environments                             */
/*-------------------------------------------------------------------------*/

void PMAPI PM_resetMouseDriver(int hardReset)
{
    ps2MouseReset();
}

void PMAPI PM_setRealTimeClockFrequency(int frequency)
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
	_PM_writeCMOS(0x0A,_PM_oldCMOSRegA);
	_PM_writeCMOS(0x0B,_PM_oldCMOSRegB & 0x0F);
	}
    else {
	/* Convert frequency value to RTC clock indexes */
	for (i = 0; convert[i] != -1; i++) {
	    if (convert[i] == frequency)
		break;
	    }

	/* Set RTC timout value and enable timeout */
	_PM_writeCMOS(0x0A,(_PM_oldCMOSRegA & 0xF0) | (i+3));
	_PM_writeCMOS(0x0B,(_PM_oldCMOSRegB & 0x0F) | 0x40);
	}
}

static void PMAPI lockPMHandlers(void)
{
    static int      locked = 0;
    int             stat = 0;
    PM_lockHandle   lh;

    /* Lock all of the code and data used by our protected mode interrupt
     * handling routines, so that it will continue to work correctly
     * under real mode.
     */
    if (!locked) {
	PM_saveDS();
	stat  = !PM_lockDataPages(&globalDataStart-2048,4096,&lh);
	stat |= !PM_lockDataPages(&_PM_pmsmxDataStart,(int)&_PM_pmsmxDataEnd - (int)&_PM_pmsmxDataStart,&lh);
	stat |= !PM_lockCodePages((__codePtr)_PM_pmsmxCodeStart,(int)_PM_pmsmxCodeEnd-(int)_PM_pmsmxCodeStart,&lh);
	if (stat) {
	    printf("Page locking services failed - interrupt handling not safe!\n");
	    exit(1);
	    }
	locked = 1;
	}
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
    PMREGS  regs;

    regs.x.ax = 0x204;
    regs.h.bl = intno;
    PM_int386(0x31,&regs,&regs);
    isr->sel = regs.x.cx;
    isr->off = regs.e.edx;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
    PMSREGS sregs;
    PMREGS  regs;

    PM_saveDS();
    regs.x.ax = 0x205;          /* Set protected mode vector        */
    regs.h.bl = intno;
    PM_segread(&sregs);
    regs.x.cx = sregs.cs;
    regs.e.edx = (uint)isr;
    PM_int386(0x31,&regs,&regs);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
    PMREGS  regs;

    regs.x.ax = 0x205;
    regs.h.bl = intno;
    regs.x.cx = isr.sel;
    regs.e.edx = isr.off;
    PM_int386(0x31,&regs,&regs);
}

static long prevRealBreak;      /* Previous real mode break handler     */
static long prevRealCtrlC;      /* Previous real mode CtrlC handler     */
static long prevRealCritical;   /* Prev real mode critical handler      */

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
    lockPMHandlers();           /* Ensure our handlers are locked   */

    _PM_mouseHandler = mh;
    return 0;
}

void PMAPI PM_restoreMouseHandler(void)
{
    if (_PM_mouseHandler)
	_PM_mouseHandler = NULL;
}

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
    PM_getPMvect(intno,pmisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
    PM_restorePMvect(intno,pmisr);
}

static void setISR(int intno, void (* PMAPI pmisr)())
{
    lockPMHandlers();           /* Ensure our handlers are locked   */
    PM_setPMvect(intno,pmisr);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
    getISR(PM_IRQ0, &_PM_prevTimer, &_PM_prevRealTimer);
    _PM_timerHandler = th;
    setISR(PM_IRQ0, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
    if (_PM_timerHandler) {
	restoreISR(PM_IRQ0, _PM_prevTimer, _PM_prevRealTimer);
	_PM_timerHandler = NULL;
	}
}

ibool PMAPI PM_setRealTimeClockHandler(PM_intHandler th,int frequency)
{
    /* Save the old CMOS real time clock values */
    _PM_oldCMOSRegA = _PM_readCMOS(0x0A);
    _PM_oldCMOSRegB = _PM_readCMOS(0x0B);

    /* Set the real time clock interrupt handler */
    getISR(0x70, &_PM_prevRTC, &_PM_prevRealRTC);
    _PM_rtcHandler = th;
    setISR(0x70, _PM_rtcISR);

    /* Program the real time clock default frequency */
    PM_setRealTimeClockFrequency(frequency);

    /* Unmask IRQ8 in the PIC2 */
    _PM_oldRTCPIC2 = PM_inpb(0xA1);
    PM_outpb(0xA1,_PM_oldRTCPIC2 & 0xFE);
    return true;
}

void PMAPI PM_restoreRealTimeClockHandler(void)
{
    if (_PM_rtcHandler) {
	/* Restore CMOS registers and mask RTC clock */
	_PM_writeCMOS(0x0A,_PM_oldCMOSRegA);
	_PM_writeCMOS(0x0B,_PM_oldCMOSRegB);
	PM_outpb(0xA1,(PM_inpb(0xA1) & 0xFE) | (_PM_oldRTCPIC2 & ~0xFE));

	/* Restore the interrupt vector */
	restoreISR(0x70, _PM_prevRTC, _PM_prevRealRTC);
	_PM_rtcHandler = NULL;
	}
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
    getISR(PM_IRQ1, &_PM_prevKey, &_PM_prevRealKey);
    _PM_keyHandler = kh;
    setISR(PM_IRQ1, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
    if (_PM_keyHandler) {
	restoreISR(PM_IRQ1, _PM_prevKey, _PM_prevRealKey);
	_PM_keyHandler = NULL;
	}
}

void PMAPI PM_setKey15Handler(PM_key15Handler kh)
{
    getISR(0x15, &_PM_prevKey15, &_PM_prevRealKey15);
    _PM_key15Handler = kh;
    setISR(0x15, _PM_key15ISR);
}

void PMAPI PM_restoreKey15Handler(void)
{
    if (_PM_key15Handler) {
	restoreISR(0x15, _PM_prevKey15, _PM_prevRealKey15);
	_PM_key15Handler = NULL;
	}
}

/* Real mode Ctrl-C and Ctrl-Break handler. This handler simply sets a
 * flag in the real mode code segment and exit. We save the location
 * of this flag in real mode memory so that both the real mode and
 * protected mode code will be modifying the same flags.
 */

static uchar ctrlHandler[] = {
    0x00,0x00,0x00,0x00,            /*  ctrlBFlag                       */
    0x66,0x2E,0xC7,0x06,0x00,0x00,
    0x01,0x00,0x00,0x00,            /*  mov     [cs:ctrlBFlag],1        */
    0xCF,                           /*  iretf                           */
    };

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
    uint    rseg,roff;

    getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
    getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
    _PM_breakHandler = bh;
    setISR(0x1B, _PM_breakISR);
    setISR(0x23, _PM_ctrlCISR);

    /* Hook the real mode vectors for these handlers, as these are not
     * normally reflected by the DPMI server up to protected mode
     */
    _PM_ctrlBPtr = PM_allocRealSeg(sizeof(ctrlHandler)*2, &rseg, &roff);
    memcpy(_PM_ctrlBPtr,ctrlHandler,sizeof(ctrlHandler));
    memcpy(_PM_ctrlBPtr+sizeof(ctrlHandler),ctrlHandler,sizeof(ctrlHandler));
    _PM_ctrlCPtr = _PM_ctrlBPtr + sizeof(ctrlHandler);
    _PM_setRMvect(0x1B,((long)rseg << 16) | (roff+4));
    _PM_setRMvect(0x23,((long)rseg << 16) | (roff+sizeof(ctrlHandler)+4));
}

void PMAPI PM_installBreakHandler(void)
{
    PM_installAltBreakHandler(NULL);
}

void PMAPI PM_restoreBreakHandler(void)
{
    if (_PM_prevBreak.sel) {
	restoreISR(0x1B, _PM_prevBreak, prevRealBreak);
	restoreISR(0x23, _PM_prevCtrlC, prevRealCtrlC);
	_PM_prevBreak.sel = 0;
	_PM_breakHandler = NULL;
	PM_freeRealSeg(_PM_ctrlBPtr);
	}
}

/* Real mode Critical Error handler. This handler simply saves the AX and
 * DI values in the real mode code segment and exits. We save the location
 * of this flag in real mode memory so that both the real mode and
 * protected mode code will be modifying the same flags.
 */

static uchar criticalHandler[] = {
    0x00,0x00,                      /*  axCode                          */
    0x00,0x00,                      /*  diCode                          */
    0x2E,0xA3,0x00,0x00,            /*  mov     [cs:axCode],ax          */
    0x2E,0x89,0x3E,0x02,0x00,       /*  mov     [cs:diCode],di          */
    0xB8,0x03,0x00,                 /*  mov     ax,3                    */
    0xCF,                           /*  iretf                           */
    };

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
    uint    rseg,roff;

    getISR(0x24, &_PM_prevCritical, &prevRealCritical);
    _PM_critHandler = ch;
    setISR(0x24, _PM_criticalISR);

    /* Hook the real mode vector, as this is not normally reflected by the
     * DPMI server up to protected mode.
     */
    _PM_critPtr = PM_allocRealSeg(sizeof(criticalHandler)*2, &rseg, &roff);
    memcpy(_PM_critPtr,criticalHandler,sizeof(criticalHandler));
    _PM_setRMvect(0x24,((long)rseg << 16) | (roff+4));
}

void PMAPI PM_installCriticalHandler(void)
{
    PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
    if (_PM_prevCritical.sel) {
	restoreISR(0x24, _PM_prevCritical, prevRealCritical);
	PM_freeRealSeg(_PM_critPtr);
	_PM_prevCritical.sel = 0;
	_PM_critHandler = NULL;
	}
}

int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    PMSREGS sregs;
    PM_segread(&sregs);
    return DPMI_lockLinearPages((uint)p + DPMI_getSelectorBase(sregs.ds),len);
}

int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    PMSREGS sregs;
    PM_segread(&sregs);
    return DPMI_unlockLinearPages((uint)p + DPMI_getSelectorBase(sregs.ds),len);
}

int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    PMSREGS sregs;
    PM_segread(&sregs);
/*AM: causes minor glitch with */
/*AM: older versions pmEasy which don't allow DPMI 06 on */
/*AM: Code selector 0x0C -- assume base is 0 which it should be. */
    return DPMI_lockLinearPages((uint)p,len);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    PMSREGS sregs;
    PM_segread(&sregs);
    return DPMI_unlockLinearPages((uint)p,len);
}
