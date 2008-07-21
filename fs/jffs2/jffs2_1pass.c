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
 *	(1) most pages are 4096 bytes
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

/*
 * Bugfixing by Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>, (C) Mar/2003
 *
 * - overhaul of the memory management. Removed much of the "paper-bagging"
 *   in that part of the code, fixed several bugs, now frees memory when
 *   partition is changed.
 *   It's still ugly :-(
 * - fixed a bug in jffs2_1pass_read_inode where the file length calculation
 *   was incorrect. Removed a bit of the paper-bagging as well.
 * - removed double crc calculation for fragment headers in jffs2_private.h
 *   for speedup.
 * - scan_empty rewritten in a more "standard" manner (non-paperbag, that is).
 * - spinning wheel now spins depending on how much memory has been scanned
 * - lots of small changes all over the place to "improve" readability.
 * - implemented fragment sorting to ensure that the newest data is copied
 *   if there are multiple copies of fragments for a certain file offset.
 *
 * The fragment sorting feature must be enabled by CFG_JFFS2_SORT_FRAGMENTS.
 * Sorting is done while adding fragments to the lists, which is more or less a
 * bubble sort. This takes a lot of time, and is most probably not an issue if
 * the boot filesystem is always mounted readonly.
 *
 * You should define it if the boot filesystem is mounted writable, and updates
 * to the boot files are done by copying files to that filesystem.
 *
 *
 * There's a big issue left: endianess is completely ignored in this code. Duh!
 *
 *
 * You still should have paper bags at hand :-(. The code lacks more or less
 * any comment, and is still arcane and difficult to read in places. As this
 * might be incompatible with any new code from the jffs2 maintainers anyway,
 * it should probably be dumped and replaced by something like jffs2reader!
 */


#include <common.h>
#include <config.h>
#include <malloc.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <watchdog.h>

#if defined(CONFIG_CMD_JFFS2)

#include <jffs2/jffs2.h>
#include <jffs2/jffs2_1pass.h>

#include "jffs2_private.h"


#define	NODE_CHUNK	1024	/* size of memory allocation chunk in b_nodes */
#define	SPIN_BLKSIZE	18	/* spin after having scanned 1<<BLKSIZE bytes */

/* Debugging switches */
#undef	DEBUG_DIRENTS		/* print directory entry list after scan */
#undef	DEBUG_FRAGMENTS		/* print fragment list after scan */
#undef	DEBUG			/* enable debugging messages */


#ifdef  DEBUG
# define DEBUGF(fmt,args...)	printf(fmt ,##args)
#else
# define DEBUGF(fmt,args...)
#endif

/* keeps pointer to currentlu processed partition */
static struct part_info *current_part;

#if (defined(CONFIG_JFFS2_NAND) && \
     defined(CONFIG_CMD_NAND) )
#if defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand_legacy.h>
#else
#include <nand.h>
#endif
/*
 * Support for jffs2 on top of NAND-flash
 *
 * NAND memory isn't mapped in processor's address space,
 * so data should be fetched from flash before
 * being processed. This is exactly what functions declared
 * here do.
 *
 */

#if defined(CFG_NAND_LEGACY)
/* this one defined in nand_legacy.c */
int read_jffs2_nand(size_t start, size_t len,
		size_t * retlen, u_char * buf, int nanddev);
#endif

#define NAND_PAGE_SIZE 512
#define NAND_PAGE_SHIFT 9
#define NAND_PAGE_MASK (~(NAND_PAGE_SIZE-1))

#ifndef NAND_CACHE_PAGES
#define NAND_CACHE_PAGES 16
#endif
#define NAND_CACHE_SIZE (NAND_CACHE_PAGES*NAND_PAGE_SIZE)

static u8* nand_cache = NULL;
static u32 nand_cache_off = (u32)-1;

