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

/* XXX U-BOOT XXX */
#include <common.h>
#include <malloc.h>

#include "yaffsfs.h"
#include "yaffs_guts.h"
#include "yaffscfg.h"
#include "yportenv.h"

/* XXX U-BOOT XXX */
#if 0
#include <string.h> // for memset
#endif

#define YAFFSFS_MAX_SYMLINK_DEREFERENCES 5

#ifndef NULL
#define NULL ((void *)0)
#endif


const char *yaffsfs_c_version="$Id: yaffsfs.c,v 1.18 2007/07/18 19:40:38 charles Exp $";

// configurationList is the list of devices that are supported
static yaffsfs_DeviceConfiguration *yaffsfs_configurationList;


/* Some forward references */
static yaffs_Object *yaffsfs_FindObject(yaffs_Object *relativeDirectory, const char *path, int symDepth);
static void yaffsfs_RemoveObjectCallback(yaffs_Object *obj);


// Handle management.
//


unsigned int yaffs_wr_attempts;

typedef struct
{
	__u8  inUse:1;		// this handle is in use
	__u8  readOnly:1;	// this handle is read only
	__u8  append:1;		// append only
	__u8  exclusive:1;	// exclusive
	__u32 position;		// current position in file
	yaffs_Object *obj;	// the object
}yaffsfs_Handle;


static yaffsfs_Handle yaffsfs_handle[YAFFSFS_N_HANDLES];

// yaffsfs_InitHandle
/// Inilitalise handles on start-up.
//
static int yaffsfs_InitHandles(void)
{
	int i;
	for(i = 0; i < YAFFSFS_N_HANDLES; i++)
	{
		yaffsfs_handle[i].inUse = 0;
		yaffsfs_handle[i].obj = NULL;
	}
	return 0;
}

yaffsfs_Handle *yaffsfs_GetHandlePointer(int h)
{
	if(h < 0 || h >= YAFFSFS_N_HANDLES)
	{
		return NULL;
	}

	return &yaffsfs_handle[h];
}

yaffs_Object *yaffsfs_GetHandleObject(int handle)
{
	yaffsfs_Handle *h = yaffsfs_GetHandlePointer(handle);

	if(h && h->inUse)
	{
		return h->obj;
	}

	return NULL;
}


//yaffsfs_GetHandle
// Grab a handle (when opening a file)
//

static int yaffsfs_GetHandle(void)
{
	int i;
	yaffsfs_Handle *h;

	for(i = 0; i < YAFFSFS_N_HANDLES; i++)
	{
		h = yaffsfs_GetHandlePointer(i);
		if(!h)
		{
			// todo bug: should never happen
		}
		if(!h->inUse)
		{
			memset(h,0,sizeof(yaffsfs_Handle));
			h->inUse=1;
			return i;
		}
	}
	return -1;
}

// yaffs_PutHandle
// Let go of a handle (when closing a file)
//
static int yaffsfs_PutHandle(int handle)
{
	yaffsfs_Handle *h = yaffsfs_GetHandlePointer(handle);

	if(h)
	{
		h->inUse = 0;
		h->obj = NULL;
	}
	return 0;
}



// Stuff to search for a directory from a path


int yaffsfs_Match(char a, char b)
{
	// case sensitive
	return (a == b);
}

// yaffsfs_FindDevice
// yaffsfs_FindRoot
// Scan the configuration list to find the root.
// Curveballs: Should match paths that end in '/' too
// Curveball2 Might have "/x/ and "/x/y". Need to return the longest match
static yaffs_Device *yaffsfs_FindDevice(const char *path, char **restOfPath)
{
	yaffsfs_DeviceConfiguration *cfg = yaffsfs_configurationList;
	const char *leftOver;
	const char *p;
	yaffs_Device *retval = NULL;
	int thisMatchLength;
	int longestMatch = -1;

	// Check all configs, choose the one that:
	// 1) Actually matches a prefix (ie /a amd /abc will not match
	// 2) Matches the longest.
	while(cfg && cfg->prefix && cfg->dev)
	{
		leftOver = path;
		p = cfg->prefix;
		thisMatchLength = 0;

		while(*p &&  //unmatched part of prefix
		      strcmp(p,"/") && // the rest of the prefix is not / (to catch / at end)
		      *leftOver &&
		      yaffsfs_Match(*p,*leftOver))
		{
			p++;
			leftOver++;
			thisMatchLength++;
		}
		if((!*p || strcmp(p,"/") == 0) &&      // end of prefix
		   (!*leftOver || *leftOver == '/') && // no more in this path name part
		   (thisMatchLength > longestMatch))
		{
			// Matched prefix
			*restOfPath = (char *)leftOver;
			retval = cfg->dev;
			longestMatch = thisMatchLength;
		}
		cfg++;
	}
	return retval;
}

