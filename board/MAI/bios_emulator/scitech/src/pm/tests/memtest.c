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
*
* Language:     ANSI C
* Environment:  any
*
* Description:  Test program to determine just how much memory can be
*               allocated with the compiler in use. Compile and link
*               with the appropriate command line for your DOS extender.
*
*               Functions tested:   PM_malloc()
*                                   PM_availableMemory()
*
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pmapi.h"

#ifdef  __16BIT__
#define MAXALLOC    64
#else
#define MAXALLOC    2000
#endif

int main(void)
{
    int     i;
    ulong   allocs;
    ulong   physical,total;
    char    *p,*pa[MAXALLOC];

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

    printf("Memory available at start:\n");
    PM_availableMemory(&physical,&total);
    printf("   Physical memory:           %ld Kb\n", physical / 1024);
    printf("   Total (including virtual): %ld Kb\n", total / 1024);
    printf("\n");
    for (allocs = i = 0; i < MAXALLOC; i++) {
	if ((pa[i] = PM_malloc(10*1024)) != 0) {    /* in 10k blocks    */
	    p = pa[allocs];
	    memset(p, 0, 10*1024); /* touch every byte              */
	    *p = 'x';           /* do something, anything with      */
	    p[1023] = 'y';      /* the allocated memory             */
	    allocs++;
	    printf("Allocated %lu bytes\r", 10*(allocs << 10));
	    }
	else break;
	if (PM_kbhit() && (PM_getch() == 0x1B))
	    break;
	}

    printf("\n\nAllocated total of %lu bytes\n", 10 * (allocs << 10));

    printf("\nMemory available at end:\n");
    PM_availableMemory(&physical,&total);
    printf("   Physical memory:           %ld Kb\n", physical / 1024);
    printf("   Total (including virtual): %ld Kb\n", total / 1024);

    for (i = allocs-1; i >= 0; i--)
	PM_free(pa[i]);

    printf("\nMemory available after freeing all blocks (note that under protected mode\n");
    printf("this will most likely not be correct after freeing blocks):\n\n");
    PM_availableMemory(&physical,&total);
    printf("   Physical memory:           %ld Kb\n", physical / 1024);
    printf("   Total (including virtual): %ld Kb\n", total / 1024);

    return 0;
}
