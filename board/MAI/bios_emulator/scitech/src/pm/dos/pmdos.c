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
* Environment:  16/32 bit DOS
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

/*--------------------------- Global variables ----------------------------*/

#ifndef REALMODE
static int  globalDataStart;
#endif

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

/* Structure to maintain information about hardware interrupt handlers,
 * include a copy of the hardware IRQ assembler thunk (one for each
 * hooked interrupt handler).
 */

typedef struct {
    uchar       IRQ;
    uchar       IRQVect;
    uchar       prevPIC;
    uchar       prevPIC2;
    PMFARPTR    prevHandler;
    long        prevRealhandler;
    uchar       thunk[1];
    /* IRQ assembler thunk follows ... */
    } _PM_IRQHandle;

/*----------------------------- Implementation ----------------------------*/

/* Globals for locking interrupt handlers in _pmdos.asm */

#ifndef REALMODE
extern int  _VARAPI _PM_pmdosDataStart;
extern int  _VARAPI _PM_pmdosDataEnd;
extern int  _VARAPI _PM_DMADataStart;
extern int  _VARAPI _PM_DMADataEnd;
void _ASMAPI _PM_pmdosCodeStart(void);
void _ASMAPI _PM_pmdosCodeEnd(void);
void _ASMAPI _PM_DMACodeStart(void);
void _ASMAPI _PM_DMACodeEnd(void);
#endif

/* Protected mode interrupt handlers, also called by PM callbacks below */

void _ASMAPI _PM_timerISR(void);
void _ASMAPI _PM_rtcISR(void);
void _ASMAPI _PM_irqISRTemplate(void);
void _ASMAPI _PM_irqISRTemplateEnd(void);
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

ibool _ASMAPI _DPMI_allocateCallback(void (_ASMAPI *pmcode)(),void *rmregs,long *RMCB);
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
    RMREGS          regs;
    PM_mouseHandler oldHandler = _PM_mouseHandler;

    PM_restoreMouseHandler();
    regs.x.ax = hardReset ? 0 : 33;
    PM_int86(0x33, &regs, &regs);
    if (oldHandler)
	PM_setMouseHandler(_PM_mouseMask, oldHandler);
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
	_PM_writeCMOS(0x0A,0x20 | (i+3));
	_PM_writeCMOS(0x0B,(_PM_oldCMOSRegB & 0x0F) | 0x40);
	}
}

#ifndef REALMODE

static void PMAPI lockPMHandlers(void)
{
    static int      locked = 0;
    int             stat;
    PM_lockHandle   lh; /* Unused in DOS */

    /* Lock all of the code and data used by our protected mode interrupt
     * handling routines, so that it will continue to work correctly
     * under real mode.
     */
    if (!locked) {
	PM_saveDS();
	stat  = !PM_lockDataPages(&globalDataStart-2048,4096,&lh);
	stat |= !PM_lockDataPages(&_PM_pmdosDataStart,(int)&_PM_pmdosDataEnd - (int)&_PM_pmdosDataStart,&lh);
	stat |= !PM_lockCodePages((__codePtr)_PM_pmdosCodeStart,(int)_PM_pmdosCodeEnd-(int)_PM_pmdosCodeStart,&lh);
	stat |= !PM_lockDataPages(&_PM_DMADataStart,(int)&_PM_DMADataEnd - (int)&_PM_DMADataStart,&lh);
	stat |= !PM_lockCodePages((__codePtr)_PM_DMACodeStart,(int)_PM_DMACodeEnd-(int)_PM_DMACodeStart,&lh);
	if (stat) {
	    printf("Page locking services failed - interrupt handling not safe!\n");
	    exit(1);
	    }
	locked = 1;
	}
}

#endif

/*-------------------------------------------------------------------------*/
/* DOS Real Mode support.                                                  */
/*-------------------------------------------------------------------------*/

#ifdef REALMODE

#ifndef MK_FP
#define MK_FP(s,o)  ( (void far *)( ((ulong)(s) << 16) + \
		    (ulong)(o) ))
#endif

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
    PM_saveDS();
    _PM_mouseHandler = mh;
    _PM_setMouseHandler(_PM_mouseMask = mask);
    return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
    union REGS      regs;

    if (_PM_mouseHandler) {
	regs.x.ax = 33;
	int86(0x33, &regs, &regs);
	_PM_mouseHandler = NULL;
	}
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
    _PM_getRMvect(0x8, (long*)&_PM_prevTimer);
    _PM_timerHandler = th;
    _PM_setRMvect(0x8, (long)_PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
    if (_PM_timerHandler) {
	_PM_setRMvect(0x8, (long)_PM_prevTimer);
	_PM_timerHandler = NULL;
	}
}

