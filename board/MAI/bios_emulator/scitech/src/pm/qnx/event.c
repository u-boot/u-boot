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
* Environment:  QNX
*
* Description:  QNX fullscreen console implementation for the SciTech
*               cross platform event library.
*
****************************************************************************/

#include <errno.h>
#include <unistd.h>

/*--------------------------- Global variables ----------------------------*/

#ifndef __QNXNTO__
static struct _mouse_ctrl   *_PM_mouse_ctl;
static int          _PM_keyboard_fd = -1;
/*static int            _PM_modifiers, _PM_leds; */
#else
static int          kbd_fd = -1, mouse_fd = -1;
#endif
static int          kill_pid = 0;
static ushort       keyUpMsg[256] = {0};/* Table of key up messages     */
static int          rangeX,rangeY;      /* Range of mouse coordinates   */

#define TIME_TO_MSEC(__t)   ((__t).tv_nsec / 1000000 + (__t).tv_sec * 1000)

#define LED_NUM         1
#define LED_CAP         2
#define LED_SCR         4

/* Scancode mappings on QNX for special keys */

typedef struct {
    int scan;
    int map;
    } keymap;

/* TODO: Fix this and set it up so we can do a binary search! */

keymap keymaps[] = {
    {96, KB_padEnter},
    {74, KB_padMinus},
    {78, KB_padPlus},
    {55, KB_padTimes},
    {98, KB_padDivide},
    {71, KB_padHome},
    {72, KB_padUp},
    {73, KB_padPageUp},
    {75, KB_padLeft},
    {76, KB_padCenter},
    {77, KB_padRight},
    {79, KB_padEnd},
    {80, KB_padDown},
    {81, KB_padPageDown},
    {82, KB_padInsert},
    {83, KB_padDelete},
    {105,KB_left},
    {108,KB_down},
    {106,KB_right},
    {103,KB_up},
    {110,KB_insert},
    {102,KB_home},
    {104,KB_pageUp},
    {111,KB_delete},
    {107,KB_end},
    {109,KB_pageDown},
    {125,KB_leftWindows},
    {126,KB_rightWindows},
    {127,KB_menu},
    {100,KB_rightAlt},
    {97,KB_rightCtrl},
    };

/* And the keypad with num lock turned on (changes the ASCII code only) */

keymap keypad[] = {
    {71, ASCII_7},
    {72, ASCII_8},
    {73, ASCII_9},
    {75, ASCII_4},
    {76, ASCII_5},
    {77, ASCII_6},
    {79, ASCII_1},
    {80, ASCII_2},
    {81, ASCII_3},
    {82, ASCII_0},
    {83, ASCII_period},
    };

#define NB_KEYMAPS (sizeof(keymaps)/sizeof(keymaps[0]))
#define NB_KEYPAD (sizeof(keypad)/sizeof(keypad[0]))

/*---------------------------- Implementation -----------------------------*/

/****************************************************************************
REMARKS:
Include generic raw scancode keyboard module.
****************************************************************************/
#include "common/keyboard.c"

/* These are not used under QNX */
#define _EVT_disableInt()       1
#define _EVT_restoreInt(flags)

/****************************************************************************
REMARKS:
This function is used to return the number of ticks since system
startup in milliseconds. This should be the same value that is placed into
the time stamp fields of events, and is used to implement auto mouse down
events.
****************************************************************************/
ulong _EVT_getTicks(void)
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME,&t);
    return (t.tv_nsec / 1000000 + t.tv_sec * 1000);
}

/****************************************************************************
REMARKS:
Converts a mickey movement value to a pixel adjustment value.
****************************************************************************/
static int MickeyToPixel(
    int mickey)
{
    /* TODO: We can add some code in here to handle 'acceleration' for */
    /*       the mouse cursor. For now just use the mickeys. */
    return mickey;
}

