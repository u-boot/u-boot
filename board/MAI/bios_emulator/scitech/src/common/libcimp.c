/****************************************************************************
*
*                       SciTech MGL Graphics Library
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
* Description:  Module to implement a the OS specific side of the Binary
*               Portable DLL C runtime library. The functions in here
*               are imported into the Binary Portable DLL's to implement
*               OS specific services.
*
****************************************************************************/

#include "pmapi.h"
#if defined(__WIN32_VXD__) || defined(__NT_DRIVER__)
#include "drvlib/peloader.h"
#include "drvlib/attrib.h"
#include "drvlib/libc/init.h"
#define __BUILDING_PE_LOADER__
#include "drvlib/libc/file.h"
#if defined(__WIN32_VXD__)
#include "vxdfile.h"
#endif
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#if defined(__GNUC__) || defined(__UNIX__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <io.h>
#endif
#include "drvlib/attrib.h"
#include "drvlib/libc/init.h"
#define __BUILDING_PE_LOADER__
#include "drvlib/libc/file.h"
#if defined(__WINDOWS__) || defined(TNT) || defined(__RTTARGET__)
#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#endif
#ifdef  __MSDOS__
#include <dos.h>
#endif
#ifdef  __OS2__
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_SUB
#include <os2.h>
#endif
#endif

/* No text or binary modes for Unix */

#ifndef O_BINARY
#define O_BINARY    0
#define O_TEXT      0
#endif

/*--------------------------- Global variables ----------------------------*/

#if defined(__WIN32_VXD__) || defined(__NT_DRIVER__)
#define MAX_FILES   16
static FILE *openHandles[MAX_FILES] = {NULL};
#endif

/* <stdlib.h> stub functions */
void    _CDECL stub_abort(void);
int     _CDECL stub_atexit(void (*)(void));
void *  _CDECL stub_calloc(size_t _nelem, size_t _size);
void    _CDECL stub_exit(int _status);
void    _CDECL stub_free(void *_ptr);
char *  _CDECL stub_getenv(const char *_name);
void *  _CDECL stub_malloc(size_t _size);
void *  _CDECL stub_realloc(void *_ptr, size_t _size);
int     _CDECL stub_system(const char *_s);
int     _CDECL stub_putenv(const char *_val);

/* <libc/file.h> stub functions */
int     _CDECL stub_open(const char *_path, int _oflag, unsigned _mode);
int     _CDECL stub_access(const char *_path, int _amode);
int     _CDECL stub_close(int _fildes);
off_t   _CDECL stub_lseek(int _fildes, off_t _offset, int _whence);
size_t  _CDECL stub_read(int _fildes, void *_buf, size_t _nbyte);
int     _CDECL stub_unlink(const char *_path);
size_t  _CDECL stub_write(int _fildes, const void *_buf, size_t _nbyte);
int     _CDECL stub_isatty(int _fildes);

/* <stdio.h> stub functions */
int     _CDECL stub_remove(const char *_filename);
int     _CDECL stub_rename(const char *_old, const char *_new);

/* <time.h> stub functions */
time_t  _CDECL stub_time(time_t *_tod);

/* <signal.h> stub functions */
int     _CDECL stub_raise(int);
void *  _CDECL stub_signal(int, void *);

/* <drvlib/attrib.h> functions */
#define stub_OS_setfileattr    _OS_setfileattr
#define stub_OS_getcurrentdate _OS_getcurrentdate

LIBC_imports    _VARAPI ___imports = {
    sizeof(LIBC_imports),

    /* <stdlib.h> exports */
    stub_abort,
    stub_atexit,
    stub_calloc,
    stub_exit,
    stub_free,
    stub_getenv,
    stub_malloc,
    stub_realloc,
    stub_system,
    stub_putenv,

    /* <libc/file.h> exports */
    stub_open,
    stub_access,
    stub_close,
    stub_lseek,
    stub_read,
    stub_unlink,
    stub_write,
    stub_isatty,

    /* <stdio.h> exports */
    stub_remove,
    stub_rename,

    /* <signal.h> functions */
    stub_raise,
    stub_signal,

    /* <time.h> exports */
    stub_time,

    /* <drvlib/attrib.h> exports */
    stub_OS_setfileattr,
    stub_OS_getcurrentdate,
    };

