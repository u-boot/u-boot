// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google
 */

#include <common.h>

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_SPL_BUILD
/* provided to defeat compiler optimisation in board_init_f() */
void gru_dummy_function(int i)
{
}

int board_early_init_f(void)
{
# ifdef CONFIG_TARGET_CHROMEBOOK_BOB
	int sum, i;

	/*
	 * Add a delay and ensure that the compiler does not optimise this out.
	 * This is needed since the power rails tail a while to turn on, and
	 * we get garbage serial output otherwise.
	 */
	sum = 0;
	for (i = 0; i < 150000; i++)
		sum += i;
	gru_dummy_function(sum);
#endif /* CONFIG_TARGET_CHROMEBOOK_BOB */

	return 0;
}
#endif