#ifdef __QNXNTO__
/****************************************************************************
REMARKS:
Retrieves all events from the mouse/keyboard event queue and stuffs them
into the MGL event queue for further processing.
****************************************************************************/
static void _EVT_pumpMessages(void)
{
    int         rc1, rc2;
    struct _keyboard_packet key;
    struct _mouse_packet    ms;
    static long     old_buttons = 0;
    uint            message = 0, but_stat = 0, mods = 0;
    event_t         evt;

    while (EVT.count < EVENTQSIZE) {
	rc1 = read(kbd_fd, (void *)&key, sizeof(key));
	if (rc1 == -1) {
	    if (errno == EAGAIN)
		rc1 = 0;
	    else {
		perror("getEvents");
		PM_fatalError("Keyboard error");
		}
	    }
	if (rc1 > 0) {
	    memset(&evt, 0, sizeof(evt));
	    if (key.data.modifiers & KEYMOD_SHIFT)
		mods |= EVT_LEFTSHIFT;
	    if (key.data.modifiers & KEYMOD_CTRL)
		mods |= EVT_CTRLSTATE;
	    if (key.data.modifiers & KEYMOD_ALT)
		mods |= EVT_ALTSTATE;

	    /* Now store the keyboard event data */
	    evt.when = TIME_TO_MSEC(key.time);
	    if (key.data.flags & KEY_SCAN_VALID)
		evt.message |= (key.data.key_scan & 0x7F) << 8;
	    if ((key.data.flags & KEY_SYM_VALID) &&
		(((key.data.key_sym & 0xff00) == 0xf000 &&
		(key.data.key_sym & 0xff) < 0x20) ||
		key.data.key_sym < 0x80))
		evt.message |= (key.data.key_sym & 0xFF);
	    evt.modifiers = mods;
	    if (key.data.flags & KEY_DOWN) {
		evt.what = EVT_KEYDOWN;
		keyUpMsg[evt.message >> 8] = (ushort)evt.message;
		}
	    else if (key.data.flags & KEY_REPEAT) {
		evt.message |= 0x10000;
		evt.what = EVT_KEYREPEAT;
		}
	    else {
		evt.what = EVT_KEYUP;
		evt.message = keyUpMsg[evt.message >> 8];
		if (evt.message == 0)
		    continue;
		keyUpMsg[evt.message >> 8] = 0;
		}

	    /* Now add the new event to the event queue */
	    addEvent(&evt);
	    }
	rc2 = read(mouse_fd, (void *)&ms, sizeof (ms));
	if (rc2 == -1) {
	    if (errno == EAGAIN)
		rc2 = 0;
	    else {
		perror("getEvents");
		PM_fatalError("Mouse error");
		}
	    }
	if (rc2 > 0) {
	    memset(&evt, 0, sizeof(evt));
	    ms.hdr.buttons &=
		(_POINTER_BUTTON_LEFT | _POINTER_BUTTON_RIGHT);
	    if (ms.hdr.buttons & _POINTER_BUTTON_LEFT)
		but_stat = EVT_LEFTBUT;
	    if ((ms.hdr.buttons & _POINTER_BUTTON_LEFT) !=
		(old_buttons & _POINTER_BUTTON_LEFT))
		message = EVT_LEFTBMASK;
	    if (ms.hdr.buttons & _POINTER_BUTTON_RIGHT)
		but_stat |= EVT_RIGHTBUT;
	    if ((ms.hdr.buttons & _POINTER_BUTTON_RIGHT) !=
		(old_buttons & _POINTER_BUTTON_RIGHT))
		message |= EVT_RIGHTBMASK;
	    if (ms.dx || ms.dy) {
		ms.dy = -ms.dy;
		EVT.mx += MickeyToPixel(ms.dx);
		EVT.my += MickeyToPixel(ms.dy);
		if (EVT.mx < 0) EVT.mx = 0;
		if (EVT.my < 0) EVT.my = 0;
		if (EVT.mx > rangeX)    EVT.mx = rangeX;
		if (EVT.my > rangeY)    EVT.my = rangeY;
		evt.what = EVT_MOUSEMOVE;
		evt.when = TIME_TO_MSEC(ms.hdr.time);
		evt.where_x = EVT.mx;
		evt.where_y = EVT.my;
		evt.relative_x = ms.dx;
		evt.relative_y = ms.dy;
		evt.modifiers = but_stat;
		addEvent(&evt);
		}
	    evt.what = ms.hdr.buttons < old_buttons ?
		EVT_MOUSEUP : EVT_MOUSEDOWN;
	    evt.when = TIME_TO_MSEC(ms.hdr.time);
	    evt.where_x = EVT.mx;
	    evt.where_y = EVT.my;
	    evt.relative_x = ms.dx;
	    evt.relative_y = ms.dy;
	    evt.modifiers = but_stat;
	    evt.message = message;
	    if (ms.hdr.buttons != old_buttons) {
		addEvent(&evt);
		old_buttons = ms.hdr.buttons;
		}
	    }
	if (rc1 + rc2 == 0)
	    break;
	}
}
#else
/****************************************************************************
REMARKS:
Retrieves all events from the mouse/keyboard event queue and stuffs them
into the MGL event queue for further processing.
****************************************************************************/
static void _EVT_pumpMessages(void)
{
    struct mouse_event      ev;
    int             rc;
    static long         old_buttons = 0;
    uint                message = 0, but_stat = 0;
    event_t             evt;
    char                buf[32];
    int             numkeys, i;

    /* Poll keyboard events */
    while ((numkeys = read(_PM_keyboard_fd, buf, sizeof buf)) > 0) {
	for (i = 0; i < numkeys; i++) {
	    processRawScanCode(buf[i]);
	    }
	}

    if (_PM_mouse_ctl == NULL)
	return;

    /* Gobble pending mouse events */
    while (EVT.count < EVENTQSIZE) {
	rc = mouse_read(_PM_mouse_ctl, &ev, 1, 0, NULL);
	if (rc == -1) {
	    perror("getEvents");
	    PM_fatalError("Mouse error (Input terminated?)");
	    }
	if (rc == 0)
	    break;

	message = 0, but_stat = 0;
	memset(&evt, 0, sizeof(evt));

	ev.buttons &= (_MOUSE_LEFT | _MOUSE_RIGHT);
	if (ev.buttons & _MOUSE_LEFT)
	    but_stat = EVT_LEFTBUT;
	if ((ev.buttons & _MOUSE_LEFT) != (old_buttons & _MOUSE_LEFT))
	    message = EVT_LEFTBMASK;
	if (ev.buttons & _MOUSE_RIGHT)
	    but_stat |= EVT_RIGHTBUT;
	if ((ev.buttons & _MOUSE_RIGHT) != (old_buttons & _MOUSE_RIGHT))
	    message |= EVT_RIGHTBMASK;
	if (ev.dx || ev.dy) {
	    ev.dy = -ev.dy;
	    EVT.mx += MickeyToPixel(ev.dx);
	    EVT.my += MickeyToPixel(ev.dy);
	    if (EVT.mx < 0) EVT.mx = 0;
	    if (EVT.my < 0) EVT.my = 0;
	    if (EVT.mx > rangeX)    EVT.mx = rangeX;
	    if (EVT.my > rangeY)    EVT.my = rangeY;
	    evt.what = EVT_MOUSEMOVE;
	    evt.when = ev.timestamp*100;
	    evt.where_x = EVT.mx;
	    evt.where_y = EVT.my;
	    evt.relative_x = ev.dx;
	    evt.relative_y = ev.dy;
	    evt.modifiers = but_stat;
	    addEvent(&evt);
	    }
	evt.what = ev.buttons < old_buttons ? EVT_MOUSEUP : EVT_MOUSEDOWN;
	evt.when = ev.timestamp*100;
	evt.where_x = EVT.mx;
	evt.where_y = EVT.my;
	evt.relative_x = ev.dx;
	evt.relative_y = ev.dy;
	evt.modifiers = but_stat;
	evt.message = message;
	if (ev.buttons != old_buttons) {
	    addEvent(&evt);
	    old_buttons = ev.buttons;
	    }
	}
}
#endif  /* __QNXNTO__ */

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
    struct stat st;
    char        *iarg[16];
