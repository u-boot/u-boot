/*
-------------------------------------------------------------------------
 * Filename:      jffs2.c
 * Version:       $Id: jffs2_1pass.c,v 1.7 2002/01/25 01:56:47 nyet Exp $
 * Copyright:     Copyright (C) 2001, Russ Dill
 * Author:        Russ Dill <Russ.Dill@asu.edu>
 * Description:   Module to load kernel from jffs2
 *-----------------------------------------------------------------------*/
/*
 * some portions of this code are taken from jffs2, and as such, the
 * following copyright notice is included.
 *
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@cambridge.redhat.com>
 *
 * The original JFFS, from which the design for JFFS2 was derived,
 * was designed and implemented by Axis Communications AB.
 *
 * The contents of this file are subject to the Red Hat eCos Public
 * License Version 1.1 (the "Licence"); you may not use this file
 * except in compliance with the Licence.  You may obtain a copy of
 * the Licence at http://www.redhat.com/
 *
 * Software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing rights and
 * limitations under the Licence.
 *
 * The Original Code is JFFS2 - Journalling Flash File System, version 2
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License version 2 (the "GPL"), in
 * which case the provisions of the GPL are applicable instead of the
 * above.  If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the RHEPL, indicate your decision by
 * deleting the provisions above and replace them with the notice and
 * other provisions required by the GPL.  If you do not delete the
 * provisions above, a recipient may use your version of this file
 * under either the RHEPL or the GPL.
 *
 * $Id: jffs2_1pass.c,v 1.7 2002/01/25 01:56:47 nyet Exp $
 *
 */

/* Ok, so anyone who knows the jffs2 code will probably want to get a papar
 * bag to throw up into before reading this code. I looked through the jffs2
 * code, the caching scheme is very elegant. I tried to keep the version
 * for a bootloader as small and simple as possible. Instead of worring about
 * unneccesary data copies, node scans, etc, I just optimized for the known
 * common case, a kernel, which looks like:
 * 	(1) most pages are 4096 bytes
 *	(2) version numbers are somewhat sorted in acsending order
 *	(3) multiple compressed blocks making up one page is uncommon
 *
 * So I create a linked list of decending version numbers (insertions at the
 * head), and then for each page, walk down the list, until a matching page
 * with 4096 bytes is found, and then decompress the watching pages in
 * reverse order.
 *
 */

/*
 * Adapted by Nye Liu <nyet@zumanetworks.com> and
 * Rex Feany <rfeany@zumanetworks.com>
 * on Jan/2002 for U-Boot.
 *
 * Clipped out all the non-1pass functions, cleaned up warnings,
 * wrappers, etc. No major changes to the code.
 * Please, he really means it when he said have a paper bag
 * handy. We needed it ;).
 *
 */

#include <common.h>
#include <config.h>
#include <malloc.h>
#include <linux/stat.h>
#include <linux/time.h>

#if (CONFIG_COMMANDS & CFG_CMD_JFFS2)

#include <jffs2/jffs2.h>
#include <jffs2/jffs2_1pass.h>

#include "jffs2_private.h"

/* Compression names */
static char *compr_names[] = {
        "NONE",
        "ZERO",
        "RTIME",
        "RUBINMIPS",
        "COPY",
        "DYNRUBIN",
        "ZLIB" };

static char spinner[] = { '|', '\\', '-', '/' };

#define DEBUG
#ifdef  DEBUG
# define DEBUGF(fmt,args...)	printf(fmt ,##args)
#else
# define DEBUGF(fmt,args...)
#endif

#define MALLOC_CHUNK (10*1024)


