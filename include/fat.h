/*
 * R/O (V)FAT 12/16/32 filesystem implementation by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to u-boot
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
 *
 */

#ifndef _FAT_H_
#define _FAT_H_

#include <asm/byteorder.h>

#define CONFIG_SUPPORT_VFAT
/* Maximum Long File Name length supported here is 128 UTF-16 code units */
#define VFAT_MAXLEN_BYTES	256 /* Maximum LFN buffer in bytes */
#define VFAT_MAXSEQ		9   /* Up to 9 of 13 2-byte UTF-16 entries */
#define PREFETCH_BLOCKS		2

#define MAX_CLUSTSIZE	65536
#define DIRENTSPERBLOCK	(mydata->sect_size / sizeof(dir_entry))
#define DIRENTSPERCLUST	((mydata->clust_size * mydata->sect_size) / \
			 sizeof(dir_entry))

#define FATBUFBLOCKS	6
#define FATBUFSIZE	(mydata->sect_size * FATBUFBLOCKS)
#define FAT12BUFSIZE	((FATBUFSIZE*2)/3)
#define FAT16BUFSIZE	(FATBUFSIZE/2)
#define FAT32BUFSIZE	(FATBUFSIZE/4)


/* Filesystem identifiers */
#define FAT12_SIGN	"FAT12   "
#define FAT16_SIGN	"FAT16   "
#define FAT32_SIGN	"FAT32   "
#define SIGNLEN		8

/* File attributes */
#define ATTR_RO	1
#define ATTR_HIDDEN	2
#define ATTR_SYS	4
#define ATTR_VOLUME	8
#define ATTR_DIR	16
#define ATTR_ARCH	32

#define ATTR_VFAT	(ATTR_RO | ATTR_HIDDEN | ATTR_SYS | ATTR_VOLUME)

#define DELETED_FLAG	((char)0xe5) /* Marks deleted files when in name[0] */
#define aRING		0x05	     /* Used as special character in name[0] */

/*
 * Indicates that the entry is the last long entry in a set of long
 * dir entries
 */
#define LAST_LONG_ENTRY_MASK	0x40

/* Flags telling whether we should read a file or list a directory */
#define LS_NO		0
#define LS_YES		1
#define LS_DIR		1
#define LS_ROOT		2

#define ISDIRDELIM(c)	((c) == '/' || (c) == '\\')

#define FSTYPE_NONE	(-1)

#if defined(__linux__) && defined(__KERNEL__)
#define FAT2CPU16	le16_to_cpu
#define FAT2CPU32	le32_to_cpu
#else
#if __LITTLE_ENDIAN
#define FAT2CPU16(x)	(x)
#define FAT2CPU32(x)	(x)
#else
#define FAT2CPU16(x)	((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
#define FAT2CPU32(x)	((((x) & 0x000000ff) << 24)  |	\
			 (((x) & 0x0000ff00) << 8)  |	\
			 (((x) & 0x00ff0000) >> 8)  |	\
			 (((x) & 0xff000000) >> 24))
#endif
#endif

#define TOLOWER(c)	if((c) >= 'A' && (c) <= 'Z'){(c)+=('a' - 'A');}
#define TOUPPER(c)	if ((c) >= 'a' && (c) <= 'z') \
				(c) -= ('a' - 'A');
#define START(dent)	(FAT2CPU16((dent)->start) \
			+ (mydata->fatsize != 32 ? 0 : \
			  (FAT2CPU16((dent)->starthi) << 16)))
#define CHECK_CLUST(x, fatsize) ((x) <= 1 || \
				(x) >= ((fatsize) != 32 ? 0xfff0 : 0xffffff0))

