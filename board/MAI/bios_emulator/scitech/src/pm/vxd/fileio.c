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
* Environment:  32-bit Windows VxD
*
* Description:  C library compatible I/O functions for use within a VxD.
*
****************************************************************************/

#include "pmapi.h"
#include "vxdfile.h"

/*------------------------ Main Code Implementation -----------------------*/

#define EOF -1

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fopen function.
****************************************************************************/
FILE * fopen(
    const char *filename,
    const char *mode)
{
    FILE    *f = PM_malloc(sizeof(FILE));
    long    oldpos;

    if (f) {
	f->offset = 0;
	f->text = (mode[1] == 't' || mode[2] == 't');
	f->writemode = (mode[0] == 'w') || (mode[0] == 'a');
	if (initComplete) {
	    WORD    omode,error;
	    BYTE    action;

	    if (mode[0] == 'r') {
		omode = OPEN_ACCESS_READONLY | OPEN_SHARE_COMPATIBLE;
		action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_FAIL;
		}
	    else if (mode[0] == 'w') {
		omode = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_COMPATIBLE;
		action = ACTION_IFEXISTS_TRUNCATE | ACTION_IFNOTEXISTS_CREATE;
		}
	    else {
		omode = OPEN_ACCESS_READWRITE | OPEN_SHARE_COMPATIBLE;
		action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_CREATE;
		}
	    f->handle = (int)R0_OpenCreateFile(false,(char*)filename,omode,ATTR_NORMAL,action,0,&error,&action);
	    if (f->handle == 0) {
		PM_free(f);
		return NULL;
		}
	    f->filesize = R0_GetFileSize((HANDLE)f->handle,&error);
	    if (mode[0] == 'a')
		fseek(f,0,2);
	    }
	else {
	    int oflag,pmode;

	    if (mode[0] == 'r') {
		pmode = _S_IREAD;
		oflag = _O_RDONLY;
		}
	    else if (mode[0] == 'w') {
		pmode = _S_IWRITE;
		oflag = _O_WRONLY | _O_CREAT | _O_TRUNC;
		}
	    else {
		pmode = _S_IWRITE;
		oflag = _O_RDWR | _O_CREAT | _O_APPEND;
		}
	    if (f->text)
		oflag |= _O_TEXT;
	    else
		oflag |= _O_BINARY;
	    if ((f->handle = i_open(filename,oflag,pmode)) == -1) {
		PM_free(f);
		return NULL;
		}
	    oldpos = i_lseek(f->handle,0,1);
	    f->filesize = i_lseek(f->handle,0,2);
	    i_lseek(f->handle,oldpos,0);
	    }
	}
    return f;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fread function. Note that the VxD file I/O
functions are layered on DOS, so can only read up to 64K at a time. Since
we are expected to handle much larger chunks than this, we handle larger
blocks automatically in here.
****************************************************************************/
size_t fread(
    void *ptr,
    size_t size,
    size_t n,
    FILE *f)
{
    char    *buf = ptr;
    WORD    error;
    int     bytes = size * n;
    int     readbytes,totalbytes = 0;

    while (bytes > 0x10000) {
	if (initComplete) {
	    readbytes  = R0_ReadFile(false,(HANDLE)f->handle,buf,0x8000,f->offset,&error);
	    readbytes += R0_ReadFile(false,(HANDLE)f->handle,buf+0x8000,0x8000,f->offset+0x8000,&error);
	    }
	else {
	    readbytes  = i_read(f->handle,buf,0x8000);
	    readbytes += i_read(f->handle,buf+0x8000,0x8000);
	    }
	totalbytes += readbytes;
	f->offset += readbytes;
	buf += 0x10000;
	bytes -= 0x10000;
	}
    if (bytes) {
	if (initComplete)
	    readbytes = R0_ReadFile(false,(HANDLE)f->handle,buf,bytes,f->offset,&error);
	else
	    readbytes = i_read(f->handle,buf,bytes);
	totalbytes += readbytes;
	f->offset += readbytes;
	}
    return totalbytes / size;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fwrite function. Note that the VxD file I/O
functions are layered on DOS, so can only read up to 64K at a time. Since
we are expected to handle much larger chunks than this, we handle larger
blocks automatically in here.
****************************************************************************/
size_t fwrite(
    const void *ptr,
    size_t size,
    size_t n,
    FILE *f)
{
    const char  *buf = ptr;
    WORD        error;
    int         bytes = size * n;
    int         writtenbytes,totalbytes = 0;

    if (!f->writemode)
	return 0;
    while (bytes > 0x10000) {
	if (initComplete) {
	    writtenbytes  = R0_WriteFile(false,(HANDLE)f->handle,buf,0x8000,f->offset,&error);
	    writtenbytes += R0_WriteFile(false,(HANDLE)f->handle,buf+0x8000,0x8000,f->offset+0x8000,&error);
	    }
	else {
	    writtenbytes  = i_write(f->handle,buf,0x8000);
	    writtenbytes += i_write(f->handle,buf+0x8000,0x8000);
	    }
	totalbytes += writtenbytes;
	f->offset += writtenbytes;
	buf += 0x10000;
	bytes -= 0x10000;
	}
    if (initComplete)
	writtenbytes = R0_WriteFile(false,(HANDLE)f->handle,buf,bytes,f->offset,&error);
    else
	writtenbytes = i_write(f->handle,buf,bytes);
    totalbytes += writtenbytes;
    f->offset += writtenbytes;
    if (f->offset > f->filesize)
	f->filesize = f->offset;
    return totalbytes / size;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fflush function.
****************************************************************************/
int fflush(
    FILE *f)
{
    /* Nothing to do since we are not doing buffered file I/O */
    (void)f;
    return 0;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fseek function.
****************************************************************************/
int fseek(
    FILE *f,
    long int offset,
    int whence)
{
    if (whence == 0)
	f->offset = offset;
    else if (whence == 1)
	f->offset += offset;
    else if (whence == 2)
	f->offset = f->filesize + offset;
    if (!initComplete)
	i_lseek(f->handle,f->offset,0);
    return 0;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C ftell function.
****************************************************************************/
long ftell(
    FILE *f)
{
    return f->offset;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C feof function.
****************************************************************************/
int feof(
    FILE *f)
{
    return (f->offset == f->filesize);
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C fgets function.
****************************************************************************/
char *fgets(
    char *s,
    int n,
    FILE *f)
{
    int     len;
    char    *cs;

    /* Read the entire buffer into memory (our functions are unbuffered!) */
    if ((len = fread(s,1,n,f)) == 0)
	return NULL;

    /* Search for '\n' or end of string */
    if (n > len)
	n = len;
    cs = s;
    while (--n > 0) {
	if (*cs == '\n')
	    break;
	cs++;
	}
    *cs = '\0';
    return s;
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C fputs function.
****************************************************************************/
int fputs(
    const char *s,
    FILE *f)
{
    return fwrite(s,1,strlen(s),f);
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fclose function.
****************************************************************************/
int fclose(
    FILE *f)
{
    WORD error;

    if (initComplete)
	R0_CloseFile((HANDLE)f->handle,&error);
    else
	i_close(f->handle);
    PM_free(f);
    return 0;
}
