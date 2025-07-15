/*
	io.c (02.09.09)
	exFAT file system implementation library.

	Free exFAT implementation.
	Copyright (C) 2010-2023  Andrew Nayenko

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "exfat.h"
#include <inttypes.h>
#ifndef __UBOOT__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#if defined(__APPLE__)
#include <sys/disk.h>
#elif defined(__OpenBSD__)
#include <sys/param.h>
#include <sys/disklabel.h>
#include <sys/dkio.h>
#include <sys/ioctl.h>
#elif defined(__NetBSD__)
#include <sys/ioctl.h>
#elif __linux__
#include <sys/mount.h>
#endif
#ifdef USE_UBLIO
#include <sys/uio.h>
#include <ublio.h>
#endif
#else
#include <fs.h>
#include <fs_internal.h>

static struct exfat_ctxt {
	struct disk_partition	cur_part_info;
	struct blk_desc		*cur_dev;
	struct exfat		ef;
} ctxt;
#endif

struct exfat_dev
{
	int fd;
	enum exfat_mode mode;
	off_t size; /* in bytes */
#ifdef USE_UBLIO
	off_t pos;
	ublio_filehandle_t ufh;
#endif
#ifdef __UBOOT__
	struct exfat_ctxt *ctxt;
#endif
};

#ifndef __UBOOT__
static bool is_open(int fd)
{
	return fcntl(fd, F_GETFD) != -1;
}

static int open_ro(const char* spec)
{
	return open(spec, O_RDONLY);
}

static int open_rw(const char* spec)
{
	int fd = open(spec, O_RDWR);
#ifdef __linux__
	int ro = 0;

	/*
	   This ioctl is needed because after "blockdev --setro" kernel still
	   allows to open the device in read-write mode but fails writes.
	*/
	if (fd != -1 && ioctl(fd, BLKROGET, &ro) == 0 && ro)
	{
		close(fd);
		errno = EROFS;
		return -1;
	}
#endif
	return fd;
}

struct exfat_dev* exfat_open(const char* spec, enum exfat_mode mode)
{
	struct exfat_dev* dev;
	struct stat stbuf;
#ifdef USE_UBLIO
	struct ublio_param up;
#endif

	/* The system allocates file descriptors sequentially. If we have been
	   started with stdin (0), stdout (1) or stderr (2) closed, the system
	   will give us descriptor 0, 1 or 2 later when we open block device,
	   FUSE communication pipe, etc. As a result, functions using stdin,
	   stdout or stderr will actually work with a different thing and can
	   corrupt it. Protect descriptors 0, 1 and 2 from such misuse. */
	while (!is_open(STDIN_FILENO)
		|| !is_open(STDOUT_FILENO)
		|| !is_open(STDERR_FILENO))
	{
		/* we don't need those descriptors, let them leak */
		if (open("/dev/null", O_RDWR) == -1)
		{
			exfat_error("failed to open /dev/null");
			return NULL;
		}
	}

	dev = malloc(sizeof(struct exfat_dev));
	if (dev == NULL)
	{
		exfat_error("failed to allocate memory for device structure");
		return NULL;
	}

	switch (mode)
	{
	case EXFAT_MODE_RO:
		dev->fd = open_ro(spec);
		if (dev->fd == -1)
		{
			free(dev);
			exfat_error("failed to open '%s' in read-only mode: %s", spec,
					strerror(errno));
			return NULL;
		}
		dev->mode = EXFAT_MODE_RO;
		break;
	case EXFAT_MODE_RW:
		dev->fd = open_rw(spec);
		if (dev->fd == -1)
		{
			free(dev);
			exfat_error("failed to open '%s' in read-write mode: %s", spec,
					strerror(errno));
			return NULL;
		}
		dev->mode = EXFAT_MODE_RW;
		break;
	case EXFAT_MODE_ANY:
		dev->fd = open_rw(spec);
		if (dev->fd != -1)
		{
			dev->mode = EXFAT_MODE_RW;
			break;
		}
		dev->fd = open_ro(spec);
		if (dev->fd != -1)
		{
			dev->mode = EXFAT_MODE_RO;
			exfat_warn("'%s' is write-protected, mounting read-only", spec);
			break;
		}
		free(dev);
		exfat_error("failed to open '%s': %s", spec, strerror(errno));
		return NULL;
	}

