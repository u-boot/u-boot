/*
 * fat.c
 *
 * R/O (V)FAT 12/16/32 filesystem implementation by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to uboot
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
#include <fat.h>
#include <asm/byteorder.h>
#include <part.h>

/*
 * Convert a string to lowercase.
 */
static void downcase (char *str)
{
	while (*str != '\0') {
		TOLOWER(*str);
		str++;
	}
}

static block_dev_desc_t *cur_dev = NULL;

static unsigned long part_offset = 0;

static int cur_part = 1;

#define DOS_PART_TBL_OFFSET	0x1be
#define DOS_PART_MAGIC_OFFSET	0x1fe
#define DOS_FS_TYPE_OFFSET	0x36
#define DOS_FS32_TYPE_OFFSET	0x52

static int disk_read (__u32 startblock, __u32 getsize, __u8 * bufptr)
{
	if (cur_dev == NULL)
		return -1;

	startblock += part_offset;

	if (cur_dev->block_read) {
		return cur_dev->block_read(cur_dev->dev, startblock, getsize,
					   (unsigned long *) bufptr);
	}
	return -1;
}

int fat_register_device (block_dev_desc_t * dev_desc, int part_no)
{
	unsigned char buffer[SECTOR_SIZE];

	disk_partition_t info;

	if (!dev_desc->block_read)
		return -1;

	cur_dev = dev_desc;
	/* check if we have a MBR (on floppies we have only a PBR) */
	if (dev_desc->block_read(dev_desc->dev, 0, 1, (ulong *)buffer) != 1) {
		printf("** Can't read from device %d **\n",
			dev_desc->dev);
		return -1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 ||
	    buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) {
		/* no signature found */
		return -1;
	}
#if (defined(CONFIG_CMD_IDE) || \
     defined(CONFIG_CMD_MG_DISK) || \
     defined(CONFIG_CMD_SATA) || \
     defined(CONFIG_CMD_SCSI) || \
     defined(CONFIG_CMD_USB) || \
     defined(CONFIG_MMC) || \
     defined(CONFIG_SYSTEMACE) )
	/* First we assume there is a MBR */
	if (!get_partition_info(dev_desc, part_no, &info)) {
		part_offset = info.start;
		cur_part = part_no;
	} else if ((strncmp((char *)&buffer[DOS_FS_TYPE_OFFSET], "FAT", 3) == 0) ||
		   (strncmp((char *)&buffer[DOS_FS32_TYPE_OFFSET], "FAT32", 5) == 0)) {
		/* ok, we assume we are on a PBR only */
		cur_part = 1;
		part_offset = 0;
	} else {
		printf("** Partition %d not valid on device %d **\n",
			part_no, dev_desc->dev);
		return -1;
	}

#else
	if ((strncmp((char *)&buffer[DOS_FS_TYPE_OFFSET], "FAT", 3) == 0) ||
	    (strncmp((char *)&buffer[DOS_FS32_TYPE_OFFSET], "FAT32", 5) == 0)) {
		/* ok, we assume we are on a PBR only */
		cur_part = 1;
		part_offset = 0;
		info.start = part_offset;
	} else {
		/* FIXME we need to determine the start block of the
		 * partition where the DOS FS resides. This can be done
		 * by using the get_partition_info routine. For this
		 * purpose the libpart must be included.
		 */
		part_offset = 32;
		cur_part = 1;
	}
#endif
	return 0;
}

/*
 * Get the first occurence of a directory delimiter ('/' or '\') in a string.
 * Return index into string if found, -1 otherwise.
 */
static int dirdelim (char *str)
{
	char *start = str;

	while (*str != '\0') {
		if (ISDIRDELIM(*str))
			return str - start;
		str++;
	}
	return -1;
}

/*
 * Extract zero terminated short name from a directory entry.
 */
