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
* Description:  Header file for the SciTech cross platform event library
*
****************************************************************************/

#ifndef __EVENT_H
#define __EVENT_H

#include "scitech.h"

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)

/* 'C' calling conventions always */

#define EVTAPI  _ASMAPI
#define EVTAPIP _ASMAPIP

/* Event message masks for keyDown events */

#define EVT_ASCIIMASK   0x00FF      /* ASCII code of key pressed        */
#define EVT_SCANMASK    0xFF00      /* Scan code of key pressed         */
#define EVT_COUNTMASK   0x7FFF0000L /* Count for KEYREPEAT's            */

/* Macros to extract values from the message fields */

#define EVT_asciiCode(m)    ( (uchar) (m & EVT_ASCIIMASK) )
#define EVT_scanCode(m)     ( (uchar) ( (m & EVT_SCANMASK) >> 8 ) )
#define EVT_repeatCount(m)  ( (short) ( (m & EVT_COUNTMASK) >> 16 ) )

/****************************************************************************
REMARKS:
Defines the set of ASCII codes reported by the event library functions
in the message field. Use the EVT_asciiCode macro to extract the code
from the event structure.

HEADER:
event.h
****************************************************************************/
typedef enum {
    ASCII_ctrlA             = 0x01,
    ASCII_ctrlB             = 0x02,
    ASCII_ctrlC             = 0x03,
    ASCII_ctrlD             = 0x04,
    ASCII_ctrlE             = 0x05,
    ASCII_ctrlF             = 0x06,
    ASCII_ctrlG             = 0x07,
    ASCII_backspace         = 0x08,
    ASCII_ctrlH             = 0x08,
    ASCII_tab               = 0x09,
    ASCII_ctrlI             = 0x09,
    ASCII_ctrlJ             = 0x0A,
    ASCII_ctrlK             = 0x0B,
    ASCII_ctrlL             = 0x0C,
    ASCII_enter             = 0x0D,
    ASCII_ctrlM             = 0x0D,
    ASCII_ctrlN             = 0x0E,
    ASCII_ctrlO             = 0x0F,
    ASCII_ctrlP             = 0x10,
    ASCII_ctrlQ             = 0x11,
    ASCII_ctrlR             = 0x12,
    ASCII_ctrlS             = 0x13,
    ASCII_ctrlT             = 0x14,
    ASCII_ctrlU             = 0x15,
    ASCII_ctrlV             = 0x16,
    ASCII_ctrlW             = 0x17,
    ASCII_ctrlX             = 0x18,
    ASCII_ctrlY             = 0x19,
    ASCII_ctrlZ             = 0x1A,
    ASCII_esc               = 0x1B,
    ASCII_space             = 0x20,
    ASCII_exclamation       = 0x21, /* ! */
    ASCII_quote             = 0x22, /* " */
    ASCII_pound             = 0x23, /* # */
    ASCII_dollar            = 0x24, /* $ */
    ASCII_percent           = 0x25, /* % */
    ASCII_ampersand         = 0x26, /* & */
    ASCII_apostrophe        = 0x27, /* ' */
    ASCII_leftBrace         = 0x28, /* ( */
    ASCII_rightBrace        = 0x29, /* ) */
    ASCII_times             = 0x2A, /* * */
    ASCII_plus              = 0x2B, /* + */
    ASCII_comma             = 0x2C, /* , */
    ASCII_minus             = 0x2D, /* - */
    ASCII_period            = 0x2E, /* . */
    ASCII_divide            = 0x2F, /* / */
    ASCII_0                 = 0x30,
    ASCII_1                 = 0x31,
    ASCII_2                 = 0x32,
    ASCII_3                 = 0x33,
    ASCII_4                 = 0x34,
    ASCII_5                 = 0x35,
    ASCII_6                 = 0x36,
    ASCII_7                 = 0x37,
    ASCII_8                 = 0x38,
    ASCII_9                 = 0x39,
    ASCII_colon             = 0x3A, /* : */
    ASCII_semicolon         = 0x3B, /* ; */
    ASCII_lessThan          = 0x3C, /* < */
    ASCII_equals            = 0x3D, /* = */
    ASCII_greaterThan       = 0x3E, /* > */
    ASCII_question          = 0x3F, /* ? */
    ASCII_at                = 0x40, /* @ */
    ASCII_A                 = 0x41,
    ASCII_B                 = 0x42,
    ASCII_C                 = 0x43,
    ASCII_D                 = 0x44,
    ASCII_E                 = 0x45,
    ASCII_F                 = 0x46,
    ASCII_G                 = 0x47,
    ASCII_H                 = 0x48,
    ASCII_I                 = 0x49,
    ASCII_J                 = 0x4A,
    ASCII_K                 = 0x4B,
    ASCII_L                 = 0x4C,
    ASCII_M                 = 0x4D,
    ASCII_N                 = 0x4E,
    ASCII_O                 = 0x4F,
    ASCII_P                 = 0x50,
    ASCII_Q                 = 0x51,
    ASCII_R                 = 0x52,
    ASCII_S                 = 0x53,
    ASCII_T                 = 0x54,
    ASCII_U                 = 0x55,
    ASCII_V                 = 0x56,
    ASCII_W                 = 0x57,
    ASCII_X                 = 0x58,
    ASCII_Y                 = 0x59,
    ASCII_Z                 = 0x5A,
    ASCII_leftSquareBrace   = 0x5B, /* [ */
    ASCII_backSlash         = 0x5C, /* \ */
    ASCII_rightSquareBrace  = 0x5D, /* ] */
    ASCII_caret             = 0x5E, /* ^ */
    ASCII_underscore        = 0x5F, /* _ */
    ASCII_leftApostrophe    = 0x60, /* ` */
    ASCII_a                 = 0x61,
    ASCII_b                 = 0x62,
    ASCII_c                 = 0x63,
    ASCII_d                 = 0x64,
    ASCII_e                 = 0x65,
    ASCII_f                 = 0x66,
    ASCII_g                 = 0x67,
    ASCII_h                 = 0x68,
    ASCII_i                 = 0x69,
    ASCII_j                 = 0x6A,
    ASCII_k                 = 0x6B,
    ASCII_l                 = 0x6C,
    ASCII_m                 = 0x6D,
    ASCII_n                 = 0x6E,
    ASCII_o                 = 0x6F,
    ASCII_p                 = 0x70,
    ASCII_q                 = 0x71,
    ASCII_r                 = 0x72,
    ASCII_s                 = 0x73,
    ASCII_t                 = 0x74,
    ASCII_u                 = 0x75,
    ASCII_v                 = 0x76,
    ASCII_w                 = 0x77,
    ASCII_x                 = 0x78,
    ASCII_y                 = 0x79,
    ASCII_z                 = 0x7A,
    ASCII_leftCurlyBrace    = 0x7B, /* { */
    ASCII_verticalBar       = 0x7C, /* | */
    ASCII_rightCurlyBrace   = 0x7D, /* } */
    ASCII_tilde             = 0x7E  /* ~ */
    } EVT_asciiCodesType;