static yaffs_Object *yaffsfs_FindRoot(const char *path, char **restOfPath)
{

	yaffs_Device *dev;

	dev= yaffsfs_FindDevice(path,restOfPath);
	if(dev && dev->isMounted)
	{
		return dev->rootDir;
	}
	return NULL;
}

static yaffs_Object *yaffsfs_FollowLink(yaffs_Object *obj,int symDepth)
{

	while(obj && obj->variantType == YAFFS_OBJECT_TYPE_SYMLINK)
	{
		char *alias = obj->variant.symLinkVariant.alias;

		if(*alias == '/')
		{
			// Starts with a /, need to scan from root up
			obj = yaffsfs_FindObject(NULL,alias,symDepth++);
		}
		else
		{
			// Relative to here, so use the parent of the symlink as a start
			obj = yaffsfs_FindObject(obj->parent,alias,symDepth++);
		}
	}
	return obj;
}


// yaffsfs_FindDirectory
// Parse a path to determine the directory and the name within the directory.
//
// eg. "/data/xx/ff" --> puts name="ff" and returns the directory "/data/xx"
static yaffs_Object *yaffsfs_DoFindDirectory(yaffs_Object *startDir,const char *path,char **name,int symDepth)
{
	yaffs_Object *dir;
	char *restOfPath;
	char str[YAFFS_MAX_NAME_LENGTH+1];
	int i;

	if(symDepth > YAFFSFS_MAX_SYMLINK_DEREFERENCES)
	{
		return NULL;
	}

	if(startDir)
	{
		dir = startDir;
		restOfPath = (char *)path;
	}
	else
	{
		dir = yaffsfs_FindRoot(path,&restOfPath);
	}

	while(dir)
	{
		// parse off /.
		// curve ball: also throw away surplus '/'
		// eg. "/ram/x////ff" gets treated the same as "/ram/x/ff"
		while(*restOfPath == '/')
		{
			restOfPath++; // get rid of '/'
		}

		*name = restOfPath;
		i = 0;

		while(*restOfPath && *restOfPath != '/')
		{
			if (i < YAFFS_MAX_NAME_LENGTH)
			{
				str[i] = *restOfPath;
				str[i+1] = '\0';
				i++;
			}
			restOfPath++;
		}

		if(!*restOfPath)
		{
			// got to the end of the string
			return dir;
		}
		else
		{
			if(strcmp(str,".") == 0)
			{
				// Do nothing
			}
			else if(strcmp(str,"..") == 0)
			{
				dir = dir->parent;
			}
			else
			{
				dir = yaffs_FindObjectByName(dir,str);

				while(dir && dir->variantType == YAFFS_OBJECT_TYPE_SYMLINK)
				{

					dir = yaffsfs_FollowLink(dir,symDepth);

				}

				if(dir && dir->variantType != YAFFS_OBJECT_TYPE_DIRECTORY)
				{
					dir = NULL;
				}
			}
		}
	}
	// directory did not exist.
	return NULL;
}

static yaffs_Object *yaffsfs_FindDirectory(yaffs_Object *relativeDirectory,const char *path,char **name,int symDepth)
{
	return yaffsfs_DoFindDirectory(relativeDirectory,path,name,symDepth);
}

// yaffsfs_FindObject turns a path for an existing object into the object
//
static yaffs_Object *yaffsfs_FindObject(yaffs_Object *relativeDirectory, const char *path,int symDepth)
{
	yaffs_Object *dir;
	char *name;

	dir = yaffsfs_FindDirectory(relativeDirectory,path,&name,symDepth);

	if(dir && *name)
	{
		return yaffs_FindObjectByName(dir,name);
	}

	return dir;
}



