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
* Description:  Module to implement a simple Portable Binary DLL loader
*               library. This library can be used to load PE DLL's under
*               any Intel based OS, provided the DLL's do not have any
*               imports in the import table.
*
*               NOTE: This loader module expects the DLL's to be built with
*                     Watcom C++ and may produce unexpected results with
*                     DLL's linked by another compiler.
*
****************************************************************************/

#include "drvlib/peloader.h"
#include "pmapi.h"
#include "drvlib/os/os.h"
#include "drvlib/libc/init.h"
#if (defined(__WINDOWS32__) || defined(__DRIVER__)) && defined(CHECKED)
#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#endif
#include "drvlib/pe.h"

/*--------------------------- Global variables ----------------------------*/

static int          result = PE_ok;

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
PARAMETERS:
f           - Handle to open file to read driver from
startOffset - Offset to the start of the driver within the file

RETURNS:
Handle to loaded PE DLL, or NULL on failure.

REMARKS:
This function loads a Portable Binary DLL library from disk, relocates
the code and returns a handle to the loaded library. This function is the
same as the regular PE_loadLibrary except that it take a handle to an
open file and an offset within that file for the DLL to load.
****************************************************************************/
static int PE_readHeader(
    FILE *f,
    long startOffset,
    FILE_HDR *filehdr,
    OPTIONAL_HDR *opthdr)
{
    EXE_HDR exehdr;
    ulong   offset,signature;

    /* Read the EXE header and check for valid header signature */
    result = PE_invalidDLLImage;
    fseek(f, startOffset, SEEK_SET);
    if (fread(&exehdr, 1, sizeof(exehdr), f) != sizeof(exehdr))
	return false;
    if (exehdr.signature != 0x5A4D)
	return false;

    /* Now seek to the start of the PE header defined at offset 0x3C
     * in the MS-DOS EXE header, and read the signature and check it.
     */
    fseek(f, startOffset+0x3C, SEEK_SET);
    if (fread(&offset, 1, sizeof(offset), f) != sizeof(offset))
	return false;
    fseek(f, startOffset+offset, SEEK_SET);
    if (fread(&signature, 1, sizeof(signature), f) != sizeof(signature))
	return false;
    if (signature != 0x00004550)
	return false;

    /* Now read the PE file header and check that it is correct */
    if (fread(filehdr, 1, sizeof(*filehdr), f) != sizeof(*filehdr))
	return false;
    if (filehdr->Machine != IMAGE_FILE_MACHINE_I386)
	return false;
    if (!(filehdr->Characteristics & IMAGE_FILE_32BIT_MACHINE))
	return false;
    if (!(filehdr->Characteristics & IMAGE_FILE_DLL))
	return false;
    if (fread(opthdr, 1, sizeof(*opthdr), f) != sizeof(*opthdr))
	return false;
    if (opthdr->Magic != 0x10B)
	return false;

    /* Success, so return true! */
    return true;
}

/****************************************************************************
PARAMETERS:
f           - Handle to open file to read driver from
startOffset - Offset to the start of the driver within the file

RETURNS:
Size of the DLL file on disk, or -1 on error

REMARKS:
This function scans the headers for a Portable Binary DLL to determine the
length of the DLL file on disk.
{secret}
****************************************************************************/
ulong PEAPI PE_getFileSize(
    FILE *f,
    ulong startOffset)
{
    FILE_HDR        filehdr;
    OPTIONAL_HDR    opthdr;
    SECTION_HDR     secthdr;
    ulong           size;
    int             i;

    /* Read the PE file headers from disk */
    if (!PE_readHeader(f,startOffset,&filehdr,&opthdr))
	return 0xFFFFFFFF;

    /* Scan all the section headers summing up the total size */
    size = opthdr.SizeOfHeaders;
    for (i = 0; i < filehdr.NumberOfSections; i++) {
	if (fread(&secthdr, 1, sizeof(secthdr), f) != sizeof(secthdr))
	    return 0xFFFFFFFF;
	size += secthdr.SizeOfRawData;
	}
    return size;
}