ibool PMAPI PM_setRealTimeClockHandler(PM_intHandler th,int frequency)
{
    /* Save the old CMOS real time clock values */
    _PM_oldCMOSRegA = _PM_readCMOS(0x0A);
    _PM_oldCMOSRegB = _PM_readCMOS(0x0B);

    /* Set the real time clock interrupt handler */
    _PM_getRMvect(0x70, (long*)&_PM_prevRTC);
    _PM_rtcHandler = th;
    _PM_setRMvect(0x70, (long)_PM_rtcISR);

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
	_PM_setRMvect(0x70, (long)_PM_prevRTC);
	_PM_rtcHandler = NULL;
	}
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
    _PM_getRMvect(0x9, (long*)&_PM_prevKey);
    _PM_keyHandler = kh;
    _PM_setRMvect(0x9, (long)_PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
    if (_PM_keyHandler) {
	_PM_setRMvect(0x9, (long)_PM_prevKey);
	_PM_keyHandler = NULL;
	}
}

void PMAPI PM_setKey15Handler(PM_key15Handler kh)
{
    _PM_getRMvect(0x15, (long*)&_PM_prevKey15);
    _PM_key15Handler = kh;
    _PM_setRMvect(0x15, (long)_PM_key15ISR);
}

void PMAPI PM_restoreKey15Handler(void)
{
    if (_PM_key15Handler) {
	_PM_setRMvect(0x15, (long)_PM_prevKey15);
	_PM_key15Handler = NULL;
	}
}

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
    static int  ctrlCFlag,ctrlBFlag;

    _PM_ctrlCPtr = (uchar*)&ctrlCFlag;
    _PM_ctrlBPtr = (uchar*)&ctrlBFlag;
    _PM_getRMvect(0x1B, (long*)&_PM_prevBreak);
    _PM_getRMvect(0x23, (long*)&_PM_prevCtrlC);
    _PM_breakHandler = bh;
    _PM_setRMvect(0x1B, (long)_PM_breakISR);
    _PM_setRMvect(0x23, (long)_PM_ctrlCISR);
}

void PMAPI PM_installBreakHandler(void)
{
    PM_installAltBreakHandler(NULL);
}

void PMAPI PM_restoreBreakHandler(void)
{
    if (_PM_prevBreak) {
	_PM_setRMvect(0x1B, (long)_PM_prevBreak);
	_PM_setRMvect(0x23, (long)_PM_prevCtrlC);
	_PM_prevBreak = NULL;
	_PM_breakHandler = NULL;
	}
}

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
    static  short critBuf[2];

    _PM_critPtr = (uchar*)critBuf;
    _PM_getRMvect(0x24, (long*)&_PM_prevCritical);
    _PM_critHandler = ch;
    _PM_setRMvect(0x24, (long)_PM_criticalISR);
}

void PMAPI PM_installCriticalHandler(void)
{
    PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
    if (_PM_prevCritical) {
	_PM_setRMvect(0x24, (long)_PM_prevCritical);
	_PM_prevCritical = NULL;
	_PM_critHandler = NULL;
	}
}

int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    p = p;  len = len;      /* Do nothing for real mode */
    return 1;
}

int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    p = p;  len = len;      /* Do nothing for real mode */
    return 1;
}

int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    p = p;  len = len;      /* Do nothing for real mode */
    return 1;
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    p = p;  len = len;      /* Do nothing for real mode */
    return 1;
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
    long t;
    _PM_getRMvect(intno,&t);
    *isr = (void*)t;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
    PM_saveDS();
    _PM_setRMvect(intno,(long)isr);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
    _PM_setRMvect(intno,(long)isr);
}

#endif

/*-------------------------------------------------------------------------*/
/* Phar Lap TNT DOS Extender support.                                      */
/*-------------------------------------------------------------------------*/

#ifdef TNT

#include <pldos32.h>
#include <pharlap.h>
#include <hw386.h>

static long prevRealBreak;      /* Previous real mode break handler     */
static long prevRealCtrlC;      /* Previous real mode CtrlC handler     */
static long prevRealCritical;   /* Prev real mode critical handler      */
static uchar *mousePtr;