int yaffs_open(const char *path, int oflag, int mode)
{
	yaffs_Object *obj = NULL;
	yaffs_Object *dir = NULL;
	char *name;
	int handle = -1;
	yaffsfs_Handle *h = NULL;
	int alreadyOpen = 0;
	int alreadyExclusive = 0;
	int openDenied = 0;
	int symDepth = 0;
	int errorReported = 0;

	int i;


	// todo sanity check oflag (eg. can't have O_TRUNC without WRONLY or RDWR


	yaffsfs_Lock();

	handle = yaffsfs_GetHandle();

	if(handle >= 0)
	{

		h = yaffsfs_GetHandlePointer(handle);


		// try to find the exisiting object
		obj = yaffsfs_FindObject(NULL,path,0);

		if(obj && obj->variantType == YAFFS_OBJECT_TYPE_SYMLINK)
		{

			obj = yaffsfs_FollowLink(obj,symDepth++);
		}

		if(obj)
		{
			// Check if the object is already in use
			alreadyOpen = alreadyExclusive = 0;

			for(i = 0; i <= YAFFSFS_N_HANDLES; i++)
			{

				if(i != handle &&
				   yaffsfs_handle[i].inUse &&
				    obj == yaffsfs_handle[i].obj)
				 {
				 	alreadyOpen = 1;
					if(yaffsfs_handle[i].exclusive)
					{
						alreadyExclusive = 1;
					}
				 }
			}

			if(((oflag & O_EXCL) && alreadyOpen) || alreadyExclusive)
			{
				openDenied = 1;
			}

			// Open should fail if O_CREAT and O_EXCL are specified
			if((oflag & O_EXCL) && (oflag & O_CREAT))
			{
				openDenied = 1;
				yaffsfs_SetError(-EEXIST);
				errorReported = 1;
			}

			// Check file permissions
			if( (oflag & (O_RDWR | O_WRONLY)) == 0 &&     // ie O_RDONLY
			   !(obj->yst_mode & S_IREAD))
			{
				openDenied = 1;
			}

			if( (oflag & O_RDWR) &&
			   !(obj->yst_mode & S_IREAD))
			{
				openDenied = 1;
			}

			if( (oflag & (O_RDWR | O_WRONLY)) &&
			   !(obj->yst_mode & S_IWRITE))
			{
				openDenied = 1;
			}

		}

		else if((oflag & O_CREAT))
		{
			// Let's see if we can create this file
			dir = yaffsfs_FindDirectory(NULL,path,&name,0);
			if(dir)
			{
				obj = yaffs_MknodFile(dir,name,mode,0,0);
			}
			else
			{
				yaffsfs_SetError(-ENOTDIR);
			}
		}

		if(obj && !openDenied)
		{
			h->obj = obj;
			h->inUse = 1;
	    	h->readOnly = (oflag & (O_WRONLY | O_RDWR)) ? 0 : 1;
			h->append =  (oflag & O_APPEND) ? 1 : 0;
			h->exclusive = (oflag & O_EXCL) ? 1 : 0;
			h->position = 0;

			obj->inUse++;
			if((oflag & O_TRUNC) && !h->readOnly)
			{
				//todo truncate
				yaffs_ResizeFile(obj,0);
			}

		}
		else
		{
			yaffsfs_PutHandle(handle);
			if(!errorReported)
			{
				yaffsfs_SetError(-EACCESS);
				errorReported = 1;
			}
			handle = -1;
		}

	}

	yaffsfs_Unlock();

	return handle;
}

int yaffs_close(int fd)
{
	yaffsfs_Handle *h = NULL;
	int retVal = 0;

	yaffsfs_Lock();

	h = yaffsfs_GetHandlePointer(fd);

	if(h && h->inUse)
	{
		// clean up
		yaffs_FlushFile(h->obj,1);
		h->obj->inUse--;
		if(h->obj->inUse <= 0 && h->obj->unlinked)
		{
			yaffs_DeleteFile(h->obj);
		}
		yaffsfs_PutHandle(fd);
		retVal = 0;
	}
	else
	{
		// bad handle
		yaffsfs_SetError(-EBADF);
		retVal = -1;
	}

	yaffsfs_Unlock();

	return retVal;
}

int yaffs_read(int fd, void *buf, unsigned int nbyte)
{
	yaffsfs_Handle *h = NULL;
	yaffs_Object *obj = NULL;
	int pos = 0;
	int nRead = -1;
	int maxRead;

	yaffsfs_Lock();
	h = yaffsfs_GetHandlePointer(fd);
	obj = yaffsfs_GetHandleObject(fd);

	if(!h || !obj)
	{
		// bad handle
		yaffsfs_SetError(-EBADF);
	}
	else if( h && obj)
	{
		pos=  h->position;
		if(yaffs_GetObjectFileLength(obj) > pos)
		{
			maxRead = yaffs_GetObjectFileLength(obj) - pos;
		}
		else
		{
			maxRead = 0;
		}

		if(nbyte > maxRead)
		{
			nbyte = maxRead;
		}


		if(nbyte > 0)
		{
			nRead = yaffs_ReadDataFromFile(obj,buf,pos,nbyte);
			if(nRead >= 0)
			{
				h->position = pos + nRead;
			}
			else
			{
				//todo error
			}
		}
		else
		{
			nRead = 0;
		}

	}

	yaffsfs_Unlock();


	return (nRead >= 0) ? nRead : -1;

}