	if (fstat(dev->fd, &stbuf) != 0)
	{
		close(dev->fd);
		free(dev);
		exfat_error("failed to fstat '%s'", spec);
		return NULL;
	}
	if (!S_ISBLK(stbuf.st_mode) &&
		!S_ISCHR(stbuf.st_mode) &&
		!S_ISREG(stbuf.st_mode))
	{
		close(dev->fd);
		free(dev);
		exfat_error("'%s' is neither a device, nor a regular file", spec);
		return NULL;
	}

#if defined(__APPLE__)
	if (!S_ISREG(stbuf.st_mode))
	{
		uint32_t block_size = 0;
		uint64_t blocks = 0;

		if (ioctl(dev->fd, DKIOCGETBLOCKSIZE, &block_size) != 0)
		{
			close(dev->fd);
			free(dev);
			exfat_error("failed to get block size");
			return NULL;
		}
		if (ioctl(dev->fd, DKIOCGETBLOCKCOUNT, &blocks) != 0)
		{
			close(dev->fd);
			free(dev);
			exfat_error("failed to get blocks count");
			return NULL;
		}
		dev->size = blocks * block_size;
	}
	else
#elif defined(__OpenBSD__)
	if (!S_ISREG(stbuf.st_mode))
	{
		struct disklabel lab;
		struct partition* pp;
		char* partition;

		if (ioctl(dev->fd, DIOCGDINFO, &lab) == -1)
		{
			close(dev->fd);
			free(dev);
			exfat_error("failed to get disklabel");
			return NULL;
		}

		/* Don't need to check that partition letter is valid as we won't get
		   this far otherwise. */
		partition = strchr(spec, '\0') - 1;
		pp = &(lab.d_partitions[*partition - 'a']);
		dev->size = DL_GETPSIZE(pp) * lab.d_secsize;

		if (pp->p_fstype != FS_NTFS)
			exfat_warn("partition type is not 0x07 (NTFS/exFAT); "
					"you can fix this with fdisk(8)");
	}
	else
#elif defined(__NetBSD__)
	if (!S_ISREG(stbuf.st_mode))
	{
		off_t size;

		if (ioctl(dev->fd, DIOCGMEDIASIZE, &size) == -1)
		{
			close(dev->fd);
			free(dev);
			exfat_error("failed to get media size");
			return NULL;
		}
		dev->size = size;
	}
	else
#endif
	{
		/* works for Linux, FreeBSD, Solaris */
		dev->size = exfat_seek(dev, 0, SEEK_END);
		if (dev->size <= 0)
		{
			close(dev->fd);
			free(dev);
			exfat_error("failed to get size of '%s'", spec);
			return NULL;
		}
		if (exfat_seek(dev, 0, SEEK_SET) == -1)
		{
			close(dev->fd);
			free(dev);
			exfat_error("failed to seek to the beginning of '%s'", spec);
			return NULL;
		}
	}

#ifdef USE_UBLIO
	memset(&up, 0, sizeof(struct ublio_param));
	up.up_blocksize = 256 * 1024;
	up.up_items = 64;
	up.up_grace = 32;
	up.up_priv = &dev->fd;

	dev->pos = 0;
	dev->ufh = ublio_open(&up);
	if (dev->ufh == NULL)
	{
		close(dev->fd);
		free(dev);
		exfat_error("failed to initialize ublio");
		return NULL;
	}
#endif

	return dev;
}

int exfat_close(struct exfat_dev* dev)
{
	int rc = 0;

#ifdef USE_UBLIO
	if (ublio_close(dev->ufh) != 0)
	{
		exfat_error("failed to close ublio");
		rc = -EIO;
	}
#endif
	if (close(dev->fd) != 0)
	{
		exfat_error("failed to close device: %s", strerror(errno));
		rc = -EIO;
	}
	free(dev);
	return rc;
}

