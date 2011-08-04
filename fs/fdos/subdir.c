/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <malloc.h>

#include "dos.h"
#include "fdos.h"

static int cache_sect;
static unsigned char cache [SZ_STD_SECTOR];


#define min(x,y) ((x)<(y)?(x):(y))

static int descend (Slot_t *parent,
	     Fs_t *fs,
		    char *path);

/*-----------------------------------------------------------------------------
 * init_subdir --
 *-----------------------------------------------------------------------------
 */
void init_subdir (void)
{
    cache_sect = -1;
}
/*-----------------------------------------------------------------------------
 * basename --
 *-----------------------------------------------------------------------------
 */
char *basename (char *name)
{
    register char *cptr;

    if (!name || !*name) {
	return ("");
    }

    for (cptr= name; *cptr++; );
    while (--cptr >= name) {
	if (*cptr == '/')    {
	    return (cptr + 1);
	}
    }
    return(name);
}
/*-----------------------------------------------------------------------------
 * root_map --
 *-----------------------------------------------------------------------------
 */
static int root_map (Fs_t *fs, Slot_t *file, int where, int *len)
{
    *len = min (*len, fs -> dir_len * SZ_STD_SECTOR - where);
    if (*len < 0 ) {
	*len = 0;
	return (-1);
    }
    return fs -> dir_start * SZ_STD_SECTOR + where;
}
/*-----------------------------------------------------------------------------
 * normal_map --
 *-----------------------------------------------------------------------------
 */
static int normal_map (Fs_t *fs, Slot_t *file, int where, int *len)
{
    int offset;
    int NrClu;
    unsigned short RelCluNr;
    unsigned short CurCluNr;
    unsigned short NewCluNr;
    unsigned short AbsCluNr;
    int clus_size;

    clus_size = fs -> cluster_size * SZ_STD_SECTOR;
    offset = where % clus_size;

    *len = min (*len, file -> FileSize - where);

    if (*len < 0 ) {
	*len = 0;
	return (0);
    }

    if (file -> FirstAbsCluNr < 2){
	*len = 0;
	return (0);
    }

    RelCluNr = where / clus_size;

    if (RelCluNr >= file -> PreviousRelCluNr){
	CurCluNr = file -> PreviousRelCluNr;
	AbsCluNr = file -> PreviousAbsCluNr;
    } else {
	CurCluNr = 0;
	AbsCluNr = file -> FirstAbsCluNr;
    }


    NrClu = (offset + *len - 1) / clus_size;
    while (CurCluNr <= RelCluNr + NrClu) {
	if (CurCluNr == RelCluNr){
	    /* we have reached the beginning of our zone. Save
	     * coordinates */
	    file -> PreviousRelCluNr = RelCluNr;
	    file -> PreviousAbsCluNr = AbsCluNr;
	}
	NewCluNr = fat_decode (fs, AbsCluNr);
	if (NewCluNr == 1 || NewCluNr == 0) {
	    PRINTF("Fat problem while decoding %d %x\n",
		    AbsCluNr, NewCluNr);
	    return (-1);
	}
	if (CurCluNr == RelCluNr + NrClu) {
	    break;
	}

	if (CurCluNr < RelCluNr && NewCluNr == FAT12_END) {
	    *len = 0;
	    return 0;
	}

	if (CurCluNr >= RelCluNr && NewCluNr != AbsCluNr + 1)
	    break;
	CurCluNr++;
	AbsCluNr = NewCluNr;
    }

    *len = min (*len, (1 + CurCluNr - RelCluNr) * clus_size - offset);

    return (((file -> PreviousAbsCluNr - 2) * fs -> cluster_size +
	     fs -> dir_start + fs -> dir_len) *
	    SZ_STD_SECTOR + offset);
}
/*-----------------------------------------------------------------------------
 * open_subdir -- open the subdir containing the file
 *-----------------------------------------------------------------------------
 */