static struct b_node *
add_node(struct b_node *tail, u32 * count, u32 * memBase)
{
	u32 index;
	u32 memLimit;
	struct b_node *b;

	index = (*count) * sizeof(struct b_node) % MALLOC_CHUNK;
	memLimit = MALLOC_CHUNK;

#if 0
	putLabeledWord("add_node: index = ", index);
	putLabeledWord("add_node: memLimit = ", memLimit);
	putLabeledWord("add_node: memBase = ", *memBase);
#endif

	/* we need not keep a list of bases since we'll never free the */
	/* memory, just jump the the kernel */
	if ((index == 0) || (index > memLimit)) {	/* we need mode space before we continue */
		if ((*memBase = (u32) mmalloc(MALLOC_CHUNK)) == (u32) NULL) {
			putstr("add_node: malloc failed\n");
			return NULL;
		}
#if 0
		putLabeledWord("add_node: alloced a new membase at ", *memBase);
#endif

	}
	/* now we have room to add it. */
	b = (struct b_node *) (*memBase + index);

	/* null on first call */
	if (tail)
		tail->next = b;

#if 0
	putLabeledWord("add_node: tail = ", (u32) tail);
	if (tail)
		putLabeledWord("add_node: tail->next = ", (u32) tail->next);

#endif

#if 0
	putLabeledWord("add_node: mb+i = ", (u32) (*memBase + index));
	putLabeledWord("add_node: b = ", (u32) b);
#endif
	(*count)++;
	b->next = (struct b_node *) NULL;
	return b;

}

/* we know we have empties at the start offset so we will hop */
/* t points that would be non F if there were a node here to speed this up. */
struct jffs2_empty_node {
	u32 first;
	u32 second;
};

static u32
jffs2_scan_empty(u32 start_offset, struct part_info *part)
{
	u32 max = part->size - sizeof(struct jffs2_raw_inode);

	/* this would be either dir node_crc or frag isize */
	u32 offset = start_offset + 32;
	struct jffs2_empty_node *node;

	start_offset += 4;
	while (offset < max) {
		node = (struct jffs2_empty_node *) (part->offset + offset);
		if ((node->first == 0xFFFFFFFF) && (node->second == 0xFFFFFFFF)) {
			/* we presume that there were no nodes in between and advance in a hop */
			/* putLabeledWord("\t\tjffs2_scan_empty: empty at offset=",offset); */
			start_offset = offset + 4;
			offset = start_offset + 32;	/* orig 32 + 4 bytes for the second==0xfffff */
		} else {
			return start_offset;
		}
	}
	return start_offset;
}

static u32
jffs_init_1pass_list(struct part_info *part)
{
	if ( 0 != ( part->jffs2_priv=malloc(sizeof(struct b_lists)))){
		struct b_lists *pL =(struct b_lists *)part->jffs2_priv;

		pL->dirListHead = pL->dirListTail = NULL;
		pL->fragListHead = pL->fragListTail = NULL;
		pL->dirListCount = 0;
		pL->dirListMemBase = 0;
		pL->fragListCount = 0;
		pL->fragListMemBase = 0;
		pL->partOffset = 0x0;
	}
	return 0;
}