/****************************************************************************
DESCRIPTION:
Loads a Portable Binary DLL into memory from an open file

HEADER:
peloader.h

PARAMETERS:
f           - Handle to open file to read driver from
startOffset - Offset to the start of the driver within the file
size        - Place to store the size of the driver loaded
shared      - True to load module into shared memory

RETURNS:
Handle to loaded PE DLL, or NULL on failure.

REMARKS:
This function loads a Portable Binary DLL library from disk, relocates
the code and returns a handle to the loaded library. This function is the
same as the regular PE_loadLibrary except that it take a handle to an
open file and an offset within that file for the DLL to load.

SEE ALSO:
PE_loadLibrary, PE_getProcAddress, PE_freeLibrary
****************************************************************************/
PE_MODULE * PEAPI PE_loadLibraryExt(
    FILE *f,
    ulong startOffset,
    ulong *size,
    ibool shared)
{
    FILE_HDR        filehdr;
    OPTIONAL_HDR    opthdr;
    SECTION_HDR     secthdr;
    ulong           offset,pageOffset;
    ulong           text_off,text_addr,text_size;
    ulong           data_off,data_addr,data_size,data_end;
    ulong           export_off,export_addr,export_size,export_end;
    ulong           reloc_off,reloc_size;
    ulong           image_size;
    int             i,delta,numFixups;
    ushort          relocType,*fixup;
    PE_MODULE       *hMod = NULL;
    void            *reloc = NULL;
    BASE_RELOCATION *baseReloc;
    InitLibC_t      InitLibC;

    /* Read the PE file headers from disk */
    if (!PE_readHeader(f,startOffset,&filehdr,&opthdr))
	return NULL;

    /* Scan all the section headers and find the necessary sections */
    text_off = data_off = reloc_off = export_off = 0;
    text_addr = text_size = 0;
    data_addr = data_size = data_end = 0;
    export_addr = export_size = export_end = 0;
    reloc_size = 0;
    for (i = 0; i < filehdr.NumberOfSections; i++) {
	if (fread(&secthdr, 1, sizeof(secthdr), f) != sizeof(secthdr))
	    goto Error;
	if (strcmp(secthdr.Name, ".edata") == 0 || strcmp(secthdr.Name, ".rdata") == 0) {
	    /* Exports section */
	    export_off = secthdr.PointerToRawData;
	    export_addr = secthdr.VirtualAddress;
	    export_size = secthdr.SizeOfRawData;
	    export_end = export_addr + export_size;
	    }
	else if (strcmp(secthdr.Name, ".idata") == 0) {
	    /* Imports section, ignore */
	    }
	else if (strcmp(secthdr.Name, ".reloc") == 0) {
	    /* Relocations section */
	    reloc_off = secthdr.PointerToRawData;
	    reloc_size = secthdr.SizeOfRawData;
	    }
	else if (!text_off && secthdr.Characteristics & IMAGE_SCN_CNT_CODE) {
	    /* Code section */
	    text_off = secthdr.PointerToRawData;
	    text_addr = secthdr.VirtualAddress;
	    text_size = secthdr.SizeOfRawData;
	    }
	else if (!data_off && secthdr.Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) {
	    /* Data section */
	    data_off = secthdr.PointerToRawData;
	    data_addr = secthdr.VirtualAddress;
	    data_size = secthdr.SizeOfRawData;
	    data_end = data_addr + data_size;
	    }
	}

    /* Check to make sure that we have all the sections we need */
    if (!text_off || !data_off || !export_off || !reloc_off) {
	result = PE_invalidDLLImage;
	goto Error;
	}

    /* Find the size of the image to load allocate memory for it */
    image_size = MAX(export_end,data_end) - text_addr;
    *size = sizeof(PE_MODULE) + image_size + 4096;
    if (shared)
	hMod = PM_mallocShared(*size);
    else
	hMod = PM_malloc(*size);
    reloc = PM_malloc(reloc_size);
    if (!hMod || !reloc) {
	result = PE_outOfMemory;
	goto Error;
	}

    hMod->text = (uchar*)ROUND_4K((ulong)hMod + sizeof(PE_MODULE));
    hMod->data = (uchar*)((ulong)hMod->text + (data_addr - text_addr));
    hMod->export = (uchar*)((ulong)hMod->text + (export_addr - text_addr));
    hMod->textBase = text_addr;
    hMod->dataBase = data_addr;
    hMod->exportBase = export_addr;
    hMod->exportDir = opthdr.DataDirectory[0].RelVirtualAddress - export_addr;
    hMod->shared = shared;

    /* Now read the section images from disk */
    result = PE_invalidDLLImage;
    fseek(f, startOffset+text_off, SEEK_SET);
    if (fread(hMod->text, 1, text_size, f) != text_size)
	goto Error;
    fseek(f, startOffset+data_off, SEEK_SET);
    if (fread(hMod->data, 1, data_size, f) != data_size)
	goto Error;
    fseek(f, startOffset+export_off, SEEK_SET);
    if (fread(hMod->export, 1, export_size, f) != export_size)
	goto Error;
    fseek(f, startOffset+reloc_off, SEEK_SET);
    if (fread(reloc, 1, reloc_size, f) != reloc_size)
	goto Error;

    /* Now perform relocations on all sections in the image */
    delta = (ulong)hMod->text - opthdr.ImageBase - text_addr;
    baseReloc = (BASE_RELOCATION*)reloc;
    for (;;) {
	/* Check for termination condition */
	if (!baseReloc->PageRVA || !baseReloc->BlockSize)
	    break;

	/* Do fixups */
	pageOffset = baseReloc->PageRVA - hMod->textBase;
	numFixups = (baseReloc->BlockSize - sizeof(BASE_RELOCATION)) / sizeof(ushort);
	fixup = (ushort*)(baseReloc + 1);
	for (i = 0; i < numFixups; i++) {
	    relocType = *fixup >> 12;
	    if (relocType) {
		offset = pageOffset + (*fixup & 0x0FFF);
		*(ulong*)(hMod->text + offset) += delta;
		}
	    fixup++;
	    }

	/* Move to next relocation block */
	baseReloc = (BASE_RELOCATION*)((ulong)baseReloc + baseReloc->BlockSize);
	}

    /* Initialise the C runtime library for the loaded DLL */
    result = PE_unableToInitLibC;
    if ((InitLibC = (InitLibC_t)PE_getProcAddress(hMod,"_InitLibC")) == NULL)
	goto Error;
    if (!InitLibC(&___imports,PM_getOSType()))
	goto Error;

    /* Clean up, close the file and return the loaded module handle */
    PM_free(reloc);
    result = PE_ok;
    return hMod;

Error:
    if (shared)
	PM_freeShared(hMod);
    else
	PM_free(hMod);
    PM_free(reloc);
    return NULL;
}

