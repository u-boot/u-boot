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
* Environment:  *** TODO: ADD YOUR OS ENVIRONMENT NAME HERE ***
*
* Description:  **** implementation for the SciTech cross platform
*               event library.
*
****************************************************************************/

/*---------------------------- Global Variables ---------------------------*/

static ushort       keyUpMsg[256] = {0};/* Table of key up messages     */
static int          rangeX,rangeY;      /* Range of mouse coordinates   */

/*---------------------------- Implementation -----------------------------*/

/* These are not used under non-DOS systems */
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
This function is used to return the number of ticks since system
startup in milliseconds. This should be the same value that is placed into
the time stamp fields of events, and is used to implement auto mouse down
events.
****************************************************************************/
ulong _EVT_getTicks(void)
{
    /* TODO: Implement this for your OS! */
}

/****************************************************************************
REMARKS:
Pumps all messages in the application message queue into our event queue.
****************************************************************************/
static void _EVT_pumpMessages(void)
{
    /* TODO: The purpose of this function is to read all keyboard and mouse */
    /*       events from the OS specific event queue, translate them and post */
    /*       them into the SciTech event queue. */
    /* */
    /* NOTE: There are a couple of important things that this function must */
    /*       take care of: */
    /* */
    /*  1. Support for KEYDOWN, KEYREPEAT and KEYUP is required. */
    /* */
    /*  2. Support for reading hardware scan code as well as ASCII */
    /*     translated values is required. Games use the scan codes rather */
    /*     than ASCII values. Scan codes go into the high order byte of the */
    /*     keyboard message field. */
    /* */
    /*  3. Support for at least reading mouse motion data (mickeys) from the */
    /*     mouse is required. Using the mickey values, we can then translate */
    /*     to mouse cursor coordinates scaled to the range of the current */
    /*     graphics display mode. Mouse values are scaled based on the */
    /*     global 'rangeX' and 'rangeY'. */
    /* */
    /*  4. Support for a timestamp for the events is required, which is */
    /*     defined as the number of milliseconds since some event (usually */
    /*     system startup). This is the timestamp when the event occurred */
    /*     (ie: at interrupt time) not when it was stuff into the SciTech */
    /*     event queue. */
    /* */
    /*  5. Support for mouse double click events. If the OS has a native */
    /*     mechanism to determine this, it should be used. Otherwise the */
    /*     time stamp information will be used by the generic event code */
    /*     to generate double click events. */
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
    _mouseMove = mouseMove;
    initEventQueue();
    memset(keyUpMsg,0,sizeof(keyUpMsg));

    /* TODO: Do any OS specific initialisation here */

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
    /* Do nothing for non DOS systems */
}

/****************************************************************************
REMARKS
Suspends all of our event handling operations. This is also used to
de-install the event handling code.
****************************************************************************/
void EVT_suspend(void)
{
    /* Do nothing for non DOS systems */
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

    /* TODO: Do any OS specific cleanup in here */
}