/* find the inode from the slashless name given a parent */
static long
jffs2_1pass_read_inode(struct b_lists *pL, u32 inode, char *dest)
{
	struct b_node *b;
	struct jffs2_raw_inode *jNode;
	u32 totalSize = 1;
	u32 oldTotalSize = 0;
	u32 size = 0;
	char *lDest = (char *) dest;
	char *src;
	long ret;
	int i;
	u32 counter = 0;
	char totalSizeSet = 0;

#if 0
	b = pL->fragListHead;
	while (b) {
		jNode = (struct jffs2_raw_inode *) (b->offset);
		if ((inode == jNode->ino)) {
			putLabeledWord("\r\n\r\nread_inode: totlen = ", jNode->totlen);
			putLabeledWord("read_inode: inode = ", jNode->ino);
			putLabeledWord("read_inode: version = ", jNode->version);
			putLabeledWord("read_inode: isize = ", jNode->isize);
			putLabeledWord("read_inode: offset = ", jNode->offset);
			putLabeledWord("read_inode: csize = ", jNode->csize);
			putLabeledWord("read_inode: dsize = ", jNode->dsize);
			putLabeledWord("read_inode: compr = ", jNode->compr);
			putLabeledWord("read_inode: usercompr = ", jNode->usercompr);
			putLabeledWord("read_inode: flags = ", jNode->flags);
		}

		b = b->next;
	}

#endif

#if 1
	b = pL->fragListHead;
	while (b && (size < totalSize)) {
		jNode = (struct jffs2_raw_inode *) (b->offset);
		if ((inode == jNode->ino)) {
			if ((jNode->isize == oldTotalSize) && (jNode->isize > totalSize)) {
				/* 2 consecutive isizes indicate file length */
				totalSize = jNode->isize;
				totalSizeSet = 1;
			} else if (!totalSizeSet) {
				totalSize = size + jNode->dsize + 1;
			}
			oldTotalSize = jNode->isize;

			if(dest) {
				src = ((char *) jNode) + sizeof(struct jffs2_raw_inode);
				/* lDest = (char *) (dest + (jNode->offset & ~3)); */
				lDest = (char *) (dest + jNode->offset);
#if 0
				putLabeledWord("\r\n\r\nread_inode: src = ", src);
				putLabeledWord("read_inode: dest = ", lDest);
				putLabeledWord("read_inode: dsize = ", jNode->dsize);
				putLabeledWord("read_inode: csize = ", jNode->csize);
				putLabeledWord("read_inode: version = ", jNode->version);
				putLabeledWord("read_inode: isize = ", jNode->isize);
				putLabeledWord("read_inode: offset = ", jNode->offset);
				putLabeledWord("read_inode: compr = ", jNode->compr);
				putLabeledWord("read_inode: flags = ", jNode->flags);
#endif
				switch (jNode->compr) {
				case JFFS2_COMPR_NONE:
#if 0
					{
						int i;

						if ((dest > 0xc0092ff0) && (dest < 0xc0093000))
							for (i = 0; i < first->length; i++) {
								putLabeledWord("\tCOMPR_NONE: src =", src + i);
								putLabeledWord("\tCOMPR_NONE: length =", first->length);
								putLabeledWord("\tCOMPR_NONE: dest =", dest + i);
								putLabeledWord("\tCOMPR_NONE: data =", (unsigned char) *(src + i));
							}
					}
#endif

					ret = (unsigned long) ldr_memcpy(lDest, src, jNode->dsize);
					break;
				case JFFS2_COMPR_ZERO:
					ret = 0;
					for (i = 0; i < jNode->dsize; i++)
						*(lDest++) = 0;
					break;
				case JFFS2_COMPR_RTIME:
					ret = 0;
					rtime_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
				case JFFS2_COMPR_DYNRUBIN:
					/* this is slow but it works */
					ret = 0;
					dynrubin_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
				case JFFS2_COMPR_ZLIB:
					ret = zlib_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
				default:
					/* unknown */
					putLabeledWord("UNKOWN COMPRESSION METHOD = ", jNode->compr);
					return -1;
					break;
				}
			}

			size += jNode->dsize;
#if 0
			putLabeledWord("read_inode: size = ", size);
			putLabeledWord("read_inode: totalSize = ", totalSize);
			putLabeledWord("read_inode: compr ret = ", ret);
#endif
		}
		b = b->next;
		counter++;
	}
#endif

#if 0
	putLabeledWord("read_inode: returning = ", size);
#endif
	return size;
}

/* find the inode from the slashless name given a parent */
static u32
jffs2_1pass_find_inode(struct b_lists * pL, const char *name, u32 pino)
{
	struct b_node *b;
	struct jffs2_raw_dirent *jDir;
	int len;
	u32 counter;
	u32 version = 0;
	u32 inode = 0;

	/* name is assumed slash free */
	len = strlen(name);

	counter = 0;
	/* we need to search all and return the inode with the highest version */
	for(b = pL->dirListHead;b;b=b->next,counter++) {
		jDir = (struct jffs2_raw_dirent *) (b->offset);
		if ((pino == jDir->pino) && (len == jDir->nsize) && (jDir->ino) &&	/* 0 for unlink */
		    (!strncmp(jDir->name, name, len))) {	/* a match */
			if (jDir->version < version) continue;

		        if(jDir->version==0) {
			    	/* Is this legal? */
				putstr(" ** WARNING ** ");
				putnstr(jDir->name, jDir->nsize);
				putstr(" is version 0 (in find, ignoring)\r\n");
			} else if(jDir->version==version) {
			    	/* Im pretty sure this isn't ... */
				putstr(" ** ERROR ** ");
				putnstr(jDir->name, jDir->nsize);
				putLabeledWord(" has dup version =", version);
			}
			inode = jDir->ino;
			version = jDir->version;
		}
#if 0
		putstr("\r\nfind_inode:p&l ->");
		putnstr(jDir->name, jDir->nsize);
		putstr("\r\n");
		putLabeledWord("pino = ", jDir->pino);
		putLabeledWord("nsize = ", jDir->nsize);
		putLabeledWord("b = ", (u32) b);
		putLabeledWord("counter = ", counter);
#endif
	}
	return inode;
}