static int read_nand_cached(u32 off, u32 size, u_char *buf)
{
	struct mtdids *id = current_part->dev->id;
	u32 bytes_read = 0;
	size_t retlen;
	int cpy_bytes;

	while (bytes_read < size) {
		if ((off + bytes_read < nand_cache_off) ||
		    (off + bytes_read >= nand_cache_off+NAND_CACHE_SIZE)) {
			nand_cache_off = (off + bytes_read) & NAND_PAGE_MASK;
			if (!nand_cache) {
				/* This memory never gets freed but 'cause
				   it's a bootloader, nobody cares */
				nand_cache = malloc(NAND_CACHE_SIZE);
				if (!nand_cache) {
					printf("read_nand_cached: can't alloc cache size %d bytes\n",
					       NAND_CACHE_SIZE);
					return -1;
				}
			}

#if defined(CFG_NAND_LEGACY)
			if (read_jffs2_nand(nand_cache_off, NAND_CACHE_SIZE,
						&retlen, nand_cache, id->num) < 0 ||
					retlen != NAND_CACHE_SIZE) {
				printf("read_nand_cached: error reading nand off %#x size %d bytes\n",
						nand_cache_off, NAND_CACHE_SIZE);
				return -1;
			}
#else
			retlen = NAND_CACHE_SIZE;
			if (nand_read(&nand_info[id->num], nand_cache_off,
						&retlen, nand_cache) != 0 ||
					retlen != NAND_CACHE_SIZE) {
				printf("read_nand_cached: error reading nand off %#x size %d bytes\n",
						nand_cache_off, NAND_CACHE_SIZE);
				return -1;
			}
#endif
		}
		cpy_bytes = nand_cache_off + NAND_CACHE_SIZE - (off + bytes_read);
		if (cpy_bytes > size - bytes_read)
			cpy_bytes = size - bytes_read;
		memcpy(buf + bytes_read,
		       nand_cache + off + bytes_read - nand_cache_off,
		       cpy_bytes);
		bytes_read += cpy_bytes;
	}
	return bytes_read;
}

static void *get_fl_mem_nand(u32 off, u32 size, void *ext_buf)
{
	u_char *buf = ext_buf ? (u_char*)ext_buf : (u_char*)malloc(size);

	if (NULL == buf) {
		printf("get_fl_mem_nand: can't alloc %d bytes\n", size);
		return NULL;
	}
	if (read_nand_cached(off, size, buf) < 0) {
		if (!ext_buf)
			free(buf);
		return NULL;
	}

	return buf;
}

static void *get_node_mem_nand(u32 off)
{
	struct jffs2_unknown_node node;
	void *ret = NULL;

	if (NULL == get_fl_mem_nand(off, sizeof(node), &node))
		return NULL;

	if (!(ret = get_fl_mem_nand(off, node.magic ==
			       JFFS2_MAGIC_BITMASK ? node.totlen : sizeof(node),
			       NULL))) {
		printf("off = %#x magic %#x type %#x node.totlen = %d\n",
		       off, node.magic, node.nodetype, node.totlen);
	}
	return ret;
}

static void put_fl_mem_nand(void *buf)
{
	free(buf);
}
#endif


#if defined(CONFIG_CMD_FLASH)
/*
 * Support for jffs2 on top of NOR-flash
 *
 * NOR flash memory is mapped in processor's address space,
 * just return address.
 */
static inline void *get_fl_mem_nor(u32 off)
{
	u32 addr = off;
	struct mtdids *id = current_part->dev->id;

	extern flash_info_t flash_info[];
	flash_info_t *flash = &flash_info[id->num];

	addr += flash->start[0];
	return (void*)addr;
}

static inline void *get_node_mem_nor(u32 off)
{
	return (void*)get_fl_mem_nor(off);
}
#endif


/*
 * Generic jffs2 raw memory and node read routines.
 *
 */
