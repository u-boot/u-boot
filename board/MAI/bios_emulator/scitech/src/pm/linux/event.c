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
* Environment:  Linux
*
* Description:  Linux fullscreen console implementation for the SciTech
*               cross platform event library.
*               Portions ripped straigth from the gpm source code for mouse
*               handling.
*
****************************************************************************/

/*---------------------------- Global Variables ---------------------------*/

extern int              _PM_console_fd;
static ushort           keyUpMsg[256] = {0};
static int              _EVT_mouse_fd = 0;
static int              range_x, range_y;
static int              opt_baud = 1200, opt_sample = 100;
#ifdef USE_OS_JOYSTICK
static short            *axis0 = NULL, *axis1 = NULL;
static uchar            *buts0 = NULL, *buts1 = NULL;
static int              joystick0_fd = 0, joystick1_fd = 0;
static int              js_version = 0;
#endif

/* This defines the supported mouse drivers */

typedef enum {
    EVT_noMouse = -1,
    EVT_microsoft = 0,
    EVT_ps2,
    EVT_mousesystems,
    EVT_gpm,
    EVT_MMseries,
    EVT_logitech,
    EVT_busmouse,
    EVT_mouseman,
    EVT_intellimouse,
    EVT_intellimouse_ps2,
    } mouse_drivers_t;

static mouse_drivers_t mouse_driver = EVT_noMouse;
static char mouse_dev[20] = "/dev/mouse";

typedef struct {
    char    *name;
    int     flags;
    void    (*init)(void);
    uchar   proto[4];
    int     packet_len;
    int     read;
    } mouse_info;

#define STD_FLG (CREAD | CLOCAL | HUPCL)

static void _EVT_mouse_init(void);
static void _EVT_logitech_init(void);
static void _EVT_pnpmouse_init(void);

mouse_info mouse_infos[] = {
    {"Microsoft",       CS7 | B1200 | STD_FLG,              _EVT_mouse_init,    {0x40, 0x40, 0x40, 0x00}, 3, 1},
    {"PS2",             STD_FLG,                            NULL,               {0xc0, 0x00, 0x00, 0x00}, 3, 1},
    {"MouseSystems",    CS8 | CSTOPB | STD_FLG,             _EVT_mouse_init,    {0xf8, 0x80, 0x00, 0x00}, 5, 5},
    {"GPM",             CS8 | CSTOPB | STD_FLG,             NULL,               {0xf8, 0x80, 0x00, 0x00}, 5, 5},
    {"MMSeries",        CS8 | PARENB | PARODD | STD_FLG,    _EVT_mouse_init,    {0xe0, 0x80, 0x80, 0x00}, 3, 1},
    {"Logitech",        CS8 | CSTOPB | STD_FLG,             _EVT_logitech_init, {0xe0, 0x80, 0x80, 0x00}, 3, 3},
    {"BusMouse",        STD_FLG,                            NULL,               {0xf8, 0x80, 0x00, 0x00}, 3, 3},
    {"MouseMan",        CS7 | STD_FLG,                      _EVT_mouse_init,    {0x40, 0x40, 0x40, 0x00}, 3, 1},
    {"IntelliMouse",    CS7 | STD_FLG,                      _EVT_pnpmouse_init, {0xc0, 0x40, 0xc0, 0x00}, 4, 1},
    {"IMPS2",           CS7 | STD_FLG,                      NULL,               {0xc0, 0x40, 0xc0, 0x00}, 4, 1}, /* ? */
    };

#define NB_MICE (sizeof(mouse_infos)/sizeof(mouse_info))

/* The name of the environment variables that are used to change the defaults above */

#define ENV_MOUSEDRV "MGL_MOUSEDRV"
#define ENV_MOUSEDEV "MGL_MOUSEDEV"
#define ENV_MOUSESPD "MGL_MOUSESPD"
#define ENV_JOYDEV0  "MGL_JOYDEV1"
#define ENV_JOYDEV1  "MGL_JOYDEV2"

/* Scancode mappings on Linux for special keys */

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

typedef struct {
    int     sample;
    char    code[2];
    } sample_rate;

sample_rate sampletab[]={
    {  0,"O"},
    { 15,"J"},
    { 27,"K"},
    { 42,"L"},
    { 60,"R"},
    { 85,"M"},
    {125,"Q"},
    {1E9,"N"},
    };

/* Number of keycodes to read at a time from the console */

#define KBDREADBUFFERSIZE 32

/*---------------------------- Implementation -----------------------------*/