static char *mkmodestr(unsigned long mode, char *str)
{
    static const char *l="xwr";
    int mask=1, i;
    char c;

    switch (mode & S_IFMT) {
        case S_IFDIR:    str[0]='d'; break;
        case S_IFBLK:    str[0]='b'; break;
        case S_IFCHR:    str[0]='c'; break;
        case S_IFIFO:    str[0]='f'; break;
        case S_IFLNK:    str[0]='l'; break;
        case S_IFSOCK:   str[0]='s'; break;
        case S_IFREG:    str[0]='-'; break;
        default:        str[0]='?';
    }

    for(i=0;i<9;i++) {
        c=l[i%3];
        str[9-i]=(mode & mask)?c:'-';
        mask=mask<<1;
    }

    if(mode & S_ISUID) str[3]=(mode & S_IXUSR)?'s':'S';
    if(mode & S_ISGID) str[6]=(mode & S_IXGRP)?'s':'S';
    if(mode & S_ISVTX) str[9]=(mode & S_IXOTH)?'t':'T';
    str[10]='\0';
    return str;
}

static inline void dump_stat(struct stat *st, const char *name)
{
    char str[20];
    char s[64], *p;

    if (st->st_mtime == (time_t)(-1))	/* some ctimes really hate -1 */
        st->st_mtime = 1;

    ctime_r(&st->st_mtime, s/*, 64*/);   /* newlib ctime doesn't have buflen */

    if((p=strchr(s,'\n'))!=NULL) *p='\0';
    if((p=strchr(s,'\r'))!=NULL) *p='\0';

/*
    printf("%6lo %s %8ld %s %s\n", st->st_mode, mkmodestr(st->st_mode, str),
        st->st_size, s, name);
*/

    printf(" %s %8ld %s %s", mkmodestr(st->st_mode,str), st->st_size, s, name);
}

static inline u32 dump_inode(struct b_lists * pL, struct jffs2_raw_dirent *d, struct jffs2_raw_inode *i)
{
	char fname[256];
	struct stat st;

	if(!d || !i) return -1;

	strncpy(fname, d->name, d->nsize);
	fname[d->nsize]='\0';

	memset(&st,0,sizeof(st));

	st.st_mtime=i->mtime;
	st.st_mode=i->mode;
	st.st_ino=i->ino;

	/* neither dsize nor isize help us.. do it the long way */
	st.st_size=jffs2_1pass_read_inode(pL, i->ino, NULL);

	dump_stat(&st, fname);

	if (d->type == DT_LNK) {
		unsigned char *src = (unsigned char *) (&i[1]);
	        putstr(" -> ");
		putnstr(src, (int)i->dsize);
	}

	putstr("\r\n");

	return 0;
}

/* list inodes with the given pino */
static u32
jffs2_1pass_list_inodes(struct b_lists * pL, u32 pino)
{
	struct b_node *b;
	struct jffs2_raw_dirent *jDir;

	for(b = pL->dirListHead;b;b=b->next) {
		jDir = (struct jffs2_raw_dirent *) (b->offset);
		if ((pino == jDir->pino) && (jDir->ino)) {	/* 0 inode for unlink */
			u32 i_version=0;
			struct jffs2_raw_inode *jNode, *i=NULL;
			struct b_node *b2 = pL->fragListHead;

			while (b2) {
				jNode = (struct jffs2_raw_inode *) (b2->offset);
				if (jNode->ino == jDir->ino
				    && jNode->version>=i_version)
					i=jNode;
				b2 = b2->next;
			}

			dump_inode(pL, jDir, i);
		}
	}
	return pino;
}

