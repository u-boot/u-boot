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
* Environment:  32-bit Windows NT device drivers.
*
* Description:  Implementation for the NT driver memory management functions
*               for the PM library.
*
****************************************************************************/

#include "pmapi.h"
#include "drvlib/os/os.h"
#include "sdd/sddhelp.h"
#include "mtrr.h"
#include "oshdr.h"

/*--------------------------- Global variables ----------------------------*/

#define MAX_MEMORY_SHARED           100
#define MAX_MEMORY_MAPPINGS         100
#define MAX_MEMORY_LOCKED           100

typedef struct {
    void    *linear;
    ulong   length;
    PMDL    pMdl;
    } memshared;

typedef struct {
    void    *linear;
    void    *mmIoMapped;
    ulong   length;
    PMDL    pMdl;
    } memlocked;

typedef struct {
    ulong   physical;
    ulong   linear;
    ulong   length;
    ibool   isCached;
    } mmapping;

static int          numMappings = 0;
static memshared    shared[MAX_MEMORY_MAPPINGS] = {0};
static mmapping     maps[MAX_MEMORY_MAPPINGS];
static memlocked    locked[MAX_MEMORY_LOCKED];

/*----------------------------- Implementation ----------------------------*/

ulong   PMAPI _PM_getPDB(void);

/* Page table entry flags */

#define PAGE_FLAGS_PRESENT			0x00000001
#define PAGE_FLAGS_WRITEABLE		0x00000002
#define PAGE_FLAGS_USER				0x00000004
#define PAGE_FLAGS_WRITE_THROUGH	0x00000008
#define PAGE_FLAGS_CACHE_DISABLE	0x00000010
#define PAGE_FLAGS_ACCESSED			0x00000020
#define PAGE_FLAGS_DIRTY			0x00000040
#define PAGE_FLAGS_4MB	            0x00000080

/****************************************************************************
PARAMETERS:
base        - Physical base address of the memory to maps in
limit       - Limit of physical memory to region to maps in

RETURNS:
Linear address of the newly mapped memory.

REMARKS:
Maps a physical memory range to a linear memory range.
****************************************************************************/
static ulong _PM_mapPhysicalToLinear(
    ulong base,
    ulong limit,
    ibool isCached)
{
    ulong               length = limit+1;
    PHYSICAL_ADDRESS    paIoBase = {0};

    /* NT loves large Ints */
    paIoBase = RtlConvertUlongToLargeInteger( base );

    /* Map IO space into Kernel */
    if (isCached)
	return (ULONG)MmMapIoSpace(paIoBase, length, MmCached );
    else
	return (ULONG)MmMapIoSpace(paIoBase, length, MmNonCached );
}