/* These are not used under Linux */
#define _EVT_disableInt()       1
#define _EVT_restoreInt(flaps)

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
    static uint     starttime = 0;
    struct timeval  t;

    gettimeofday(&t, NULL);
    if (starttime == 0)
      starttime = t.tv_sec * 1000 + (t.tv_usec/1000);
    return ((t.tv_sec * 1000 + (t.tv_usec/1000)) - starttime);
}

/****************************************************************************
REMARKS:
Small Unix function that checks for availability on a file using select()
****************************************************************************/
static ibool dataReady(
    int fd)
{
    static struct timeval   t = { 0L, 0L };
    fd_set                  fds;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    return select(fd+1, &fds, NULL, NULL, &t) > 0;
}

/****************************************************************************
REMARKS:
Reads mouse data according to the selected mouse driver.
****************************************************************************/
static ibool readMouseData(
    int *buttons,
    int *dx,
    int *dy)
{
    static uchar    data[32],prev = 0;
    int             cnt = 0,ret;
    mouse_info      *drv;

    /* Read the first byte to check for the protocol */
    drv = &mouse_infos[mouse_driver];
    if (read(_EVT_mouse_fd, data, drv->read) != drv->read) {
	perror("read");
	return false;
	}
    if ((data[0] & drv->proto[0]) != drv->proto[1])
	return false;

    /* Load a whole protocol packet */
    cnt += drv->read;
    while (cnt < drv->packet_len) {
	ret = read(_EVT_mouse_fd, data+cnt, drv->read);
	if (ret == drv->read)
	    cnt += ret;
	else {
	    perror("read");
	    return false;
	    }
	}
    if ((data[1] & drv->proto[2]) != drv->proto[3])
	return false;

    /* Now decode the protocol packet */
    switch (mouse_driver) {
	case EVT_microsoft:
	    if (data[0] == 0x40 && !(prev|data[1]|data[2]))
		*buttons = 2;   /* Third button on MS compatible mouse */
	    else
		*buttons= ((data[0] & 0x20) >> 3) | ((data[0] & 0x10) >> 4);
	    prev = *buttons;
	    *dx = (char)(((data[0] & 0x03) << 6) | (data[1] & 0x3F));
	    *dy = (char)(((data[0] & 0x0C) << 4) | (data[2] & 0x3F));
	    break;
	case EVT_ps2:
	    *buttons = !!(data[0]&1) * 4 + !!(data[0]&2) * 1 + !!(data[0]&4) * 2;
	    if (data[1] != 0)
		*dx = (data[0] & 0x10) ? data[1]-256 : data[1];
	    else
		*dx = 0;
	    if (data[2] != 0)
		*dy = -((data[0] & 0x20) ? data[2]-256 : data[2]);
	    else
		*dy = 0;
	    break;
	case EVT_mousesystems: case EVT_gpm:
	    *buttons = (~data[0]) & 0x07;
	    *dx = (char)(data[1]) + (char)(data[3]);
	    *dy = -((char)(data[2]) + (char)(data[4]));
	    break;
	case EVT_logitech:
	    *buttons= data[0] & 0x07;
	    *dx = (data[0] & 0x10) ?   data[1] : - data[1];
	    *dy = (data[0] & 0x08) ? - data[2] :   data[2];
	    break;
	case EVT_busmouse:
	    *buttons= (~data[0]) & 0x07;
	    *dx = (char)data[1];
	    *dy = -(char)data[2];
	    break;
	case EVT_MMseries:
	    *buttons = data[0] & 0x07;
	    *dx = (data[0] & 0x10) ?   data[1] : - data[1];
	    *dy = (data[0] & 0x08) ? - data[2] :   data[2];
	    break;
	case EVT_intellimouse:
	    *buttons = ((data[0] & 0x20) >> 3)  /* left */
		     | ((data[3] & 0x10) >> 3)  /* middle */
		     | ((data[0] & 0x10) >> 4); /* right */
	    *dx = (char)(((data[0] & 0x03) << 6) | (data[1] & 0x3F));
	    *dy = (char)(((data[0] & 0x0C) << 4) | (data[2] & 0x3F));
	    break;
	case EVT_intellimouse_ps2:
	    *buttons = (data[0] & 0x04) >> 1 /* Middle */
		| (data[0] & 0x02) >> 1 /* Right */
		| (data[0] & 0x01) << 2; /* Left */
	    *dx = (data[0] & 0x10) ?    data[1]-256  :  data[1];
	    *dy = (data[0] & 0x20) ?  -(data[2]-256) : -data[2];
	    break;
	case EVT_mouseman: {
	    static int      getextra;
	    static uchar    prev=0;
	    uchar           b;

	    /* The damned MouseMan has 3/4 bytes packets. The extra byte
	     * is only there if the middle button is active.
	     * I get the extra byte as a packet with magic numbers in it.
	     * and then switch to 4-byte mode.
	     */
	    if (data[1] == 0xAA && data[2] == 0x55) {
		/* Got unexpected fourth byte */
		if ((b = (*data>>4)) > 0x3)
		    return false;  /* just a sanity check */
		*dx = *dy = 0;
		drv->packet_len=4;
		getextra=0;
		}
	    else {
		/* Got 3/4, as expected */
		/* Motion is independent of packetlen... */
		*dx = (char)(((data[0] & 0x03) << 6) | (data[1] & 0x3F));
		*dy = (char)(((data[0] & 0x0C) << 4) | (data[2] & 0x3F));
		prev = ((data[0] & 0x20) >> 3) | ((data[0] & 0x10) >> 4);
		if (drv->packet_len==4)
		    b = data[3]>>4;
		}
	    if (drv->packet_len == 4) {
		if (b == 0) {
		    drv->packet_len = 3;
		    getextra = 1;
		    }
		else {
		    if (b & 0x2)
			prev |= 2;
		    }
		}
	    *buttons = prev;

	    /* This "chord-middle" behaviour was reported by David A. van Leeuwen */
	    if (((prev ^ *buttons) & 5) == 5)
		*buttons = *buttons ? 2 : 0;
	    prev = *buttons;
	    break;
	    }
	case EVT_noMouse:
	    return false;
	    break;
	}
    return true;
}