static u32
jffs2_1pass_search_inode(struct b_lists * pL, const char *fname, u32 pino)
{
	int i;
	char tmp[256];
	char working_tmp[256];
	char *c;

	/* discard any leading slash */
	i = 0;
	while (fname[i] == '/')
		i++;
	strcpy(tmp, &fname[i]);

	while ((c = (char *) strchr(tmp, '/')))	/* we are still dired searching */
	{
		strncpy(working_tmp, tmp, c - tmp);
		working_tmp[c - tmp] = '\0';
#if 0
		putstr("search_inode: tmp = ");
		putstr(tmp);
		putstr("\r\n");
		putstr("search_inode: wtmp = ");
		putstr(working_tmp);
		putstr("\r\n");
		putstr("search_inode: c = ");
		putstr(c);
		putstr("\r\n");
#endif
		for (i = 0; i < strlen(c) - 1; i++)
			tmp[i] = c[i + 1];
		tmp[i] = '\0';
#if 0
		putstr("search_inode: post tmp = ");
		putstr(tmp);
		putstr("\r\n");
#endif

		if (!(pino = jffs2_1pass_find_inode(pL, working_tmp, pino))) {
			putstr("find_inode failed for name=");
			putstr(working_tmp);
			putstr("\r\n");
			return 0;
		}
	}
	/* this is for the bare filename, directories have already been mapped */
	if (!(pino = jffs2_1pass_find_inode(pL, tmp, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	return pino;

}

static u32
jffs2_1pass_resolve_inode(struct b_lists * pL, u32 ino)
{
	struct b_node *b;
	struct b_node *b2;
	struct jffs2_raw_dirent *jDir;
	struct jffs2_raw_inode *jNode;
	struct jffs2_raw_dirent *jDirFound = NULL;
	char tmp[256];
	u32 version = 0;
	u32 pino;
	unsigned char *src;

	/* we need to search all and return the inode with the highest version */
	for(b = pL->dirListHead; b; b=b->next) {
		jDir = (struct jffs2_raw_dirent *) (b->offset);
		if (ino == jDir->ino) {
		    	if(jDir->version < version) continue;

			if(jDir->version == 0) {
			    	/* Is this legal? */
				putstr(" ** WARNING ** ");
				putnstr(jDir->name, jDir->nsize);
				putstr(" is version 0 (in resolve, ignoring)\r\n");
			} else if(jDir->version == version) {
			    	/* Im pretty sure this isn't ... */
				putstr(" ** ERROR ** ");
				putnstr(jDir->name, jDir->nsize);
				putLabeledWord(" has dup version (resolve) = ",
					version);
			}

			jDirFound = jDir;
			version = jDir->version;
		}
	}
	/* now we found the right entry again. (shoulda returned inode*) */
	if (jDirFound->type != DT_LNK)
		return jDirFound->ino;
	/* so its a soft link so we follow it again. */
	b2 = pL->fragListHead;
	while (b2) {
		jNode = (struct jffs2_raw_inode *) (b2->offset);
		if (jNode->ino == jDirFound->ino) {
			src = (unsigned char *) (b2->offset + sizeof(struct jffs2_raw_inode));

#if 0
			putLabeledWord("\t\t dsize = ", jNode->dsize);
			putstr("\t\t target = ");
			putnstr(src, jNode->dsize);
			putstr("\r\n");
#endif
			strncpy(tmp, src, jNode->dsize);
			tmp[jNode->dsize] = '\0';
			break;
		}
		b2 = b2->next;
	}
	/* ok so the name of the new file to find is in tmp */
	/* if it starts with a slash it is root based else shared dirs */
	if (tmp[0] == '/')
		pino = 1;
	else
		pino = jDirFound->pino;

	return jffs2_1pass_search_inode(pL, tmp, pino);
}

static u32
jffs2_1pass_search_list_inodes(struct b_lists * pL, const char *fname, u32 pino)
{
	int i;
	char tmp[256];
	char working_tmp[256];
	char *c;

	/* discard any leading slash */
	i = 0;
	while (fname[i] == '/')
		i++;
	strcpy(tmp, &fname[i]);
	working_tmp[0] = '\0';
	while ((c = (char *) strchr(tmp, '/')))	/* we are still dired searching */
	{
		strncpy(working_tmp, tmp, c - tmp);
		working_tmp[c - tmp] = '\0';
		for (i = 0; i < strlen(c) - 1; i++)
			tmp[i] = c[i + 1];
		tmp[i] = '\0';
		/* only a failure if we arent looking at top level */
		if (!(pino = jffs2_1pass_find_inode(pL, working_tmp, pino)) && (working_tmp[0])) {
			putstr("find_inode failed for name=");
			putstr(working_tmp);
			putstr("\r\n");
			return 0;
		}
	}

	if (tmp[0] && !(pino = jffs2_1pass_find_inode(pL, tmp, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	/* this is for the bare filename, directories have already been mapped */
	if (!(pino = jffs2_1pass_list_inodes(pL, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	return pino;

}

unsigned char
jffs2_1pass_rescan_needed(struct part_info *part)
{
	struct b_node *b;
	struct jffs2_unknown_node *node;
	struct b_lists *pL=(struct b_lists *)part->jffs2_priv;

	if (part->jffs2_priv == 0){
		DEBUGF ("rescan: First time in use\n");
		return 1;
	}
	/* if we have no list, we need to rescan */
	if (pL->fragListCount == 0) {
		DEBUGF ("rescan: fraglist zero\n");
		return 1;
	}

	/* or if we are scanninga new partition */
	if (pL->partOffset != part->offset) {
		DEBUGF ("rescan: different partition\n");
		return 1;
	}
	/* but suppose someone reflashed the root partition at the same offset... */
	b = pL->dirListHead;
	while (b) {
		node = (struct jffs2_unknown_node *) (b->offset);
		if (node->nodetype != JFFS2_NODETYPE_DIRENT) {
			DEBUGF ("rescan: fs changed beneath me? (%lx)\n", (unsigned long) b->offset);
			return 1;
		}
		b = b->next;
	}
	return 0;
}

static u32
jffs2_1pass_build_lists(struct part_info * part)
{
	struct b_lists *pL;
	struct jffs2_unknown_node *node;
	u32 offset;
	u32 max = part->size - sizeof(struct jffs2_raw_inode);
	u32 counter = 0;
	u32 counter4 = 0;
	u32 counterF = 0;
	u32 counterN = 0;

	/* turn off the lcd.  Refreshing the lcd adds 50% overhead to the */
	/* jffs2 list building enterprise nope.  in newer versions the overhead is */
	/* only about 5 %.  not enough to inconvenience people for. */
	/* lcd_off(); */

	/* if we are building a list we need to refresh the cache. */
	/* note that since we don't free our memory, eventually this will be bad. */
	/* but we're a bootldr so what the hell. */
	jffs_init_1pass_list(part);
	pL=(struct b_lists *)part->jffs2_priv;
	pL->partOffset = part->offset;
	offset = 0;
	printf("Scanning JFFS2 FS:   ");

	/* start at the beginning of the partition */
	while (offset < max) {
	    	if (! (++counter%10000))
			printf("\b\b%c ", spinner[(counter / 10000) % 4]);

		node = (struct jffs2_unknown_node *) (part->offset + offset);
		if (node->magic == JFFS2_MAGIC_BITMASK && hdr_crc(node)) {
			/* if its a fragment add it */
			if (node->nodetype == JFFS2_NODETYPE_INODE && inode_crc((struct jffs2_raw_inode *) node)) {
				if (!(pL->fragListTail = add_node(pL->fragListTail, &(pL->fragListCount),
								  &(pL->fragListMemBase)))) {
					putstr("add_node failed!\r\n");
					return 0;
				}
				pL->fragListTail->offset = (u32) (part->offset + offset);
				if (!pL->fragListHead)
					pL->fragListHead = pL->fragListTail;
			} else if (node->nodetype == JFFS2_NODETYPE_DIRENT &&
				   dirent_crc((struct jffs2_raw_dirent *) node)  &&
				   dirent_name_crc((struct jffs2_raw_dirent *) node)) {
				if (! (counterN%100))
					printf("\b\b.  ");
#if 0
				printf("Found DIRENT @ 0x%lx\n", offset);
				putstr("\r\nbuild_lists:p&l ->");
				putnstr(((struct jffs2_raw_dirent *) node)->name, ((struct jffs2_raw_dirent *) node)->nsize);
				putstr("\r\n");
				putLabeledWord("\tpino = ", ((struct jffs2_raw_dirent *) node)->pino);
				putLabeledWord("\tnsize = ", ((struct jffs2_raw_dirent *) node)->nsize);
#endif

				if (!(pL->dirListTail = add_node(pL->dirListTail, &(pL->dirListCount), &(pL->dirListMemBase)))) {
					putstr("add_node failed!\r\n");
					return 0;
				}
				pL->dirListTail->offset = (u32) (part->offset + offset);
#if 0
				putLabeledWord("\ttail = ", (u32) pL->dirListTail);
				putstr("\ttailName ->");
				putnstr(((struct jffs2_raw_dirent *) (pL->dirListTail->offset))->name,
					((struct jffs2_raw_dirent *) (pL->dirListTail->offset))->nsize);
				putstr("\r\n");
#endif
				if (!pL->dirListHead)
					pL->dirListHead = pL->dirListTail;
				counterN++;
			} else if (node->nodetype == JFFS2_NODETYPE_CLEANMARKER) {
				if (node->totlen != sizeof(struct jffs2_unknown_node))
					printf("OOPS Cleanmarker has bad size %d != %d\n", node->totlen, sizeof(struct jffs2_unknown_node));
			} else {
				printf("Unknown node type: %x len %d offset 0x%x\n", node->nodetype, node->totlen, offset);
			}
			offset += ((node->totlen + 3) & ~3);
			counterF++;
		} else if (node->magic == JFFS2_EMPTY_BITMASK && node->nodetype == JFFS2_EMPTY_BITMASK) {
			offset = jffs2_scan_empty(offset, part);
		} else {	/* if we know nothing of the filesystem, we just step and look. */
			offset += 4;
			counter4++;
		}
/*             printf("unknown node magic %4.4x %4.4x @ %lx\n", node->magic, node->nodetype, (unsigned long)node); */

	}

	putstr("\b\b done.\r\n");		/* close off the dots */
	/* turn the lcd back on. */
	/* splash(); */

#if 0
	putLabeledWord("dir entries = ", pL->dirListCount);
	putLabeledWord("frag entries = ", pL->fragListCount);
	putLabeledWord("+4 increments = ", counter4);
	putLabeledWord("+file_offset increments = ", counterF);

#endif

#undef SHOW_ALL
#undef SHOW_ALL_FRAGMENTS

#ifdef SHOW_ALL
	{
		struct b_node *b;
		struct b_node *b2;
		struct jffs2_raw_dirent *jDir;
		struct jffs2_raw_inode *jNode;

		putstr("\r\n\r\n******The directory Entries******\r\n");
		b = pL->dirListHead;
		while (b) {
			jDir = (struct jffs2_raw_dirent *) (b->offset);
			putstr("\r\n");
			putnstr(jDir->name, jDir->nsize);
			putLabeledWord("\r\n\tbuild_list: magic = ", jDir->magic);
			putLabeledWord("\tbuild_list: nodetype = ", jDir->nodetype);
			putLabeledWord("\tbuild_list: hdr_crc = ", jDir->hdr_crc);
			putLabeledWord("\tbuild_list: pino = ", jDir->pino);
			putLabeledWord("\tbuild_list: version = ", jDir->version);
			putLabeledWord("\tbuild_list: ino = ", jDir->ino);
			putLabeledWord("\tbuild_list: mctime = ", jDir->mctime);
			putLabeledWord("\tbuild_list: nsize = ", jDir->nsize);
			putLabeledWord("\tbuild_list: type = ", jDir->type);
			putLabeledWord("\tbuild_list: node_crc = ", jDir->node_crc);
			putLabeledWord("\tbuild_list: name_crc = ", jDir->name_crc);
			b = b->next;
		}

#ifdef SHOW_ALL_FRAGMENTS
		putstr("\r\n\r\n******The fragment Entries******\r\n");
		b = pL->fragListHead;
		while (b) {
			jNode = (struct jffs2_raw_inode *) (b->offset);
			putLabeledWord("\r\n\tbuild_list: FLASH_OFFSET = ", b->offset);
			putLabeledWord("\tbuild_list: totlen = ", jNode->totlen);
			putLabeledWord("\tbuild_list: inode = ", jNode->ino);
			putLabeledWord("\tbuild_list: version = ", jNode->version);
			putLabeledWord("\tbuild_list: isize = ", jNode->isize);
			putLabeledWord("\tbuild_list: atime = ", jNode->atime);
			putLabeledWord("\tbuild_list: offset = ", jNode->offset);
			putLabeledWord("\tbuild_list: csize = ", jNode->csize);
			putLabeledWord("\tbuild_list: dsize = ", jNode->dsize);
			putLabeledWord("\tbuild_list: compr = ", jNode->compr);
			putLabeledWord("\tbuild_list: usercompr = ", jNode->usercompr);
			putLabeledWord("\tbuild_list: flags = ", jNode->flags);
			b = b->next;
		}
#endif /* SHOW_ALL_FRAGMENTS */
	}

#endif	/* SHOW_ALL */
	/* give visual feedback that we are done scanning the flash */
	led_blink(0x0, 0x0, 0x1, 0x1);	/* off, forever, on 100ms, off 100ms */
	return 1;
}





static u32
jffs2_1pass_fill_info(struct b_lists * pL, struct b_jffs2_info * piL)
{
	struct b_node *b;
	struct jffs2_raw_inode *jNode;
	int i;

	b = pL->fragListHead;
	for (i = 0; i < JFFS2_NUM_COMPR; i++) {
		piL->compr_info[i].num_frags = 0;
		piL->compr_info[i].compr_sum = 0;
		piL->compr_info[i].decompr_sum = 0;
	}

	while (b) {
		jNode = (struct jffs2_raw_inode *) (b->offset);
		if (jNode->compr < JFFS2_NUM_COMPR) {
			piL->compr_info[jNode->compr].num_frags++;
			piL->compr_info[jNode->compr].compr_sum += jNode->csize;
			piL->compr_info[jNode->compr].decompr_sum += jNode->dsize;
		}
		b = b->next;
	}
	return 0;
}



static struct b_lists *
jffs2_get_list(struct part_info * part, const char *who)
{
	if (jffs2_1pass_rescan_needed(part)) {
		if (!jffs2_1pass_build_lists(part)) {
			printf("%s: Failed to scan JFFSv2 file structure\n", who);
			return NULL;
		}
	}
	return (struct b_lists *)part->jffs2_priv;
}


/* Print directory / file contents */
u32
jffs2_1pass_ls(struct part_info * part, const char *fname)
{
	struct b_lists *pl;
	long ret = 0;
	u32 inode;

	if (! (pl  = jffs2_get_list(part, "ls")))
		return 0;

	if (! (inode = jffs2_1pass_search_list_inodes(pl, fname, 1))) {
		putstr("ls: Failed to scan jffs2 file structure\r\n");
		return 0;
	}


#if 0
	putLabeledWord("found file at inode = ", inode);
	putLabeledWord("read_inode returns = ", ret);
#endif

	return ret;
}





/* Load a file from flash into memory. fname can be a full path */
u32
jffs2_1pass_load(char *dest, struct part_info * part, const char *fname)
{

	struct b_lists *pl;
	long ret = 0;
	u32 inode;

	if (! (pl  = jffs2_get_list(part, "load")))
		return 0;

	if (! (inode = jffs2_1pass_search_inode(pl, fname, 1))) {
		putstr("load: Failed to find inode\r\n");
		return 0;
	}

	/* Resolve symlinks */
	if (! (inode = jffs2_1pass_resolve_inode(pl, inode))) {
		putstr("load: Failed to resolve inode structure\r\n");
		return 0;
	}

	if ((ret = jffs2_1pass_read_inode(pl, inode, dest)) < 0) {
		putstr("load: Failed to read inode\r\n");
		return 0;
	}

	DEBUGF ("load: loaded '%s' to 0x%lx (%ld bytes)\n", fname,
				(unsigned long) dest, ret);
	return ret;
}

/* Return information about the fs on this partition */
u32
jffs2_1pass_info(struct part_info * part)
{
	struct b_jffs2_info info;
	struct b_lists *pl;
	int i;

	if (! (pl  = jffs2_get_list(part, "info")))
		return 0;

	jffs2_1pass_fill_info(pl, &info);
	for (i=0; i < JFFS2_NUM_COMPR; i++) {
		printf("Compression: %s\n", compr_names[i]);
		printf("\tfrag count: %d\n", info.compr_info[i].num_frags);
		printf("\tcompressed sum: %d\n", info.compr_info[i].compr_sum);
		printf("\tuncompressed sum: %d\n", info.compr_info[i].decompr_sum);
	}
	return 1;
}

#endif /* CFG_CMD_JFFS2 */
