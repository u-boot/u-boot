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
* Environment:  Any
*
* Description:  Main implementation for the SciTech cross platform event
*               library. This module contains all the generic cross platform
*               code, and pulls in modules specific to each target OS
*               environment.
*
****************************************************************************/

#include "event.h"
#include "pmapi.h"
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oshdr.h"

/*--------------------------- Global variables ----------------------------*/

#define EVENTQSIZE      100     /* Number of events in event queue      */
#define JOY_NUM_AXES    4       /* Number of joystick axes supported    */

static struct {
    int         mx,my;              /* Current mouse position           */
    int         head;               /* Head of event queue              */
    int         tail;               /* Tail of event queue              */
    int         freeHead;           /* Head of free list                */
    int         count;              /* No. of items currently in queue  */
    event_t     evtq[EVENTQSIZE];   /* The queue structure itself       */
    int         oldMove;            /* Previous movement event          */
    int         oldKey;             /* Previous key repeat event        */
    int         oldJoyMove;         /* Previous joystick movement event */
    int         joyMask;            /* Mask of joystick axes present    */
    int         joyMin[JOY_NUM_AXES];
    int         joyCenter[JOY_NUM_AXES];
    int         joyMax[JOY_NUM_AXES];
    int         joyPrev[JOY_NUM_AXES];
    int         joyButState;
    ulong       doubleClick;
    ulong       autoRepeat;
    ulong       autoDelay;
    ulong       autoTicks;
    ulong       doubleClickThresh;
    ulong       firstAuto;
    int         autoMouse_x;
    int         autoMouse_y;
    event_t     downMouse;
    ulong       keyModifiers;       /* Current keyboard modifiers       */
    uchar       keyTable[128];      /* Table of key up/down flags       */
    ibool       allowLEDS;          /* True if LEDS should change       */
    _EVT_userEventFilter    userEventCallback;
    _EVT_mouseMoveHandler   mouseMove;
    _EVT_heartBeatCallback  heartBeat;
    void                    *heartBeatParams;
    codepage_t              *codePage;
    } EVT;

/*---------------------------- Implementation -----------------------------*/

#if defined(__REALDOS__) || defined(__SMX32__)
/* {secret} */
void EVTAPI _EVT_cCodeStart(void) {}
#endif

/* External assembler functions */

int EVTAPI _EVT_readJoyAxis(int mask,int *axis);
int EVTAPI _EVT_readJoyButtons(void);

/* Forward declaration */

ulong _EVT_getTicks(void);

/****************************************************************************
PARAMETERS:
evt - Event to add to the event queue

REMARKS:
Adds an event to the event queue by tacking it onto the tail of the event
queue. This routine assumes that at least one spot is available on the
freeList for the event to be inserted.

NOTE:   Interrupts MUST be OFF while this routine is called to ensure we have
	mutually exclusive access to our internal data structures for
	interrupt driven systems (like under DOS).
****************************************************************************/
static void addEvent(
    event_t *evt)
{
    int evtID;

    /* Check for mouse double click events */
    if (evt->what & EVT_MOUSEEVT) {
	EVT.autoMouse_x = evt->where_x;
	EVT.autoMouse_y = evt->where_y;
	if ((evt->what & EVT_MOUSEDOWN) && !(evt->message & EVT_DBLCLICK)) {
	    /* Determine if the last mouse event was a double click event */
	    uint diff_x = ABS(evt->where_x - EVT.downMouse.where_x);
	    uint diff_y = ABS(evt->where_y - EVT.downMouse.where_y);
	    if ((evt->message == EVT.downMouse.message)
		&& ((evt->when - EVT.downMouse.when) <= EVT.doubleClick)
		&& (diff_x <= EVT.doubleClickThresh)
		&& (diff_y <= EVT.doubleClickThresh)) {
		evt->message |= EVT_DBLCLICK;
		EVT.downMouse = *evt;
		EVT.downMouse.when = 0;
		}
	    else
		EVT.downMouse = *evt;
	    EVT.autoTicks = _EVT_getTicks();
	    }
	else if (evt->what & EVT_MOUSEUP) {
	    EVT.downMouse.what = EVT_NULLEVT;
	    EVT.firstAuto = true;
	    }
	}

    /* Call user supplied callback to modify the event if desired */
    if (EVT.userEventCallback) {
	if (!EVT.userEventCallback(evt))
	    return;
	}

    /* Get spot to place the event from the free list */
    evtID = EVT.freeHead;
    EVT.freeHead = EVT.evtq[EVT.freeHead].next;

    /* Add to the EVT.tail of the event queue   */
    evt->next = -1;
    evt->prev = EVT.tail;
    if (EVT.tail != -1)
	EVT.evtq[EVT.tail].next = evtID;
    else
	EVT.head = evtID;
    EVT.tail = evtID;
    EVT.evtq[evtID] = *evt;
    EVT.count++;
}