/****************************************************************************
REMARKS:
Defines the set of scan codes reported by the event library functions
in the message field. Use the EVT_scanCode macro to extract the code
from the event structure. Note that the scan codes reported will be the
same across all keyboards (assuming the placement of keys on a 101 key US
keyboard), but the translated ASCII values may be different depending on
the country code pages in use.

NOTE:   Scan codes in the event library are not really hardware scan codes,
	but rather virtual scan codes as generated by a low level keyboard
	interface driver. All virtual codes begin with scan code 0x60 and
	range up from there.

HEADER:
event.h
****************************************************************************/
typedef enum {
    KB_padEnter             = 0x60, /* Keypad keys */
    KB_padMinus             = 0x4A,
    KB_padPlus              = 0x4E,
    KB_padTimes             = 0x37,
    KB_padDivide            = 0x61,
    KB_padLeft              = 0x62,
    KB_padRight             = 0x63,
    KB_padUp                = 0x64,
    KB_padDown              = 0x65,
    KB_padInsert            = 0x66,
    KB_padDelete            = 0x67,
    KB_padHome              = 0x68,
    KB_padEnd               = 0x69,
    KB_padPageUp            = 0x6A,
    KB_padPageDown          = 0x6B,
    KB_padCenter            = 0x4C,
    KB_F1                   = 0x3B, /* Function keys */
    KB_F2                   = 0x3C,
    KB_F3                   = 0x3D,
    KB_F4                   = 0x3E,
    KB_F5                   = 0x3F,
    KB_F6                   = 0x40,
    KB_F7                   = 0x41,
    KB_F8                   = 0x42,
    KB_F9                   = 0x43,
    KB_F10                  = 0x44,
    KB_F11                  = 0x57,
    KB_F12                  = 0x58,
    KB_left                 = 0x4B, /* Cursor control keys */
    KB_right                = 0x4D,
    KB_up                   = 0x48,
    KB_down                 = 0x50,
    KB_insert               = 0x52,
    KB_delete               = 0x53,
    KB_home                 = 0x47,
    KB_end                  = 0x4F,
    KB_pageUp               = 0x49,
    KB_pageDown             = 0x51,
    KB_capsLock             = 0x3A,
    KB_numLock              = 0x45,
    KB_scrollLock           = 0x46,
    KB_leftShift            = 0x2A,
    KB_rightShift           = 0x36,
    KB_leftCtrl             = 0x1D,
    KB_rightCtrl            = 0x6C,
    KB_leftAlt              = 0x38,
    KB_rightAlt             = 0x6D,
    KB_leftWindows          = 0x5B,
    KB_rightWindows         = 0x5C,
    KB_menu                 = 0x5D,
    KB_sysReq               = 0x54,
    KB_esc                  = 0x01, /* Normal keyboard keys */
    KB_1                    = 0x02,
    KB_2                    = 0x03,
    KB_3                    = 0x04,
    KB_4                    = 0x05,
    KB_5                    = 0x06,
    KB_6                    = 0x07,
    KB_7                    = 0x08,
    KB_8                    = 0x09,
    KB_9                    = 0x0A,
    KB_0                    = 0x0B,
    KB_minus                = 0x0C,
    KB_equals               = 0x0D,
    KB_backSlash            = 0x2B,
    KB_backspace            = 0x0E,
    KB_tab                  = 0x0F,
    KB_Q                    = 0x10,
    KB_W                    = 0x11,
    KB_E                    = 0x12,
    KB_R                    = 0x13,
    KB_T                    = 0x14,
    KB_Y                    = 0x15,
    KB_U                    = 0x16,
    KB_I                    = 0x17,
    KB_O                    = 0x18,
    KB_P                    = 0x19,
    KB_leftSquareBrace      = 0x1A,
    KB_rightSquareBrace     = 0x1B,
    KB_enter                = 0x1C,
    KB_A                    = 0x1E,
    KB_S                    = 0x1F,
    KB_D                    = 0x20,
    KB_F                    = 0x21,
    KB_G                    = 0x22,
    KB_H                    = 0x23,
    KB_J                    = 0x24,
    KB_K                    = 0x25,
    KB_L                    = 0x26,
    KB_semicolon            = 0x27,
    KB_apostrophe           = 0x28,
    KB_Z                    = 0x2C,
    KB_X                    = 0x2D,
    KB_C                    = 0x2E,
    KB_V                    = 0x2F,
    KB_B                    = 0x30,
    KB_N                    = 0x31,
    KB_M                    = 0x32,
    KB_comma                = 0x33,
    KB_period               = 0x34,
    KB_divide               = 0x35,
    KB_space                = 0x39,
    KB_tilde                = 0x29
    } EVT_scanCodesType;

