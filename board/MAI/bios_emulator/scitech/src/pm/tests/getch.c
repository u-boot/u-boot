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
* Description:  Test program to test out the cross platform event handling
*               library.
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "pmapi.h"
#include "event.h"

/* Translation table for key codes */

typedef struct {
    int     code;
    char    *name;
    } KeyEntry;

KeyEntry ASCIICodes[] = {
    {ASCII_ctrlA            ,"ASCII_ctrlA"},
    {ASCII_ctrlB            ,"ASCII_ctrlB"},
    {ASCII_ctrlC            ,"ASCII_ctrlC"},
    {ASCII_ctrlD            ,"ASCII_ctrlD"},
    {ASCII_ctrlE            ,"ASCII_ctrlE"},
    {ASCII_ctrlF            ,"ASCII_ctrlF"},
    {ASCII_ctrlG            ,"ASCII_ctrlG"},
    {ASCII_backspace        ,"ASCII_backspace"},
    {ASCII_ctrlH            ,"ASCII_ctrlH"},
    {ASCII_tab              ,"ASCII_tab"},
    {ASCII_ctrlI            ,"ASCII_ctrlI"},
    {ASCII_ctrlJ            ,"ASCII_ctrlJ"},
    {ASCII_ctrlK            ,"ASCII_ctrlK"},
    {ASCII_ctrlL            ,"ASCII_ctrlL"},
    {ASCII_enter            ,"ASCII_enter"},
    {ASCII_ctrlM            ,"ASCII_ctrlM"},
    {ASCII_ctrlN            ,"ASCII_ctrlN"},
    {ASCII_ctrlO            ,"ASCII_ctrlO"},
    {ASCII_ctrlP            ,"ASCII_ctrlP"},
    {ASCII_ctrlQ            ,"ASCII_ctrlQ"},
    {ASCII_ctrlR            ,"ASCII_ctrlR"},
    {ASCII_ctrlS            ,"ASCII_ctrlS"},
    {ASCII_ctrlT            ,"ASCII_ctrlT"},
    {ASCII_ctrlU            ,"ASCII_ctrlU"},
    {ASCII_ctrlV            ,"ASCII_ctrlV"},
    {ASCII_ctrlW            ,"ASCII_ctrlW"},
    {ASCII_ctrlX            ,"ASCII_ctrlX"},
    {ASCII_ctrlY            ,"ASCII_ctrlY"},
    {ASCII_ctrlZ            ,"ASCII_ctrlZ"},
    {ASCII_esc              ,"ASCII_esc"},
    {ASCII_space            ,"ASCII_space"},
    {ASCII_exclamation      ,"ASCII_exclamation"},
    {ASCII_quote            ,"ASCII_quote"},
    {ASCII_pound            ,"ASCII_pound"},
    {ASCII_dollar           ,"ASCII_dollar"},
    {ASCII_percent          ,"ASCII_percent"},
    {ASCII_ampersand        ,"ASCII_ampersand"},
    {ASCII_apostrophe       ,"ASCII_apostrophe"},
    {ASCII_leftBrace        ,"ASCII_leftBrace"},
    {ASCII_rightBrace       ,"ASCII_rightBrace"},
    {ASCII_times            ,"ASCII_times"},
    {ASCII_plus             ,"ASCII_plus"},
    {ASCII_comma            ,"ASCII_comma"},
    {ASCII_minus            ,"ASCII_minus"},
    {ASCII_period           ,"ASCII_period"},
    {ASCII_divide           ,"ASCII_divide"},
    {ASCII_0                ,"ASCII_0"},
    {ASCII_1                ,"ASCII_1"},
    {ASCII_2                ,"ASCII_2"},
    {ASCII_3                ,"ASCII_3"},
    {ASCII_4                ,"ASCII_4"},
    {ASCII_5                ,"ASCII_5"},
    {ASCII_6                ,"ASCII_6"},
    {ASCII_7                ,"ASCII_7"},
    {ASCII_8                ,"ASCII_8"},
    {ASCII_9                ,"ASCII_9"},
    {ASCII_colon            ,"ASCII_colon"},
    {ASCII_semicolon        ,"ASCII_semicolon"},
    {ASCII_lessThan         ,"ASCII_lessThan"},
    {ASCII_equals           ,"ASCII_equals"},
    {ASCII_greaterThan      ,"ASCII_greaterThan"},
    {ASCII_question         ,"ASCII_question"},
    {ASCII_at               ,"ASCII_at"},
    {ASCII_A                ,"ASCII_A"},
    {ASCII_B                ,"ASCII_B"},
    {ASCII_C                ,"ASCII_C"},
    {ASCII_D                ,"ASCII_D"},
    {ASCII_E                ,"ASCII_E"},
    {ASCII_F                ,"ASCII_F"},
    {ASCII_G                ,"ASCII_G"},
    {ASCII_H                ,"ASCII_H"},
    {ASCII_I                ,"ASCII_I"},
    {ASCII_J                ,"ASCII_J"},
    {ASCII_K                ,"ASCII_K"},
    {ASCII_L                ,"ASCII_L"},
    {ASCII_M                ,"ASCII_M"},
    {ASCII_N                ,"ASCII_N"},
    {ASCII_O                ,"ASCII_O"},
    {ASCII_P                ,"ASCII_P"},
    {ASCII_Q                ,"ASCII_Q"},
    {ASCII_R                ,"ASCII_R"},
    {ASCII_S                ,"ASCII_S"},
    {ASCII_T                ,"ASCII_T"},
    {ASCII_U                ,"ASCII_U"},
    {ASCII_V                ,"ASCII_V"},
    {ASCII_W                ,"ASCII_W"},
    {ASCII_X                ,"ASCII_X"},
    {ASCII_Y                ,"ASCII_Y"},
    {ASCII_Z                ,"ASCII_Z"},
    {ASCII_leftSquareBrace  ,"ASCII_leftSquareBrace"},
    {ASCII_backSlash        ,"ASCII_backSlash"},
    {ASCII_rightSquareBrace ,"ASCII_rightSquareBrace"},
    {ASCII_caret            ,"ASCII_caret"},
    {ASCII_underscore       ,"ASCII_underscore"},
    {ASCII_leftApostrophe   ,"ASCII_leftApostrophe"},
    {ASCII_a                ,"ASCII_a"},
    {ASCII_b                ,"ASCII_b"},
    {ASCII_c                ,"ASCII_c"},
    {ASCII_d                ,"ASCII_d"},
    {ASCII_e                ,"ASCII_e"},
    {ASCII_f                ,"ASCII_f"},
    {ASCII_g                ,"ASCII_g"},
    {ASCII_h                ,"ASCII_h"},
    {ASCII_i                ,"ASCII_i"},
    {ASCII_j                ,"ASCII_j"},
    {ASCII_k                ,"ASCII_k"},
    {ASCII_l                ,"ASCII_l"},
    {ASCII_m                ,"ASCII_m"},
    {ASCII_n                ,"ASCII_n"},
    {ASCII_o                ,"ASCII_o"},
    {ASCII_p                ,"ASCII_p"},
    {ASCII_q                ,"ASCII_q"},
    {ASCII_r                ,"ASCII_r"},
    {ASCII_s                ,"ASCII_s"},
    {ASCII_t                ,"ASCII_t"},
    {ASCII_u                ,"ASCII_u"},
    {ASCII_v                ,"ASCII_v"},
    {ASCII_w                ,"ASCII_w"},
    {ASCII_x                ,"ASCII_x"},
    {ASCII_y                ,"ASCII_y"},
    {ASCII_z                ,"ASCII_z"},
    {ASCII_leftCurlyBrace   ,"ASCII_leftCurlyBrace"},
    {ASCII_verticalBar      ,"ASCII_verticalBar"},
    {ASCII_rightCurlyBrace  ,"ASCII_rightCurlyBrace"},
    {ASCII_tilde            ,"ASCII_tilde"},
    {0                      ,"ASCII_unknown"},
    };