#ifdef __QNXNTO__
    char        buf[128];
    FILE        *p;
    int         argno,len;
#endif

#ifdef __QNXNTO__
    ThreadCtl(_NTO_TCTL_IO, 0); /* So joystick code won't blow up */
#endif

    /* Initialise the event queue */
    EVT.mouseMove = mouseMove;
    initEventQueue();
    memset(keyUpMsg,0,sizeof(keyUpMsg));

#ifdef __QNXNTO__
    /*
     * User may already have input running with the right parameters.
     * Thus they could start input at boot time, using the output of
     * inputtrap, passing the the -r flag to make it run as a resource
     * manager.
     */
    if ((mouse_fd = open("/dev/mouse0", O_RDONLY | O_NONBLOCK)) < 0) {
	/* Run inputtrap to get the args for input */
	if ((p = popen("inputtrap", "r")) == NULL)
	    PM_fatalError("Error running 'inputtrap'");
	fgets(buf, sizeof(buf), p);
	pclose(p);

	/* Build the argument list */
	len = strlen(buf);
	iarg[0] = buf;
	for (i = 0, argno = 0; i < len && argno < 15;) {
	    if (argno == 1) {
		/*
		 * Add flags to input's arg list.
		 * '-r' means run as resource
		 * manager, providing the /dev/mouse
		 * and /dev/keyboard interfaces.
		 * '-P' supresses the /dev/photon
		 * mechanism.
		 */
		iarg[argno++] = "-Pr";
		continue;
		}
	    while (buf[i] == ' ')
		i++;
	    if (buf[i] == '\0' || buf[i] == '\n')
		break;
	    iarg[argno++] = &buf[i];
	    while (buf[i] != ' '
		&& buf[i] != '\0' && buf[i] != '\n')
		i++;
	    buf[i++] = '\0';
	    }
	iarg[argno] = NULL;

	if ((kill_pid = spawnvp(P_NOWAITO, iarg[0], iarg)) == -1) {
	    perror("spawning input resmgr");
	    PM_fatalError("Could not start input resmgr");
	    }
	for (i = 0; i < 10; i++) {
	    if (stat("/dev/mouse0", &st) == 0)
		break;
	    sleep(1);
	    }
	if ((mouse_fd = open("/dev/mouse0", O_RDONLY|O_NONBLOCK)) < 0) {
	    perror("/dev/mouse0");
	    PM_fatalError("Could not open /dev/mouse0");
	    }
	}
    if ((kbd_fd = open("/dev/keyboard0", O_RDONLY|O_NONBLOCK)) < 0) {
	perror("/dev/keyboard0");
	PM_fatalError("Could not open /dev/keyboard0");
	}