/* The following real mode routine is used to call a 32 bit protected
 * mode FAR function from real mode. We use this for passing up control
 * from the real mode mouse callback to our protected mode code.
 */

static UCHAR realHandler[] = {      /* Real mode code generic handler   */
    0x00,0x00,0x00,0x00,            /* __PM_callProtp                   */
    0x00,0x00,                      /* __PM_protCS                      */
    0x00,0x00,0x00,0x00,            /* __PM_protHandler                 */
    0x66,0x60,                      /*  pushad                          */
    0x1E,                           /*  push    ds                      */
    0x6A,0x00,                      /*  push    0                       */
    0x6A,0x00,                      /*  push    0                       */
    0x2E,0xFF,0x36,0x04,0x00,       /*  push    [cs:__PM_protCS]        */
    0x66,0x2E,0xFF,0x36,0x06,0x00,  /*  push    [cs:__PM_protHandler]   */
    0x2E,0xFF,0x1E,0x00,0x00,       /*  call    [cs:__PM_callProtp]     */
    0x83,0xC4,0x0A,                 /*  add     sp,10                   */
    0x1F,                           /*  pop     ds                      */
    0x66,0x61,                      /*  popad                           */
    0xCB,                           /*  retf                            */
    };

/* The following functions installs the above realmode callback mechanism
 * in real mode memory for calling the protected mode routine.
 */

uchar * installCallback(void (PMAPI *pmCB)(),uint *rseg, uint *roff)
{
    CONFIG_INF  config;
    REALPTR     realBufAdr,callProtp;
    ULONG       bufSize;
    FARPTR      protBufAdr;
    uchar       *p;

    /* Get address of real mode routine to call up to protected mode    */
    _dx_rmlink_get(&callProtp, &realBufAdr, &bufSize, &protBufAdr);
    _dx_config_inf(&config, (UCHAR*)&config);

    /* Fill in the values in the real mode code segment so that it will
     * call the correct routine.
     */
    *((REALPTR*)&realHandler[0]) = callProtp;
    *((USHORT*)&realHandler[4]) = config.c_cs_sel;
    *((ULONG*)&realHandler[6]) = (ULONG)pmCB;

    /* Copy the real mode handler to real mode memory   */
    if ((p = PM_allocRealSeg(sizeof(realHandler),rseg,roff)) == NULL)
	return NULL;
    memcpy(p,realHandler,sizeof(realHandler));

    /* Skip past global variabls in real mode code segment */
    *roff += 0x0A;
    return p;
}

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
    RMREGS      regs;
    RMSREGS     sregs;
    uint        rseg,roff;

    lockPMHandlers();           /* Ensure our handlers are locked   */

    if ((mousePtr = installCallback(_PM_mouseISR, &rseg, &roff)) == NULL)
	return 0;
    _PM_mouseHandler = mh;

    /* Install the real mode mouse handler  */
    sregs.es = rseg;
    regs.x.dx = roff;
    regs.x.cx = _PM_mouseMask = mask;
    regs.x.ax = 0xC;
    PM_int86x(0x33, &regs, &regs, &sregs);
    return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
    RMREGS  regs;

    if (_PM_mouseHandler) {
	regs.x.ax = 33;
	PM_int86(0x33, &regs, &regs);
	PM_freeRealSeg(mousePtr);
	_PM_mouseHandler = NULL;
	}
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
    FARPTR  ph;

    _dx_pmiv_get(intno, &ph);
    isr->sel = FP_SEL(ph);
    isr->off = FP_OFF(ph);
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
    CONFIG_INF  config;
    FARPTR      ph;

    PM_saveDS();
    _dx_config_inf(&config, (UCHAR*)&config);
    FP_SET(ph,(uint)isr,config.c_cs_sel);
    _dx_pmiv_set(intno,ph);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
    FARPTR  ph;

    FP_SET(ph,isr.off,isr.sel);
    _dx_pmiv_set(intno,ph);
}

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
    PM_getPMvect(intno,pmisr);
    _PM_getRMvect(intno, realisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
    _PM_setRMvect(intno,realisr);
    PM_restorePMvect(intno,pmisr);
}