KeyEntry ScanCodes[] = {
    {KB_padEnter            ,"KB_padEnter"},
    {KB_padMinus            ,"KB_padMinus"},
    {KB_padPlus             ,"KB_padPlus"},
    {KB_padTimes            ,"KB_padTimes"},
    {KB_padDivide           ,"KB_padDivide"},
    {KB_padLeft             ,"KB_padLeft"},
    {KB_padRight            ,"KB_padRight"},
    {KB_padUp               ,"KB_padUp"},
    {KB_padDown             ,"KB_padDown"},
    {KB_padInsert           ,"KB_padInsert"},
    {KB_padDelete           ,"KB_padDelete"},
    {KB_padHome             ,"KB_padHome"},
    {KB_padEnd              ,"KB_padEnd"},
    {KB_padPageUp           ,"KB_padPageUp"},
    {KB_padPageDown         ,"KB_padPageDown"},
    {KB_padCenter           ,"KB_padCenter"},
    {KB_F1                  ,"KB_F1"},
    {KB_F2                  ,"KB_F2"},
    {KB_F3                  ,"KB_F3"},
    {KB_F4                  ,"KB_F4"},
    {KB_F5                  ,"KB_F5"},
    {KB_F6                  ,"KB_F6"},
    {KB_F7                  ,"KB_F7"},
    {KB_F8                  ,"KB_F8"},
    {KB_F9                  ,"KB_F9"},
    {KB_F10                 ,"KB_F10"},
    {KB_F11                 ,"KB_F11"},
    {KB_F12                 ,"KB_F12"},
    {KB_left                ,"KB_left"},
    {KB_right               ,"KB_right"},
    {KB_up                  ,"KB_up"},
    {KB_down                ,"KB_down"},
    {KB_insert              ,"KB_insert"},
    {KB_delete              ,"KB_delete"},
    {KB_home                ,"KB_home"},
    {KB_end                 ,"KB_end"},
    {KB_pageUp              ,"KB_pageUp"},
    {KB_pageDown            ,"KB_pageDown"},
    {KB_capsLock            ,"KB_capsLock"},
    {KB_numLock             ,"KB_numLock"},
    {KB_scrollLock          ,"KB_scrollLock"},
    {KB_leftShift           ,"KB_leftShift"},
    {KB_rightShift          ,"KB_rightShift"},
    {KB_leftCtrl            ,"KB_leftCtrl"},
    {KB_rightCtrl           ,"KB_rightCtrl"},
    {KB_leftAlt             ,"KB_leftAlt"},
    {KB_rightAlt            ,"KB_rightAlt"},
    {KB_leftWindows         ,"KB_leftWindows"},
    {KB_rightWindows        ,"KB_rightWindows"},
    {KB_menu                ,"KB_menu"},
    {KB_sysReq              ,"KB_sysReq"},
    {KB_esc                 ,"KB_esc"},
    {KB_1                   ,"KB_1"},
    {KB_2                   ,"KB_2"},
    {KB_3                   ,"KB_3"},
    {KB_4                   ,"KB_4"},
    {KB_5                   ,"KB_5"},
    {KB_6                   ,"KB_6"},
    {KB_7                   ,"KB_7"},
    {KB_8                   ,"KB_8"},
    {KB_9                   ,"KB_9"},
    {KB_0                   ,"KB_0"},
    {KB_minus               ,"KB_minus"},
    {KB_equals              ,"KB_equals"},
    {KB_backSlash           ,"KB_backSlash"},
    {KB_backspace           ,"KB_backspace"},
    {KB_tab                 ,"KB_tab"},
    {KB_Q                   ,"KB_Q"},
    {KB_W                   ,"KB_W"},
    {KB_E                   ,"KB_E"},
    {KB_R                   ,"KB_R"},
    {KB_T                   ,"KB_T"},
    {KB_Y                   ,"KB_Y"},
    {KB_U                   ,"KB_U"},
    {KB_I                   ,"KB_I"},
    {KB_O                   ,"KB_O"},
    {KB_P                   ,"KB_P"},
    {KB_leftSquareBrace     ,"KB_leftSquareBrace"},
    {KB_rightSquareBrace    ,"KB_rightSquareBrace"},
    {KB_enter               ,"KB_enter"},
    {KB_A                   ,"KB_A"},
    {KB_S                   ,"KB_S"},
    {KB_D                   ,"KB_D"},
    {KB_F                   ,"KB_F"},
    {KB_G                   ,"KB_G"},
    {KB_H                   ,"KB_H"},
    {KB_J                   ,"KB_J"},
    {KB_K                   ,"KB_K"},
    {KB_L                   ,"KB_L"},
    {KB_semicolon           ,"KB_semicolon"},
    {KB_apostrophe          ,"KB_apostrophe"},
    {KB_Z                   ,"KB_Z"},
    {KB_X                   ,"KB_X"},
    {KB_C                   ,"KB_C"},
    {KB_V                   ,"KB_V"},
    {KB_B                   ,"KB_B"},
    {KB_N                   ,"KB_N"},
    {KB_M                   ,"KB_M"},
    {KB_comma               ,"KB_comma"},
    {KB_period              ,"KB_period"},
    {KB_divide              ,"KB_divide"},
    {KB_space               ,"KB_space"},
    {KB_tilde               ,"KB_tilde"},
    {0                      ,"KB_unknown"},
    };

