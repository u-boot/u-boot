// SPDX-License-Identifier: GPL-2.0+
/*
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 */
#include <linux/types.h>
#include <i2c.h>
#include <init.h>
#include "../../common/br_resetc.h"
#include "../../common/bur_common.h"

int board_boot_key(void)
{
	unsigned char u8buf = 0;
	int rc;

	rc = br_resetc_regget(RSTCTRL_ENHSTATUS, &u8buf);
	if (rc == 0)
		return (u8buf & 0x1);

	return 0;
}

#if defined(CONFIG_SPL_BUILD)
int br_board_late_init(void)
{
	brdefaultip_setup(0, 0x57);

	return 0;
}
#endif