typedef struct boot_sector {
	__u8	ignored[3];	/* Bootstrap code */
	char	system_id[8];	/* Name of fs */
	__u8	sector_size[2];	/* Bytes/sector */
	__u8	cluster_size;	/* Sectors/cluster */
	__u16	reserved;	/* Number of reserved sectors */
	__u8	fats;		/* Number of FATs */
	__u8	dir_entries[2];	/* Number of root directory entries */
	__u8	sectors[2];	/* Number of sectors */
	__u8	media;		/* Media code */
	__u16	fat_length;	/* Sectors/FAT */
	__u16	secs_track;	/* Sectors/track */
	__u16	heads;		/* Number of heads */
	__u32	hidden;		/* Number of hidden sectors */
	__u32	total_sect;	/* Number of sectors (if sectors == 0) */

	/* FAT32 only */
	__u32	fat32_length;	/* Sectors/FAT */
	__u16	flags;		/* Bit 8: fat mirroring, low 4: active fat */
	__u8	version[2];	/* Filesystem version */
	__u32	root_cluster;	/* First cluster in root directory */
	__u16	info_sector;	/* Filesystem info sector */
	__u16	backup_boot;	/* Backup boot sector */
	__u16	reserved2[6];	/* Unused */
} boot_sector;

typedef struct volume_info
{
	__u8 drive_number;	/* BIOS drive number */
	__u8 reserved;		/* Unused */
	__u8 ext_boot_sign;	/* 0x29 if fields below exist (DOS 3.3+) */
	__u8 volume_id[4];	/* Volume ID number */
	char volume_label[11];	/* Volume label */
	char fs_type[8];	/* Typically FAT12, FAT16, or FAT32 */
	/* Boot code comes next, all but 2 bytes to fill up sector */
	/* Boot sign comes last, 2 bytes */
} volume_info;

typedef struct dir_entry {
	char	name[8],ext[3];	/* Name and extension */
	__u8	attr;		/* Attribute bits */
	__u8	lcase;		/* Case for base and extension */
	__u8	ctime_ms;	/* Creation time, milliseconds */
	__u16	ctime;		/* Creation time */
	__u16	cdate;		/* Creation date */
	__u16	adate;		/* Last access date */
	__u16	starthi;	/* High 16 bits of cluster in FAT32 */
	__u16	time,date,start;/* Time, date and first cluster */
	__u32	size;		/* File size in bytes */
} dir_entry;

typedef struct dir_slot {
	__u8	id;		/* Sequence number for slot */
	__u8	name0_4[10];	/* First 5 characters in name */
	__u8	attr;		/* Attribute byte */
	__u8	reserved;	/* Unused */
	__u8	alias_checksum;/* Checksum for 8.3 alias */
	__u8	name5_10[12];	/* 6 more characters in name */
	__u16	start;		/* Unused */
	__u8	name11_12[4];	/* Last 2 characters in name */
} dir_slot;

/*
 * Private filesystem parameters
 *
 * Note: FAT buffer has to be 32 bit aligned
 * (see FAT32 accesses)
 */
typedef struct {
	__u8	*fatbuf;	/* Current FAT buffer */
	int	fatsize;	/* Size of FAT in bits */
	__u32	fatlength;	/* Length of FAT in sectors */
	__u16	fat_sect;	/* Starting sector of the FAT */
	__u32	rootdir_sect;	/* Start sector of root directory */
	__u16	sect_size;	/* Size of sectors in bytes */
	__u16	clust_size;	/* Size of clusters in sectors */
	int	data_begin;	/* The sector of the first cluster, can be negative */
	int	fatbufnum;	/* Used by get_fatent, init to -1 */
} fsdata;

typedef int	(file_detectfs_func)(void);
typedef int	(file_ls_func)(const char *dir);
typedef long	(file_read_func)(const char *filename, void *buffer,
				 unsigned long maxsize);

struct filesystem {
	file_detectfs_func	*detect;
	file_ls_func		*ls;
	file_read_func		*read;
	const char		name[12];
};

/* FAT tables */
file_detectfs_func	file_fat_detectfs;
file_ls_func		file_fat_ls;
file_read_func		file_fat_read;

/* Currently this doesn't check if the dir exists or is valid... */
int file_cd(const char *path);
int file_fat_detectfs(void);
int file_fat_ls(const char *dir);
long file_fat_read_at(const char *filename, unsigned long pos, void *buffer,
		      unsigned long maxsize);
long file_fat_read(const char *filename, void *buffer, unsigned long maxsize);
const char *file_getfsname(int idx);
int fat_set_blk_dev(block_dev_desc_t *rbdd, disk_partition_t *info);
int fat_register_device(block_dev_desc_t *dev_desc, int part_no);

int file_fat_write(const char *filename, void *buffer, unsigned long maxsize);
#endif /* _FAT_H_ */