int exfat_fsync(struct exfat_dev* dev)
{
	int rc = 0;

#ifdef USE_UBLIO
	if (ublio_fsync(dev->ufh) != 0)
	{
		exfat_error("ublio fsync failed");
		rc = -EIO;
	}
#endif
	if (fsync(dev->fd) != 0)
	{
		exfat_error("fsync failed: %s", strerror(errno));
		rc = -EIO;
	}
	return rc;
}

enum exfat_mode exfat_get_mode(const struct exfat_dev* dev)
{
	return dev->mode;
}

off_t exfat_get_size(const struct exfat_dev* dev)
{
	return dev->size;
}

off_t exfat_seek(struct exfat_dev* dev, off_t offset, int whence)
{
#ifdef USE_UBLIO
	/* XXX SEEK_CUR will be handled incorrectly */
	return dev->pos = lseek(dev->fd, offset, whence);
#else
	return lseek(dev->fd, offset, whence);
#endif
}

ssize_t exfat_read(struct exfat_dev* dev, void* buffer, size_t size)
{
#ifdef USE_UBLIO
	ssize_t result = ublio_pread(dev->ufh, buffer, size, dev->pos);
	if (result >= 0)
		dev->pos += size;
	return result;
#else
	return read(dev->fd, buffer, size);
#endif
}

ssize_t exfat_write(struct exfat_dev* dev, const void* buffer, size_t size)
{
#ifdef USE_UBLIO
	ssize_t result = ublio_pwrite(dev->ufh, (void*) buffer, size, dev->pos);
	if (result >= 0)
		dev->pos += size;
	return result;
#else
	return write(dev->fd, buffer, size);
#endif
}

ssize_t exfat_pread(struct exfat_dev* dev, void* buffer, size_t size,
		off_t offset)
{
#ifdef USE_UBLIO
	return ublio_pread(dev->ufh, buffer, size, offset);
#else
	return pread(dev->fd, buffer, size, offset);
#endif
}

ssize_t exfat_pwrite(struct exfat_dev* dev, const void* buffer, size_t size,
		off_t offset)
{
#ifdef USE_UBLIO
	return ublio_pwrite(dev->ufh, (void*) buffer, size, offset);
#else
	return pwrite(dev->fd, buffer, size, offset);
#endif
}
#else	/* U-Boot */
struct exfat_dev* exfat_open(const char* spec, enum exfat_mode mode)
{
	struct exfat_dev* dev;

	dev = malloc(sizeof(struct exfat_dev));
	if (!dev) {
		exfat_error("failed to allocate memory for device structure");
		return NULL;
	}
	dev->mode = EXFAT_MODE_RW;
	dev->size = ctxt.cur_part_info.size * ctxt.cur_part_info.blksz;
	dev->ctxt = &ctxt;

	return dev;
}

int exfat_close(struct exfat_dev* dev)
{
	free(dev);
	return 0;
}

int exfat_fsync(struct exfat_dev* dev)
{
	return 0;
}

enum exfat_mode exfat_get_mode(const struct exfat_dev* dev)
{
	return dev->mode;
}

off_t exfat_get_size(const struct exfat_dev* dev)
{
	return dev->size;
}

ssize_t exfat_pread(struct exfat_dev* dev, void* buffer, size_t size,
		off_t offset)
{
	lbaint_t sect;
	int off;

	if (!ctxt.cur_dev)
		return -EIO;

	sect = offset >> ctxt.cur_dev->log2blksz;
	off = offset & (ctxt.cur_dev->blksz - 1);

	if (fs_devread(ctxt.cur_dev, &ctxt.cur_part_info, sect,
		       off, size, buffer))
		return 0;
	return -EIO;
}

