/*
 * (C) Copyright 2000
 * Subodh Nijsure, SkyStream Networks, snijsure@skystream.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#if defined(CONFIG_8xx)
void mpc8xx_reginfo(void);
#elif defined(CONFIG_MPC86xx)
extern void mpc86xx_reginfo(void);
#elif defined(CONFIG_MPC85xx)
extern void mpc85xx_reginfo(void);
#endif

static int do_reginfo(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
#if defined(CONFIG_8xx)
	mpc8xx_reginfo();

#elif defined(CONFIG_MPC86xx)
	mpc86xx_reginfo();

#elif defined(CONFIG_MPC85xx)
	mpc85xx_reginfo();
#endif

	return 0;
}

 /**************************************************/

#if defined(CONFIG_CMD_REGINFO)
U_BOOT_CMD(
	reginfo,	2,	1,	do_reginfo,
	"print register information",
	""
);
#endif