static inline void *get_fl_mem(u32 off, u32 size, void *ext_buf)
{
	struct mtdids *id = current_part->dev->id;

#if defined(CONFIG_CMD_FLASH)
	if (id->type == MTD_DEV_TYPE_NOR)
		return get_fl_mem_nor(off);
#endif

#if defined(CONFIG_JFFS2_NAND) && defined(CONFIG_CMD_NAND)
	if (id->type == MTD_DEV_TYPE_NAND)
		return get_fl_mem_nand(off, size, ext_buf);
#endif

	printf("get_fl_mem: unknown device type, using raw offset!\n");
	return (void*)off;
}

static inline void *get_node_mem(u32 off)
{
	struct mtdids *id = current_part->dev->id;

#if defined(CONFIG_CMD_FLASH)
	if (id->type == MTD_DEV_TYPE_NOR)
		return get_node_mem_nor(off);
#endif

#if defined(CONFIG_JFFS2_NAND) && \
    defined(CONFIG_CMD_NAND)
	if (id->type == MTD_DEV_TYPE_NAND)
		return get_node_mem_nand(off);
#endif

	printf("get_node_mem: unknown device type, using raw offset!\n");
	return (void*)off;
}

static inline void put_fl_mem(void *buf)
{
#if defined(CONFIG_JFFS2_NAND) && \
    defined(CONFIG_CMD_NAND)
	struct mtdids *id = current_part->dev->id;

	if (id->type == MTD_DEV_TYPE_NAND)
		return put_fl_mem_nand(buf);
#endif
}

/* Compression names */
static char *compr_names[] = {
	"NONE",
	"ZERO",
	"RTIME",
	"RUBINMIPS",
	"COPY",
	"DYNRUBIN",
	"ZLIB",
#if defined(CONFIG_JFFS2_LZO_LZARI)
	"LZO",
	"LZARI",
#endif
};

/* Spinning wheel */
static char spinner[] = { '|', '/', '-', '\\' };

/* Memory management */
struct mem_block {
	u32	index;
	struct mem_block *next;
	struct b_node nodes[NODE_CHUNK];
};


static void
free_nodes(struct b_list *list)
{
	while (list->listMemBase != NULL) {
		struct mem_block *next = list->listMemBase->next;
		free( list->listMemBase );
		list->listMemBase = next;
	}
}

static struct b_node *
add_node(struct b_list *list)
{
	u32 index = 0;
	struct mem_block *memBase;
	struct b_node *b;

	memBase = list->listMemBase;
	if (memBase != NULL)
		index = memBase->index;
#if 0
	putLabeledWord("add_node: index = ", index);
	putLabeledWord("add_node: memBase = ", list->listMemBase);
#endif

	if (memBase == NULL || index >= NODE_CHUNK) {
		/* we need more space before we continue */
		memBase = mmalloc(sizeof(struct mem_block));
		if (memBase == NULL) {
			putstr("add_node: malloc failed\n");
			return NULL;
		}
		memBase->next = list->listMemBase;
		index = 0;
#if 0
		putLabeledWord("add_node: alloced a new membase at ", *memBase);
#endif

	}
	/* now we have room to add it. */
	b = &memBase->nodes[index];
	index ++;

	memBase->index = index;
	list->listMemBase = memBase;
	list->listCount++;
	return b;
}

static struct b_node *
insert_node(struct b_list *list, u32 offset)
{
	struct b_node *new;
#ifdef CFG_JFFS2_SORT_FRAGMENTS
	struct b_node *b, *prev;
#endif

	if (!(new = add_node(list))) {
		putstr("add_node failed!\r\n");
		return NULL;
	}
	new->offset = offset;

#ifdef CFG_JFFS2_SORT_FRAGMENTS
	if (list->listTail != NULL && list->listCompare(new, list->listTail))
		prev = list->listTail;
	else if (list->listLast != NULL && list->listCompare(new, list->listLast))
		prev = list->listLast;
	else
		prev = NULL;

	for (b = (prev ? prev->next : list->listHead);
	     b != NULL && list->listCompare(new, b);
	     prev = b, b = b->next) {
		list->listLoops++;
	}
	if (b != NULL)
		list->listLast = prev;

	if (b != NULL) {
		new->next = b;
		if (prev != NULL)
			prev->next = new;
		else
			list->listHead = new;
	} else
#endif
	{
		new->next = (struct b_node *) NULL;
		if (list->listTail != NULL) {
			list->listTail->next = new;
			list->listTail = new;
		} else {
			list->listTail = list->listHead = new;
		}
	}

	return new;
}