/****************************************************************************
PARAMETERS:
x   - X coordinate of the mouse cursor position (screen coordinates)
y   - Y coordinate of the mouse cursor position (screen coordinates)

REMARKS:
This gets called periodically to move the mouse. It will get called when
the mouse may not have actually moved, so check if it has before redrawing
it.
****************************************************************************/
void EVTAPI moveMouse(
    int x,
    int y)
{
}

/****************************************************************************
PARAMETERS:
code    - Code to translate
keys    - Table of translation key values to look up

REMARKS:
Simple function to look up the printable name for the keyboard code.
****************************************************************************/
KeyEntry *FindKey(
    int code,
    KeyEntry *keys)
{
    KeyEntry    *key;

    for (key = keys; key->code != 0; key++) {
	if (key->code == code)
	    break;
	}
    return key;
}

/****************************************************************************
PARAMETERS:
evt - Event to display modifiers for

REMARKS:
Function to display shift modifiers flags
****************************************************************************/
void DisplayModifiers(
    event_t *evt)
{
    if (evt->modifiers & EVT_LEFTBUT)
	printf(", LBUT");
    if (evt->modifiers & EVT_RIGHTBUT)
	printf(", RBUT");
    if (evt->modifiers & EVT_MIDDLEBUT)
	printf(", MBUT");
    if (evt->modifiers & EVT_SHIFTKEY) {
	if (evt->modifiers & EVT_LEFTSHIFT)
	    printf(", LSHIFT");
	if (evt->modifiers & EVT_RIGHTSHIFT)
	    printf(", RSHIFT");
	}
    if (evt->modifiers & EVT_CTRLSTATE) {
	if (evt->modifiers & EVT_LEFTCTRL)
	    printf(", LCTRL");
	if (evt->modifiers & EVT_RIGHTCTRL)
	    printf(", RCTRL");
	}
    if (evt->modifiers & EVT_ALTSTATE) {
	if (evt->modifiers & EVT_LEFTALT)
	    printf(", LALT");
	if (evt->modifiers & EVT_RIGHTALT)
	    printf(", RALT");
	}
}