static void get_name (dir_entry *dirent, char *s_name)
{
	char *ptr;

	memcpy(s_name, dirent->name, 8);
	s_name[8] = '\0';
	ptr = s_name;
	while (*ptr && *ptr != ' ')
		ptr++;
	if (dirent->ext[0] && dirent->ext[0] != ' ') {
		*ptr = '.';
		ptr++;
		memcpy(ptr, dirent->ext, 3);
		ptr[3] = '\0';
		while (*ptr && *ptr != ' ')
			ptr++;
	}
	*ptr = '\0';
	if (*s_name == DELETED_FLAG)
		*s_name = '\0';
	else if (*s_name == aRING)
		*s_name = DELETED_FLAG;
	downcase(s_name);
}

/*
 * Get the entry at index 'entry' in a FAT (12/16/32) table.
 * On failure 0x00 is returned.
 */
static __u32 get_fatent (fsdata *mydata, __u32 entry)
{
	__u32 bufnum;
	__u32 off16, offset;
	__u32 ret = 0x00;
	__u16 val1, val2;

	switch (mydata->fatsize) {
	case 32:
		bufnum = entry / FAT32BUFSIZE;
		offset = entry - bufnum * FAT32BUFSIZE;
		break;
	case 16:
		bufnum = entry / FAT16BUFSIZE;
		offset = entry - bufnum * FAT16BUFSIZE;
		break;
	case 12:
		bufnum = entry / FAT12BUFSIZE;
		offset = entry - bufnum * FAT12BUFSIZE;
		break;

	default:
		/* Unsupported FAT size */
		return ret;
	}

	debug("FAT%d: entry: 0x%04x = %d, offset: 0x%04x = %d\n",
	       mydata->fatsize, entry, entry, offset, offset);

	/* Read a new block of FAT entries into the cache. */
	if (bufnum != mydata->fatbufnum) {
		int getsize = FATBUFSIZE / FS_BLOCK_SIZE;
		__u8 *bufptr = mydata->fatbuf;
		__u32 fatlength = mydata->fatlength;
		__u32 startblock = bufnum * FATBUFBLOCKS;

		fatlength *= SECTOR_SIZE;	/* We want it in bytes now */
		startblock += mydata->fat_sect;	/* Offset from start of disk */

		if (getsize > fatlength)
			getsize = fatlength;
		if (disk_read(startblock, getsize, bufptr) < 0) {
			debug("Error reading FAT blocks\n");
			return ret;
		}
		mydata->fatbufnum = bufnum;
	}

	/* Get the actual entry from the table */
	switch (mydata->fatsize) {
	case 32:
		ret = FAT2CPU32(((__u32 *) mydata->fatbuf)[offset]);
		break;
	case 16:
		ret = FAT2CPU16(((__u16 *) mydata->fatbuf)[offset]);
		break;
	case 12:
		off16 = (offset * 3) / 4;

		switch (offset & 0x3) {
		case 0:
			ret = FAT2CPU16(((__u16 *) mydata->fatbuf)[off16]);
			ret &= 0xfff;
			break;
		case 1:
			val1 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16]);
			val1 &= 0xf000;
			val2 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16 + 1]);
			val2 &= 0x00ff;
			ret = (val2 << 4) | (val1 >> 12);
			break;
		case 2:
			val1 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16]);
			val1 &= 0xff00;
			val2 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16 + 1]);
			val2 &= 0x000f;
			ret = (val2 << 8) | (val1 >> 8);
			break;
		case 3:
			ret = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16]);
			ret = (ret & 0xfff0) >> 4;
			break;
		default:
			break;
		}
		break;
	}
	debug("FAT%d: ret: %08x, offset: %04x\n",
	       mydata->fatsize, ret, offset);

	return ret;
}

/*
 * Read at most 'size' bytes from the specified cluster into 'buffer'.
 * Return 0 on success, -1 otherwise.
 */
