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
* Description:  Keyboard translation code pages for US English keyboards.
*
****************************************************************************/

#include "event.h"

/*--------------------------- Global variables ----------------------------*/

/* This table is used for all normal key translations, and is the fallback
 * table if the key is not found in any of the other translation tables.
 * If the code is not found in this table, the ASCII code is set to 0 to
 * indicate that there is no ASCII code equivalent for this key.
 */
static codepage_entry_t US_normal[] = {
    {0x01,  0x1B},
    {0x02,  '1'},
    {0x03,  '2'},
    {0x04,  '3'},
    {0x05,  '4'},
    {0x06,  '5'},
    {0x07,  '6'},
    {0x08,  '7'},
    {0x09,  '8'},
    {0x0A,  '9'},
    {0x0B,  '0'},
    {0x0C,  '-'},
    {0x0D,  '='},
    {0x0E,  0x08},
    {0x0F,  0x09},
    {0x10,  'q'},
    {0x11,  'w'},
    {0x12,  'e'},
    {0x13,  'r'},
    {0x14,  't'},
    {0x15,  'y'},
    {0x16,  'u'},
    {0x17,  'i'},
    {0x18,  'o'},
    {0x19,  'p'},
    {0x1A,  '['},
    {0x1B,  ']'},
    {0x1C,  0x0D},
    {0x1E,  'a'},
    {0x1F,  's'},
    {0x20,  'd'},
    {0x21,  'f'},
    {0x22,  'g'},
    {0x23,  'h'},
    {0x24,  'j'},
    {0x25,  'k'},
    {0x26,  'l'},
    {0x27,  ';'},
    {0x28,  '\''},
    {0x29,  '`'},
    {0x2B,  '\\'},
    {0x2C,  'z'},
    {0x2D,  'x'},
    {0x2E,  'c'},
    {0x2F,  'v'},
    {0x30,  'b'},
    {0x31,  'n'},
    {0x32,  'm'},
    {0x33,  ','},
    {0x34,  '.'},
    {0x35,  '/'},
    {0x37,  '*'},           /* Keypad */
    {0x39,  ' '},
    {0x4A,  '-'},           /* Keypad */
    {0x4E,  '+'},           /* Keypad */
    {0x60,  0x0D},          /* Keypad */
    {0x61,  '/'},           /* Keypad */
    };

/* This table is used for when CAPSLOCK is active and the shift or ctrl
 * keys are not down. If the code is not found in this table, the normal
 * table above is then searched.
 */
static codepage_entry_t US_caps[] = {
    {0x10,  'Q'},
    {0x11,  'W'},
    {0x12,  'E'},
    {0x13,  'R'},
    {0x14,  'T'},
    {0x15,  'Y'},
    {0x16,  'U'},
    {0x17,  'I'},
    {0x18,  'O'},
    {0x19,  'P'},
    {0x1E,  'A'},
    {0x1F,  'S'},
    {0x20,  'D'},
    {0x21,  'F'},
    {0x22,  'G'},
    {0x23,  'H'},
    {0x24,  'J'},
    {0x25,  'K'},
    {0x26,  'L'},
    {0x2C,  'Z'},
    {0x2D,  'X'},
    {0x2E,  'C'},
    {0x2F,  'V'},
    {0x30,  'B'},
    {0x31,  'N'},
    {0x32,  'M'},
    };

/* This table is used for when shift key is down, but the ctrl key is not
 * down and CAPSLOCK is not active. If the code is not found in this table,
 * the normal table above is then searched.
 */