/****************************************************************************
REMARKS:
Map a keypress via the key mapping table
****************************************************************************/
static int getKeyMapping(
    keymap *tab,
    int nb,
    int key)
{
    int i;

    for(i = 0; i < nb; i++) {
	if (tab[i].scan == key)
	    return tab[i].map;
	}
    return key;
}

#ifdef USE_OS_JOYSTICK

static char js0_axes = 0, js0_buttons = 0;
static char js1_axes = 0, js1_buttons = 0;
static char joystick0_dev[20] = "/dev/js0";
static char joystick1_dev[20] = "/dev/js1";

/****************************************************************************
REMARKS:
Create a joystick event from the joystick data
****************************************************************************/
static void makeJoyEvent(
    event_t *evt)
{
    evt->message = 0;
    if (buts0 && axis0) {
	if (buts0[0]) evt->message |= EVT_JOY1_BUTTONA;
	if (buts0[1]) evt->message |= EVT_JOY1_BUTTONB;
	evt->where_x = axis0[0];
	evt->where_y = axis0[1];
	}
    else
	evt->where_x = evt->where_y = 0;
    if (buts1 && axis1) {
	if (buts1[0]) evt->message |= EVT_JOY2_BUTTONA;
	if (buts1[1]) evt->message |= EVT_JOY2_BUTTONB;
	evt->where_x = axis1[0];
	evt->where_y = axis1[1];
	}
    else
	evt->where_x = evt->where_y = 0;
}

/****************************************************************************
REMARKS:
Read the joystick axis data
****************************************************************************/
int EVTAPI _EVT_readJoyAxis(
    int jmask,
    int *axis)
{
    int mask = 0;

    if ((js_version & ~0xffff) == 0) {
	/* Old 0.x driver */
	struct JS_DATA_TYPE js;
	if (joystick0_fd && read(joystick0_fd, &js, JS_RETURN) == JS_RETURN) {
	    if (jmask & EVT_JOY_AXIS_X1)
		axis[0] = js.x;
	    if (jmask & EVT_JOY_AXIS_Y1)
		axis[1] = js.y;
	    mask |= EVT_JOY_AXIS_X1|EVT_JOY_AXIS_Y1;
	    }
	if (joystick1_fd && read(joystick1_fd, &js, JS_RETURN) == JS_RETURN) {
	    if (jmask & EVT_JOY_AXIS_X2)
		axis[2] = js.x;
	    if (jmask & EVT_JOY_AXIS_Y2)
		axis[3] = js.y;
	    mask |= EVT_JOY_AXIS_X2|EVT_JOY_AXIS_Y2;
	    }
	}
    else {
	if (axis0) {
	    if (jmask & EVT_JOY_AXIS_X1)
		axis[0] = axis0[0];
	    if (jmask & EVT_JOY_AXIS_Y1)
		axis[1] = axis0[1];
	    mask |= EVT_JOY_AXIS_X1 | EVT_JOY_AXIS_Y1;
	    }
	if (axis1) {
	    if (jmask & EVT_JOY_AXIS_X2)
		axis[2] = axis1[0];
	    if (jmask & EVT_JOY_AXIS_Y2)
		axis[3] = axis1[1];
	    mask |= EVT_JOY_AXIS_X2 | EVT_JOY_AXIS_Y2;
	    }
	}
    return mask;
}