ssize_t exfat_pwrite(struct exfat_dev* dev, const void* buffer, size_t size,
		off_t offset)
{
	lbaint_t sect;
	int off;

	if (!ctxt.cur_dev)
		return -EIO;

	sect = offset >> ctxt.cur_dev->log2blksz;
	off = offset & (ctxt.cur_dev->blksz - 1);

	if (fs_devwrite(ctxt.cur_dev, &ctxt.cur_part_info, sect,
		       off, size, buffer))
		return 0;
	return -EIO;
}
#endif

ssize_t exfat_generic_pread(const struct exfat* ef, struct exfat_node* node,
		void* buffer, size_t size, off_t offset)
{
	cluster_t cluster;
	char* bufp = buffer;
	off_t lsize, loffset, remainder;

	if (offset >= node->size)
		return 0;
	if (size == 0)
		return 0;

	if (offset + size > node->valid_size)
	{
		ssize_t bytes = 0;

		if (offset < node->valid_size)
		{
			bytes = exfat_generic_pread(ef, node, buffer,
					node->valid_size - offset, offset);
			if (bytes < 0 || (size_t)bytes < node->valid_size - offset)
				return bytes;
		}
		memset(buffer + bytes, 0,
				MIN(size - bytes, node->size - node->valid_size));
		return MIN(size, node->size - offset);
	}

	cluster = exfat_advance_cluster(ef, node, offset / CLUSTER_SIZE(*ef->sb));
	if (CLUSTER_INVALID(*ef->sb, cluster))
	{
		exfat_error("invalid cluster 0x%x while reading", cluster);
		return -EIO;
	}

	loffset = offset % CLUSTER_SIZE(*ef->sb);
	remainder = MIN(size, node->size - offset);
	while (remainder > 0)
	{
		if (CLUSTER_INVALID(*ef->sb, cluster))
		{
			exfat_error("invalid cluster 0x%x while reading", cluster);
			return -EIO;
		}
		lsize = MIN(CLUSTER_SIZE(*ef->sb) - loffset, remainder);
		if (exfat_pread(ef->dev, bufp, lsize,
					exfat_c2o(ef, cluster) + loffset) < 0)
		{
			exfat_error("failed to read cluster %#x", cluster);
			return -EIO;
		}
		bufp += lsize;
		loffset = 0;
		remainder -= lsize;
		cluster = exfat_next_cluster(ef, node, cluster);
	}
	if (!(node->attrib & EXFAT_ATTRIB_DIR) && !ef->ro && !ef->noatime)
		exfat_update_atime(node);
	return MIN(size, node->size - offset) - remainder;
}

ssize_t exfat_generic_pwrite(struct exfat* ef, struct exfat_node* node,
		const void* buffer, size_t size, off_t offset)
{
	int rc;
	cluster_t cluster;
	const char* bufp = buffer;
	off_t lsize, loffset, remainder;

	if (offset > node->size)
	{
		rc = exfat_truncate(ef, node, offset, true);
		if (rc != 0)
			return rc;
	}
	if (offset + size > node->size)
	{
		rc = exfat_truncate(ef, node, offset + size, false);
		if (rc != 0)
			return rc;
	}
	if (size == 0)
		return 0;

	cluster = exfat_advance_cluster(ef, node, offset / CLUSTER_SIZE(*ef->sb));
	if (CLUSTER_INVALID(*ef->sb, cluster))
	{
		exfat_error("invalid cluster 0x%x while writing", cluster);
		return -EIO;
	}

	loffset = offset % CLUSTER_SIZE(*ef->sb);
	remainder = size;
	while (remainder > 0)
	{
		if (CLUSTER_INVALID(*ef->sb, cluster))
		{
			exfat_error("invalid cluster 0x%x while writing", cluster);
			return -EIO;
		}
		lsize = MIN(CLUSTER_SIZE(*ef->sb) - loffset, remainder);
		if (exfat_pwrite(ef->dev, bufp, lsize,
				exfat_c2o(ef, cluster) + loffset) < 0)
		{
			exfat_error("failed to write cluster %#x", cluster);
			return -EIO;
		}
		bufp += lsize;
		loffset = 0;
		remainder -= lsize;
		node->valid_size = MAX(node->valid_size, offset + size - remainder);
		cluster = exfat_next_cluster(ef, node, cluster);
	}
	if (!(node->attrib & EXFAT_ATTRIB_DIR))
		/* directory's mtime should be updated by the caller only when it
		   creates or removes something in this directory */
		exfat_update_mtime(node);
	return size - remainder;
}