/****************************************************************************
REMARKS:
Internal function to initialise the event queue to the empty state.
****************************************************************************/
static void initEventQueue(void)
{
    int i;

    /* Build free list, and initialize global data structures */
    for (i = 0; i < EVENTQSIZE; i++)
	EVT.evtq[i].next = i+1;
    EVT.evtq[EVENTQSIZE-1].next = -1;       /* Terminate list           */
    EVT.count = EVT.freeHead = 0;
    EVT.head = EVT.tail = -1;
    EVT.oldMove = -1;
    EVT.oldKey = -1;
    EVT.oldJoyMove = -1;
    EVT.joyButState = 0;
    EVT.mx = EVT.my = 0;
    EVT.keyModifiers = 0;
    EVT.allowLEDS = true;

    /* Set default values for mouse double click and mouse auto events */
    EVT.doubleClick = 440;
    EVT.autoRepeat = 55;
    EVT.autoDelay = 330;
    EVT.autoTicks = 0;
    EVT.doubleClickThresh = 5;
    EVT.firstAuto = true;
    EVT.autoMouse_x = EVT.autoMouse_y = 0;
    memset(&EVT.downMouse,0,sizeof(EVT.downMouse));

    /* Setup default pointers for event library */
    EVT.userEventCallback = NULL;
    EVT.codePage = &_CP_US_English;

    /* Initialise the joystick module and do basic calibration (which assumes
     * the joystick is centered.
     */
    EVT.joyMask = EVT_joyIsPresent();
}

#if defined(NEED_SCALE_JOY_AXIS) || !defined(USE_OS_JOYSTICK)
/****************************************************************************
REMARKS:
This function scales a joystick axis value to normalised form.
****************************************************************************/
static int scaleJoyAxis(
    int raw,
    int axis)
{
    int scaled,range;

    /* Make sure the joystick is calibrated properly */
    if (EVT.joyCenter[axis] - EVT.joyMin[axis] < 5)
	return raw;
    if (EVT.joyMax[axis] - EVT.joyCenter[axis] < 5)
	return raw;

    /* Now scale the coordinates to -128 to 127 */
    raw -= EVT.joyCenter[axis];
    if (raw < 0)
	range = EVT.joyCenter[axis]-EVT.joyMin[axis];
    else
	range = EVT.joyMax[axis]-EVT.joyCenter[axis];
    scaled = (raw * 128) / range;
    if (scaled < -128)
	scaled = -128;
    if (scaled > 127)
	scaled = 127;
    return scaled;
}
#endif

#if     defined(__SMX32__)
#include "smx/event.c"
#elif   defined(__RTTARGET__)
#include "rttarget/event.c"
#elif   defined(__REALDOS__)
#include "dos/event.c"
#elif   defined(__WINDOWS32__)
#include "win32/event.c"
#elif   defined(__OS2__)
#if     defined(__OS2_PM__)
#include "os2pm/event.c"
#else
#include "os2/event.c"
#endif
#elif   defined(__LINUX__)
#if     defined(__USE_X11__)
#include "x11/event.c"
#else
#include "linux/event.c"
#endif
#elif   defined(__QNX__)
#if     defined(__USE_PHOTON__)
#include "photon/event.c"
#elif   defined(__USE_X11__)
#include "x11/event.c"
#else
#include "qnx/event.c"
#endif
#elif   defined(__BEOS__)
#include "beos/event.c"
#else
#error  Event library not ported to this platform yet!
#endif

/*------------------------ Public interface routines ----------------------*/

/* If USE_OS_JOYSTICK is defined, the OS specific libraries will implement
 * the joystick code rather than using the generic OS portable version.
 */