/****************************************************************************
REMARKS:
Defines the mask for the joystick axes that are present

HEADER:
event.h

MEMBERS:
EVT_JOY_AXIS_X1     - Joystick 1, X axis is present
EVT_JOY_AXIS_Y1     - Joystick 1, Y axis is present
EVT_JOY_AXIS_X2     - Joystick 2, X axis is present
EVT_JOY_AXIS_Y2     - Joystick 2, Y axis is present
EVT_JOY_AXIS_ALL    - Mask for all axes
****************************************************************************/
typedef enum {
    EVT_JOY_AXIS_X1     = 0x00000001,
    EVT_JOY_AXIS_Y1     = 0x00000002,
    EVT_JOY_AXIS_X2     = 0x00000004,
    EVT_JOY_AXIS_Y2     = 0x00000008,
    EVT_JOY_AXIS_ALL    = 0x0000000F
    } EVT_eventJoyAxisType;

/****************************************************************************
REMARKS:
Defines the event message masks for joystick events

HEADER:
event.h

MEMBERS:
EVT_JOY1_BUTTONA    - Joystick 1, button A is down
EVT_JOY1_BUTTONB    - Joystick 1, button B is down
EVT_JOY2_BUTTONA    - Joystick 2, button A is down
EVT_JOY2_BUTTONB    - Joystick 2, button B is down
****************************************************************************/
typedef enum {
    EVT_JOY1_BUTTONA    = 0x00000001,
    EVT_JOY1_BUTTONB    = 0x00000002,
    EVT_JOY2_BUTTONA    = 0x00000004,
    EVT_JOY2_BUTTONB    = 0x00000008
    } EVT_eventJoyMaskType;

