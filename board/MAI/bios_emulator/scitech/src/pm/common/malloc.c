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
* Description:  Module for implementing the PM library overrideable memory
*               allocator functions.
*
****************************************************************************/

#include "pmapi.h"

/*--------------------------- Global variables ----------------------------*/

void * (*__PM_malloc)(size_t size)              = malloc;
void * (*__PM_calloc)(size_t nelem,size_t size) = calloc;
void * (*__PM_realloc)(void *ptr,size_t size)   = realloc;
void (*__PM_free)(void *p)                      = free;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
DESCRIPTION:
Use local memory allocation routines.

HEADER:
pmapi.h

PARAMETERS:
malloc  - Pointer to new malloc routine to use
calloc  - Pointer to new caalloc routine to use
realloc - Pointer to new realloc routine to use
free    - Pointer to new free routine to use

REMARKS:
Tells the PM library to use a set of user specified memory allocation
routines instead of using the normal malloc/calloc/realloc/free standard
C library functions. This is useful if you wish to use a third party
debugging malloc library or perhaps a set of faster memory allocation
functions with the PM library, or any apps that use the PM library (such as
the MGL). Once you have registered your memory allocation routines, all
calls to PM_malloc, PM_calloc, PM_realloc and PM_free will be revectored to
your local memory allocation routines.

This is also useful if you need to keep track of just how much physical
memory your program has been using. You can use the PM_availableMemory
function to find out how much physical memory is available when the program
starts, and then you can use your own local memory allocation routines to
keep track of how much memory has been used and freed.

NOTE: This function should be called right at the start of your application,
      before you initialise any other components or libraries.

NOTE: Code compiled into Binary Portable DLL's and Drivers automatically
      end up calling these functions via the BPD C runtime library.

SEE ALSO:
PM_malloc, PM_calloc, PM_realloc, PM_free, PM_availableMemory
****************************************************************************/
void PMAPI PM_useLocalMalloc(
    void * (*malloc)(size_t size),
    void * (*calloc)(size_t nelem,size_t size),
    void * (*realloc)(void *ptr,size_t size),
    void (*free)(void *p))
{
    __PM_malloc = malloc;
    __PM_calloc = calloc;
    __PM_realloc = realloc;
    __PM_free = free;
}

/****************************************************************************
DESCRIPTION:
Allocate a block of memory.

HEADER:
pmapi.h

PARAMETERS:
size    - Size of block to allocate in bytes

RETURNS:
Pointer to allocated block, or NULL if out of memory.

REMARKS:
Allocates a block of memory of length size. If you have changed the memory
allocation routines with the PM_useLocalMalloc function, then calls to this
function will actually make calls to the local memory allocation routines
that you have registered.

SEE ALSO:
PM_calloc, PM_realloc, PM_free, PM_useLocalMalloc
****************************************************************************/
void * PMAPI PM_malloc(
    size_t size)
{
    return __PM_malloc(size);
}

/****************************************************************************
DESCRIPTION:
Allocate and clear a large memory block.

HEADER:
pmapi.h

PARAMETERS:
nelem   - number of contiguous size-byte units to allocate
size    - size of unit in bytes

RETURNS:
Pointer to allocated memory if successful, NULL if out of memory.

REMARKS:
Allocates a block of memory of length (size * nelem), and clears the
allocated area with zeros (0). If you have changed the memory allocation
routines with the PM_useLocalMalloc function, then calls to this function
will actually make calls to the local memory allocation routines that you
have registered.

SEE ALSO:
PM_malloc, PM_realloc, PM_free, PM_useLocalMalloc
****************************************************************************/
void * PMAPI PM_calloc(
    size_t nelem,
    size_t size)
{
    return __PM_calloc(nelem,size);
}

/****************************************************************************
DESCRIPTION:
Re-allocate a block of memory

HEADER:
pmapi.h

PARAMETERS:
ptr     - Pointer to block to resize
size    - size of unit in bytes

RETURNS:
Pointer to allocated memory if successful, NULL if out of memory.

REMARKS:
This function reallocates a block of memory that has been previously been
allocated to the new of size. The new size may be smaller or larger than
the original block of memory. If you have changed the memory allocation
routines with the PM_useLocalMalloc function, then calls to this function
will actually make calls to the local memory allocation routines that you
have registered.

SEE ALSO:
PM_malloc, PM_calloc, PM_free, PM_useLocalMalloc
****************************************************************************/
void * PMAPI PM_realloc(
    void *ptr,
    size_t size)
{
    return __PM_realloc(ptr,size);
}

/****************************************************************************
DESCRIPTION:
Frees a block of memory.

HEADER:
pmapi.h

PARAMETERS:
p   - Pointer to memory block to free

REMARKS:
Frees a block of memory previously allocated with either PM_malloc,
PM_calloc or PM_realloc.

SEE ALSO:
PM_malloc, PM_calloc, PM_realloc, PM_useLocalMalloc
****************************************************************************/
void PMAPI PM_free(
    void *p)
{
    __PM_free(p);
}