static int
get_cluster (fsdata *mydata, __u32 clustnum, __u8 *buffer,
	     unsigned long size)
{
	int idx = 0;
	__u32 startsect;

	if (clustnum > 0) {
		startsect = mydata->data_begin +
				clustnum * mydata->clust_size;
	} else {
		startsect = mydata->rootdir_sect;
	}

	debug("gc - clustnum: %d, startsect: %d\n", clustnum, startsect);

	if (disk_read(startsect, size / FS_BLOCK_SIZE, buffer) < 0) {
		debug("Error reading data\n");
		return -1;
	}
	if (size % FS_BLOCK_SIZE) {
		__u8 tmpbuf[FS_BLOCK_SIZE];

		idx = size / FS_BLOCK_SIZE;
		if (disk_read(startsect + idx, 1, tmpbuf) < 0) {
			debug("Error reading data\n");
			return -1;
		}
		buffer += idx * FS_BLOCK_SIZE;

		memcpy(buffer, tmpbuf, size % FS_BLOCK_SIZE);
		return 0;
	}

	return 0;
}

/*
 * Read at most 'maxsize' bytes from the file associated with 'dentptr'
 * into 'buffer'.
 * Return the number of bytes read or -1 on fatal errors.
 */
static long
get_contents (fsdata *mydata, dir_entry *dentptr, __u8 *buffer,
	      unsigned long maxsize)
{
	unsigned long filesize = FAT2CPU32(dentptr->size), gotsize = 0;
	unsigned int bytesperclust = mydata->clust_size * SECTOR_SIZE;
	__u32 curclust = START(dentptr);
	__u32 endclust, newclust;
	unsigned long actsize;

	debug("Filesize: %ld bytes\n", filesize);

	if (maxsize > 0 && filesize > maxsize)
		filesize = maxsize;

	debug("%ld bytes\n", filesize);

	actsize = bytesperclust;
	endclust = curclust;

	do {
		/* search for consecutive clusters */
		while (actsize < filesize) {
			newclust = get_fatent(mydata, endclust);
			if ((newclust - 1) != endclust)
				goto getit;
			if (CHECK_CLUST(newclust, mydata->fatsize)) {
				debug("curclust: 0x%x\n", newclust);
				debug("Invalid FAT entry\n");
				return gotsize;
			}
			endclust = newclust;
			actsize += bytesperclust;
		}

		/* actsize >= file size */
		actsize -= bytesperclust;

		/* get remaining clusters */
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}

		/* get remaining bytes */
		gotsize += (int)actsize;
		filesize -= actsize;
		buffer += actsize;
		actsize = filesize;
		if (get_cluster(mydata, endclust, buffer, (int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}
		gotsize += actsize;
		return gotsize;
getit:
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}
		gotsize += (int)actsize;
		filesize -= actsize;
		buffer += actsize;

		curclust = get_fatent(mydata, endclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return gotsize;
		}
		actsize = bytesperclust;
		endclust = curclust;
	} while (1);
}

#ifdef CONFIG_SUPPORT_VFAT
/*
 * Extract the file name information from 'slotptr' into 'l_name',
 * starting at l_name[*idx].
 * Return 1 if terminator (zero byte) is found, 0 otherwise.
 */