/****************************************************************************
REMARKS:
Read the joystick button data
****************************************************************************/
int EVTAPI _EVT_readJoyButtons(void)
{
    int buts = 0;

    if ((js_version & ~0xffff) == 0) {
	/* Old 0.x driver */
	struct JS_DATA_TYPE js;
	if (joystick0_fd && read(joystick0_fd, &js, JS_RETURN) == JS_RETURN)
	    buts = js.buttons;
	if (joystick1_fd && read(joystick1_fd, &js, JS_RETURN) == JS_RETURN)
	    buts |= js.buttons << 2;
	}
    else {
	if (buts0)
	    buts |= EVT_JOY1_BUTTONA*buts0[0] + EVT_JOY1_BUTTONB*buts0[1];
	if (buts1)
	    buts |= EVT_JOY2_BUTTONA*buts1[0] + EVT_JOY2_BUTTONB*buts1[1];
	}
    return buts;
}

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
    static int      mask = 0;
    int             i;
    char            *tmp, name0[128], name1[128];
    static ibool    inited = false;

    if (inited)
	return mask;
    memset(EVT.joyMin,0,sizeof(EVT.joyMin));
    memset(EVT.joyCenter,0,sizeof(EVT.joyCenter));
    memset(EVT.joyMax,0,sizeof(EVT.joyMax));
    memset(EVT.joyPrev,0,sizeof(EVT.joyPrev));
    EVT.joyButState = 0;
    if ((tmp = getenv(ENV_JOYDEV0)) != NULL)
	strcpy(joystick0_dev,tmp);
    if ((tmp = getenv(ENV_JOYDEV1)) != NULL)
	strcpy(joystick1_dev,tmp);
    if ((joystick0_fd = open(joystick0_dev, O_RDONLY)) < 0)
	joystick0_fd = 0;
    if ((joystick1_fd = open(joystick1_dev, O_RDONLY)) < 0)
	joystick1_fd = 0;
    if (!joystick0_fd && !joystick1_fd) /* No joysticks detected */
	return 0;
    inited = true;
    if (ioctl(joystick0_fd ? joystick0_fd : joystick1_fd, JSIOCGVERSION, &js_version) < 0)
	return 0;

    /* Initialise joystick 0 */
    if (joystick0_fd) {
	ioctl(joystick0_fd, JSIOCGNAME(sizeof(name0)), name0);
	if (js_version & ~0xffff) {
	    struct js_event js;

	    ioctl(joystick0_fd, JSIOCGAXES, &js0_axes);
	    ioctl(joystick0_fd, JSIOCGBUTTONS, &js0_buttons);
	    axis0 = PM_calloc((int)js0_axes, sizeof(short));
	    buts0 = PM_malloc((int)js0_buttons);
	    /* Read the initial events */
	    while(dataReady(joystick0_fd)
		  && read(joystick0_fd, &js, sizeof(struct js_event)) == sizeof(struct js_event)
		  && (js.type & JS_EVENT_INIT)
		  ) {
		if (js.type & JS_EVENT_BUTTON)
		    buts0[js.number] = js.value;
		else if (js.type & JS_EVENT_AXIS)
		    axis0[js.number] = scaleJoyAxis(js.value,js.number);
		}
	    }
	else {
	    js0_axes = 2;
	    js0_buttons = 2;
	    axis0 = PM_calloc((int)js0_axes, sizeof(short));
	    buts0 = PM_malloc((int)js0_buttons);
	    }
	}

    /* Initialise joystick 1 */
    if (joystick1_fd) {
	ioctl(joystick1_fd, JSIOCGNAME(sizeof(name1)), name1);
	if (js_version & ~0xffff) {
	    struct js_event js;

	    ioctl(joystick1_fd, JSIOCGAXES, &js1_axes);
	    ioctl(joystick1_fd, JSIOCGBUTTONS, &js1_buttons);
	    axis1 = PM_calloc((int)js1_axes, sizeof(short));
	    buts1 = PM_malloc((int)js1_buttons);
	    /* Read the initial events */
	    while(dataReady(joystick1_fd)
		  && read(joystick1_fd, &js, sizeof(struct js_event))==sizeof(struct js_event)
		  && (js.type & JS_EVENT_INIT)
		  ) {
		if (js.type & JS_EVENT_BUTTON)
		    buts1[js.number] = js.value;
		else if (js.type & JS_EVENT_AXIS)
		    axis1[js.number] = scaleJoyAxis(js.value,js.number<<2);
		}
	    }
	else {
	    js1_axes = 2;
	    js1_buttons = 2;
	    axis1 = PM_calloc((int)js1_axes, sizeof(short));
	    buts1 = PM_malloc((int)js1_buttons);
	    }
	}