/****************************************************************************
REMARKS:
Adjust the page table caching bits directly. Requires ring 0 access and
only works with DOS4GW and compatible extenders (CauseWay also works since
it has direct support for the ring 0 instructions we need from ring 3). Will
not work in a DOS box, but we call into the ring 0 helper VxD so we should
never get here in a DOS box anyway (assuming the VxD is present). If we
do get here and we are in windows, this code will be skipped.
****************************************************************************/
static void _PM_adjustPageTables(
    ulong linear,
    ulong limit,
	ibool isGlobal,
    ibool isCached)
{
    int     startPDB,endPDB,iPDB,startPage,endPage,start,end,iPage;
    ulong   pageTable,*pPDB,*pPageTable;
	ulong	mask = 0xFFFFFFFF;
	ulong	bits = 0x00000000;

	/* Enable user level access for page table entry */
	if (isGlobal) {
		mask &= ~PAGE_FLAGS_USER;
		bits |= PAGE_FLAGS_USER;
		}

	/* Disable PCD bit if page table entry should be uncached */
	if (!isCached) {
		mask &= ~(PAGE_FLAGS_CACHE_DISABLE | PAGE_FLAGS_WRITE_THROUGH);
		bits |= (PAGE_FLAGS_CACHE_DISABLE | PAGE_FLAGS_WRITE_THROUGH);
		}

    pPDB = (ulong*)_PM_mapPhysicalToLinear(_PM_getPDB(),0xFFF,true);
    if (pPDB) {
	startPDB = (linear >> 22) & 0x3FF;
	startPage = (linear >> 12) & 0x3FF;
	endPDB = ((linear+limit) >> 22) & 0x3FF;
	endPage = ((linear+limit) >> 12) & 0x3FF;
	for (iPDB = startPDB; iPDB <= endPDB; iPDB++) {
	    /* Set the bits in the page directory entry - required as per */
	    /* Pentium 4 manual. This also takes care of the 4MB page entries */
	    pPDB[iPDB] = (pPDB[iPDB] & mask) | bits;
	    if (!(pPDB[iPDB] & PAGE_FLAGS_4MB)) {
		/* If we are dealing with 4KB pages then we need to iterate */
		/* through each of the page table entries */
		pageTable = pPDB[iPDB] & ~0xFFF;
		pPageTable = (ulong*)_PM_mapPhysicalToLinear(pageTable,0xFFF,true);
		start = (iPDB == startPDB) ? startPage : 0;
		end = (iPDB == endPDB) ? endPage : 0x3FF;
		for (iPage = start; iPage <= end; iPage++) {
		    pPageTable[iPage] = (pPageTable[iPage] & mask) | bits;
		    }
		MmUnmapIoSpace(pPageTable,0xFFF);
		}
	    }
	MmUnmapIoSpace(pPDB,0xFFF);
	PM_flushTLB();
	}
}

/****************************************************************************
REMARKS:
Allocate a block of shared memory. For NT we allocate shared memory
as locked, global memory that is accessible from any memory context
(including interrupt time context), which allows us to load our important
data structure and code such that we can access it directly from a ring
0 interrupt context.
****************************************************************************/
void * PMAPI PM_mallocShared(
    long size)
{
    int         i;

    /* First find a free slot in our shared memory table */
    for (i = 0; i < MAX_MEMORY_SHARED; i++) {
	if (shared[i].linear == 0)
	    break;
	}
    if (i == MAX_MEMORY_SHARED)
	return NULL;

    /* Allocate the paged pool */
    shared[i].linear = ExAllocatePool(PagedPool, size);

    /* Create a list to manage this allocation */
    shared[i].pMdl = IoAllocateMdl(shared[i].linear,size,FALSE,FALSE,(PIRP) NULL);

    /* Lock this allocation in memory */
    MmProbeAndLockPages(shared[i].pMdl,KernelMode,IoModifyAccess);

    /* Modify bits to grant user access */
    _PM_adjustPageTables((ulong)shared[i].linear, size, true, true);
    return (void*)shared[i].linear;
}

/****************************************************************************
REMARKS:
Free a block of shared memory
****************************************************************************/
void PMAPI PM_freeShared(
    void *p)
{
    int i;

    /* Find a shared memory block in our table and free it */
    for (i = 0; i < MAX_MEMORY_SHARED; i++) {
	if (shared[i].linear == p) {
	    /* Unlock what we locked */
	    MmUnlockPages(shared[i].pMdl);

	    /* Free our MDL */
	    IoFreeMdl(shared[i].pMdl);

	    /* Free our mem */
	    ExFreePool(shared[i].linear);

	    /* Flag that is entry is available */
	    shared[i].linear = 0;
	    break;
	    }
	}
}

