/****************************************************************************
*
*                   SciTech Nucleus Graphics Architecture
*
*               Copyright (C) 1991-1998 SciTech Software, Inc.
*                            All rights reserved.
*
*  ======================================================================
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  |                                                                    |
*  |This copyrighted computer code contains proprietary technology      |
*  |owned by SciTech Software, Inc., located at 505 Wall Street,        |
*  |Chico, CA 95928 USA (http://www.scitechsoft.com).                   |
*  |                                                                    |
*  |The contents of this file are subject to the SciTech Nucleus        |
*  |License; you may *not* use this file or related software except in  |
*  |compliance with the License. You may obtain a copy of the License   |
*  |at http://www.scitechsoft.com/nucleus-license.txt                   |
*  |                                                                    |
*  |Software distributed under the License is distributed on an         |
*  |"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or      |
*  |implied. See the License for the specific language governing        |
*  |rights and limitations under the License.                           |
*  |                                                                    |
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  ======================================================================
*
* Language:     ANSI C
* Environment:  RTTarget-32
*
* Description:  OS specific Nucleus Graphics Architecture services for
*               the RTTarget-32 operating system environments.
*
****************************************************************************/

#include "nucleus/graphics.h"

/*------------------------- Global Variables ------------------------------*/

static ibool            haveRDTSC;

/*-------------------------- Implementation -------------------------------*/

/****************************************************************************
REMARKS:
Nothing special for this OS.
****************************************************************************/
GA_sharedInfo * NAPI GA_getSharedInfo(
    int device)
{
    (void)device;
    return NULL;
}

/****************************************************************************
REMARKS:
Nothing special for this OS.
****************************************************************************/
ibool NAPI GA_getSharedExports(
    GA_exports *gaExp)
{
    (void)gaExp;
    return false;
}

/****************************************************************************
REMARKS:
This function initialises the high precision timing functions for the
Nucleus loader library.
****************************************************************************/
ibool NAPI GA_TimerInit(void)
{
    if (_GA_haveCPUID() && (_GA_getCPUIDFeatures() & CPU_HaveRDTSC) != 0) {
	haveRDTSC = true;
	return true;
	}
    return false;
}

/****************************************************************************
REMARKS:
This function reads the high resolution timer.
****************************************************************************/
void NAPI GA_TimerRead(
    GA_largeInteger *value)
{
    if (haveRDTSC)
	_GA_readTimeStamp(value);
}