#ifndef USE_OS_JOYSTICK
/****************************************************************************
DESCRIPTION:
Returns the mask indicating what joystick axes are attached.

HEADER:
event.h

REMARKS:
This function is used to detect the attached joysticks, and determine
what axes are present and functioning. This function will re-detect any
attached joysticks when it is called, so if the user forgot to attach
the joystick when the application started, you can call this function to
re-detect any newly attached joysticks.

SEE ALSO:
EVT_joySetLowerRight, EVT_joySetCenter, EVT_joyIsPresent
****************************************************************************/
int EVTAPI EVT_joyIsPresent(void)
{
    int mask,i;

    memset(EVT.joyMin,0,sizeof(EVT.joyMin));
    memset(EVT.joyCenter,0,sizeof(EVT.joyCenter));
    memset(EVT.joyMax,0,sizeof(EVT.joyMax));
    memset(EVT.joyPrev,0,sizeof(EVT.joyPrev));
    EVT.joyButState = 0;
#ifdef __LINUX__
    PM_init();
#endif
    mask = _EVT_readJoyAxis(EVT_JOY_AXIS_ALL,EVT.joyCenter);
    if (mask) {
	for (i = 0; i < JOY_NUM_AXES; i++)
	    EVT.joyMax[i] = EVT.joyCenter[i]*2;
	}
    return mask;
}

/****************************************************************************
DESCRIPTION:
Polls the joystick for position and button information.

HEADER:
event.h

REMARKS:
This routine is used to poll analogue joysticks for button and position
information. It should be called once for each main loop of the user
application, just before processing all pending events via EVT_getNext.
All information polled from the joystick will be posted to the event
queue for later retrieval.

Note:   Most analogue joysticks will provide readings that change even
	though the joystick has not moved. Hence if you call this routine
	you will likely get an EVT_JOYMOVE event every time through your
	event loop.

SEE ALSO:
EVT_getNext, EVT_peekNext, EVT_joySetUpperLeft, EVT_joySetLowerRight,
EVT_joySetCenter, EVT_joyIsPresent
****************************************************************************/
void EVTAPI EVT_pollJoystick(void)
{
    event_t evt;
    int     i,axis[JOY_NUM_AXES],newButState,mask,moved,ps;

    if (EVT.joyMask) {
	/* Read joystick axes and post movement events if they have
	 * changed since the last time we polled. Until the events are
	 * actually flushed, we keep modifying the same joystick movement
	 * event, so you won't get multiple movement event
	 */
	mask = _EVT_readJoyAxis(EVT.joyMask,axis);
	newButState = _EVT_readJoyButtons();
	moved = false;
	for (i = 0; i < JOY_NUM_AXES; i++) {
	    if (mask & (EVT_JOY_AXIS_X1 << i))
		axis[i] = scaleJoyAxis(axis[i],i);
	    else
		axis[i] = EVT.joyPrev[i];
	    if (axis[i] != EVT.joyPrev[i])
		moved = true;
	    }
	if (moved) {
	    memcpy(EVT.joyPrev,axis,sizeof(EVT.joyPrev));
	    ps = _EVT_disableInt();
	    if (EVT.oldJoyMove != -1) {
		/* Modify the existing joystick movement event */
		EVT.evtq[EVT.oldJoyMove].message = newButState;
		EVT.evtq[EVT.oldJoyMove].where_x = EVT.joyPrev[0];
		EVT.evtq[EVT.oldJoyMove].where_y = EVT.joyPrev[1];
		EVT.evtq[EVT.oldJoyMove].relative_x = EVT.joyPrev[2];
		EVT.evtq[EVT.oldJoyMove].relative_y = EVT.joyPrev[3];
		}
	    else if (EVT.count < EVENTQSIZE) {
		/* Add a new joystick movement event */
		EVT.oldJoyMove = EVT.freeHead;
		memset(&evt,0,sizeof(evt));
		evt.what = EVT_JOYMOVE;
		evt.message = EVT.joyButState;
		evt.where_x = EVT.joyPrev[0];
		evt.where_y = EVT.joyPrev[1];
		evt.relative_x = EVT.joyPrev[2];
		evt.relative_y = EVT.joyPrev[3];
		addEvent(&evt);
		}
	    _EVT_restoreInt(ps);
	    }

	/* Read the joystick buttons, and post events to reflect the change
	 * in state for the joystick buttons.
	 */
	if (newButState != EVT.joyButState) {
	    if (EVT.count < EVENTQSIZE) {
		/* Add a new joystick click event */
		ps = _EVT_disableInt();
		memset(&evt,0,sizeof(evt));
		evt.what = EVT_JOYCLICK;
		evt.message = newButState;
		EVT.evtq[EVT.oldJoyMove].where_x = EVT.joyPrev[0];
		EVT.evtq[EVT.oldJoyMove].where_y = EVT.joyPrev[1];
		EVT.evtq[EVT.oldJoyMove].relative_x = EVT.joyPrev[2];
		EVT.evtq[EVT.oldJoyMove].relative_y = EVT.joyPrev[3];
		addEvent(&evt);
		_EVT_restoreInt(ps);
		}
	    EVT.joyButState = newButState;
	    }
	}
}

