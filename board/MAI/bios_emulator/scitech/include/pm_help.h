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
* Description:  Include file for the SciTech Portability Manager 32-bit
*               helper VxD for Windows 9x for and the 16-bit ring 0
*               helper device driver for OS/2.
*
*               This file documents all the public services used by the
*               SciTech Portability Manager library and SciTech Nucleus
*               loader library.
*
****************************************************************************/

#ifndef __PMHELP_H
#define __PMHELP_H

/* Include version information */

#include "sdd/sddver.h"
#define PMHELP_Major            SDD_RELEASE_MAJOR
#define PMHELP_Minor            SDD_RELEASE_MINOR
#define PMHELP_VERSION          ((PMHELP_Major << 8) | PMHELP_Minor)

#ifdef  __OS2__

/****************************************************************************
* Public OS/2 Support functions
****************************************************************************/

#include "scitech.h"
#include "nucleus/graphics.h"

/* Name of device driver */

#define PMHELP_NAME                 (PSZ)"sddhelp$"

/* Main IOCTL function to talk to device driver */

#define PMHELP_IOCTL                0x0080

/* Macro definition for defining IOCTL function control codes for the SDDHELP
 * device driver for OS/2. Similar to that used for the DOS/Win32 version.
 */

#define PMHELP_CTL_CODE(name,value) \
    PMHELP_##name = value

typedef enum {
    /* Version function used by all drivers */
    PMHELP_CTL_CODE(GETVER                      ,0x0001),
    PMHELP_CTL_CODE(MAPPHYS                     ,0x0002),
    PMHELP_CTL_CODE(ALLOCLOCKED                 ,0x0003),
    PMHELP_CTL_CODE(FREELOCKED                  ,0x0004),
    PMHELP_CTL_CODE(GETGDT32                    ,0x0005),
    PMHELP_CTL_CODE(MALLOCSHARED                ,0x0007),
    PMHELP_CTL_CODE(FREESHARED                  ,0x0008),
    PMHELP_CTL_CODE(MAPTOPROCESS                ,0x0009),
    PMHELP_CTL_CODE(FREEPHYS                    ,0x000A),
    PMHELP_CTL_CODE(FLUSHTLB                    ,0x000B),
    PMHELP_CTL_CODE(SAVECR4                     ,0x000C),
    PMHELP_CTL_CODE(RESTORECR4                  ,0x000D),
    PMHELP_CTL_CODE(READMSR                     ,0x000E),
    PMHELP_CTL_CODE(WRITEMSR                    ,0x000F),
    PMHELP_CTL_CODE(GETPHYSICALADDR             ,0x0010),
    PMHELP_CTL_CODE(GETPHYSICALADDRRANGE        ,0x0011),
    PMHELP_CTL_CODE(LOCKPAGES                   ,0x0012),
    PMHELP_CTL_CODE(UNLOCKPAGES                 ,0x0013),
    PMHELP_CTL_CODE(GETSHAREDEXP                ,0x0042),
    PMHELP_CTL_CODE(SETSHAREDEXP                ,0x0043),
    PMHELP_CTL_CODE(GETSTACKSWITCHRTN           ,0x0044),
    PMHELP_CTL_CODE(GETBUILDNO                  ,0x0050),
    } PMHELP_ctlCodes;

#else

/****************************************************************************
* Public DOS/Windows Support functions
****************************************************************************/

#ifdef  DEVICE_MAIN
#include <vtoolsc.h>
#define PMHELP_Init_Order   (VDD_INIT_ORDER-1)
#define RETURN_LONGS(n)     *p->dioc_bytesret = (n) * sizeof(ulong)
#endif  /* DEVICE_MAIN */
#include "scitech.h"
#include "nucleus/graphics.h"

/* We connect to the SDDHELP.VXD module if it is staticly loaded (as part
 * of SciTech Display Doctor), otherwise we dynamically load the PMHELP.VXD
 * public helper VxD.
 */

#define PMHELP_DeviceID         0x0000
#define SDDHELP_DeviceID        0x3DF8
#define VXDLDR_DeviceID         0x0027
#define SDDHELP_MODULE          "SDDHELP"
#define SDDHELP_NAME            "SDDHELP.VXD"
#define PMHELP_MODULE           "PMHELP"
#define PMHELP_NAME             "PMHELP.VXD"
#define PMHELP_DDBNAME          "pmhelp  "
#define SDDHELP_MODULE_PATH     "\\\\.\\" SDDHELP_MODULE
#define PMHELP_MODULE_PATH      "\\\\.\\" PMHELP_MODULE
#define PMHELP_VXD_PATH         "\\\\.\\" PMHELP_NAME

/* Macro definition for defining IOCTL function control codes for the PMHELP
 * device drivers for Windows 9x and NT. This macro is basically derived from
 * the CTL_CODE macro in the Windows 2000 DDK, but we hard code it here to
 * avoid having to #include any of the Windows 2000 DDK header files. We also
 * define both a 16-bit and 32-bit version of the control code within the same
 * macro to simplify future additions.
 *
 * Essentially the Win32 macro would normally expand to the following:
 *
 *  CTL_CODE(FILE_DEVICE_VIDEO,0x800+value,METHOD_BUFFERED,FILE_ANY_ACCESS)
 */

#define PMHELP_CTL_CODE(name,value)                                             \
    PMHELP_##name = value,                                                      \
    PMHELP_##name##32 = ((0x23 << 16) | (0 << 14) | ((0x800+value) << 2) | (0))

typedef enum {
    /* Include all the control codes. We keep them in a separate header
     * file so we can include them in multiple places to make this
     * more versatile.
     */
    #include "pm_wctl.h"
    } PMHELP_ctlCodes;

/* For real mode VxD calls, we put the function number into the high
 * order word of EAX, and a value of 0x4FFF in AX. This allows our
 * VxD handler which is set up to handle Int 10's to recognise a native
 * PMHELP API call from a real mode DOS program.
 */

#ifdef  REALMODE
#define API_NUM(num)    (((ulong)(num) << 16) | 0x4FFF)
#else
#define API_NUM(num)    (num)
#endif

#endif  /* !__OS2__ */

#endif  /* __PMHELP_H */