/*---------------------- Stub function implementation ---------------------*/

/* <stdlib.h> stub functions */
void _CDECL stub_abort(void)
{
#if !defined( __WIN32_VXD__) && !defined(__NT_DRIVER__)
    abort();
#endif
}

int _CDECL stub_atexit(void (*func)(void))
{
#if !defined( __WIN32_VXD__) && !defined(__NT_DRIVER__)
    return atexit((void(*)(void))func);
#else
    return -1;
#endif
}

void * _CDECL stub_calloc(size_t _nelem, size_t _size)
{ return __PM_calloc(_nelem,_size); }

void _CDECL stub_exit(int _status)
{
#if !defined( __WIN32_VXD__) && !defined(__NT_DRIVER__)
    exit(_status);
#endif
}

void _CDECL stub_free(void *_ptr)
{ __PM_free(_ptr); }

char * _CDECL stub_getenv(const char *_name)
{
#if defined( __WIN32_VXD__) || defined(__NT_DRIVER__)
    return NULL;
#else
    return getenv(_name);
#endif
}

void * _CDECL stub_malloc(size_t _size)
{ return __PM_malloc(_size); }

void * _CDECL stub_realloc(void *_ptr, size_t _size)
{ return __PM_realloc(_ptr,_size); }

int _CDECL stub_system(const char *_s)
{
#if defined(__WINDOWS__) || defined(__WIN32_VXD__) || defined(__NT_DRIVER__) || defined(__SMX32__) || defined(__RTTARGET__)
    (void)_s;
    return -1;
#else
    return system(_s);
#endif
}

int _CDECL stub_putenv(const char *_val)
{
#if defined( __WIN32_VXD__) || defined(__NT_DRIVER__)
    return -1;
#else
    return putenv((char*)_val);
#endif
}

time_t _CDECL stub_time(time_t *_tod)
{
#if defined( __WIN32_VXD__) || defined(__NT_DRIVER__)
    return 0;
#else
    return time(_tod);
#endif
}

#if     defined(__MSDOS__)

#if defined(TNT) && defined(_MSC_VER)

void _CDECL _OS_setfileattr(const char *filename,unsigned attrib)
{ SetFileAttributes((LPSTR)filename, (DWORD)attrib); }

#else

void _CDECL _OS_setfileattr(const char *filename,unsigned attrib)
{ _dos_setfileattr(filename,attrib); }

#endif

#elif   defined(__WIN32_VXD__)

#define USE_LOCAL_FILEIO
#define USE_LOCAL_GETDATE

/* <libc/file.h> stub functions */
int _CDECL stub_open(const char *_path, int _oflag, unsigned _mode)
{
    char    mode[10];
    int     i;

    /* Find an empty file handle to use */
    for (i = 3; i < MAX_FILES; i++) {
	if (!openHandles[i])
	    break;
	}
    if (openHandles[i])
	return -1;

    /* Find the open flags to use */
    if (_oflag & ___O_TRUNC)
	strcpy(mode,"w");
    else if (_oflag & ___O_CREAT)
	strcpy(mode,"a");
    else
	strcpy(mode,"r");
    if (_oflag & ___O_BINARY)
	strcat(mode,"b");
    if (_oflag & ___O_TEXT)
	strcat(mode,"t");

    /* Open the file and store the file handle */
    if ((openHandles[i] = fopen(_path,mode)) == NULL)
	return -1;
    return i;
}

int _CDECL stub_access(const char *_path, int _amode)
{ return -1; }

int _CDECL stub_close(int _fildes)
{
    if (_fildes >= 3 && openHandles[_fildes]) {
	fclose(openHandles[_fildes]);
	openHandles[_fildes] = NULL;
	}
    return 0;
}

