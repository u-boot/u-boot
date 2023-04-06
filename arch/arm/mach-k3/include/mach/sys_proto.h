/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

void k3_spl_init(void);
void k3_mem_init(void);
bool check_rom_loaded_sysfw(void);
#endif
