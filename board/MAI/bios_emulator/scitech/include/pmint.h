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
* Environment:  Real mode and 16/32 bit Protected Mode
*
* Description:  Header file for the interrupt handling extensions to the OS
*               Portability Manager Library. These extensions includes
*               simplified interrupt handling, allowing all common interrupt
*               handlers to be hooked and handled directly with normal C
*               functions, both in 16 bit and 32 bit modes. Note however that
*               simplified handling does not mean slow performance! All low
*               level interrupt handling is done efficiently in assembler
*               for speed (well actually necessary to insulate the
*               application from the lack of far pointers in 32 bit PM). The
*               interrupt handlers currently supported are:
*
*                   Mouse (0x33 callback)
*                   Timer Tick (0x8)
*                   Keyboard (0x9 and 0x15)
*                   Control C/Break (0x23/0x1B)
*                   Critical Error (0x24)
*
****************************************************************************/

#ifndef __PMINT_H
#define __PMINT_H

/*--------------------------- Macros and Typedefs -------------------------*/

#ifdef __SMX32__
/* PC interrupts (Ensure consistent with pme.inc) */
#define PM_IRQ0      0x40
#define PM_IRQ1      (PM_IRQ0+1)
#define PM_IRQ6      (PM_IRQ0+6)
#define PM_IRQ14     (PM_IRQ0+14)
#endif

/* Define the different types of interrupt handlers that we support     */

typedef uint    (PMAPIP PM_criticalHandler)(uint axValue,uint diValue);
typedef void    (PMAPIP PM_breakHandler)(uint breakHit);
typedef short   (PMAPIP PM_key15Handler)(short scanCode);
typedef void    (PMAPIP PM_mouseHandler)(uint event, uint butstate,int x,int y,int mickeyX,int mickeyY);

/* Create a type for representing far pointers in both 16 and 32 bit
 * protected mode.
 */

#ifdef  PM386
typedef struct {
    long    off;
    short   sel;
    } PMFARPTR;
#define PMNULL  {0,0}
#else
typedef void *PMFARPTR;
#define PMNULL  NULL
#endif

/*--------------------------- Function Prototypes -------------------------*/

#ifdef  __cplusplus
extern "C" {            /* Use "C" linkage when in C++ mode */
#endif

/* Routine to load save default data segment selector value into a code
 * segment variable, and another to load the value into the DS register.
 */

void    PMAPI PM_loadDS(void);
void    PMAPI PM_saveDS(void);

/* Routine to install a mouse interrupt handling routine. The
 * mouse handler routine is a normal C function, and the PM library
 * will take care of passing the correct parameters to the function,
 * and switching to a local stack.
 *
 * Note that you _must_ lock the memory containing the mouse interrupt
 * handler with the PM_lockPages() function otherwise you may encounter
 * problems in virtual memory environments.
 */

int     PMAPI PM_setMouseHandler(int mask,PM_mouseHandler mh);
void    PMAPI PM_restoreMouseHandler(void);

/* Routine to reset the mouse driver, and re-install the current
 * mouse interrupt handler if one was currently installed (since the
 * mouse reset will automatically remove this handler.
 */

void    PMAPI PM_resetMouseDriver(int hardReset);

/* Routine to reset the mouse driver, and re-install the current
 * mouse interrupt handler if one was currently installed (since the
 * mouse reset will automatically remove this handler.
 */

void    PMAPI PM_resetMouseDriver(int hardReset);

/* Routines to install and remove timer interrupt handlers.
 *
 * Note that you _must_ lock the memory containing the interrupt
 * handlers with the PM_lockPages() function otherwise you may encounter
 * problems in virtual memory environments.
 */

void    PMAPI PM_setTimerHandler(PM_intHandler ih);
void    PMAPI PM_chainPrevTimer(void);
void    PMAPI PM_restoreTimerHandler(void);

/* Routines to install and keyboard interrupt handlers.
 *
 * Note that you _must_ lock the memory containing the interrupt
 * handlers with the PM_lockPages() function otherwise you may encounter
 * problems in virtual memory environments.
 */

void    PMAPI PM_setKeyHandler(PM_intHandler ih);
void    PMAPI PM_chainPrevKey(void);
void    PMAPI PM_restoreKeyHandler(void);

/* Routines to hook and unhook the alternate Int 15h keyboard intercept
 * callout routine. Your event handler will need to return the following:
 *
 *  scanCode    - Let the BIOS process scan code (chains to previous handler)
 *  0           - You have processed the scan code so flush from BIOS
 *
 * Note that this is not available under all DOS extenders, but does
 * work under real mode, DOS4GW and X32-VM. It does not work under the
 * PowerPack 32 bit DOS extenders. If you figure out how to do it let us know!
 */

void    PMAPI PM_setKey15Handler(PM_key15Handler ih);
void    PMAPI PM_restoreKey15Handler(void);

/* Routines to install and remove the control c/break interrupt handlers.
 * Interrupt handling is performed by the PM/Pro library, and you can call
 * the supplied routines to test the status of the Ctrl-C and Ctrl-Break
 * flags. If you pass the value TRUE for 'clearFlag' to these routines,
 * the internal flags will be reset in order to catch another Ctrl-C or
 * Ctrl-Break interrupt.
 */

void    PMAPI PM_installBreakHandler(void);
int     PMAPI PM_ctrlCHit(int clearFlag);
int     PMAPI PM_ctrlBreakHit(int clearFlag);
void    PMAPI PM_restoreBreakHandler(void);

/* Routine to install an alternate break handler that will call your
 * code directly. This is not available under all DOS extenders, but does
 * work under real mode, DOS4GW and X32-VM. It does not work under the
 * PowerPack 32 bit DOS extenders. If you figure out how to do it let us know!
 *
 * Note that you should either install one or the other, but not both!
 */

void    PMAPI PM_installAltBreakHandler(PM_breakHandler bh);

/* Routines to install and remove the critical error handler. The interrupt
 * is handled by the PM/Pro library, and the operation will always be failed.
 * You can check the status of the critical error handler with the
 * appropriate function. If you pass the value TRUE for 'clearFlag', the
 * internal flag will be reset ready to catch another critical error.
 */

void    PMAPI PM_installCriticalHandler(void);
int     PMAPI PM_criticalError(int *axValue, int *diValue, int clearFlag);
void    PMAPI PM_restoreCriticalHandler(void);

/* Routine to install an alternate critical handler that will call your
 * code directly. This is not available under all DOS extenders, but does
 * work under real mode, DOS4GW and X32-VM. It does not work under the
 * PowerPack 32 bit DOS extenders. If you figure out how to do it let us know!
 *
 * Note that you should either install one or the other, but not both!
 */

void    PMAPI PM_installAltCriticalHandler(PM_criticalHandler);

/* Functions to manage protected mode only interrupt handlers */

void    PMAPI PM_getPMvect(int intno, PMFARPTR *isr);
void    PMAPI PM_setPMvect(int intno, PM_intHandler ih);
void    PMAPI PM_restorePMvect(int intno, PMFARPTR isr);

#ifdef  __cplusplus
}                       /* End of "C" linkage for C++   */
#endif

#endif /* __PMINT_H */
