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
* Environment:  Unix / X11
*
* Description:  X11 event queue implementation for the MGL.
*               This can be used both for windowed and fullscreen (DGA) modes.
*
****************************************************************************/

/*---------------------------- Global Variables ---------------------------*/

static ushort       keyUpMsg[256] = {0};/* Table of key up messages     */
static int          rangeX,rangeY;      /* Range of mouse coordinates   */

static Display  *_EVT_dpy;
static Window    _EVT_win;

typedef struct {
  int keycode;
  int scancode;
} xkeymap;

xkeymap xkeymaps[] = {
  { 9, KB_esc},
  {24, KB_Q},
  {25, KB_W},
  {26, KB_E},
  {27, KB_R},
  {28, KB_T},
  {29, KB_Y},
  {30, KB_U},
  {31, KB_I},
  {32, KB_O},
  {33, KB_P},
};

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
  static unsigned starttime = 0;
  struct timeval t;

  gettimeofday(&t, NULL);
  if (starttime == 0)
    starttime = t.tv_sec * 1000 + (t.tv_usec/1000);
  return ((t.tv_sec * 1000 + (t.tv_usec/1000)) - starttime);
}

static int getScancode(int keycode)
{
  return keycode-8;
}

/****************************************************************************
REMARKS:
Pumps all messages in the application message queue into our event queue.
****************************************************************************/
#ifdef X11_CORE
static void _EVT_pumpX11Messages(void)
#else
static void _EVT_pumpMessages(void)
#endif
{
  /* TODO: The purpose of this function is to read all keyboard and mouse */
  /*         events from the OS specific event queue, translate them and post */
  /*         them into the SciTech event queue. */
  event_t evt;
  XEvent  ev;
  static int old_mx = 0, old_my = 0, buts = 0, c;
  char buf[2];

  while (XPending(_EVT_dpy) && XNextEvent(_EVT_dpy,&ev)) {
    evt.when = _MGL_getTicks();

    switch(ev.type){
    case KeyPress:
      c = getScancode(ev.xkey.keycode);
      evt.what = EVT_KEYDOWN;
      evt.message = c << 8;
      XLookupString(&ev.xkey, buf, 2, NULL, NULL);
      evt.message |= buf[0];
      break;
    case KeyRelease:
      c = getScancode(ev.xkey.keycode);
      evt.what = EVT_KEYUP;
      evt.message = keyUpMsg[c];
      if(count < EVENTQSIZE)
	addEvent(&evt);
      keyUpMsg[c] = 0;
      repeatKey[c] = 0;
      break;
    case ButtonPress:
      evt.what = EVT_MOUSEDOWN;
      if(ev.xbutton.button == 1){
	buts |= EVT_LEFTBUT;
	evt.message = EVT_LEFTBMASK;
      }else if(ev.xbutton.button == 2){
	buts |= EVT_MIDDLEBUT;
	evt.message = EVT_MIDDLEBMASK;
      }else if(ev.xbutton.button == 3){
	buts |= EVT_RIGHTBUT;
	evt.message = EVT_RIGHTBMASK;
      }
      evt.modifiers = modifiers | buts;

      break;
    case ButtonRelease:
      evt.what = EVT_MOUSEUP;
      if(ev.xbutton.button == 1){
	buts &= ~EVT_LEFTBUT;
	evt.message = EVT_LEFTBMASK;
      }else if(ev.xbutton.button == 2){
	buts &= ~EVT_MIDDLEBUT;
	evt.message = EVT_MIDDLEBMASK;
      }else if(ev.xbutton.button == 3){
	buts &= ~EVT_RIGHTBUT;
	evt.message = EVT_RIGHTBMASK;
      }
      evt.modifiers = modifiers | buts;

      break;
    case MotionNotify:
      evt.what = EVT_MOUSEMOVE;
      evt.where_x = ev.xmotion.x;
      evt.where_y = ev.xmotion.y;
      evt.relative_x = evt.where_x - old_mx;
      evt.relative_y = evt.where_y - old_my;
      old_mx = evt.where_x;
      old_my = evt.where_y;
      break;
    }
    if (count < EVENTQSIZE)
      addEvent(&evt);
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
#ifdef X11_CORE
void EVTAPI EVT_initX11(
#else
void EVTAPI EVT_init(
#endif
    _EVT_mouseMoveHandler mouseMove)
{
  int result, i,j,k;
  XDeviceInfoPtr    list,slist;

  /* Initialise the event queue */
  _mouseMove = mouseMove;
  initEventQueue();
  memset(keyUpMsg,0,sizeof(keyUpMsg));


  /* query server for input extensions */
  result =XQueryExtension(_EVT_dpy,"XInputExtension",&i,&j,&k);
  if(!result) {
    fprintf(stderr,"Your server doesn't support XInput Extensions\n");
    fprintf(stderr,"X11 Joystick disabled\n");
  }
  list = XListInputDevices(_EVT_dpy,&result);
  if (!list) {
    fprintf(stderr,"No extended input devices found !!\n");
    fprintf(stderr,"X11 Joystick disabled\n");
  }


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

/****************************************************************************
REMARKS
Sets the current X11 display
****************************************************************************/
void EVT_setX11Display(Display *dpy, Window win)
{
  _EVT_dpy = dpy;
  _EVT_win = win;
}