static void setISR(int intno, void (PMAPI *isr)())
{
    CONFIG_INF  config;
    FARPTR      ph;

    lockPMHandlers();           /* Ensure our handlers are locked   */

    _dx_config_inf(&config, (UCHAR*)&config);
    FP_SET(ph,(uint)isr,config.c_cs_sel);
    _dx_apmiv_set(intno,ph);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
    getISR(0x8, &_PM_prevTimer, &_PM_prevRealTimer);
    _PM_timerHandler = th;
    setISR(0x8, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
    if (_PM_timerHandler) {
	restoreISR(0x8, _PM_prevTimer, _PM_prevRealTimer);
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
    getISR(0x9, &_PM_prevKey, &_PM_prevRealKey);
    _PM_keyHandler = kh;
    setISR(0x9, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
    if (_PM_keyHandler) {
	restoreISR(0x9, _PM_prevKey, _PM_prevRealKey);
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

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
    static int  ctrlCFlag,ctrlBFlag;

    _PM_ctrlCPtr = (uchar*)&ctrlCFlag;
    _PM_ctrlBPtr = (uchar*)&ctrlBFlag;
    getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
    getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
    _PM_breakHandler = bh;
    setISR(0x1B, _PM_breakISR);
    setISR(0x23, _PM_ctrlCISR);
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
	}
}

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
    static short    critBuf[2];

    _PM_critPtr = (uchar*)critBuf;
    getISR(0x24, &_PM_prevCritical, &prevRealCritical);
    _PM_critHandler = ch;
    setISR(0x24, _PM_criticalISR);
}

void PMAPI PM_installCriticalHandler(void)
{
    PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
    if (_PM_prevCritical.sel) {
	restoreISR(0x24, _PM_prevCritical, prevRealCritical);
	_PM_prevCritical.sel = 0;
	_PM_critHandler = NULL;
	}
}

int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    return (_dx_lock_pgsn(p,len) == 0);
}

int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    return (_dx_ulock_pgsn(p,len) == 0);
}

int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    CONFIG_INF  config;
    FARPTR      fp;

    _dx_config_inf(&config, (UCHAR*)&config);
    FP_SET(fp,p,config.c_cs_sel);
    return (_dx_lock_pgs(fp,len) == 0);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    CONFIG_INF  config;
    FARPTR      fp;

    _dx_config_inf(&config, (UCHAR*)&config);
    FP_SET(fp,p,config.c_cs_sel);
    return (_dx_ulock_pgs(fp,len) == 0);
}

#endif

/*-------------------------------------------------------------------------*/
/* Symantec C++ DOSX and FlashTek X-32/X-32VM support                      */
/*-------------------------------------------------------------------------*/

#if defined(DOSX) || defined(X32VM)

#ifdef  X32VM
#include <x32.h>
#endif

static long prevRealBreak;      /* Previous real mode break handler     */
static long prevRealCtrlC;      /* Previous real mode CtrlC handler     */
static long prevRealCritical;   /* Prev real mode critical handler      */

static uint mouseSel = 0,mouseOff;

/* The following real mode routine is used to call a 32 bit protected
 * mode FAR function from real mode. We use this for passing up control
 * from the real mode mouse callback to our protected mode code.
 */

static char realHandler[] = {       /* Real mode code generic handler   */
    0x00,0x00,0x00,0x00,            /* __PM_callProtp                   */
    0x00,0x00,                      /* __PM_protCS                      */
    0x00,0x00,0x00,0x00,            /* __PM_protHandler                 */
    0x1E,                           /*  push    ds                      */
    0x6A,0x00,                      /*  push    0                       */
    0x6A,0x00,                      /*  push    0                       */
    0x2E,0xFF,0x36,0x04,0x00,       /*  push    [cs:__PM_protCS]        */
    0x66,0x2E,0xFF,0x36,0x06,0x00,  /*  push    [cs:__PM_protHandler]   */
    0x2E,0xFF,0x1E,0x00,0x00,       /*  call    [cs:__PM_callProtp]     */
    0x83,0xC4,0x0A,                 /*  add     sp,10                   */
    0x1F,                           /*  pop     ds                      */
    0xCB,                           /*  retf                            */
    };

/* The following functions installs the above realmode callback mechanism
 * in real mode memory for calling the protected mode routine.
 */