/****************************************************************************
DESCRIPTION:
Calibrates the joystick upper left position

HEADER:
event.h

REMARKS:
This function can be used to zero in on better joystick calibration factors,
which may work better than the default simplistic calibration (which assumes
the joystick is centered when the event library is initialised).
To use this function, ask the user to hold the stick in the upper left
position and then have them press a key or button. and then call this
function. This function will then read the joystick and update the
calibration factors.

Usually, assuming that the stick was centered when the event library was
initialized, you really only need to call EVT_joySetLowerRight since the
upper left position is usually always 0,0 on most joysticks. However, the
safest procedure is to call all three calibration functions.

SEE ALSO:
EVT_joySetUpperLeft, EVT_joySetLowerRight, EVT_joyIsPresent
****************************************************************************/
void EVTAPI EVT_joySetUpperLeft(void)
{
    _EVT_readJoyAxis(EVT_JOY_AXIS_ALL,EVT.joyMin);
}

/****************************************************************************
DESCRIPTION:
Calibrates the joystick lower right position

HEADER:
event.h

REMARKS:
This function can be used to zero in on better joystick calibration factors,
which may work better than the default simplistic calibration (which assumes
the joystick is centered when the event library is initialised).
To use this function, ask the user to hold the stick in the lower right
position and then have them press a key or button. and then call this
function. This function will then read the joystick and update the
calibration factors.

Usually, assuming that the stick was centered when the event library was
initialized, you really only need to call EVT_joySetLowerRight since the
upper left position is usually always 0,0 on most joysticks. However, the
safest procedure is to call all three calibration functions.

SEE ALSO:
EVT_joySetUpperLeft, EVT_joySetCenter, EVT_joyIsPresent
****************************************************************************/
void EVTAPI EVT_joySetLowerRight(void)
{
    _EVT_readJoyAxis(EVT_JOY_AXIS_ALL,EVT.joyMax);
}

/****************************************************************************
DESCRIPTION:
Calibrates the joystick center position

HEADER:
event.h

REMARKS:
This function can be used to zero in on better joystick calibration factors,
which may work better than the default simplistic calibration (which assumes
the joystick is centered when the event library is initialised).
To use this function, ask the user to hold the stick in the center
position and then have them press a key or button. and then call this
function. This function will then read the joystick and update the
calibration factors.

Usually, assuming that the stick was centered when the event library was
initialized, you really only need to call EVT_joySetLowerRight since the
upper left position is usually always 0,0 on most joysticks. However, the
safest procedure is to call all three calibration functions.

SEE ALSO:
EVT_joySetUpperLeft, EVT_joySetLowerRight, EVT_joySetCenter
****************************************************************************/
void EVTAPI EVT_joySetCenter(void)
{
    _EVT_readJoyAxis(EVT_JOY_AXIS_ALL,EVT.joyCenter);
}
#endif

/****************************************************************************
DESCRIPTION:
Posts a user defined event to the event queue

HEADER:
event.h

RETURNS:
True if event was posted, false if event queue is full.

PARAMETERS:
what        - Type code for message to post
message     - Event specific message to post
modifiers   - Event specific modifier flags to post

REMARKS:
This routine is used to post user defined events to the event queue.

SEE ALSO:
EVT_flush, EVT_getNext, EVT_peekNext, EVT_halt
****************************************************************************/
ibool EVTAPI EVT_post(
    ulong which,
    ulong what,
    ulong message,
    ulong modifiers)
{
    event_t evt;
    uint    ps;

    if (EVT.count < EVENTQSIZE) {
	/* Save information in event record */
	ps = _EVT_disableInt();
	evt.which = which;
	evt.when = _EVT_getTicks();
	evt.what = what;
	evt.message = message;
	evt.modifiers = modifiers;
	addEvent(&evt);             /* Add to EVT.tail of event queue   */
	_EVT_restoreInt(ps);
	return true;
	}
    else
	return false;
}

