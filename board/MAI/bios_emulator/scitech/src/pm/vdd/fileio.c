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
* Environment:  32-bit OS/2 VDD
*
* Description:  C library compatible I/O functions for use within a VDD.
*
****************************************************************************/

#include "pmapi.h"
#include "vddfile.h"

/*------------------------ Main Code Implementation -----------------------*/

#define EOF -1

/* NB: none of the file VDHs are available during the DOS session          */
/* initialzation context!                                                  */

/* Macros for Open/Close APIs to allow using this module in both VDDs and  */
/* normal OS/2 applications. Unfortunately VDHRead/Write/Seek don't map to */
/* their Dos* counterparts so cleanly.                                     */
#ifdef __OS2_VDD__
#define _OS2Open    VDHOpen
#define _OS2Close   VDHClose
#else
#define _OS2Open    DosOpen
#define _OS2Close   DosClose
#endif

/****************************************************************************
REMARKS:
VDD implementation of the ANSI C fopen function.
****************************************************************************/
FILE * fopen(
    const char *filename,
    const char *mode)
{
    FILE    *f = PM_malloc(sizeof(FILE));
    long    oldpos;
    ULONG   rc, ulAction;
    ULONG   omode, oflags;

    if (f != NULL) {
	f->offset = 0;
	f->text = (mode[1] == 't' || mode[2] == 't');
	f->writemode = (mode[0] == 'w') || (mode[0] == 'a');
	f->unputc = EOF;
	f->endp = f->buf + sizeof(f->buf);
	f->curp = f->startp = f->buf;

	if (mode[0] == 'r') {
	    #ifdef __OS2_VDD__
	    omode  = VDHOPEN_ACCESS_READONLY | VDHOPEN_SHARE_DENYNONE;
	    oflags = VDHOPEN_ACTION_OPEN_IF_EXISTS | VDHOPEN_ACTION_FAIL_IF_NEW;
	    #else
	    omode  = OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE;
	    oflags = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW;
	    #endif
	    }
	else if (mode[0] == 'w') {
	    #ifdef __OS2_VDD__
	    omode  = VDHOPEN_ACCESS_WRITEONLY | VDHOPEN_SHARE_DENYWRITE;
	    oflags = VDHOPEN_ACTION_REPLACE_IF_EXISTS | VDHOPEN_ACTION_CREATE_IF_NEW;
	    #else
	    omode  = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE;
	    oflags = OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW;
	    #endif
	    }
	else {
	    #ifdef __OS2_VDD__
	    omode  = VDHOPEN_ACCESS_READWRITE | VDHOPEN_SHARE_DENYWRITE;
	    oflags = VDHOPEN_ACTION_OPEN_IF_EXISTS | VDHOPEN_ACTION_CREATE_IF_NEW;
	    #else
	    omode  = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;
	    oflags = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW;
	    #endif
	    }
	rc = _OS2Open((PSZ)filename, (PHFILE)&f->handle, &ulAction, 0, VDHOPEN_FILE_NORMAL, oflags, omode, NULL);
	if (rc != 0) {
	    PM_free(f);
	    return NULL;
	    }

	#ifdef __OS2_VDD__
	f->filesize = VDHSeek((HFILE)f->handle, 0, VDHSK_END_OF_FILE);
	#else
	rc = DosSetFilePtr((HFILE)f->handle, 0, FILE_END, &f->filesize);
	#endif

	if (mode[0] == 'a')
	    fseek(f,0,2);
    }
    return f;
}

/****************************************************************************
REMARKS:
VDD implementation of the ANSI C fread function. Note that unlike Windows VxDs,
OS/2 VDDs are not limited to 64K reads or writes.
****************************************************************************/
size_t fread(
    void *ptr,
    size_t size,
    size_t n,
    FILE *f)
{
    char    *buf = ptr;
    int     bytes,readbytes,totalbytes = 0;

    /* First copy any data already read into our buffer */
    if ((bytes = (f->curp - f->startp)) > 0) {
	memcpy(buf,f->curp,bytes);
	f->startp = f->curp = f->buf;
	buf += bytes;
	totalbytes += bytes;
	bytes = (size * n) - bytes;
	}
    else
	bytes = size * n;
    if (bytes) {
	#ifdef __OS2_VDD__
	readbytes = VDHRead((HFILE)f->handle, buf, bytes);
	#else
	DosRead((HFILE)f->handle, buf, bytes, &readbytes);
	#endif
	totalbytes += readbytes;
	f->offset += readbytes;
	}
    return totalbytes / size;
}