int installCallback(void (PMAPI *pmCB)(),uint *psel, uint *poff,
    uint *rseg, uint *roff)
{
    PMREGS          regs;
    PMSREGS         sregs;

    regs.x.ax = 0x250D;
    PM_segread(&sregs);
    PM_int386x(0x21,&regs,&regs,&sregs);    /* Get RM callback address  */

    /* Fill in the values in the real mode code segment so that it will
     * call the correct routine.
     */
    *((ulong*)&realHandler[0]) = regs.e.eax;
    *((ushort*)&realHandler[4]) = sregs.cs;
    *((ulong*)&realHandler[6]) = (ulong)pmCB;

    /* Copy the real mode handler to real mode memory (only allocate the
     * buffer once since we cant dealloate it with X32).
     */
    if (*psel == 0) {
	if (!PM_allocRealSeg(sizeof(realHandler),psel,poff,rseg,roff))
	    return 0;
	}
    PM_memcpyfn(*psel,*poff,realHandler,sizeof(realHandler));

    /* Skip past global variables in real mode code segment */
    *roff += 0x0A;
    return 1;
}

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
    RMREGS      regs;
    RMSREGS     sregs;
    uint    rseg,roff;

    lockPMHandlers();           /* Ensure our handlers are locked   */

    if (!installCallback(_PM_mouseISR, &mouseSel, &mouseOff, &rseg, &roff))
	return 0;
    _PM_mouseHandler = mh;

    /* Install the real mode mouse handler  */
    sregs.es = rseg;
    regs.x.dx = roff;
    regs.x.cx = _PM_mouseMask = mask;
    regs.x.ax = 0xC;
    PM_int86x(0x33, &regs, &regs, &sregs);
    return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
    RMREGS  regs;

    if (_PM_mouseHandler) {
	regs.x.ax = 33;
	PM_int86(0x33, &regs, &regs);
	_PM_mouseHandler = NULL;
	}
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
    PMREGS  regs;
    PMSREGS sregs;

    PM_segread(&sregs);
    regs.x.ax = 0x2502;         /* Get PM interrupt vector              */
    regs.x.cx = intno;
    PM_int386x(0x21, &regs, &regs, &sregs);
    isr->sel = sregs.es;
    isr->off = regs.e.ebx;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
    PMFARPTR    pmisr;
    PMSREGS     sregs;

    PM_saveDS();
    PM_segread(&sregs);
    pmisr.sel = sregs.cs;
    pmisr.off = (uint)isr;
    PM_restorePMvect(intno, pmisr);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
    PMREGS  regs;
    PMSREGS sregs;

    PM_segread(&sregs);
    regs.x.ax = 0x2505;         /* Set PM interrupt vector              */
    regs.x.cx = intno;
    sregs.ds = isr.sel;
    regs.e.edx = isr.off;
    PM_int386x(0x21, &regs, &regs, &sregs);
}

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
    PM_getPMvect(intno,pmisr);
    _PM_getRMvect(intno,realisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
    PMREGS  regs;
    PMSREGS sregs;

    PM_segread(&sregs);
    regs.x.ax = 0x2507;         /* Set real and PM vectors              */
    regs.x.cx = intno;
    sregs.ds = pmisr.sel;
    regs.e.edx = pmisr.off;
    regs.e.ebx = realisr;
    PM_int386x(0x21, &regs, &regs, &sregs);
}

static void setISR(int intno, void *isr)
{
    PMREGS  regs;
    PMSREGS sregs;

    lockPMHandlers();           /* Ensure our handlers are locked       */

    PM_segread(&sregs);
    regs.x.ax = 0x2506;         /* Hook real and protected vectors      */
    regs.x.cx = intno;
    sregs.ds = sregs.cs;
    regs.e.edx = (uint)isr;
    PM_int386x(0x21, &regs, &regs, &sregs);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
    getISR(0x8, &_PM_prevTimer, &_PM_prevRealTimer);
    _PM_timerHandler = th;
    setISR(0x8, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
    if (_PM_timerHandler) {
	restoreISR(0x8, _PM_prevTimer, _PM_prevRealTimer);
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
    getISR(0x9, &_PM_prevKey, &_PM_prevRealKey);
    _PM_keyHandler = kh;
    setISR(0x9, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
    if (_PM_keyHandler) {
	restoreISR(0x9, _PM_prevKey, _PM_prevRealKey);
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

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
    static int  ctrlCFlag,ctrlBFlag;

    _PM_ctrlCPtr = (uchar*)&ctrlCFlag;
    _PM_ctrlBPtr = (uchar*)&ctrlBFlag;
    getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
    getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
    _PM_breakHandler = bh;
    setISR(0x1B, _PM_breakISR);
    setISR(0x23, _PM_ctrlCISR);
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
	}
}

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
    static short    critBuf[2];

    _PM_critPtr = (uchar*)critBuf;
    getISR(0x24, &_PM_prevCritical, &prevRealCritical);
    _PM_critHandler = ch;
    setISR(0x24, _PM_criticalISR);
}

void PMAPI PM_installCriticalHandler(void)
{
    PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
    if (_PM_prevCritical.sel) {
	restoreISR(0x24, _PM_prevCritical, prevRealCritical);
	_PM_prevCritical.sel = 0;
	_PM_critHandler = NULL;
	}
}

int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    return (_x386_memlock(p,len) == 0);
}

int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    return (_x386_memunlock(p,len) == 0);
}

