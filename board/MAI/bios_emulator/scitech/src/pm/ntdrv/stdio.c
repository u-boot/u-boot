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
* Description:  C library compatible I/O functions for use within a Windows
*               NT driver.
*
****************************************************************************/

#include "pmapi.h"
#include "oshdr.h"

/*------------------------ Main Code Implementation -----------------------*/

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C fopen function.
****************************************************************************/
FILE * fopen(
    const char *filename,
    const char *mode)
{
    ACCESS_MASK                 DesiredAccess;      /* for ZwCreateFile... */
    OBJECT_ATTRIBUTES           ObjectAttributes;
    ULONG                       ShareAccess;
    ULONG                       CreateDisposition;
    NTSTATUS                    status;
    HANDLE                      FileHandle;
    UNICODE_STRING              *uniFile = NULL;
    PWCHAR                      bufFile = NULL;
    IO_STATUS_BLOCK             IoStatusBlock;
    FILE_STANDARD_INFORMATION   FileInformation;
    FILE_POSITION_INFORMATION   FilePosition;
    char                        kernelFilename[PM_MAX_PATH+5];
    FILE                        *f;

    /* Add prefix for addressing the file system. "\??\" is short for "\DosDevices\" */
    strcpy(kernelFilename, "\\??\\");
    strcat(kernelFilename, filename);
    if ((f = PM_malloc(sizeof(FILE))) == NULL)
	goto Error;
    f->offset = 0;
    f->text = (mode[1] == 't' || mode[2] == 't');
    f->writemode = (mode[0] == 'w') || (mode[0] == 'a');
    if (mode[0] == 'r') {
	/* omode = OPEN_ACCESS_READONLY | OPEN_SHARE_COMPATIBLE; */
	/* action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_FAIL; */
	DesiredAccess = GENERIC_READ;
	ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
	CreateDisposition = FILE_OPEN;
	}
    else if (mode[0] == 'w') {
	/* omode = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_COMPATIBLE; */
	/* action = ACTION_IFEXISTS_TRUNCATE | ACTION_IFNOTEXISTS_CREATE; */
	DesiredAccess = GENERIC_WRITE;
	ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
	CreateDisposition = FILE_SUPERSEDE;
	}
    else {
	/* omode = OPEN_ACCESS_READWRITE | OPEN_SHARE_COMPATIBLE; */
	/* action = ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_CREATE; */
	DesiredAccess = GENERIC_READ | GENERIC_WRITE;
	ShareAccess = FILE_SHARE_READ;
	CreateDisposition = FILE_OPEN_IF;
	}

    /* Convert filename string to ansi string and then to UniCode string */
    if ((uniFile = _PM_CStringToUnicodeString(kernelFilename)) == NULL)
	return NULL;

    /* Create the file */
    InitializeObjectAttributes (&ObjectAttributes,
				uniFile,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL);
    status = ZwCreateFile( &FileHandle,
			    DesiredAccess | SYNCHRONIZE,
			    &ObjectAttributes,
			    &IoStatusBlock,
			    NULL,                   /* AllocationSize  OPTIONAL, */
			    FILE_ATTRIBUTE_NORMAL,
			    ShareAccess,
			    CreateDisposition,
			    FILE_RANDOM_ACCESS,     /* CreateOptions, */
			    NULL,                   /* EaBuffer  OPTIONAL, */
			    0                       /* EaLength (required if EaBuffer) */
			    );
    if (!NT_SUCCESS (status))
	goto Error;
    f->handle = (int)FileHandle;

    /* Determine size of the file */
    status = ZwQueryInformationFile(  FileHandle,
				      &IoStatusBlock,
				      &FileInformation,
				      sizeof(FILE_STANDARD_INFORMATION),
				      FileStandardInformation
				      );
    if (!NT_SUCCESS (status))
	goto Error;
    f->filesize = FileInformation.EndOfFile.LowPart;

    /* Move to the end of the file if we are appending */
    if (mode[0] == 'a') {
	FilePosition.CurrentByteOffset.HighPart = 0;
	FilePosition.CurrentByteOffset.LowPart = f->filesize;
	status = ZwSetInformationFile(  FileHandle,
					&IoStatusBlock,
					&FilePosition,
					sizeof(FILE_POSITION_INFORMATION),
					FilePositionInformation
					);
	if (!NT_SUCCESS (status))
	    goto Error;
	}
    return f;

Error:
    if (f) PM_free(f);
    if (uniFile) _PM_FreeUnicodeString(uniFile);
    return NULL;
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C fread function.
****************************************************************************/
size_t fread(
    void *ptr,
    size_t size,
    size_t n,
    FILE *f)
{
    NTSTATUS        status;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER   ByteOffset;

    /* Read any extra bytes from the file */
    ByteOffset.HighPart = 0;
    ByteOffset.LowPart = f->offset;
    status = ZwReadFile( (HANDLE)f->handle,
			 NULL,              /*IN HANDLE  Event  OPTIONAL, */
			 NULL,              /*  IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL, */
			 NULL,              /*  IN PVOID  ApcContext  OPTIONAL, */
			 &IoStatusBlock,
			 ptr,               /*  OUT PVOID  Buffer, */
			 size * n,          /*IN ULONG  Length, */
			 &ByteOffset,       /*OPTIONAL, */
			 NULL               /*IN PULONG  Key  OPTIONAL */
			 );
    if (!NT_SUCCESS (status))
	return 0;
    f->offset += IoStatusBlock.Information;
    return IoStatusBlock.Information / size;
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C fwrite function.
****************************************************************************/
size_t fwrite(
    const void *ptr,
    size_t size,
    size_t n,
    FILE *f)
{
    NTSTATUS        status;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER   ByteOffset;

    if (!f->writemode)
	return 0;
    ByteOffset.HighPart = 0;
    ByteOffset.LowPart = f->offset;
    status = ZwWriteFile( (HANDLE)f->handle,
			  NULL,             /*IN HANDLE  Event  OPTIONAL, */
			  NULL,             /*  IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL, */
			  NULL,             /*  IN PVOID  ApcContext  OPTIONAL, */
			  &IoStatusBlock,
			  (void*)ptr,       /*  OUT PVOID  Buffer, */
			  size * n,         /*IN ULONG  Length, */
			  &ByteOffset,      /*OPTIONAL, */
			  NULL              /*IN PULONG  Key  OPTIONAL */
			  );
    if (!NT_SUCCESS (status))
	return 0;
    f->offset += IoStatusBlock.Information;
    if (f->offset > f->filesize)
	f->filesize = f->offset;
    return IoStatusBlock.Information / size;
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C fflush function.
****************************************************************************/
int fflush(
    FILE *f)
{
    /* Nothing to do here as we are not doing buffered I/O */
    (void)f;
    return 0;
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C fseek function.
****************************************************************************/
int fseek(
    FILE *f,
    long int offset,
    int whence)
{
    NTSTATUS                    status;
    FILE_POSITION_INFORMATION   FilePosition;
    IO_STATUS_BLOCK             IoStatusBlock;

    if (whence == 0)
	f->offset = offset;
    else if (whence == 1)
	f->offset += offset;
    else if (whence == 2)
	f->offset = f->filesize + offset;
    FilePosition.CurrentByteOffset.HighPart = 0;
    FilePosition.CurrentByteOffset.LowPart = f->offset;
    status = ZwSetInformationFile( (HANDLE)f->handle,
				   &IoStatusBlock,
				   &FilePosition,
				   sizeof(FILE_POSITION_INFORMATION),
				   FilePositionInformation
				   );
    if (!NT_SUCCESS (status))
	return -1;
    return 0;
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C ftell function.
****************************************************************************/
long ftell(
    FILE *f)
{
    return f->offset;
}

/****************************************************************************
REMARKS:
NT driver implementation of the ANSI C feof function.
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
NT driver implementation of the ANSI C fclose function.
****************************************************************************/
int fclose(
    FILE *f)
{
    ZwClose((HANDLE)f->handle);
    PM_free(f);
    return 0;
}