/****************************************************************************
REMARKS:
VDD implementation of the ANSI C fwrite function.
****************************************************************************/
size_t fwrite(
    void *ptr,
    size_t size,
    size_t n,
    FILE *f)
{
    char        *buf = ptr;
    int         bytes,writtenbytes,totalbytes = 0;

    /* Flush anything already in the buffer */
    if (!f->writemode)
	return 0;
    fflush(f);
    bytes = size * n;
    #ifdef __OS2_VDD__
    writtenbytes = VDHWrite((HFILE)f->handle, buf, bytes);
    #else
    DosWrite((HFILE)f->handle, buf, bytes, &writtenbytes);
    #endif
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
    ULONG     bytes;

    /* First copy any data already written into our buffer */
    if (f->writemode && (bytes = (f->curp - f->startp)) > 0) {
	#ifdef __OS2_VDD__
	bytes = VDHWrite((HFILE)f->handle, f->startp, bytes);
	#else
	DosWrite((HFILE)f->handle, f->startp, bytes, &bytes);
	#endif
	f->offset += bytes;
	if (f->offset > f->filesize)
	    f->filesize = f->offset;
	f->startp = f->curp = f->buf;
	}
    return 0;
}

/****************************************************************************
REMARKS:
VDD implementation of the ANSI C fseek function.
****************************************************************************/
int fseek(
    FILE *f,
    long int offset,
    int whence)
{
    fflush(f);

    if (whence == 0)
	f->offset = offset;
    else if (whence == 1)
	f->offset += offset;
    else if (whence == 2)
	f->offset = f->filesize + offset;

    #ifdef __OS2_VDD__
    VDHSeek((HFILE)f->handle, f->offset, VDHSK_ABSOLUTE);
    #else
    DosSetFilePtr((HFILE)f->handle, f->offset, FILE_BEGIN, NULL);
    #endif

    return 0;
}

/****************************************************************************
REMARKS:
VDD implementation of the ANSI C ftell function.
****************************************************************************/
long ftell(
    FILE *f)
{
    long    offset;

    offset = (f->curp - f->startp);
    offset += f->offset;
    return offset;
}

/****************************************************************************
REMARKS:
VDD implementation of the ANSI C feof function.
****************************************************************************/
int feof(
    FILE *f)
{
    return (f->offset == f->filesize);
}

/****************************************************************************
REMARKS:
Read a single character from the input file buffer, including translation
of the character in text transation modes.
****************************************************************************/
static int __getc(
    FILE *f)
{
    int c;

    if (f->unputc != EOF) {
	c = f->unputc;
	f->unputc = EOF;
	}
    else {
	if (f->startp == f->curp) {
	    int bytes = fread(f->buf,1,sizeof(f->buf),f);
	    if (bytes == 0)
		return EOF;
	    f->curp = f->startp + bytes;
	    }
	c = *f->startp++;
	if (f->text && c == '\r') {
	    int nc = __getc(f);
	    if (nc != '\n')
		f->unputc = nc;
	    }
	}
    return c;
}

/****************************************************************************
REMARKS:
Write a single character from to input buffer, including translation of the
character in text transation modes.
****************************************************************************/
static int __putc(int c,FILE *f)
{
    int count = 1;
    if (f->text && c == '\n') {
	__putc('\r',f);
	count = 2;
	}
    if (f->curp == f->endp)
	fflush(f);
    *f->curp++ = c;
    return count;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fgets function.
****************************************************************************/
char *fgets(
    char *s,
    int n,
    FILE *f)
{
    int c = 0;
    char *cs;

    cs = s;
    while (--n > 0 && (c = __getc(f)) != EOF) {
	*cs++ = c;
	if (c == '\n')
	    break;
	}
    if (c == EOF && cs == s)
	return NULL;
    *cs = '\0';
    return s;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fputs function.
****************************************************************************/
int fputs(
    const char *s,
    FILE *f)
{
    int r = 0;
    int c;

    while ((c = *s++) != 0)
	r = __putc(c, f);
    return r;
}

/****************************************************************************
REMARKS:
VxD implementation of the ANSI C fclose function.
****************************************************************************/
int fclose(
    FILE *f)
{
    fflush(f);
    _OS2Close((HFILE)f->handle);
    PM_free(f);
    return 0;
}