int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    return (_x386_memlock(p,len) == 0);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    return (_x386_memunlock(p,len) == 0);
}

#endif

/*-------------------------------------------------------------------------*/
/* Borland's DPMI32 DOS Power Pack Extender support.                       */
/*-------------------------------------------------------------------------*/

#ifdef  DPMI32
#define GENERIC_DPMI32          /* Use generic 32 bit DPMI routines */

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
#endif

/*-------------------------------------------------------------------------*/
/* Watcom C/C++ with Rational DOS/4GW support.                             */
/*-------------------------------------------------------------------------*/

#ifdef  DOS4GW
#define GENERIC_DPMI32          /* Use generic 32 bit DPMI routines */

#define MOUSE_SUPPORTED         /* DOS4GW directly supports mouse   */

/* We use the normal DOS services to save and restore interrupts handlers
 * for Watcom C++, because using the direct DPMI functions does not
 * appear to work properly. At least if we use the DPMI functions, we
 * dont get the auto-passup feature that we need to correctly trap
 * real and protected mode interrupts without installing Bi-model
 * interrupt handlers.
 */

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
    PMREGS  regs;
    PMSREGS sregs;

    PM_segread(&sregs);
    regs.h.ah = 0x35;
    regs.h.al = intno;
    PM_int386x(0x21,&regs,&regs,&sregs);
    isr->sel = sregs.es;
    isr->off = regs.e.ebx;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
    PMREGS  regs;
    PMSREGS sregs;

    PM_saveDS();
    PM_segread(&sregs);
    regs.h.ah = 0x25;
    regs.h.al = intno;
    sregs.ds = sregs.cs;
    regs.e.edx = (uint)isr;
    PM_int386x(0x21,&regs,&regs,&sregs);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
    PMREGS  regs;
    PMSREGS sregs;

    PM_segread(&sregs);
    regs.h.ah = 0x25;
    regs.h.al = intno;
    sregs.ds = isr.sel;
    regs.e.edx = isr.off;
    PM_int386x(0x21,&regs,&regs,&sregs);
}

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
    lockPMHandlers();           /* Ensure our handlers are locked   */

    _PM_mouseHandler = mh;
    _PM_setMouseHandler(_PM_mouseMask = mask);
    return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
    PMREGS  regs;

    if (_PM_mouseHandler) {
	regs.x.ax = 33;
	PM_int386(0x33, &regs, &regs);
	_PM_mouseHandler = NULL;
	}
}

#endif

/*-------------------------------------------------------------------------*/
/* DJGPP port of GNU C++ support.                                          */
/*-------------------------------------------------------------------------*/

#ifdef DJGPP
#define GENERIC_DPMI32          /* Use generic 32 bit DPMI routines */

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

#endif

/*-------------------------------------------------------------------------*/
/* Generic 32 bit DPMI routines                                            */
/*-------------------------------------------------------------------------*/

#if defined(GENERIC_DPMI32)

static long prevRealBreak;      /* Previous real mode break handler     */
static long prevRealCtrlC;      /* Previous real mode CtrlC handler     */
static long prevRealCritical;   /* Prev real mode critical handler      */

#ifndef MOUSE_SUPPORTED

/* The following real mode routine is used to call a 32 bit protected
 * mode FAR function from real mode. We use this for passing up control
 * from the real mode mouse callback to our protected mode code.
 */