#ifdef CFG_JFFS2_SORT_FRAGMENTS
/* Sort data entries with the latest version last, so that if there
 * is overlapping data the latest version will be used.
 */
static int compare_inodes(struct b_node *new, struct b_node *old)
{
	struct jffs2_raw_inode ojNew;
	struct jffs2_raw_inode ojOld;
	struct jffs2_raw_inode *jNew =
		(struct jffs2_raw_inode *)get_fl_mem(new->offset, sizeof(ojNew), &ojNew);
	struct jffs2_raw_inode *jOld =
		(struct jffs2_raw_inode *)get_fl_mem(old->offset, sizeof(ojOld), &ojOld);

	return jNew->version > jOld->version;
}

/* Sort directory entries so all entries in the same directory
 * with the same name are grouped together, with the latest version
 * last. This makes it easy to eliminate all but the latest version
 * by marking the previous version dead by setting the inode to 0.
 */
static int compare_dirents(struct b_node *new, struct b_node *old)
{
	struct jffs2_raw_dirent ojNew;
	struct jffs2_raw_dirent ojOld;
	struct jffs2_raw_dirent *jNew =
		(struct jffs2_raw_dirent *)get_fl_mem(new->offset, sizeof(ojNew), &ojNew);
	struct jffs2_raw_dirent *jOld =
		(struct jffs2_raw_dirent *)get_fl_mem(old->offset, sizeof(ojOld), &ojOld);
	int cmp;

	/* ascending sort by pino */
	if (jNew->pino != jOld->pino)
		return jNew->pino > jOld->pino;

	/* pino is the same, so use ascending sort by nsize, so
	 * we don't do strncmp unless we really must.
	 */
	if (jNew->nsize != jOld->nsize)
		return jNew->nsize > jOld->nsize;

	/* length is also the same, so use ascending sort by name
	 */
	cmp = strncmp((char *)jNew->name, (char *)jOld->name, jNew->nsize);
	if (cmp != 0)
		return cmp > 0;

	/* we have duplicate names in this directory, so use ascending
	 * sort by version
	 */
	if (jNew->version > jOld->version) {
		/* since jNew is newer, we know jOld is not valid, so
		 * mark it with inode 0 and it will not be used
		 */
		jOld->ino = 0;
		return 1;
	}

	return 0;
}
#endif

static u32
jffs2_scan_empty(u32 start_offset, struct part_info *part)
{
	char *max = (char *)(part->offset + part->size - sizeof(struct jffs2_raw_inode));
	char *offset = (char *)(part->offset + start_offset);
	u32 off;

	while (offset < max &&
	       *(u32*)get_fl_mem((u32)offset, sizeof(u32), &off) == 0xFFFFFFFF) {
		offset += sizeof(u32);
		/* return if spinning is due */
		if (((u32)offset & ((1 << SPIN_BLKSIZE)-1)) == 0) break;
	}

	return (u32)offset - part->offset;
}

void
jffs2_free_cache(struct part_info *part)
{
	struct b_lists *pL;

	if (part->jffs2_priv != NULL) {
		pL = (struct b_lists *)part->jffs2_priv;
		free_nodes(&pL->frag);
		free_nodes(&pL->dir);
		free(pL);
	}
}

static u32
jffs_init_1pass_list(struct part_info *part)
{
	struct b_lists *pL;

	jffs2_free_cache(part);

	if (NULL != (part->jffs2_priv = malloc(sizeof(struct b_lists)))) {
		pL = (struct b_lists *)part->jffs2_priv;

		memset(pL, 0, sizeof(*pL));
#ifdef CFG_JFFS2_SORT_FRAGMENTS
		pL->dir.listCompare = compare_dirents;
		pL->frag.listCompare = compare_inodes;
#endif
	}
	return 0;
}

