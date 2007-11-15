/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * yaffscfg2k.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

/* XXX U-BOOT XXX */
#include <common.h>

#include "yaffscfg.h"
#include "yaffsfs.h"
#include "yaffs_fileem2k.h"
#include "yaffs_nandemul2k.h"

#include <errno.h>

unsigned yaffs_traceMask = 

	YAFFS_TRACE_SCAN |  
	YAFFS_TRACE_GC | YAFFS_TRACE_GC_DETAIL | 
	YAFFS_TRACE_ERASE | 
	YAFFS_TRACE_TRACING | 
	YAFFS_TRACE_ALLOCATE | 
	YAFFS_TRACE_CHECKPOINT |
	YAFFS_TRACE_BAD_BLOCKS |
	YAFFS_TRACE_VERIFY |
	YAFFS_TRACE_VERIFY_NAND |
	YAFFS_TRACE_VERIFY_FULL |
//	(~0) |
	
	0;
        


void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	errno = err;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}


static int yaffs_kill_alloc = 0;
static size_t total_malloced = 0;
static size_t malloc_limit = 0 & 6000000;

void *yaffs_malloc(size_t size)
{
	size_t this;
	if(yaffs_kill_alloc)
		return NULL;
	if(malloc_limit && malloc_limit <(total_malloced + size) )
		return NULL;

	this = malloc(size);
	if(this)
		total_malloced += size;
	return this;
}

void yaffs_free(void *ptr)
{
	free(ptr);
}

void yaffsfs_LocalInitialisation(void)
{
	// Define locking semaphore.
}

// Configuration for:
// /ram  2MB ramdisk
// /boot 2MB boot disk (flash)
// /flash 14MB flash disk (flash)
// NB Though /boot and /flash occupy the same physical device they
// are still disticnt "yaffs_Devices. You may think of these as "partitions"
// using non-overlapping areas in the same device.
// 

#include "yaffs_ramdisk.h"
#include "yaffs_flashif.h"
#include "yaffs_nandemul2k.h"

static yaffs_Device ramDev;
static yaffs_Device bootDev;
static yaffs_Device flashDev;
static yaffs_Device ram2kDev;

static yaffsfs_DeviceConfiguration yaffsfs_config[] = {
#if 0
	{ "/ram", &ramDev},
	{ "/boot", &bootDev},
	{ "/flash/", &flashDev},
	{ "/ram2k", &ram2kDev},
	{(void *)0,(void *)0}
#else
	{ "/", &ramDev},
	{ "/flash/boot", &bootDev},
	{ "/flash/flash", &flashDev},
	{ "/ram2k", &ram2kDev},
	{(void *)0,(void *)0} /* Null entry to terminate list */
#endif
};


