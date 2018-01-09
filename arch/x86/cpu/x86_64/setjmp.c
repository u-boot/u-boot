/*
 * Copyright (c) 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/setjmp.h>

int setjmp(struct jmp_buf_data *jmp_buf)
{
	printf("WARNING: setjmp() is not supported\n");

	return 0;
}

void longjmp(struct jmp_buf_data *jmp_buf, int val)
{
	printf("WARNING: longjmp() is not supported\n");
}