int yaffs_write(int fd, const void *buf, unsigned int nbyte)
{
	yaffsfs_Handle *h = NULL;
	yaffs_Object *obj = NULL;
	int pos = 0;
	int nWritten = -1;
	int writeThrough = 0;

	yaffsfs_Lock();
	h = yaffsfs_GetHandlePointer(fd);
	obj = yaffsfs_GetHandleObject(fd);

	if(!h || !obj)
	{
		// bad handle
		yaffsfs_SetError(-EBADF);
	}
	else if( h && obj && h->readOnly)
	{
		// todo error
	}
	else if( h && obj)
	{
		if(h->append)
		{
			pos =  yaffs_GetObjectFileLength(obj);
		}
		else
		{
			pos = h->position;
		}

		nWritten = yaffs_WriteDataToFile(obj,buf,pos,nbyte,writeThrough);

		if(nWritten >= 0)
		{
			h->position = pos + nWritten;
		}
		else
		{
			//todo error
		}

	}

	yaffsfs_Unlock();


	return (nWritten >= 0) ? nWritten : -1;

}

int yaffs_truncate(int fd, off_t newSize)
{
	yaffsfs_Handle *h = NULL;
	yaffs_Object *obj = NULL;
	int result = 0;

	yaffsfs_Lock();
	h = yaffsfs_GetHandlePointer(fd);
	obj = yaffsfs_GetHandleObject(fd);

	if(!h || !obj)
	{
		// bad handle
		yaffsfs_SetError(-EBADF);
	}
	else
	{
		// resize the file
		result = yaffs_ResizeFile(obj,newSize);
	}
	yaffsfs_Unlock();


	return (result) ? 0 : -1;

}

off_t yaffs_lseek(int fd, off_t offset, int whence)
{
	yaffsfs_Handle *h = NULL;
	yaffs_Object *obj = NULL;
	int pos = -1;
	int fSize = -1;

	yaffsfs_Lock();
	h = yaffsfs_GetHandlePointer(fd);
	obj = yaffsfs_GetHandleObject(fd);

	if(!h || !obj)
	{
		// bad handle
		yaffsfs_SetError(-EBADF);
	}
	else if(whence == SEEK_SET)
	{
		if(offset >= 0)
		{
			pos = offset;
		}
	}
	else if(whence == SEEK_CUR)
	{
		if( (h->position + offset) >= 0)
		{
			pos = (h->position + offset);
		}
	}
	else if(whence == SEEK_END)
	{
		fSize = yaffs_GetObjectFileLength(obj);
		if(fSize >= 0 && (fSize + offset) >= 0)
		{
			pos = fSize + offset;
		}
	}

	if(pos >= 0)
	{
		h->position = pos;
	}
	else
	{
		// todo error
	}


	yaffsfs_Unlock();

	return pos;
}