#ifdef __UBOOT__
#define PATH_MAX FS_DIRENT_NAME_LEN

struct exfat_dir_stream {
	char dirname[PATH_MAX];
	struct fs_dir_stream fs_dirs;
	struct fs_dirent dirent;
	int offset;
};

int exfat_fs_probe(struct blk_desc *fs_dev_desc,
		struct disk_partition *fs_partition)
{
	int ret;

	ctxt.cur_dev = fs_dev_desc;
	ctxt.cur_part_info = *fs_partition;

	ret = exfat_mount(&ctxt.ef, NULL, "");
	if (ret)
		goto error;

	return 0;
error:
	ctxt.cur_dev = NULL;
	return ret;
}

/* Adapted from uclibc 1.0.35 */
static char *exfat_realpath(const char *path, char got_path[])
{
	char copy_path[PATH_MAX];
	char *max_path, *new_path;
	size_t path_len;

	if (path == NULL)
		return NULL;

	if (*path == '\0')
		return NULL;

	/* Make a copy of the source path since we may need to modify it. */
	path_len = strlen(path);
	if (path_len >= PATH_MAX - 2)
		return NULL;

	/* Copy so that path is at the end of copy_path[] */
	strcpy(copy_path + (PATH_MAX-1) - path_len, path);
	path = copy_path + (PATH_MAX-1) - path_len;
	max_path = got_path + PATH_MAX - 2; /* points to last non-NUL char */
	new_path = got_path;
	*new_path++ = '/';
	path++;

	/* Expand each slash-separated pathname component. */
	while (*path != '\0') {
		/* Ignore stray "/". */
		if (*path == '/') {
			path++;
			continue;
		}

		if (*path == '.') {
			/* Ignore ".". */
			if (path[1] == '\0' || path[1] == '/') {
				path++;
				continue;
			}

			if (path[1] == '.') {
				if (path[2] == '\0' || path[2] == '/') {
					path += 2;
					/* Ignore ".." at root. */
					if (new_path == got_path + 1)
						continue;
					/* Handle ".." by backing up. */
					while ((--new_path)[-1] != '/')
						;
					continue;
				}
			}
		}

		/* Safely copy the next pathname component. */
		while (*path != '\0' && *path != '/') {
			if (new_path > max_path)
				return NULL;
			*new_path++ = *path++;
		}

		*new_path++ = '/';
	}

	/* Delete trailing slash but don't whomp a lone slash. */
	if (new_path != got_path + 1 && new_path[-1] == '/')
		new_path--;

	/* Make sure it's null terminated. */
	*new_path = '\0';
	return got_path;
}

int exfat_lookup_realpath(struct exfat* ef, struct exfat_node** node,
			  const char* path)
{
	char input_path[FS_DIRENT_NAME_LEN];
	char real_path[FS_DIRENT_NAME_LEN];
	char *name;

	/* Input is always absolute path */
	snprintf(input_path, FS_DIRENT_NAME_LEN, "/%s", path);
	name = exfat_realpath(input_path, real_path);
	if (!name)
		return -EINVAL;

	return exfat_lookup(ef, node, real_path);
}