static long mouseRMCB;          /* Mouse real mode callback address     */
static uchar *mousePtr;
static char mouseRegs[0x32];    /* Real mode regs for mouse callback    */
static uchar mouseHandler[] = {
    0x00,0x00,0x00,0x00,        /* _realRMCB                            */
    0x2E,0xFF,0x1E,0x00,0x00,   /*  call    [cs:_realRMCB]              */
    0xCB,                       /*  retf                                */
    };

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
    RMREGS      regs;
    RMSREGS     sregs;
    uint        rseg,roff;

    lockPMHandlers();           /* Ensure our handlers are locked   */

    /* Copy the real mode handler to real mode memory   */
    if ((mousePtr = PM_allocRealSeg(sizeof(mouseHandler),&rseg,&roff)) == NULL)
	return 0;
    memcpy(mousePtr,mouseHandler,sizeof(mouseHandler));
    if (!_DPMI_allocateCallback(_PM_mousePMCB, mouseRegs, &mouseRMCB))
	PM_fatalError("Unable to allocate real mode callback!\n");
    PM_setLong(mousePtr,mouseRMCB);

    /* Install the real mode mouse handler  */
    _PM_mouseHandler = mh;
    sregs.es = rseg;
    regs.x.dx = roff+4;
    regs.x.cx = _PM_mouseMask = mask;
    regs.x.ax = 0xC;
    PM_int86x(0x33, &regs, &regs, &sregs);
    return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
    RMREGS  regs;

    if (_PM_mouseHandler) {
	regs.x.ax = 33;
	PM_int86(0x33, &regs, &regs);
	PM_freeRealSeg(mousePtr);
	_DPMI_freeCallback(mouseRMCB);
	_PM_mouseHandler = NULL;
	}
}

#endif

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
    PM_getPMvect(intno,pmisr);
    _PM_getRMvect(intno,realisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
    _PM_setRMvect(intno,realisr);
    PM_restorePMvect(intno,pmisr);
}