/****************************************************************************
DESCRIPTION:
Flushes all events of a specified type from the event queue.

PARAMETERS:
mask    - Mask specifying the types of events that should be removed

HEADER:
event.h

REMARKS:
Flushes (removes) all pending events of the specified type from the event
queue. You may combine the masks for different event types with a simple
logical OR.

SEE ALSO:
EVT_getNext, EVT_halt, EVT_peekNext
****************************************************************************/
void EVTAPI EVT_flush(
    ulong mask)
{
    event_t evt;

    do {                            /* Flush all events */
	EVT_getNext(&evt,mask);
	} while (evt.what != EVT_NULLEVT);
}

/****************************************************************************
DESCRIPTION:
Halts until and event of the specified type is recieved.

HEADER:
event.h

PARAMETERS:
evt     - Pointer to
mask    - Mask specifying the types of events that should be removed

REMARKS:
This functions halts exceution until an event of the specified type is
recieved into the event queue. It does not flush the event queue of events
before performing the busy loop. However this function does throw away
any events other than the ones you have requested via the event mask, to
avoid the event queue filling up with unwanted events (like EVT_KEYUP or
EVT_MOUSEMOVE events).

SEE ALSO:
EVT_getNext, EVT_flush, EVT_peekNext
****************************************************************************/
void EVTAPI EVT_halt(
    event_t *evt,
    ulong mask)
{
    do {                            /* Wait for an event    */
	if (mask & (EVT_JOYEVT))
	    EVT_pollJoystick();
	EVT_getNext(evt,EVT_EVERYEVT);
	} while (!(evt->what & mask));
}

/****************************************************************************
DESCRIPTION:
Peeks at the next pending event in the event queue.

HEADER:
event.h

RETURNS:
True if an event is pending, false if not.

PARAMETERS:
evt     - Pointer to structure to return the event info in
mask    - Mask specifying the types of events that should be removed

REMARKS:
Peeks at the next pending event of the specified type in the event queue. The
mask parameter is used to specify the type of events to be peeked at, and
can be any logical combination of any of the flags defined by the
EVT_eventType enumeration.

In contrast to EVT_getNext, the event is not removed from the event queue.
You may combine the masks for different event types with a simple logical OR.

SEE ALSO:
EVT_flush, EVT_getNext, EVT_halt
****************************************************************************/
ibool EVTAPI EVT_peekNext(
    event_t *evt,
    ulong mask)
{
    int     evtID;
    uint    ps;

    if (EVT.heartBeat)
	EVT.heartBeat(EVT.heartBeatParams);
    _EVT_pumpMessages();                /* Pump all messages into queue */
    EVT.mouseMove(EVT.mx,EVT.my);       /* Move the mouse cursor        */
    evt->what = EVT_NULLEVT;            /* Default to null event        */
    if (EVT.count) {
	/* It is possible that an event be posted while we are trying
	 * to access the event queue. This would create problems since
	 * we may end up with invalid data for our event queue pointers. To
	 * alleviate this, all interrupts are suspended while we manipulate
	 * our pointers.
	 */
	ps = _EVT_disableInt();             /* disable interrupts       */
	for (evtID = EVT.head; evtID != -1; evtID = EVT.evtq[evtID].next) {
	    if (EVT.evtq[evtID].what & mask)
		break;                      /* Found an event           */
	    }
	if (evtID == -1) {
	    _EVT_restoreInt(ps);
	    return false;                   /* Event was not found      */
	    }
	*evt = EVT.evtq[evtID];                 /* Return the event         */
	_EVT_restoreInt(ps);
	if (evt->what & EVT_KEYEVT)
	    _EVT_maskKeyCode(evt);
	}
    return evt->what != EVT_NULLEVT;
}