int yaffsfs_DoUnlink(const char *path,int isDirectory)
{
	yaffs_Object *dir = NULL;
	yaffs_Object *obj = NULL;
	char *name;
	int result = YAFFS_FAIL;

	yaffsfs_Lock();

	obj = yaffsfs_FindObject(NULL,path,0);
	dir = yaffsfs_FindDirectory(NULL,path,&name,0);
	if(!dir)
	{
		yaffsfs_SetError(-ENOTDIR);
	}
	else if(!obj)
	{
		yaffsfs_SetError(-ENOENT);
	}
	else if(!isDirectory && obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
	{
		yaffsfs_SetError(-EISDIR);
	}
	else if(isDirectory && obj->variantType != YAFFS_OBJECT_TYPE_DIRECTORY)
	{
		yaffsfs_SetError(-ENOTDIR);
	}
	else
	{
		result = yaffs_Unlink(dir,name);

		if(result == YAFFS_FAIL && isDirectory)
		{
			yaffsfs_SetError(-ENOTEMPTY);
		}
	}

	yaffsfs_Unlock();

	// todo error

	return (result == YAFFS_FAIL) ? -1 : 0;
}
int yaffs_rmdir(const char *path)
{
	return yaffsfs_DoUnlink(path,1);
}

int yaffs_unlink(const char *path)
{
	return yaffsfs_DoUnlink(path,0);
}

int yaffs_rename(const char *oldPath, const char *newPath)
{
	yaffs_Object *olddir = NULL;
	yaffs_Object *newdir = NULL;
	yaffs_Object *obj = NULL;
	char *oldname;
	char *newname;
	int result= YAFFS_FAIL;
	int renameAllowed = 1;

	yaffsfs_Lock();

	olddir = yaffsfs_FindDirectory(NULL,oldPath,&oldname,0);
	newdir = yaffsfs_FindDirectory(NULL,newPath,&newname,0);
	obj = yaffsfs_FindObject(NULL,oldPath,0);

	if(!olddir || !newdir || !obj)
	{
		// bad file
		yaffsfs_SetError(-EBADF);
		renameAllowed = 0;
	}
	else if(olddir->myDev != newdir->myDev)
	{
		// oops must be on same device
		// todo error
		yaffsfs_SetError(-EXDEV);
		renameAllowed = 0;
	}
	else if(obj && obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
	{
		// It is a directory, check that it is not being renamed to
		// being its own decendent.
		// Do this by tracing from the new directory back to the root, checking for obj

		yaffs_Object *xx = newdir;

		while( renameAllowed && xx)
		{
			if(xx == obj)
			{
				renameAllowed = 0;
			}
			xx = xx->parent;
		}
		if(!renameAllowed) yaffsfs_SetError(-EACCESS);
	}

	if(renameAllowed)
	{
		result = yaffs_RenameObject(olddir,oldname,newdir,newname);
	}

	yaffsfs_Unlock();

	return (result == YAFFS_FAIL) ? -1 : 0;
}


static int yaffsfs_DoStat(yaffs_Object *obj,struct yaffs_stat *buf)
{
	int retVal = -1;

	if(obj)
	{
		obj = yaffs_GetEquivalentObject(obj);
	}

	if(obj && buf)
	{
    	buf->st_dev = (int)obj->myDev->genericDevice;
    	buf->st_ino = obj->objectId;
    	buf->st_mode = obj->yst_mode & ~S_IFMT; // clear out file type bits

		if(obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
		{
			buf->st_mode |= S_IFDIR;
		}
		else if(obj->variantType == YAFFS_OBJECT_TYPE_SYMLINK)
		{
			buf->st_mode |= S_IFLNK;
		}
		else if(obj->variantType == YAFFS_OBJECT_TYPE_FILE)
		{
			buf->st_mode |= S_IFREG;
		}

    	buf->st_nlink = yaffs_GetObjectLinkCount(obj);
    	buf->st_uid = 0;
    	buf->st_gid = 0;;
    	buf->st_rdev = obj->yst_rdev;
    	buf->st_size = yaffs_GetObjectFileLength(obj);
		buf->st_blksize = obj->myDev->nDataBytesPerChunk;
    	buf->st_blocks = (buf->st_size + buf->st_blksize -1)/buf->st_blksize;
    	buf->yst_atime = obj->yst_atime;
    	buf->yst_ctime = obj->yst_ctime;
    	buf->yst_mtime = obj->yst_mtime;
		retVal = 0;
	}
	return retVal;
}

static int yaffsfs_DoStatOrLStat(const char *path, struct yaffs_stat *buf,int doLStat)
{
	yaffs_Object *obj;

	int retVal = -1;

	yaffsfs_Lock();
	obj = yaffsfs_FindObject(NULL,path,0);

	if(!doLStat && obj)
	{
		obj = yaffsfs_FollowLink(obj,0);
	}

	if(obj)
	{
		retVal = yaffsfs_DoStat(obj,buf);
	}
	else
	{
		// todo error not found
		yaffsfs_SetError(-ENOENT);
	}

	yaffsfs_Unlock();

	return retVal;

}

int yaffs_stat(const char *path, struct yaffs_stat *buf)
{
	return yaffsfs_DoStatOrLStat(path,buf,0);
}

int yaffs_lstat(const char *path, struct yaffs_stat *buf)
{
	return yaffsfs_DoStatOrLStat(path,buf,1);
}

int yaffs_fstat(int fd, struct yaffs_stat *buf)
{
	yaffs_Object *obj;

	int retVal = -1;

	yaffsfs_Lock();
	obj = yaffsfs_GetHandleObject(fd);

	if(obj)
	{
		retVal = yaffsfs_DoStat(obj,buf);
	}
	else
	{
		// bad handle
		yaffsfs_SetError(-EBADF);
	}

	yaffsfs_Unlock();

	return retVal;
}

static int yaffsfs_DoChMod(yaffs_Object *obj,mode_t mode)
{
	int result = YAFFS_FAIL;

	if(obj)
	{
		obj = yaffs_GetEquivalentObject(obj);
	}

	if(obj)
	{
		obj->yst_mode = mode;
		obj->dirty = 1;
		result = yaffs_FlushFile(obj,0);
	}

	return result == YAFFS_OK ? 0 : -1;
}


int yaffs_chmod(const char *path, mode_t mode)
{
	yaffs_Object *obj;

	int retVal = -1;

	yaffsfs_Lock();
	obj = yaffsfs_FindObject(NULL,path,0);

	if(obj)
	{
		retVal = yaffsfs_DoChMod(obj,mode);
	}
	else
	{
		// todo error not found
		yaffsfs_SetError(-ENOENT);
	}

	yaffsfs_Unlock();

	return retVal;

}


int yaffs_fchmod(int fd, mode_t mode)
{
	yaffs_Object *obj;

	int retVal = -1;

	yaffsfs_Lock();
	obj = yaffsfs_GetHandleObject(fd);

	if(obj)
	{
		retVal = yaffsfs_DoChMod(obj,mode);
	}
	else
	{
		// bad handle
		yaffsfs_SetError(-EBADF);
	}

	yaffsfs_Unlock();

	return retVal;
}


int yaffs_mkdir(const char *path, mode_t mode)
{
	yaffs_Object *parent = NULL;
	yaffs_Object *dir = NULL;
	char *name;
	int retVal= -1;

	yaffsfs_Lock();
	parent = yaffsfs_FindDirectory(NULL,path,&name,0);
	if(parent)
		dir = yaffs_MknodDirectory(parent,name,mode,0,0);
	if(dir)
	{
		retVal = 0;
	}
	else
	{
		yaffsfs_SetError(-ENOSPC); // just assume no space for now
		retVal = -1;
	}

	yaffsfs_Unlock();

	return retVal;
}

int yaffs_mount(const char *path)
{
	int retVal=-1;
	int result=YAFFS_FAIL;
	yaffs_Device *dev=NULL;
	char *dummy;

	T(YAFFS_TRACE_ALWAYS,("yaffs: Mounting %s\n",path));

	yaffsfs_Lock();
	dev = yaffsfs_FindDevice(path,&dummy);
	if(dev)
	{
		if(!dev->isMounted)
		{
			result = yaffs_GutsInitialise(dev);
			if(result == YAFFS_FAIL)
			{
				// todo error - mount failed
				yaffsfs_SetError(-ENOMEM);
			}
			retVal = result ? 0 : -1;

		}
		else
		{
			//todo error - already mounted.
			yaffsfs_SetError(-EBUSY);
		}
	}
	else
	{
		// todo error - no device
		yaffsfs_SetError(-ENODEV);
	}
	yaffsfs_Unlock();
	return retVal;

}

int yaffs_unmount(const char *path)
{
	int retVal=-1;
	yaffs_Device *dev=NULL;
	char *dummy;

	yaffsfs_Lock();
	dev = yaffsfs_FindDevice(path,&dummy);
	if(dev)
	{
		if(dev->isMounted)
		{
			int i;
			int inUse;

			yaffs_FlushEntireDeviceCache(dev);
			yaffs_CheckpointSave(dev);

			for(i = inUse = 0; i < YAFFSFS_N_HANDLES && !inUse; i++)
			{
				if(yaffsfs_handle[i].inUse && yaffsfs_handle[i].obj->myDev == dev)
				{
					inUse = 1; // the device is in use, can't unmount
				}
			}

			if(!inUse)
			{
				yaffs_Deinitialise(dev);

				retVal = 0;
			}
			else
			{
				// todo error can't unmount as files are open
				yaffsfs_SetError(-EBUSY);
			}

		}
		else
		{
			//todo error - not mounted.
			yaffsfs_SetError(-EINVAL);

		}
	}
	else
	{
		// todo error - no device
		yaffsfs_SetError(-ENODEV);
	}
	yaffsfs_Unlock();
	return retVal;

}

loff_t yaffs_freespace(const char *path)
{
	loff_t retVal=-1;
	yaffs_Device *dev=NULL;
	char *dummy;

	yaffsfs_Lock();
	dev = yaffsfs_FindDevice(path,&dummy);
	if(dev  && dev->isMounted)
	{
		retVal = yaffs_GetNumberOfFreeChunks(dev);
		retVal *= dev->nDataBytesPerChunk;

	}
	else
	{
		yaffsfs_SetError(-EINVAL);
	}

	yaffsfs_Unlock();
	return retVal;
}



void yaffs_initialise(yaffsfs_DeviceConfiguration *cfgList)
{

	yaffsfs_DeviceConfiguration *cfg;

	yaffsfs_configurationList = cfgList;

	yaffsfs_InitHandles();

	cfg = yaffsfs_configurationList;

	while(cfg && cfg->prefix && cfg->dev)
	{
		cfg->dev->isMounted = 0;
		cfg->dev->removeObjectCallback = yaffsfs_RemoveObjectCallback;
		cfg++;
	}
}


//
// Directory search stuff.

//
// Directory search context
//
// NB this is an opaque structure.


typedef struct
{
	__u32 magic;
	yaffs_dirent de;		/* directory entry being used by this dsc */
	char name[NAME_MAX+1];		/* name of directory being searched */
	yaffs_Object *dirObj;		/* ptr to directory being searched */
	yaffs_Object *nextReturn;	/* obj to be returned by next readddir */
	int offset;
	struct list_head others;
} yaffsfs_DirectorySearchContext;



static struct list_head search_contexts;


static void yaffsfs_SetDirRewound(yaffsfs_DirectorySearchContext *dsc)
{
	if(dsc &&
	   dsc->dirObj &&
	   dsc->dirObj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY){

	   dsc->offset = 0;

	   if( list_empty(&dsc->dirObj->variant.directoryVariant.children)){
	   	dsc->nextReturn = NULL;
	   } else {
	      	dsc->nextReturn = list_entry(dsc->dirObj->variant.directoryVariant.children.next,
						yaffs_Object,siblings);
	   }
	} else {
		/* Hey someone isn't playing nice! */
	}
}

static void yaffsfs_DirAdvance(yaffsfs_DirectorySearchContext *dsc)
{
	if(dsc &&
	   dsc->dirObj &&
	   dsc->dirObj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY){

	   if( dsc->nextReturn == NULL ||
	       list_empty(&dsc->dirObj->variant.directoryVariant.children)){
	   	dsc->nextReturn = NULL;
	   } else {
		   struct list_head *next = dsc->nextReturn->siblings.next;

		   if( next == &dsc->dirObj->variant.directoryVariant.children)
	   		dsc->nextReturn = NULL; /* end of list */
	   	   else
		   	dsc->nextReturn = list_entry(next,yaffs_Object,siblings);
	   }
	} else {
		/* Hey someone isn't playing nice! */
	}
}

static void yaffsfs_RemoveObjectCallback(yaffs_Object *obj)
{

	struct list_head *i;
	yaffsfs_DirectorySearchContext *dsc;

	/* if search contexts not initilised then skip */
	if(!search_contexts.next)
		return;

	/* Iteratethrough the directory search contexts.
	 * If any are the one being removed, then advance the dsc to
	 * the next one to prevent a hanging ptr.
	 */
	 list_for_each(i, &search_contexts) {
		if (i) {
			dsc = list_entry(i, yaffsfs_DirectorySearchContext,others);
			if(dsc->nextReturn == obj)
				yaffsfs_DirAdvance(dsc);
		}
	}

}

yaffs_DIR *yaffs_opendir(const char *dirname)
{
	yaffs_DIR *dir = NULL;
 	yaffs_Object *obj = NULL;
	yaffsfs_DirectorySearchContext *dsc = NULL;

	yaffsfs_Lock();

	obj = yaffsfs_FindObject(NULL,dirname,0);

	if(obj && obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
	{

		dsc = YMALLOC(sizeof(yaffsfs_DirectorySearchContext));
		dir = (yaffs_DIR *)dsc;
		if(dsc)
		{
			memset(dsc,0,sizeof(yaffsfs_DirectorySearchContext));
			dsc->magic = YAFFS_MAGIC;
			dsc->dirObj = obj;
			strncpy(dsc->name,dirname,NAME_MAX);
			INIT_LIST_HEAD(&dsc->others);

			if(!search_contexts.next)
				INIT_LIST_HEAD(&search_contexts);

			list_add(&dsc->others,&search_contexts);
			yaffsfs_SetDirRewound(dsc);		}

	}

	yaffsfs_Unlock();

	return dir;
}

struct yaffs_dirent *yaffs_readdir(yaffs_DIR *dirp)
{
	yaffsfs_DirectorySearchContext *dsc = (yaffsfs_DirectorySearchContext *)dirp;
	struct yaffs_dirent *retVal = NULL;

	yaffsfs_Lock();

	if(dsc && dsc->magic == YAFFS_MAGIC){
		yaffsfs_SetError(0);
		if(dsc->nextReturn){
			dsc->de.d_ino = yaffs_GetEquivalentObject(dsc->nextReturn)->objectId;
			dsc->de.d_dont_use = (unsigned)dsc->nextReturn;
			dsc->de.d_off = dsc->offset++;
			yaffs_GetObjectName(dsc->nextReturn,dsc->de.d_name,NAME_MAX);
			if(strlen(dsc->de.d_name) == 0)
			{
				// this should not happen!
				strcpy(dsc->de.d_name,"zz");
			}
			dsc->de.d_reclen = sizeof(struct yaffs_dirent);
			retVal = &dsc->de;
			yaffsfs_DirAdvance(dsc);
		} else
			retVal = NULL;
	}
	else
	{
		yaffsfs_SetError(-EBADF);
	}

	yaffsfs_Unlock();

	return retVal;

}


void yaffs_rewinddir(yaffs_DIR *dirp)
{
	yaffsfs_DirectorySearchContext *dsc = (yaffsfs_DirectorySearchContext *)dirp;

	yaffsfs_Lock();

	yaffsfs_SetDirRewound(dsc);

	yaffsfs_Unlock();
}


int yaffs_closedir(yaffs_DIR *dirp)
{
	yaffsfs_DirectorySearchContext *dsc = (yaffsfs_DirectorySearchContext *)dirp;

	yaffsfs_Lock();
	dsc->magic = 0;
	list_del(&dsc->others); /* unhook from list */
	YFREE(dsc);
	yaffsfs_Unlock();
	return 0;
}

// end of directory stuff


int yaffs_symlink(const char *oldpath, const char *newpath)
{
	yaffs_Object *parent = NULL;
	yaffs_Object *obj;
	char *name;
	int retVal= -1;
	int mode = 0; // ignore for now

	yaffsfs_Lock();
	parent = yaffsfs_FindDirectory(NULL,newpath,&name,0);
	obj = yaffs_MknodSymLink(parent,name,mode,0,0,oldpath);
	if(obj)
	{
		retVal = 0;
	}
	else
	{
		yaffsfs_SetError(-ENOSPC); // just assume no space for now
		retVal = -1;
	}

	yaffsfs_Unlock();

	return retVal;

}

int yaffs_readlink(const char *path, char *buf, int bufsiz)
{
	yaffs_Object *obj = NULL;
	int retVal;


	yaffsfs_Lock();

	obj = yaffsfs_FindObject(NULL,path,0);

	if(!obj)
	{
		yaffsfs_SetError(-ENOENT);
		retVal = -1;
	}
	else if(obj->variantType != YAFFS_OBJECT_TYPE_SYMLINK)
	{
		yaffsfs_SetError(-EINVAL);
		retVal = -1;
	}
	else
	{
		char *alias = obj->variant.symLinkVariant.alias;
		memset(buf,0,bufsiz);
		strncpy(buf,alias,bufsiz - 1);
		retVal = 0;
	}
	yaffsfs_Unlock();
	return retVal;
}

int yaffs_link(const char *oldpath, const char *newpath)
{
	// Creates a link called newpath to existing oldpath
	yaffs_Object *obj = NULL;
	yaffs_Object *target = NULL;
	int retVal = 0;


	yaffsfs_Lock();

	obj = yaffsfs_FindObject(NULL,oldpath,0);
	target = yaffsfs_FindObject(NULL,newpath,0);

	if(!obj)
	{
		yaffsfs_SetError(-ENOENT);
		retVal = -1;
	}
	else if(target)
	{
		yaffsfs_SetError(-EEXIST);
		retVal = -1;
	}
	else
	{
		yaffs_Object *newdir = NULL;
		yaffs_Object *link = NULL;

		char *newname;

		newdir = yaffsfs_FindDirectory(NULL,newpath,&newname,0);

		if(!newdir)
		{
			yaffsfs_SetError(-ENOTDIR);
			retVal = -1;
		}
		else if(newdir->myDev != obj->myDev)
		{
			yaffsfs_SetError(-EXDEV);
			retVal = -1;
		}
		if(newdir && strlen(newname) > 0)
		{
			link = yaffs_Link(newdir,newname,obj);
			if(link)
				retVal = 0;
			else
			{
				yaffsfs_SetError(-ENOSPC);
				retVal = -1;
			}

		}
	}
	yaffsfs_Unlock();

	return retVal;
}

int yaffs_mknod(const char *pathname, mode_t mode, dev_t dev);

int yaffs_DumpDevStruct(const char *path)
{
	char *rest;

	yaffs_Object *obj = yaffsfs_FindRoot(path,&rest);

	if(obj)
	{
		yaffs_Device *dev = obj->myDev;

		printf("\n"
			   "nPageWrites.......... %d\n"
			   "nPageReads........... %d\n"
			   "nBlockErasures....... %d\n"
			   "nGCCopies............ %d\n"
			   "garbageCollections... %d\n"
			   "passiveGarbageColl'ns %d\n"
			   "\n",
				dev->nPageWrites,
				dev->nPageReads,
				dev->nBlockErasures,
				dev->nGCCopies,
				dev->garbageCollections,
				dev->passiveGarbageCollections
		);

	}
	return 0;
}
