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
* Environment:  QNX
*
* Description:  MTRR helper functions module. To make it easier to implement
*               the MTRR support under QNX, we simply put our ring 0 helper
*               functions into stubs that run them at ring 0 using whatever
*               mechanism is available.
*
****************************************************************************/

#include "pmapi.h"
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#ifdef __QNXNTO__
#include <sys/neutrino.h>
#include <sys/syspage.h>
#else
#include <i86.h>
#include <sys/irqinfo.h>
#endif

/*--------------------------- Global variables ----------------------------*/

#define R0_FLUSH_TLB    0
#define R0_SAVE_CR4     1
#define R0_RESTORE_CR4  2
#define R0_READ_MSR     3
#define R0_WRITE_MSR    4

typedef struct {
    int     service;
    int     reg;
    ulong   eax;
    ulong   edx;
    } R0_data;

extern volatile R0_data _PM_R0;

/*----------------------------- Implementation ----------------------------*/

#ifdef __QNXNTO__
const struct sigevent * _ASMAPI _PM_ring0_isr(void *arg, int id);
#else
pid_t far _ASMAPI _PM_ring0_isr();
#endif

/****************************************************************************
REMARKS:
Return true if ring 0 (or if we can call the helpers functions at ring 0)
****************************************************************************/
ibool _ASMAPI _MTRR_isRing0(void)
{
#ifdef __QNXNTO__
    return false;   /* Not implemented yet! */
#else
    return true;
#endif
}

/****************************************************************************
REMARKS:
Function to execute a service at ring 0. This is done using the clock
interrupt handler since the code we attach to it will always run at ring 0.
****************************************************************************/
static void CallRing0(void)
{
#ifdef __QNXNTO__
    uint    clock_intno = SYSPAGE_ENTRY(qtime)->intr;
#else
    uint    clock_intno = 0;    /* clock irq */
#endif
    int     intrid;

#ifdef __QNXNTO__
    mlock((void*)&_PM_R0, sizeof(_PM_R0));
    ThreadCtl(_NTO_TCTL_IO, 0);
#endif
#ifdef __QNXNTO__
    if ((intrid = InterruptAttach(_NTO_INTR_CLASS_EXTERNAL | clock_intno,
	_PM_ring0_isr, (void*)&_PM_R0, sizeof(_PM_R0), _NTO_INTR_FLAGS_END)) == -1) {
#else
    if ((intrid = qnx_hint_attach(clock_intno, _PM_ring0_isr, FP_SEG(&_PM_R0))) == -1) {
#endif
	perror("Attach");
	exit(-1);
	}
    while (_PM_R0.service != -1)
	;
#ifdef __QNXNTO__
    InterruptDetachId(intrid);
#else
    qnx_hint_detach(intrid);
#endif
}

/****************************************************************************
REMARKS:
Flush the translation lookaside buffer.
****************************************************************************/
void PMAPI PM_flushTLB(void)
{
    _PM_R0.service = R0_FLUSH_TLB;
    CallRing0();
}

/****************************************************************************
REMARKS:
Read and return the value of the CR4 register
****************************************************************************/
ulong _ASMAPI _MTRR_saveCR4(void)
{
    _PM_R0.service = R0_SAVE_CR4;
    CallRing0();
    return _PM_R0.reg;
}

/****************************************************************************
REMARKS:
Restore the value of the CR4 register
****************************************************************************/
void _ASMAPI _MTRR_restoreCR4(ulong cr4Val)
{
    _PM_R0.service = R0_RESTORE_CR4;
    _PM_R0.reg = cr4Val;
    CallRing0();
}

/****************************************************************************
REMARKS:
Read a machine status register for the CPU.
****************************************************************************/
void _ASMAPI _MTRR_readMSR(
    int reg,
    ulong *eax,
    ulong *edx)
{
    _PM_R0.service = R0_READ_MSR;
    _PM_R0.reg = reg;
    CallRing0();
    *eax = _PM_R0.eax;
    *edx = _PM_R0.edx;
}

/****************************************************************************
REMARKS:
Write a machine status register for the CPU.
****************************************************************************/
void _ASMAPI _MTRR_writeMSR(
    int reg,
    ulong eax,
    ulong edx)
{
    _PM_R0.service = R0_WRITE_MSR;
    _PM_R0.reg = reg;
    _PM_R0.eax = eax;
    _PM_R0.edx = edx;
    CallRing0();
}
