/*
 * file.c
 *
 * Mini "VFS" by Marcus Sundberg
 *
 * 2002-07-28 - rjones@nexus-tech.net - ported to ppcboot v1.1.6
 * 2003-03-10 - kharris@nexus-tech.net - ported to uboot
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <malloc.h>
#include <fat.h>
#include <linux/stat.h>
#include <linux/time.h>

/* Supported filesystems */
static const struct filesystem filesystems[] = {
	{ file_fat_detectfs,  file_fat_ls,  file_fat_read,  "FAT" },
};
#define NUM_FILESYS	(sizeof(filesystems)/sizeof(struct filesystem))

/* The filesystem which was last detected */
static int current_filesystem = FSTYPE_NONE;

/* The current working directory */
#define CWD_LEN		511
char file_cwd[CWD_LEN+1] = "/";

const char *
file_getfsname(int idx)
{
	if (idx < 0 || idx >= NUM_FILESYS)
		return NULL;

	return filesystems[idx].name;
}

static void
pathcpy(char *dest, const char *src)
{
	char *origdest = dest;

	do {
		if (dest-file_cwd >= CWD_LEN) {
			*dest = '\0';
			return;
		}
		*(dest) = *(src);
		if (*src == '\0') {
			if (dest-- != origdest && ISDIRDELIM(*dest)) {
				*dest = '\0';
			}
			return;
		}
		++dest;

		if (ISDIRDELIM(*src))
			while (ISDIRDELIM(*src)) src++;
		else
			src++;
	} while (1);
}

int
file_cd(const char *path)
{
	if (ISDIRDELIM(*path)) {
		while (ISDIRDELIM(*path)) path++;
		strncpy(file_cwd+1, path, CWD_LEN-1);
	} else {
		const char *origpath = path;
		char *tmpstr = file_cwd;
		int back = 0;

		while (*tmpstr != '\0') tmpstr++;
		do {
			tmpstr--;
		} while (ISDIRDELIM(*tmpstr));

		while (*path == '.') {
			path++;
			while (*path == '.') {
				path++;
				back++;
			}
			if (*path != '\0' && !ISDIRDELIM(*path)) {
				path = origpath;
				back = 0;
				break;
			}
			while (ISDIRDELIM(*path)) path++;
			origpath = path;
		}

		while (back--) {
			/* Strip off path component */
			while (!ISDIRDELIM(*tmpstr)) {
				tmpstr--;
			}
			if (tmpstr == file_cwd) {
				/* Incremented again right after the loop. */
				tmpstr--;
				break;
			}
			/* Skip delimiters */
			while (ISDIRDELIM(*tmpstr)) tmpstr--;
		}
		tmpstr++;
		if (*path == '\0') {
			if (tmpstr == file_cwd) {
				*tmpstr = '/';
				tmpstr++;
			}
			*tmpstr = '\0';
			return 0;
		}
		*tmpstr = '/';
		pathcpy(tmpstr+1, path);
	}

	return 0;
}

int
file_detectfs(void)
{
	int i;

	current_filesystem = FSTYPE_NONE;

	for (i = 0; i < NUM_FILESYS; i++) {
		if (filesystems[i].detect() == 0) {
			strcpy(file_cwd, "/");
			current_filesystem = i;
			break;
		}
	}

	return current_filesystem;
}

int
file_ls(const char *dir)
{
	char fullpath[1024];
	const char *arg;

	if (current_filesystem == FSTYPE_NONE) {
		printf("Can't list files without a filesystem!\n");
		return -1;
	}

	if (ISDIRDELIM(*dir)) {
		arg = dir;
	} else {
		sprintf(fullpath, "%s/%s", file_cwd, dir);
		arg = fullpath;
	}
	return filesystems[current_filesystem].ls(arg);
}

long
file_read(const char *filename, void *buffer, unsigned long maxsize)
{
	char fullpath[1024];
	const char *arg;

	if (current_filesystem == FSTYPE_NONE) {
		printf("Can't load file without a filesystem!\n");
		return -1;
	}

	if (ISDIRDELIM(*filename)) {
		arg = filename;
	} else {
		sprintf(fullpath, "%s/%s", file_cwd, filename);
		arg = fullpath;
	}

	return filesystems[current_filesystem].read(arg, buffer, maxsize);
}
