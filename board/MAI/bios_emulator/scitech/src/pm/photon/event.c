/****************************************************************************
*
*                   SciTech Multi-platform Graphics Library
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
* Environment:  QNX Photon GUI
*
* Description:  QNX fullscreen console implementation for the SciTech
*               cross platform event library.
*
****************************************************************************/

/*--------------------------- Global variables ----------------------------*/

static ushort       keyUpMsg[256] = {0};/* Table of key up messages     */

/*---------------------------- Implementation -----------------------------*/

/* These are not used under Linux */
#define _EVT_disableInt()       1
#define _EVT_restoreInt(flags)

/****************************************************************************
PARAMETERS:
scanCode    - Scan code to test

REMARKS:
This macro determines if a specified key is currently down at the
time that the call is made.
****************************************************************************/
static ibool _EVT_isKeyDown(
    uchar scancode)
{
    return (KeyState[(scancode & 0xf8) >> 3] & (1 << (scancode & 0x7)) ?
	true : false);
}

/****************************************************************************
REMARKS:
Retrieves all events from the mouse/keyboard event queue and stuffs them
into the MGL event queue for further processing.
****************************************************************************/
static void _EVT_pumpMessages(void)
{
    int         pid;
    uint            msg, but_stat, message;
    uchar           evt[sizeof (PhEvent_t) + 1024];
    PhEvent_t       *event = (void *)evt;
    PhKeyEvent_t        *key;
    PhPointerEvent_t    *mouse;
    static int      extended;
    event_t         _evt;

    while (count < EVENTQSIZE) {
	uint    mods = 0, keyp = 0;

	pid = Creceive(0, &msg, sizeof (msg));

	if (pid == -1)
	    return;

	if (PhEventRead(pid, event, sizeof (evt)) == Ph_EVENT_MSG) {
	    memset(&evt, 0, sizeof (evt));
	    if (event->type == Ph_EV_KEY) {
		key = PhGetData(event);

		if (key->key_flags & KEY_SCAN_VALID) {
		    keyp = key->key_scan;
		    if (key->key_flags & KEY_DOWN)
			KeyState[(keyp & 0xf800) >> 11]
			    |= 1 << ((keyp & 0x700) >> 8);
		    else
			KeyState[(keyp & 0xf800) >> 11]
			    &= ~(1 << ((keyp & 0x700) >> 8));
		}
		if ((key->key_flags & KEY_SYM_VALID) || extended)
		    keyp |= key->key_sym;

		/* No way to tell left from right... */
		if (key->key_mods & KEYMOD_SHIFT)
		    mods = (EVT_LEFTSHIFT | EVT_RIGHTSHIFT);
		if (key->key_mods & KEYMOD_CTRL)
		    mods |= (EVT_CTRLSTATE | EVT_LEFTCTRL);
		if (key->key_mods & KEYMOD_ALT)
		    mods |= (EVT_ALTSTATE | EVT_LEFTALT);

		_evt.when = evt->timestamp;
		if (key->key_flags & KEY_REPEAT) {
		    _evt.what = EVT_KEYREPEAT;
		    _evt.message = 0x10000;
		    }
		else if (key->key_flags & KEY_DOWN)
		    _evt.what = EVT_KEYDOWN;
		else
		    _evt.what = EVT_KEYUP;
		_evt.modifiers = mods;
		_evt.message |= keyp;

		addEvent(&_evt);

		switch(key->key_scan & 0xff00) {
		    case 0xe000:
			extended = 1;
			break;
		    case 0xe001:
			extended = 2;
			break;
		    default:
			if (extended)
			    extended--;
		    }
		}
	    else if (event->type & Ph_EV_PTR_ALL) {
		but_stat = message = 0;
		mouse = PhGetData(event);

		if (mouse->button_state & Ph_BUTTON_3)
		    but_stat = EVT_LEFTBUT;
		if (mouse->buttons & Ph_BUTTON_3)
		    message = EVT_LEFTBMASK;

		if (mouse->button_state & Ph_BUTTON_1)
		    but_stat |= EVT_RIGHTBUT;
		if (mouse->buttons & Ph_BUTTON_1)
		    message |= EVT_RIGHTBMASK;

		_evt.when = evt->timestamp;
		if (event->type & Ph_EV_PTR_MOTION) {
		    _evt.what = EVT_MOUSEMOVE;
		    _evt.where_x = mouse->pos.x;
		    _evt.where_y = mouse->pos.y;
		    _evt.modifiers = but_stat;
		    addEvent(&_evt);
		    }
		if (event->type & Ph_EV_BUT_PRESS)
		    _evt.what = EVT_MOUSEDOWN;
		else
		    _evt.what = EVT_MOUSEUP;
		_evt.where_x = mouse->pos.x;
		_evt.where_y = mouse->pos.y;
		_evt.modifiers = but_stat;
		_evt.message = message;
		addEvent(&_evt);
		}
	    }
	else
	    return;
	}
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
void _EVT_abort(
    int signo)
{
    char    buf[80];

    EVT_exit();
    sprintf(buf,"Terminating on signal %d",signo);
    PM_fatalError(buf);
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
    int         i;

    /* Initialise the event queue */
    _mouseMove = mouseMove;
    initEventQueue();
    memset((void *)KeyState, 0, sizeof (KeyState));

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
    /* TODO: Need to call Input to change the coordinates that it returns */
    /*       for mouse events!! */
}

/****************************************************************************
REMARKS:
Initiailises the internal event handling modules. The EVT_suspend function
can be called to suspend event handling (such as when shelling out to DOS),
and this function can be used to resume it again later.
****************************************************************************/
void EVT_resume(void)
{
    /* Do nothing for Photon */
}

/****************************************************************************
REMARKS
Suspends all of our event handling operations. This is also used to
de-install the event handling code.
****************************************************************************/
void EVT_suspend(void)
{
    /* Do nothing for Photon */
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
}