/****************************************************************************
DESCRIPTION:
Retrieves the next pending event from the event queue.

PARAMETERS:
evt     - Pointer to structure to return the event info in
mask    - Mask specifying the types of events that should be removed

HEADER:
event.h

RETURNS:
True if an event was pending, false if not.

REMARKS:
Retrieves the next pending event from the event queue, and stores it in a
event_t structure. The mask parameter is used to specify the type of events
to be removed, and can be any logical combination of any of the flags defined
by the EVT_eventType enumeration.

The what field of the event contains the event code of the event that was
extracted. All application specific events should begin with the EVT_USEREVT
code and build from there. Since the event code is stored in an integer,
there is a maximum of 32 different event codes that can be distinguished.
You can store extra information about the event in the message field to
distinguish between events of the same class (for instance the button used in
a EVT_MOUSEDOWN event).

If an event of the specified type was not in the event queue, the what field
of the event will be set to NULLEVT, and the return value will return false.

Note:   You should /always/ use the EVT_EVERYEVT mask for extracting events
	from your main event loop handler. Using a mask for only a specific
	type of event for long periods of time will cause the event queue to
	fill up with events of the type you are ignoring, eventually causing
	the application to hang when the event queue becomes full.

SEE ALSO:
EVT_flush, EVT_halt, EVT_peekNext
****************************************************************************/
ibool EVTAPI EVT_getNext(
    event_t *evt,
    ulong mask)
{
    int     evtID,next,prev;
    uint    ps;

    if (EVT.heartBeat)
	EVT.heartBeat(EVT.heartBeatParams);
    _EVT_pumpMessages();                /* Pump all messages into queue */
    EVT.mouseMove(EVT.mx,EVT.my);       /* Move the mouse cursor        */
    evt->what = EVT_NULLEVT;            /* Default to null event        */
    if (EVT.count) {
	/* It is possible that an event be posted while we are trying
	 * to access the event queue. This would create problems since
	 * we may end up with invalid data for our event queue pointers. To
	 * alleviate this, all interrupts are suspended while we manipulate
	 * our pointers.
	 */
	ps = _EVT_disableInt();             /* disable interrupts       */
	for (evtID = EVT.head; evtID != -1; evtID = EVT.evtq[evtID].next) {
	    if (EVT.evtq[evtID].what & mask)
		break;                      /* Found an event           */
	    }
	if (evtID == -1) {
	    _EVT_restoreInt(ps);
	    return false;                   /* Event was not found      */
	    }
	next = EVT.evtq[evtID].next;
	prev = EVT.evtq[evtID].prev;
	if (prev != -1)
	    EVT.evtq[prev].next = next;
	else
	    EVT.head = next;
	if (next != -1)
	    EVT.evtq[next].prev = prev;
	else
	    EVT.tail = prev;
	*evt = EVT.evtq[evtID];                 /* Return the event         */
	EVT.evtq[evtID].next = EVT.freeHead;        /* and return to free list  */
	EVT.freeHead = evtID;
	EVT.count--;
	if (evt->what == EVT_MOUSEMOVE)
	    EVT.oldMove = -1;
	if (evt->what == EVT_KEYREPEAT)
	    EVT.oldKey = -1;
	if (evt->what == EVT_JOYMOVE)
	    EVT.oldJoyMove = -1;
	_EVT_restoreInt(ps);                /* enable interrupts        */
	if (evt->what & EVT_KEYEVT)
	    _EVT_maskKeyCode(evt);
	}

    /* If there is no event pending, check if we should generate an auto
     * mouse down event if the mouse is still currently down.
     */
    if (evt->what == EVT_NULLEVT && EVT.autoRepeat && (mask & EVT_MOUSEAUTO) && (EVT.downMouse.what & EVT_MOUSEDOWN)) {
	ulong ticks = _EVT_getTicks();
	if ((ticks - EVT.autoTicks) >= (EVT.autoRepeat + (EVT.firstAuto ? EVT.autoDelay : 0))) {
	    evt->what = EVT_MOUSEAUTO;
	    evt->message = EVT.downMouse.message;
	    evt->modifiers = EVT.downMouse.modifiers;
	    evt->where_x = EVT.autoMouse_x;
	    evt->where_y = EVT.autoMouse_y;
	    evt->relative_x = 0;
	    evt->relative_y = 0;
	    EVT.autoTicks = evt->when = ticks;
	    EVT.firstAuto = false;
	    }
	}
    return evt->what != EVT_NULLEVT;
}

