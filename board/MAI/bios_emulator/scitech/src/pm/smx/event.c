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
* Description:  32-bit SMX implementation for the SciTech cross platform
*               event library.
*
****************************************************************************/

#include "smx/ps2mouse.h"

/*--------------------------- Global variables ----------------------------*/

ibool _VARAPI   _EVT_useEvents = true;  /* True to use event handling   */
ibool _VARAPI   _EVT_installed = 0;     /* Event handers installed?     */
uchar _VARAPI   *_EVT_biosPtr = NULL;   /* Pointer to the BIOS data area */
static ibool    haveMouse = false;      /* True if we have a mouse      */

/*---------------------------- Implementation -----------------------------*/

/* External assembler functions */

void    EVTAPI _EVT_pollJoystick(void);
uint    EVTAPI _EVT_disableInt(void);
uint    EVTAPI _EVT_restoreInt(uint flags);
void    EVTAPI _EVT_codeStart(void);
void    EVTAPI _EVT_codeEnd(void);
void    EVTAPI _EVT_cCodeStart(void);
void    EVTAPI _EVT_cCodeEnd(void);
int     EVTAPI _EVT_getKeyCode(void);
int     EVTAPI EVT_rdinx(int port,int index);
void    EVTAPI EVT_wrinx(int port,int index,int value);

/****************************************************************************
REMARKS:
Do nothing for DOS, because we are fully interrupt driven.
****************************************************************************/
#define _EVT_pumpMessages()

/****************************************************************************
REMARKS:
This function is used to return the number of ticks since system
startup in milliseconds. This should be the same value that is placed into
the time stamp fields of events, and is used to implement auto mouse down
events.
****************************************************************************/
ulong _EVT_getTicks(void)
{
    return (ulong)PM_getLong(_EVT_biosPtr+0x6C) * 55UL;
}

/****************************************************************************
REMARKS:
Include generic raw scancode keyboard module.
****************************************************************************/
#include "common/keyboard.c"

/****************************************************************************
REMARKS:
Determines if we have a mouse attached and functioning.
****************************************************************************/
static ibool detectMouse(void)
{
   return(ps2Query());
}

/****************************************************************************
PARAMETERS:
what        - Event code
message     - Event message
x,y         - Mouse position at time of event
but_stat    - Mouse button status at time of event

REMARKS:
Adds a new mouse event to the event queue. This routine is called from within
the mouse interrupt subroutine, so it must be efficient.

NOTE:   Interrupts MUST be OFF while this routine is called to ensure we have
	mutually exclusive access to our internal data structures for
	interrupt driven systems (like under DOS).
****************************************************************************/
static void addMouseEvent(
    uint what,
    uint message,
    int x,
    int y,
    int mickeyX,
    int mickeyY,
    uint but_stat)
{
    event_t evt;

    if (EVT.count < EVENTQSIZE) {
	/* Save information in event record. */
	evt.when = _EVT_getTicks();
	evt.what = what;
	evt.message = message;
	evt.modifiers = but_stat;
	evt.where_x = x;                /* Save mouse event position    */
	evt.where_y = y;
	evt.relative_x = mickeyX;
	evt.relative_y = mickeyY;
	evt.modifiers |= EVT.keyModifiers;
	addEvent(&evt);                 /* Add to tail of event queue   */
	}
}

/****************************************************************************
PARAMETERS:
mask        - Event mask
butstate    - Button state
x           - Mouse x coordinate
y           - Mouse y coordinate

REMARKS:
Mouse event handling routine. This gets called when a mouse event occurs,
and we call the addMouseEvent() routine to add the appropriate mouse event
to the event queue.

Note: Interrupts are ON when this routine is called by the mouse driver code.
/*AM: NOTE: This function has not actually been ported from DOS yet and should not */
/*AM: be installed until it is. */
****************************************************************************/
static void EVTAPI mouseISR(
    uint mask,
    uint butstate,
    int x,
    int y,
    int mickeyX,
    int mickeyY)
{
    RMREGS  regs;
    uint    ps;

    if (mask & 1) {
	/* Save the current mouse coordinates */
	EVT.mx = x; EVT.my = y;

	/* If the last event was a movement event, then modify the last
	 * event rather than post a new one, so that the queue will not
	 * become saturated. Before we modify the data structures, we
	 * MUST ensure that interrupts are off.
	 */
	ps = _EVT_disableInt();
	if (EVT.oldMove != -1) {
	    EVT.evtq[EVT.oldMove].where_x = x;          /* Modify existing one  */
	    EVT.evtq[EVT.oldMove].where_y = y;
	    EVT.evtq[EVT.oldMove].relative_x += mickeyX;
	    EVT.evtq[EVT.oldMove].relative_y += mickeyY;
	    }
	else {
	    EVT.oldMove = EVT.freeHead;         /* Save id of this move event   */
	    addMouseEvent(EVT_MOUSEMOVE,0,x,y,mickeyX,mickeyY,butstate);
	    }
	_EVT_restoreInt(ps);
	}
    if (mask & 0x2A) {
	ps = _EVT_disableInt();
	addMouseEvent(EVT_MOUSEDOWN,mask >> 1,x,y,0,0,butstate);
	EVT.oldMove = -1;
	_EVT_restoreInt(ps);
	}
    if (mask & 0x54) {
	ps = _EVT_disableInt();
	addMouseEvent(EVT_MOUSEUP,mask >> 2,x,y,0,0,butstate);
	EVT.oldMove = -1;
	_EVT_restoreInt(ps);
	}
    EVT.oldKey = -1;
}

