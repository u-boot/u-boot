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
* Description:  Header file to pull in OS specific headers for the target
*               OS environment.
*
****************************************************************************/

#if     defined(__SMX32__)
#include "smx/oshdr.h"
#elif   defined(__RTTARGET__)
#include "rttarget/oshdr.h"
#elif   defined(__REALDOS__)
#include "dos/oshdr.h"
#elif   defined(__WIN32_VXD__)
#include "vxd/oshdr.h"
#elif   defined(__NT_DRIVER__)
#include "ntdrv/oshdr.h"
#elif   defined(__WINDOWS32__)
#include "win32/oshdr.h"
#elif   defined(__OS2_VDD__)
#include "vxd/oshdr.h"
#elif   defined(__OS2__)
#if     defined(__OS2_PM__)
#include "os2pm/oshdr.h"
#else
#include "os2/oshdr.h"
#endif
#elif   defined(__LINUX__)
#if     defined(__USE_X11__)
#include "x11/oshdr.h"
#else
#include "linux/oshdr.h"
#endif
#elif   defined(__QNX__)
#if     defined(__USE_PHOTON__)
#include "photon/oshdr.h"
#elif   defined(__USE_X11__)
#include "x11/oshdr.h"
#else
#include "qnx/oshdr.h"
#endif
#elif   defined(__BEOS__)
#include "beos/oshdr.h"
#else
#error  PM library not ported to this platform yet!
#endif