static codepage_entry_t US_shift[] = {
    {0x02,  '!'},
    {0x03,  '@'},
    {0x04,  '#'},
    {0x05,  '$'},
    {0x06,  '%'},
    {0x07,  '^'},
    {0x08,  '&'},
    {0x09,  '*'},
    {0x0A,  '('},
    {0x0B,  ')'},
    {0x0C,  '_'},
    {0x0D,  '+'},
    {0x10,  'Q'},
    {0x11,  'W'},
    {0x12,  'E'},
    {0x13,  'R'},
    {0x14,  'T'},
    {0x15,  'Y'},
    {0x16,  'U'},
    {0x17,  'I'},
    {0x18,  'O'},
    {0x19,  'P'},
    {0x1A,  '{'},
    {0x1B,  '}'},
    {0x1E,  'A'},
    {0x1F,  'S'},
    {0x20,  'D'},
    {0x21,  'F'},
    {0x22,  'G'},
    {0x23,  'H'},
    {0x24,  'J'},
    {0x25,  'K'},
    {0x26,  'L'},
    {0x27,  ':'},
    {0x28,  '"'},
    {0x29,  '~'},
    {0x2B,  '|'},
    {0x2C,  'Z'},
    {0x2D,  'X'},
    {0x2E,  'C'},
    {0x2F,  'V'},
    {0x30,  'B'},
    {0x31,  'N'},
    {0x32,  'M'},
    {0x33,  '<'},
    {0x34,  '>'},
    {0x35,  '?'},
    };

/* This table is used for when CAPSLOCK is active and the shift key is
 * down, but the ctrl key is not. If the code is not found in this table,
 * the shift table above is then searched.
 */
static codepage_entry_t US_shiftCaps[] = {
    {0x10,  'q'},
    {0x11,  'w'},
    {0x12,  'e'},
    {0x13,  'r'},
    {0x14,  't'},
    {0x15,  'y'},
    {0x16,  'u'},
    {0x17,  'i'},
    {0x18,  'o'},
    {0x19,  'p'},
    {0x1E,  'a'},
    {0x1F,  's'},
    {0x20,  'd'},
    {0x21,  'f'},
    {0x22,  'g'},
    {0x23,  'h'},
    {0x24,  'j'},
    {0x25,  'k'},
    {0x26,  'l'},
    {0x2C,  'z'},
    {0x2D,  'x'},
    {0x2E,  'c'},
    {0x2F,  'v'},
    {0x30,  'b'},
    {0x31,  'n'},
    {0x32,  'm'},
    };

/* This table is used for all key translations when the ctrl key is down,
 * regardless of the state of the shift key and CAPSLOCK. If the code is
 * not found in this table, the ASCII code is set to 0 to indicate that
 * there is no ASCII code equivalent for this key.
 */
static codepage_entry_t US_ctrl[] = {
    {0x01,  0x1B},
    {0x06,  0x1E},
    {0x0C,  0x1F},
    {0x0E,  0x7F},
    {0x10,  0x11},
    {0x11,  0x17},
    {0x12,  0x05},
    {0x13,  0x12},
    {0x14,  0x14},
    {0x15,  0x19},
    {0x16,  0x16},
    {0x17,  0x09},
    {0x18,  0x0F},
    {0x19,  0x10},
    {0x1A,  0x1B},
    {0x1B,  0x1D},
    {0x1C,  0x0A},
    {0x1E,  0x01},
    {0x1F,  0x13},
    {0x20,  0x04},
    {0x21,  0x06},
    {0x22,  0x07},
    {0x23,  0x08},
    {0x24,  0x0A},
    {0x25,  0x0B},
    {0x26,  0x0C},
    {0x2B,  0x1C},
    {0x2C,  0x1A},
    {0x2D,  0x18},
    {0x2E,  0x03},
    {0x2F,  0x16},
    {0x30,  0x02},
    {0x31,  0x0E},
    {0x32,  0x0D},
    {0x39,  ' '},
    };

static codepage_entry_t US_numPad[] = {
    {0x4C,  '5'},
    {0x62,  '4'},
    {0x63,  '6'},
    {0x64,  '8'},
    {0x65,  '2'},
    {0x66,  '0'},
    {0x67,  '.'},
    {0x68,  '7'},
    {0x69,  '1'},
    {0x6A,  '9'},
    {0x6B,  '3'},
    };

codepage_t _CP_US_English = {
    "US English",
    US_normal,      EVT_ARR_SIZE(US_normal),
    US_caps,        EVT_ARR_SIZE(US_caps),
    US_shift,       EVT_ARR_SIZE(US_shift),
    US_shiftCaps,   EVT_ARR_SIZE(US_shiftCaps),
    US_ctrl,        EVT_ARR_SIZE(US_ctrl),
    US_numPad,      EVT_ARR_SIZE(US_numPad),
    };