off_t _CDECL stub_lseek(int _fildes, off_t _offset, int _whence)
{
    if (_fildes >= 3) {
	fseek(openHandles[_fildes],_offset,_whence);
	return ftell(openHandles[_fildes]);
	}
    return 0;
}

size_t _CDECL stub_read(int _fildes, void *_buf, size_t _nbyte)
{
    if (_fildes >= 3)
	return fread(_buf,1,_nbyte,openHandles[_fildes]);
    return 0;
}

int _CDECL stub_unlink(const char *_path)
{
    WORD error;

    if (initComplete) {
	if (R0_DeleteFile((char*)_path,0,&error))
	    return 0;
	return -1;
	}
    else
	return i_remove(_path);
}

size_t _CDECL stub_write(int _fildes, const void *_buf, size_t _nbyte)
{
    if (_fildes >= 3)
	return fwrite(_buf,1,_nbyte,openHandles[_fildes]);
    return _nbyte;
}

int _CDECL stub_isatty(int _fildes)
{ return 0; }

/* <stdio.h> stub functions */
int _CDECL stub_remove(const char *_filename)
{ return stub_unlink(_filename); }

int _CDECL stub_rename(const char *_old, const char *_new)
{ return -1; }

void _CDECL _OS_setfileattr(const char *filename,unsigned attrib)
{
    WORD error;
    if (initComplete)
	R0_SetFileAttributes((char*)filename,attrib,&error);
}

/* Return the current date in days since 1/1/1980 */
ulong _CDECL _OS_getcurrentdate(void)
{
    DWORD   date;
    VTD_Get_Date_And_Time(&date);
    return date;
}

#elif   defined(__NT_DRIVER__)

#define USE_LOCAL_FILEIO
#define USE_LOCAL_GETDATE

/* <libc/file.h> stub functions */
int _CDECL stub_open(const char *_path, int _oflag, unsigned _mode)
{
    char    mode[10];
    int     i;

    /* Find an empty file handle to use */
    for (i = 3; i < MAX_FILES; i++) {
	if (!openHandles[i])
	    break;
	}
    if (openHandles[i])
	return -1;

    /* Find the open flags to use */
    if (_oflag & ___O_TRUNC)
	strcpy(mode,"w");
    else if (_oflag & ___O_CREAT)
	strcpy(mode,"a");
    else
	strcpy(mode,"r");
    if (_oflag & ___O_BINARY)
	strcat(mode,"b");
    if (_oflag & ___O_TEXT)
	strcat(mode,"t");

    /* Open the file and store the file handle */
    if ((openHandles[i] = fopen(_path,mode)) == NULL)
	return -1;
    return i;
}

int _CDECL stub_close(int _fildes)
{
    if (_fildes >= 3 && openHandles[_fildes]) {
	fclose(openHandles[_fildes]);
	openHandles[_fildes] = NULL;
	}
    return 0;
}

off_t _CDECL stub_lseek(int _fildes, off_t _offset, int _whence)
{
    if (_fildes >= 3) {
	fseek(openHandles[_fildes],_offset,_whence);
	return ftell(openHandles[_fildes]);
	}
    return 0;
}

size_t _CDECL stub_read(int _fildes, void *_buf, size_t _nbyte)
{
    if (_fildes >= 3)
	return fread(_buf,1,_nbyte,openHandles[_fildes]);
    return 0;
}

size_t _CDECL stub_write(int _fildes, const void *_buf, size_t _nbyte)
{
    if (_fildes >= 3)
	return fwrite(_buf,1,_nbyte,openHandles[_fildes]);
    return _nbyte;
}

int _CDECL stub_access(const char *_path, int _amode)
{ return -1; }

int _CDECL stub_isatty(int _fildes)
{ return 0; }

int _CDECL stub_unlink(const char *_path)
{
    /* TODO: Implement this! */
    return -1;
}

/* <stdio.h> stub functions */
int _CDECL stub_remove(const char *_filename)
{ return stub_unlink(_filename); }

int _CDECL stub_rename(const char *_old, const char *_new)
{
    /* TODO: Implement this! */
    return -1;
}