#ifdef  CHECKED
    fprintf(stderr,"Using joystick driver version %d.%d.%d\n",
	    js_version >> 16, (js_version >> 8) & 0xff, js_version & 0xff);
    if (joystick0_fd)
	fprintf(stderr,"Joystick 1 (%s): %s\n", joystick0_dev, name0);
    if (joystick1_fd)
	fprintf(stderr,"Joystick 2 (%s): %s\n", joystick1_dev, name1);
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

    if ((js_version & ~0xFFFF) == 0 && EVT.joyMask) {
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
		/* Add a new joystick movement event */
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
REMARKS:
Pumps all messages in the message queue from Linux into our event queue.
****************************************************************************/
static void _EVT_pumpMessages(void)
{
    event_t                 evt;
    int                     i,numkeys, c;
    ibool                   release;
    static struct kbentry   ke;
    static char             buf[KBDREADBUFFERSIZE];
    static ushort           repeatKey[128] = {0};

    /* Poll keyboard events */
    while (dataReady(_PM_console_fd) && (numkeys = read(_PM_console_fd, buf, KBDREADBUFFERSIZE)) > 0) {
	for (i = 0; i < numkeys; i++) {
	    c = buf[i];
	    release = c & 0x80;
	    c &= 0x7F;

	    /* TODO:    This is wrong! We need this to be the time stamp at */
	    /*          ** interrupt ** time!! One solution would be to */
	    /*          put the keyboard and mouse polling loops into */
	    /*          a separate thread that can block on I/O to the */
	    /*          necessay file descriptor. */
	    evt.when = _EVT_getTicks();

	    if (release) {
		/* Key released */
		evt.what = EVT_KEYUP;
		switch (c) {
		    case KB_leftShift:
			_PM_modifiers &= ~EVT_LEFTSHIFT;
			break;
		    case KB_rightShift:
			_PM_modifiers &= ~EVT_RIGHTSHIFT;
			break;
		    case 29:
			_PM_modifiers &= ~(EVT_LEFTCTRL|EVT_CTRLSTATE);
			break;
		    case 97:            /* Control */
			_PM_modifiers &= ~EVT_CTRLSTATE;
			break;
		    case 56:
			_PM_modifiers &= ~(EVT_LEFTALT|EVT_ALTSTATE);
			break;
		    case 100:
			_PM_modifiers &= ~EVT_ALTSTATE;
			break;
		    default:
		    }
		evt.modifiers = _PM_modifiers;
		evt.message = keyUpMsg[c];
		if (EVT.count < EVENTQSIZE)
		    addEvent(&evt);
		keyUpMsg[c] = 0;
		repeatKey[c] = 0;
		}
	    else {
		/* Key pressed */
		evt.what = EVT_KEYDOWN;
		switch (c) {
		    case KB_leftShift:
			_PM_modifiers |= EVT_LEFTSHIFT;
			break;
		    case KB_rightShift:
			_PM_modifiers |= EVT_RIGHTSHIFT;
			break;
		    case 29:
			_PM_modifiers |= EVT_LEFTCTRL|EVT_CTRLSTATE;
			break;
		    case 97:            /* Control */
			_PM_modifiers |= EVT_CTRLSTATE;
			break;
		    case 56:
			_PM_modifiers |= EVT_LEFTALT|EVT_ALTSTATE;
			break;
		    case 100:
			_PM_modifiers |= EVT_ALTSTATE;
			break;
		    case KB_capsLock:   /* Caps Lock */
			_PM_leds ^= LED_CAP;
			ioctl(_PM_console_fd, KDSETLED, _PM_leds);
			break;
		    case KB_numLock:    /* Num Lock */
			_PM_leds ^= LED_NUM;
			ioctl(_PM_console_fd, KDSETLED, _PM_leds);
			break;
		    case KB_scrollLock: /* Scroll Lock */
			_PM_leds ^= LED_SCR;
			ioctl(_PM_console_fd, KDSETLED, _PM_leds);
			break;
		    default:
		    }
		evt.modifiers = _PM_modifiers;
		if (keyUpMsg[c]) {
		    evt.what = EVT_KEYREPEAT;
		    evt.message = keyUpMsg[c] | (repeatKey[c]++ << 16);
		    }
		else {
		    int asc;

		    evt.message = getKeyMapping(keymaps, NB_KEYMAPS, c) << 8;
		    ke.kb_index = c;
		    ke.kb_table = 0;
		    if ((_PM_modifiers & EVT_SHIFTKEY) || (_PM_leds & LED_CAP))
			ke.kb_table |= K_SHIFTTAB;
		    if (_PM_modifiers & (EVT_LEFTALT | EVT_ALTSTATE))
			ke.kb_table |= K_ALTTAB;
		    if (ioctl(_PM_console_fd, KDGKBENT, (unsigned long)&ke)<0)
			perror("ioctl(KDGKBENT)");
		    if ((_PM_leds & LED_NUM) && (getKeyMapping(keypad, NB_KEYPAD, c)!=c)) {
			asc = getKeyMapping(keypad, NB_KEYPAD, c);
			}
		    else {
			switch (c) {
			    case 14:
				asc = ASCII_backspace;
				break;
			    case 15:
				asc = ASCII_tab;
				break;
			    case 28:
			    case 96:
				asc = ASCII_enter;
				break;
			    case 1:
				asc = ASCII_esc;
			    default:
				asc = ke.kb_value & 0xFF;
				if (asc < 0x1B)
				    asc = 0;
				break;
			    }
			}
		    if ((_PM_modifiers & (EVT_CTRLSTATE|EVT_LEFTCTRL)) && isalpha(asc))
			evt.message |= toupper(asc) - 'A' + 1;
		    else
			evt.message |= asc;
		    keyUpMsg[c] = evt.message;
		    repeatKey[c]++;
		    }
		if (EVT.count < EVENTQSIZE)
		    addEvent(&evt);
		}
	    }
	}

    /* Poll mouse events */
    if (_EVT_mouse_fd) {
	int         dx, dy, buts;
	static int  oldbuts;

	while (dataReady(_EVT_mouse_fd)) {
	    if (readMouseData(&buts, &dx, &dy)) {
		EVT.mx += dx;
		EVT.my += dy;
		if (EVT.mx < 0) EVT.mx = 0;
		if (EVT.my < 0) EVT.my = 0;
		if (EVT.mx > range_x) EVT.mx = range_x;
		if (EVT.my > range_y) EVT.my = range_y;
		evt.where_x = EVT.mx;
		evt.where_y = EVT.my;
		evt.relative_x = dx;
		evt.relative_y = dy;

		/* TODO:    This is wrong! We need this to be the time stamp at */
		/*          ** interrupt ** time!! One solution would be to */
		/*          put the keyboard and mouse polling loops into */
		/*          a separate thread that can block on I/O to the */
		/*          necessay file descriptor. */
		evt.when = _EVT_getTicks();
		evt.modifiers = _PM_modifiers;
		if (buts & 4)
		    evt.modifiers |= EVT_LEFTBUT;
		if (buts & 1)
		    evt.modifiers |= EVT_RIGHTBUT;
		if (buts & 2)
		    evt.modifiers |= EVT_MIDDLEBUT;

		/* Left click events */
		if ((buts&4) != (oldbuts&4)) {
		    if (buts&4)
			evt.what = EVT_MOUSEDOWN;
		    else
			evt.what = EVT_MOUSEUP;
		    evt.message = EVT_LEFTBMASK;
		    EVT.oldMove = -1;
		    if (EVT.count < EVENTQSIZE)
			addEvent(&evt);
		    }

		/* Right click events */
		if ((buts&1) != (oldbuts&1)) {
		    if (buts&1)
			evt.what = EVT_MOUSEDOWN;
		    else
			evt.what = EVT_MOUSEUP;
		    evt.message = EVT_RIGHTBMASK;
		    EVT.oldMove = -1;
		    if (EVT.count < EVENTQSIZE)
			addEvent(&evt);
		    }

		/* Middle click events */
		if ((buts&2) != (oldbuts&2)) {
		    if (buts&2)
			evt.what = EVT_MOUSEDOWN;
		    else
			evt.what = EVT_MOUSEUP;
		    evt.message = EVT_MIDDLEBMASK;
		    EVT.oldMove = -1;
		    if (EVT.count < EVENTQSIZE)
			addEvent(&evt);
		    }

		/* Mouse movement event */
		if (dx || dy) {
		    evt.what = EVT_MOUSEMOVE;
		    evt.message = 0;
		    if (EVT.oldMove != -1) {
			/* Modify existing movement event */
			EVT.evtq[EVT.oldMove].where_x = evt.where_x;
			EVT.evtq[EVT.oldMove].where_y = evt.where_y;
			}
		    else {
			/* Save id of this movement event */
			EVT.oldMove = EVT.freeHead;
			if (EVT.count < EVENTQSIZE)
			    addEvent(&evt);
			}
		    }
		oldbuts = buts;
		}
	    }
	}

#ifdef USE_OS_JOYSTICK
    /* Poll joystick events using the 1.x joystick driver API in the 2.2 kernels */
    if (js_version & ~0xffff) {
	static struct js_event  js;

	/* Read joystick axis 0 */
	evt.when = 0;
	evt.modifiers = _PM_modifiers;
	if (joystick0_fd && dataReady(joystick0_fd) &&
		read(joystick0_fd, &js, sizeof(js)) == sizeof(js)) {
	    if (js.type & JS_EVENT_BUTTON) {
		if (js.number < 2) { /* Only 2 buttons for now :( */
		    buts0[js.number] = js.value;
		    evt.what = EVT_JOYCLICK;
		    makeJoyEvent(&evt);
		    if (EVT.count < EVENTQSIZE)
			addEvent(&evt);
		    }
		}
	    else if (js.type & JS_EVENT_AXIS) {
		axis0[js.number] = scaleJoyAxis(js.value,js.number);
		evt.what = EVT_JOYMOVE;
		if (EVT.oldJoyMove != -1) {
		    makeJoyEvent(&EVT.evtq[EVT.oldJoyMove]);
		    }
		else if (EVT.count < EVENTQSIZE) {
		    EVT.oldJoyMove = EVT.freeHead;
		    makeJoyEvent(&evt);
		    addEvent(&evt);
		    }
		}
	    }

	/* Read joystick axis 1 */
	if (joystick1_fd && dataReady(joystick1_fd) &&
		read(joystick1_fd, &js, sizeof(js))==sizeof(js)) {
	    if (js.type & JS_EVENT_BUTTON) {
		if (js.number < 2) { /* Only 2 buttons for now :( */
		    buts1[js.number] = js.value;
		    evt.what = EVT_JOYCLICK;
		    makeJoyEvent(&evt);
		    if (EVT.count < EVENTQSIZE)
			addEvent(&evt);
		    }
		}
	    else if (js.type & JS_EVENT_AXIS) {
		axis1[js.number] = scaleJoyAxis(js.value,js.number<<2);
		evt.what = EVT_JOYMOVE;
		if (EVT.oldJoyMove != -1) {
		    makeJoyEvent(&EVT.evtq[EVT.oldJoyMove]);
		    }
		else if (EVT.count < EVENTQSIZE) {
		    EVT.oldJoyMove = EVT.freeHead;
		    makeJoyEvent(&evt);
		    addEvent(&evt);
		    }
		}
	    }
	}
#endif
}

/****************************************************************************
REMARKS:
This macro/function is used to converts the scan codes reported by the
keyboard to our event libraries normalised format. We only have one scan
code for the 'A' key, and use shift _PM_modifiers to determine if it is a
Ctrl-F1, Alt-F1 etc. The raw scan codes from the keyboard work this way,
but the OS gives us 'cooked' scan codes, we have to translate them back
to the raw format.
****************************************************************************/
#define _EVT_maskKeyCode(evt)

/****************************************************************************
REMARKS:
Set the speed of the serial port
****************************************************************************/
static int setspeed(
    int fd,
    int old,
    int new,
    unsigned short flags)
{
    struct termios tty;
    char *c;

    tcgetattr(fd, &tty);
    tty.c_iflag = IGNBRK | IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_line = 0;
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 1;
    switch (old) {
	case 9600:  tty.c_cflag = flags | B9600; break;
	case 4800:  tty.c_cflag = flags | B4800; break;
	case 2400:  tty.c_cflag = flags | B2400; break;
	case 1200:
	default:    tty.c_cflag = flags | B1200; break;
	}
    tcsetattr(fd, TCSAFLUSH, &tty);
    switch (new) {
	case 9600:  c = "*q";  tty.c_cflag = flags | B9600; break;
	case 4800:  c = "*p";  tty.c_cflag = flags | B4800; break;
	case 2400:  c = "*o";  tty.c_cflag = flags | B2400; break;
	case 1200:
	default:    c = "*n";  tty.c_cflag = flags | B1200; break;
	}
    write(fd, c, 2);
    usleep(100000);
    tcsetattr(fd, TCSAFLUSH, &tty);
    return 0;
}

/****************************************************************************
REMARKS:
Generic mouse driver init code
****************************************************************************/
static void _EVT_mouse_init(void)
{
    int i;

    /* Change from any available speed to the chosen one */
    for (i = 9600; i >= 1200; i /= 2)
	setspeed(_EVT_mouse_fd, i, opt_baud, mouse_infos[mouse_driver].flags);
}

/****************************************************************************
REMARKS:
Logitech mouse driver init code
****************************************************************************/
static void _EVT_logitech_init(void)
{
    int         i;
    struct stat buf;
    int         busmouse;

    /* is this a serial- or a bus- mouse? */
    if (fstat(_EVT_mouse_fd,&buf) == -1)
	perror("fstat");
    i = MAJOR(buf.st_rdev);
    if (stat("/dev/ttyS0",&buf) == -1)
	perror("stat");
    busmouse=(i != MAJOR(buf.st_rdev));

    /* Fix the howmany field, so that serial mice have 1, while busmice have 3 */
    mouse_infos[mouse_driver].read = busmouse ? 3 : 1;

    /* Change from any available speed to the chosen one */
    for (i = 9600; i >= 1200; i /= 2)
	setspeed(_EVT_mouse_fd, i, opt_baud, mouse_infos[mouse_driver].flags);

    /* This stuff is peculiar of logitech mice, also for the serial ones */
    write(_EVT_mouse_fd, "S", 1);
    setspeed(_EVT_mouse_fd, opt_baud, opt_baud,CS8 |PARENB |PARODD |CREAD |CLOCAL |HUPCL);

    /* Configure the sample rate */
    for (i = 0; opt_sample <= sampletab[i].sample; i++)
	;
    write(_EVT_mouse_fd,sampletab[i].code,1);
}

/****************************************************************************
REMARKS:
Microsoft Intellimouse init code
****************************************************************************/
static void _EVT_pnpmouse_init(void)
{
    struct termios tty;

    tcgetattr(_EVT_mouse_fd, &tty);
    tty.c_iflag = IGNBRK | IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_line = 0;
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cflag = mouse_infos[mouse_driver].flags | B1200;
    tcsetattr(_EVT_mouse_fd, TCSAFLUSH, &tty); /* set parameters */
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
    char        *tmp;

    /* Initialise the event queue */
    EVT.mouseMove = mouseMove;
    initEventQueue();
    for (i = 0; i < 256; i++)
	keyUpMsg[i] = 0;

    /* Keyboard initialization */
    if (_PM_console_fd == -1)
	PM_fatalError("You must first call PM_openConsole to use the EVT functions!");
    _PM_keyboard_rawmode();
    fcntl(_PM_console_fd,F_SETFL,fcntl(_PM_console_fd,F_GETFL) | O_NONBLOCK);

    /* Mouse initialization */
    if ((tmp = getenv(ENV_MOUSEDRV)) != NULL) {
	for (i = 0; i < NB_MICE; i++) {
	    if (!strcasecmp(tmp, mouse_infos[i].name)) {
		mouse_driver = i;
		break;
		}
	    }
	if (i == NB_MICE) {
	    fprintf(stderr,"Unknown mouse driver: %s\n", tmp);
	    mouse_driver = EVT_noMouse;
	    _EVT_mouse_fd = 0;
	    }
	}
    if (mouse_driver != EVT_noMouse) {
	if (mouse_driver == EVT_gpm)
	    strcpy(mouse_dev,"/dev/gpmdata");
	if ((tmp = getenv(ENV_MOUSEDEV)) != NULL)
	    strcpy(mouse_dev,tmp);
#ifdef  CHECKED
	fprintf(stderr,"Using the %s MGL mouse driver on %s.\n", mouse_infos[mouse_driver].name, mouse_dev);
#endif
	if ((_EVT_mouse_fd = open(mouse_dev, O_RDWR)) < 0) {
	    perror("open");
	    fprintf(stderr, "Unable to open mouse device %s, dropping mouse support.\n", mouse_dev);
	    sleep(1);
	    mouse_driver = EVT_noMouse;
	    _EVT_mouse_fd = 0;
	    }
	else {
	    char c;

	    /* Init and flush the mouse pending input queue */
	    if (mouse_infos[mouse_driver].init)
		mouse_infos[mouse_driver].init();
	    while(dataReady(_EVT_mouse_fd) && read(_EVT_mouse_fd, &c, 1) == 1)
		;
	    }
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
    range_x = xRes;
    range_y = yRes;
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
    /* Do nothing for Linux */
}

/****************************************************************************
REMARKS
Suspends all of our event handling operations. This is also used to
de-install the event handling code.
****************************************************************************/
void EVT_suspend(void)
{
    /* Do nothing for Linux */
}

/****************************************************************************
REMARKS
Exits the event module for program terminatation.
****************************************************************************/
void EVT_exit(void)
{
    /* Restore signal handlers */
    _PM_restore_kb_mode();
    if (_EVT_mouse_fd) {
	close(_EVT_mouse_fd);
	_EVT_mouse_fd = 0;
	}
#ifdef USE_OS_JOYSTICK
    if (joystick0_fd) {
	close(joystick0_fd);
	free(axis0);
	free(buts0);
	joystick0_fd = 0;
	}
    if (joystick1_fd) {
	close(joystick1_fd);
	free(axis1);
	free(buts1);
	joystick1_fd = 0;
	}
#endif
}
