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
* Language:     C++ 3.0
* Environment:  Any
*
* Description:  Test program for the Zen Timer Library C++ interface.
*
****************************************************************************/

#include <iostream.h>
#include "pmapi.h"
#include "ztimer.h"

/*-------------------------- Implementation -------------------------------*/

int     i,j,k;                              /* NON register variables! */

void dummy() {}

int main(void)
{
    LZTimer     ltimer;
    ULZTimer    ultimer;

    ZTimerInit();

    /* Test the long period Zen Timer (we don't check for overflow coz
     * it would take tooooo long!)
     */

    cout << endl;
    ultimer.restart();
    ltimer.start();
    for (j = 0; j < 10; j++)
        for (i = 0; i < 20000; i++)
            dummy();
    ltimer.stop();
    ultimer.stop();
    cout << "LCount:  " << ltimer.count() << endl;
    cout << "Time:    " << ltimer << " secs\n";
    cout << "ULCount: " << ultimer.count() << endl;
    cout << "ULTime:  " << ultimer << " secs\n";

    cout << endl << "Timing ... \n";
    ultimer.restart();
    ltimer.restart();
    for (j = 0; j < 200; j++)
        for (i = 0; i < 20000; i++)
            dummy();
    ltimer.stop();
    ultimer.stop();
    cout << "LCount:  " << ltimer.count() << endl;
    cout << "Time:    " << ltimer << " secs\n";
    cout << "ULCount: " << ultimer.count() << endl;
    cout << "ULTime:  " << ultimer << " secs\n";

    /* Test the lap function of the long period Zen Timer */

    cout << endl << "Timing ... \n";
    ultimer.restart();
    ltimer.restart();
    for (j = 0; j < 20; j++) {
        for (k = 0; k < 10; k++)
            for (i = 0; i < 20000; i++)
                dummy();
        cout << "lap: " << ltimer.lap() << endl;
        }
    ltimer.stop();
    ultimer.stop();
    cout << "LCount:  " << ltimer.count() << endl;
    cout << "Time:    " << ltimer << " secs\n";
    cout << "ULCount: " << ultimer.count() << endl;
    cout << "ULTime:  " << ultimer << " secs\n";

#ifdef  LONG_TEST
    /* Test the ultra long period Zen Timer */

    ultimer.start();
    delay(DELAY_SECS * 1000);
    ultimer.stop();
    cout << "Delay of " << DELAY_SECS << " secs took " << ultimer.count()
         << " 1/10ths of a second\n";
    cout << "Time: " << ultimer << " secs\n";
#endif
    return 0;
}