/****************************************************************************
REMARKS:
Map a physical address to a linear address in the callers process.
****************************************************************************/
void * PMAPI PM_mapPhysicalAddr(
    ulong base,
    ulong limit,
    ibool isCached)
{
    ulong   linear,length = limit+1;
    int     i;

    /* Search table of existing mappings to see if we have already mapped */
    /* a region of memory that will serve this purpose. */
    for (i = 0; i < numMappings; i++) {
	if (maps[i].physical == base && maps[i].length == length && maps[i].isCached == isCached) {
	    _PM_adjustPageTables((ulong)maps[i].linear, maps[i].length, true, isCached);
	    return (void*)maps[i].linear;
	    }
	}
    if (numMappings == MAX_MEMORY_MAPPINGS)
	return NULL;

    /* We did not find any previously mapped memory region, so maps it in. */
    if ((linear = _PM_mapPhysicalToLinear(base,limit,isCached)) == 0xFFFFFFFF)
	return NULL;
    maps[numMappings].physical = base;
    maps[numMappings].length = length;
    maps[numMappings].linear = linear;
    maps[numMappings].isCached = isCached;
    numMappings++;

    /* Grant user access to this I/O space */
    _PM_adjustPageTables((ulong)linear, length, true, isCached);
    return (void*)linear;
}

/****************************************************************************
REMARKS:
Free a physical address mapping allocated by PM_mapPhysicalAddr.
****************************************************************************/
void PMAPI PM_freePhysicalAddr(
    void *ptr,
    ulong limit)
{
    /* We don't free the memory mappings in here because we cache all */
    /* the memory mappings we create in the system for later use. */
}

/****************************************************************************
REMARKS:
Called when the device driver unloads to free all the page table mappings!
****************************************************************************/
void PMAPI _PM_freeMemoryMappings(void)
{
    int i;

    for (i = 0; i < numMappings; i++)
	MmUnmapIoSpace((void *)maps[i].linear,maps[i].length);
}

/****************************************************************************
REMARKS:
Find the physical address of a linear memory address in current process.
****************************************************************************/
ulong PMAPI PM_getPhysicalAddr(
    void *p)
{
    PHYSICAL_ADDRESS    paOurAddress;

    paOurAddress = MmGetPhysicalAddress(p);
    return paOurAddress.LowPart;
}

/****************************************************************************
REMARKS:
Find the physical address of a linear memory address in current process.
****************************************************************************/
ibool PMAPI PM_getPhysicalAddrRange(
    void *p,
    ulong length,
    ulong *physAddress)
{
    int     i;
    ulong   linear = (ulong)p & ~0xFFF;

    for (i = (length + 0xFFF) >> 12; i > 0; i--) {
	if ((*physAddress++ = PM_getPhysicalAddr((void*)linear)) == 0xFFFFFFFF)
	    return false;
	linear += 4096;
	}
    return true;
}

/****************************************************************************
REMARKS:
Allocates a block of locked physical memory.
****************************************************************************/
void * PMAPI PM_allocLockedMem(
    uint size,
    ulong *physAddr,
    ibool contiguous,
    ibool below16M)
{
    int                 i;
    PHYSICAL_ADDRESS    paOurAddress;

    /* First find a free slot in our shared memory table */
    for (i = 0; i < MAX_MEMORY_LOCKED; i++) {
	if (locked[i].linear == 0)
	    break;
	}
    if (i == MAX_MEMORY_LOCKED)
	return NULL;

    /* HighestAcceptableAddress - Specifies the highest valid physical address */
    /* the driver can use. For example, if a device can only reference physical */
    /* memory in the lower 16MB, this value would be set to 0x00000000FFFFFF. */
    paOurAddress.HighPart = 0;
    if (below16M)
	paOurAddress.LowPart = 0x00FFFFFF;
    else
	paOurAddress.LowPart = 0xFFFFFFFF;

    if (contiguous) {
	/* Allocate from the non-paged pool (unfortunately 4MB pages) */
	locked[i].linear = MmAllocateContiguousMemory(size, paOurAddress);
	if (!locked[i].linear)
	    return NULL;

	/* Flag no MDL */
	locked[i].pMdl = NULL;

	/* Map the physical address for the memory so we can manage */
	/* the page tables in 4KB chunks mapped into user space. */

	/* TODO: Map this with the physical address to the linear addresss */
	locked[i].mmIoMapped = locked[i].linear;

	/* Modify bits to grant user access, flag not cached */
	_PM_adjustPageTables((ulong)locked[i].mmIoMapped, size, true, false);
	return (void*)locked[i].mmIoMapped;
	}
    else {
	/* Allocate from the paged pool */
	locked[i].linear = ExAllocatePool(PagedPool, size);
	if (!locked[i].linear)
	    return NULL;

	/* Create a list to manage this allocation */
	locked[i].pMdl = IoAllocateMdl(locked[i].linear,size,FALSE,FALSE,(PIRP) NULL);

	/* Lock this allocation in memory */
	MmProbeAndLockPages(locked[i].pMdl,KernelMode,IoModifyAccess);

	/* Modify bits to grant user access, flag not cached */
	_PM_adjustPageTables((ulong)locked[i].linear, size, true, false);
	return (void*)locked[i].linear;
	}
}

