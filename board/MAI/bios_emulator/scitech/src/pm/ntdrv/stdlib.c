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
* Environment:  32-bit Windows NT driver
*
* Description:  C library compatible stdlib.h functions for use within a
*               Windows NT driver.
*
****************************************************************************/

#include "pmapi.h"
#include "oshdr.h"

/*------------------------ Main Code Implementation -----------------------*/

/****************************************************************************
REMARKS:
PM_malloc override function for Nucleus drivers loaded in NT drivers's.
****************************************************************************/
void * malloc(
    size_t size)
{
    return PM_mallocShared(size);
}

/****************************************************************************
REMARKS:
calloc library function for Nucleus drivers loaded in NT drivers's.
****************************************************************************/
void * calloc(
    size_t nelem,
    size_t size)
{
    void *p = PM_mallocShared(nelem * size);
    if (p)
	memset(p,0,nelem * size);
    return p;
}

/****************************************************************************
REMARKS:
PM_realloc override function for Nucleus drivers loaded in VxD's.
****************************************************************************/
void * realloc(
    void *ptr,
    size_t size)
{
    void *p = PM_mallocShared(size);
    if (p) {
	memcpy(p,ptr,size);
	PM_freeShared(ptr);
	}
    return p;
}

/****************************************************************************
REMARKS:
PM_free override function for Nucleus drivers loaded in VxD's.
****************************************************************************/
void free(
    void *p)
{
    PM_freeShared(p);
}

/****************************************************************************
PARAMETERS:
cstr    - C style ANSI string to convert

RETURNS:
Pointer to the UniCode string structure or NULL on failure to allocate memory

REMARKS:
Converts a C style string to a UniCode string structure that can be passed
directly to NT kernel functions.
****************************************************************************/
UNICODE_STRING *_PM_CStringToUnicodeString(
    const char *cstr)
{
    int             length;
    ANSI_STRING     ansiStr;
    UNICODE_STRING  *uniStr;

    /* Allocate memory for the string structure */
    if ((uniStr = ExAllocatePool(NonPagedPool, sizeof(UNICODE_STRING))) == NULL)
	return NULL;

    /* Allocate memory for the wide string itself */
    length = (strlen(cstr) * sizeof(WCHAR)) + sizeof(WCHAR);
    if ((uniStr->Buffer = ExAllocatePool(NonPagedPool, length)) == NULL) {
	ExFreePool(uniStr);
	return NULL;
	}
    RtlZeroMemory(uniStr->Buffer, length);
    uniStr->Length = 0;
    uniStr->MaximumLength = (USHORT)length;

    /* Convert filename string to ansi string and then to UniCode string */
    RtlInitAnsiString(&ansiStr, cstr);
    RtlAnsiStringToUnicodeString(uniStr, &ansiStr, FALSE);
    return uniStr;
}

/****************************************************************************
PARAMETERS:
uniStr  - UniCode string structure to free

REMARKS:
Frees a string allocated by the above _PM_CStringToUnicodeString function.
****************************************************************************/
void _PM_FreeUnicodeString(
    UNICODE_STRING *uniStr)
{
    if (uniStr) {
	ExFreePool(uniStr->Buffer);
	ExFreePool(uniStr);
	}
}