void _CDECL _OS_setfileattr(const char *filename,unsigned attrib)
{
    uint _attr = 0;
    if (attrib & __A_RDONLY)
	_attr |= FILE_ATTRIBUTE_READONLY;
    if (attrib & __A_HIDDEN)
	_attr |= FILE_ATTRIBUTE_HIDDEN;
    if (attrib & __A_SYSTEM)
	_attr |= FILE_ATTRIBUTE_SYSTEM;
    PM_setFileAttr(filename,_attr);
}

/* Return the current date in days since 1/1/1980 */
ulong _CDECL _OS_getcurrentdate(void)
{
    TIME_FIELDS tm;
    _int64      count,count_1_1_1980;

    tm.Year         = 1980;
    tm.Month        = 1;
    tm.Day          = 1;
    tm.Hour         = 0;
    tm.Minute       = 0;
    tm.Second       = 0;
    tm.Milliseconds = 0;
    tm.Weekday      = 0;
    RtlTimeFieldsToTime(&tm,(PLARGE_INTEGER)&count_1_1_1980);
    KeQuerySystemTime((PLARGE_INTEGER)&count);
    return (ulong)( (count - count_1_1_1980) / ((_int64)24 * (_int64)3600 * (_int64)10000000) );
}

#elif   defined(__WINDOWS32__) || defined(__RTTARGET__)

void _CDECL _OS_setfileattr(const char *filename,unsigned attrib)
{ SetFileAttributes((LPSTR)filename, (DWORD)attrib); }

#elif   defined(__OS2__)

#define USE_LOCAL_FILEIO

#ifndef W_OK
#define W_OK                    0x02
#endif

void _CDECL _OS_setfileattr(const char *filename,unsigned attrib)
{
    FILESTATUS3 s;
    if (DosQueryPathInfo((PSZ)filename,FIL_STANDARD,(PVOID)&s,sizeof(s)))
	return;
    s.attrFile = attrib;
    DosSetPathInfo((PSZ)filename,FIL_STANDARD,(PVOID)&s,sizeof(s),0L);
}

/* <libc/file.h> stub functions */

#define BUF_SIZE    4096

/* Note: the implementation of the standard Unix-ish handle-based I/O isn't
 *       complete - but that wasn't the intent either. Note also that we
 *       don't presently support text file I/O, so all text files end
 *       up in Unix format (and are not translated!).
 */
int _CDECL stub_open(const char *_path, int _oflag, unsigned _mode)
{
    HFILE   handle;
    ULONG   error, actiontaken, openflag, openmode;
    char    path[PM_MAX_PATH];

    /* Determine open flags */
    if (_oflag & ___O_CREAT) {
	if (_oflag & ___O_EXCL)
	    openflag = OPEN_ACTION_FAIL_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW;
	else if (_oflag & ___O_TRUNC)
	    openflag = OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW;
	else
	    openflag = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW;
	}
    else if (_oflag & ___O_TRUNC)
	openflag = OPEN_ACTION_REPLACE_IF_EXISTS;
    else
	openflag = OPEN_ACTION_OPEN_IF_EXISTS;

    /* Determine open mode flags */
    if (_oflag & ___O_RDONLY)
	openmode = OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE;
    else if (_oflag & ___O_WRONLY)
	openmode = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE;
    else
	openmode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;

    /* Copy the path to a variable on the stack. We need to do this
     * for OS/2 as when the drivers are loaded into shared kernel
     * memory, we can't pass an address from that memory range to
     * this function.
     */
    strcpy(path,_path);
    if (DosOpen(path, &handle, &actiontaken, 0, FILE_NORMAL,
	    openflag, openmode, NULL) != NO_ERROR)
	return -1;

    /* Handle append mode of operation */
    if (_oflag & ___O_APPEND) {
	if (DosSetFilePtr(handle, 0, FILE_END, &error) != NO_ERROR)
	    return -1;
	}
    return handle;
}

