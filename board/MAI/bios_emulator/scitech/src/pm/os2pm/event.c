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
* Environment:  IBM PC (OS/2)
*
* Description:  OS/2 implementation for the SciTech cross platform
*               event library.
*
****************************************************************************/

/*---------------------------- Global Variables ---------------------------*/

static int          oldMouseState;      /* Old mouse state              */
static ulong        oldKeyMessage;      /* Old keyboard state           */
static ushort       keyUpMsg[256] = {0};/* Table of key up messages     */
static int          rangeX,rangeY;      /* Range of mouse coordinates   */
HMOU                _EVT_hMouse;        /* Handle to the mouse driver   */

/*---------------------------- Implementation -----------------------------*/

/* These are not used under OS/2 */
#define _EVT_disableInt()       1
#define _EVT_restoreInt(flags)

/****************************************************************************
PARAMETERS:
scanCode    - Scan code to test

REMARKS:
This macro determines if a specified key is currently down at the
time that the call is made.
****************************************************************************/
#define _EVT_isKeyDown(scanCode)    (keyUpMsg[scanCode] != 0)

/****************************************************************************
REMARKS:
Pumps all messages in the message queue from OS/2 into our event queue.
****************************************************************************/
static void _EVT_pumpMessages(void)
{
    /* TODO: Implement this for OS/2 Presentation Manager apps! */
}

/****************************************************************************
REMARKS:
This macro/function is used to converts the scan codes reported by the
keyboard to our event libraries normalised format. We only have one scan
code for the 'A' key, and use shift modifiers to determine if it is a
Ctrl-F1, Alt-F1 etc. The raw scan codes from the keyboard work this way,
but the OS gives us 'cooked' scan codes, we have to translate them back
to the raw format.
****************************************************************************/
#define _EVT_maskKeyCode(evt)

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
    /* Initialise the event queue */
    EVT.mouseMove = mouseMove;
    initEventQueue();
    oldMouseState = 0;
    oldKeyMessage = 0;
    memset(keyUpMsg,0,sizeof(keyUpMsg));

    /* TODO: OS/2 PM specific initialisation code! */

    /* Catch program termination signals so we can clean up properly */
    signal(SIGABRT, _EVT_abort);
    signal(SIGFPE, _EVT_abort);
    signal(SIGINT, _EVT_abort);
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
    rangeX = xRes;
    rangeY = yRes;
}

/****************************************************************************
REMARKS:
Initiailises the internal event handling modules. The EVT_suspend function
can be called to suspend event handling (such as when shelling out to DOS),
and this function can be used to resume it again later.
****************************************************************************/
void EVT_resume(void)
{
    /* Do nothing for OS/2 */
}

/****************************************************************************
REMARKS
Suspends all of our event handling operations. This is also used to
de-install the event handling code.
****************************************************************************/
void EVT_suspend(void)
{
    /* Do nothing for OS/2 */
}

/****************************************************************************
REMARKS
Exits the event module for program terminatation.
****************************************************************************/
void EVT_exit(void)
{
    /* Restore signal handlers */
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    /* TODO: OS/2 PM specific exit code */
}

/****************************************************************************
REMARKS
Modifes the mouse coordinates as necessary if scaling to OS coordinates,
and sets the OS mouse cursor position.
****************************************************************************/
#define _EVT_setMousePos(x,y)
