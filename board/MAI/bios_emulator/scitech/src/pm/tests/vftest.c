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
* Filename:     $Workfile$
* Version:      $Revision: 1.1 $
*
* Language:     ANSI C
* Environment:  any
*
* Description:  Test program to test the VFlat virtual framebuffer functions.
*
*               Functions tested:   VF_available()
*                                   VF_init()
*                                   VF_exit()
*
* $Date: 2002/10/02 15:35:21 $ $Author: hfrieden $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

uchar code[] = {
    0xC3,                   /* ret          */
    };

int main(void)
{
    void    *vfBuffer;

    printf("Program running in ");
    switch (PM_getModeType()) {
	case PM_realMode:
	    printf("real mode.\n\n");
	    break;
	case PM_286:
	    printf("16 bit protected mode.\n\n");
	    break;
	case PM_386:
	    printf("32 bit protected mode.\n\n");
	    break;
	}

    if (!VF_available()) {
	printf("Virtual Linear Framebuffer not available.\n");
	exit(1);
	}

    vfBuffer = VF_init(0xA0000,64,sizeof(code),code);
    if (!vfBuffer) {
	printf("Failure to initialise Virtual Linear Framebuffer!\n");
	exit(1);
	}
    VF_exit();
    printf("Virtual Linear Framebuffer set up successfully!\n");
    return 0;
}