/****************************************************************************
REMARKS:
Keyboard interrupt handler function.

NOTE:   Interrupts are OFF when this routine is called by the keyboard ISR,
	and we leave them OFF the entire time. This has been modified to work
      in conjunction with smx keyboard handler.
****************************************************************************/
static void EVTAPI keyboardISR(void)
{
   PM_chainPrevKey();
    processRawScanCode(PM_inpb(0x60));
    PM_outpb(0x20,0x20);
}

/****************************************************************************
REMARKS:
Safely abort the event module upon catching a fatal error.
****************************************************************************/
void _EVT_abort()
{
    EVT_exit();
    PM_fatalError("Unhandled exception!");
}

/****************************************************************************
PARAMETERS:
mouseMove   - Callback function to call wheneve the mouse needs to be moved

REMARKS:
Initiliase the event handling module. Here we install our mouse handling ISR
to be called whenever any button's are pressed or released. We also build
the free list of events in the event queue.

We use handler number 2 of the mouse libraries interrupt handlers for our
event handling routines.
****************************************************************************/
void EVTAPI EVT_init(
    _EVT_mouseMoveHandler mouseMove)
{
    int     i;

    EVT.mouseMove = mouseMove;
    _EVT_biosPtr = PM_getBIOSPointer();
    EVT_resume();
}

/****************************************************************************
REMARKS:
Initiailises the internal event handling modules. The EVT_suspend function
can be called to suspend event handling (such as when shelling out to DOS),
and this function can be used to resume it again later.
****************************************************************************/
void EVTAPI EVT_resume(void)
{
    static int      locked = 0;
    int             stat;
    uchar           mods;
    PM_lockHandle   lh;

    if (_EVT_useEvents) {
	/* Initialise the event queue and enable our interrupt handlers */
	initEventQueue();
	PM_setKeyHandler(keyboardISR);
	if ((haveMouse = detectMouse()) != 0)
	    PM_setMouseHandler(0xFFFF,mouseISR);

	/* Read the keyboard modifier flags from the BIOS to get the
	 * correct initialisation state. The only state we care about is
	 * the correct toggle state flags such as SCROLLLOCK, NUMLOCK and
	 * CAPSLOCK.
	 */
	EVT.keyModifiers = 0;
	mods = PM_getByte(_EVT_biosPtr+0x17);
	if (mods & 0x10)
	    EVT.keyModifiers |= EVT_SCROLLLOCK;
	if (mods & 0x20)
	    EVT.keyModifiers |= EVT_NUMLOCK;
	if (mods & 0x40)
	    EVT.keyModifiers |= EVT_CAPSLOCK;

	/* Lock all of the code and data used by our protected mode interrupt
	 * handling routines, so that it will continue to work correctly
	 * under real mode.
	 */
	if (!locked) {
	    /* It is difficult to ensure that we lock our global data, so we
	     * do this by taking the address of a variable locking all data
	     * 2Kb on either side. This should properly cover the global data
	     * used by the module (the other alternative is to declare the
	     * variables in assembler, in which case we know it will be
	     * correct).
	     */
	    stat  = !PM_lockDataPages(&EVT,sizeof(EVT),&lh);
	    stat |= !PM_lockDataPages(&_EVT_biosPtr,sizeof(_EVT_biosPtr),&lh);
	    stat |= !PM_lockCodePages((__codePtr)_EVT_cCodeStart,(int)_EVT_cCodeEnd-(int)_EVT_cCodeStart,&lh);
	    stat |= !PM_lockCodePages((__codePtr)_EVT_codeStart,(int)_EVT_codeEnd-(int)_EVT_codeStart,&lh);
	    if (stat) {
		PM_fatalError("Page locking services failed - interrupt handling not safe!");
		exit(1);
		}
	    locked = 1;
	    }

	_EVT_installed = true;
	}
}

/****************************************************************************
REMARKS
Changes the range of coordinates returned by the mouse functions to the
specified range of values. This is used when changing between graphics
modes set the range of mouse coordinates for the new display mode.
****************************************************************************/
void EVTAPI EVT_setMouseRange(
    int xRes,
    int yRes)
{
    if (haveMouse) {
	ps2MouseStop();
	ps2MouseStart( 0, xRes, 0, yRes, -1, -1, -1);
	}
}

/****************************************************************************
REMARKS
Modifes the mouse coordinates as necessary if scaling to OS coordinates,
and sets the OS mouse cursor position.
****************************************************************************/
void _EVT_setMousePos(
    int *x,
    int *y)
{
    if (haveMouse)
	ps2MouseMove(*x, *y);
}

/****************************************************************************
REMARKS
Suspends all of our event handling operations. This is also used to
de-install the event handling code.
****************************************************************************/
void EVTAPI EVT_suspend(void)
{
    uchar   mods;

    if (_EVT_installed) {
	PM_restoreKeyHandler();
    if (haveMouse)
	PM_restoreMouseHandler();

	/* Set the keyboard modifier flags in the BIOS to our values */
	EVT_allowLEDS(true);
	mods = PM_getByte(_EVT_biosPtr+0x17) & ~0x70;
	if (EVT.keyModifiers & EVT_SCROLLLOCK)
	    mods |= 0x10;
	if (EVT.keyModifiers & EVT_NUMLOCK)
	    mods |= 0x20;
	if (EVT.keyModifiers & EVT_CAPSLOCK)
	    mods |= 0x40;
	PM_setByte(_EVT_biosPtr+0x17,mods);

	/* Flag that we are no longer installed */
	_EVT_installed = false;
	}
}

/****************************************************************************
REMARKS
Exits the event module for program terminatation.
****************************************************************************/
void EVTAPI EVT_exit(void)
{
    EVT_suspend();
}
