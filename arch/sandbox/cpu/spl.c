// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <hang.h>
#include <handoff.h>
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

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOARD;
}

static int spl_board_load_image(struct spl_image_info *spl_image,
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

	return 0;
}
SPL_LOAD_IMAGE_METHOD("sandbox", 9, BOOT_DEVICE_BOARD, spl_board_load_image);

void spl_board_init(void)
{
	struct sandbox_state *state = state_get_current();

	if (state->run_unittests) {
		struct unit_test *tests = UNIT_TEST_ALL_START();
		const int count = UNIT_TEST_ALL_COUNT();
		int ret;

		ret = ut_run_list("spl", NULL, tests, count,
				  state->select_unittests);
		/* continue execution into U-Boot */
	}
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	const char *fname = spl_image->arg;

	if (fname) {
		os_fd_restore();
		os_spl_to_uboot(fname);
	} else {
		printf("No filename provided for U-Boot\n");
	}
	hang();
}

int handoff_arch_save(struct spl_handoff *ho)
{
	ho->arch.magic = TEST_HANDOFF_MAGIC;

	return 0;
}
