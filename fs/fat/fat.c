/*
 * fat.c
 *
 * R/O (V)FAT 12/16/32 filesystem implementation by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to uboot
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <blk.h>
#include <config.h>
#include <exports.h>
#include <fat.h>
#include <asm/byteorder.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/compiler.h>
#include <linux/ctype.h>

#ifdef CONFIG_SUPPORT_VFAT
static const int vfat_enabled = 1;
#else
static const int vfat_enabled = 0;
#endif

/*
 * Convert a string to lowercase.
 */
static void downcase(char *str)
{
	while (*str != '\0') {
		*str = tolower(*str);
		str++;
	}
}

static struct blk_desc *cur_dev;
static disk_partition_t cur_part_info;

#define DOS_BOOT_MAGIC_OFFSET	0x1fe
#define DOS_FS_TYPE_OFFSET	0x36
#define DOS_FS32_TYPE_OFFSET	0x52

static int disk_read(__u32 block, __u32 nr_blocks, void *buf)
{
	ulong ret;

	if (!cur_dev)
		return -1;

	ret = blk_dread(cur_dev, cur_part_info.start + block, nr_blocks, buf);

	if (nr_blocks && ret == 0)
		return -1;

	return ret;
}

int fat_set_blk_dev(struct blk_desc *dev_desc, disk_partition_t *info)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);

	cur_dev = dev_desc;
	cur_part_info = *info;

	/* Make sure it has a valid FAT header */
	if (disk_read(0, 1, buffer) != 1) {
		cur_dev = NULL;
		return -1;
	}

	/* Check if it's actually a DOS volume */
	if (memcmp(buffer + DOS_BOOT_MAGIC_OFFSET, "\x55\xAA", 2)) {
		cur_dev = NULL;
		return -1;
	}

	/* Check for FAT12/FAT16/FAT32 filesystem */
	if (!memcmp(buffer + DOS_FS_TYPE_OFFSET, "FAT", 3))
		return 0;
	if (!memcmp(buffer + DOS_FS32_TYPE_OFFSET, "FAT32", 5))
		return 0;

	cur_dev = NULL;
	return -1;
}

int fat_register_device(struct blk_desc *dev_desc, int part_no)
{
	disk_partition_t info;

	/* First close any currently found FAT filesystem */
	cur_dev = NULL;

	/* Read the partition table, if present */
	if (part_get_info(dev_desc, part_no, &info)) {
		if (part_no != 0) {
			printf("** Partition %d not valid on device %d **\n",
					part_no, dev_desc->devnum);
			return -1;
		}

		info.start = 0;
		info.size = dev_desc->lba;
		info.blksz = dev_desc->blksz;
		info.name[0] = 0;
		info.type[0] = 0;
		info.bootable = 0;
#ifdef CONFIG_PARTITION_UUIDS
		info.uuid[0] = 0;
#endif
	}

	return fat_set_blk_dev(dev_desc, &info);
}

/*
 * Get the first occurence of a directory delimiter ('/' or '\') in a string.
 * Return index into string if found, -1 otherwise.
 */
