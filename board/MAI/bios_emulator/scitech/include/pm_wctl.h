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
* Environment:  Win32, OS/2
*
* Description:  Header file to define all the control codes for the DOS
*               and Win32 device driver API's for calling from ring 3
*               into the ring 0 device drivers.
*
****************************************************************************/

/* Version function used by all drivers */
PMHELP_CTL_CODE(GETVER                      ,0x0000),

/* Functions used by obsolete 16-bit DOS TSR */
PMHELP_CTL_CODE(RDREGB                      ,0x0003),
PMHELP_CTL_CODE(WRREGB                      ,0x0004),
PMHELP_CTL_CODE(RDREGW                      ,0x0005),
PMHELP_CTL_CODE(WRREGW                      ,0x0006),
PMHELP_CTL_CODE(RDREGL                      ,0x0008),
PMHELP_CTL_CODE(WRREGL                      ,0x0009),

/* Functions used by obsolete WinDirect */
PMHELP_CTL_CODE(MAPPHYS                     ,0x000F),
PMHELP_CTL_CODE(GETVESABUF                  ,0x0013),

/* Functions used by PM library */
PMHELP_CTL_CODE(DPMIINT86                   ,0x0014),
PMHELP_CTL_CODE(INT86                       ,0x0015),
PMHELP_CTL_CODE(INT86X                      ,0x0016),
PMHELP_CTL_CODE(CALLREALMODE                ,0x0017),
PMHELP_CTL_CODE(ALLOCLOCKED                 ,0x0018),
PMHELP_CTL_CODE(FREELOCKED                  ,0x0019),
PMHELP_CTL_CODE(ENABLELFBCOMB               ,0x001A),
PMHELP_CTL_CODE(GETPHYSICALADDR             ,0x001B),
PMHELP_CTL_CODE(MALLOCSHARED                ,0x001D),
PMHELP_CTL_CODE(FREESHARED                  ,0x001F),
PMHELP_CTL_CODE(LOCKDATAPAGES               ,0x0020),
PMHELP_CTL_CODE(UNLOCKDATAPAGES             ,0x0021),
PMHELP_CTL_CODE(LOCKCODEPAGES               ,0x0022),
PMHELP_CTL_CODE(UNLOCKCODEPAGES             ,0x0023),
PMHELP_CTL_CODE(GETCALLGATE                 ,0x0024),
PMHELP_CTL_CODE(SETCNTPATH                  ,0x0025),
PMHELP_CTL_CODE(GETPDB                      ,0x0026),
PMHELP_CTL_CODE(FLUSHTLB                    ,0x0027),
PMHELP_CTL_CODE(GETPHYSICALADDRRANGE        ,0x0028),
PMHELP_CTL_CODE(ALLOCPAGE                   ,0x0029),
PMHELP_CTL_CODE(FREEPAGE                    ,0x002A),
PMHELP_CTL_CODE(ENABLERING3IOPL             ,0x002B),
PMHELP_CTL_CODE(DISABLERING3IOPL            ,0x002C),
PMHELP_CTL_CODE(GASETLOCALPATH              ,0x002D),
PMHELP_CTL_CODE(GAGETEXPORTS                ,0x002E),
PMHELP_CTL_CODE(GATHUNK                     ,0x002F),
PMHELP_CTL_CODE(SETNUCLEUSPATH              ,0x0030),
