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
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

/* XXX U-BOOT XXX */
#include <common.h>

#include <config.h>
#include "nand.h"
#include "yaffscfg.h"
#include "yaffsfs.h"
#include "yaffs_packedtags2.h"
#include "yaffs_mtdif.h"
#include "yaffs_mtdif2.h"
#if 0
#include <errno.h>
#else
#include "malloc.h"
#endif

unsigned yaffs_traceMask = 0x0; /* Disable logging */
static int yaffs_errno = 0;

void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	yaffs_errno = err;
}

int yaffsfs_GetError(void)
{
	return yaffs_errno;
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

void *yaffs_malloc(size_t size)
{
	return malloc(size);
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

static int isMounted = 0;
#define MOUNT_POINT "/flash"
extern nand_info_t nand_info[];

/* XXX U-BOOT XXX */
#if 0
static yaffs_Device ramDev;
static yaffs_Device bootDev;
static yaffs_Device flashDev;
#endif

static yaffsfs_DeviceConfiguration yaffsfs_config[] = {
/* XXX U-BOOT XXX */
#if 0
	{ "/ram", &ramDev},
	{ "/boot", &bootDev},
	{ "/flash", &flashDev},
#else
	{ MOUNT_POINT, 0},
#endif
	{(void *)0,(void *)0}
};


int yaffs_StartUp(void)
{
	struct mtd_info *mtd = &nand_info[0];
	int yaffsVersion = 2;
	int nBlocks;

	yaffs_Device *flashDev = calloc(1, sizeof(yaffs_Device));
	yaffsfs_config[0].dev = flashDev;

	/* store the mtd device for later use */
	flashDev->genericDevice = mtd;

	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_LocalInitialisation();

	// Set up devices

/* XXX U-BOOT XXX */
#if 0
	// /ram
	ramDev.nBytesPerChunk = 512;
	ramDev.nChunksPerBlock = 32;
	ramDev.nReservedBlocks = 2; // Set this smaller for RAM
	ramDev.startBlock = 1; // Can't use block 0
	ramDev.endBlock = 127; // Last block in 2MB.
	ramDev.useNANDECC = 1;
	ramDev.nShortOpCaches = 0;	// Disable caching on this device.
	ramDev.genericDevice = (void *) 0;	// Used to identify the device in fstat.
	ramDev.writeChunkWithTagsToNAND = yramdisk_WriteChunkWithTagsToNAND;
	ramDev.readChunkWithTagsFromNAND = yramdisk_ReadChunkWithTagsFromNAND;
	ramDev.eraseBlockInNAND = yramdisk_EraseBlockInNAND;
	ramDev.initialiseNAND = yramdisk_InitialiseNAND;

	// /boot
	bootDev.nBytesPerChunk = 612;
	bootDev.nChunksPerBlock = 32;
	bootDev.nReservedBlocks = 5;
	bootDev.startBlock = 1; // Can't use block 0
	bootDev.endBlock = 127; // Last block in 2MB.
	bootDev.useNANDECC = 0; // use YAFFS's ECC
	bootDev.nShortOpCaches = 10; // Use caches
	bootDev.genericDevice = (void *) 1;	// Used to identify the device in fstat.
	bootDev.writeChunkToNAND = yflash_WriteChunkToNAND;
	bootDev.readChunkFromNAND = yflash_ReadChunkFromNAND;
	bootDev.eraseBlockInNAND = yflash_EraseBlockInNAND;
	bootDev.initialiseNAND = yflash_InitialiseNAND;
#endif

		// /flash
	flashDev->nReservedBlocks = 5;
//  flashDev->nShortOpCaches = (options.no_cache) ? 0 : 10;
	flashDev->nShortOpCaches = 10; // Use caches
	flashDev->useNANDECC = 0; // do not use YAFFS's ECC

	if (yaffsVersion == 2)
	{
		flashDev->writeChunkWithTagsToNAND = nandmtd2_WriteChunkWithTagsToNAND;
		flashDev->readChunkWithTagsFromNAND = nandmtd2_ReadChunkWithTagsFromNAND;
		flashDev->markNANDBlockBad = nandmtd2_MarkNANDBlockBad;
		flashDev->queryNANDBlock = nandmtd2_QueryNANDBlock;
		flashDev->spareBuffer = YMALLOC(mtd->oobsize);
		flashDev->isYaffs2 = 1;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
		flashDev->nDataBytesPerChunk = mtd->writesize;
		flashDev->nChunksPerBlock = mtd->erasesize / mtd->writesize;
#else
		flashDev->nDataBytesPerChunk = mtd->oobblock;
		flashDev->nChunksPerBlock = mtd->erasesize / mtd->oobblock;
#endif
		nBlocks = mtd->size / mtd->erasesize;

		flashDev->nCheckpointReservedBlocks = 10;
		flashDev->startBlock = 0;
		flashDev->endBlock = nBlocks - 1;
	}
	else
	{
		flashDev->writeChunkToNAND = nandmtd_WriteChunkToNAND;
		flashDev->readChunkFromNAND = nandmtd_ReadChunkFromNAND;
		flashDev->isYaffs2 = 0;
		nBlocks = mtd->size / (YAFFS_CHUNKS_PER_BLOCK * YAFFS_BYTES_PER_CHUNK);
		flashDev->startBlock = 320;
		flashDev->endBlock = nBlocks - 1;
		flashDev->nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
		flashDev->nDataBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
	}

	/* ... and common functions */
	flashDev->eraseBlockInNAND = nandmtd_EraseBlockInNAND;
	flashDev->initialiseNAND = nandmtd_InitialiseNAND;

	yaffs_initialise(yaffsfs_config);

	return 0;
}


void make_a_file(char *yaffsName,char bval,int sizeOfFile)
{
	int outh;
	int i;
	unsigned char buffer[100];

	outh = yaffs_open(yaffsName, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (outh < 0)
	{
		printf("Error opening file: %d\n", outh);
		return;
	}

	memset(buffer,bval,100);

	do{
		i = sizeOfFile;
		if(i > 100) i = 100;
		sizeOfFile -= i;

		yaffs_write(outh,buffer,i);

	} while (sizeOfFile > 0);


	yaffs_close(outh);
}

void read_a_file(char *fn)
{
	int h;
	int i = 0;
	unsigned char b;

	h = yaffs_open(fn, O_RDWR,0);
	if(h<0)
	{
		printf("File not found\n");
		return;
	}

	while(yaffs_read(h,&b,1)> 0)
	{
		printf("%02x ",b);
		i++;
		if(i > 32)
		{
		   printf("\n");
		   i = 0;;
		 }
	}
	printf("\n");
	yaffs_close(h);
}

void cmd_yaffs_mount(char *mp)
{
	yaffs_StartUp();
	int retval = yaffs_mount(mp);
	if( retval != -1)
		isMounted = 1;
	else
		printf("Error mounting %s, return value: %d\n", mp, yaffsfs_GetError());
}

static void checkMount(void)
{
	if( !isMounted )
	{
		cmd_yaffs_mount(MOUNT_POINT);
	}
}

void cmd_yaffs_umount(char *mp)
{
	checkMount();
	if( yaffs_unmount(mp) == -1)
		printf("Error umounting %s, return value: %d\n", mp, yaffsfs_GetError());
}

void cmd_yaffs_write_file(char *yaffsName,char bval,int sizeOfFile)
{
	checkMount();
	make_a_file(yaffsName,bval,sizeOfFile);
}


void cmd_yaffs_read_file(char *fn)
{
	checkMount();
	read_a_file(fn);
}


void cmd_yaffs_mread_file(char *fn, char *addr)
{
	int h;
	struct yaffs_stat s;

	checkMount();

	yaffs_stat(fn,&s);

	printf ("Copy %s to 0x%p... ", fn, addr);
	h = yaffs_open(fn, O_RDWR,0);
	if(h<0)
	{
		printf("File not found\n");
		return;
	}

	yaffs_read(h,addr,(int)s.st_size);
	printf("\t[DONE]\n");

	yaffs_close(h);
}


void cmd_yaffs_mwrite_file(char *fn, char *addr, int size)
{
	int outh;

	checkMount();
	outh = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (outh < 0)
	{
		printf("Error opening file: %d\n", outh);
	}

	yaffs_write(outh,addr,size);

	yaffs_close(outh);
}


void cmd_yaffs_ls(const char *mountpt, int longlist)
{
	int i;
	yaffs_DIR *d;
	yaffs_dirent *de;
	struct yaffs_stat stat;
	char tempstr[255];

	checkMount();
	d = yaffs_opendir(mountpt);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		for(i = 0; (de = yaffs_readdir(d)) != NULL; i++)
		{
			if (longlist)
			{
				sprintf(tempstr, "%s/%s", mountpt, de->d_name);
				yaffs_stat(tempstr, &stat);
				printf("%-25s\t%7ld\n",de->d_name, stat.st_size);
			}
			else
			{
				printf("%s\n",de->d_name);
			}
		}
	}
}


void cmd_yaffs_mkdir(const char *dir)
{
	checkMount();

	int retval = yaffs_mkdir(dir, 0);

	if ( retval < 0)
		printf("yaffs_mkdir returning error: %d\n", retval);
}

void cmd_yaffs_rmdir(const char *dir)
{
	checkMount();

	int retval = yaffs_rmdir(dir);

	if ( retval < 0)
		printf("yaffs_rmdir returning error: %d\n", retval);
}

void cmd_yaffs_rm(const char *path)
{
	checkMount();

	int retval = yaffs_unlink(path);

	if ( retval < 0)
		printf("yaffs_unlink returning error: %d\n", retval);
}

void cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	checkMount();

	int retval = yaffs_rename(newPath, oldPath);

	if ( retval < 0)
		printf("yaffs_unlink returning error: %d\n", retval);
}