/****************************************************************************
REMARKS:
Defines the event message masks for mouse events

HEADER:
event.h

MEMBERS:
EVT_LEFTBMASK   - Left button is held down
EVT_RIGHTBMASK  - Right button is held down
EVT_MIDDLEBMASK - Middle button is held down
EVT_BOTHBMASK   - Both left and right held down together
EVT_ALLBMASK    - All buttons pressed
EVT_DBLCLICK    - Set if mouse down event was a double click
****************************************************************************/
typedef enum {
    EVT_LEFTBMASK   = 0x00000001,
    EVT_RIGHTBMASK  = 0x00000002,
    EVT_MIDDLEBMASK = 0x00000004,
    EVT_BOTHBMASK   = 0x00000007,
    EVT_ALLBMASK    = 0x00000007,
    EVT_DBLCLICK    = 0x00010000
    } EVT_eventMouseMaskType;

/****************************************************************************
REMARKS:
Defines the event modifier masks. These are the masks used to extract
the modifier information from the modifiers field of the event_t structure.
Note that the values in the modifiers field represent the values of these
modifier keys at the time the event occurred, not the time you decided
to process the event.

HEADER:
event.h

MEMBERS:
EVT_LEFTBUT     - Set if left mouse button was down
EVT_RIGHTBUT    - Set if right mouse button was down
EVT_MIDDLEBUT   - Set if the middle button was down
EVT_RIGHTSHIFT  - Set if right shift was down
EVT_LEFTSHIFT   - Set if left shift was down
EVT_RIGHTCTRL   - Set if right ctrl key was down
EVT_RIGHTALT    - Set if right alt key was down
EVT_LEFTCTRL    - Set if left ctrl key was down
EVT_LEFTALT     - Set if left alt key was down
EVT_SHIFTKEY    - Mask for any shift key down
EVT_CTRLSTATE   - Set if ctrl key was down
EVT_ALTSTATE    - Set if alt key was down
EVT_CAPSLOCK    - Caps lock is active
EVT_NUMLOCK     - Num lock is active
EVT_SCROLLLOCK  - Scroll lock is active
****************************************************************************/
typedef enum {
    EVT_LEFTBUT     = 0x00000001,
    EVT_RIGHTBUT    = 0x00000002,
    EVT_MIDDLEBUT   = 0x00000004,
    EVT_RIGHTSHIFT  = 0x00000008,
    EVT_LEFTSHIFT   = 0x00000010,
    EVT_RIGHTCTRL   = 0x00000020,
    EVT_RIGHTALT    = 0x00000040,
    EVT_LEFTCTRL    = 0x00000080,
    EVT_LEFTALT     = 0x00000100,
    EVT_SHIFTKEY    = 0x00000018,
    EVT_CTRLSTATE   = 0x000000A0,
    EVT_ALTSTATE    = 0x00000140,
    EVT_SCROLLLOCK  = 0x00000200,
    EVT_NUMLOCK     = 0x00000400,
    EVT_CAPSLOCK    = 0x00000800
    } EVT_eventModMaskType;

/****************************************************************************
REMARKS:
Defines the event codes returned in the event_t structures what field. Note
that these are defined as a set of mutually exlusive bit fields, so you
can test for multiple event types using the combined event masks defined
in the EVT_eventMaskType enumeration.

HEADER:
event.h

MEMBERS:
EVT_NULLEVT     - A null event
EVT_KEYDOWN     - Key down event
EVT_KEYREPEAT   - Key repeat event
EVT_KEYUP       - Key up event
EVT_MOUSEDOWN   - Mouse down event
EVT_MOUSEAUTO   - Mouse down autorepeat event
EVT_MOUSEUP     - Mouse up event
EVT_MOUSEMOVE   - Mouse movement event
EVT_JOYCLICK    - Joystick button state change event
EVT_JOYMOVE     - Joystick movement event
EVT_USEREVT     - First user event
****************************************************************************/
typedef enum {
    EVT_NULLEVT     = 0x00000000,
    EVT_KEYDOWN     = 0x00000001,
    EVT_KEYREPEAT   = 0x00000002,
    EVT_KEYUP       = 0x00000004,
    EVT_MOUSEDOWN   = 0x00000008,
    EVT_MOUSEAUTO   = 0x00000010,
    EVT_MOUSEUP     = 0x00000020,
    EVT_MOUSEMOVE   = 0x00000040,
    EVT_JOYCLICK    = 0x00000080,
    EVT_JOYMOVE     = 0x00000100,
    EVT_USEREVT     = 0x00000200
    } EVT_eventType;