static int slot2str (dir_slot *slotptr, char *l_name, int *idx)
{
	int j;

	for (j = 0; j <= 8; j += 2) {
		l_name[*idx] = slotptr->name0_4[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}
	for (j = 0; j <= 10; j += 2) {
		l_name[*idx] = slotptr->name5_10[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}
	for (j = 0; j <= 2; j += 2) {
		l_name[*idx] = slotptr->name11_12[j];
		if (l_name[*idx] == 0x00)
			return 1;
		(*idx)++;
	}

	return 0;
}

/*
 * Extract the full long filename starting at 'retdent' (which is really
 * a slot) into 'l_name'. If successful also copy the real directory entry
 * into 'retdent'
 * Return 0 on success, -1 otherwise.
 */
__attribute__ ((__aligned__ (__alignof__ (dir_entry))))
__u8 get_vfatname_block[MAX_CLUSTSIZE];

static int
get_vfatname (fsdata *mydata, int curclust, __u8 *cluster,
	      dir_entry *retdent, char *l_name)
{
	dir_entry *realdent;
	dir_slot *slotptr = (dir_slot *)retdent;
	__u8 *nextclust = cluster + mydata->clust_size * SECTOR_SIZE;
	__u8 counter = (slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff;
	int idx = 0;

	while ((__u8 *)slotptr < nextclust) {
		if (counter == 0)
			break;
		if (((slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff) != counter)
			return -1;
		slotptr++;
		counter--;
	}

	if ((__u8 *)slotptr >= nextclust) {
		dir_slot *slotptr2;

		slotptr--;
		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}

		if (get_cluster(mydata, curclust, get_vfatname_block,
				mydata->clust_size * SECTOR_SIZE) != 0) {
			debug("Error: reading directory block\n");
			return -1;
		}

		slotptr2 = (dir_slot *)get_vfatname_block;
		while (slotptr2->id > 0x01)
			slotptr2++;

		/* Save the real directory entry */
		realdent = (dir_entry *)slotptr2 + 1;
		while ((__u8 *)slotptr2 >= get_vfatname_block) {
			slot2str(slotptr2, l_name, &idx);
			slotptr2--;
		}
	} else {
		/* Save the real directory entry */
		realdent = (dir_entry *)slotptr;
	}

	do {
		slotptr--;
		if (slot2str(slotptr, l_name, &idx))
			break;
	} while (!(slotptr->id & LAST_LONG_ENTRY_MASK));

	l_name[idx] = '\0';
	if (*l_name == DELETED_FLAG)
		*l_name = '\0';
	else if (*l_name == aRING)
		*l_name = DELETED_FLAG;
	downcase(l_name);

	/* Return the real directory entry */
	memcpy(retdent, realdent, sizeof(dir_entry));

	return 0;
}

/* Calculate short name checksum */
static __u8 mkcksum (const char *str)
{
	int i;

	__u8 ret = 0;

	for (i = 0; i < 11; i++) {
		ret = (((ret & 1) << 7) | ((ret & 0xfe) >> 1)) + str[i];
	}

	return ret;
}
#endif	/* CONFIG_SUPPORT_VFAT */

/*
 * Get the directory entry associated with 'filename' from the directory
 * starting at 'startsect'
 */
__attribute__ ((__aligned__ (__alignof__ (dir_entry))))
__u8 get_dentfromdir_block[MAX_CLUSTSIZE];

static dir_entry *get_dentfromdir (fsdata *mydata, int startsect,
				   char *filename, dir_entry *retdent,
				   int dols)
{
	__u16 prevcksum = 0xffff;
	__u32 curclust = START(retdent);
	int files = 0, dirs = 0;

	debug("get_dentfromdir: %s\n", filename);

	while (1) {
		dir_entry *dentptr;

		int i;

		if (get_cluster(mydata, curclust, get_dentfromdir_block,
				mydata->clust_size * SECTOR_SIZE) != 0) {
			debug("Error: reading directory block\n");
			return NULL;
		}

		dentptr = (dir_entry *)get_dentfromdir_block;

		for (i = 0; i < DIRENTSPERCLUST; i++) {
			char s_name[14], l_name[256];

			l_name[0] = '\0';
			if (dentptr->name[0] == DELETED_FLAG) {
				dentptr++;
				continue;
			}
			if ((dentptr->attr & ATTR_VOLUME)) {
#ifdef CONFIG_SUPPORT_VFAT
				if ((dentptr->attr & ATTR_VFAT) &&
				    (dentptr-> name[0] & LAST_LONG_ENTRY_MASK)) {
					prevcksum = ((dir_slot *)dentptr)->alias_checksum;
					get_vfatname(mydata, curclust,
						     get_dentfromdir_block,
						     dentptr, l_name);
					if (dols) {
						int isdir;
						char dirc;
						int doit = 0;

						isdir = (dentptr->attr & ATTR_DIR);

						if (isdir) {
							dirs++;
							dirc = '/';
							doit = 1;
						} else {
							dirc = ' ';
							if (l_name[0] != 0) {
								files++;
								doit = 1;
							}
						}
						if (doit) {
							if (dirc == ' ') {
								printf(" %8ld   %s%c\n",
									(long)FAT2CPU32(dentptr->size),
									l_name,
									dirc);
							} else {
								printf("            %s%c\n",
									l_name,
									dirc);
							}
						}
						dentptr++;
						continue;
					}
					debug("vfatname: |%s|\n", l_name);
				} else
#endif
				{
					/* Volume label or VFAT entry */
					dentptr++;
					continue;
				}
			}
			if (dentptr->name[0] == 0) {
				if (dols) {
					printf("\n%d file(s), %d dir(s)\n\n",
						files, dirs);
				}
				debug("Dentname == NULL - %d\n", i);
				return NULL;
			}
#ifdef CONFIG_SUPPORT_VFAT
			if (dols && mkcksum(dentptr->name) == prevcksum) {
				dentptr++;
				continue;
			}
#endif
			get_name(dentptr, s_name);
			if (dols) {
				int isdir = (dentptr->attr & ATTR_DIR);
				char dirc;
				int doit = 0;

				if (isdir) {
					dirs++;
					dirc = '/';
					doit = 1;
				} else {
					dirc = ' ';
					if (s_name[0] != 0) {
						files++;
						doit = 1;
					}
				}

				if (doit) {
					if (dirc == ' ') {
						printf(" %8ld   %s%c\n",
							(long)FAT2CPU32(dentptr->size),
							s_name, dirc);
					} else {
						printf("            %s%c\n",
							s_name, dirc);
					}
				}

				dentptr++;
				continue;
			}

			if (strcmp(filename, s_name)
			    && strcmp(filename, l_name)) {
				debug("Mismatch: |%s|%s|\n", s_name, l_name);
				dentptr++;
				continue;
			}

			memcpy(retdent, dentptr, sizeof(dir_entry));

			debug("DentName: %s", s_name);
			debug(", start: 0x%x", START(dentptr));
			debug(", size:  0x%x %s\n",
			      FAT2CPU32(dentptr->size),
			      (dentptr->attr & ATTR_DIR) ? "(DIR)" : "");

			return retdent;
		}

		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return NULL;
		}
	}

	return NULL;
}

/*
 * Read boot sector and volume info from a FAT filesystem
 */
static int
read_bootsectandvi (boot_sector *bs, volume_info *volinfo, int *fatsize)
{
	__u8 block[FS_BLOCK_SIZE];

	volume_info *vistart;

	if (disk_read (0, 1, block) < 0) {
		debug("Error: reading block\n");
		return -1;
	}

	memcpy(bs, block, sizeof(boot_sector));
	bs->reserved = FAT2CPU16(bs->reserved);
	bs->fat_length = FAT2CPU16(bs->fat_length);
	bs->secs_track = FAT2CPU16(bs->secs_track);
	bs->heads = FAT2CPU16(bs->heads);
	bs->total_sect = FAT2CPU32(bs->total_sect);

	/* FAT32 entries */
	if (bs->fat_length == 0) {
		/* Assume FAT32 */
		bs->fat32_length = FAT2CPU32(bs->fat32_length);
		bs->flags = FAT2CPU16(bs->flags);
		bs->root_cluster = FAT2CPU32(bs->root_cluster);
		bs->info_sector = FAT2CPU16(bs->info_sector);
		bs->backup_boot = FAT2CPU16(bs->backup_boot);
		vistart = (volume_info *)(block + sizeof(boot_sector));
		*fatsize = 32;
	} else {
		vistart = (volume_info *)&(bs->fat32_length);
		*fatsize = 0;
	}
	memcpy(volinfo, vistart, sizeof(volume_info));

	if (*fatsize == 32) {
		if (strncmp(FAT32_SIGN, vistart->fs_type, SIGNLEN) == 0)
			return 0;
	} else {
		if (strncmp(FAT12_SIGN, vistart->fs_type, SIGNLEN) == 0) {
			*fatsize = 12;
			return 0;
		}
		if (strncmp(FAT16_SIGN, vistart->fs_type, SIGNLEN) == 0) {
			*fatsize = 16;
			return 0;
		}
	}

	debug("Error: broken fs_type sign\n");
	return -1;
}

__attribute__ ((__aligned__ (__alignof__ (dir_entry))))
__u8 do_fat_read_block[MAX_CLUSTSIZE];

long
do_fat_read (const char *filename, void *buffer, unsigned long maxsize,
	     int dols)
{
	char fnamecopy[2048];
	boot_sector bs;
	volume_info volinfo;
	fsdata datablock;
	fsdata *mydata = &datablock;
	dir_entry *dentptr;
	__u16 prevcksum = 0xffff;
	char *subname = "";
	int cursect;
	int idx, isdir = 0;
	int files = 0, dirs = 0;
	long ret = 0;
	int firsttime;
	int root_cluster;
	int j;

	if (read_bootsectandvi(&bs, &volinfo, &mydata->fatsize)) {
		debug("Error: reading boot sector\n");
		return -1;
	}

	root_cluster = bs.root_cluster;

	if (mydata->fatsize == 32)
		mydata->fatlength = bs.fat32_length;
	else
		mydata->fatlength = bs.fat_length;

	mydata->fat_sect = bs.reserved;

	cursect = mydata->rootdir_sect
		= mydata->fat_sect + mydata->fatlength * bs.fats;

	mydata->clust_size = bs.cluster_size;

	if (mydata->fatsize == 32) {
		mydata->data_begin = mydata->rootdir_sect -
					(mydata->clust_size * 2);
	} else {
		int rootdir_size;

		rootdir_size = ((bs.dir_entries[1]  * (int)256 +
				 bs.dir_entries[0]) *
				 sizeof(dir_entry)) /
				 SECTOR_SIZE;
		mydata->data_begin = mydata->rootdir_sect +
					rootdir_size -
					(mydata->clust_size * 2);
	}

	mydata->fatbufnum = -1;

#ifdef CONFIG_SUPPORT_VFAT
	debug("VFAT Support enabled\n");
#endif
	debug("FAT%d, fat_sect: %d, fatlength: %d\n",
	       mydata->fatsize, mydata->fat_sect, mydata->fatlength);
	debug("Rootdir begins at cluster: %d, sector: %d, offset: %x\n"
	       "Data begins at: %d\n",
	       root_cluster,
	       mydata->rootdir_sect,
	       mydata->rootdir_sect * SECTOR_SIZE, mydata->data_begin);
	debug("Cluster size: %d\n", mydata->clust_size);

	/* "cwd" is always the root... */
	while (ISDIRDELIM(*filename))
		filename++;

	/* Make a copy of the filename and convert it to lowercase */
	strcpy(fnamecopy, filename);
	downcase(fnamecopy);

	if (*fnamecopy == '\0') {
		if (!dols)
			return -1;

		dols = LS_ROOT;
	} else if ((idx = dirdelim(fnamecopy)) >= 0) {
		isdir = 1;
		fnamecopy[idx] = '\0';
		subname = fnamecopy + idx + 1;

		/* Handle multiple delimiters */
		while (ISDIRDELIM(*subname))
			subname++;
	} else if (dols) {
		isdir = 1;
	}

	j = 0;
	while (1) {
		int i;

		debug("FAT read sect=%d, clust_size=%d, DIRENTSPERBLOCK=%d\n",
			cursect, mydata->clust_size, DIRENTSPERBLOCK);

		if (disk_read(cursect, mydata->clust_size, do_fat_read_block) < 0) {
			debug("Error: reading rootdir block\n");
			return -1;
		}

		dentptr = (dir_entry *) do_fat_read_block;

		for (i = 0; i < DIRENTSPERBLOCK; i++) {
			char s_name[14], l_name[256];

			l_name[0] = '\0';
			if ((dentptr->attr & ATTR_VOLUME)) {
#ifdef CONFIG_SUPPORT_VFAT
				if ((dentptr->attr & ATTR_VFAT) &&
				    (dentptr->name[0] & LAST_LONG_ENTRY_MASK)) {
					prevcksum =
						((dir_slot *)dentptr)->alias_checksum;

					get_vfatname(mydata, 0,
						     do_fat_read_block,
						     dentptr, l_name);

					if (dols == LS_ROOT) {
						char dirc;
						int doit = 0;
						int isdir =
							(dentptr->attr & ATTR_DIR);

						if (isdir) {
							dirs++;
							dirc = '/';
							doit = 1;
						} else {
							dirc = ' ';
							if (l_name[0] != 0) {
								files++;
								doit = 1;
							}
						}
						if (doit) {
							if (dirc == ' ') {
								printf(" %8ld   %s%c\n",
									(long)FAT2CPU32(dentptr->size),
									l_name,
									dirc);
							} else {
								printf("            %s%c\n",
									l_name,
									dirc);
							}
						}
						dentptr++;
						continue;
					}
					debug("Rootvfatname: |%s|\n",
					       l_name);
				} else
#endif
				{
					/* Volume label or VFAT entry */
					dentptr++;
					continue;
				}
			} else if (dentptr->name[0] == 0) {
				debug("RootDentname == NULL - %d\n", i);
				if (dols == LS_ROOT) {
					printf("\n%d file(s), %d dir(s)\n\n",
						files, dirs);
					return 0;
				}
				return -1;
			}
#ifdef CONFIG_SUPPORT_VFAT
			else if (dols == LS_ROOT &&
				 mkcksum(dentptr->name) == prevcksum) {
				dentptr++;
				continue;
			}
#endif
			get_name(dentptr, s_name);

			if (dols == LS_ROOT) {
				int isdir = (dentptr->attr & ATTR_DIR);
				char dirc;
				int doit = 0;

				if (isdir) {
					dirc = '/';
					if (s_name[0] != 0) {
						dirs++;
						doit = 1;
					}
				} else {
					dirc = ' ';
					if (s_name[0] != 0) {
						files++;
						doit = 1;
					}
				}
				if (doit) {
					if (dirc == ' ') {
						printf(" %8ld   %s%c\n",
							(long)FAT2CPU32(dentptr->size),
							s_name, dirc);
					} else {
						printf("            %s%c\n",
							s_name, dirc);
					}
				}
				dentptr++;
				continue;
			}

			if (strcmp(fnamecopy, s_name)
			    && strcmp(fnamecopy, l_name)) {
				debug("RootMismatch: |%s|%s|\n", s_name,
				       l_name);
				dentptr++;
				continue;
			}

			if (isdir && !(dentptr->attr & ATTR_DIR))
				return -1;

			debug("RootName: %s", s_name);
			debug(", start: 0x%x", START(dentptr));
			debug(", size:  0x%x %s\n",
			       FAT2CPU32(dentptr->size),
			       isdir ? "(DIR)" : "");

			goto rootdir_done;	/* We got a match */
		}
		debug("END LOOP: j=%d   clust_size=%d\n", j,
		       mydata->clust_size);

		/*
		 * On FAT32 we must fetch the FAT entries for the next
		 * root directory clusters when a cluster has been
		 * completely processed.
		 */
		if ((mydata->fatsize == 32) && (++j == mydata->clust_size)) {
			int nxtsect;
			int nxt_clust;

			nxt_clust = get_fatent(mydata, root_cluster);

			nxtsect = mydata->data_begin +
				(nxt_clust * mydata->clust_size);

			debug("END LOOP: sect=%d, root_clust=%d, "
			      "n_sect=%d, n_clust=%d\n",
			      cursect, root_cluster,
			      nxtsect, nxt_clust);

			root_cluster = nxt_clust;

			cursect = nxtsect;
			j = 0;
		} else {
			cursect++;
		}
	}
rootdir_done:

	firsttime = 1;

	while (isdir) {
		int startsect = mydata->data_begin
			+ START(dentptr) * mydata->clust_size;
		dir_entry dent;
		char *nextname = NULL;

		dent = *dentptr;
		dentptr = &dent;

		idx = dirdelim(subname);

		if (idx >= 0) {
			subname[idx] = '\0';
			nextname = subname + idx + 1;
			/* Handle multiple delimiters */
			while (ISDIRDELIM(*nextname))
				nextname++;
			if (dols && *nextname == '\0')
				firsttime = 0;
		} else {
			if (dols && firsttime) {
				firsttime = 0;
			} else {
				isdir = 0;
			}
		}

		if (get_dentfromdir(mydata, startsect, subname, dentptr,
				     isdir ? 0 : dols) == NULL) {
			if (dols && !isdir)
				return 0;
			return -1;
		}

		if (idx >= 0) {
			if (!(dentptr->attr & ATTR_DIR))
				return -1;
			subname = nextname;
		}
	}

	ret = get_contents(mydata, dentptr, buffer, maxsize);
	debug("Size: %d, got: %ld\n", FAT2CPU32(dentptr->size), ret);

	return ret;
}

int file_fat_detectfs (void)
{
	boot_sector bs;
	volume_info volinfo;
	int fatsize;
	char vol_label[12];

	if (cur_dev == NULL) {
		printf("No current device\n");
		return 1;
	}

#if defined(CONFIG_CMD_IDE) || \
    defined(CONFIG_CMD_MG_DISK) || \
    defined(CONFIG_CMD_SATA) || \
    defined(CONFIG_CMD_SCSI) || \
    defined(CONFIG_CMD_USB) || \
    defined(CONFIG_MMC)
	printf("Interface:  ");
	switch (cur_dev->if_type) {
	case IF_TYPE_IDE:
		printf("IDE");
		break;
	case IF_TYPE_SATA:
		printf("SATA");
		break;
	case IF_TYPE_SCSI:
		printf("SCSI");
		break;
	case IF_TYPE_ATAPI:
		printf("ATAPI");
		break;
	case IF_TYPE_USB:
		printf("USB");
		break;
	case IF_TYPE_DOC:
		printf("DOC");
		break;
	case IF_TYPE_MMC:
		printf("MMC");
		break;
	default:
		printf("Unknown");
	}

	printf("\n  Device %d: ", cur_dev->dev);
	dev_print(cur_dev);
#endif

	if (read_bootsectandvi(&bs, &volinfo, &fatsize)) {
		printf("\nNo valid FAT fs found\n");
		return 1;
	}

	memcpy(vol_label, volinfo.volume_label, 11);
	vol_label[11] = '\0';
	volinfo.fs_type[5] = '\0';

	printf("Partition %d: Filesystem: %s \"%s\"\n", cur_part,
		volinfo.fs_type, vol_label);

	return 0;
}

int file_fat_ls (const char *dir)
{
	return do_fat_read(dir, NULL, 0, LS_YES);
}

long file_fat_read (const char *filename, void *buffer, unsigned long maxsize)
{
	printf("reading %s\n", filename);
	return do_fat_read(filename, buffer, maxsize, LS_NO);
}
