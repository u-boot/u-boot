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
* Description:  Direct keyboard event handling module. This module contains
*               code to process raw scan code information, convert it to
*               virtual scan codes and do code page translation to ASCII
*               for different international keyboard layouts.
*
****************************************************************************/

/*---------------------------- Implementation -----------------------------*/

/****************************************************************************
PARAMETERS:
scanCode    - Keyboard scan code to translate
table       - Code page table to search
count       - Number of entries in the code page table

REMARKS:
This function translates the scan codes from keyboard scan codes to ASCII
codes using a binary search on the code page table.
****************************************************************************/
static uchar translateScan(
    uchar scanCode,
    codepage_entry_t *table,
    int count)
{
    codepage_entry_t    *test;
    int                 n,pivot,val;

    for (n = count; n > 0; ) {
	pivot = n >> 1;
	test = table + pivot;
	val = scanCode - test->scanCode;
	if (val < 0)
	    n = pivot;
	else if (val == 0)
	    return test->asciiCode;
	else {
	    table = test + 1;
	    n -= pivot + 1;
	    }
	}
    return 0;
}

/****************************************************************************
REMARKS:
This macro/function is used to converts the scan codes reported by the
keyboard to our event libraries normalised format. We only have one scan
code for the 'A' key, and use shift modifiers to determine if it is a
Ctrl-F1, Alt-F1 etc. The raw scan codes from the keyboard work this way,
but the OS gives us 'cooked' scan codes, we have to translate them back
to the raw format.
{secret}
****************************************************************************/
void _EVT_maskKeyCode(
    event_t *evt)
{
    int ascii,scan = EVT_scanCode(evt->message);

    evt->message &= ~0xFF;
    if (evt->modifiers & EVT_NUMLOCK) {
	if ((ascii = translateScan(scan,EVT.codePage->numPad,EVT.codePage->numPadLen)) != 0) {
	    evt->message |= ascii;
	    return;
	    }
	}
    if (evt->modifiers & EVT_CTRLSTATE) {
	evt->message |= translateScan(scan,EVT.codePage->ctrl,EVT.codePage->ctrlLen);
	return;
	}
    if (evt->modifiers & EVT_CAPSLOCK) {
	if (evt->modifiers & EVT_SHIFTKEY) {
	    if ((ascii = translateScan(scan,EVT.codePage->shiftCaps,EVT.codePage->shiftCapsLen)) != 0) {
		evt->message |= ascii;
		return;
		}
	    }
	else {
	    if ((ascii = translateScan(scan,EVT.codePage->caps,EVT.codePage->capsLen)) != 0) {
		evt->message |= ascii;
		return;
		}
	    }
	}
    if (evt->modifiers & EVT_SHIFTKEY) {
	if ((ascii = translateScan(scan,EVT.codePage->shift,EVT.codePage->shiftLen)) != 0) {
	    evt->message |= ascii;
	    return;
	    }
	}
    evt->message |= translateScan(scan,EVT.codePage->normal,EVT.codePage->normalLen);
}

/****************************************************************************
REMARKS:
Returns true if the key with the specified scan code is being held down.
****************************************************************************/
static ibool _EVT_isKeyDown(
    uchar scanCode)
{
    if (scanCode > 0x7F)
	return false;
    else
	return EVT.keyTable[scanCode] != 0;
}

/****************************************************************************
PARAMETERS:
what        - Event code
message     - Event message (ASCII code and scan code)

REMARKS:
Adds a new keyboard event to the event queue. This routine is called from
within the keyboard interrupt subroutine!

NOTE:   Interrupts are OFF when this routine is called by the keyboard ISR,
	and we leave them OFF the entire time.
****************************************************************************/
static void addKeyEvent(
    uint what,
    uint message)
{
    event_t evt;

    if (EVT.count < EVENTQSIZE) {
	/* Save information in event record */
	evt.when = _EVT_getTicks();
	evt.what = what;
	evt.message = message | 0x10000UL;
	evt.where_x = 0;
	evt.where_y = 0;
	evt.relative_x = 0;
	evt.relative_y = 0;
	evt.modifiers = EVT.keyModifiers;
	if (evt.what == EVT_KEYREPEAT) {
	    if (EVT.oldKey != -1)
		EVT.evtq[EVT.oldKey].message += 0x10000UL;
	    else {
		EVT.oldKey = EVT.freeHead;
		addEvent(&evt);         /* Add to tail of event queue   */
		}
	    }
	else {
#ifdef __QNX__
	    _EVT_maskKeyCode(&evt);
#endif
	    addEvent(&evt);             /* Add to tail of event queue   */
	    }
	EVT.oldMove = -1;
	}
}