static int dirdelim(char *str)
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
static void get_name(dir_entry *dirent, char *s_name)
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
static __u32 get_fatent(fsdata *mydata, __u32 entry)
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
		__u32 getsize = FATBUFBLOCKS;
		__u8 *bufptr = mydata->fatbuf;
		__u32 fatlength = mydata->fatlength;
		__u32 startblock = bufnum * FATBUFBLOCKS;

		if (startblock + getsize > fatlength)
			getsize = fatlength - startblock;

		startblock += mydata->fat_sect;	/* Offset from start of disk */

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
get_cluster(fsdata *mydata, __u32 clustnum, __u8 *buffer, unsigned long size)
{
	__u32 idx = 0;
	__u32 startsect;
	int ret;

	if (clustnum > 0) {
		startsect = mydata->data_begin +
				clustnum * mydata->clust_size;
	} else {
		startsect = mydata->rootdir_sect;
	}

	debug("gc - clustnum: %d, startsect: %d\n", clustnum, startsect);

	if ((unsigned long)buffer & (ARCH_DMA_MINALIGN - 1)) {
		ALLOC_CACHE_ALIGN_BUFFER(__u8, tmpbuf, mydata->sect_size);

		printf("FAT: Misaligned buffer address (%p)\n", buffer);

		while (size >= mydata->sect_size) {
			ret = disk_read(startsect++, 1, tmpbuf);
			if (ret != 1) {
				debug("Error reading data (got %d)\n", ret);
				return -1;
			}

			memcpy(buffer, tmpbuf, mydata->sect_size);
			buffer += mydata->sect_size;
			size -= mydata->sect_size;
		}
	} else {
		idx = size / mydata->sect_size;
		ret = disk_read(startsect, idx, buffer);
		if (ret != idx) {
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}
		startsect += idx;
		idx *= mydata->sect_size;
		buffer += idx;
		size -= idx;
	}
	if (size) {
		ALLOC_CACHE_ALIGN_BUFFER(__u8, tmpbuf, mydata->sect_size);

		ret = disk_read(startsect, 1, tmpbuf);
		if (ret != 1) {
			debug("Error reading data (got %d)\n", ret);
			return -1;
		}

		memcpy(buffer, tmpbuf, size);
	}

	return 0;
}

/*
 * Read at most 'maxsize' bytes from 'pos' in the file associated with 'dentptr'
 * into 'buffer'.
 * Update the number of bytes read in *gotsize or return -1 on fatal errors.
 */
__u8 get_contents_vfatname_block[MAX_CLUSTSIZE]
	__aligned(ARCH_DMA_MINALIGN);

static int get_contents(fsdata *mydata, dir_entry *dentptr, loff_t pos,
			__u8 *buffer, loff_t maxsize, loff_t *gotsize)
{
	loff_t filesize = FAT2CPU32(dentptr->size);
	unsigned int bytesperclust = mydata->clust_size * mydata->sect_size;
	__u32 curclust = START(dentptr);
	__u32 endclust, newclust;
	loff_t actsize;

	*gotsize = 0;
	debug("Filesize: %llu bytes\n", filesize);

	if (pos >= filesize) {
		debug("Read position past EOF: %llu\n", pos);
		return 0;
	}

	if (maxsize > 0 && filesize > pos + maxsize)
		filesize = pos + maxsize;

	debug("%llu bytes\n", filesize);

	actsize = bytesperclust;

	/* go to cluster at pos */
	while (actsize <= pos) {
		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			debug("Invalid FAT entry\n");
			return 0;
		}
		actsize += bytesperclust;
	}

	/* actsize > pos */
	actsize -= bytesperclust;
	filesize -= actsize;
	pos -= actsize;

	/* align to beginning of next cluster if any */
	if (pos) {
		actsize = min(filesize, (loff_t)bytesperclust);
		if (get_cluster(mydata, curclust, get_contents_vfatname_block,
				(int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}
		filesize -= actsize;
		actsize -= pos;
		memcpy(buffer, get_contents_vfatname_block + pos, actsize);
		*gotsize += actsize;
		if (!filesize)
			return 0;
		buffer += actsize;

		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			debug("Invalid FAT entry\n");
			return 0;
		}
	}

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
				return 0;
			}
			endclust = newclust;
			actsize += bytesperclust;
		}

		/* get remaining bytes */
		actsize = filesize;
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}
		*gotsize += actsize;
		return 0;
getit:
		if (get_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			printf("Error reading cluster\n");
			return -1;
		}
		*gotsize += (int)actsize;
		filesize -= actsize;
		buffer += actsize;

		curclust = get_fatent(mydata, endclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return 0;
		}
		actsize = bytesperclust;
		endclust = curclust;
	} while (1);
}

/*
 * Extract the file name information from 'slotptr' into 'l_name',
 * starting at l_name[*idx].
 * Return 1 if terminator (zero byte) is found, 0 otherwise.
 */
