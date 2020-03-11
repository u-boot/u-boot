/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 */

#ifndef _SYSFW_LOADER_H_
#define _SYSFW_LOADER_H_

void k3_sysfw_loader(void (*config_pm_pre_callback)(void), void (*config_pm_done_callback)(void));

#endif