#else
    /* Connect to Input/Mouse for event handling */
    if (_PM_mouse_ctl == NULL) {
	_PM_mouse_ctl = mouse_open(0, "/dev/mouse", 0);

	/* "Mouse" is not running; attempt to start it */
	if (_PM_mouse_ctl == NULL) {
	    iarg[0] = "mousetrap";
	    iarg[1] = "start";
	    iarg[2] = NULL;
	    if ((kill_pid = spawnvp(P_NOWAITO, iarg[0], (void*)iarg)) == -1)
		perror("spawn (mousetrap)");
	    else {
		for (i = 0; i < 10; i++) {
		    if (stat("/dev/mouse", &st) == 0)
			break;
		    sleep(1);
		    }
		_PM_mouse_ctl = mouse_open(0, "/dev/mouse", 0);
		}
	    }
	}
    if (_PM_keyboard_fd == -1)
	_PM_keyboard_fd = open("/dev/kbd", O_RDONLY|O_NONBLOCK);
#endif

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
REMARKS
Modifes the mouse coordinates as necessary if scaling to OS coordinates,
and sets the OS mouse cursor position.
****************************************************************************/
#define _EVT_setMousePos(x,y)

/****************************************************************************
REMARKS:
Initiailises the internal event handling modules. The EVT_suspend function
can be called to suspend event handling (such as when shelling out to DOS),
and this function can be used to resume it again later.
****************************************************************************/
void EVT_resume(void)
{
    /* Do nothing for QNX */
}

/****************************************************************************
REMARKS
Suspends all of our event handling operations. This is also used to
de-install the event handling code.
****************************************************************************/
void EVT_suspend(void)
{
    /* Do nothing for QNX */
}

/****************************************************************************
REMARKS
Exits the event module for program terminatation.
****************************************************************************/
void EVT_exit(void)
{
#ifdef __QNXNTO__
    char    c;
    int flags;

    if (kbd_fd != -1) {
	close(kbd_fd);
	kbd_fd = -1;
	}
    if (mouse_fd != -1) {
	close(mouse_fd);
	mouse_fd = -1;
	}
#endif

    /* Restore signal handlers */
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGINT, SIG_DFL);

#ifndef __QNXNTO__
    /* Kill the Input/Mouse driver if we have spawned it */
    if (_PM_mouse_ctl != NULL) {
	struct _fd_entry    fde;
	uint            pid = 0;

	/* Find out the pid of the mouse driver */
	if (kill_pid > 0) {
	    if (qnx_fd_query(0,
		0, _PM_mouse_ctl->fd, &fde) != -1)
		pid = fde.pid;
	    }
	mouse_close(_PM_mouse_ctl);
	_PM_mouse_ctl = NULL;

	if (pid > 0) {
	    /* For some reasons the PID's are different under QNX4,
	     * so we use the old mechanism to kill the mouse server.
	     */
	    kill(pid, SIGTERM);
	    kill_pid = 0;
	    }
	}
#endif
    if (kill_pid > 0) {
	kill(kill_pid, SIGTERM);
	kill_pid = 0;
	}
}
