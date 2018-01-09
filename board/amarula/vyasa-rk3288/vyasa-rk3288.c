/*
 * Copyright (C) 2017 Amarula Solutions
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>

#ifndef CONFIG_TPL_BUILD
#include <spl.h>

int spl_start_uboot(void)
{
        /* break into full u-boot on 'c' */
        if (serial_tstc() && serial_getc() == 'c')
                return 1;

        return 0;
}
#endif