/****************************************************************************
DESCRIPTION:
Installs a user supplied event filter callback for event handling.

HEADER:
event.h

PARAMETERS:
userEventFilter - Address of user supplied event filter callback

REMARKS:
This function allows the application programmer to install an event filter
callback for event handling. Once you install your callback, the MGL
event handling routines will call your callback with a pointer to the
new event that will be placed into the event queue. Your callback can the
modify the contents of the event before it is placed into the queue (for
instance adding custom information or perhaps high precision timing
information).

If your callback returns FALSE, the event will be ignore and will not be
posted to the event queue. You should always return true from your event
callback unless you plan to use the events immediately that they are
recieved.

Note:   Your event callback may be called in response to a hardware
	interrupt and will be executing in the context of the hardware
	interrupt handler under MSDOS (ie: keyboard interrupt or mouse
	interrupt). For this reason the code pages for the callback that
	you register must be locked in memory with the PM_lockCodePages
	function. You must also lock down any data pages that your function
	needs to reference as well.

Note:   You can also use this filter callback to process events at the
	time they are activated by the user (ie: when the user hits the
	key or moves the mouse), but make sure your code runs as fast as
	possible as it will be executing inside the context of an interrupt
	handler on some systems.

SEE ALSO:
EVT_getNext, EVT_peekNext
****************************************************************************/
void EVTAPI EVT_setUserEventFilter(
    _EVT_userEventFilter filter)
{
    EVT.userEventCallback = filter;
}

/****************************************************************************
DESCRIPTION:
Installs a user supplied event heartbeat callback function.

HEADER:
event.h

PARAMETERS:
callback    - Address of user supplied event heartbeat callback
params      - Parameters to pass to the event heartbeat function

REMARKS:
This function allows the application programmer to install an event heatbeat
function that gets called every time that EVT_getNext or EVT_peekNext
is called. This is primarily useful for simulating text mode cursors inside
event handling code when running in graphics modes as opposed to hardware
text modes.

SEE ALSO:
EVT_getNext, EVT_peekNext, EVT_getHeartBeatCallback
****************************************************************************/
void EVTAPI EVT_setHeartBeatCallback(
    _EVT_heartBeatCallback callback,
    void *params)
{
    EVT.heartBeat = callback;
    EVT.heartBeatParams = params;
}


/****************************************************************************
DESCRIPTION:
Returns the current user supplied event heartbeat callback function.

HEADER:
event.h

PARAMETERS:
callback    - Place to store the address of user supplied event heartbeat callback
params      - Place to store the parameters to pass to the event heartbeat function

REMARKS:
This function retrieves the current event heatbeat function that gets called
every time that EVT_getNext or EVT_peekNext is called.

SEE ALSO:
EVT_getNext, EVT_peekNext, EVT_setHeartBeatCallback
****************************************************************************/
void EVTAPI EVT_getHeartBeatCallback(
    _EVT_heartBeatCallback *callback,
    void **params)
{
    *callback = EVT.heartBeat;
    *params = EVT.heartBeatParams;
}

/****************************************************************************
DESCRIPTION:
Determines if a specified key is currently down.

PARAMETERS:
scanCode    - Scan code to test

RETURNS:
True of the specified key is currently held down.

HEADER:
event.h

REMARKS:
This function determines if a specified key is currently down at the
time that the call is made. You simply need to pass in the scan code of
the key that you wish to test, and the MGL will tell you if it is currently
down or not. The MGL does this by keeping track of the up and down state
of all the keys.
****************************************************************************/
ibool EVTAPI EVT_isKeyDown(
    uchar scanCode)
{
    return _EVT_isKeyDown(scanCode);
}

/****************************************************************************
DESCRIPTION:
Set the mouse position for the event module

PARAMETERS:
x   - X coordinate to move the mouse cursor position to
y   - Y coordinate to move the mouse cursor position to

HEADER:
event.h

REMARKS:
This function moves the mouse cursor position for the event module to the
specified location.

SEE ALSO:
EVT_getMousePos
****************************************************************************/
void EVTAPI EVT_setMousePos(
    int x,
    int y)
{
    EVT.mx = x;
    EVT.my = y;
    _EVT_setMousePos(&EVT.mx,&EVT.my);
    EVT.mouseMove(EVT.mx,EVT.my);
}