static void setISR(int intno, void (* PMAPI pmisr)())
{
    lockPMHandlers();           /* Ensure our handlers are locked   */
    PM_setPMvect(intno,pmisr);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
    getISR(0x8, &_PM_prevTimer, &_PM_prevRealTimer);
    _PM_timerHandler = th;
    setISR(0x8, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
    if (_PM_timerHandler) {
	restoreISR(0x8, _PM_prevTimer, _PM_prevRealTimer);
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

PM_IRQHandle PMAPI PM_setIRQHandler(
    int IRQ,
    PM_irqHandler ih)
{
    int             thunkSize,PICmask,chainPrevious;
    ulong           offsetAdjust;
    _PM_IRQHandle   *handle;

    thunkSize = (ulong)_PM_irqISRTemplateEnd - (ulong)_PM_irqISRTemplate;
    if ((handle = PM_malloc(sizeof(_PM_IRQHandle) + thunkSize)) == NULL)
	return NULL;
    handle->IRQ = IRQ;
    handle->prevPIC = PM_inpb(0x21);
    handle->prevPIC2 = PM_inpb(0xA1);
    if (IRQ < 8) {
	handle->IRQVect = (IRQ + 8);
	PICmask = (1 << IRQ);
	chainPrevious = ((handle->prevPIC & PICmask) == 0);
	}
    else {
	handle->IRQVect = (0x60 + IRQ + 8);
	PICmask = ((1 << IRQ) | 0x4);
	chainPrevious = ((handle->prevPIC2 & (PICmask >> 8)) == 0);
	}

    /* Copy and setup the assembler thunk */
    offsetAdjust = (ulong)handle->thunk - (ulong)_PM_irqISRTemplate;
    memcpy(handle->thunk,_PM_irqISRTemplate,thunkSize);
    *((ulong*)&handle->thunk[2]) = offsetAdjust;
    *((ulong*)&handle->thunk[11+0]) = (ulong)ih;
    if (chainPrevious) {
	*((ulong*)&handle->thunk[11+4]) = handle->prevHandler.off;
	*((ulong*)&handle->thunk[11+8]) = handle->prevHandler.sel;
	}
    else {
	*((ulong*)&handle->thunk[11+4]) = 0;
	*((ulong*)&handle->thunk[11+8]) = 0;
	}
    *((ulong*)&handle->thunk[11+12]) = IRQ;

    /* Set the real time clock interrupt handler */
    getISR(handle->IRQVect, &handle->prevHandler, &handle->prevRealhandler);
    setISR(handle->IRQVect, (PM_intHandler)handle->thunk);

    /* Unmask the IRQ in the PIC */
    PM_outpb(0xA1,handle->prevPIC2 & ~(PICmask >> 8));
    PM_outpb(0x21,handle->prevPIC & ~PICmask);
    return handle;
}

void PMAPI PM_restoreIRQHandler(
    PM_IRQHandle irqHandle)
{
    int             PICmask;
    _PM_IRQHandle   *handle = irqHandle;

    /* Restore PIC mask for the interrupt */
    if (handle->IRQ < 8)
	PICmask = (1 << handle->IRQ);
    else
	PICmask = ((1 << handle->IRQ) | 0x4);
    PM_outpb(0xA1,(PM_inpb(0xA1) & ~(PICmask >> 8)) | (handle->prevPIC2 & (PICmask >> 8)));
    PM_outpb(0x21,(PM_inpb(0x21) & ~PICmask) | (handle->prevPIC & PICmask));

    /* Restore the interrupt vector */
    restoreISR(handle->IRQVect, handle->prevHandler, handle->prevRealhandler);

    /* Finally free the thunk */
    PM_free(handle);
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
    getISR(0x9, &_PM_prevKey, &_PM_prevRealKey);
    _PM_keyHandler = kh;
    setISR(0x9, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
    if (_PM_keyHandler) {
	restoreISR(0x9, _PM_prevKey, _PM_prevRealKey);
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

#ifndef DOS4GW
static uchar ctrlHandler[] = {
    0x00,0x00,0x00,0x00,            /*  ctrlBFlag                       */
    0x66,0x2E,0xC7,0x06,0x00,0x00,
    0x01,0x00,0x00,0x00,            /*  mov     [cs:ctrlBFlag],1        */
    0xCF,                           /*  iretf                           */
    };
#endif

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
#ifndef DOS4GW
    uint    rseg,roff;
#else
    static int  ctrlCFlag,ctrlBFlag;

    _PM_ctrlCPtr = (uchar*)&ctrlCFlag;
    _PM_ctrlBPtr = (uchar*)&ctrlBFlag;
#endif

    getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
    getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
    _PM_breakHandler = bh;
    setISR(0x1B, _PM_breakISR);
    setISR(0x23, _PM_ctrlCISR);

#ifndef DOS4GW
    /* Hook the real mode vectors for these handlers, as these are not
     * normally reflected by the DPMI server up to protected mode
     */
    _PM_ctrlBPtr = PM_allocRealSeg(sizeof(ctrlHandler)*2, &rseg, &roff);
    memcpy(_PM_ctrlBPtr,ctrlHandler,sizeof(ctrlHandler));
    memcpy(_PM_ctrlBPtr+sizeof(ctrlHandler),ctrlHandler,sizeof(ctrlHandler));
    _PM_ctrlCPtr = _PM_ctrlBPtr + sizeof(ctrlHandler);
    _PM_setRMvect(0x1B,((long)rseg << 16) | (roff+4));
    _PM_setRMvect(0x23,((long)rseg << 16) | (roff+sizeof(ctrlHandler)+4));
#endif
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
#ifndef DOS4GW
	PM_freeRealSeg(_PM_ctrlBPtr);
#endif
	}
}

/* Real mode Critical Error handler. This handler simply saves the AX and
 * DI values in the real mode code segment and exits. We save the location
 * of this flag in real mode memory so that both the real mode and
 * protected mode code will be modifying the same flags.
 */

#ifndef DOS4GW
static uchar criticalHandler[] = {
    0x00,0x00,                      /*  axCode                          */
    0x00,0x00,                      /*  diCode                          */
    0x2E,0xA3,0x00,0x00,            /*  mov     [cs:axCode],ax          */
    0x2E,0x89,0x3E,0x02,0x00,       /*  mov     [cs:diCode],di          */
    0xB8,0x03,0x00,                 /*  mov     ax,3                    */
    0xCF,                           /*  iretf                           */
    };
#endif

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
#ifndef DOS4GW
    uint    rseg,roff;
#else
    static  short   critBuf[2];

    _PM_critPtr = (uchar*)critBuf;
#endif

    getISR(0x24, &_PM_prevCritical, &prevRealCritical);
    _PM_critHandler = ch;
    setISR(0x24, _PM_criticalISR);

#ifndef DOS4GW
    /* Hook the real mode vector, as this is not normally reflected by the
     * DPMI server up to protected mode.
     */
    _PM_critPtr = PM_allocRealSeg(sizeof(criticalHandler)*2, &rseg, &roff);
    memcpy(_PM_critPtr,criticalHandler,sizeof(criticalHandler));
    _PM_setRMvect(0x24,((long)rseg << 16) | (roff+4));
#endif
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
    return DPMI_lockLinearPages((uint)p + DPMI_getSelectorBase(sregs.cs),len);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    PMSREGS sregs;
    PM_segread(&sregs);
    return DPMI_unlockLinearPages((uint)p + DPMI_getSelectorBase(sregs.cs),len);
}

#endif