/****************************************************************************
PARAMETERS:
msg - Message to display for type of event
evt - Event to display

REMARKS:
Function to display the status of the keyboard event to the screen.
****************************************************************************/
void DisplayKey(
    char *msg,
    event_t *evt)
{
    KeyEntry    *ascii,*scan;
    char        ch = EVT_asciiCode(evt->message);

    ascii = FindKey(ch,ASCIICodes);
    scan = FindKey(EVT_scanCode(evt->message),ScanCodes);
    printf("%s: 0x%04X -> %s, %s, '%c'",
	msg, (int)evt->message & 0xFFFF, scan->name, ascii->name, isprint(ch) ? ch : ' ');
    DisplayModifiers(evt);
    printf("\n");
}

/****************************************************************************
PARAMETERS:
msg - Message to display for type of event
evt - Event to display

REMARKS:
Function to display the status of the mouse event to the screen.
****************************************************************************/
void DisplayMouse(
    char *msg,
    event_t *evt)
{
    printf("%s: ", msg);
    if (evt->message & EVT_LEFTBMASK)
	printf("LEFT ");
    if (evt->message & EVT_RIGHTBMASK)
	printf("RIGHT ");
    if (evt->message & EVT_MIDDLEBMASK)
	printf("MIDDLE ");
    printf("abs(%d,%d), rel(%d,%d)", evt->where_x, evt->where_y, evt->relative_x, evt->relative_y);
    DisplayModifiers(evt);
    if (evt->message & EVT_DBLCLICK)
	printf(", DBLCLICK");
    printf("\n");
}