/****************************************************************************
REMARKS:
This function waits for the keyboard controller to set the ready-for-write
bit.
****************************************************************************/
static int kbWaitForWriteReady(void)
{
    int timeout = 8192;
    while ((timeout > 0) && (PM_inpb(0x64) & 0x02))
	timeout--;
    return (timeout > 0);
}

/****************************************************************************
REMARKS:
This function waits for the keyboard controller to set the ready-for-read
bit.
****************************************************************************/
static int kbWaitForReadReady(void)
{
    int timeout = 8192;
    while ((timeout > 0) && (!(PM_inpb(0x64) & 0x01)))
	timeout--;
    return (timeout > 0);
}

/****************************************************************************
PARAMETERS:
data    - Data to send to the keyboard

REMARKS:
This function sends a data byte to the keyboard controller.
****************************************************************************/
static int kbSendData(
    uchar data)
{
    int resends = 4;
    int timeout, temp;

    do {
	if (!kbWaitForWriteReady())
	    return 0;
	PM_outpb(0x60,data);
	timeout = 8192;
	while (--timeout > 0) {
	    if (!kbWaitForReadReady())
		return 0;
	    temp = PM_inpb(0x60);
	    if (temp == 0xFA)
		return 1;
	    if (temp == 0xFE)
		break;
	    }
	} while ((resends-- > 0) && (timeout > 0));
    return 0;
}

/****************************************************************************
PARAMETERS:
modifiers   - Keyboard modifier flags

REMARKS:
This function re-programs the LED's on the keyboard to the values stored
in the passed in modifier flags. If the 'allowLEDS' flag is false, this
function does nothing.
****************************************************************************/
static void setLEDS(
    uint modifiers)
{
    if (EVT.allowLEDS) {
	if (!kbSendData(0xED) || !kbSendData((modifiers>>9) & 7)) {
	    kbSendData(0xF4);
	    }
	}
}

