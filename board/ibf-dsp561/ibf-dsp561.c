/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2009 I-SYST.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: I-SYST IBF-DSP561 Micromodule\n");
	printf("       Support: http://www.i-syst.com/\n");
	return 0;
}
