// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI utils
 *
 *  Copyright (c) 2017 Rob Clark
 */

#include <common.h>
#include <charset.h>
#include <efi_loader.h>
#include <malloc.h>
#include <fs.h>

/* GUID for file system information */
const efi_guid_t efi_file_system_info_guid = EFI_FILE_SYSTEM_INFO_GUID;

struct file_system {
	struct efi_simple_file_system_protocol base;
	struct efi_device_path *dp;
	struct blk_desc *desc;
	int part;
};
#define to_fs(x) container_of(x, struct file_system, base)

struct file_handle {
	struct efi_file_handle base;
	struct file_system *fs;
	loff_t offset;       /* current file position/cursor */
	int isdir;

	/* for reading a directory: */
	struct fs_dir_stream *dirs;
	struct fs_dirent *dent;

	char path[0];
};
#define to_fh(x) container_of(x, struct file_handle, base)

static const struct efi_file_handle efi_file_handle_protocol;

static char *basename(struct file_handle *fh)
{
	char *s = strrchr(fh->path, '/');
	if (s)
		return s + 1;
	return fh->path;
}

static int set_blk_dev(struct file_handle *fh)
{
	return fs_set_blk_dev_with_part(fh->fs->desc, fh->fs->part);
}

static int is_dir(struct file_handle *fh)
{
	struct fs_dir_stream *dirs;

	set_blk_dev(fh);
	dirs = fs_opendir(fh->path);
	if (!dirs)
		return 0;

	fs_closedir(dirs);

	return 1;
}

/*
 * Normalize a path which may include either back or fwd slashes,
 * double slashes, . or .. entries in the path, etc.
 */
static int sanitize_path(char *path)
{
	char *p;

	/* backslash to slash: */
	p = path;
	while ((p = strchr(p, '\\')))
		*p++ = '/';

	/* handle double-slashes: */
	p = path;
	while ((p = strstr(p, "//"))) {
		char *src = p + 1;
		memmove(p, src, strlen(src) + 1);
	}

	/* handle extra /.'s */
	p = path;
	while ((p = strstr(p, "/."))) {
		/*
		 * You'd be tempted to do this *after* handling ".."s
		 * below to avoid having to check if "/." is start of
		 * a "/..", but that won't have the correct results..
		 * for example, "/foo/./../bar" would get resolved to
		 * "/foo/bar" if you did these two passes in the other
		 * order
		 */
		if (p[2] == '.') {
			p += 2;
			continue;
		}
		char *src = p + 2;
		memmove(p, src, strlen(src) + 1);
	}

	/* handle extra /..'s: */
	p = path;
	while ((p = strstr(p, "/.."))) {
		char *src = p + 3;

		p--;

		/* find beginning of previous path entry: */
		while (true) {
			if (p < path)
				return -1;
			if (*p == '/')
				break;
			p--;
		}

		memmove(p, src, strlen(src) + 1);
	}

	return 0;
}

/* NOTE: despite what you would expect, 'file_name' is actually a path.
 * With windoze style backlashes, ofc.
 */
static struct efi_file_handle *file_open(struct file_system *fs,
		struct file_handle *parent, s16 *file_name, u64 mode)
{
	struct file_handle *fh;
	char f0[MAX_UTF8_PER_UTF16] = {0};
	int plen = 0;
	int flen = 0;

	if (file_name) {
		utf16_to_utf8((u8 *)f0, (u16 *)file_name, 1);
		flen = utf16_strlen((u16 *)file_name);
	}

	/* we could have a parent, but also an absolute path: */
	if (f0[0] == '\\') {
		plen = 0;
	} else if (parent) {
		plen = strlen(parent->path) + 1;
	}

	/* +2 is for null and '/' */
	fh = calloc(1, sizeof(*fh) + plen + (flen * MAX_UTF8_PER_UTF16) + 2);

	fh->base = efi_file_handle_protocol;
	fh->fs = fs;