int _CDECL stub_access(const char *_path, int _amode)
{
    char        path[PM_MAX_PATH];
    FILESTATUS  fs;

    /* Copy the path to a variable on the stack. We need to do this
     * for OS/2 as when the drivers are loaded into shared kernel
     * memory, we can't pass an address from that memory range to
     * this function.
     */
    strcpy(path,_path);
    if (DosQueryPathInfo(path, FIL_STANDARD, &fs, sizeof(fs)) != NO_ERROR)
	return -1;
    if ((_amode & W_OK) && (fs.attrFile & FILE_READONLY))
	return -1;
    return 0;
}

int _CDECL stub_close(int _fildes)
{
    if (DosClose(_fildes) != NO_ERROR)
	return -1;
    return 0;
}

off_t _CDECL stub_lseek(int _fildes, off_t _offset, int _whence)
{
    ULONG  cbActual, origin;

    switch (_whence) {
	case SEEK_CUR:
	    origin = FILE_CURRENT;
	    break;
	case SEEK_END:
	    origin = FILE_END;
	    break;
	default:
	    origin = FILE_BEGIN;
	}
    if (DosSetFilePtr(_fildes, _offset, origin, &cbActual) != NO_ERROR)
	return -1;
    return cbActual;
}

size_t _CDECL stub_read(int _fildes, void *_buf, size_t _nbyte)
{
    ULONG   cbActual = 0,cbRead;
    uchar   *p = _buf;
    uchar   file_io_buf[BUF_SIZE];

    /* We need to perform the physical read in chunks into a
     * a temporary static buffer, since the buffer passed in may be
     * in kernel space and will cause DosRead to bail internally.
     */
    while (_nbyte > BUF_SIZE) {
	if (DosRead(_fildes, file_io_buf, BUF_SIZE, &cbRead) != NO_ERROR)
	    return -1;
	cbActual += cbRead;
	memcpy(p,file_io_buf,BUF_SIZE);
	p += BUF_SIZE;
	_nbyte -= BUF_SIZE;
	}
    if (_nbyte) {
	if (DosRead(_fildes, file_io_buf, _nbyte, &cbRead) != NO_ERROR)
	    return -1;
	cbActual += cbRead;
	memcpy(p,file_io_buf,_nbyte);
	}
    return cbActual;
}

size_t _CDECL stub_write(int _fildes, const void *_buf, size_t _nbyte)
{
    ULONG   cbActual = 0,cbWrite;
    uchar   *p = (PVOID)_buf;
    uchar   file_io_buf[BUF_SIZE];

    /* We need to perform the physical write in chunks from a
     * a temporary static buffer, since the buffer passed in may be
     * in kernel space and will cause DosWrite to bail internally.
     */
    while (_nbyte > BUF_SIZE) {
	memcpy(file_io_buf,p,BUF_SIZE);
	if (DosWrite(_fildes, file_io_buf, BUF_SIZE, &cbWrite) != NO_ERROR)
	    return -1;
	cbActual += cbWrite;
	p += BUF_SIZE;
	_nbyte -= BUF_SIZE;
	}
    if (_nbyte) {
	memcpy(file_io_buf,p,_nbyte);
	if (DosWrite(_fildes, file_io_buf, _nbyte, &cbWrite) != NO_ERROR)
	    return -1;
	cbActual += cbWrite;
	}
    return cbActual;
}

int _CDECL stub_unlink(const char *_path)
{
    char    path[PM_MAX_PATH];

    /* Copy the path to a variable on the stack. We need to do this
     * for OS/2 as when the drivers are loaded into shared kernel
     * memory, we can't pass an address from that memory range to
     * this function.
     */
    strcpy(path,_path);
    if (DosDelete(path) != NO_ERROR)
	return -1;
    return 0;
}

int _CDECL stub_isatty(int _fildes)
{
    ULONG  htype, flags;

    if (DosQueryHType(_fildes, &htype, &flags) != NO_ERROR)
	return 0;
    return ((htype & 0xFF) == HANDTYPE_DEVICE);
}