/* find the inode from the slashless name given a parent */
static long
jffs2_1pass_read_inode(struct b_lists *pL, u32 inode, char *dest)
{
	struct b_node *b;
	struct jffs2_raw_inode *jNode;
	u32 totalSize = 0;
	u32 latestVersion = 0;
	uchar *lDest;
	uchar *src;
	long ret;
	int i;
	u32 counter = 0;
#ifdef CFG_JFFS2_SORT_FRAGMENTS
	/* Find file size before loading any data, so fragments that
	 * start past the end of file can be ignored. A fragment
	 * that is partially in the file is loaded, so extra data may
	 * be loaded up to the next 4K boundary above the file size.
	 * This shouldn't cause trouble when loading kernel images, so
	 * we will live with it.
	 */
	for (b = pL->frag.listHead; b != NULL; b = b->next) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
		        sizeof(struct jffs2_raw_inode), NULL);
		if ((inode == jNode->ino)) {
			/* get actual file length from the newest node */
			if (jNode->version >= latestVersion) {
				totalSize = jNode->isize;
				latestVersion = jNode->version;
			}
		}
		put_fl_mem(jNode);
	}
#endif

	for (b = pL->frag.listHead; b != NULL; b = b->next) {
		jNode = (struct jffs2_raw_inode *) get_node_mem(b->offset);
		if ((inode == jNode->ino)) {
#if 0
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
#endif

#ifndef CFG_JFFS2_SORT_FRAGMENTS
			/* get actual file length from the newest node */
			if (jNode->version >= latestVersion) {
				totalSize = jNode->isize;
				latestVersion = jNode->version;
			}
#endif

			if(dest) {
				src = ((uchar *) jNode) + sizeof(struct jffs2_raw_inode);
				/* ignore data behind latest known EOF */
				if (jNode->offset > totalSize) {
					put_fl_mem(jNode);
					continue;
				}

				lDest = (uchar *) (dest + jNode->offset);
#if 0
				putLabeledWord("read_inode: src = ", src);
				putLabeledWord("read_inode: dest = ", lDest);
#endif
				switch (jNode->compr) {
				case JFFS2_COMPR_NONE:
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
#if defined(CONFIG_JFFS2_LZO_LZARI)
				case JFFS2_COMPR_LZO:
					ret = lzo_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
				case JFFS2_COMPR_LZARI:
					ret = lzari_decompress(src, lDest, jNode->csize, jNode->dsize);
					break;
#endif
				default:
					/* unknown */
					putLabeledWord("UNKOWN COMPRESSION METHOD = ", jNode->compr);
					put_fl_mem(jNode);
					return -1;
					break;
				}
			}

#if 0
			putLabeledWord("read_inode: totalSize = ", totalSize);
			putLabeledWord("read_inode: compr ret = ", ret);
#endif
		}
		counter++;
		put_fl_mem(jNode);
	}

#if 0
	putLabeledWord("read_inode: returning = ", totalSize);
#endif
	return totalSize;
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
	for(b = pL->dir.listHead; b; b = b->next, counter++) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset);
		if ((pino == jDir->pino) && (len == jDir->nsize) &&
		    (jDir->ino) &&	/* 0 for unlink */
		    (!strncmp((char *)jDir->name, name, len))) {	/* a match */
			if (jDir->version < version) {
				put_fl_mem(jDir);
				continue;
			}

			if (jDir->version == version && inode != 0) {
				/* I'm pretty sure this isn't legal */
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
		put_fl_mem(jDir);
	}
	return inode;
}