/****************************************************************************
DESCRIPTION:
Loads a Portable Binary DLL into memory

HEADER:
peloader.h

PARAMETERS:
szDLLName   - Name of the PE DLL library to load
shared      - True to load module into shared memory

RETURNS:
Handle to loaded PE DLL, or NULL on failure.

REMARKS:
This function loads a Portable Binary DLL library from disk, relocates
the code and returns a handle to the loaded library. This function
will only work on DLL's that do not have any imports, since we don't
resolve import dependencies in this function.

SEE ALSO:
PE_getProcAddress, PE_freeLibrary
****************************************************************************/
PE_MODULE * PEAPI PE_loadLibrary(
    const char *szDLLName,
    ibool shared)
{
    PE_MODULE   *hMod;

#if (defined(__WINDOWS32__) || defined(__DRIVER__)) && defined(CHECKED)
    if (!shared) {
	PM_MODULE       hInst;
	InitLibC_t      InitLibC;

	/* For Win32 if are building checked libraries for debugging, we use
	 * the real Win32 DLL functions so that we can debug the resulting DLL
	 * files with the Win32 debuggers. Note that we can't do this if
	 * we need to load the files into a shared memory context.
	 */
	if ((hInst = PM_loadLibrary(szDLLName)) == NULL) {
	    result = PE_fileNotFound;
	    return NULL;
	    }

	/* Initialise the C runtime library for the loaded DLL */
	result = PE_unableToInitLibC;
	if ((InitLibC = (void*)PM_getProcAddress(hInst,"_InitLibC")) == NULL)
	    return NULL;
	if (!InitLibC(&___imports,PM_getOSType()))
	    return NULL;

	/* Allocate the PE_MODULE structure */
	if ((hMod = PM_malloc(sizeof(*hMod))) == NULL)
	    return NULL;
	hMod->text = (void*)hInst;
	hMod->shared = -1;

	/* DLL loaded successfully so return module handle */
	result = PE_ok;
	return hMod;
	}
    else
#endif
	{
	FILE        *f;
	ulong       size;

	/* Attempt to open the file on disk */
	if (shared < 0)
	    shared = 0;
	if ((f = fopen(szDLLName,"rb")) == NULL) {
	    result = PE_fileNotFound;
	    return NULL;
	    }
	hMod = PE_loadLibraryExt(f,0,&size,shared);
	fclose(f);
	return hMod;
	}
}

