// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for qfw command
 *
 * Copyright 2025 Simon Glass <sjg@chromium.org>
 */

#include <console.h>
#include <dm.h>
#include <mapmem.h>
#include <qfw.h>
#include <dm/test.h>
#include <test/cmd.h>
#include <test/ut.h>

/* Test 'qfw list' command */
static int cmd_test_qfw_list(struct unit_test_state *uts)
{
	struct fw_cfg_file_iter iter;
	struct fw_file *file;
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_QFW, &dev));

	ut_assertok(run_command("qfw list", 0));
	ut_assert_nextline("            Addr     Size Sel Name");
	ut_assert_nextlinen("--");

	for (file = qfw_file_iter_init(dev, &iter); !qfw_file_iter_end(&iter);
	     file = qfw_file_iter_next(&iter)) {
		ut_assert_nextline("%16lx %8x %3x %-48s", file->addr,
				   be32_to_cpu(file->cfg.size),
				   be16_to_cpu(file->cfg.select),
				   file->cfg.name);
	}
	ut_assert_console_end();

	return 0;
}
CMD_TEST(cmd_test_qfw_list, UTF_CONSOLE);