/****************************************************************************
PARAMETERS:
msg - Message to display for type of event
evt - Event to display

REMARKS:
Function to display the status of the joystick event to the screen.
****************************************************************************/
void DisplayJoy(
    char *msg,
    event_t *evt)
{
    printf("%s: Joy1(%4d,%4d,%c%c), Joy2(%4d,%4d,%c%c)\n", msg,
	evt->where_x,evt->where_y,
	(evt->message & EVT_JOY1_BUTTONA) ? 'A' : 'a',
	(evt->message & EVT_JOY1_BUTTONB) ? 'B' : 'b',
	evt->relative_x,evt->relative_y,
	(evt->message & EVT_JOY2_BUTTONA) ? 'A' : 'a',
	(evt->message & EVT_JOY2_BUTTONB) ? 'B' : 'b');
}

/****************************************************************************
REMARKS:
Joystick calibration routine
****************************************************************************/
void CalibrateJoy(void)
{
  event_t evt;
  if(EVT_joyIsPresent()){
    printf("Joystick Calibration\nMove the joystick to the upper left corner and press any button.\n");
    EVT_halt(&evt, EVT_JOYCLICK);
    EVT_halt(&evt, EVT_JOYCLICK);
    EVT_joySetUpperLeft();
    printf("Move the joystick to the lower right corner and press any button.\n");
    EVT_halt(&evt, EVT_JOYCLICK);
    EVT_halt(&evt, EVT_JOYCLICK);
    EVT_joySetLowerRight();
    printf("Move the joystick to center position and press any button.\n");
    EVT_halt(&evt, EVT_JOYCLICK);
    EVT_halt(&evt, EVT_JOYCLICK);
    EVT_joySetCenter();
    printf("Joystick calibrated\n");
  }
}

/****************************************************************************
REMARKS:
Main program entry point
****************************************************************************/
int main(void)
{
    event_t     evt;
    ibool       done = false;
    PM_HWND     hwndConsole;

    hwndConsole = PM_openConsole(0,0,0,0,0,true);
    EVT_init(&moveMouse);
    EVT_setMouseRange(1024,768);
    CalibrateJoy();
    do {
	EVT_pollJoystick();
	if (EVT_getNext(&evt,EVT_EVERYEVT)) {
	    switch (evt.what) {
		case EVT_KEYDOWN:
		    DisplayKey("EVT_KEYDOWN  ", &evt);
		    if (EVT_scanCode(evt.message) == KB_esc)
			done = true;
		    break;
		case EVT_KEYREPEAT:
		    DisplayKey("EVT_KEYREPEAT", &evt);
		    break;
		case EVT_KEYUP:
		    DisplayKey("EVT_KEYUP    ", &evt);
		    break;
		case EVT_MOUSEDOWN:
		    DisplayMouse("EVT_MOUSEDOWN", &evt);
		    break;
		case EVT_MOUSEAUTO:
		    DisplayMouse("EVT_MOUSEAUTO", &evt);
		    break;
		case EVT_MOUSEUP:
		    DisplayMouse("EVT_MOUSEUP  ", &evt);
		    break;
		case EVT_MOUSEMOVE:
		    DisplayMouse("EVT_MOUSEMOVE", &evt);
		    break;
		case EVT_JOYCLICK:
		    DisplayJoy("EVT_JOYCLICK ", &evt);
		    break;
		case EVT_JOYMOVE:
		    DisplayJoy("EVT_JOYMOVE  ", &evt);
		    break;
		}
	    }
	} while (!done);
    EVT_exit();
    PM_closeConsole(hwndConsole);
    return 0;
}