char *mkmodestr(unsigned long mode, char *str)
{
	static const char *l = "xwr";
	int mask = 1, i;
	char c;

	switch (mode & S_IFMT) {
		case S_IFDIR:    str[0] = 'd'; break;
		case S_IFBLK:    str[0] = 'b'; break;
		case S_IFCHR:    str[0] = 'c'; break;
		case S_IFIFO:    str[0] = 'f'; break;
		case S_IFLNK:    str[0] = 'l'; break;
		case S_IFSOCK:   str[0] = 's'; break;
		case S_IFREG:    str[0] = '-'; break;
		default:         str[0] = '?';
	}

	for(i = 0; i < 9; i++) {
		c = l[i%3];
		str[9-i] = (mode & mask)?c:'-';
		mask = mask<<1;
	}

	if(mode & S_ISUID) str[3] = (mode & S_IXUSR)?'s':'S';
	if(mode & S_ISGID) str[6] = (mode & S_IXGRP)?'s':'S';
	if(mode & S_ISVTX) str[9] = (mode & S_IXOTH)?'t':'T';
	str[10] = '\0';
	return str;
}

static inline void dump_stat(struct stat *st, const char *name)
{
	char str[20];
	char s[64], *p;

	if (st->st_mtime == (time_t)(-1)) /* some ctimes really hate -1 */
		st->st_mtime = 1;

	ctime_r((time_t *)&st->st_mtime, s/*,64*/); /* newlib ctime doesn't have buflen */

	if ((p = strchr(s,'\n')) != NULL) *p = '\0';
	if ((p = strchr(s,'\r')) != NULL) *p = '\0';

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

	strncpy(fname, (char *)d->name, d->nsize);
	fname[d->nsize] = '\0';

	memset(&st,0,sizeof(st));

	st.st_mtime = i->mtime;
	st.st_mode = i->mode;
	st.st_ino = i->ino;

	/* neither dsize nor isize help us.. do it the long way */
	st.st_size = jffs2_1pass_read_inode(pL, i->ino, NULL);

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

	for (b = pL->dir.listHead; b; b = b->next) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset);
		if ((pino == jDir->pino) && (jDir->ino)) { /* ino=0 -> unlink */
			u32 i_version = 0;
			struct jffs2_raw_inode ojNode;
			struct jffs2_raw_inode *jNode, *i = NULL;
			struct b_node *b2 = pL->frag.listHead;

			while (b2) {
				jNode = (struct jffs2_raw_inode *)
					get_fl_mem(b2->offset, sizeof(ojNode), &ojNode);
				if (jNode->ino == jDir->ino && jNode->version >= i_version) {
					if (i)
						put_fl_mem(i);

					if (jDir->type == DT_LNK)
						i = get_node_mem(b2->offset);
					else
						i = get_fl_mem(b2->offset, sizeof(*i), NULL);
				}
				b2 = b2->next;
			}

			dump_inode(pL, jDir, i);
			put_fl_mem(i);
		}
		put_fl_mem(jDir);
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
	u8 jDirFoundType = 0;
	u32 jDirFoundIno = 0;
	u32 jDirFoundPino = 0;
	char tmp[256];
	u32 version = 0;
	u32 pino;
	unsigned char *src;

	/* we need to search all and return the inode with the highest version */
	for(b = pL->dir.listHead; b; b = b->next) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset);
		if (ino == jDir->ino) {
			if (jDir->version < version) {
				put_fl_mem(jDir);
				continue;
			}

			if (jDir->version == version && jDirFoundType) {
				/* I'm pretty sure this isn't legal */
				putstr(" ** ERROR ** ");
				putnstr(jDir->name, jDir->nsize);
				putLabeledWord(" has dup version (resolve) = ",
					version);
			}

			jDirFoundType = jDir->type;
			jDirFoundIno = jDir->ino;
			jDirFoundPino = jDir->pino;
			version = jDir->version;
		}
		put_fl_mem(jDir);
	}
	/* now we found the right entry again. (shoulda returned inode*) */
	if (jDirFoundType != DT_LNK)
		return jDirFoundIno;

	/* it's a soft link so we follow it again. */
	b2 = pL->frag.listHead;
	while (b2) {
		jNode = (struct jffs2_raw_inode *) get_node_mem(b2->offset);
		if (jNode->ino == jDirFoundIno) {
			src = (unsigned char *)jNode + sizeof(struct jffs2_raw_inode);

#if 0
			putLabeledWord("\t\t dsize = ", jNode->dsize);
			putstr("\t\t target = ");
			putnstr(src, jNode->dsize);
			putstr("\r\n");
#endif
			strncpy(tmp, (char *)src, jNode->dsize);
			tmp[jNode->dsize] = '\0';
			put_fl_mem(jNode);
			break;
		}
		b2 = b2->next;
		put_fl_mem(jNode);
	}
	/* ok so the name of the new file to find is in tmp */
	/* if it starts with a slash it is root based else shared dirs */
	if (tmp[0] == '/')
		pino = 1;
	else
		pino = jDirFoundPino;

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
		if (!(pino = jffs2_1pass_find_inode(pL, working_tmp, pino)) &&
		    (working_tmp[0])) {
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
	struct jffs2_unknown_node onode;
	struct jffs2_unknown_node *node;
	struct b_lists *pL = (struct b_lists *)part->jffs2_priv;

	if (part->jffs2_priv == 0){
		DEBUGF ("rescan: First time in use\n");
		return 1;
	}

	/* if we have no list, we need to rescan */
	if (pL->frag.listCount == 0) {
		DEBUGF ("rescan: fraglist zero\n");
		return 1;
	}

	/* but suppose someone reflashed a partition at the same offset... */
	b = pL->dir.listHead;
	while (b) {
		node = (struct jffs2_unknown_node *) get_fl_mem(b->offset,
			sizeof(onode), &onode);
		if (node->nodetype != JFFS2_NODETYPE_DIRENT) {
			DEBUGF ("rescan: fs changed beneath me? (%lx)\n",
					(unsigned long) b->offset);
			return 1;
		}
		b = b->next;
	}
	return 0;
}