/****************************************************************************
REMARKS:
Function to process raw scan codes read from the keyboard controller.

NOTE:   Interrupts are OFF when this routine is called by the keyboard ISR,
	and we leave them OFF the entire time.
{secret}
****************************************************************************/
void processRawScanCode(
    int scan)
{
    static int  pauseLoop = 0;
    static int  extended = 0;
    int         what;

    if (pauseLoop) {
	/* Skip scan codes until the pause key sequence has been read */
	pauseLoop--;
	}
    else if (scan == 0xE0) {
	/* This signals the start of an extended scan code sequence */
	extended = 1;
	}
    else if (scan == 0xE1) {
	/* The Pause key sends a strange scan code sequence, which is:
	 *
	 *  E1 1D 52 E1 9D D2
	 *
	 * However there is never any release code nor any auto-repeat for
	 * this key. For this reason we simply ignore the key and skip the
	 * next 5 scan codes read from the keyboard.
	 */
	pauseLoop = 5;
	}
    else {
	/* Process the scan code normally (it may be an extended code
	 * however!). Bit 7 means key was released, and bits 0-6 are the
	 * scan code.
	 */
	what = (scan & 0x80) ? EVT_KEYUP : EVT_KEYDOWN;
	scan &= 0x7F;
	if (extended) {
	    extended = 0;
	    if (scan == 0x2A || scan == 0x36) {
		/* Ignore these extended scan code sequences. These are
		 * used by the keyboard controller to wrap around certain
		 * key sequences for the keypad (and when NUMLOCK is down
		 * internally).
		 */
		return;
		}

	    /* Convert extended codes for key sequences that we map to
	     * virtual scan codes so the user can detect them in their
	     * code.
	     */
	    switch (scan) {
		case KB_leftCtrl:   scan = KB_rightCtrl;    break;
		case KB_leftAlt:    scan = KB_rightAlt;     break;
		case KB_divide:     scan = KB_padDivide;    break;
		case KB_enter:      scan = KB_padEnter;     break;
		case KB_padTimes:   scan = KB_sysReq;       break;
		}
	    }
	else {
	    /* Convert regular scan codes for key sequences that we map to
	     * virtual scan codes so the user can detect them in their
	     * code.
	     */
	    switch (scan) {
		case KB_left:       scan = KB_padLeft;      break;
		case KB_right:      scan = KB_padRight;     break;
		case KB_up:         scan = KB_padUp;        break;
		case KB_down:       scan = KB_padDown;      break;
		case KB_insert:     scan = KB_padInsert;    break;
		case KB_delete:     scan = KB_padDelete;    break;
		case KB_home:       scan = KB_padHome;      break;
		case KB_end:        scan = KB_padEnd;       break;
		case KB_pageUp:     scan = KB_padPageUp;    break;
		case KB_pageDown:   scan = KB_padPageDown;  break;
		}
	    }

	/* Determine if the key is an UP, DOWN or REPEAT and maintain the
	 * up/down status of all keys in our global key array.
	 */
	if (what == EVT_KEYDOWN) {
	    if (EVT.keyTable[scan])
		what = EVT_KEYREPEAT;
	    else
		EVT.keyTable[scan] = scan;
	    }
	else {
	    EVT.keyTable[scan] = 0;
	    }

	/* Handle shift key modifiers */
	if (what != EVT_KEYREPEAT) {
	    switch (scan) {
		case KB_capsLock:
		    if (what == EVT_KEYDOWN)
			EVT.keyModifiers ^= EVT_CAPSLOCK;
		    setLEDS(EVT.keyModifiers);
		    break;
		case KB_numLock:
		    if (what == EVT_KEYDOWN)
			EVT.keyModifiers ^= EVT_NUMLOCK;
		    setLEDS(EVT.keyModifiers);
		    break;
		case KB_scrollLock:
		    if (what == EVT_KEYDOWN)
			EVT.keyModifiers ^= EVT_SCROLLLOCK;
		    setLEDS(EVT.keyModifiers);
		    break;
		case KB_leftShift:
		    if (what == EVT_KEYUP)
			EVT.keyModifiers &= ~EVT_LEFTSHIFT;
		    else
			EVT.keyModifiers |= EVT_LEFTSHIFT;
		    break;
		case KB_rightShift:
		    if (what == EVT_KEYUP)
			EVT.keyModifiers &= ~EVT_RIGHTSHIFT;
		    else
			EVT.keyModifiers |= EVT_RIGHTSHIFT;
		    break;
		case KB_leftCtrl:
		    if (what == EVT_KEYUP)
			EVT.keyModifiers &= ~EVT_LEFTCTRL;
		    else
			EVT.keyModifiers |= EVT_LEFTCTRL;
		    break;
		case KB_rightCtrl:
		    if (what == EVT_KEYUP)
			EVT.keyModifiers &= ~EVT_RIGHTCTRL;
		    else
			EVT.keyModifiers |= EVT_RIGHTCTRL;
		    break;
		case KB_leftAlt:
		    if (what == EVT_KEYUP)
			EVT.keyModifiers &= ~EVT_LEFTALT;
		    else
			EVT.keyModifiers |= EVT_LEFTALT;
		    break;
		case KB_rightAlt:
		    if (what == EVT_KEYUP)
			EVT.keyModifiers &= ~EVT_RIGHTALT;
		    else
			EVT.keyModifiers |= EVT_RIGHTALT;
		    break;
#ifdef SUPPORT_CTRL_ALT_DEL
		case KB_delete:
		    if ((EVT.keyModifiers & EVT_CTRLSTATE) && (EVT.keyModifiers & EVT_ALTSTATE))
			Reboot();
		    break;
#endif
		}
	    }

	/* Add the untranslated key code to the event queue. All
	 * translation to ASCII from the key codes occurs when the key
	 * is extracted from the queue, saving time in the low level
	 * interrupt handler.
	 */
	addKeyEvent(what,scan << 8);
	}
}

/****************************************************************************
DESCRIPTION:
Enables/disables the update of the keyboard LED status indicators.

HEADER:
event.h

PARAMETERS:
enable  - True to enable, false to disable

REMARKS:
Enables the update of the keyboard LED status indicators. Sometimes it may
be convenient in the application to turn off the updating of the LED
status indicators (such as if a game is using the CAPSLOCK key for some
function). Passing in a value of FALSE to this function will turn off all
the LEDS, and stop updating them when the internal status changes (note
however that internally we still keep track of the toggle key status!).
****************************************************************************/
void EVTAPI EVT_allowLEDS(
    ibool enable)
{
    EVT.allowLEDS = true;
    if (enable)
	setLEDS(EVT.keyModifiers);
    else
	setLEDS(0);
    EVT.allowLEDS = enable;
}