/****************************************************************************
DESCRIPTION:
Loads a Portable Binary DLL into memory

HEADER:
peloader.h

PARAMETERS:
szDLLName   - Name of the PE DLL library to load
shared      - True to load module into shared memory

RETURNS:
Handle to loaded PE DLL, or NULL on failure.

REMARKS:
This function is the same as the regular PE_loadLibrary function, except
that it looks for the drivers in the MGL_ROOT/drivers directory or a
/drivers directory relative to the current directory.

SEE ALSO:
PE_loadLibraryMGL, PE_getProcAddress, PE_freeLibrary
****************************************************************************/
PE_MODULE * PEAPI PE_loadLibraryMGL(
    const char *szDLLName,
    ibool shared)
{
#if !defined(__WIN32_VXD__) && !defined(__NT_DRIVER__)
    PE_MODULE   *hMod;
#endif
    char        path[256] = "";

    /* We look in the 'drivers' directory, optionally under the MGL_ROOT
     * environment variable directory.
     */
#if !defined(__WIN32_VXD__) && !defined(__NT_DRIVER__)
    if (getenv("MGL_ROOT")) {
	strcpy(path,getenv("MGL_ROOT"));
	PM_backslash(path);
	}
    strcat(path,"drivers");
    PM_backslash(path);
    strcat(path,szDLLName);
    if ((hMod = PE_loadLibrary(path,shared)) != NULL)
	return hMod;
#endif
    strcpy(path,"drivers");
    PM_backslash(path);
    strcat(path,szDLLName);
    return PE_loadLibrary(path,shared);
}

/****************************************************************************
DESCRIPTION:
Gets a function address from a Portable Binary DLL

HEADER:
peloader.h

PARAMETERS:
hModule     - Handle to a loaded PE DLL library
szProcName  - Name of the function to get the address of

RETURNS:
Pointer to the function, or NULL on failure.

REMARKS:
This function searches for the named, exported function in a loaded PE
DLL library, and returns the address of the function. If the function is
not found in the library, this function return NULL.

SEE ALSO:
PE_loadLibrary, PE_freeLibrary
****************************************************************************/
void * PEAPI PE_getProcAddress(
    PE_MODULE *hModule,
    const char *szProcName)
{
#if (defined(__WINDOWS32__) || defined(__DRIVER__)) && defined(CHECKED)
    if (hModule->shared == -1)
	return (void*)PM_getProcAddress(hModule->text,szProcName);
    else
#endif
	{
	uint                i;
	EXPORT_DIRECTORY    *exports;
	ulong               funcOffset;
	ulong               *AddressTable;
	ulong               *NameTable;
	ushort              *OrdinalTable;
	char                *name;

	/* Find the address of the export tables from the export section */
	if (!hModule)
	    return NULL;
	exports = (EXPORT_DIRECTORY*)(hModule->export + hModule->exportDir);
	AddressTable = (ulong*)(hModule->export + exports->AddressTableRVA - hModule->exportBase);
	NameTable = (ulong*)(hModule->export + exports->NameTableRVA - hModule->exportBase);
	OrdinalTable = (ushort*)(hModule->export + exports->OrdinalTableRVA - hModule->exportBase);

	/* Search the export name table to find the function name */
	for (i = 0; i < exports->NumberOfNamePointers; i++) {
	    name = (char*)(hModule->export + NameTable[i] - hModule->exportBase);
	    if (strcmp(name,szProcName) == 0)
		break;
	    }
	if (i == exports->NumberOfNamePointers)
	    return NULL;
	funcOffset = AddressTable[OrdinalTable[i]];
	if (!funcOffset)
	    return NULL;
	return (void*)(hModule->text + funcOffset - hModule->textBase);
	}
}

/****************************************************************************
DESCRIPTION:
Frees a loaded Portable Binary DLL

HEADER:
peloader.h

PARAMETERS:
hModule     - Handle to a loaded PE DLL library to free

REMARKS:
This function frees a loaded PE DLL library from memory.

SEE ALSO:
PE_getProcAddress, PE_loadLibrary
****************************************************************************/
void PEAPI PE_freeLibrary(
    PE_MODULE *hModule)
{
    TerminateLibC_t TerminateLibC;

#if (defined(__WINDOWS32__) || defined(__DRIVER__)) && defined(CHECKED)
    if (hModule->shared == -1) {
	/* Run the C runtime library exit code on module unload */
	if ((TerminateLibC = (TerminateLibC_t)PM_getProcAddress(hModule->text,"_TerminateLibC")) != NULL)
	    TerminateLibC();
	PM_freeLibrary(hModule->text);
	PM_free(hModule);
	}
    else
#endif
	{
	if (hModule) {
	    /* Run the C runtime library exit code on module unload */
	    if ((TerminateLibC = (TerminateLibC_t)PE_getProcAddress(hModule,"_TerminateLibC")) != NULL)
		TerminateLibC();
	    if (hModule->shared)
		PM_freeShared(hModule);
	    else
		PM_free(hModule);
	    }
	}
}

/****************************************************************************
DESCRIPTION:
Returns the error code for the last operation

HEADER:
peloader.h

RETURNS:
Error code for the last operation.

SEE ALSO:
PE_getProcAddress, PE_loadLibrary
****************************************************************************/
int PEAPI PE_getError(void)
{
    return result;
}