static int slot2str(dir_slot *slotptr, char *l_name, int *idx)
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
static int
get_vfatname(fsdata *mydata, int curclust, __u8 *cluster,
	     dir_entry *retdent, char *l_name)
{
	dir_entry *realdent;
	dir_slot *slotptr = (dir_slot *)retdent;
	__u8 *buflimit = cluster + mydata->sect_size * ((curclust == 0) ?
							PREFETCH_BLOCKS :
							mydata->clust_size);
	__u8 counter = (slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff;
	int idx = 0;

	if (counter > VFAT_MAXSEQ) {
		debug("Error: VFAT name is too long\n");
		return -1;
	}

	while ((__u8 *)slotptr < buflimit) {
		if (counter == 0)
			break;
		if (((slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff) != counter)
			return -1;
		slotptr++;
		counter--;
	}

	if ((__u8 *)slotptr >= buflimit) {
		dir_slot *slotptr2;

		if (curclust == 0)
			return -1;
		curclust = get_fatent(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}

		if (get_cluster(mydata, curclust, get_contents_vfatname_block,
				mydata->clust_size * mydata->sect_size) != 0) {
			debug("Error: reading directory block\n");
			return -1;
		}

		slotptr2 = (dir_slot *)get_contents_vfatname_block;
		while (counter > 0) {
			if (((slotptr2->id & ~LAST_LONG_ENTRY_MASK)
			    & 0xff) != counter)
				return -1;
			slotptr2++;
			counter--;
		}

		/* Save the real directory entry */
		realdent = (dir_entry *)slotptr2;
		while ((__u8 *)slotptr2 > get_contents_vfatname_block) {
			slotptr2--;
			slot2str(slotptr2, l_name, &idx);
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
static __u8 mkcksum(const char name[8], const char ext[3])
{
	int i;

	__u8 ret = 0;

	for (i = 0; i < 8; i++)
		ret = (((ret & 1) << 7) | ((ret & 0xfe) >> 1)) + name[i];
	for (i = 0; i < 3; i++)
		ret = (((ret & 1) << 7) | ((ret & 0xfe) >> 1)) + ext[i];

	return ret;
}

/*
 * Get the directory entry associated with 'filename' from the directory
 * starting at 'startsect'
 */
__u8 get_dentfromdir_block[MAX_CLUSTSIZE]
	__aligned(ARCH_DMA_MINALIGN);

static dir_entry *get_dentfromdir(fsdata *mydata, int startsect,
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
				mydata->clust_size * mydata->sect_size) != 0) {
			debug("Error: reading directory block\n");
			return NULL;
		}

		dentptr = (dir_entry *)get_dentfromdir_block;

		for (i = 0; i < DIRENTSPERCLUST; i++) {
			char s_name[14], l_name[VFAT_MAXLEN_BYTES];

			l_name[0] = '\0';
			if (dentptr->name[0] == DELETED_FLAG) {
				dentptr++;
				continue;
			}
			if ((dentptr->attr & ATTR_VOLUME)) {
				if (vfat_enabled &&
				    (dentptr->attr & ATTR_VFAT) == ATTR_VFAT &&
				    (dentptr->name[0] & LAST_LONG_ENTRY_MASK)) {
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
								printf(" %8u   %s%c\n",
								       FAT2CPU32(dentptr->size),
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
				} else {
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
			if (vfat_enabled) {
				__u8 csum = mkcksum(dentptr->name, dentptr->ext);
				if (dols && csum == prevcksum) {
					prevcksum = 0xffff;
					dentptr++;
					continue;
				}
			}

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
						printf(" %8u   %s%c\n",
						       FAT2CPU32(dentptr->size),
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
read_bootsectandvi(boot_sector *bs, volume_info *volinfo, int *fatsize)
{
	__u8 *block;
	volume_info *vistart;
	int ret = 0;

	if (cur_dev == NULL) {
		debug("Error: no device selected\n");
		return -1;
	}

	block = memalign(ARCH_DMA_MINALIGN, cur_dev->blksz);
	if (block == NULL) {
		debug("Error: allocating block\n");
		return -1;
	}

	if (disk_read(0, 1, block) < 0) {
		debug("Error: reading block\n");
		goto fail;
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
			goto exit;
	} else {
		if (strncmp(FAT12_SIGN, vistart->fs_type, SIGNLEN) == 0) {
			*fatsize = 12;
			goto exit;
		}
		if (strncmp(FAT16_SIGN, vistart->fs_type, SIGNLEN) == 0) {
			*fatsize = 16;
			goto exit;
		}
	}

	debug("Error: broken fs_type sign\n");
fail:
	ret = -1;
exit:
	free(block);
	return ret;
}

__u8 do_fat_read_at_block[MAX_CLUSTSIZE]
	__aligned(ARCH_DMA_MINALIGN);

int do_fat_read_at(const char *filename, loff_t pos, void *buffer,
		   loff_t maxsize, int dols, int dogetsize, loff_t *size)
{
	char fnamecopy[2048];
	boot_sector bs;
	volume_info volinfo;
	fsdata datablock;
	fsdata *mydata = &datablock;
	dir_entry *dentptr = NULL;
	__u16 prevcksum = 0xffff;
	char *subname = "";
	__u32 cursect;
	int idx, isdir = 0;
	int files = 0, dirs = 0;
	int ret = -1;
	int firsttime;
	__u32 root_cluster = 0;
	__u32 read_blk;
	int rootdir_size = 0;
	int buffer_blk_cnt;
	int do_read;
	__u8 *dir_ptr;

	if (read_bootsectandvi(&bs, &volinfo, &mydata->fatsize)) {
		debug("Error: reading boot sector\n");
		return -1;
	}

	if (mydata->fatsize == 32) {
		root_cluster = bs.root_cluster;
		mydata->fatlength = bs.fat32_length;
	} else {
		mydata->fatlength = bs.fat_length;
	}

	mydata->fat_sect = bs.reserved;

	cursect = mydata->rootdir_sect
		= mydata->fat_sect + mydata->fatlength * bs.fats;

	mydata->sect_size = (bs.sector_size[1] << 8) + bs.sector_size[0];
	mydata->clust_size = bs.cluster_size;
	if (mydata->sect_size != cur_part_info.blksz) {
		printf("Error: FAT sector size mismatch (fs=%hu, dev=%lu)\n",
				mydata->sect_size, cur_part_info.blksz);
		return -1;
	}

	if (mydata->fatsize == 32) {
		mydata->data_begin = mydata->rootdir_sect -
					(mydata->clust_size * 2);
	} else {
		rootdir_size = ((bs.dir_entries[1]  * (int)256 +
				 bs.dir_entries[0]) *
				 sizeof(dir_entry)) /
				 mydata->sect_size;
		mydata->data_begin = mydata->rootdir_sect +
					rootdir_size -
					(mydata->clust_size * 2);
	}

	mydata->fatbufnum = -1;
	mydata->fatbuf = memalign(ARCH_DMA_MINALIGN, FATBUFSIZE);
	if (mydata->fatbuf == NULL) {
		debug("Error: allocating memory\n");
		return -1;
	}

	if (vfat_enabled)
		debug("VFAT Support enabled\n");

	debug("FAT%d, fat_sect: %d, fatlength: %d\n",
	       mydata->fatsize, mydata->fat_sect, mydata->fatlength);
	debug("Rootdir begins at cluster: %d, sector: %d, offset: %x\n"
	       "Data begins at: %d\n",
	       root_cluster,
	       mydata->rootdir_sect,
	       mydata->rootdir_sect * mydata->sect_size, mydata->data_begin);
	debug("Sector size: %d, cluster size: %d\n", mydata->sect_size,
	      mydata->clust_size);

	/* "cwd" is always the root... */
	while (ISDIRDELIM(*filename))
		filename++;

	/* Make a copy of the filename and convert it to lowercase */
	strcpy(fnamecopy, filename);
	downcase(fnamecopy);

root_reparse:
	if (*fnamecopy == '\0') {
		if (!dols)
			goto exit;

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

	buffer_blk_cnt = 0;
	firsttime = 1;
	while (1) {
		int i;

		if (mydata->fatsize == 32 || firsttime) {
			dir_ptr = do_fat_read_at_block;
			firsttime = 0;
		} else {
			/**
			 * FAT16 sector buffer modification:
			 * Each loop, the second buffered block is moved to
			 * the buffer begin, and two next sectors are read
			 * next to the previously moved one. So the sector
			 * buffer keeps always 3 sectors for fat16.
			 * And the current sector is the buffer second sector
			 * beside the "firsttime" read, when it is the first one.
			 *
			 * PREFETCH_BLOCKS is 2 for FAT16 == loop[0:1]
			 * n = computed root dir sector
			 * loop |  cursect-1  | cursect    | cursect+1  |
			 *   0  |  sector n+0 | sector n+1 | none       |
			 *   1  |  none       | sector n+0 | sector n+1 |
			 *   0  |  sector n+1 | sector n+2 | sector n+3 |
			 *   1  |  sector n+3 | ...
			*/
			dir_ptr = (do_fat_read_at_block + mydata->sect_size);
			memcpy(do_fat_read_at_block, dir_ptr, mydata->sect_size);
		}

		do_read = 1;

		if (mydata->fatsize == 32 && buffer_blk_cnt)
			do_read = 0;

		if (do_read) {
			read_blk = (mydata->fatsize == 32) ?
				    mydata->clust_size : PREFETCH_BLOCKS;

			debug("FAT read(sect=%d, cnt:%d), clust_size=%d, DIRENTSPERBLOCK=%zd\n",
				cursect, read_blk, mydata->clust_size, DIRENTSPERBLOCK);

			if (disk_read(cursect, read_blk, dir_ptr) < 0) {
				debug("Error: reading rootdir block\n");
				goto exit;
			}

			dentptr = (dir_entry *)dir_ptr;
		}

		for (i = 0; i < DIRENTSPERBLOCK; i++) {
			char s_name[14], l_name[VFAT_MAXLEN_BYTES];
			__u8 csum;

			l_name[0] = '\0';
			if (dentptr->name[0] == DELETED_FLAG) {
				dentptr++;
				continue;
			}

			if (vfat_enabled)
				csum = mkcksum(dentptr->name, dentptr->ext);

			if (dentptr->attr & ATTR_VOLUME) {
				if (vfat_enabled &&
				    (dentptr->attr & ATTR_VFAT) == ATTR_VFAT &&
				    (dentptr->name[0] & LAST_LONG_ENTRY_MASK)) {
					prevcksum =
						((dir_slot *)dentptr)->alias_checksum;

					get_vfatname(mydata,
						     root_cluster,
						     dir_ptr,
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
								printf(" %8u   %s%c\n",
								       FAT2CPU32(dentptr->size),
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
				} else {
					/* Volume label or VFAT entry */
					dentptr++;
					continue;
				}
			} else if (dentptr->name[0] == 0) {
				debug("RootDentname == NULL - %d\n", i);
				if (dols == LS_ROOT) {
					printf("\n%d file(s), %d dir(s)\n\n",
						files, dirs);
					ret = 0;
				}
				goto exit;
			}
			else if (vfat_enabled &&
				 dols == LS_ROOT && csum == prevcksum) {
				prevcksum = 0xffff;
				dentptr++;
				continue;
			}

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
						printf(" %8u   %s%c\n",
						       FAT2CPU32(dentptr->size),
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
				goto exit;

			debug("RootName: %s", s_name);
			debug(", start: 0x%x", START(dentptr));
			debug(", size:  0x%x %s\n",
			       FAT2CPU32(dentptr->size),
			       isdir ? "(DIR)" : "");

			goto rootdir_done;	/* We got a match */
		}
		debug("END LOOP: buffer_blk_cnt=%d   clust_size=%d\n", buffer_blk_cnt,
		       mydata->clust_size);

		/*
		 * On FAT32 we must fetch the FAT entries for the next
		 * root directory clusters when a cluster has been
		 * completely processed.
		 */
		++buffer_blk_cnt;
		int rootdir_end = 0;
		if (mydata->fatsize == 32) {
			if (buffer_blk_cnt == mydata->clust_size) {
				int nxtsect = 0;
				int nxt_clust = 0;

				nxt_clust = get_fatent(mydata, root_cluster);
				rootdir_end = CHECK_CLUST(nxt_clust, 32);

				nxtsect = mydata->data_begin +
					(nxt_clust * mydata->clust_size);

				root_cluster = nxt_clust;

				cursect = nxtsect;
				buffer_blk_cnt = 0;
			}
		} else {
			if (buffer_blk_cnt == PREFETCH_BLOCKS)
				buffer_blk_cnt = 0;

			rootdir_end = (++cursect - mydata->rootdir_sect >=
				       rootdir_size);
		}

		/* If end of rootdir reached */
		if (rootdir_end) {
			if (dols == LS_ROOT) {
				printf("\n%d file(s), %d dir(s)\n\n",
				       files, dirs);
				*size = 0;
			}
			goto exit;
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
				*size = 0;
			goto exit;
		}

		if (isdir && !(dentptr->attr & ATTR_DIR))
			goto exit;

		/*
		 * If we are looking for a directory, and found a directory
		 * type entry, and the entry is for the root directory (as
		 * denoted by a cluster number of 0), jump back to the start
		 * of the function, since at least on FAT12/16, the root dir
		 * lives in a hard-coded location and needs special handling
		 * to parse, rather than simply following the cluster linked
		 * list in the FAT, like other directories.
		 */
		if (isdir && (dentptr->attr & ATTR_DIR) && !START(dentptr)) {
			/*
			 * Modify the filename to remove the prefix that gets
			 * back to the root directory, so the initial root dir
			 * parsing code can continue from where we are without
			 * confusion.
			 */
			strcpy(fnamecopy, nextname ?: "");
			/*
			 * Set up state the same way as the function does when
			 * first started. This is required for the root dir
			 * parsing code operates in its expected environment.
			 */
			subname = "";
			cursect = mydata->rootdir_sect;
			isdir = 0;
			goto root_reparse;
		}

		if (idx >= 0)
			subname = nextname;
	}

	if (dogetsize) {
		*size = FAT2CPU32(dentptr->size);
		ret = 0;
	} else {
		ret = get_contents(mydata, dentptr, pos, buffer, maxsize, size);
	}
	debug("Size: %u, got: %llu\n", FAT2CPU32(dentptr->size), *size);

exit:
	free(mydata->fatbuf);
	return ret;
}

int do_fat_read(const char *filename, void *buffer, loff_t maxsize, int dols,
		loff_t *actread)
{
	return do_fat_read_at(filename, 0, buffer, maxsize, dols, 0, actread);
}

int file_fat_detectfs(void)
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
    defined(CONFIG_CMD_SATA) || \
    defined(CONFIG_SCSI) || \
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

	printf("\n  Device %d: ", cur_dev->devnum);
	dev_print(cur_dev);
#endif

	if (read_bootsectandvi(&bs, &volinfo, &fatsize)) {
		printf("\nNo valid FAT fs found\n");
		return 1;
	}

	memcpy(vol_label, volinfo.volume_label, 11);
	vol_label[11] = '\0';
	volinfo.fs_type[5] = '\0';

	printf("Filesystem: %s \"%s\"\n", volinfo.fs_type, vol_label);

	return 0;
}

int file_fat_ls(const char *dir)
{
	loff_t size;

	return do_fat_read(dir, NULL, 0, LS_YES, &size);
}

int fat_exists(const char *filename)
{
	int ret;
	loff_t size;

	ret = do_fat_read_at(filename, 0, NULL, 0, LS_NO, 1, &size);
	return ret == 0;
}

int fat_size(const char *filename, loff_t *size)
{
	return do_fat_read_at(filename, 0, NULL, 0, LS_NO, 1, size);
}

int file_fat_read_at(const char *filename, loff_t pos, void *buffer,
		     loff_t maxsize, loff_t *actread)
{
	printf("reading %s\n", filename);
	return do_fat_read_at(filename, pos, buffer, maxsize, LS_NO, 0,
			      actread);
}

int file_fat_read(const char *filename, void *buffer, int maxsize)
{
	loff_t actread;
	int ret;

	ret =  file_fat_read_at(filename, 0, buffer, maxsize, &actread);
	if (ret)
		return ret;
	else
		return actread;
}

int fat_read_file(const char *filename, void *buf, loff_t offset, loff_t len,
		  loff_t *actread)
{
	int ret;

	ret = file_fat_read_at(filename, offset, buf, len, actread);
	if (ret)
		printf("** Unable to read file %s **\n", filename);

	return ret;
}

void fat_close(void)
{
}
