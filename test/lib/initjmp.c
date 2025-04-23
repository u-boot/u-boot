// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2025 Linaro Limited
 *
 * Unit test for initjmp()
 */

#include <compiler.h>
#include <setjmp.h>
#include <stdbool.h>
#include <test/lib.h>
#include <test/ut.h>
#include <vsprintf.h>

static bool ep_entered;
static jmp_buf return_buf;

static void __noreturn entrypoint(void)
{
	ep_entered = true;

	/* Jump back to the main routine */
	longjmp(return_buf, 1);

	/* Not reached */
	panic("longjmp failed\n");
}

static int lib_initjmp(struct unit_test_state *uts)
{
	int ret;
	void *stack;
	jmp_buf buf;
	/* Arbitrary but smaller values (< page size?) fail on SANDBOX */
	size_t stack_sz = 8192;

	(void)entrypoint;

	ep_entered = false;

	stack = malloc(stack_sz);
	ut_assertnonnull(stack);

	/*
	 * Prepare return_buf so that entrypoint may jump back just after the
	 * if()
	 */
	if (!setjmp(return_buf)) {
		/* return_buf initialized, entrypoint not yet called */

		/*
		 * Prepare another jump buffer to jump into entrypoint with the
		 * given stack
		 */
		ret = initjmp(buf, entrypoint, stack, stack_sz);
		ut_assertok(ret);

		/* Jump into entrypoint */
		longjmp(buf, 1);
		/*
		 * Not reached since entrypoint is expected to branch after
		 * the if()
		 */
		ut_assert(false);
	}

	ut_assert(ep_entered);

	free(stack);

	return 0;
}
LIB_TEST(lib_initjmp, 0);
