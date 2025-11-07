// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Tests for acpi command
 */

#include <linux/bitops.h>
#include <test/cmd.h>
#include <test/ut.h>

#define HAVE_RSDP BIT(0)
#define HAVE_XSDT BIT(1)
#define HAVE_FADT BIT(2)
#define HAVE_ALL (HAVE_RSDP | HAVE_XSDT | HAVE_FADT)

/**
 * cmd_test_acpi() - test the acpi command
 */
static int cmd_test_acpi(struct unit_test_state *uts)
{
	unsigned int actual = 0;
	int ret;

	/*
	 * Check that some mandatory ACPI tables exist:
	 * - RSDP
	 * - RSDT or XSDT
	 * - FADT
	 */
	ut_assertok(run_commandf("acpi list"));
	ut_assert_nextline("Name              Base   Size  Detail");
	ut_assert_nextline("----  ----------------  -----  ----------------------------");
	for (;;) {
		ret = console_record_readline(uts->actual_str, sizeof(uts->actual_str));
		if (ret == -ENOENT) {
			ut_asserteq(HAVE_ALL, actual);

			return 0;
		}
		if (ret < 0)
			ut_asserteq(0, ret);

		if (!strncmp("RSDP", uts->actual_str, 4))
			actual |= HAVE_RSDP;
		else if (!strncmp("RSDT", uts->actual_str, 4))
			actual |= HAVE_XSDT;
		else if (!strncmp("XSDT", uts->actual_str, 4))
			actual |= HAVE_XSDT;
		else if (!strncmp("FACP", uts->actual_str, 4))
			actual |= HAVE_FADT;
	}
}
CMD_TEST(cmd_test_acpi, UTF_CONSOLE);