int open_subdir (File_t *desc)
{
    char *pathname;
    char *tmp, *s, *path;
    char terminator;

    if ((pathname = (char *)malloc (MAX_PATH)) == NULL) {
	return (-1);
    }

    strcpy (pathname, desc -> name);

    /* Suppress file name                                                    */
    tmp = basename (pathname);
    *tmp = '\0';

    /* root directory  init                                                  */
    desc -> subdir.FirstAbsCluNr = 0;
    desc -> subdir.FileSize = -1;
    desc -> subdir.map = root_map;
    desc -> subdir.dir.attr = ATTR_DIRECTORY;

    tmp = pathname;
    for (s = tmp; ; ++s) {
	if (*s == '/' || *s == '\0') {
	    path = tmp;
	    terminator = *s;
	    *s = '\0';
	    if (s != tmp && strcmp (path,".")) {
		if (descend (&desc -> subdir, desc -> fs, path) < 0) {
		    free (pathname);
		    return (-1);
		}
	    }
	    if (terminator == 0) {
		break;
	    }
	    tmp = s + 1;
	}
    }
    free (pathname);
    return (0);
}
/*-----------------------------------------------------------------------------
 * descend --
 *-----------------------------------------------------------------------------
 */
static int descend (Slot_t *parent,
	     Fs_t *fs,
	     char *path)
{
    int entry;
    Slot_t SubDir;

    if(path[0] == '\0' || strcmp (path, ".") == 0) {
	return (0);
    }


    entry = 0;
    if (vfat_lookup (parent,
		     fs,
		     &(SubDir.dir),
		     &entry,
		     0,
		     path,
		     ACCEPT_DIR | SINGLE | DO_OPEN,
		     0,
		     &SubDir) == 0) {
	*parent = SubDir;
	return (0);
    }

    if (strcmp(path, "..") == 0) {
	parent -> FileSize = -1;
	parent -> FirstAbsCluNr = 0;
	parent -> map = root_map;
	return (0);
    }
    return (-1);
}
/*-----------------------------------------------------------------------------
 * open_file --
 *-----------------------------------------------------------------------------
 */
int open_file (Slot_t *file, Directory_t *dir)
{
    int first;
    unsigned long size;

    first = __le16_to_cpu (dir -> start);

    if(first == 0 &&
       (dir -> attr & ATTR_DIRECTORY) != 0) {
	file -> FirstAbsCluNr = 0;
	file -> FileSize = -1;
	file -> map = root_map;
	return (0);
    }

    if ((dir -> attr & ATTR_DIRECTORY) != 0) {
	size = (1UL << 31) - 1;
    }
    else {
	size = __le32_to_cpu (dir -> size);
    }

    file -> map = normal_map;
    file -> FirstAbsCluNr = first;
    file -> PreviousRelCluNr = 0xffff;
    file -> FileSize = size;
    return (0);
}
/*-----------------------------------------------------------------------------
 * read_file --
 *-----------------------------------------------------------------------------
 */
int read_file (Fs_t *fs,
	       Slot_t *file,
	       char *buf,
	       int where,
	       int len)
{
    int pos;
    int read, nb, sect, offset;

    pos = file -> map (fs, file, where, &len);
    if  (pos < 0) {
	return -1;
    }
    if (len == 0) {
	return (0);
    }

    /* Compute sector number                                                 */
    sect = pos / SZ_STD_SECTOR;
    offset = pos % SZ_STD_SECTOR;
    read = 0;

    if (offset) {
	/* Read doesn't start at the sector beginning. We need to use our    */
	/* cache                                                             */
	if (sect != cache_sect) {
	    if (dev_read (cache, sect, 1) < 0) {
		return (-1);
	    }
	    cache_sect = sect;
	}
	nb = min (len, SZ_STD_SECTOR - offset);

	memcpy (buf, cache + offset, nb);
	read += nb;
	len -= nb;
	sect += 1;
    }

    if (len > SZ_STD_SECTOR) {
	nb = (len - 1) / SZ_STD_SECTOR;
	if (dev_read (buf + read, sect, nb) < 0) {
	    return ((read) ? read : -1);
	}
	/* update sector position                                            */
	sect += nb;

	/* Update byte position                                              */
	nb *= SZ_STD_SECTOR;
	read += nb;
	len -= nb;
    }

    if (len) {
	if (sect != cache_sect) {
	    if (dev_read (cache, sect, 1) < 0) {
		return ((read) ? read : -1);
		cache_sect = -1;
	    }
	    cache_sect = sect;
	}

	memcpy (buf + read, cache, len);
	read += len;
    }
    return (read);
}