	if (parent) {
		char *p = fh->path;

		if (plen > 0) {
			strcpy(p, parent->path);
			p += plen - 1;
			*p++ = '/';
		}

		utf16_to_utf8((u8 *)p, (u16 *)file_name, flen);

		if (sanitize_path(fh->path))
			goto error;

		/* check if file exists: */
		if (set_blk_dev(fh))
			goto error;

		if (!((mode & EFI_FILE_MODE_CREATE) || fs_exists(fh->path)))
			goto error;

		/* figure out if file is a directory: */
		fh->isdir = is_dir(fh);
	} else {
		fh->isdir = 1;
		strcpy(fh->path, "");
	}

	return &fh->base;

error:
	free(fh);
	return NULL;
}

static efi_status_t EFIAPI efi_file_open(struct efi_file_handle *file,
		struct efi_file_handle **new_handle,
		s16 *file_name, u64 open_mode, u64 attributes)
{
	struct file_handle *fh = to_fh(file);

	EFI_ENTRY("%p, %p, \"%ls\", %llx, %llu", file, new_handle, file_name,
		  open_mode, attributes);

	*new_handle = file_open(fh->fs, fh, file_name, open_mode);
	if (!*new_handle)
		return EFI_EXIT(EFI_NOT_FOUND);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t file_close(struct file_handle *fh)
{
	fs_closedir(fh->dirs);
	free(fh);
	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_file_close(struct efi_file_handle *file)
{
	struct file_handle *fh = to_fh(file);
	EFI_ENTRY("%p", file);
	return EFI_EXIT(file_close(fh));
}

static efi_status_t EFIAPI efi_file_delete(struct efi_file_handle *file)
{
	struct file_handle *fh = to_fh(file);
	EFI_ENTRY("%p", file);
	file_close(fh);
	return EFI_EXIT(EFI_WARN_DELETE_FAILURE);
}

static efi_status_t file_read(struct file_handle *fh, u64 *buffer_size,
		void *buffer)
{
	loff_t actread;

	if (fs_read(fh->path, (ulong)buffer, fh->offset,
		    *buffer_size, &actread))
		return EFI_DEVICE_ERROR;

	*buffer_size = actread;
	fh->offset += actread;

	return EFI_SUCCESS;
}

static efi_status_t dir_read(struct file_handle *fh, u64 *buffer_size,
		void *buffer)
{
	struct efi_file_info *info = buffer;
	struct fs_dirent *dent;
	unsigned int required_size;

	if (!fh->dirs) {
		assert(fh->offset == 0);
		fh->dirs = fs_opendir(fh->path);
		if (!fh->dirs)
			return EFI_DEVICE_ERROR;
	}

	/*
	 * So this is a bit awkward.  Since fs layer is stateful and we
	 * can't rewind an entry, in the EFI_BUFFER_TOO_SMALL case below
	 * we might have to return without consuming the dent.. so we
	 * have to stash it for next call.
	 */
	if (fh->dent) {
		dent = fh->dent;
		fh->dent = NULL;
	} else {
		dent = fs_readdir(fh->dirs);
	}


	if (!dent) {
		/* no more files in directory: */
		/* workaround shim.efi bug/quirk.. as find_boot_csv()
		 * loops through directory contents, it initially calls
		 * read w/ zero length buffer to find out how much mem
		 * to allocate for the EFI_FILE_INFO, then allocates,
		 * and then calls a 2nd time.  If we return size of
		 * zero the first time, it happily passes that to
		 * AllocateZeroPool(), and when that returns NULL it
		 * thinks it is EFI_OUT_OF_RESOURCES.  So on first
		 * call return a non-zero size:
		 */
		if (*buffer_size == 0)
			*buffer_size = sizeof(*info);
		else
			*buffer_size = 0;
		return EFI_SUCCESS;
	}

	/* check buffer size: */
	required_size = sizeof(*info) + 2 * (strlen(dent->name) + 1);
	if (*buffer_size < required_size) {
		*buffer_size = required_size;
		fh->dent = dent;
		return EFI_BUFFER_TOO_SMALL;
	}

	*buffer_size = required_size;
	memset(info, 0, required_size);

	info->size = required_size;
	info->file_size = dent->size;
	info->physical_size = dent->size;

	if (dent->type == FS_DT_DIR)
		info->attribute |= EFI_FILE_DIRECTORY;

	ascii2unicode((u16 *)info->file_name, dent->name);

	fh->offset++;

	return EFI_SUCCESS;
}

static efi_status_t EFIAPI efi_file_read(struct efi_file_handle *file,
					 efi_uintn_t *buffer_size, void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;
	u64 bs;

	EFI_ENTRY("%p, %p, %p", file, buffer_size, buffer);

	if (!buffer_size || !buffer) {
		ret = EFI_INVALID_PARAMETER;
		goto error;
	}

	if (set_blk_dev(fh)) {
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	bs = *buffer_size;
	if (fh->isdir)
		ret = dir_read(fh, &bs, buffer);
	else
		ret = file_read(fh, &bs, buffer);
	if (bs <= SIZE_MAX)
		*buffer_size = bs;
	else
		*buffer_size = SIZE_MAX;

error:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_write(struct efi_file_handle *file,
					  efi_uintn_t *buffer_size,
					  void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;
	loff_t actwrite;

	EFI_ENTRY("%p, %p, %p", file, buffer_size, buffer);

	if (set_blk_dev(fh)) {
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	if (fs_write(fh->path, (ulong)buffer, fh->offset, *buffer_size,
		     &actwrite)) {
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	*buffer_size = actwrite;
	fh->offset += actwrite;

error:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_getpos(struct efi_file_handle *file,
					   efi_uintn_t *pos)
{
	struct file_handle *fh = to_fh(file);

	EFI_ENTRY("%p, %p", file, pos);

	if (fh->offset <= SIZE_MAX) {
		*pos = fh->offset;
		return EFI_EXIT(EFI_SUCCESS);
	} else {
		return EFI_EXIT(EFI_DEVICE_ERROR);
	}
}

static efi_status_t EFIAPI efi_file_setpos(struct efi_file_handle *file,
		efi_uintn_t pos)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %zu", file, pos);

	if (fh->isdir) {
		if (pos != 0) {
			ret = EFI_UNSUPPORTED;
			goto error;
		}
		fs_closedir(fh->dirs);
		fh->dirs = NULL;
	}

	if (pos == ~0ULL) {
		loff_t file_size;

		if (set_blk_dev(fh)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		if (fs_size(fh->path, &file_size)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		pos = file_size;
	}

	fh->offset = pos;

error:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_getinfo(struct efi_file_handle *file,
					    const efi_guid_t *info_type,
					    efi_uintn_t *buffer_size,
					    void *buffer)
{
	struct file_handle *fh = to_fh(file);
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %p, %p, %p", file, info_type, buffer_size, buffer);

	if (!guidcmp(info_type, &efi_file_info_guid)) {
		struct efi_file_info *info = buffer;
		char *filename = basename(fh);
		unsigned int required_size;
		loff_t file_size;

		/* check buffer size: */
		required_size = sizeof(*info) + 2 * (strlen(filename) + 1);
		if (*buffer_size < required_size) {
			*buffer_size = required_size;
			ret = EFI_BUFFER_TOO_SMALL;
			goto error;
		}

		if (set_blk_dev(fh)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		if (fs_size(fh->path, &file_size)) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}

		memset(info, 0, required_size);

		info->size = required_size;
		info->file_size = file_size;
		info->physical_size = file_size;

		if (fh->isdir)
			info->attribute |= EFI_FILE_DIRECTORY;

		ascii2unicode((u16 *)info->file_name, filename);
	} else if (!guidcmp(info_type, &efi_file_system_info_guid)) {
		struct efi_file_system_info *info = buffer;
		disk_partition_t part;
		efi_uintn_t required_size;
		int r;

		if (fh->fs->part >= 1)
			r = part_get_info(fh->fs->desc, fh->fs->part, &part);
		else
			r = part_get_info_whole_disk(fh->fs->desc, &part);
		if (r < 0) {
			ret = EFI_DEVICE_ERROR;
			goto error;
		}
		required_size = sizeof(info) + 2 *
				(strlen((const char *)part.name) + 1);
		if (*buffer_size < required_size) {
			*buffer_size = required_size;
			ret = EFI_BUFFER_TOO_SMALL;
			goto error;
		}

		memset(info, 0, required_size);

		info->size = required_size;
		info->read_only = true;
		info->volume_size = part.size * part.blksz;
		info->free_space = 0;
		info->block_size = part.blksz;
		/*
		 * TODO: The volume label is not available in U-Boot.
		 * Use the partition name as substitute.
		 */
		ascii2unicode((u16 *)info->volume_label,
			      (const char *)part.name);
	} else {
		ret = EFI_UNSUPPORTED;
	}

error:
	return EFI_EXIT(ret);
}

static efi_status_t EFIAPI efi_file_setinfo(struct efi_file_handle *file,
					    const efi_guid_t *info_type,
					    efi_uintn_t buffer_size,
					    void *buffer)
{
	EFI_ENTRY("%p, %p, %zu, %p", file, info_type, buffer_size, buffer);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_file_flush(struct efi_file_handle *file)
{
	EFI_ENTRY("%p", file);
	return EFI_EXIT(EFI_SUCCESS);
}

static const struct efi_file_handle efi_file_handle_protocol = {
	.rev = EFI_FILE_PROTOCOL_REVISION,
	.open = efi_file_open,
	.close = efi_file_close,
	.delete = efi_file_delete,
	.read = efi_file_read,
	.write = efi_file_write,
	.getpos = efi_file_getpos,
	.setpos = efi_file_setpos,
	.getinfo = efi_file_getinfo,
	.setinfo = efi_file_setinfo,
	.flush = efi_file_flush,
};

struct efi_file_handle *efi_file_from_path(struct efi_device_path *fp)
{
	struct efi_simple_file_system_protocol *v;
	struct efi_file_handle *f;
	efi_status_t ret;

	v = efi_fs_from_path(fp);
	if (!v)
		return NULL;

	EFI_CALL(ret = v->open_volume(v, &f));
	if (ret != EFI_SUCCESS)
		return NULL;

	/* skip over device-path nodes before the file path: */
	while (fp && !EFI_DP_TYPE(fp, MEDIA_DEVICE, FILE_PATH))
		fp = efi_dp_next(fp);

	while (fp) {
		struct efi_device_path_file_path *fdp =
			container_of(fp, struct efi_device_path_file_path, dp);
		struct efi_file_handle *f2;

		if (!EFI_DP_TYPE(fp, MEDIA_DEVICE, FILE_PATH)) {
			printf("bad file path!\n");
			f->close(f);
			return NULL;
		}

		EFI_CALL(ret = f->open(f, &f2, (s16 *)fdp->str,
				       EFI_FILE_MODE_READ, 0));
		if (ret != EFI_SUCCESS)
			return NULL;

		fp = efi_dp_next(fp);

		EFI_CALL(f->close(f));
		f = f2;
	}

	return f;
}

static efi_status_t EFIAPI
efi_open_volume(struct efi_simple_file_system_protocol *this,
		struct efi_file_handle **root)
{
	struct file_system *fs = to_fs(this);

	EFI_ENTRY("%p, %p", this, root);

	*root = file_open(fs, NULL, NULL, 0);

	return EFI_EXIT(EFI_SUCCESS);
}

struct efi_simple_file_system_protocol *
efi_simple_file_system(struct blk_desc *desc, int part,
		       struct efi_device_path *dp)
{
	struct file_system *fs;

	fs = calloc(1, sizeof(*fs));
	fs->base.rev = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
	fs->base.open_volume = efi_open_volume;
	fs->desc = desc;
	fs->part = part;
	fs->dp = dp;

	return &fs->base;
}