/****************************************************************************
REMARKS:
Frees a block of locked physical memory.
****************************************************************************/
void PMAPI PM_freeLockedMem(
    void *p,
    uint size,
    ibool contiguous)
{
    int i;

    /* Find a locked memory block in our table and free it */
    for (i = 0; i < MAX_MEMORY_LOCKED; i++) {
	if (locked[i].linear == p) {
	    /* An Mdl indicates that we used the paged pool, and locked it, */
	    /* so now we have to unlock, free the MDL, and free paged */
	    if (locked[i].pMdl) {
		/* Unlock what we locked and free the Mdl */
		MmUnlockPages(locked[i].pMdl);
		IoFreeMdl(locked[i].pMdl);
		ExFreePool(locked[i].linear);
		}
	    else {
		/* TODO: Free the mmIoMap mapping for the memory! */

		/* Free non-paged pool */
		MmFreeContiguousMemory(locked[i].linear);
		}

	    /* Flag that is entry is available */
	    locked[i].linear = 0;
	    break;
	    }
	}
}

/****************************************************************************
REMARKS:
Allocates a page aligned and page sized block of memory
****************************************************************************/
void * PMAPI PM_allocPage(
    ibool locked)
{
    /* Allocate the memory from the non-paged pool if we want the memory */
    /* to be locked. */
    return ExAllocatePool(
	locked ? NonPagedPoolCacheAligned : PagedPoolCacheAligned,
	PAGE_SIZE);
}

/****************************************************************************
REMARKS:
Free a page aligned and page sized block of memory
****************************************************************************/
void PMAPI PM_freePage(
    void *p)
{
    if (p) ExFreePool(p);
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockDataPages(
    void *p,
    uint len,
    PM_lockHandle *lh)
{
    MDL *pMdl;

    /* Create a list to manage this allocation */
    if ((pMdl = IoAllocateMdl(p,len,FALSE,FALSE,(PIRP)NULL)) == NULL)
	return false;

    /* Lock this allocation in memory */
    MmProbeAndLockPages(pMdl,KernelMode,IoModifyAccess);
    *((PMDL*)(&lh->h)) = pMdl;
    return true;
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockDataPages(
    void *p,
    uint len,
    PM_lockHandle *lh)
{
    if (p && lh) {
	/* Unlock what we locked */
	MDL *pMdl = *((PMDL*)(&lh->h));
	MmUnlockPages(pMdl);
	IoFreeMdl(pMdl);
	}
    return true;
}

/****************************************************************************
REMARKS:
Lock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_lockCodePages(
    void (*p)(),
    uint len,
    PM_lockHandle *lh)
{
    return PM_lockDataPages((void*)p,len,lh);
}

/****************************************************************************
REMARKS:
Unlock linear memory so it won't be paged.
****************************************************************************/
int PMAPI PM_unlockCodePages(
    void (*p)(),
    uint len,
    PM_lockHandle *lh)
{
    return PM_unlockDataPages((void*)p,len,lh);
}