int yaffs_StartUp(void)
{
	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_LocalInitialisation();
	
	// Set up devices
	// /ram
	memset(&ramDev,0,sizeof(ramDev));
	ramDev.nDataBytesPerChunk = 512;
	ramDev.nChunksPerBlock = 32;
	ramDev.nReservedBlocks = 2; // Set this smaller for RAM
	ramDev.startBlock = 0; // Can use block 0
	ramDev.endBlock = 127; // Last block in 2MB.	
	//ramDev.useNANDECC = 1;
	ramDev.nShortOpCaches = 0;	// Disable caching on this device.
	ramDev.genericDevice = (void *) 0;	// Used to identify the device in fstat.
	ramDev.writeChunkWithTagsToNAND = yramdisk_WriteChunkWithTagsToNAND;
	ramDev.readChunkWithTagsFromNAND = yramdisk_ReadChunkWithTagsFromNAND;
	ramDev.eraseBlockInNAND = yramdisk_EraseBlockInNAND;
	ramDev.initialiseNAND = yramdisk_InitialiseNAND;

	// /boot
	memset(&bootDev,0,sizeof(bootDev));
	bootDev.nDataBytesPerChunk = 512;
	bootDev.nChunksPerBlock = 32;
	bootDev.nReservedBlocks = 5;
	bootDev.startBlock = 0; // Can use block 0
	bootDev.endBlock = 63; // Last block
	//bootDev.useNANDECC = 0; // use YAFFS's ECC
	bootDev.nShortOpCaches = 10; // Use caches
	bootDev.genericDevice = (void *) 1;	// Used to identify the device in fstat.
	bootDev.writeChunkWithTagsToNAND = yflash_WriteChunkWithTagsToNAND;
	bootDev.readChunkWithTagsFromNAND = yflash_ReadChunkWithTagsFromNAND;
	bootDev.eraseBlockInNAND = yflash_EraseBlockInNAND;
	bootDev.initialiseNAND = yflash_InitialiseNAND;
	bootDev.markNANDBlockBad = yflash_MarkNANDBlockBad;
	bootDev.queryNANDBlock = yflash_QueryNANDBlock;



	// /flash
	// Set this puppy up to use
	// the file emulation space as
	// 2kpage/64chunk per block/128MB device
	memset(&flashDev,0,sizeof(flashDev));

	flashDev.nDataBytesPerChunk = 2048;
	flashDev.nChunksPerBlock = 64;
	flashDev.nReservedBlocks = 5;
	flashDev.nCheckpointReservedBlocks = 5;
	//flashDev.checkpointStartBlock = 1;
	//flashDev.checkpointEndBlock = 20;
	flashDev.startBlock = 0;
	flashDev.endBlock = 200; // Make it smaller
	//flashDev.endBlock = yflash_GetNumberOfBlocks()-1;
	flashDev.isYaffs2 = 1;
	flashDev.wideTnodesDisabled=0;
	flashDev.nShortOpCaches = 10; // Use caches
	flashDev.genericDevice = (void *) 2;	// Used to identify the device in fstat.
	flashDev.writeChunkWithTagsToNAND = yflash_WriteChunkWithTagsToNAND;
	flashDev.readChunkWithTagsFromNAND = yflash_ReadChunkWithTagsFromNAND;
	flashDev.eraseBlockInNAND = yflash_EraseBlockInNAND;
	flashDev.initialiseNAND = yflash_InitialiseNAND;
	flashDev.markNANDBlockBad = yflash_MarkNANDBlockBad;
	flashDev.queryNANDBlock = yflash_QueryNANDBlock;

	// /ram2k
	// Set this puppy up to use
	// the file emulation space as
	// 2kpage/64chunk per block/128MB device
	memset(&ram2kDev,0,sizeof(ram2kDev));

	ram2kDev.nDataBytesPerChunk = nandemul2k_GetBytesPerChunk();
	ram2kDev.nChunksPerBlock = nandemul2k_GetChunksPerBlock();
	ram2kDev.nReservedBlocks = 5;
	ram2kDev.startBlock = 0; // First block after /boot
	//ram2kDev.endBlock = 127; // Last block in 16MB
	ram2kDev.endBlock = nandemul2k_GetNumberOfBlocks() - 1; // Last block in 512MB
	ram2kDev.isYaffs2 = 1;
	ram2kDev.nShortOpCaches = 10; // Use caches
	ram2kDev.genericDevice = (void *) 3;	// Used to identify the device in fstat.
	ram2kDev.writeChunkWithTagsToNAND = nandemul2k_WriteChunkWithTagsToNAND;
	ram2kDev.readChunkWithTagsFromNAND = nandemul2k_ReadChunkWithTagsFromNAND;
	ram2kDev.eraseBlockInNAND = nandemul2k_EraseBlockInNAND;
	ram2kDev.initialiseNAND = nandemul2k_InitialiseNAND;
	ram2kDev.markNANDBlockBad = nandemul2k_MarkNANDBlockBad;
	ram2kDev.queryNANDBlock = nandemul2k_QueryNANDBlock;

	yaffs_initialise(yaffsfs_config);
	
	return 0;
}



void SetCheckpointReservedBlocks(int n)
{
	flashDev.nCheckpointReservedBlocks = n;
}
