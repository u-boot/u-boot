// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Linaro Limited
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/libfdt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mkimage.h"

#define ESL_SIG_NODENAME	"signature"

static int get_esl_pub_key(const char *esl_file, void **key_ptr, int *key_fd,
			   off_t *key_size)
{
	int ret;
	struct stat pub_key;

	debug("%s: esl file => %s\n", __func__, esl_file);
	*key_fd = open(esl_file, O_RDONLY);
	if (*key_fd == -1) {
		fprintf(stderr, "Unable to open %s: %s\n",
			esl_file, strerror(errno));
		return -EACCES;
	}

	ret = fstat(*key_fd, &pub_key);
	if (ret == -1) {
		fprintf(stderr, "Can't stat %s: %s\n",
			esl_file, strerror(errno));
		ret = errno;
		goto err;
	}
	*key_size = pub_key.st_size;

	/* mmap the public key esl file */
	*key_ptr = mmap(0, *key_size, PROT_READ, MAP_SHARED, *key_fd, 0);
	if ((*key_ptr == MAP_FAILED) || (errno != 0)) {
		fprintf(stderr, "Failed to mmap %s:%s\n",
			esl_file, strerror(errno));
		ret = errno;
		goto err;
	}

	return 0;
err:
	close(*key_fd);

	return ret;
}

int fdt_embed_esl(const char *esl_file, void *keydest)
{
	int ret, key_fd;
	off_t key_size = 0;
	void *key_ptr = NULL;
	int parent;

	ret = get_esl_pub_key(esl_file, &key_ptr, &key_fd, &key_size);
	if (ret) {
		debug("Unable to open the public key esl file\n");
		goto out;
	}

	parent = fdt_subnode_offset(keydest, 0, ESL_SIG_NODENAME);
	if (parent == -FDT_ERR_NOTFOUND) {
		parent = fdt_add_subnode(keydest, 0, ESL_SIG_NODENAME);
		if (parent < 0) {
			ret = parent;
			if (ret != -FDT_ERR_NOSPACE) {
				fprintf(stderr, "Couldn't create signature node: %s\n",
					fdt_strerror(parent));
			}
		}
	}

	if (ret)
		goto out;

	ret = fdt_setprop(keydest, parent, "capsule-key",
			  key_ptr, key_size);

out:
	close(key_fd);
	munmap(key_ptr, key_size);

	if (ret)
		return ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;

	return ret;
}