/****************************************************************************
DESCRIPTION:
Returns the current mouse cursor location.

HEADER:
event.h

PARAMETERS:
x   - Place to store value for mouse x coordinate (screen coordinates)
y   - Place to store value for mouse y coordinate (screen coordinates)

REMARKS:
Obtains the current mouse cursor position in screen coordinates. Normally the
mouse cursor location is tracked using the mouse movement events that are
posted to the event queue when the mouse moves, however this routine
provides an alternative method of polling the mouse cursor location.

SEE ALSO:
EVT_setMousePos
****************************************************************************/
void EVTAPI EVT_getMousePos(
    int *x,
    int *y)
{
    *x = EVT.mx;
    *y = EVT.my;
}

/****************************************************************************
DESCRIPTION:
Returns the currently active code page for translation of keyboard characters.

HEADER:
event.h

RETURNS:
Pointer to the currently active code page translation table.

REMARKS:
This function is returns a pointer to the currently active code page
translation table. See EVT_setCodePage for more information.

SEE ALSO:
EVT_setCodePage
****************************************************************************/
codepage_t * EVTAPI EVT_getCodePage(void)
{
    return EVT.codePage;
}

/****************************************************************************
DESCRIPTION:
Sets the currently active code page for translation of keyboard characters.

HEADER:
event.h

PARAMETERS:
page    - New code page to make active

REMARKS:
This function is used to set a new code page translation table that is used
to translate virtual scan code values to ASCII characters for different
keyboard configurations. The default is usually US English, although if
possible the PM library will auto-detect the correct code page translation
for the target OS if OS services are available to determine what type of
keyboard is currently attached.

SEE ALSO:
EVT_getCodePage
****************************************************************************/
void EVTAPI EVT_setCodePage(
    codepage_t *page)
{
    EVT.codePage = page;
}

/* The following contains fake C prototypes and documentation for the
 * macro functions in the event.h header file. These exist soley so
 * that DocJet will correctly pull in the documentation for these functions.
 */
#ifdef  INCLUDE_DOC_FUNCTIONS

/****************************************************************************
DESCRIPTION:
Macro to extract the ASCII code from a message.

PARAMETERS:
message - Message to extract ASCII code from

RETURNS:
ASCII code extracted from the message.

HEADER:
event.h

REMARKS:
Macro to extract the ASCII code from the message field of the event_t
structure. You pass the message field to the macro as the parameter and
the ASCII code is the result, for example:

    event_t EVT.myEvent;
    uchar   code;
    code = EVT_asciiCode(EVT.myEvent.message);

SEE ALSO:
EVT_scanCode, EVT_repeatCount
****************************************************************************/
uchar EVT_asciiCode(
    ulong message);

/****************************************************************************
DESCRIPTION:
Macro to extract the keyboard scan code from a message.

HEADER:
event.h

PARAMETERS:
message - Message to extract scan code from

RETURNS:
Keyboard scan code extracted from the message.

REMARKS:
Macro to extract the keyboard scan code from the message field of the event
structure. You pass the message field to the macro as the parameter and
the scan code is the result, for example:

    event_t EVT.myEvent;
    uchar   code;
    code = EVT_scanCode(EVT.myEvent.message);

NOTE:   Scan codes in the event library are not really hardware scan codes,
	but rather virtual scan codes as generated by a low level keyboard
	interface driver. All virtual scan code values are defined by the
	EVT_scanCodesType enumeration, and will be identical across all
	supports OS'es and platforms.

SEE ALSO:
EVT_asciiCode, EVT_repeatCount
****************************************************************************/
uchar EVT_scanCode(
    ulong message);

/****************************************************************************
DESCRIPTION:
Macro to extract the repeat count from a message.

HEADER:
event.h

PARAMETERS:
message - Message to extract repeat count from

RETURNS:
Repeat count extracted from the message.

REMARKS:
Macro to extract the repeat count from the message field of the event
structure. The repeat count is the number of times that the key repeated
before there was another keyboard event to be place in the queue, and
allows the event handling code to avoid keyboard buffer overflow
conditions when a single key is held down by the user. If you are processing
a key repeat code, you will probably want to check this field to see how
many key repeats you should process for this message.

SEE ALSO:
EVT_asciiCode, EVT_repeatCount
****************************************************************************/
short EVT_repeatCount(
    ulong message);

#endif  /* DOC FUNCTIONS */

#if defined(__REALDOS__) || defined(__SMX32__)
/* {secret} */
void EVTAPI _EVT_cCodeEnd(void) {}
#endif