/****************************************************************************
REMARKS:
Defines the event code masks you can use to test for multiple types of
events, since the event codes are mutually exlusive bit fields.

HEADER:
event.h

MEMBERS:
EVT_KEYEVT      - Mask for any key event
EVT_MOUSEEVT    - Mask for any mouse event
EVT_MOUSECLICK  - Mask for any mouse click event
EVT_JOYEVT      - Mask for any joystick event
EVT_EVERYEVT    - Mask for any event
****************************************************************************/
typedef enum {
    EVT_KEYEVT      = (EVT_KEYDOWN | EVT_KEYREPEAT | EVT_KEYUP),
    EVT_MOUSEEVT    = (EVT_MOUSEDOWN | EVT_MOUSEAUTO | EVT_MOUSEUP | EVT_MOUSEMOVE),
    EVT_MOUSECLICK  = (EVT_MOUSEDOWN | EVT_MOUSEUP),
    EVT_JOYEVT      = (EVT_JOYCLICK | EVT_JOYMOVE),
    EVT_EVERYEVT    = 0x7FFFFFFF
    } EVT_eventMaskType;

/****************************************************************************
REMARKS:
Structure describing the information contained in an event extracted from
the event queue.

HEADER:
event.h

MEMBERS:
which       - Window identifier for message for use by high level window manager
	      code (i.e. MegaVision GUI or Windows API).
what        - Type of event that occurred. Will be one of the values defined by
	      the EVT_eventType enumeration.
when        - Time that the event occurred in milliseconds since startup
where_x     - X coordinate of the mouse cursor location at the time of the event
	      (in screen coordinates). For joystick events this represents
	      the position of the first joystick X axis.
where_y     - Y coordinate of the mouse cursor location at the time of the event
	      (in screen coordinates). For joystick events this represents
	      the position of the first joystick Y axis.
relative_x  - Relative movement of the mouse cursor in the X direction (in
	      units of mickeys, or 1/200th of an inch). For joystick events
	      this represents the position of the second joystick X axis.
relative_y  - Relative movement of the mouse cursor in the Y direction (in
	      units of mickeys, or 1/200th of an inch). For joystick events
	      this represents the position of the second joystick Y axis.
message     - Event specific message for the event. For use events this can be
	      any user specific information. For keyboard events this contains
	      the ASCII code in bits 0-7, the keyboard scan code in bits 8-15 and
	      the character repeat count in bits 16-30. You can use the
	      EVT_asciiCode, EVT_scanCode and EVT_repeatCount macros to extract
	      this information from the message field. For mouse events this
	      contains information about which button was pressed, and will be a
	      combination of the flags defined by the EVT_eventMouseMaskType
	      enumeration. For joystick events, this conatins information
	      about which buttons were pressed, and will be a combination of
	      the flags defined by the EVT_eventJoyMaskType enumeration.
modifiers   - Contains additional information about the state of the keyboard
	      shift modifiers (Ctrl, Alt and Shift keys) when the event
	      occurred. For mouse events it will also contain the state of
	      the mouse buttons. Will be a combination of the values defined
	      by the EVT_eventModMaskType enumeration.
next        - Internal use; do not use.
prev        - Internal use; do not use.
****************************************************************************/
typedef struct {
    ulong       which;
    ulong       what;
    ulong       when;
    int         where_x;
    int         where_y;
    int         relative_x;
    int         relative_y;
    ulong       message;
    ulong       modifiers;
    int         next;
    int         prev;
    } event_t;

/****************************************************************************
REMARKS:
Structure describing an entry in the code page table. A table of translation
codes for scan codes to ASCII codes is provided in this table to be used
by the keyboard event libraries. On some OS'es the keyboard translation is
handled by the OS, but for DOS and embedded systems you must register a
different code page translation table if you want to support keyboards
other than the US English keyboard (the default).

NOTE:   Entries in code page tables *must* be in ascending order for the
	scan codes as we do a binary search on the tables for the ASCII
	code equivalents.

HEADER:
event.h

MEMBERS:
scanCode    - Scan code to translate (really the virtual scan code).
asciiCode   - ASCII code for this scan code.
****************************************************************************/
typedef struct {
    uchar       scanCode;
    uchar       asciiCode;
    } codepage_entry_t;