#ifdef DEBUG_FRAGMENTS
static void
dump_fragments(struct b_lists *pL)
{
	struct b_node *b;
	struct jffs2_raw_inode ojNode;
	struct jffs2_raw_inode *jNode;

	putstr("\r\n\r\n******The fragment Entries******\r\n");
	b = pL->frag.listHead;
	while (b) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
			sizeof(ojNode), &ojNode);
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
		putLabeledWord("\tbuild_list: offset = ", b->offset);	/* FIXME: ? [RS] */
		b = b->next;
	}
}
#endif

#ifdef DEBUG_DIRENTS
static void
dump_dirents(struct b_lists *pL)
{
	struct b_node *b;
	struct jffs2_raw_dirent *jDir;

	putstr("\r\n\r\n******The directory Entries******\r\n");
	b = pL->dir.listHead;
	while (b) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset);
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
		putLabeledWord("\tbuild_list: offset = ", b->offset);	/* FIXME: ? [RS] */
		b = b->next;
		put_fl_mem(jDir);
	}
}
#endif

static u32
jffs2_1pass_build_lists(struct part_info * part)
{
	struct b_lists *pL;
	struct jffs2_unknown_node *node;
	u32 offset, oldoffset = 0;
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
	jffs_init_1pass_list(part);
	pL = (struct b_lists *)part->jffs2_priv;
	offset = 0;
	puts ("Scanning JFFS2 FS:   ");

	/* start at the beginning of the partition */
	while (offset < max) {
		if ((oldoffset >> SPIN_BLKSIZE) != (offset >> SPIN_BLKSIZE)) {
			printf("\b\b%c ", spinner[counter++ % sizeof(spinner)]);
			oldoffset = offset;
		}

		WATCHDOG_RESET();

		node = (struct jffs2_unknown_node *) get_node_mem((u32)part->offset + offset);
		if (node->magic == JFFS2_MAGIC_BITMASK && hdr_crc(node)) {
			/* if its a fragment add it */
			if (node->nodetype == JFFS2_NODETYPE_INODE &&
				    inode_crc((struct jffs2_raw_inode *) node) &&
				    data_crc((struct jffs2_raw_inode *) node)) {
				if (insert_node(&pL->frag, (u32) part->offset +
						offset) == NULL) {
					put_fl_mem(node);
					return 0;
				}
			} else if (node->nodetype == JFFS2_NODETYPE_DIRENT &&
				   dirent_crc((struct jffs2_raw_dirent *) node)  &&
				   dirent_name_crc((struct jffs2_raw_dirent *) node)) {
				if (! (counterN%100))
					puts ("\b\b.  ");
				if (insert_node(&pL->dir, (u32) part->offset +
						offset) == NULL) {
					put_fl_mem(node);
					return 0;
				}
				counterN++;
			} else if (node->nodetype == JFFS2_NODETYPE_CLEANMARKER) {
				if (node->totlen != sizeof(struct jffs2_unknown_node))
					printf("OOPS Cleanmarker has bad size "
						"%d != %zu\n",
						node->totlen,
						sizeof(struct jffs2_unknown_node));
			} else if (node->nodetype == JFFS2_NODETYPE_PADDING) {
				if (node->totlen < sizeof(struct jffs2_unknown_node))
					printf("OOPS Padding has bad size "
						"%d < %zu\n",
						node->totlen,
						sizeof(struct jffs2_unknown_node));
			} else {
				printf("Unknown node type: %x len %d offset 0x%x\n",
					node->nodetype,
					node->totlen, offset);
			}
			offset += ((node->totlen + 3) & ~3);
			counterF++;
		} else if (node->magic == JFFS2_EMPTY_BITMASK &&
			   node->nodetype == JFFS2_EMPTY_BITMASK) {
			offset = jffs2_scan_empty(offset, part);
		} else {	/* if we know nothing, we just step and look. */
			offset += 4;
			counter4++;
		}
/*             printf("unknown node magic %4.4x %4.4x @ %lx\n", node->magic, node->nodetype, (unsigned long)node); */
		put_fl_mem(node);
	}

	putstr("\b\b done.\r\n");		/* close off the dots */
	/* turn the lcd back on. */
	/* splash(); */

#if 0
	putLabeledWord("dir entries = ", pL->dir.listCount);
	putLabeledWord("frag entries = ", pL->frag.listCount);
	putLabeledWord("+4 increments = ", counter4);
	putLabeledWord("+file_offset increments = ", counterF);

#endif

#ifdef DEBUG_DIRENTS
	dump_dirents(pL);
#endif

#ifdef DEBUG_FRAGMENTS
	dump_fragments(pL);
#endif

	/* give visual feedback that we are done scanning the flash */
	led_blink(0x0, 0x0, 0x1, 0x1);	/* off, forever, on 100ms, off 100ms */
	return 1;
}


static u32
jffs2_1pass_fill_info(struct b_lists * pL, struct b_jffs2_info * piL)
{
	struct b_node *b;
	struct jffs2_raw_inode ojNode;
	struct jffs2_raw_inode *jNode;
	int i;

	for (i = 0; i < JFFS2_NUM_COMPR; i++) {
		piL->compr_info[i].num_frags = 0;
		piL->compr_info[i].compr_sum = 0;
		piL->compr_info[i].decompr_sum = 0;
	}

	b = pL->frag.listHead;
	while (b) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
			sizeof(ojNode), &ojNode);
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
	/* copy requested part_info struct pointer to global location */
	current_part = part;

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
	long ret = 1;
	u32 inode;

	if (! (pl = jffs2_get_list(part, "ls")))
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
	long ret = 1;
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
	for (i = 0; i < JFFS2_NUM_COMPR; i++) {
		printf ("Compression: %s\n"
			"\tfrag count: %d\n"
			"\tcompressed sum: %d\n"
			"\tuncompressed sum: %d\n",
			compr_names[i],
			info.compr_info[i].num_frags,
			info.compr_info[i].compr_sum,
			info.compr_info[i].decompr_sum);
	}
	return 1;
}

#endif
