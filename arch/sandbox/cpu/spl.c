// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <dm.h>
#include <hang.h>
#include <handoff.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <os.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/spl.h>
#include <asm/state.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

int sandbox_find_next_phase(char *fname, int maxlen, bool use_img)
{
	const char *cur_prefix, *next_prefix;
	int ret;

	cur_prefix = spl_phase_prefix(spl_phase());
	next_prefix = spl_phase_prefix(spl_next_phase());
	ret = os_find_u_boot(fname, maxlen, use_img, cur_prefix, next_prefix);
	if (ret)
		return log_msg_ret("find", ret);

	return 0;
}

/* SPL / TPL / VPL init function */
void board_init_f(ulong flag)
{
	struct sandbox_state *state = state_get_current();
	int ret;

	gd->arch.ram_buf = state->ram_buf;
	gd->ram_size = state->ram_size;

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}
	preloader_console_init();
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = BOOT_DEVICE_VBE;
	spl_boot_list[1] = BOOT_DEVICE_BOARD;
}

static int spl_board_load_file(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	char fname[256];
	int ret;

	ret = sandbox_find_next_phase(fname, sizeof(fname), false);
	if (ret) {
		printf("(%s not found, error %d)\n", fname, ret);
		return ret;
	}

	/*
	 * Set up spl_image to boot from jump_to_image_no_args(). Allocate this
	 * outsdide the RAM buffer (i.e. don't use strdup()).
	 */
	spl_image->arg = os_malloc(strlen(fname) + 1);
	if (!spl_image->arg)
		return log_msg_ret("exec", -ENOMEM);
	strcpy(spl_image->arg, fname);
	spl_image->flags = SPL_SANDBOXF_ARG_IS_FNAME;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("sandbox_file", 9, BOOT_DEVICE_BOARD,
		      spl_board_load_file);

static int load_from_image(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev)
{
	struct sandbox_state *state = state_get_current();
	enum u_boot_phase next_phase;
	const char *fname;
	ulong pos, size;
	int full_size;
	void *buf;
	int ret;

	if (!IS_ENABLED(CONFIG_SANDBOX_VPL))
		return -ENOENT;

	next_phase = spl_next_phase();
	pos = spl_get_image_pos();
	size = spl_get_image_size();
	if (pos == BINMAN_SYM_MISSING || size == BINMAN_SYM_MISSING) {
		log_debug("No image found\n");
		return -ENOENT;
	}
	log_info("Reading from pos %lx size %lx\n", pos, size);

	/*
	 * Set up spl_image to boot from jump_to_image_no_args(). Allocate this
	 * outside the RAM buffer (i.e. don't use strdup()).
	 */
	fname = state->prog_fname ? state->prog_fname : state->argv[0];
	ret = os_read_file(fname, &buf, &full_size);
	if (ret)
		return log_msg_ret("rd", -ENOMEM);
	spl_image->flags = SPL_SANDBOXF_ARG_IS_BUF;
	spl_image->arg = buf;
	spl_image->offset = pos;
	spl_image->size = size;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("sandbox_image", 7, BOOT_DEVICE_BOARD, load_from_image);

void spl_board_init(void)
{
	struct sandbox_state *state = state_get_current();

	if (!CONFIG_IS_ENABLED(UNIT_TEST))
		return;

	/* These are necessary so TFTP can use LMBs to check its load address */
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = get_effective_memsize();

	if (state->run_unittests) {
		struct unit_test *tests = UNIT_TEST_ALL_START();
		const int count = UNIT_TEST_ALL_COUNT();
		int ret;

		ret = ut_run_list("spl", NULL, tests, count,
				  state->select_unittests, 1, false, NULL);
		/* continue execution into U-Boot */
	}
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	switch (spl_image->flags) {
	case SPL_SANDBOXF_ARG_IS_FNAME: {
		const char *fname = spl_image->arg;

		if (fname) {
			os_fd_restore();
			os_spl_to_uboot(fname);
		} else {
			log_err("No filename provided for U-Boot\n");
		}
		break;
	}
	case SPL_SANDBOXF_ARG_IS_BUF: {
		int ret;

		ret = os_jump_to_image(spl_image->arg + spl_image->offset,
				       spl_image->size);
		if (ret)
			log_err("Failed to load image\n");
		break;
	}
	default:
		log_err("Invalid flags\n");
		break;
	}
	hang();
}

int handoff_arch_save(struct spl_handoff *ho)
{
	ho->arch.magic = TEST_HANDOFF_MAGIC;

	return 0;
}

/* Context used to hold file descriptor */
struct load_ctx {
	int fd;
};

static ulong read_fit_image(struct spl_load_info *load, ulong offset,
			    ulong size, void *buf)
{
	struct load_ctx *load_ctx = load->priv;
	off_t ret;
	ssize_t res;

	ret = os_lseek(load_ctx->fd, offset, OS_SEEK_SET);
	if (ret < 0) {
		printf("Failed to seek to %zx, got %zx (errno=%d)\n", offset,
		       ret, errno);
		return log_msg_ret("lse", -errno);
	}

	res = os_read(load_ctx->fd, buf, size);
	if (res < 0) {
		printf("Failed to read %lx bytes, got %ld (errno=%d)\n",
		       size, res, errno);
		return log_msg_ret("osr", -errno);
	}

	return size;
}

int sandbox_spl_load_fit(char *fname, int maxlen, struct spl_image_info *image)
{
	struct legacy_img_hdr *header;
	struct load_ctx load_ctx;
	struct spl_load_info load;
	int ret;
	int fd;

	memset(&load, '\0', sizeof(load));
	spl_set_bl_len(&load, IS_ENABLED(CONFIG_SPL_LOAD_BLOCK) ? 512 : 1);
	load.read = read_fit_image;

	ret = sandbox_find_next_phase(fname, maxlen, true);
	if (ret) {
		printf("%s not found, error %d\n", fname, ret);
		return log_msg_ret("nph", ret);
	}

	header = spl_get_load_buffer(-sizeof(*header), sizeof(*header));

	log_debug("reading from %s\n", fname);
	fd = os_open(fname, OS_O_RDONLY);
	if (fd < 0) {
		printf("Failed to open '%s'\n", fname);
		return log_msg_ret("ope", -errno);
	}
	ret = os_read(fd, header, sizeof(*header));
	if (ret != sizeof(*header)) {
		printf("Failed to read %lx bytes, got %ld (errno=%d)\n",
		       sizeof(*header), ret, -errno);
		return log_msg_ret("rea", -errno);
	}
	load_ctx.fd = fd;

	load.priv = &load_ctx;

	ret = spl_load_simple_fit(image, &load, 0, header);
	if (ret)
		return log_msg_ret("slf", ret);

	return 0;
}