int exfat_fs_opendir(const char *filename, struct fs_dir_stream **dirsp)
{
	struct exfat_dir_stream *dirs;
	struct exfat_node *dnode;
	int err;

	if (strlen(filename) >= PATH_MAX)
		return -ENAMETOOLONG;

	err = exfat_lookup_realpath(&ctxt.ef, &dnode, filename);
	if (err)
		return err;

	if (!(dnode->attrib & EXFAT_ATTRIB_DIR))
		err = -ENOTDIR;

	exfat_put_node(&ctxt.ef, dnode);

	if (err)
		return err;

	dirs = calloc(1, sizeof(*dirs));
	if (!dirs)
		return -ENOMEM;

	strncpy(dirs->dirname, filename, PATH_MAX - 1);
	dirs->offset = -1;

	*dirsp = &dirs->fs_dirs;

	return 0;
}

int exfat_fs_readdir(struct fs_dir_stream *fs_dirs, struct fs_dirent **dentp)
{
	struct exfat_dir_stream *dirs =
		container_of(fs_dirs, struct exfat_dir_stream, fs_dirs);
	struct fs_dirent *dent = &dirs->dirent;
	struct exfat_node *dnode, *node;
	struct exfat_iterator it;
	int offset = 0;
	int err;

	err = exfat_lookup_realpath(&ctxt.ef, &dnode, dirs->dirname);
	if (err)
		return err;

	if (!(dnode->attrib & EXFAT_ATTRIB_DIR)) {
		err = -ENOTDIR;
		goto err_out;
	}

	/* Emulate current directory ./ */
	if (dirs->offset == -1) {
		dirs->offset++;
		snprintf(dent->name, FS_DIRENT_NAME_LEN, ".");
		dent->type = FS_DT_DIR;
		*dentp = dent;
		goto err_out;
	}

	/* Emulate parent directory ../ */
	if (dirs->offset == 0) {
		dirs->offset++;
		snprintf(dent->name, FS_DIRENT_NAME_LEN, "..");
		dent->type = FS_DT_DIR;
		*dentp = dent;
		goto err_out;
	}

	err = exfat_opendir(&ctxt.ef, dnode, &it);
	if (err)
		goto err_out;

	*dentp = NULL;

	/* Read actual directory content */
	while ((node = exfat_readdir(&it))) {
		if (dirs->offset != ++offset) {
			exfat_put_node(&ctxt.ef, node);
			continue;
		}

		exfat_get_name(node, dent->name);
		if (node->attrib & EXFAT_ATTRIB_DIR) {
			dent->type = FS_DT_DIR;
		} else {
			dent->type = FS_DT_REG;
			dent->size = node->size;
		}
		exfat_put_node(&ctxt.ef, node);
		*dentp = dent;
		dirs->offset++;
		break;
	}

	exfat_closedir(&ctxt.ef, &it);

err_out:
	exfat_put_node(&ctxt.ef, dnode);
	return err;
}

void exfat_fs_closedir(struct fs_dir_stream *fs_dirs)
{
	struct exfat_dir_stream *dirs =
		container_of(fs_dirs, struct exfat_dir_stream, fs_dirs);

	free(dirs);
}

int exfat_fs_ls(const char *dirname)
{
	struct exfat_node *dnode, *node;
	char name[FS_DIRENT_NAME_LEN];
	int nfiles = 0, ndirs = 2;
	struct exfat_iterator it;
	int err;

	err = exfat_lookup_realpath(&ctxt.ef, &dnode, dirname);
	if (err)
		return err;

	if (!(dnode->attrib & EXFAT_ATTRIB_DIR)) {
		err = -ENOTDIR;
		goto err_out;
	}

	err = exfat_opendir(&ctxt.ef, dnode, &it);
	if (err)
		goto err_out;

	printf("            ./\n");
	printf("            ../\n");

	/* Read actual directory content */
	while ((node = exfat_readdir(&it))) {
		exfat_get_name(node, name);
		if (node->attrib & EXFAT_ATTRIB_DIR) {
			printf("            %s/\n", name);
			ndirs++;
		} else {
			printf(" %8lld   %s\n", node->size, name);
			nfiles++;
		}
		exfat_put_node(&ctxt.ef, node);
	}

	printf("\n%d file(s), %d dir(s)\n\n", nfiles, ndirs);

	exfat_closedir(&ctxt.ef, &it);

err_out:
	exfat_put_node(&ctxt.ef, dnode);
	return err;
}

