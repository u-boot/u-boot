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
#include <asm/byteorder.h>

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

#undef DEBUG_ROMFS

#define ROMFS_ALIGN(x)	(((x) & 0xfffffff0))
#define HEADERSIZE(name)	(0x20 + ROMFS_ALIGN(strlen(name)))

/* find romfs inode */
static unsigned long romfs_resolve (unsigned long begin, unsigned long offset,
				unsigned long size, int raw, char *filename)
{
	unsigned long inodeoffset = 0, nextoffset;
	struct romfs_inode *inode;
#ifdef DEBUG_ROMFS
	printf ("ROMFS_resolve: begin 0x%lx, offset 0x%lx, size 0x%lx, "
		"raw 0x%x, filename %s\n\n",
					begin, offset, size, raw, filename);
#endif

	while (inodeoffset < size) {
		inode = (struct romfs_inode *)(begin + offset + inodeoffset);
		nextoffset = ROMFS_ALIGN(inode->next);
#ifdef DEBUG_ROMFS
		printf("inode 0x%x, name %s - len 0x%x, next inode 0x%lx,"
			"compare names 0x%x\n", (u32)inode, inode->name,
			strlen (inode->name), nextoffset,
			strncmp (filename, inode->name, strlen (filename)));
#endif
		if (!strncmp (filename, inode->name, strlen (inode->name))) {
			char *p = strtok (NULL, "/");

			if (raw && (p == NULL || *p == '\0'))
				return offset + inodeoffset;

			return romfs_resolve (begin,
					inodeoffset + HEADERSIZE (inode->name),
					size, raw, p);
		}
		inodeoffset = nextoffset;
		offset = 0; /* clear offset at the end because first label */
	}

	printf ("Can't find corresponding entry\n");
	return 0;
}

int romfs_load (char *loadoffset, int info, char *filename)
{
	struct romfs_inode *inode;
	struct romfs_super *sb;
	char *data;
	int pocet;
	unsigned long offset;

	sb = (struct romfs_super *) info;

	offset = romfs_resolve (info, HEADERSIZE (sb->name),
				sb->size, 1, strtok (filename, "/"));
	if (offset <= 0)
		return offset;

	inode = (struct romfs_inode *)(info + offset);
	data = (char *)((int)inode + HEADERSIZE (inode->name));
	pocet = inode->size;
	while (pocet--)
		*loadoffset++ = *data++;

	return inode->size;
}

static int romfs_list_inode (int info, unsigned long offset)
{
	struct romfs_inode *inode =
			(struct romfs_inode *)(info + offset);
	struct romfs_inode *hardlink = NULL;
	char str[3], *data;

/*	mapping		spec.info means
 0	hard link	link destination [file header]
 1	directory	first file's header
 2	regular file	unused, must be zero [MBZ]
 3	symbolic link	unused, MBZ (file data is the link content)
 4	block device	16/16 bits major/minor number
 5	char device		- " -
 6	socket		unused, MBZ
 7	fifo		unused, MBZ */

	switch (inode->next & 0x7) {
	case 0:
		str[0] = 'h';
		break;
	case 1:
		str[0] = 'd';
		break;
	case 2:
		str[0] = 'f';
		break;
	case 3:
		str[0] = 'l';
		break;
	case 4:
		str[0] = 'b';
		break;
	case 5:
		str[0] = 'c';
		break;
	case 6:
		str[0] = 's';
		break;
	case 7:
		str[0] = 'p';
		break;
	default:
		str[0] = '?';
	}

	if (inode->next & 0x8) {
		str[1] = 'x';
	} else {
		str[1] = '-';
	}
	str[2] = '\0';

	if ((str[0] == 'b') || (str[0] == 'c')) {
#ifdef DEBUG_ROMFS
		printf (" %s  %3d,%3d %12s 0x%08x 0x%08x", str,
			(inode->spec & 0xffff0000) >> 16,
			inode->spec & 0x0000ffff, inode->name, (u32)inode,
			inode->spec);
#else
		printf (" %s  %3d,%3d %12s", str,
			(inode->spec & 0xffff0000) >> 16,
			inode->spec & 0x0000ffff, inode->name);

#endif
	} else {
#ifdef DEBUG_ROMFS
		printf (" %s  %7d %12s 0x%08x 0x%08x", str, inode->size,
			inode->name, (u32)inode, inode->spec);
#else
		printf (" %s  %7d %12s", str, inode->size, inode->name);
#endif
		if (str[0] == 'l') {
			data = (char *)((int)inode + HEADERSIZE (inode->name));
			puts (" -> ");
			puts (data);
		}
		if (str[0] == 'h') {
			hardlink = (struct romfs_inode *)(info + inode->spec);
			puts (" -> ");
			puts (hardlink->name);
		}
	}
	puts ("\n");
	return ROMFS_ALIGN(inode->next);
}

/* listing directory */
int romfs_ls (int info, char *filename)
{
	struct romfs_inode *inode;
	struct romfs_super *sb;
	unsigned long inodeoffset = 0, nextoffset;
	unsigned long size;

	sb = (struct romfs_super *) info;
	inode = (struct romfs_inode *) info;

	if ((strlen (filename) != 0) && strcmp (filename, "/")) {
		inodeoffset = romfs_resolve (info,
			HEADERSIZE (sb->name), sb->size, 1,
			strtok (filename, "/"));

		/* inode not found */
		if (inodeoffset == 0)
			return 0;

		/* look at what is it */
		inode = (struct romfs_inode *)(info + inodeoffset);
		if ((inode->next & 0x7) != 1)
			return (romfs_list_inode (info, inodeoffset) > 0);
	}

	/* print directory */
	size = sb->size;
	inodeoffset = inodeoffset + HEADERSIZE (inode->name);

	while (inodeoffset < size) {
		nextoffset = romfs_list_inode (info, inodeoffset);
		if (nextoffset == 0)
			break;
		inodeoffset = nextoffset;
	}
	return 1;
}

/* cat file */
int romfs_cat (int info, char *filename)
{
	struct romfs_inode *inode;
	struct romfs_super *sb;
	unsigned long inodeoffset = 0;
	char *data;

	sb = (struct romfs_super *) info;
	inode = (struct romfs_inode *) info;

	if ((strlen (filename) != 0) && strcmp (filename, "/")) {
		inodeoffset = romfs_resolve (info,
			HEADERSIZE (sb->name), sb->size, 1,
			strtok (filename, "/"));

		/* inode not found */
		if (inodeoffset == 0)
			return 0;

		/* look at what it is */
		inode = (struct romfs_inode *)(info + inodeoffset);
		if ((inode->next & 0x7) == 2) {
			data =(char *) (info + inodeoffset +
					(u32)HEADERSIZE (inode->name));
			printf("%s\n", data);
			return 1;
		}
	}
	return 0;
}

int romfs_info (int info)
{
	struct romfs_super *sb;
	sb = (struct romfs_super *)info;

	printf ("name: \t\t%s, len %d B\n", sb->name, strlen (sb->name));
	printf ("size of SB:\t%d B\n", HEADERSIZE (sb->name));
	printf ("full size:\t%d B\n", sb->size);
	printf ("checksum:\t0x%x\n", sb->checksum);
	printf ("fs address:\t0x%x\n", (u32)sb);
	return 1;
}

int romfs_check (int info)
{
	struct romfs_super *sb;

	sb = (struct romfs_super *) info;

	if ((sb->word0 != 0x2D726F6D) || (sb->word1 != 0x3166732D))
		return 0;

	return 1;
}