/****************************************************************************
REMARKS:
Structure describing a complete code page translation table. The table
contains translation tables for normal keys, shifted keys and ctrl keys.
The Ctrl key always has precedence over the shift table, and the shift
table is used when the shift key is down or the CAPSLOCK key is down.

HEADER:
event.h

MEMBERS:
name            - Name of the code page table (ie: "US English")
normal          - Code page for translating normal keys
normalLen       - Length of normal translation table
caps            - Code page for translating keys when CAPSLOCK is down
capsLen         - Length of CAPSLOCK translation table
shift           - Code page for shifted keys (ie: shift key is held down)
shiftLen        - Length of shifted translation table
shiftCaps       - Code page for shifted keys when CAPSLOCK is down
shiftCapsLen    - Length of shifted CAPSLOCK translation table
ctrl            - Code page for ctrl'ed keys (ie: ctrl key is held down)
ctrlLen         - Length of ctrl'ed translation table
numPad          - Code page for NUMLOCK'ed keypad keys
numPadLen       - Length of NUMLOCK'ed translation table
****************************************************************************/
typedef struct {
    char                name[20];
    codepage_entry_t    *normal;
    int                 normalLen;
    codepage_entry_t    *caps;
    int                 capsLen;
    codepage_entry_t    *shift;
    int                 shiftLen;
    codepage_entry_t    *shiftCaps;
    int                 shiftCapsLen;
    codepage_entry_t    *ctrl;
    int                 ctrlLen;
    codepage_entry_t    *numPad;
    int                 numPadLen;
    } codepage_t;

/* {secret} */
typedef ibool (EVTAPIP _EVT_userEventFilter)(event_t *evt);
/* {secret} */
typedef void (EVTAPIP _EVT_mouseMoveHandler)(int x,int y);
/* {secret} */
typedef void (EVTAPIP _EVT_heartBeatCallback)(void *params);

/* Macro to find the size of a static array */

#define EVT_ARR_SIZE(a)         (sizeof(a)/sizeof((a)[0]))

#pragma pack()

/*--------------------------- Global variables ----------------------------*/

#ifdef  __cplusplus
extern "C" {            /* Use "C" linkage when in C++ mode */
#endif

/* Standard code page tables */

extern codepage_t _CP_US_English;

/*------------------------- Function Prototypes ---------------------------*/

/* Public API functions for user applications */

ibool   EVTAPI EVT_getNext(event_t *evt,ulong mask);
ibool   EVTAPI EVT_peekNext(event_t *evt,ulong mask);
ibool   EVTAPI EVT_post(ulong which,ulong what,ulong message,ulong modifiers);
void    EVTAPI EVT_flush(ulong mask);
void    EVTAPI EVT_halt(event_t *evt,ulong mask);
ibool   EVTAPI EVT_isKeyDown(uchar scanCode);
void    EVTAPI EVT_setMousePos(int x,int y);
void    EVTAPI EVT_getMousePos(int *x,int *y);

/* Function to enable/disable updating of keyboard LED status indicators */

void    EVTAPI EVT_allowLEDS(ibool enable);

/* Function to install a custom keyboard code page. Default is US English */

codepage_t *EVTAPI EVT_getCodePage(void);
void    EVTAPI EVT_setCodePage(codepage_t *page);

/* Functions for fine grained joystick calibration */

void    EVTAPI EVT_pollJoystick(void);
int     EVTAPI EVT_joyIsPresent(void);
void    EVTAPI EVT_joySetUpperLeft(void);
void    EVTAPI EVT_joySetLowerRight(void);
void    EVTAPI EVT_joySetCenter(void);

/* Install user supplied event filter callback */

void    EVTAPI EVT_setUserEventFilter(_EVT_userEventFilter filter);

/* Install user supplied event heartbeat callback function */

void    EVTAPI EVT_setHeartBeatCallback(_EVT_heartBeatCallback callback,void *params);
void    EVTAPI EVT_getHeartBeatCallback(_EVT_heartBeatCallback *callback,void **params);

/* Internal functions to initialise and kill the event manager. MGL
 * applications should never call these functions directly as the MGL
 * libraries do it for you.
 */

/* {secret} */
void    EVTAPI EVT_init(_EVT_mouseMoveHandler mouseMove);
/* {secret} */
void    EVTAPI EVT_setMouseRange(int xRes,int yRes);
/* {secret} */
void    EVTAPI EVT_suspend(void);
/* {secret} */
void    EVTAPI EVT_resume(void);
/* {secret} */
void    EVTAPI EVT_exit(void);

#ifdef  __cplusplus
}                       /* End of "C" linkage for C++   */
#endif  /* __cplusplus */

#endif  /* __EVENT_H */