int exfat_fs_exists(const char *filename)
{
	struct exfat_node* node;
	int err;

	err = exfat_lookup_realpath(&ctxt.ef, &node, filename);
	if (err)
		return 0;

	exfat_put_node(&ctxt.ef, node);

	return 1;
}

int exfat_fs_size(const char *filename, loff_t *size)
{
	struct exfat_node* node;
	int err;

	err = exfat_lookup_realpath(&ctxt.ef, &node, filename);
	if (err)
		return err;

	*size = node->size;

	exfat_put_node(&ctxt.ef, node);

	return 0;
}

int exfat_fs_read(const char *filename, void *buf, loff_t offset, loff_t len,
	       loff_t *actread)
{
	struct exfat_node* node;
	ssize_t sz;
	int err;

	err = exfat_lookup_realpath(&ctxt.ef, &node, filename);
	if (err)
		return err;

	if (!len)
		len = node->size;

	sz = exfat_generic_pread(&ctxt.ef, node, buf, len, offset);
	if (sz < 0) {
		*actread = 0;
		err = -EINVAL;
		goto exit;
	}

	*actread = sz;

	err = exfat_flush_node(&ctxt.ef, node);
exit:
	exfat_put_node(&ctxt.ef, node);
	return err;
}

int exfat_fs_unlink(const char *filename)
{
	struct exfat_node* node;
	int err;

	err = exfat_lookup_realpath(&ctxt.ef, &node, filename);
	if (err) {
		printf("%s: doesn't exist (%d)\n", filename, err);
		return err;
	}

	if (node->attrib & EXFAT_ATTRIB_DIR) {
		err = exfat_rmdir(&ctxt.ef, node);
		if (err == -ENOTEMPTY)
			printf("Error: directory is not empty: %d\n", err);
	} else {
		err = exfat_unlink(&ctxt.ef, node);
	}

	if (err)
		goto exit;

	exfat_put_node(&ctxt.ef, node);

	return exfat_cleanup_node(&ctxt.ef, node);
exit:
	exfat_put_node(&ctxt.ef, node);
	return err;
}

int exfat_fs_mkdir(const char *dirname)
{
	if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
		return -EINVAL;

	return exfat_mkdir(&ctxt.ef, dirname);
}

int exfat_fs_write(const char *filename, void *buf, loff_t offset,
		   loff_t len, loff_t *actwrite)
{
	struct exfat_node* node;
	ssize_t sz;
	int err;

	/*
	 * Ignore -EEXIST error here, if the file exists,
	 * this write should act as an append to offset.
	 */
	err = exfat_mknod(&ctxt.ef, filename);
	if (err && err != -EEXIST)
		return err;

	err = exfat_lookup_realpath(&ctxt.ef, &node, filename);
	if (err)
		return err;

	/* Write into directories is not allowed. */
	if (node->attrib & EXFAT_ATTRIB_DIR)
		return -EISDIR;

	/* Write past end of file is not allowed. */
	if (offset > node->size) {
		err = -EINVAL;
		goto exit;
	}

	sz = exfat_generic_pwrite(&ctxt.ef, node, buf, len, offset);
	if (sz < 0) {
		*actwrite = 0;
		err = -EINVAL;
		goto exit;
	}

	err = exfat_truncate(&ctxt.ef, node, offset + sz, false);
	if (err)
		goto exit;

	*actwrite = sz;

	err = exfat_flush_node(&ctxt.ef, node);
exit:
	exfat_put_node(&ctxt.ef, node);
	return err;
}

int exfat_fs_rename(const char *old_path, const char *new_path)
{
	return exfat_rename(&ctxt.ef, old_path, new_path);
}

void exfat_fs_close(void)
{
	exfat_unmount(&ctxt.ef);
	ctxt.cur_dev = NULL;
}
#endif	/* __U_BOOT__ */
