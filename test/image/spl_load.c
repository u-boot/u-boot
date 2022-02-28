// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <image.h>
#include <mapmem.h>
#include <os.h>
#include <spl.h>
#include <test/ut.h>

/* Declare a new SPL test */
#define SPL_TEST(_name, _flags)		UNIT_TEST(_name, _flags, spl_test)

/* Context used for this test */
struct text_ctx {
	int fd;
};

static ulong read_fit_image(struct spl_load_info *load, ulong sector,
			    ulong count, void *buf)
{
	struct text_ctx *text_ctx = load->priv;
	off_t offset, ret;
	ssize_t res;

	offset = sector * load->bl_len;
	ret = os_lseek(text_ctx->fd, offset, OS_SEEK_SET);
	if (ret != offset) {
		printf("Failed to seek to %zx, got %zx (errno=%d)\n", offset,
		       ret, errno);
		return 0;
	}

	res = os_read(text_ctx->fd, buf, count * load->bl_len);
	if (res == -1) {
		printf("Failed to read %lx bytes, got %ld (errno=%d)\n",
		       count * load->bl_len, res, errno);
		return 0;
	}

	return count;
}

int board_fit_config_name_match(const char *name)
{
	return 0;
}

struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return map_sysmem(0x100000, 0);
}

static int spl_test_load(struct unit_test_state *uts)
{
	struct spl_image_info image;
	struct image_header *header;
	struct text_ctx text_ctx;
	struct spl_load_info load;
	char fname[256];
	int ret;
	int fd;

	memset(&load, '\0', sizeof(load));
	load.bl_len = 512;
	load.read = read_fit_image;

	ret = sandbox_find_next_phase(fname, sizeof(fname), true);
	if (ret) {
		printf("(%s not found, error %d)\n", fname, ret);
		return ret;
	}
	load.filename = fname;

	header = spl_get_load_buffer(-sizeof(*header), sizeof(*header));

	fd = os_open(fname, OS_O_RDONLY);
	ut_assert(fd >= 0);
	ut_asserteq(512, os_read(fd, header, 512));
	text_ctx.fd = fd;

	load.priv = &text_ctx;

	ut_assertok(spl_load_simple_fit(&image, &load, 0, header));

	return 0;
}
SPL_TEST(spl_test_load, 0);
