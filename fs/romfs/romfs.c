/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <command.h>

#if defined(CONFIG_CMD_JFFS2)

#include <asm/byteorder.h>
#include <linux/stat.h>
#include <jffs2/jffs2.h>
#include <jffs2/load_kernel.h>

#undef DEBUG_ROMFS

/* ROMFS superblock */
struct romfs_super {
	u32 word0;
	u32 word1;
	u32 size;
	u32 checksum;
	char name[0];
};

struct romfs_inode {
	u32 next;
	u32 spec;
	u32 size;
	u32 checksum;
	char name[0];
};

extern flash_info_t flash_info[];
#define PART_OFFSET(x)	(x->offset + flash_info[x->dev->id->num].start[0])
#define ALIGN(x)	(((x) & 0xfffffff0))
#define HEADERSIZE(name)	(0x20 + ALIGN(strlen(name)))

static unsigned long romfs_resolve (unsigned long begin, unsigned long offset,
				unsigned long size, int raw, char *filename)
{
	unsigned long inodeoffset = 0, nextoffset;
	struct romfs_inode *inode;
#ifdef DEBUG_ROMFS
	printf ("ROMFS_resolve: begin 0x%x, offset 0x%x, size 0x%x, raw 0x%x, \
		filename %s\n", begin, offset, size, raw, filename);
#endif

	while (inodeoffset < size) {
		inode = (struct romfs_inode *)(begin + offset + inodeoffset);
		offset = 0;
		nextoffset = ALIGN (inode->next);
#ifdef DEBUG_ROMFS
		printf("inode 0x%x, name %s - len 0x%x, next inode 0x%x, \
			compare names 0x%x\n",
			inode, inode->name, strlen (inode->name), nextoffset,
			strncmp (filename, inode->name, strlen (filename)));
#endif
		if (!strncmp (filename, inode->name, strlen (inode->name))) {
			char *p = strtok (NULL, "/");
			if (raw && (p == NULL || *p == '\0')) {
				return offset + inodeoffset;
			}
			return romfs_resolve (begin,
					inodeoffset + HEADERSIZE (inode->name),
					size, raw, p);
		}
		inodeoffset = nextoffset;
	}

	printf ("can't find corresponding entry\n");
	return 0;
}

int romfs_load (char *loadoffset, struct part_info *info, char *filename)
{
	struct romfs_inode *inode;
	struct romfs_super *sb;
	char *data;
	int pocet;
	sb = (struct romfs_super *) PART_OFFSET (info);

	unsigned long offset;

	offset = romfs_resolve (PART_OFFSET (info), HEADERSIZE (sb->name),
				sb->size, 1, strtok (filename, "/"));
	if (offset <= 0)
		return offset;

	inode = (struct romfs_inode *)(PART_OFFSET (info) + offset);
	data = (char *)((int)inode + HEADERSIZE (inode->name));
	pocet = inode->size;
	while (pocet--) {
		*loadoffset++ = *data++;
	}
	return inode->size;
}

static int romfs_list_inode (struct part_info *info, unsigned long offset)
{
	struct romfs_inode *inode =
			(struct romfs_inode *)(PART_OFFSET (info) + offset);
	struct romfs_inode *hardlink = NULL;
	char str[3], *data;

/*
 *	mapping		spec.info means
 * 0	hard link	link destination [file header]
 * 1	directory	first file's header
 * 2	regular file	unused, must be zero [MBZ]
 * 3	symbolic link	unused, MBZ (file data is the link content)
 * 4	block device	16/16 bits major/minor number
 * 5	char device		- " -
 * 6	socket		unused, MBZ
 * 7	fifo		unused, MBZ
 */
	char attributes[] = "hdflbcsp";
	str[0] = attributes[inode->next & 0x7];
	str[1] = (inode->next & 0x8) ? 'x' : '-';
	str[2] = '\0';

	if ((str[0] == 'b') || (str[0] == 'c')) {
#ifdef DEBUG_ROMFS
		printf (" %s  %3d,%3d %12s 0x%08x 0x%08x", str,
			(inode->spec & 0xffff0000) >> 16,
			inode->spec & 0x0000ffff, inode->name, inode,
			inode->spec);
#else
		printf (" %s  %3d,%3d %12s", str,
			(inode->spec & 0xffff0000) >> 16,
			inode->spec & 0x0000ffff);
#endif
	} else {
#ifdef DEBUG_ROMFS
		printf (" %s  %7d %12s 0x%08x 0x%08x", str, inode->size,
			inode->name, inode, inode->spec);
#else
		printf (" %s  %7d %12s", str, inode->size, inode->name);
#endif
		if (str[0] == 'l') {
			data = (char *)((int)inode + HEADERSIZE (inode->name));
			puts (" -> ");
			puts (data);
		}
		if (str[0] == 'h') {
			hardlink = (struct romfs_inode *)(PART_OFFSET (info) +
						inode->spec);
			puts (" -> ");
			puts (hardlink->name);
		}
	}
	puts ("\n");
	return ALIGN (inode->next);
}

int romfs_ls (struct part_info *info, char *filename)
{
	struct romfs_inode *inode;
	unsigned long inodeoffset = 0, nextoffset;
	unsigned long offset, size;
	struct romfs_super *sb;
	sb = (struct romfs_super *)PART_OFFSET (info);

	if (strlen (filename) == 0 || !strcmp (filename, "/")) {
		offset = HEADERSIZE (sb->name);
		size = sb->size;
	} else {
		offset = romfs_resolve (PART_OFFSET (info),
			HEADERSIZE (sb->name), sb->size, 1,
			strtok (filename, "/"));

		if (offset == 0) {
			return offset;
		}
		inode = (struct romfs_inode *)(PART_OFFSET (info) + offset);
		if ((inode->next & 0x7) != 1) {
			return (romfs_list_inode (info, offset) > 0);
		}

		size = sb->size;
		offset = offset + HEADERSIZE (inode->name);
	}

	inodeoffset = offset + inodeoffset;
	while (inodeoffset < size) {
		nextoffset = romfs_list_inode (info, inodeoffset);
		if (nextoffset == 0)
			break;
		inodeoffset = nextoffset;
	}
	return 1;
}

int romfs_info (struct part_info *info)
{
	struct romfs_super *sb;
	sb = (struct romfs_super *)PART_OFFSET (info);

	printf ("name: \t\t%s, len %d B\n", sb->name, strlen (sb->name));
	printf ("size of SB:\t%d B\n", HEADERSIZE (sb->name));
	printf ("full size:\t%d B\n", sb->size);
	printf ("checksum:\t0x%x\n", sb->checksum);
	return 0;
}

int romfs_check (struct part_info *info)
{
	struct romfs_super *sb;
	if (info->dev->id->type != MTD_DEV_TYPE_NOR)
		return 0;

	sb = (struct romfs_super *)PART_OFFSET (info);
	if ((sb->word0 != 0x2D726F6D) || (sb->word1 != 0x3166732D)) {
		return 0;
	}
	return 1;
}

#endif