/* <stdio.h> stub functions */
int _CDECL stub_remove(const char *_path)
{
    char    path[PM_MAX_PATH];

    /* Copy the path to a variable on the stack. We need to do this
     * for OS/2 as when the drivers are loaded into shared kernel
     * memory, we can't pass an address from that memory range to
     * this function.
     */
    strcpy(path,_path);
    if (DosDelete(path) != NO_ERROR)
	return -1;
    return 0;
}

int _CDECL stub_rename(const char *_old, const char *_new)
{
    char    old[PM_MAX_PATH];
    char    new[PM_MAX_PATH];

    /* Copy the path to a variable on the stack. We need to do this
     * for OS/2 as when the drivers are loaded into shared kernel
     * memory, we can't pass an address from that memory range to
     * this function.
     */
    strcpy(old,_old);
    strcpy(new,_new);
    if (DosMove(old, new) != NO_ERROR)
	return -1;
    return 0;
}

#else

void _CDECL _OS_setfileattr(const char *filename,unsigned attrib)
{ /* Unable to set hidden, system attributes on Unix. */ }

#endif

#ifndef USE_LOCAL_FILEIO

/* <libc/file.h> stub functions */
int _CDECL stub_open(const char *_path, int _oflag, unsigned _mode)
{
    int oflag_tab[] = {
	___O_RDONLY,    O_RDONLY,
	___O_WRONLY,    O_WRONLY,
	___O_RDWR,      O_RDWR,
	___O_BINARY,    O_BINARY,
	___O_TEXT,      O_TEXT,
	___O_CREAT,     O_CREAT,
	___O_EXCL,      O_EXCL,
	___O_TRUNC,     O_TRUNC,
	___O_APPEND,    O_APPEND,
	};
    int i,oflag = 0;

    /* Translate the oflag's to the OS dependent versions */
    for (i = 0; i < sizeof(oflag_tab) / sizeof(int); i += 2) {
	if (_oflag & oflag_tab[i])
	    oflag |= oflag_tab[i+1];
	}
    return open(_path,oflag,_mode);
}

int _CDECL stub_access(const char *_path, int _amode)
{ return access(_path,_amode); }

int _CDECL stub_close(int _fildes)
{ return close(_fildes); }

off_t _CDECL stub_lseek(int _fildes, off_t _offset, int _whence)
{ return lseek(_fildes,_offset,_whence); }

size_t _CDECL stub_read(int _fildes, void *_buf, size_t _nbyte)
{ return read(_fildes,_buf,_nbyte); }

int _CDECL stub_unlink(const char *_path)
{ return unlink(_path); }

size_t _CDECL stub_write(int _fildes, const void *_buf, size_t _nbyte)
{ return write(_fildes,_buf,_nbyte); }

int _CDECL stub_isatty(int _fildes)
{ return isatty(_fildes); }

/* <stdio.h> stub functions */
int _CDECL stub_remove(const char *_filename)
{ return remove(_filename); }

int _CDECL stub_rename(const char *_old, const char *_new)
{ return rename(_old,_new); }

#endif

#ifndef USE_LOCAL_GETDATE

/* Return the current date in days since 1/1/1980 */
ulong _CDECL _OS_getcurrentdate(void)
{
    struct tm refTime;
    refTime.tm_year = 80;
    refTime.tm_mon  = 0;
    refTime.tm_mday = 1;
    refTime.tm_hour = 0;
    refTime.tm_min  = 0;
    refTime.tm_sec  = 0;
    refTime.tm_isdst = -1;
    return (time(NULL) - mktime(&refTime)) / (24 * 3600L);
}

#endif

int _CDECL stub_raise(int sig)
{
#if defined(__WIN32_VXD__) || defined(__NT_DRIVER__) || defined(__SMX32__)
    return -1;
#else
    return raise(sig);
#endif
}

#ifdef __WINDOWS32__
typedef void (*__code_ptr)(int);
#else
typedef void (*__code_ptr)();
#endif

void * _CDECL stub_signal(int sig, void *handler)
{
#if defined(__WIN32_VXD__) || defined(__NT_DRIVER__) || defined(__SMX32__)
    return NULL;
#else
    return (void*)signal(sig,(__code_ptr)handler);
#endif
}
